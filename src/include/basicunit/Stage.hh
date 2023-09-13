#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "basicunit/Register.hh"
#include "DynInst.hh"

typedef std::shared_ptr<std::vector<Register<InstPtr>>> RegistersPtr;

class Stage {
public:
    // All the interface of the method of Stage should be InstPtr list
    Stage(
            RegistersPtr input_queue,
            RegistersPtr output_queue
    );

    void Evaluate();

protected:
    // To see if all the data in input Register is valid
    bool IsValid() const;

    void RequestReady(InstPtr &) const;

    void SetPermission(InstPtr &) const;

    void Process(InstPtr &) const;

    virtual void Execute(InstPtr &) const;

    void Accept(InstPtr &);

protected:
    RegistersPtr input_queue_;
    RegistersPtr output_queue_;
};


#endif //STAGE_HH_