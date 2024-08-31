#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "PortInterface.hpp"

#include <deque>

namespace TimingModel {
    class PerfectLsu : public sparta::Unit {
    public:
        class PerfectLsuParameter : public sparta::ParameterSet {
        public:
            PerfectLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, issue_width, 1, "lsu issuing width")
            PARAMETER(uint32_t, load_to_use_latency, 1, "lsu load to use latency")
            PARAMETER(uint64_t, ld_queue_size, 20, "load queue size")
            PARAMETER(uint64_t, st_queue_size, 20, "store queue size")
        };

        static const char* name;

        PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void AcceptCredit_(const Credit&);

        void RobWakeUp(const InstPtr&);

        void AllocateInst_(const InstGroupPtr&);

        void RecieveInst_(const InstPtr&);

        void RecieveInsts_(const InstGroupPtr&);

        void SendInitCredit();

        void WriteBack_();

    private:
    /* ports */
    sparta::DataInPort<FlushingCriteria> lsu_flush_in
            {&unit_port_set_, "lsu_flush_in", sparta::SchedulingPhase::Flush, 1};

        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> preceding_func_insts_in
                {&unit_port_set_, "preceding_func_insts_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<Credit> write_back_func_credit_in
            {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 1};

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

        sparta::DataOutPort<InstGroupPtr> func_following_finishs_out
                {&unit_port_set_, "func_following_finishs_out"};

    /* events */

        sparta::SingleCycleUniqueEvent<> write_back_event
                {&unit_event_set_, "write_back_event", CREATE_SPARTA_HANDLER(PerfectLsu, WriteBack_)};

    private:
        const uint32_t ld_queue_size_;
        const uint32_t st_queue_size_;
        const uint32_t lsu_width_;
        const uint32_t load_to_use_latency_;

        uint32_t credit_ = 0;

        std::deque<InstPtr> lsu_queue_;
    };

}