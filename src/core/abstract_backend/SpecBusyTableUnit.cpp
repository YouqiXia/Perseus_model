#include "SpecBusyTableUnit.hpp"

namespace TimingModel {

    const char* SpecBusyTableUnit::name = "spec_busy_table";

    SpecBusyTableUnit::SpecBusyTableUnit(sparta::TreeNode *node,
                                 const TimingModel::SpecBusyTableUnit::SpecBusyTableParameter *p) :
            sparta::Unit(node),
            pipe_rank_(p->pipe_rank),
            busy_table_(p->phy_reg_num),
            wakeup_latency_(p->wakeup_latency),
            is_spec_wakeup_(p->is_spec_wakeup)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(SpecBusyTableUnit, Startup_));

        flush_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                SpecBusyTableUnit, HandleFlush_, FlushingCriteria));
        check_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                SpecBusyTableUnit, CheckBusy_, InstGroupPairPtr));
        update_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                 SpecBusyTableUnit, UpdateBusy_, InstGroupPtr));
        wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                SpecBusyTableUnit, SpecWakeup_, InstGroupPtr));
        wakeup_resolve_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(
                SpecBusyTableUnit, WakeupResolve_, InstGroupPtr));

        /* should not forward updated wakeup signal to input checking
         * because it will be processed in scheduler
         */
        check_in >> sparta::GlobalOrderingPoint(node, "busy_table_update");
        sparta::GlobalOrderingPoint(node, "busy_table_update") >> wakeup_in;
    }

    void SpecBusyTableUnit::Startup_() {
        global_param_ptr_ = getGlobalParams(getContainer());
        fu_latency_map_ = &global_param_ptr_->getLatencyMap();
        recorded_ranks = global_param_ptr_->getGroupRanksMap()[pipe_rank_];
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());
    }

    void SpecBusyTableUnit::CheckBusy_(const TimingModel::InstGroupPairPtr &inst_group_pair_ptr) {
        if (recorded_ranks.count(inst_group_pair_ptr->pipe_rank) == 0) {
            return;
        }

        for (auto inst_ptr: inst_group_pair_ptr->inst_group) {
            inst_ptr->setGroupIdx(pipe_rank_);
        }

        if (!is_spec_wakeup_) {
            return;
        }

        for (auto& inst_ptr: inst_group_pair_ptr->inst_group) {
            if (inst_ptr->getRs1Type() != RegType_t::NONE) {
                auto idx = inst_ptr->getPhyRs1();
                if (busy_table_.GetBusyBit(idx)) {
                    if (latency_table_.count(idx)) {
                        inst_ptr->setRs1SpecWakeupTag(latency_table_[idx]);
                    } else {
                        inst_ptr->setRs1SpecWakeupTag(1);
                    }
                } else {
                    inst_ptr->setRs1SpecWakeupTag(0);
                }
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE) {
                auto idx = inst_ptr->getPhyRs2();
                if (busy_table_.GetBusyBit(idx)) {
                    if (latency_table_.count(idx)) {
                        inst_ptr->setRs2SpecWakeupTag(latency_table_[idx]);
                    } else {
                        inst_ptr->setRs2SpecWakeupTag(1);
                    }
                } else {
                    inst_ptr->setRs2SpecWakeupTag(0);
                }
            }
        }
    }

    void SpecBusyTableUnit::SpecWakeup_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        sparta_assert(is_spec_wakeup_, "speculative wakeup");
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRdType() == RegType_t::NONE) {
                continue;
            }
            busy_table_.SetBusyBit(inst_ptr->getPhyRd());

            auto latency = fu_latency_map_->at(inst_ptr->getFuType());
            if (latency > wakeup_latency_ + 1) {
                // speculative wakeup start up
                latency_table_[inst_ptr->getPhyRd()] = latency - wakeup_latency_ - 1;
                process_event.schedule(sparta::Clock::Cycle(0));
            }
        }
    }

    void SpecBusyTableUnit::WakeupResolve_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        if (!is_spec_wakeup_) {
            return;
        }
        for (auto inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRdType() != RegType_t::NONE) {
                busy_table_.ClearBusyBit(inst_ptr->getPhyRd());
            }
        }
    }

    void SpecBusyTableUnit::UpdateBusy_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        if (!is_spec_wakeup_) {
            return;
        }
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRdType() != RegType_t::NONE) {
                busy_table_.ClearBusyBit(inst_ptr->getPhyRd());
            }
        }
    }

    void SpecBusyTableUnit::Process_() {
        sparta_assert(is_spec_wakeup_, "speculative wakeup");
        for (auto it = latency_table_.begin(); it != latency_table_.end(); ) {
            if (it->second - 1 <= 1) {
                it = latency_table_.erase(it);
            } else {
                --it->second;
                ++it;
            }
        }
        ILOG("latency table tick");


        if (!latency_table_.empty()) {
            process_event.schedule(1);
        }
    }

    void SpecBusyTableUnit::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        busy_table_.Flush();
    }
}