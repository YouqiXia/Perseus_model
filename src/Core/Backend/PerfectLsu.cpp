#include "PerfectLsu.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectLsu::name = "perfect_lsu";

    PerfectLsu::PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p) :
        sparta::Unit(node),
        load_to_use_latency_(p->load_to_use_latency),
        mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator),
        next_level_credit(0),
        is_perfect_lsu_(p->is_perfect_lsu)
    {
        // backend_lsu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, WriteBack, InstGroup));
        backend_lsu_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectLsu, SendInitCredit));
        if(is_perfect_lsu_){
            backend_lsu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, WriteBack, InstGroup));
        }else{
            backend_lsu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, recvReq, InstGroup));
        }
        in_lowlevel_credit.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, recvCredit, Credit));
        in_access_resp.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, recvResp, MemAccInfoPtr));
    }

    void PerfectLsu::WriteBack(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ILOG("lsu get load inst: " << inst_ptr->getPC());
                lsu_backend_finish_out.send(inst_ptr->getRobTag(), load_to_use_latency_ - 1);
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                ILOG("lsu get store inst: " << inst_ptr->getPC());
                lsu_backend_finish_out.send(inst_ptr->getRobTag());
            }
        }

        backend_lsu_credit_out.send(inst_group.size());
    }

    void PerfectLsu::SendInitCredit() {
        backend_lsu_credit_out.send(2);
    }

    void PerfectLsu::recvReq(const InstGroup& inst_group){
        for (InstPtr inst_ptr: inst_group) {
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ILOG("lsu get load inst: " << inst_ptr->getPC());
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                ILOG("lsu get store inst: " << inst_ptr->getPC());
            }
            MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
            req->insn = inst_ptr;
            out_access_req.send(req);
        }
    }
    void PerfectLsu::recvResp(const MemAccInfoPtr& resp){
        InstPtr inst_ptr = resp->insn;
        lsu_backend_finish_out.send(inst_ptr->getRobTag());
        backend_lsu_credit_out.send(1);
    }
}