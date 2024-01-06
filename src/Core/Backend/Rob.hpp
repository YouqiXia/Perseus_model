//
// Created by yzhang on 1/2/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

    class Rob : public sparta::Unit {
    public:
        class RobParameter : public sparta::ParameterSet {
        public:
            RobParameter(sparta::TreeNode* n) :
                    sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, issue_width, 2, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, rob_depth, 32, "the number of isa register file")
        };

        static const char* name;

        Rob(sparta::TreeNode* node, const RobParameter* p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void Finish_(const InstPtr&);

        void AllocateRob_(const InstGroupPtr&);

        void Commit_();

    private:
        // ports
        // flush
        sparta::DataInPort<FlushingCriteria> rob_flush_in
                {&unit_port_set_, "rob_flush_in", sparta::SchedulingPhase::Tick, 1};

        // with frontend
        sparta::DataInPort<InstGroupPtr> preceding_rob_inst_in
                {&unit_port_set_, "preceding_rob_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> rob_preceding_credit_out
                {&unit_port_set_, "rob_preceding_credit_out"};

        // with function unit
        sparta::DataInPort<InstPtr> func_unit_rob_finish_in
                {&unit_port_set_, "following_renaming_credit_in", sparta::SchedulingPhase::Tick, 1};

        // with Renaming table
        sparta::DataOutPort<InstGroupPtr> Rob_cmt_inst_out
                {&unit_port_set_, "Rob_cmt_inst_out"};


        // events

        sparta::SingleCycleUniqueEvent<> commit_event
                {&unit_event_set_, "commit_event", CREATE_SPARTA_HANDLER(Rob, Commit_)};

    private:
        InstQueue rob_;

        sparta::Queue<bool> finish_queue_;

        const uint64_t issue_width_;

        Credit credit_ = 0;

    };

}