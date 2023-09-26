#include <utility>

#include "InstPkg.hh"

namespace Emulator {

    InstPkg::InstPkg() {
        Reset();
    }

    void InstPkg::Reset() {
        queue_.Reset();
    }

    uint64_t InstPkg::Size() {
        return queue_.Size();
    }

    void InstPkg::Transfer(InstPtr inst_ptr) {
        InstPkgEntry inst_pkg_entry{std::make_shared<bool>(false), std::move(inst_ptr)};
        queue_.Transfer(inst_pkg_entry);
    }

    std::vector<InstPkgEntry>::iterator InstPkg::Begin() {
        return queue_.Begin();
    }

    std::vector<InstPkgEntry>::iterator InstPkg::End() {
        return queue_.End();
    }

    std::vector<InstPkgEntry>::iterator InstPkg::Delete(typename std::vector<InstPkgEntry>::iterator iterator) {
        return queue_.Delete(iterator);
    }
}
