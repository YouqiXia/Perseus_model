#include "InstBufferFetchAdapter.hh"

namespace Emulator {
    InstBufferFetchAdapter::InstBufferFetchAdapter(
            std::string name,
            Emulator::InstBuffer &inst_buffer
    ) : Trace::TraceObject(std::move(name)),
        inst_buffer_(inst_buffer) {}

    void InstBufferFetchAdapter::Reset() {}

    void InstBufferFetchAdapter::Process(InstPkgPtr inst_pkg_ptr) {
        uint64_t inst_buffer_current_cnt = inst_buffer_.GetInstBuffer().GetHeader();
        Inst_t inst_tmp;
        for (auto i = inst_pkg_ptr->Begin(); i != inst_pkg_ptr->End(); i++) {
            /* ****** pc ****** */
            inst_tmp = inst_buffer_.GetInstBuffer()[inst_buffer_current_cnt];
            i->data->BasicInfo.instruction = inst_tmp;
            inst_buffer_current_cnt = inst_buffer_.GetInstBuffer().GetNextPtr(inst_buffer_current_cnt);
            if (!(inst_tmp & 0b11)) continue;
            inst_tmp = inst_buffer_.GetInstBuffer()[inst_buffer_current_cnt];
            /* should be checked */
            i->data->BasicInfo.instruction = inst_tmp << 16 | i->data->BasicInfo.instruction;
            inst_buffer_current_cnt = inst_buffer_.GetInstBuffer().GetNextPtr(inst_buffer_current_cnt);
        }
    }

}