#pragma once

#include "basic/Instruction.hh"


class IRenamingTable {
public:

    virtual ~IRenamingTable() = 0;

    // read operation
    virtual xReg_t GetPhyReg(IsaRegId_t) = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void RenameIsaReg(IsaRegId_t, PhyRegId_t) = 0;

    virtual void RollBack() = 0;
};