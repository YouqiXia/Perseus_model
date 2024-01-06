//
// Created by yzhang on 1/1/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/utils/ValidValue.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "FuncUnits.hpp"

#include "PhyRegfileReadPort.hpp"
#include "PhyRegfileWritePort.hpp"
#include "ReservationStation.hpp"
#include "Scoreboard.hpp"

namespace TimingModel {

    class DispatchStage : public sparta::Unit {
    public:
        class DispatchStageParameter : public sparta::ParameterSet {
        public:
            DispatchStageParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, issue_num, 1, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(uint32_t, issue_queue_depth, 16, "the issuing bandwidth in a cycle")
            PARAMETER(uint8_t, func_unit_num, 4, "the number of read ports")
            PARAMETER(uint8_t, reg_write_port_num, 4, "the number of read ports")
        };

        struct IssueQueueEntry {
            InstPtr inst_ptr;
            bool is_issued = false;
        };

        using IssueQueueEntryPtr = sparta::SpartaSharedPointer<IssueQueueEntry>;

        using ReadPortOut = sparta::DataOutPort<PhyRegfileRequestPtr>;
        using ReadPortOutPtr = sparta::SpartaSharedPointer<ReadPortOut>;
        using ReadPortIn = sparta::DataInPort<PhyRegfileRequestPtr>;
        using ReadPortInPtr = sparta::SpartaSharedPointer<ReadPortIn>;

        using WritePortIn = sparta::DataInPort<PhyRegfileWriteBackPtr>;
        using WritePortInPtr = sparta::SpartaSharedPointer<WritePortIn>;

        static const char *name;

        DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p);

    private:
        void HandleFlush_(const FlushingCriteria&);

        void bindTree_();

        void InitCredit_();

        void AcceptCredit_(const RsCreditPtr&);

        void AllocateInst_(const InstGroupPtr&);

        void CheckRegStatus_();

        void CheckRegStatusImp_(InstPtr&);

        void FuncUnitBack_(const InstGroupPtr&);

        void ReadPhyReg_();

        void ReadFromPhyReg_(const PhyRegfileRequestPtr&);

        void IssueInst_();

        void SelectInst_();


    private:
        // ports
        
            //flush
            sparta::DataInPort<FlushingCriteria> dispatch_flush_in
                {&unit_port_set_, "dispatch_flush_in", sparta::SchedulingPhase::Tick, 1};

            // with Rs Credits
            std::vector<sparta::SpartaSharedPointer<sparta::DataInPort<RsCreditPtr>>> rs_dispatch_credits_in;

            // with renaming
            sparta::DataInPort<InstGroupPtr> preceding_dispatch_inst_in
                {&unit_port_set_, "preceding_dispatch_inst_in", sparta::SchedulingPhase::Tick, 1};

            sparta::DataOutPort<Credit> dispatch_preceding_credit_out
                {&unit_port_set_, "dispatch_preceding_credit_out"};

            // with physical register -> should be constructed in dispatch stage constructor
            std::vector<ReadPortOutPtr> dispatch_read_reg_ports_out;

            std::vector<ReadPortInPtr> read_reg_dispatch_ports_in;

            // physical register ports
            std::vector<ReadPortOutPtr> phy_reg_read_ports_out;

            std::vector<ReadPortInPtr> phy_reg_read_ports_in;

            std::vector<WritePortInPtr> phy_reg_write_port_in;

            // with rs -> also should be constructed in dispatch stage constructor
            std::map<FuncUnitType, sparta::SpartaSharedPointer<sparta::DataOutPort<InstPtr>>> dispatch_rs_out;

            //with rob
            sparta::DataInPort<InstGroupPtr> Rob_cmt_inst_in
                    {&unit_port_set_, "Rob_cmt_inst_in", sparta::SchedulingPhase::Tick, 0};

        // events
            sparta::SingleCycleUniqueEvent<> dispatch_select_events_
                {&unit_event_set_, "dispatch_issue_events_", CREATE_SPARTA_HANDLER(DispatchStage, SelectInst_)};

            sparta::SingleCycleUniqueEvent<> dispatch_issue_events_
                {&unit_event_set_, "dispatch_issue_events_", CREATE_SPARTA_HANDLER(DispatchStage, IssueInst_)};

            sparta::SingleCycleUniqueEvent<> dispatch_scoreboard_events_
                {&unit_event_set_, "dispatch_scoreboard_events_", CREATE_SPARTA_HANDLER(DispatchStage, CheckRegStatus_)};

            sparta::SingleCycleUniqueEvent<> dispatch_get_operator_events_
                {&unit_event_set_, "dispatch_get_operator_events_", CREATE_SPARTA_HANDLER(DispatchStage, ReadPhyReg_)};

    private:
        uint64_t issue_num_;

        std::map<FuncUnitType, Credit> rs_credits_;

        PhysicalReg phy_regfile_;

        Scoreboard scoreboard_;

        const uint64_t inst_queue_depth_;

        sparta::Queue<IssueQueueEntryPtr> inst_queue_;

        std::map<FuncUnitType, InstPtr> dispatch_pending_queue_;

        std::vector<std::unique_ptr<PhyRegfileReadPort>> to_delete_read_port_;
        std::vector<std::unique_ptr<PhyRegfileWritePort>> to_delete_write_port_;

    };

}