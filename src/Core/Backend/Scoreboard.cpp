//
// Created by yzhang on 1/1/24.
//

#include "Scoreboard.hpp"

namespace TimingModel {

    Scoreboard::Scoreboard(const std::string &name,
                           TimingModel::PhyRegId_t phy_regfile_num,
                           sparta::log::MessageSource &info_logger) :
            score_board_(phy_regfile_num, 0),
            phy_regfile_num_(phy_regfile_num)
    {}

    void Scoreboard::Flush() {
        score_board_.clear();
    }

    bool Scoreboard::IsForwarding(PhyRegId_t phy_reg_idx) {
        if (phy_reg_idx == 0) {
            return false;
        }
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        return score_board_[phy_reg_idx].isValid();
    }

    RobIdx_t Scoreboard::GetForwardingEntry(PhyRegId_t phy_reg_idx) {
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        return score_board_[phy_reg_idx].getValue();
    }

    void Scoreboard::SetForwardingEntry(PhyRegId_t phy_reg_idx, RobIdx_t rob_idx) {
        if (phy_reg_idx == 0) {
            return;
        }
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        score_board_[phy_reg_idx] = rob_idx;
    }

    void Scoreboard::ClearForwardingEntry(PhyRegId_t phy_reg_idx) {
        if (phy_reg_idx == 0) {
            return;
        }
        sparta_assert(phy_reg_idx <= phy_regfile_num_, "Scoreboard access is out of range");
        score_board_[phy_reg_idx].clearValid();
    }

}