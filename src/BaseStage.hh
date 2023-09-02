#ifndef BASESTAGE_HH_
#define BASESTAGE_HH_

#include<vector>
//代码规范之后再调整，包括ifndef的东西

class BaseStage {

private:
    std::shared_ptr<BaseTiming> baseTiming;
    std::shared_ptr<BaseCombination> baseCombination;
    std::shared_ptr<BaseStage> preStage; //使用链表来管理stage
};

class BaseTiming {

// 如timebuffer
// 需要被下一级的BaseCombination的ready决定
    virtual bool IsStall() = 0;

};

class BaseCombination {
public:
/* 使用结构解耦所有可能造成非ready状态的模块
 * eg:buffer full, bus handshack等 
 * 默认为true
 */
    virtual bool IsReady() = 0; 
/* 使用结构解耦所有产生data依赖的valid状态的模块
 * eg:buffer empty, bus handshack等
 * 默认为无
 */
    virtual bool IsValid() = 0;
};

#endif
