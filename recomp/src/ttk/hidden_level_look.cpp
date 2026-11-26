// D61B: textures for the hidden levels' untextured models. See
// documentation/561-d61b-hidden-levels.md.
//
// Models of True Grit (4) and Modern 4 (13) load with faces whose texture
// the level's set does not have: the model loader leaves CLUT 0xffff on page
// 0x20 and the face draws the frame buffer (True Grit's striped totem wings,
// Modern 4's magenta bar fronts and crates). build-local/ttk-look.pack
// (build_ttk_look.py, from the owned disc and recomp/assets/hidden-levels)
// names, per level, the model, its faces and the texture each takes: a block
// copied from the same model in a shipped level, a texture the level already
// has, or the project's own painting, with the VRAM blocks and CLUTs to put in
// free places of the level's texture VRAM. Once the level is in play (Duke's
// update, 0x8005a210) and a listed face still names CLUT 0xffff, this uploads
// the level's blocks with the GPU's own CPU-to-VRAM copy (GP0 0xa0) and gives
// those faces the CLUT, page and UVs (shifted onto the block). Faces already
// textured, and models whose vertex and face counts are not the disc's, are
// left alone.
//
// Modernized only; no pack, Vanilla or another level: nothing changes. VRAM
// and RAM are both in a savestate. Models load with each level, so this runs
// again after every load. Developer: DNTTK_HIDDEN_LOOK=0 off.
#include "ttk_register.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "gpu.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace {
constexpr uint32_t player=0x800d7198u,level_word=0x800be570u,models_word=0x800ddb18u;

struct Fix {uint16_t model=0,verts=0,total=0,clut=0;uint8_t page=0,du=0,dv=0,every=0;std::vector<uint16_t> faces;std::vector<uint16_t> uvs;};   // every: 0 listed, 1 all untextured, 2 explicit UVs (4 a face)
struct Upload {uint16_t x=0,y=0,w=0,h=0;std::vector<uint16_t> words;};
struct Level {uint32_t level=0;std::vector<Fix> fixes;std::vector<Upload> uploads;};
std::vector<Level> pack;
std::once_flag pack_once;
uint64_t fixed_faces=0,uploads_sent=0;


void load_pack() {
    std::string path;
    if(const char* p=std::getenv("DNTTK_LOOK_PACK")) path=p;
    else if(auto base=SDL_GetBasePath()) {
        path=std::string(base)+"ttk-look.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    } else return;
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>256*1024) return;
    std::vector<uint8_t> b(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(b.data()),b.size())) return;
    size_t o=0;
    auto need=[&](size_t n){return o+n<=b.size();};
    auto u16=[&](){const uint16_t v=uint16_t(b[o]|b[o+1]<<8);o+=2;return v;};
    auto u32=[&](){const uint32_t v=uint32_t(b[o])|uint32_t(b[o+1])<<8|uint32_t(b[o+2])<<16|uint32_t(b[o+3])<<24;o+=4;return v;};
    if(std::memcmp(b.data(),"TTKLOOK2",8)) return;
    o=8;
    if(u32()!=2) return;
    const uint32_t levels=u32();
    if(levels>8) return;
    std::vector<Level> out(levels);
    for(auto& l:out) {
        if(!need(12)) return;
        l.level=u32();const uint32_t fixes=u32(),uploads=u32();
        if(fixes>64 || uploads>64) return;
        for(uint32_t i=0;i<fixes;++i) {
            if(!need(16)) return;
            Fix f;f.model=u16();f.verts=u16();f.total=u16();f.clut=u16();
            f.page=b[o];f.du=b[o+1];f.dv=b[o+2];f.every=b[o+3];o+=4;
            const uint32_t n=u32();
            if(n>256 || !need(2*n) || f.page>0x1f || !f.model || f.model>=4096) return;
            for(uint32_t k=0;k<n;++k) f.faces.push_back(u16());
            if(f.every==2) {
                if(!need(8*n)) return;
                for(uint32_t k=0;k<4*n;++k) f.uvs.push_back(u16());
            } else if(f.every>2) return;
            l.fixes.push_back(std::move(f));
        }
        for(uint32_t i=0;i<uploads;++i) {
            if(!need(8)) return;
            Upload u;u.x=u16();u.y=u16();u.w=u16();u.h=u16();
            const size_t n=size_t(u.w)*u.h;
            if(!u.w || !u.h || u.x<512 || u.x+u.w>1024 || u.y+u.h>512 || !need(2*n)) return;
            for(size_t k=0;k<n;++k) u.words.push_back(u16());
            l.uploads.push_back(std::move(u));
        }
    }
    if(o!=b.size()) return;
    pack=std::move(out);
    std::fprintf(stderr,"[TTK look] hidden-level look pack %s (%zu levels)\n",path.c_str(),pack.size());
}

