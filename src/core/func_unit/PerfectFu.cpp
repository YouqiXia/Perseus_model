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

        // FIXME the correctness of event scheduling need to be detected by unit test
        allocate_event >> write_back_event;
    }

    void PerfectFu::Startup_() {
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());
        fu_latency_map_ = getGlobalParams(getContainer())->getLatencyMap();
        SendInitCredit_();
        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void PerfectFu::SendInitCredit_() {
        func_rs_credit_out.send(alu_depth_, sparta::Clock::Cycle(1));
    }

    void PerfectFu::HandleFlush_(const FlushingCriteria& flush_criteria) {
        ILOG(getName() << " is flushed");

        func_rs_credit_out.send(alu_queue_.size(), sparta::Clock::Cycle(1));
        alu_queue_.clear();
        allocate_event.cancel();
        size_ = 0;
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
                schedule(sparta::Clock::Cycle(fu_latency_map_[inst_ptr->getFuType()] - 1));
            SizeUp();
        }
    }

    void PerfectFu::Allocate_delay_(const TimingModel::InstPtr &inst_ptr) {
        ILOG("get instruction: " << inst_ptr);
        alu_queue_.push_back(inst_ptr);
        write_back_event.schedule(0);
    }

    void PerfectFu::WriteBack_() {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "1 event", 1);

        if (size_ < alu_width_) {
            pmu_->Monitor(getName(), "3 queue loss", alu_width_-size_);
        }
        if (size_ == 0) {
            pmu_->Monitor(getName(), "4 queue empty", 1);
            pmu_->Monitor(getName(), "8 total loss", alu_width_);
            return;
        }

        uint64_t produce_num_max = std::min<uint64_t>(size_, alu_width_);
        if (credit_ < produce_num_max) {
            pmu_->Monitor(getName(), "5 writeback loss", produce_num_max-credit_);
        }

        InstGroupPairPtr inst_group_tmp_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroupPair>(*allocator_->inst_group_pair_allocator);
        inst_group_tmp_ptr->name = getName();
        uint64_t produce_num = std::min(credit_, produce_num_max);
        for (int i = 0; i < alu_width_; i++) {
            if (alu_queue_.empty()) {
                break;
            }

            if (!produce_num) {
                break;
            }

            --credit_;
            --produce_num;
            if (alu_queue_.front()->getFuType() == FuncType::LDU) {
                pmu_->Monitor(getName(), "7 load num", 1);
            }
            inst_group_tmp_ptr->inst_group.emplace_back(alu_queue_.front());
            ILOG("write back: " << alu_queue_.front());
            alu_queue_.pop_front();
            SizeDown();
        }
        pmu_->Monitor(getName(), "6 latency loss", produce_num);
        pmu_->Monitor(getName(), "8 total loss", alu_width_-inst_group_tmp_ptr->inst_group.size());

        if (!inst_group_tmp_ptr->inst_group.empty()) {
            func_following_finish_out.send(inst_group_tmp_ptr);
            ILOG("size after updating: " << alu_queue_.size());
        }

        if (!inst_group_tmp_ptr->inst_group.empty()) {
            func_rs_credit_out.send(inst_group_tmp_ptr->inst_group.size(), sparta::Clock::Cycle(1));
        }

        if (alu_queue_.size() != 0) {
            write_back_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void PerfectFu::Process_(const InstPtr& inst_ptr) {}

    void PerfectFu::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        pmu_->Monitor(getName(), "2 pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "8 total loss", alu_width_);

        if (size_ < alu_width_) {
            pmu_->Monitor(getName(), "3 queue loss", alu_width_-size_);
        }
        if (size_ == 0) {
            pmu_->Monitor(getName(), "4 queue empty", 1);
            return;
        }

        uint64_t produce_num_max = std::min<uint64_t>(size_, alu_width_);
        if (credit_ < produce_num_max) {
            pmu_->Monitor(getName(), "5 writeback loss", produce_num_max-credit_);
        }
        pmu_->Monitor(getName(), "6 latency loss", std::min(produce_num_max, credit_));
    }
}