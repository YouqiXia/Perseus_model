#include "Register.hh"

Register::Register() {
    Reset();
}

void Register::Reset() {
    tick_queue_.clear();
    write_queue_.clear();
}

void Register::SetTickQueue(InstPkgEntryPtr &inst_pkg_entry_ptr) {
    tick_queue_.push_back(inst_pkg_entry_ptr);
}

void Register::SetWriteQueue(InstPkgEntryPtr &inst_pkg_entry_ptr) {
    write_queue_.push_back(inst_pkg_entry_ptr);
}

InstPkgEntryPtr &Register::GetTickQueue() {
    InstPkgEntryPtr& tmp_inst_pkg_entry_ptr = tick_queue_.back();
    tick_queue_.pop_back();
    return tmp_inst_pkg_entry_ptr;
}

InstPkgEntryPtr &Register::GetWriteQueue() {
    InstPkgEntryPtr& tmp_inst_pkg_entry_ptr = write_queue_.back();
    write_queue_.pop_back();
    return tmp_inst_pkg_entry_ptr;
}

uint64_t Register::GetTickQueueSize() {
    return tick_queue_.size();
}

uint64_t Register::GetWriteQueueSize() {
    return write_queue_.size();
}
