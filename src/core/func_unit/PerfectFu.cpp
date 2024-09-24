#include "PerfectFu.hpp"

#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char* PerfectFu::name = "perfect_fu";

    PerfectFu::PerfectFu(sparta::TreeNode* node, const PerfectFuParameter* p) :
        sparta::Unit(node),
        alu_width_(p->issue_width),
        alu_depth_(p->queue_depth),
        alu_queue_()
    {
        preceding_func_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFu, Allocate_, InstGroupPtr));
        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_alu_multiport_order");
        func_flush_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFu, HandleFlush_, FlushingCriteria));

        preceding_rob_wakeup_store_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFu, DummyFunc_, InstGroupPtr));

        write_back_func_credit_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFu, AcceptCredit_, CreditPairPtr));
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectFu, Startup_));
    }

    void PerfectFu::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        fu_latency_map_ = getGlobalParams(getContainer())->getLatencyMap();
        SendInitCredit_();
    }

    void PerfectFu::SendInitCredit_() {
        func_rs_credit_out.send(alu_depth_, sparta::Clock::Cycle(1));
    }

    void PerfectFu::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG(getName() << " is flushed");

        func_rs_credit_out.send(alu_queue_.size());
        alu_queue_.clear();
    }

    void PerfectFu::AcceptCredit_(const CreditPairPtr& credit_pair_ptr) {
        if (credit_pair_ptr->name != getName()) {
            return;
        }
        credit_ += credit_pair_ptr->credit;
        ILOG(getName() << " accept credits: " << credit_pair_ptr->credit << ", updated credits: " << credit_);
        write_back_event.schedule(0);
    }

    void PerfectFu::Allocate_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            allocate_event.preparePayload(inst_ptr)->
                schedule(sparta::Clock::Cycle(fu_latency_map_[inst_ptr->getFuType()]) - 1);
        }
    }

    void PerfectFu::Allocate_delay_(const TimingModel::InstPtr &inst_ptr) {
        ILOG("get instruction: " << inst_ptr);
        alu_queue_.push_back(inst_ptr);
        write_back_event.schedule(0);
    }

    void PerfectFu::WriteBack_() {
        InstGroupPairPtr inst_group_tmp_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroupPair>(*allocator_->inst_group_pair_allocator);
        inst_group_tmp_ptr->name = getName();
        for (int i = 0; i < alu_width_; i++) {
            if (alu_queue_.empty()) {
                break;
            }
            inst_group_tmp_ptr->inst_group.emplace_back(alu_queue_.front());
            ILOG("write back instruction: " << alu_queue_.front());
            alu_queue_.pop_front();
        }

        if (!inst_group_tmp_ptr->inst_group.empty()) {
            func_following_finish_out.send(inst_group_tmp_ptr);
        }

        if (!inst_group_tmp_ptr->inst_group.empty()) {
            func_rs_credit_out.send(inst_group_tmp_ptr->inst_group.size());
        }
    }

    void PerfectFu::Process_(const InstPtr& inst_ptr) {}
}