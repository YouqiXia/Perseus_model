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
    for (int i = 0; i < rob_.getUsage(); ++i) {
        if (!rob_[i].is_issued && rob_[i].is_valid) {
            tmp_inst_group.emplace_back(rob_[i].inst_ptr);
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

void IntegrateRob::AllocateRobEntry(InstPtr inst_ptr) {
    std::cout << "Rob allocation instrution pc: " << inst_ptr->getPC() 
              << " is issued. function unit is " << inst_ptr->getFuType()
              << std::endl;
    inst_ptr->setRobTag(rob_.getTail());
    rob_.Push(RobEntry{inst_ptr, true, false, false});
}

void IntegrateRob::IssueInst(RobIdx rob_idx) {
    std::cout << "instrution pc: " << rob_[rob_idx].inst_ptr->getPC() 
              << " is issued. function unit is " << rob_[rob_idx].inst_ptr->getFuType()
              << std::endl;
    rob_[rob_idx].is_issued = true;
}

void IntegrateRob::FinishInst(RobIdx rob_idx) {
    std::cout << "instrution pc: " << rob_[rob_idx].inst_ptr->getPC() 
              << " is finished"
              << std::endl;
    rob_[rob_idx].is_finished = true;
}

void IntegrateRob::Clear(RobIdx rob_idx) {
    rob_[rob_idx].is_finished = false;
    rob_[rob_idx].is_issued = false;
    rob_[rob_idx].is_valid = false;
}

void IntegrateRob::Commit(IssueNum issue_num) {
    for (int i = 0; i < issue_num; ++i) {
        uint64_t idx = rob_.getHeader();
        if (rob_[idx].is_finished && rob_[idx].is_issued && rob_[idx].is_valid) {
            std::cout << "instrution pc: " << rob_[idx].inst_ptr->getPC() << " is committed" << std::endl;
            Clear(idx);
            rob_.Pop();
        } else {
            break;
        }
    }
}

}