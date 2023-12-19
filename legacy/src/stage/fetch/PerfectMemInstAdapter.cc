#include "PerfectMemInstAdapter.hh"

namespace Emulator {

    PerfectMemInstAdapter::PerfectMemInstAdapter(
            std::string name,
            PerfectMemory& perfect_memory
            ) : Trace::TraceObject(std::move(name)),
            perfect_memory_(perfect_memory) {}

    void PerfectMemInstAdapter::Reset() {
        perfect_memory_.Reset();
    }

    void PerfectMemInstAdapter::Process(Emulator::InstPkgPtr &inst_pkg_ptr) {
        for (auto i = inst_pkg_ptr->Begin(); i != inst_pkg_ptr->End(); ++i) {
//            (*i)->data->
        }
    }
}