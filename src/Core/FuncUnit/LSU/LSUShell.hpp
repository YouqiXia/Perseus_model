#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"

#include "Core/FuncUnit/WriteBackStage.hpp"

namespace TimingModel {
    class LSUShell : public sparta::Unit {
    public:
        class LSUShellParameter : public sparta::ParameterSet {
        public:
            LSUShellParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(FuncUnitType, func_type, "", "the type of function unit")
        };

        static const char* name;

        LSUShell(sparta::TreeNode* node, const LSUShellParameter* p);

    private:
        void ByPassPrecedingInst_(const InstPtr&);

        void WriteBack_(const InstPtr&);

        void AcceptCredit_(const Credit&);

        void ByPassAcceptCredit_(const Credit&);

    public:
    // ports
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstPtr> preceding_func_inst_bp_out
            {&unit_port_set_, "preceding_func_inst_bp_out"};

        sparta::DataInPort<Credit> write_back_func_credit_in
            {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<Credit> write_back_func_credit_bp_out
            {&unit_port_set_, "write_back_func_credit_bp_out"};

        sparta::DataOutPort<Credit> func_rs_credit_out
            {&unit_port_set_, "func_rs_credit_out"};

        sparta::DataInPort<Credit> func_rs_credit_bp_in
            {&unit_port_set_, "func_rs_credit_bp_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<FuncInstPtr> func_following_finish_out
            {&unit_port_set_, "func_following_finish_out"};
        
        sparta::DataInPort<InstPtr> func_following_finish_bp_in
            {&unit_port_set_, "func_following_finish_bp_in", sparta::SchedulingPhase::Tick, 0};

    private:
        FuncUnitType func_type_;
    };

}