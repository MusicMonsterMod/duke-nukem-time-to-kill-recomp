#include <chrono>
// D11B: near clipping for the Modernized eye view.
//
// Two hand-written mesh renderers draw the level:
// - 0x80011020, the world renderer (rooms, walls, floors; callers ra
//   0x80037344/0x80037c54/0x80062db8). It projects vertices with RTPT, drops a
//   polygon when any vertex has SZ 0 (at or behind the eye) and projects
//   vertices nearer than H/2 with the GTE divide saturated. Its screen-space
//   subdivision (0x80012960) reuses those points.
// - 0x80010000, the object renderer (props and actor joints). It drops a
//   polygon only when every vertex is nearer than H, so a polygon with some
//   vertices nearer than H/2 is drawn with saturated coordinates.
// In first person that tears or removes walls, floors and props beside the eye.
//
// While the eye view is live, polygons with an unsafe vertex (divide
// saturated, IR clamped, or projected outside +-1000) are drawn here instead:
// clipped against the view frustum in view space, subdivided, and emitted with
// the original colors, fog, light table, texture animation, command bits,
// packet arena and ordering-table rule of their renderer. The original routine
// then runs on a copy of the mesh without those polygons, so every other
// polygon still takes the original path. Third person and Vanilla never reach
// this code, and Duke's own model is left to the original (D12).
//
// World mesh (0x80011020): +0x14 vertex count (half), +0x18 offset of the
// polygon list, +0x1c vertices (8 bytes: x/y/z grid bytes and a color index
// byte, then an RGBC word). The vertex pass reads vertices in threes, so up
// to two records past the count are read (and may get their color index
// nibble cleared); the copy keeps those bytes in place. Groups are a word
// (type | count << 16), 0x60 GT3 / 0x61 GT4 8-byte records, 0xff ends.
// Object mesh (0x80010000): +6 vertex count (byte), +0x10 vertex pointer
// (SVECTOR, followed by a color word table), +0x14 polygon list pointer.
// Record sizes: 0x20 F3 8, 0x28 F4 8, 0x30 G3 12, 0x38 G4 12, 0x24 FT3 16,
// 0x2c FT4 16, 0x34 GT3 20, 0x3c GT4 20. The top 7 bits of the flags word are
// double-sided, flat-color source, OT -1, OT +1, semi-transparent, animated.
// Render context 0x800d67a8: +0 packet cursor, +4 arena start, +8 arena end,
// +0x48/+0x4c command/tpage bits, +0x50 flags, +0x54/+0x5c flat colors,
// +0x58 texture table, +0x60 animation table, +0x68 light table, +0x6c fog
// start, +0x78 OT depth limit. Ordering table 0x800d27a0 (2048 head/tail
// pairs) and its bitmap 0x800d26a0.
#include "near_clip.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
namespace ttk {
namespace {
struct Guard { uint32_t address, size; const char* digest; };
const Guard near_guards[]={
    {0x80010000,0xd18,"cf197784e09dc094b6c5552df68f71230c5b90ac416612abf3788a811e038acd"},
    {0x80011020,0xd4c,"f5a4dfa2c0f306b1c3d3250a160717afec9244d79fd5e321f39e767df1587e75"},
    {0x8002ee50,0x140,"51137c3bebd7496a800d365fab7182a782adf13dd118c13ed37862d88bd4246d"},
    {0x80031fa0,0xe0,"1b0716bbbd34f60bef3a25c3dd0d6c5ffc05b0c5aac3790876d97901e89afb47"},
};
constexpr uint32_t context=0x800d67a8, ordering=0x800d27a0, bitmap=0x800d26a0;
constexpr uint32_t color_table=0x800c37e4;
constexpr uint32_t copy_size=0x10000;
// Guard band (pixels from the projection centre) and near plane. A triangle
// inside it stays within the GPU's 1023x511 primitive limit.
constexpr double guard_x=480, guard_y=240, near_z=16;
constexpr double split_pixels=96;
// Pieces sort by their average depth, like the object renderer's AVSZ3.
// Pieces that span a large depth range are cut further so two surfaces a
// short distance apart (a closet door and the wall behind it) cannot
// interleave in the ordering table.
constexpr double depth_ratio=1.25, depth_pixels=16;
int env_int(const char* name,int fallback,int lo,int hi) {
    const char* t=std::getenv(name);char* end=nullptr;const long v=t?std::strtol(t,&end,10):0;
    return t && end && !*end && v>=lo && v<=hi ? (int)v : fallback;
}
// Developer tuning (not saved preferences).
// Host prop pieces sort four OT slots (128 units) nearer than their average
// depth. Props stand flush against world walls (the apartment wardrobe); with
// both surfaces subdivided and sorted by average depth, wall pieces behind
// otherwise won the sort in a comb pattern (-2 and -3 still left teeth where
// a wardrobe door meets the wall). Actors (drawn through 0x800348d8) keep no
// bias, so an enemy just behind a wall edge cannot show through it.
// DNTTK_NEAR_OBJECT_BIAS overrides.
int object_bias() {static const int v=env_int("DNTTK_NEAR_OBJECT_BIAS",-4,-8,8);return v;}
// DNTTK_NEAR_TINT=1 (developer): host world pieces red, object pieces blue.
bool tint() {static const bool v=env_int("DNTTK_NEAR_TINT",0,0,1)==1;return v;}
// D17A: host precise vertices for every emitted piece (DNTTK_NEAR_PRECISE=0 off).
bool precise() {static const bool v=env_int("DNTTK_NEAR_PRECISE",1,0,1)==1;return v;}
uint64_t precise_triangles;
double precise_split_pixels() {static const double v=env_int("DNTTK_NEAR_SPLIT_PX",192,32,1024);return v;}
double precise_depth_ratio() {static const double v=env_int("DNTTK_NEAR_SPLIT_RATIO",150,105,400)/100.0;return v;}
// New corners are floored like the GTE. Rounding them outward closed hairline
// cracks but, at 1080p, drew dotted dark lines at floor-tile edges (pixels
// past a tile's UV range). DNTTK_NEAR_ROUND=outward restores it for testing.
bool round_outward() {static const bool v=[]{const char* t=std::getenv("DNTTK_NEAR_ROUND");return t && std::string_view(t)=="outward";}();return v;}
// Default (D17A, full): every polygon near the eye (world 1536, props 3072)
// is clipped and drawn here, in pieces that each sort by their own depth, with
// host precise vertices: exact positions that follow the original renderer's
// edges and per-vertex depth and texture coordinates, so the GPU maps them
// with correct perspective. That removed the affine warp of tables and walls
// beside the eye, the flicker where a taken wall polygon and the original
// polygons of the same slot traded places between in-between images, and the
// seams that made D11B keep this mode off (integer piece corners at
// T-junctions). DNTTK_NEAR_MODE=conservative (developer) takes over only the
// polygons the original would draw wrongly or drop, one piece each; it is
// also used when precise vertices are off (DNTTK_NEAR_PRECISE=0).
bool precise();
bool conservative() {static const bool v=[]{const char* t=std::getenv("DNTTK_NEAR_MODE");
    if(t && std::string_view(t)=="full") return false;
    if(t && std::string_view(t)=="conservative") return true;
    return !precise();}();return v;}
uint32_t near_depth(uint32_t full) {return conservative()?0:full;}
double depth_span() {static const double v=env_int("DNTTK_NEAR_DEPTH_SPAN",0,0,4096);return v;}
constexpr unsigned max_depth=12;
// Packet budget. The render arena (ctx+4..+8, 139744 bytes) is a ring the game
// fills continuously; the club already uses about half of it per frame. Host
// packets are capped per frame, and none are written once the frame's total
// use would pass 60% of the ring, so the ring can never wrap onto packets of
// the same frame. With 64 KB per frame and 60% of the ring as limits, short of budget, pieces stop splitting, and a mesh that
// cannot be afforded stays entirely on the original path.
constexpr uint32_t host_frame_bytes=64*1024, split_reserve=8*1024, takeover_reserve=6*1024;
constexpr double ring_share=0.6;
// Polygons with a corner this near are also drawn here. The PS1 maps textures
// affinely; where an original polygon meets a subdivided one the shared edge
// would map differently and the texture kinks. Past this depth the difference
// is below a pixel or two on 1024-unit cells.
constexpr uint32_t perspective_z=1536;
// Props are taken over further out: an original prop polygon sorts by its
// whole average depth, and a host wall piece beside it could cover it (the
// apartment wardrobe's inner door, about 2000 units away).
constexpr uint32_t object_perspective_z=3072;

struct Stats { uint64_t seen,taken,polys,culled,triangles,budget_hits; };
Stats world_stats,object_stats;
uint64_t refused,copy_overflows,duke_skips,fades_skipped,fade_tests;
uint32_t empty_rect;
uint32_t copy_base;
int32_t last_zsf3;
uint32_t arena_size;uint64_t packet_skips,mesh_fallbacks;
// Frame accounting: a frame starts when the OT bitmap is still empty.
uint32_t frame_first_cursor,frame_host_bytes,frame_used_peak,host_bytes_peak;
// D12 first-person weapon. Every polygon of the hand, weapon, flash and held
// item meshes is drawn here while first_person_weapon_drawing(). Their packets
// are written as usual but linked only when Duke's draw ends: sorted by depth
// and appended to ordering-table slot 1, drawn after everything but slot 0
// (the HUD and geometry nearer than 32 units), so a wall between the eye and
// the weapon cannot cover or cut it.
// Slot 1, not 0: the HUD (health and ammo boxes) is in slot 0 and must stay
// on top of the weapon.
constexpr uint32_t viewmodel_slot=1;
struct HeldPacket { uint32_t prim,tag; double z; };
std::vector<HeldPacket> held_packets;
uint32_t held_frame;
uint64_t viewmodel_meshes,viewmodel_packets,viewmodel_discards;

// DNTTK_NEAR_CLIP (developer A/B, not a saved preference): 0 off, "world" or
// "object" for one renderer only; anything else, or unset, both.
unsigned renderers() {
    static const unsigned value=[]{
        const char* t=std::getenv("DNTTK_NEAR_CLIP");
        if(!t) return 3u;
        const std::string_view v(t);
        return v=="0"?0u:v=="world"?1u:v=="object"?2u:3u;
    }();
    return value;
}
bool enabled() {return renderers()!=0;}
// DNTTK_NEAR_SORT (developer): "min" sorts pieces by their nearest corner.
bool sort_nearest() {
    static const bool value=[]{const char* t=std::getenv("DNTTK_NEAR_SORT");return t && std::string_view(t)=="min";}();
    return value;
}
bool identity() {
    static std::array<std::vector<uint32_t>,4> expected;
    static IdentityMemo memo;
    const uint64_t frame=input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen))
        memo.set(frame,g_dirty_ram_code_gen,code_identity(near_guards,expected,psx_mod_read_word,g_psx_ram));
    return memo.ok;
}
int32_t clamp(int64_t v,int64_t lo,int64_t hi){return (int32_t)std::min(hi,std::max(lo,v));}
// Hardware divide (psx-spx UNR), as the runtime's gte_divide.
uint8_t div_table[0x101];
int32_t gte_divide(uint16_t h,uint16_t sz) {
    static const bool ready=[]{
        for(uint32_t d=0x8000;d<0x10000;d+=0x80) {
            uint32_t xa=512;
            for(unsigned i=1;i<5;++i) xa=(xa*(1024u*512u-((d>>7)*xa)))>>18;
            div_table[(d>>7)&0xff]=(uint8_t)(((xa+1)>>1)-0x101);
        }
        div_table[0x100]=div_table[0xff];return true;
    }();(void)ready;
    if((uint32_t)sz*2<=h) return 0x1ffff;
    unsigned shift=0;while(shift<16 && !(sz&(0x8000u>>shift)))++shift;
    const uint32_t n=(uint32_t)h<<shift,d=(uint32_t)sz<<shift;
    const uint16_t dd=(uint16_t)(d|0x8000);
    const int32_t x=0x101+div_table[((dd&0x7fff)+0x40)>>7];
    const int32_t t=(((int32_t)dd*-x)+0x80)>>8;
    const int32_t r=((x*(131072+t))+0x80)>>8;
    const uint32_t q=(uint32_t)(((uint64_t)n*(uint32_t)r+32768)>>16);
    return (int32_t)std::min<uint32_t>(q,0x1ffff);
}
NearGte read_gte(const CPUState* cpu) {
    NearGte g{};
    const uint32_t* c=cpu->gte_ctrl;
    g.r[0][0]=(int16_t)c[0];g.r[0][1]=(int16_t)(c[0]>>16);g.r[0][2]=(int16_t)c[1];
    g.r[1][0]=(int16_t)(c[1]>>16);g.r[1][1]=(int16_t)c[2];g.r[1][2]=(int16_t)(c[2]>>16);
    g.r[2][0]=(int16_t)c[3];g.r[2][1]=(int16_t)(c[3]>>16);g.r[2][2]=(int16_t)c[4];
    for(int i=0;i<3;++i){g.tr[i]=(int32_t)c[5+i];g.fc[i]=(int32_t)c[21+i];}
    g.ofx=(int32_t)c[24];g.ofy=(int32_t)c[25];g.h=(uint16_t)c[26];
    return g;
}

