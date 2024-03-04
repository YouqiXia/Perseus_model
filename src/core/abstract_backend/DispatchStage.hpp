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
#include "FuncUnits.hpp"
#include "PortInterface.hpp"

#include "Scoreboard.hpp"

namespace TimingModel {

    class DispatchStage : public sparta::Unit {
    public:
        class DispatchStageParameter : public sparta::ParameterSet {
        public:
            DispatchStageParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, issue_num, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, issue_queue_depth, 16, "the issuing bandwidth in a cycle")
        };

        struct IssueQueueEntry {
            InstPtr inst_ptr;
            bool is_issued = false;
        };

        using IssueQueueEntryPtr = sparta::SpartaSharedPointer<IssueQueueEntry>;

        static const char *name;

        DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void InitCredit_();

        void AcceptCredit_(const RsCreditPtr&);

        void AllocateInst_(const InstGroupPtr&);

        void CheckRegStatus_();

        void CheckRegStatusImp_(InstPtr&);

        void FuncUnitBack_(const InstGroupPtr&);

        void ReadPhyReg_();

        void ReadFromPhysicalReg_(const InstGroupPtr&);

        void IssueInst_();

        void SelectInst_();

        void PopIssueQueue_();

    private:
        // ports
        
            //flush
            sparta::DataInPort<FlushingCriteria> dispatch_flush_in
                {&unit_port_set_, "dispatch_flush_in", sparta::SchedulingPhase::Flush, 1};

            // with renaming
            sparta::DataInPort<InstGroupPtr> preceding_dispatch_inst_in
                {&unit_port_set_, "preceding_dispatch_inst_in", sparta::SchedulingPhase::Tick, 1};

            sparta::DataOutPort<Credit> dispatch_preceding_credit_out
                {&unit_port_set_, "dispatch_preceding_credit_out"};

            // with physical register file
            sparta::DataOutPort<InstGroupPtr> dispatch_physical_reg_read_out
                    {&unit_port_set_, "dispatch_physical_reg_read_out"};

            sparta::DataInPort<InstGroupPtr> physical_reg_dispatch_read_in
                    {&unit_port_set_, "dispatch_physical_reg_read_in", sparta::SchedulingPhase::Tick, 1};

            // with rs -> also should be constructed in dispatch stage constructor
            std::map<RsType, sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>>> dispatch_rs_out;

            // with rs Credits
            std::vector<sparta::SpartaSharedPointer<sparta::DataInPort<RsCreditPtr>>> rs_dispatch_credits_in;

            // write back ports
            sparta::DataInPort<InstGroupPtr> write_back_dispatch_port_in
                    {&unit_port_set_, "write_back_dispatch_port_in", sparta::SchedulingPhase::Tick, 1};


        // events
            sparta::SingleCycleUniqueEvent<> dispatch_select_events_
                {&unit_event_set_, "dispatch_select_events_", CREATE_SPARTA_HANDLER(DispatchStage, SelectInst_)};

            sparta::SingleCycleUniqueEvent<> dispatch_issue_events_
                {&unit_event_set_, "dispatch_issue_events_", CREATE_SPARTA_HANDLER(DispatchStage, IssueInst_)};

            sparta::SingleCycleUniqueEvent<> dispatch_pop_events_
                {&unit_event_set_, "dispatch_pop_events_", CREATE_SPARTA_HANDLER(DispatchStage, PopIssueQueue_)};

            sparta::SingleCycleUniqueEvent<> dispatch_scoreboard_events_
                {&unit_event_set_, "dispatch_scoreboard_events_", CREATE_SPARTA_HANDLER(DispatchStage, CheckRegStatus_)};

            sparta::SingleCycleUniqueEvent<> dispatch_get_operator_events_
                {&unit_event_set_, "dispatch_get_operator_events_", CREATE_SPARTA_HANDLER(DispatchStage, ReadPhyReg_)};

    private:
        uint64_t issue_num_;

        std::map<RsType, Credit> rs_credits_;

        Scoreboard scoreboard_;

        const uint64_t inst_queue_depth_;

        sparta::Queue<IssueQueueEntryPtr> inst_queue_;

        std::map<RsType, InstPtr> dispatch_pending_queue_;
    };

}