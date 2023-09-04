#ifndef PROCESSOR_HH_
#define PROCESSOR_HH_

#include "FlipFlop.hh"
#include "Latch.hh"
#include "DynInsn.hh"
#include "NullDigitalUnit.hh"

typedef std::shared_ptr<BaseDigital<Dyninsn>> BaseSchedule;

class Processor {
public:
    Processor() {
        stall_ = std::make_shared<bool>();
        flush_ = std::make_shared<bool>();
        BaseSchedule NullDigital = std::make_shared<NullDigitalUnit<Dyninsn>>();
        for (int i = 0; i < 2; ++i) {
            BaseSchedule BasePipeReg = std::make_shared<FlipFlop<Dyninsn>>(stall_, flush_, NullDigital, 1);
        }
    }

private:
    std::vector<BaseSchedule> ScheduleComposition;
    std::shared_ptr<bool> stall_;
    std::shared_ptr<bool> flush_;
};


#endif