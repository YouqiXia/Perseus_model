#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

// #include "basic/Instruction.hh"
#include "basic/Inst.hpp"
#include "olympia/InstGenerator.hpp"

#include "LoopQueue.hh"
#include <string>

namespace TimingModel {

    class PerfectFrontend : public sparta::Unit {
    public:
        class PerfectFrontendParameter : public sparta::ParameterSet {
        public:
            PerfectFrontendParameter(sparta::TreeNode* n):
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_num, 2, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, inst_queue_depth, 128, "the rob depth")
            PARAMETER(std::string, input_file, "", "the stf entry")
        };

        static const char* name;

        PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p);

        bool IsNextStageReady(IssueNum);

        void Trigger();

        InstGroup GetAvailInst();

        void Pop();

        InstPtr GetInstFromSTF();

        /* for test */
        void SetInst(InstPtr);

    public:
    // ports
        sparta::DataOutPort<InstGroup> fetch_backend_inst_out
            {&unit_port_set_, "fetch_backend_inst_out"};
    
    // Events
        sparta::UniqueEvent<> self_trigger 
            {&unit_event_set_, "frontend_trigger", CREATE_SPARTA_HANDLER(PerfectFrontend, Trigger)};

        sparta::Event<> pop_inst_queue
            {&unit_event_set_, "pop_inst_queue", CREATE_SPARTA_HANDLER(PerfectFrontend, Pop)};
    
    private:
        LoopQueue<InstPtr> inst_queue_;

        const uint64_t issue_num_;

        sparta::TreeNode* node_;

        MavisType* mavis_facade_ = nullptr;

        std::unique_ptr<InstGenerator> inst_generator_;
    };
}
