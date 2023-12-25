#pragma once

#include "basic/Inst.hpp"

namespace TimingModel {

class IInstQueue {
public:
    virtual ~IInstQueue() = 0;

    virtual DispatchQueueIdx GetFrontIdx() {}
    
    virtual DispatchQueueIdx GetNexIdx(DispatchQueueIdx) {}

    virtual void Pop() {}
};

}