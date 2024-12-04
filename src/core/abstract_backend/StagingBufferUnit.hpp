//
// Created by yzhang on 12/2/24.
//
#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/GlobalParamUnit.hpp"
#include "basic/SelfAllocatorsUnit.hpp"

#include "SchedulerUnit.hpp"
#include "resources/Scoreboard.hpp"
#include "resources/LatencyQueue.hpp"
#include "resources/DependencyTable.hpp"

namespace TimingModel {

    class StagingBufferUnit : public sparta::Unit {
    public:
        class StagingBufferParameter : public sparta::ParameterSet {
        public:
            StagingBufferParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, pipe_rank, 0, "the rank of which pipeline the unit is")
            PARAMETER(uint64_t, issue_width, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, queue_depth, 8, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, latency, 0, "the latency of operand accessing")
            PARAMETER(bool, is_spec_wakeup, false, "if instruction can be speculatively waked up")
        };

        static const char *name;

        StagingBufferUnit(sparta::TreeNode *node, const StagingBufferParameter *p);

    private: // port binding
        void Startup_();

        void AcceptCredit_(const CreditPairPtr&);

        void HandleFlush_(const FlushingCriteria&);

        void RecieveInsts_(const InstGroupPtr&);

        void BypassInst_(const InstGroupPairPtr&);

    private: // inner implementation
        void InitCredit_();

        void ProcessInsts_();

        uint64_t GetProduceNum_();

        void DataTransfer_(InstGroupPtr processed_group_ptr,
                           InstGroupPtr wakeup_resolve_group_ptr);

        void TickLatencyQueue_();

    private:
        /* ports */
        // read/write port
        sparta::DataInPort<InstGroupPtr> preceding_inst_in
                {&unit_port_set_, "preceding_inst_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<CreditPairPtr> preceding_credit_out
                {&unit_port_set_, "preceding_credit_out"};

        sparta::DataOutPort<InstGroupPtr> following_inst_out
                {&unit_port_set_, "following_inst_out"};

        sparta::DataInPort<CreditPairPtr> following_credit_in
                {&unit_port_set_, "following_credit_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPairPtr> bypass_inst_in
                {&unit_port_set_, "bypass_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroupPtr> wakeup_resolve_inst_out
                {&unit_port_set_, "wakeup_resolve_inst_out"};

        /* events */
        sparta::SingleCycleUniqueEvent<> process_event
                {&unit_event_set_, "process_event", CREATE_SPARTA_HANDLER(StagingBufferUnit, ProcessInsts_)};
    private:
        SelfAllocatorsUnit* allocator_;
        GlobalParamUnit* global_param_ptr_ = nullptr;
        PmuUnit* pmu_;

    private:
        const uint64_t issue_width_;
        const uint64_t queue_depth_;
        const uint64_t latency_;
        const uint64_t pipe_rank_;
        const bool is_spec_wakeup_;
        uint64_t group_idx_;

        uint64_t credit_;

        DependencyTable dependency_table_;
        LatencyQueue<InstPtr> latency_queue_;
        std::deque<InstPtr> inst_queue_;
    };

}
