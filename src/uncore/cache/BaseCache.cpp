#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/utils/LogUtils.hpp"
#include "sparta/kernel/SpartaHandler.hpp"
#include "BaseCache.hpp"
#include <cmath>

namespace TimingModel {
    const char BaseCache::name[] = "base_cache";

    BaseCache::BaseCache(sparta::TreeNode* node, const BaseCacheParameterSet* p):
        sparta::Unit(node),
        init_credits_(p->init_credits),
        cacheline_size_(p->cacheline_size),
        way_num_(p->way_num),
        cache_size_(p->cache_size),
        mshr_size_(p->mshr_size),
        perfect_cache_(p->is_perfect_cache),
        cache_latency_(p->cache_latency),
        next_level_credit(0),
        upstream_access_ports_num_(p->upstream_access_ports_num),
        downstream_access_ports_num_(p->downstream_access_ports_num),
        upstream_access_ports_bandwidth_(p->upstream_access_ports_bandwidth),
        downstream_access_ports_bandwidth_(p->downstream_access_ports_bandwidth),
        cache_level_(p->cache_level),
        upstream_is_core_(p->upstream_is_core),
        downstream_is_mem_(p->downstream_is_mem),
        mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator)
    {
        set_num_ = cache_size_/cacheline_size_/way_num_;
        cacheline_size_bits = static_cast<int>(std::log2(cacheline_size_));
        index_bits = static_cast<int>(std::log2(set_num_));
        tag_mask = ~((1 << (cacheline_size_bits+index_bits)) -1);
        index_mask = ((1 << (index_bits))-1) << cacheline_size_bits;
        ILOG("tag_mask: " << HEX16(tag_mask) << " index_mask: " << HEX16(index_mask) );
        sparta_assert((std::pow(2, cacheline_size_bits) == cacheline_size_),
                        "cacheline_size is not power of 2!");
        //tagram init
        tagram.resize(set_num_);
        for(int si = 0; si < set_num_; si++){
            tagram[si].resize(way_num_);
        }
        //dataram init
        dataram.resize(set_num_);
        for(int si = 0; si < set_num_; si++){
            dataram[si].resize(way_num_);
            for(int wi = 0; wi < way_num_; wi++){
                dataram[si][wi].resize(cacheline_size_, 0);
            }
        }
        //mshr init
        mshr.resize(mshr_size_);
        //
        out_resp_queue.resize(0);
        in_req_queue.resize(0);
        out_req_queue.resize(0);

        //input request
        in_access_req.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, funcaccess, MemAccInfoGroup));
        in_downstream_credit.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, recvCredit, Credit));
        in_access_resp.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, recvResp, MemAccInfoGroup));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(BaseCache, SendInitCredit));
        
    }
    void BaseCache::SendInitCredit() {
        out_upstream_credit.send(init_credits_);
        ILOG("send 1 credit to upstream");
    }
    void BaseCache::funcaccess(const MemAccInfoGroup& reqs){
        sparta_assert((reqs.size()<=upstream_access_ports_num_));
        if(perfect_cache_){
            sendResp(reqs, cache_latency_);
            out_upstream_credit.send(upstream_access_ports_num_, cache_latency_);

            ILOG(" send" << upstream_access_ports_num_ <<" credits to upstream");
            for(auto req: reqs){
                ILOG("request hit addr: " << HEX16(req->address));
            }
            return;
        }

        for (auto req: reqs){
            setTags& read_setTags = accessTagRam(req);
            setData& read_setData = accessDataRam(req);
            
            uint32_t hit_index = 0;
            bool hit = tagCompare(read_setTags, req->address, hit_index);
            if(hit){
                MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
                makeResp(req);
                cache_hits_++;
                sendCredit();
                ILOG("request hit addr: "<<HEX16(req->address) );
                ev_out_resp_arbiter.schedule(1);
            }else{
                bool mshr_avail = checkMshrAvail();
                sparta_assert((mshr_avail));
                uint32_t id = allocMshr(req);
                cache_misses_++;
                ILOG("request miss addr: "<<HEX16(req->address) << " mshr id: " << id);
            }
        }
    }

    setTags& BaseCache::accessTagRam(const MemAccInfoPtr& req){
        uint64_t addr = req->address;
        uint32_t index = getIndex(addr);
        sparta_assert((index<set_num_), "index " << index << " set_num_ " << set_num_);
        ILOG("access tag ram addr: " << HEX16(addr) << " index " << index);
        return tagram[index];
    }

    setData& BaseCache::accessDataRam(const MemAccInfoPtr& req){
        uint64_t addr = req->address;
        uint32_t index = getIndex(addr);
        sparta_assert((index<set_num_));
        ILOG("access data ram addr: " << HEX16(addr) << " index " << index);
        return dataram[index];
    }

    void BaseCache::makeResp(const MemAccInfoPtr& req){
        if(upstream_is_core_){
            out_resp_queue.emplace_back(req);
        }else{
            uint32_t cnt = cacheline_size_/upstream_access_ports_bandwidth_;

            for(auto i=0u; i<cnt; i++){
                MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
                *resp = *req;
                resp->subindex = i;
                out_resp_queue.emplace_back(resp);
            }
        }
        ev_out_resp_arbiter.schedule(1);
    }

    void BaseCache::makeCriticalResp(const MemAccInfoPtr& req){
        if(upstream_is_core_){
            out_resp_queue.emplace_back(req);
            ILOG("upstream_is_core_ is true and send req addr: " << HEX16(req->address));
        }else{
            MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
            *resp = *req;
            resp->subindex = 0;
            out_resp_queue.emplace_back(resp);
            ILOG("upstream_is_core_ is false and send req addr: " << HEX16(req->address));
        }
        ev_out_resp_arbiter.schedule(1);
    }
    void BaseCache::makeNonCriticalResp(const MemAccInfoPtr& req){
        if(upstream_is_core_){
            ILOG("upstream_is_core_ is true and send no req");
            return;
        }else{
            uint32_t cnt = cacheline_size_/upstream_access_ports_bandwidth_;
            
            for(auto i=1u; i<cnt; i++){
                MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
                *resp = *req;
                resp->subindex = i;
                out_resp_queue.emplace_back(resp);
                ILOG("upstream_is_core_ is false and send req addr: " << HEX16(req->address));
            }
        }
        ev_out_resp_arbiter.schedule(1);
    }
    bool BaseCache::tagCompare(std::vector<TagEntry>& settag, uint64_t access_addr, uint32_t& index){
        sparta_assert((way_num_ == settag.size()));
        uint64_t tag = getTag(access_addr);
        
        for(int i=0; i<way_num_; i++){
            if(settag[i].valid && settag[i].tag == tag ){
                index = i;
                ILOG("compare tag match, tag: " << HEX16(tag));
                return true;
            }
        }
        ILOG("compare tag nomatch, tag: " << HEX16(tag));
        return false;
    }

    wayData& BaseCache::DataMux(setData& setdata, uint32_t way){
        sparta_assert((way_num_ == setdata.size()));
        sparta_assert((way_num_ > way));
        return setdata[way];
    }

    void BaseCache::sendCredit(){
        if(checkMshrAvail()){
            out_upstream_credit.send(1);
            ILOG("send 1 credit to upstream");
        }
    }

    uint32_t BaseCache::allocMshr(const MemAccInfoPtr& req){
        uint32_t id = mshr.allocate();
        mshr.queue[id].addr = req->address;
        mshr.queue[id].req = req;
        mshr.queue[id].req->mshrid = id;
        mshr.queue[id].status = MshrStatus::ALLOCATE;
        mshr.queue[id].valid = true;
        ev_handle_mshr.schedule(1);
        ILOG("allocate MSHR entry id " << id << " for request addr " << HEX16(req->address));
        return id;
    };

    void BaseCache::handleMshr(){
        if(!mshr.isEmpty()){
            mshrDeallocate();
        }
        if(!mshr.isEmpty()){
            mshrRefill();
            mshrEvict();
            if(next_level_credit > 0){
                mshrSendRequest();
            }
        }

        if(!mshr.isEmpty()){
            ev_handle_mshr.schedule(1);
        }
    }
    void BaseCache::mshrSendRequest(){
        sparta_assert((!mshr.isEmpty()));
        uint32_t header = mshr.header;
        uint32_t cnt = std::min(mshr.used, downstream_access_ports_num_);
        MemAccInfoGroup reqs;
        for(int i=0; i<cnt; i++){
            if(mshr.queue[header+i].status == MshrStatus::ALLOCATE){
                reqs.emplace_back(mshr.queue[header+i].req);
                mshr.queue[header+i].status = MshrStatus::SEND_REQ;
                ILOG("MSHR entry send request id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                break;
            }
        }
        if (reqs.size() > 0)
            sendRequest(reqs); 
    }
    void BaseCache::recvResp(const MemAccInfoGroup& resps){
        sparta_assert((resps.size() <= downstream_access_ports_num_));
        for (auto resp : resps){
            ILOG("MSHR recv resp mshrid " << resp->mshrid 
                << " subindex "<< resp->subindex 
                << " for resp addr " << HEX16(resp->address));
            mshrRecvResp(resp->mshrid, resp->subindex);
        }
        ev_handle_mshr.schedule(1);
    }
    
    void BaseCache::sendResp(const MemAccInfoGroup& resps){
        sparta_assert((resps.size() <= upstream_access_ports_num_));
        out_access_resp.send(resps);
        ILOG("send resp to upstream, resp cnt: " << resps.size());
    }
    void BaseCache::sendResp(const MemAccInfoGroup& resps, uint32_t latency){
        sparta_assert((resps.size() <= upstream_access_ports_num_));
        out_access_resp.send(resps, latency);
        ILOG("send resp to upstream, resp cnt: " << resps.size());
    }
    void BaseCache::sendRequest(const MemAccInfoGroup& reqs){
        sparta_assert((reqs.size() <= downstream_access_ports_num_));
        sparta_assert((reqs.size() <= next_level_credit));
        out_access_req.send(reqs);
        next_level_credit--;
    }
    void BaseCache::recvCredit(const Credit& in){
        next_level_credit += in;
        ILOG("receive " << in <<" credit from downstream now next_level_credit is: " << next_level_credit);
    }
    void BaseCache::mshrRefill(){
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if((mshr.queue[header+i].status == MshrStatus::RECV_WHOLE_LINE)
            ||(mshr.queue[header+i].status == MshrStatus::EVICT)){
                uint64_t addr = mshr.queue[header+i].req->address;
                uint32_t way_id = 0;
                uint32_t index = getIndex(addr);
                sparta_assert((index<set_num_));
                bool no_need_evict = checkWayAvail(index, way_id);
                if(no_need_evict){
                    ramRefill(mshr.queue[header+i].req, way_id);
                    mshr.queue[header+i].status = MshrStatus::REFILL;
                    ILOG("MSHR entry refill id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                }
                break;
            }
        }
    }

    void BaseCache::mshrEvict(){
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if(mshr.queue[header+i].status == MshrStatus::RECV_WHOLE_LINE){
                uint64_t addr = mshr.queue[header+i].req->address;
                uint32_t way_id = 0;
                uint32_t index = getIndex(addr);
                sparta_assert((index<set_num_));
                bool no_need_evict = checkWayAvail(index, way_id);
                if(!no_need_evict){
                    way_id = replacementCal();
                    evict(index, way_id);
                    mshr.queue[header+i].status = MshrStatus::EVICT;
                    ILOG("MSHR entry evict mshrid " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                }
            }
        }
    }

    void BaseCache::mshrRecvResp(uint32_t id, uint32_t subindex){
        sparta_assert((mshr.queue[id].status == MshrStatus::SEND_REQ)||(mshr.queue[id].status == MshrStatus::RECV_CRITICAL_SEG));
        if(downstream_is_mem_){
            mshr.queue[id].status = MshrStatus::RECV_WHOLE_LINE;
            makeResp(mshr.queue[id].req);
            ILOG("downstream_is_mem_ true and makeResp all");
            return;
        }
        if(subindex == 0){
            mshr.queue[id].status = MshrStatus::RECV_CRITICAL_SEG;
            makeCriticalResp(mshr.queue[id].req);
        }else if(subindex == cacheline_size_/downstream_access_ports_bandwidth_-1){
            sparta_assert((mshr.queue[id].status == MshrStatus::RECV_CRITICAL_SEG));
            mshr.queue[id].status = MshrStatus::RECV_WHOLE_LINE;
            makeNonCriticalResp(mshr.queue[id].req);
        }
    }


    void BaseCache::outReqArbiter(){


    }

    void BaseCache::inReqArbiter(){


    }
    void BaseCache::outRespArbiter(){
        uint32_t queue_size = out_resp_queue.size();
        if(queue_size == 0)
            return;

        uint32_t send_cnt = std::min(upstream_access_ports_num_, queue_size);
        MemAccInfoGroup resps;
        for(auto i = 0u; i<send_cnt; i++){
            resps.emplace_back(out_resp_queue[i]);
        }
        for(auto i = 0u; i<send_cnt; i++){
            out_resp_queue.erase(out_resp_queue.begin());
        }
        if (resps.size() > 0)
            sendResp(resps, cache_latency_);
        
        if((queue_size-send_cnt) > 0)
            ev_out_resp_arbiter.schedule(1);
    }

    void BaseCache::mshrDeallocate(){
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if(mshr.queue[header+i].status == MshrStatus::REFILL){
                ILOG("MSHR entry deallocatemshr entry id: " << (header+i) 
                    << " request addr " << HEX16(mshr.queue[header+i].req->address));
                mshr.deallocate(header+i);
                sendCredit();
            }
        }
    }
    bool BaseCache::checkWayAvail(uint32_t index, uint32_t way_avail){
        for(int wi=0; wi<way_num_; wi++){
            if(!tagram[index][wi].isValid()){
                way_avail = wi;
                return true;
            }
        }
        return false;
    }
    void BaseCache::ramRefill(const MemAccInfoPtr& req, uint32_t way_id){
        uint32_t index = getIndex(req->address);
        sparta_assert((index<set_num_));
        tagram[index][way_id].tag = getTag(req->address);
        tagram[index][way_id].valid = true;
        //todo copy data
        //memcpy(dataram[index][way_id], req->data, cacheline_size_);
    }

    void BaseCache::evict(uint32_t index, uint32_t way_id){
        MemAccInfoGroup reqs;
        MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
        uint64_t tag = tagram[index][way_id].tag;
        req->address = tag & (way_id << cacheline_size_bits) & (tag_mask|index_mask);
        req->length = cacheline_size_;
        req->mem_op = MemOp::STORE;
        reqs.emplace_back(req);
        if(next_level_credit){
            out_access_req.send(reqs);
            next_level_credit--;
        }
		ev_handle_mshr.schedule(1);
    }
} //namespace TimingModel
