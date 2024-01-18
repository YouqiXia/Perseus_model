#include "Lsq.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* LSQ::name = "LSQ";

    LSQ::LSQ(sparta::TreeNode* node, const LSQParameter* p) :
        sparta::Unit(node),
        load_to_use_latency_(p->load_to_use_latency),
        cache_access_ports_num_(p->cache_access_ports_num),
        ld_queue_(p->ld_queue_size),
        ld_queue_size_(p->ld_queue_size),
        st_queue_(p->st_queue_size),
        st_queue_size_(p->st_queue_size),
        abstract_lsu_mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator)
    {
        renaming_lsu_allocate_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, AllocateInst_, InstGroupPtr));

        Rob_lsu_wakeup_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, RobWakeUp, InstPtr));

        agu_lsq_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, handleAgu, InstPtr));

        l1d_cache_lsu_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, handleCacheResp, MemAccInfoGroup));
        l1d_cache_lsu_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, acceptCredit, Credit));

        write_back_func_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, AcceptCredit_, Credit));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(LSQ, SendInitCredit));

        std::cout << "create LSQ" << std::endl;

        uev_dealloc_inst_ >> uev_issue_inst_; 
    }

    void LSQ::AcceptCredit_(const Credit& credit) {
        ILOG("LSQ get credits from write back stage: " << credit);
        wb_credit_++;
    }

    void LSQ::AllocateInst_(const InstGroupPtr& inst_grp) {
        // Receive insts to lsq_queue_
        ILOG("get insn from renaming, size: " << inst_grp->size(););
        for (auto& inst_ptr: *inst_grp) {
            if(inst_ptr->getFuType() == FuncType::LDU) {
                LoadEntry entry;
                entry.RobTag = inst_ptr->getRobTag();
                entry.status = Status_t::ALLOC;
                inst_ptr->setLSQTag(ld_queue_.tail());
                ld_queue_.push(entry);
                ILOG("alloc ldq entry: " << inst_ptr->getLSQTag() << ", " << inst_ptr);
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                StoreEntry entry;
                entry.RobTag = inst_ptr->getRobTag();
                entry.status = Status_t::ALLOC;
                entry.IsWkup = inst_ptr->getStoreWkup();
                inst_ptr->setLSQTag(st_queue_.tail());
                st_queue_.push(entry);
                ILOG("alloc stq entry: " << inst_ptr->getLSQTag() << ", " << inst_ptr);
            }   
        }

        lsu_renaming_allocate_out.send(inst_grp);
        
    }

    void LSQ::RobWakeUp(const InstPtr& inst) {
        // wakeup store insn in st_queue_
        ILOG("try wake up store insn:" << inst);
        sparta_assert(!st_queue_.empty(), "rob wakeup, but store queueu is empty");
        auto idx = st_queue_.head();
        auto usage = st_queue_.size(); 
        while(usage--) {
            auto& entry = st_queue_[idx];
            if (!entry.IsWkup && (entry.RobTag == inst->getRobTag())) {
                entry.IsWkup = true;
                ILOG("Wake up store insn:" << inst);
                break;
            } 
            idx = st_queue_.getNextPtr(idx);
        }

        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        
    }

    void LSQ::handleAgu(const InstPtr& inst) {
        if(inst->getFuType()==FuncType::LDU) {
            sparta_assert(!ld_queue_.empty(), "agu->ldq addr, but ldq is empty");
            ld_queue_[inst->getLSQTag()].inst_ptr = inst;
            ld_queue_[inst->getLSQTag()].status = Status_t::INSNRDY;
            ILOG("ldEntry: " << inst->getLSQTag() << ", set INSNRDY for getting load insn from agu: " << inst);
        } else if (inst->getFuType()==FuncType::STU) {
            sparta_assert(!st_queue_.empty(), "agu->stq addr, but stq is empty");
            st_queue_[inst->getLSQTag()].inst_ptr = inst;
            st_queue_[inst->getLSQTag()].status = Status_t::INSNRDY;
            ILOG("stEntry: " << inst->getLSQTag() << ", set INSNRDY for getting store insn from agu: " << inst);
        } 
        
        uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
    }

    //control ldq/stq visit DCache in program order.
    void LSQ::InOrderIssue() {
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
            auto& ld_entry = ld_queue_[ld_idx];
            ILOG("ldEntry: " << ld_idx);
            if (ld_entry.status == Status_t::INSNRDY) {
                ILOG("insn ready ldEntry: " << ld_idx << ", insn: " << ld_entry.inst_ptr);
                ld_to_issue_cnt++;
                if(ld_to_issue_cnt == 1) {
                    first_to_issue_ld_idx = ld_idx;
                }
            } else if (ld_entry.status == Status_t::ALLOC) {
                ILOG("just alloc ldEntry: " << ld_idx);
                break;
            }
            ld_idx = ld_queue_.getNextPtr(ld_idx);
        }        
        while(st_usage--) {
            auto& st_entry = st_queue_[st_idx];
            ILOG("stEntry: " << st_idx);
            if (st_entry.status == Status_t::INSNRDY) {
                ILOG("insn ready stEntry: " << st_idx << ", insn: " << st_entry.inst_ptr);
                if(st_entry.IsWkup) {
                    st_to_issue_cnt++;
                }else {
                    break;
                }
                if(st_to_issue_cnt == 1) {
                    first_to_issue_st_idx = st_idx;
                }
            } else if (st_entry.status == Status_t::ALLOC) {
                 ILOG("just alloc stEntry: " << st_idx);
                break;
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
                auto& ld_entry = ld_queue_[first_to_issue_ld_idx];
                auto& st_entry = st_queue_[first_to_issue_st_idx];
                if(ld_entry.inst_ptr->getUniqueID() > st_entry.inst_ptr->getUniqueID()) {
                    //larger means younger.
                    st_entry.status = Status_t::ISSUED;
                    st_entry.inst_ptr->setStoreWkup(true);
                    first_to_issue_st_idx++;
                    st_to_issue_cnt--;
                    insn_grp.emplace_back(st_entry.inst_ptr);
                }else {
                    ld_entry.status = Status_t::ISSUED;
                    first_to_issue_ld_idx++;
                    ld_to_issue_cnt--;
                    insn_grp.emplace_back(ld_entry.inst_ptr);
                }
            } else if (exist_ld_to_issue) {
                auto& ld_entry = ld_queue_[first_to_issue_ld_idx];
                ld_entry.status = Status_t::ISSUED;
                first_to_issue_ld_idx++;
                ld_to_issue_cnt--;
                insn_grp.emplace_back(ld_entry.inst_ptr);
            } else if (exist_st_to_issue) {
                auto& st_entry = st_queue_[first_to_issue_st_idx];
                st_entry.status = Status_t::ISSUED;
                st_entry.inst_ptr->setStoreWkup(true);
                first_to_issue_st_idx++;
                st_to_issue_cnt--;
                insn_grp.emplace_back(st_entry.inst_ptr);
            }
        }
        ILOG("mid order control")
        sendInsts(insn_grp);
        ILOG("exit order control");
        if(!ld_queue_.empty() || !st_queue_.empty()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }


    void LSQ::sendInsts(const InstGroup& inst_group) {
        MemAccInfoGroup reqs;
        for (auto insn :inst_group){
            MemAccInfoPtr req = sparta::allocate_sparta_shared_pointer<MemAccInfo>(abstract_lsu_mem_acc_info_allocator_);
            req->insn = insn;
            req->address = insn->getTargetVAddr();
            reqs.emplace_back(req);
            ILOG("LSQ access cache: " << insn);
            cache_credit_--;
        }
        if(reqs.size())
            lsu_l1d_cache_out.send(reqs);
    }

    // Check for ready to issue instructions
    bool LSQ::isReadyToIssueInsts() {
        bool ready = false;
        if(!ld_queue_.empty() || !st_queue_.empty()){
            ready = true;
        }
        return ready;
    }

    void LSQ::acceptCredit(const Credit& credit) {
        cache_credit_ += credit;

        ILOG("LSQ get credits from cache: " << credit);

        // Schedule another instruction issue event if possible
        if (isReadyToIssueInsts()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    void LSQ::SendInitCredit() {
        lsu_renaming_ldq_credit_out.send(ld_queue_size_);
        ILOG("Ldq initial credits for BE: " << ld_queue_size_);

        lsu_renaming_stq_credit_out.send(st_queue_size_);
        ILOG("Stq initial credits for BE: " << st_queue_size_);
    }

    void LSQ::handleCacheResp(const MemAccInfoGroup& resps){
        // std::cout << "LSQ::handleCacheResp, received cache resp." << std::endl;
        ILOG("received cache resp.");
        for( auto respMemInfoPtr : resps) {
            auto inst_ptr = respMemInfoPtr->insn;
            ILOG("LSQ Receive DCache Resp, LSQTag: " << inst_ptr->getLSQTag() << ", " << inst_ptr);
            if (inst_ptr->getFuType() == FuncType::LDU) {
                ILOG("ldq entry insn: " << ld_queue_[inst_ptr->getLSQTag()].inst_ptr);
                sparta_assert(ld_queue_[inst_ptr->getLSQTag()].status == Status_t::ISSUED, "NOT issued ld get resp");
                ld_queue_[inst_ptr->getLSQTag()].status = Status_t::RESP;
            }else if (inst_ptr->getFuType() == FuncType::STU) {
                ILOG("stq entry insn: " << st_queue_[inst_ptr->getLSQTag()].inst_ptr);
                sparta_assert(st_queue_[inst_ptr->getLSQTag()].status == Status_t::ISSUED, "NOT issued st get resp");
                st_queue_[inst_ptr->getLSQTag()].status = Status_t::RESP;
            }
        }

        uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
    }

    void LSQ::LSQ_Dealloc() {
        auto ld_idx = ld_queue_.head();
        auto ld_usage = ld_queue_.size();       
        auto st_idx = st_queue_.head();
        auto st_usage = st_queue_.size();
        uint32_t ld_dealloc_cnt = 0;
        uint32_t st_dealloc_cnt = 0;

        while(ld_usage--) {
            auto& ld_entry = ld_queue_[ld_idx];
            if ((ld_entry.status == Status_t::RESP) && (wb_credit_ > 0)) {
                ILOG("load insn dealloc from LDQ: " << ld_entry.inst_ptr);
                ld_queue_.pop();
                ld_dealloc_cnt++;
                ld_idx = ld_queue_.getNextPtr(ld_idx);
                sparta_assert((wb_credit_>=0));
                func_following_finish_out.send(ld_entry.inst_ptr);
            } else {
                break;
            }           
        }        
        while(st_usage--) {
            auto& st_entry = st_queue_[st_idx];
            if (st_entry.status == Status_t::RESP) {
                ILOG("store insn dealloc from STQ: " << st_entry.inst_ptr);
                st_queue_.pop();
                st_dealloc_cnt++;
                st_idx = st_queue_.getNextPtr(st_idx);
                func_following_finish_out.send(st_entry.inst_ptr);
            } else {
                break;
            }            
        }

        lsu_renaming_ldq_credit_out.send(ld_dealloc_cnt);
        lsu_renaming_stq_credit_out.send(st_dealloc_cnt);

        if (!ld_queue_.empty() || !st_queue_.empty()) {
            uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }
}
