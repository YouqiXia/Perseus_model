#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "instgen/InstGenerator.hpp"
#include "basic/SelfAllocatorsUnit.hpp"
#include "simulation/PmuUnit.hpp"

#include <string>

namespace TimingModel {

    class PerfectFrontend : public sparta::Unit {
    public:
        class PerfectFrontendParameter : public sparta::ParameterSet {
        public:
            PerfectFrontendParameter(sparta::TreeNode* n):
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_width, 4, "the issuing bandwidth in a cycle")
            PARAMETER(std::string, input_file, "", "the stf entry")
            PARAMETER(std::string, insn_gen_type, "trace", "the type of insnGen: trace or spike")
            PARAMETER(bool, is_speculation, false, "if model is running in speculative way")
            PARAMETER(bool, is_config, true, "if model is in configuring, ")
        };

        static const char* name;

        PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p);

        void AcceptCredit(const Credit&);

        void ProduceInst();

        void BranchResolve(const InstPtr&);

        void BranchCommit(const InstGroupPtr &);

        void RedirectPc(const InstPtr&);

    private:
        void Startup_();

        void PmuMonitor_();

    public:
    // ports
        sparta::DataOutPort<InstGroupPtr> fetch_backend_inst_out
            {&unit_port_set_, "fetch_backend_inst_out"};

        sparta::DataInPort<Credit> backend_fetch_credit_in
            {&unit_port_set_, "backend_fetch_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<InstPtr> backend_branch_resolve_inst_in
            {&unit_port_set_, "backend_branch_resolve_inst_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<InstGroupPtr> backend_bpu_inst_in
                {&unit_port_set_, "backend_bpu_inst_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<InstPtr> backend_redirect_pc_inst_in
                {&unit_port_set_, "backend_redirect_pc_inst_in", sparta::SchedulingPhase::Tick, 0};
    
    private:
    // Events
        sparta::SingleCycleUniqueEvent<> produce_inst_event_
            {&unit_event_set_, "produce_inst", CREATE_SPARTA_HANDLER(PerfectFrontend, ProduceInst)};

        sparta::SingleCycleUniqueEvent<sparta::SchedulingPhase::Trigger> startup_event_
            {&unit_event_set_, "startup_event", CREATE_SPARTA_HANDLER(PerfectFrontend, Startup_)};

        sparta::SingleCycleUniqueEvent<sparta::SchedulingPhase::PostTick> pmu_event
            {&unit_event_set_, "pmu_event", CREATE_SPARTA_HANDLER(PerfectFrontend, PmuMonitor_)};

    private:
        MavisUnit* mavis_ = nullptr;
        SelfAllocatorsUnit* allocator_;
        PmuUnit* pmu_;

    private:
        const uint64_t issue_num_;

        const bool is_speculation_;

        const bool is_config_;

        const std::string input_file_;

        Credit credit_ = 0;

        std::unique_ptr<InstGenerator> inst_generator_;

        std::string insn_gen_type_;
    };
}
