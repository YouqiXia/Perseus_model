//
// Created by yzhang on 10/30/24.
//

#include "MAE1RenamingTable.hpp"


namespace TimingModel {

    MAE1RenamingTable::MAE1RenamingTable(IsaRegId_t isa_reg_num,
                                         GroupId_t group_num):
            renaming_tables_(group_num, RenamingTable(isa_reg_num))
    {}

    MAE1RenamingTable::RenamingTable& MAE1RenamingTable::operator[](TimingModel::GroupId_t group_idx) {
        return renaming_tables_[group_idx];
    }

    const MAE1RenamingTable::RenamingTable MAE1RenamingTable::operator[](TimingModel::GroupId_t group_idx) const {
        return renaming_tables_[group_idx];
    }

    void MAE1RenamingTable::RollBack() {
        for (auto& renaming_table: renaming_tables_) {
            renaming_table.RollBack();
        }
    }
}