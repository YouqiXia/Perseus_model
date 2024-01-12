//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"

#include <cmath>

#include "ReservationStation.hpp"

namespace TimingModel {
    const char *ReservationStation::name = "reservation_station";

    ReservationStation::ReservationStation(sparta::TreeNode *node,
                                           const TimingModel::ReservationStation::ReservationStationParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_num),
            rs_func_type_(p->rs_func_type),
            rs_depth_(p->rs_depth),
            reservation_station_()
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(ReservationStation, InitCredit_));
        reservation_flush_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, HandleFlush_, FlushingCriteria));
        preceding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AllocateReStation, InstPtr));
        following_reservation_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AcceptCredit_, Credit));
        forwarding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, GetForwardingData, InstGroupPtr));
        sparta_assert(p->rs_func_type != "", "reservation with no func type assigned");
        preceding_reservation_inst_in >> sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding");
        sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding") >> forwarding_reservation_inst_in;
    }


    void ReservationStation::AcceptCredit_(const TimingModel::Credit &credit) {
        credit_ += credit;
        ILOG(getName() << " accept credits: " << credit);

        passing_event.schedule(sparta::Clock::Cycle(0));
    }

    void ReservationStation::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        passing_event.cancel();
        ILOG(getName() << " is flushed.");

        RsCreditPtr rs_credit_ptr_tmp{new RsCredit{rs_func_type_, reservation_station_.size()}};
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
        reservation_station_.clear();
    }

    void ReservationStation::InitCredit_() {
        RsCreditPtr rs_credit_ptr_tmp {new RsCredit{rs_func_type_, rs_depth_}};
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
    }

    void ReservationStation::AllocateReStation(const TimingModel::InstPtr &inst_ptr) {
        ILOG(getName() << " get instructions: 1");
        ReStationEntryPtr tmp_restation_entry {new ReStationEntry};
        tmp_restation_entry->inst_ptr = inst_ptr;
        if (!inst_ptr->getIsRs1Forward()) {
            tmp_restation_entry->rs1_valid = true;
        }
        if (!inst_ptr->getIsRs2Forward()) {
            tmp_restation_entry->rs2_valid = true;

        }
        reservation_station_.emplace_back(tmp_restation_entry);

        passing_event.
                schedule(sparta::Clock::Cycle(1));
    }

    void ReservationStation::GetForwardingData(const TimingModel::InstGroupPtr &forwarding_inst_group_ptr) {
        for (auto &rs_entry_ptr: reservation_station_) {
            for (auto& forwarding_inst_ptr: *forwarding_inst_group_ptr) {
                if (!rs_entry_ptr->rs1_valid) {
                    if (rs_entry_ptr->inst_ptr->getRs1ForwardRob() == forwarding_inst_ptr->getRobTag()) {
                        ILOG(getName() << " rs1 get forwarding data form rob tag: " << forwarding_inst_ptr->getRobTag());
                        rs_entry_ptr->inst_ptr->setOperand1(forwarding_inst_ptr->getPhyRd());
                        rs_entry_ptr->rs1_valid = true;
                        passing_event.schedule(sparta::Clock::Cycle(1));
                    }
                }
                if (!rs_entry_ptr->rs2_valid) {
                    if (rs_entry_ptr->inst_ptr->getRs2ForwardRob() == forwarding_inst_ptr->getRobTag()) {
                        ILOG(getName() << " rs2 get forwarding data form rob tag: " << forwarding_inst_ptr->getRobTag());
                        rs_entry_ptr->inst_ptr->setOperand2(forwarding_inst_ptr->getPhyRd());
                        rs_entry_ptr->rs2_valid = true;
                        passing_event.schedule(sparta::Clock::Cycle(1));
                    }
                }
            }
        }
    }

    void ReservationStation::PassingInst() {
        if (reservation_station_.empty()) {
            return;
        }
        // in-order passing
        InstPtr inst_ptr_tmp;
        uint64_t produce_num = std::min(credit_, issue_num_);
        for (auto rs_entry_iter = reservation_station_.begin();
                rs_entry_iter != reservation_station_.end(); ++rs_entry_iter) {
            if (produce_num == 0) {
                break;
            }
            if (rs_entry_iter->get()->rs1_valid && rs_entry_iter->get()->rs2_valid) {
                ILOG(getName() << " passing instruction rob tag: " << rs_entry_iter->get()->inst_ptr->getRobTag());
                --credit_;
                --produce_num;
                inst_ptr_tmp = rs_entry_iter->get()->inst_ptr;
                reservation_station_.erase(rs_entry_iter);
                break;
            }
        }

        RsCreditPtr rs_credit_ptr_tmp {new RsCredit{rs_func_type_, 1}};
        if (inst_ptr_tmp != nullptr) {
            reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
            reservation_following_inst_out.send(inst_ptr_tmp);
        }

        if (!reservation_station_.empty() && credit_ > 0) {
            passing_event.schedule(sparta::Clock::Cycle(1));
        }

        ILOG(getName() << " queue size is after update: " << reservation_station_.size());
    }
}