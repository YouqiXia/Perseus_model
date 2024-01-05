#pragma once
 
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/ports/SyncPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/ports/DataPort.hpp"
#include "MemAccessInfo.hpp"
#include "basic/Inst.hpp"
#include "olympia/OlympiaAllocators.hpp"
#include <stdlib.h>


namespace TimingModel {
    class TagEntry;
    class BaseCache;

    using setTags = std::vector<TagEntry>;
    using wayData = std::vector<uint8_t>;
    using setData = std::vector<wayData>;

    class TagEntry{
    friend class BaseCache;
    protected:
        uint64_t tag;
        uint64_t access_time;
        bool valid;

    public:
        TagEntry(){
            tag = 0;
            access_time = 0;
            valid = 0;
        }
        uint64_t getTag(){
            return tag;
        }
        bool isValid(){
            return valid;
        }
    };

    enum class MshrStatus: std::uint8_t {
        NO_TYPE = 0,
        __FIRST = NO_TYPE,
        ALLOCATE,
        SEND_REQ,
        RECV_RESP,
        REFILL,
        SEND_RESP,
        NUM_TYPES,
        __LAST = NUM_TYPES
    };
    class MSHREntry{
    public:
        uint64_t addr;
        MemAccInfoPtr req;
        MshrStatus status;
        bool valid;
        MSHREntry(){
            addr = 0;
            status = MshrStatus::NO_TYPE;
            valid = false;
        }
    };

    class MSHR{
    public:
        std::vector<MSHREntry> queue;
        uint32_t qsize;
        uint32_t used;
        uint32_t header;
        uint32_t tail;
    public:
        void resize(uint32_t size){
            qsize = size;
            queue.resize(qsize);
            used = 0;
            header = 0;
            tail = 0;
        }
        bool isAvail(){
            return (used != qsize);
        }

        uint32_t allocate(){
            sparta_assert((qsize != used));
            uint32_t allocated = tail;
            used++;
            tail += 1;
            if(tail == qsize)
                tail = 0;
            return allocated;
        }
        void deallocate(uint32_t id){
            sparta_assert((id < qsize));
            used--;
            header += 1;
            if(header == qsize)
                header = 0;
            return;
        }
        MSHREntry& getHeader(){
            return queue[header];
        }

        void recvResp(uint32_t id){
            queue[id].status = MshrStatus::RECV_RESP; 
        }
        bool isEmpty(){ return (used == 0); }

    };

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
            };
            static const char name[];

            BaseCache(sparta::TreeNode* node, const BaseCacheParameterSet* p);

        
        private:
            //Functional interface
            virtual void access(const MemAccInfoPtr& req){};

            virtual void funcaccess(const MemAccInfoPtr& req);

            virtual void sendCredit();

            virtual void sendResp(const MemAccInfoPtr& resp){out_access_resp.send(resp);};

            virtual void sendRequest(const MemAccInfoPtr& req);

            virtual void recvResp(const MemAccInfoPtr& resp);

            virtual void recvCredit(const Credit& in);

            virtual setTags& accessTagRam(const MemAccInfoPtr& req);

            virtual setData& accessDataRam(const MemAccInfoPtr& req);

            // virtual bool isHit(){return true;};
            // virtual MemAccInfoPtr& makeResp(const MemAccInfoPtr& req);

            virtual uint32_t allocMshr(const MemAccInfoPtr& req);

            virtual void handle_mshr();

            virtual void deallocMshr(uint32_t id){ return mshr.deallocate(id);};

            void mshrSendRequest();

            virtual void mshrRefill();

            virtual void mshrSendResp();

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
            sparta::DataInPort<MemAccInfoPtr> in_access_req
                {&unit_port_set_, "in_access_req", 1};
            sparta::DataOutPort<MemAccInfoPtr> out_access_resp
                {&unit_port_set_, "out_access_resp", 1};
            sparta::DataOutPort<Credit> out_uplevel_credit
                {&unit_port_set_, "out_uplevel_credit", 1};

            sparta::DataInPort<Credit> in_lowlevel_credit
                {&unit_port_set_, "in_lowlevel_credit", 1};
            sparta::DataInPort<MemAccInfoPtr> in_access_resp
                {&unit_port_set_, "in_access_resp", 1};
            sparta::DataOutPort<MemAccInfoPtr> out_access_req
                {&unit_port_set_, "out_access_req", 1}; 
            

            //events.
            sparta::UniqueEvent<> ev_handle_mshr{&unit_event_set_, "ev_handle_mshr", CREATE_SPARTA_HANDLER(BaseCache, handle_mshr)};
            //sparta::UniqueEvent<> ev_handle_mshr{&unit_event_set_, "ev_handle_mshr", CREATE_SPARTA_HANDLER(BaseCache, handle_mshr)};

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

            std::vector<setTags> tagram;
            std::vector<setData> dataram;
            MSHR mshr;

            MemAccInfoAllocator& mem_acc_info_allocator_;

    };

} // namespace TimingModel