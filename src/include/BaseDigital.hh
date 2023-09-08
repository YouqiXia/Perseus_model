#ifndef BASESTAGE_HH_
#define BASESTAGE_HH_

#include <memory>
#include <vector>

template<typename T>
class port {
public:
    port();

    bool IsValid() const;

    void Kill();

    void SetData(T data);

    const T &GetData() const;

private:
    bool valid_;
    T data_;
};

template<typename T>
class BaseDigital {
public:
// Both Latch & FlipFlop
    virtual void Reset() = 0;

// FlipFlop
    virtual void Stall() = 0;

    virtual bool IsStall() = 0; // 需要被下一级的BaseCombination的ready决定
    virtual void Flush() = 0;

    virtual bool IsFlush() = 0;

    virtual bool IsDrained() = 0;

    virtual void Advance() = 0;

// Latch
    virtual bool IsReady() = 0;

    virtual void Evaluate() = 0; //负责kill掉latch port数据
// port interface
    virtual bool IsPortValid() = 0;

    virtual std::shared_ptr<port<T>> &GetPort() = 0;
};

//Implementation

template<typename T>
port<T>::port() {
    valid_ = false;
}

template<typename T>
bool port<T>::IsValid() const {
    return valid_;
}

template<typename T>
void port<T>::Kill() {
    valid_ = false;
}

template<typename T>
void port<T>::SetData(T data) {
    valid_ = true;
    data_ = data;
}

template<typename T>
const T &port<T>::GetData() const {
    return data_;
}


#endif // BASESTAGE_HH_
