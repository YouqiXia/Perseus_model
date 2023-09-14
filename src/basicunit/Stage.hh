#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "basicunit/Register.hh"
#include "common/DynInst.hh"

typedef std::shared_ptr<std::vector<Register>> RegistersPtr;

class Stage {
public:
    // All the interface of the method of Stage should be InstPtr list
    Stage(
            RegistersPtr input_queue,
            RegistersPtr output_queue
    );

    void Evaluate();

protected:
    void Reset();

    void SetPermission();

    void Produce();

    void Process();

    virtual void Execute() {}

    void Accept();

    bool IsNothingToDo();

protected:
    RegistersPtr input_queue_;
    RegistersPtr output_queue_;
    InstPkgPtr inst_pkg_ptr_;
};


#endif //STAGE_HH_