//
// Created by yzhang on 12/30/23.
//

#pragma once

#include <vector>

#include "Inst.hpp"

namespace TimingModel {

    class RenamingTable {
    public:
        RenamingTable() = default;

        RenamingTable(IsaRegId_t, PhyRegId_t);

        PhyRegId_t& operator[](IsaRegId_t);

        const PhyRegId_t& operator[](IsaRegId_t) const;

        void RollBack();

        PhyRegId_t& GetBackup(IsaRegId_t);

    private:
        IsaRegId_t isa_reg_num_;
        std::vector<PhyRegId_t> renaming_table_;
        std::vector<PhyRegId_t> renaming_table_backup_;
    };

}