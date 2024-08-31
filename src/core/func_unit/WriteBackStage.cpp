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
            is_wb_perfect_(p->is_perfect_mode),
            inst_queue_()
    {
        writeback_flush_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(WriteBackStage, HandleFlush_, FlushingCriteria));
        write_back_insts_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                                                    (WriteBackStage, AcceptFuncInsts_, InstGroupPtr));
        FuMap& fu_map = getFuMap();
        for (auto &func_pair: fu_map) {
            auto *func_unit_write_back_in_tmp = new sparta::DataInPort<FuncInstPtr>
                    {&unit_port_set_, func_pair + "_write_back_port_in", sparta::SchedulingPhase::Tick, wb_latency_};
            auto *func_unit_credit_out_tmp = new sparta::DataOutPort<Credit>
                    {&unit_port_set_,  func_pair + "_rs_credit_out"};
            func_unit_write_back_ports_in_[func_pair] = func_unit_write_back_in_tmp;
            func_unit_credit_ports_out_[func_pair] = func_unit_credit_out_tmp;
            func_unit_write_back_in_tmp->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                                                                 (WriteBackStage, AcceptFuncInst_, FuncInstPtr));
        }

    }

    void WriteBackStage::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG(getName() << " is flushed");
        func_inst_map_.clear();
        func_pop_pending_queue_.clear();
    };

    void WriteBackStage::AcceptFuncInst_(const TimingModel::FuncInstPtr &func_inst_ptr) {
        ILOG(func_inst_ptr->func_type << " get instruction");
        func_inst_map_[func_inst_ptr->func_type].emplace_back(func_inst_ptr->inst_ptr);
        arbitrate_inst_event.schedule(0);
    }

    void WriteBackStage::AcceptFuncInsts_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get instructions: " << inst_ptr);
            inst_queue_.push_back(inst_ptr);
        }
        arbitrate_inst_event.schedule(0);
    }


    void WriteBackStage::ArbitrateInst_() {
        uint64_t produce_num = issue_num_;
        InstGroupPtr inst_group_ptr_tmp = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        if (!is_wb_perfect_) {
            for (auto &func_name: getFuMap()) {
                if (!produce_num) {
                    break;
                }
                auto inst_group_queue_itr = func_inst_map_.find(func_name);
                if (func_inst_map_.find(func_name) == func_inst_map_.end()) {
                    continue;
                }
                int i = 1;
                for (const auto &inst_ptr: inst_group_queue_itr->second) {
                    inst_group_ptr_tmp->emplace_back(inst_ptr);
                    ILOG(getName() << " arbitrate instructions rob tag: " << inst_ptr->getRobTag());
                    ILOG(getName() << " arbitrate instructions: " << inst_ptr);
                    if (inst_group_queue_itr->second.size() == i) {
                        func_pop_pending_queue_.emplace_back(func_name);
                    }
                    func_unit_credit_ports_out_.at(func_name)->send(1);
                    produce_num--;
                    i++;
                }
            }

            for (auto &func_type: func_pop_pending_queue_) {
                func_inst_map_.erase(func_type);
            }
            func_pop_pending_queue_.clear();

            if (!inst_group_ptr_tmp->empty()) {
                write_back_following_port_out.send(inst_group_ptr_tmp);
            }

            if (!func_inst_map_.empty()) {
                arbitrate_inst_event.schedule(1);
            }
        } else {
            for (auto& inst_ptr: inst_queue_) {
                inst_group_ptr_tmp->emplace_back(inst_ptr);
                ILOG(getName() << " arbitrate instructions rob tag: " << inst_ptr->getRobTag());
                ILOG(getName() << " arbitrate instructions: " << inst_ptr);
            }

            inst_queue_.clear();
            if (!inst_group_ptr_tmp->empty()) {
                write_back_following_port_out.send(inst_group_ptr_tmp);
            }

            if (!inst_queue_.empty()) {
                arbitrate_inst_event.schedule(1);
            }
        }
    }

}