bool ram(uint32_t a,uint32_t n) {return a>=0x80010000u && a+n<=0x80200000u && !(a&3);}

uint32_t record_size(uint32_t type) {
    switch(type) {
        case 0x20: case 0x28: return 8;
        case 0x30: case 0x38: return 12;
        case 0x24: case 0x2c: return 16;
        case 0x34: case 0x3c: return 20;
        default: return 0;
    }
}
// The model's textured faces (0x24 FT3, 0x2c FT4) in polygon-list order, and
// its vertex count; false when the mesh or its list is not readable.
bool textured_faces(uint16_t id,std::vector<std::pair<uint32_t,uint32_t>>& out,unsigned& verts) {
    out.clear();
    const uint32_t table=psx_mod_read_word(models_word);
    if(!ram(table,4*4096)) return false;
    const uint32_t mesh=psx_mod_read_word(table+4*id);
    if(!ram(mesh,0x18)) return false;
    verts=psx_mod_read_byte(mesh+6);
    uint32_t cur=psx_mod_read_word(mesh+0x14);
    for(unsigned guard=0;guard<64;++guard) {
        if(!ram(cur,4)) return false;
        const uint32_t header=psx_mod_read_word(cur);cur+=4;
        const uint32_t type=header&0xffff,n=header>>16,size=record_size(type);
        if(type==0xff) return true;
        if(!size || !n || n>1024 || !ram(cur,n*size)) return false;
        if(type==0x24 || type==0x2c) for(uint32_t k=0;k<n;++k) out.push_back({cur+size*k,type});
        cur+=n*size;
    }
    return false;
}

void upload(const Upload& u) {
    gpu_write_gp0(0xa0000000u);
    gpu_write_gp0(uint32_t(u.y)<<16|u.x);
    gpu_write_gp0(uint32_t(u.h)<<16|u.w);
    const size_t n=u.words.size();
    for(size_t i=0;i<n;i+=2) gpu_write_gp0(uint32_t(u.words[i])|(i+1<n ? uint32_t(u.words[i+1])<<16 : 0u));
    ++uploads_sent;
}

uint32_t shift(uint32_t uv,const Fix& f) {
    return (uv&~0xffffu) | (((uv>>8)&0xff)+f.dv)<<8 | ((uv&0xff)+f.du);
}

// True Grit's totem poles (type 81, model 87). Their object +0x44 is the mesh
// (the draw's mesh and the collision box list at once): header +4's low byte
// is the box count, +8 points at the boxes (16 bytes each: min x, y, z, the
// radius across, max x, y, z, the corner radius; shipped props use up to 3).
// The disc gives the pole one box, its whole bounds: x -339..229, z -853..847,
// wings and beak included, so Duke met a wall under the wings and stopped well
// short of the pole (user 2026-11-26). The pole is a hexagonal prism (corners
// (+-228, +-132) and (0, +-264) from the floor to y -629, (+-156, +-90) and
// (0, +-180) up to -1364; the wings, from -1285 up, and the head clear Duke's
// 1280). Two boxes per section cover each hexagon (core and points), so
// nothing stands out more than a few units past the faces. The boxes live in
// mod memory (the draw never reads +8; the mesh header's radius, +2, stays),
// rewritten while in use, since a savestate restores mod memory as saved.
constexpr int16_t pole_boxes[4][8]={
    {-228,-629,-132,263,228,0,132,682},
    {-114,-629,-198,228,114,0,198,669},
    {-156,-1364,-90,180,156,-629,90,1376},
    {-78,-1364,-135,156,78,-629,135,1373},
};
uint32_t pole_array=0;uint64_t pole_writes=0;
void write_pole_boxes() {
    for(int b=0;b<4;++b) for(int k=0;k<8;++k) psx_mod_write_half(pole_array+16*b+2*k,uint16_t(pole_boxes[b][k]));
}
void totem_collision() {
    const uint32_t table=psx_mod_read_word(models_word);
    if(!ram(table,4*4096)) return;
    const uint32_t mesh=psx_mod_read_word(table+4*87);
    if(!ram(mesh,0x18) || psx_mod_read_byte(mesh+6)!=81) return;
    const uint32_t boxes=psx_mod_read_word(mesh+8);
    if(pole_array && boxes==pole_array) {   // ours: keep the contents right
        write_pole_boxes();
        return;
    }
    if(!ram(boxes,16) || (psx_mod_read_half(mesh+4)&0xff)!=1) return;
    // The disc's single box (or D61B's first in-place pole box): replace.
    const int16_t z0=int16_t(psx_mod_read_half(boxes+4));
    if(int16_t(psx_mod_read_half(boxes+2))!=-1671 || (z0!=-853 && z0!=-264)) return;
    if(!pole_array) {
        if(psx_mod_replay_active()) return;
        pole_array=psx_mod_alloc_guest_memory(64,16);
        if(!pole_array) return;
    }
    write_pole_boxes();
    psx_mod_write_word(mesh+8,pole_array);
    psx_mod_write_half(mesh+4,uint16_t((psx_mod_read_half(mesh+4)&0xff00u)|4u));
    ++pole_writes;
}

