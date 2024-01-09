#include "IntegrateRob.hpp"

#include <iostream>

namespace TimingModel {

IntegrateRob::IntegrateRob(uint64_t rob_depth) :
    rob_(rob_depth)
{}

IIntegrateRob::~IIntegrateRob() {}

IntegrateRob::~IntegrateRob() {}

bool IntegrateRob::IsRobFull(IssueNum issue_num) {
    return rob_.getAvailEntryCount() >= issue_num;
}

bool IntegrateRob::IsRobEmpty() {
    return rob_.getUsage() == 0;
}

InstPtr IntegrateRob::GetRobEntry(RobIdx rob_idx) {
    return rob_[rob_idx].inst_ptr;
}

InstGroup IntegrateRob::GetCommitingEntry(IssueNum issue_num) {
    InstGroup tmp_inst_group;
    uint64_t idx = rob_.getHeader();
    for (int i = 0; i < issue_num; ++i) {
        if (rob_[idx].is_finished) {
            tmp_inst_group.emplace_back(rob_[idx].inst_ptr);
            idx = rob_.getNextPtr(idx);
        } else {
            break;
        }
    }
    return tmp_inst_group;
}

InstGroup IntegrateRob::GetIssuingEntry(IssueNum issue_num) {
    InstGroup tmp_inst_group;
    uint64_t idx = rob_.getHeader();
    uint64_t usage = rob_.getUsage();
    while(usage--) {
        if (!rob_[idx].is_issued && rob_[idx].is_valid) {
            tmp_inst_group.emplace_back(rob_[idx].inst_ptr);
        }
        if (tmp_inst_group.size() == issue_num) {
            break;
        }
        idx = rob_.getNextPtr(idx);
    }
    return tmp_inst_group;
}

void IntegrateRob::Flush() {
    rob_.Reset();
}

bool IntegrateRob::getStoreRobIdx(RobIdx& rob_idx) {
    bool find_store = false;
    uint64_t idx = rob_.getHeader();
    // std::cout << "rob header[" << idx << "] do store wakeup check: insn_RobTag[" << rob_[idx].inst_ptr->getRobTag() 
    //           << "], '" << std::hex << rob_[idx].inst_ptr->getDisasm() << "' " << std::endl;
    if ((rob_[idx].inst_ptr->getFuType() == FuncType::STU) && !rob_[idx].inst_ptr->getStoreWkup()) {
        find_store = true;
        rob_idx = idx;
    }
    return  find_store;
}

void IntegrateRob::AllocateRobEntry(InstPtr inst_ptr) {
    inst_ptr->setRobTag(rob_.getTail());
    // std::cout << "Pc[0x" << std::hex << inst_ptr->getPC() << std::dec << "] allocate Rob entry[" << inst_ptr->getRobTag() << "], "
    //           << "rob_header[" << rob_.getHeader() << "]" << std::endl;
    if((inst_ptr->getFuType() == FuncType::STU) && (inst_ptr->getRobTag() == rob_.getHeader())){
        inst_ptr->setStoreWkup(true);
    }
    rob_.Push(RobEntry{inst_ptr, true, false, false});
}

void IntegrateRob::IssueInst(RobIdx rob_idx) {
    rob_[rob_idx].is_issued = true;
}

void IntegrateRob::FinishInst(RobIdx rob_idx) {
    rob_[rob_idx].is_finished = true;
}

void IntegrateRob::Clear(RobIdx rob_idx) {
    rob_[rob_idx].is_finished = false;
    rob_[rob_idx].is_issued = false;
    rob_[rob_idx].is_valid = false;
}

uint64_t IntegrateRob::Commit(IssueNum issue_num) {
    uint64_t commit_num = 0;
    for (int i = 0; i < issue_num; ++i) {
        uint64_t idx = rob_.getHeader();
        if (rob_[idx].is_finished && rob_[idx].is_issued && rob_[idx].is_valid) {
            Clear(idx);
            rob_.Pop();
            ++commit_num;
        } else {
            break;
        }
    }
    return commit_num;
}

}