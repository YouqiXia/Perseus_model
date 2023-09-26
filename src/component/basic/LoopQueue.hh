#ifndef LOOPQUEUE_HH_
#define LOOPQUEUE_HH_

#include <vector>
#include "trace/Logging.hh"

template<typename T>
class LoopQueue {
public:
    LoopQueue(uint64_t depth);

    ~LoopQueue() {}

    uint64_t GetHeader() const;

    uint64_t GetTail() const;

    uint64_t GetSize() const;

    uint64_t GetUsage() const;

    uint64_t GetAvailEntryCount() const;

    bool IsFull() const;

    bool IsEmpty() const;

    void HeaderInc();

    void TailInc();

    void Push(const T &data);

    T Pop();

    uint64_t Allocate();

    void RollBack();

    uint64_t GetNextPtr(const uint64_t CurPtr) const;

    uint64_t GetLastPtr(const uint64_t CurPtr) const;

    T &operator[](uint64_t pos) const;

    T &GetFront() const;

    void Reset();

    bool IsOlder(uint64_t tag1, uint64_t tag2) const;

private:
    std::vector<T> data_queue_;
    uint64_t header_;
    uint64_t tail_;
    uint64_t usage_;
};

template<typename T>
LoopQueue<T>::LoopQueue(uint64_t depth) : data_queue_(depth) {
    Reset();
}

template<typename T>
uint64_t LoopQueue<T>::GetHeader() const {
    return header_;
}

template<typename T>
uint64_t LoopQueue<T>::GetTail() const {
    return tail_;
}

template<typename T>
uint64_t LoopQueue<T>::GetSize() const {
    return data_queue_.size();
}

template<typename T>
uint64_t LoopQueue<T>::GetUsage() const {
    return usage_;
}

template<typename T>
uint64_t LoopQueue<T>::GetAvailEntryCount() const {
    return data_queue_.size() - usage_;
}

template<typename T>
bool LoopQueue<T>::IsFull() const {
    return usage_ == data_queue_.size();
}

template<typename T>
bool LoopQueue<T>::IsEmpty() const {
    return usage_ == 0;
}

template<typename T>
void LoopQueue<T>::HeaderInc() {
    if (header_ == data_queue_.size() - 1) {
        header_ = 0;
    } else {
        header_++;
    }
    usage_--;
}

template<typename T>
void LoopQueue<T>::TailInc() {
    if (tail_ == data_queue_.size() - 1) {
        tail_ = 0;
    } else {
        tail_++;
    }
    usage_++;
}

template<typename T>
uint64_t LoopQueue<T>::GetNextPtr(const uint64_t CurPtr) const {
    if (CurPtr == data_queue_.size() - 1) {
        return 0;
    } else {
        return CurPtr + 1;
    }
}

template<typename T>
uint64_t LoopQueue<T>::GetLastPtr(const uint64_t CurPtr) const {
    if (CurPtr == 0) {
        return data_queue_.size() - 1;
    } else {
        return CurPtr - 1;
    }
}

template<typename T>
void LoopQueue<T>::Push(const T &data) {
    DASSERT(!IsFull(), "Push When Queue Full");
    data_queue_[tail_] = data;
    TailInc();
}

template<typename T>
T LoopQueue<T>::Pop() {
    DASSERT(!IsEmpty(), "Pop When Queue Empty");
    T data = GetFront();
    HeaderInc();
    return data;
}

template<typename T>
void LoopQueue<T>::RollBack() {
    DASSERT(!IsEmpty(), "RollBack When Queue Empty");
    if (tail_ == 0) {
        tail_ = data_queue_.size() - 1;
    } else {
        tail_--;
    }
    usage_--;
}

template<typename T>
uint64_t LoopQueue<T>::Allocate() {
    DASSERT(!IsFull(), "Allocate When Queue Full");
    uint64_t ptr = tail_;
    TailInc();
    return ptr;
}

template<typename T>
T &LoopQueue<T>::operator[](uint64_t pos) {
    return data_queue_[pos];
}

template<typename T>
T &LoopQueue<T>::GetFront() const {
    return data_queue_[header_];
}

template<typename T>
void LoopQueue<T>::Reset() {
    tail_ = 0;
    header_ = 0;
    usage_ = 0;

    uint64_t size = data_queue_.size();
    data_queue_.clear();
    data_queue_.resize(size);
}

template<typename T>
bool LoopQueue<T>::IsOlder(uint64_t tag1, uint64_t tag2) const {
    bool tag1GeHeader = tag1 >= header_;
    bool tag2GeHeader = tag2 >= header_;
//    bool tag1GeqTag2 = tag1 > tag2;
    return (tag1GeHeader ^ tag2GeHeader) ? tag1 > tag2 : tag1 < tag2;
}


#endif // LOOPQUEUE_HH_