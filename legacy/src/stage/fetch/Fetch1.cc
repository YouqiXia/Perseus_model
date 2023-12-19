#include "Fetch1.hh"

namespace Emulator {
    Fetch1::Fetch1(std::string name) : Stage(std::move(name)) {}

    void Fetch1::Produce() {
        for (int i = 0; i < 2; i++) {
            InstPtr inst_ptr = std::make_shared<InstData>();
            inst_pkg_ptr_->Transfer(inst_ptr);
        }
    }

    void Fetch1::Execute() {
        for (auto i = inst_pkg_ptr_->Begin(); i!=inst_pkg_ptr_->End(); ++i) {
            int pc_inc = 4;
            if ((*i)->data->is_rvc) {
                pc_inc = 2;
            }
            if (!(*i)->data->is_branch) {
                pc_ = pc_ + pc_inc;
            }
        }
    }
}