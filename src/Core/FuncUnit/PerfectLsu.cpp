#include "PerfectLsu.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectLsu::name = "perfect_lsu";

    PerfectLsu::PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p) :
        sparta::Unit(node),
        func_type_(p->func_type),
        load_to_use_latency_(p->load_to_use_latency)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, WriteBack_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");
    }

    void PerfectLsu::WriteBack_(const InstPtr& inst_ptr) {
        FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
        if (inst_ptr->getFuType() == FuncType::LDU) {
            ILOG(getName() << " get load inst: " << inst_ptr->getRobTag());
            func_following_finish_out.send(func_inst_ptr, load_to_use_latency_ - 1);
        } else if (inst_ptr->getFuType() == FuncType::STU) {
            ILOG(getName() << " get store inst: " << inst_ptr->getRobTag());
            func_following_finish_out.send(func_inst_ptr, load_to_use_latency_ - 1);
        }
    }

}