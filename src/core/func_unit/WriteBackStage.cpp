//
// Created by yzhang on 1/6/24.
//

#include "WriteBackStage.hpp"

namespace TimingModel {

    const char* WriteBackStage::name = "write_back_stage";

    WriteBackStage::WriteBackStage(sparta::TreeNode *node,
                                   const TimingModel::WriteBackStage::WriteBackStageParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_width),
            wb_latency_(p->wb_latency),
            is_wb_perfect_(p->is_perfect_mode)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(WriteBackStage, Startup_));
        writeback_flush_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(WriteBackStage, HandleFlush_, FlushingCriteria));
        preceding_write_back_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                                                    (WriteBackStage, AcceptFuncInst_, InstGroupPairPtr));
    }

    void WriteBackStage::Startup_() {
        global_param_ptr_ = getGlobalParams(getContainer());
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());

        for (auto pipe_pair: global_param_ptr_->getWriteBackMap()) {
            inst_queue_map_[pipe_pair.first] = std::deque<InstPtr>();
            write_back_width_map_[pipe_pair.first] = pipe_pair.second;
        }

        SendInitCredit_();

        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void WriteBackStage::SendInitCredit_() {
        for (auto pipe_pair: write_back_width_map_){
            CreditPairPtr credit_pair_ptr =
                    sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
            credit_pair_ptr->name = pipe_pair.first;
            credit_pair_ptr->credit = pipe_pair.second;
            preceding_write_back_credit_out.send(credit_pair_ptr, sparta::Clock::Cycle(1));
        }
    }

    void WriteBackStage::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG(getName() << " is flushed");
        for (auto& inst_pair: inst_queue_map_) {
            CreditPairPtr credit_pair_ptr =
                    sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
            credit_pair_ptr->name = getName();
            credit_pair_ptr->credit = inst_pair.second.size();
            preceding_write_back_credit_out.send(credit_pair_ptr);
        }
        inst_queue_map_.clear();
    };

    void WriteBackStage::AcceptFuncInst_(const InstGroupPairPtr &inst_group_pair_ptr) {
        auto inst_group_ptr = &inst_group_pair_ptr->inst_group;
        auto pipe_name = inst_group_pair_ptr->name;
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get instructions: " << inst_ptr);
            inst_queue_map_[pipe_name].emplace_back(inst_ptr);
        }
        arbitrate_inst_event.schedule(0);
    }

    void WriteBackStage::ArbitrateInst_() {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "1 event", 1);

        size_t queue_size_ = 0;
        for (auto& inst_pair: inst_queue_map_) {
                queue_size_ += inst_pair.second.size();
            }
        if (queue_size_ < issue_num_) {
            pmu_->Monitor(getName(), "3 queue loss", issue_num_-queue_size_);
        }
        if (queue_size_ == 0) {
            pmu_->Monitor(getName(), "4 queue empty", 1);
            pmu_->Monitor(getName(), "5 total loss", issue_num_);
            return;
        }

        uint64_t produce_num = std::min<uint64_t>(queue_size_, issue_num_);
        uint64_t consume_num = 0;
        InstGroupPtr inst_group_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        for (auto& inst_pair: inst_queue_map_) {
            CreditPairPtr credit_pair_ptr =
                    sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
            credit_pair_ptr->name = inst_pair.first;
            credit_pair_ptr->credit = 0;
            uint32_t consume_per_entry = 0;
            for (auto inst_ptr: inst_pair.second) {
                if (!produce_num) {
                    break;
                }
                inst_group_ptr_tmp->emplace_back(inst_ptr);
                ILOG(getName() << " arbitrate instructions: " << inst_ptr);
                --produce_num;
                consume_per_entry++;
                consume_num++;
                credit_pair_ptr->credit++;
            }

            for (int i = 0; i < consume_per_entry; ++i) {
                inst_pair.second.pop_front();
            }
            preceding_write_back_credit_out.send(credit_pair_ptr);
        }
        pmu_->Monitor(getName(), "5 total loss", issue_num_-consume_num);

        if (!inst_group_ptr_tmp->empty()) {
            write_back_following_port_out.send(inst_group_ptr_tmp);
            size_t write_back_size = 0;
            for (auto& inst_pair: inst_queue_map_) {
                write_back_size += inst_pair.second.size();
            }
            ILOG("queue size is after update: " << write_back_size);
        }

        bool empty = true;
        for (auto& inst_pair: inst_queue_map_) {
            empty &= inst_pair.second.empty();
        }
        if (!empty) {
            arbitrate_inst_event.schedule(1);
        }
    }

    void WriteBackStage::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        pmu_->Monitor(getName(), "2 pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "5 total loss", issue_num_);
        pmu_->Monitor(getName(), "3 queue loss", issue_num_);
        pmu_->Monitor(getName(), "4 queue empty", 1);
    }
}