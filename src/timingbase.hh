class TimeBase {
public:
    virtual void Stall() = 0;
    virtual bool IsStall() = 0;
    virtual void Flush() = 0;
    virtual bool IsFlush() = 0;
    virtual bool isDrained() = 0;
    virtual void Reset() = 0;
    virtual void Evaluate() = 0;
    virtual void Advance() = 0;
};

class TimeBuffer : public TimeBase {};

class PipelineBuffer : public TimeBase {};

class WriteoverBuffer : public TimeBase {};