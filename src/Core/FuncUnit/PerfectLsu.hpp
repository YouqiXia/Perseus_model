#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"

#include "WriteBackStage.hpp"

namespace TimingModel {
    class PerfectLsu : public sparta::Unit {
    public:
        class PerfectLsuParameter : public sparta::ParameterSet {
        public:
            PerfectLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
            PARAMETER(FuncUnitType, func_type, "", "the type of function unit")
        };

        static const char* name;

        PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p);

    private:
        void WriteBack_(const InstPtr&);

    public:
    // ports
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<FuncInstPtr> func_following_finish_out
            {&unit_port_set_, "func_following_finish_out"};

    private:
        const uint64_t load_to_use_latency_;
        FuncUnitType func_type_;
    };

}