//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"

#include <cmath>

#include "ReservationStation.hpp"

namespace TimingModel {
    const char* ReservationStation::name = "reservation_station";

    ReservationStation::ReservationStation(sparta::TreeNode *node,
                                           const TimingModel::ReservationStation::ReservationStationParameter *p) :
            sparta::Unit(node),
            reservation_station_("reservation_station_queue", p->rs_depth, getClock(), &unit_stat_set_)
    {
        reservation_flush_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, HandleFlush_, FlushingCriteria));
        preceding_reservation_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AllocateReStation, InstGroupPtr));
        following_reservation_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AcceptCredit_, Credit));
        forwarding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, GetForwardingData, InstGroupPtr));
    }


    void ReservationStation::AcceptCredit_(const TimingModel::Credit &credit) {
        credit_ += credit;
        ILOG(name << "accept credits: " << credit);

        passing_event.schedule(sparta::Clock::Cycle(0));
    }

    void ReservationStation::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        passing_event.cancel();
        ILOG(name << " is flushed.");

        reservation_preceding_credit_out.send(reservation_station_.size());
        reservation_station_.clear();
    }

    void ReservationStation::AllocateReStation(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ReStationEntryPtr tmp_restation_entry;
            tmp_restation_entry->inst_ptr = inst_ptr;
            if (!inst_ptr->getIsRs1Forward()) {
                tmp_restation_entry->rs1_valid = true;
            }
            if (!inst_ptr->getIsRs2Forward()) {
                tmp_restation_entry->rs1_valid = true;

            }
            reservation_station_.push(tmp_restation_entry);
        }
        passing_event.schedule(sparta::Clock::Cycle(1));
    }

    void ReservationStation::GetForwardingData(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& forwarding_inst_ptr: *inst_group_ptr) {
            for (auto& rs_entry_ptr: reservation_station_) {
                if (!rs_entry_ptr->rs1_valid) {
                    if (rs_entry_ptr->inst_ptr->getRs1ForwardRob() == forwarding_inst_ptr->getRobTag()) {
                        rs_entry_ptr->inst_ptr->setOperand1(forwarding_inst_ptr->getPhyRd());
                    }
                }
                if (!rs_entry_ptr->rs2_valid) {
                    if (rs_entry_ptr->inst_ptr->getRs2ForwardRob() == forwarding_inst_ptr->getRobTag()) {
                        rs_entry_ptr->inst_ptr->setOperand2(forwarding_inst_ptr->getPhyRd());
                    }
                }
            }
        }
    }

    void ReservationStation::PassingInst() {
        // in-order passing
        if (reservation_station_.empty()) {
            return;
        }

        if (reservation_station_.front()->rs1_valid && reservation_station_.front()->rs2_valid) {
            reservation_following_inst_out.send(reservation_station_.front()->inst_ptr);
        }

        passing_event.schedule(sparta::Clock::Cycle(1));
    }
}