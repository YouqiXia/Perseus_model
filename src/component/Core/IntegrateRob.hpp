#include "interface/backend/IRob.hh"
#include "LoopQueue.hh"


namespace TimingModel {

class IntegrateRob : public IIntegrateRob {
public:
    IntegrateRob(uint64_t rob_depth);

    ~IntegrateRob();

    // read operation
    bool IsRobFull(IssueNum) final;

    bool IsRobEmpty() final;

    InstPtr GetRobEntry(RobIdx) final;

    InstGroup GetCommitingEntry(IssueNum) final;

    InstGroup GetIssuingEntry(IssueNum) final;

    // write operation
    void Flush() final;

    void AllocateRobEntry(InstPtr) final;

    void IssueInst(RobIdx) final;

    void FinishInst(RobIdx) final;

    void Clear(RobIdx) final;

    uint64_t Commit(IssueNum) final;

private:
    LoopQueue<RobEntry> rob_;
};

}