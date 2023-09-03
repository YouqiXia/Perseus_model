#ifndef BASESTAGE_HH_
#define BASESTAGE_HH_

#include<vector>
//代码规范之后再调整，包括ifndef的东西

template <typename T>
class BaseTiming {
// 如timebuffer, writebackbuffer
public:
    // ctrl signals
    virtual void Reset() = 0;
    virtual void Stall() = 0;
    virtual bool IsStall() = 0; // 需要被下一级的BaseCombination的ready决定
    virtual void Flush() = 0;
    virtual bool IsFlush() = 0;
    virtual bool isDrained() = 0;
    virtual void Advance() = 0;
    // data signals
    virtual bool IsValid() = 0;
};

template <typename T>
class BaseCombination {
public:
/* 使用结构解耦所有可能造成非ready状态的模块
 * eg:buffer full, bus handshack等 
 * 默认为true
 * 向上timebuffer握手
 */
    virtual bool IsReady() = 0; 
/* 使用结构解耦所有产生data依赖的valid状态的模块
 * eg:buffer empty, bus handshack等
 * 默认为无
 * 向下timebuffer握手
 */
    virtual bool IsValid() = 0;
    virtual void SetData(T data) = 0;
    virtual T&   GetData() = 0;     // 引用？
    virtual void Evaluate() = 0; //负责kill掉outport数据
};

#endif // BASESTAGE_HH_
