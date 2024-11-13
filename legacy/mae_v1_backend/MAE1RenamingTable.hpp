#pragma once

#include "core/abstract_backend/RenamingTable.hpp"
#include "sparta/utils/ValidValue.hpp"

#include<vector>

namespace TimingModel {

    class MAE1RenamingTable {
    public:
        using RenamingTable = RenamingTable<sparta::utils::ValidValue<PhyRegId_t>>;

        MAE1RenamingTable(IsaRegId_t, GroupId_t);

        RenamingTable& operator[](GroupId_t);

        const RenamingTable operator[](GroupId_t) const;

        void RollBack();

    private:
        std::vector<RenamingTable> renaming_tables_;
    };

}
