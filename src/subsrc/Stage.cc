#include <memory>
#include <utility>

#include "temp/Stage.hh"
#include "DynInst.hh"

Stage::Stage(
        RegistersPtr input_queue,
        RegistersPtr output_queue
) : input_queue_(std::move(input_queue)),
    output_queue_(std::move(output_queue)) {}

void Stage::Evaluate() {
    DynInst inst = nullptr;

    if (!IsValid()) {
        return;
    }

    // Process first, then check the permission
    Process(inst);

    SetPermission(inst);

    RequestReady(inst);

    Execute(inst);

    Accept(inst);
}

bool Stage::IsValid() const {
    for (auto i = input_queue_->begin(); i != input_queue_->end(); ++i) {
        if (!i->IsValid()) {
            return false;
        }
    }
    return true;
}

void Stage::RequestReady(DynInst &inst) const {
    for (auto i = output_queue_->begin(); i != output_queue_->end(); ++i) {
        i->RequestReady(inst);
    }
}

void Stage::Process(DynInst &inst) const {
    bool traversal_completed = false;
    while (!traversal_completed) {
        traversal_completed = true;
        for (auto i = input_queue_->begin(); i != input_queue_->end(); ++i) {
            i->Process(inst);
            // TODO: there may some problem while assigning nullptr to inst
            if (inst == nullptr) {
                traversal_completed = false;
            }
            // TODO: if all process operation return nullptr, there may be an assertion
        }
    }
}

void Stage::Accept(DynInst &inst) {
    for (auto i = output_queue_->begin(); i != output_queue_->end(); ++i) {
        i->Accept(inst);
    }
}

void Stage::SetPermission(DynInst &inst) const {
    for (auto i = input_queue_->begin(); i != input_queue_->end(); ++i) {
        i->SetPermission(inst);
    }
}

void Stage::Execute(DynInst &) const {

}