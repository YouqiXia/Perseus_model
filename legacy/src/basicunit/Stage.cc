#include <memory>
#include <utility>

#include "Stage.hh"

namespace Emulator {

    Stage::Stage(std::string name)
            : Trace::TraceObject(std::move(name)) {}

    void Stage::Reset() {
        inst_pkg_ptr_->Reset();
    }

    void Stage::Evaluate() {

        Produce();

        if (IsNothingToDo()) return;

        // generate valid signal to not set stall = true
        SetPermission();

        if (IsNothingToDo()) return;

        // generate ready signal to not set stall = true
        Accept();

        if (IsNothingToDo()) return;

        Process();

        Execute();
    }

    bool Stage::IsNothingToDo() {
        // FIXME: 两个相同的if结构需要修改
        if (!inst_pkg_ptr_->Size())
            return true;
        for (auto i = inst_pkg_ptr_->Begin(); i != inst_pkg_ptr_->End();) {
            if ((*i)->stall) {
                i = inst_pkg_ptr_->Delete(i);
            } else {
                ++i;
            }
        }
        if (!inst_pkg_ptr_->Size())
            return true;
        return false;
    }

    void Stage::Produce() {
        for (auto i = produce_queue_.begin(); i != produce_queue_.begin(); ++i) {
            (*i)->Produce(inst_pkg_ptr_);
        }
    }

    void Stage::Process() {
        for (auto i = process_queue_.begin(); i != process_queue_.begin(); ++i) {
            (*i)->Process(inst_pkg_ptr_);
        }
    }

    void Stage::SetPermission() {
        for (auto i = process_queue_.begin(); i != process_queue_.begin(); ++i) {
            (*i)->SetPermission(inst_pkg_ptr_);
        }
    }

    void Stage::Accept() {
        for (auto i = write_into_queue_.begin(); i != write_into_queue_.begin(); ++i) {
            (*i)->Accept(inst_pkg_ptr_);
        }
    }

    void Stage::SetProduceQueue(const RegisterPtr& register_ptr) {
        produce_queue_.push_back(register_ptr);
    }

    void Stage::SetProcessQueue(const RegisterPtr& register_ptr) {
        process_queue_.push_back(register_ptr);
    }

    void Stage::SetWriteIntoQueue(const RegisterPtr& register_ptr) {
        write_into_queue_.push_back(register_ptr);
    }
}