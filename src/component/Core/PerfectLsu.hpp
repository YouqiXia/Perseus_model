#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
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

        void WriteBack(const InstGroup&);

        void SendInitCredit();

    public:
    // ports
        sparta::DataInPort<InstGroup> backend_lsu_inst_in
            {&unit_port_set_, "backend_lsu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> backend_lsu_credit_out
            {&unit_port_set_, "backend_lsu_credit_out"};

        sparta::DataOutPort<RobIdx> lsu_backend_finish_out
            {&unit_port_set_, "lsu_backend_finish_out"};

    private:
        IperfectLsu* perfect_lsu;

        const uint64_t load_to_use_latency_;
    };

}