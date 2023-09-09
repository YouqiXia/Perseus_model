#ifndef REGISTER_HH_
#define REGISTER_HH_

template<typename T>
class Register {
public:
    /*
     * Simulate the behavior of register(storage unit in rtl)
     * Including ctrl(valid, ready) + data
     */
    Register();

// Read port
    /* Data in the Register is valid */
    virtual bool IsValid() const;

    /* Read data from inner data structure */
    virtual void Process(T &) const;

    /*
     * Read ctrl signal
     * Modify the ctrl signal which controls whether the Register will do Advance
     */
    virtual void Feedback(T &);

    /* Stall the Register which will do nothing in the Advance operation */
    virtual void Stall();

// Write port
    /* Data can be written into the Register */
    virtual bool IsReady() const;

    /* Accept data if the Register is ready but do not update */
    virtual void Accept(T &);

    /*
     * Whether the Register will do Advance is according to ctrl signal
     * Update data from instruction queue according to timing
     */
    virtual void Advance();

protected:
    virtual void Evaluate();
};

#endif // REGISTER_HH_