//
// Created by yzhang on 2/29/24.
//

#ifndef MODEL_MEMORYBACKUP_HPP
#define MODEL_MEMORYBACKUP_HPP

#include <vector>
#include <deque>
#include <cassert>
#include <cstdint>

class MemoryBackup {
public:
    struct MemoryEntry{
        uint64_t addr;
        uint64_t data;
        uint64_t len;
    };

public:
    MemoryBackup() = default;

    bool IsEmpty();

    // When there is a branch, make a new memory entry
    void MakeMemoryEntry();

    // When making backup, push data into new memory entry
    void Push(MemoryEntry);

    // When a branch resolves with correct prediction, erase the oldest memory entry
    void Pop();

    // When a branch resolves with wrong prediction, pop all the data with a reverse iterator
    MemoryEntry GetBackupEntry();

private:
    /* when starting to get the backup entry, the whole Memory backup is unstable to push into new entry */
    bool stable_ = true;
    std::vector<MemoryEntry> memory_entry_;
    std::deque<std::vector<MemoryEntry>> memory_backup_;
};


#endif //MODEL_MEMORYBACKUP_HPP
