// D53: Duke 3D-style security monitors (security_monitor.h). See
// documentation/432-d53-security-monitors.md.
//
// SECURITY MONITORS (DNTTK_SECURITY_MONITORS, live): DUKE 3D (the Modernized
// default) or TTK. In the Duke 3D look, Polymorph only:
//
// - The TTK monitor (a flat 400x500 panel of nine quads, mesh found by its
//   shape) is drawn as a larger TV in the style and colours of Duke 3D's
//   security screen: a 620x620 box standing 140 out from the wall, a warm
//   grey bezel with a double frame and a control strip, and a recessed
//   square glass screen (olive SECURITY, camera status, a blinking cursor). The object renderer's
//   entry (near_clip.cpp) is handed a guest copy of the TV mesh instead; its
//   polygons are drawn by the near-clip host path from the second texture
//   bank (as the D17AO1 borrowed sky), where the TV atlas sits at x 0..255,
//   so no retail texture or VRAM is touched. The prop, its interaction and
//   collision are the game's own.
// - The screen is unlit by the room (gouraud faces, fixed colour) with a
//   faint flicker, and each TV near Duke adds a soft light in the screen's
//   olive phosphor in front of it (D41A light list).
// - The monitor props keep the TV's bounds (+0x4c) while the look is on, so
//   the game does not cull the larger TV early at the edge of the view (the
//   D17AZ train case); the TTK look puts the mesh's own back.
// - The control console by LEVEL00's door (found by shape) gives a soft green
//   light over its controls in Modernized with DYNAMIC LIGHTS, either look.
// - While the game draws through a security camera (Duke +0x224 bit
//   0x20000000, the alternate camera, D52), the present pass draws Duke
//   3D's camera frame over the 4:3 picture: four corners, REC and the record
//   light blinking 133 ms on, 133 ms off (EDuke32 G_DrawCameraText).
//
// Art: assets/ui/security-monitor (Photon Studio), packed by
// tools/local/build_ttk_security_monitor.py into ttk-security-monitor.pack.
// TTK, Classic and Vanilla: nothing changes.
#include "security_monitor.h"
#include "live_env.h"
#include "pc_input.h"
#include "mod_plugins.h"
#include "cpu_state.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace ttk {
namespace {
constexpr uint32_t game_mode=0x800bcbb0u,player_index=0x800d2654u,players=0x800d7198u;

// ---- pack ---------------------------------------------------------------
struct Piece {int kind,x,y,w,h;std::vector<uint32_t> argb;};
struct Pack {
    bool ok=false;
    int aw=0,ah=0;
    std::vector<uint16_t> atlas;
    std::vector<Piece> pieces;
};
Pack pack;
std::once_flag pack_once;
uint32_t rd16(const std::vector<uint8_t>& b,size_t o) {return b[o]|b[o+1]<<8;}
void load_pack() {
    std::string path;
    if(const char* p=std::getenv("DNTTK_SECURITY_PACK")) path=p;
    else if(auto base=SDL_GetBasePath()) {
        path=std::string(base)+"ttk-security-monitor.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    } else return;
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>4*1024*1024) return;
    std::vector<uint8_t> b(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(b.data()),b.size())) return;
    if(std::memcmp(b.data(),"TTKSEC1\0",8)) return;
    size_t o=8;
    pack.aw=(int)rd16(b,o);pack.ah=(int)rd16(b,o+2);o+=4;
    if(pack.aw!=256 || pack.ah<1 || pack.ah>256 || o+size_t(pack.aw*pack.ah)*2+4>b.size()) return;
    pack.atlas.resize(size_t(pack.aw*pack.ah));
    for(size_t i=0;i<pack.atlas.size();++i,o+=2) pack.atlas[i]=(uint16_t)rd16(b,o);
    const uint32_t n=b[o]|b[o+1]<<8|b[o+2]<<16|(uint32_t)b[o+3]<<24;o+=4;
    if(n>16) return;
    for(uint32_t i=0;i<n;++i) {
        if(o+10>b.size()) return;
        Piece p;p.kind=(int)rd16(b,o);p.x=(int)rd16(b,o+2);p.y=(int)rd16(b,o+4);p.w=(int)rd16(b,o+6);p.h=(int)rd16(b,o+8);o+=10;
        if(p.w<1 || p.h<1 || p.x+p.w>1600 || p.y+p.h>1200 || o+size_t(p.w*p.h)*4>b.size()) return;
        p.argb.resize(size_t(p.w*p.h));
        for(auto& c:p.argb) {c=(uint32_t(b[o+3])<<24)|(uint32_t(b[o])<<16)|(uint32_t(b[o+1])<<8)|b[o+2];o+=4;}
        pack.pieces.push_back(std::move(p));
    }
    pack.ok=true;
    std::fprintf(stderr,"[TTK security] monitor pack %s (%zu overlay pieces)\n",path.c_str(),pack.pieces.size());
}
bool pack_ok() {std::call_once(pack_once,load_pack);return pack.ok;}

