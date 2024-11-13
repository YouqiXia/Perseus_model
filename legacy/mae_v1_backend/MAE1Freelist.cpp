//
// Created by yzhang on 10/30/24.
//

#include "MAE1Freelist.hpp"

namespace TimingModel {
    MAE1Freelist::MAE1Freelist(const std::string name,
                               const TimingModel::GroupId_t group_num,
                               const uint32_t depth):
        free_lists_(group_num, Freelist(name, depth))
    {}

    Freelist &MAE1Freelist::operator[](TimingModel::GroupId_t group_idx) {
        return free_lists_[group_idx];
    }

    const Freelist MAE1Freelist::operator[](TimingModel::GroupId_t group_idx) const {
        return free_lists_[group_idx];
    }
}
