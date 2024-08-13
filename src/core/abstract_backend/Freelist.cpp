//
// Created by yzhang on 12/30/23.
//

#include "Freelist.hpp"
#include "sparta/utils/SpartaAssert.hpp"

namespace TimingModel {

    Freelist::Freelist(const std::string name,
                       const uint32_t depth,
                       const sparta::Clock * clk,
                       sparta::StatisticSet * stat_set) :
        free_list_(),
        free_list_backup_(),
        free_list_idx_vector_(depth, 0),
        free_list_idx_vector_backup_(depth, 0)
    {
        for(uint64_t i = 1; i <= depth - 1; ++i) {
            free_list_.push_back(i);
            free_list_backup_.push_back(i);
            free_list_idx_vector_[i]++;
            free_list_idx_vector_backup_[i]++;
        }
    }

    bool Freelist::IsEmpty() {
        return free_list_.empty();
    }

    void Freelist::Push(uint64_t token) {
        free_list_.push_back(token);
        free_list_backup_.push_back(token);
        free_list_idx_vector_[token]++;
        free_list_idx_vector_backup_[token]++;
        sparta_assert(free_list_idx_vector_[token] <= 1, "token is：" << token);
        sparta_assert(free_list_idx_vector_backup_[token] <= 1, "token is：" << token);
    }

    void Freelist::Pop() {
        free_list_idx_vector_[free_list_.front()]--;
        sparta_assert(free_list_idx_vector_[free_list_.front()] >= 0, "token is：" << free_list_.front());
        free_list_.pop_front();
    }

    void Freelist::BackupPop() {
        free_list_idx_vector_backup_[free_list_backup_.front()]--;
        sparta_assert(free_list_idx_vector_backup_[free_list_backup_.front()] >= 0,
                      "token is：" << free_list_backup_.front());
        free_list_backup_.pop_front();
    }

    uint64_t Freelist::Front() {
        return free_list_.front();
    }

    void Freelist::RollBack() {
        free_list_ = free_list_backup_;

        free_list_idx_vector_ = free_list_idx_vector_backup_;
    }

}
