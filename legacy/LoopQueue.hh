#ifndef LOOPQUEUE_HH_
#define LOOPQUEUE_HH_

#include <vector>

template <typename T>
class LoopQueue {
    uint64_t getHeader();

    uint64_t getTail();

    uint64_t getLastest();

    uint64_t getSize();

    uint64_t getUsage();

    uint64_t getAvailEntryCount();

    bool full();

    bool empty();

    void HeaderInc();

    void TailInc();

    uint64_t  getNextPtr;

    uint64_t  getLastPtr;

    void Push(const T& data);

    T Pop();

    void RollBack();

    uint64_t Allocate();

    T& operator[](uint64_t pos);

    T&  front();

    void Reset();

    bool isOlder(uint64_t tag1, uint64_t tag2);
private:
    std::vector<T>  data_queue_;
    uint64_t        header_;
    uint64_t        tail_;
    uint64_t        usage_;
};

#endif // LOOPQUEUE_HH_