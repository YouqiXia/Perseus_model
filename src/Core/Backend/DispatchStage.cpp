//
// Created by yzhang on 1/1/24.
//

#include "DispatchStage.hpp"

namespace TimingModel {

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

        // credits initialization
        for (auto& func_pair: getFuncMap()) {
            rs_credits_.at(func_pair.first) = Credit(0);
        }

        // credits in ports initialization
        for (auto& func_pair: getFuncMap()) {
            rs_dispatch_credits_in.emplace_back(new sparta::DataInPort<RsCreditPtr>
                    (&unit_port_set_, func_pair.first+"_rs_dispatch_credit_in", sparta::SchedulingPhase::Tick, 0));
            rs_dispatch_credits_in.back()->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                (DispatchStage, AcceptCredit_, RsCreditPtr));
        }

        // construct the in-port and out-port of the physical register read ports
        uint8_t reg_read_port_num = p->func_unit_num * 2;
        while(reg_read_port_num--) {
            dispatch_read_reg_ports_out.emplace_back(new ReadPortOut
                    (&unit_port_set_, "dispatch_read_reg_port_out_"+std::to_string(reg_read_port_num)));
            read_reg_dispatch_ports_in.emplace_back(new ReadPortIn
                    (&unit_port_set_, "read_reg_dispatch_port_in_"+std::to_string(reg_read_port_num)));
            auto * phy_reg_read_port_in_tmp = new ReadPortIn
                    (&unit_port_set_, "phy_reg_read_port_in_"+std::to_string(reg_read_port_num));
            auto * phy_reg_read_port_out_tmp = new ReadPortOut
                    (&unit_port_set_, "phy_reg_read_ports_out_"+std::to_string(reg_read_port_num));
            to_delete_read_port_.emplace_back(new PhyRegfileReadPort("phy_reg_read_port_"+std::to_string(reg_read_port_num),
                                                           &phy_regfile_,
                                                           info_logger_,
                                                           phy_reg_read_port_in_tmp,
                                                           phy_reg_read_port_out_tmp));
            phy_reg_read_ports_in.emplace_back(phy_reg_read_port_in_tmp);
            phy_reg_read_ports_out.emplace_back(phy_reg_read_port_out_tmp);
        }

        // construct the physical register write ports
        uint8_t reg_write_port_num = p->func_unit_num;
        while(reg_write_port_num--) {
            auto * phy_reg_write_port_in_tmp = new WritePortIn
                    (&unit_port_set_, "phy_reg_write_port_in_"+std::to_string(reg_write_port_num));
            auto * scoreboard_write_port_in_tmp = new sparta::DataInPort<InstPtr>
                    (&unit_port_set_, "phy_reg_write_port_in_"+std::to_string(reg_write_port_num));
            to_delete_write_port_.emplace_back( new PhyRegfileWritePort("phy_reg_write_port_"+std::to_string(reg_write_port_num),
                                                                        &phy_regfile_,
                                                                        info_logger_,
                                                                        phy_reg_write_port_in_tmp));
            phy_reg_write_port_in.emplace_back(phy_reg_write_port_in_tmp);
        }


        // construct ports will be connected with Reservation Station
        for (auto& func_pair: getFuncMap()) {
            sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>> tmp_out_port_ptr
                {new sparta::DataOutPort<InstPtr>
                        (&unit_port_set_, "dispatch_"+func_pair.first+"_rs_out")};
            std::pair<FuncUnitType, sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>>> tmp_pair =
                    {func_pair.first, tmp_out_port_ptr};
            dispatch_rs_out.emplace(tmp_pair);
        }

        for (auto& dispatch_read_port_in_ptr: read_reg_dispatch_ports_in) {
            dispatch_read_port_in_ptr->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                (DispatchStage, ReadFromPhyReg_ ,PhyRegfileRequestPtr));
        }

        // precedence determination
            // for dispatch itself
            dispatch_scoreboard_events_ >> dispatch_get_operator_events_ >> dispatch_issue_events_;

            // for read physical register and dispatch
            for (auto& read_port_ptr: read_reg_dispatch_ports_in) {
                *read_port_ptr >> sparta::GlobalOrderingPoint(node, "phy_regfile_read_to_dispatch");
            }

            sparta::GlobalOrderingPoint(node, "phy_regfile_read_to_dispatch") >> dispatch_issue_events_;

    }

    void DispatchStage::bindTree_() {
        uint8_t reg_read_port_num = dispatch_read_reg_ports_out.size();
        for (uint8_t i = 0; i < reg_read_port_num; ++i) {
            sparta::bind(*phy_reg_read_ports_out[i], *dispatch_read_reg_ports_out[i]);
            sparta::bind(*phy_reg_read_ports_in[i], *read_reg_dispatch_ports_in[i]);
        }
    }

    void DispatchStage::AllocateInst_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            IssueQueueEntryPtr issue_entry_ptr_tmp;
            issue_entry_ptr_tmp->inst_ptr = inst_ptr;
            inst_queue_.push(issue_entry_ptr_tmp);
        }

        dispatch_issue_events_.schedule(1);
    }

    void DispatchStage::ReadPhyReg_() {
        std::vector<PhyRegfileRequestPtr> phy_regfile_requests;
        int entry_count = 0;
        for (auto& inst_pair : dispatch_pending_queue_) {
            auto inst_ptr = inst_pair.second;
            PhyRegfileRequestPtr phy_regfile_request_1;
            PhyRegfileRequestPtr phy_regfile_request_2;
            if (inst_ptr->getRs1Type() != RegType_t::NONE && !inst_ptr->getIsRs1Forward()) {
                phy_regfile_request_1->is_rs1 = true;
                phy_regfile_request_1->func_unit = inst_pair.first;
                phy_regfile_request_1->phy_reg_idx = inst_ptr->getPhyRs1();
                phy_regfile_requests.emplace_back(phy_regfile_request_1);
                dispatch_read_reg_ports_out[entry_count++]->send(phy_regfile_request_1);
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE && !inst_ptr->getIsRs2Forward()) {
                phy_regfile_request_2->is_rs2 = true;
                phy_regfile_request_2->func_unit = inst_pair.first;
                phy_regfile_request_2->phy_reg_idx = inst_ptr->getPhyRs2();
                phy_regfile_requests.emplace_back(phy_regfile_request_2);
                dispatch_read_reg_ports_out[entry_count++]->send(phy_regfile_request_2);
            }
        }
    }

    void DispatchStage::ReadFromPhyReg_(const TimingModel::PhyRegfileRequestPtr &phy_regfile_request_ptr) {
        if (!phy_regfile_request_ptr->data.isValid()) {
            return;
        }
        if (phy_regfile_request_ptr->is_rs1) {
            dispatch_pending_queue_.at(phy_regfile_request_ptr->func_unit)->setOperand1(phy_regfile_request_ptr->data.getValue());
        }
        if (phy_regfile_request_ptr->is_rs2) {
            dispatch_pending_queue_.at(phy_regfile_request_ptr->func_unit)->setOperand2(phy_regfile_request_ptr->data.getValue());
        }
    }

    void DispatchStage::CheckRegStatus_() {
        for (auto& inst_pair: dispatch_pending_queue_) {
            CheckRegStatusImp_(inst_pair.second);
        }
    }

    void DispatchStage::FuncUnitBack_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            scoreboard_.ClearForwardingEntry(inst_ptr->getPhyRd());
        }
    }

    void DispatchStage::CheckRegStatusImp_(InstPtr & inst_ptr) {
        if (inst_ptr->getRs1Type() != RegType_t::NONE && inst_ptr->getPhyRs1() != 0) {
            inst_ptr->setIsRs1Forward(scoreboard_.IsForwarding(inst_ptr->getPhyRs1()));
            inst_ptr->setRs1ForwardRob(scoreboard_.GetForwardingEntry(inst_ptr->getPhyRs1()));
        }
        if (inst_ptr->getRs2Type() != RegType_t::NONE && inst_ptr->getPhyRs1() != 0) {
            inst_ptr->setIsRs2Forward(scoreboard_.IsForwarding(inst_ptr->getPhyRs2()));
            inst_ptr->setRs2ForwardRob(scoreboard_.GetForwardingEntry(inst_ptr->getPhyRs2()));
        }
        if (inst_ptr->getRdType() != RegType_t::NONE && inst_ptr->getPhyRd() != 0) {
            scoreboard_.SetForwardingEntry(inst_ptr->getPhyRd(), inst_ptr->getRobTag());
        }
    }

    void DispatchStage::SelectInst_() {
        uint64_t produce_max = issue_num_;
        for (auto& func_pair: getFuncMap()) {
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

                --produce_max;
                dispatch_pending_queue_.at(func_pair.first) = issue_entry_ptr->inst_ptr;
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
        for (auto& rs_out_pair: dispatch_rs_out) {
            rs_out_pair.second->send(dispatch_pending_queue_.at(rs_out_pair.first));
        }
        dispatch_preceding_credit_out.send(dispatch_pending_queue_.size());
        dispatch_pending_queue_.clear();
    }

    void DispatchStage::AcceptCredit_(const TimingModel::RsCreditPtr &rs_credit_ptr) {
        rs_credits_.at(rs_credit_ptr->func_unit) += rs_credit_ptr->credit;
    }

}