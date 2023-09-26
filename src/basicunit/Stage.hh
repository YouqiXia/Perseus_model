#ifndef STAGE_HH_
#define STAGE_HH_

#include <memory>
#include <vector>
#include "basicunit/Register.hh"
#include "common/DynInst.hh"

namespace Emulator {

    typedef std::vector<RegisterPtr> RegisterVec;

    class Stage : public Trace::TraceObject {
    public:
        // All the interface of the method of Stage should be InstPtr list
        Stage(std::string name);

        void Evaluate();

        void Reset();

        void SetProduceQueue(const RegisterPtr&);

        void SetProcessQueue(const RegisterPtr&);

        void SetWriteIntoQueue(const RegisterPtr&);

    protected:
        void SetPermission();

        virtual void Produce();

        void Process();

        virtual void Execute() {}

        void Accept();

        // To make sure all the invalid instructions are deleted
        bool IsNothingToDo();

    protected:
        RegisterPtr commit_unit_;
        RegisterVec produce_queue_;
        RegisterVec process_queue_;
        RegisterVec write_into_queue_;
        InstPkgPtr inst_pkg_ptr_;
    };

}

#endif //STAGE_HH_