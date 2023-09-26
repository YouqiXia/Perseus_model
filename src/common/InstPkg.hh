#ifndef INTERFACE_HH_
#define INTERFACE_HH_

#include <memory>
#include "basicunit/TransferQueue.hh"
#include "DynInst.hh"

namespace Emulator {

    struct InstPkgEntry {
        std::shared_ptr<bool> stall;
        InstPtr data;
    };

    typedef std::shared_ptr<InstPkgEntry> InstPkgEntryPtr;

    class InstPkg {
    public:

        InstPkg();

        void Reset();

        void Transfer(InstPtr);

        uint64_t Size();

        std::vector<InstPkgEntry>::iterator Begin();

        std::vector<InstPkgEntry>::iterator End();

        std::vector<InstPkgEntry>::iterator Delete(typename std::vector<InstPkgEntry>::iterator);

    private:
        TransferQueue<InstPkgEntry> queue_;
    };

    typedef std::shared_ptr<InstPkg> InstPkgPtr;
}

#endif // INTERFACE_HH_