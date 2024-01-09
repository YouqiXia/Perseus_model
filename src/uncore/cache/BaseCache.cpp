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

        MemAccInfoGroup resps;
        for (auto req: reqs){
            setTags& read_setTags = accessTagRam(req);
            setData& read_setData = accessDataRam(req);
            
            uint32_t hit_index = 0;
            bool hit = tagCompare(read_setTags, req->address, hit_index);
            if(hit){
                MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
                makeResp(req, resp);
                resps.emplace_back(resp);
                cache_hits_++;
                ILOG("request hit addr: "<<HEX16(req->address) );
            }else{
                bool mshr_avail = checkMshrAvail();
                sparta_assert((mshr_avail));
                uint32_t id = allocMshr(req);
                cache_misses_++;
                ILOG("request miss addr: "<<HEX16(req->address) << " mshr id: " << id);
            }
            sendCredit();
        }
        if (resps.size() > 0)
            sendResp(resps, cache_latency_);
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
    void BaseCache::makeResp(const MemAccInfoPtr& req, MemAccInfoPtr& resp){
        resp = req;
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

    void BaseCache::handle_mshr(){
        if(!mshr.isEmpty()){
            mshrSendResp();
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
            ILOG("MSHR recv resp mshrid " << resp->mshrid << " for resp addr " << HEX16(resp->address));
            mshr.recvResp(resp->mshrid);
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
            if((mshr.queue[header+i].status == MshrStatus::RECV_RESP)
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
            if(mshr.queue[header+i].status == MshrStatus::RECV_RESP){
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

    void BaseCache::OutReqAbitor(){


    }

    void BaseCache::InReqAbitor(){


    }


    void BaseCache::mshrSendResp(){
        uint32_t header = mshr.header;
        MemAccInfoGroup resps;
        uint32_t cnt = std::min(mshr.used, upstream_access_ports_num_);
        for(int i=0; i<cnt; i++){
            if(mshr.queue[header+i].status == MshrStatus::REFILL){
                resps.emplace_back(mshr.queue[header+i].req);
                mshr.queue[header+i].status = MshrStatus::SEND_RESP;
                ILOG("MSHR entry send response id " << header+i 
                    << " for request addr " << HEX16(mshr.queue[header+i].req->address) 
                    << " and deallocate mshr entry idd: " << (header+i));
                //one entry once
                mshr.deallocate(header+i);
                sendCredit();
            }
        }
        if (resps.size() > 0)
            sendResp(resps, cache_latency_);
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
