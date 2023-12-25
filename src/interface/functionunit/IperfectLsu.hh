#pragma once

#include "basic/Inst.hpp"

namespace TimingModel {

class IperfectLsu{
public:
    virtual ~IperfectLsu() = 0;

    virtual void ProcessInst(InstPtr) {}
};

}