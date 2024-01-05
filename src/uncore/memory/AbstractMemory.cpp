#include "AbstractMemory.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char AbstractMemroy::name[] = "abstract_memory";

    AbstractMemroy::AbstractMemroy(sparta::TreeNode *node, const AbstractMemroyParameterSet *p) :
        sparta::Unit(node),
        access_latency_(p->access_latency)
    {
        mem_req_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(AbstractMemroy, receive_mem_req, MemAccInfoPtr));
        // mem_req_in.setPortDelay(static_cast<sparta::Clock::Cycle>(1));
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AbstractMemroy, SendInitCredit));
        
    }
    void AbstractMemroy::SendInitCredit() {
        out_uplevel_credit.send(1);
        ILOG("send 1 credit to uplevel");
    }

    void AbstractMemroy::receive_mem_req(const MemAccInfoPtr & req)
    {
        ev_handle_mem_req.preparePayload(req)->schedule(access_latency_);
        ILOG("access mem addr: "<< HEX16(req->address));
    }

    void AbstractMemroy::handle_mem_req(const MemAccInfoPtr & req)
    {
        mem_resp_out.send(req); 
        ILOG("send resp");
    }


} //namespace TimingModel