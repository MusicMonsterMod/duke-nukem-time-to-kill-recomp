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
// Both are found by their model data, so every copy in every level takes
// part. The lights join the D41A list (DYNAMIC LIGHTS on) after the level
// lamps and the D53 screens, nearest to Duke first, gated by the room
// Polymorph shows; the flames and glows are drawn by the near-clip object
// path from the second texture bank (x 256..335, painted here). Classic and
// Vanilla: nothing changes.
#include "prop_lights.h"
#include "live_env.h"
#include "pc_input.h"
#include "cpu_state.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>
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
int kind_of(uint32_t mesh) {
    if(stove_mesh(mesh)) return prop_light_stove;
    if(chandelier_mesh(mesh)) return prop_light_chandelier;
    return prop_light_none;
}
bool visuals() {
    return prop_lights_enabled.load(std::memory_order_relaxed) && input_modernized() && !ttk_world_classic();
}
uint32_t now_ms() {return (uint32_t)SDL_GetTicks();}

// Tuning (developer): DNTTK_PROP_LIGHT_<NAME>="r g b strength radius".
struct Tune {float rgb[3],k,radius;};
Tune fire_light{{1.0f,0.46f,0.15f},1.5f,2800};          // the stove's fire, before its door
Tune candle_light{{1.0f,0.80f,0.52f},0.9f,1600};        // among the candles: ceiling and arms
Tune fill_light{{1.0f,0.74f,0.46f},0.5f,5000};          // the chandelier's broad fill below
void tune(const char* name,Tune& t) {
    if(const char* e=std::getenv(name)) std::sscanf(e,"%f %f %f %f %f",&t.rgb[0],&t.rgb[1],&t.rgb[2],&t.k,&t.radius);
}

// Candle flames and halos (developer: DNTTK_PROP_FLAMES="w h level halo halo_level").
float flame_w=40,flame_h=84,flame_rgb=175,halo_size=240,halo_rgb=80;
// Model points (y down; the stove's door faces model +z).
constexpr float candle_r=795,candle_y=-212;               // each arm's candle top
constexpr float fire_at[3]={0,1180,560};                 // before the grate, low
constexpr float candle_at[3]={0,-120,0},fill_at[3]={0,700,0};
constexpr float grate_x=150,grate_y0=1160,grate_y1=1305,grate_z=233;
// The chandelier's outer arm corners (bottom row): the candle directions.
constexpr int arm_corner[12]={0,4,6,8,10,12,14,16,18,20,22,24};

