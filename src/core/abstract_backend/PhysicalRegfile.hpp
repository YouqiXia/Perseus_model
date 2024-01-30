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
#include "FuncUnits.hpp"

#include "ReservationStation.hpp"
#include "Scoreboard.hpp"

namespace TimingModel {

    class PhysicalRegfile : public sparta::Unit {
    public:
        class PhysicalRegfileParameter : public sparta::ParameterSet {
        public:
            PhysicalRegfileParameter(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
        };

        static const char *name;

        PhysicalRegfile(sparta::TreeNode *node, const PhysicalRegfileParameter *p);

    private:
        void ReadPhysicalReg_(const InstGroupPtr&);

        void WritePhysicalReg_(const InstGroupPtr&);

    private:
        /* ports */
        // read/write port
        sparta::DataInPort<InstGroupPtr> preceding_physical_regfile_read_in
                {&unit_port_set_, "preceding_physical_regfile_read_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataInPort<InstGroupPtr> preceding_physical_regfile_write_in
                {&unit_port_set_, "preceding_physical_regfile_write_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroupPtr> physical_regfile_following_read_out
                {&unit_port_set_, "physical_regfile_following_read_out"};

        /* events */

    private:
        PhysicalReg phy_regfile_;

        const PhyRegId_t phy_reg_num_;
    };

}