#ifndef DYNINST_HH_
#define DYNINST_HH_

#include <memory>

struct InstData {
    // while processing, set stall ptr to all Register into input queue
    uint64_t count;
};

typedef std::shared_ptr<InstData> InstPtr;

#endif
