#include "AbstractLsu.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* AbstractLsu::name = "abstract_lsu";

    AbstractLsu::AbstractLsu(sparta::TreeNode* node, const AbstractLsuParameter* p) :
        sparta::Unit(node),
        load_to_use_latency_(p->load_to_use_latency),
        cache_access_ports_num_(p->cache_access_ports_num),
        issue_queue_("lsu_inst_queue", p->issue_queue_size, getClock()),
        issue_queue_size_(p->issue_queue_size),
        ld_queue_(p->ld_queue_size),
        st_queue_(p->st_queue_size),
        abstract_lsu_mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator)
    {
        backend_lsu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, handleDispatch, InstGroup));

        backend_lsu_rob_idx_wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, RobWakeUp, RobIdx));

        backend_lsu_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        l1d_cache_lsu_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, handleCacheResp, MemAccInfoGroup));
        l1d_cache_lsu_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, acceptCredit, Credit));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AbstractLsu, SendInitCredit));
    }

    void AbstractLsu::handleDispatch(const InstGroup& inst_group) {
        // Receive insts to issue_queue_
        for (InstPtr inst_ptr: inst_group) {
           ILOG("LSU get inst: " << inst_ptr);
           issue_queue_.push_back(inst_ptr);
        }

        uev_issue_inst_.schedule(sparta::Clock::Cycle(0));
    }

    void AbstractLsu::RobWakeUp(const RobIdx& rob_idx) {
        // wakeup store insn in issue_queue_
        for (auto const &inst_ptr : issue_queue_) {
            if(!inst_ptr->getStoreWkup() && (inst_ptr->getRobTag() == rob_idx)){
                inst_ptr->setStoreWkup(true);
                ILOG("Wake up store insn:" << inst_ptr);
                break;
            }
        }

        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
    }

    void AbstractLsu::handleAgu() {
        // TODO add agu logic
    }

    // Issue/Re-issue ready instructions in the issue queue
    void AbstractLsu::issueInst()
    {
        // Instruction issue arbitration
        // For now, we just pop every inst for queue to ldq & stq
        for (auto const &inst_ptr : issue_queue_) {
            if (!inst_ptr->getLsuIssued()) {
                if (inst_ptr->getFuType() == FuncType::LDU) {
                    if(!ld_queue_.full()) {
                        ld_queue_.Push(inst_ptr);
                        inst_ptr->setLsuIssued(true);

                        ILOG("LSU issue load inst to ld_queue_: Pc[0x" << std::hex << inst_ptr->getPC() << "]");
                    }
                } else if (inst_ptr->getFuType() == FuncType::STU) {
                    if(!inst_ptr->getStoreWkup()){
                        ILOG("Rob NOT wakeup causes fail issue store insn:" << inst_ptr);
                        break;
                    }
                    if(!st_queue_.full()) {
                        st_queue_.Push(inst_ptr);
                        inst_ptr->setLsuIssued(true);

                        ILOG("LSU issue store inst to st_queue_: Pc[0x" << std::hex << inst_ptr->getPC() << "]");
                    }
                }
            }
        }

        // Send request to cache
        // TODO reordered insts from ld_queue_ and st_queue_
        uint64_t cnt = std::min(cache_credit_, cache_access_ports_num_);
        InstGroup insn_grp;
        auto inst_ptr = issue_queue_.begin();
        for(int i=0; i<cnt; i++){
            if (!issue_queue_.empty() && cache_credit_) {
                //send only one be req to cache each cycle
                if ((*inst_ptr)->getLsuIssued()) {
                    if ((*inst_ptr) == ld_queue_[ld_queue_.getHeader()]) {
                        ld_queue_.Pop();
                        insn_grp.emplace_back((*inst_ptr));
                    } else if ((*inst_ptr) == st_queue_[st_queue_.getHeader()]) {
                        st_queue_.Pop();
                        insn_grp.emplace_back((*inst_ptr));
                    } 
                }
                inst_ptr++;
            }
            else{
                sendInsts(insn_grp);
                break;
            }
            sendInsts(insn_grp);
        }

        // Schedule another instruction issue event if possible
        if (isReadyToIssueInsts()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    void AbstractLsu::sendInsts(const InstGroup& inst_group) {
        MemAccInfoGroup reqs;
        for (auto insn :inst_group){
            MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(abstract_lsu_mem_acc_info_allocator_);
            req->insn = insn;
            req->address = insn->getTargetVAddr();
            reqs.emplace_back(req);
            ILOG("abstract lsu access cache: Pc[0x" << std::hex << insn->getPC() << "]");
            cache_credit_--;
        }
        if(reqs.size())
            lsu_l1d_cache_out.send(reqs);
    }

    // Check for ready to issue instructions
    bool AbstractLsu::isReadyToIssueInsts() const
    {
        // Check if ldq and stq is full
        // if (ld_queue_.full() && st_queue_.full()) {
        //     return false;
        // }

        return !issue_queue_.empty();
    }

    void AbstractLsu::acceptCredit(const Credit& credit) {
        cache_credit_ += credit;

        ILOG("abstract lsu get credits from cache: " << credit);

        // Schedule another instruction issue event if possible
        if (isReadyToIssueInsts()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    void AbstractLsu::WriteBack(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            lsu_backend_finish_out.send(inst_ptr->getRobTag());
        }

        backend_lsu_credit_out.send(inst_group.size());
    }

    void AbstractLsu::SendInitCredit() {
        backend_lsu_credit_out.send(issue_queue_size_);
        ILOG("LSU initial credits for BE: " << issue_queue_size_);
    }

    void AbstractLsu::handleCacheResp(const MemAccInfoGroup& resps){
        // std::cout << "AbstractLsu::handleCacheResp, received cache resp." << std::endl;
        ILOG("AbstractLsu::handleCacheResp, received cache resp.");
        for( auto respMemInfoPtr : resps) {
            for (auto iter = issue_queue_.begin(); iter != issue_queue_.end(); iter++) {
                if ((*iter) == respMemInfoPtr->insn) {
                    issue_queue_.erase(iter);

                    InstPtr inst_ptr = respMemInfoPtr->insn;
                    lsu_backend_finish_out.send(inst_ptr->getRobTag());
                    if(inst_ptr->getFuType() == FuncType::LDU){
                        lsu_backend_wr_data_out.send(inst_ptr);
                    }
                    backend_lsu_credit_out.send(1);

                    ILOG("abstract lsu write back: Pc[0x" << std::hex << inst_ptr->getPC() << "]");
                    break;
                }
            }
        }
    }
}
