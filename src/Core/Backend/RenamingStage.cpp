#include "sparta/utils/LogUtils.hpp"

#include <cmath>
#include <iostream>

#include "RenamingStage.hpp"

namespace TimingModel {

    const char* RenamingStage::name = "renaming_stage";

    RenamingStage::RenamingStage(sparta::TreeNode* node, const RenamingParameter* p) :
        sparta::Unit(node),
        issue_width_(p->issue_width),
        renaming_table_(p->isa_reg_num, 0),
        is_perfect_lsu_(p->is_perfect_lsu),
        renaming_stage_queue_depth_(p->renaming_stage_queue_depth),
        renaming_stage_queue_
            ("rename_queue", p->renaming_stage_queue_depth, node->getClock(), &unit_stat_set_),
        free_list_("free_list", p->free_list_depth, node->getClock(), &unit_stat_set_)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(RenamingStage, InitCredit_));
        renaming_flush_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, HandleFlush_, FlushingCriteria));
        preceding_renaming_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AllocateInst_, InstGroupPtr));
        following_renaming_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptDispatchCredit_, Credit));
        rob_renaming_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptRobCredit_, Credit));
        lsu_renaming_ldq_credit_in.registerConsumerHandler
           (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptLoadQueueCredit_, Credit));
        lsu_renaming_stq_credit_in.registerConsumerHandler
           (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptStoreQueueCredit_, Credit));
        lsu_renaming_allocate_in.registerConsumerHandler
           (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptLsuAllocateIdx, InstGroupPtr));
        Rob_cmt_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, RobCommit_, InstGroupPtr));


        // preceding -> rob & dispatch stage
        following_renaming_credit_in >> sparta::GlobalOrderingPoint(node, "renaming_node");
        rob_renaming_credit_in >> sparta::GlobalOrderingPoint(node, "renaming_node");
        sparta::GlobalOrderingPoint(node, "renaming_node") >> rename_event;
    }

    RenamingStage::~RenamingStage() {
    }

    void RenamingStage::AcceptRobCredit_(const Credit& credit) {
        rob_credit_ += credit;

        ILOG("RenamingStage get rob credits: " << credit);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptDispatchCredit_(const Credit& credit) {
        dispatch_credit_ += credit;

        ILOG("RenamingStage get dispatch credits: " << credit << ", total dispatch_credit_ = " << dispatch_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptLoadQueueCredit_(const Credit& credit) {
        ldq_credit_ += credit;

        ILOG("RenamingStage get Ldq credits: " << credit << ", total ldq_credit_ = " << ldq_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptStoreQueueCredit_(const Credit& credit) {
        stq_credit_ += credit;

        ILOG("RenamingStage get Stq credits: " << credit << ", total stq_credit_ = " << stq_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptLsuAllocateIdx(const InstGroupPtr &inst_ptr) {}

    void RenamingStage::CreditDecrease_() {
        --dispatch_credit_;
        --rob_credit_;
        --ldq_credit_;
        --stq_credit_;
    }

    void RenamingStage::CreditWithoutLsuDecrease_() {
        --dispatch_credit_;
        --rob_credit_;
    }

    void RenamingStage::InitCredit_() {
        renaming_preceding_credit_out.send(renaming_stage_queue_depth_);
    }

    void RenamingStage::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        ILOG("renaming stage get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("renaming stage get instructions: " << inst_ptr);
            renaming_stage_queue_.push(inst_ptr);
        }

        // For pipeline, tick = 0. For queue, tick = 1.
        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::HandleFlush_(const FlushingCriteria& flush_criteria) {
        rename_event.cancel();
        ILOG("RenamingStage is flushed");

        if (!renaming_stage_queue_.empty()) {
            renaming_preceding_credit_out.send(renaming_stage_queue_.size());
            renaming_stage_queue_.clear();
        }

        free_list_.RollBack();
        renaming_table_.RollBack();
    }

    void RenamingStage::RenameInst_() {
        if (renaming_stage_queue_.empty() || free_list_.IsEmpty()) {
            return;
        }
        uint64_t produce_inst_num = std::min<uint64_t>(dispatch_credit_, rob_credit_);
        produce_inst_num = std::min<uint64_t>(produce_inst_num, ldq_credit_);
        produce_inst_num = std::min<uint64_t>(produce_inst_num, stq_credit_);
        produce_inst_num = std::min<uint64_t>(produce_inst_num, issue_width_);

        ILOG(getName() << " rename instructions, produce_inst_num = " << produce_inst_num 
             << ", dispatch_credit_ = " << dispatch_credit_ << ", rob_credit_ = " << rob_credit_
             << ", ldq_credit_ = " << ldq_credit_ << ", stq_credit_ = " << stq_credit_
             << ", issue_width_ = " << issue_width_);

        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        InstGroupPtr inst_lsu_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);

        if (produce_inst_num == 0) {
            return;
        }
        renaming_preceding_credit_out.send(produce_inst_num);

        while(produce_inst_num--) {
            if (renaming_stage_queue_.empty()) {
                break;
            }
            auto inst_tmp_ptr = renaming_stage_queue_.front();
            RenameInstImp_(inst_tmp_ptr);
            inst_group_tmp_ptr->emplace_back(inst_tmp_ptr);
            ILOG("send insn to following: " << inst_tmp_ptr);
            renaming_stage_queue_.pop();
            if (inst_tmp_ptr->getFuType() == FuncType::STU || inst_tmp_ptr->getFuType() == FuncType::LDU) {
                inst_lsu_group_tmp_ptr->emplace_back(inst_tmp_ptr);
            }

            CreditWithoutLsuDecrease_();
            if (!is_perfect_lsu_) {
                if (inst_tmp_ptr->getFuType() == FuncType::STU) {
                    stq_credit_--;
                }else if (inst_tmp_ptr->getFuType() == FuncType::LDU) {
                    ldq_credit_--;
                }
            }
        }

        uint64_t whole_credit_ = std::min(dispatch_credit_, rob_credit_);
        whole_credit_ = std::min(whole_credit_, ldq_credit_);
        whole_credit_ = std::min(whole_credit_, stq_credit_);

        if (!renaming_stage_queue_.empty() && whole_credit_ > 0) {
            rename_event.schedule(sparta::Clock::Cycle(1));
        }

        if (!inst_group_tmp_ptr->empty()) {
            renaming_following_inst_out.send(inst_group_tmp_ptr);
        }

        if (!inst_lsu_group_tmp_ptr->empty()) {
            ILOG("issue insn to lsu");
            renaming_lsu_allocate_out.send(inst_lsu_group_tmp_ptr);
        }

        ILOG(getName() << " queue size is after update: " << renaming_stage_queue_.size());

    }

    void RenamingStage::RenameInstImp_(const InstPtr& inst_ptr) {
        inst_ptr->setPhyRs1(renaming_table_[inst_ptr->getIsaRs1()]);
        inst_ptr->setPhyRs2(renaming_table_[inst_ptr->getIsaRs2()]);
        inst_ptr->setLPhyRd(renaming_table_[inst_ptr->getIsaRd()]);
        PhyRegId_t phy_reg_idx = free_list_.Front();
        if (inst_ptr->getIsaRd() != 0) {
            renaming_table_[inst_ptr->getIsaRd()] = phy_reg_idx;
            inst_ptr->setPhyRd(phy_reg_idx);
            ILOG("free list pop: " << free_list_.Front() << " rob tag: " << inst_ptr->getRobTag());
            free_list_.Pop();
        } else {
            inst_ptr->setPhyRd(0);
        }
    }

    void RenamingStage::RollBack_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            renaming_table_[inst_ptr->getIsaRd()] = inst_ptr->getLPhyRd();
            free_list_.Push(inst_ptr->getPhyRd());
        }
    }

    void RenamingStage::RobCommit_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() != 0) {
                free_list_.BackupPop();
            }
            if (inst_ptr->getLPhyRd() != 0) {
                free_list_.Push(inst_ptr->getLPhyRd());
                rename_event.schedule(1);
            }
            renaming_table_.GetBackup(inst_ptr->getIsaRd()) = inst_ptr->getPhyRd();
            ILOG("inst release resource: " << inst_ptr);
            ILOG("free list push: " << inst_ptr->getLPhyRd() << " rob tag: " << inst_ptr->getRobTag());
        }
    }
}