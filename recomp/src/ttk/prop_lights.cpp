// D41L: props that are light sources (prop_lights.h). See
// documentation/436-d41l-saloon-light.md.
//
// Time to Kill paints its room light into vertex colours; a saloon's
// woodburner and candle chandelier give none. In Modernized (Polymorph):
//
// - The iron woodburner (28 vertices, a 540 x 460 body on a pipe, the grate
//   on the door face at model +z) burns: embers glow behind its grate (an
//   additive quad over it) with a faint bloom in front, and a low fire light
//   flickers on the floor and walls before it.
// - The candle chandelier (twelve radial arm planes, 896 across, a candle at
//   each arm's tip) is lit: a small flame on every candle, each with a soft
//   halo, a warm light among the candles for the ceiling and arms, and a
//   broad soft fill that reaches the room below.
//
// D57F: an energy crystal (types 176-178) set into a crystal holder other
// than the one it starts in (the time machine's three holders in OBEY OR
// DIE) shines there as in its first holder: a light in the
// colour, reach and strength of the level lamp bound to it there (D41D1 sign
// lamp), at the same offset; it goes when the crystal is taken.
//
// Both are found by their model data, so every copy in every level takes
// part. The lights join the D41A list (DYNAMIC LIGHTS on) after the level
// lamps and the D53 screens, nearest to Duke first, gated by the room
// Polymorph shows; the flames and glows are drawn by the near-clip object
// path from the second texture bank (x 256..335, painted here). Classic and
// Vanilla: nothing changes.
#include "prop_lights.h"
#include "prop_state.h"
#include "live_env.h"
#include "pc_input.h"
#include "cpu_state.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace ttk {
namespace {
constexpr uint32_t players=0x800d7198u,player_index=0x800d2654u;
constexpr double pi=3.14159265358979;

// ---- models -------------------------------------------------------------
bool mesh_header(uint32_t mesh,unsigned count,uint32_t& vp,uint32_t& list) {
    if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) return false;
    if(psx_mod_read_byte(mesh+6)!=count) return false;
    vp=psx_mod_read_word(mesh+0x10);list=psx_mod_read_word(mesh+0x14);
    return vp>=0x80010000u && vp<=0x801fff00u && list>=0x80010000u && list<=0x801fff00u && !(vp&3) && !(list&3);
}
// The woodburner: first corner (-270, 1435, -230), pipe top (109, -1535, -18).
bool stove_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,28,vp,list) && psx_mod_read_word(vp)==0x059bfef2u && psx_mod_read_half(vp+4)==0xff1au &&
           psx_mod_read_word(vp+8*24)==0xfa01006du && psx_mod_read_half(vp+8*24+4)==0xffeeu;
}
// The chandelier: twelve FT4 arm planes, first corner (-896, 235, 0), the
// chain's foot (0, -235, -25).
bool chandelier_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,28,vp,list) && psx_mod_read_word(list)==0x000c002cu && psx_mod_read_word(vp)==0x00ebfc80u &&
           psx_mod_read_half(vp+4)==0 && psx_mod_read_word(vp+8*26)==0xff150000u && psx_mod_read_half(vp+8*26+4)==0xffe7u;
}
// D57G: the iron-ring chandelier (prop type 372; RESISTANCE IS FEUDAL):
// 51 vertices, a pentagonal ring (y 570..950) on five chains to the apex,
// a candle flame at each corner (part of the model). List header
// 0x0014002c, first vertex (-4, 948, 828), the apex (0, -955, -67).
bool ring_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,51,vp,list) && psx_mod_read_word(list)==0x0014002cu && psx_mod_read_word(vp)==0x03b4fffcu &&
           psx_mod_read_half(vp+4)==0x033cu && psx_mod_read_word(vp+8*50)==0xfc450000u && psx_mod_read_half(vp+8*50+4)==0xffbdu;
}
// D57F: the time machine's hanging top (prop type 192; levels 0 and 5): 40
// vertices, an octagonal body (y -763..-87) on four prongs down to y 666.
// List header 0x0018002c, first vertex (-590, -763, -244), the last
// (162, 666, -518).
// D41X: the fire barrel (prop type 220; levels 0 and 5): 20 vertices, the
// drum (y 374 at the floor up to its open rim at -122, 320 x 276) with a
// fire cross standing in it up to y -374. The original paints its glow into
// the room's vertex colours; it gives no light of its own. First vertex
// (0, -122, 0), the last (80, 374, -138).
bool barrel_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,20,vp,list) && psx_mod_read_word(vp)==0xff860000u && psx_mod_read_half(vp+4)==0 &&
           psx_mod_read_word(vp+8*19)==0x01760050u && psx_mod_read_half(vp+8*19+4)==0xff76u;
}
bool portal_top_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,40,vp,list) && psx_mod_read_word(list)==0x0018002cu && psx_mod_read_word(vp)==0xfd05fdb2u &&
           psx_mod_read_half(vp+4)==0xff0cu && psx_mod_read_word(vp+8*39)==0x029a00a2u && psx_mod_read_half(vp+8*39+4)==0xfdfau;
}
int kind_of(uint32_t mesh) {
    if(portal_top_mesh(mesh)) return prop_light_portal_top;
    if(stove_mesh(mesh)) return prop_light_stove;
    if(chandelier_mesh(mesh)) return prop_light_chandelier;
    if(ring_mesh(mesh)) return prop_light_ring;
    return prop_light_none;
}
bool visuals() {
    return prop_lights_enabled.load(std::memory_order_relaxed) && input_modernized() && !ttk_world_classic();
}
uint32_t now_ms() {return ttk_draw_clock_ms();}   // D23N1: held during a redraw job

