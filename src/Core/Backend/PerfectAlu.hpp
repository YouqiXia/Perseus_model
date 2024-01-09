#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"


namespace TimingModel {
    class PerfectAlu : public sparta::Unit {
    public:
        class PerfectAluParameter : public sparta::ParameterSet {
        public:
            PerfectAluParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}
	    PARAMETER(uint64_t, init_alu_credits, 2, "initial alu credits to BE")
        };

        static const char* name;

        PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p);

        void WriteBack(const InstGroup&);

        void SendInitCredit();
    
    private:
        const uint64_t init_alu_credits_;
        
    public:
    // ports
        sparta::DataInPort<InstGroup> backend_alu_inst_in
            {&unit_port_set_, "backend_alu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> backend_alu_credit_out
            {&unit_port_set_, "backend_alu_credit_out"};

        sparta::DataOutPort<RobIdx> alu_backend_finish_out
            {&unit_port_set_, "alu_backend_finish_out"};

    };

}
