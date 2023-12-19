#pragma once

#include "basic/Instruction.hh"

class IPhysicalReg {
public:
    virtual ~IPhysicalReg() = 0;

    // notice: p0 is always 0 in no matter what situation

    // read operation
    virtual xReg_t GetOperand(PhyRegId_t) = 0;

    // write operation
    virtual void WriteBackOperand(PhyRegId_t, xReg_t) = 0;
};