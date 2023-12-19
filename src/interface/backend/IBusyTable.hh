#pragma once

#include "basic/Instruction.hh"

class BusyTable {
public:
    virtual ~BusyTable() = 0;

    // read operation
    virtual bool IsValid(BusyTableIdx) = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void InValidate(BusyTableIdx) = 0;

    virtual void Validate(BusyTableIdx) = 0;
};