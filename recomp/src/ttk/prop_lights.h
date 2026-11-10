// D41L: props that are light sources (prop_lights.cpp). See
// documentation/436-d41l-saloon-light.md.
//
// Recognised by their models, so every copy in every level takes part: the
// iron woodburner (a fire behind its grate) and the candle chandelier.
// The near-clip object path, the second texture bank and the light list call
// the module through these pointers, set by its constructor, so test
// binaries that link near_clip.cpp or modern_controls.cpp alone need nothing.
#pragma once
#include <atomic>
#include <cmath>
#include <cstdint>
#include "mod_plugins.h"

namespace ttk {
enum {prop_light_none=0,prop_light_stove=1,prop_light_chandelier=2};
// The model kind of an object mesh (prop_light_none for every other mesh).
inline int (*prop_light_kind)(uint32_t mesh)=nullptr;
// Whether the drawn extras (flames, the grate's glow) are on: Modernized,
// Polymorph, PROP LIGHTS not off.
inline bool (*prop_light_visuals)()=nullptr;
// The props' lights for this image, nearest first; shown(room) is the room
// gate (a light whose room Polymorph does not show fades out).
inline unsigned (*prop_light_publish)(PSXModLight* out,unsigned room,bool (*shown)(uint32_t room))=nullptr;
// Glowing extras drawn with a prop (flames, halos, the grate's embers), in
// the mesh's model space, animated from the clock (SDL ticks, as the lights): billboard quads face the
// camera (p[0] the anchor; anchor_y 1 = the quad's bottom edge sits on it,
// 0.5 = centred; sway moves the top edge sideways), others are the quad
// p[0..3] (corners in PS1 order 0 1 2 3: top-left, top-right, bottom-left,
// bottom-right). u, v, tw, th: the texture rectangle in the page at
// prop_light_tpage. rgb: vertex colour (128 = the texture as painted).
struct PropGlow {bool billboard;float p[4][3];float w,h,anchor_y,sway;int u,v,tw,th;uint8_t rgb[3];};
inline unsigned (*prop_light_glows)(uint32_t mesh,int kind,PropGlow* out,unsigned max)=nullptr;
// Paints the flame, halo and ember textures into the second bank.
inline void (*prop_light_paint_bank)(uint16_t* bank)=nullptr;

// Console "lamp props off|on" (session, for comparison).
inline std::atomic<bool> prop_lights_enabled{true};
// D56: console "lamp pickups off|on" (glowing pickups, session).
inline std::atomic<bool> prop_pickups_enabled{true};

// Second-bank textures (15-bit, semi-transparent texels), page base x 256:
// tpage 0x124 = x 256, 15-bit, additive blending.
constexpr uint32_t prop_light_tpage=0x124;
constexpr int prop_flame_u=0,prop_flame_w=16,prop_flame_h=32;     // a candle flame, base at the bottom
constexpr int prop_halo_u=16,prop_halo_w=32,prop_halo_h=32;       // a soft round glow
constexpr int prop_ember_u=48,prop_ember_w=32,prop_ember_h=16;    // embers behind a grate, hottest low

// Smooth value noise, 0..1, from a time in steps and a seed.
inline float prop_light_noise(double t,uint32_t seed) {
    const double fl=std::floor(t);const uint32_t i=(uint32_t)(int64_t)fl;
    auto h=[seed](uint32_t k){uint32_t x=(k+seed*0x9e3779b9u)*2654435761u;x^=x>>15;x*=0x2c1b3c6du;x^=x>>12;return (float)(x&0xffff)/65535.0f;};
    const float f=(float)(t-fl),s=f*f*(3-2*f);
    return h(i)+(h(i+1)-h(i))*s;
}
// A wood fire, about 0.55..1.0: a slow breath, a lively flutter and now and
// then a lick of flame. ms: a clock in milliseconds (the same in redraw
// workers and the light list).
inline float prop_fire_level(uint32_t ms,uint32_t seed) {
    const double t=ms/1000.0;
    float v=0.80f+0.08f*std::sin((float)(t*2*3.14159265*0.55+seed));
    v+=0.16f*(prop_light_noise(t*7.0,seed)-0.5f)+0.10f*(prop_light_noise(t*17.0,seed+7)-0.5f);
    v+=0.10f*std::max(0.0f,prop_light_noise(t*2.3,seed+13)-0.6f)*2.5f;
    return std::fmin(1.0f,std::fmax(0.5f,v));
}
// Candles: nearly steady, a soft shimmer (about 0.9..1.0).
inline float prop_candle_level(uint32_t ms,uint32_t seed) {
    const double t=ms/1000.0;
    return 0.95f+0.035f*(prop_light_noise(t*5.0,seed)-0.5f)*2+0.015f*std::sin((float)(t*2*3.14159265*0.31+seed));
}
}  // namespace ttk
