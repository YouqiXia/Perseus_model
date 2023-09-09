#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "temp/Register.hh"
#include "DynInsn.hh"

class Stage {
public:
    Stage(
            std::shared_ptr<std::vector<Register<DynInsn>>> read_queue_,
            std::shared_ptr<std::vector<Register<DynInsn>>> write_queue_
    );

    void Evaluate();

    // To see if some instruction read from Register is permitted to operate
    virtual bool IsPermitted(DynInsn &) const;

protected:
    // To see if all the Register is ready to be written into
    bool IsReady() const;

    // To see if all the data in Register is valid
    bool IsValid() const;

    void Process(DynInsn &) const;

    void Feedback(DynInsn &) const;

    virtual void Strategy(DynInsn &) const;

protected:
    std::shared_ptr<std::vector<Register<DynInsn>>> process_queue_;
    std::shared_ptr<std::vector<Register<DynInsn>>> accept_queue_;
};


#endif //STAGE_HH_