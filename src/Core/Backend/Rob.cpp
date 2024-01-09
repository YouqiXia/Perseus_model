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
             rob_("rob_queue", p->rob_depth, getClock(), &unit_stat_set_),
             finish_queue_("finish_queue", p->rob_depth, getClock(), &unit_stat_set_)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(Rob, InitCredit_));
        rob_flush_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, HandleFlush_, FlushingCriteria));
        preceding_rob_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, AllocateRob_, InstGroupPtr));
        write_back_rob_finish_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(Rob, Finish_, InstGroupPtr));
    }

    void Rob::InitCredit_() {
        rob_preceding_credit_out.send(rob_depth_);
    }

    void Rob::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        commit_event.cancel();
        ILOG(name << " is flushed.");

        rob_preceding_credit_out.send(rob_.size());
        rob_.clear();
        finish_queue_.clear();
    }

    void Rob::AllocateRob_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        ILOG("rob get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            inst_ptr->setRobTag(rob_.push(inst_ptr).getIndex());
            finish_queue_.push(false);
        }
        commit_event.schedule(1);
    }

    void Rob::Finish_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            finish_queue_.access(inst_ptr->getRobTag()) = true;
        }
    }

    void Rob::Commit_() {
        InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        uint64_t issue_num = std::min(issue_width_, uint64_t(rob_.size()));
        while(issue_num--) {
            if (finish_queue_.front()) {
                inst_group_ptr->emplace_back(rob_.front());
                rob_.pop();
                finish_queue_.pop();
            } else {
                break;
            }
        }
        if (!inst_group_ptr->empty()) {
            Rob_cmt_inst_out.send(inst_group_ptr);
        }
    }

}