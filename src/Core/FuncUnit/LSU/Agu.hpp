#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"

#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"

#include "basic/Inst.hpp"

namespace TimingModel {
    class AGU : public sparta::Unit {
    public:
        class AGUParameter : public sparta::ParameterSet {
        public:
            AGUParameter(sparta::TreeNode* n) :
                sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, agu_queue_size, 4, "agu queue size equals core issue width")
            PARAMETER(uint64_t, agu_latency, 0, "agu calculate latency")
        };

        enum AddrStatus_t{
            WAIT, CALCU, DONE
        };

        struct AguEntry {
            InstPtr inst_ptr;
            AddrStatus_t Addr_status;
        };

        using AguEntryPtr = sparta::SpartaSharedPointer<AguEntry>;

        static const char* name;

        AGU(sparta::TreeNode* node, const AGUParameter* p);

        void SendInitCredit();

        void handleAgu();

        void AllocateInst_(const InstPtr&);

        void TriggerAgu();

    public:       
        //ports with BE
        sparta::DataInPort<InstPtr> preceding_func_inst_in
            {&unit_port_set_, "preceding_func_inst_in", sparta::SchedulingPhase::Tick, 0};

        sparta::DataOutPort<Credit> func_rs_credit_out
            {&unit_port_set_, "func_rs_credit_out"};

        //ports with LSQ
        sparta::DataOutPort<InstPtr> agu_lsq_inst_out
            {&unit_port_set_, "agu_lsq_inst_out"};

        // Event
        // To split instruction
        sparta::SingleCycleUniqueEvent<> uev_start_agu_
            {&unit_event_set_, "uev_start_agu_", CREATE_SPARTA_HANDLER(AGU, TriggerAgu)};

        sparta::SingleCycleUniqueEvent<> uev_handle_agu_
            {&unit_event_set_, "uev_handle_agu_", CREATE_SPARTA_HANDLER(AGU, handleAgu)};

    private:
        // agu queue
        sparta::Queue<AguEntryPtr> agu_queue_;

        const uint32_t agu_queue_size_ = 0;

        const uint64_t agu_latency_ = 0;

    };

}
