#include "AbstractLsu.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* AbstractLsu::name = "abstract_lsu";

    AbstractLsu::AbstractLsu(sparta::TreeNode* node, const AbstractLsuParameter* p) :
        sparta::Unit(node),
        load_to_use_latency_(p->load_to_use_latency),
        cache_access_ports_num_(p->cache_access_ports_num),
        issue_queue_(p->issue_queue_size),
        issue_queue_size_(p->issue_queue_size),
        ld_queue_(p->ld_queue_size),
        st_queue_(p->st_queue_size),
        agu_num_(p->agu_num),
        agu_latency_(p->agu_latency),
        func_type_(p->func_type),
        abstract_lsu_mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator)
    {
        preceding_func_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, handleDispatch, InstPtr));

        //backend_lsu_rob_idx_wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, RobWakeUp, RobIdx_t));

        preceding_func_inst_in >> sparta::GlobalOrderingPoint(node, "backend_lsu_multiport_order");

        l1d_cache_lsu_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, handleCacheResp, MemAccInfoGroup));
        l1d_cache_lsu_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, acceptCredit, Credit));

        write_back_func_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(AbstractLsu, AcceptCredit_, Credit));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(AbstractLsu, SendInitCredit));

        uev_dealloc_inst_ >> uev_issue_inst_; 

        std::cout << "create lsu from abstract lsu" << std::endl;
    }

    void AbstractLsu::handleDispatch(const InstPtr& inst_ptr) {
        // Receive insts to issue_queue_
        //for (InstPtr inst_ptr: inst_group) {
           ILOG("LSU get inst: " << inst_ptr);
           issue_queue_.push(inst_ptr);
        //}

        uev_split_inst_.schedule(sparta::Clock::Cycle(1));
        
    }

    void AbstractLsu::RobWakeUp(const RobIdx_t& rob_idx) {
        // wakeup store insn in issue_queue_
        auto idx = issue_queue_.head();
        auto usage = issue_queue_.size(); 
        while(usage--) {
            auto& inst_ptr = issue_queue_[idx];
            if (!inst_ptr->getStoreWkup() && (inst_ptr->getRobTag() == rob_idx)) {
                inst_ptr->setStoreWkup(true);
                ILOG("Wake up store insn:" << inst_ptr);
                break;
            } 
            idx = issue_queue_.getNextPtr(idx);
        }

        uev_split_inst_.schedule(sparta::Clock::Cycle(1));
        
    }

    void AbstractLsu::handleAgu(const InstGroup& inst_group) {
        for (InstPtr inst: inst_group) {
            if(inst->getFuType()==FuncType::LDU) {
                ld_queue_[inst->getLSQTag()]->setAddrReady(true);
                ILOG("set load insn addr ready: " << inst);
            } else if (inst->getFuType()==FuncType::STU) {
                st_queue_[inst->getLSQTag()]->setAddrReady(true);
                ILOG("set store insn addr ready: " << inst);
            }         
        }
        if (!issue_queue_.empty()) {
            uev_split_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    //control ldq/stq visit DCache in program order.
    void AbstractLsu::InOrderIssue() {
        ILOG("enter order control");
        uint64_t cnt = std::min(cache_credit_, cache_access_ports_num_);

        auto ld_idx = ld_queue_.head();
        auto ld_usage = ld_queue_.size();       
        auto st_idx = st_queue_.head();
        auto st_usage = st_queue_.size();
        uint64_t first_to_issue_ld_idx = 0;
        uint64_t first_to_issue_st_idx = 0;
        uint32_t ld_to_issue_cnt = 0;
        uint32_t st_to_issue_cnt = 0;
        bool exist_ld_to_issue = false;
        bool exist_st_to_issue = false;
        InstGroup insn_grp;

        //count ld/st insn to be issued.
        while(ld_usage--) {
            auto inst_ptr = ld_queue_[ld_idx];
            if (!inst_ptr->getLsuIssued()) {
                if(inst_ptr->getAddrReady()) {
                    ld_to_issue_cnt++;
                }else {
                    break;
                }
                if(ld_to_issue_cnt == 1) {
                    first_to_issue_ld_idx = ld_idx;
                }
            } 
            ld_idx = ld_queue_.getNextPtr(ld_idx);
        }        
        while(st_usage--) {
            auto inst_ptr = st_queue_[st_idx];
            if (!inst_ptr->getLsuIssued()) {
                if(inst_ptr->getAddrReady()) {
                    st_to_issue_cnt++;
                }else {
                    break;
                }
                if(st_to_issue_cnt == 1) {
                    first_to_issue_st_idx = st_idx;
                }
            } 
            st_idx = st_queue_.getNextPtr(st_idx);
        }

        ILOG("issue_max = " << cnt << ", ld_to_issue_cnt = " << ld_to_issue_cnt << ", first_to_issue_ld_idx = " << first_to_issue_ld_idx <<
             ", st_to_issue_cnt = " << st_to_issue_cnt << ", first_to_issue_st_idx = " << first_to_issue_st_idx);

        //visit DCache in program order.
        for(int i=0; i<cnt; i++) {
            exist_ld_to_issue = false;
            exist_st_to_issue = false;            
            if(ld_to_issue_cnt != 0) {
                exist_ld_to_issue = true;
            }
            if(st_to_issue_cnt != 0) {
                exist_st_to_issue = true;
            }
            if(exist_ld_to_issue && exist_st_to_issue) {
                auto& ld_inst_ptr = ld_queue_[first_to_issue_ld_idx];
                auto& st_inst_ptr = st_queue_[first_to_issue_st_idx];
                if(ld_inst_ptr->getUniqueID() > st_inst_ptr->getUniqueID()) {
                    //larger means younger.
                    st_inst_ptr->setLsuIssued(true);
                    first_to_issue_st_idx++;
                    st_to_issue_cnt--;
                    insn_grp.emplace_back(st_inst_ptr);
                }else {
                    ld_inst_ptr->setLsuIssued(true);
                    first_to_issue_ld_idx++;
                    ld_to_issue_cnt--;
                    insn_grp.emplace_back(ld_inst_ptr);
                }
            } else if (exist_ld_to_issue) {
                auto& ld_inst_ptr = ld_queue_[first_to_issue_ld_idx];
                ld_inst_ptr->setLsuIssued(true);
                first_to_issue_ld_idx++;
                ld_to_issue_cnt--;
                insn_grp.emplace_back(ld_inst_ptr);
            } else if (exist_st_to_issue) {
                auto& st_inst_ptr = st_queue_[first_to_issue_st_idx];
                st_inst_ptr->setLsuIssued(true);
                first_to_issue_st_idx++;
                st_to_issue_cnt--;
                insn_grp.emplace_back(st_inst_ptr);
            }
        }
        ILOG("mid order control")
        sendInsts(insn_grp);
        ILOG("exit order control");
        if(!ld_queue_.empty() || !st_queue_.empty()) {
            uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    // split load/store insn in issue_queue_ to ldq/stq
    void AbstractLsu::SplitInst()
    {
        Credit split_cnt = 0;
        InstGroup agu_insn_grp;
        auto idx = issue_queue_.head();
        auto usage = issue_queue_.size();
        while(usage--) {
            auto& inst_ptr = issue_queue_[idx];
            if (inst_ptr->getFuType() == FuncType::LDU) {
                if(!ld_queue_.full()) {
                    inst_ptr->setLSQTag(ld_queue_.tail());
                    agu_insn_grp.emplace_back(inst_ptr);
                    ld_queue_.push(inst_ptr);
                    issue_queue_.pop();
                    split_cnt++;
                    ILOG("LSU issue load inst to ld_queue_: " << inst_ptr);
                }else {
                    break;
                }
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                if(!inst_ptr->getStoreWkup()){
                    ILOG("Rob NOT wakeup causes fail issue store insn:" << inst_ptr);
                    break;
                }
                if(!st_queue_.full()) {
                    inst_ptr->setLSQTag(st_queue_.tail());
                    agu_insn_grp.emplace_back(inst_ptr);
                    st_queue_.push(inst_ptr);
                    issue_queue_.pop();
                    split_cnt++;
                    ILOG("LSU issue store inst to st_queue_: " << inst_ptr);
                } else {
                    break;
                }
            }
            if(split_cnt >= agu_num_) {
                break;
            }
            idx = issue_queue_.getNextPtr(idx);
        }

        func_rs_credit_out.send(split_cnt);
        if (agu_insn_grp.size() > 0) {
            ILOG("schedule handleAgu");
            ev_agu_.preparePayload(agu_insn_grp)->schedule(agu_latency_);
        }

        // Schedule another instruction split event if possible
        if (isReadyToSplitInsts()) {
            uev_split_inst_.schedule(sparta::Clock::Cycle(1));
        }

        uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
    }

    void AbstractLsu::sendInsts(const InstGroup& inst_group) {
        MemAccInfoGroup reqs;
        for (auto insn :inst_group){
            MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(abstract_lsu_mem_acc_info_allocator_);
            req->insn = insn;
            req->address = insn->getTargetVAddr();
            reqs.emplace_back(req);
            ILOG("abstract lsu access cache: " << insn);
            cache_credit_--;
        }
        if(reqs.size())
            lsu_l1d_cache_out.send(reqs);
    }

    // Check for ready to issue instructions
    bool AbstractLsu::isReadyToSplitInsts() {
        // Check if ldq and stq is full
        // if (ld_queue_.full() && st_queue_.full()) {
        //     return false;
        // }
        bool ready = false;
        if(!issue_queue_.empty()){
            ready = true;
        }
        return ready;
    }

    void AbstractLsu::acceptCredit(const Credit& credit) {
        cache_credit_ += credit;

        ILOG("abstract lsu get credits from cache: " << credit);

        // Schedule another instruction issue event if possible
        if (isReadyToSplitInsts()) {
            uev_split_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    void AbstractLsu::SendInitCredit() {
        func_rs_credit_out.send(issue_queue_size_);
        ILOG("LSU initial credits for BE: " << issue_queue_size_);
    }

    void AbstractLsu::handleCacheResp(const MemAccInfoGroup& resps){
        // std::cout << "AbstractLsu::handleCacheResp, received cache resp." << std::endl;
        ILOG("AbstractLsu::handleCacheResp, received cache resp.");
        for( auto respMemInfoPtr : resps) {
            auto inst_ptr = respMemInfoPtr->insn;
            ILOG("LSQ Receive DCache Resp: " << inst_ptr);
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ld_queue_[inst_ptr->getLSQTag()]->setResp(true);
            }else if (inst_ptr->getFuType() == FuncType::STU) {
                st_queue_[inst_ptr->getLSQTag()]->setResp(true);
            }
        }

        uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));

        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
    }

    void AbstractLsu::AcceptCredit_(const Credit& credit) {
        wb_credit_++;
    }

    void AbstractLsu::LSQ_Dealloc() {
        auto ld_idx = ld_queue_.head();
        auto ld_usage = ld_queue_.size();       
        auto st_idx = st_queue_.head();
        auto st_usage = st_queue_.size();
        //InstGroup insn_grp;

        while(ld_usage--) {
            auto inst_ptr = ld_queue_[ld_idx];
            if (inst_ptr->getLsuIssued() && inst_ptr->getResp() && (wb_credit_ > 0)) {
                //insn_grp.emplace_back(inst_ptr);
                ILOG("load insn dealloc from LDQ: " << inst_ptr);
                ld_queue_.pop();
                ld_idx = ld_queue_.getNextPtr(ld_idx);
                wb_credit_--;
                sparta_assert((wb_credit_>=0));
                FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
                func_following_finish_out.send(func_inst_ptr);
                break;
            } else {
                break;
            }           
        }        
        while(st_usage--) {
            auto inst_ptr = st_queue_[st_idx];
            if (inst_ptr->getLsuIssued() && inst_ptr->getResp()) {
                //insn_grp.emplace_back(inst_ptr);
                ILOG("store insn dealloc from STQ: " << inst_ptr);
                st_queue_.pop();
                st_idx = st_queue_.getNextPtr(st_idx);
                FuncInstPtr func_inst_ptr {new FuncInst{func_type_, inst_ptr}};
                func_following_finish_out.send(func_inst_ptr);
                break;
            } else {
                break;
            }            
        }

        //func_following_finish_out.send(insn_grp);

        if (!ld_queue_.empty() || !st_queue_.empty()) {
            uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }
}
