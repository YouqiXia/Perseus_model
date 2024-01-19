#pragma once
 
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/ports/SyncPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/ports/DataPort.hpp"
#include "MemAccessInfo.hpp"
#include "basic/Inst.hpp"
#include "olympia/OlympiaAllocators.hpp"
#include "Mshr.hpp"
#include "TagEntry.hpp"
#include <stdlib.h>


namespace TimingModel {
    using setData = std::vector<wayData>;

    class BaseCache: public sparta::Unit{
        public:
            class BaseCacheParameterSet : public sparta::ParameterSet
            {
            public:
                BaseCacheParameterSet(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
                { }
                PARAMETER(uint32_t, init_credits, 1, "initial credits value")
                PARAMETER(uint32_t, cacheline_size, 64, "cache line size")
                PARAMETER(uint32_t, way_num, 4, "way number")
                PARAMETER(uint32_t, cache_size, 128*1024, "cache size")
                PARAMETER(uint32_t, mshr_size, 1, "MSHR size")
                PARAMETER(bool, is_perfect_cache, false, "perfect cache that always hit")
                PARAMETER(uint32_t, cache_latency, 1, "cache access latency")
                PARAMETER(uint32_t, upstream_access_ports_num, 1, "upstream access ports number")
                PARAMETER(uint32_t, downstream_access_ports_num, 1, "downstream access ports number")
                PARAMETER(uint32_t, upstream_access_ports_bandwidth, 16, "upstream access ports number")
                PARAMETER(uint32_t, downstream_access_ports_bandwidth, 16, "downstream access ports number")
            };
            static const char name[];

            BaseCache(sparta::TreeNode* node, const BaseCacheParameterSet* p);

        
        public:
	        const uint32_t init_credits_;
            //Functional interface
            virtual void access(const MemAccInfoGroup& req){};

            virtual void funcaccess(const MemAccInfoGroup& req);

            virtual void sendCredit();

            virtual void sendResp(const MemAccInfoGroup& resp);

            virtual void sendResp(const MemAccInfoGroup& resps, uint32_t latency);

            virtual void sendRequest(const MemAccInfoGroup& req);

            virtual void recvResp(const MemAccInfoGroup& resp);

            virtual void recvCredit(const Credit& in);

            virtual void inReqArbiter();

            virtual void outReqArbiter();

            virtual void outRespArbiter();

            virtual setTags& accessTagRam(const MemAccInfoPtr& req);

            virtual setData& accessDataRam(const MemAccInfoPtr& req);

            virtual void makeResp(const MemAccInfoPtr& req);

            virtual void makeCriticalResp(const MemAccInfoPtr& req);

            virtual void makeNonCriticalResp(const MemAccInfoPtr& req);

            virtual uint32_t allocMshr(const MemAccInfoPtr& req);

            virtual void handleMshr();

            virtual void deallocMshr(uint32_t id){ return mshr.deallocate(id);};

            virtual void mshrSendRequest();

            virtual void mshrRefill();

            virtual void mshrDeallocate();

            virtual void mshrEvict();

            virtual void mshrRecvResp(uint32_t id, uint32_t subindex);

            virtual bool checkMshrAvail(){ return mshr.isAvail(); };

            virtual void ramRefill(const MemAccInfoPtr& req, uint32_t way_id);

            virtual void evict(uint32_t index, uint32_t way_id);

            virtual bool checkWayAvail(uint32_t index, uint32_t way_avail);

            virtual uint32_t replacementCal(){ return random() % way_num_;};

            virtual inline uint32_t getIndex(uint64_t addr){ return (addr&index_mask) >> cacheline_size_bits;}

            virtual inline uint64_t getTag(uint64_t addr){ return addr&tag_mask;};

            bool tagCompare(std::vector<TagEntry>& settag, uint64_t access_addr, uint32_t& index);

            wayData& DataMux(setData& setdata, uint32_t way);

            void SendInitCredit();

        public:
            uint32_t getCacheLineSize(){ return cacheline_size_;}
            uint32_t getWayNum(){ return way_num_; }
            uint32_t getSetNum(){ return set_num_; }

