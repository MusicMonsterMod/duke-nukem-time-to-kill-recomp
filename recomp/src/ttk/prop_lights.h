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
// D57G: prop_light_ring, the five-candle iron-ring chandelier (prop type 372).
// D57F: prop_light_portal_top, the time machine's hanging top (prop type 192,
// levels 0 and 5): no light of its own, darker metal, a glow at its centre.
// D41Y: prop_light_barricade, the construction barricade (prop type 128):
// a halo over its warning lamp while the game's own blink shows it lit.
// D41Z: prop_light_cage, the caged wall lamp (prop type 667): a soft halo
// over its bulb.
// D57C: prop_light_globe, the subway entrance globe lamp (prop type 315):
// a soft halo over the globe.
// D41AA: prop_light_train, the subway train (prop type 77): halos over its
// bumper lamps, roof beacon and headlight, and its crash flash and sparks.
// D57I: prop_light_street, the LA street lamp post (prop type 160): a soft
// sodium-orange glow on its lens.
// D57J: the LA strip club's fixtures (levels 0, 5 and 9): prop_light_neon_bar,
// the ceiling bar with its magenta neon tube and four spot cans (prop type
// 193); prop_light_canopy, the red lampshade over a pole stage (prop type
// 117); prop_light_club_bar, the back bar's shelves (prop type 4). Their
// rooms are nightclub rooms (club_lights.inc); the bar and the canopy carry
// haze glows, the back bar only marks its room.
// D57D: prop_light_table_lamp, the shaded table lamp (prop type 228; level
// 0, the strip club's back room and a furnished room off the street): a
// soft warm glow on its shade.
// D57K: prop_light_pendant, the hanging ceiling lamp (prop type 229; level
// 0 room 8, the LA apartment): a soft warm glow round its bulb.
enum {prop_light_none=0,prop_light_stove=1,prop_light_chandelier=2,prop_light_ring=3,prop_light_portal_top=4,prop_light_barricade=5,
      prop_light_cage=6,prop_light_train=7,prop_light_globe=8,prop_light_street=9,
      prop_light_neon_bar=10,prop_light_canopy=11,prop_light_club_bar=12,prop_light_table_lamp=13,prop_light_pendant=14};
inline bool prop_light_club_kind(int kind) {return kind>=prop_light_neon_bar && kind<=prop_light_club_bar;}
// D57J: the club look (club_lights.inc and the haze glows here): console
// "lamp club off|on" (session; off = the lamps and look before D57J).
inline std::atomic<bool> club_lights_enabled{true};
// D57L: the LA apartment's mood lighting and lamps that follow their room's
// switch (prop_lights.cpp, club_lights.inc); "lamp apartment off|on".
inline std::atomic<bool> apartment_lights_enabled{true};
// D57L: the prop record being drawn when prop_light_glows runs (0: not known),
// so a lamp's halo can follow its own room (near_clip.cpp sets it).
inline thread_local uint32_t prop_glow_object=0;
// D57J: the neon bar's spot cans, in the bar's model space: the lens
// centres (x of can i, y, z), the aim (down 25 degrees, along model -z) and
// each can's gel (r g b). Shared by the gel lamps (club_lights.inc) and
// the lens halos and beams (prop_lights.cpp).
constexpr float club_can_x[4]={-1600,-550,550,1600},club_can_y=232,club_can_z=-160;
constexpr float club_can_aim[3]={0,0.42f,-0.91f};
inline float club_gel[4][3]={{1.0f,0.10f,0.55f},{0.30f,0.22f,1.0f},{0.30f,0.22f,1.0f},{1.0f,0.10f,0.55f}};
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
// D41Q: an upright billboard (a flame) stands along the prop's own up axis
// (model -y) as the camera sees it, turned to face the camera about that
// axis, so its foot stays on the candle and its tip above it from any angle;
// h then foreshortens when looked at along the axis (not below 40%).
struct PropGlow {bool billboard;float p[4][3];float w,h,anchor_y,sway;int u,v,tw,th;uint8_t rgb[3];bool upright;};
inline unsigned (*prop_light_glows)(uint32_t mesh,int kind,PropGlow* out,unsigned max)=nullptr;
// D57F: the vertex colour scale for a mesh of this kind (1: as the game
// lit it), when the drawn extras are on.
inline float (*prop_light_tone)(int kind)=nullptr;
// Paints the flame, halo and ember textures into the second bank.
inline void (*prop_light_paint_bank)(uint16_t* bank)=nullptr;

// D57F: the level lamp bound to a prop (D41D1 sign lamps: each energy crystal
// has one at the holder it starts in). Returns 2 when a lamp is bound to a
// prop of this type at exactly p (it lights the prop there), 1 when the level
// has a lamp bound to this type elsewhere (*like filled from it: colour,
// radius, strength and its offset from its prop), 0 when none, -1 when the
// level's lamps are not loaded. Game thread. Set by modern_controls.cpp.
struct BoundLamp {float rgb[3],radius,k,off[3];};
inline int (*level_lamp_bound)(uint16_t type,const int32_t p[3],BoundLamp* like)=nullptr;
// D57F: console "lamp crystals off|on" (moved crystals' lights, session).
inline std::atomic<bool> prop_crystals_enabled{true};
// D41X: a prop model whose light this file gives (the fire barrel), so the
// D41D1 sign scan ("lamp signs") makes no second lamp for it.
// D41Y: and the construction barricade's warning lamp.
inline bool (*prop_light_lit_mesh)(uint32_t mesh)=nullptr;
// D41Y: the D55 signal reading of a mesh's animated faces (level 0-1 of the
// frame shown against its brightest frame, and the lit colour); false when
// the mesh has none. Game thread. Set by modern_controls.cpp.
inline bool (*prop_signal_now)(uint32_t mesh,float& level,float rgb[3])=nullptr;

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
// D41S: a neutral halo (tinted by the vertex colour: glows at flames and
// missiles) and a black semi-transparent patch (planar model shadows, drawn
// with prop_shade_tpage: the same page in mode 0, half the background).
constexpr int prop_white_halo_u=80,prop_white_halo_w=32,prop_white_halo_h=32;
constexpr int prop_shade_u=112,prop_shade_w=4,prop_shade_h=4;
// D57J: a soft light shaft for haze (club spot cans): brightest at the top
// edge (the lens), widening and fading downwards, soft at the sides.
constexpr int prop_beam_u=128,prop_beam_w=32,prop_beam_h=32;
constexpr uint32_t prop_shade_tpage=0x104;

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
