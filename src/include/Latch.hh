#ifndef LATCH_HH_
#define LATCH_HH_

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
        if (digital_unit_->IsPortValid() && IsReady()) {
            port_->SetData(digital_unit_->GetPort()->GetData());
        } else {
            digital_unit_->Stall();
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

    virtual bool IsStall() { Error(); }

    virtual void Flush() { Error(); }

    virtual bool IsFlush() { Error(); }

    virtual bool IsDrained() { Error(); }

    virtual void Advance() { Error(); }

private:
    void Error() {
        printf("no implement error occurs in Latch");
        assert(false);
    }

protected:
    std::shared_ptr <BaseDigital<T>> digital_unit_;
    std::shared_ptr <port<T>> port_;
};

#endif
