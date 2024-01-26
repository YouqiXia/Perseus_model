#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

// #include "basic/Instruction.hh"
#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "olympia/InstGenerator.hpp"

#include <string>

namespace TimingModel {

    class PerfectFrontend : public sparta::Unit {
    public:
        class PerfectFrontendParameter : public sparta::ParameterSet {
        public:
            PerfectFrontendParameter(sparta::TreeNode* n):
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_num, 4, "the issuing bandwidth in a cycle")
            PARAMETER(std::string, input_file, "", "the stf entry")
            PARAMETER(std::string, insn_gen_type, "spike", "the type of insnGen: trace or spike")
        };

        static const char* name;

        PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p);

        void AcceptCredit(const Credit&);

        void ProduceInst();

    public:
    // ports
        sparta::DataOutPort<InstGroupPtr> fetch_backend_inst_out
            {&unit_port_set_, "fetch_backend_inst_out"};

        sparta::DataInPort<Credit> backend_fetch_credit_in
            {&unit_port_set_, "backend_fetch_credit_in", sparta::SchedulingPhase::Tick, 0};
    
    private:
    // Events
        sparta::SingleCycleUniqueEvent<> produce_inst_event_
            {&unit_event_set_, "produce_inst", CREATE_SPARTA_HANDLER(PerfectFrontend, ProduceInst)};
    
    private:
        const uint64_t issue_num_;

        Credit credit_ = 0;

        sparta::TreeNode* node_;

        MavisType* mavis_facade_ = nullptr;

        std::unique_ptr<InstGenerator> inst_generator_;

        std::string insn_gen_type_;
    };
}
