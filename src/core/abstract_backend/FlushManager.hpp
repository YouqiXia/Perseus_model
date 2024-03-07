//
// Created by yzhang on 3/1/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

// #include "basic/Instruction.hh"
#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

#include <string>

namespace TimingModel {

    class FlushManager : public sparta::Unit {
    public:
        class FlushManagerParameter : public sparta::ParameterSet {
        public:
            FlushManagerParameter(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
            {}
        };

        static const char* name;

        FlushManager(sparta::TreeNode* node, const FlushManagerParameter* p);

        void ForwardingFlushSignal(const FlushingCriteria&);

        void DoFlush();

        void Nop() {}

    public:
        // ports
        sparta::DataOutPort<FlushingCriteria> global_flush_signal_out
                {&unit_port_set_, "global_flush_signal_out"};

        sparta::DataInPort<FlushingCriteria> rob_flush_manager_in
                {&unit_port_set_, "rob_flush_manager_in", sparta::SchedulingPhase::Tick, 0};

    private:
        // events
        sparta::SingleCycleUniqueEvent<> flush_event_
                {&unit_event_set_, "flush_event", CREATE_SPARTA_HANDLER(FlushManager, DoFlush)};

        sparta::SingleCycleUniqueEvent<> nop_event_
                {&unit_event_set_, "nop_event", CREATE_SPARTA_HANDLER(FlushManager, Nop)};

    private:
        sparta::Scheduler* scheduler_;

        FlushingCriteria flush_criteria_;
    };
}
