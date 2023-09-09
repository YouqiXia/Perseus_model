#include <memory>

#include "temp/Stage.hh"
#include "DynInsn.hh"

void Stage::Evaluate() {
    DynInsn insn;
    std::vector<Register<DynInsn>>::iterator i;

    // To see if all the data in Register is valid
    for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
        if (!i->IsValid()) {
            return;
        }
    }

    // Read data in Register and check the ctrl signal
    for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
        i->Process(insn);
    }

    // To see if some instruction is not permitted to operate
    if (Analyse(insn)) {
        for (i = process_queue_->begin(); i != process_queue_->end(); ++i) {
            i->Feedback(insn);
        }
        return;
    }

    // To see if all the Register is ready to be written into
    for (i = accept_queue_->begin(); i != accept_queue_->end(); ++i) {
        if (!i->IsReady()) {
            i ->Stall();
            return;
        }
    }

}