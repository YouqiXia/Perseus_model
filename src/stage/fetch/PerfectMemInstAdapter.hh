#ifndef MODEL_PERFECTMEMADAPTER_HH
#define MODEL_PERFECTMEMADAPTER_HH

#include "component/PerfectMemory/PerfectMemory.hh"
#include "basicunit/RegisterInterface.hh"
#include "trace/TraceObject.hh"

namespace Emulator {

    class PerfectMemInstAdapter : public RegisterInterface, public Trace::TraceObject{
    public:
        PerfectMemInstAdapter(std::string, PerfectMemory&);

        ~PerfectMemInstAdapter();

        void Reset() override;

        void Produce(InstPkgPtr) override {}

        void Process(InstPkgPtr) override;

        void SetPermission(InstPkgPtr) override {}

        void Accept(InstPkgPtr) override {}

        void Advance() override {}

    private:
        PerfectMemory& perfect_memory_;
    };

}

#endif //MODEL_PERFECTMEMADAPTER_HH
