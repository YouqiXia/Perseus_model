#include "PerfectAlu.hpp"

#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectAlu::name = "perfect_alu";

    PerfectAlu::PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p) :
        sparta::Unit(node),
        alu_width_(p->alu_width),
        credit_(p->alu_width),
        alu_queue_()
    {
        preceding_func_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, Allocate_, InstPtr));
        preceding_func_insts_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, AllocateInsts_, InstGroupPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");
        preceding_func_insts_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");
        func_flush_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, HandleFlush_, FlushingCriteria));

        write_back_func_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, AcceptCredit_, Credit));
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectAlu, SendInitCredit_));
    }

    void PerfectAlu::SendInitCredit_() {
        func_rs_credit_out.send(alu_width_);
    }

    void PerfectAlu::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG(getName() << " is flushed");

        credit_ = alu_width_;
        func_rs_credit_out.send(alu_width_);
        alu_queue_.clear();
    }

    void PerfectAlu::AcceptCredit_(const Credit& credit) {
        credit_ += credit;
        write_back_event.schedule(0);
    }

    void PerfectAlu::Allocate_(const TimingModel::InstPtr &inst_ptr) {
        ILOG("get instruction: " << inst_ptr);
        alu_queue_.push_back(inst_ptr);
        write_back_event.schedule(0);
    }

    void PerfectAlu::AllocateInsts_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            ILOG("get instruction: " << inst_ptr);
            alu_queue_.push_back(inst_ptr);
        }
        write_back_event.schedule(0);
    }

    void PerfectAlu::WriteBack_() {
        if (alu_width_ == 1) {
            if (!credit_ || alu_queue_.empty()) {
                return;
            }

            auto inst_ptr = alu_queue_.front();
            ILOG(getName() << " get inst and writeback: " << inst_ptr);
            Process_(inst_ptr);

            if (inst_ptr->getFuType() == FuncType::BRU) {
                func_branch_resolve_inst_out.send(inst_ptr);
            }

            FuncInstPtr func_inst_ptr {new FuncInst{getName(), inst_ptr}};
            func_following_finish_out.send(func_inst_ptr);
            alu_queue_.pop_front();
            --credit_;
            func_rs_credit_out.send(1);
        } else {
            InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
            for (int i = 0; i < alu_width_; i++) {
                if (alu_queue_.empty()) {
                    break;
                }
                inst_group_tmp_ptr->emplace_back(alu_queue_.front());
                ILOG("write back instruction: " << alu_queue_.front());
                alu_queue_.pop_front();
            }
            if (!inst_group_tmp_ptr->empty()) {
                func_following_finishs_out.send(inst_group_tmp_ptr);
                func_rs_credit_out.send(inst_group_tmp_ptr->size());
            }

        }
    }

    void PerfectAlu::Process_(const InstPtr& inst_ptr) {}
}