//
// Created by yzhang on 1/23/24.
//

#ifndef MODEL_BASEMEMORY_HPP
#define MODEL_BASEMEMORY_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <cassert>

namespace TimingModel {

    class BaseMemory {
    private:
        uint64_t base_address_;
        uint64_t length_;
        char* memory_;

    public:
        explicit BaseMemory(uint64_t base_address);

        BaseMemory(uint64_t base_address, uint64_t length);

        ~BaseMemory();

        uint64_t size();

        bool isInRange(uint64_t address);

        void Read(uint64_t address, char** data, uint64_t len);

        uint64_t ReadDouble(uint64_t address);

        uint32_t ReadWord(uint64_t address);

        uint16_t ReadHalf(uint64_t address);

        uint8_t ReadByte(uint64_t address);

        void Write(uint64_t address, const char* data, uint64_t len);

        void WriteDouble(uint64_t address, uint64_t data);

        void WriteWord(uint64_t address, uint32_t data);

        void WriteHalf(uint64_t address, uint16_t data);

        void WriteByte(uint64_t address, uint8_t data);

    };

} // TimingModel

#endif //MODEL_BASEMEMORY_HPP
