#ifndef PROCESSUNIT_HH_
#define PROCESSUNIT_HH_

template<typename T>
class ProcessUnit {
public:
    // 带存储性的单元，eg: register file, memory, fifo, etc.
    ProcessUnit();

    /* fifo: not full */
    virtual bool IsReady();

    /* fifo: not empty */
    virtual bool IsValid();

    /* ctrl module permit to execute */
    virtual bool IsPermitted(T &);

    /* register file & memory: read data */
    virtual void Process(T &) {}

    /* write back data(callback function) */
    virtual void Accept(T &) {}

    /*
     * fifo: pop or push a line
     */
    virtual void Advance() {}

    virtual void Evaluate() {}
};

template<typename T>
ProcessUnit<T>::ProcessUnit() {}

template<typename T>
bool ProcessUnit<T>::IsReady() {
    return true;
}

template<typename T>
bool ProcessUnit<T>::IsValid() {
    return true;
}

template<typename T>
bool ProcessUnit<T>::IsPermitted(T &) {
    return true;
}

#endif // PROCESSUNIT_HH_