// Tuning (developer): DNTTK_PROP_LIGHT_<NAME>="r g b strength radius".
struct Tune {float rgb[3],k,radius;};
Tune fire_light{{1.0f,0.46f,0.15f},1.5f,2800};          // the stove's fire, before its door
Tune candle_light{{1.0f,0.80f,0.52f},0.9f,1600};        // among the candles: ceiling and arms
Tune fill_light{{1.0f,0.74f,0.46f},0.5f,5000};          // the chandelier's broad fill below
// D41X: the fire barrel's open fire, in the flame above the rim (developer
// DNTTK_PROP_LIGHT_BARREL; DNTTK_PROP_BARREL_Y its model height).
Tune barrel_light{{1.0f,0.52f,0.18f},1.3f,3000};
float barrel_at[3]={0,-300,0};
// D57G: the iron ring's five candles (one light at the ring's centre, among
// the flames) and its fill on the hall below.
Tune ring_light{{1.0f,0.80f,0.52f},0.9f,2400};
Tune ring_fill{{1.0f,0.74f,0.46f},0.6f,6000};
// D56: pickups that glow (documentation/442-d56-necro-atomic-lights.md),
// by object type (+0x2c) of a pickup object (kind +0x14 = 4), so every copy
// in every level, dropped or spawned, takes part. The atomic health (581):
// a red and blue atom in cyan orbits; a cool cyan-blue light, a slow throb.
Tune atomic_light{{0.42f,0.82f,1.30f},0.85f,1700};
// D57F: an energy crystal away from its first holder, when the level binds
// no lamp of its type (a custom level): the D41D1 sign scan's colours.
Tune crystal_red{{1.00f,0.09f,0.36f},1.0f,1791},crystal_blue{{0.26f,0.19f,1.00f},1.0f,1758},crystal_green{{0.31f,1.00f,0.25f},1.0f,1710};
struct PickupLight {uint16_t type;const Tune* tune;float lift;};
const PickupLight pickup_lights[]={{581,&atomic_light,260}};
void tune(const char* name,Tune& t) {
    if(const char* e=std::getenv(name)) std::sscanf(e,"%f %f %f %f %f",&t.rgb[0],&t.rgb[1],&t.rgb[2],&t.k,&t.radius);
}

