#include <memory>
#include <utility>

#include "Stage.hh"

Stage::Stage(
        RegistersPtr input_queue,
        RegistersPtr output_queue
) : input_queue_(std::move(input_queue)),
    output_queue_(std::move(output_queue)) {}

void Stage::Reset() {
    for (auto & i : *input_queue_) {
        i.Reset();
    }
    for (auto & i : *output_queue_) {
        i.Reset();
    }
    inst_pkg_ptr_->Reset();
}

void Stage::Evaluate() {

    // Process first, then check the permission
    Produce();

    if (IsNothingToDo()) return;

    Process();

    SetPermission();

    Accept();

    if (IsNothingToDo()) return;

    Execute();
}

bool Stage::IsNothingToDo() {
    // FIXME: 两个相同的if结构需要修改
    if (!inst_pkg_ptr_->Size())
        return true;
    for(auto i = inst_pkg_ptr_->Begin(); i != inst_pkg_ptr_->End();) {
        if ((*i)->stall) {
            i = inst_pkg_ptr_->Delete(i);
        } else {
            ++i;
        }
    }
    if (!inst_pkg_ptr_->Size())
        return true;
    return false;
}

void Stage::Produce() {
    for(auto i = input_queue_->begin(); i != input_queue_->begin(); ++i) {
        (*i).Produce(inst_pkg_ptr_);
    }
}

void Stage::Process() {
    for(auto i = input_queue_->begin(); i != input_queue_->begin(); ++i) {
        (*i).Process(inst_pkg_ptr_);
    }
}

void Stage::SetPermission() {
    for(auto i = input_queue_->begin(); i != input_queue_->begin(); ++i) {
        (*i).SetPermission(inst_pkg_ptr_);
    }
}

void Stage::Accept() {
    for(auto i = output_queue_->begin(); i != output_queue_->begin(); ++i) {
        (*i).Accept(inst_pkg_ptr_);
    }
}