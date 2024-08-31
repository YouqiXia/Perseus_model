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
            issue_num_(p->issue_width),
            rs_depth_(p->queue_depth),
            phy_reg_num_(p->phy_reg_num),
            is_perfect_mode_(p->is_perfect_mode),
            rs_dependency_table_(p->phy_reg_num),
            reservation_station_()
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(ReservationStation, InitCredit_));
        reservation_flush_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, HandleFlush_, FlushingCriteria));
        preceding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AllocateReStation, InstPtr));
        preceding_reservation_insts_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AllocateInstsReStation_, InstGroupPtr));
        following_reservation_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AcceptCredit_, Credit));
        forwarding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, GetForwardingData, InstGroupPtr));
        preceding_reservation_inst_in >> sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding");
        preceding_reservation_insts_in >> sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding");
        sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding") >> forwarding_reservation_inst_in;
    }


    void ReservationStation::AcceptCredit_(const TimingModel::Credit &credit) {
        credit_ += credit;
        ILOG(getName() << " accept credits: " << credit);

        passing_event.schedule(sparta::Clock::Cycle(0));
    }

    void ReservationStation::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        ILOG(getName() << " is flushed.");

        credit_ = 0;
        RsCreditPtr rs_credit_ptr_tmp{new RsCredit{getName(), rs_depth_}};
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
        reservation_station_.clear();
    }

    void ReservationStation::InitCredit_() {
        RsCreditPtr rs_credit_ptr_tmp {new RsCredit{getName(), rs_depth_}};
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
    }

    void ReservationStation::AllocateReStation(const TimingModel::InstPtr &inst_ptr) {
        ILOG(getName() << " get instructions: 1");
        ILOG("get insn from preceding: " << inst_ptr);
        ReStationEntryPtr tmp_restation_entry {new ReStationEntry};
        tmp_restation_entry->inst_ptr = inst_ptr;
        if (!inst_ptr->getIsRs1Forward()) {
            tmp_restation_entry->rs1_valid = true;
        }
        if (!inst_ptr->getIsRs2Forward()) {
            tmp_restation_entry->rs2_valid = true;

        }
        rs_dependency_table_.Allocate(tmp_restation_entry);
        reservation_station_.emplace_back(tmp_restation_entry);

        pop_event.schedule(sparta::Clock::Cycle(1));
        passing_event.schedule(sparta::Clock::Cycle(0));
    }

    void ReservationStation::AllocateInstsReStation_(const InstGroupPtr& inst_group_ptr) {
        ILOG(getName() << " get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get insn from preceding: " << inst_ptr);
            ReStationEntryPtr tmp_restation_entry{new ReStationEntry};
            tmp_restation_entry->inst_ptr = inst_ptr;
            if (!inst_ptr->getIsRs1Forward()) {
                tmp_restation_entry->rs1_valid = true;
            }
            if (!inst_ptr->getIsRs2Forward()) {
                tmp_restation_entry->rs2_valid = true;

            }
            rs_dependency_table_.Allocate(tmp_restation_entry);
            reservation_station_.emplace_back(tmp_restation_entry);
        }

        pop_event.schedule(sparta::Clock::Cycle(1));
        passing_event.schedule(sparta::Clock::Cycle(0));
    }

    void ReservationStation::GetForwardingData(const TimingModel::InstGroupPtr &forwarding_inst_group_ptr) {
        bool find = false;
        for (auto& forwarding_inst_ptr: *forwarding_inst_group_ptr) {
            find |= rs_dependency_table_.Resolve(forwarding_inst_ptr);
        }
    }

    void ReservationStation::PopInst_() {
        for (int i = 0; i < reservation_station_.size(); i++) {
            if (reservation_station_.front()->is_issued) {
                reservation_station_.pop_front();
            } else {
                break;
            }
        }

        if (!reservation_station_.empty()) {
            pop_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void ReservationStation::PassingInst() {
        if (reservation_station_.empty()) {
            return;
        }
        // in-order passing
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        uint64_t produce_num = std::min(credit_, issue_num_);
        uint64_t consume_num = 0;
        ILOG("produce_num = " << produce_num << ", credit_ = " << credit_ << ", issue_num = " << issue_num_);
        for (auto &rs_entry: reservation_station_) {
            if (produce_num == 0) {
                break;
            }

            if (rs_entry->is_issued) {
                continue;
            }

            if (rs_entry->rs1_valid && rs_entry->rs2_valid) {
                ILOG(getName() << " passing instruction: " << rs_entry->inst_ptr);
                --credit_;
                --produce_num;
                inst_group_tmp_ptr->emplace_back(rs_entry->inst_ptr);
                rs_entry->is_issued = true;
                consume_num++;
            }
        }

        RsCreditPtr rs_credit_ptr_tmp {new RsCredit{getName(), consume_num}};
        if (!is_perfect_mode_) {
            for (auto &inst_ptr: *inst_group_tmp_ptr) {
                ILOG("send insn to following: " << inst_ptr);
                reservation_following_inst_out.send(inst_ptr);
            }
        } else {
            for (auto &inst_ptr: *inst_group_tmp_ptr) {
                ILOG("send insn to following: " << inst_ptr);
            }
            if (!inst_group_tmp_ptr->empty()) {
                reservation_following_insts_out.send(inst_group_tmp_ptr);
            }
        }

        if (!inst_group_tmp_ptr->empty()) {
            reservation_preceding_credit_out.send(rs_credit_ptr_tmp);
        }

        if (!reservation_station_.empty() && credit_ > 0) {
            passing_event.schedule(sparta::Clock::Cycle(1));
        }

        ILOG(getName() << " queue size is after update: " << reservation_station_.size());
    }
}