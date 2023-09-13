#ifndef NULLDIGITALUNIT_HH_
#define NULLDIGITALUNIT_HH_

#include "BaseDigital.hh"

template<typename T>
class NullDigitalUnit : public BaseDigital<T> {
public:
    NullDigitalUnit() {
        port_ = std::make_shared<port<T>>();
    }

    void Reset() {}

    void Stall() {}

    bool IsStall() {
        return false;
    }

    void Flush() {}

    bool IsFlush() {
        return false;
    }

    bool IsDrained() {
        return true;
    }

    void Advance() {}

    std::shared_ptr<port<T>> &GetPort() {
        return port_;
    }

    bool IsReady() {
        return true;
    }

    void Evaluate() {}

    bool IsPortValid() {
        return true;
    }

private:
    std::shared_ptr<port<T>> port_;
};

#endif // NULLDIGITALUNIT_HH_