//
// Created by yzhang on 1/16/24.
//

#include "PhysicalRegfile.hpp"

namespace TimingModel {

    const char* PhysicalRegfile::name = "physical_regfile";

    PhysicalRegfile::PhysicalRegfile(sparta::TreeNode *node,
            const TimingModel::PhysicalRegfile::PhysicalRegfileParameter *p) :
            sparta::Unit(node),
            phy_regfile_(p->phy_reg_num, 0),
            phy_reg_num_(p->phy_reg_num)
    {
        preceding_physical_regfile_read_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfile, ReadPhysicalReg_, InstGroupPtr));
        preceding_physical_regfile_write_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA
            (PhysicalRegfile, WritePhysicalReg_, InstGroupPtr));
    }

    void PhysicalRegfile::ReadPhysicalReg_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getRs1Type() != RegType_t::NONE && !inst_ptr->getIsRs1Forward()) {
                if (inst_ptr->getPhyRs1() == 0) {
                    inst_ptr->setOperand1(0);
                } else {
                    sparta_assert(inst_ptr->getPhyRs1() < phy_reg_num_, "physical register accessing is out of range");
                    inst_ptr->setOperand1(phy_regfile_[inst_ptr->getPhyRs1()]);
                }
            }

            if (inst_ptr->getRs2Type() != RegType_t::NONE && !inst_ptr->getIsRs2Forward()) {
                if (inst_ptr->getPhyRs2() == 0) {
                    inst_ptr->setOperand2(0);
                } else {
                    sparta_assert(inst_ptr->getPhyRs2() < phy_reg_num_, "physical register accessing is out of range");
                    inst_ptr->setOperand2(phy_regfile_[inst_ptr->getPhyRs2()]);
                }
            }
        }
        physical_regfile_following_read_out.send(inst_group_ptr);
    }

    void PhysicalRegfile::WritePhysicalReg_(const TimingModel::InstGroupPtr &inst_group_ptr) {
        for (auto& inst_ptr: *inst_group_ptr) {
            if (inst_ptr->getPhyRd() == 0) {
                continue;
            }
            sparta_assert(inst_ptr->getPhyRd() < phy_reg_num_, "physical register accessing is out of range");
            phy_regfile_[inst_ptr->getPhyRd()] = inst_ptr->getRdResult();
        }
    }
}