// ---- option -------------------------------------------------------------
bool duke_look() {
    if(!input_modernized() || ttk_world_classic()) return false;
    const char* v=ttk_live_env("DNTTK_SECURITY_MONITORS");
    return !(v && !std::strcmp(v,"ttk")) && pack_ok();
}

// ---- the TV mesh ----------------------------------------------------------
// Model space as the TTK monitor: the panel at z 0 on the wall, outward -z,
// y down, u along +x. Front 620x620 at z -140; screen opening (bezel art
// 16..111 of 128) recessed to z -125.
constexpr int W=310,H=310,F=-140,S=-125,OX0=-232,OX1=232,OY0=-232,OY1=232;
constexpr uint32_t tpage=0x100;   // 15-bit direct, second bank x 0, y 0
constexpr uint32_t screen_uv_v[2]={0,96};
struct V {int x,y,z;};
// kind: 0 flat (room-lit), 1 the screen (self-lit, cursor frames), 2 the
// glow ring (self-lit, additive). rot: v runs from TL to TR (left and right
// haze strips) instead of from TL to BL.
enum {flat=0,screen_face=1,haze_face=2};
struct Face {V tl,tr,bl,br;int u0,v0,u1,v1;int out[3];int kind;bool rot=false;};
// The glow ring (revision 7): four mitred strips just in front of the bezel,
// from 10 units over the screen's edge out to 300, sharing the haze
// gradient (144,192 32x16, row 0 at the screen's edge).
constexpr int HZ=F-3,HI=222,HO=300;
const Face faces[]={
    {{-W,-H,F},{W,-H,F},{-W,H,F},{W,H,F},0,0,127,127,{0,0,-1},flat},                    // front bezel
    // Box sides: the top and bottom strip (0,192 128x32, front edge at row
    // 0) and the left and right strip (224,0 32x128, front at column 0),
    // turned so the lit edge always meets the bezel.
    {{-W,-H,0},{W,-H,0},{-W,-H,F},{W,-H,F},0,223,127,192,{0,-1,0},flat},               // top
    {{-W,H,F},{W,H,F},{-W,H,0},{W,H,0},0,192,127,223,{0,1,0},flat},                    // bottom
    {{-W,-H,0},{-W,-H,F},{-W,H,0},{-W,H,F},255,0,224,127,{-1,0,0},flat},               // left
    {{W,-H,F},{W,-H,0},{W,H,F},{W,H,0},224,0,255,127,{1,0,0},flat},                    // right
    {{OX0,OY0,F},{OX1,OY0,F},{OX0,OY0,S},{OX1,OY0,S},128,192,143,207,{0,1,0},flat},    // recess top
    {{OX0,OY1,S},{OX1,OY1,S},{OX0,OY1,F},{OX1,OY1,F},128,192,143,207,{0,-1,0},flat},   // recess bottom
    {{OX0,OY0,S},{OX0,OY0,F},{OX0,OY1,S},{OX0,OY1,F},128,192,143,207,{1,0,0},flat},    // recess left
    {{OX1,OY0,F},{OX1,OY0,S},{OX1,OY1,F},{OX1,OY1,S},128,192,143,207,{-1,0,0},flat},   // recess right
    {{OX0,OY0,S},{OX1,OY0,S},{OX0,OY1,S},{OX1,OY1,S},128,0,223,95,{0,0,-1},screen_face},  // screen
    {{-HO,-HO,HZ},{HO,-HO,HZ},{-HI,-HI,HZ},{HI,-HI,HZ},144,207,175,192,{0,0,-1},haze_face},        // glow top
    {{-HI,HI,HZ},{HI,HI,HZ},{-HO,HO,HZ},{HO,HO,HZ},144,192,175,207,{0,0,-1},haze_face},            // glow bottom
    {{-HO,-HO,HZ},{-HI,-HI,HZ},{-HO,HO,HZ},{-HI,HI,HZ},144,207,175,192,{0,0,-1},haze_face,true},   // glow left
    {{HI,-HI,HZ},{HO,-HO,HZ},{HI,HI,HZ},{HO,HO,HZ},144,192,175,207,{0,0,-1},haze_face,true},       // glow right
};
constexpr unsigned face_count=sizeof faces/sizeof faces[0];
struct Built {
    std::vector<V> verts;
    struct Rec {unsigned idx[4];uint32_t uv[4];int kind;};
    std::vector<Rec> recs;
    uint32_t vertices=0,palette=0,list=0,screen_rec=0,bytes=0;
};
Built built;
unsigned vert_index(std::vector<V>& vs,const V& v) {
    for(unsigned i=0;i<vs.size();++i) if(vs[i].x==v.x && vs[i].y==v.y && vs[i].z==v.z) return i;
    vs.push_back(v);return (unsigned)vs.size()-1;
}
// PS1 quad order is TL, TR, BL, BR; the object renderer's faces wind so that
// minus (P1-P0) x (P2-P0) points outward. A face that would not is mirrored
// (TR, TL, BR, BL with its texture mirrored back), keeping the art upright.
void build_mesh() {
    for(const Face& f:faces) {
        V c[4]={f.tl,f.tr,f.bl,f.br};
        uint32_t uv[4]={uint32_t(f.u0|f.v0<<8),uint32_t(f.u1|f.v0<<8),uint32_t(f.u0|f.v1<<8),uint32_t(f.u1|f.v1<<8)};
        if(f.rot) {uv[0]=uint32_t(f.u0|f.v0<<8);uv[1]=uint32_t(f.u0|f.v1<<8);uv[2]=uint32_t(f.u1|f.v0<<8);uv[3]=uint32_t(f.u1|f.v1<<8);}
        const double a[3]={double(c[1].x-c[0].x),double(c[1].y-c[0].y),double(c[1].z-c[0].z)};
        const double b[3]={double(c[2].x-c[0].x),double(c[2].y-c[0].y),double(c[2].z-c[0].z)};
        const double n[3]={-(a[1]*b[2]-a[2]*b[1]),-(a[2]*b[0]-a[0]*b[2]),-(a[0]*b[1]-a[1]*b[0])};
        if(n[0]*f.out[0]+n[1]*f.out[1]+n[2]*f.out[2]<0) {std::swap(c[0],c[1]);std::swap(c[2],c[3]);std::swap(uv[0],uv[1]);std::swap(uv[2],uv[3]);}
        Built::Rec r{};r.kind=f.kind;
        for(int k=0;k<4;++k) {r.idx[k]=vert_index(built.verts,c[k]);r.uv[k]=uv[k];}
        built.recs.push_back(r);
    }
}
// Layout: header 0x18, bounds 0x10, vertices, palette (4 entries), list.
bool allocate() {
    if(security_tv_copy) return true;
    if(psx_mod_replay_active()) return false;
    build_mesh();
    const uint32_t nv=(uint32_t)built.verts.size();
    unsigned flat=0,screen=0;for(const auto& r:built.recs) (r.kind!=0?screen:flat)++;
    const uint32_t bytes=0x18+0x10+nv*8+16+4+flat*16+4+screen*20+4;
    const uint32_t base=psx_mod_alloc_guest_memory((bytes+15)&~15u,16);
    if(!base) return false;
    built.vertices=base+0x28;built.palette=built.vertices+nv*8;built.list=built.palette+16;built.bytes=bytes;
    security_tv_copy=base;security_tv_copy_end=base+bytes;
    return true;
}
// Cursor blink (530 ms) and a faint flicker of the screen and its light, a
// new level every 33 ms, from the clock: the same in redraw workers.
bool cursor_on() {return ((uint32_t)SDL_GetTicks()/530)&1;}
// The screen's self-lit level (128 is the texture as painted; DNTTK_SCREEN_LEVEL).
uint32_t screen_base=230;
uint32_t flicker_level() {
    uint32_t h=(uint32_t)SDL_GetTicks()/33*2654435761u;h^=h>>15;
    return std::min<uint32_t>(255,screen_base-4+((h>>7)&7));   // a faint flicker around screen_base
}
void write_mesh(uint32_t mesh_template) {
    const uint32_t base=security_tv_copy;
    // Header: the monitor's own first words (ids and flags), then our parts.
    psx_mod_write_word(base,psx_mod_read_word(mesh_template));
    psx_mod_write_word(base+4,(psx_mod_read_word(mesh_template+4)&0xff00ffffu)|uint32_t(built.verts.size())<<16);
    psx_mod_write_word(base+8,base+0x18);psx_mod_write_word(base+0xc,0);
    psx_mod_write_word(base+0x10,built.vertices);psx_mod_write_word(base+0x14,built.list);
    // Bounds: min x,y,z and the template's fourth half, max x,y,z and radius.
    const int radius=(int)std::lround(std::sqrt(double(W*W+H*H+F*F)));
    psx_mod_write_word(base+0x18,uint32_t(uint16_t(-W))|uint32_t(uint16_t(-H))<<16);
    psx_mod_write_word(base+0x1c,uint32_t(uint16_t(F))|uint32_t(uint16_t(W))<<16);
    psx_mod_write_word(base+0x20,uint32_t(uint16_t(W))|uint32_t(uint16_t(H))<<16);
    psx_mod_write_word(base+0x24,uint32_t(uint16_t(0))|uint32_t(uint16_t(radius))<<16);
    const uint32_t vertex_tag=psx_mod_read_word(psx_mod_read_word(mesh_template+0x10)+4)&0xffff0000u;
    for(unsigned i=0;i<built.verts.size();++i) {
        const V& v=built.verts[i];
        psx_mod_write_word(built.vertices+8*i,uint32_t(uint16_t(v.x))|uint32_t(uint16_t(v.y))<<16);
        psx_mod_write_word(built.vertices+8*i+4,uint32_t(uint16_t(v.z))|vertex_tag);
    }
    const uint32_t level=flicker_level(),lit=level|level<<8|level<<16;
    // Palette entry 0 lights the screen; entry 1 the glow ring, at the
    // texture's own strength (128) times the screen's flicker.
    const uint32_t hl=std::min<uint32_t>(255,128*level/screen_base),haze_lit=hl|hl<<8|hl<<16;
    psx_mod_write_word(built.palette,lit);psx_mod_write_word(built.palette+4,haze_lit);
    psx_mod_write_word(built.palette+8,lit);psx_mod_write_word(built.palette+12,lit);
    uint32_t at=built.list;
    for(int pass=0;pass<2;++pass) {
        const bool screen=pass==1;
        unsigned n=0;for(const auto& r:built.recs) n+=(r.kind!=0)==screen;
        psx_mod_write_word(at,(screen?0x3cu:0x2cu)|n<<16);at+=4;
        for(const auto& r:built.recs) {
            if((r.kind!=0)!=screen) continue;
            uint32_t uv[4];for(int k=0;k<4;++k) uv[k]=r.uv[k];
            const bool haze=r.kind==haze_face;
            if(r.kind==screen_face) {
                const uint32_t dv=screen_uv_v[cursor_on()?1:0]<<8;
                for(auto& x:uv) x+=dv;
                built.screen_rec=at;
            }
            psx_mod_write_word(at,r.idx[0]|r.idx[1]<<8|r.idx[2]<<16|r.idx[3]<<24);
            psx_mod_write_word(at+4,uv[0]);                    // CLUT 0 (direct colour)
            // The ring: semi-transparent (bit 0x10 of the flags above the
            // page) with blend mode 1 in the page (B + F, additive).
            psx_mod_write_word(at+8,uv[1]|(haze?(tpage|0x20u)<<16|0x10u<<25:tpage<<16));
            psx_mod_write_word(at+12,uv[2]|uv[3]<<16);
            if(screen) psx_mod_write_word(at+16,haze?0x01010101u:0);   // palette entry 1 (ring) or 0 (screen) at each corner
            at+=screen?20:16;
        }
    }
    psx_mod_write_word(at,0xff);
}
// The TTK monitor: 16 vertices, nine flat textured quads, its first corner at
// (-200, 250, 0) and bounding radius 320.
bool monitor_mesh(uint32_t mesh) {
    if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) return false;
    if(psx_mod_read_byte(mesh+6)!=16) return false;
    const uint32_t vp=psx_mod_read_word(mesh+0x10),list=psx_mod_read_word(mesh+0x14),bounds=psx_mod_read_word(mesh+8);
    if(vp<0x80010000u || vp>0x801fff00u || list<0x80010000u || list>0x801fff00u || bounds<0x80010000u || bounds>0x801ffff0u) return false;
    return psx_mod_read_word(list)==0x0009002cu && psx_mod_read_word(vp)==0x00faff38u && psx_mod_read_half(vp+4)==0 &&
           psx_mod_read_half(bounds+14)==320;
}
uint64_t swaps=0,lamps_published=0;
uint32_t last_template=0;

