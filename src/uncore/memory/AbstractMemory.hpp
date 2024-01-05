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
            };

            AbstractMemroy(sparta::TreeNode* node, const AbstractMemroyParameterSet* p);

            static const char name[];

        private:
        // sparta::SyncInPort<MemAccInfoPtr> mem_req_in
        //     {&unit_port_set_, "mem_req_in", getClock()};
        // sparta::SyncOutPort<MemAccInfoPtr> mem_resp_out
        //     {&unit_port_set_, "mem_resp_out", getClock()};

        sparta::DataInPort<MemAccInfoPtr> mem_req_in
            {&unit_port_set_, "mem_req_in", 1};
        sparta::DataOutPort<MemAccInfoPtr> mem_resp_out
            {&unit_port_set_, "mem_resp_out", 1};
        sparta::DataOutPort<Credit> out_uplevel_credit
            {&unit_port_set_, "out_uplevel_credit", 1};

        sparta::PayloadEvent<MemAccInfoPtr> ev_handle_mem_req
            {&unit_event_set_, "handle_mem_req", CREATE_SPARTA_HANDLER_WITH_DATA(AbstractMemroy, handle_mem_req, MemAccInfoPtr)};

        void receive_mem_req(const MemAccInfoPtr & req);
        void handle_mem_req(const MemAccInfoPtr & );
        void SendInitCredit();

        uint32_t access_latency_;

    };


} //namespace TimingModel