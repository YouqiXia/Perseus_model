#include "PerfectAlu.hpp"

#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectAlu::name = "perfect_alu";

    PerfectAlu::PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p) :
        sparta::Unit(node)
    {
        backend_alu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, WriteBack, InstGroup));
        backend_alu_inst_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectAlu, SendInitCredit));
    }

    void PerfectAlu::WriteBack(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            ILOG("alu get inst: " << inst_ptr->getPC());
            alu_backend_finish_out.send(inst_ptr->getRobTag());
        }

        backend_alu_credit_out.send(inst_group.size());
    }

    void PerfectAlu::SendInitCredit() {
        backend_alu_credit_out.send(2);
    }
}