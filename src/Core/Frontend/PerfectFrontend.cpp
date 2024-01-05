#include <algorithm>

#include "sparta/simulation/ResourceTreeNode.hpp"
#include "sparta/utils/LogUtils.hpp"

#include "PerfectFrontend.hpp"

namespace TimingModel {

    const char* PerfectFrontend::name = "perfect_frontend";

    PerfectFrontend::PerfectFrontend(sparta::TreeNode* node, const PerfectFrontendParameter* p) : 
        sparta::Unit(node),
        node_(node),
        issue_num_(p->issue_num),
        mavis_facade_(getMavis(node))
    {
        inst_generator_ = InstGenerator::createGenerator(mavis_facade_, p->input_file, false);
        fetch_backend_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectFrontend, AcceptCredit, Credit));
    }

    void PerfectFrontend::AcceptCredit(const Credit& credit) {
        credit_ += credit;

        ILOG("perfect frontend get credits from backend: " << credit);

        produce_inst_event_.schedule(sparta::Clock::Cycle(0));
    }

    void PerfectFrontend::ProduceInst() {
        uint64_t produce_num = std::min(issue_num_, credit_);
        InstGroup inst_group;
        if (!produce_num) { 
            return; 
        }
        while(produce_num--) {
            if (!inst_generator_) {
                return;
            }
            InstPtr dinst;
            dinst = inst_generator_->getNextInst(getClock());

            if(nullptr == dinst) {
                if (inst_group.size()) {
                    break;
                }

                return;
            }
            inst_group.emplace_back(dinst);
            --credit_;
        }

        if (credit_) {
            produce_inst_event_.schedule(1);
        }
        ILOG("perfect frontend send " << inst_group.size() << " instructions to backend");
        fetch_backend_inst_out.send(inst_group);
    }

}