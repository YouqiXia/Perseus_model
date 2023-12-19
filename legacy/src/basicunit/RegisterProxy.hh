#ifndef MODEL_REGISTERPROXY_HH
#define MODEL_REGISTERPROXY_HH

#include "basicunit/RegisterInterface.hh"
#include "basicunit/Register.hh"

namespace Emulator {

    class RegisterProxy : public RegisterInterface, public Trace::TraceObject {
    public:
        RegisterProxy(std::string, RegisterPtr &);

        ~RegisterProxy() {}

        void Reset();

        virtual void Process(InstPkgPtr &);

        virtual void Produce(InstPkgPtr &);

        virtual void SetPermission(InstPkgPtr &);

        virtual void Accept(InstPkgPtr &);

    private:
        RegisterPtr& register_ptr_;
    };

}

#endif //MODEL_REGISTERPROXY_HH
