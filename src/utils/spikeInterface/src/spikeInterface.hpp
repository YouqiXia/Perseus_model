#pragma once
#include "decode.h"
#include "sim.h"


#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ResourceFactory.hpp"
#include "sparta/simulation/ResourceFactory.hpp"
#include "mmu.h"

#include "config.h"
#include "cfg.h"
#include "sim.h"
#include "mmu.h"
#include "arith.h"
// #include "remote_bitbang.h"
#include "cachesim.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <limits>
#include <cinttypes>
#include <sstream>
#include "../VERSION"

#include "DataBackup.hpp"
//from spike decode_macros.h
#define invalid_pc(pc) ((pc) & 1)

class spike_insn{
    public:
        spike_insn(insn_fetch_t* in, uint64_t pc_in): valid(false){
            memcpy(&spike_insn_, in, sizeof(insn_fetch_t));
            pc = pc_in;
            spike_last_inst_priv = 0;
            spike_last_inst_xlen = 0;
            spike_last_inst_flen = 0;
            spike_log_reg_write.clear();
            spike_log_mem_read.clear();
            spike_log_mem_write.clear();
        }
    public:
        bool valid;
        uint64_t pc;

        insn_fetch_t spike_insn_;

        commit_log_reg_t spike_log_reg_write;
        commit_log_mem_t spike_log_mem_read;
        commit_log_mem_t spike_log_mem_write;
        reg_t spike_last_inst_priv;
        int spike_last_inst_xlen;
        int spike_last_inst_flen;

        bool isRvc(){ return spike_insn_.insn.length() ==2; }
        uint64_t getPc(){ return pc; }
};

using spikeInsnPtr = std::shared_ptr<spike_insn>;

class spikeAdapter{

public:
    static spikeAdapter* getSpikeAdapter();

    ~spikeAdapter(){ delete spike_adapter_; }

    int spikeInit(std::vector<std::string>& commandLineArgs);

    spikeInsnPtr spikeGetNextInst();

    void decodeHook(void*, uint64_t , uint64_t);

    bool commitHook();

    reg_t getNpcHook(reg_t spike_npc);

    void excptionHook();

    void catchDataBeforeWriteHook(addr_t addr, reg_t data, size_t len);

    void getCsrHook(int which, reg_t val);

    uint32_t spikeTunnelAvailCnt();

    int spikeStep(uint32_t n);

    void spikeSingleStepFromNpc(reg_t npc);

    void spikeRunStart();

    void setNpc(reg_t npc);

    uint64_t getSpikeNpc() { return spike_npc_; }

    void MakeBackup();

    void RollBack();

    void BranchResolve();

private:
    static spikeAdapter* spike_adapter_;

    spikeAdapter() = default;

    int spikeInit_(int argc, char** argv);

    void setElfName_(std::string name) { elf_name = name; }

    int spikeRunEnd_();

    void MakeRegBackup_();

    void RegRollBack_();

public:
    std::string elf_name;
    sim_t * spike_sim = nullptr;

    spikeInsnPtr spike_tunnel;
    bool is_done = false;

    std::queue<reg_t> fromhost_queue;
    std::function<void(reg_t)> fromhost_callback;

private:
    sparta::utils::ValidValue<reg_t> npc_;

    //main variables
    cfg_t cfg;

    uint64_t target_addr_ = -1;
    int target_csr_ = -1;
    DataBackup<MemoryEntry> memory_backup_;
    DataBackup<CsrEntry> csr_backup_;
    std::queue<RegEntry> reg_backup_;

    uint64_t spike_npc_;

};