// Candle flames and halos (developer: DNTTK_PROP_FLAMES="w h level halo
// halo_level candle_at candle_y").
float flame_w=40,flame_h=84,flame_rgb=175,halo_size=240,halo_rgb=80;
// D57G: the iron ring's flames are the model's own (a small tongue on each
// candle block); a soft halo round each (developer: DNTTK_PROP_RING_HALO=
// "size level y") reads as candlelight from the hall below.
float ring_halo_size=420,ring_halo_rgb=70,ring_halo_y=500;
// D57F: the machine's top. The game lights it nearly white over a dark room,
// so the whole metal read as a lamp; its colours are scaled to
// portal_top_tone, and the light comes from its centre between the claws,
// over the pad: a live light there (portal_light) and a soft glow at that
// point (developer: DNTTK_PROP_PORTAL_TOP="tone size level y").
float portal_top_tone=0.6f,portal_top_halo=520,portal_top_level=95,portal_top_y=260;
// Its light, in the pad lamp's colour (the D41D1 sign lamp of type 179),
// from the centre between the claws onto them and down to the pad
// (developer: DNTTK_PROP_LIGHT_PORTAL).
Tune portal_light{{0.35f,0.63f,1.00f},0.9f,1900};
float portal_breath(uint32_t ms) {return 0.925f+0.075f*std::sin((float)(ms/1000.0*2*pi/3.0));}
// Model points (y down; the stove's door faces model +z).
// D41Q: each arm plane maps texture u 96 at its outer corner to u 127 at the
// column; the candle is texel column u 100 (centre 100.5), its tip the top
// row (v 64, y -212). So a candle stands 26.5/31 of the arm's length out:
// radius 766 on the 896 arm (D41L had 795, one texel outward).
float candle_at_u=(127-100.5f)/31,candle_y=-212;
constexpr float fire_at[3]={0,1180,560};                 // before the grate, low
constexpr float candle_at[3]={0,-120,0},fill_at[3]={0,700,0};
// D57G: the ring's flames stand at model y 334..562 on its top (y 566).
float ring_at[3]={0,450,0},ring_fill_at[3]={0,1800,0};
constexpr float grate_x=150,grate_y0=1160,grate_y1=1305,grate_z=233;
// The chandelier's outer arm corners (bottom row): the candle directions.
constexpr int arm_corner[12]={0,4,6,8,10,12,14,16,18,20,22,24};

// ---- props and their lights (game thread) --------------------------------
// D54: ref is the prop; when it breaks (prop_state.h) the light sputters out
// from broke_ms (standing 0) and the scans no longer find it.
struct Emitter {double p[3];const Tune* tune;bool fire;uint32_t seed;uint32_t room;float vis=1;PropRef ref;int standing=1;uint32_t broke_ms=0;bool pickup=false;
               bool crystal=false;Tune own{};bool portal=false;};   // D57F: own = the crystal's light (its level lamp's look)
