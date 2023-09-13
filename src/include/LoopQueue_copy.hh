#ifndef LOOPQUEUE_HH_
#define LOOPQUEUE_HH_

#include <vector>

template <typename T>
class LoopQueue
{
private:
    std::vector<T>  m_dataVec;
    uint64_t        m_header;
    uint64_t        m_tail;
    uint64_t        m_usage;
public:

    LoopQueue(uint64_t depth) : m_dataVec(depth) {
        Reset();
    };

    ~LoopQueue(){};

    uint64_t getHeader(){
        return this->m_header;
    };

    uint64_t getTail(){
        return this->m_tail;
    };

    uint64_t getLastest(){
        return this->getLastPtr(this->m_tail);
    };

    uint64_t getSize(){
        return this->m_dataVec.size();
    };

    uint64_t getUsage(){
        return this->m_usage;
    };

    uint64_t getAvailEntryCount(){
        return this->m_dataVec.size() - this->m_usage;
    };

    bool full(){
        return this->m_usage == this->m_dataVec.size();
    };

    bool empty(){
        return this->m_usage == 0;
    };

    void HeaderInc(){
        if(this->m_header == this->m_dataVec.size() - 1){
            this->m_header = 0;
        }else{
            this->m_header++;
        }
        this->m_usage--;
    };

    void TailInc(){
        if(this->m_tail == this->m_dataVec.size() - 1){
            this->m_tail = 0;
        }else{
            this->m_tail++;
        }
        this->m_usage++;
    };

    uint64_t  getNextPtr(const uint64_t CurPtr){
        if(CurPtr == this->m_dataVec.size() - 1){
            return 0;
        }else{
            return CurPtr+1;
        }
    };

    uint64_t  getLastPtr(const uint64_t CurPtr){
        if(CurPtr == 0){
            return this->m_dataVec.size() - 1;
        }else{
            return CurPtr-1;
        }
    };

    void Push(const T& data){
        DASSERT(!this->full(),"Push When Queue Full");
        this->m_dataVec[this->m_tail] = data;
        this->TailInc();
    };

    T Pop(){
        DASSERT(!this->empty(),"Pop When Queue Empty");
        T data = this->front();
        this->HeaderInc();
        return data;
    };

    void RollBack(){
        DASSERT(!this->empty(),"RollBack When Queue Empty");
        if(this->m_tail == 0){
            this->m_tail = this->m_dataVec.size() - 1;
        }else{
            this->m_tail--;
        }
        this->m_usage--;
    };

    uint64_t Allocate(){
        DASSERT(!this->full(),"Allocate When Queue Full");
        uint64_t ptr = this->m_tail;
        this->TailInc();
        return ptr;
    };

    T& operator[](uint64_t pos){
        return this->m_dataVec[pos];
    }

    T&  front(){
        return this->m_dataVec[m_header];
    };

    void Reset(){
        this->m_tail = 0;
        this->m_header = 0;
        this->m_usage = 0;

        uint64_t size = this->m_dataVec.size();
        this->m_dataVec.clear();
        this->m_dataVec.resize(size);
    };

    bool isOlder(uint64_t tag1, uint64_t tag2){
        bool tag1GeHeader = tag1 >= this->m_header;
        bool tag2GeHeader = tag2 >= this->m_header;
        bool tag1GeqTag2  = tag1 > tag2;
        return (tag1GeHeader ^ tag2GeHeader) ? tag1 > tag2 : tag1 < tag2;
    }

};

#endif // LOOPQUEUE_HH_