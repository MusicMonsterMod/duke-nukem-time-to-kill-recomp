// D19: settings the OPTIONS screens change while the game runs. A module that
// supports a live change reads its DNTTK_* setting through ttk_live_env()
// instead of getenv(): the value the menu last confirmed, else the launch
// environment. No dependencies, so module unit tests link it as is.
#include "live_env.h"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace {
struct Slot {std::atomic<const char*> name{nullptr},value{nullptr};};
Slot slots[32];
}

// D23AK (developer PSX_RW_LEAN=1): what a name resolves to (its slot, or the
// launch environment's value, which nothing changes after start: no setenv of
// a DNTTK_ name anywhere), kept per thread by the name's address (every caller
// passes a string literal) and the slot list's generation (a new name moves
// it). A slot's value is still read at each call. The result is the same
// pointer the full lookup returns.
namespace {
std::atomic<unsigned> slot_gen{1};
const char* live_env_full(const char* name,int& slot) {
    slot=-1;
    for(int i=0;i<32;++i) {
        const char* n=slots[i].name.load(std::memory_order_acquire);
        if(!n) break;
        if(!std::strcmp(n,name)) {slot=i;return slots[i].value.load(std::memory_order_acquire);}
    }
    return std::getenv(name);
}
bool live_env_lean() {static const bool v=[]{const char* e=std::getenv("PSX_RW_LEAN");const char* b=std::getenv("DNTTK_LEAN_BISECT");
    return e && e[0]=='1' && !(b && (std::strtoul(b,nullptr,0)&32u));}();return v;}
}

extern "C" const char* ttk_live_env(const char* name) {
    if(live_env_lean()) {
        struct Entry {const char* name;const char* env;unsigned gen;int slot;};
        thread_local Entry cache[64];
        Entry& e=cache[((uintptr_t)name>>3)&63];
        const unsigned gen=slot_gen.load(std::memory_order_acquire);
        if(e.name!=name || e.gen!=gen) {
            int slot;const char* v=live_env_full(name,slot);
            e={name,slot<0?v:nullptr,gen,slot};
            return v;
        }
        return e.slot>=0 ? slots[e.slot].value.load(std::memory_order_acquire) : e.env;
    }
    int slot;return live_env_full(name,slot);
}

// Values must stay valid for the life of the process (string literals).
extern "C" void ttk_live_env_set(const char* name,const char* value) {
    for(Slot& s:slots) {
        const char* n=s.name.load(std::memory_order_acquire);
        if(n && !std::strcmp(n,name)) {s.value.store(value,std::memory_order_release);return;}
        if(!n) {s.value.store(value,std::memory_order_release);s.name.store(name,std::memory_order_release);slot_gen.fetch_add(1,std::memory_order_acq_rel);return;}
    }
}

namespace {std::atomic<int> world_classic{-1};}
extern "C" int ttk_world_classic(void) {
    int v=world_classic.load(std::memory_order_relaxed);
    if(v<0) {
        const char* r=std::getenv("DNTTK_WORLD_RENDERER");
        v=r && !std::strcmp(r,"classic");
        world_classic.store(v,std::memory_order_relaxed);
    }
    return v;
}
extern "C" void ttk_world_classic_set(int classic) {world_classic.store(classic?1:0,std::memory_order_relaxed);}
extern "C" int ttk_env_switch(const char* name) {
    const char* t=std::getenv(name);
    if(!t || !*t) return -1;
    return !(t[0]=='0' && !t[1]);
}
// D23N1: the draw clock (live_env.h).
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
namespace {uint32_t draw_clock_held=0;}
extern "C" uint32_t ttk_draw_clock_ms(void) {return draw_clock_held ? draw_clock_held : (uint32_t)SDL_GetTicks();}
extern "C" void ttk_draw_clock_hold(uint32_t ms) {draw_clock_held=ms;}
