//
// Created by yzhang on 1/26/24.
//

#ifndef MODEL_PORTINTERFACE_HPP
#define MODEL_PORTINTERFACE_HPP

#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaSharedPointerAllocator.hpp"

#include <cstdint>
#include "Inst.hpp"
#include "InstGroup.hpp"

namespace TimingModel {

    struct CreditPair {
        std::string name;
        Credit credit;
    };

    using CreditPairPtr = sparta::SpartaSharedPointer<CreditPair>;
    using CreditPairAllocator = sparta::SpartaSharedPointerAllocator<CreditPair>;
    extern CreditPairAllocator    credit_pair_allocator;

    struct InstGroupPair {
        std::string name;
        std::vector<InstPtr> inst_group;
    };

    using InstGroupPairPtr = sparta::SpartaSharedPointer<InstGroupPair>;
    using InstGroupPairAllocator = sparta::SpartaSharedPointerAllocator<InstGroupPair>;
    extern InstGroupPairAllocator inst_group_pair_allocator;
}

#endif //MODEL_PORTINTERFACE_HPP
