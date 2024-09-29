//
// Created by yzhang on 12/30/23.
//

#pragma once

#include "sparta/statistics/StatisticSet.hpp"
#include "sparta/simulation/Clock.hpp"

#include <string>
#include <deque>
#include "basic/Inst.hpp"
#include "basic/InstGroup.hpp"

namespace TimingModel {

    class Freelist {
    public:
        Freelist(const std::string name,
                 const uint32_t depth,
                 const sparta::Clock * clk,
                 sparta::StatisticSet * statset = nullptr);

        bool IsEmpty();

        void Push(uint64_t);

        void Pop();

        void BackupPop();

        uint64_t Front();

        void RollBack();

        size_t Size();

    private:
        std::deque<uint64_t> free_list_;

        std::deque<uint64_t> free_list_backup_;

        std::vector<uint64_t> free_list_idx_vector_;

        std::vector<uint64_t> free_list_idx_vector_backup_;
    };
}

