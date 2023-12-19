#ifndef MODEL_INSTBUFFERFETCHADAPTER_HH
#define MODEL_INSTBUFFERFETCHADAPTER_HH

#include "component/InstBuffer.hh"
#include "basicunit/RegisterInterface.hh"
#include "trace/TraceObject.hh"

namespace Emulator {

    class InstBufferFetchAdapter : public RegisterInterface, public Trace::TraceObject {
    public:
        InstBufferFetchAdapter(std::string name, InstBuffer &inst_buffer);

        ~InstBufferFetchAdapter() {}

        void Reset() override;

        void Produce(InstPkgPtr) override {}

        // generate instruction and judge if it is compressed instruction
        void Process(InstPkgPtr) override;

        void SetPermission(InstPkgPtr) override {}

        void Accept(InstPkgPtr) override {}

        void Advance() override {}

    private:
        InstBuffer &inst_buffer_;
    };

}


#endif //MODEL_INSTBUFFERFETCHADAPTER_HH
