//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"

#include <cmath>

#include "ReservationStation.hpp"

namespace TimingModel {
    ReservationStation::ReStationEntryAllocator re_station_entry_allocator {300000, 250000};

    const char *ReservationStation::name = "reservation_station";

    ReservationStation::ReservationStation(sparta::TreeNode *node,
                                           const TimingModel::ReservationStation::ReservationStationParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_width),
            rs_depth_(p->queue_depth),
            phy_reg_num_(p->phy_reg_num),
            rs_dependency_table_(p->phy_reg_num),
            reservation_station_()
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(ReservationStation, Startup_));
        reservation_flush_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, HandleFlush_, FlushingCriteria));
        preceding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AllocateReStation, InstGroupPairPtr));
        following_reservation_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, AcceptCredit_, Credit));
        forwarding_reservation_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(ReservationStation, GetForwardingData, InstGroupPtr));
        preceding_reservation_inst_in >> sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding");
        sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding") >> forwarding_reservation_inst_in;
    }

    void ReservationStation::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        InitCredit_();
    }

    void ReservationStation::InitCredit_() {
        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->name = getName();
        rs_credit_ptr_tmp->credit = rs_depth_;
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
    }

    void ReservationStation::AcceptCredit_(const TimingModel::Credit &credit) {
        credit_ += credit;
        ILOG(getName() << " accept credits: " << credit << ", updated credits: " << credit_);

        passing_event.schedule(sparta::Clock::Cycle(1));
    }

    void ReservationStation::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        ILOG(getName() << " is flushed.");

        credit_ = 0;
        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->name = getName();
        rs_credit_ptr_tmp->credit = rs_depth_;
        reservation_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
        reservation_station_.clear();
    }

    void ReservationStation::AllocateReStation(const TimingModel::InstGroupPairPtr &inst_group_pair_ptr) {
        if (inst_group_pair_ptr->name != getName()) {
            return;
        }
        auto inst_group_ptr = &inst_group_pair_ptr->inst_group;
        ILOG(getName() << " get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get insn from preceding: " << inst_ptr);
            ReStationEntryPtr tmp_restation_entry =
                    sparta::allocate_sparta_shared_pointer<ReStationEntry>(re_station_entry_allocator);
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
        for (auto& forwarding_inst_ptr: *forwarding_inst_group_ptr) {
            bool find = false;
            find = rs_dependency_table_.Resolve(forwarding_inst_ptr);
            if (find) {
                ILOG("get forwarding data from: " << forwarding_inst_ptr);
            }
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
        InstGroupPtr inst_group_tmp_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        uint64_t produce_num = std::min(credit_, issue_num_);
        uint64_t consume_num = 0;
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

        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->name = getName();
        rs_credit_ptr_tmp->credit = consume_num;
        for (auto &inst_ptr: *inst_group_tmp_ptr) {
            ILOG("send insn to following: " << inst_ptr);
        }
        if (!inst_group_tmp_ptr->empty()) {
            reservation_following_inst_out.send(inst_group_tmp_ptr);
            ILOG(getName() << " queue size is after update: " << reservation_station_.size());
        }

        if (!inst_group_tmp_ptr->empty()) {
            reservation_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
        }

        if (!reservation_station_.empty() && credit_ > 0) {
            passing_event.schedule(sparta::Clock::Cycle(1));
        }

    }
}