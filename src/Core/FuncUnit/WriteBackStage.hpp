//
// Created by yzhang on 1/6/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "FuncUnits.hpp"
#include "PortInterface.hpp"

namespace TimingModel {

    class WriteBackStage : public sparta::Unit {
    public:
        class WriteBackStageParameter : public sparta::ParameterSet {
        public:
            WriteBackStageParameter(sparta::TreeNode* n) :
                    sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_num, 4, "the issuing bandwidth in a cycle")
            PARAMETER(uint64_t, wb_latency, 1, "write back latency")
        };

        static const char* name;

        WriteBackStage(sparta::TreeNode* node, const WriteBackStageParameter* p);

        ~WriteBackStage() {}

    private:
        void AcceptFuncInst_(const FuncInstPtr&);

        void ArbitrateInst_();

        void SendInitCredit_();

    private:
        // ports
        sparta::DataOutPort<InstGroupPtr> write_back_following_port_out
                {&unit_port_set_, "write_back_following_port_out"};

        // events
        sparta::UniqueEvent<> arbitrate_inst_event
                {&unit_event_set_, "arbitrate_inst_event", CREATE_SPARTA_HANDLER(WriteBackStage, ArbitrateInst_)};

    private:
        uint64_t issue_num_;

        std::map<FuncUnitType, InstPtr> func_inst_map_;
        std::vector<FuncUnitType> func_pop_pending_queue_;

        std::map<FuncUnitType, sparta::DataInPort<FuncInstPtr>*> func_unit_write_back_ports_in_;
        std::map<FuncUnitType, sparta::DataOutPort<Credit>*> func_unit_credit_ports_out_;

        std::map<FuncUnitType, Credit> func_credit_map_;

        uint64_t wb_latency_;
    };

}