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

    virtual bool Analyse(DynInsn &) const;

protected:
    bool IsReady() const;

    bool IsValid() const;

    virtual void Strategy(DynInsn &) const;

protected:
    std::shared_ptr<std::vector<Register<DynInsn>>> process_queue_;
    std::shared_ptr<std::vector<Register<DynInsn>>> accept_queue_;
};


#endif //STAGE_HH_