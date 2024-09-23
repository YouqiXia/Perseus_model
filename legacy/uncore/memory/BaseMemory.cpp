//
// Created by yzhang on 1/23/24.
//

#include "BaseMemory.hpp"

namespace TimingModel {

    BaseMemory::BaseMemory(uint64_t base_address) :
        base_address_(base_address)
    {}


    BaseMemory::BaseMemory(uint64_t base_address, uint64_t length) :
        base_address_(base_address),
        length_(length)
    {
        memory_ = new char[length];
    }

    BaseMemory::~BaseMemory() {
        delete memory_;
    }

    uint64_t BaseMemory::size() {
        return length_;
    }

    bool BaseMemory::isInRange(uint64_t address) {
        if (address > base_address_ && address < base_address_ + length_) {
            return true;
        }

        return false;
    }

    void BaseMemory::Read(uint64_t address, char **data, uint64_t len) {
        assert(isInRange(address));
        *data = memory_ + (address - base_address_);
    }

    uint64_t BaseMemory::ReadDouble(uint64_t address) {
        assert(isInRange(address));
        return * reinterpret_cast<uint64_t*> (memory_ + (address - base_address_));
    }

    uint32_t BaseMemory::ReadWord(uint64_t address) {
        assert(isInRange(address));
        return * reinterpret_cast<uint32_t*> (memory_ + (address - base_address_));
    }

    uint16_t BaseMemory::ReadHalf(uint64_t address) {
        assert(isInRange(address));
        return * reinterpret_cast<uint16_t*> (memory_ + (address - base_address_));
    }

    uint8_t BaseMemory::ReadByte(uint64_t address) {
        assert(isInRange(address));
        return * reinterpret_cast<uint8_t*> (memory_ + (address - base_address_));
    }

    void BaseMemory::Write(uint64_t address, const char* data, uint64_t len) {
        assert(isInRange(address));
        std::copy(data, data + len, memory_ + address - base_address_);
    }

    void BaseMemory::WriteDouble(uint64_t address, uint64_t data) {
        assert(isInRange(address));
        std::copy(&data, &data + 64 / 8, memory_ + address - base_address_);
    }

    void BaseMemory::WriteWord(uint64_t address, uint32_t data) {
        assert(isInRange(address));
        std::copy(&data, &data + 32 / 8, memory_ + address - base_address_);
    }

    void BaseMemory::WriteHalf(uint64_t address, uint16_t data) {
        assert(isInRange(address));
        std::copy(&data, &data + 16 / 8, memory_ + address - base_address_);
    }

    void BaseMemory::WriteByte(uint64_t address, uint8_t data) {
        assert(isInRange(address));
        std::copy(&data, &data + 8 / 8, memory_ + address - base_address_);
    }


} // TimingModel