#pragma once

#include "core/abstract_backend/Freelist.hpp"

#include "basic/Instruction.hpp"

namespace TimingModel {

class MAE1Freelist {
public:
    MAE1Freelist(const std::string name,
                 const GroupId_t group_idx,
                 const uint32_t depth);

    Freelist& operator[](GroupId_t);

    const Freelist operator[](GroupId_t) const;


private:
    std::vector<Freelist> free_lists_;
};

}

