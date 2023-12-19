#pragma once

#include "basic/Instruction.hh"

class IDispatchQueue {
public:
    virtual ~IDispatchQueue() = 0;

    // read operation
    virtual bool IsFull(IssueNum) = 0;

    virtual bool IsEmpty(IssueNum) = 0;

    virtual DispatchQueueIdx GetFrontIdx() = 0;
    
    virtual DispatchQueueIdx GetNexIdx(DispatchQueueIdx) = 0;

    virtual uint64_t GetFreeEntryNum() = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void Allocate(InstPtr) = 0;

    virtual void PopEntry(DispatchQueueIdx) = 0;
};