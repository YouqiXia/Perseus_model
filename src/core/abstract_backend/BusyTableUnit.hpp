//
// Created by yzhang on 11/11/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include <string>
#include <deque>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"
#include "basic/GlobalParamUnit.hpp"
#include "basic/SelfAllocatorsUnit.hpp"

#include "Scoreboard.hpp"

#include "simulation/PmuUnit.hpp"

namespace TimingModel {

    class BusyTableUnit: public sparta::Unit {
    public:
        class BusyTableParameter: public sparta::ParameterSet {
        public:
            BusyTableParameter(sparta::TreeNode *n) :
                sparta::ParameterSet(n) {}

            PARAMETER(uint64_t, phy_reg_num, 64, "the number of physical registers")
        };

        static const char *name;

        BusyTableUnit(sparta::TreeNode *node, const BusyTableParameter *p);

    private: // port binding implementation
        void CheckBusy_(const InstGroupPtr&);

        void FuncUpdate_(const InstGroupPtr&);

        void HandleFlush_(const FlushingCriteria&);

    private: // port
        sparta::DataInPort<FlushingCriteria> busy_table_flush_in
                {&unit_port_set_, "busy_table_flush_in", sparta::SchedulingPhase::Flush, 1};

        sparta::DataInPort<InstGroupPtr> check_busy_in
                {&unit_port_set_, "check_busy_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstGroupPtr> update_busy_in
                {&unit_port_set_, "update_busy_in", sparta::SchedulingPhase::Tick, 1};

    private:
        Scoreboard scoreboard_;

    };

}
