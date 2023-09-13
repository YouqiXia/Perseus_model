#include <memory>
#include <utility>

#include "basicunit/Stage.hh"
#include "DynInst.hh"

Stage::Stage(
        RegistersPtr input_queue,
        RegistersPtr output_queue
) : input_queue_(std::move(input_queue)),
    output_queue_(std::move(output_queue)) {}

void Stage::Evaluate() {
    InstPtr inst = nullptr;

    if (!IsValid()) {
        return;
    }

    // Process first, then check the permission
    Process(inst);

    SetPermission(inst);

    Accept(inst);

    Execute(inst);
}