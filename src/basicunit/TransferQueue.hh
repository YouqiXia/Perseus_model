#ifndef TRANSFERQUEUE_HH_
#define TRANSFERQUEUE_HH_

#include <vector>

template<typename T>
class TransferQueue {
public:

    void Reset();

    void Transfer(T &);

    uint64_t Size();

    typename std::vector<T>::iterator Begin();

    typename std::vector<T>::iterator End();

    typename std::vector<T>::iterator Delete(typename std::vector<T>::iterator &);

private:
    std::vector<T> queue_;
};

template<typename T>
void TransferQueue<T>::Reset() {
    queue_.clear();
}

template<typename T>
void TransferQueue<T>::Transfer(T &data) {
    queue_.push_back(data);
}

template<typename T>
uint64_t TransferQueue<T>::Size() {
    return queue_.size();
}

template<typename T>
typename std::vector<T>::iterator TransferQueue<T>::Begin() {
    return queue_.begin();
}

template<typename T>
typename std::vector<T>::iterator TransferQueue<T>::End() {
    return queue_.end();
}

template<typename T>
typename std::vector<T>::iterator TransferQueue<T>::Delete(typename std::vector<T>::iterator &iterator) {
    return queue_.erase(iterator);
}


#endif