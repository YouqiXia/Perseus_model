//
// Created by yzhang on 1/2/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>
#include <deque>
#include <vector>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "PortInterface.hpp"

namespace TimingModel {


    class ReservationStation : public sparta::Unit {
    public:
        class ReservationStationParameter : public sparta::ParameterSet {
        public:
            ReservationStationParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_width, 1, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, queue_depth, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(bool, is_perfect_mode, false, "if it is perfect mode")
        };

        struct ReStationEntry {
            InstPtr inst_ptr;
            bool rs1_valid = false;
            bool rs2_valid = false;
            bool is_issued = false;
        };

        using ReStationEntryPtr = sparta::SpartaSharedPointer<ReStationEntry>;

        class ReservationTable {
        public:
            ReservationTable(PhyRegId_t phy_reg_num) :
            reservation_table_(phy_reg_num, std::vector<std::vector<ReStationEntryPtr>>(2)) {}

            void Allocate(ReStationEntryPtr rs_entry_ptr) {
                reservation_table_[rs_entry_ptr->inst_ptr->getPhyRs1()][0].emplace_back(rs_entry_ptr);
                reservation_table_[rs_entry_ptr->inst_ptr->getPhyRs2()][1].emplace_back(rs_entry_ptr);
            }

            bool Resolve(InstPtr inst_ptr) {
                bool find = false;
                for (auto& rs_entry_ptr: reservation_table_[inst_ptr->getPhyRd()][0]) {
                    rs_entry_ptr->rs1_valid = true;
                    find = true;
                }
                reservation_table_[inst_ptr->getPhyRd()][0].clear();

                for (auto& rs_entry_ptr: reservation_table_[inst_ptr->getPhyRd()][1]) {
                    rs_entry_ptr->rs2_valid = true;
                    find = true;
                }
                reservation_table_[inst_ptr->getPhyRd()][1].clear();


                return find;
            }

        private:
            std::vector<std::vector<std::vector<ReStationEntryPtr>>> reservation_table_;
        };

        static const char* name;

        ReservationStation(sparta::TreeNode* node, const ReservationStationParameter* p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void InitCredit_();

        void AllocateReStation(const InstPtr&);

        void AllocateInstsReStation_(const InstGroupPtr&);

        void PassingInst();

        void GetForwardingData(const InstGroupPtr&);

        void AcceptCredit_(const Credit&);

        void PopInst_();

    private:
        // ports
        // flush
        sparta::DataInPort<FlushingCriteria> reservation_flush_in
                {&unit_port_set_, "reservation_flush_in", sparta::SchedulingPhase::Flush, 1};

        // with issue queue
        sparta::DataInPort<InstPtr> preceding_reservation_inst_in
                {&unit_port_set_, "preceding_reservation_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> preceding_reservation_insts_in
                {&unit_port_set_, "preceding_reservation_insts_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<RsCreditPtr> reservation_preceding_credit_out
                {&unit_port_set_, "reservation_preceding_credit_out"};

        // with function unit
        sparta::DataOutPort<InstPtr> reservation_following_inst_out
                {&unit_port_set_, "reservation_following_inst_out"};

        sparta::DataOutPort<InstGroupPtr> reservation_following_insts_out
                {&unit_port_set_, "reservation_following_insts_out"};

        sparta::DataInPort<Credit> following_reservation_credit_in
                {&unit_port_set_, "following_reservation_credit_in", sparta::SchedulingPhase::Tick, 0};

        // with CDB
        sparta::DataInPort<InstGroupPtr> forwarding_reservation_inst_in
                {&unit_port_set_, "forwarding_reservation_inst_in", sparta::SchedulingPhase::Tick, 1};

        // events
        sparta::SingleCycleUniqueEvent<> passing_event
                {&unit_event_set_, "passing_event", CREATE_SPARTA_HANDLER(ReservationStation, PassingInst)};

        sparta::SingleCycleUniqueEvent<> pop_event
                {&unit_event_set_, "pop_event", CREATE_SPARTA_HANDLER(ReservationStation, PopInst_)};

    private:
        uint64_t issue_num_;
        uint64_t rs_depth_;

        PhyRegId_t phy_reg_num_;
        bool is_perfect_mode_;
        std::deque<ReStationEntryPtr> reservation_station_;
        ReservationTable rs_dependency_table_;
        Credit credit_ = 0;
    };
}