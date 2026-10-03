#ifndef TTK_FMV_POLL_MATH_H
#define TTK_FMV_POLL_MATH_H
#include <stdint.h>
static inline int ttk_overlap(uint32_t lo,uint32_t hi,uint32_t a,uint32_t b) {
    return lo<b && a<hi;
}
static inline int ttk_poll_dma_safe(unsigned channel,uint32_t active,
        uint32_t madr,uint32_t chcr,uint32_t remaining,uint32_t entry) {
    if(!active)return 1;
    if(channel==0 && (chcr&1u))return 1;
    if((channel!=1 && channel!=3) || (chcr&3u))return 0;
    uint32_t lo=madr&0x1ffffcu;
    uint64_t hi=(uint64_t)lo+4ull*remaining;
    if(hi>0x200000u)return 0;
    return !ttk_overlap(lo,(uint32_t)hi,0xab370,0xab388) &&
           !ttk_overlap(lo,(uint32_t)hi,0xb81bc,0xb8274) &&
           !ttk_overlap(lo,(uint32_t)hi,0xe811c,0xe8134) &&
           !ttk_overlap(lo,(uint32_t)hi,entry,entry+2);
}
/* Whole side-effect-free loop iterations, leaving the timeout nonzero and
 * stopping before the next hardware event. No skipped iteration reaches it. */
static inline uint32_t ttk_poll_iterations(uint32_t quantum,uint64_t distance,uint32_t timeout) {
    if(!quantum || distance<=quantum || timeout<=1)return 0;
    uint64_t count=(distance-1)/quantum;
    if(count>timeout-1u)count=timeout-1u;
    if(count>1200000u/quantum)count=1200000u/quantum;
    return (uint32_t)count;
}
#endif
