//
// Created by yzhang on 1/11/24.
//

#pragma once

#include <vector>
#include <stdint.h>

template <typename T>
class LoopQueue
{
private:
    std::vector<T>  m_dataVec;
    uint64_t        m_header;
    uint64_t        m_tail;
    uint64_t        m_usage;
public:

    LoopQueue(uint64_t depth)
    {
        this->m_dataVec.resize(depth);
        this->clear();
    };

    ~LoopQueue(){ this->clear(); };

    uint64_t head(){
        return this->m_header;
    };

    uint64_t tail(){
        return this->m_tail;
    };

    uint64_t getLastest(){
        return this->getLastPtr(this->m_tail);
    };

    uint64_t getDepth(){
        return this->m_dataVec.size();
    };

    uint64_t size(){
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

    void push(const T& data){
        this->m_dataVec[this->m_tail] = data;
        this->TailInc();
    };

    void pop(){
        this->HeaderInc();
    };

    T& operator[](uint64_t pos){
        return this->m_dataVec[pos];
    }

    T&  front(){
        return this->m_dataVec[m_header];
    };

    T&  back(){
        return this->m_dataVec[m_tail];
    };

    void clear(){
        this->m_tail = 0;
        this->m_header = 0;
        this->m_usage = 0;

        uint64_t size = this->m_dataVec.size();
        this->m_dataVec.clear();
        this->m_dataVec.resize(size);
    };

};
