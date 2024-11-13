//
// Created by yzhang on 10/25/24.
//

#include "MAE1RenamingUnit.hpp"

namespace TimingModel {

    MAE1RenamingUnit::MAE1RenamingUnit(sparta::TreeNode* node, const MAE1RenamingParameter* p) :
            sparta::Unit(node),
            issue_width_(p->issue_width),
            group_num_(p->group_num),
            renaming_tables_(p->isa_reg_num, p->group_num),
            is_perfect_lsu_(p->is_perfect_lsu),
            renaming_stage_queue_depth_(p->queue_depth),
            renaming_stage_queue_(),
            free_lists_("free_list", p->phy_reg_num, p->group_num)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(MAE1RenamingUnit, Startup_));
        renaming_flush_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, HandleFlush_, FlushingCriteria));
        preceding_renaming_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, AllocateInst_, InstGroupPtr));
        following_renaming_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, AcceptDispatchCredit_, Credit));
        rob_renaming_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, AcceptRobCredit_, Credit));
        lsu_renaming_ldq_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, AcceptLoadQueueCredit_, Credit));
        lsu_renaming_stq_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, AcceptStoreQueueCredit_, Credit));
        Rob_cmt_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(MAE1RenamingUnit, RobCommit_, InstGroupPtr));
    }

    void MAE1RenamingUnit::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());
        InitCredit_();
        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void MAE1RenamingUnit::InitCredit_() {
        renaming_preceding_credit_out.send(renaming_stage_queue_depth_, sparta::Clock::Cycle(1));
    }

    void MAE1RenamingUnit::AcceptRobCredit_(const Credit& credit) {
        rob_credit_ += credit;

        ILOG("MAE1RenamingUnit get rob credits: " << credit << "total rob credit is: " << rob_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void MAE1RenamingUnit::AcceptDispatchCredit_(const Credit& credit) {
        dispatch_credit_ += credit;

        ILOG("MAE1RenamingUnit get dispatch credits: " << credit << ", total dispatch_credit_ = " << dispatch_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void MAE1RenamingUnit::AcceptLoadQueueCredit_(const Credit& credit) {
        ldq_credit_ += credit;

        ILOG("MAE1RenamingUnit get Ldq credits: " << credit << ", total ldq_credit_ = " << ldq_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void MAE1RenamingUnit::AcceptStoreQueueCredit_(const Credit& credit) {
        stq_credit_ += credit;

        ILOG("MAE1RenamingUnit get Stq credits: " << credit << ", total stq_credit_ = " << stq_credit_);

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void MAE1RenamingUnit::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("renaming stage get instructions: " << inst_ptr);
            renaming_stage_queue_.push_back(inst_ptr);
        }

        rename_event.schedule(sparta::Clock::Cycle(0));
    }

    void MAE1RenamingUnit::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG("MAE1RenamingUnit is flushed");

        dispatch_credit_ = 0;
        rob_credit_ = 0;
        ldq_credit_ = 0;
        stq_credit_ = 0;
        renaming_preceding_credit_out.send(renaming_stage_queue_depth_, sparta::Clock::Cycle(1));
        renaming_stage_queue_.clear();

        for (int i = 0; i < group_num_; i++) {
            free_lists_[i].RollBack();
        }

        for (int i = 0; i < group_num_; i++) {
            renaming_tables_[i].RollBack();
        }
    }

    void MAE1RenamingUnit::ProcessInst_() {

        uint64_t produce_inst_num = GetProduceNum_();
        if (!produce_inst_num) { // low ratio of early return
            return;
        }

        InstGroupPtr inst_produced_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        InstGroupPtr inst_produced_lsu_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        // set group idx & cross group ctrl signal
        GetGroupIdx(inst_produced_group_ptr, inst_produced_lsu_group_ptr, produce_inst_num);

        RenameInst_(inst_produced_group_ptr, inst_produced_lsu_group_ptr, produce_inst_num);

        PortTransferCtrl_(inst_produced_group_ptr, inst_produced_lsu_group_ptr);
    }

    uint64_t MAE1RenamingUnit::GetProduceNum_() {
        uint64_t produce_num_max = std::min<uint64_t>(renaming_stage_queue_.size(), issue_width_);

        uint64_t produce_inst_num = std::min<uint64_t>(dispatch_credit_, rob_credit_);

        produce_inst_num = std::min<uint64_t>(produce_inst_num, produce_num_max);
        return produce_inst_num;
    }

    void MAE1RenamingUnit::GetGroupIdx(InstGroupPtr inst_produced_group_ptr,
                                            InstGroupPtr inst_produced_lsu_group_ptr,
                                            uint64_t produce_inst_num) {

        for (auto& inst_ptr: renaming_stage_queue_) {
            if (!produce_inst_num) {
                break;
            }


            produce_inst_num--;
        }
    }

    void MAE1RenamingUnit::RenameInst_(TimingModel::InstGroupPtr inst_produced_group_ptr,
                                       TimingModel::InstGroupPtr inst_produced_lsu_group_ptr,
                                       uint64_t produce_inst_num) {
        while(produce_inst_num--) {
            auto inst_tmp_ptr = renaming_stage_queue_.front();

            if (!RenameInstImp_(inst_tmp_ptr)) { // rename & check if there is free physical idx in freelist
                break;
            }

            inst_produced_group_ptr->emplace_back(inst_tmp_ptr);
            ILOG("send inst to following: " << inst_tmp_ptr);
            renaming_stage_queue_.pop_front();
            if (inst_tmp_ptr->getFuType() == FuncType::STU || inst_tmp_ptr->getFuType() == FuncType::LDU) {
                inst_produced_lsu_group_ptr->emplace_back(inst_tmp_ptr);
            }

            --dispatch_credit_;
            --rob_credit_;
            if (inst_tmp_ptr->getFuType() == FuncType::STU && !is_perfect_lsu_) {
                stq_credit_--;
            }else if (inst_tmp_ptr->getFuType() == FuncType::LDU && !is_perfect_lsu_) {
                ldq_credit_--;
            }
        }
    }

    bool MAE1RenamingUnit::RenameInstImp_(const InstPtr& inst_ptr) {
        uint64_t reg_being_renamed_num = 0;
        if (inst_ptr->isRs1CrossGroup()) {
            reg_being_renamed_num++;
        }
        if (inst_ptr->isRs2CrossGroup()) {
            reg_being_renamed_num++;
        }
        if (inst_ptr->getRdType() != RegType_t::NONE) {
            reg_being_renamed_num++;
        }

        if (free_lists_[inst_ptr->getGroupIdx()].Size() < reg_being_renamed_num) {
            return false;
        }

        if (inst_ptr->isRs1CrossGroup()) {
            auto renamed_idx = free_lists_[inst_ptr->getGroupIdx()].Front();
            renaming_tables_[inst_ptr->getGroupIdx()][inst_ptr->getIsaRs1()] = renamed_idx;
            inst_ptr->setPhyRs1(renamed_idx);
            free_lists_[inst_ptr->getGroupIdx()].Pop();
        } else {
            inst_ptr->setPhyRs1(renaming_tables_[inst_ptr->getGroupIdx()][inst_ptr->getIsaRs1()]);
        }

        if (inst_ptr->isRs2CrossGroup()) {
            auto renamed_idx = free_lists_[inst_ptr->getGroupIdx()].Front();
            renaming_tables_[inst_ptr->getGroupIdx()][inst_ptr->getIsaRs2()] = renamed_idx;
            inst_ptr->setPhyRs2(renamed_idx);
            free_lists_[inst_ptr->getGroupIdx()].Pop();
        } else {
            inst_ptr->setPhyRs2(renaming_tables_[inst_ptr->getGroupIdx()][inst_ptr->getIsaRs2()]);
        }

        if (inst_ptr->getRdType() != RegType_t::NONE) {
            auto renamed_idx = free_lists_[inst_ptr->getGroupIdx()].Front();
            renaming_tables_[inst_ptr->getGroupIdx()][inst_ptr->getIsaRd()] = renamed_idx;
            inst_ptr->setPhyRd(renamed_idx);
            free_lists_[inst_ptr->getGroupIdx()].Pop();
        } else {
            inst_ptr->setPhyRd(0);
        }

        return true;
    }

    bool MAE1RenamingUnit::PortTransferCtrl_(InstGroupPtr inst_produced_group_ptr,
                                             InstGroupPtr inst_produced_lsu_group_ptr) {

        uint64_t whole_credit_ = std::min(dispatch_credit_, rob_credit_);

        if (!renaming_stage_queue_.empty() && whole_credit_ > 0) {
            rename_event.schedule(sparta::Clock::Cycle(1));
        }

        if (!inst_produced_group_ptr->empty()) {
            renaming_preceding_credit_out.send(inst_produced_group_ptr->size(), sparta::Clock::Cycle(1));
            renaming_following_inst_out.send(inst_produced_group_ptr);
        }

        if (!inst_produced_lsu_group_ptr->empty() && !is_perfect_lsu_) {
            renaming_lsu_allocate_out.send(inst_produced_lsu_group_ptr);
        }

        ILOG(getName() << " queue size is after update: " << renaming_stage_queue_.size());
    }

    void MAE1RenamingUnit::RobCommit_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() != 0) {
                free_list_.BackupPop();
            }
            if (inst_ptr->getLPhyRd() != 0) {
                free_list_.Push(inst_ptr->getLPhyRd());
                rename_event.schedule(1);
            }
            renaming_table_.GetBackup(inst_ptr->getIsaRd()) = inst_ptr->getPhyRd();
        }
    }

    void MAE1RenamingUnit::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        if (pmu_->IsTurnOffNextCycle()) {
            pmu_->TurnOff();
            return;
        }
        pmu_->Monitor(getName(), "02 pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "14 total loss", issue_width_);

        if (renaming_stage_queue_.size() < issue_width_) {
            pmu_->Monitor(getName(), "03 queue loss", issue_width_-renaming_stage_queue_.size());
        }
        if (renaming_stage_queue_.empty()) {
            pmu_->Monitor(getName(), "04 queue empty", 1);
            return;
        }

        uint64_t produce_num_max = std::min<uint64_t>(renaming_stage_queue_.size(), issue_width_);

        uint64_t produce_inst_num = std::min<uint64_t>(dispatch_credit_, rob_credit_);
        if (!is_perfect_lsu_) {
            produce_inst_num = std::min<uint64_t>(produce_inst_num, ldq_credit_);
            produce_inst_num = std::min<uint64_t>(produce_inst_num, stq_credit_);
        }

        if (produce_inst_num < produce_num_max) {
            pmu_->Monitor(getName(), "11 total credit loss", produce_num_max-produce_inst_num);
            if (rob_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "05 rob loss", produce_num_max-rob_credit_);
            }
            if (rob_credit_ == 0) {
                pmu_->Monitor(getName(), "06 rob full", 1);
            }
            if (dispatch_credit_ < produce_num_max) {
                pmu_->Monitor(getName(), "07 dispatch loss", produce_num_max-dispatch_credit_);
            }
            if (dispatch_credit_ == 0) {
                pmu_->Monitor(getName(), "08 dispatch full", 1);
            }
            if (ldq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "09 ldq loss", produce_num_max-ldq_credit_);
            }
            if (stq_credit_ < produce_num_max && !is_perfect_lsu_) {
                pmu_->Monitor(getName(), "10 stq loss", produce_num_max-stq_credit_);
            }
        }
    }
}