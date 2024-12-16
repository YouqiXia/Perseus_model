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
        renaming_stage_queue_depth_(p->queue_depth),
        renaming_stage_queue_(),
        free_list_("free_list", p->phy_reg_num)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(RenamingStage, Startup_));
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
        Rob_cmt_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(RenamingStage, RobCommit_, InstGroupPtr));
    }

    void RenamingStage::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());
        InitCredit_();
        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void RenamingStage::InitCredit_() {
        renaming_preceding_credit_out.send(renaming_stage_queue_depth_, sparta::Clock::Cycle(1));
    }

    void RenamingStage::AcceptRobCredit_(const Credit& credit) {
        rob_credit_ += credit;

        ILOG("RenamingStage get rob credits: " << credit << "total rob credit is: " << rob_credit_);

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptDispatchCredit_(const Credit& credit) {
        dispatch_credit_ += credit;

        ILOG("RenamingStage get dispatch credits: " << credit << ", total dispatch_credit_ = " << dispatch_credit_);

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptLoadQueueCredit_(const Credit& credit) {
        ldq_credit_ += credit;

        ILOG("RenamingStage get Ldq credits: " << credit << ", total ldq_credit_ = " << ldq_credit_);

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AcceptStoreQueueCredit_(const Credit& credit) {
        stq_credit_ += credit;

        ILOG("RenamingStage get Stq credits: " << credit << ", total stq_credit_ = " << stq_credit_);

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("renaming stage get instructions: " << inst_ptr);
            renaming_stage_queue_.push_back(inst_ptr);
        }

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void RenamingStage::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG("RenamingStage is flushed");

        dispatch_credit_ = 0;
        rob_credit_ = 0;
        ldq_credit_ = 0;
        stq_credit_ = 0;

        renaming_preceding_credit_out.send(renaming_stage_queue_depth_, sparta::Clock::Cycle(1));
        renaming_stage_queue_.clear();

        free_list_.RollBack();
        renaming_table_.RollBack();
    }

    void RenamingStage::ProcessInst_() {
        uint64_t produce_num_max = std::min<uint64_t>(renaming_stage_queue_.size(), issue_width_);
        uint64_t produce_num = GetProduceNum_();
        PmuBandwidthAcc_(produce_num_max, produce_num);

        InstGroupPtr processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        InstGroupPtr lsu_processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        RenameInst_(produce_num, processed_group_ptr, lsu_processed_group_ptr);

        CheckRegStatus_(processed_group_ptr);

        DataTransfer_(processed_group_ptr, lsu_processed_group_ptr);
    }

    uint64_t RenamingStage::GetProduceNum_() {
        uint64_t produce_num = std::min<uint64_t>(dispatch_credit_, rob_credit_);
        produce_num = std::min<uint64_t>(produce_num, renaming_stage_queue_.size());
        produce_num = std::min<uint64_t>(produce_num, issue_width_);
        return produce_num;
    }

    void RenamingStage::PmuBandwidthAcc_(uint64_t produce_num_max, uint64_t produce_num) {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "event", 1);

        if (renaming_stage_queue_.size() < issue_width_) {
            pmu_->Monitor(getName(), "queue loss", issue_width_ - renaming_stage_queue_.size());
        }
        if (renaming_stage_queue_.empty()) {
            pmu_->Monitor(getName(), "queue empty", 1);
            return;
        }

        if (produce_num < produce_num_max) {
            pmu_->Monitor(getName(), "total credit loss", produce_num_max - produce_num);
            if (rob_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "rob loss", produce_num_max - rob_credit_);
            }
            if (rob_credit_ == 0) {
                pmu_->Monitor(getName(), "rob full", 1);
            }
            if (dispatch_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "dispatch loss", produce_num_max - dispatch_credit_);
            }
            if (dispatch_credit_ == 0) {
                pmu_->Monitor(getName(), "dispatch full", 1);
            }
            if (ldq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "ldq loss", produce_num_max - ldq_credit_);
            }
            if (stq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "stq loss", produce_num_max - stq_credit_);
            }
        }
    }

    void RenamingStage::RenameInst_(uint64_t produce_num,
                                    TimingModel::InstGroupPtr processed_group_ptr,
                                    TimingModel::InstGroupPtr lsu_processed_group_ptr) {

        while(produce_num--) {
            auto inst_ptr = renaming_stage_queue_.front();
            if (!is_perfect_lsu_ && ldq_credit_ == 0 && inst_ptr->getFuType() == FuncType::STU) {
                pmu_->Monitor(getName(), "ldq loss", produce_num - ldq_credit_ + 1);
                break;
            }
            if (!is_perfect_lsu_ && stq_credit_ == 0 && inst_ptr->getFuType() == FuncType::LDU) {
                pmu_->Monitor(getName(), "stq loss", produce_num - stq_credit_ + 1);
                break;
            }

            if (!RenameInstImp_(inst_ptr)) {
                pmu_->Monitor(getName(), "freelist loss", produce_num + 1);
                pmu_->Monitor(getName(), "freelist empty", 1);
                break;
            }
            processed_group_ptr->emplace_back(inst_ptr);
            ILOG("send insn to following: " << inst_ptr);
            renaming_stage_queue_.pop_front();
            if (inst_ptr->getFuType() == FuncType::STU || inst_ptr->getFuType() == FuncType::LDU) {
                lsu_processed_group_ptr->emplace_back(inst_ptr);
            }

            --dispatch_credit_;
            --rob_credit_;
            if (inst_ptr->getFuType() == FuncType::STU && !is_perfect_lsu_) {
                stq_credit_--;
            }else if (inst_ptr->getFuType() == FuncType::LDU && !is_perfect_lsu_) {
                ldq_credit_--;
            }
        }

        pmu_->Monitor(getName(), "total loss", issue_width_ - processed_group_ptr->size());
    }

    bool RenamingStage::RenameInstImp_(const InstPtr& inst_ptr) {
        inst_ptr->setPhyRs1(renaming_table_[inst_ptr->getIsaRs1()]);
        inst_ptr->setPhyRs2(renaming_table_[inst_ptr->getIsaRs2()]);
        inst_ptr->setLPhyRd(renaming_table_[inst_ptr->getIsaRd()]);

        PhyRegId_t phy_reg_idx = free_list_.Front();
        if (inst_ptr->getIsaRd() != RegType_t::NONE && !free_list_.IsEmpty()) {
            renaming_table_[inst_ptr->getIsaRd()] = phy_reg_idx;
            inst_ptr->setPhyRd(phy_reg_idx);
            free_list_.Pop();
        } else if (free_list_.IsEmpty()) {
            return false;
        } else {
            inst_ptr->setPhyRd(0);
        }
        return true;
    }

    void RenamingStage::CheckRegStatus_(InstGroupPtr processed_group_ptr) {
        renaming_check_busy_out.send(processed_group_ptr);
    }

    void RenamingStage::DataTransfer_(TimingModel::InstGroupPtr processed_group_ptr,
                                          TimingModel::InstGroupPtr lsu_processed_group_ptr) {

        uint64_t whole_credit_ = std::min(dispatch_credit_, rob_credit_);
        if (!is_perfect_lsu_) {
            whole_credit_ = std::min(whole_credit_, ldq_credit_);
            whole_credit_ = std::min(whole_credit_, stq_credit_);
        }

        if (!renaming_stage_queue_.empty() && whole_credit_ > 0) {
            process_event.schedule(sparta::Clock::Cycle(1));
        }

        if (!processed_group_ptr->empty()) {
            renaming_preceding_credit_out.send(processed_group_ptr->size(), sparta::Clock::Cycle(1));
            renaming_following_inst_out.send(processed_group_ptr);
        }

        if (!lsu_processed_group_ptr->empty() && !is_perfect_lsu_) {
            renaming_lsu_allocate_out.send(lsu_processed_group_ptr);
        }

        ILOG(getName() << " queue size is after update: " << renaming_stage_queue_.size());
    }

    void RenamingStage::RobCommit_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() != 0) {
                free_list_.BackupPop();
            }
            if (inst_ptr->getLPhyRd() != 0) {
                free_list_.Push(inst_ptr->getLPhyRd());
                process_event.schedule(1);
            }
            renaming_table_.GetBackup(inst_ptr->getIsaRd()) = inst_ptr->getPhyRd();
        }
    }

    void RenamingStage::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        if (pmu_->IsTurnOffNextCycle()) {
            pmu_->TurnOff();
            return;
        }
        pmu_->Monitor(getName(), "pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "total loss", issue_width_);

        if (renaming_stage_queue_.size() < issue_width_) {
            pmu_->Monitor(getName(), "queue loss", issue_width_-renaming_stage_queue_.size());
        }
        if (renaming_stage_queue_.empty()) {
            pmu_->Monitor(getName(), "queue empty", 1);
            return;
        }

        uint64_t produce_num_max = std::min<uint64_t>(renaming_stage_queue_.size(), issue_width_);

        uint64_t produce_inst_num = std::min<uint64_t>(dispatch_credit_, rob_credit_);
        if (!is_perfect_lsu_) {
            produce_inst_num = std::min<uint64_t>(produce_inst_num, ldq_credit_);
            produce_inst_num = std::min<uint64_t>(produce_inst_num, stq_credit_);
        }

        if (produce_inst_num < produce_num_max) {
            pmu_->Monitor(getName(), "total credit loss", produce_num_max-produce_inst_num);
            if (rob_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "rob loss", produce_num_max-rob_credit_);
            }
            if (rob_credit_ == 0) {
                pmu_->Monitor(getName(), "rob full", 1);
            }
            if (dispatch_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "dispatch loss", produce_num_max-dispatch_credit_);
            }
            if (dispatch_credit_ == 0) {
                pmu_->Monitor(getName(), "dispatch full", 1);
            }
            if (ldq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "ldq loss", produce_num_max-ldq_credit_);
            }
            if (stq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "stq loss", produce_num_max-stq_credit_);
            }
        }
    }
}