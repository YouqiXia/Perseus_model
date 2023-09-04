#ifndef BASEUNIT_HH_
#define BASEUNIT_HH_

#include <iostream>
#include <cassert>
#include <queue>

#include "BaseDigital.hh"
#include "Tick.hh"

template<typename T>
class FlipFlop : public BaseDigital<T> {
public:
    struct TimeBufferEntry {
        T data;
        uint64_t desk_tick;
    };
    typedef std::queue<TimeBufferEntry> TimeBufferQueue;

    FlipFlop(
            std::shared_ptr<bool> &stall,
            std::shared_ptr<bool> &flush,
            std::shared_ptr<BaseDigital<T>> &digital_unit,
            uint64_t latency
    ) : stall_(stall),
        flush_(flush),
        digital_unit_(digital_unit),
        latency_(latency) {
        data_queue_ = std::make_shared<TimeBufferQueue>();
        port_ = std::make_shared<port<T>>();
        if (!latency) { Error(); }
    }

    void Reset() {
        while (!data_queue_->empty()) {
            data_queue_->pop();
        }
        port_->Kill();
    }

    void Stall() {
        *stall_ = true;
    }

    bool IsStall() {
        return *stall_;
    }

    void Flush() {
        *flush_ = true;
    }

    bool IsFlush() {
        return *flush_;
    }

    bool IsDrained() {
        return data_queue_->empty() && !port_->IsValid();
    }

    void Advance() {
        if (*flush_) {
            Reset();
            return;
        }
        if (*stall_) {
            *stall_ = false;
            return;
        }
        // put port data of pre-stage into data queue
        if (digital_unit_->IsPortValid()) {
            data_queue_->push({digital_unit_->GetPort()->GetData(), Clock::Instance()->CurTick() + latency_ - 1});
        }
        port_->Kill();
        if (data_queue_->front().desk_tick <= Clock::CurTick() && !data_queue_->empty()) {
            port_->SetData(digital_unit_->GetPort()->GetData());
        }
    }

    std::shared_ptr<port<T>> &GetPort() {
        return port_;
    }

    // implementation for default
    virtual bool IsReady() {
        printf("%s", __func__);
        Error();
    }

    virtual void Evaluate() { Error(); }

    virtual bool IsPortValid() { Error(); }

private:
    void Error() {
        std::printf("no implement error occurs in FlipFlop");
        assert(false);
    }

private:
    std::shared_ptr<TimeBufferQueue> data_queue_;
    std::shared_ptr<bool> stall_;
    std::shared_ptr<bool> flush_;
    std::shared_ptr<port<T>> port_;
    std::shared_ptr<BaseDigital<T>> digital_unit_;
    const uint64_t latency_;
};

#endif // BASEUNIT_HH_