// D19: settings the OPTIONS screens change while the game runs. A module that
// supports a live change reads its DNTTK_* setting through ttk_live_env()
// instead of getenv(): the value the menu last confirmed, else the launch
// environment. No dependencies, so module unit tests link it as is.
#include "live_env.h"
#include <atomic>
#include <cstdlib>
#include <cstring>

namespace {
struct Slot {std::atomic<const char*> name{nullptr},value{nullptr};};
Slot slots[32];
}

extern "C" const char* ttk_live_env(const char* name) {
    for(Slot& s:slots) {
        const char* n=s.name.load(std::memory_order_acquire);
        if(!n) break;
        if(!std::strcmp(n,name)) return s.value.load(std::memory_order_acquire);
    }
    return std::getenv(name);
}

// Values must stay valid for the life of the process (string literals).
extern "C" void ttk_live_env_set(const char* name,const char* value) {
    for(Slot& s:slots) {
        const char* n=s.name.load(std::memory_order_acquire);
        if(n && !std::strcmp(n,name)) {s.value.store(value,std::memory_order_release);return;}
        if(!n) {s.value.store(value,std::memory_order_release);s.name.store(name,std::memory_order_release);return;}
    }
}
