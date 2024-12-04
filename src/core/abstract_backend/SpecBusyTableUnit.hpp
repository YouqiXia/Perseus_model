//
// Created by yzhang on 11/11/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>
#include <deque>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"
#include "basic/GlobalParamUnit.hpp"
#include "basic/SelfAllocatorsUnit.hpp"

#include "resources/Scoreboard.hpp"

#include "simulation/PmuUnit.hpp"

namespace TimingModel {

    /*
     * speculative busy table record the speculative wakeup ctrl signal
     * if an operand is speculatively waking up, the latency = 1
     * if latency > 1, latency = latency
     * if not, the latency = 0
     * */
    class SpecBusyTableUnit: public sparta::Unit {
    public:
        class SpecBusyTableParameter: public sparta::ParameterSet {
        public:
            SpecBusyTableParameter(sparta::TreeNode *n) :
                sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, pipe_rank, 0, "the rank of which pipeline the unit is")
            PARAMETER(uint64_t, wakeup_latency, 0, "the latency of speculative wakeup")
            PARAMETER(uint64_t, phy_reg_num, 64, "the number of physical registers")
            PARAMETER(bool, is_spec_wakeup, false, "if instruction can be speculatively waked up")
        };

        static const char *name;

        SpecBusyTableUnit(sparta::TreeNode *node, const SpecBusyTableParameter *p);

    private: // port binding implementation
        void Startup_();

        void CheckBusy_(const InstGroupPairPtr&);

        void UpdateBusy_(const InstGroupPtr&);

        void SpecWakeup_(const InstGroupPtr&);

        void WakeupResolve_(const InstGroupPtr&);

        void HandleFlush_(const FlushingCriteria&);

    private: // inner implementation
        void Process_();

    private:
        // port
        sparta::DataInPort<FlushingCriteria> flush_in
                {&unit_port_set_, "flush_in", sparta::SchedulingPhase::Flush, 1};

        sparta::DataInPort<InstGroupPairPtr> check_in
                {&unit_port_set_, "check_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> update_in
                {&unit_port_set_, "update_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> wakeup_in
                {&unit_port_set_, "wakeup_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> wakeup_resolve_in
                {&unit_port_set_, "wakeup_resolve_in", sparta::SchedulingPhase::Tick, 1};

        // event
        // process event
        sparta::SingleCycleUniqueEvent<> process_event
                {&unit_event_set_, "process_event", CREATE_SPARTA_HANDLER(SpecBusyTableUnit, Process_)};

    private:
        GlobalParamUnit* global_param_ptr_ = nullptr;
        GlobalParamUnit::FuLatencyMap* fu_latency_map_;
        std::set<uint32_t> recorded_ranks;
        SelfAllocatorsUnit* allocator_;
        PmuUnit* pmu_;

    private:
        const uint64_t wakeup_latency_;
        const uint64_t pipe_rank_;
        const bool is_spec_wakeup_;

        Scoreboard busy_table_;
        std::unordered_map<PhyRegId_t, uint64_t> latency_table_;
    };

}
