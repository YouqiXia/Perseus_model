#include "PerfectMemory.hh"
#include "trace/Logging.hh"

namespace Emulator {
    PerfectMemory::PerfectMemory(
            std::string name,
            uint64_t base,
            uint64_t length
    ) : Register(std::move(name)),
        base_(base),
        length_(length) {
        ram_ = new char[length];
    }

    PerfectMemory::~PerfectMemory() {
        delete[] ram_;
    }

    void PerfectMemory::Process(Emulator::InstPkgPtr &) {}

    void PerfectMemory::Accept(Emulator::InstPkgPtr &) {}

    void PerfectMemory::Advance() {}

    void
    PerfectMemory::Read(uint64_t address, char **data) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        *data = ram_ + (address - base_);
    }

    bool
    PerfectMemory::CheckRange(uint64_t address) {
        return (address >= base_) && (address <= base_ + length_);
    }

    uint64_t
    PerfectMemory::ReadDouble(uint64_t address) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        return *(uint64_t *) (ram_ + (address - base_));
    }

    char
    PerfectMemory::ReadByte(uint64_t address) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        return *(ram_ + (address - base_));
    }

    void
    PerfectMemory::Write(uint64_t address, const char *data, const uint64_t len) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        std::copy(data, data + len, ram_ + address - base_);
    }

    void
    PerfectMemory::WriteDouble(uint64_t address, const uint64_t data) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        std::copy(&data, &data + 8, ram_ + address - base_);
    }

    void
    PerfectMemory::Write(uint64_t address, const char *data, const uint64_t len, const uint64_t mask) {
        DASSERT(((address >= base_) && (address <= base_ + length_)), "Access Unknown Address : {:#x}", address);
        for (size_t i = 0; i < len; i++) {
            if ((mask >> i) & 1) {
                ram_[address - base_ + i] = data[i];
            }
        }
    }


    const uint64_t &
    PerfectMemory::Size() {
        return length_;
    }

}