//
// Created by yzhang on 12/2/24.
//

#include "DependencyTable.hpp"

namespace TimingModel {

    void DependencyTable::Allocate(InstPtr inst_ptr) {
        auto prd_idx = inst_ptr->getPhyRd();
        if (inst_ptr->getIsRs1Forward()) {
            auto idx = inst_ptr->getPhyRs1();
            if (!dependent_table_.count(idx)) {
                dependent_table_[idx] = OperandDeps();
            }
            dependent_table_[idx].rs1_deps[prd_idx] = inst_ptr;
        }
        if (inst_ptr->getIsRs2Forward()) {
            auto idx = inst_ptr->getPhyRs2();
            if (!dependent_table_.count(idx)) {
                dependent_table_[idx] = OperandDeps();
            }
            dependent_table_[idx].rs2_deps[prd_idx] = inst_ptr;
        }
    }

    void DependencyTable::Resolve(TimingModel::InstPtr inst_ptr) {
        auto idx = inst_ptr->getPhyRd();
        if (!dependent_table_.count(idx)) {
            return;
        }

        for (auto dep_inst_pair: dependent_table_[idx].rs1_deps) {
            sparta_assert(dep_inst_pair.second->getIsRs1Forward());
            dep_inst_pair.second->setIsRs1Forward(false);
        }

        for (auto dep_inst_pair: dependent_table_[idx].rs2_deps) {
            sparta_assert(dep_inst_pair.second->getIsRs2Forward());
            dep_inst_pair.second->setIsRs2Forward(false);
        }

        dependent_table_.erase(idx);
    }

    void DependencyTable::Pop(TimingModel::InstPtr inst_ptr) {
        auto prd_idx = inst_ptr->getPhyRd();
        if (inst_ptr->getIsRs1Forward()) {
            auto idx = inst_ptr->getPhyRs1();
            dependent_table_[idx].rs1_deps.erase(prd_idx);
            if (dependent_table_[idx].rs1_deps.empty() && dependent_table_[idx].rs2_deps.empty()) {
                dependent_table_.erase(idx);
            }
        }

        if (inst_ptr->getIsRs2Forward()) {
            auto idx = inst_ptr->getPhyRs2();
            dependent_table_[idx].rs2_deps.erase(prd_idx);
            if (dependent_table_[idx].rs1_deps.empty() && dependent_table_[idx].rs2_deps.empty()) {
                dependent_table_.erase(idx);
            }
        }
    }








}