// Controls that glow (scan_props, game thread; read by redraw workers too,
// so fixed atomic slots): interactive keypads (type 477's one-quad panel,
// on consoles and on walls) and the lightning power sign beside a keypad.
std::atomic<uint32_t> keypad_meshes[4],sign_meshes[4];
bool listed(const std::atomic<uint32_t>* slots,uint32_t mesh) {
    for(unsigned i=0;i<4;++i) if(mesh && slots[i].load(std::memory_order_relaxed)==mesh) return true;
    return false;
}
uint64_t glow_swaps=0;
uint32_t keypad_level=255;
// The power sign's electric pulse, 0..1 from the clock (the same in redraw
// workers): a slow 1.8 s breath with a quick crackle now and then (about one
// 50 ms step in 14 jumps to full or drops low).
float power_pulse() {
    const uint32_t ms=(uint32_t)SDL_GetTicks();
    float v=0.5f+0.5f*std::sin(ms*(2*3.14159265f/1800.0f));
    uint32_t h=ms/50*2654435761u;h^=h>>13;h*=0x5bd1e995u;h^=h>>15;
    if(h%14==0) v=(h>>8)&1?1.0f:0.15f;
    return v;
}
// A one-quad prop drawn self-lit: a copy of its header, vertices and its face
// as a gouraud quad whose corners take one fixed colour (palette entry 0), in
// its own 0x80 slot of the glow block. Retail textures; no dynamic light.
uint32_t glow_copy(unsigned slot,uint32_t mesh,uint32_t level) {
    constexpr uint32_t slot_bytes=0x80,slots=2;
    if(!security_glow_copy) {
        if(psx_mod_replay_active()) return 0;
        const uint32_t base=psx_mod_alloc_guest_memory(slot_bytes*slots,16);
        if(!base) return 0;
        security_glow_copy=base;security_glow_copy_end=base+slot_bytes*slots;
    }
    const uint32_t base=security_glow_copy+slot*slot_bytes,verts=base+0x18,palette=verts+32,list=palette+4;
    const uint32_t vp=psx_mod_read_word(mesh+0x10),src=psx_mod_read_word(mesh+0x14);
    for(uint32_t i=0;i<0x18;i+=4) psx_mod_write_word(base+i,psx_mod_read_word(mesh+i));
    psx_mod_write_word(base+0x10,verts);psx_mod_write_word(base+0x14,list);
    for(uint32_t i=0;i<32;i+=4) psx_mod_write_word(verts+i,psx_mod_read_word(vp+i));
    psx_mod_write_word(palette,level*0x010101u);
    psx_mod_write_word(list,0x0001003cu);
    for(uint32_t i=0;i<16;i+=4) psx_mod_write_word(list+4+i,psx_mod_read_word(src+4+i));
    psx_mod_write_word(list+20,0);psx_mod_write_word(list+24,0xff);
    ++glow_swaps;
    return base;
}
uint32_t swap(uint32_t mesh) {
    if(input_modernized() && !ttk_world_classic()) {
        // Keypads at DNTTK_KEYPAD_GLOW (255: twice the neutral 128, the most
        // a vertex colour gives); the power sign breathes 170..255.
        if(listed(keypad_meshes,mesh)) return glow_copy(0,mesh,keypad_level);
        if(listed(sign_meshes,mesh)) return glow_copy(1,mesh,170+(uint32_t)(85*power_pulse()));
    }
    if(!duke_look() || !monitor_mesh(mesh) || !allocate()) return 0;
    last_template=mesh;
    write_mesh(mesh);
    ++swaps;
    return security_tv_copy;
}

