#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "interface/backend/IRob.hh"

#include <vector>

namespace TimingModel {
    class PerfectBackend : public sparta::Unit {
    public:
        class PerfectBackendParameter : public sparta::ParameterSet {
        public:
            PerfectBackendParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_num, 2, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, rob_depth, 8, "the rob depth")
        };

        static const char* name;

        PerfectBackend(sparta::TreeNode* node, const PerfectBackendParameter* p);

        ~PerfectBackend() final;

        bool IsReady(IssueNum);

        bool IsAluReady(IssueNum);

        bool IsLsuReady(IssueNum);

        void Trigger();

        /* about rob */
        void AllocateRobEntry(const InstGroup&);

        void Finish(const RobIdx&);

        void SetRobIssued(const RobIdx&);

        void RobCommit();

    public:
    // ports
        sparta::DataInPort<InstGroup> fetch_backend_inst_in
            {&unit_port_set_, "fetch_backend_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroup> backend_lsu_inst_out
            {&unit_port_set_, "backend_lsu_inst_out"};

        sparta::DataOutPort<InstGroup> backend_alu_inst_out
            {&unit_port_set_, "backend_alu_inst_out"};

        sparta::DataInPort<RobIdx> alu_backend_finish_in
            {&unit_port_set_, "alu_backend_finish_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<RobIdx> lsu_backend_finish_in
            {&unit_port_set_, "lsu_backend_finish_in", sparta::SchedulingPhase::Tick, 1};

    // Events
        sparta::UniqueEvent<> self_trigger 
            {&unit_event_set_, "backend_trigger", CREATE_SPARTA_HANDLER(PerfectBackend, Trigger)};
        
        sparta::PayloadEvent<RobIdx> set_rob_issued
            {&unit_event_set_, "set_rob_issued", CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, SetRobIssued, RobIdx)};

        sparta::UniqueEvent<> set_rob_commmited
            {&unit_event_set_, "set_rob_commmited", CREATE_SPARTA_HANDLER(PerfectBackend, RobCommit)};

    private:
        IIntegrateRob* rob_;

        const uint64_t issue_num_;

        // optimization: store the pointer of next stage instead
        sparta::TreeNode* node_;
    };
}