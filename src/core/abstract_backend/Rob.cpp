//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"
#include "sparta/utils/SpartaAssert.hpp"

#include "Rob.hpp"

namespace TimingModel {
    const char* Rob::name = "rob";

    Rob::Rob(sparta::TreeNode *node,
             const TimingModel::Rob::RobParameter *p) :
             sparta::Unit(node),
             issue_width_(p->issue_width),
             rob_depth_(p->queue_depth),
             rob_(p->queue_depth),
             retire_heartbeat_(p->retire_heartbeat),
             stat_ipc_(&unit_stat_set_,
                       "ipc",
                       "Instructions retired per cycle",
                       &unit_stat_set_,
                       "total_number_retired/cycles"),
             num_retired_(&unit_stat_set_,
                          "total_number_retired",
                          "The total number of instructions retired by this core",
                          sparta::Counter::COUNT_NORMAL),
             num_insts_to_retire_(p->num_insts_to_retire),
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
        ILOG(name << " is flushed.");

        rob_preceding_credit_out.send(rob_depth_, sparta::Clock::Cycle(1));
        rob_.clear();
    }

    void Rob::AllocateRob_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            RobEntry rob_entry;
            rob_entry.inst_ptr = inst_ptr;
            inst_ptr->setRobTag(rob_.push(rob_entry).getIndex());
            ILOG("get inst from preceding: " << inst_ptr);
        }
        commit_event.schedule(1);
    }

    void Rob::Finish_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("rob finish instruction: " << inst_ptr);
            rob_.access(inst_ptr->getRobTag()).finish = true;
        }
    }

    void Rob::TryWakeupStore() {
        if(rob_.empty()) {
            return;
        }
        if (rob_.front().inst_ptr->getFuType() == FuncType::STU && !rob_.front().waked_up) {
            InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
            rob_.front().waked_up = true;
            ILOG("rob wakeup: " << rob_.front().inst_ptr);
            inst_group_ptr->emplace_back(rob_.front().inst_ptr);
            Rob_lsu_wakeup_out.send(inst_group_ptr);
        }
    }

    void Rob::Commit_() {
        InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        InstGroupPtr inst_bpu_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        uint64_t issue_num = std::min(issue_width_, uint64_t(rob_.size()));
        bool do_flush = false;
        while(issue_num > 0) {
            if (rob_.empty()) {
                break;
            }

            if (!rob_.front().finish) {
                break;
            }

            inst_group_ptr->emplace_back(rob_.front().inst_ptr);
            rob_.front().inst_ptr->clearCommitInfo();
            stall_cycle_count_ = 0;

            if (rob_.front().inst_ptr->getFuType() == FuncType::BRU) {
                inst_bpu_group_ptr->emplace_back(rob_.front().inst_ptr);
            }

            if (rob_.front().inst_ptr->getIsMissPrediction()) {
                do_flush = true;
                rob_redirect_pc_inst_out.send(rob_.front().inst_ptr);
                break;
            }

            rob_.pop();
            issue_num--;
        }
        TryWakeupStore();

        if (!inst_group_ptr->empty()) {
            Rob_cmt_inst_out.send(inst_group_ptr);
        }

        if (!inst_bpu_group_ptr->empty()) {
            rob_bpu_inst_out.send(inst_bpu_group_ptr);
        }

        uint64_t commit_num = inst_group_ptr->size();

        if (commit_num) {
            rob_preceding_credit_out.send(commit_num, sparta::Clock::Cycle(1));
        } else {
            stall_cycle_count_++;
            sparta_assert(stall_cycle_count_ < 1000, "commit stall: " << rob_.front().inst_ptr);
        }


        ILOG(getName() << " commit instructions: " << commit_num << " , remaining: " << rob_.size());

        num_retired_ += commit_num;

        if((num_retired_ > times_ * retire_heartbeat_) && num_retired_ != 0) {
            ++times_;
            std::cout << "model: Retired " << num_retired_.get()
                      << " instructions in " << getClock()->currentCycle()
                      << " overall IPC: " << ipc_.getValue()
                      << std::endl;
        }

        // Will be true if the user provides a -i option
        if (SPARTA_EXPECT_FALSE((num_retired_ >= num_insts_to_retire_) && (num_insts_to_retire_ != 0))) {
            getScheduler()->stopRunning();
            return;
        }

        if (!rob_.empty()) {
            commit_event.schedule(1);
        }

        if (do_flush) {
            rob_flush_out.send(1);
        }
    }

}