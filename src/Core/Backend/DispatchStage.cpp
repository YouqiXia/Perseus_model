//
// Created by yzhang on 1/1/24.
//

#include "DispatchStage.hpp"

namespace TimingModel {

    const char* DispatchStage::name = "dispatch_stage";

    DispatchStage::DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p) :
            sparta::Unit(node),
            phy_regfile_(p->phy_reg_num, 0),
            issue_num_(p->issue_num),
            inst_queue_depth_(p->issue_queue_depth),
            scoreboard_("scoreboard", p->phy_reg_num, info_logger_),
            inst_queue_("centralized_issue_queue", p->issue_queue_depth, node->getClock(), &unit_stat_set_)
    {
        // Startup events
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(DispatchStage, InitCredit_));

        // normal ports binding
        preceding_dispatch_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, AllocateInst_, InstGroupPtr));

        write_back_dispatch_port_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, FuncUnitBack_, InstGroupPtr));

        dispatch_flush_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, HandleFlush_, FlushingCriteria));

        // credits initialization
        FuncMap& fu_map = getFuncMap();
        for (auto& func_pair: fu_map) {
            rs_credits_[func_pair.first] = Credit(0);

            // credits in ports initialization
            rs_dispatch_credits_in.emplace_back(new sparta::DataInPort<RsCreditPtr>
                    (&unit_port_set_, func_pair.first+"_rs_dispatch_credit_in", sparta::SchedulingPhase::Tick, 0));
            rs_dispatch_credits_in.back()->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                (DispatchStage, AcceptCredit_, RsCreditPtr));
            *(rs_dispatch_credits_in.back()) >> sparta::GlobalOrderingPoint(node, "dispatch_node");

            // construct ports will be connected with Reservation Station
            sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>> tmp_out_port_ptr
                {new sparta::DataOutPort<InstPtr>
                        (&unit_port_set_, "dispatch_"+func_pair.first+"_rs_out")};
            std::pair<FuncUnitType, sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>>> tmp_pair =
                    {func_pair.first, tmp_out_port_ptr};
            dispatch_rs_out.emplace(tmp_pair);
        }

        // precedence determination
            // for dispatch itself
            dispatch_pop_events_ >> dispatch_select_events_ >>
            dispatch_scoreboard_events_ >> dispatch_get_operator_events_ >> dispatch_issue_events_;

            sparta::GlobalOrderingPoint(node, "dispatch_node") >> dispatch_select_events_;
            write_back_dispatch_port_in >> sparta::GlobalOrderingPoint(node, "dispatch_node");

            sparta::GlobalOrderingPoint(node, "rob_dispatch_node") >> dispatch_scoreboard_events_;

    }

    void DispatchStage::AllocateInst_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG(getName() << " get instruction rob tag: " << inst_ptr->getRobTag());
            ILOG("get inst from preceding: " << inst_ptr);
            IssueQueueEntryPtr issue_entry_ptr_tmp {new IssueQueueEntry};
            issue_entry_ptr_tmp->inst_ptr = inst_ptr;
            inst_queue_.push(issue_entry_ptr_tmp);
        }

        dispatch_select_events_.schedule(1);
        dispatch_pop_events_.schedule(1);
    }

    void DispatchStage::ReadPhyReg_() {
        ILOG(getName() << " read physical register.");
        for (auto& inst_pair : dispatch_pending_queue_) {
            auto inst_ptr = inst_pair.second;
            if (inst_ptr->getRs1Type() != RegType_t::NONE && !inst_ptr->getIsRs1Forward()) {
                if (inst_ptr->getPhyRs1() == 0) {
                    inst_ptr->setOperand1(0);
                } else {
                    inst_ptr->setOperand1(phy_regfile_[inst_ptr->getPhyRs1()]);
                }
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE && !inst_ptr->getIsRs2Forward()) {
                if (inst_ptr->getPhyRs2() == 0) {
                    inst_ptr->setOperand2(0);
                } else {
                    inst_ptr->setOperand2(phy_regfile_[inst_ptr->getPhyRs2()]);
                }
            }
        }
    }

    void DispatchStage::CheckRegStatus_() {
        ILOG(getName() << " check register status.");
        for (auto& inst_pair: dispatch_pending_queue_) {
            CheckRegStatusImp_(inst_pair.second);
        }
    }

    void DispatchStage::FuncUnitBack_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() == 0) {
                continue;
            }
            scoreboard_.ClearForwardingEntry(inst_ptr->getPhyRd());
            phy_regfile_[inst_ptr->getPhyRd()] = inst_ptr->getRdResult();
        }
    }

    void DispatchStage::CheckRegStatusImp_(InstPtr & inst_ptr) {
        if (inst_ptr->getRs1Type() != RegType_t::NONE) {
            if (scoreboard_.IsForwarding(inst_ptr->getPhyRs1())) {
                inst_ptr->setIsRs1Forward(true);
                inst_ptr->setRs1ForwardRob(scoreboard_.GetForwardingEntry(inst_ptr->getPhyRs1()));
            }
        }
        if (inst_ptr->getRs2Type() != RegType_t::NONE) {
            if (scoreboard_.IsForwarding(inst_ptr->getPhyRs2())) {
                inst_ptr->setIsRs2Forward(true);
                inst_ptr->setRs2ForwardRob(scoreboard_.GetForwardingEntry(inst_ptr->getPhyRs2()));
            }
        }
        if (inst_ptr->getRdType() != RegType_t::NONE) {
            scoreboard_.SetForwardingEntry(inst_ptr->getPhyRd(), inst_ptr->getRobTag());
        }
    }

    void DispatchStage::SelectInst_() {
        uint64_t produce_max = issue_num_;
        FuncMap& fu_map = getFuncMap();
        for (auto& func_pair: fu_map) {
            if (!produce_max) {
                break;
            }

            if (!rs_credits_.at(func_pair.first)) {
                continue;
            }

            for (auto& issue_entry_ptr: inst_queue_) {
                if (issue_entry_ptr->is_issued) {
                    continue;
                }

                if (func_pair.second.find(issue_entry_ptr->inst_ptr->getFuType()) == func_pair.second.end()) {
                    continue;
                }

                ILOG(getName() << " Instruction Select rob tag: " << issue_entry_ptr->inst_ptr->getRobTag()
                               << ", function unit type is " << func_pair.first);
                --produce_max;
                dispatch_pending_queue_[func_pair.first] = issue_entry_ptr->inst_ptr;
                --rs_credits_.at(func_pair.first);
                issue_entry_ptr->is_issued = true;
                break;
            }
        }

        dispatch_get_operator_events_.schedule(0);
        dispatch_scoreboard_events_.schedule(0);
        dispatch_issue_events_.schedule(0);
    }

    void DispatchStage::HandleFlush_(const TimingModel::FlushingCriteria &flushing_criteria) {
        dispatch_select_events_.cancel();
        ILOG(name << "is flushed.");

        dispatch_preceding_credit_out.send(inst_queue_.size());
        inst_queue_.clear();
    }

    void DispatchStage::InitCredit_() {
        dispatch_preceding_credit_out.send(inst_queue_depth_);
    }

    void DispatchStage::IssueInst_() {
        for (auto& dispatch_pending_pair: dispatch_pending_queue_) {
            ILOG(getName() << " issue instruction rob tag: " << dispatch_pending_pair.second->getRobTag());
            ILOG("issue insn to following: " << dispatch_pending_pair.second);
            dispatch_rs_out.at(dispatch_pending_pair.first)->send(dispatch_pending_pair.second);
        }
        dispatch_pending_queue_.clear();

        uint64_t whole_credit = 256;

        for (auto& rs_credit_pair: rs_credits_) {
            whole_credit = std::min(whole_credit, rs_credit_pair.second);
        }

        if (!inst_queue_.empty() && whole_credit > 0) {
            dispatch_select_events_.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());
    }

    void DispatchStage::PopIssueQueue_() {
        ILOG(getName() << " try to pop instructions.");
        uint64_t issue_queue_pop_size = 0;
        uint64_t max_pop_width = issue_num_;
        for (auto& issue_queue_entry_ptr: inst_queue_) {
            if (issue_queue_entry_ptr->is_issued && max_pop_width != 0) {
                inst_queue_.pop();
                ++issue_queue_pop_size;
                --max_pop_width;
            } else {
                break;
            }
        }

        if (issue_queue_pop_size) {
            dispatch_preceding_credit_out.send(issue_queue_pop_size);
        }

        if (!inst_queue_.empty()) {
            dispatch_pop_events_.schedule(1);
            if (max_pop_width != 0) {
                dispatch_select_events_.schedule(1);
            }
        }
    }

    void DispatchStage::AcceptCredit_(const TimingModel::RsCreditPtr &rs_credit_ptr) {
        ILOG("dispatch stage accept credits from " << rs_credit_ptr->func_unit << "_rs , credits is " << rs_credit_ptr->credit);
        rs_credits_.at(rs_credit_ptr->func_unit) += rs_credit_ptr->credit;
    }

}