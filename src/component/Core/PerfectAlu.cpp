#include "PerfectAlu.hpp"

namespace TimingModel {
    const char* PerfectAlu::name = "perfect_alu";

    PerfectAlu::PerfectAlu(sparta::TreeNode* node, const PerfectAluParameter* p) :
        sparta::Unit(node)
    {
        backend_alu_inst_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(PerfectAlu, WriteBack, InstGroup));
    }

    bool PerfectAlu::IsReady(IssueNum issue_num) {
        (void) issue_num;
        return true;
    }

    void PerfectAlu::WriteBack(const InstGroup& inst_group) {
        for (InstPtr inst_ptr: inst_group) {
            std::cout << "alu get inst: " << inst_ptr->getPC() << std::endl;
            alu_backend_finish_out.send(inst_ptr->getRobTag());
        }
    }
}