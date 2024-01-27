//
// Created by yzhang on 1/11/24.
//

#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <iostream>
#include "IteratorTraits.hpp"

namespace youqixia::resources {

    template<typename DataT>
    class LoopQueue {
    public:
        using size_type = uint32_t;
        using value_type = DataT;
        using LoopQueueType = LoopQueue<value_type>;

    public:
        template <bool is_const_iterator = true>
        class LoopQueueIterator : public utils::IteratorTraits<std::bidirectional_iterator_tag, value_type>
        {
        private:
            using DataReferenceType = typename std::conditional<is_const_iterator,
                    const value_type &, value_type &>::type;
            using LoopQueuePointerType =  typename std::conditional<is_const_iterator,
                    const LoopQueueType * , LoopQueueType * >::type;

            /**
             * \brief construct.
             * \param q  a pointer to the underlying queue.
             * \param begin_itr if true iterator points to tail (begin()), if false points to head (end())
             */
            LoopQueueIterator(LoopQueuePointerType q, uint32_t physical_index) :
                    attached_queue_(q),
                    index_(physical_index)
            { }

            /// Only the Queue can attach itself
            friend class LoopQueue<DataT>;

            /// True if this iterator is attached to a valid queue
            bool isAttached_() const { return nullptr != attached_queue_; }

        public:

            //! \brief Default constructor
            LoopQueueIterator() = default;

            /* Make LoopQueueIterator<true> a friend class of LoopQueueIterator<false>
             * So const iterator can access non-const iterators private members
             */
            friend class LoopQueueIterator<true>;

            /** Copy constructor.
             * Allows for implicit conversion from a regular iterator to a const_iterator
             */
            LoopQueueIterator(const LoopQueueIterator<false> & iter) :
                    attached_queue_(iter.attached_queue_),
                    index_(iter.index_)
            {}

            /** Copy constructor.
             * Allows for implicit conversion from a regular iterator to a const_iterator
             */
            LoopQueueIterator(const LoopQueueIterator<true> & iter) :
                    attached_queue_(iter.attached_queue_),
                    index_(iter.index_)
            {}

            /// Checks validity of iterator -- is it related to a
            /// Queue and points to a valid entry in the queue
            /// \return Returns true if iterator is valid else false
            bool isValid() const {
                if(nullptr == attached_queue_) { return false; }
                return attached_queue_->determineIteratorValidity_(this);
            }

            /**
             * \brief Assignment operator
             */
            LoopQueueIterator& operator=(const LoopQueueIterator& rhs) = default;

//            /// overload the comparison operator.
//            bool operator<(const LoopQueueIterator& rhs) const
//            {
//                assert(attached_queue_ == rhs.attached_queue_,
//                              "Cannot compare QueueIterators created by different Queues");
//                return getIndex() < rhs.getIndex();
//            }
//
//            /// overload the comparison operator.
//            bool operator>(const LoopQueueIterator& rhs) const
//            {
//                assert(attached_queue_ == rhs.attached_queue_,
//                              "Cannot compare QueueIterators created by different Queues");
//                return getIndex() > rhs.getIndex();
//            }

            /// overload the comparison operator.
            bool operator==(const LoopQueueIterator& rhs) const
            {
                assert(attached_queue_ == rhs.attached_queue_);
                return (index_ == rhs.index_);
            }

            /// overload the comparison operator.
            bool operator!=(const LoopQueueIterator& rhs) const {
                return !(*this == rhs);
            }

            /// Pre-Increment operator
            LoopQueueIterator & operator++()
            {
                assert(isAttached_());
                attached_queue_->incrementIterator_(this);
                return *this;
            }

            /// Post-Increment iterator
            LoopQueueIterator operator++(int)
            {
                LoopQueueIterator it(*this);
                operator++();
                return it;
            }

            /// Pre-decrement iterator
            LoopQueueIterator & operator--()
            {
                assert(isAttached_());
                attached_queue_->decrementIterator_(this);
                return *this;
            }

            /// Post-decrement iterator
            LoopQueueIterator operator-- (int){
                LoopQueueIterator it(*this);
                operator--();
                return it;
            }

            /// Dereferencing operator
            DataReferenceType operator* ()
            {
                assert(isValid());
                return getAccess_(std::integral_constant<bool, is_const_iterator>());
            }

            ///support -> operator
            value_type* operator->()
            {
                assert(isValid());
                return std::addressof(getAccess_(std::integral_constant<bool, is_const_iterator>()));
            }

            const value_type* operator->() const
            {
                assert(isValid());
                return std::addressof(getAccess_(std::integral_constant<bool, is_const_iterator>()));
            }

            /// Get the logical index of this entry in the queue.
            /// This is expensive and should be avoided.  It makes
            /// better sense to simply retrieve the object directly
            /// from the iterator.
            uint32_t getIndex() const
            {
                assert(isAttached_());
                return index_;
            }

        private:

