//
// Created by yzhang on 1/1/24.
//

#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "sparta/log/MessageSource.hpp"
#include "sparta/utils/LogUtils.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

    struct PhyRegfileWriteBack {
        PhyRegId_t  phy_reg_idx;
        xReg_t      data;
    };

    class PhyRegfileWritePort {
    public:

        PhyRegfileWritePort(const std::string& name,
                           PhysicalReg* phy_reg,
                            sparta::log::MessageSource   & info_logger,
                            sparta::DataInPort<PhyRegfileWriteBack>* read_port_in);

        const std::string& GetName();

    private:
        void WriteBackRegfile_(const PhyRegfileWriteBack&);

    private:
        const std::string name_;
        PhysicalReg* phy_regfile_ = nullptr;
        sparta::log::MessageSource & info_logger_;
    };

}