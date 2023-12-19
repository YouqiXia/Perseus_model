#pragma once

#include "basic/Instruction.hh"

class IFreeList {
public:
    virtual ~IFreeList() = 0;

    // read operation
    /* if the freelist full for the specific rank of issuing */
    virtual bool IsFreeListFull(IssueNum) = 0;

    virtual bool IsFreeListEmpty(IssueNum) = 0;

    virtual PhyRegId_t GetFreeListEntry(FreeListIdx) = 0;

    virtual FreeListIdx GetFrontIdx() = 0;

    virtual FreeListIdx GetNexIdx(FreeListIdx) = 0;

    virtual uint64_t GetFreeEntryNum() = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void WriteBackPhyRegIdx(PhyRegId_t) = 0;

    /* read ptr move to the next one */
    virtual void GottenFreeList() = 0;

    /* maintain a rollback pointer. Initailly rollback ptr = read ptr = 0*/
    virtual void RollBack() = 0;

    /* roll back ptr move to the next one */
    virtual void RollBackPtrMove() = 0;
};