#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
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

// D39C: the player's Screen Flashes option (DNTTK_SCREEN_FLASHES, live).
// The record and its fade run the same under every option (replay and
// savestates never depend on it); the option only scales what is drawn.
enum class FlashOption {On, Reduced, Off};
// Reduced: half the measured weight, never above a quarter (palfrom 16):
// god mode off (32) draws as 16, a pickup (16) as 8, white 63 as 16.
constexpr float flash_reduced_cap=0.25f;
inline bool flash_option_rgbw(FlashOption o,float rgbw[4]) {
    if(o==FlashOption::Off) return false;
    if(o==FlashOption::Reduced) rgbw[3]=std::min(rgbw[3]*0.5f,flash_reduced_cap);
    return rgbw[3]>0.f;
}
inline FlashOption flash_option_parse(const char* v) {
    if(v && (!std::strcmp(v,"off") || !std::strcmp(v,"0"))) return FlashOption::Off;
    if(v && !std::strcmp(v,"reduced")) return FlashOption::Reduced;
    return FlashOption::On;
}
inline const char* flash_option_name(FlashOption o) {
    return o==FlashOption::Off?"off":o==FlashOption::Reduced?"reduced":"on";
}
// Console arguments "<intensity> [r] [g] [b]", Duke3D palfrom's: whole
// numbers, intensity 0..255, components 0..63, omitted components 0. Returns
// nullptr and fills v, or the reason the line was refused.
inline const char* flash_parse_palfrom(const char* text,int v[4]) {
    v[0]=v[1]=v[2]=v[3]=0;
    int n=0;
    for(const char* p=text;;) {
        while(*p==' ' || *p=='\t') ++p;
        if(!*p) break;
        if(n==4) return "at most four numbers: intensity, red, green, blue";
        long x=0; int digits=0;
        for(;*p>='0' && *p<='9';++p,++digits) if((x=x*10+(*p-'0'))>9999) x=9999;
        if(!digits || (*p && *p!=' ' && *p!='\t')) return "whole numbers only";
        if(x>(n?63:255)) return n?"colour components are 0..63":"intensity is 0..255";
        v[n++]=(int)x;
    }
    return n?nullptr:"an intensity is needed";
}

// D39E: the flash for one real hit on Duke, from the original handler's
// damage type (0x800a40a8 a3; the families are its own switch, 0x80018aec),
// the attacker's object type and projectile row (0x800c4bc0) when known, and
// what the hit took: health plus armour in the game's units (10000 is 100%),
// Duke's death, or (type 7, the Freezer) freezing him. Duke3D's palfrom
// values from note 370's catalog; intensity 0 is no flash. Note 370, D39E.
struct DamageFlash {int intensity=0,r=0,g=0,b=0; const char* kind="none";};
constexpr int damage_heavy_hit=300, damage_heavy_blast=1500, damage_heavy_fall=2500;
// Necro (object type 50, TTK's Octabrain) and its bolt (projectile row 20):
// Duke3D's Octabrain blast, COOLEXPLOSION1, palfrom 48 48 0 48.
constexpr unsigned damage_necro_type=50; constexpr int damage_necro_projectile=20;
// Frozen: EDuke32's frozen colour, palfrom 16 0 0 24 (out = 0.75 x in, blue
// + 24), given every frame while frozen and gone at once on the thaw (user,
// 2026-11-03: "it absolutely does not need to fade out. in eduke it's
// instant"). Fitted from research/screencaps/freezethrow.mp4: every channel
// x0.75 and blue +23 on the same view (moment match; the frozen eye is lower).
constexpr DamageFlash damage_frozen{16,0,0,24,"freeze"};
inline DamageFlash damage_flash_for(unsigned type,int lost,bool died,bool frozen=false,unsigned source_type=0,int projectile=-1) {
    if(type==7)                                     // the Freezer: the hit freezes Duke
        return frozen || lost>0 || died?damage_frozen:DamageFlash{};
    if(lost<=0 && !died) return {};                 // cancelled or absorbed: no hit
    if(source_type==damage_necro_type || projectile==damage_necro_projectile)
        return {48,48,0,48,"necro"};
    switch(type) {
    case 4: case 13: case 16:                       // blasts (the knockback branch)
        return lost>=damage_heavy_blast || died?DamageFlash{48,52,0,0,"blast"}:DamageFlash{32,32,0,0,"blast"};
    case 11:                                        // a fall
        return lost>=damage_heavy_fall || died?DamageFlash{32,32,0,0,"fall"}:DamageFlash{16,32,0,0,"fall"};
    case 12:                                        // toxic gas, every frame in it
        return {16,24,32,0,"gas"};
    default:                                        // shots, blows and the rest
        return lost>=damage_heavy_hit || died?DamageFlash{32,32,0,0,"hit"}:DamageFlash{16,32,0,0,"hit"};
    }
}
// Diagnostic: the last damage hit and what it drew (debug JSON "damage").
void screen_flash_damage_note(const char* note);
// Developer (DNTTK_TEST_INPUT hurt=type,amount): one hit through Duke's own
// damage handler at the next player update, to test each damage type.
void screen_flash_test_hurt(int type,int amount);
bool screen_flash_take_hurt(int& type,int& amount);

// Live record (mod memory); all zero without one.
FlashState screen_flash_read();
// D39C game API, Duke3D palfrom <intensity> <r> <g> <b> (emulation thread,
// Modernized player update): replaces the current flash, weaker or stronger;
// the last call in a game frame wins. Components 0..63, intensity 0..255
// (above 63 holds full strength for I - 63 tics; 0 clears the flash).
void screen_flash(int intensity,int r=0,int g=0,int b=0);
FlashOption screen_flash_option();
// One player update (Modernized): minus the game's frame delta.
void screen_flash_update();
// The world renderer is Classic (DNTTK_WORLD_RENDERER, fixed at launch): no
// present tint (D39G decides Classic's flashes).
bool screen_flash_classic();
// D39B developer setting DNTTK_FLASH_STEPS=1: the 30 Hz staircase.
bool screen_flash_steps();
// A present outside replay presentation (60 Hz, the real image, pause):
// the live record, the fade carried on from its last player update.
bool screen_flash_live(float rgbw[4]);
// Diagnostic: one present's tint (slot as the provider's, w the weight).
void screen_flash_note(int slot,const float rgbw[4],bool drawn);
// asked/drawn/last: the renderer's tint counters (gl_renderer_present_tint_diag).
const char* screen_flash_json(uint64_t asked,uint64_t drawn,const float last[4]);
// Another thread (the console, DNTTK_TEST_INPUT flash=I,r,g,b[,hold]): the
// flash starts at the next player update; hold 1 starts it again every
// update (Duke3D's palfrom each tic: a steady level) until the next request.
void screen_flash_request(int intensity,int r,int g,int b,bool hold=false);
}
