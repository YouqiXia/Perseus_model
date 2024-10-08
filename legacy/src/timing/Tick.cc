#include "Tick.hh"

namespace Emulator {
    Clock::Clock() : cur_tick_(0) {}

    std::shared_ptr<Clock> Clock::clock_ = nullptr;

    std::shared_ptr<Clock> &Clock::Instance() {
        struct ClockProxy : public Clock {
        };
        if (clock_ == nullptr) {
            clock_ = std::make_shared<ClockProxy>();
        }
        return clock_;
    }

    uint64_t Clock::CurTick() const {
        return cur_tick_;
    }

    void Clock::Tick() {
        cur_tick_++;
    }

    void Clock::Reset() {
        cur_tick_ = 0;
    }
}