//
// Created by yzhang on 1/16/24.
//

#include "PhysicalRegfileUnit.hpp"

namespace TimingModel {

    const char* PhysicalRegfileUnit::name = "physical_regfile";

    PhysicalRegfileUnit::PhysicalRegfileUnit(sparta::TreeNode *node,
            const TimingModel::PhysicalRegfileUnit::PhysicalRegfileParameter *p) :
            sparta::Unit(node),
            preceding_physical_regfile_read_in(&unit_port_set_, "preceding_physical_regfile_read_in", sparta::SchedulingPhase::Tick, p->latency),
            queue_depth_(p->queue_depth),
            issue_width_(p->issue_width),
            latency_(p->latency),
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
    }

    void PhysicalRegfileUnit::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        global_param_ptr_ = getGlobalParams(getContainer());
        pmu_ = getPmuUnit(getContainer());

        for (auto pipe_pair: global_param_ptr_->getDispatchMap()) {
            credit_map_[pipe_pair.first] = 0;
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
//        if (latency_ == 0) {
//            physical_regfile_following_read_out.send(inst_group_ptr);
//            return;
//        }
        ILOG("get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get inst: " << inst_ptr);
            inst_queue_.emplace_back(inst_ptr);
        }

        process_event.schedule(0);
    }

    void PhysicalRegfileUnit::ProcessInsts_() {
        uint64_t produce_num = GetProduceNum_();

        InstGroupPtr processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        while(produce_num--) {
            auto inst_ptr = inst_queue_.front();
            processed_group_ptr->emplace_back(inst_ptr);
            ILOG("send insn to following: " << inst_ptr);
            credit_map_[inst_ptr->getPipeRank()]--;
            preceding_credit_map_[inst_ptr->getPipeRank()]++;
            inst_queue_.pop_front();
        }

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

        if (!inst_queue_.empty()) {
            process_event.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());

    }

    uint64_t PhysicalRegfileUnit::GetProduceNum_() {
        uint64_t produce_num = std::min(inst_queue_.size(), issue_width_);

        return produce_num;
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
