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

namespace TimingModel {

    struct PhyRegfileRequest {
        uint64_t    rob_idx;
        PhyRegId_t  phy_reg_idx;
        sparta::utils::ValidValue<xReg_t>  data;
    };

    struct PhyRegfileWriteBack {
        PhyRegId_t  phy_reg_idx;
        xReg_t      data;
    };

    class PhyRegfileReadPort : public sparta::Unit {
    public:
        class PhyRegfileParameter : public sparta::ParameterSet {
        public:
            PhyRegfileParameter(sparta::TreeNode* n) :
                    sparta::ParameterSet(n)
            {}

            PARAMETER(uint32_t, phy_reg_num, 64, "the issuing bandwidth in a cycle")
            PARAMETER(uint8_t, read_port_num, 4, "the number of read ports")
        };

        static const char* name;

        PhyRegfileReadPort(sparta::TreeNode* node, const PhyRegfileParameter* p);

    private:
        void ReadRegfile_(const PhyRegfileRequest&);

        void WriteBackRegfile_(const PhyRegfileWriteBack&);

    private:
        // ports
        // read ports
        sparta::DataInPort<PhyRegfileRequest> read_phy_regfile_in
                {&unit_port_set_, "read_phy_regfile_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<PhyRegfileRequest> read_phy_regfile_out
                {&unit_port_set_, "read_phy_regfile_out"};

        // write ports
        sparta::DataInPort<InstGroupPtr> preceding_renaming_inst_in
                {&unit_port_set_, "preceding_renaming_inst_in", sparta::SchedulingPhase::Tick, 1};

        // events

    private:
        uint32_t phy_regfile_num_;
        std::vector<xReg_t> phy_regfile_;
    };

}
