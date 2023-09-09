#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "temp/Register.hh"
#include "DynInsn.hh"

class Stage {
public:
    Stage(
            std::shared_ptr<std::vector<Register<Dyninsn>>> read_queue_,
            std::shared_ptr<std::vector<Register<Dyninsn>>> write_queue_
    );

    void Evaluate();

    std::vector<Register<Dyninsn>>::iterator &GetProcessIterator();

    std::vector<Register<Dyninsn>>::iterator &GetAcceptIterator();

private:
    bool IsReady();

    bool IsValid();

    void Strategy(Dyninsn &);

private:
    std::shared_ptr<std::vector<Register<Dyninsn>>> process_queue_;
    std::shared_ptr<std::vector<Register<Dyninsn>>> accept_queue_;
};


#endif //STAGE_HH_