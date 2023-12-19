#pragma once

#include "basic/Instruction.hh"

class IperfectLsu{
public:
    virtual ~IperfectLsu() = 0;

    virtual void ProcessInst(InstPtr) {}
};