// e: the GTE's integer screen position minus the true projection (original
// corners); new points carry it interpolated along their edge, so a piece's
// edge runs exactly where the original renderer's neighbouring polygon has it.
struct Vertex { double p[3],uv[2],rgb[3]; int original; double ex=0,ey=0; };
// One polygon to draw here: attributes resolved as its renderer would.
struct Poly {
    unsigned count;int idx[4];uint32_t colors[4],uv[4],clut,tpage,command;bool textured;int bias;
    bool actor;
    // Conservative mode: one sort depth for the whole clipped polygon.
    double key_z;
};
struct Frame {
    NearGte g;bool object,viewmodel;uint32_t cursor,start,end,far_limit,zsf3;
    std::vector<NearProjected> verts;unsigned emitted;Stats* stats;
};

// The GPU skips a primitive wider than 1023 or taller than 511 pixels, so a
// polygon whose corners project safely can still vanish when it fills the
// view up close (the apartment wardrobe). Such polygons are drawn here.
bool oversize(const Frame& f,const int* idx,unsigned corners) {
    int x0=4096,x1=-4096,y0=4096,y1=-4096;
    for(unsigned j=0;j<corners;++j) {
        const uint32_t p=f.verts[idx[j]].sxy;const int x=(int16_t)p,y=(int16_t)(p>>16);
        x0=std::min(x0,x);x1=std::max(x1,x);y0=std::min(y0,y);y1=std::max(y1,y);
    }
    return x1-x0>1023 || y1-y0>511;
}
uint32_t ring_distance(const Frame& f,uint32_t from,uint32_t to) {
    const uint32_t size=f.end-f.start;
    return to>=from ? to-from : size-(from-to);
}
// Bytes still available to host packets this frame.
uint32_t budget(const Frame& f) {
    if(f.end<=f.start || f.cursor<f.start || f.cursor>=f.end) return 0;
    const uint32_t used=ring_distance(f,frame_first_cursor,f.cursor);
    const uint32_t ring=(uint32_t)((f.end-f.start)*ring_share);
    const uint32_t by_ring=used<ring?ring-used:0;
    const uint32_t by_host=frame_host_bytes<host_frame_bytes?host_frame_bytes-frame_host_bytes:0;
    return std::min(by_ring,by_host);
}
void screen(const Frame& f,const Vertex& v,double& x,double& y) {
    x=f.g.ofx/65536.0+f.g.h*v.p[0]/v.p[2];y=f.g.ofy/65536.0+f.g.h*v.p[1]/v.p[2];
}
// New corners round away from the piece's centre, so neighbouring pieces that
// split a shared edge at different points overlap by under a pixel instead of
// leaving hairline cracks (T-junctions). Original corners keep the GTE value.
uint32_t packed_xy(const Frame& f,const Vertex& v,double cx,double cy) {
    if(v.original>=0) return f.verts[v.original].sxy;
    double x,y;screen(f,v,x,y);x+=v.ex;y+=v.ey;
    const bool out=round_outward();
    const double rx=out&&x>=cx?std::ceil(x):std::floor(x),ry=out&&y>=cy?std::ceil(y):std::floor(y);
    const int32_t sx=clamp((int64_t)rx,-0x400,0x3ff),sy=clamp((int64_t)ry,-0x400,0x3ff);
    return (uint32_t)(uint16_t)sx | (uint32_t)(uint16_t)sy<<16;
}
bool tint_object;
uint32_t rgb_word(const Vertex& v) {
    if(tint()) return tint_object?0xc04020u:0x2040c0u;
    uint32_t w=0;
    for(int i=0;i<3;++i) w|=(uint32_t)clamp(std::lround(v.rgb[i]),0,255)<<(8*i);
    return w;
}
uint32_t uv_word(const Vertex& v) {
    return (uint32_t)clamp(std::lround(v.uv[0]),0,255) | (uint32_t)clamp(std::lround(v.uv[1]),0,255)<<8;
}
// Ordering-table slot: world 0x800117d0 (nearest SZ >> 5), object 0x800104a8
// (AVSZ3 >> 3 with the polygon's +-1 bias). -1 when past the depth limit.
int slot_index(const Frame& f,const Poly& poly,const Vertex* v) {
    uint32_t sz[3];
    for(int i=0;i<3;++i) sz[i]=(uint32_t)clamp(std::lround(poly.key_z>0?poly.key_z:v[i].p[2]),1,0xffff);
    if(!f.object) {
        const uint32_t nearest=sort_nearest()?std::min({sz[0],sz[1],sz[2]}):(sz[0]+sz[1]+sz[2])/3;
        if((int32_t)(nearest>>2)>=(int32_t)f.far_limit) return -1;
        return (int)std::min<uint32_t>(nearest>>5,0x7ff);
    }
    const int32_t otz=clamp(((int64_t)f.zsf3*(sz[0]+sz[1]+sz[2]))>>12,0,0xffff);
    if(otz>=(int32_t)f.far_limit) return -1;
    return clamp((otz>>3)+poly.bias+(poly.actor || conservative()?0:object_bias()),0,0x7ff);
}
// Packet allocation (0x800117a8/0x80010480) and single-packet OT insert.
void emit(Frame& f,const Poly& poly,const Vertex* v) {
    tint_object=f.object;
    const int index=f.viewmodel?0:slot_index(f,poly,v);
    if(index<0) return;
    const uint32_t size=poly.textured?0x28:0x1c;
    if(budget(f)<size) {++packet_skips;return;}
    if(!(f.cursor+size<f.end)) f.cursor=f.start;
    const uint32_t prim=f.cursor;
    double cx=0,cy=0;
    for(int i=0;i<3;++i){double x,y;screen(f,v[i],x,y);cx+=x/3;cy+=y/3;}
    if(poly.textured) {
        psx_mod_write_word(prim+4,poly.command|rgb_word(v[0]));
        psx_mod_write_word(prim+8,packed_xy(f,v[0],cx,cy));
        psx_mod_write_word(prim+12,poly.clut<<16|uv_word(v[0]));
        psx_mod_write_word(prim+16,rgb_word(v[1]));
        psx_mod_write_word(prim+20,packed_xy(f,v[1],cx,cy));
        psx_mod_write_word(prim+24,poly.tpage<<16|uv_word(v[1]));
        psx_mod_write_word(prim+28,rgb_word(v[2]));
        psx_mod_write_word(prim+32,packed_xy(f,v[2],cx,cy));
        psx_mod_write_word(prim+36,uv_word(v[2]));
    } else {
        psx_mod_write_word(prim+4,poly.command|rgb_word(v[0]));
        psx_mod_write_word(prim+8,packed_xy(f,v[0],cx,cy));
        psx_mod_write_word(prim+12,rgb_word(v[1]));
        psx_mod_write_word(prim+16,packed_xy(f,v[1],cx,cy));
        psx_mod_write_word(prim+20,rgb_word(v[2]));
        psx_mod_write_word(prim+24,packed_xy(f,v[2],cx,cy));
    }
    const uint32_t slot=ordering+(uint32_t)index*8,head=psx_mod_read_word(slot),tag=((size>>2)-1)<<24;
    if(f.viewmodel) {
        held_packets.push_back({prim,tag,(v[0].p[2]+v[1].p[2]+v[2].p[2])/3});
    } else if(!head) {
        psx_mod_write_word(prim,tag);psx_mod_write_word(slot,prim);psx_mod_write_word(slot+4,prim);
        const uint32_t word=bitmap+((uint32_t)index>>5)*4;
        psx_mod_write_word(word,psx_mod_read_word(word)|(0x80000000u>>(index&31)));
    } else {
        psx_mod_write_word(slot,prim);psx_mod_write_word(prim,tag|(head&0xffffff));
    }
    // Host precise vertices (D17A): new corners at their exact sub-pixel
    // position, original mesh corners at the GTE's integer one (they meet the
    // original renderer's neighbouring polygons exactly), every corner with its
    // view depth so the GPU maps the texture with correct perspective instead
    // of the PS1's affine warp across a polygon beside the eye.
    if(precise()) {
        for(int i=0;i<3;++i) {
            const uint32_t addr=prim+(poly.textured?8+12*i:8+8*i),word=psx_mod_read_word(addr);
            int32_t x16=(int32_t)(int16_t)word*65536,y16=(int32_t)(int16_t)(word>>16)*65536;
            if(v[i].original<0) {
                double x,y;screen(f,v[i],x,y);x+=v[i].ex;y+=v[i].ey;
                const double fx=std::floor(x),fy=std::floor(y);
                if(fx==(double)(int16_t)word && fy==(double)(int16_t)(word>>16)) {
                    x16=(int32_t)std::lround(x*65536.0);y16=(int32_t)std::lround(y*65536.0);
                }
            }
            // The first-person weapon is drawn last on purpose (D12): exact
            // perspective, but never depth-tested against the room.
            psx_mod_gpu_host_vertex(addr,word,x16,y16,(float)(f.viewmodel?-v[i].p[2]:v[i].p[2]),poly.textured?(float)std::clamp(v[i].uv[0],0.0,255.0):-1.0f,poly.textured?(float)std::clamp(v[i].uv[1],0.0,255.0):-1.0f);
        }
        ++precise_triangles;
    }
    f.cursor+=size;frame_host_bytes+=size;host_bytes_peak=std::max(host_bytes_peak,frame_host_bytes);++f.emitted;++f.stats->triangles;
}
Vertex mid(const Vertex& a,const Vertex& b,double t) {
    Vertex m{};
    for(int i=0;i<3;++i){m.p[i]=a.p[i]+(b.p[i]-a.p[i])*t;m.rgb[i]=a.rgb[i]+(b.rgb[i]-a.rgb[i])*t;}
    for(int i=0;i<2;++i) m.uv[i]=a.uv[i]+(b.uv[i]-a.uv[i])*t;
    // The snap error is linear along the edge on screen: screen parameter from
    // the view-space one (perspective) where both ends are in front of the eye.
    const double den=a.p[2]+t*(b.p[2]-a.p[2]);
    const double sp=a.p[2]>0 && b.p[2]>0 && den>0 ? t*b.p[2]/den : t;
    m.ex=a.ex+(b.ex-a.ex)*sp;m.ey=a.ey+(b.ey-a.ey)*sp;
    m.original=-1;return m;
}
// Split in view space, so each piece is perspective-correct at its corners
// and sorts by its own depth.
void subdivide(Frame& f,const Poly& poly,const Vertex* v,unsigned depth) {
    double sx[3],sy[3];
    for(int i=0;i<3;++i) screen(f,v[i],sx[i],sy[i]);
    int longest=0;double length=0;
    for(int i=0;i<3;++i) {
        const int j=(i+1)%3;const double l=std::hypot(sx[j]-sx[i],sy[j]-sy[i]);
        if(l>length){length=l;longest=i;}
    }
    double zmin=v[0].p[2],zmax=v[0].p[2];
    for(int i=1;i<3;++i){zmin=std::min(zmin,v[i].p[2]);zmax=std::max(zmax,v[i].p[2]);}
    // With host precise vertices the GPU maps textures with correct
    // perspective, so pieces are split only to keep their sort depth local
    // (a long wall must not sort as one polygon against what stands by it).
    const bool wanted=precise()
        ? length>precise_split_pixels() || (zmax>zmin*precise_depth_ratio() && length>32)
        : length>split_pixels || (zmax>zmin*depth_ratio && length>depth_pixels) || (depth_span()>0 && zmax-zmin>depth_span() && length>8);
    if(!wanted || depth>=max_depth || budget(f)<split_reserve) {
        if(wanted) ++f.stats->budget_hits;
        emit(f,poly,v);return;
    }
    const int a=longest,b=(longest+1)%3,c=(longest+2)%3;
    // Split where the edge's dominant texture coordinate is a whole texel:
    // packets carry integer UVs, and near the eye one texel spans many
    // pixels, so a rounded midpoint would kink straight texture lines.
    double t=0.5;
    const int axis=std::abs(v[b].uv[0]-v[a].uv[0])>=std::abs(v[b].uv[1]-v[a].uv[1])?0:1;
    const double span=v[b].uv[axis]-v[a].uv[axis];
    if(std::abs(span)>=2) t=std::clamp((std::round(v[a].uv[axis]+span/2)-v[a].uv[axis])/span,0.25,0.75);
    const Vertex m=mid(v[a],v[b],t);
    const Vertex first[3]={v[a],m,v[c]},second[3]={m,v[b],v[c]};
    subdivide(f,poly,first,depth+1);subdivide(f,poly,second,depth+1);
}
// The point where edge a-b crosses a plane, computed from the edge's
// lexicographically smaller end. Two polygons sharing an edge walk it in
// opposite directions; mid(a,b,t) and mid(b,a,1-t) differ in the last bits,
// and after flooring to whole pixels the shared corner could land one pixel
// apart: a hairline crack that moved with the camera (D17 showed it on most
// in-between images beside door frames and wall corners).
bool vertex_less(const Vertex& a,const Vertex& b) {
    for(int i=0;i<3;++i) if(a.p[i]!=b.p[i]) return a.p[i]<b.p[i];
    return false;
}
Vertex edge_cut(const Vertex& a,const Vertex& b,double da,double db) {
    if(vertex_less(b,a)) return mid(b,a,db/(db-da));
    return mid(a,b,da/(da-db));
}
// Inside when >= 0: near plane and the four guard-band planes.
double plane(const Vertex& v,int k,double h) {
    switch(k) {
        case 0: return v.p[2]-near_z;
        case 1: return guard_x*v.p[2]-h*v.p[0];
        case 2: return guard_x*v.p[2]+h*v.p[0];
        case 3: return guard_y*v.p[2]-h*v.p[1];
        default: return guard_y*v.p[2]+h*v.p[1];
    }
}
double det(const double* a,const double* b,const double* c) {
    return a[0]*(b[1]*c[2]-b[2]*c[1])-a[1]*(b[0]*c[2]-b[2]*c[0])+a[2]*(b[0]*c[1]-b[1]*c[0]);
}
// Textured pieces are cut along whole-texel lines (u or v = integer) in view
// space. Packets carry integer UVs and near the eye one texel spans many
// pixels, so a cut at a fractional coordinate would kink straight texture
// lines; cuts on texel lines leave interior corners exactly on the grid.
void split(const std::vector<Vertex>& in,int axis,double value,std::vector<Vertex>& low,std::vector<Vertex>& high) {
    for(size_t i=0;i<in.size();++i) {
        const Vertex& a=in[i];const Vertex& b=in[(i+1)%in.size()];
        const double da=a.uv[axis]-value,db=b.uv[axis]-value;
        if(da<=0) low.push_back(a);
        if(da>=0) high.push_back(a);
        if((da<0 && db>0) || (da>0 && db<0)) {
            Vertex m=edge_cut(a,b,da,db);m.uv[axis]=value;
            low.push_back(m);high.push_back(m);
        }
    }
}
void refine(Frame& f,const Poly& poly,const std::vector<Vertex>& shape,unsigned depth) {
    double lo[2]={1e9,1e9},hi[2]={-1e9,-1e9},x0=1e9,x1=-1e9,y0=1e9,y1=-1e9,zmin=1e9,zmax=0;
    for(const auto& v:shape) {
        for(int a=0;a<2;++a){lo[a]=std::min(lo[a],v.uv[a]);hi[a]=std::max(hi[a],v.uv[a]);}
        double x,y;screen(f,v,x,y);
        x0=std::min(x0,x);x1=std::max(x1,x);y0=std::min(y0,y);y1=std::max(y1,y);
        zmin=std::min(zmin,v.p[2]);zmax=std::max(zmax,v.p[2]);
    }
    const double size=std::max(x1-x0,y1-y0);
    const bool wanted=size>split_pixels || (zmax>zmin*depth_ratio && size>depth_pixels) || (depth_span()>0 && zmax-zmin>depth_span() && size>8);
    const int axis=hi[0]-lo[0]>=hi[1]-lo[1]?0:1;
    const double value=std::round((lo[axis]+hi[axis])/2);
    const bool can=value>lo[axis]+0.25 && value<hi[axis]-0.25;
    if(wanted && can && depth<max_depth*2 && budget(f)>=split_reserve) {
        std::vector<Vertex> low,high;split(shape,axis,value,low,high);
        if(low.size()>=3) refine(f,poly,low,depth+1);
        if(high.size()>=3) refine(f,poly,high,depth+1);
        return;
    }
    if(wanted) ++f.stats->budget_hits;
    for(size_t i=1;i+1<shape.size();++i) {
        const Vertex tri[3]={shape[0],shape[i],shape[i+1]};
        // A piece a texel or less across can still be large on screen; split
        // it geometrically (its texture is nearly flat there).
        if(wanted && !can) subdivide(f,poly,tri,depth); else emit(f,poly,tri);
    }
}
// Clip and draw one polygon. Quads are PS1 order (v0 v1 v2 v3 = two rows),
// so the outline is 0 1 3 2.
void draw(Frame& f,const Poly& poly) {
    ++f.stats->polys;
    std::vector<Vertex> shape;
    static const int order3[3]={0,1,2},order4[4]={0,1,3,2};
    const int* order=poly.count==3?order3:order4;
    for(unsigned j=0;j<poly.count;++j) {
        const int k=order[j],i=poly.idx[k];
        Vertex v{};
        for(int a=0;a<3;++a){v.p[a]=f.verts[i].view[a];v.rgb[a]=(poly.colors[k]>>(8*a))&0xff;}
        v.uv[0]=poly.uv[k]&0xff;v.uv[1]=(poly.uv[k]>>8)&0xff;
        v.original=f.verts[i].safe?i:-1;
        if(v.original>=0 && v.p[2]>0) {
            double x,y;screen(f,v,x,y);
            v.ex=(int16_t)f.verts[i].sxy-x;v.ey=(int16_t)(f.verts[i].sxy>>16)-y;
        }
        shape.push_back(v);
    }
    for(int k=0;k<5 && shape.size()>=3;++k) {
        std::vector<Vertex> out;
        for(size_t i=0;i<shape.size();++i) {
            const Vertex& a=shape[i];const Vertex& b=shape[(i+1)%shape.size()];
            const double da=plane(a,k,f.g.h),db=plane(b,k,f.g.h);
            if(da>=0) out.push_back(a);
            if((da>=0)!=(db>=0)) out.push_back(edge_cut(a,b,da,db));
        }
        shape.swap(out);
    }
    if(shape.size()<3) {++f.stats->culled;return;}
    // Every piece sorts by its own average depth (one key for a whole
    // polygon let near wall pieces cover a long wardrobe door).
    Poly local=poly;
    if(conservative()) {
        double z=0;for(const auto& v:shape) z+=v.p[2];local.key_z=z/shape.size();
        for(size_t i=1;i+1<shape.size();++i) {const Vertex tri[3]={shape[0],shape[i],shape[i+1]};emit(f,local,tri);}
        return;
    }
    const Poly& use=local;
    if(use.textured && !precise()) {refine(f,use,shape,0);return;}
    for(size_t i=1;i+1<shape.size();++i) {
        const Vertex tri[3]={shape[0],shape[i],shape[i+1]};
        subdivide(f,use,tri,0);
    }
}
// NCLIP's sign equals the sign of det(v0,v1,v2) in view space.
bool front_facing(const Frame& f,const Poly& poly,uint32_t flags,bool second_triangle) {
    const double* v0=f.verts[poly.idx[0]].view;const double* v1=f.verts[poly.idx[1]].view;const double* v2=f.verts[poly.idx[2]].view;
    const double sign=(flags&0x400)?-1:1;
    if(sign*det(v0,v1,v2)>0) return true;
    // World quads with 0x400 clear also accept a front-facing (v3,v1,v2).
    return second_triangle && poly.count==4 && !(flags&0x400) && -det(f.verts[poly.idx[3]].view,v1,v2)>0;
}
bool begin(Frame& f,CPUState* cpu,bool object) {
    f.g=read_gte(cpu);f.object=object;f.stats=object?&object_stats:&world_stats;
    f.cursor=psx_mod_read_word(context);f.start=psx_mod_read_word(context+4);f.end=psx_mod_read_word(context+8);
    f.far_limit=psx_mod_read_word(context+0x78);f.zsf3=(uint32_t)(int32_t)(int16_t)cpu->gte_ctrl[29];last_zsf3=(int32_t)f.zsf3;
    return f.g.h>=64;
}
uint32_t guest_copy() {
    if(!copy_base) copy_base=psx_mod_alloc_guest_memory(copy_size,16);
    return copy_base;
}
// Group writer for the filtered polygon list in the copy.
struct Group { uint32_t header,record_size; std::vector<uint32_t> kept; };
uint32_t list_bytes(const std::vector<Group>& groups) {
    uint32_t n=4;for(const auto& g:groups) n+=4+g.record_size*(uint32_t)g.kept.size();return n;
}
void write_list(uint32_t out,const std::vector<Group>& groups) {
    for(const auto& g:groups) {
        psx_mod_write_word(out,(g.header&0xffff)|(uint32_t)g.kept.size()<<16);out+=4;
        for(uint32_t r:g.kept) for(uint32_t i=0;i<g.record_size;i+=4){psx_mod_write_word(out,psx_mod_read_word(r+i));out+=4;}
    }
    psx_mod_write_word(out,0xff);
}
// Texture animation for textured polygons (world 0x80011c60, object 0x800105d0).
bool animate(uint32_t& t1,uint32_t& t2,uint32_t& t3,bool object) {
    const uint32_t anim=psx_mod_read_word(context+0x60);
    if(!anim) return false;
    const uint32_t frame=object?psx_mod_read_word(anim+(t1>>16)*16+8):psx_mod_read_word(anim+(t1>>16)*4);
    const uint32_t s1=psx_mod_read_word(frame),s2=psx_mod_read_word(frame+4),s4=s1&0xffff;
    const uint32_t bits=object?((t2>>21)&3)<<21:0;
    t2=(t2&0xffff)+((s2&0xffff0000u)|s4);t1=(t1&0xffff)+s1;t3=t3+((s4<<16)|s4);
    t2|=bits;
    return true;
}

