#include "DRAMsim3.hpp"
#include "sparta/utils/LogUtils.hpp"
#include "sparta/utils/SpartaAssert.hpp"

namespace TimingModel {
    const char DRAMsim3::name[] = "DRAMsim3";

     DRAMsim3::DRAMsim3(sparta::TreeNode* node, const DRAMsim3ParameterSet* p):
        sparta::Unit(node),
        memory_req_queue(memory_req_queue_num),
        access_latency_(p->access_latency),
        upstream_access_ports_num_(p->upstream_access_ports_num),
        mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator),
        memory_system(
            config_file, output_dir,
            std::bind(&DRAMsim3::Read_CallBack, this, std::placeholders::_1),
            std::bind(&DRAMsim3::Write_CallBack, this, std::placeholders::_1)
        )
    {
        mem_req_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(DRAMsim3, receive_mem_req, MemAccInfoGroup));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(DRAMsim3, SendInitCredit));
    }

    DRAMsim3::~DRAMsim3(){
    }

    void DRAMsim3::SendInitCredit(){
        out_upstream_credit.send(upstream_access_ports_num_);
        ILOG("send " << upstream_access_ports_num_ << " credit to upstream");
    }

    void DRAMsim3::ClockTick(){
        // std::cout << "DRAM clock tick" << std::endl;
        memory_system.ClockTick();
        if (!memory_req_queue.empty())
            DRAM_clk_event_.schedule(1);
    }

    void DRAMsim3::receive_mem_req(const MemAccInfoGroup & reqs)
    {
        // std::cout << "reqs size " << reqs.size() << " ports_num " << upstream_access_ports_num_ << std::endl;
        sparta_assert((reqs.size() <= upstream_access_ports_num_));
        for (auto req : reqs){
            // std::cout << "receive_mem_req addr " << std::hex << req->address << std::endl;

            if(req->mem_op == MemOp::STORE){
                last_write = true;
            } else {
                last_write = false;
            }

            //check if DRAMsim3 can recieve req
            get_next = memory_system.WillAcceptTransaction(req->address, last_write);
            if (get_next && (memory_req_queue.size() < memory_req_queue_num)) {
                memory_system.AddTransaction(req->address, last_write);
                MemAccInfoPtr mem_info = sparta::allocate_sparta_shared_pointer<MemAccInfo>(mem_acc_info_allocator_);
                mem_info->address = req->address;
                memory_req_queue.push_back(mem_info);
            }
        }
        DRAM_clk_event_.schedule(1);
    }

    void DRAMsim3::Read_CallBack(uint64_t Addr){
        // std::cout << "DRAM read callback, memory_req_queue.size " << memory_req_queue.size() << std::endl;
        MemAccInfoGroup DRAM_resp;

        for (auto it = memory_req_queue.begin(); it != memory_req_queue.end(); ++it){
            if ((*it)->address == Addr){
                DRAM_resp.push_back((*it));
                mem_resp_out.send(DRAM_resp);
                memory_req_queue.erase(it);
                break;
            }
        }

        if (memory_req_queue.size() < memory_req_queue_num){
            out_upstream_credit.send(DRAM_resp.size());
        }
        return;
    }

    void DRAMsim3::Write_CallBack(uint64_t Addr){
        // std::cout << "DRAM write callback, memory_req_queue.size " << memory_req_queue.size() << std::endl;
        MemAccInfoGroup DRAM_resp;

        for (auto it = memory_req_queue.begin(); it != memory_req_queue.end(); ++it){
            if ((*it)->address == Addr){
                DRAM_resp.push_back((*it));
                mem_resp_out.send(DRAM_resp);
                memory_req_queue.erase(it);
                break;
            }
        }

        if (memory_req_queue.size() < memory_req_queue_num){
            out_upstream_credit.send(DRAM_resp.size());
        }
        return;
    }

} //namespace TimingModel