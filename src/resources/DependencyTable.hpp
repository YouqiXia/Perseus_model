//
// Created by yzhang on 12/2/24.
//
#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "basic/Inst.hpp"

namespace TimingModel {

    class DependencyTable {
    public:
        struct OperandDeps {
            std::map<PhyRegId_t, InstPtr> rs1_deps;
            std::map<PhyRegId_t, InstPtr> rs2_deps;
        };

    public:
        DependencyTable() = default;

        void Allocate(InstPtr);

        void Resolve(InstPtr);

        void Pop(InstPtr);

        bool Empty() { return dependent_table_.empty(); };

    private:
        std::unordered_map<uint64_t, OperandDeps> dependent_table_;
    };

}