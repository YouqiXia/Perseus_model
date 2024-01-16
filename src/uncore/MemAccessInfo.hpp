#pragma once
#include "sparta/utils/SpartaSharedPointer.hpp"
#include "basic/Inst.hpp"


namespace TimingModel {

enum class MemOp: std::uint8_t {
    NO_TYPE = 0,
    __FIRST = NO_TYPE,
    FETCH,
    LOAD,
    STORE,
    FLUSH,
    NUM_TYPES,
    __LAST = NUM_TYPES
};

class MemAccInfo{
public:
    uint64_t address;
    uint64_t length;
    MemOp mem_op;
    InstPtr insn;
    uint8_t* data;
    uint32_t mshrid;
    uint32_t subindex;

    MemAccInfo(){
        address = 0;
        length = 0;
        mem_op = MemOp::NO_TYPE;
        insn = nullptr;
        data = nullptr;
        mshrid = 0;
        subindex = 0;  
    }
    MemAccInfo& operator=(MemAccInfo & rval){
        address = rval.address;
        length = rval.length;
        mem_op = rval.mem_op;
        insn = rval.insn;
        data = rval.data;
        mshrid = rval.mshrid;
        subindex = rval.subindex;
        return *this;
    }
};

using MemAccInfoPtr = sparta::SpartaSharedPointer<MemAccInfo>;
using MemAccInfoAllocator = sparta::SpartaSharedPointerAllocator<MemAccInfo>;
using MemAccInfoGroup = std::vector<MemAccInfoPtr>;

} //namespace TimingModel