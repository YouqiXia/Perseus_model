#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Instruction.hh"
#include "interface/functionunit/IperfectLsu.hh"


namespace TimingModel {
    class PerfectLsu : public sparta::Unit {
    public:
        class PerfectLsuParameter : public sparta::ParameterSet {
        public:
            PerfectLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
        };

        static const char* name;

        PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p);

        bool IsReady(IssueNum);

        void WriteBack(const InstGroup&);

        void Trigger() {}
        
    public:
    // ports
        sparta::DataInPort<InstGroup> backend_lsu_inst_in
            {&unit_port_set_, "backend_lsu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<RobIdx> lsu_backend_finish_out
            {&unit_port_set_, "lsu_backend_finish_out"};

    // Events
        sparta::UniqueEvent<> self_trigger 
            {&unit_event_set_, "perfectlsu_trigger", CREATE_SPARTA_HANDLER(PerfectLsu, Trigger)};

    private:
        IperfectLsu* perfect_lsu;

        const uint64_t load_to_use_latency_;
    };

}