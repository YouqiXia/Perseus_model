#include "AbstractMemory.hpp"
#include "sparta/utils/LogUtils.hpp"

namespace TimingModel {
    const char AbstractMemroy::name[] = "abstract_memory";

    AbstractMemroy::AbstractMemroy(sparta::TreeNode *node, const AbstractMemroyParameterSet *p) :
        sparta::Unit(node),
        access_latency_(p->access_latency),
        upstream_access_ports_num_(p->upstream_access_ports_num)
    {
        mem_req_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(AbstractMemroy, receive_mem_req, MemAccInfoGroup));
        // mem_req_in.setPortDelay(static_cast<sparta::Clock::Cycle>(1));
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AbstractMemroy, SendInitCredit));
    }
    void AbstractMemroy::SendInitCredit() {
        out_upstream_credit.send(upstream_access_ports_num_);
        ILOG("send " << upstream_access_ports_num_ << " credit to upstream");
    }

    void AbstractMemroy::receive_mem_req(const MemAccInfoGroup & reqs)
    {
        sparta_assert((reqs.size() <= upstream_access_ports_num_));
        ev_handle_mem_req.preparePayload(reqs)->schedule(access_latency_);
        for (auto req : reqs)
            ILOG("access mem addr: "<< HEX16(req->address));
    }

    void AbstractMemroy::handle_mem_req(const MemAccInfoGroup & reqs)
    {
        sparta_assert((reqs.size() <= upstream_access_ports_num_));
        mem_resp_out.send(reqs);
        out_upstream_credit.send(reqs.size());
        for (auto resp : reqs)
            ILOG("send resp to upstream, resp addr "<< HEX16(resp->address)); 
    }

} //namespace TimingModel