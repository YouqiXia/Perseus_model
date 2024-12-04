//
// Created by yzhang on 1/1/24.
//

#include "DispatchStage.hpp"

namespace TimingModel {
    const char* DispatchStage::name = "dispatch_stage";

    DispatchStage::DispatchStage(sparta::TreeNode *node, const DispatchStageParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_width),
            inst_queue_depth_(p->queue_depth),
            inst_queue_()
    {
        // Startup events
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(DispatchStage, Startup_));

        // normal ports binding
        rs_dispatch_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
           (DispatchStage, AcceptCredit_, CreditPairPtr));

        preceding_dispatch_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, AllocateInst_, InstGroupPtr));

        write_back_dispatch_port_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, FuncUnitBack_, InstGroupPtr));

        dispatch_flush_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (DispatchStage, HandleFlush_, FlushingCriteria));

        // precedence
//        write_back_dispatch_port_in >> sparta::GlobalOrderingPoint(node, "dispatch_busy_update");
//        sparta::GlobalOrderingPoint(node, "dispatch_busy_update") >> process_event;
    }

    void DispatchStage::Startup_() {
        global_param_ptr_ = getGlobalParams(getContainer());
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());

        for(auto dispatch_map_pair: global_param_ptr_->getDispatchMap()) {
            credit_map_[dispatch_map_pair.first] = 0;
        }

        InitCredit_();

        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void DispatchStage::InitCredit_() {
        dispatch_preceding_credit_out.send(inst_queue_depth_, sparta::Clock::Cycle(1));
    }

    void DispatchStage::AllocateInst_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get inst from preceding: " << inst_ptr);
            IssueQueueEntryPtr issue_entry_ptr_tmp {new IssueQueueEntry};
            issue_entry_ptr_tmp->inst_ptr = inst_ptr;
            inst_queue_.push_back(issue_entry_ptr_tmp);
        }

        process_event.schedule(0);
    }

    void DispatchStage::ProcessInst_() {
        PopDispatchQueue_();
        SelectInst_();
        DispatchInsts_();
    }

    void DispatchStage::FuncUnitBack_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& fu_back_inst_ptr: *inst_group_ptr) {
            for (auto& dispatch_queue_inst_ptr: inst_queue_) {
                if (dispatch_queue_inst_ptr->is_issued) {
                    continue;
                }

                if (dispatch_queue_inst_ptr->inst_ptr->getPhyRs1() == fu_back_inst_ptr->getPhyRd()) {
                    dispatch_queue_inst_ptr->inst_ptr->setIsRs1Forward(false);
                }

                if (dispatch_queue_inst_ptr->inst_ptr->getPhyRs2() == fu_back_inst_ptr->getPhyRd()) {
                    dispatch_queue_inst_ptr->inst_ptr->setIsRs2Forward(false);
                }
            }
        }
    }

    void DispatchStage::SelectInst_() {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "event", 1);

        uint64_t produce_num = issue_num_;
        uint64_t produced_num = 0;

        for (auto &func_pair: global_param_ptr_->getDispatchMap()) {
            uint32_t issue_width_per_pipe = global_param_ptr_->getDispatchIssueWidthMap().at(func_pair.first);
            size_t size_ = 0;
            for (auto &issue_entry_ptr: inst_queue_) {
                if (func_pair.second.find(issue_entry_ptr->inst_ptr->getFuType()) != func_pair.second.end()) {
                    size_++;
                }
            }
            if (size_ < issue_width_per_pipe) {
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) + " loss", issue_width_per_pipe - size_);
            }
            if (size_ == 0) {
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) +" queue empty", 1);
                continue;
            }

            uint64_t produce_max_per_pipe = std::min<uint64_t>(size_, issue_width_per_pipe);
            if (credit_map_.at(func_pair.first) < produce_max_per_pipe) {
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) +" rs loss", produce_max_per_pipe - credit_map_.at(func_pair.first));
            }
            if (credit_map_.at(func_pair.first) == 0) {
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) +" rs full", 1);
            }
            if (!credit_map_.at(func_pair.first)) {
                continue;
            }

            for (auto &issue_entry_ptr: inst_queue_) {
                if (!credit_map_.at(func_pair.first)) {
                    break;
                }

                if (!issue_width_per_pipe) {
                    break;
                }

                if (issue_entry_ptr->is_issued) {
                    continue;
                }

                if (func_pair.second.find(issue_entry_ptr->inst_ptr->getFuType()) == func_pair.second.end()) {
                    continue;
                }

                ILOG(getName() << " Instruction Select: " << issue_entry_ptr->inst_ptr);
                --produce_num;
                // sparta assert needed here
                
                ++produced_num;
                --issue_width_per_pipe;
                issue_entry_ptr->inst_ptr->setPipeRank(func_pair.first);
                dispatch_pending_queue_[func_pair.first].emplace_back(issue_entry_ptr->inst_ptr);
                --credit_map_.at(func_pair.first);
                issue_entry_ptr->is_issued = true;
            }
        }
        pmu_->Monitor(getName(), "total loss", issue_num_-produced_num);

        if (produced_num) {
            dispatch_preceding_credit_out.send(produced_num, sparta::Clock::Cycle(1));
        }
    }

    void DispatchStage::HandleFlush_(const TimingModel::FlushingCriteria &flushing_criteria) {
        ILOG(name << "is flushed.");

        for (auto& credit_pair: credit_map_) {
            credit_pair.second = 0;
        }
        dispatch_preceding_credit_out.send(inst_queue_depth_, sparta::Clock::Cycle(1));
        inst_queue_.clear();
    }

    void DispatchStage::DispatchInsts_() {
        for (auto& dispatch_pending_pair: dispatch_pending_queue_) {
            InstGroupPairPtr inst_group_tmp_ptr =
                    sparta::allocate_sparta_shared_pointer<InstGroupPair>(*allocator_->inst_group_pair_allocator);
            for (auto& inst_ptr: dispatch_pending_pair.second) {
                ILOG("issue inst to following: " << inst_ptr);
                inst_group_tmp_ptr->inst_group.emplace_back(inst_ptr);
                inst_group_tmp_ptr->pipe_rank = dispatch_pending_pair.first;
            }
            dispatch_rs_inst_out.send(inst_group_tmp_ptr);
        }
        dispatch_pending_queue_.clear();

        if (!inst_queue_.empty()) {
            process_event.schedule(1);
        }

        ILOG(getName() << " queue size is after update: " << inst_queue_.size());
    }

    void DispatchStage::PopDispatchQueue_() {
        ILOG(getName() << " try to pop instructions.");
        uint64_t issue_queue_pop_size = 0;
        for (auto& issue_queue_entry_ptr: inst_queue_) {
            if (issue_queue_entry_ptr->is_issued) {
                inst_queue_.pop_front();
                ++issue_queue_pop_size;
            } else {
                break;
            }
        }
    }

    void DispatchStage::AcceptCredit_(const TimingModel::CreditPairPtr &credit_pair_ptr) {
        credit_map_.at(credit_pair_ptr->pipe_rank) += credit_pair_ptr->credit;
        ILOG("accept credits from " << credit_pair_ptr->pipe_rank << " , credits is " << credit_pair_ptr->credit <<
            " updated credits is: " << credit_map_.at(credit_pair_ptr->pipe_rank));
    }

    void DispatchStage::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        pmu_->Monitor(getName(), "pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "total loss", issue_num_);

        if (inst_queue_.empty()) {
            for (auto &func_pair: global_param_ptr_->getDispatchMap()) {
                uint32_t issue_width_per_pipe = global_param_ptr_->getDispatchIssueWidthMap().at(func_pair.first);
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) +" queue loss", issue_width_per_pipe);
                pmu_->Monitor(getName(), "scheduler " + std::to_string(func_pair.first) +" queue empty", 1);
            }
        }
    }

}