#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"

#include "WriteBackStage.hpp"

namespace TimingModel {
    class PerfectAlu : public sparta::Unit {
    public:
        class PerfectAluParameter : public sparta::ParameterSet {
        public:
            PerfectAluParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(FuncUnitType, func_type, "", "the type of function unit")
        };

        static const char* name;

        PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p);

    private:
        void Process_(const InstPtr&);

        void WriteBack_(const InstPtr&);

        void AcceptCredit_(const Credit&);


    private:
    // ports
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<Credit> write_back_func_credit_in
                {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<Credit> func_rs_credit_out
                {&unit_port_set_, "func_rs_credit_out"};

        sparta::DataOutPort<FuncInstPtr> func_following_finish_out
            {&unit_port_set_, "func_following_finish_out"};


    private:
        FuncUnitType func_type_;
    };

}