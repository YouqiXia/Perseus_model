//
// Created by yzhang on 3/1/24.
//

#include "FlushManager.hpp"

namespace TimingModel {

    const char *FlushManager::name = "flush_manager";

    FlushManager::FlushManager(sparta::TreeNode *node, const TimingModel::FlushManager::FlushManagerParameter *p):
        sparta::Unit(node),
        scheduler_(node->getScheduler())
    {
        rob_flush_manager_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(FlushManager, ForwardingFlushSignal, FlushingCriteria));
    }

    void FlushManager::ForwardingFlushSignal(const FlushingCriteria& flush_criteria) {
        auto tick = scheduler_->getCurrentTick();
        scheduler_->stopRunning();
        scheduler_->restartAt(tick);
        flush_criteria_ = flush_criteria;
        nop_event_.schedule(0);
        flush_event_.schedule(1);
        scheduler_->keepRunning();
    }

    void FlushManager::DoFlush() {
        global_flush_signal_out.send(flush_criteria_);
    }
}