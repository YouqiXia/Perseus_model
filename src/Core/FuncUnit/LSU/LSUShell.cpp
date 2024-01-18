#include "LSUShell.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* LSUShell::name = "LSUShell";

    LSUShell::LSUShell(sparta::TreeNode* node, const LSUShellParameter* p) :
        sparta::Unit(node),
        func_type_(p->func_type)
    {
        preceding_func_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, ByPassPrecedingInst_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, AcceptCredit_, Credit));

        func_rs_credit_bp_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, ByPassAcceptCredit_, Credit));

        func_following_finish_bp_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, WriteBack_, InstPtr));

        std::cout << "create lsu shell" << std::endl;

    }

    void LSUShell::ByPassPrecedingInst_(const InstPtr& inst) {
        ILOG("get preceding insn and bypass to following: " << inst);
        preceding_func_inst_bp_out.send(inst);
    }

    void LSUShell::AcceptCredit_(const Credit& credit) {
        write_back_func_credit_bp_out.send(credit);
    }

    void LSUShell::ByPassAcceptCredit_(const Credit& credit) {
        func_rs_credit_out.send(credit);
    }

    void LSUShell::WriteBack_(const InstPtr& inst_ptr) {
        FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
        if (inst_ptr->getFuType() == FuncType::LDU) {
            ILOG(getName() << " get load inst RobTag: " << inst_ptr->getRobTag());
            ILOG(getName() << " get load inst: " << inst_ptr);
            func_following_finish_out.send(func_inst_ptr);
        } else if (inst_ptr->getFuType() == FuncType::STU) {
            ILOG(getName() << " get store inst RobTag: " << inst_ptr->getRobTag());
            ILOG(getName() << " get store inst: " << inst_ptr);
            func_following_finish_out.send(func_inst_ptr);
        }
    }

}