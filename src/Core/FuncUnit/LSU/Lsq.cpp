#include "Lsq.hpp"
#include "sparta/utils/LogUtils.hpp"
#include <stdlib.h>

namespace TimingModel {
    const char* LSQ::name = "LSQ";

    LSQ::LSQ(sparta::TreeNode* node, const LSQParameter* p) :
        sparta::Unit(node),
        issue_width_(p->issue_width),
        load_to_use_latency_(p->load_to_use_latency),
        cache_access_ports_num_(p->cache_access_ports_num),
        ld_queue_(p->ld_queue_size),
        ld_queue_size_(p->ld_queue_size),
        st_queue_(p->st_queue_size),
        st_queue_size_(p->st_queue_size),
        abstract_lsu_mem_acc_info_allocator_(sparta::notNull(OlympiaAllocators::getOlympiaAllocators(node))->
                                 mem_acc_info_allocator)
    {
        renaming_lsu_allocate_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, AllocateInst_, InstGroupPtr));
        Rob_lsu_wakeup_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, RobWakeUp, InstPtr));
        agu_lsq_inst_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, handleAgu, InstGroupPtr));
        write_back_func_credit_in.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(LSQ, AcceptCredit_, Credit));

        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(LSQ, SendInitCredit));

        std::cout << "create LSQ" << std::endl;

        uev_dealloc_inst_ >> uev_issue_inst_; 
    }

    void LSQ::SendInitCredit() {
        lsu_renaming_ldq_credit_out.send(ld_queue_size_);
        ILOG("Ldq initial credits for BE: " << ld_queue_size_);

        lsu_renaming_stq_credit_out.send(st_queue_size_);
        ILOG("Stq initial credits for BE: " << st_queue_size_);
    }

    void LSQ::AcceptCredit_(const Credit& credit) {
        ILOG("LSQ get credits from write back stage: " << credit);
        wb_credit_++;
        uev_issue_inst_.schedule(0);
    }

    void LSQ::AllocateInst_(const InstGroupPtr& inst_group_ptr) {
        // Receive insts to lsq_queue_
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
        ILOG("get insn from renaming, size: " << inst_group_ptr->size());
        for (auto& inst_ptr: *inst_group_ptr) {
            if(inst_ptr->getFuType() == FuncType::LDU) {
                LoadEntry entry;
                entry.RobTag = inst_ptr->getRobTag();
                entry.status = Status_t::ALLOC;
                ILOG("alloc ldq entry: " << inst_ptr->getLSQTag() << ", " << inst_ptr);
                if (!st_queue_.empty() &&
                    st_queue_.back().status != Status_t::ISSUED &&
                    st_queue_.back().status != Status_t::RESP) {
                    entry.order_ready = false;
                    entry.stq_idx = st_queue_.getBackIterator().getIndex();
                } else {
                    entry.order_ready = true;
                    entry.stq_idx = 0;
                }
                inst_ptr->setLSQTag(ld_queue_.push(entry).getIndex());
            } else if (inst_ptr->getFuType() == FuncType::STU) {
                StoreEntry entry;
                entry.RobTag = inst_ptr->getRobTag();
                entry.status = Status_t::ALLOC;
                ILOG("alloc stq entry: " << inst_ptr->getLSQTag() << ", " << inst_ptr);
                if (!ld_queue_.empty() &&
                    ld_queue_.back().status != Status_t::ISSUED &&
                    ld_queue_.back().status != Status_t::RESP) {
                    entry.order_ready = false;
                    entry.ldq_idx = ld_queue_.getBackIterator().getIndex();
                } else {
                    entry.order_ready = true;
                    entry.ldq_idx = 0;
                }
                inst_ptr->setLSQTag(st_queue_.push(entry).getIndex());
            }
            inst_group_tmp_ptr->emplace_back(inst_ptr);
        }

        lsu_renaming_allocate_out.send(inst_group_ptr);
    }

    void LSQ::RobWakeUp(const InstPtr& inst) {
        // wakeup store insn in st_queue_
        ILOG("try wake up store insn:" << inst);
        sparta_assert(!st_queue_.empty(), "rob wakeup, but store queueu is empty");
        st_queue_.access(inst->getLSQTag()).IsWkup = true;
        ILOG("Wake up store insn:" << inst);
        uev_issue_inst_.schedule(sparta::Clock::Cycle(0));
    }

    void LSQ::handleAgu(const InstGroupPtr& inst_group_ptr) {
        for (auto& inst: *inst_group_ptr) {
            if(inst->getFuType()==FuncType::LDU) {
                sparta_assert(!ld_queue_.empty(), "agu->ldq addr, but ldq is empty");
                ld_queue_.access(inst->getLSQTag()).inst_ptr = inst;
                ld_queue_.access(inst->getLSQTag()).status = Status_t::INSNRDY;
                ILOG("ldEntry: " << inst->getLSQTag() << ", set INSNRDY for getting load insn from agu: " << inst);
            } else if (inst->getFuType()==FuncType::STU) {
                sparta_assert(!st_queue_.empty(), "agu->stq addr, but stq is empty");
                st_queue_.access(inst->getLSQTag()).inst_ptr = inst;
                st_queue_.access(inst->getLSQTag()).status = Status_t::INSNRDY;
                ILOG("stEntry: " << inst->getLSQTag() << ", set INSNRDY for getting store insn from agu: " << inst);
            }
        }

        uev_issue_inst_.schedule(sparta::Clock::Cycle(0));
    }

    //control ldq/stq visit DCache in program order.
    void LSQ::InOrderIssue() {
        ILOG("enter order control");
//        uint64_t cnt = std::min(cache_credit_, cache_access_ports_num_);
        auto cnt = cache_access_ports_num_;

        InstGroup inst_group_tmp;
        for (auto ldq_entry_itr = ld_queue_.begin(); ldq_entry_itr != ld_queue_.end(); ldq_entry_itr++) {
            if (cnt == 0) {
                break;
            }
            if (ldq_entry_itr->status == Status_t::INSNRDY && ldq_entry_itr->order_ready) {
                inst_group_tmp.emplace_back(ldq_entry_itr->inst_ptr);
                ldq_entry_itr->status = Status_t::ISSUED;
                sparta_assert(ldq_entry_itr->inst_ptr != nullptr, "load inst uid: " <<
                              ldq_entry_itr->inst_ptr->getUniqueID() << " is nullptr");
                auto ldq_entry_idx = ldq_entry_itr.getIndex();
                for (auto& st_entry: st_queue_) {
                    if (st_entry.ldq_idx == ldq_entry_idx) {
                        st_entry.order_ready = true;
                    }
                }
                --cnt;
                uev_resp_event.schedule(1);
            }
        }

        for (auto stq_entry_itr = st_queue_.begin(); stq_entry_itr != st_queue_.end(); stq_entry_itr++) {
            if (cnt == 0) {
                break;
            }
            if (stq_entry_itr->status == Status_t::INSNRDY && stq_entry_itr->order_ready && stq_entry_itr->IsWkup) {
                inst_group_tmp.emplace_back(stq_entry_itr->inst_ptr);
                stq_entry_itr->status = Status_t::ISSUED;
                sparta_assert(stq_entry_itr->inst_ptr != nullptr, "store inst uid: " <<
                              stq_entry_itr->inst_ptr->getUniqueID() << " is nullptr");
                auto ldq_entry_idx = stq_entry_itr.getIndex();
                for (auto& ld_entry: ld_queue_) {
                    if (ld_entry.stq_idx == ldq_entry_idx) {
                        ld_entry.order_ready = true;
                    }
                }
                --cnt;
                uev_resp_event.schedule(1);
            }
        }

        ILOG("lsq try to issue, issue num is " << inst_group_tmp.size() << " , ldq size is " <<
             ld_queue_.size() << " , stq size is " << st_queue_.size());

        if (inst_group_tmp.size() != 0) {
            sendInsts(inst_group_tmp);
        }

        if(!ld_queue_.empty() || !st_queue_.empty()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
            uev_dealloc_inst_.schedule(1);
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
//        if(reqs.size())
//            lsu_l1d_cache_out.send(reqs);
    }

    void LSQ::acceptCredit(const Credit& credit) {
        cache_credit_ += credit;

        ILOG("LSQ get credits from cache: " << credit);

        // Schedule another instruction issue event if possible
        if (!ld_queue_.empty() || !st_queue_.empty()) {
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }

    void LSQ::GetRespWithoutCache() {
        for (auto& ld_entry: ld_queue_) {
            if (ld_entry.status == Status_t::ISSUED) {
                ld_entry.status = Status_t::RESP;
            }
        }

        for (auto& st_entry: st_queue_) {
            if (st_entry.status == Status_t::ISSUED) {
                st_entry.status = Status_t::RESP;
            }
        }

    }

    void LSQ::LSQDealloc() {

        auto dealloc_num = std::min(issue_width_, wb_credit_);
        uint32_t ld_dealloc_num = 0, st_dealloc_num = 0;
        InstGroupPtr inst_group_tmp_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);

        for (auto& ld_entry: ld_queue_) {
            if (!dealloc_num) {
                break;
            }
            if (ld_queue_.front().status == Status_t::RESP) {
                inst_group_tmp_ptr->emplace_back(ld_queue_.front().inst_ptr);
                ld_queue_.pop();
                --wb_credit_;
                --dealloc_num;
                ++ld_dealloc_num;
            } else {
                break;
            }
        }

        for (auto& st_entry: st_queue_) {
            if (!dealloc_num) {
                break;
            }
            if (st_queue_.front().status == Status_t::RESP) {
                inst_group_tmp_ptr->emplace_back(st_queue_.front().inst_ptr);
                st_queue_.pop();
                --wb_credit_;
                --dealloc_num;
                ++st_dealloc_num;
            } else {
                break;
            }
        }

        if (inst_group_tmp_ptr->size() != 0) {
            func_following_finish_out.send(inst_group_tmp_ptr);
            lsu_renaming_ldq_credit_out.send(ld_dealloc_num);
            lsu_renaming_stq_credit_out.send(st_dealloc_num);
        }

        if ((!ld_queue_.empty() || !st_queue_.empty()) && wb_credit_ != 0) {
            uev_dealloc_inst_.schedule(sparta::Clock::Cycle(1));
            uev_issue_inst_.schedule(sparta::Clock::Cycle(1));
        }
    }
}
