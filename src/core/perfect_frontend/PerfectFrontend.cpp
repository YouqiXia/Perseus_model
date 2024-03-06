#include <algorithm>

#include "sparta/utils/LogUtils.hpp"

#include "PerfectFrontend.hpp"

namespace TimingModel {

    const char* PerfectFrontend::name = "perfect_frontend";

    PerfectFrontend::PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p) : 
        sparta::Unit(node),
        is_speculation_(p->is_speculation),
        node_(node),
        issue_num_(p->issue_num),
        mavis_facade_(getMavis(node)),
        insn_gen_type_(p->insn_gen_type)
    {
        if (insn_gen_type_.compare("spike") == 0){
            inst_generator_ = InstGenerator::createGenerator(mavis_facade_, p->input_file);
        }else if (insn_gen_type_.compare("trace") == 0){
            inst_generator_ = InstGenerator::createGenerator(mavis_facade_, p->input_file, false);
        }
        
        backend_fetch_credit_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, AcceptCredit, Credit));
        backend_bpu_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, BranchResolve, InstGroupPtr));
        backend_redirect_pc_inst_in.registerConsumerHandler(
                CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, RedirectPc, InstPtr));
    }

    void PerfectFrontend::AcceptCredit(const Credit& credit) {
        credit_ += credit;

        ILOG("perfect frontend get credits from backend: " << credit);

        produce_inst_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectFrontend::BranchResolve(const InstGroupPtr& inst_group_ptr) {
        if (!is_speculation_) {
            return;
        }
        for (auto inst_ptr: *inst_group_ptr) {
            inst_generator_->branchResolve(inst_ptr->getIsMissPrediction());
        }
    }

    void PerfectFrontend::RedirectPc(const TimingModel::InstPtr &inst_ptr) {
        inst_generator_->setNpc(inst_ptr->getSpikeNpc());
    }

    void PerfectFrontend::ProduceInst() {
        uint64_t produce_num = std::min(issue_num_, credit_);
        InstGroupPtr inst_group_ptr = sparta::allocate_sparta_shared_pointer<InstGroup>(instgroup_allocator);
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
                    }
                }

            }
            /* ============================ */

            inst_group_ptr->emplace_back(dinst);
            --credit_;
        }

        if (credit_ && (false == inst_generator_->isDone())) {
            produce_inst_event_.schedule(1);
        }
        ILOG("perfect frontend send " << inst_group_ptr->size() << " instructions to backend");
        fetch_backend_inst_out.send(inst_group_ptr);
    }

}