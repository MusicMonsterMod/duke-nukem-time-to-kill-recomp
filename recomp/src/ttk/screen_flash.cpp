// D39B: Duke3D palfrom-style screen flashes (screen_flash.h, note 370).
// The record lives in mod memory; Polymorph's present tint draws it.
#include "screen_flash.h"
#include "near_clip.h"
#include "mod_plugins.h"
#include "psx_sdl.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ttk {
namespace {
constexpr uint32_t colour_tag=0x39000000u, level_tag=0x5d390000u;
constexpr uint32_t frame_delta=0x800d21fc, game_mode=0x800bcbb0, paused=0x800be568, suspended=0x800d2540;
double now_ms() {return 1000.0*(double)SDL_GetPerformanceCounter()/(double)SDL_GetPerformanceFrequency();}
// Host time and frame delta of the last player update (live fade).
double tick_ms=0; uint32_t tick_delta=flash_tic; uint64_t updates=0, starts=0;
std::atomic<uint32_t> requested{0};
uint32_t held=0;
struct Note {double t; float w; int16_t slot; bool drawn;};
Note notes[2048]; unsigned notes_n=0;
}

FlashState screen_flash_read() {
    FlashState s;
    const uint32_t a=near_clip_flash_record();
    if(!a) return s;
    const uint32_t c=psx_mod_read_word(a), l=psx_mod_read_word(a+4);
    if((c&0xff000000u)!=colour_tag || (l&0xffff0000u)!=level_tag) return s;
    s.level=std::min<uint32_t>(l&0xffff,flash_level_max);
    s.seq=(uint8_t)((c>>18)&0x3f); s.r=(uint8_t)((c>>12)&0x3f); s.g=(uint8_t)((c>>6)&0x3f); s.b=(uint8_t)(c&0x3f);
    return s;
}
namespace {
void write(const FlashState& s) {
    const uint32_t a=near_clip_flash_record();
    if(!a) return;
    psx_mod_write_word(a,colour_tag|(uint32_t)(s.seq&0x3f)<<18|(uint32_t)(s.r&0x3f)<<12|(uint32_t)(s.g&0x3f)<<6|(s.b&0x3f));
    psx_mod_write_word(a+4,level_tag|std::min(s.level,flash_level_max));
}
}
void screen_flash_palfrom(int intensity,int r,int g,int b) {
    FlashState s=screen_flash_read();
    s.seq=(uint8_t)((s.seq+1)&0x3f);
    s.level=(uint32_t)std::clamp(intensity,0,255)*flash_tic;
    s.r=(uint8_t)std::clamp(r,0,63); s.g=(uint8_t)std::clamp(g,0,63); s.b=(uint8_t)std::clamp(b,0,63);
    write(s);
    ++starts;
}
void screen_flash_request(int intensity,int r,int g,int b,bool hold) {
    requested.store(1u<<31|(hold?1u<<30:0u)|(uint32_t)std::clamp(intensity,0,255)<<18|(uint32_t)std::clamp(r,0,63)<<12|(uint32_t)std::clamp(g,0,63)<<6|(uint32_t)std::clamp(b,0,63));
}
// Before anything this update starts: a flash started now is drawn at its
// full intensity with this frame, then falls one step per tic.
void screen_flash_update() {
    FlashState s=screen_flash_read();
    if(s.level) {
        const int16_t delta=(int16_t)psx_mod_read_half(frame_delta);
        if(delta>0) {s.level=(uint32_t)delta<s.level?s.level-(uint32_t)delta:0; write(s); tick_delta=(uint32_t)delta;}
    }
    tick_ms=now_ms(); ++updates;
    uint32_t q=requested.exchange(0);
    if(q) held=q&(1u<<30)?q:0;
    else q=held;
    if(q)
        screen_flash_palfrom((int)((q>>18)&0xff),(int)((q>>12)&0x3f),(int)((q>>6)&0x3f),(int)(q&0x3f));
}
bool screen_flash_steps() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_FLASH_STEPS");return t && t[0]=='1';}();
    return v;
}
bool screen_flash_live(float rgbw[4]) {
    if(psx_mod_read_half(game_mode)!=1 || psx_mod_read_half(suspended)) return false;
    const FlashState s=screen_flash_read();
    if(!s.level) return false;
    const bool hold=psx_mod_read_half(paused)!=0;   // paused: frozen, still drawn (Duke3D)
    const double since=now_ms()-tick_ms;
    if(!hold && since>500.0) return false;           // no player update: a load or a scene
    double level=s.level;
    // Carry the fade on from the last update by at most one update's step
    // (exactly where the next update lands), so 60 Hz presents of a 30 Hz
    // game frame show two levels, not one.
    if(!hold && !screen_flash_steps()) level-=std::min(since*0.3,(double)tick_delta);
    return flash_rgbw(s,level,rgbw);
}
void screen_flash_note(int slot,const float rgbw[4],bool drawn) {
    notes[notes_n++%2048]={now_ms(),drawn?rgbw[3]:0.f,(int16_t)slot,drawn};
}
const char* screen_flash_json(uint64_t asked,uint64_t drawn,const float last[4]) {
    static char buffer[65536];
    const FlashState s=screen_flash_read();
    int n=std::snprintf(buffer,sizeof buffer,"{\"record\":\"0x%08x\",\"level\":%u,\"rgb\":[%u,%u,%u],\"seq\":%u,\"steps\":%s,\"updates\":%llu,\"starts\":%llu,"
        "\"tint_asked\":%llu,\"tint_drawn\":%llu,\"tint_last\":[%.4f,%.4f,%.4f,%.5f],\"notes\":\"",
        near_clip_flash_record(),s.level,s.r,s.g,s.b,s.seq,screen_flash_steps()?"true":"false",(unsigned long long)updates,(unsigned long long)starts,
        (unsigned long long)asked,(unsigned long long)drawn,last[0],last[1],last[2],last[3]);
    // Per present, oldest first: "host ms/slot/weight".
    const unsigned first=notes_n>2048?notes_n-2048:0;
    for(unsigned i=first;i<notes_n && n<(int)sizeof buffer-48;++i) {
        const Note& e=notes[i%2048];
        n+=std::snprintf(buffer+n,sizeof buffer-n,"%.3f/%d/%.5f ",e.t,e.slot,e.w);
    }
    std::snprintf(buffer+n,sizeof buffer-n,"\"}");
    return buffer;
}
}