uint64_t scan_sig=0;   // the level the emitters were found on (prop_state.h)
std::vector<Emitter> emitters;
uint32_t scan_level=~0u,scan_frame=0,frames=0,vis_ms=0,scan_loads=~0u;
size_t stoves=0,chandeliers=0,rings=0,barrels=0;
// D56: glowing pickups, rescanned every 10 frames (dropped and spawned ones
// appear within a sixth of a second); one taken goes dark at once.
std::vector<Emitter> pickups;
uint32_t pickup_frame=0;
// D57F: crystals away from their first holder, rescanned with the pickups.
std::vector<Emitter> crystals;
// D56: a pickup's slow throb, about 0.8..1.0 (1.4 s a breath).
float pickup_level(uint32_t ms,uint32_t seed) {
    return 0.90f+0.10f*std::sin((float)(ms/1000.0*2*pi/1.4+(seed&255)*0.1));
}
void place(double out[3],const double prop[3],double yaw,const float m[3]) {
    // Model to world: yaw +0x1c (4096 a turn); model -z goes to (-sin, -cos).
    const double ca=std::cos(yaw),sa=std::sin(yaw);
    out[0]=prop[0]+m[0]*ca+m[2]*sa;out[1]=prop[1]+m[1];out[2]=prop[2]-m[0]*sa+m[2]*ca;
}
void scan_props() {
    emitters.clear();stoves=chandeliers=rings=barrels=0;scan_sig=0;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    std::vector<uint32_t> seen;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3)) continue;
            if(std::find(seen.begin(),seen.end(),o)!=seen.end()) continue;
            seen.push_back(o);
            if(psx_mod_read_byte(o+0x14)!=1 || prop_broken_flag(o)) continue;   // D54: broken props give no light
            const uint32_t mesh=psx_mod_read_word(o+0x44);
            // D41X: the fire barrel has no drawn extras (its fire is the
            // model's own), so it is found here only, not by kind_of on
            // every draw.
            const bool barrel=barrel_mesh(mesh);
            const int kind=barrel?prop_light_none:kind_of(mesh);
            if(!kind && !barrel) continue;
            double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            const double yaw=(int16_t)psx_mod_read_half(o+0x1c)*(2*pi/4096);
            Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);
            if(barrel) {
                place(e.p,p,yaw,barrel_at);e.tune=&barrel_light;e.fire=true;emitters.push_back(e);++barrels;
            } else if(kind==prop_light_portal_top) {
                // D57F: the light is the centre between the claws, not the metal.
                const float at[3]={0,portal_top_y,0};
                place(e.p,p,yaw,at);e.tune=&portal_light;e.portal=true;emitters.push_back(e);
            } else if(kind==prop_light_stove) {
                place(e.p,p,yaw,fire_at);e.tune=&fire_light;e.fire=true;emitters.push_back(e);++stoves;
            } else if(kind==prop_light_ring) {
                place(e.p,p,yaw,ring_at);e.tune=&ring_light;emitters.push_back(e);
                place(e.p,p,yaw,ring_fill_at);e.tune=&ring_fill;e.seed=o+1;emitters.push_back(e);++rings;
            } else {
                place(e.p,p,yaw,candle_at);e.tune=&candle_light;emitters.push_back(e);
                place(e.p,p,yaw,fill_at);e.tune=&fill_light;e.seed=o+1;emitters.push_back(e);++chandeliers;
            }
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE"))
        std::fprintf(stderr,"[TTK prop light] level %u: %zu stoves, %zu chandeliers, %zu rings, %zu fire barrels, %zu lights\n",scan_level,stoves,chandeliers,rings,barrels,emitters.size());
}
void scan_pickups() {
    std::vector<Emitter> found;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3)) {
        for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
            uint32_t node=psx_mod_read_word(table+88*r+head_at);
            for(unsigned guard=0;node && guard<1024;++guard) {
                if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
                const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
                if(o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x14)!=4) continue;
                const uint16_t type=(uint16_t)psx_mod_read_half(o+0x2c);
                for(const PickupLight& k:pickup_lights) if(k.type==type) {
                    bool dup=false;for(const Emitter& e:found) dup|=e.ref.o==o;
                    if(dup) break;
                    Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);e.tune=k.tune;e.pickup=true;
                    for(int c=0;c<3;++c) e.p[c]=e.ref.p[c];
                    e.p[1]-=k.lift;
                    // Keep the room-gate fade of one already lit.
                    for(const Emitter& old:pickups) if(old.ref.o==o && old.ref.type==type) e.vis=old.vis;
                    found.push_back(e);break;
                }
            }
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") && found.size()!=pickups.size())
        std::fprintf(stderr,"[TTK prop light] level %u: %zu glowing pickups\n",scan_level,found.size());
    pickups.swap(found);
}
// D57F: every crystal seated in a crystal holder (linked in a room; one in
// Duke's hand is in no room) where no level lamp is bound to it.
void scan_crystals() {
    std::vector<Emitter> found;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    // The crystal holders (level object table 0x800d2550, kind +0x19 = 17,
    // the generic holder 0x80092e98): a seated crystal sits 194 units above
    // one. Devices (the conveyor slot) show their power their own way.
    std::vector<std::array<int32_t,3>> holders;
    for(uint32_t i=0;i<64;++i) {
        const uint32_t o=psx_mod_read_word(0x800d2550u+4*i);
        if(o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x19)!=17) continue;
        holders.push_back({(int32_t)psx_mod_read_word(o+4),(int32_t)psx_mod_read_word(o+8),(int32_t)psx_mod_read_word(o+12)});
    }
    auto seated=[&](const int32_t p[3]) {
        for(const auto& h:holders) if(std::abs(p[0]-h[0])<=64 && std::abs(p[2]-h[2])<=64 && h[1]-p[1]>=120 && h[1]-p[1]<=400) return true;
        return false;
    };
    // The level's lamps not loaded yet (a level change): none until they are.
    static const int32_t nowhere[3]={0,0,0};
    const bool ok=level_lamp_bound && level_lamp_bound(0,nowhere,nullptr)>=0 && rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3);
    for(uint32_t r=0;ok && r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3) || prop_broken_flag(o)) continue;
            const uint16_t type=(uint16_t)psx_mod_read_half(o+0x2c);
            if(type<176 || type>178) continue;
            bool dup=false;for(const Emitter& e:found) dup|=e.ref.o==o;
            if(dup) continue;
            Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);e.crystal=true;
            BoundLamp like{};
            const int bound=level_lamp_bound(type,e.ref.p,&like);
            if(bound==2 || !seated(e.ref.p)) continue;   // its own lamp lights it here; or not in a holder
            const Tune& fallback=type==176?crystal_red:type==177?crystal_blue:crystal_green;
            float off[3]={0,20,0};
            if(bound==1) {e.own=Tune{{like.rgb[0],like.rgb[1],like.rgb[2]},like.k,like.radius};for(int c=0;c<3;++c) off[c]=like.off[c];}
            else e.own=fallback;
            e.tune=&e.own;
            for(int c=0;c<3;++c) e.p[c]=e.ref.p[c]+off[c];
            for(const Emitter& old:crystals) if(old.ref.o==o && old.ref.type==type) e.vis=old.vis;
            found.push_back(e);
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") && found.size()!=crystals.size())
        std::fprintf(stderr,"[TTK prop light] level %u: %zu crystals away from their first holder lit\n",scan_level,found.size());
    crystals.swap(found);
    for(Emitter& e:crystals) e.tune=&e.own;   // after the move
}
// A pickup still lies there: linked in a room, still a pickup of the same
// type. Its light follows it (a dropped one falls to the floor).
bool pickup_present(const PropSnapshot& snap,Emitter& e) {
    if(!snap.linked.count(e.ref.o) || psx_mod_read_byte(e.ref.o+0x14)!=4 || psx_mod_read_half(e.ref.o+0x2c)!=e.ref.type) return false;
    const double lift=e.ref.p[1]-e.p[1];
    for(int c=0;c<3;++c) e.p[c]=(int32_t)psx_mod_read_word(e.ref.o+4+4*c);
    e.p[1]-=lift;for(int c=0;c<3;++c) e.ref.p[c]=(int32_t)e.p[c];e.ref.p[1]=(int32_t)(e.p[1]+lift);
    return true;
}
void composition_hook(CPUState*,uint32_t) {
    if(psx_mod_replay_active()) return;
    if(!input_modernized()) {emitters.clear();pickups.clear();crystals.clear();scan_level=~0u;return;}   // Vanilla: nothing
    ++frames;
    const uint32_t level=psx_mod_read_word(0x800be570u);
    // D54: a savestate load rescans too (a prop broken since, or standing again).
    const uint32_t loads=psx_mod_savestate_loads();
    const bool rescan=level!=scan_level || loads!=scan_loads;
    if(rescan || frames-scan_frame>300) {scan_level=level;scan_loads=loads;scan_frame=frames;scan_props();}
    if(rescan || frames-pickup_frame>=10) {pickup_frame=frames;if(rescan) {pickups.clear();crystals.clear();}scan_pickups();scan_crystals();}
}
unsigned publish(PSXModLight* out,unsigned room,bool (*shown)(uint32_t)) {
    if(!room || (emitters.empty() && pickups.empty() && crystals.empty()) || !prop_lights_enabled.load(std::memory_order_relaxed) || !input_modernized()) return 0;
    const uint32_t pl=players+psx_mod_read_word(player_index)%2*0x8a4u;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(pl+4),(double)(int32_t)psx_mod_read_word(pl+8),(double)(int32_t)psx_mod_read_word(pl+12)};
    const uint32_t ms=now_ms();
    // D54: a broken prop's light sputters out.
    const PropSnapshot& snap=prop_snapshot_take();
    if(prop_snapshot_usable(snap,scan_sig)) for(Emitter& e:emitters) if(e.standing && !prop_standing(snap,e.ref)) {
        e.standing=0;e.broke_ms=ms;
        std::fprintf(stderr,"[TTK prop light] %s light goes out: its prop %08x broke\n",e.fire?"fire":e.portal?"portal":"candle",e.ref.o);
    }
    // Room gate (level_lamp_room_lit): a light comes on at once, fades out over 0.25 s.
    const float step=std::min(1.0f,(ms-vis_ms)/250.0f);vis_ms=ms;
    struct Near {double d;Emitter* e;};
    std::vector<Near> near;
    // D56: a pickup taken (unlinked) gives no more light; between scans the
    // snapshot (all linked objects) tells.
    const bool snap_ok=snap.valid || !snap.linked.empty();
    auto consider=[&](Emitter& e) {
        if(e.pickup && snap_ok && !pickup_present(snap,e)) return;
        if(e.crystal && snap_ok && !prop_standing(snap,e.ref)) return;   // D57F: taken (or moved) since the scan
        const float target=!shown || shown(e.room)?1.0f:0.0f;
        e.vis=e.vis<target?target:std::max(target,e.vis-step);   // D41E: on at once, out over 0.25 s
        const double dx=e.p[0]-duke[0],dy=e.p[1]-duke[1],dz=e.p[2]-duke[2];
        const double d=std::sqrt(dx*dx+dy*dy+dz*dz)-e.tune->radius;
        // D41E: no range cut beyond the far limit; a light fades only when
        // nearer ones push it out of the budget (as the level lamps).
        if(d<32000 && e.vis>0.01f && (e.standing || ms-e.broke_ms<prop_dying_ticks*34u)) near.push_back({d,&e});
    };
    for(Emitter& e:emitters) consider(e);
    if(prop_pickups_enabled.load(std::memory_order_relaxed)) for(Emitter& e:pickups) consider(e);
    if(prop_crystals_enabled.load(std::memory_order_relaxed)) for(Emitter& e:crystals) consider(e);
    std::sort(near.begin(),near.end(),[](const Near& a,const Near& b){return a.d<b.d;});
    unsigned n=0;
    const unsigned cap=std::min(room,6u);const double cut=near.size()>cap ? near[cap].d : 32000.0;
    for(const Near& q:near) {
        if(n>=cap) break;
        const Emitter& e=*q.e;
        float k=e.tune->k*e.vis*(float)std::clamp((cut-q.d)/4000,0.0,1.0);
        k*=e.portal?portal_breath(ms):e.crystal?1.0f:e.pickup?pickup_level(ms,e.seed):e.fire?prop_fire_level(ms,e.seed):prop_candle_level(ms,e.seed);   // D57F: steady, as its sign lamp
        if(!e.standing) k*=prop_dying_level((ms-e.broke_ms)/34u,e.seed);
        PSXModLight& l=out[n++];
        l.x=(float)e.p[0];l.y=(float)e.p[1];l.z=(float)e.p[2];l.radius=e.tune->radius;
        l.r=e.tune->rgb[0]*k;l.g=e.tune->rgb[1]*k;l.b=e.tune->rgb[2]*k;l.flags=0;
    }
    return n;
}