void look_update(CPUState* cpu,uint32_t) {
    if(cpu->gpr[4]!=player || !ttk::input_modernized()) return;
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_HIDDEN_LOOK");return !t || std::strcmp(t,"0");}();
    if(!enabled) return;
    if(psx_mod_read_word(level_word)==4) totem_collision();
    std::call_once(pack_once,load_pack);
    if(pack.empty()) return;
    const uint32_t level=psx_mod_read_word(level_word);
    const Level* l=nullptr;
    for(const auto& p:pack) if(p.level==level) l=&p;
    if(!l) return;
    // The faces to change this time: listed (or, every=1, all), still CLUT 0xffff.
    struct Edit {uint32_t at,type;const Fix* fix;int slot;};
    std::vector<Edit> edits;
    auto claimed=[&](uint32_t at){for(const auto& e:edits) if(e.at==at) return true;return false;};
    std::vector<std::pair<uint32_t,uint32_t>> faces;
    for(const auto& f:l->fixes) {
        unsigned verts=0;
        if(!textured_faces(f.model,faces,verts) || verts!=f.verts || faces.size()!=f.total) continue;
        auto open=[&](const std::pair<uint32_t,uint32_t>& face){return (psx_mod_read_word(face.first+4)>>16)==0xffffu && !claimed(face.first);};
        if(f.every==2) {   // explicit UVs: until the face names this fix's CLUT and page
            for(size_t k=0;k<f.faces.size();++k) {
                const uint16_t i=f.faces[k];
                if(i>=faces.size()) continue;
                const uint32_t at=faces[i].first;
                const bool done=(psx_mod_read_word(at+4)>>16)==f.clut && ((psx_mod_read_word(at+8)>>16)&0x1fu)==f.page;
                if(!done) edits.push_back({at,faces[i].second,&f,int(k)});
                else edits.push_back({at,0,nullptr,-1});   // claimed, nothing to write
            }
        } else if(f.every) {for(const auto& face:faces) if(open(face)) edits.push_back({face.first,face.second,&f,-1});}
        else for(uint16_t i:f.faces) if(i<faces.size() && open(faces[i])) edits.push_back({faces[i].first,faces[i].second,&f,-1});
    }
    size_t todo=0;
    for(const auto& e:edits) todo+=e.fix!=nullptr;
    if(!todo) return;
    for(const auto& u:l->uploads) upload(u);
    for(const auto& e:edits) {
        if(!e.fix) continue;
        const Fix& f=*e.fix;
        if(e.slot>=0) {
            const uint16_t* uv=&f.uvs[4*size_t(e.slot)];
            const uint32_t w2=psx_mod_read_word(e.at+8),w3=psx_mod_read_word(e.at+12);
            psx_mod_write_word(e.at+4,uint32_t(uv[0])|uint32_t(f.clut)<<16);
            psx_mod_write_word(e.at+8,uint32_t(uv[1])|((((w2>>16)&~0x1fu)|f.page)<<16));
            psx_mod_write_word(e.at+12,uint32_t(uv[2])|(e.type==0x2c ? uint32_t(uv[3])<<16 : w3&0xffff0000u));
            ++fixed_faces;continue;
        }
        const uint32_t w1=psx_mod_read_word(e.at+4),w2=psx_mod_read_word(e.at+8),w3=psx_mod_read_word(e.at+12);
        const uint32_t page=((w2>>16)&~0x1fu)|f.page;
        psx_mod_write_word(e.at+4,(shift(w1,f)&0xffffu)|uint32_t(f.clut)<<16);
        psx_mod_write_word(e.at+8,(shift(w2,f)&0xffffu)|page<<16);
        const uint32_t uv3=e.type==0x2c ? (shift(w3>>16,f)&0xffffu) : (w3>>16);
        psx_mod_write_word(e.at+12,(shift(w3,f)&0xffffu)|uv3<<16);
        ++fixed_faces;
    }
    std::fprintf(stderr,"[TTK look] level %u: %zu faces textured, %zu VRAM blocks (D61B)\n",level,todo,l->uploads.size());
}

