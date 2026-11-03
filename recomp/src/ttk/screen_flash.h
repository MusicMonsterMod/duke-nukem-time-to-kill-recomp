#pragma once
#include <algorithm>
#include <cstdint>
namespace ttk {
// D39B: Duke3D palfrom-style screen flashes, drawn by Polymorph's present
// tint (mod_plugins.h psx_mod_set_present_tint). Spec: note 370.
//
// The flash record is two words of mod memory after the D08G7 word, so
// savestates and redraw workers carry it; a state without them reads none.
// level is the palfrom intensity x 10 in frame-delta units (300 a second,
// 0x800d21fc): one 30 Hz tic is 10, so a fade lasts the same time when the
// game runs at 20 fps. seq changes with every flash started (a new flash
// replaces the old one at once, never blended into it).
struct FlashState {uint32_t level=0; uint8_t r=0,g=0,b=0,seq=0;};
constexpr uint32_t flash_tic=10, flash_level_max=255*flash_tic;

// The weight and colour of a flash state, Duke3D's measured blend:
// w = min(I, 63) / 64, colour = 4 x the 6-bit component (display values).
// Level in tenths of a tic (not rounded to tics: the smooth fade).
inline bool flash_rgbw(const FlashState& s,double level,float rgbw[4]) {
    if(level<=0.0) return false;
    rgbw[0]=(float)(4*s.r)/255.f; rgbw[1]=(float)(4*s.g)/255.f; rgbw[2]=(float)(4*s.b)/255.f;
    rgbw[3]=(float)(std::min(level/flash_tic,63.0)/64.0);
    return true;
}
// The flash shown at alpha (0..1) between two published game frames, a and
// the next one b, as the redraw for that present blends their poses. A fade
// interpolates linearly, so it is smooth at any display rate. A flash started
// in b appears with b (alpha 0 of the next pair), as Duke3D shows a new flash
// first on the frame of the tic that started it. steps: Duke3D's 30 Hz
// staircase (each game frame's own level).
inline bool flash_mix(const FlashState& a,const FlashState& b,double alpha,bool steps,float rgbw[4]) {
    if(steps || b.seq!=a.seq || b.level>a.level) return flash_rgbw(a,a.level,rgbw);
    alpha=std::clamp(alpha,0.0,1.0);
    return flash_rgbw(a,a.level+((double)b.level-(double)a.level)*alpha,rgbw);
}

// Live record (mod memory); all zero without one.
FlashState screen_flash_read();
// palfrom <intensity> <r> <g> <b>: replaces the current flash. Components
// 0..63, intensity 0..255 (above 63 holds full strength longer).
void screen_flash_palfrom(int intensity,int r,int g,int b);
// One player update (Modernized): minus the game's frame delta.
void screen_flash_update();
// D39B developer setting DNTTK_FLASH_STEPS=1: the 30 Hz staircase.
bool screen_flash_steps();
// A present outside replay presentation (60 Hz, the real image, pause):
// the live record, the fade carried on from its last player update.
bool screen_flash_live(float rgbw[4]);
// Diagnostic: one present's tint (slot as the provider's, w the weight).
void screen_flash_note(int slot,const float rgbw[4],bool drawn);
// asked/drawn/last: the renderer's tint counters (gl_renderer_present_tint_diag).
const char* screen_flash_json(uint64_t asked,uint64_t drawn,const float last[4]);
// Diagnostic input (DNTTK_TEST_INPUT flash=I,r,g,b[,hold]): started at the
// next update; hold 1 starts it again every update (Duke3D's palfrom each
// tic: a steady level) until the next request.
void screen_flash_request(int intensity,int r,int g,int b,bool hold=false);
}
