//
// Created by yzhang on 1/16/24.
//

#include "PhysicalRegfileUnit.hpp"

namespace TimingModel {

    const char* PhysicalRegfileUnit::name = "physical_regfile";

    PhysicalRegfileUnit::PhysicalRegfileUnit(sparta::TreeNode *node,
            const TimingModel::PhysicalRegfileUnit::PhysicalRegfileParameter *p) :
            sparta::Unit(node),
            queue_depth_(p->queue_depth + p->issue_width * p->latency),
            issue_width_(p->issue_width),
            latency_(p->latency),
            is_spec_wakeup_(p->is_spec_wakeup),
            phy_regfile_(p->phy_reg_num, 0),
            phy_reg_num_(p->phy_reg_num)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PhysicalRegfileUnit, Startup_));
        preceding_physical_regfile_read_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfileUnit, RecieveInsts_, InstGroupPtr));
        preceding_physical_regfile_write_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfileUnit, WritePhysicalReg_, InstGroupPtr));
        following_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfileUnit, AcceptCredit_, CreditPairPtr));
        bypass_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfileUnit, BypassInst_, InstGroupPairPtr));

    }

    void PhysicalRegfileUnit::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        global_param_ptr_ = getGlobalParams(getContainer());
        dispatch_map_ptr_ = &global_param_ptr_->getDispatchMap();
        group_ranks_map_ptr_ = &global_param_ptr_->getGroupRanksMap();
        pmu_ = getPmuUnit(getContainer());
        issue_width_per_pipe_ = issue_width_ / dispatch_map_ptr_->size();

        for (const auto& pipe_pair: global_param_ptr_->getDispatchMap()) {
            credit_map_[pipe_pair.first] = 0;
        }

        for (const auto& pipe_pair: *group_ranks_map_ptr_) {
            dependency_table_[pipe_pair.first] = DependencyTable();
        }

        for (const auto& pipe_pair: global_param_ptr_->getDispatchMap()) {
            inst_queue_[pipe_pair.first] = std::deque<InstPtr>();
        }

        InitCredit_();

        if (pmu_->IsPmuOn()) {
//            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void PhysicalRegfileUnit::InitCredit_() {
        for (auto& credit_pair: credit_map_) {
            CreditPairPtr rs_credit_ptr =
                    sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
            rs_credit_ptr->pipe_rank = credit_pair.first;
            rs_credit_ptr->credit = queue_depth_;
            preceding_credit_out.send(rs_credit_ptr, sparta::Clock::Cycle(0));
        }
    }

    void PhysicalRegfileUnit::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        preceding_physical_regfile_read_in.cancel();
        preceding_physical_regfile_write_in.cancel();
        inst_queue_.clear();
    }

    void PhysicalRegfileUnit::AcceptCredit_(const TimingModel::CreditPairPtr &credit_pair_ptr) {
        credit_map_[credit_pair_ptr->pipe_rank] += credit_pair_ptr->credit;
        ILOG("get " << credit_pair_ptr->pipe_rank << " rank, credit: " << credit_pair_ptr->credit <<
                    "after updating: " << credit_map_[credit_pair_ptr->pipe_rank]);
        process_event.schedule(0);
    }

    void PhysicalRegfileUnit::RecieveInsts_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        ILOG("get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            auto pipe_rank = inst_ptr->getPipeRank();
            auto group_idx = inst_ptr->getGroupIdx();
            ILOG("get inst: " << inst_ptr);
            dependency_table_[group_idx].Allocate(inst_ptr);
            if (latency_ == 0) {
                inst_queue_[pipe_rank].emplace_back(inst_ptr);
            } else {
                latency_queue_.Push(latency_, inst_ptr);
            }
        }

        process_event.schedule(0);
    }

    void PhysicalRegfileUnit::BypassInst_(const TimingModel::InstGroupPairPtr &inst_group_pair_ptr) {
        auto group_idx = inst_group_pair_ptr->inst_group.front()->getGroupIdx();
        if (!is_spec_wakeup_) {
            sparta_assert(dependency_table_[group_idx].Empty(), "speculative wakeup");
            return;
        }

        for (auto inst_ptr: inst_group_pair_ptr->inst_group) {
            dependency_table_[group_idx].Resolve(inst_ptr);
        }
    }

    void PhysicalRegfileUnit::ProcessInsts_() {

        TickLatencyQueue_();

        InstGroupPtr processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        for (const auto& pipe_pair: *dispatch_map_ptr_) {
            auto pipe_rank = pipe_pair.first;
            uint64_t produce_num = GetProduceNum_(pipe_rank);

            while(produce_num--) {
                auto inst_ptr = inst_queue_[pipe_rank].front();
                auto group_idx = inst_ptr->getGroupIdx();
                processed_group_ptr->emplace_back(inst_ptr);
                ILOG("send insn to following: " << inst_ptr);
                credit_map_[pipe_rank]--;
                preceding_credit_map_[pipe_rank]++;
                inst_queue_[pipe_rank].pop_front();
                dependency_table_[group_idx].Pop(inst_ptr);
            }
        }

        DataTransfer_(processed_group_ptr);
    }

    uint64_t PhysicalRegfileUnit::GetProduceNum_(uint64_t pipe_rank) {
        uint64_t produce_num = std::min(inst_queue_[pipe_rank].size(), issue_width_per_pipe_);
        produce_num = std::min(produce_num, credit_map_[pipe_rank]);

        return produce_num;
    }

    void PhysicalRegfileUnit::TickLatencyQueue_() {
        while (!latency_queue_.Empty()) {
            auto inst_ptr = latency_queue_.PopFront();
            auto pipe_rank = inst_ptr->getPipeRank();
            inst_queue_[pipe_rank].emplace_back(inst_ptr);
        }

        latency_queue_.Tick();
        ILOG("latency queue tick");

        if (!latency_queue_.IsStopped()) {
            process_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void PhysicalRegfileUnit::DataTransfer_(TimingModel::InstGroupPtr processed_group_ptr) {
        if (!processed_group_ptr->empty()) {
            physical_regfile_following_read_out.send(processed_group_ptr);
            for (auto& credit_pair: preceding_credit_map_) {
                CreditPairPtr rs_credit_ptr =
                        sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
                rs_credit_ptr->pipe_rank = credit_pair.first;
                rs_credit_ptr->credit = credit_pair.second;
                preceding_credit_out.send(rs_credit_ptr, sparta::Clock::Cycle(0));
                credit_pair.second = 0;
            }
        }

        bool empty = true;
        for (const auto& inst_queue_pair: inst_queue_) {
            empty &= inst_queue_pair.second.empty();
        }

        if (!empty) {
           process_event.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());
    }

    void PhysicalRegfileUnit::ReadPhysicalReg_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRs1Type() != RegType_t::NONE && !inst_ptr->getIsRs1Forward()) {
                if (inst_ptr->getPhyRs1() == 0) {
                    inst_ptr->setOperand1(0);
                } else {
                    sparta_assert(inst_ptr->getPhyRs1() < phy_reg_num_, "physical register accessing is out of range");
                    inst_ptr->setOperand1(phy_regfile_[inst_ptr->getPhyRs1()]);
                }
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE && !inst_ptr->getIsRs2Forward()) {
                if (inst_ptr->getPhyRs2() == 0) {
                    inst_ptr->setOperand2(0);
                } else {
                    sparta_assert(inst_ptr->getPhyRs2() < phy_reg_num_, "physical register accessing is out of range");
                    inst_ptr->setOperand2(phy_regfile_[inst_ptr->getPhyRs2()]);
                }
            }
        }
        physical_regfile_following_read_out.send(inst_group_ptr);
    }

    void PhysicalRegfileUnit::WritePhysicalReg_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() == 0) {
                continue;
            }
            sparta_assert(inst_ptr->getPhyRd() < phy_reg_num_, "physical register accessing is out of range");
            phy_regfile_[inst_ptr->getPhyRd()] = inst_ptr->getRdResult();
        }
    }
}
