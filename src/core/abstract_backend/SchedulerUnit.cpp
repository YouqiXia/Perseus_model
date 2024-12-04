//
// Created by yzhang on 1/2/24.
//
#include "sparta/utils/LogUtils.hpp"

#include <cmath>

#include "SchedulerUnit.hpp"

namespace TimingModel {

    const char *SchedulerUnit::name = "scheduler";

    SchedulerUnit::SchedulerUnit(sparta::TreeNode *node,
                                 const TimingModel::SchedulerUnit::ReservationStationParameter *p) :
            sparta::Unit(node),
            pipe_rank_(p->pipe_rank),
            wakeup_latency_(p->wakeup_latency),
            is_spec_wakeup_(p->is_spec_wakeup),
            issue_num_(p->issue_width),
            rs_depth_(p->queue_depth),
            dependency_table_(p->phy_reg_num),
            issue_window_()
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(SchedulerUnit, Startup_));
        scheduler_flush_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(SchedulerUnit, HandleFlush_, FlushingCriteria));
        preceding_scheduler_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(SchedulerUnit, AllocateReStation, InstGroupPairPtr));
        following_scheduler_credit_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(SchedulerUnit, AcceptCredit_, CreditPairPtr));
        forwarding_scheduler_inst_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(SchedulerUnit, GetForwardingData, InstGroupPtr));
        spec_wake_up_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(SchedulerUnit, SpecWakeup_, InstGroupPtr));
        preceding_scheduler_inst_in >> sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding");
        sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding") >> forwarding_scheduler_inst_in;
        sparta::GlobalOrderingPoint(node, "rs_allocate_forwarding") >> spec_wake_up_in;
    }

    void SchedulerUnit::Startup_() {
        global_param_ptr_ = getGlobalParams(getContainer());
        fu_latency_map_ = &global_param_ptr_->getLatencyMap();
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());
        InitCredit_();
        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void SchedulerUnit::InitCredit_() {
        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->pipe_rank = pipe_rank_;
        rs_credit_ptr_tmp->credit = rs_depth_;
        scheduler_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
    }

    void SchedulerUnit::AcceptCredit_(const TimingModel::CreditPairPtr &credit_pair_ptr) {
        if (credit_pair_ptr->pipe_rank != pipe_rank_) {
            return;
        }
        credit_ += credit_pair_ptr->credit;
        ILOG(getName() << " accept credits: " << credit_pair_ptr->credit << ", updated credits: " << credit_);

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void SchedulerUnit::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        ILOG(getName() << " is flushed.");

        credit_ = 0;
        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->pipe_rank = pipe_rank_;
        rs_credit_ptr_tmp->credit = rs_depth_;
        scheduler_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
        issue_window_.clear();
        size_ = 0;
    }

    void SchedulerUnit::AllocateReStation(const TimingModel::InstGroupPairPtr &inst_group_pair_ptr) {
        if (inst_group_pair_ptr->pipe_rank != pipe_rank_) {
            return;
        }
        auto inst_group_ptr = &inst_group_pair_ptr->inst_group;
        ILOG(getName() << " get instructions: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get insn from preceding: " << inst_ptr);
            ReStationEntryPtr tmp_restation_entry =
                    sparta::allocate_sparta_shared_pointer<ReStationEntry>(*allocator_->re_station_entry_allocator);
            tmp_restation_entry->inst_ptr = inst_ptr;
            if (!inst_ptr->getIsRs1Forward()) {
                tmp_restation_entry->rs1_valid = true;
            }
            if (!inst_ptr->getIsRs2Forward()) {
                tmp_restation_entry->rs2_valid = true;
            }

            if (is_spec_wakeup_) {
                if (inst_ptr->getRs1SpecWakeupTag() > 1) {
                    auto idx = inst_ptr->getPhyRs1();
                    sparta_assert(latency_table_.count(idx) == 1, "speculative wakeup");
                    sparta_assert(latency_table_[idx] == inst_ptr->getRs1SpecWakeupTag() - 1, "speculative wakeup");
                } else if (inst_ptr->getRs1SpecWakeupTag() == 1) {
                    tmp_restation_entry->rs1_spec_wakeup = true;
                }

                if (inst_ptr->getRs2SpecWakeupTag() > 1) {
                    auto idx = inst_ptr->getPhyRs2();
                    sparta_assert(latency_table_.count(idx) == 1, "speculative wakeup");
                    sparta_assert(latency_table_[idx] == inst_ptr->getRs2SpecWakeupTag() - 1, "speculative wakeup");
                } else if (inst_ptr->getRs2SpecWakeupTag() == 1) {
                    tmp_restation_entry->rs2_spec_wakeup = true;
                }
            }

            SizeUp_();
            dependency_table_.Allocate(tmp_restation_entry);
            issue_window_.emplace_back(tmp_restation_entry);
            inst_ptr->setWindowEntry(tmp_restation_entry.get());
        }

        process_event.schedule(sparta::Clock::Cycle(0));
    }

    void SchedulerUnit::GetForwardingData(const TimingModel::InstGroupPtr &forwarding_inst_group_ptr) {
        for (auto& forwarding_inst_ptr: *forwarding_inst_group_ptr) {
            bool find = false;
            find = dependency_table_.Resolve(forwarding_inst_ptr);
            if (find) {
                ILOG("get forwarding data from: " << forwarding_inst_ptr);
            }
        }
    }

    void SchedulerUnit::ProcessInsts_() {
        PopInst_();

        uint64_t produce_num_max = std::min<uint64_t>(size_, issue_num_);
        uint64_t produce_num = GetProduceNum_();
        PmuBandwidthAcc_(produce_num_max, produce_num);

        InstGroupPtr processed_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);

        SelectInst_(produce_num, processed_group_ptr);

        if (is_spec_wakeup_) {
            EarlyWakeupCtrl(processed_group_ptr);
        }

        DataTransfer_(processed_group_ptr);
    }

    void SchedulerUnit::PopInst_() {
        for (int i = 0; i < issue_window_.size(); i++) {
            if (issue_window_.front()->is_issued) {
                issue_window_.pop_front();
            } else {
                break;
            }
        }
    }

    void SchedulerUnit::PmuBandwidthAcc_(uint64_t produce_num_max, uint64_t produce_num) {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "event", 1);

        if (size_ < issue_num_) {
            pmu_->Monitor(getName(), "queue loss", issue_num_ - size_);
        }
        if (size_ == 0) {
            pmu_->Monitor(getName(), "queue empty", 1);
        }

        if (credit_ < produce_num_max) {
            pmu_->Monitor(getName(), "fu loss", produce_num_max-credit_);
        }
        if (credit_ == 0) {
            pmu_->Monitor(getName(), "fu full", 1);
        }
    }

    uint64_t SchedulerUnit::GetProduceNum_() {
        uint64_t produce_num = std::min(credit_, issue_num_);
        produce_num = std::min(size_, produce_num);
        return produce_num;
    }

    void SchedulerUnit::SelectInst_(uint64_t produce_num, TimingModel::InstGroupPtr processed_group_ptr) {
        // oldest passing
        uint64_t consume_num = 0;
        for (auto &rs_entry: issue_window_) {
            if (produce_num == 0) {
                break;
            }

            if (rs_entry->is_issued) {
                continue;
            }

            if (rs_entry->is_spec_issued && !rs_entry->is_canceled && is_spec_wakeup_) {
                continue;
            }

            if (rs_entry->rs1_valid && rs_entry->rs2_valid) {
                ILOG(getName() << " passing instruction: " << rs_entry->inst_ptr);
                --credit_;
                --produce_num;
                rs_entry->inst_ptr->setIsRs1Forward(!rs_entry->rs1_valid);
                rs_entry->inst_ptr->setIsRs2Forward(!rs_entry->rs2_valid);
                processed_group_ptr->emplace_back(rs_entry->inst_ptr);
                rs_entry->is_issued = true;
                consume_num++;
                SizeDown_();
//            } else if (rs_entry->rs1_valid ^ rs_entry->rs2_valid) {
            } else if ((rs_entry->rs1_valid || rs_entry->rs1_spec_wakeup) &&
                       (rs_entry->rs2_valid || rs_entry->rs2_spec_wakeup) && is_spec_wakeup_) {
                --credit_;
                --produce_num;
                rs_entry->inst_ptr->setIsSpecWakeup(true);
                rs_entry->inst_ptr->setIsRs1Forward(!rs_entry->rs1_valid);
                rs_entry->inst_ptr->setIsRs2Forward(!rs_entry->rs2_valid);
                processed_group_ptr->emplace_back(rs_entry->inst_ptr);
                rs_entry->is_spec_issued = true;
                consume_num++;
//                SizeDown_(); Do not pop this speculative issued entry
            }
        }
        pmu_->Monitor(getName(), "operand loss", produce_num);
        pmu_->Monitor(getName(), "total loss", issue_num_ - consume_num);
    }

    void SchedulerUnit::EarlyWakeupCtrl(TimingModel::InstGroupPtr processed_group_ptr) {
        spec_wake_up_out.send(processed_group_ptr, sparta::Clock::Cycle(wakeup_latency_));

        for (auto it = latency_table_.begin(); it != latency_table_.end(); ) {
            if (it->second - 1 == 0) {
                dependency_table_.EarlyWakeup(it->first);
                it = latency_table_.erase(it);
            } else {
                --it->second;
                ++it;
            }
        }

        if (!latency_table_.empty()) {
            process_event.schedule(1);
        }
    }

    void SchedulerUnit::DataTransfer_(TimingModel::InstGroupPtr processed_group_ptr) {
        CreditPairPtr rs_credit_ptr_tmp =
                sparta::allocate_sparta_shared_pointer<CreditPair>(*allocator_->credit_pair_allocator);
        rs_credit_ptr_tmp->pipe_rank = pipe_rank_;
        rs_credit_ptr_tmp->credit = processed_group_ptr->size();
        for (auto &inst_ptr: *processed_group_ptr) {
            ILOG("send insn to following: " << inst_ptr);
        }
        if (!processed_group_ptr->empty()) {
            scheduler_following_inst_out.send(processed_group_ptr);
            ILOG(getName() << " queue size is after update: " << size_);
        }

        if (!processed_group_ptr->empty()) {
            scheduler_preceding_credit_out.send(rs_credit_ptr_tmp, sparta::Clock::Cycle(1));
        }

        if (!issue_window_.empty() && credit_ > 0) {
            process_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void SchedulerUnit::SpecWakeup_(const InstGroupPtr& inst_group_ptr) {
        sparta_assert(is_spec_wakeup_, "speculative wakeup");
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRdType() == RegType_t::NONE) {
                continue;
            }

            auto latency = fu_latency_map_->at(inst_ptr->getFuType());

            if (latency > wakeup_latency_ + 1) {
                // speculative wakeup start up
                latency_table_[inst_ptr->getPhyRd()] = latency - wakeup_latency_ - 1;
            } else {
                dependency_table_.EarlyWakeup(inst_ptr->getPhyRd());
            }
        }
    }

    void SchedulerUnit::WakeupResolve_(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getIsCanceled()) {
                sparta_assert(is_spec_wakeup_, "speculative wakeup");
                sparta_assert(!inst_ptr->getWindowEntry()->is_canceled, "speculative wakeup")
                inst_ptr->getWindowEntry()->is_canceled = true;
            } else if (inst_ptr->getIsSpecWakeup()) {
                sparta_assert(!inst_ptr->getWindowEntry()->is_issued, "speculative wakeup")
                inst_ptr->getWindowEntry()->is_issued = true;
            }
        }
    }

    void SchedulerUnit::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        pmu_->Monitor(getName(), "pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "total loss", issue_num_);

        if (size_ < issue_num_) {
            pmu_->Monitor(getName(), "queue loss", issue_num_-size_);
        }
        if (size_ == 0) {
            pmu_->Monitor(getName(), "queue empty", 1);
            return;
        }

        uint64_t produce_num_max = std::min<uint64_t>(size_, issue_num_);
        pmu_->Monitor(getName(), "fu loss", produce_num_max);
        pmu_->Monitor(getName(), "fu full", 1);
    }
}