#ifndef __SPIKE_ADPTER_HOOKS__
#define __SPIKE_ADPTER_HOOKS__
#include <cstdint>

extern void decodeHook(void*, uint64_t,uint64_t) __attribute__((weak));
extern bool commitHook() __attribute__((weak));
extern void excptionHook() __attribute__((weak));


#endif

