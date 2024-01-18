#include "Agu.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* AGU::name = "AGU";

    AGU::AGU(sparta::TreeNode* node, const AGUParameter* p) :
        sparta::Unit(node),
        agu_latency_(p->agu_latency),
        agu_queue_("agu_queue", p->agu_queue_size, node->getClock(), &unit_stat_set_),
        agu_queue_size_(p->agu_queue_size)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AGU, AllocateInst_, InstPtr));

        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AGU, SendInitCredit));
        std::cout << "create AGU" << std::endl;
    }

    void AGU::AllocateInst_(const InstPtr& inst_ptr) {
        // Receive insts to agu_queue_
        ILOG("AGU get inst from BE: " << inst_ptr);
        AguEntryPtr entry {new AguEntry};
        entry->inst_ptr = inst_ptr;
        entry->Addr_status = AddrStatus_t::WAIT;
        agu_queue_.push(entry);

        uev_start_agu_.schedule(sparta::Clock::Cycle(1));
    }

    //this function is to select first addr waiting calculation entry to do addr calculation.
    void AGU::TriggerAgu() {
        if (agu_queue_.empty()) {
            return;
        }
        
        for (auto& entry : agu_queue_) {
            if (entry->Addr_status == AddrStatus_t::WAIT) {
                entry->Addr_status = AddrStatus_t::CALCU;
                uev_handle_agu_.schedule(sparta::Clock::Cycle(agu_latency_));
                break;
            }
        }
        
        uev_start_agu_.schedule(sparta::Clock::Cycle(1));
    }

    void AGU::handleAgu() {
        for (auto& entry : agu_queue_) {
            if (entry->Addr_status == AddrStatus_t::CALCU) {
                entry->Addr_status = AddrStatus_t::DONE;
                ILOG("Finish calculate addr, then send insn to LSQ: " << entry->inst_ptr);
                agu_lsq_inst_out.send(entry->inst_ptr);
                func_rs_credit_out.send(1);
                agu_queue_.pop();
                break;
            }
        }
    }

    void AGU::SendInitCredit() {
        func_rs_credit_out.send(agu_queue_size_);
        ILOG("AGU initial credits for BE: " << agu_queue_size_);
    }
}
