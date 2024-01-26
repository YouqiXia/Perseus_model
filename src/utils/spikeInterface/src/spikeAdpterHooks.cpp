#include "spikeAdpterHooks.hpp"
#include "spikeInterface.hpp"


void decodeHook(void* in, uint64_t pc,uint64_t npc){
    spikeAdpter::decodeHook(in, pc, npc);
}

bool commitHook(){
    return spikeAdpter::commitHook();
}
void excptionHook(){

}

