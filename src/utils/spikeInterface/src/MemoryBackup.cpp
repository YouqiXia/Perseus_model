//
// Created by yzhang on 2/29/24.
//

#include "MemoryBackup.hpp"

bool MemoryBackup::IsEmpty() {
    return memory_backup_.empty();
}

MemoryBackup::MemoryEntry MemoryBackup::GetBackupEntry() {
    stable_ = false;
    if (memory_backup_.back().empty()) {
        memory_backup_.pop_back();
        if (memory_backup_.empty()) {
            stable_ = true;
        }
        return MemoryEntry{0,0,0};
    }
    MemoryEntry memory_entry = memory_backup_.back().back();
    memory_backup_.back().pop_back();

    if (memory_backup_.back().empty()) {
        memory_backup_.pop_back();
    }

    if (memory_backup_.empty()) {
        stable_ = true;
    }

    return memory_entry;
}

void MemoryBackup::MakeMemoryEntry() {
    assert(stable_);
    memory_backup_.emplace_back(std::vector<MemoryEntry>());
}

void MemoryBackup::Push(MemoryBackup::MemoryEntry memory_entry) {
    assert(stable_);
    memory_backup_.back().push_back(memory_entry);
}

void MemoryBackup::Pop() {
    assert(stable_);
    memory_backup_.pop_front();
}