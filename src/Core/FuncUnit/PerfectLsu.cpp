#include "PerfectLsu.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectLsu::name = "perfect_lsu";

    PerfectLsu::PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p) :
        sparta::Unit(node),
        func_type_(p->func_type),
        load_to_use_latency_(p->load_to_use_latency),
        store_latency_(p->store_latency),
        ld_queue_size_(p->ld_queue_size),
        st_queue_size_(p->st_queue_size),
        lsu_queue_("lsu_queue", p->ld_queue_size + p->st_queue_size, node->getClock(), &unit_stat_set_)
    {
        preceding_func_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, RecieveInst_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, AcceptCredit_, Credit));

        Rob_lsu_wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, RobWakeUp, InstPtr));

        renaming_lsu_allocate_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, AllocateInst_, InstGroupPtr));

        std::cout << "create lsu from perfect lsu" << std::endl;
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectLsu, SendInitCredit));

    }

    void PerfectLsu::SendInitCredit() {
        lsu_renaming_ldq_credit_out.send(ld_queue_size_);
        ILOG("Ldq initial credits for BE: " << ld_queue_size_);

        lsu_renaming_stq_credit_out.send(st_queue_size_);
        ILOG("Stq initial credits for BE: " << st_queue_size_);

        func_rs_credit_out.send(ld_queue_size_ + st_queue_size_);
    }

    void PerfectLsu::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr : *inst_group_ptr) {
            ILOG("lsq allocation: " << *inst_ptr);
        }
    }

    void PerfectLsu::RecieveInst_(const InstPtr& inst_ptr) {
        lsu_queue_.push(inst_ptr);
        write_back_event.schedule(1);
    }

    void PerfectLsu::RobWakeUp(const InstPtr& inst) {
        ILOG("store insn get rob wakeup: " << inst);
        
    }

    void PerfectLsu::AcceptCredit_(const Credit& credit) {
        credit_ += credit;

        ILOG(getName() << "accept credits: " << credit);
        write_back_event.schedule(0);
    }

    void PerfectLsu::WriteBack_() {
        if (lsu_queue_.empty()) {
            return;
        }
        if (credit_) {
            auto inst_ptr = lsu_queue_.front();
            FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ILOG(getName() << " get load inst: " << inst_ptr->getRobTag());
                func_following_finish_out.send(func_inst_ptr, load_to_use_latency_);
                lsu_renaming_ldq_credit_out.send(1);
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                ILOG(getName() << " get store inst: " << inst_ptr->getRobTag());
                func_following_finish_out.send(func_inst_ptr, store_latency_);
                lsu_renaming_stq_credit_out.send(1);
            }
            lsu_queue_.pop();
            func_rs_credit_out.send(1);
            --credit_;
        }

        if (!lsu_queue_.empty() && credit_) {
            write_back_event.schedule(1);
        }
    }
}