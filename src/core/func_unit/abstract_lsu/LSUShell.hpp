#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"

namespace TimingModel {
    class LSUShell : public sparta::Unit {
    public:
        class LSUShellParameter : public sparta::ParameterSet {
        public:
            LSUShellParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, issue_width, 1, "the type of function unit")
        };

        static const char* name;

        LSUShell(sparta::TreeNode* node, const LSUShellParameter* p);

    private:
        void preceding_func_inst_in_(const InstPtr&);

        void write_back_func_credit_in_(const Credit&);

        void func_rs_credit_bp_in_(const Credit&);

        void renaming_lsu_allocate_in_(const InstGroupPtr&);

        void Rob_lsu_wakeup_in_(const InstPtr&);

        void lsu_renaming_allocate_bp_in_(const InstGroupPtr&);

        void lsu_renaming_ldq_credit_bp_in_(const Credit&);

        void lsu_renaming_stq_credit_bp_in_(const Credit&);

        void func_following_finish_bp_in_(const InstGroupPtr&);

    public:
    // ports
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<InstGroupPtr> preceding_func_inst_bp_out
                {&unit_port_set_, "preceding_func_inst_bp_out"};


        sparta::DataInPort<Credit> write_back_func_credit_in
                {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<Credit> write_back_func_credit_bp_out
                {&unit_port_set_, "write_back_func_credit_bp_out"};


        sparta::DataOutPort<Credit> func_rs_credit_out
                {&unit_port_set_, "func_rs_credit_out"};

        sparta::DataInPort<Credit> func_rs_credit_bp_in
                {&unit_port_set_, "func_rs_credit_bp_in", sparta::SchedulingPhase::Tick, 0};


        sparta::DataInPort<InstGroupPtr> renaming_lsu_allocate_in
                {&unit_port_set_, "renaming_lsu_allocate_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<InstGroupPtr> renaming_lsu_allocate_bp_out
                {&unit_port_set_, "renaming_lsu_allocate_bp_out"};


        sparta::DataInPort<InstPtr> Rob_lsu_wakeup_in
                {&unit_port_set_, "Rob_lsu_wakeup_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<InstPtr> Rob_lsu_wakeup_bp_out
                {&unit_port_set_, "Rob_lsu_wakeup_bp_out"};


        sparta::DataOutPort<InstGroupPtr> lsu_renaming_allocate_out
                {&unit_port_set_, "lsu_renaming_allocate_out"};

        sparta::DataInPort<InstGroupPtr> lsu_renaming_allocate_bp_in
                {&unit_port_set_, "lsu_renaming_allocate_bp_in", sparta::SchedulingPhase::Tick, 0};


        sparta::DataOutPort<Credit> lsu_renaming_ldq_credit_out
                {&unit_port_set_, "lsu_renaming_ldq_credit_out"};

        sparta::DataInPort<Credit> lsu_renaming_ldq_credit_bp_in
                {&unit_port_set_, "lsu_renaming_ldq_credit_bp_in", sparta::SchedulingPhase::Tick, 0};


        sparta::DataOutPort<Credit> lsu_renaming_stq_credit_out
                {&unit_port_set_, "lsu_renaming_stq_credit_out"};

        sparta::DataInPort<Credit> lsu_renaming_stq_credit_bp_in
                {&unit_port_set_, "lsu_renaming_stq_credit_bp_in", sparta::SchedulingPhase::Tick, 0};


        std::vector<sparta::DataOutPort<FuncInstPtr>*> func_following_finish_out_ports;

        sparta::DataInPort<InstGroupPtr> func_following_finish_bp_in
                {&unit_port_set_, "func_following_finish_bp_in", sparta::SchedulingPhase::Tick, 0};

    private:
        const uint32_t issue_width_;

    };

}