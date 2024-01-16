//
// Created by yzhang on 1/11/24.
//

#pragma once

#include <vector>
#include <stdint.h>
#include <iostream>

namespace resources {

    template<typename T>
    class LoopQueue {
    private:
        std::vector<T> dataVec_;
        uint32_t head_;
        uint32_t tail_;
        uint32_t usage_;
        uint32_t depth_;

    public:
        using size_type = uint32_t;
        using value_type = T;
        using pointer = typename std::vector<value_type>::pointer;
        using iterator = typename std::vector<value_type>::iterator;
        using const_iterator = typename std::vector<value_type>::const_iterator;

    public:
        LoopQueue(uint32_t depth) :
                depth_(depth) {
            clear();
        };

        ~LoopQueue() { clear(); };

        void clear() {
            tail_ = 0;
            head_ = 0;
            usage_ = 0;
            dataVec_.clear();
            dataVec_.resize(depth_);
        };

        uint32_t capacity() const {
            return dataVec_.size();
        }

        size_type size() const {
            return usage_;
        };

        size_type numFree() const {
            return dataVec_.size() - usage_;
        };

        uint32_t head() { return head_; }

        uint32_t tail() { return tail_; }

        bool full() const {
            return usage_ == dataVec_.size();
        };

        bool empty() const {
            return usage_ == 0;
        };

        const value_type &read(uint64_t idx) const {
            return dataVec_[idx];
        }

        value_type &access(uint64_t idx) {
            return dataVec_[idx];
        }

        value_type &front() {
            return dataVec_[head_];
        };

        value_type &back() {
            uint32_t index = decrementIndexValue_(tail_);
            return dataVec_[index];
        };

        iterator push(const value_type &data) {
            assert(!full());
            dataVec_[tail_] = data;
            uint32_t allocate_idx = tail_;
            usage_++;
            tail_ = incrementIndexValue_(tail_);
            return iterator(&dataVec_[tail_]);
        };

//    iterator push(const value_type&& data){
//        dataVec_[tail_](std::move(data));
//        uint32_t allocate_idx = tail_;
//        tail_ = incrementIndexValue_(tail_);
//        return iterator(&dataVec_[tail_]);
//    };

        void pop() {
            assert(!empty());
            usage_--;
            head_ = incrementIndexValue_(head_);
        };

        void pop_back() {
            assert(!empty());
            usage_--;
            tail_ = decrementIndexValue_(tail_);
        };

//    iterator begin() {
//        return iterator(&dataVec_[head_]);
//    }
//
//    const_iterator begin() const {
//        return iterator(&dataVec_[head_]);
//    }
//
//    iterator end() {
//        uint32_t index = decrementIndexValue_(tail_);
//        return iterator(&dataVec_[index]);
//    }
//
//    const_iterator end() const {
//        uint32_t index = decrementIndexValue_(tail_);
//        return iterator(&dataVec_[index]);
//    }

        std::ostream &operator<<(std::ostream &os) {
            uint32_t idx = head_;
            size_type usage = usage_;
            while (usage--) {
                os << dataVec_[idx] << ", ";
                idx = incrementIndexValue_(idx);
            }
            os << std::endl;
            return os;
        }

    private:
        uint32_t incrementIndexValue_(uint32_t idx) {
            if (idx == dataVec_.size() - 1) {
                return 0;
            } else {
                return idx + 1;
            }
        }

        uint32_t decrementIndexValue_(uint32_t idx) {
            if (idx == 0) {
                return dataVec_.size() - 1;
            } else {
                return idx - 1;
            }
        }
    };

    template<class T>
    std::ostream &operator<<(std::ostream &os, LoopQueue<T> loop_queue) {
        loop_queue << os;
        return os;
    }

}