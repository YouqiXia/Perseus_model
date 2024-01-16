#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "Freelist.hpp"
#include "RenamingTable.hpp"

namespace TimingModel {

class RenamingStage : public sparta::Unit {
public:
    class RenamingParameter : public sparta::ParameterSet {
    public:
        RenamingParameter(sparta::TreeNode* n) :
            sparta::ParameterSet(n) 
        {}

        PARAMETER(uint32_t, issue_width, 2, "the issuing bandwidth in a cycle")
        PARAMETER(uint32_t, isa_reg_num, 32, "the number of isa register file")
        PARAMETER(uint32_t, renaming_stage_queue_depth, 4, "the depth of renaming queue")
        PARAMETER(uint32_t, free_list_depth, 64, "the depth of freelist")
        PARAMETER(bool, is_perfect_lsu, true, "if it cooperates with a perfect lsu")
    };

    static const char* name;

    RenamingStage(sparta::TreeNode* node, const RenamingParameter* p);

    ~RenamingStage();

private:
    void AcceptRobCredit_(const Credit&);

    void AcceptDispatchCredit_(const Credit&);

    void AcceptLsuCredit_(const Credit&);

    void CreditDecrease_();

    void CreditWithoutLsuDecrease_();

    void InitCredit_();

    void AllocateInst_(const InstGroupPtr&);

    void RenameInst_();

    void HandleFlush_(const FlushingCriteria&);

    void RenameInstImp_(const InstPtr&);

    void RollBack_(const InstGroupPtr&);

    void RobCommit_(const InstGroupPtr&);

private:
    // ports
        // flush
        sparta::DataInPort<FlushingCriteria> renaming_flush_in
            {&unit_port_set_, "renaming_flush_in", sparta::SchedulingPhase::Tick, 1};

        // with frontend
        sparta::DataInPort<InstGroupPtr> preceding_renaming_inst_in
            {&unit_port_set_, "preceding_renaming_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> renaming_preceding_credit_out
            {&unit_port_set_, "renaming_preceding_credit_out"};

        // with dispatch queue
        sparta::DataOutPort<InstGroupPtr> renaming_following_inst_out
            {&unit_port_set_, "renaming_following_inst_out"};

        sparta::DataInPort<Credit> following_renaming_credit_in
            {&unit_port_set_, "following_renaming_credit_in", sparta::SchedulingPhase::Tick, 0};

        // with rob
        sparta::DataInPort<Credit> rob_renaming_credit_in
            {&unit_port_set_, "rob_renaming_credit_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<InstGroupPtr> Rob_cmt_inst_in
            {&unit_port_set_, "Rob_cmt_inst_in", sparta::SchedulingPhase::Tick, 0};

        // with lsu
//        sparta::DataOutPort<InstPtr> renaming_lsu_allocate_out
//            {&unit_port_set_, "renaming_lsu_allocate_out"};

//        sparta::DataInPort<Credit> lsu_renaming_credit_in
//            {&unit_port_set_, "lsu_renaming_credit_in", sparta::SchedulingPhase::Tick, 0};


    // events
        sparta::SingleCycleUniqueEvent<> rename_event
            {&unit_event_set_, "rename_event", CREATE_SPARTA_HANDLER(RenamingStage, RenameInst_)};

private:
    InstQueue renaming_stage_queue_;

    Freelist free_list_;

    RenamingTable renaming_table_;

    const uint64_t issue_width_;

    const uint64_t renaming_stage_queue_depth_;

    const bool is_perfect_lsu_;

    Credit dispatch_credit_ = 0;

    Credit rob_credit_ = 0;

    Credit lsu_credit_ = 256;

};

}