//
// Created by yzhang on 1/1/24.
//

#include "DispatchStage.hpp"


namespace TimingModel {
    const char* DispatchStage::name = "dispatch_stage";

    DispatchStage::DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_width),
            inst_queue_depth_(p->queue_depth),
            scoreboard_("scoreboard", p->phy_reg_num, info_logger_),
            global_param_ptr_(getGlobalParams(node)),
            inst_queue_()
    {
        // Startup events
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(DispatchStage, InitCredit_));

        // normal ports binding
        rs_dispatch_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
           (DispatchStage, AcceptCredit_, CreditPairPtr));

        preceding_dispatch_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, AllocateInst_, InstGroupPtr));

        write_back_dispatch_port_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, FuncUnitBack_, InstGroupPtr));

        dispatch_flush_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, HandleFlush_, FlushingCriteria));

        physical_reg_dispatch_read_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, ReadFromPhysicalReg_, InstGroupPtr));

        for(auto dispatch_map_pair: global_param_ptr_->getDispatchMap()) {
            credit_map_[dispatch_map_pair.first] = 0;
        }

        // precedence determination
            // for dispatch itself
            dispatch_pop_events_ >> dispatch_select_events_ >>
            dispatch_scoreboard_events_ >> dispatch_get_operator_events_ >> dispatch_issue_events_;

            physical_reg_dispatch_read_in >> sparta::GlobalOrderingPoint(node, "dispatch_physical_reg_node");
            sparta::GlobalOrderingPoint(node, "dispatch_physical_reg_node") >> dispatch_issue_events_;

            write_back_dispatch_port_in >> sparta::GlobalOrderingPoint(node, "dispatch_node");
            sparta::GlobalOrderingPoint(node, "dispatch_node") >> dispatch_select_events_;

            sparta::GlobalOrderingPoint(node, "rob_dispatch_node") >> dispatch_scoreboard_events_;

    }

    void DispatchStage::AllocateInst_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG(getName() << " get instruction rob tag: " << inst_ptr->getRobTag());
            ILOG("get inst from preceding: " << inst_ptr);
            IssueQueueEntryPtr issue_entry_ptr_tmp {new IssueQueueEntry};
            issue_entry_ptr_tmp->inst_ptr = inst_ptr;
            if (inst_ptr->getRdType() != RegType_t::NONE) {
                scoreboard_.SetBusyBit(inst_ptr->getPhyRd());
            }
            inst_queue_.push_back(issue_entry_ptr_tmp);
        }

        dispatch_select_events_.schedule(0);
        dispatch_pop_events_.schedule(0);
    }

    void DispatchStage::ReadPhyReg_() {
        ILOG(getName() << " read physical register.");
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        for (auto& dispatch_pending_pair : dispatch_pending_queue_) {
            for (auto& inst_ptr: dispatch_pending_pair.second) {
                inst_group_tmp_ptr->emplace_back(inst_ptr);
            }
        }
        dispatch_physical_reg_read_out.send(inst_group_tmp_ptr);
    }

    void DispatchStage::ReadFromPhysicalReg_(const TimingModel::InstGroupPtr &inst_group_ptr) {
    }

    void DispatchStage::CheckRegStatus_() {
        ILOG(getName() << " check register status.");
    }

    void DispatchStage::FuncUnitBack_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() == 0) {
                continue;
            }
            scoreboard_.ClearBusyBit(inst_ptr->getPhyRd());
        }
    }

    void DispatchStage::CheckRegStatusImp_(InstPtr & inst_ptr) {
        if (inst_ptr->getRs1Type() != RegType_t::NONE) {
            if (scoreboard_.GetBusyBit(inst_ptr->getPhyRs1())) {
                inst_ptr->setIsRs1Forward(true);
            }
        }
        if (inst_ptr->getRs2Type() != RegType_t::NONE) {
            if (scoreboard_.GetBusyBit(inst_ptr->getPhyRs2())) {
                inst_ptr->setIsRs2Forward(true);
            }
        }
    }

    void DispatchStage::SelectInst_() {
        uint64_t produce_max = issue_num_;
        uint64_t produce_num = 0;

        for (auto &func_pair: global_param_ptr_->getDispatchMap()) {
            uint32_t issue_width_per_pipe = global_param_ptr_->getDispatchIssueWidthMap().at(func_pair.first);
            if (!credit_map_.at(func_pair.first)) {
                continue;
            }

            for (auto &issue_entry_ptr: inst_queue_) {
                if (!credit_map_.at(func_pair.first)) {
                    break;
                }

                if (!issue_width_per_pipe) {
                    break;
                }

                if (issue_entry_ptr->is_issued) {
                    continue;
                }

                if (func_pair.second.find(issue_entry_ptr->inst_ptr->getFuType()) == func_pair.second.end()) {
                    continue;
                }

                ILOG(getName() << " Instruction Select rob tag: " << issue_entry_ptr->inst_ptr->getRobTag()
                               << ", function unit type is " << func_pair.first);
                --produce_max;
                ++produce_num;
                --issue_width_per_pipe;
                dispatch_pending_queue_[func_pair.first].emplace_back(issue_entry_ptr->inst_ptr);
                CheckRegStatusImp_(issue_entry_ptr->inst_ptr);
                --credit_map_.at(func_pair.first);
                issue_entry_ptr->is_issued = true;
            }
        }

        if (produce_num) {
            dispatch_preceding_credit_out.send(produce_num);
        }

        dispatch_get_operator_events_.schedule(0);
        dispatch_scoreboard_events_.schedule(0);
        dispatch_issue_events_.schedule(0);
    }

    void DispatchStage::HandleFlush_(const TimingModel::FlushingCriteria &flushing_criteria) {
        ILOG(name << "is flushed.");

        for (auto& credit_pair: credit_map_) {
            credit_pair.second = 0;
        }
        dispatch_preceding_credit_out.send(inst_queue_depth_);
        inst_queue_.clear();
    }

    void DispatchStage::InitCredit_() {
        dispatch_preceding_credit_out.send(inst_queue_depth_);
    }

    void DispatchStage::IssueInst_() {
        for (auto& dispatch_pending_pair: dispatch_pending_queue_) {
            InstGroupPairPtr inst_group_tmp_ptr =
                    sparta::allocate_sparta_shared_pointer<InstGroupPair>(inst_group_pair_allocator);
            for (auto& inst_ptr: dispatch_pending_pair.second) {
                ILOG(getName() << " issue instruction rob tag: " << inst_ptr->getRobTag());
                ILOG("issue insn to following: " << inst_ptr);
                inst_group_tmp_ptr->inst_group.emplace_back(inst_ptr);
                inst_group_tmp_ptr->name = dispatch_pending_pair.first;
            }
            dispatch_rs_inst_out.send(inst_group_tmp_ptr);
        }
        dispatch_pending_queue_.clear();

        if (!inst_queue_.empty()) {
            dispatch_select_events_.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());
    }

    void DispatchStage::PopIssueQueue_() {
        ILOG(getName() << " try to pop instructions.");
        uint64_t issue_queue_pop_size = 0;
        for (auto& issue_queue_entry_ptr: inst_queue_) {
            if (issue_queue_entry_ptr->is_issued) {
                inst_queue_.pop_front();
                ++issue_queue_pop_size;
            } else {
                break;
            }
        }

        if (!inst_queue_.empty()) {
            dispatch_pop_events_.schedule(1);
            dispatch_select_events_.schedule(1);
        }
    }

    void DispatchStage::AcceptCredit_(const TimingModel::CreditPairPtr &credit_pair_ptr) {
        credit_map_.at(credit_pair_ptr->name) += credit_pair_ptr->credit;
        ILOG("accept credits from " << credit_pair_ptr->name << " , credits is " << credit_pair_ptr->credit <<
            " updated credits is: " << credit_map_.at(credit_pair_ptr->name));
    }

}