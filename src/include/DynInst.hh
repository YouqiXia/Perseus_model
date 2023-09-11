#ifndef DYNINSN_HH_
#define DYNINSN_HH_

#include <memory>

struct InstData {
    uint64_t count;
};

typedef std::shared_ptr<InstData> DynInsn;

#endif