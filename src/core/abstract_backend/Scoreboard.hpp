//
// Created by yzhang on 1/1/24.
//

#pragma once

#include "sparta/log/MessageSource.hpp"
#include "sparta/utils/ValidValue.hpp"

#include "sparta/utils/SpartaAssert.hpp"

#include <string>

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

    class Scoreboard {
    public:

        Scoreboard(const std::string &name,
                   PhyRegId_t phy_regfile_num,
                   sparta::log::MessageSource &info_logger);

        void Flush();

        bool GetBusyBit(PhyRegId_t);

        void SetBusyBit(PhyRegId_t);

        void ClearBusyBit(PhyRegId_t);

    private:
        std::vector<bool> score_board_;
        PhyRegId_t phy_regfile_num_;
    };

}