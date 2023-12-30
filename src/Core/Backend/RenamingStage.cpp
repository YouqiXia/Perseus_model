#include "sparta/utils/LogUtils.hpp"

#include <cmath>

#include "RenamingStage.hpp"

namespace TimingModel {

    const char* RenamingStage::name = "renaming_stage";

    RenamingStage::RenamingStage(sparta::TreeNode* node, const RenamingParameter* p) :
        sparta::Unit(node),
        issue_width_(p->issue_width),
        renaming_table_(p->isa_reg_num, 0),
        renaming_stage_queue_
            ("renaming queue", p->renaming_stage_queue_depth, node->getClock(), &unit_stat_set_),
        free_list_("free list", p->free_list_depth, node->getClock(), &unit_stat_set_)
    {
        renaming_flush_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, HandleFlush, FlushingCriteria));
        preceding_renaming_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AllocateInst, InstGroupPtr));
        following_renaming_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, AcceptCredit, Credit));
        Rob_cmt_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, RobCommit_, InstGroupPtr));
    }

    void RenamingStage::AcceptCredit(const Credit& credit) {
        credit_ += credit;

        ILOG("RenamingStage get credits: " << credit);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AllocateInst(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            renaming_stage_queue_.push(inst_ptr);
        }

        // For pipeline, tick = 0. For queue, tick = 1.
        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::HandleFlush(const FlushingCriteria& flush_criteria) {
        rename_event.cancel();
        ILOG("RenamingStage is flushed");

        if (!renaming_stage_queue_.empty()) {
            renaming_preceding_credit_out.send(renaming_stage_queue_.size());
            renaming_stage_queue_.clear();
        }

        free_list_.RollBack();
        renaming_table_.RollBack();
    }

    void RenamingStage::RenameInst() {
        uint64_t produce_inst_num = std::min<uint64_t>(credit_, renaming_stage_queue_.size());
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);

        renaming_preceding_credit_out.send(produce_inst_num);

        while(produce_inst_num--) {
            auto inst_tmp_ptr = renaming_stage_queue_.front();
            RenameInstImp_(inst_tmp_ptr);
            inst_group_tmp_ptr->emplace_back(inst_tmp_ptr);
            renaming_stage_queue_.pop();
            --credit_;
        }

        if (!renaming_stage_queue_.empty()) {
            rename_event.schedule(sparta::Clock::Cycle(1));
        }

        renaming_following_inst_out.send(inst_group_tmp_ptr);
    }

    void RenamingStage::RenameInstImp_(const InstPtr& inst_ptr) {
        inst_ptr->setPhyRs1(renaming_table_[inst_ptr->getIsaRs1()]);
        inst_ptr->setPhyRs2(renaming_table_[inst_ptr->getIsaRs2()]);
        inst_ptr->setLPhyRd(renaming_table_[inst_ptr->getIsaRd()]);
        PhyRegId_t phy_reg_idx = free_list_.Front();
        if (inst_ptr->getIsaRd() != 0) {
            renaming_table_[inst_ptr->getIsaRd()] = phy_reg_idx;
            inst_ptr->setPhyRd(phy_reg_idx);
            free_list_.Pop();
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
            free_list_.BackupPop();
            free_list_.Push(inst_ptr->getLPhyRd());
            renaming_table_.GetBackup(inst_ptr->getIsaRd()) = inst_ptr->getPhyRd();
        }
    }
}