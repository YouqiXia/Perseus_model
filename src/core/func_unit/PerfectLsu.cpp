#include "PerfectLsu.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectLsu::name = "perfect_lsu";

    PerfectLsu::PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p) :
        sparta::Unit(node),
        lsu_width_(p->lsu_width),
        credit_(p->load_to_use_latency * p->lsu_width),
        load_to_use_latency_(p->load_to_use_latency),
        ld_queue_size_(p->ld_queue_size),
        st_queue_size_(p->st_queue_size),
        lsu_queue_("lsu_queue", p->ld_queue_size + p->st_queue_size, node->getClock(), &unit_stat_set_)
    {
        lsu_flush_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, HandleFlush_, FlushingCriteria));

        preceding_func_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, RecieveInst_, InstPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        write_back_func_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, AcceptCredit_, Credit));

        Rob_lsu_wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, RobWakeUp, InstPtr));

        renaming_lsu_allocate_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, AllocateInst_, InstGroupPtr));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectLsu, SendInitCredit));

    }

    void PerfectLsu::SendInitCredit() {
        lsu_renaming_ldq_credit_out.send(ld_queue_size_);
        ILOG("Ldq initial credits for BE: " << ld_queue_size_);

        lsu_renaming_stq_credit_out.send(st_queue_size_);
        ILOG("Stq initial credits for BE: " << st_queue_size_);

        func_rs_credit_out.send(ld_queue_size_ + st_queue_size_);
    }

    void PerfectLsu::HandleFlush_(const TimingModel::FlushingCriteria &flush_criteria) {
        ILOG(getName() << " is flushed.");
        credit_ = lsu_width_ * load_to_use_latency_;
        lsu_renaming_ldq_credit_out.send(ld_queue_size_);
        lsu_renaming_stq_credit_out.send(st_queue_size_);
        func_rs_credit_out.send(ld_queue_size_ + st_queue_size_);
        lsu_queue_.clear();
    }

    void PerfectLsu::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst_ptr : *inst_group_ptr) {
            ILOG("lsq allocation: " << *inst_ptr);
        }
        write_back_event.schedule(0);
    }

    void PerfectLsu::RecieveInst_(const InstPtr& inst_ptr) {
        lsu_queue_.push(inst_ptr);
        write_back_event.schedule(0);
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
        uint32_t produce_num = std::min(lsu_width_, credit_);

        if (lsu_queue_.empty()) {
            return;
        }
        while(produce_num--) {
            if (lsu_queue_.empty()) {
                break;
            }
            auto inst_ptr = lsu_queue_.front();
            FuncInstPtr func_inst_ptr {new FuncInst{getName(), inst_ptr}};
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ILOG(getName() << " get load inst: " << inst_ptr->getRobTag());
                func_following_finish_out.send(func_inst_ptr, load_to_use_latency_ - 1);
                lsu_renaming_ldq_credit_out.send(1);
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                ILOG(getName() << " get store inst: " << inst_ptr->getRobTag());
                func_following_finish_out.send(func_inst_ptr, load_to_use_latency_ - 1);
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