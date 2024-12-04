//
// Created by yzhang on 12/2/24.
//

#pragma once

template <typename T>
class LatencyQueue {
public:
    struct PendingStruct {
        uint64_t latency;
        T data;
    };
public:
    LatencyQueue() = default;

    void Push(uint64_t latency, T data);

    void Push(uint64_t latency, std::vector<T>& data_vector);

    void Tick();

    bool IsStopped();

    bool Empty();

    T PopFront();

private:
    std::deque<PendingStruct> pending_queue_;
    std::deque<T> candidate_queue_;
};

template <typename T>
void LatencyQueue<T>::Push(uint64_t latency, T data) {
    pending_queue_.emplace_back(LatencyQueue<T>::PendingStruct{latency, data});
}

template <typename T>
void LatencyQueue<T>::Push(uint64_t latency, std::vector<T>& data_vector) {
    for (auto data: data_vector) {
        this->Push(latency, data);
    }
}

template <typename T>
void LatencyQueue<T>::Tick() {
    for (auto it = pending_queue_.begin(); it != pending_queue_.end(); ) {
        --it->latency;
        if (it->latency == 0) {
            candidate_queue_.emplace_back(it->data);
            it = pending_queue_.erase(it);
        } else {
            ++it;
        }
    }
}

template <typename T>
bool LatencyQueue<T>::Empty() {
    return candidate_queue_.empty();
}

template <typename T>
bool LatencyQueue<T>::IsStopped() {
    return candidate_queue_.empty() && pending_queue_.empty();
}


template <typename T>
T LatencyQueue<T>::PopFront() {
    auto data = candidate_queue_.front();
    candidate_queue_.pop_front();
    return data;
}
