//
// Created by yzhang on 1/16/24.
//

#pragma once


#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/GlobalParamUnit.hpp"
#include "basic/SelfAllocatorsUnit.hpp"

#include "SchedulerUnit.hpp"
#include "Scoreboard.hpp"

namespace TimingModel {

    class PhysicalRegfileUnit : public sparta::Unit {
    public:
        class PhysicalRegfileParameter : public sparta::ParameterSet {
        public:
            PhysicalRegfileParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, issue_width, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, queue_depth, 8, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, latency, 1, "the latency of operand accessing")
            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
        };

        static const char *name;

        PhysicalRegfileUnit(sparta::TreeNode *node, const PhysicalRegfileParameter *p);

    private: // port binding
        void Startup_();

        void AcceptCredit_(const CreditPairPtr&);

        void HandleFlush_(const FlushingCriteria&);

        void RecieveInsts_(const InstGroupPtr&);

        void ReadPhysicalReg_(const InstGroupPtr&);

        void WritePhysicalReg_(const InstGroupPtr&);

    private: // inner implementation
        void InitCredit_();

        void ProcessInsts_();

        uint64_t GetProduceNum_();


    private: // pmu implementation
        void PmuBandwidthAcc_(uint64_t produce_num_max, uint64_t produce_num);

    private:
        /* ports */
        // read/write port
        sparta::DataInPort<InstGroupPtr> preceding_physical_regfile_read_in;

        sparta::DataOutPort<CreditPairPtr> preceding_credit_out
                {&unit_port_set_, "preceding_credit_out"};

        sparta::DataInPort<InstGroupPtr> preceding_physical_regfile_write_in
                {&unit_port_set_, "preceding_physical_regfile_write_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroupPtr> physical_regfile_following_read_out
                {&unit_port_set_, "physical_regfile_following_read_out"};

        sparta::DataInPort<CreditPairPtr> following_credit_in
                {&unit_port_set_, "following_credit_in", sparta::SchedulingPhase::Tick, 0};

        /* events */
        sparta::SingleCycleUniqueEvent<> process_event
                {&unit_event_set_, "process_event", CREATE_SPARTA_HANDLER(PhysicalRegfileUnit, ProcessInsts_)};
    private:
        SelfAllocatorsUnit* allocator_;
        GlobalParamUnit* global_param_ptr_ = nullptr;
        PmuUnit* pmu_;

    private:
        std::map<uint64_t, Credit> credit_map_;

        std::map<uint64_t, Credit> preceding_credit_map_;

        const uint64_t issue_width_;

        const uint64_t queue_depth_;

        const uint64_t latency_;

        PhysicalReg phy_regfile_;

        const PhyRegId_t phy_reg_num_;

        std::deque<InstPtr> inst_queue_;
    };

}
