#ifndef MODEL_INSTBUFFER_HH
#define MODEL_INSTBUFFER_HH

#include "trace/TraceObject.hh"
#include "component/basic/LoopQueue.hh"
#include "common/DynInst.hh"

namespace Emulator {

    class InstBuffer : public Trace::TraceObject {
    public:

        struct InstBufferInterface {
            std::shared_ptr<bool> stall;
            enum {
                PUSH, POP
            } func_type;
            void *func_core_arg;
        };

        InstBuffer(
                std::string name,
                uint64_t depth,
                uint64_t cache_line_width
        );

        ~InstBuffer();

        const LoopQueue<CompressedInst_t> &GetInstBuffer() const;

        uint64_t GetCacheLineWidth() {return cache_line_width_;}

        void Reset();

        void Advance();

    /* outside interface for submitting event */
        void Push(std::shared_ptr<bool> stall, char* );

        void Pop(std::shared_ptr<bool> stall);

    /* not const function should be private */
    private:
        void Push(char* );

        void Pop();

    private:
        LoopQueue<CompressedInst_t> inst_buffer_;
        uint64_t cache_line_width_;
        std::vector<InstBufferInterface> event_queue_;
    };

}


#endif //MODEL_INSTBUFFER_HH
