#include "RegisterProxy.hh"

namespace Emulator {

    RegisterProxy::RegisterProxy(std::string name, RegisterPtr &register_ptr)
            : Trace::TraceObject(std::move(name)),
              register_ptr_(register_ptr) {}

    void RegisterProxy::Reset() {
        register_ptr_->Reset();
    }

    void RegisterProxy::Process(Emulator::InstPkgPtr &inst_pkg_ptr) {
        register_ptr_->Process(inst_pkg_ptr);
    }

    void RegisterProxy::Produce(Emulator::InstPkgPtr &inst_pkg_ptr) {
        register_ptr_->Produce(inst_pkg_ptr);
    }

    void RegisterProxy::SetPermission(Emulator::InstPkgPtr &inst_pkg_ptr) {
        register_ptr_->SetPermission(inst_pkg_ptr);
    }

    void RegisterProxy::Accept(Emulator::InstPkgPtr &inst_pkg_ptr) {
        register_ptr_->Accept(inst_pkg_ptr);
    }
}