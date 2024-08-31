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
        flush_criteria_ = flush_criteria;
        flush_event_.schedule(1);
    }

    void FlushManager::DoFlush() {
        global_flush_signal_out.send(flush_criteria_);
    }
}