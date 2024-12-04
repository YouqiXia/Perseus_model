//
// Created by yzhang on 1/1/24.
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

    class DispatchStage : public sparta::Unit {
    public:
        class DispatchStageParameter : public sparta::ParameterSet {
        public:
            DispatchStageParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, issue_width, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, queue_depth, 16, "the issuing bandwidth in a cycle")
        };

        struct IssueQueueEntry {
            InstPtr inst_ptr;
            bool is_issued = false;
        };

        using IssueQueueEntryPtr = sparta::SpartaSharedPointer<IssueQueueEntry>;

        static const char *name;

        DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p);

    private: // port binding implementation
        void Startup_();

        void HandleFlush_(const FlushingCriteria&);

        // credit
        void InitCredit_();

        void AcceptCredit_(const CreditPairPtr&);
        // credit

        void AllocateInst_(const InstGroupPtr&);

        void FuncUnitBack_(const InstGroupPtr&);

    private: // inner implementation
        void ProcessInst_();

        void PopDispatchQueue_();

        void SelectInst_();

        void DispatchInsts_();

    private: // pmu
        void PmuMonitor_();

    private:
        // ports
        
            //flush
            sparta::DataInPort<FlushingCriteria> dispatch_flush_in
                {&unit_port_set_, "dispatch_flush_in", sparta::SchedulingPhase::Flush, 1};

            // with renaming
            sparta::DataInPort<InstGroupPtr> preceding_dispatch_inst_in
                {&unit_port_set_, "preceding_dispatch_inst_in", sparta::SchedulingPhase::Tick, 1};

            sparta::DataOutPort<Credit> dispatch_preceding_credit_out
                {&unit_port_set_, "dispatch_preceding_credit_out"};

            // with rs -> also should be constructed in dispatch stage constructor
            sparta::DataOutPort<InstGroupPairPtr> dispatch_rs_inst_out
                    {&unit_port_set_, "dispatch_rs_inst_out"};

            // with rs Credits
            sparta::DataInPort<CreditPairPtr> rs_dispatch_credit_in
                    {&unit_port_set_, "rs_dispatch_credit_in", sparta::SchedulingPhase::Tick, 0};

            // write back ports
            sparta::DataInPort<InstGroupPtr> write_back_dispatch_port_in
                    {&unit_port_set_, "write_back_dispatch_port_in", sparta::SchedulingPhase::Tick, 1};


        // events
            sparta::SingleCycleUniqueEvent<> process_event
                    {&unit_event_set_, "process_event", CREATE_SPARTA_HANDLER(DispatchStage, ProcessInst_)};

            sparta::SingleCycleUniqueEvent<sparta::SchedulingPhase::PostTick> pmu_event
                {&unit_event_set_, "pmu_event", CREATE_SPARTA_HANDLER(DispatchStage, PmuMonitor_)};

    private:
        GlobalParamUnit* global_param_ptr_ = nullptr;
        SelfAllocatorsUnit* allocator_;
        PmuUnit* pmu_;

    private:
        uint64_t issue_num_;

        std::unordered_map<uint64_t, Credit> credit_map_;

        const uint64_t inst_queue_depth_;

        std::deque<IssueQueueEntryPtr> inst_queue_;

        std::unordered_map<uint64_t, std::vector<InstPtr>> dispatch_pending_queue_;
    };

}