// ---- props: TV bounds and lights (game thread) ----------------------------
// Keypads: type 477's one-quad panel (256x192, first corner (-128, 96, 0)),
// on the control console by LEVEL00's door and on walls by power signs.
// The lightning power sign (type 481 in LEVEL00): one 256x260 quad, first
// corner (-128, 130, 0).
bool sign_mesh(uint32_t mesh) {
    if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) return false;
    if(psx_mod_read_byte(mesh+6)!=4) return false;
    const uint32_t vp=psx_mod_read_word(mesh+0x10),list=psx_mod_read_word(mesh+0x14);
    if(vp<0x80010000u || vp>0x801fff00u || list<0x80010000u || list>0x801fff00u) return false;
    return psx_mod_read_word(vp)==0x0082ff80u && psx_mod_read_half(vp+4)==0 && psx_mod_read_word(list)==0x0001002cu &&
           psx_mod_read_word(vp+16)==0xff7e0080u;   // third corner (128, -130)
}
bool keypad_mesh(uint32_t mesh) {
    if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) return false;
    if(psx_mod_read_byte(mesh+6)!=4) return false;
    const uint32_t vp=psx_mod_read_word(mesh+0x10),list=psx_mod_read_word(mesh+0x14);
    if(vp<0x80010000u || vp>0x801fff00u || list<0x80010000u || list>0x801fff00u) return false;
    return psx_mod_read_word(vp)==0x0060ff80u && psx_mod_read_half(vp+4)==0 && psx_mod_read_word(list)==0x0001002cu &&
           psx_mod_read_word(vp+16)==0xffa00080u;   // third corner (128, -96)
}
struct Emitter {double p[3];float rgb[3];float strength,radius;bool tv,pulse;};
std::vector<Emitter> emitters;
std::vector<uint32_t> monitor_props;
uint32_t scan_level=~0u,scan_frame=0,frames=0;
int bounds_look=-1;          // the look the monitor props' bounds were last set for
uint64_t bounds_writes=0;
// DNTTK_SECURITY_LIGHT="strength radius" (developer tuning of the TV light).
float tv_strength=0.9f,tv_radius=1800;
float power_strength=1.8f,power_radius=1200;   // DNTTK_POWER_LIGHT="strength radius"
void scan_props() {
    emitters.clear();monitor_props.clear();
    unsigned found=0;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    std::vector<uint32_t> seen;
    struct Place {double p[3];double yaw;uint32_t mesh;};
    std::vector<Place> keypads,signs;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3)) continue;
            if(std::find(seen.begin(),seen.end(),o)!=seen.end()) continue;
            seen.push_back(o);
            if(psx_mod_read_byte(o+0x14)!=1) continue;
            const uint32_t mesh=psx_mod_read_word(o+0x44);
            const bool tv=monitor_mesh(mesh),sign=!tv && sign_mesh(mesh);
            // A keypad counts when the game makes it usable (subtype +0x34).
            const bool keypad=!tv && !sign && keypad_mesh(mesh) && psx_mod_read_byte(o+0x34)!=0;
            if(!tv && !sign && !keypad) continue;
            Place pl{};pl.mesh=mesh;for(int k=0;k<3;++k) pl.p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            pl.yaw=(int16_t)psx_mod_read_half(o+0x1c)*(2*3.14159265358979/4096);
            if(keypad) {keypads.push_back(pl);continue;}
            if(sign) {signs.push_back(pl);continue;}
            monitor_props.push_back(o);
            // Model to world: yaw +0x1c (4096 a turn, 0 = +Z, 1024 = +X).
            const double ca=std::cos(pl.yaw),sa=std::sin(pl.yaw);
            Emitter e{};const double c[3]={-420*sa,0,-420*ca};   // 420 out in front of the screen (model -z)
            e.rgb[0]=0.92f;e.rgb[1]=0.95f;e.rgb[2]=0.66f;e.strength=tv_strength;e.radius=tv_radius;e.tv=true;
            for(int k=0;k<3;++k) e.p[k]=pl.p[k]+c[k];
            emitters.push_back(e);
        }
    }
    for(const Place& k:keypads) if(found<4 && !listed(keypad_meshes,k.mesh)) keypad_meshes[found++].store(k.mesh,std::memory_order_relaxed);
    for(unsigned i=found;i<4;++i) keypad_meshes[i].store(0,std::memory_order_relaxed);
    // A power sign glows when a usable keypad is right by it (the power
    // switch); it throws a pulsing electric light on the wall in front of it.
    unsigned signs_found=0;
    for(const Place& g:signs) for(const Place& k:keypads) {
        double d=0;for(int i=0;i<3;++i) d+=(g.p[i]-k.p[i])*(g.p[i]-k.p[i]);
        if(d>600.0*600.0) continue;
        if(signs_found<4 && !listed(sign_meshes,g.mesh)) sign_meshes[signs_found++].store(g.mesh,std::memory_order_relaxed);
        const double ca=std::cos(g.yaw),sa=std::sin(g.yaw);
        Emitter e{};const double c[3]={-260*sa,110,-260*ca};  // in front of the panel (model -z), between sign and keypad
        for(int i=0;i<3;++i) e.p[i]=g.p[i]+c[i];
        e.rgb[0]=0.38f;e.rgb[1]=0.62f;e.rgb[2]=1.0f;e.strength=power_strength;e.radius=power_radius;e.pulse=true;   // electric blue-white
        emitters.push_back(e);
        break;
    }
    for(unsigned i=signs_found;i<4;++i) sign_meshes[i].store(0,std::memory_order_relaxed);
}
// The game culls a prop by the bounds it keeps (+0x4c: min x,y,z, a fourth
// half, max x,y,z, radius at +0x5a, copied from its mesh). A monitor's are
// TTK's 400x500 panel, so the larger TV left the view early. In the Duke 3D
// look they are the TV's; in the TTK look the mesh's own again.
void apply_bounds(bool duke) {
    for(uint32_t o:monitor_props) {
        uint32_t w[4];
        if(duke) {
            const int radius=(int)std::lround(std::sqrt(double(W*W+H*H+F*F)));
            w[0]=uint32_t(uint16_t(-W))|uint32_t(uint16_t(-H))<<16;w[1]=uint32_t(uint16_t(F))|uint32_t(uint16_t(W))<<16;
            w[2]=uint32_t(uint16_t(W))|uint32_t(uint16_t(H))<<16;w[3]=uint32_t(uint16_t(radius))<<16;
        } else {
            const uint32_t b=psx_mod_read_word(psx_mod_read_word(o+0x44)+8);
            for(int i=0;i<4;++i) w[i]=psx_mod_read_word(b+4*i);
        }
        for(int i=0;i<4;++i) if(psx_mod_read_word(o+0x4c+4*i)!=w[i]) {psx_mod_write_word(o+0x4c+4*i,w[i]);++bounds_writes;}
    }
}
// Scene composition, live frames only: find the TVs and consoles once a level
// (and every 300 frames), and keep the monitors' bounds matching the look.
void composition_hook(CPUState*,uint32_t) {
    if(psx_mod_replay_active()) return;
    ++frames;
    const uint32_t level=psx_mod_read_word(0x800be570u);
    const bool modern=input_modernized();
    if(level!=scan_level || frames-scan_frame>300) {scan_level=level;scan_frame=frames;scan_props();bounds_look=-1;}
    static const bool bounds_off=[]{const char* t=std::getenv("DNTTK_SECURITY_BOUNDS");return t && t[0]=='0';}();   // developer A/B
    const int look=modern && duke_look() && !bounds_off?1:0;
    if(look!=bounds_look || frames%60==0) {apply_bounds(look==1);bounds_look=look;}
}
unsigned lights(PSXModLight* out,unsigned room) {
    if(!room || !input_modernized() || emitters.empty()) return 0;
    const bool duke=duke_look();
    const uint32_t pl=players+psx_mod_read_word(player_index)%2*0x8a4u;
    const double duke_p[3]={(double)(int32_t)psx_mod_read_word(pl+4),(double)(int32_t)psx_mod_read_word(pl+8),(double)(int32_t)psx_mod_read_word(pl+12)};
    const float flick=flicker_level()/(float)screen_base;   // the wall light follows the flicker, not the level
    struct Near {double d;const Emitter* e;};
    std::vector<Near> near;
    for(const Emitter& e:emitters) {
        if(e.tv && !duke) continue;
        const double dx=e.p[0]-duke_p[0],dy=e.p[1]-duke_p[1],dz=e.p[2]-duke_p[2],d=dx*dx+dy*dy+dz*dz;
        near.push_back({d,&e});   // no range cut: a lit screen lights its wall seen from afar too
    }
    std::sort(near.begin(),near.end(),[](const Near& a,const Near& b){return a.d<b.d;});
    unsigned n=0;
    for(const Near& q:near) {
        if(n>=room || n>=6) break;
        const Emitter& e=*q.e;const float k=e.strength*(e.tv?flick:e.pulse?0.45f+0.55f*power_pulse():1.0f);
        PSXModLight& l=out[n++];
        l.x=(float)e.p[0];l.y=(float)e.p[1];l.z=(float)e.p[2];l.radius=e.radius;
        l.r=e.rgb[0]*k;l.g=e.rgb[1]*k;l.b=e.rgb[2]*k;l.flags=0;
    }
    lamps_published+=n;
    if(static const bool trace=std::getenv("DNTTK_SECURITY_TRACE")!=nullptr;trace && frames%120==0)
        std::fprintf(stderr,"[TTK security] emitters %zu monitors %zu room %u lights %u swaps %llu glow %llu bounds writes %llu\n",
                     emitters.size(),monitor_props.size(),room,n,(unsigned long long)swaps,(unsigned long long)glow_swaps,(unsigned long long)bounds_writes);
    return n;
}
}  // namespace

