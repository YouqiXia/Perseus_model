#ifndef LATCH_HH_
#define LATCH_HH_

#include <iostream>
#include "BaseDigital.hh"

template<typename T>
class Latch : public BaseDigital<T> {
public:
    explicit Latch(
            std::shared_ptr <BaseDigital<T>> digital_unit
    );

    void Reset();

    virtual bool IsReady();

    virtual void Evaluate();

    bool IsPortValid();

    std::shared_ptr<port<T>> &GetPort();

    // implementation for default
    virtual void Stall();

    virtual bool IsStall();

    virtual void Flush();

    virtual bool IsFlush();

    virtual bool IsDrained();

    virtual void Advance();

private:
    void Error();

protected:
    std::shared_ptr <BaseDigital<T>> digital_unit_;
    std::shared_ptr <port<T>> port_;
};

template<typename T>
Latch<T>::Latch(
        std::shared_ptr <BaseDigital<T>> digital_unit
) : digital_unit_(digital_unit) {
    port_ = std::make_shared<port<T>>();
}

template<typename T>
void Latch<T>::Reset() {
    port_->Kill();
}

template<typename T>
bool Latch<T>::IsReady() {
    return true;
}

template<typename T>
void Latch<T>::Evaluate() {
    port_->Kill();
    if (!IsReady()) {
        digital_unit_->Stall();
        return;
    }
    if (digital_unit_->IsPortValid()) {
        port_->SetData(digital_unit_->GetPort()->GetData());
    }
}

template<typename T>
bool Latch<T>::IsPortValid() {
    return port_->IsValid();
}

template<typename T>
std::shared_ptr<port<T>> &Latch<T>::GetPort() {
    return port_;
}

// implementation for default
template<typename T>
void Latch<T>::Stall() { Error(); }

template<typename T>
bool Latch<T>::IsStall() {
    Error();
    return true;
}

template<typename T>
void Latch<T>::Flush() { Error(); }

template<typename T>
bool Latch<T>::IsFlush() {
    Error();
    return true;
}

template<typename T>
bool Latch<T>::IsDrained() {
    Error();
    return true;
}

template<typename T>
void Latch<T>::Advance() {
    std::printf("Latch is advanced\n");
}

template<typename T>
void Latch<T>::Error() {
    std::printf("no implement error occurs in Latch\n");
    assert(false);
}

#endif
