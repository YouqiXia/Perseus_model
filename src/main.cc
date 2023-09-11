#include "Processor.hh"
#include "Tick.hh"

int main() {
    Processor only_processor(10);
    only_processor.Reset();
    Clock::Instance()->Reset();
    DynInst instruction = std::make_shared<InstData>();
    instruction->count = 0;
    while(1) {
        instruction->count++;
        only_processor.SetInst(instruction);
        Clock::Instance()->Tick();
        only_processor.Advance();
        only_processor.Evaluate();
        if (Clock::Instance()->CurTick() >= 10 && Clock::Instance()->CurTick() <= 20) {
            only_processor.Stall();
        }
        if (Clock::Instance()->CurTick() == 5) {
            only_processor.Flush();
        } else {
            only_processor.ClearFlush();
        }
    }
}