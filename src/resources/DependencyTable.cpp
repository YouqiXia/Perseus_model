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
            dependent_table_[idx].rs1_deps.emplace_back(inst_ptr);
            backward_iterator_table_[prd_idx] = std::prev(dependent_table_[idx].rs1_deps.end());
        }
        if (inst_ptr->getIsRs2Forward()) {
            auto idx = inst_ptr->getPhyRs2();
            if (!dependent_table_.count(idx)) {
                dependent_table_[idx] = OperandDeps();
            }
            dependent_table_[idx].rs2_deps.emplace_back(inst_ptr);
            backward_iterator_table_[prd_idx] = std::prev(dependent_table_[idx].rs2_deps.end());
        }
    }

    void DependencyTable::Resolve(TimingModel::InstPtr inst_ptr) {
        auto idx = inst_ptr->getPhyRd();
        if (!dependent_table_.count(idx)) {
            return;
        }

        for (auto dep_inst_ptr: dependent_table_[idx].rs1_deps) {
            sparta_assert(dep_inst_ptr->getIsRs1Forward());
            dep_inst_ptr->setIsRs1Forward(false);
            backward_iterator_table_.erase(dep_inst_ptr->getPhyRd());
        }

        for (auto dep_inst_ptr: dependent_table_[idx].rs2_deps) {
            sparta_assert(dep_inst_ptr->getIsRs1Forward());
            dep_inst_ptr->setIsRs1Forward(false);
            backward_iterator_table_.erase(dep_inst_ptr->getPhyRd());
        }

        dependent_table_.erase(idx);
    }

    void DependencyTable::Pop(TimingModel::InstPtr inst_ptr) {
        auto it = inst_ptr->getPhyRd();
        if (inst_ptr->getIsRs1Forward()) {
            auto idx = inst_ptr->getPhyRs1();
            dependent_table_[idx].rs1_deps.erase(backward_iterator_table_[it]);
        }

        if (inst_ptr->getIsRs2Forward()) {
            auto idx = inst_ptr->getPhyRs2();
            dependent_table_[idx].rs2_deps.erase(backward_iterator_table_[it]);
        }

        backward_iterator_table_.erase(it);
    }








}