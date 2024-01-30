#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "olympia/OlympiaAllocators.hpp"

#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"
#include "basic/PortInterface.hpp"
#include "resources/LoopQueue.hpp"

#include "sparta/utils/SpartaSharedPointer.hpp"
namespace TimingModel {
    class LSQ : public sparta::Unit {
    public:
        class LSQParameter : public sparta::ParameterSet {
        public:
            LSQParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, issue_width, 1, "load store unit bandwidth")
            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
            PARAMETER(uint64_t, ld_queue_size, 20, "load queue size")
            PARAMETER(uint64_t, st_queue_size, 20, "store queue size")
            PARAMETER(uint64_t, cache_access_ports_num, 1, "cache access ports number")
        };

        enum Status_t{
            ALLOC,      //renaming->lsq allocate, only ROB enter.
            INSNRDY,    //get insn from agu
            ISSUED,     //is visiting DC
            RESP        //got resp from DC
        };

        struct StoreEntry {
            InstPtr inst_ptr;
            Status_t status;
            RobIdx_t RobTag;
            uint32_t ldq_idx;
            bool order_ready;
            bool  IsWkup = false;
        };

        struct LoadEntry {
            InstPtr inst_ptr;
            Status_t status;
            RobIdx_t RobTag;
            uint32_t stq_idx;
            bool order_ready;
        };

        static const char* name;

        LSQ(sparta::TreeNode* node, const LSQParameter* p);

        void AllocateInst_(const InstGroupPtr&);

        void SendInitCredit();

        void RobWakeUp(const InstPtr&);

        void handleAgu(const InstGroupPtr&);

        void InOrderIssue();

        void sendInsts(const InstGroup&);

        void handleCacheResp(const MemAccInfoGroup& resps);

        void acceptCredit(const Credit& credit);

        void AcceptCredit_(const Credit& credit);

        void LSQDealloc();

        void GetRespWithoutCache(const InstPtr&);
    public:       
        //ports with be
        sparta::DataInPort<InstGroupPtr> renaming_lsu_allocate_in
            {&unit_port_set_, "renaming_lsu_allocate_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataInPort<InstPtr> Rob_lsu_wakeup_in
            {&unit_port_set_, "Rob_lsu_wakeup_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<InstGroupPtr> lsu_renaming_allocate_out
            {&unit_port_set_, "lsu_renaming_allocate_out"};

        sparta::DataOutPort<InstGroupPtr> func_following_finish_out
            {&unit_port_set_, "func_following_finish_out"}; 

        sparta::DataOutPort<Credit> lsu_renaming_ldq_credit_out
            {&unit_port_set_, "lsu_renaming_ldq_credit_out"};

        sparta::DataOutPort<Credit> lsu_renaming_stq_credit_out
            {&unit_port_set_, "lsu_renaming_stq_credit_out"};

        //ports with write backstage
        sparta::DataInPort<Credit> write_back_func_credit_in
            {&unit_port_set_, "write_back_func_credit_in", sparta::SchedulingPhase::Tick, 0};

        //ports with agu
        sparta::DataInPort<InstGroupPtr> agu_lsq_inst_in
            {&unit_port_set_, "agu_lsq_inst_in", sparta::SchedulingPhase::Tick, 1};

        // Event
        // To issue instruction
        sparta::SingleCycleUniqueEvent<> uev_issue_inst_
            {&unit_event_set_, "issue_inst", CREATE_SPARTA_HANDLER(LSQ, InOrderIssue)};

        // To dealloc instruction
        sparta::SingleCycleUniqueEvent<> uev_dealloc_inst_
            {&unit_event_set_, "dealloc_inst", CREATE_SPARTA_HANDLER(LSQ, LSQDealloc)};

        // to get resp without cache
        sparta::PayloadEvent<InstPtr> uev_resp_event
            {&unit_event_set_, "uev_resp_event", CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, GetRespWithoutCache, InstPtr)};

        MemAccInfoAllocator& abstract_lsu_mem_acc_info_allocator_;
    private:
        const uint64_t load_to_use_latency_;
        const uint64_t issue_width_;
        uint64_t cache_access_ports_num_;

        // ld and st queue
        youqixia::resources::LoopQueue<LoadEntry> ld_queue_;
        const uint32_t ld_queue_size_;

        youqixia::resources::LoopQueue<StoreEntry> st_queue_;
        const uint32_t st_queue_size_;
        
        // cache credit
        Credit cache_credit_ = 0;

        Credit wb_credit_ = 0;

    };

}
