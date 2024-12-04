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
            std::vector<InstPtr> rs1_deps;
            std::vector<InstPtr> rs2_deps;
        };

    public:
        DependencyTable() = default;

        void Allocate(InstPtr);

        void Resolve(InstPtr);

        void Pop(InstPtr);

        bool Empty() { return dependent_table_.empty(); };

    private:
        std::unordered_map<uint64_t, OperandDeps> dependent_table_;
        std::unordered_map<uint64_t, std::vector<InstPtr>::iterator> backward_iterator_table_;
    };

}