#include "spikeAdpterHooks.hpp"
#include "spikeInterface.hpp"


void decodeHook(void* in, uint64_t pc,uint64_t npc){
    spikeAdapter::getSpikeAdapter()->decodeHook(in, pc, npc);
}

bool commitHook(){
    return spikeAdapter::getSpikeAdapter()->commitHook();
}

uint64_t getNpcHook(uint64_t spike_npc) {
    return spikeAdapter::getSpikeAdapter()->getNpcHook(spike_npc);
}

void excptionHook(){

}

void catchDataBeforeWriteHook(uint64_t addr, uint64_t data, uint32_t len) {
    spikeAdapter::getSpikeAdapter()->catchDataBeforeWriteHook(addr, data, len);
}

