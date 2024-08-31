#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "PortInterface.hpp"

#include <deque>

namespace TimingModel {
    class PerfectAlu : public sparta::Unit {
    public:
        class PerfectAluParameter : public sparta::ParameterSet {
        public:
            PerfectAluParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, issue_width, 1, "alu issue width")
            PARAMETER(uint32_t, queue_depth, 1, "alu queue width")
        };

        static const char* name;

        PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p);

    private:
        void Process_(const InstPtr&);

        void Allocate_(const InstPtr&);

        void AllocateInsts_(const InstGroupPtr&);

        void SendInitCredit_();

        void HandleFlush_(const FlushingCriteria&);

        void WriteBack_();

        void AcceptCredit_(const Credit&);


    private:
    /* ports */
        sparta::DataInPort<InstPtr> preceding_func_inst_in
                {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> preceding_func_insts_in
                {&unit_port_set_, "preceding_func_insts_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<FlushingCriteria> func_flush_in
                {&unit_port_set_, "func_flush_in", sparta::SchedulingPhase::Flush, 1};

        sparta::DataInPort<Credit> write_back_func_credit_in
                {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> func_rs_credit_out
                {&unit_port_set_, "func_rs_credit_out"};

        sparta::DataOutPort<FuncInstPtr> func_following_finish_out
                {&unit_port_set_, "func_following_finish_out"};

        sparta::DataOutPort<InstGroupPtr> func_following_finishs_out
                {&unit_port_set_, "func_following_finishs_out"};

        sparta::DataOutPort<InstPtr> func_branch_resolve_inst_out
                {&unit_port_set_, "func_branch_resolve_out"};

    /* events */
        sparta::SingleCycleUniqueEvent<> write_back_event
            {&unit_event_set_, "write_back_event", CREATE_SPARTA_HANDLER(PerfectAlu, WriteBack_)};


    private:
        std::deque<InstPtr> alu_queue_;

        const uint32_t alu_width_;

        uint32_t credit_ = 0;
    };

}