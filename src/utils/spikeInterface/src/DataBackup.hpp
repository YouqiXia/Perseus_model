//
// Created by yzhang on 2/29/24.
//

#ifndef MODEL_DATABACKUP_HPP
#define MODEL_DATABACKUP_HPP

#include <vector>
#include <deque>
#include <cassert>
#include <cstdint>

struct MemoryEntry{
    uint64_t addr = 0;
    uint64_t data = 0;
    uint64_t len = 0;
};

struct CsrEntry{
    int which = 0;
    uint64_t val = 0;
};

struct RegEntry {
    regfile_t<reg_t, NXPR, true> XPR;
    regfile_t<freg_t, NFPR, false> FPR;

    reg_t prv;
    reg_t prev_prv;
    bool prv_changed;
    bool v_changed;
    bool v;
    bool prev_v;

    bool debug_mode;

    bool serialized;

    commit_log_reg_t log_reg_write;
    commit_log_mem_t log_mem_read;
    commit_log_mem_t log_mem_write;
    reg_t last_inst_priv;
    int last_inst_xlen;
    int last_inst_flen;
};

template <typename T>
class DataBackup {
public:
    DataBackup() = default;

    bool IsEmpty() {
        return data_backup_.empty();
    }

    // When there is a branch, make a new memory entry
    void MakeBackupEntry() {
        assert(stable_);
        data_backup_.emplace_back(std::vector<T>());
    }

    // When making backup, push data into new memory entry
    void Push(T data_entry) {
        assert(stable_);
        data_backup_.back().push_back(data_entry);
    }

    // When a branch resolves with correct prediction, erase the oldest memory entry
    void Pop() {
        assert(stable_);
        data_backup_.pop_front();
    }

    // When a branch resolves with wrong prediction, pop all the data with a reverse iterator
    T GetBackupEntry() {
        stable_ = false;
        if (data_backup_.back().empty()) {
            data_backup_.pop_back();
            if (data_backup_.empty()) {
                stable_ = true;
            }
            return T();
        }
        T data_entry = data_backup_.back().back();
        data_backup_.back().pop_back();

        if (data_backup_.back().empty()) {
            data_backup_.pop_back();
        }

        if (data_backup_.empty()) {
            stable_ = true;
        }

        return data_entry;
    }

    bool getPredictionMiss() { return is_prediction_miss_; }

    void setPredictionMiss() { is_prediction_miss_ = true; }

    void clearPredictionMiss() { is_prediction_miss_ = false; }

private:
    /* when starting to get the backup entry, the whole Memory backup is unstable to push into new entry */
    bool stable_ = true;
    std::vector<T> data_entry_;
    std::deque<std::vector<T>> data_backup_;
    bool is_prediction_miss_ = false;
};


#endif //MODEL_DATABACKUP_HPP
