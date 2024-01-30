#include "PerfectAlu.hpp"

#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectAlu::name = "perfect_alu";

    PerfectAlu::PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p) :
        sparta::Unit(node),
        alu_width_(p->alu_width),
        credit_(p->alu_width),
        alu_queue_("alu_queue", p->alu_width ,node->getClock(), &unit_stat_set_)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, Allocate_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, AcceptCredit_, Credit));
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectAlu, SendInitCredit_));
    }

    void PerfectAlu::SendInitCredit_() {
        func_rs_credit_out.send(alu_width_);
    }

    void PerfectAlu::AcceptCredit_(const Credit& credit) {
        credit_ += credit;
        write_back_event.schedule(0);
    }

    void PerfectAlu::Allocate_(const TimingModel::InstPtr &inst_ptr) {
        alu_queue_.push(inst_ptr);
        write_back_event.schedule(0);
    }

    void PerfectAlu::WriteBack_() {
        if (!credit_ || alu_queue_.empty()) {
            return;
        }
        auto inst_ptr = alu_queue_.front();
        ILOG(getName() << " get inst and writeback: " << inst_ptr);
        Process_(inst_ptr);
        FuncInstPtr func_inst_ptr {new FuncInst{getName(), inst_ptr}};
        func_following_finish_out.send(func_inst_ptr);
        alu_queue_.pop();
        --credit_;
        func_rs_credit_out.send(1);
    }

    void PerfectAlu::Process_(const InstPtr& inst_ptr) {}
}