// ---- props and their lights (game thread) --------------------------------
struct Emitter {double p[3];const Tune* tune;bool fire;uint32_t seed;uint32_t room;float vis=1;};
std::vector<Emitter> emitters;
uint32_t scan_level=~0u,scan_frame=0,frames=0,vis_ms=0;
size_t stoves=0,chandeliers=0;
void place(double out[3],const double prop[3],double yaw,const float m[3]) {
    // Model to world: yaw +0x1c (4096 a turn); model -z goes to (-sin, -cos).
    const double ca=std::cos(yaw),sa=std::sin(yaw);
    out[0]=prop[0]+m[0]*ca+m[2]*sa;out[1]=prop[1]+m[1];out[2]=prop[2]-m[0]*sa+m[2]*ca;
}
void scan_props() {
    emitters.clear();stoves=chandeliers=0;
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
            if(psx_mod_read_byte(o+0x14)!=1) continue;
            const int kind=kind_of(psx_mod_read_word(o+0x44));
            if(!kind) continue;
            double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            const double yaw=(int16_t)psx_mod_read_half(o+0x1c)*(2*pi/4096);
            Emitter e{};e.room=r;e.seed=o;
            if(kind==prop_light_stove) {
                place(e.p,p,yaw,fire_at);e.tune=&fire_light;e.fire=true;emitters.push_back(e);++stoves;
            } else {
                place(e.p,p,yaw,candle_at);e.tune=&candle_light;emitters.push_back(e);
                place(e.p,p,yaw,fill_at);e.tune=&fill_light;e.seed=o+1;emitters.push_back(e);++chandeliers;
            }
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE"))
        std::fprintf(stderr,"[TTK prop light] level %u: %zu stoves, %zu chandeliers, %zu lights\n",scan_level,stoves,chandeliers,emitters.size());
}
void composition_hook(CPUState*,uint32_t) {
    if(psx_mod_replay_active()) return;
    if(!input_modernized()) {emitters.clear();scan_level=~0u;return;}   // Vanilla: nothing
    ++frames;
    const uint32_t level=psx_mod_read_word(0x800be570u);
    if(level!=scan_level || frames-scan_frame>300) {scan_level=level;scan_frame=frames;scan_props();}
}
unsigned publish(PSXModLight* out,unsigned room,bool (*shown)(uint32_t)) {
    if(!room || emitters.empty() || !prop_lights_enabled.load(std::memory_order_relaxed) || !input_modernized()) return 0;
    const uint32_t pl=players+psx_mod_read_word(player_index)%2*0x8a4u;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(pl+4),(double)(int32_t)psx_mod_read_word(pl+8),(double)(int32_t)psx_mod_read_word(pl+12)};
    const uint32_t ms=now_ms();
    // Room gate: each light fades toward whether its room is shown (0.25 s).
    const float step=std::min(1.0f,(ms-vis_ms)/250.0f);vis_ms=ms;
    struct Near {double d;Emitter* e;};
    std::vector<Near> near;
    for(Emitter& e:emitters) {
        const float target=!shown || shown(e.room)?1.0f:0.0f;
        e.vis=e.vis<target?std::min(target,e.vis+step):std::max(target,e.vis-step);
        const double dx=e.p[0]-duke[0],dy=e.p[1]-duke[1],dz=e.p[2]-duke[2];
        const double d=std::sqrt(dx*dx+dy*dy+dz*dz)-e.tune->radius;
        if(d<6000 && e.vis>0.01f) near.push_back({d,&e});
    }
    std::sort(near.begin(),near.end(),[](const Near& a,const Near& b){return a.d<b.d;});
    unsigned n=0;
    for(const Near& q:near) {
        if(n>=room || n>=6) break;
        const Emitter& e=*q.e;
        float k=e.tune->k*e.vis*(float)std::clamp((6000-q.d)/2000,0.0,1.0);
        k*=e.fire?prop_fire_level(ms,e.seed):prop_candle_level(ms,e.seed);
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
            const float x=vx/len*candle_r,z=vz/len*candle_r;
            const uint32_t seed=mesh*13u+(uint32_t)i*101u;
            const float t=ms/1000.0f;
            // The flame stretches and leans a little, each on its own.
            const float stretch=0.88f+0.24f*prop_light_noise(t*6.0+i*3.1,seed);
            const float sway=10.0f*(prop_light_noise(t*3.0+i*1.7,seed+5)-0.5f);
            const float k=prop_candle_level(ms,seed);
            out[n++]=billboard(x,candle_y+6,z,flame_w,flame_h*stretch,1.0f,sway,prop_flame_u,0,prop_flame_w,prop_flame_h,flame_rgb*k,flame_rgb*k,flame_rgb*k);
            out[n++]=billboard(x,candle_y-30,z,halo_size,halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,halo_rgb*k,halo_rgb*k,halo_rgb*k);
        }
    }
    return n;
}

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
    if(const char* e=std::getenv("DNTTK_PROP_FLAMES")) std::sscanf(e,"%f %f %f %f %f",&ttk::flame_w,&ttk::flame_h,&ttk::flame_rgb,&ttk::halo_size,&ttk::halo_rgb);
    psx_mod_register_function_entry_plugin("ttk.prop.lights",0x80026164u,ttk::composition_hook);
    ttk::prop_light_kind=ttk::kind_of;
    ttk::prop_light_visuals=ttk::visuals;
    ttk::prop_light_publish=ttk::publish;
    ttk::prop_light_glows=ttk::glows;
    ttk::prop_light_paint_bank=ttk::paint;
}
