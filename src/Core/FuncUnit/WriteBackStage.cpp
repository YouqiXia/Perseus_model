//
// Created by yzhang on 1/6/24.
//

#include "WriteBackStage.hpp"

namespace TimingModel {

    const char* WriteBackStage::name = "write_back_stage";

    WriteBackStage::WriteBackStage(sparta::TreeNode *node,
                                   const TimingModel::WriteBackStage::WriteBackStageParameter *p) :
            sparta::Unit(node),
            issue_num_(p->issue_num)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(WriteBackStage, SendInitCredit_));
        for (auto &func_pair: getFuncMap()) {
            auto *func_unit_write_back_in_tmp = new sparta::DataInPort<FuncInstPtr>
                    {&unit_port_set_, func_pair.first + "_write_back_port_in", sparta::SchedulingPhase::Tick, 0};
            auto *func_unit_credit_out_tmp = new sparta::DataOutPort<Credit>
                    {&unit_port_set_,  func_pair.first + "_rs_credit_out"};
            func_unit_write_back_ports_in_[func_pair.first] = func_unit_write_back_in_tmp;
            func_unit_credit_ports_out_[func_pair.first] = func_unit_credit_out_tmp;
            func_unit_write_back_in_tmp->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
                                                                 (WriteBackStage, AcceptFuncInst_, FuncInstPtr));
        }
    }

    void WriteBackStage::AcceptFuncInst_(const TimingModel::FuncInstPtr &func_inst_ptr) {
        ILOG(func_inst_ptr->func_type << " get instruction");
        func_inst_map_[func_inst_ptr->func_type] = func_inst_ptr->inst_ptr;
        arbitrate_inst_event.schedule(0);
    }

    void WriteBackStage::SendInitCredit_() {
        for (auto& func_credit_pair: getFuncCredit()) {
            func_unit_credit_ports_out_.at(func_credit_pair.first)->send(func_credit_pair.second);
        }
    }

    void WriteBackStage::ArbitrateInst_() {
        uint64_t produce_num = issue_num_;
        InstGroupPtr inst_group_ptr_tmp = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        for (auto& func_inst_pair: func_inst_map_) {
            if (!produce_num) {
                break;
            }
            inst_group_ptr_tmp->emplace_back(func_inst_pair.second);
            func_unit_credit_ports_out_.at(func_inst_pair.first)->send(1);
            produce_num--;
        }

        if (!inst_group_ptr_tmp->empty()) {
            write_back_following_port_out.send(inst_group_ptr_tmp);
        }

        if (!func_inst_map_.empty()) {
            arbitrate_inst_event.schedule(1);
        }
    }

}