// D17T: the frontend (game mode 0x800bcbb0 == 0: title, attract demo, main
// menu, options, load game) runs one tick per 6 video fields (its flip waits
// 6; gameplay waits 2) and reads the pad once per tick. A press shorter than
// a tick was often never seen: on the title menu, 3-field Down taps moved the
// cursor 1 time in 8, 6-field presses 8 times in 8 (one step each), 12-field
// holds sometimes two steps (the original repeat). In Modernized every new
// frontend press is therefore held for at least 6 fields from its start;
// longer holds, gameplay and Vanilla are untouched.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "psx_sdl.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" uint64_t s_frame_count;

namespace ttk {
namespace {
constexpr uint32_t game_mode=0x800bcbb0;
constexpr uint64_t hold_fields=6;
uint64_t until[16];        // field until which each button stays pressed
uint16_t last=0xffff;      // previous host word (active low)
uint64_t held_taps=0;      // diagnostics: presses that were extended
double press_ms=0;         // D19C diagnostics: host time of the newest new press
uint64_t press_field=0;    // and its video field
bool enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_FRONTEND_HOLD");return !(t && !std::strcmp(t,"off"));}();return v;}
}

uint16_t frontend_pad_hold(uint16_t pad) {
    const uint64_t field=s_frame_count;
    if(!enabled() || !input_modernized() || psx_mod_read_half(game_mode)!=0) {
        last=pad; for(auto& u:until) u=0;
        return pad;
    }
    uint16_t out=pad;
    for(int b=0;b<16;++b) {
        const uint16_t bit=(uint16_t)(1u<<b);
        const bool down=!(pad&bit), was=!(last&bit);
        if(down && !was) {
            until[b]=field+hold_fields;
            press_ms=1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency();
            press_field=field;
        }
        if(!down && until[b] && field<until[b]) {
            if(was) ++held_taps;
            out&=(uint16_t)~bit;
        }
    }
    last=pad;
    return out;
}
} // namespace ttk

// D19C: host time (ms, SDL performance clock) of the newest frontend press.
extern "C" double ttk_frontend_press_ms() {return ttk::press_ms;}
extern "C" uint64_t ttk_frontend_press_field() {return ttk::press_field;}

extern "C" const char* ttk_frontend_input_json() {
    static char buf[96];
    std::snprintf(buf,sizeof buf,"{\"held_taps\":%llu}",(unsigned long long)ttk::held_taps);
    return buf;
}
