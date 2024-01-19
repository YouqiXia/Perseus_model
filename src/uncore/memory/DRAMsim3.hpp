#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/events/PayloadEvent.hpp"
#include "olympia/OlympiaAllocators.hpp"
#include "MemAccessInfo.hpp"
#include "memory_system.h"

namespace TimingModel {
    using dramsim3::MemorySystem;

    class DRAMsim3: public sparta::Unit{
        public:
            class DRAMsim3ParameterSet : public sparta::ParameterSet{
            public:
                DRAMsim3ParameterSet(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
                {}

                PARAMETER(uint32_t, access_latency, 5, "memory access latency")
                PARAMETER(uint32_t, upstream_access_ports_num, 1, "upstream access ports number")
                PARAMETER(std::string, config_file, "", "DRAM config file")
                PARAMETER(std::string, output_dir, "", "DRAM output dir")
            };

            DRAMsim3(sparta::TreeNode* node, const DRAMsim3ParameterSet* p);
            ~DRAMsim3();

            static const char name[];

        private:
            //Port
            sparta::DataInPort<MemAccInfoGroup> mem_req_in
                {&unit_port_set_, "mem_req_in", 1};
            sparta::DataOutPort<MemAccInfoGroup> mem_resp_out
                {&unit_port_set_, "mem_resp_out", 1};
            sparta::DataOutPort<Credit> out_upstream_credit
                {&unit_port_set_, "out_upstream_credit", 1};

            //Event
            sparta::SingleCycleUniqueEvent<> DRAM_clk_event_
            {&unit_event_set_, "DRAM_clk", CREATE_SPARTA_HANDLER(DRAMsim3, ClockTick)};

            void ClockTick();
            void receive_mem_req(const MemAccInfoGroup & req);
            void SendInitCredit();
            void Read_CallBack(uint64_t Addr);
            void Write_CallBack(uint64_t Addr);

            uint32_t access_latency_;
            uint32_t upstream_access_ports_num_;

        private:
            MemorySystem memory_system;
            std::vector<MemAccInfoPtr> memory_req_queue;
            uint64_t memory_req_queue_num = 8;
            MemAccInfoAllocator& mem_acc_info_allocator_;

            bool last_write = false;
            bool get_next = true;
    };
} //namespace TimingModel