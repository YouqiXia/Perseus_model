#include "sparta/simulation/ResourceTreeNode.hpp"
#include "sparta/utils/LogUtils.hpp"

#include "PerfectBackend.hpp"
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
        rob_depth_(p->rob_depth),
        rob_(new IntegrateRob(p->rob_depth)),
        stat_ipc_(&unit_stat_set_,
                  "ipc",
                  "Instructions retired per cycle",
                  &unit_stat_set_,
                  "total_number_retired/cycles"),
        num_retired_(&unit_stat_set_,
                     "total_number_retired",
                     "The total number of instructions retired by this core",
                     sparta::Counter::COUNT_NORMAL),
        ipc_(&stat_ipc_)
    {
        fetch_backend_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, AllocateRobEntry, InstGroup));
        alu_backend_finish_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, Finish, RobIdx));
        lsu_backend_finish_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, Finish, RobIdx));
        lsu_backend_wr_data_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, WriteBack, InstPtr));
        alu_backend_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, AcceptCreditFromALU, Credit));
        lsu_backend_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectBackend, AcceptCreditFromLSU, Credit));

        sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order") >> issue_event_;
        sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order") >> issue_event_;
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectBackend, SendInitCredit));
    }

    PerfectBackend::~PerfectBackend() {
        std::cout << "model: Retired " << num_retired_.get()
                        << " instructions in " << getClock()->currentCycle()
                        << " overall IPC: " << ipc_.getValue()
                        << std::endl << std::endl << std::endl;
        
        delete rob_;
    }

    void PerfectBackend::SendInitCredit() {
        fetch_backend_credit_out.send(rob_depth_);
    }

    void PerfectBackend::AcceptCreditFromALU(const Credit& credit) {
        alu_credit_ += credit;

        ILOG("perfect backend get credits from alu: " << credit);

        issue_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectBackend::AcceptCreditFromLSU(const Credit& credit) {
        lsu_credit_ += credit;

        ILOG("perfect backend get credits from lsu: " << credit);

        issue_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectBackend::Issue() {
        /* for issuing */
        InstGroup lsu_tmp_instgroup;
        InstGroup alu_tmp_instgroup;
        for (InstPtr inst_ptr: rob_->GetIssuingEntry(issue_num_)) {
            if (inst_ptr->getFuType() != FuncType::STU && inst_ptr->getFuType() != FuncType::LDU) {
                if (alu_credit_) {
                    alu_tmp_instgroup.push_back(inst_ptr);
                    ILOG("issue insn to alu:" << inst_ptr);
                    SetRobIssued(inst_ptr->getRobTag());
                    --alu_credit_;
                }
            } else {
                if (lsu_credit_) {
                    lsu_tmp_instgroup.push_back(inst_ptr);
                    ILOG("issue insn to lsu:" << inst_ptr);
                    SetRobIssued(inst_ptr->getRobTag());
                    --lsu_credit_;
                }
            }
        }

        if (lsu_tmp_instgroup.size()) {
            backend_lsu_inst_out.send(lsu_tmp_instgroup);
        }

        if (alu_tmp_instgroup.size()) {
            backend_alu_inst_out.send(alu_tmp_instgroup);
        }

        if (!rob_->IsRobEmpty()) {
            rob_wakeup_st.schedule(1);
        }
    }

    void PerfectBackend::AllocateRobEntry(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            rob_->AllocateRobEntry(inst_ptr);
        }
        set_rob_commmited.schedule(1);
        issue_event_.schedule(1);
        rob_wakeup_st.schedule();
    }

    void PerfectBackend::Finish(const RobIdx& rob_idx) {
        rob_->FinishInst(rob_idx);
    }

    void PerfectBackend::WriteBack(const InstPtr& inst) {
        ILOG("load insn writeback prf: " << inst);
    }

    void PerfectBackend::SetRobIssued(const RobIdx& rob_idx) {
        rob_->IssueInst(rob_idx);
    }

    void PerfectBackend::RobCommit() {
        if (rob_->IsRobEmpty()) {
            return;
        }

        uint64_t commit_num = rob_->Commit(issue_num_);
        if (commit_num) {
            ILOG(commit_num << " instructions is commited");
            fetch_backend_credit_out.send(commit_num);
        }

        while (commit_num--) {
            num_retired_ += 1;

            if((num_retired_ % retire_heartbeat_) == 0) {
                std::cout << "model: Retired " << num_retired_.get()
                            << " instructions in " << getClock()->currentCycle()
                            << " overall IPC: " << ipc_.getValue()
                            << std::endl;
            }
        }

        if (rob_->GetIssuingEntry(issue_num_).size()) {
            issue_event_.schedule(1);
        }
        set_rob_commmited.schedule(1);

        if (!rob_->IsRobEmpty()) {
            rob_wakeup_st.schedule(1);
        }
    }

    void PerfectBackend::RobWkupStore() {
        if (rob_->IsRobEmpty()) {
            return;
        }

        RobIdx rob_idx = 0;
        if(rob_->getStoreRobIdx(rob_idx)){
            ILOG("send wakeup rob_idx: " << rob_idx << " to lsu");
            backend_lsu_rob_idx_wakeup_out.send(rob_idx);
        }

        if (!rob_->IsRobEmpty()) {
            ILOG("trigger next rob wakeup store event");
            rob_wakeup_st.schedule(1);
        }
    }
}
