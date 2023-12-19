#include "PerfectLsu.hpp"

namespace TimingModel {
    const char* PerfectLsu::name = "perfect_lsu";

    PerfectLsu::PerfectLsu(sparta::TreeNode* node, const PerfectLsuParameter* p) :
        sparta::Unit(node),
        load_to_use_latency_(p->load_to_use_latency)
    {
        backend_lsu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectLsu, WriteBack, InstGroup));
    }

    bool PerfectLsu::IsReady(IssueNum issue_num) {
        return issue_num <= 1;
    }

    void PerfectLsu::WriteBack(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            if (inst_ptr->Fu == FuncType::LDU) {
                std::cout << "lsu get load inst: " << inst_ptr->pc << std::endl;
                lsu_backend_finish_out.send(inst_ptr->RobTag, load_to_use_latency_ - 1);
            } else if (inst_ptr->Fu == FuncType::STU) {
                std::cout << "lsu get store inst: " << inst_ptr->pc << std::endl;
                lsu_backend_finish_out.send(inst_ptr->RobTag);
            }
        }
    }
}