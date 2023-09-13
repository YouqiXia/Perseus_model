#ifndef INTERFACE_HH_
#define INTERFACE_HH_

#include <memory>
#include "basicunit/TransferQueue.hh"
#include "DynInst.hh"

struct InstPkgEntry {
    bool stall;
    InstPtr data;
};

class InstPkg {
public:

    InstPkg();

    void Reset();

    void Transfer(InstPtr &);

    std::vector<InstPkgEntry>::iterator Begin();

    std::vector<InstPkgEntry>::iterator End();

    std::vector<InstPkgEntry>::iterator Delete(typename std::vector<InstPkgEntry>::iterator &);

private:
    TransferQueue<InstPkgEntry> queue_;
};

typedef std::shared_ptr<InstPkg> InstPkgPtr;

#endif // INTERFACE_HH_