            uint32_t getUpStreamAccPortsNum(){ return upstream_access_ports_num_; }
            uint32_t getDownStreamAccPortsNum(){ return downstream_access_ports_num_; }
            uint32_t getUpStreamAccPortsBW(){ return upstream_access_ports_bandwidth_; }
            uint32_t getDownStreamAccPortsBW(){ return downstream_access_ports_bandwidth_; }


            void respQueuePush(const MemAccInfoPtr& resp) { out_resp_queue.emplace_back(resp); }
            void outRespArbiterSchedule(sparta::Clock::Cycle delay) { ev_out_resp_arbiter.schedule(delay); }

            MshrStatus getMshrEntryStatus(uint32_t id);
            void setMshrEntryStatus(uint32_t id, MshrStatus);
            MemAccInfoPtr& getMshrEntryReq(uint32_t id);

        private:
            //Timing
            //ports
            sparta::DataInPort<MemAccInfoGroup> in_access_req
                {&unit_port_set_, "in_access_req", 1};
            sparta::DataOutPort<MemAccInfoGroup> out_access_resp
                {&unit_port_set_, "out_access_resp", 1};
            sparta::DataOutPort<Credit> out_upstream_credit
                {&unit_port_set_, "out_upstream_credit", 1};

            sparta::DataInPort<Credit> in_downstream_credit
                {&unit_port_set_, "in_downstream_credit", 1};
            sparta::DataInPort<MemAccInfoGroup> in_access_resp
                {&unit_port_set_, "in_access_resp", 1};
            sparta::DataOutPort<MemAccInfoGroup> out_access_req
                {&unit_port_set_, "out_access_req", 1}; 
            

            //events.
            sparta::SingleCycleUniqueEvent<> ev_handle_mshr{&unit_event_set_, "ev_handle_mshr", 
                    CREATE_SPARTA_HANDLER(BaseCache, handleMshr)};
            sparta::SingleCycleUniqueEvent<> ev_out_req_arbiter{&unit_event_set_, "ev_out_req_arbiter", 
                    CREATE_SPARTA_HANDLER(BaseCache, outReqArbiter)};
            sparta::SingleCycleUniqueEvent<> ev_in_req_arbiter{&unit_event_set_, "ev_in_req_arbiter", 
                    CREATE_SPARTA_HANDLER(BaseCache, inReqArbiter)};
            sparta::SingleCycleUniqueEvent<> ev_out_resp_arbiter{&unit_event_set_, "ev_out_resp_arbiter", 
                    CREATE_SPARTA_HANDLER(BaseCache, outRespArbiter)};


            uint32_t cacheline_size_;
            uint32_t way_num_;
            uint32_t set_num_;
            uint8_t cacheline_size_bits;
            uint8_t index_bits;
            uint32_t cache_size_;
            uint32_t mshr_size_;
            uint64_t tag_mask;
            uint64_t index_mask;
            bool perfect_cache_;
            uint32_t cache_latency_;

            Credit next_level_credit;
            uint32_t upstream_access_ports_num_;
            uint32_t downstream_access_ports_num_;
            uint32_t upstream_access_ports_bandwidth_;
            uint32_t downstream_access_ports_bandwidth_;


            std::vector<setTags> tagram;
            std::vector<setData> dataram;
            MSHR mshr;

            std::vector<MemAccInfoPtr>  out_resp_queue;
            std::vector<MemAccInfoPtr>  in_req_queue;
            std::vector<MemAccInfoPtr>  out_req_queue;


            sparta::Counter cache_hits_{getStatisticSet(), "cache_hits",
                    "Number of cache hits", sparta::Counter::COUNT_NORMAL};

            sparta::Counter cache_misses_{getStatisticSet(), "cache_misses",
                    "Number of cache misses", sparta::Counter::COUNT_NORMAL};
        public:
            MemAccInfoAllocator& mem_acc_info_allocator_;
    };

} // namespace TimingModel
