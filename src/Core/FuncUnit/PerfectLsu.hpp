#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"

#include "WriteBackStage.hpp"

namespace TimingModel {
    class PerfectLsu : public sparta::Unit {
    public:
        class PerfectLsuParameter : public sparta::ParameterSet {
        public:
            PerfectLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
            PARAMETER(uint64_t, ld_queue_size, 256, "load queue size")
            PARAMETER(uint64_t, st_queue_size, 256, "store queue size")
            PARAMETER(FuncUnitType, func_type, "", "the type of function unit")
        };

        static const char* name;

        PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p);

    private:
        void WriteBack_(const InstPtr&);

        void AcceptCredit_(const Credit&);

        void RobWakeUp(const InstPtr&);

        void AllocateInst_(const InstGroupPtr&);

        void SendInitCredit();

    public:
    // ports
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<Credit> write_back_func_credit_in
            {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<Credit> func_rs_credit_out
            {&unit_port_set_, "func_rs_credit_out"};

        sparta::DataInPort<InstGroupPtr> renaming_lsu_allocate_in
            {&unit_port_set_, "renaming_lsu_allocate_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstPtr> Rob_lsu_wakeup_in
            {&unit_port_set_, "Rob_lsu_wakeup_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroupPtr> lsu_renaming_allocate_out
            {&unit_port_set_, "lsu_renaming_allocate_out"};

        sparta::DataOutPort<Credit> lsu_renaming_ldq_credit_out
            {&unit_port_set_, "lsu_renaming_ldq_credit_out"};

        sparta::DataOutPort<Credit> lsu_renaming_stq_credit_out
            {&unit_port_set_, "lsu_renaming_stq_credit_out"};

        sparta::DataOutPort<FuncInstPtr> func_following_finish_out
            {&unit_port_set_, "func_following_finish_out"};

    private:
        const uint64_t load_to_use_latency_;
        const uint32_t ld_queue_size_;
        const uint32_t st_queue_size_;
        FuncUnitType func_type_;
    };

}