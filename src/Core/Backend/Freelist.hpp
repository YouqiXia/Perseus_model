//
// Created by yzhang on 12/30/23.
//

#pragma once

#include "sparta/statistics/StatisticSet.hpp"
#include "sparta/simulation/Clock.hpp"

#include <string>
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

    private:
        sparta::Queue<uint64_t> free_list_;

        sparta::Queue<uint64_t> free_list_backup_;
    };
}

