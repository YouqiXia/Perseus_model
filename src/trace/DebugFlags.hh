#ifndef __DEBUGFLAGS_HH__
#define __DEBUGFLAGS_HH__

#include <string>
#include <set>
#include <map>

namespace Emulator {

#define DEFFLAG(flag, desp) {#flag , #desp}

    const std::map<std::string, std::string> debugFlags = {
            DEFFLAG(ALL, "Open All Flags"),
            DEFFLAG(Tick, "Tick Object"),
            DEFFLAG(Global, "Global Flag"),
            DEFFLAG(ICacheReq, "ICache Request"),
            DEFFLAG(ICacheResp, "ICache Response"),

            DEFFLAG(DCacheReq, "DCache Request"),
            DEFFLAG(DCacheResp, "DCache Response"),

            DEFFLAG(LoadReq, "LSU Load Request"),

            DEFFLAG(StoreReq, "LSU Store Request"),

            DEFFLAG(Flush, "Flush Stage"),

            DEFFLAG(RollBack, "Recovery->Rollback instruction"),

            /* inter-Stage Latch */
            DEFFLAG(Redirect, "Redirect Request to Fetch1 Stage"),
            DEFFLAG(ReceiveReq, "Stage Receive Request from Last Stage"),

            DEFFLAG(Replay, "Instruction Buffer Full, Replay Fetch"),

            DEFFLAG(Issue, "Issue a Insn to Schedulars"),
            DEFFLAG(ReadOperand, "Insn read Operand"),
            DEFFLAG(Execute, "Issue Insn & Start Execute"),
            DEFFLAG(WriteBack, "WriteBack a insn"),
            DEFFLAG(Forwarding, "Forwarding Result of a insn"),
            DEFFLAG(Stall, "Function Unit Stalled beacuse of Write Back Failed"),
            DEFFLAG(Commit, "Commit"),
            DEFFLAG(CommitLog, "Commit Log (Clean)"),

            DEFFLAG(MemoryOrder, "Fence & AMO State"),
            DEFFLAG(MemoryDependancy, "MemoryDependancy")
    };

    const std::map<std::string, std::string> registedObject = {
            DEFFLAG(DRAM, "DRAM Request"),
            DEFFLAG(Rcu, "Resource Allocate/Rename/RollBack/Commit Managerment"),
            /* Stage */
            DEFFLAG(Fetch1, "Fetch1"),
            DEFFLAG(Fetch2, "Fetch2"),
            DEFFLAG(Decode, "Decode"),
            DEFFLAG(Dispatch, "Dispatch")
    };

#undef DEFFLAG

    void setFlagEnable(std::string flag);

    bool getFlagEnable(std::string flag);

    void setObjectEnable(std::string Name);

    bool getObjectEnable(std::string Name);

} // namespace Emulator





#endif	
