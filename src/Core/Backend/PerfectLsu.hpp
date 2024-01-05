#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "olympia/OlympiaAllocators.hpp"
#include "basic/Inst.hpp"


namespace TimingModel {
    class PerfectLsu : public sparta::Unit {
    public:
        class PerfectLsuParameter : public sparta::ParameterSet {
        public:
            PerfectLsuParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, load_to_use_latency, 4, "load to use latency")
            PARAMETER(bool, is_perfect_lsu, true, "load to use latency")
        };

        static const char* name;

        PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p);

        void WriteBack(const InstGroup&);

        void SendInitCredit();

    public:
    // ports
        sparta::DataInPort<InstGroup> backend_lsu_inst_in
            {&unit_port_set_, "backend_lsu_inst_in", sparta::SchedulingPhase::Tick, 1};

        sparta::DataOutPort<Credit> backend_lsu_credit_out
            {&unit_port_set_, "backend_lsu_credit_out"};

        sparta::DataOutPort<RobIdx> lsu_backend_finish_out
            {&unit_port_set_, "lsu_backend_finish_out"};


        sparta::DataInPort<Credit> in_lowlevel_credit
            {&unit_port_set_, "in_lowlevel_credit", 1};
        sparta::DataInPort<MemAccInfoPtr> in_access_resp
            {&unit_port_set_, "in_access_resp", 1};
        sparta::DataOutPort<MemAccInfoPtr> out_access_req
            {&unit_port_set_, "out_access_req"}; 

        MemAccInfoAllocator& mem_acc_info_allocator_;
        Credit next_level_credit;
        bool is_perfect_lsu_;

        void recvReq(const InstGroup&);
        void recvCredit(const Credit& credit);
        void recvResp(const MemAccInfoPtr& resp);
        void sendReq(const MemAccInfoPtr& req){ if(next_level_credit>0) out_access_req.send(req);}


    private:
        const uint64_t load_to_use_latency_;
    };

}