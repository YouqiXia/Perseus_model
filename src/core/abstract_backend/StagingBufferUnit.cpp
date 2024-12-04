//
// Created by yzhang on 12/2/24.
//

#include "StagingBufferUnit.hpp"

namespace TimingModel {

    const char* StagingBufferUnit::name = "staging_buffer";

    StagingBufferUnit::StagingBufferUnit(sparta::TreeNode *node,
                                             const TimingModel::StagingBufferUnit::StagingBufferParameter *p) :
            sparta::Unit(node),
            queue_depth_(p->queue_depth + p->issue_width * p->latency),
            issue_width_(p->issue_width),
            pipe_rank_(p->pipe_rank),
            latency_(p->latency),
            is_spec_wakeup_(p->is_spec_wakeup)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(StagingBufferUnit, Startup_));
        bypass_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                        (StagingBufferUnit, BypassInst_, InstGroupPairPtr));
        following_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                        (StagingBufferUnit, AcceptCredit_, CreditPairPtr));
        preceding_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                        (StagingBufferUnit, RecieveInsts_, InstGroupPtr));
    }

    void StagingBufferUnit::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        global_param_ptr_ = getGlobalParams(getContainer());
        pmu_ = getPmuUnit(getContainer());

        for (auto group_pair: global_param_ptr_->getGroupRanksMap()) {
            if (group_pair.second.count(pipe_rank_) != 0) {
                group_idx_ = group_pair.first;
                break;
            }
        }

        InitCredit_();

        if (pmu_->IsPmuOn()) {
//            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void StagingBufferUnit::InitCredit_() {
        CreditPairPtr rs_credit_ptr =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr->pipe_rank = pipe_rank_;
        rs_credit_ptr->credit = queue_depth_;
        preceding_credit_out.send(rs_credit_ptr);
    }

    void StagingBufferUnit::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        preceding_inst_in.cancel();
        bypass_inst_in.cancel();
        following_credit_in.cancel();
        inst_queue_.clear();
    }

    void StagingBufferUnit::AcceptCredit_(const TimingModel::CreditPairPtr &credit_pair_ptr) {
        credit_ += credit_pair_ptr->credit;
        ILOG("get credit: " << credit_pair_ptr->credit << ", after updating: " << credit_);
        process_event.schedule(0);
    }

    void StagingBufferUnit::RecieveInsts_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        ILOG("get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPipeRank() != pipe_rank_) {
                continue;
            }
            ILOG("get inst: " << inst_ptr);
            if (latency_ == 0) {
                inst_queue_.emplace_back(inst_ptr);
            } else {
                latency_queue_.Push(latency_, inst_ptr);
            }
            dependency_table_.Allocate(inst_ptr);
        }

        process_event.schedule(0);
    }

    void StagingBufferUnit::BypassInst_(const TimingModel::InstGroupPairPtr &inst_group_pair_ptr) {
        sparta_assert(inst_group_pair_ptr->inst_group.front()->getGroupIdx() == group_idx_);
        if (!is_spec_wakeup_) {
            sparta_assert(dependency_table_.Empty(), "speculative wakeup");
            return;
        }
        for (auto inst_ptr: inst_group_pair_ptr->inst_group) {
            dependency_table_.Resolve(inst_ptr);
        }
    }

    void StagingBufferUnit::ProcessInsts_() {
        uint64_t produce_num = GetProduceNum_();

        InstGroupPtr processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        InstGroupPtr wakeup_resolve_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        TickLatencyQueue_();

        while(produce_num--) {
            auto inst_ptr = inst_queue_.front();
            if (!inst_ptr->getIsRs1Forward() && !inst_ptr->getIsRs2Forward()) {
                processed_group_ptr->emplace_back(inst_ptr);
                ILOG("send insn to following: " << inst_ptr);
            } else {
                inst_ptr->setIsCanceled(true);
                ILOG("cancel insn: " << inst_ptr);
            }
            wakeup_resolve_group_ptr->emplace_back(inst_ptr);
            --credit_;
            inst_queue_.pop_front();
            dependency_table_.Pop(inst_ptr);
        }

        DataTransfer_(processed_group_ptr, wakeup_resolve_group_ptr);
    }

    uint64_t StagingBufferUnit::GetProduceNum_() {
        uint64_t produce_num = std::min(inst_queue_.size(), issue_width_);
        produce_num = std::min(produce_num, credit_);

        return produce_num;
    }

    void StagingBufferUnit::TickLatencyQueue_() {
        while (!latency_queue_.Empty()) {
            auto inst_ptr = latency_queue_.PopFront();
            inst_queue_.emplace_back(inst_ptr);
        }

        latency_queue_.Tick();
        ILOG("latency queue tick");

        if (!latency_queue_.IsStopped()) {
            process_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void StagingBufferUnit::DataTransfer_(TimingModel::InstGroupPtr processed_group_ptr, InstGroupPtr wakeup_resolve_group_ptr) {
        if (!processed_group_ptr->empty()) {
            following_inst_out.send(processed_group_ptr);
        }

        if (!processed_group_ptr->empty()) {
            CreditPairPtr rs_credit_ptr =
                    sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
            rs_credit_ptr->pipe_rank = pipe_rank_;
            rs_credit_ptr->credit = processed_group_ptr->size();
            preceding_credit_out.send(rs_credit_ptr, sparta::Clock::Cycle(1));
        }

        if (!wakeup_resolve_group_ptr->empty()) {
            wakeup_resolve_inst_out.send(wakeup_resolve_group_ptr);
        }

        if (!inst_queue_.empty()) {
            process_event.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());
    }


}
