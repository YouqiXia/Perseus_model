#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "olympia/OlympiaAllocators.hpp"

#include "basic/Inst.hpp"
#include "LoopQueue.hh"

#include "sparta/resources/Buffer.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"
namespace TimingModel {
    class AbstractLsu : public sparta::Unit {
    public:
        class AbstractLsuParameter : public sparta::ParameterSet {
        public:
            AbstractLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
            PARAMETER(uint64_t, issue_queue_size, 4, "issue queue size")
            PARAMETER(uint64_t, ld_queue_size, 3, "load queue size")
            PARAMETER(uint64_t, st_queue_size, 3, "store queue size")
            PARAMETER(uint64_t, cache_access_ports_num, 1, "cache access ports number")
            PARAMETER(Credit, agu_num, 2, "agu number")
            PARAMETER(uint64_t, agu_latency, 0, "agu calculate latency")
        };

        static const char* name;

        AbstractLsu(sparta::TreeNode* node, const AbstractLsuParameter* p);

        void handleDispatch(const InstGroup&);

        void SendInitCredit();

        void RobWakeUp(const RobIdx&);

        void handleAgu(const InstGroup&);

        // split insn in issue queue to ldq/stq
        void SplitInst();

        void InOrderIssue();

        void sendInsts(const InstGroup&);

        bool isReadyToSplitInsts();

        void handleCacheResp(const MemAccInfoGroup& resps);

        void acceptCredit(const Credit& credit);

        void LSQ_Dealloc();
    public:
        // ports
        // sparta::DataOutPort<Credit> rob_wakeup_stq_idx_in
        //     {&unit_port_set_, "rob_wakeup_stq_idx_in"};
        
        //ports with be
        sparta::DataInPort<InstGroup> backend_lsu_inst_in
            {&unit_port_set_, "backend_lsu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<RobIdx> backend_lsu_rob_idx_wakeup_in
            {&unit_port_set_, "backend_lsu_rob_idx_wakeup_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroup> lsu_backend_wr_data_out
            {&unit_port_set_, "lsu_backend_wr_data_out"};

        sparta::DataOutPort<Credit> backend_lsu_credit_out
            {&unit_port_set_, "backend_lsu_credit_out"};

        sparta::DataOutPort<InstGroup> lsu_backend_finish_out
            {&unit_port_set_, "lsu_backend_finish_out"};

        //ports with cache
        sparta::DataOutPort<MemAccInfoGroup> lsu_l1d_cache_out
            {&unit_port_set_, "lsu_l1d_cache_out"};
        sparta::DataInPort<MemAccInfoGroup> l1d_cache_lsu_in
            {&unit_port_set_, "l1d_cache_lsu_in"};
        sparta::DataInPort<Credit> l1d_cache_lsu_credit_in
            {&unit_port_set_, "l1d_cache_lsu_credit_in", 1};

        // sparta::DataOutPort<InstGroup> lsu_backend_wr_data_out
        //     {&unit_port_set_, "lsu_backend_wr_data_out"};

        // Event
        // To split instruction
        sparta::UniqueEvent<> uev_split_inst_{&unit_event_set_, "split_inst",
                CREATE_SPARTA_HANDLER(AbstractLsu, SplitInst)};

        sparta::PayloadEvent<InstGroup> ev_agu_
            {&unit_event_set_, "handle_agu", CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, handleAgu, InstGroup)};
        
        // To issue instruction
        sparta::UniqueEvent<> uev_issue_inst_{&unit_event_set_, "issue_inst",
                CREATE_SPARTA_HANDLER(AbstractLsu, InOrderIssue)};

        // To dealloc instruction
        sparta::UniqueEvent<> uev_dealloc_inst_{&unit_event_set_, "dealloc_inst",
                CREATE_SPARTA_HANDLER(AbstractLsu, LSQ_Dealloc)};

        MemAccInfoAllocator& abstract_lsu_mem_acc_info_allocator_;
    private:
        const uint64_t load_to_use_latency_;
        uint64_t cache_access_ports_num_;

        // inst queue
        using LoadStoreIssueQueue = sparta::Buffer<InstPtr>;
        //LoadStoreIssueQueue issue_queue_;
        LoopQueue<InstPtr> issue_queue_;
        const uint32_t issue_queue_size_;

        // ld and st queue
        LoopQueue<InstPtr> ld_queue_;
        LoopQueue<InstPtr> st_queue_;
        // LoadStoreIssueQueue ld_queue_;
        // LoadStoreIssueQueue st_queue_;
        // cache credit
        Credit cache_credit_ = 0;

        const Credit agu_num_;

        const uint64_t agu_latency_;

    };

}
