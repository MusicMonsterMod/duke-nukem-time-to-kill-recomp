// D17T: the frontend (game mode 0x800bcbb0 == 0: title, attract demo, main
// menu, options, load game) runs one tick per 6 video fields (its flip waits
// 6; gameplay waits 2) and reads the pad once per tick. A press shorter than
// a tick was often never seen: on the title menu, 3-field Down taps moved the
// cursor 1 time in 8, 6-field presses 8 times in 8 (one step each), 12-field
// holds sometimes two steps (the original repeat). In Modernized every new
// frontend press is therefore held for at least 6 fields from its start;
// longer holds, gameplay and Vanilla are untouched.
// D19E: the tick does not always come within 6 fields. When a demo scene
// starts loading behind a menu, the tick blocks in the CD load for about 110
// fields (the menu disappears behind the spinning logo), and later ticks of
// the load can be 45 fields apart; taps there were lost (3 of 9 measured).
// The tick decodes the pad (0x8001d5ac) into one history word per button at
// 0x800d1aa4 + 4 * bit, shifted each tick with bit 0 = pressed at that tick.
// A new press is therefore held until that word shows it was read (changed,
// bit 0 set), at least 6 fields and at most 3 seconds. In ordinary ticks this
// is the same 6 fields as before; in a stall the press lands at the next tick.
// Taps made while one is still waiting are queued (up to 6): once the tick has
// read the press, the button is held released until a tick reads that, then
// the next tap is pressed, so a stall never merges or drops taps. A tap made
// while the tick last read the button as pressed (its release not seen yet)
// likewise waits for a read release first.
// DNTTK_FRONTEND_SEEN=off (developer) keeps the D17T 6-field hold.
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
constexpr uint32_t pad_history=0x800d1aa4;   // + 4 * bit, bit 0 = pressed at the last tick
constexpr uint64_t hold_fields=6, max_fields=180;
constexpr int max_queued=6;
uint64_t until[16];        // field until which each button stays pressed
uint64_t started[16];      // field the current wait (press or release) started
bool pending[16];          // D19E: a press the tick has not read yet
bool releasing[16];        // D19E: a queued tap waits for the tick to read a release
int queued[16];            // D19E: taps made while one was still waiting
uint32_t history0[16];     // the button's history word when the wait started
uint64_t up_since[16];     // field the host button was last released
uint64_t waited_taps=0, waited_fields=0, queued_taps=0, dropped_taps=0;   // D19E diagnostics
uint16_t last=0xffff;      // previous host word (active low)
uint64_t held_taps=0;      // diagnostics: presses that were extended
double press_ms=0;         // D19C diagnostics: host time of the newest new press
uint64_t press_field=0;    // and its video field
bool enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_FRONTEND_HOLD");return !(t && !std::strcmp(t,"off"));}();return v;}
bool seen_enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_FRONTEND_SEEN");return !(t && !std::strcmp(t,"off"));}();return v;}
uint32_t history(int b) {return psx_mod_read_word(pad_history+4u*(uint32_t)b);}
void start_press(int b,uint64_t field) {
    until[b]=field+hold_fields; started[b]=field;
    history0[b]=history(b); pending[b]=seen_enabled() && history0[b]!=0xffffffffu;
    // The tick last read this button as pressed (it has not seen the previous
    // tap's release, e.g. after a stall): it must see a release first, or the
    // new press is no new edge to it.
    if(pending[b] && (history0[b]&1u)) {pending[b]=false; releasing[b]=true; ++queued[b]; until[b]=0;}
}
}

bool tracing() {static const bool v=std::getenv("DNTTK_FRONTEND_TRACE")!=nullptr;return v;}
void tr(const char* what,int b,uint64_t field) {
    if(tracing()) std::fprintf(stderr,"[fe-hold] %llu %s b%d hist=%x cursor=%u sub=%u q=%d\n",(unsigned long long)field,what,b,history(b),psx_mod_read_half(0x800d24e6),psx_mod_read_half(0x800d24e2),queued[b]);
}
uint16_t frontend_pad_hold(uint16_t pad) {
    const uint64_t field=s_frame_count;
    if(!enabled() || !input_modernized() || psx_mod_read_half(game_mode)!=0) {
        last=pad;
        for(int b=0;b<16;++b) {until[b]=0; pending[b]=releasing[b]=false; queued[b]=0; up_since[b]=0;}
        return pad;
    }
    uint16_t out=pad;
    for(int b=0;b<16;++b) {
        const uint16_t bit=(uint16_t)(1u<<b);
        const bool down=!(pad&bit), was=!(last&bit);
        if(!down && was) up_since[b]=field;
        // A release shorter than 2 fields (contact bounce, a flickering
        // input) continues the tap being delivered rather than adding one.
        const bool busy=pending[b] || releasing[b] || field<until[b];
        if(down && !was && !(busy && field-up_since[b]<2)) {
            press_ms=1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency();
            press_field=field;
            if(pending[b] || releasing[b]) {if(queued[b]<max_queued) {++queued[b]; ++queued_taps;} tr("queue",b,field);}
            else {start_press(b,field); tr("press",b,field);}
        }
        if(pending[b]) {
            const uint32_t h=history(b);
            if(h!=history0[b] && (h&1u)) {
                pending[b]=false; tr("seen",b,field);
                if(field>until[b]) {++waited_taps; waited_fields+=field-started[b];}
                // A queued tap: the tick must first see the button released.
                if(queued[b]) {releasing[b]=true; started[b]=field; history0[b]=h;}
            } else if(field-started[b]>=max_fields) {pending[b]=false; queued[b]=0; ++dropped_taps;}
        } else if(releasing[b]) {
            const uint32_t h=history(b);
            if(h!=history0[b] && !(h&1u)) {releasing[b]=false; --queued[b]; start_press(b,field); tr("release seen, press",b,field);}
            else if(field-started[b]>=max_fields) {releasing[b]=false; queued[b]=0; ++dropped_taps;}
        }
        if(releasing[b]) out|=bit;
        else if(!down && until[b] && (field<until[b] || pending[b])) {
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
    static char buf[192];
    std::snprintf(buf,sizeof buf,"{\"held_taps\":%llu,\"waited_taps\":%llu,\"waited_fields\":%llu,\"queued_taps\":%llu,\"dropped_taps\":%llu}",
        (unsigned long long)ttk::held_taps,(unsigned long long)ttk::waited_taps,(unsigned long long)ttk::waited_fields,(unsigned long long)ttk::queued_taps,(unsigned long long)ttk::dropped_taps);
    return buf;
}