// 0x80011020 entry: a0 mesh, a1 ordering table, a2 render context, a3 bitmap.
void world(CPUState* cpu) {
    const uint32_t mesh=cpu->gpr[4],ra=cpu->gpr[31];
    if(ra!=0x80037344 && ra!=0x80037c54 && ra!=0x80062db8) return;
    ++world_stats.seen;
    const unsigned count=psx_mod_read_half(mesh+0x14);
    if(!count || count>256) return;
    Frame f{};if(!begin(f,cpu,false)) return;
    const uint32_t flags=psx_mod_read_word(context+0x50);
    const int32_t fog=(int32_t)psx_mod_read_word(context+0x6c);
    std::vector<uint32_t> colors(count);
    f.verts.resize(count);
    bool any=false;
    for(unsigned i=0;i<count;++i) {
        const uint32_t record=mesh+0x1c+8*i,w0=psx_mod_read_word(record),w1=psx_mod_read_word(record+4);
        const uint32_t xy=((w0&0xff)<<10)|((w0&0xff00)<<15);
        f.verts[i]=near_project(f.g,(int16_t)xy,(int16_t)(xy>>16),(int16_t)((((w0>>16)&0xff)<<10)&0xffff));
        any|=!f.verts[i].safe || f.verts[i].sz<std::max<uint32_t>(near_depth(perspective_z),2048);
        uint32_t c=w1;
        if(flags&2) c=(w1&0xff000000u)|(psx_mod_read_word(context+0x54)&0xffffff);
        else if(const uint32_t nibble=(w0>>24)&0xf) c=(w1&0xff000000u)|(psx_mod_read_word(color_table+nibble*4)&0xffffff);
        const int32_t d=(int32_t)f.verts[i].sz-fog;
        colors[i]=d>0?near_dpcs(f.g,c,(int16_t)d):c;
    }
    if(!any) return;
    const uint32_t list=mesh+psx_mod_read_word(mesh+0x18);
    std::vector<Group> groups;std::vector<uint32_t> taken;
    uint32_t cursor=list;
    for(unsigned guard=0;;++guard) {
        if(guard>256) return;
        const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff) break;
        if((type!=0x60 && type!=0x61) || !n || n>4096) return;
        Group group{type,8,{}};
        const unsigned corners=type==0x60?3:4;
        for(uint32_t k=0;k<n;++k,cursor+=8) {
            const uint32_t word=psx_mod_read_word(cursor);
            bool unsafe=false;uint32_t zmin=0xffff,zmax=0;int idx[4];
            for(unsigned j=0;j<corners;++j) {
                const unsigned i=(word>>(8*j))&0xff;if(i>=count) return;idx[j]=(int)i;
                unsafe|=!f.verts[i].safe;zmin=std::min<uint32_t>(zmin,f.verts[i].sz);zmax=std::max<uint32_t>(zmax,f.verts[i].sz);
            }
            unsafe|=oversize(f,idx,corners);
            unsafe|=zmin<near_depth(perspective_z);(void)zmax;
            if(unsafe) taken.push_back(cursor|(type==0x61)); else group.kept.push_back(cursor);
        }
        if(!group.kept.empty()) groups.push_back(std::move(group));
    }
    if(taken.empty()) return;
    if(budget(f)<takeover_reserve+(uint32_t)taken.size()*0x28*3) {++mesh_fallbacks;return;}
    const uint32_t copy=guest_copy();
    const unsigned read_verts=(count+2)/3*3;
    const uint32_t head_size=0x1c+8*read_verts,list_offset=(head_size+15)&~15u;
    if(!copy || list_offset+list_bytes(groups)>copy_size) {++copy_overflows;return;}
    for(uint32_t i=0;i<head_size;i+=4) psx_mod_write_word(copy+i,psx_mod_read_word(mesh+i));
    psx_mod_write_word(copy+0x18,list_offset);
    write_list(copy+list_offset,groups);
    // The original clears the color-index nibble of every vertex it reads
    // (0x80011170); it now reads the copy, so apply that to the real mesh.
    if(!(flags&2))
        for(unsigned i=0;i<read_verts;++i) {
            const uint32_t b=mesh+0x1c+8*i+3;const uint8_t v=psx_mod_read_byte(b);
            if(v&0xf) psx_mod_write_byte(b,v&0xf0);
        }
    cpu->gpr[4]=copy;
    ++world_stats.taken;
    const uint32_t textures=psx_mod_read_word(context+0x58);
    for(uint32_t tagged:taken) {
        const uint32_t record=tagged&~1u;
        Poly p{};p.count=(tagged&1)?4:3;p.textured=true;p.command=0x34000000u;
        const uint32_t word=psx_mod_read_word(record);
        for(unsigned j=0;j<p.count;++j) p.idx[j]=(word>>(8*j))&0xff;
        if(!(psx_mod_read_half(record+6)&0x200) && !front_facing(f,p,flags,true)) {++world_stats.culled;continue;}
        const int16_t tex=(int16_t)psx_mod_read_half(record+4);
        const uint32_t entry=textures+(uint32_t)(tex*12);
        uint32_t t1=psx_mod_read_word(entry),t2=psx_mod_read_word(entry+4),t3=psx_mod_read_word(entry+8);
        if(((t2>>16)&0x200) && !animate(t1,t2,t3,false)) continue;
        p.clut=t1>>16;p.tpage=t2>>16;
        p.uv[0]=t1&0xffff;p.uv[1]=t2&0xffff;p.uv[2]=t3&0xffff;p.uv[3]=t3>>16;
        // Light table when every corner's color-index byte has bit 4 (0x80011908).
        uint32_t all=~0u;for(unsigned j=0;j<p.count;++j) all&=psx_mod_read_word(mesh+0x1c+8*p.idx[j])>>24;
        for(unsigned j=0;j<p.count;++j) {
            uint32_t c=colors[p.idx[j]];
            if(all&0x10) c=near_dpcs(f.g,c,(int16_t)psx_mod_read_word(psx_mod_read_word(context+0x68)+(c>>24)*4));
            p.colors[j]=c&0xffffff;
        }
        draw(f,p);
    }
    psx_mod_write_word(context,f.cursor);
}
unsigned object_record_size(uint32_t type) {
    switch(type) {
        case 0x20: case 0x28: return 8;
        case 0x30: case 0x38: return 12;
        case 0x24: case 0x2c: return 16;
        case 0x34: case 0x3c: return 20;
        default: return 0;
    }
}
// 0x80010000 entry, same register contract as the world renderer.
void object(CPUState* cpu) {
    const uint32_t mesh=cpu->gpr[4];
    const bool viewmodel=first_person_weapon_drawing();
    if(!viewmodel && first_person_duke_drawing()) {++duke_skips;return;}
    const bool actor=first_person_actor_drawing();
    ++object_stats.seen;
    const unsigned count=psx_mod_read_byte(mesh+6);
    const uint32_t vertices=psx_mod_read_word(mesh+0x10),list=psx_mod_read_word(mesh+0x14);
    if(!count || vertices<0x80010000 || vertices>0x801ffff0 || list<0x80010000 || list>0x801ffff0) return;
    Frame f{};if(!begin(f,cpu,true)) return;
    f.viewmodel=viewmodel;
    if(viewmodel && held_frame!=frame_first_cursor) {held_packets.clear();held_frame=frame_first_cursor;}
    f.verts.resize(count);
    bool any=viewmodel;
    for(unsigned i=0;i<count;++i) {
        const uint32_t xy=psx_mod_read_word(vertices+8*i);
        f.verts[i]=near_project(f.g,(int16_t)xy,(int16_t)(xy>>16),(int16_t)psx_mod_read_half(vertices+8*i+4));
        any|=!f.verts[i].safe || f.verts[i].sz<std::max<uint32_t>(near_depth(object_perspective_z),2048);
    }
    if(!any) return;
    std::vector<Group> groups;std::vector<std::pair<uint32_t,uint32_t>> taken;
    uint32_t cursor=list;
    for(unsigned guard=0;;++guard) {
        if(guard>256) return;
        const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff) break;
        const unsigned size=object_record_size(type);
        if(!size || !n || n>4096) return;
        Group group{type,size,{}};
        const unsigned corners=(type&8)?4:3;
        for(uint32_t k=0;k<n;++k,cursor+=size) {
            const uint32_t word=psx_mod_read_word(cursor);
            bool unsafe=false,all_near=true;int idx[4];
            for(unsigned j=0;j<corners;++j) {
                const unsigned i=(word>>(8*j))&0xff;if(i>=count) return;idx[j]=(int)i;
                unsafe|=!f.verts[i].safe || f.verts[i].sz<near_depth(object_perspective_z);all_near&=f.verts[i].sz<f.g.h;
            }
            // The original drops a polygon whose corners are all nearer than
            // H; in the eye view that made props see-through up close, so such
            // polygons are drawn here too.
            unsafe|=all_near || oversize(f,idx,corners) || viewmodel;
            if(unsafe) taken.push_back({cursor,type}); else group.kept.push_back(cursor);
        }
        if(!group.kept.empty()) groups.push_back(std::move(group));
    }
    if(taken.empty()) return;
    if(budget(f)<takeover_reserve+(uint32_t)taken.size()*0x28*3) {++mesh_fallbacks;return;}
    const uint32_t copy=guest_copy();
    if(!copy || 0x20+list_bytes(groups)>copy_size) {++copy_overflows;return;}
    for(uint32_t i=0;i<0x18;i+=4) psx_mod_write_word(copy+i,psx_mod_read_word(mesh+i));
    psx_mod_write_word(copy+0x14,copy+0x20);
    write_list(copy+0x20,groups);
    cpu->gpr[4]=copy;
    ++object_stats.taken;viewmodel_meshes+=viewmodel;
    const uint32_t flags=psx_mod_read_word(context+0x50),palette=vertices+count*8;
    const uint32_t command_bits=psx_mod_read_word(context+0x48),tpage_bits=psx_mod_read_word(context+0x4c);
    for(const auto& [record,type]:taken) {
        Poly p{};p.count=(type&8)?4:3;p.actor=actor;
        const uint32_t word=psx_mod_read_word(record);
        for(unsigned j=0;j<p.count;++j) p.idx[j]=(word>>(8*j))&0xff;
        const bool textured=type==0x24 || type==0x2c || type==0x34 || type==0x3c;
        const bool gouraud=type==0x30 || type==0x38 || type==0x34 || type==0x3c;
        const uint32_t bits=psx_mod_read_word(record+(type==0x20 || type==0x28 || type==0x30 || type==0x38 ? 4 : 8))>>25;
        if(!(bits&1) && !front_facing(f,p,flags,false)) {++object_stats.culled;continue;}
        p.textured=textured;p.bias=((bits&8)?1:0)-((bits&4)?1:0);
        p.command=(textured?0x34000000u:0x30000000u)|((bits&0x10)<<21)|command_bits;
        if(textured) {
            uint32_t t1=psx_mod_read_word(record+4),t2=psx_mod_read_word(record+8),t3=psx_mod_read_word(record+12);
            if((bits&0x20) && !animate(t1,t2,t3,true)) continue;
            t2|=tpage_bits;
            p.clut=t1>>16;p.tpage=t2>>16;
            p.uv[0]=t1&0xffff;p.uv[1]=t2&0xffff;p.uv[2]=t3&0xffff;p.uv[3]=t3>>16;
        }
        const uint32_t color_bytes=type==0x34 || type==0x3c ? 16 : 8;
        for(unsigned j=0;j<p.count;++j) {
            uint32_t c;
            if(gouraud) c=psx_mod_read_word(palette+4*psx_mod_read_byte(record+color_bytes+j));
            else if(textured) c=psx_mod_read_word(context+((bits&2)?0x5c:0x54));
            else c=psx_mod_read_word(palette+4*psx_mod_read_byte(record+4));
            p.colors[j]=c&0xffffff;
        }
        draw(f,p);
    }
    psx_mod_write_word(context,f.cursor);
}
// Occluder fade. The prop loop (0x80031fa0..0x80032078) makes a prop
// semi-transparent and dims it by distance when it is nearer than the fade
// distance (camera+0xa0) and its screen rectangle overlaps Duke's
// (camera+0xa8), tested by 0x8002ee50(a0 Duke's rect, a1 prop rect; halves
// x,y,w,h). That keeps Duke visible in third person, but in the eye view
// Duke's rectangle covers most of the screen, so a door walked up to turns
// nearly invisible. In the eye view the test is given an empty rectangle far
// off screen (only this call's argument changes). DNTTK_FP_OCCLUDER_FADE=1
// keeps the original fade (a future menu toggle, D19).
bool keep_fade() {static const bool v=env_int("DNTTK_FP_OCCLUDER_FADE",0,0,1)==1;return v;}
void occluder_test(CPUState* cpu) {
    ++fade_tests;
    if(keep_fade() || cpu->gpr[31]!=0x80032010) return;
    if(!empty_rect) {
        empty_rect=psx_mod_alloc_guest_memory(16,16);
        if(!empty_rect) return;
        psx_mod_write_word(empty_rect,0x8300u<<16|0x8300u); // x=y=-32000
        psx_mod_write_word(empty_rect+4,0);                   // w=h=0
    }
    cpu->gpr[4]=empty_rect;++fades_skipped;
}
void near_hook_body(CPUState* cpu,uint32_t address) {
    const bool eye=first_person_view_live();
    if(!eye && !widescreen_near_clip_live()) return;
    if(address==0x8002ee50) {if(eye && identity()) occluder_test(cpu); return;}
    {
        const uint32_t cursor=psx_mod_read_word(context),start=psx_mod_read_word(context+4),end=psx_mod_read_word(context+8);
        bool empty=true;
        for(uint32_t i=0;i<0x100 && empty;i+=4) empty=!psx_mod_read_word(bitmap+i);
        if(empty) {frame_first_cursor=cursor;frame_host_bytes=0;}
        if(end>start && cursor>=start && cursor<end) {
            arena_size=end-start;
            const uint32_t used=cursor>=frame_first_cursor?cursor-frame_first_cursor:arena_size-(frame_first_cursor-cursor);
            frame_used_peak=std::max(frame_used_peak,used);
        }
    }
    if(!enabled()) return;
    const uint32_t mesh=cpu->gpr[4];
    if(!mesh || cpu->gpr[5]!=ordering || cpu->gpr[6]!=context || cpu->gpr[7]!=bitmap) return;
    if(mesh<0x80010000 || mesh>0x801f0000 || (mesh&3)) return;
    if(!identity()) {++refused;return;}
    if(address==0x80011020 && (renderers()&1)) world(cpu);
    else if(address==0x80010000 && (renderers()&2)) object(cpu);
}
void hook(CPUState* cpu,uint32_t address) {
    if(!frame_trace_on()) {near_hook_body(cpu,address);return;}
    const auto t0=std::chrono::steady_clock::now();near_hook_body(cpu,address);
    frame_trace_account(address|0x1u,(long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count());
}
}

