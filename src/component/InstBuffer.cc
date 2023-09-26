#include "InstBuffer.hh"

namespace Emulator {
    InstBuffer::InstBuffer(
            std::string name,
            uint64_t depth,
            uint64_t cache_line_width
    ) : Trace::TraceObject(std::move(name)),
        inst_buffer_(depth),
        cache_line_width_(cache_line_width) {}

    InstBuffer::~InstBuffer() {}

    void InstBuffer::Reset() {
        inst_buffer_.Reset();
        event_queue_.clear();
    }

    void InstBuffer::Advance() {
        for (auto i: event_queue_) {
            if (*i.stall) continue;
            switch (i.func_type) {
                case InstBufferInterface::POP :
                    Pop();
                    break;
                case InstBufferInterface::PUSH :
                    Push((char *) i.func_core_arg);
                    break;
                default:
                    break;
            }
        }
    }

    void InstBuffer::Push(char *memory_ptr) {
        for (uint64_t i = 0; i < cache_line_width_; i += 2) {
            CompressedInst_t compressed_inst_tmp = memory_ptr[i + 1] << 8 | memory_ptr[i];
            inst_buffer_.Push(compressed_inst_tmp);
        }
    }

    void InstBuffer::Pop() {
        inst_buffer_.Pop();
    }

    const LoopQueue<CompressedInst_t> &InstBuffer::GetInstBuffer() const {
        return inst_buffer_;
    }

    void InstBuffer::Push(std::shared_ptr<bool> stall, char *memory) {
        event_queue_.push_back({stall, InstBufferInterface::PUSH, memory});
    }

    void InstBuffer::Pop(std::shared_ptr<bool> stall) {
        event_queue_.push_back({stall, InstBufferInterface::POP, nullptr});
    }
}