#pragma once

#include "basic/Inst.hpp"

namespace TimingModel {

class IperfectAlu{
public:
    virtual ~IperfectAlu() = 0;

    virtual void ProcessInst(InstPtr) {}
};

}