// ---- the drawn extras (any thread: RAM reads and the clock only) ----------
uint8_t level8(float v) {return (uint8_t)std::clamp((int)std::lround(v),0,255);}
PropGlow billboard(float x,float y,float z,float w,float h,float anchor_y,float sway,int u,int v,int tw,int th,float r,float g,float b) {
    PropGlow q{};q.billboard=true;q.p[0][0]=x;q.p[0][1]=y;q.p[0][2]=z;q.w=w;q.h=h;q.anchor_y=anchor_y;q.sway=sway;
    q.u=u;q.v=v;q.tw=tw;q.th=th;q.rgb[0]=level8(r);q.rgb[1]=level8(g);q.rgb[2]=level8(b);
    return q;
}
unsigned glows(uint32_t mesh,int kind,PropGlow* out,unsigned max) {
    if(!visuals()) return 0;
    const uint32_t ms=now_ms();unsigned n=0;
    if(kind==prop_light_stove && max>=2) {
        const float f=prop_fire_level(ms,mesh);
        // Embers behind the grate: the quad just in front of the door face.
        PropGlow& e=out[n++];e={};
        const float c[4][3]={{-grate_x,grate_y0,grate_z},{grate_x,grate_y0,grate_z},{-grate_x,grate_y1,grate_z},{grate_x,grate_y1,grate_z}};
        std::memcpy(e.p,c,sizeof c);e.u=prop_ember_u;e.v=0;e.tw=prop_ember_w;e.th=prop_ember_h;
        e.rgb[0]=e.rgb[1]=e.rgb[2]=level8(150*f);
        // A faint bloom before it.
        out[n++]=billboard(0,(grate_y0+grate_y1)/2,grate_z+90,560,420,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,70*f,70*f,70*f);
    } else if(kind==prop_light_chandelier) {
        uint32_t vp,list;
        if(!mesh_header(mesh,28,vp,list)) return 0;
        for(int i=0;i<12 && n+2<=max;++i) {
            const uint32_t xy=psx_mod_read_word(vp+8*arm_corner[i]);
            const float vx=(float)(int16_t)xy,vz=(float)(int16_t)psx_mod_read_half(vp+8*arm_corner[i]+4);
            const float len=std::sqrt(vx*vx+vz*vz);if(len<1) continue;
            const float x=vx*candle_at_u,z=vz*candle_at_u;
            const uint32_t seed=mesh*13u+(uint32_t)i*101u;
            const float t=ms/1000.0f;
            // The flame stretches and leans a little, each on its own.
            const float stretch=0.88f+0.24f*prop_light_noise(t*6.0+i*3.1,seed);
            const float sway=10.0f*(prop_light_noise(t*3.0+i*1.7,seed+5)-0.5f);
            const float k=prop_candle_level(ms,seed);
            out[n]=billboard(x,candle_y+6,z,flame_w,flame_h*stretch,1.0f,sway,prop_flame_u,0,prop_flame_w,prop_flame_h,flame_rgb*k,flame_rgb*k,flame_rgb*k);
            out[n++].upright=true;   // D41Q: stands on its candle seen from below or above too
            out[n++]=billboard(x,candle_y-30,z,halo_size,halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,halo_rgb*k,halo_rgb*k,halo_rgb*k);
        }
    } else if(kind==prop_light_portal_top) {
        // A slow breath, about 0.85..1.0 (3 s), in the pad lamp's colour.
        const float l=portal_top_level*portal_breath(ms);   // in step with its light
        if(n<max) out[n++]=billboard(0,portal_top_y,0,portal_top_halo,portal_top_halo,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,l*0.55f,l*0.80f,l);
    } else if(kind==prop_light_ring) {
        uint32_t vp,list;
        if(!mesh_header(mesh,51,vp,list)) return 0;
        // Each candle is a block of vertices 14+8c..17+8c; the halo sits
        // over its middle, at the flame.
        for(int c=0;c<5 && n<max;++c) {
            float x=0,z=0;
            for(int v=14+8*c;v<18+8*c;++v) {x+=(float)(int16_t)psx_mod_read_half(vp+8*v);z+=(float)(int16_t)psx_mod_read_half(vp+8*v+4);}
            const float k=prop_candle_level(ms,mesh*13u+(uint32_t)c*101u);
            out[n++]=billboard(x/4,ring_halo_y,z/4,ring_halo_size,ring_halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,ring_halo_rgb*k,ring_halo_rgb*k,ring_halo_rgb*k);
        }
    }
    return n;
}

