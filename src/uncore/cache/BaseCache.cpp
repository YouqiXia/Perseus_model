#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/utils/LogUtils.hpp"
#include "sparta/kernel/SpartaHandler.hpp"
#include "BaseCache.hpp"
#include <cmath>

namespace TimingModel {
    const char BaseCache::name[] = "l1d_cache";

    BaseCache::BaseCache(sparta::TreeNode* node, const BaseCacheParameterSet* p):
        sparta::Unit(node),
        cacheline_size_(p->cacheline_size),
        way_num_(p->way_num),
        cache_size_(p->cache_size),
        mshr_size_(p->mshr_size),
        perfect_cache_(p->is_perfect_cache),
        perfect_cache_latency_(p->perfect_cache_latency-1),
        next_level_credit(0),
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
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, funcaccess, MemAccInfoPtr));
        in_lowlevel_credit.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, recvCredit, Credit));
        in_access_resp.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(BaseCache, recvResp, MemAccInfoPtr));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(BaseCache, SendInitCredit));
        
    }
    void BaseCache::SendInitCredit() {
        out_uplevel_credit.send(1);
        ILOG("send 1 credit to uplevel");
    }
    void BaseCache::funcaccess(const MemAccInfoPtr& req){
        ILOG("request hit addr: "<<HEX16(req->address));
        if(perfect_cache_){
            out_access_resp.send(req, perfect_cache_latency_);
            return;
        }

        setTags& read_setTags = accessTagRam(req);
        setData& read_setData = accessDataRam(req);

        uint32_t hit_index = 0;
        bool hit = tagCompare(read_setTags, req->address, hit_index);
        if(hit){
            // MemAccInfoPtr& resp = makeResp(req);
            sendResp(req);
            ILOG("request hit addr: "<<HEX16(req->address) );
        }else{
            bool mshr_avail = checkMshrAvail();
            sparta_assert((mshr_avail));
            allocMshr(req);
            ILOG("request hit addr: "<<HEX16(req->address) );
        }
        sendCredit();
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
    // MemAccInfoPtr& BaseCache::makeResp(const MemAccInfoPtr& req){
    //     //return req;
    // }
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
            out_uplevel_credit.send(1);
            ILOG("send 1 credit to uplevel");
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
            if(next_level_credit > 0)
                mshrSendRequest();
            mshrRefill();
            mshrSendResp();
        }

        if(!mshr.isEmpty()){
            ev_handle_mshr.schedule(1);
        }
    }
    void BaseCache::mshrSendRequest(){
        sparta_assert((!mshr.isEmpty()));
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if(mshr.queue[header+i].status == MshrStatus::ALLOCATE){
                sendRequest(mshr.queue[header+i].req);
                mshr.queue[header+i].status = MshrStatus::SEND_REQ;
                ILOG("MSHR entry send request id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                break;
            }
        }
    }
    void BaseCache::recvResp(const MemAccInfoPtr& resp){
        mshr.recvResp(resp->mshrid);
        ev_handle_mshr.schedule(1);
    }
    void BaseCache::sendRequest(const MemAccInfoPtr& req){
        out_access_req.send(req);
    }
    void BaseCache::recvCredit(const Credit& in){
        next_level_credit += in;
        ILOG("receive " << in <<"credit from lowlevel");
    }
    void BaseCache::mshrRefill(){
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if(mshr.queue[header+i].status == MshrStatus::RECV_RESP){
                uint64_t addr = mshr.queue[header+i].req->address;
                uint32_t way_id = 0;
                uint32_t index = getIndex(addr);
                sparta_assert((index<set_num_));
                bool need_evict = checkWayAvail(index, way_id);
                if(need_evict){
                    way_id = replacementCal();
                    evict(index, way_id);
                    ramRefill(mshr.queue[header+i].req, way_id);
                    mshr.queue[header+i].status = MshrStatus::REFILL;
                    ILOG("MSHR entry evict and refile id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                }else{
                    ramRefill(mshr.queue[header+i].req, way_id);
                    mshr.queue[header+i].status = MshrStatus::REFILL;
                    ILOG("MSHR entry refill id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                }
                break;
            }
        }
    }

    void BaseCache::mshrSendResp(){
        uint32_t header = mshr.header;
        for(int i=0; i<mshr.used; i++){
            if(mshr.queue[header+i].status == MshrStatus::REFILL){
                sendResp(mshr.queue[header+i].req);
                mshr.queue[header+i].status = MshrStatus::SEND_RESP;
                ILOG("MSHR entry send response id " << header+i << " for request addr " << HEX16(mshr.queue[header+i].req->address));
                //one entry once
                mshr.deallocate(header+i);
                sendCredit();
                break;
            }
        }
    }
    bool BaseCache::checkWayAvail(uint32_t index, uint32_t way_avail){
        for(int wi=0; wi<way_num_; wi++){
            if(tagram[index][wi].isValid()){
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
        MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
        uint64_t tag = tagram[index][way_id].tag;
        req->address = tag & (way_id << cacheline_size_bits) & (tag_mask|index_mask);
        req->length = cacheline_size_;
        req->mem_op = MemOp::STORE;
        out_access_req.send(req);
    }
} //namespace TimingModel