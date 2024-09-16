//
// Created by yzhang on 1/6/24.
//

#pragma once

#include <deque>

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"
#include "basic/GlobalParam.hpp"

namespace TimingModel {

    class WriteBackStage : public sparta::Unit {
    public:
        class WriteBackStageParameter : public sparta::ParameterSet {
        public:
            WriteBackStageParameter(sparta::TreeNode* n) :
                    sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_width, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, wb_latency, 1, "write back latency")
            PARAMETER(bool, is_perfect_mode, false, "write back latency")
        };

        static const char* name;

        WriteBackStage(sparta::TreeNode* node, const WriteBackStageParameter* p);

        ~WriteBackStage() {}

    private:
        void HandleFlush_(const FlushingCriteria&);

        void AcceptFuncInst_(const InstGroupPairPtr&);

        void SendInitCredit_();

        void ArbitrateInst_();

    private:
        // ports
        sparta::DataOutPort<InstGroupPtr> write_back_following_port_out
                {&unit_port_set_, "write_back_following_port_out"};

        sparta::DataOutPort<CreditPairPtr> preceding_write_back_credit_out
                {&unit_port_set_, "preceding_write_back_credit_out"};

        sparta::DataInPort<FlushingCriteria> writeback_flush_in
                {&unit_port_set_, "writeback_flush_in", sparta::SchedulingPhase::Flush, 1};

        sparta::DataInPort<InstGroupPairPtr> preceding_write_back_inst_in
                {&unit_port_set_, "preceding_write_back_inst_in", sparta::SchedulingPhase::Tick, 0};


        // events
        sparta::SingleCycleUniqueEvent<> arbitrate_inst_event
                {&unit_event_set_, "arbitrate_inst_event", CREATE_SPARTA_HANDLER(WriteBackStage, ArbitrateInst_)};

    private:
        uint64_t issue_num_;

        bool is_wb_perfect_;

        GlobalParam* global_param_ptr_ = nullptr;

        uint64_t wb_latency_;

        std::map<std::string, std::deque<InstPtr>> inst_queue_map_;

        std::map<std::string, Credit> write_back_width_map_;
    };

}