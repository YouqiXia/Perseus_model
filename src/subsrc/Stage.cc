#include <memory>

#include "temp/Stage.hh"
#include "DynInsn.hh"

void Stage::Evaluate() {
    DynInsn insn;

    if (!IsValid()) {
        return;
    }

    // Process first, then check the permission
    Process(insn);

    if (IsPermitted(insn)) {
        Feedback(insn);
        return;
    }

    if (!IsReady()) {
        return;
    }

    Strategy(insn);
}

bool Stage::IsValid() const {
    std::vector<Register<DynInsn>>::iterator i;
    for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
        if (!i->IsValid()) {
            return false;
        }
    }
    return true;
}

bool Stage::IsReady() const {
    std::vector<Register<DynInsn>>::iterator i;
    for (i = accept_queue_->begin(); i != accept_queue_->end(); ++i) {
        if (!i->IsReady()) {
            i->Stall();
            return false;
        }
    }
    return true;
}

void Stage::Process(DynInsn &insn) const {
    std::vector<Register<DynInsn>>::iterator i;
    for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
        i->Process(insn);
    }
}

void Stage::Feedback(DynInsn &insn) const {
    std::vector<Register<DynInsn>>::iterator i;
    for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
        i->Feedback(insn);
    }
}