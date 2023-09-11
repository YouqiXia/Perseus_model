#ifndef DYNINST_HH_
#define DYNINST_HH_

#include <memory>

struct InstData {
    // while processing, set stall ptr to all Register into input queue
    std::shared_ptr<bool> stall;
    uint64_t count;
};

typedef std::shared_ptr<InstData> DynInst;

// FIXME: there should be an DynInst list (what kind of data structure?)

#endif
