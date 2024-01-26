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

class spikeAdpter{

public:
    spikeAdpter(){
        spike_tunnel.resize(32);
    }
    ~spikeAdpter(){ delete spike_sim; }
    int spikeInit(int argc, char** argv);

    int spikeInit(std::vector<std::string>& commandLineArgs);

    spikeInsnPtr spikeGetNextInst();

    static void decodeHook(void*, uint64_t , uint64_t);

    static bool commitHook();

    static void excptionHook();

    void setElfName(std::string name) { elf_name = name; }

    uint32_t spikeTunnelAvailCnt();

    void spikeStep(uint32_t n);

    void spikeRunStart();

    int spikeRunEnd();

    void spikeStep();
public:
    std::string elf_name;
    static sim_t * spike_sim;
    
    static std::vector<spikeInsnPtr> spike_tunnel;
    static uint32_t spike_tunnel_size;
    static uint32_t spike_tunnel_tail;
    static uint32_t spike_tunnel_head;
    static uint32_t spike_tunnel_used;
    static uint32_t spike_tunnel_tocommit;
    bool is_done;

    std::queue<reg_t> fromhost_queue;
    std::function<void(reg_t)> fromhost_callback;


    //main variables
    cfg_t cfg;
};

