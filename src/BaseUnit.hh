#ifndef BASEUNIT_HH_
#define BASEUNIT_HH_

#include <memory>
#include <queue>

#include"BaseStage.hh"

template <typename T>
class port {
public:
    port() {
        _valid = false;
    }
    bool IsValid() {
        return _valid;
    }
    bool Kill() {
        _valid = false;
    }
    void SetData(T data) {
        _valid  = true;
        _data   = data;
    }
private:
    bool _valid;
    T _data;
};

template <typename T>
class timebuffer : public BaseTiming<T> {
public:
    struct timebuf_entry_t {
        T           data;
        uint64_t    DeskTick;
    };
    typedef std::queue<timebuf_entry_t> timebuf_t;
    timebuffer(
        std::shared_ptr<bool>& stall,
        std::shared_ptr<bool>& flush,
        uint64_t latency
    ) : _stall(stall), 
        _flush(flush), 
        _latency(latency) 
    {    
        _dataqueue = std::make_shared<timebuf_t>();
        _port = std::make_shared<port>();
    }
    void Reset() {
        _dataqueue->reset();
        _port->Kill();
    }
    void Stall() {
        _stall = true;
    }
    bool IsStall() {
        return _stall;
    }
    void Flush() {
        _flush = true;
    }
    bool IsFlush() {
        return _flush;
    }
    bool IsDrained() {
        return _dataqueue->empty() && !_port->IsValid();
    }
private:
    std::shared_ptr<timebuf_t> _dataqueue;
    std::shared_ptr<bool> _stall;
    std::shared_ptr<bool> _flush;
    std::shared_ptr<port> _port;
    uint64_t _latency;
};

template <typename T>
class BaseStage {
public:
    bool IsReady() {
        return baseCombination->IsReady();
    }
    bool IsValid() {
        return baseTiming->IsValid();
    }
    void Evaluate() {
        if (this->IsReady() && preStage->IsValid()) {
            baseCombination->Evaluate();
        }
    };
    void Advance() {
        if (!baseTiming->IsStall()) {
            baseTiming->Advance();
        }
    }
private:
    std::shared_ptr<BaseTiming> baseTiming;
    std::shared_ptr<BaseCombination> baseCombination;
    std::shared_ptr<BaseStage> preStage; //使用链表来管理stage
};

#endif // BASEUNIT_HH_