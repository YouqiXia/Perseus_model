#include "Agu.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* AGU::name = "AGU";

    AGU::AGU(sparta::TreeNode* node, const AGUParameter* p) :
        sparta::Unit(node),
        agu_queue_("agu_queue", p->agu_queue_size, node->getClock(), &unit_stat_set_),
        agu_queue_size_(p->agu_queue_size)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AGU, AllocateInst_, InstGroupPtr));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AGU, SendInitCredit));
        std::cout << "create AGU" << std::endl;
    }

    void AGU::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        // Receive insts to agu_queue_
        ILOG("AGU get inst from BE: " << inst_group_ptr);
        sparta_assert(inst_group_ptr->size() != 0, "agu get empty input");
        for (auto& inst_ptr: *inst_group_ptr) {
            agu_queue_.push(inst_ptr);
        }
        uev_handle_agu_.schedule(sparta::Clock::Cycle(0));
    }

    void AGU::handleAgu() {
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        for (auto& inst_ptr : agu_queue_) {
            ILOG("Finish calculate addr, then send insn to LSQ: " << inst_ptr);
            inst_group_tmp_ptr->emplace_back(inst_ptr);
            agu_queue_.pop();
        }
        agu_lsq_inst_out.send(inst_group_tmp_ptr);
        func_rs_credit_out.send(inst_group_tmp_ptr->size());
    }

    void AGU::SendInitCredit() {
        func_rs_credit_out.send(agu_queue_size_);
        ILOG("AGU initial credits for BE: " << agu_queue_size_);
    }
}
