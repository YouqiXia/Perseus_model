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
        };

        static const char* name;

        AbstractLsu(sparta::TreeNode* node, const AbstractLsuParameter* p);

        void handleDispatch(const InstGroup&);

        void SendInitCredit();

        void RobWakeUp();

        void handleAgu();

        void WriteBack(const InstGroup&);

        // Issue/Re-issue ready instructions in the issue queue
        void issueInst();

        void sendInsts(const InstGroup& inst_group);

        bool isReadyToIssueInsts() const;

        void handleCacheResp(const MemAccInfoGroup& resps);

        void acceptCredit(const Credit& credit);
    public:
        // ports
        // sparta::DataOutPort<Credit> rob_wakeup_stq_idx_in
        //     {&unit_port_set_, "rob_wakeup_stq_idx_in"};
        
        //ports to be
        sparta::DataInPort<InstGroup> backend_lsu_inst_in
            {&unit_port_set_, "backend_lsu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> backend_lsu_credit_out
            {&unit_port_set_, "backend_lsu_credit_out"};

        sparta::DataOutPort<RobIdx> lsu_backend_finish_out
            {&unit_port_set_, "lsu_backend_finish_out"};

        //ports to cache
        sparta::DataOutPort<MemAccInfoGroup> lsu_l1d_cache_out
            {&unit_port_set_, "lsu_l1d_cache_out"};
        sparta::DataInPort<MemAccInfoGroup> l1d_cache_lsu_in
            {&unit_port_set_, "l1d_cache_lsu_in"};
        sparta::DataInPort<Credit> l1d_cache_lsu_credit_in
            {&unit_port_set_, "l1d_cache_lsu_credit_in", 1};

        // sparta::DataOutPort<InstGroup> lsu_backend_wr_data_out
        //     {&unit_port_set_, "lsu_backend_wr_data_out"};

        // Event
        // To issue instruction
        sparta::UniqueEvent<> uev_issue_inst_{&unit_event_set_, "issue_inst",
                CREATE_SPARTA_HANDLER(AbstractLsu, issueInst)};

        MemAccInfoAllocator& abstract_lsu_mem_acc_info_allocator_;
    private:
        const uint64_t load_to_use_latency_;
        uint64_t cache_access_ports_num_;

        // inst queue
        using LoadStoreIssueQueue = sparta::Buffer<InstPtr>;
        LoadStoreIssueQueue issue_queue_;
        const uint32_t issue_queue_size_;

        // ld and st queue
        LoopQueue<InstPtr> ld_queue_;
        LoopQueue<InstPtr> st_queue_;
        // cache credit
        Credit cache_credit_;
    };

}
