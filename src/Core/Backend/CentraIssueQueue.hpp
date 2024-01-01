//
// Created by yzhang on 1/1/24.
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

    class CentraIssueQueue : public sparta::Unit {
    public:
        class CentraIssueQueueParameter : public sparta::ParameterSet {
        public:
            CentraIssueQueueParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, issue_width, 2, "the issuing bandwidth in a cycle")
        };

        static const char* name;

        CentraIssueQueue(sparta::TreeNode* node, const CentraIssueQueueParameter* p);

    private:
        void AcceptCredit(const Credit&);

        void AllocateInst(const InstGroupPtr&);

        void RenameInst();

        void HandleFlush(const FlushingCriteria&);

        void RenameInstImp_(const InstPtr&);

        void RollBack_(const InstGroupPtr&);

        void RobCommit_(const InstGroupPtr&);

    private:
        // ports
        // flush
        sparta::DataInPort<FlushingCriteria> issue_queue_flush_in
                {&unit_port_set_, "issueQueue_flush_in", sparta::SchedulingPhase::Tick, 1};

        // with Renaming
        sparta::DataInPort<InstGroupPtr> preceding_issue_queue_inst_in
                {&unit_port_set_, "preceding_issue_queue_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> issue_queue_preceding_credit_out
                {&unit_port_set_, "issue_queue_preceding_credit_out"};

        // with Physical Register File

        // with Scoreboard

        // with Reservation Station -> need to be scalable in constructor
        sparta::DataOutPort<InstGroupPtr> issue_queue_following_inst_out
                {&unit_port_set_, "issue_queue_following_inst_out"};

        sparta::DataInPort<Credit> following_issue_queue_credit_in
                {&unit_port_set_, "following_issue_queue_credit_in", sparta::SchedulingPhase::Tick, 0};


        // events

        sparta::SingleCycleUniqueEvent<> rename_event;

    private:
        InstQueue renaming_stage_queue_;

        const uint64_t issue_width_;

        Credit credit_ = 0;

    };

}