#include <algorithm>

#include "sparta/utils/LogUtils.hpp"

#include "PerfectFrontend.hpp"

namespace TimingModel {

    const char* PerfectFrontend::name = "perfect_frontend";

    PerfectFrontend::PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p) : 
        sparta::Unit(node),
        is_speculation_(p->is_speculation),
        is_config_(p->is_config),
        input_file_(p->input_file),
        issue_num_(p->issue_width),
        insn_gen_type_(p->insn_gen_type)
    {
        sparta::StartupEvent(node, CREATE_SPARTA_HANDLER(PerfectFrontend, Startup_));

        backend_fetch_credit_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, AcceptCredit, Credit));
        backend_branch_resolve_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, BranchResolve, InstPtr));
        backend_bpu_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, BranchCommit, InstGroupPtr));
        backend_redirect_pc_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, RedirectPc, InstPtr));
    }

    void PerfectFrontend::Startup_() {
        mavis_ = getMavis(getContainer());
        if (mavis_->getFacade() == nullptr) {
            startup_event_.schedule(sparta::Clock::Cycle(1));
            return;
        }
        allocator_ = getSelfAllocators(getContainer());
        pmu_ = getPmuUnit(getContainer());

        if (!is_config_) {
            if (insn_gen_type_.compare("spike") == 0){
                inst_generator_ = InstGenerator::createGenerator(mavis_->getFacade(), input_file_);
            }else if (insn_gen_type_.compare("trace") == 0){
                inst_generator_ = InstGenerator::createGenerator(mavis_->getFacade(), input_file_, false);
            }
        }

        if (pmu_->IsPmuOn()) {
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
    }

    void PerfectFrontend::AcceptCredit(const Credit& credit) {
        credit_ += credit;

        ILOG("perfect frontend get credits from backend: " << credit);

        produce_inst_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectFrontend::BranchResolve(const InstPtr& inst_ptr) {
        if (!is_speculation_) {
            return;
        }

        /* bpu state will be change due to operation here */

    }

    void PerfectFrontend::BranchCommit(const TimingModel::InstGroupPtr &inst_group_ptr) {
        if (!is_speculation_) {
            return;
        }

        for (auto inst_ptr: *inst_group_ptr) {
            inst_generator_->branchResolve(inst_ptr->getIsMissPrediction());
        }
    }

    void PerfectFrontend::RedirectPc(const TimingModel::InstPtr &inst_ptr) {
        inst_generator_->setNpc(inst_ptr->getSpikeNpc());
        inst_generator_->setPredictionMiss(false);
    }

    void PerfectFrontend::ProduceInst() {
        if (pmu_->IsPmuOn()) {
            pmu_event.cancel();
            pmu_event.schedule(sparta::Clock::Cycle(1));
        }
        pmu_->Monitor(getName(), "1 event", 1);

        if (credit_ < issue_num_) {
            pmu_->Monitor(getName(), "3 backend loss", issue_num_-credit_);
        }
        
        uint64_t produce_num = std::min(issue_num_, credit_);
        InstGroupPtr inst_group_ptr =
                sparta::allocate_sparta_shared_pointer<InstGroup>(*allocator_->instgroup_allocator);
        if (!produce_num) { 
            return; 
        }
        if (!inst_generator_) {
            return;
        }
        while(produce_num--) {

            InstPtr dinst;
            dinst = inst_generator_->getNextInst(getClock());

            if(nullptr == dinst) {
                break;
            }
            ILOG("perfect frontend generate: " << dinst);

            /* simulate bpu */
            /* ============================ */
            uint64_t predict_npc;
            if (dinst->getPc() == 0x1010) {
                dinst->setFuType(FuncType::ALU);
            }
            if (dinst->getFuType() == FuncType::BRU) {

                if (dinst->getIsRvcInst()) {
                    predict_npc = dinst->getPc() + 2;
                } else {
                    predict_npc = dinst->getPc() + 4;
                }

                if (is_speculation_) {
                    inst_generator_->makeBackup();
                    inst_generator_->setNpc(predict_npc);

                    if (predict_npc != dinst->getSpikeNpc()) {
                        dinst->setIsMissPrediction(true);
                        inst_generator_->setPredictionMiss(true);
                    }
                }

            }
            /* ============================ */

            inst_group_ptr->emplace_back(dinst);
            --credit_;
        }
        pmu_->Monitor(getName(), "5 total loss", issue_num_-inst_group_ptr->size());
        pmu_->Monitor(getName(), "4 frontend loss", produce_num+1);

        if (credit_ && (false == inst_generator_->isDone())) {
            produce_inst_event_.schedule(1);
        }
        fetch_backend_inst_out.send(inst_group_ptr);
    }

    void PerfectFrontend::PmuMonitor_() {
        if (!pmu_->IsPmuOn()) {
            return;
        }
        pmu_->Monitor(getName(), "2 pmu event", 1);
        pmu_event.schedule(sparta::Clock::Cycle(1));

        pmu_->Monitor(getName(), "5 total loss", issue_num_);

        if (!credit_) {
            pmu_->Monitor(getName(), "3 backend loss", issue_num_);
        } else {
            pmu_->Monitor(getName(), "4 frontend loss", issue_num_);
        }
    }
}