#ifndef TICK_HH_
#define TICK_HH_

#include <memory>

class Observer {
public:
    Observer();
    void update();
};

//Observer
class Clock {
public:
    static std::shared_ptr<Clock>& Instance();
    uint64_t CurTick() const;
    void Tick();
    void Reset();
    //Observer Interface
    void Attach(std::shared_ptr<Observer>);
    void Detach(std::shared_ptr<Observer>);
    void Notify();
private:
    Clock() ;
private:
    uint64_t cur_tick_;
    static std::shared_ptr<Clock> clock_;
};

#endif