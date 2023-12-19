#pragma once

#include <vector>

#include "basic/Instruction.hh"

class IReservationStation {
public:
    virtual ~IReservationStation() = 0;

    // read operation

    virtual bool IsEmpty(IssueNum) = 0;

    virtual bool IsFull(IssueNum) = 0;

    virtual bool IsForwardEntryExist(RobIdx) = 0;

    virtual const std::vector<ScalarRSEntry> GetValidRSIdx(IssueNum) = 0;

    // write operation

    virtual void Flush() = 0;

    virtual void AllocateRSEntry(ScalarRSEntry&) = 0;

    virtual void SetForwardOperand(RobIdx, xReg_t) = 0;

    virtual void PopRSEntry(RSIdx) = 0;
};