#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "interface/functionunit/IperfectAlu.hh"


namespace TimingModel {
    class PerfectAlu : public sparta::Unit {
    public:
        class PerfectAluParameter : public sparta::ParameterSet {
        public:
            PerfectAluParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}
        };

        static const char* name;

        PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p);

        bool IsReady(IssueNum);

        void WriteBack(const InstGroup&);

        void Trigger() {}
        
    public:
    // ports
        sparta::DataInPort<InstGroup> backend_alu_inst_in
            {&unit_port_set_, "backend_alu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<RobIdx> alu_backend_finish_out
            {&unit_port_set_, "alu_backend_finish_out"};

    // Events
        sparta::UniqueEvent<> self_trigger 
            {&unit_event_set_, "perfectlsu_trigger", CREATE_SPARTA_HANDLER(PerfectAlu, Trigger)};
    };

}