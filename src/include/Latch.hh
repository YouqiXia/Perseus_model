#ifndef LATCH_HH_
#define LATCH_HH_

#include <iostream>
#include "BaseDigital.hh"

template<typename T>
class Latch : public BaseDigital<T> {
public:
    Latch(
            std::shared_ptr <BaseDigital<T>> digital_unit
    ) : digital_unit_(digital_unit) {
        port_ = std::make_shared<port<T>>();
    }

    void Reset() {
        port_->Kill();
    }

    virtual bool IsReady() {
        return true;
    }

    virtual void Evaluate() {
        port_->Kill();
        if (!IsReady()) {
            digital_unit_->Stall();
            return;
        }
        if (digital_unit_->IsPortValid()) {
            port_->SetData(digital_unit_->GetPort()->GetData());
        }
    }

    bool IsPortValid() {
        return port_->IsValid();
    }

    std::shared_ptr<port<T>> &GetPort() {
        return port_;
    }

    // implementation for default
    virtual void Stall() { Error(); }

    virtual bool IsStall() {
        Error();
        return true;
    }

    virtual void Flush() { Error(); }

    virtual bool IsFlush() {
        Error();
        return true;
    }

    virtual bool IsDrained() {
        Error();
        return true;
    }

    virtual void Advance() {
        std::printf("Latch is advanced\n");
    }

private:
    void Error() {
        std::printf("no implement error occurs in Latch\n");
        assert(false);
    }

protected:
    std::shared_ptr <BaseDigital<T>> digital_unit_;
    std::shared_ptr <port<T>> port_;
};

#endif
