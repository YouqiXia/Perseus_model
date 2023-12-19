#ifndef MODEL_PERFECTMEMORY_HH
#define MODEL_PERFECTMEMORY_HH

#include "basicunit/Register.hh"
#include "common/InstPkg.hh"
#include <string>

namespace Emulator {

    class PerfectMemory : public Register {
    public:
        PerfectMemory(
                std::string name,
                uint64_t base,
                uint64_t length);

        ~PerfectMemory();

        virtual void Process(InstPkgPtr &) override;

        void Accept(InstPkgPtr &) override;

        void Advance() override;

        // for IMemory Proxy
        void InstMemoryProcess(InstPkgPtr &);

        // for Dmemory Proxy
        void DataMemoryProcess(InstPkgPtr &);

    protected:
        // Inner function
        const uint64_t &Size();

        bool CheckRange(uint64_t address);

        void Read(uint64_t address, char **data);

        uint64_t ReadDouble(uint64_t address);

        char ReadByte(uint64_t address);

        void Write(uint64_t address, const char *data, const uint64_t len);

        void WriteDouble(uint64_t address, const uint64_t data);

        void Write(uint64_t address, const char *data, const uint64_t len, const uint64_t mask);

    private:
        // FIXME: Should ram_ be a vector?
        char *ram_;
        uint64_t base_;
        uint64_t length_;
    };

}
#endif //MODEL_PERFECTMEMORY_HH
