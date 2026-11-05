// D17AO1: Duke Hill's sky behind painted skylights (sky_borrow.h). See
// documentation/398-d17ao1-borrowed-sky.md.
//
// The composition 0x80026164 calls 0x8001fc44(0) (ra 0x800265c4) and then the
// sky routine 0x800388e4 only when the render requested sky (sky_widescreen
// sky_background forces that request while a painted face was left out); it
// calls 0x8002e850 next (ra 0x800265d4). For that one sky call the routine's
// level words are swapped: the four layer selectors and the fill colour
// (0x800da344..0x800da46c, note 92) take Duke Hill's values and the model
// table pointer 0x800ddb18 points just before a guest copy of Duke Hill's
// four meshes, so its entries for 0x611..0x614 are those meshes. All six
// words are restored before 0x8002e850, so a savestate or any other code never
// sees them; the routine, its timer, matrices and draw calls are unchanged.
// The near-clip host path draws the copy's polygons with host vertices marked
// for the runtime's second texture bank (psx_mod_gpu_host_vertex_foreign):
// Duke Hill's VRAM image from the pack, not the level's own textures. Retail
// packets can never carry that mark (an earlier CLUT bit 15 marker could:
// level 3's own models set it).
//
// The meshes come from build-local/ttk-sky.pack (build_ttk_sky.py, from the
// owned disc). No pack, Vanilla, two players, a level with its own sky or an
// unauthenticated routine: nothing changes. DNTTK_SKY_BORROW=0 (developer)
// turns it off.
#include "sky_borrow.h"
#include "sky_render.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
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

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;

