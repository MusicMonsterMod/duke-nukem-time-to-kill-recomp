// D41M: face record -> baked surface, readable by the redraw workers.
//
// The workers are forked copies of this process (render_worker.c); a table
// built after the fork would not reach them. The map lives in a shared
// anonymous mapping made at startup, before any fork, in two halves: the
// game thread writes the idle half and then flips the index, so a worker
// reading mid-update sees one whole table. Elsewhere (no fork) it is plain
// memory.
#include "lamp_bake_map.h"

#include <algorithm>
#include <atomic>
#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#define LBM_SHARED 1
#endif

namespace ttk {
namespace {

constexpr uint32_t lbm_cap = 1u << 17;   // faces (level 0: about 25,000 triangles)
struct Half {uint32_t n;uint32_t pad;uint64_t key[lbm_cap];};   // record << 32 | surface
struct Shared {std::atomic<uint32_t> active;uint32_t pad;Half half[2];};
Shared* lbm=nullptr;

Shared* lbm_get() {
    if(lbm) return lbm;
#ifdef LBM_SHARED
    void* m=mmap(nullptr,sizeof(Shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(m!=MAP_FAILED) lbm=new(m) Shared();
#endif
    if(!lbm) lbm=new Shared();
    lbm->active.store(0);lbm->half[0].n=lbm->half[1].n=0;
    return lbm;
}

// Before the workers fork (static initialisation).
struct Init {Init(){lbm_get();}} lbm_init;

} // namespace

void lamp_bake_map_publish(const std::vector<std::pair<uint32_t,uint32_t>>& records) {
    Shared* s=lbm_get();
    const uint32_t idle=s->active.load(std::memory_order_acquire)^1u;
    Half& h=s->half[idle];
    const size_t n=std::min<size_t>(records.size(),lbm_cap);
    for(size_t i=0;i<n;++i) h.key[i]=(uint64_t)records[i].first<<32|records[i].second;
    std::sort(h.key,h.key+n);
    h.n=(uint32_t)n;
    s->active.store(idle,std::memory_order_release);
}

void lamp_bake_map_clear() {
    Shared* s=lbm_get();
    const uint32_t idle=s->active.load(std::memory_order_acquire)^1u;
    s->half[idle].n=0;
    s->active.store(idle,std::memory_order_release);
}

// D23N1: a redraw job reads one table from start to end. A publish during a
// job (the level's first bake appearing) takes effect at the next job, never
// half way through one, and a rebuilt job sees what the job it rebuilds saw.
// The writer only ever rewrites the idle half, so the pinned half stays whole
// until the next publish but one (seconds apart: a bake takes seconds).
int lbm_pinned=-1;
void lamp_bake_pin(bool on) {lbm_pinned=on && lbm ? (int)(lbm->active.load(std::memory_order_acquire)&1u) : -1;}

uint32_t lamp_bake_surface(uint32_t record) {
    const Shared* s=lbm;
    if(!s) return 0;
    const Half& h=s->half[lbm_pinned>=0 ? (unsigned)lbm_pinned : s->active.load(std::memory_order_acquire)&1u];
    if(!h.n) return 0;
    const uint64_t lo=(uint64_t)record<<32;
    const uint64_t* p=std::lower_bound(h.key,h.key+h.n,lo);
    return p!=h.key+h.n && (uint32_t)(*p>>32)==record ? (uint32_t)*p : 0u;
}

} // namespace ttk
