#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/PayloadEvent.hpp"
#include "MemAccessInfo.hpp"

namespace TimingModel {

    class AbstractMemroy: public sparta::Unit{

        public:
            class AbstractMemroyParameterSet : public sparta::ParameterSet
            {
            public:
                AbstractMemroyParameterSet(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
                { }

                PARAMETER(uint32_t, access_latency, 5, "memory access latency")
                PARAMETER(uint32_t, upstream_access_ports_num, 1, "upstream access ports number")
            };

            AbstractMemroy(sparta::TreeNode* node, const AbstractMemroyParameterSet* p);

            static const char name[];

        private:
        // sparta::SyncInPort<MemAccInfoPtr> mem_req_in
        //     {&unit_port_set_, "mem_req_in", getClock()};
        // sparta::SyncOutPort<MemAccInfoPtr> mem_resp_out
        //     {&unit_port_set_, "mem_resp_out", getClock()};

        sparta::DataInPort<MemAccInfoGroup> mem_req_in
            {&unit_port_set_, "mem_req_in", 1};
        sparta::DataOutPort<MemAccInfoGroup> mem_resp_out
            {&unit_port_set_, "mem_resp_out", 1};
        sparta::DataOutPort<Credit> out_upstream_credit
            {&unit_port_set_, "out_upstream_credit", 1};

        sparta::PayloadEvent<MemAccInfoGroup> ev_handle_mem_req
            {&unit_event_set_, "handle_mem_req", CREATE_SPARTA_HANDLER_WITH_DATA(AbstractMemroy, handle_mem_req, MemAccInfoGroup)};

        void receive_mem_req(const MemAccInfoGroup & req);
        void handle_mem_req(const MemAccInfoGroup & );
        void SendInitCredit();

        uint32_t access_latency_;
        uint32_t upstream_access_ports_num_;
        uint32_t downstream_access_ports_num_;

    };


} //namespace TimingModel