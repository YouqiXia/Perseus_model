//
// Created by yzhang on 12/30/23.
//

#include "RenamingTable.hpp"

namespace TimingModel {

    RenamingTable::RenamingTable(IsaRegId_t isa_reg_num, PhyRegId_t init_value) :
        renaming_table_(isa_reg_num, init_value),
        renaming_table_backup_(isa_reg_num, init_value)
    {}

    PhyRegId_t &RenamingTable::operator[](TimingModel::IsaRegId_t isa_reg_idx) {
        return renaming_table_[isa_reg_idx];
    }

    const PhyRegId_t &RenamingTable::operator[](TimingModel::IsaRegId_t isa_reg_idx) const {
        return renaming_table_[isa_reg_idx];
    }

    void RenamingTable::RollBack() {
        for (int i = 0; i < renaming_table_.size(); ++i) {
            renaming_table_[i] = renaming_table_backup_[i];
        }
    }

    PhyRegId_t &RenamingTable::GetBackup(TimingModel::IsaRegId_t isa_reg_idx) {
        return renaming_table_backup_[isa_reg_idx];
    }
}