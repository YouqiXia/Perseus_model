#pragma once

#include "basic/Instruction.hh"

class IperfectAlu{
public:
    virtual ~IperfectAlu() = 0;

    virtual void ProcessInst(InstPtr) {}
};