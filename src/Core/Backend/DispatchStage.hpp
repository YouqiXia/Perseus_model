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

#include "PhyRegfileReadPort.hpp"
#include "PhyRegfileWritePort.hpp"

namespace TimingModel {

    class DispatchStage : public sparta::Unit {
    public:
        class DispatchStageParameter : public sparta::ParameterSet {
        public:
            DispatchStageParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(uint8_t, reg_read_port_num, 8, "the number of read ports")
            PARAMETER(uint8_t, reg_write_port_num, 4, "the number of read ports")
        };

        static const char *name;

        DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p);

    private:

    private:
        // ports

        // write ports
        sparta::DataInPort<InstGroupPtr> preceding_renaming_inst_in
                {&unit_port_set_, "preceding_renaming_inst_in", sparta::SchedulingPhase::Tick, 1};

        // events

    private:
        uint32_t phy_regfile_num_;
        PhysicalReg phy_regfile_;
        std::map<std::string, PhyRegfileReadPort> phy_reg_read_ports_;
    };

}