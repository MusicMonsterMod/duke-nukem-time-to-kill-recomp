// D08K1: runtime animation override packs (documentation/184-d08k1-animation-pipeline.md).
//
// The level loader (0x80024fdc) keeps the group's frame table at [0x800d8324]
// (FRM, 8-byte frames) and command table at [0x800d8320] (ROT, 8-byte
// commands); the clip table (ANM, 2048 x {u16 first frame, u16 count}) is a
// fixed buffer at 0x800d8328. Every reader (0x80058c70..0x8005a0e4) indexes
// those globals afresh, so a clip is replaced by copying both tables into an
// enhancement buffer, appending the replacement frames and commands, pointing
// the clip's ANM record at them and repointing the two globals. Nothing else
// changes; the original tables stay where they were.
//
// A pack entry names a clip by the SHA-256 of its original ANM record, frame
// records and commands (ttk_anim.py clip_digest); it applies only where the
// loaded clip matches, in any group. An entry with group 0xffff is an extra
// clip (D08K4 crouch walk): appended under a host id, never bound to an
// original clip; anim_extra_clip() finds it. Modernized only: Vanilla never applies a
// pack and restores the originals if a loaded state carries an override.
//
// Packs: DNTTK_ANIM_PACK (diagnostics; comma-separated) or ttk-anim.pack next
// to the executable. Without a pack nothing is allocated or touched.
//
// Savestates: the buffer lives in the GPU-DMA aperture, reserved at the first
// application. Allocation order can differ between saving and loading sessions.
// The header keeps the original pointers and ANM records, so a state
// whose buffer does not survive restore is put back on the originals using
// validated metadata from the immutable saved allocation, not cleared live DMA.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "pc_input.h"
#include "psx_sha256.h"
#include "anim_override.h"
#include "anim_save_header.h"
#include "mod_memory.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
extern "C" { extern int g_precise_mode,g_ls_mode,g_psx_call_bail; }
namespace ttk {
namespace {
constexpr uint32_t frm_global=0x800d8324, rot_global=0x800d8320, anm_table=0x800d8328, anm_clips=2048;
constexpr uint32_t magic=0x414b5454 /* "TTKA" */, version=1, max_entries=64;
constexpr uint32_t buffer_bytes=320*1024;
// header: magic, version, original FRM, original ROT, frames, commands,
// entry count, then per entry {u16 clip, u16 original first, u16 original count, u16 0}.
constexpr uint32_t header_bytes=0x20+8*max_entries;
struct Guard {uint32_t address,size;const char* digest;};
const Guard guards[]={
    {0x800251c0u,0x70u,"94962d5835ec49f7a55104ac920e8c9f04899a04896f1e25fe9a0af5f7584442"},   // FRM/ROT globals set by the loader
    {0x80058c70u,0x1140u,"25c96c6448cfeb3f7baa45c1b78f22f6cd4fbbf71637b59aa2eae3b3bbc162af"}, // frame/command readers
};
struct Frame {uint16_t events,ticks,count;};
struct Entry {uint16_t group,clip;uint8_t digest[32];std::vector<Frame> frames;std::vector<uint32_t> words;};
std::vector<Entry> entries;
bool loaded;
uint32_t buffer;          // this process's buffer (0 until the first application)
uint32_t loads_seen;
bool invalid_saved_tables;
unsigned applied,restored,adopted,refused;

bool main_ram(uint32_t a) {return a>=0x80010000u && a<0x80200000u;}
bool read_pack(const std::string& path) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<12 || file.tellg()>4*1024*1024) return false;
    std::vector<uint8_t> p(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(p.data()),p.size()) || std::memcmp(p.data(),"TTKANIM1",8)) return false;
    auto u16=[&](size_t o){return uint16_t(p[o]|p[o+1]<<8);};
    auto u32=[&](size_t o){return uint32_t(p[o])|uint32_t(p[o+1])<<8|uint32_t(p[o+2])<<16|uint32_t(p[o+3])<<24;};
    const uint32_t count=u32(8);
    size_t o=12;
    std::vector<Entry> out;
    for(uint32_t n=0;n<count;++n) {
        if(o+40>p.size()) return false;
        Entry e;e.group=u16(o);e.clip=u16(o+2);std::memcpy(e.digest,&p[o+4],32);
        const unsigned frames=u16(o+36);o+=40;
        if(!frames || e.clip>=anm_clips || o+8ull*frames>p.size()) return false;
        size_t commands=0;
        for(unsigned f=0;f<frames;++f,o+=8) {e.frames.push_back({u16(o),u16(o+2),u16(o+4)});commands+=u16(o+4);}
        if(o+8*commands>p.size()) return false;
        for(size_t c=0;c<2*commands;++c,o+=4) e.words.push_back(u32(o));
        out.push_back(std::move(e));
    }
    if(o!=p.size() || out.size()>max_entries) return false;
    for(auto& e:out) entries.push_back(std::move(e));
    std::fprintf(stderr,"[TTK anim] Loaded %u clip override(s) from %s\n",count,path.c_str());
    return true;
}
void load_packs() {
    if(loaded) return;
    loaded=true;
    std::vector<std::string> paths;
    if(const char* env=std::getenv("DNTTK_ANIM_PACK")) {
        std::string s=env;
        for(size_t a=0;a<=s.size();) {
            size_t b=s.find(',',a);if(b==std::string::npos)b=s.size();
            if(b>a) paths.push_back(s.substr(a,b-a));
            a=b+1;
        }
    } else if(auto base=SDL_GetBasePath()) {
        paths.push_back(std::string(base)+"ttk-anim.pack");
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    for(auto& path:paths)
        if(!read_pack(path) && std::getenv("DNTTK_ANIM_PACK"))
            std::fprintf(stderr,"[TTK anim] Ignored unreadable or malformed pack %s\n",path.c_str());
}
// The loaded clip's digest, as ttk_anim.py clip_digest.
bool clip_digest(uint32_t frm,uint32_t rot,uint32_t clip,uint8_t out[32]) {
    const uint32_t rec=psx_mod_read_word(anm_table+4*clip);
    const uint32_t first=rec&0xffff,count=rec>>16;
    if(!count || first==0xffff) return false;
    std::vector<uint8_t> bytes;
    auto put16=[&](uint32_t v){bytes.push_back(uint8_t(v));bytes.push_back(uint8_t(v>>8));};
    put16(first);put16(count);
    for(uint32_t i=first;i<first+count;++i) {
        const uint32_t w0=psx_mod_read_word(frm+8*i),w1=psx_mod_read_word(frm+8*i+4);
        put16(w0&0xffff);put16(w0>>16);put16(w1>>16);
        for(uint32_t c=w1&0xffff;c<(w1&0xffff)+(w1>>16);++c)
            for(uint32_t k=0;k<8;k+=4) {const uint32_t w=psx_mod_read_word(rot+8*c+k);for(int b=0;b<4;++b)bytes.push_back(uint8_t(w>>(8*b)));}
    }
    psx_sha256_compute(bytes.data(),bytes.size(),out);
    return true;
}
// Frames and commands the clip table references (unused records: count 0 or first 0xffff).
bool table_sizes(uint32_t frm,uint32_t& frames,uint32_t& commands) {
    frames=commands=0;
    for(uint32_t c=0;c<anm_clips;++c) {
        const uint32_t rec=psx_mod_read_word(anm_table+4*c),first=rec&0xffff,count=rec>>16;
        if(count && first!=0xffff && first+count>frames) frames=first+count;
    }
    if(!frames || frames>16384) return false;
    for(uint32_t i=0;i<frames;++i) {
        const uint32_t w1=psx_mod_read_word(frm+8*i+4),end=(w1&0xffff)+(w1>>16);
        if(end>commands) commands=end;
    }
    return commands && commands<65536;
}
// Header at a buffer address: valid when the globals point at its own tables.
bool header_valid(uint32_t b) {
    if(!b || psx_mod_read_word(b)!=magic || psx_mod_read_word(b+4)!=version) return false;
    const uint32_t frames=psx_mod_read_word(b+16),n=psx_mod_read_word(b+24);
    return main_ram(psx_mod_read_word(b+8)) && main_ram(psx_mod_read_word(b+12)) && n<=max_entries &&
           psx_mod_read_word(frm_global)==b+header_bytes &&
           psx_mod_read_word(rot_global)==b+header_bytes+8*(frames+psx_mod_read_word(b+28));
}
// The player's four animation channels (player+0x60, 0x14 each: u16 animation,
// ticks left, previous frame, current frame, ...) hold absolute frame indices.
// A channel inside a moved clip follows it, frame for frame (clamped), so a
// clip already playing when the tables change keeps playing the same clip.
constexpr uint32_t player=0x800d7198u;
void rebase_channels(uint32_t from_first,uint32_t from_count,uint32_t to_first,uint32_t to_count) {
    for(uint32_t ch=0;ch<4;++ch)
        for(uint32_t field:{4u,6u}) {
            const uint32_t a=player+0x60+0x14*ch+field;
            const int16_t v=int16_t(psx_mod_read_half(a));
            if(v>=0 && uint32_t(v)>=from_first && uint32_t(v)<from_first+from_count) {
                uint32_t k=uint32_t(v)-from_first;if(k>=to_count)k=to_count-1;
                psx_mod_write_half(a,uint16_t(to_first+k));
            }
        }
}
uint32_t header_of(uint32_t frm) {return frm>=header_bytes ? frm-header_bytes : 0;}
void restore(uint32_t b) {
    const uint32_t n=psx_mod_read_word(b+24);
    for(uint32_t k=0;k<n;++k) {
        const uint32_t e=b+0x20+8*k;
        const uint32_t clip=psx_mod_read_half(e);
        if(clip&0x8000) continue;  // an extra clip: no original record
        const uint32_t now=psx_mod_read_word(anm_table+4*clip);
        rebase_channels(now&0xffff,now>>16,psx_mod_read_half(e+2),psx_mod_read_half(e+4));
        psx_mod_write_word(anm_table+4*clip,psx_mod_read_half(e+2)|uint32_t(psx_mod_read_half(e+4))<<16);
    }
    psx_mod_write_word(frm_global,psx_mod_read_word(b+8));
    psx_mod_write_word(rot_global,psx_mod_read_word(b+12));
    ++restored;
}
// Allocation order may differ between attract/new-game and load-first sessions.
// The runtime deliberately clears incompatible DMA packets; recover only our
// saved header and rebuild in this process's buffer, never resurrect packet data.
bool restore_saved(uint32_t b,uint32_t frm,uint32_t rot) {
    std::array<uint8_t,header_bytes> bytes{};
    anim_save::Header h{};
    if(!psx_mod_memory_saved_gpu_allocation(b,buffer_bytes,bytes.data(),bytes.size()) ||
       !anim_save::decode(bytes.data(),bytes.size(),b,frm,rot,h)) return false;
    for(uint32_t k=0;k<h.count;++k) {
        const auto& r=h.records[k];
        if(r.clip&0x8000) continue;
        const uint32_t now=psx_mod_read_word(anm_table+4*r.clip);
        rebase_channels(now&0xffff,now>>16,r.first,r.count);
        psx_mod_write_word(anm_table+4*r.clip,r.first|uint32_t(r.count)<<16);
    }
    psx_mod_write_word(frm_global,h.frm);
    psx_mod_write_word(rot_global,h.rot);
    ++restored;
    return true;
}
void copy_words(uint32_t dst,uint32_t src,uint32_t bytes) {for(uint32_t i=0;i<bytes;i+=4)psx_mod_write_word(dst+i,psx_mod_read_word(src+i));}
void apply(uint32_t frm,uint32_t rot) {
    uint32_t frames,commands;
    if(!table_sizes(frm,frames,commands)) {++refused;return;}
    std::vector<const Entry*> use;
    for(auto& e:entries) {
        uint8_t d[32];
        if(clip_digest(frm,rot,e.clip,d) && !std::memcmp(d,e.digest,32)) {
            bool dup=false;for(auto* u:use)dup|=u->clip==e.clip;
            if(!dup) use.push_back(&e);
        }
    }
    for(auto& e:entries) if(e.group==0xffff) use.push_back(&e);
    if(use.empty() || use.size()>max_entries) return;
    uint32_t add_frames=0,add_commands=0;
    for(auto* e:use) {add_frames+=uint32_t(e->frames.size());add_commands+=uint32_t(e->words.size()/2);}
    if(frames+add_frames>65535 || commands+add_commands>65535 ||
       header_bytes+8*(frames+add_frames)+8*(commands+add_commands)>buffer_bytes) {++refused;return;}
    if(!buffer) buffer=psx_mod_alloc_gpu_dma_memory(buffer_bytes,16);
    if(!buffer) {++refused;return;}
    const uint32_t nf=buffer+header_bytes, nr=nf+8*(frames+add_frames);
    copy_words(nf,frm,8*frames);
    copy_words(nr,rot,8*commands);
    psx_mod_write_word(buffer,magic);psx_mod_write_word(buffer+4,version);
    psx_mod_write_word(buffer+8,frm);psx_mod_write_word(buffer+12,rot);
    psx_mod_write_word(buffer+16,frames);psx_mod_write_word(buffer+20,commands);
    psx_mod_write_word(buffer+24,uint32_t(use.size()));psx_mod_write_word(buffer+28,add_frames);
    uint32_t f=frames,c=commands;
    for(size_t k=0;k<use.size();++k) {
        const Entry& e=*use[k];
        const bool extra=e.group==0xffff;
        const uint32_t rec=extra ? 0 : psx_mod_read_word(anm_table+4*e.clip);
        const uint32_t h=buffer+0x20+8*uint32_t(k);
        // Header: replaced clips keep their original record; extras (clip | 0x8000) their new one.
        psx_mod_write_half(h,extra ? uint16_t(0x8000|e.clip) : e.clip);
        psx_mod_write_half(h+2,extra ? uint16_t(f) : uint16_t(rec&0xffff));
        psx_mod_write_half(h+4,extra ? uint16_t(e.frames.size()) : uint16_t(rec>>16));psx_mod_write_half(h+6,0);
        size_t w=0;
        const uint32_t first=f;
        for(const Frame& fr:e.frames) {
            psx_mod_write_word(nf+8*f,fr.events|uint32_t(fr.ticks)<<16);
            psx_mod_write_word(nf+8*f+4,c|uint32_t(fr.count)<<16);
            for(unsigned i=0;i<fr.count;++i,++c,w+=2) {psx_mod_write_word(nr+8*c,e.words[w]);psx_mod_write_word(nr+8*c+4,e.words[w+1]);}
            ++f;
        }
        if(extra) continue;
        psx_mod_write_word(anm_table+4*e.clip,first|uint32_t(e.frames.size())<<16);
        rebase_channels(rec&0xffff,rec>>16,first,uint32_t(e.frames.size()));
    }
    psx_mod_write_word(frm_global,nf);
    psx_mod_write_word(rot_global,nr);
    ++applied;
    std::fprintf(stderr,"[TTK anim] Applied %zu clip override(s) (frames %u+%u, commands %u+%u)\n",use.size(),frames,add_frames,commands,add_commands);
}
}
}
bool ttk::anim_extra_clip(unsigned id,uint32_t& first,uint32_t& count) {
    const uint32_t b=header_of(psx_mod_read_word(frm_global));
    if(!header_valid(b)) return false;
    const uint32_t n=psx_mod_read_word(b+24);
    for(uint32_t k=0;k<n;++k) {
        const uint32_t e=b+0x20+8*k;
        if(psx_mod_read_half(e)==(0x8000|id)) {first=psx_mod_read_half(e+2);count=psx_mod_read_half(e+4);return count!=0;}
    }
    return false;
}
namespace ttk {
namespace {
void hook(CPUState* cpu,uint32_t) {
    if(g_precise_mode || g_ls_mode || g_psx_call_bail || psx_mod_replay_active() || cpu->gpr[4]!=0x800d7198u) return;
    load_packs();
    const uint32_t frm=psx_mod_read_word(frm_global),rot=psx_mod_read_word(rot_global);
    const bool modern=input_modernized() && !entries.empty();
    const uint32_t loads=psx_mod_savestate_loads();
    const bool after_load=loads!=loads_seen;
    loads_seen=loads;
    if(after_load) invalid_saved_tables=false;
    if(main_ram(frm) && main_ram(rot)) {
        invalid_saved_tables=false;
        if(!modern) return;
        static thread_local std::array<std::vector<uint32_t>,2> expected;
        if(!code_identity(guards,expected)) {++refused;return;}
        apply(frm,rot);
        return;
    }
    // The globals point outside main RAM: our tables (this process's or a
    // loaded state's). Adopt a valid buffer, or put the originals back.
    const uint32_t b=header_of(frm);
    if(invalid_saved_tables) return;
    if(after_load) {
        static thread_local std::array<std::vector<uint32_t>,2> expected;
        if(!code_identity(guards,expected)) {++refused;return;}
        const bool valid=header_valid(b);
        if(restore_saved(b,frm,rot)) {
            // A full restore can adopt the saved allocation. With a mismatch,
            // keep our own allocation (or allocate on apply); never claim b.
            if(!buffer && valid) {buffer=b;++adopted;}
            if(!valid) std::fprintf(stderr,"[TTK anim] Recovered saved animation metadata after DMA layout mismatch\n");
            if(modern) apply(psx_mod_read_word(frm_global),psx_mod_read_word(rot_global));
            return;
        }
        invalid_saved_tables=true;
        ++refused;
        std::fprintf(stderr,"[TTK anim] Cannot recover invalid saved animation metadata\n");
        return; // never fall back to interpreting rejected metadata on later ticks
    }
    if(!header_valid(b)) return;
    if(!buffer && b) {buffer=b;++adopted;}
    // A savestate can carry an older version of the host clips. Rebuild from
    // the current pack after restoring its original table references.
    if(!modern || b!=buffer) {restore(b);if(modern && main_ram(psx_mod_read_word(frm_global))) apply(psx_mod_read_word(frm_global),psx_mod_read_word(rot_global));}
}
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_anim_override) {
    // Player update (a0 = player), a gameplay-only point after the level's tables are loaded.
    psx_mod_register_function_entry_plugin("ttk.anim.override",0x8005a210,ttk::hook);
}
