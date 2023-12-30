#include "sparta/simulation/Unit.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

class Renaming : public sparta::Unit {
public:
    class RenamingParameter : sparta::ParameterSet {
        RenamingParameter(sparta::TreeNode* n) :
            sparta::ParameterSet(n) 
        {}

        PARAMETER(uint64_t, ISSUE_WIDTH, 2, "the issuing bandwidth in a cycle")
    }

    static const char* name;

    Renaming(sparta::TreeNode* node, const RenamingParameter* p);

    void AcceptCredit(Credit);

    void RenameInst(InstGroupPtr);

    void HandleFlush(FlushingCriteria);

    void Init();

private:
    // ports
        // flush
        sparta::DataInPort<FlushingCriteria> renaming_flush_in
            {&unit_port_set_, "renaming_flush_in", sparta::SchedulingPhase::Tick, 1};

        // with frontend
        sparta::DataInPort<InstGroupPtr> fetch_renaming_inst_in
            {&unit_port_set_, "fetch_renaming_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> renaming_fetch_inst_out
            {&unit_port_set_, "renaming_fetch_inst_out"};

        // with dispatch queue
        sparta::DataOutPort<InstGroupPtr> renaming_dispatch_inst_out
            {&unit_port_set_, "renaming_dispatch_inst_out"};

        sparta::DataInPort<Credit> dispatch_renaming_inst_in
            {&unit_port_set_, "dispatch_renaming_inst_in", sparta::SchedulingPhase::Tick, 0};
}

}