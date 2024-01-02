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

    using ScoreBoardEntry = sparta::utils::ValidValue<RobIdx_t>;

    class Scoreboard {
    public:

        Scoreboard(const std::string &name,
                   PhyRegId_t phy_regfile_num,
                   sparta::log::MessageSource &info_logger);

        bool IsForwarding(PhyRegId_t);

        RobIdx_t GetForwardingEntry(PhyRegId_t);

        void SetForwardingEntry(PhyRegId_t, RobIdx_t);

        void ClearForwardingEntry(PhyRegId_t);

    private:
        std::vector<ScoreBoardEntry> score_board_;
        PhyRegId_t phy_regfile_num_;
    };

}