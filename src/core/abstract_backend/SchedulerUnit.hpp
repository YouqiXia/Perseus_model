//
// Created by yzhang on 1/2/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>
#include <deque>
#include <vector>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"
#include "basic/SelfAllocatorsUnit.hpp"
#include "basic/GlobalParamUnit.hpp"

#include "simulation/PmuUnit.hpp"
#include "SpecBusyTableUnit.hpp"

namespace TimingModel {

    class SchedulerUnit : public sparta::Unit {
    public:
        class ReservationStationParameter : public sparta::ParameterSet {
        public:
            ReservationStationParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, pipe_rank, 0, "the rank of which pipeline the unit is")
            PARAMETER(uint64_t, issue_width, 1, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, queue_depth, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(bool, is_spec_wakeup, false, "if instruction can be speculatively waked up")
            PARAMETER(uint64_t, wakeup_latency, 0, "the latency of speculative wakeup")
        };

        class ReservationTable {
        public:
            ReservationTable(PhyRegId_t phy_reg_num) :
            scheduler_table_(phy_reg_num, std::vector<std::vector<ReStationEntryPtr>>(2)) {}

            void Allocate(ReStationEntryPtr rs_entry_ptr) {
                if (!rs_entry_ptr->rs1_valid) {
                    scheduler_table_[rs_entry_ptr->inst_ptr->getPhyRs1()][0].emplace_back(rs_entry_ptr);
                }
                if (!rs_entry_ptr->rs2_valid) {
                    scheduler_table_[rs_entry_ptr->inst_ptr->getPhyRs2()][1].emplace_back(rs_entry_ptr);
                }
            }

            bool Resolve(InstPtr inst_ptr) {
                bool find = false;
                for (auto& rs_entry_ptr: scheduler_table_[inst_ptr->getPhyRd()][0]) {
                    rs_entry_ptr->rs1_valid = true;
                    rs_entry_ptr->inst_ptr->setIsRs1Forward(false);
                    find = true;
                }
                scheduler_table_[inst_ptr->getPhyRd()][0].clear();

                for (auto& rs_entry_ptr: scheduler_table_[inst_ptr->getPhyRd()][1]) {
                    rs_entry_ptr->rs2_valid = true;
                    rs_entry_ptr->inst_ptr->setIsRs2Forward(false);
                    find = true;
                }
                scheduler_table_[inst_ptr->getPhyRd()][1].clear();


                return find;
            }

            bool EarlyWakeup(PhyRegId_t phy_reg_idx) {
                bool find = false;
                for (auto& rs_entry_ptr: scheduler_table_[phy_reg_idx][0]) {
                    rs_entry_ptr->rs1_spec_wakeup = true;
                    find = true;
                }

                for (auto& rs_entry_ptr: scheduler_table_[phy_reg_idx][1]) {
                    rs_entry_ptr->rs2_spec_wakeup = true;
                    find = true;
                }

                return find;
            }

        private:
            std::vector<std::vector<std::vector<ReStationEntryPtr>>> scheduler_table_;
        };

        static const char* name;

        SchedulerUnit(sparta::TreeNode* node, const ReservationStationParameter* p);

    private: // port binding functions
        void Startup_();

        void HandleFlush_(const FlushingCriteria&);

        void AllocateReStation(const InstGroupPairPtr&);

        void GetForwardingData(const InstGroupPtr&);

        void AcceptCredit_(const CreditPairPtr&);

        void WakeupResolve_(const InstGroupPtr&);

        void SpecWakeup_(const InstGroupPtr&);

    private: // inner implementation
        void InitCredit_();

        void PopInst_();

        void ProcessInsts_();

        uint64_t GetProduceNum_();

        void SelectInst_(uint64_t produce_num,
                         InstGroupPtr processed_group_ptr);

        void EarlyWakeupCtrl(InstGroupPtr processed_group_ptr);

        void DataTransfer_(InstGroupPtr processed_group_ptr);

        void SizeUp_() { ++size_; }

        void SizeDown_() { --size_; }

    private: // pmu implementation
        void PmuBandwidthAcc_(uint64_t produce_num_max, uint64_t produce_num);

        void PmuMonitor_();

    private:
        // ports
        // flush
        sparta::DataInPort<FlushingCriteria> scheduler_flush_in
                {&unit_port_set_, "scheduler_flush_in", sparta::SchedulingPhase::Flush, 1};

        // with issue queue
        sparta::DataInPort<InstGroupPairPtr> preceding_scheduler_inst_in
                {&unit_port_set_, "preceding_scheduler_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<CreditPairPtr> scheduler_preceding_credit_out
                {&unit_port_set_, "scheduler_preceding_credit_out"};

        // with function unit
        sparta::DataOutPort<InstGroupPtr> scheduler_following_inst_out
                {&unit_port_set_, "scheduler_following_inst_out"};

        // latency = 1 due to the combinational logic model of following stage
        sparta::DataInPort<CreditPairPtr> following_scheduler_credit_in
                {&unit_port_set_, "following_scheduler_credit_in", sparta::SchedulingPhase::Tick, 1};

        // with CDB
        sparta::DataInPort<InstGroupPtr> forwarding_scheduler_inst_in
                {&unit_port_set_, "forwarding_scheduler_inst_in", sparta::SchedulingPhase::Tick, 1};

        // speculative wakeup
        sparta::DataOutPort<InstGroupPtr> spec_wake_up_out
                {&unit_port_set_, "spec_wake_up_out"};

        sparta::DataInPort<InstGroupPtr> spec_wake_up_in
                {&unit_port_set_, "spec_wake_up_in", sparta::SchedulingPhase::Tick, 1};

        // early wake-up resolve
        sparta::DataInPort<InstGroupPtr> wakeup_resolve_in
                {&unit_port_set_, "wakeup_resolve_in", sparta::SchedulingPhase::Tick, 1};

        // events
        sparta::SingleCycleUniqueEvent<> process_event
                {&unit_event_set_, "process_event", CREATE_SPARTA_HANDLER(SchedulerUnit, ProcessInsts_)};

        sparta::SingleCycleUniqueEvent<> spec_wakeup_event
                {&unit_event_set_, "spec_wakeup_event", CREATE_SPARTA_HANDLER(SchedulerUnit, PmuMonitor_)};

        sparta::SingleCycleUniqueEvent<sparta::SchedulingPhase::PostTick> pmu_event
                {&unit_event_set_, "pmu_event", CREATE_SPARTA_HANDLER(SchedulerUnit, PmuMonitor_)};

    private:
        GlobalParamUnit* global_param_ptr_ = nullptr;
        SelfAllocatorsUnit* allocator_;
        PmuUnit* pmu_;
        GlobalParamUnit::FuLatencyMap* fu_latency_map_;

    private:
        size_t size_ = 0;

        const uint64_t pipe_rank_;
        const uint64_t wakeup_latency_;
        const bool is_spec_wakeup_;
        const uint64_t issue_num_;
        const uint64_t rs_depth_;

        std::deque<ReStationEntryPtr> issue_window_;
        std::unordered_map<PhyRegId_t, uint64_t> latency_table_;
        ReservationTable dependency_table_;
        Credit credit_ = 0;
    };
}