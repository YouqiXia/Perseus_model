//
// Created by yzhang on 1/2/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>

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

            PARAMETER(uint64_t, issue_num, 1, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, rs_depth, 4, "the issuing bandwidth in a cycle")
        };

        struct ReStationEntry {
            InstPtr inst_ptr;
            bool rs1_valid = false;
            bool rs2_valid = false;
        };

        using ReStationEntryPtr = sparta::SpartaSharedPointer<ReStationEntry>;

        static const char* name;

        ReservationStation(sparta::TreeNode* node, const ReservationStationParameter* p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void InitCredit_();

        void AllocateReStation(const InstPtr&);

        void PassingInst();

        void GetForwardingData(const InstGroupPtr&);

        void AcceptCredit_(const Credit&);

    private:
        // ports
        // flush
        sparta::DataInPort<FlushingCriteria> reservation_flush_in
                {&unit_port_set_, "reservation_flush_in", sparta::SchedulingPhase::Tick, 1};

        // with issue queue
        sparta::DataInPort<InstPtr> preceding_reservation_inst_in
                {&unit_port_set_, "preceding_reservation_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<RsCreditPtr> reservation_preceding_credit_out
                {&unit_port_set_, "reservation_preceding_credit_out"};

        // with function unit
        sparta::DataOutPort<InstPtr> reservation_following_inst_out
                {&unit_port_set_, "reservation_following_inst_out"};

        sparta::DataInPort<Credit> following_reservation_credit_in
                {&unit_port_set_, "following_reservation_credit_in", sparta::SchedulingPhase::Tick, 0};

        // with CDB
        sparta::DataInPort<InstGroupPtr> forwarding_reservation_inst_in
                {&unit_port_set_, "forwarding_reservation_inst_in", sparta::SchedulingPhase::Tick, 1};

        // events
        sparta::SingleCycleUniqueEvent<> passing_event
                {&unit_event_set_, "passing_event", CREATE_SPARTA_HANDLER(ReservationStation, PassingInst)};

    private:
        uint64_t issue_num_;
        uint64_t rs_depth_;
        std::vector<ReStationEntryPtr> reservation_station_;
        Credit credit_ = 0;
    };
}