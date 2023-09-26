#ifndef BASEUNIT_HH_
#define BASEUNIT_HH_

#include <iostream>
#include <cassert>
#include <queue>

#include "BaseDigital.hh"
#include "timing/Tick.hh"

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
    );

    void Reset();

    void Stall();

    bool IsStall();

    void Flush();

    bool IsFlush();

    bool IsDrained();

    void Advance();

    bool IsPortValid();

    std::shared_ptr<port<T>> &GetPort();

    // implementation for default
    virtual bool IsReady();

    virtual void Evaluate();

private:
    void Error();

private:
    std::shared_ptr<TimeBufferQueue> data_queue_;
    std::shared_ptr<bool> stall_;
    std::shared_ptr<bool> flush_;
    std::shared_ptr<port<T>> port_;
    std::shared_ptr<BaseDigital<T>> digital_unit_;
    const uint64_t latency_;
};

template<typename T>
FlipFlop<T>::FlipFlop(
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

template<typename T>
void FlipFlop<T>::Reset() {
    while (!data_queue_->empty()) {
        data_queue_->pop();
    }
    port_->Kill();
}

template<typename T>
void FlipFlop<T>::Stall() {
    *stall_ = true;
}

template<typename T>
bool FlipFlop<T>::IsStall() {
    return *stall_;
}

template<typename T>
void FlipFlop<T>::Flush() {
    *flush_ = true;
}

template<typename T>
bool FlipFlop<T>::IsFlush() {
    return *flush_;
}

template<typename T>
bool FlipFlop<T>::IsDrained() {
    return data_queue_->empty() && !port_->IsValid();
}

template<typename T>
void FlipFlop<T>::Advance() {
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
    if (data_queue_->empty()) {
        return;
    }
    if (data_queue_->front().desk_tick <= Clock::Instance()->CurTick()) {
        port_->SetData(data_queue_->front().data);
        data_queue_->pop();
    }
}

template<typename T>
bool FlipFlop<T>::IsPortValid() {
    return port_->IsValid();
}

template<typename T>
std::shared_ptr<port<T>> &FlipFlop<T>::GetPort() {
    return port_;
}

// implementation for default
template<typename T>
bool FlipFlop<T>::IsReady() {
    printf("%s", __func__);
    Error();
    return true;
}

template<typename T>
void FlipFlop<T>::Evaluate() {
    std::printf("FlipFlop is evaluated\n");
}

template<typename T>
void FlipFlop<T>::Error() {
    std::printf("no implement error occurs in FlipFlop\n");
    assert(false);
}

#endif // BASEUNIT_HH_