void near_clip_counters(uint64_t out[8]) {
    out[0]=world_stats.taken;out[1]=world_stats.triangles;out[2]=object_stats.taken;out[3]=object_stats.triangles;
    out[4]=mesh_fallbacks;out[5]=world_stats.budget_hits+object_stats.budget_hits;out[6]=copy_overflows;out[7]=refused;
}
void near_clip_prepare() {
    (void)guest_copy();
    if(!empty_rect) {
        empty_rect=psx_mod_alloc_guest_memory(16,16);
        if(empty_rect) {
            psx_mod_write_word(empty_rect,0x8300u<<16|0x8300u);
            psx_mod_write_word(empty_rect+4,0);
        }
    }
}
NearClipFrameState near_clip_frame_state() {return {frame_first_cursor,frame_host_bytes,held_frame};}
void near_clip_load_frame_state(const NearClipFrameState& s) {
    frame_first_cursor=s.first_cursor;frame_host_bytes=s.host_bytes;held_frame=s.held_frame;
    held_packets.clear();
}

void near_clip_viewmodel_flush() {
    if(held_packets.empty()) return;
    // Packets from an earlier frame may already be overwritten in the ring.
    bool empty=true;
    for(uint32_t i=0;i<0x100 && empty;i+=4) empty=!psx_mod_read_word(bitmap+i);
    if(empty || held_frame!=frame_first_cursor) {++viewmodel_discards;held_packets.clear();return;}
    // Farthest first, appended after the slot's current tail: the slot is
    // drawn head to tail, so the weapon follows anything else near the eye
    // (Duke's chest when looking down). The frame's OT link-up still patches
    // the tail's link, now the weapon's last packet.
    std::stable_sort(held_packets.begin(),held_packets.end(),[](const HeldPacket& a,const HeldPacket& b){return a.z>b.z;});
    for(size_t i=0;i+1<held_packets.size();++i)
        psx_mod_write_word(held_packets[i].prim,held_packets[i].tag|(held_packets[i+1].prim&0xffffff));
    const HeldPacket& last=held_packets.back();
    psx_mod_write_word(last.prim,last.tag);
    const uint32_t slot=ordering+viewmodel_slot*8;
    const uint32_t head=psx_mod_read_word(slot),tail=psx_mod_read_word(slot+4);
    if(!head) {
        psx_mod_write_word(slot,held_packets.front().prim);
        psx_mod_write_word(bitmap,psx_mod_read_word(bitmap)|(0x80000000u>>viewmodel_slot));
    } else {
        psx_mod_write_word(tail,(psx_mod_read_word(tail)&0xff000000u)|(held_packets.front().prim&0xffffff));
    }
    psx_mod_write_word(slot+4,last.prim);
    viewmodel_packets+=held_packets.size();held_packets.clear();
}
NearProjected near_project(const NearGte& g,int16_t vx,int16_t vy,int16_t vz) {
    NearProjected out{};
    int64_t mac[3];
    for(int k=0;k<3;++k) {
        mac[k]=(int64_t)g.tr[k]*4096+(int64_t)g.r[k][0]*vx+(int64_t)g.r[k][1]*vy+(int64_t)g.r[k][2]*vz;
        out.view[k]=mac[k]/4096.0;
    }
    const int32_t m1=(int32_t)(mac[0]>>12),m2=(int32_t)(mac[1]>>12);
    const int32_t ir1=clamp(m1,-0x8000,0x7fff),ir2=clamp(m2,-0x8000,0x7fff);
    out.sz=(uint16_t)clamp(mac[2]>>12,0,0xffff);
    const int32_t q=gte_divide(g.h,out.sz);
    const int64_t x16=(int64_t)g.ofx+(int64_t)ir1*q,y16=(int64_t)g.ofy+(int64_t)ir2*q;
    const int32_t sx=clamp(x16>>16,-0x400,0x3ff),sy=clamp(y16>>16,-0x400,0x3ff);
    out.sxy=(uint32_t)(uint16_t)sx | (uint32_t)(uint16_t)sy<<16;
    bool safe=(uint32_t)out.sz*2>g.h && ir1==m1 && ir2==m2;
    if(safe) {
        const double tx=g.ofx/65536.0+g.h*out.view[0]/out.view[2];
        const double ty=g.ofy/65536.0+g.h*out.view[1]/out.view[2];
        safe=std::abs(tx)<=1000 && std::abs(ty)<=1000;
    }
    out.safe=safe;
    return out;
}
uint32_t near_dpcs(const NearGte& g,uint32_t rgbc,int16_t ir0) {
    uint32_t out=rgbc&0xff000000u;
    for(int i=0;i<3;++i) {
        const int64_t base=(int64_t)((rgbc>>(8*i))&0xff)<<16;
        const int32_t step=clamp(((int64_t)g.fc[i]*4096-base)>>12,-0x8000,0x7fff);
        const int32_t mac=(int32_t)((base+(int64_t)ir0*step)>>12);
        out|=(uint32_t)clamp(mac>>4,0,255)<<(8*i);
    }
    return out;
}
const char* near_clip_debug_json() {
    static char buffer[768];
    auto part=[](const Stats& s,char* out,size_t n){
        std::snprintf(out,n,"{\"seen\":%llu,\"taken\":%llu,\"polys\":%llu,\"culled\":%llu,\"triangles\":%llu,\"budget_hits\":%llu}",
            (unsigned long long)s.seen,(unsigned long long)s.taken,(unsigned long long)s.polys,(unsigned long long)s.culled,
            (unsigned long long)s.triangles,(unsigned long long)s.budget_hits);
    };
    char w[192],o[192];part(world_stats,w,sizeof w);part(object_stats,o,sizeof o);
    std::snprintf(buffer,sizeof buffer,"{\"enabled\":%s,\"world\":%s,\"object\":%s,\"refused\":%llu,\"copy_overflows\":%llu,\"duke_skips\":%llu,\"fades_skipped\":%llu,\"fade_tests\":%llu,\"zsf3\":%d,\"arena_size\":%u,\"frame_used_peak\":%u,\"host_bytes_peak\":%u,\"packet_skips\":%llu,\"mesh_fallbacks\":%llu,\"viewmodel\":{\"meshes\":%llu,\"packets\":%llu,\"discards\":%llu}}",
        enabled()?"true":"false",w,o,(unsigned long long)refused,(unsigned long long)copy_overflows,(unsigned long long)duke_skips,(unsigned long long)fades_skipped,(unsigned long long)fade_tests,(int)last_zsf3,arena_size,frame_used_peak,host_bytes_peak,(unsigned long long)packet_skips,(unsigned long long)mesh_fallbacks,(unsigned long long)viewmodel_meshes,(unsigned long long)viewmodel_packets,(unsigned long long)viewmodel_discards);
    return buffer;
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_near_clip) {
    for(uint32_t address:{0x80010000u,0x80011020u,0x8002ee50u})
        psx_mod_register_function_entry_plugin("ttk.near.clip",address,ttk::hook);
}
