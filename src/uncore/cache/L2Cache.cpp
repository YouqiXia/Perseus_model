#include "L2Cache.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    L2Cache::L2Cache(sparta::TreeNode* node, const L2CacheParameterSet* p):
        BaseCache(node, p)
    {
    }


    void L2Cache::makeResp(const MemAccInfoPtr& req){

        uint32_t cnt = getCacheLineSize()/getUpStreamAccPortsBW();

        for(auto i=0u; i<cnt; i++){
            MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
            *resp = *req;
            resp->subindex = i;
            respQueuePush(resp);
        }

        outRespArbiterSchedule(1);
    }
    void L2Cache::makeCriticalResp(const MemAccInfoPtr& req){
        MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
        *resp = *req;
        resp->subindex = 0;
        respQueuePush(resp);
        ILOG("send req addr: " << HEX16(req->address));

        outRespArbiterSchedule(1);
    }
    void L2Cache::makeNonCriticalResp(const MemAccInfoPtr& req){
        uint32_t cnt = getCacheLineSize()/getUpStreamAccPortsBW();
        
        for(auto i=1u; i<cnt; i++){
            MemAccInfoPtr resp = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
            *resp = *req;
            resp->subindex = i;
            respQueuePush(resp);
            ILOG("send req addr: " << HEX16(req->address));
        }

        outRespArbiterSchedule(1);
    }
    void L2Cache::mshrRecvResp(uint32_t id, uint32_t subindex){
        sparta_assert((getMshrEntryStatus(id) == MshrStatus::SEND_REQ)||(getMshrEntryStatus(id) == MshrStatus::RECV_CRITICAL_SEG));

        setMshrEntryStatus(id, MshrStatus::RECV_WHOLE_LINE);
        makeResp(getMshrEntryReq(id));
        ILOG("makeResp all");
        return;

    }
}//namespace TimingModel