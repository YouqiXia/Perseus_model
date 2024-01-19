#include "L1dCache.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    L1dCache::L1dCache(sparta::TreeNode* node, const L1dCacheParameterSet* p):
        BaseCache(node, p)
    {
    }

    void L1dCache::makeResp(const MemAccInfoPtr& req){
        respQueuePush(req);
        ILOG("send req addr: " << HEX16(req->address));
        outRespArbiterSchedule(1);
    }
    void L1dCache::makeCriticalResp(const MemAccInfoPtr& req){
        respQueuePush(req);
        ILOG("send req addr: " << HEX16(req->address));
        outRespArbiterSchedule(1);
    }
    void L1dCache::makeNonCriticalResp(const MemAccInfoPtr& req){
        ILOG("send no req");
        return;
    }
    void L1dCache::mshrRecvResp(uint32_t id, uint32_t subindex){
        sparta_assert((getMshrEntryStatus(id) == MshrStatus::SEND_REQ)||(getMshrEntryStatus(id) == MshrStatus::RECV_CRITICAL_SEG));
        if(subindex == 0){
            setMshrEntryStatus(id, MshrStatus::RECV_CRITICAL_SEG);
            makeCriticalResp(getMshrEntryReq(id));
        }else if(subindex == getCacheLineSize()/getDownStreamAccPortsBW()-1){
            sparta_assert((getMshrEntryStatus(id) == MshrStatus::RECV_CRITICAL_SEG));
            setMshrEntryStatus(id, MshrStatus::RECV_WHOLE_LINE);
            makeNonCriticalResp(getMshrEntryReq(id));
        }
    }
}//namespace TimingModel