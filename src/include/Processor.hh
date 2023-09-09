#ifndef PROCESSOR_HH_
#define PROCESSOR_HH_

#include "basicunit/FlipFlop.hh"
#include "basicunit/Latch.hh"
#include "DynInsn.hh"
#include "basicunit/NullDigitalUnit.hh"

typedef std::shared_ptr<BaseDigital<DynInsn>> BaseSchedule;

class Processor {
public:
    explicit Processor(uint32_t max_scheduler_num) :
            schedule_composition_(max_scheduler_num),
            max_scheduler_num_(max_scheduler_num) {
        stall_ = std::make_shared<bool>();
        flush_ = std::make_shared<bool>();
        NullDigital_ = std::make_shared<NullDigitalUnit<DynInsn>>();
        // iterator is needed here
        for (int i = 0; i < max_scheduler_num_; i += 2) {
            schedule_composition_[i] = std::make_shared<FlipFlop<DynInsn>>(
                    stall_,
                    flush_,
                    (!i) ? NullDigital_ : schedule_composition_[i - 1],
                    1
            );
            schedule_composition_[i + 1] = std::make_shared<Latch<DynInsn>>(schedule_composition_[i]);
        }
    }

    void Advance() {
        for (auto &scheduler: schedule_composition_) {
            scheduler->Advance();
        }
    }

    void Evaluate() {
        for (auto &scheduler: schedule_composition_) {
            scheduler->Evaluate();
        }
    }

    void Stall() {
        *stall_ = true;
    }

    void Flush() {
        *flush_ = true;
    }

    void ClearFlush() {
        *flush_ = false;
    }

    bool IsFlush() {
        return *flush_;
    }

    bool IsStall() {
        return *stall_;
    }

    void SetInsn(DynInsn &insn) {
        NullDigital_->GetPort()->SetData(insn);
    }

    void Reset() {
        *stall_ = false;
        *flush_ = false;
        for (auto &scheduler: schedule_composition_) {
            scheduler->Reset();
        }
    }

private:
    std::vector<BaseSchedule> schedule_composition_;
    BaseSchedule NullDigital_;
    uint32_t max_scheduler_num_;
    std::shared_ptr<bool> stall_;
    std::shared_ptr<bool> flush_;
};


#endif