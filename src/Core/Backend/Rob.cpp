//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"

#include "Rob.hpp"

namespace TimingModel {
    const char* Rob::name = "rob";

    Rob::Rob(sparta::TreeNode *node,
             const TimingModel::Rob::RobParameter *p) :
             sparta::Unit(node),
             issue_width_(p->issue_width),
             rob_depth_(p->rob_depth),
             rob_(p->rob_depth),
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
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(Rob, InitCredit_));
        rob_flush_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, HandleFlush_, FlushingCriteria));
        preceding_rob_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, AllocateRob_, InstGroupPtr));
        write_back_rob_finish_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, Finish_, InstGroupPtr));
        preceding_rob_inst_in >> sparta::GlobalOrderingPoint(node, "rob_dispatch_node");
    }

    Rob::~Rob() {
        std::cout << std::endl
                  << "============================================================="
                  << std::endl
                  << "model: Retired " << num_retired_.get()
                  << " instructions in " << getClock()->currentCycle()
                  << " overall IPC: " << ipc_.getValue()
                  << std::endl
                  << "============================================================="
                  << std::endl
                  << std::endl;
    }

    void Rob::InitCredit_() {
        rob_preceding_credit_out.send(rob_depth_);
    }

    void Rob::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        commit_event.cancel();
        ILOG(name << " is flushed.");

        rob_preceding_credit_out.send(rob_.size());
        rob_.clear();
    }

    void Rob::AllocateRob_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        ILOG("rob get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            inst_ptr->setRobTag(rob_.tail());
            rob_.push({inst_ptr, true, false});
            ILOG("get insn from preceding: " << inst_ptr);
            ILOG("rob allocate instruction tag is: " << inst_ptr->getRobTag());
        }
        commit_event.schedule(1);
    }

    void Rob::Finish_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("rob finish instruction rob tag is: " << inst_ptr->getRobTag());
            ILOG("rob finish instruction: " << inst_ptr);
            rob_.access(inst_ptr->getRobTag()).finish = true;
        }
    }

    void Rob::TryWakeupStore() {
        if(rob_.empty()) {
            return;
        }
        if (rob_.front().inst_ptr->getFuType() == FuncType::STU && !rob_.front().inst_ptr->getStoreWkup()) {
            Rob_lsu_wakeup_out.send(rob_.front().inst_ptr);
        }
    }

    void Rob::Commit_() {
        InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        uint64_t issue_num = std::min(issue_width_, uint64_t(rob_.size()));
        while(issue_num--) {
            if (rob_.front().finish) {
                inst_group_ptr->emplace_back(rob_.front().inst_ptr);
                rob_.pop();
            } else {
                break;
            }
        }
        ILOG("before rob wakeup store ");
        TryWakeupStore();
        ILOG("after rob wakeup store ");

        if (!inst_group_ptr->empty()) {
            Rob_cmt_inst_out.send(inst_group_ptr);
        }
        uint64_t commit_num = inst_group_ptr->size();

        if (commit_num) {
            rob_preceding_credit_out.send(commit_num);
        }
        ILOG(getName() << " commit instructions: " << commit_num << " , remaining: " << rob_.size());
        if (rob_.front().valid) {
            ILOG(getName() << " rob front id: " << rob_.front().inst_ptr->getUniqueID());
        }

        num_retired_ += commit_num;

        if((num_retired_ > times_ * retire_heartbeat_) && num_retired_ != 0) {
            ++times_;
            std::cout << "model: Retired " << num_retired_.get()
                      << " instructions in " << getClock()->currentCycle()
                      << " overall IPC: " << ipc_.getValue()
                      << std::endl;
        }

        if (!rob_.empty()) {
            commit_event.schedule(1);
        }
    }

}