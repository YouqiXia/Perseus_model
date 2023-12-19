#include "PerfectFrontend.hpp"

#include "sparta/simulation/ResourceTreeNode.hpp"

#include "PerfectBackend.hpp"

namespace TimingModel {

    const char* PerfectFrontend::name = "perfect_frontend";

    PerfectFrontend::PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p) : 
        sparta::Unit(node),
        node_(node),
        issue_num_(p->issue_num),
        inst_queue_(p->inst_queue_depth)
    {}

    bool PerfectFrontend::IsNextStageReady(IssueNum issue_num) {
        return node_->getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_backend")-> 
            getResourceAs<TimingModel::PerfectBackend>()->IsReady(issue_num);
    }

    InstGroup PerfectFrontend::GetAvailInst() {
        InstGroup tmp_inst_group;
        uint64_t idx = inst_queue_.getHeader();
        for (int i = 1; i < issue_num_ + 1; ++i) {
            if (IsNextStageReady(i) && inst_queue_.getUsage() >= i) {
                tmp_inst_group.emplace_back(inst_queue_[idx]);
                pop_inst_queue.schedule(1);
                idx = inst_queue_.getNextPtr(idx);
            } else {
                break;
            }
        }
        return tmp_inst_group;
    }

    void PerfectFrontend::Trigger() {
        InstGroup inst_group = GetAvailInst();
        if (inst_group.size()) {
            std::cout << "perfect frontend send " << inst_group.size() << " instructions to backend" << std::endl;
            fetch_backend_inst_out.send(inst_group);
        }

        self_trigger.schedule(1);
    }

    void PerfectFrontend::Pop() {
        inst_queue_.Pop();
    }

    void PerfectFrontend::SetInst(InstPtr inst_ptr) {
        inst_queue_.Push(inst_ptr);
    }
}