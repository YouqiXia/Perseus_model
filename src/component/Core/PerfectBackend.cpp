#include "PerfectBackend.hpp"
#include "sparta/simulation/ResourceTreeNode.hpp"

#include "IntegrateRob.hpp"
#include "PerfectAlu.hpp"
#include "PerfectLsu.hpp"

#include <iostream>

namespace TimingModel {
    const char* PerfectBackend::name = "perfect_backend";

    PerfectBackend::PerfectBackend(sparta::TreeNode* node, const PerfectBackendParameter* p) :
        sparta::Unit(node),
        node_(node),
        issue_num_(p->issue_num),
        rob_(new IntegrateRob(p->rob_depth))
    {
        fetch_backend_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, AllocateRobEntry, InstGroup));
        alu_backend_finish_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, Finish, RobIdx));
        lsu_backend_finish_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, Finish, RobIdx));
    }

    PerfectBackend::~PerfectBackend() {
        delete rob_;
    }

    bool PerfectBackend::IsReady(IssueNum issue_num) {
        return rob_->IsRobFull(issue_num);
    }

    bool PerfectBackend::IsAluReady(IssueNum issue_num) {
        return node_->getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_alu")-> 
            getResourceAs<TimingModel::PerfectAlu>()->IsReady(issue_num);
    }

    bool PerfectBackend::IsLsuReady(IssueNum issue_num) {
        return node_->getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_lsu")-> 
            getResourceAs<TimingModel::PerfectLsu>()->IsReady(issue_num);
    }

    void PerfectBackend::AllocateRobEntry(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            rob_->AllocateRobEntry(inst_ptr);
        }
    }

    void PerfectBackend::Finish(const RobIdx& rob_idx) {
        rob_->FinishInst(rob_idx);
    }

    void PerfectBackend::SetRobIssued(const RobIdx& rob_idx) {
        rob_->IssueInst(rob_idx);
    }

    void PerfectBackend::RobCommit() {
        rob_->Commit(issue_num_);
    }

    void PerfectBackend::Trigger() {
        /* for issuing */
        InstGroup lsu_tmp_instgroup;
        InstGroup alu_tmp_instgroup;
        for (InstPtr inst_ptr: rob_->GetIssuingEntry(issue_num_)) {
            if (inst_ptr->Fu != FuncType::STU && inst_ptr->Fu != FuncType::LDU) {
                if (IsAluReady(alu_tmp_instgroup.size() + 1)) {
                    alu_tmp_instgroup.push_back(inst_ptr);
                    set_rob_issued.preparePayload(inst_ptr->RobTag)->schedule(1);
                }
            } else {
                if (IsLsuReady(lsu_tmp_instgroup.size() + 1)) {
                    lsu_tmp_instgroup.push_back(inst_ptr);
                    set_rob_issued.preparePayload(inst_ptr->RobTag)->schedule(1);
                }
            }
        }

        if (lsu_tmp_instgroup.size()) {
            backend_lsu_inst_out.send(lsu_tmp_instgroup);
        }

        if (alu_tmp_instgroup.size()) {
            backend_alu_inst_out.send(alu_tmp_instgroup);
        }

        /* for commiting*/
        set_rob_commmited.schedule(1);

        /* self trigger */
        self_trigger.schedule(1);
    }

}