#pragma once

#include "basic/Instruction.hh"

class IInstQueue {
public:
    virtual ~IInstQueue() = 0;

    virtual DispatchQueueIdx GetFrontIdx() {}
    
    virtual DispatchQueueIdx GetNexIdx(DispatchQueueIdx) {}

    virtual void Pop() {}
};