const char* security_monitor_debug_json() {
    static char b[256];
    std::snprintf(b,sizeof b,"{\"duke_look\":%s,\"pack\":%s,\"swaps\":%llu,\"tvs\":%zu,\"lamps\":%llu,\"copy\":\"%08x\",\"template\":\"%08x\"}",
                  duke_look()?"true":"false",pack.ok?"true":"false",(unsigned long long)swaps,emitters.size(),(unsigned long long)lamps_published,security_tv_copy,last_template);
    return b;
}
}  // namespace ttk

// The camera frame for the present pass: piece i of the frame and record
// light while a security camera view is up, in 1600x1200 picture units (the
// 4:3 picture). 0 when there is no piece i or nothing to draw.
extern "C" int ttk_security_overlay_piece(int i,const uint32_t** px,int* w,int* h,int* x,int* y) {
    using namespace ttk;
    if(!px || !w || !h || !x || !y || i<0 || !duke_look()) return 0;
    if(psx_mod_read_half(game_mode)!=1) return 0;
    const uint32_t index=psx_mod_read_word(player_index);
    if(index>1 || psx_mod_read_half(0x800c3c4cu)) return 0;
    if(!(psx_mod_read_word(players+index*0x8a4u+0x224)&0x20000000u)) return 0;
    if((size_t)i>=pack.pieces.size()) return 0;
    const Piece& p=pack.pieces[(size_t)i];
    if(p.kind==1 && ((uint32_t)SDL_GetTicks()/133)%2) return 0;   // the record light blinks
    *px=p.argb.data();*w=p.w;*h=p.h;*x=p.x;*y=p.y;
    return 1;
}
extern "C" int ttk_security_overlay_count(void) {return ttk::pack_ok()?(int)ttk::pack.pieces.size():0;}

