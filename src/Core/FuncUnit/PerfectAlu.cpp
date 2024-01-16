#include "PerfectAlu.hpp"

#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectAlu::name = "perfect_alu";

    PerfectAlu::PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p) :
        sparta::Unit(node),
        func_type_(p->func_type)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, WriteBack_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, AcceptCredit_, Credit));
    }

    void PerfectAlu::AcceptCredit_(const Credit& credit) {
        func_rs_credit_out.send(credit);
    }

    void PerfectAlu::WriteBack_(const InstPtr& inst_ptr) {
        ILOG(getName() << " get inst and writeback: " << inst_ptr);
        Process_(inst_ptr);
        FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
        func_following_finish_out.send(func_inst_ptr);
    }

    void PerfectAlu::Process_(const InstPtr& inst_ptr) {}
}