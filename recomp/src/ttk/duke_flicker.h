// D41AB: Duke Nukem 3D's random lights, as a lamp style
// (documentation/538-d41ab-duke3d-random-lights.md).
//
// Reimplemented from EDuke32 (source/duke3d/src/actors.cpp, case
// SE_4_RANDOM_LIGHTS; game.cpp A_Spawn and G_DoMoveThings):
// - Once a game tick (REALGAMETICSPERSEC, 30 a second) the game draws
//   g_globalRandom = krand() (0..65535). Every SE 4 reads that same value.
// - A light is lit that tick when ((g_globalRandom / (hitag + 1)) & 31) < 4:
//   one tick in eight, at random; hitag only picks other bits, so lamps with
//   different hitags flash apart and lamps with the same one together.
// - Lit, its sector's floor, ceiling and walls take shade
//   SE shade + (g_globalRandom & 15) (a random brightness each flash, from
//   the SE's own shade down by up to 15 steps); not lit, the sector's own
//   shades (the floor shade A_Spawn kept, and the walls' darkest).
// - The shade holds for the whole tick: no fade, no interpolation.
// - Build turns a shade into brightness linearly over its 32 shades
//   (Polymost getshadefactor: (numshades - clamp(shade, 0, numshades)) /
//   numshades; Duke 3D's palette has 32).
// The EDuke32 tutorial map's SE 4 rooms use SE shade 0 and sector shade 20,
// the defaults here. g_globalRandom is krand() after every other random draw
// of the tick, so successive ticks' values are unrelated; duke_global_random
// stands in for it with a hash of the tick, so every thread and every redraw
// of one moment agree and the result does not depend on the frame rate.
#pragma once
#include <cstdint>

namespace ttk {

constexpr uint32_t duke_ticks_per_second=30;   // REALGAMETICSPERSEC
constexpr int duke_numshades=32;
constexpr int duke_default_on_shade=0,duke_default_off_shade=20;

inline uint32_t duke_tick(uint32_t ms) {return (uint32_t)((uint64_t)ms*duke_ticks_per_second/1000u);}

// The tick's g_globalRandom (0..65535).
inline uint32_t duke_global_random(uint32_t tick) {
    uint32_t h=tick*0x9e3779b9u+0x7f4a7c15u;
    h^=h>>16;h*=0x85ebca6bu;h^=h>>13;h*=0xc2b2ae35u;h^=h>>16;
    return h>>16;
}

struct DukeLight {bool lit;int shade;};
// SE 4 at one tick.
inline DukeLight duke_random_light(uint32_t tick,int hitag,int on_shade,int off_shade) {
    const uint32_t r=duke_global_random(tick);
    const uint32_t div=(uint32_t)(hitag<0?0:hitag)+1u;
    if(((r/div)&31u)<4u) return {true,on_shade+(int)(r&15u)};
    return {false,off_shade};
}

inline float duke_shade_factor(int shade) {
    const int s=shade<0?0:shade>duke_numshades?duke_numshades:shade;
    return (float)(duke_numshades-s)/(float)duke_numshades;
}

// The lamp's level at a Duke tick: 1 at shade 0.
inline float duke_random_light_level_at(uint32_t tick,int hitag,int on_shade=duke_default_on_shade,int off_shade=duke_default_off_shade) {
    return duke_shade_factor(duke_random_light(tick,hitag,on_shade,off_shade).shade);
}
// ... at a moment (ms on the draw clock).
inline float duke_random_light_level(uint32_t ms,int hitag,int on_shade=duke_default_on_shade,int off_shade=duke_default_off_shade) {
    return duke_random_light_level_at(duke_tick(ms),hitag,on_shade,off_shade);
}

// The long-run mean and the largest stray from it (the bake's channel choice).
inline float duke_random_light_mean(int on_shade,int off_shade) {
    float lit=0;
    for(int k=0;k<16;++k) lit+=duke_shade_factor(on_shade+k);
    return 0.125f*(lit/16.0f)+0.875f*duke_shade_factor(off_shade);
}
inline float duke_random_light_swing(int on_shade,int off_shade) {
    const float m=duke_random_light_mean(on_shade,off_shade);
    float hi=duke_shade_factor(off_shade),lo=hi;
    for(int k=0;k<16;++k) {const float f=duke_shade_factor(on_shade+k);hi=f>hi?f:hi;lo=f<lo?f:lo;}
    const float a=hi-m,b=m-lo;
    return a>b?a:b;
}

}  // namespace ttk