float tone(int kind) {return kind==prop_light_portal_top && visuals() ? portal_top_tone : 1.0f;}

// ---- textures -------------------------------------------------------------
uint16_t texel(float r,float g,float b) {
    const int R=std::clamp((int)std::lround(r*31),0,31),G=std::clamp((int)std::lround(g*31),0,31),B=std::clamp((int)std::lround(b*31),0,31);
    return (R|G|B)?(uint16_t)(0x8000|R|G<<5|B<<10):0;   // semi-transparent (STP); black is clear
}
float smooth(float a,float b,float x) {const float t=std::clamp((x-a)/(b-a),0.0f,1.0f);return t*t*(3-2*t);}
void paint(uint16_t* bank) {
    constexpr int base=256;   // page x 256 (prop_light_tpage), rows 0..31
    // Flame: a teardrop, white-gold core low in it, orange edges, a
    // transparent blue foot.
    for(int y=0;y<prop_flame_h;++y) for(int x=0;x<prop_flame_w;++x) {
        const float ny=(y+0.5f)/prop_flame_h,nx=(x+0.5f)/prop_flame_w*2-1;
        const float half=0.95f*std::pow(std::max(0.0f,(float)std::sin(pi*std::pow(ny,0.75f))),0.9f)*std::sqrt(ny);
        const float d=half>0.02f?std::fabs(nx)/half:2;
        const float body=std::pow(std::max(0.0f,1-d),0.8f)*smooth(1.0f,0.82f,ny);
        const float core=std::pow(std::max(0.0f,1-d*1.6f),1.2f)*smooth(0.25f,0.7f,ny)*smooth(1.0f,0.85f,ny);
        const float foot=std::max(0.0f,1-d)*smooth(0.8f,0.97f,ny)*0.35f;
        bank[(size_t)y*1024+base+prop_flame_u+x]=texel(body*0.95f+core*0.05f,body*0.45f+core*0.5f+foot*0.15f,body*0.08f+core*0.35f+foot*0.5f);
    }
    // Halo: a soft round warm glow.
    for(int y=0;y<prop_halo_h;++y) for(int x=0;x<prop_halo_w;++x) {
        const float dx=(x+0.5f)/prop_halo_w*2-1,dy=(y+0.5f)/prop_halo_h*2-1,r=std::sqrt(dx*dx+dy*dy);
        const float g=std::max(0.0f,std::exp(-r*r*4.0f)-std::exp(-4.0f))/(1-std::exp(-4.0f));
        bank[(size_t)y*1024+base+prop_halo_u+x]=texel(g*1.0f,g*0.62f,g*0.30f);
    }
    // D41S: the neutral halo (a falloff steeper at the rim than the warm one,
    // so a small glow reads as a point of light) and the shadow patch
    // (opaque black with STP: blended in mode 0 it halves the background).
    for(int y=0;y<prop_white_halo_h;++y) for(int x=0;x<prop_white_halo_w;++x) {
        const float dx=(x+0.5f)/prop_white_halo_w*2-1,dy=(y+0.5f)/prop_white_halo_h*2-1,r=std::sqrt(dx*dx+dy*dy);
        const float g=std::max(0.0f,std::exp(-r*r*5.0f)-std::exp(-5.0f))/(1-std::exp(-5.0f));
        bank[(size_t)y*1024+base+prop_white_halo_u+x]=texel(g,g,g);
    }
    for(int y=0;y<prop_shade_h;++y) for(int x=0;x<prop_shade_w;++x) bank[(size_t)y*1024+base+prop_shade_u+x]=0x8000;
    // Embers: red-orange, hottest along the bottom, soft at the sides,
    // brighter in uneven clumps.
    for(int y=0;y<prop_ember_h;++y) for(int x=0;x<prop_ember_w;++x) {
        const float ny=(y+0.5f)/prop_ember_h,nx=(x+0.5f)/prop_ember_w*2-1;
        const float clump=0.75f+0.25f*std::sin(x*1.9f+std::sin(x*0.7f)*2.0f);
        const float g=smooth(1.0f,0.7f,std::fabs(nx))*(0.25f+0.75f*smooth(0.0f,0.85f,ny))*clump*smooth(1.0f,0.9f,ny);
        bank[(size_t)y*1024+base+prop_ember_u+x]=texel(g*1.0f,g*0.42f*(0.6f+0.4f*ny),g*0.10f);
    }
}
}  // namespace
}  // namespace ttk

