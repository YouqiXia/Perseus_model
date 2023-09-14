#include "InstPkg.hh"

InstPkg::InstPkg() {
    Reset();
}

void InstPkg::Reset() {
    queue_.Reset();
}

uint64_t InstPkg::Size() {
    return queue_.Size();
}

void InstPkg::Transfer(InstPtr &inst_ptr) {
    InstPkgEntry inst_pkg_entry {false, inst_ptr};
    InstPkgEntryPtr inst_pkg_entry_ptr = std::shared_ptr<InstPkgEntry>();
    *inst_pkg_entry_ptr = inst_pkg_entry;
    queue_.Transfer(inst_pkg_entry_ptr);
}

std::vector<InstPkgEntryPtr>::iterator InstPkg::Begin() {
    return queue_.Begin();
}

std::vector<InstPkgEntryPtr>::iterator InstPkg::End() {
    return queue_.End();
}

std::vector<InstPkgEntryPtr>::iterator InstPkg::Delete(typename std::vector<InstPkgEntryPtr>::iterator &iterator) {
    return queue_.Delete(iterator);
}
