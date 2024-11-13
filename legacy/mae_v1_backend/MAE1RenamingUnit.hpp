#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>
#include <deque>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/SelfAllocatorsUnit.hpp"

#include "MAE1Freelist.hpp"
#include "MAE1RenamingTable.hpp"

#include "simulation/PmuUnit.hpp"

namespace TimingModel {
class MAE1RenamingUnit : public sparta::Unit {
public:
    class MAE1RenamingParameter : public sparta::ParameterSet {
    public:
        MAE1RenamingParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
        {}

        PARAMETER(uint32_t, issue_width, 4, "the issuing bandwidth in a cycle")
        PARAMETER(uint32_t, isa_reg_num, 32, "the number of isa register file")
        PARAMETER(uint32_t, queue_depth, 8, "the depth of renaming queue")
        PARAMETER(uint32_t, phy_reg_num, 64, "the depth of freelist")
        PARAMETER(uint32_t, group_num, 4, "the group of scheduler")
        PARAMETER(bool, is_perfect_lsu, true, "if it cooperates with a perfect lsu")
    };

    static const char* name;

    MAE1RenamingUnit(sparta::TreeNode* node, const MAE1RenamingParameter* p);

    ~MAE1RenamingUnit() = default;

private:
    void Startup_();

    // data flow critical functions
    // credit
    void AcceptRobCredit_(const Credit&);

    void AcceptDispatchCredit_(const Credit&);

    void AcceptLoadQueueCredit_(const Credit&);

    void AcceptStoreQueueCredit_(const Credit&);
    // credit

    void AllocateInst_(const InstGroupPtr&);

    void ProcessInst_();

    void HandleFlush_(const FlushingCriteria&);

    void RobCommit_(const InstGroupPtr&);

    void PmuMonitor_();
    // data flow critical functions

    // inner implementation
    void InitCredit_();

    uint64_t GetProduceNum_();

    void RenameInst_(InstGroupPtr inst_produced_group_ptr,
                     InstGroupPtr inst_produced_lsu_group_ptr,
                     uint64_t produce_inst_num);

    void GetGroupIdx(InstGroupPtr inst_produced_group_ptr,
                     InstGroupPtr inst_produced_lsu_group_ptr,
                     uint64_t produce_inst_num);

    bool RenameInstImp_(const InstPtr&);

    bool PortTransferCtrl_(InstGroupPtr inst_produced_group_ptr,
                           InstGroupPtr inst_produced_lsu_group_ptr);
    // inner implementation


private:
    // ports
    // flush
    sparta::DataInPort<FlushingCriteria> renaming_flush_in
            {&unit_port_set_, "renaming_flush_in", sparta::SchedulingPhase::Flush, 1};

    // with frontend
    sparta::DataInPort<InstGroupPtr> preceding_renaming_inst_in
            {&unit_port_set_, "preceding_renaming_inst_in", sparta::SchedulingPhase::PortUpdate, 1};

    sparta::DataOutPort<Credit> renaming_preceding_credit_out
            {&unit_port_set_, "renaming_preceding_credit_out"};

    // with dispatch queue
    sparta::DataOutPort<InstGroupPtr> renaming_following_inst_out
            {&unit_port_set_, "renaming_following_inst_out"};

    sparta::DataInPort<Credit> following_renaming_credit_in
            {&unit_port_set_, "following_renaming_credit_in", sparta::SchedulingPhase::PortUpdate, 0};

    // with rob
    sparta::DataInPort<Credit> rob_renaming_credit_in
            {&unit_port_set_, "rob_renaming_credit_in", sparta::SchedulingPhase::PortUpdate, 0};

    sparta::DataInPort<InstGroupPtr> Rob_cmt_inst_in
            {&unit_port_set_, "Rob_cmt_inst_in", sparta::SchedulingPhase::Tick, 0};

    // with lsu
    sparta::DataOutPort<InstGroupPtr> renaming_lsu_allocate_out
            {&unit_port_set_, "renaming_lsu_allocate_out"};

    sparta::DataInPort<Credit> lsu_renaming_ldq_credit_in
            {&unit_port_set_, "lsu_renaming_ldq_credit_in", sparta::SchedulingPhase::Tick, 0};

    sparta::DataInPort<Credit> lsu_renaming_stq_credit_in
            {&unit_port_set_, "lsu_renaming_stq_credit_in", sparta::SchedulingPhase::Tick, 0};

    // events
    sparta::SingleCycleUniqueEvent<> rename_event
            {&unit_event_set_, "rename_event", CREATE_SPARTA_HANDLER(MAE1RenamingUnit, ProcessInst_)};

    sparta::SingleCycleUniqueEvent<sparta::SchedulingPhase::PostTick> pmu_event
            {&unit_event_set_, "pmu_event", CREATE_SPARTA_HANDLER(MAE1RenamingUnit, PmuMonitor_)};

private:
    SelfAllocatorsUnit* allocator_;
    PmuUnit* pmu_;

private:
    std::deque<InstPtr> renaming_stage_queue_;

    MAE1Freelist free_lists_;

    MAE1RenamingTable renaming_tables_;

    const uint64_t issue_width_;

    const uint64_t renaming_stage_queue_depth_;

    const uint64_t group_num_;

    const bool is_perfect_lsu_;

    Credit dispatch_credit_ = 0;

    Credit rob_credit_ = 0;

    Credit ldq_credit_ = 0;

    Credit stq_credit_ = 0;

};

}
