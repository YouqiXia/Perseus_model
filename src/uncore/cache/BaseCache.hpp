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
                PARAMETER(uint32_t, cacheline_size, 64, "cache line size")
                PARAMETER(uint32_t, way_num, 4, "way number")
                PARAMETER(uint32_t, cache_size, 128*1024, "cache size")
                PARAMETER(uint32_t, mshr_size, 1, "MSHR size")
                PARAMETER(bool, is_perfect_cache, false, "perfect cache that always hit")
                PARAMETER(uint32_t, perfect_cache_latency, 3, "perfect cache that always hit")
                PARAMETER(uint32_t, upstream_access_ports_num, 1, "upstream access ports number")
                PARAMETER(uint32_t, downstream_access_ports_num, 1, "downstream access ports number")
            };
            static const char name[];

            BaseCache(sparta::TreeNode* node, const BaseCacheParameterSet* p);

        
        private:
            //Functional interface
            virtual void access(const MemAccInfoGroup& req){};

            virtual void funcaccess(const MemAccInfoGroup& req);

            virtual void sendCredit();

            virtual void sendResp(const MemAccInfoGroup& resp);

            void sendResp(const MemAccInfoGroup& resps, uint32_t latency);

            virtual void sendRequest(const MemAccInfoGroup& req);

            virtual void recvResp(const MemAccInfoGroup& resp);

            virtual void recvCredit(const Credit& in);

            virtual void InReqAbitor();

            virtual void OutReqAbitor();

            virtual setTags& accessTagRam(const MemAccInfoPtr& req);

            virtual setData& accessDataRam(const MemAccInfoPtr& req);

            void makeResp(const MemAccInfoPtr& req, MemAccInfoPtr& resp);

            virtual uint32_t allocMshr(const MemAccInfoPtr& req);

            virtual void handle_mshr();

            virtual void deallocMshr(uint32_t id){ return mshr.deallocate(id);};

            void mshrSendRequest();

            virtual void mshrRefill();

            virtual void mshrSendResp();

            virtual void mshrEvict();

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
            sparta::UniqueEvent<> ev_handle_mshr{&unit_event_set_, "ev_handle_mshr", 
                    CREATE_SPARTA_HANDLER(BaseCache, handle_mshr)};

            
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
            uint32_t perfect_cache_latency_;

            Credit next_level_credit;
            uint32_t upstream_access_ports_num_;
            uint32_t downstream_access_ports_num_;

            std::vector<setTags> tagram;
            std::vector<setData> dataram;
            MSHR mshr;

            MemAccInfoAllocator& mem_acc_info_allocator_;

            sparta::Counter cache_hits_{getStatisticSet(), "cache_hits",
                    "Number of cache hits", sparta::Counter::COUNT_NORMAL};

            sparta::Counter cache_misses_{getStatisticSet(), "cache_misses",
                    "Number of cache misses", sparta::Counter::COUNT_NORMAL};
    };

} // namespace TimingModel