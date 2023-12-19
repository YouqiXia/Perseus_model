#include "interface/backend/IRob.hh"
#include "LoopQueue.hh"

class IntegrateRob : public IIntegrateRob {
public:
    IntegrateRob(uint64_t rob_depth);

    virtual ~IntegrateRob();

    // read operation
    virtual bool IsRobFull(IssueNum) final;

    virtual InstPtr GetRobEntry(RobIdx) final;

    virtual InstGroup GetCommitingEntry(IssueNum) final;

    virtual InstGroup GetIssuingEntry(IssueNum) final;

    // write operation
    virtual void Flush() final;

    virtual void AllocateRobEntry(InstPtr) final;

    virtual void IssueInst(RobIdx) final;

    virtual void FinishInst(RobIdx) final;

    virtual void Clear(RobIdx) final;

    virtual void Commit(IssueNum) final;

private:
    LoopQueue<RobEntry> rob_;
};
