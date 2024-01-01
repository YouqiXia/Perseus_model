//
// Created by yzhang on 1/1/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/utils/ValidValue.hpp"

#include "sparta/log/MessageSource.hpp"
#include "sparta/utils/LogUtils.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

    struct PhyRegfileRequest {
        uint64_t rob_idx;
        PhyRegId_t phy_reg_idx;
        sparta::utils::ValidValue <xReg_t> data;
    };

    class PhyRegfileReadPort {
    public:

        PhyRegfileReadPort(const std::string& name,
                           PhysicalReg* phy_reg,
                           sparta::log::MessageSource   & info_logger,
                           sparta::DataInPort<PhyRegfileRequest>* read_port_in,
                           sparta::DataOutPort<PhyRegfileRequest>* read_port_out);

        const std::string& GetName() const;

    private:
        void ReadRegfile_(const PhyRegfileRequest&);

    private:
        // ports
        sparta::DataOutPort<PhyRegfileRequest>* read_phy_regfile_out_;

    private:
        const std::string name_;
        PhysicalReg* phy_regfile_ = nullptr;
        sparta::log::MessageSource & info_logger_;
    };

}