PSX_MOD_CONSTRUCTOR(register_ttk_prop_lights) {
    ttk::tune("DNTTK_PROP_LIGHT_FIRE",ttk::fire_light);
    ttk::tune("DNTTK_PROP_LIGHT_CANDLES",ttk::candle_light);
    ttk::tune("DNTTK_PROP_LIGHT_FILL",ttk::fill_light);
    ttk::tune("DNTTK_PROP_LIGHT_BARREL",ttk::barrel_light);
    if(const char* e=std::getenv("DNTTK_PROP_BARREL_Y")) ttk::barrel_at[1]=(float)std::atof(e);
    ttk::prop_light_lit_mesh=ttk::barrel_mesh;
    ttk::tune("DNTTK_PROP_LIGHT_ATOMIC",ttk::atomic_light);
    ttk::tune("DNTTK_PROP_LIGHT_RING",ttk::ring_light);
    ttk::tune("DNTTK_PROP_LIGHT_RING_FILL",ttk::ring_fill);
    ttk::tune("DNTTK_PROP_LIGHT_PORTAL",ttk::portal_light);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_RED",ttk::crystal_red);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_BLUE",ttk::crystal_blue);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_GREEN",ttk::crystal_green);
    if(const char* e=std::getenv("DNTTK_PROP_RING_HALO")) std::sscanf(e,"%f %f %f",&ttk::ring_halo_size,&ttk::ring_halo_rgb,&ttk::ring_halo_y);
    if(const char* e=std::getenv("DNTTK_PROP_RING_AT")) std::sscanf(e,"%f %f",&ttk::ring_at[1],&ttk::ring_fill_at[1]);
    if(const char* e=std::getenv("DNTTK_PROP_FLAMES")) std::sscanf(e,"%f %f %f %f %f %f %f",&ttk::flame_w,&ttk::flame_h,&ttk::flame_rgb,&ttk::halo_size,&ttk::halo_rgb,&ttk::candle_at_u,&ttk::candle_y);
    psx_mod_register_function_entry_plugin("ttk.prop.lights",0x80026164u,ttk::composition_hook);
    ttk::prop_light_kind=ttk::kind_of;
    ttk::prop_light_visuals=ttk::visuals;
    ttk::prop_light_publish=ttk::publish;
    ttk::prop_light_glows=ttk::glows;
    ttk::prop_light_tone=ttk::tone;
    if(const char* e=std::getenv("DNTTK_PROP_PORTAL_TOP")) std::sscanf(e,"%f %f %f %f",&ttk::portal_top_tone,&ttk::portal_top_halo,&ttk::portal_top_level,&ttk::portal_top_y);
    ttk::prop_light_paint_bank=ttk::paint;
}