namespace ttk {
namespace {
constexpr uint32_t level_index=0x800be570u,table_pointer=0x800ddb18u,players=0x800c27bcu;
// The routine's level words (note 92): fixed ring, slow band, backdrop, fill
// colour, fast band. Duke Hill's values, read from LEVEL01 on a private run.
constexpr uint32_t sky_words[5]={0x800da344u,0x800da45cu,0x800da464u,0x800da468u,0x800da46cu};
constexpr uint32_t duke_hill_words[5]={0x612u,0x613u,0x611u,0x00661f29u,0x614u};
constexpr uint32_t first_id=0x611u,mesh_count=4;

struct Pack {
    bool ok=false;
    uint32_t ids[mesh_count]={};
    std::vector<uint8_t> meshes[mesh_count];
    std::vector<uint16_t> vram;   // 1024x512, Duke Hill's x 512..1023
};
Pack pack;
std::once_flag pack_once;

uint32_t rd32(const std::vector<uint8_t>& b,size_t o) {return b[o]|b[o+1]<<8|b[o+2]<<16|(uint32_t)b[o+3]<<24;}

// Object mesh records (near_clip.cpp object_record_size).
uint32_t record_size(uint32_t type) {
    switch(type) {
        case 0x20: case 0x28: return 8;
        case 0x30: case 0x38: return 12;
        case 0x24: case 0x2c: return 16;
        case 0x34: case 0x3c: return 20;
        default: return 0;
    }
}
void load_pack() {
    std::string path;
    if(const char* p=std::getenv("DNTTK_SKY_PACK")) path=p;
    else if(auto base=SDL_GetBasePath()) {
        path=std::string(base)+"ttk-sky.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    } else return;
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>1024*1024) return;
    std::vector<uint8_t> b(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(b.data()),b.size())) return;
    if(std::memcmp(b.data(),"TTKSKY1\0",8) || rd32(b,8)!=1 || rd32(b,12)!=mesh_count) return;
    size_t o=16;
    for(uint32_t i=0;i<mesh_count;++i) {
        if(o+8>b.size()) return;
        const uint32_t id=rd32(b,o),n=rd32(b,o+4);o+=8;
        if(id!=first_id+i || n<0x18 || n>0x4000 || o+n>b.size()) return;
        std::vector<uint8_t> m(b.begin()+o,b.begin()+o+n);o+=n;
        // Validate the polygon list.
        const uint32_t list=rd32(m,0x14);
        for(uint32_t at=list;;) {
            if(at+4>n) return;
            const uint32_t header=rd32(m,at),type=header&0xffff,k=header>>16;at+=4;
            if(type==0xff) break;
            const uint32_t size=record_size(type);
            if(!size || !k || at+k*size>n) return;
            at+=k*size;
        }
        pack.ids[i]=id;pack.meshes[i]=std::move(m);
    }
    constexpr size_t a_bytes=448*256*2,b_bytes=512*256*2;
    if(o+a_bytes+b_bytes!=b.size()) return;
    pack.vram.assign(1024*512,0);
    for(int y=0;y<256;++y) {
        std::memcpy(&pack.vram[y*1024+512],&b[o+y*448*2],448*2);
        std::memcpy(&pack.vram[(256+y)*1024+512],&b[o+a_bytes+y*512*2],512*2);
    }
    pack.ok=true;
    std::fprintf(stderr,"[TTK sky] borrowed sky pack %s\n",path.c_str());
}
bool pack_ok() {std::call_once(pack_once,load_pack);return pack.ok;}

uint32_t copy=0;         // guest copy of the meshes (allocated live, never in a worker)
uint32_t mesh_at[mesh_count]={};
bool injected=false;
uint32_t saved[6];
uint64_t injections=0;

bool enabled() {
    static const bool on=[]{const char* t=std::getenv("DNTTK_SKY_BORROW");return !(t && !std::strcmp(t,"0"));}();
    return on;
}
bool own_sky() {
    for(uint32_t a:sky_words) if(psx_mod_read_word(a)&(a==0x800da468u?0xffffffu:0xffffffffu)) return true;
    return false;
}
bool allocate() {
    if(copy) return true;
    if(psx_mod_replay_active()) return false;
    uint32_t bytes=0;
    for(uint32_t i=0;i<mesh_count;++i) {mesh_at[i]=bytes;bytes+=(uint32_t)(pack.meshes[i].size()+3)&~3u;}
    // Table words for ids 0x611..0x614 first, then the meshes.
    copy=psx_mod_alloc_guest_memory(16+bytes,16);
    if(!copy) return false;
    for(auto& m:mesh_at) m+=copy+16;
    sky_borrow_copy=copy;sky_borrow_copy_end=copy+16+bytes;
    return true;
}
// Rewritten on every use: a savestate load restores mod memory to the saved
// image, which may predate the copy.
void write_copy() {
    for(uint32_t i=0;i<mesh_count;++i) {
        const auto& m=pack.meshes[i];const uint32_t at=mesh_at[i];
        psx_mod_write_word(copy+4*i,at);
        for(size_t o=0;o+4<=m.size();o+=4) psx_mod_write_word(at+(uint32_t)o,rd32(m,o));
        for(size_t o=m.size()&~size_t(3);o<m.size();++o) psx_mod_write_byte(at+(uint32_t)o,m[o]);
        psx_mod_write_half(at,(uint16_t)pack.ids[i]);
        psx_mod_write_word(at+0xc,0);
        for(uint32_t f:{0x8u,0x10u,0x14u}) psx_mod_write_word(at+f,at+rd32(m,f));
    }
}
void restore() {
    if(!injected) return;
    for(int i=0;i<5;++i) psx_mod_write_word(sky_words[i],saved[i]);
    psx_mod_write_word(table_pointer,saved[5]);
    injected=false;
}

void hook(CPUState* cpu,uint32_t address) {
    if(address==0x80026164u) {
        restore();
        ++sky_borrow_composition;
        const uint32_t level=psx_mod_read_word(level_index);
        bool painted=false;
        for(const auto& s:painted_skies) painted|=s.level==level;
        static SkyRenderIdentity identity;
        sky_borrow_ready=painted && enabled() && input_modernized() && psx_mod_read_word(players)==1 &&
            pack_ok() && !own_sky() && allocate() &&
            identity.valid(input_host_frame(),g_dirty_ram_code_gen,g_psx_ram);
        sky_borrow_level=level;
        return;
    }
    if(address==0x8001fc44u) {
        if(cpu->gpr[31]!=0x800265c4u || cpu->gpr[4]!=0 || injected || !sky_borrow_wanted() || !copy) return;
        for(int i=0;i<5;++i) saved[i]=psx_mod_read_word(sky_words[i]);
        saved[5]=psx_mod_read_word(table_pointer);
        write_copy();
        for(int i=0;i<5;++i) psx_mod_write_word(sky_words[i],duke_hill_words[i]);
        psx_mod_write_word(table_pointer,copy-4*first_id);
        injected=true;++injections;
        return;
    }
    if(address==0x8002e850u && cpu->gpr[31]==0x800265d4u) restore();
}
}

// Second texture bank for the OpenGL renderer (near_clip marks the borrowed
// meshes' polygons with psx_mod_gpu_host_vertex_foreign).
extern "C" const uint16_t* ttk_foreign_vram(void) {return pack_ok()?pack.vram.data():nullptr;}
}

PSX_MOD_CONSTRUCTOR(register_ttk_sky_borrow) {
    for(uint32_t address:{0x80026164u,0x8001fc44u,0x8002e850u})
        psx_mod_register_function_entry_plugin("ttk.sky.borrow",address,ttk::hook);
}