// The water (D61B). The renderer 0x8002ef90 textures its tiles from the
// material table *0x800c597c: materials 0, 1 and 4 are water, and the word at
// +0x20 of each 36-byte record points to six halves (corner UV 0, CLUT, UV 1,
// page, UV 2, UV 3) that the game rewrites every tick from the water's frame
// list. A shipped level's frames are 32x32 caustic tiles (16 of them on page
// 0x1f). True Grit's and Modern 4's are an earlier set: 64x64 boxes of which
// only the top-left 32x32 holds a caustic tile (the rest of the box is flat
// light grey in VRAM, drawn as a bright sheet), and some frames are all zero
// (the water then sampled the frame buffer for a tick: the flashing stripes
// and flat slabs). At the renderer's entry, in those two levels only, a 64x64
// frame is narrowed to its caustic tile and an all-zero frame keeps the last
// good one, so the water draws as the shipped levels' water. The game's frame
// list and timing are untouched. Developer: DNTTK_HIDDEN_WATER=0 off.
constexpr uint32_t materials_word=0x800c597cu;
constexpr unsigned water_materials[3]={0,1,4};
uint16_t water_last[3][6];bool water_have[3];uint32_t water_level=~0u;uint64_t water_loads=~0ull;
uint64_t water_narrowed=0,water_held=0;
void water_fix(CPUState*,uint32_t) {
    if(!ttk::input_modernized()) return;
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_HIDDEN_WATER");return !t || std::strcmp(t,"0");}();
    if(!enabled) return;
    const uint32_t level=psx_mod_read_word(level_word);
    if(level!=4 && level!=13) return;
    const uint64_t loads=psx_mod_savestate_loads();
    if(level!=water_level || loads!=water_loads) {water_level=level;water_loads=loads;std::memset(water_have,0,sizeof water_have);}
    const uint32_t table=psx_mod_read_word(materials_word);
    if(!ram(table,36*5)) return;
    for(unsigned i=0;i<3;++i) {
        const uint32_t d=psx_mod_read_word(table+36*water_materials[i]+0x20);
        if(!ram(d,12)) continue;
        uint16_t h[6];bool zero=true;
        for(unsigned k=0;k<6;++k) {h[k]=psx_mod_read_half(d+2*k);zero&=!h[k];}
        if(zero) {
            if(!water_have[i]) continue;
            for(unsigned k=0;k<6;++k) psx_mod_write_half(d+2*k,water_last[i][k]);
            ++water_held;continue;
        }
        const unsigned u0=h[0]&0xff,v0=h[0]>>8,u1=h[2]&0xff,v2=h[4]>>8;
        if(u1>u0+31 && v2>v0+31 && u0+31<256 && v0+31<256) {
            h[2]=uint16_t(v0<<8|(u0+31));h[4]=uint16_t((v0+31)<<8|u0);h[5]=uint16_t((v0+31)<<8|(u0+31));
            for(unsigned k:{2u,4u,5u}) psx_mod_write_half(d+2*k,h[k]);
            ++water_narrowed;
        }
        std::memcpy(water_last[i],h,sizeof h);water_have[i]=true;
    }
}
}

TTK_REGISTER(register_ttk_hidden_level_look, TTK_RANK_HIDDEN_LEVEL_LOOK) {
    psx_mod_register_function_entry_plugin("ttk.hidden.level.look",0x8005a210u,look_update);
    psx_mod_register_function_entry_plugin("ttk.hidden.level.look",0x8002ef90u,water_fix);
}
