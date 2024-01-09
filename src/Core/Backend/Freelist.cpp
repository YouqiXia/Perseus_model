//
// Created by yzhang on 12/30/23.
//

#include "Freelist.hpp"

namespace TimingModel {

    Freelist::Freelist(const std::string name,
                       const uint32_t depth,
                       const sparta::Clock * clk,
                       sparta::StatisticSet * stat_set) :
        free_list_(name, depth, clk, stat_set),
        free_list_backup_(name+"_backup", depth, clk, stat_set)
    {
        for(uint64_t i = 1; i <= depth; ++i) {
            free_list_.push(i);
            free_list_backup_.push(i);
        }
    }

    bool Freelist::IsEmpty() {
        return free_list_.empty();
    }

    void Freelist::Push(uint64_t token) {
        free_list_.push(token);
        free_list_backup_.push(token);
    }

    void Freelist::Pop() {
        free_list_.pop();
    }

    void Freelist::BackupPop() {
        free_list_backup_.pop();
    }

    uint64_t Freelist::Front() {
        return free_list_.front();
    }

    void Freelist::RollBack() {
        free_list_.clear();

        for (auto& entry: free_list_backup_) {
            free_list_.push(entry);
        }
    }

}
