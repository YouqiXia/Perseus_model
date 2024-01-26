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
#include "FuncUnits.hpp"

namespace TimingModel {

    // rs <-> dispatch stage
    struct RsCredit {
        std::string rs_name;
        Credit credit;
    };

    using RsCreditPtr = sparta::SpartaSharedPointer<RsCredit>;

    // fu <-> write back stage
    struct FuncInst {
        FuncUnitType func_type;
        InstPtr inst_ptr;
    };

    using FuncInstPtr = sparta::SpartaSharedPointer<FuncInst>;
}

#endif //MODEL_PORTINTERFACE_HPP
