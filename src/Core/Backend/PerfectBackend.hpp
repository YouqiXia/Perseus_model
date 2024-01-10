#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "interface/backend/IRob.hh"

#include "sparta/statistics/Counter.hpp"
#include "sparta/statistics/StatisticDef.hpp"
#include "sparta/statistics/StatisticInstance.hpp"

#include <vector>

namespace TimingModel {
    class PerfectBackend : public sparta::Unit {
    public:
        class PerfectBackendParameter : public sparta::ParameterSet {
        public:
            PerfectBackendParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_num, 2, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, rob_depth, 8, "the rob depth")
        };

        static const char* name;

        PerfectBackend(sparta::TreeNode* node, const PerfectBackendParameter* p);

        ~PerfectBackend() final;

        void AcceptCreditFromALU(const Credit&);

        void AcceptCreditFromLSU(const Credit&);

        // should be an observe pattern here
        void Issue();

        /* about rob */
        void AllocateRobEntry(const InstGroup&);

        void Finish(const RobIdx&);

        void LSQFinish(const InstGroup&);

        void WriteBack(const InstGroup&);

        void SetRobIssued(const RobIdx&);

        void RobCommit();

        void RobWkupStore();

        void SendInitCredit();

    public:
    // ports
        sparta::DataInPort<InstGroup> fetch_backend_inst_in
            {&unit_port_set_, "fetch_backend_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroup> backend_lsu_inst_out
            {&unit_port_set_, "backend_lsu_inst_out"};

        sparta::DataOutPort<InstGroup> backend_alu_inst_out
            {&unit_port_set_, "backend_alu_inst_out"};

        sparta::DataInPort<RobIdx> alu_backend_finish_in
            {&unit_port_set_, "alu_backend_finish_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroup> lsu_backend_finish_in
            {&unit_port_set_, "lsu_backend_finish_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroup> lsu_backend_wr_data_in
            {&unit_port_set_, "lsu_backend_wr_data_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> fetch_backend_credit_out
            {&unit_port_set_, "fetch_backend_credit_out"};

        sparta::DataOutPort<RobIdx> backend_lsu_rob_idx_wakeup_out
            {&unit_port_set_, "backend_lsu_rob_idx_wakeup_out"};

        sparta::DataInPort<RobIdx> alu_backend_credit_in
            {&unit_port_set_, "alu_backend_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<RobIdx> lsu_backend_credit_in
            {&unit_port_set_, "lsu_backend_credit_in", sparta::SchedulingPhase::Tick, 0};

    private:
    // Events
        sparta::SingleCycleUniqueEvent<> issue_event_
            {&unit_event_set_, "issue_event", CREATE_SPARTA_HANDLER(PerfectBackend, Issue)};

        sparta::UniqueEvent<> set_rob_commmited
            {&unit_event_set_, "set_rob_commmited", CREATE_SPARTA_HANDLER(PerfectBackend, RobCommit)};

        sparta::UniqueEvent<> rob_wakeup_st
            {&unit_event_set_, "rob_wakeup_st", CREATE_SPARTA_HANDLER(PerfectBackend, RobWkupStore)};

    private:
        IIntegrateRob* rob_;

        uint64_t alu_credit_;

        uint64_t lsu_credit_;

        const uint64_t issue_num_;

        RobIdx rob_depth_;

        sparta::Counter             num_retired_;
        sparta::StatisticDef        stat_ipc_;
        sparta::StatisticInstance   ipc_;

        const uint64_t retire_heartbeat_ = 1000000;

        // optimization: store the pointer of next stage instead
        sparta::TreeNode* node_;
    };
}