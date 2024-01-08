#pragma once

namespace TimingModel {
    enum class MshrStatus: std::uint8_t {
        NO_TYPE = 0,
        __FIRST = NO_TYPE,
        ALLOCATE,
        SEND_REQ,
        RECV_RESP,
        EVICT,
        REFILL,
        SEND_RESP,
        NUM_TYPES,
        __LAST = NUM_TYPES
    };
    class MSHREntry{
    public:
        uint64_t addr;
        MemAccInfoPtr req;
        MshrStatus status;
        bool valid;
        MSHREntry(){
            addr = 0;
            status = MshrStatus::NO_TYPE;
            valid = false;
        }
    };

    class MSHR{
    public:
        std::vector<MSHREntry> queue;
        uint32_t qsize;
        uint32_t used;
        uint32_t header;
        uint32_t tail;
    public:
        void resize(uint32_t size){
            qsize = size;
            queue.resize(qsize);
            used = 0;
            header = 0;
            tail = 0;
        }
        bool isAvail(){
            return (used != qsize);
        }

        uint32_t allocate(){
            sparta_assert((qsize != used));
            uint32_t allocated = tail;
            used++;
            tail += 1;
            if(tail == qsize)
                tail = 0;
            return allocated;
        }
        void deallocate(uint32_t id){
            sparta_assert((id < qsize));
            used--;
            header += 1;
            if(header == qsize){
                header = 0;
            }
            queue[id].status = MshrStatus::NO_TYPE;
            return;
        }
        
        MSHREntry& getHeader(){
            return queue[header];
        }

        void recvResp(uint32_t id){
            sparta_assert((queue[id].status == MshrStatus::SEND_REQ));
            queue[id].status = MshrStatus::RECV_RESP; 
        }
        bool isEmpty(){ return (used == 0); }

    };
} // namespace TimingModel