PSX_MOD_CONSTRUCTOR(register_ttk_security_monitor) {
    if(const char* t=std::getenv("DNTTK_SECURITY_LIGHT")) std::sscanf(t,"%f %f",&ttk::tv_strength,&ttk::tv_radius);
    if(const char* t=std::getenv("DNTTK_SCREEN_LEVEL")) ttk::screen_base=(uint32_t)std::clamp(std::atoi(t),64,251);
    if(const char* t=std::getenv("DNTTK_POWER_LIGHT")) std::sscanf(t,"%f %f",&ttk::power_strength,&ttk::power_radius);
    if(const char* t=std::getenv("DNTTK_KEYPAD_GLOW")) ttk::keypad_level=(uint32_t)std::clamp(std::atoi(t),64,255);
    psx_mod_register_function_entry_plugin("ttk.security.monitor",0x80026164u,ttk::composition_hook);
    ttk::security_tv_swap=ttk::swap;
    ttk::security_tv_lights=ttk::lights;
    ttk::security_tv_paint_bank=[](uint16_t* bank) {
        if(!ttk::pack_ok()) return;
        for(int y=0;y<ttk::pack.ah;++y)
            std::memcpy(bank+size_t(y)*1024,ttk::pack.atlas.data()+size_t(y)*256,256*sizeof(uint16_t));
    };
}
