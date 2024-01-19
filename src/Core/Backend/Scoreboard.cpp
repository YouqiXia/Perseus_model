//
// Created by yzhang on 1/1/24.
//

#include "Scoreboard.hpp"

namespace TimingModel {

    Scoreboard::Scoreboard(const std::string &name,
                           TimingModel::PhyRegId_t phy_regfile_num,
                           sparta::log::MessageSource &info_logger) :
            score_board_(phy_regfile_num, false),
            phy_regfile_num_(phy_regfile_num)
    {
        Flush();
    }

    void Scoreboard::Flush() {
        for (uint32_t i = 0; i < phy_regfile_num_; i++) {
            score_board_[i] = false;
        }
    }

    bool Scoreboard::GetBusyBit(PhyRegId_t phy_reg_idx) {
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        return score_board_[phy_reg_idx];
    }

    void Scoreboard::SetBusyBit(PhyRegId_t phy_reg_idx) {
        if (phy_reg_idx == 0) {
            return;
        }
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        score_board_[phy_reg_idx] = true;
    }

    void Scoreboard::ClearBusyBit(PhyRegId_t phy_reg_idx) {
        if (phy_reg_idx == 0) {
            return;
        }
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        score_board_[phy_reg_idx] = false;
    }

}