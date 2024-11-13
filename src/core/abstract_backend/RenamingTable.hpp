//
// Created by yzhang on 12/30/23.
//

#pragma once

#include <vector>

#include "basic/Inst.hpp"

namespace TimingModel {

    template <class T>
    class RenamingTable {
    public:
        RenamingTable() = default;

        RenamingTable(IsaRegId_t);

        RenamingTable(IsaRegId_t, T);

        T& operator[](IsaRegId_t);

        const T& operator[](IsaRegId_t) const;

        void RollBack();

        T& GetBackup(IsaRegId_t);

    private:
        IsaRegId_t isa_reg_num_;
        std::vector<T> renaming_table_;
        std::vector<T> renaming_table_backup_;
    };

    template <class T>
    RenamingTable<T>::RenamingTable(IsaRegId_t isa_reg_num, T init_value) :
            isa_reg_num_(isa_reg_num),
            renaming_table_(isa_reg_num, init_value),
            renaming_table_backup_(isa_reg_num, init_value)
    {}

    template <class T>
    RenamingTable<T>::RenamingTable(IsaRegId_t isa_reg_num) :
            isa_reg_num_(isa_reg_num),
            renaming_table_(isa_reg_num),
            renaming_table_backup_(isa_reg_num)
    {}

    template <class T>
    T &RenamingTable<T>::operator[](TimingModel::IsaRegId_t isa_reg_idx) {
        sparta_assert(isa_reg_idx <= isa_reg_num_, "accessing renaming table is out of range");
        return renaming_table_[isa_reg_idx];
    }

    template <class T>
    const T &RenamingTable<T>::operator[](TimingModel::IsaRegId_t isa_reg_idx) const {
        sparta_assert(isa_reg_idx <= isa_reg_num_, "accessing renaming table is out of range");
        return renaming_table_[isa_reg_idx];
    }

    template <class T>
    void RenamingTable<T>::RollBack() {
        for (int i = 0; i < renaming_table_.size(); ++i) {
            renaming_table_[i] = renaming_table_backup_[i];
        }
    }

    template <class T>
    T &RenamingTable<T>::GetBackup(TimingModel::IsaRegId_t isa_reg_idx) {
        sparta_assert(isa_reg_idx <= isa_reg_num_, "accessing backup renaming table is out of range");
        return renaming_table_backup_[isa_reg_idx];
    }

}