            LoopQueuePointerType attached_queue_ {nullptr};
            uint32_t index_ = std::numeric_limits<uint32_t>::max();

            /// Get access on a non-const iterator
            DataReferenceType getAccess_(std::false_type) const {
                return attached_queue_->access(index_);
            }

            /// Get access on a const iterator
            DataReferenceType getAccess_(std::true_type) const {
                return attached_queue_->read(index_);
            }
        };

    public:
        using iterator = LoopQueueIterator<false> ;
        using const_iterator = LoopQueueIterator<true> ;

    public:
        LoopQueue(uint32_t depth) :
                depth_(depth),
                invalid_idx_(depth + 1)
        {
            dataList_ = new DataT[depth];
            clear();
        }

        ~LoopQueue() { delete[] dataList_; }

        void clear() {
            tail_ = 0;
            head_ = 0;
            usage_ = 0;
            is_rolling = false;
        }

        uint32_t capacity() const {
            return depth_;
        }

        size_type size() const {
            assert(is_rolling * depth_ - head_ + tail_ == usage_);
            return is_rolling * depth_ - head_ + tail_;
        }

        size_type numFree() const {
            return depth_ - size();
        }

//        uint32_t head() { return head_; }
//
//        uint32_t tail() { return tail_; }

        bool full() const {
            return size() == depth_;
        }

        bool empty() const {
            return size() == 0;
        }

        bool isValid(uint32_t idx) {
            bool in_scope = idx >= head_ && idx < tail_;
            return is_rolling ^ in_scope;
        }

        const value_type &read(uint64_t idx) const {
            return dataList_[idx];
        }

        value_type &access(uint64_t idx) {
            return dataList_[idx];
        }

        value_type &front() {
            return dataList_[head_];
        }

        value_type &back() {
            uint32_t index = decrementIndexValue_(tail_);
            return dataList_[index];
        }

        iterator getFrontIterator() {
            return begin();
        }

        iterator getBackIterator() {
            uint32_t index = decrementIndexValue_(tail_);
            return iterator(this, index);
        }

        iterator push(const value_type &data) {
            assert(!full());
            dataList_[tail_] = data;
            uint32_t allocate_idx = tail_;
            usage_++;
            tail_ = incrementPointer_(tail_);
            return iterator(this, allocate_idx);
        }

        void pop() {
            assert(!empty());
            usage_--;
            head_ = incrementPointer_(head_);
        }

        void pop_back() {
            assert(!empty());
            usage_--;
            tail_ = decrementPointer_(tail_);
        }

        iterator begin() {
            if (empty()) {
                return iterator(this, invalid_idx_);
            }
            return iterator(this, head_);
        }

        const_iterator begin() const {
            if (empty()) {
                return iterator(this, invalid_idx_);
            }
            return iterator(this, head_);
        }

        iterator end() {
            return iterator(this, invalid_idx_);
        }

        const_iterator end() const {
            return iterator(this, invalid_idx_);
        }

        std::ostream &operator<<(std::ostream &os) {
            for (auto& data_entry: *this) {
                os << data_entry << ", ";
            }
            os << std::endl;
            return os;
        }

    private:
        uint32_t incrementIndexValue_(uint32_t idx) {
            if (idx == depth_ - 1) {
                return 0;
            } else {
                return idx + 1;
            }
        }

        uint32_t decrementIndexValue_(uint32_t idx) {
            if (idx == 0) {
                return depth_ - 1;
            } else {
                return idx - 1;
            }
        }

        uint32_t incrementPointer_(uint32_t idx) {
            if (idx == depth_ - 1) {
                is_rolling = !is_rolling;
            }
            return incrementIndexValue_(idx);
        }

        uint32_t decrementPointer_(uint32_t idx) {
            if (idx == 0) {
                is_rolling = !is_rolling;
            }
            return decrementIndexValue_(idx);
        }

        template<typename IteratorType>
        void incrementIterator_(IteratorType * itr)
        {
            uint32_t next_idx = incrementIndexValue_(itr->index_);
            if (next_idx == tail_) {
                itr->index_ = invalid_idx_;
            } else {
                itr->index_ = next_idx;
            }
        }

        template<typename IteratorType>
        void decrementIterator_(IteratorType * itr)
        {
            uint32_t last_idx = decrementIndexValue_(itr->index_);
            if (itr->index_ == head_) {
                itr->index_ = invalid_idx_;
            } else {
                itr->index_ = last_idx;
            }
        }

        template<typename IteratorType>
        bool determineIteratorValidity_(IteratorType * itr) {
            return isValid(itr->index_);
        }

    private:
        DataT* dataList_;
        uint32_t head_;
        uint32_t tail_;
        uint32_t usage_;
        uint32_t depth_;
        uint32_t invalid_idx_;

        bool is_rolling;

    };

    template<class DataT>
    std::ostream &operator<<(std::ostream &os, LoopQueue<DataT>& loop_queue) {
        loop_queue << os;
        return os;
    }
}
