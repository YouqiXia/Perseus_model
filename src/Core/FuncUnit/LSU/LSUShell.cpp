#include "LSUShell.hpp"
#include "sparta/utils/LogUtils.hpp"

#include <string>

namespace TimingModel {
    const char* LSUShell::name = "LSUShell";

    LSUShell::LSUShell(sparta::TreeNode* node, const LSUShellParameter* p) :
        sparta::Unit(node),
        issue_width_(p->issue_width)
    {
        preceding_func_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, preceding_func_inst_in_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, write_back_func_credit_in_, Credit));

        func_rs_credit_bp_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, func_rs_credit_bp_in_, Credit));

        renaming_lsu_allocate_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, renaming_lsu_allocate_in_, InstGroupPtr));

        Rob_lsu_wakeup_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, Rob_lsu_wakeup_in_, InstPtr));

        lsu_renaming_allocate_bp_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, lsu_renaming_allocate_bp_in_, InstGroupPtr));

        lsu_renaming_ldq_credit_bp_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, lsu_renaming_ldq_credit_bp_in_, Credit));

        lsu_renaming_stq_credit_bp_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, lsu_renaming_stq_credit_bp_in_, Credit));

        func_following_finish_bp_in.registerConsumerHandler
                (CREATE_SPARTA_HANDLER_WITH_DATA(LSUShell, func_following_finish_bp_in_, InstGroupPtr));

        std::cout << "create lsu shell" << std::endl;

        // for multi-port rs input
        // TODO: multi-port rs input & Credit, multi-port write back output & Credit

        // for multi-port write back
        if (issue_width_ == 1) {
            auto *func_following_finish_out_tmp = new sparta::DataOutPort<FuncInstPtr>
                    {&unit_port_set_,  "func_following_finish_out"};
            func_following_finish_out_ports.emplace_back(func_following_finish_out_tmp);
        } else {
            for (uint32_t i = 0; i < issue_width_; i++) {
                auto *func_following_finish_out_tmp = new sparta::DataOutPort<FuncInstPtr>
                        {&unit_port_set_,  "func_following_finish_out_"+std::to_string(i)};
                func_following_finish_out_ports.emplace_back(func_following_finish_out_tmp);
            }
        }
    }

    void LSUShell::preceding_func_inst_in_(const InstPtr& inst) {
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        inst_group_tmp_ptr->emplace_back(inst);
        preceding_func_inst_bp_out.send(inst_group_tmp_ptr);
    }

    void LSUShell::write_back_func_credit_in_(const Credit& credit) {
        write_back_func_credit_bp_out.send(credit);
    }

    void LSUShell::func_rs_credit_bp_in_(const Credit& credit) {
        func_rs_credit_out.send(credit);
    }

    void LSUShell::renaming_lsu_allocate_in_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        renaming_lsu_allocate_bp_out.send(inst_group_ptr);
    }

    void LSUShell::Rob_lsu_wakeup_in_(const TimingModel::InstPtr &inst_ptr) {
        Rob_lsu_wakeup_bp_out.send(inst_ptr);
    }

    void LSUShell::lsu_renaming_allocate_bp_in_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        lsu_renaming_allocate_out.send(inst_group_ptr);
    }

    void LSUShell::lsu_renaming_ldq_credit_bp_in_(const TimingModel::Credit &credit) {
        lsu_renaming_ldq_credit_out.send(credit);
    }

    void LSUShell::lsu_renaming_stq_credit_bp_in_(const TimingModel::Credit &credit) {
        lsu_renaming_stq_credit_out.send(credit);
    }

    void LSUShell::func_following_finish_bp_in_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        uint32_t i = 0;
        for (auto& inst_ptr: *inst_group_ptr) {
            sparta_assert(func_following_finish_out_ports.size() > i, "lsu bandwidth error");
            FuncInstPtr func_inst_ptr {new FuncInst{getName(), inst_ptr}};
            sparta_assert(inst_ptr != nullptr, "lsu shell write back a nullptr");
            func_following_finish_out_ports[i++]->send(func_inst_ptr);
        }
    }

}