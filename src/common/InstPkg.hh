#ifndef INTERFACE_HH_
#define INTERFACE_HH_

#include <memory>
#include "basicunit/TransferQueue.hh"
#include "DynInst.hh"

struct InstPkgEntry {
    bool stall;
    InstPtr data;
};

typedef std::shared_ptr<InstPkgEntry> InstPkgEntryPtr;

class InstPkg {
public:

    InstPkg();

    void Reset();

    void Transfer(InstPtr &);

    uint64_t Size();

    std::vector<InstPkgEntryPtr>::iterator Begin();

    std::vector<InstPkgEntryPtr>::iterator End();

    std::vector<InstPkgEntryPtr>::iterator Delete(typename std::vector<InstPkgEntryPtr>::iterator &);

private:
    TransferQueue<InstPkgEntryPtr> queue_;
};

typedef std::shared_ptr<InstPkg> InstPkgPtr;

#endif // INTERFACE_HH_