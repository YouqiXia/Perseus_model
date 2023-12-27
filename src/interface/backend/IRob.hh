#pragma once

#include "basic/Inst.hpp"

namespace TimingModel {

class IRob {
public:
    virtual ~IRob() = 0;

    // read operation
    virtual bool IsRobFull(IssueNum) = 0;

    virtual InstPtr GetRobEntry(RobIdx) = 0;

    virtual std::vector<InstPtr> GetCommitingEntry(IssueNum) = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void AllocateRobEntry(InstPtr) = 0;

    virtual void FinishInst(RobIdx) = 0;

    virtual void Commit(RobIdx) = 0;
};

class IIntegrateRob {
public:
    virtual ~IIntegrateRob() = 0;

    // read operation
    virtual bool IsRobFull(IssueNum) = 0;

    virtual bool IsRobEmpty() = 0;

    virtual InstPtr GetRobEntry(RobIdx) = 0;

    virtual InstGroup GetCommitingEntry(IssueNum) = 0;

    virtual InstGroup GetIssuingEntry(IssueNum) = 0;

    // write operation
    virtual void Flush() = 0;

    virtual void AllocateRobEntry(InstPtr) = 0;

    virtual void IssueInst(RobIdx) = 0;

    virtual void FinishInst(RobIdx) = 0;

    virtual void Clear(RobIdx) = 0;

    virtual uint64_t Commit(IssueNum) = 0;
};

}