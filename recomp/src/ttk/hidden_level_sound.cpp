// D61B: the hidden levels get the shipped sounds they lack. See
// documentation/561-d61b-hidden-levels.md.
//
// A level's sound bank (SND.DAT) loads through 0x8006a508(level): 0x8006a2c0
// copies the sound table into the 0x2000-byte buffer *0x800c39c4 and makes its
// group offsets RAM addresses, 0x8006a3ac(vh) turns the VH's first 140 sample
// sizes into SPU addresses from 0x1010 (0x800da730), and 0x8006a3fc uploads
// the VB to SPU 0x1010. A sound id is group << 12 | index; its 16-byte entry's
// bytes 0 * 16 + 1 name the sample, byte 4 bit 0x80 disables it (0x80068b20
// then returns none and nothing plays). Nothing reads the VH's program or tone
// blocks.
//
// True Grit (4), Modern 4 (13), the animation test (14), the empty room (30)
// and the sound test (31) have no gadget group 0x1c (4: every entry
// disabled), so the jetpack, the gadget pickups and Duke's gadget damage
// sounds are silent; some other ids are disabled and the menu group is a set
// of prototype blips. At the entry of 0x8006a3ac, for those levels only,
// this writes the entries of build-local/ttk-sound.pack (build_ttk_sound.py,
// from the owned disc: a shipped level of the same era's entries, each naming
// a sample index past the level's own), their sample sizes into the VH and
// the samples into SPU RAM past the level's VB, below the end of the largest
// shipped bank. The game then builds the addresses, uploads its own VB and
// plays, positions and stops the sounds itself. The hidden levels' own
// variants are untouched.
//
// Modernized only. No pack, Vanilla, another level, an unauthenticated
// loader or a table or VH that is not the disc's: nothing changes. The patch
// lives in guest RAM and SPU RAM, so savestates carry it. Developer:
// DNTTK_HIDDEN_SOUND=0 off, DNTTK_SOUND_PACK another pack.
#include "ttk_register.h"
#include "code_identity.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "psx_sha256.h"
#include "spu.h"
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
#include <mutex>
#include <string>
#include <vector>

extern "C" uint8_t* g_psx_ram;

namespace {
constexpr uint32_t table_word=0x800c39c4u,table_buffer=0x2000u,entries_at=0xf4u;
constexpr uint32_t spu_bank=0x1010u,spu_shipped_end=0x7aa40u;   // the largest shipped bank's end
constexpr uint32_t sample_limit=140u,gadget=0x1cu,groups=29u;

struct Entry {uint32_t id;uint8_t bytes[16];};
struct Sample {uint32_t index;std::vector<uint8_t> bytes;};
struct Level {
    uint32_t level=0,donor=0,table_size=0,vb_size=0,first=0;
    uint8_t digest[32]={};
    std::vector<Entry> entries;
    std::vector<Sample> samples;
};
std::vector<Level> pack;
std::once_flag pack_once;

uint32_t rd32(const std::vector<uint8_t>& b,size_t o) {
    return uint32_t(b[o])|uint32_t(b[o+1])<<8|uint32_t(b[o+2])<<16|uint32_t(b[o+3])<<24;
}

void load_pack() {
    std::string path;
    if(const char* p=std::getenv("DNTTK_SOUND_PACK")) path=p;
    else if(auto base=SDL_GetBasePath()) {
        path=std::string(base)+"ttk-sound.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    } else return;
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>4*1024*1024) return;
    std::vector<uint8_t> b(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(b.data()),b.size())) return;
    if(std::memcmp(b.data(),"TTKSND1\0",8) || rd32(b,8)!=1) return;
    const uint32_t count=rd32(b,12);
    std::vector<Level> out;
    size_t o=16;
    for(uint32_t l=0;l<count;++l) {
        Level v;
        if(o+8+32+16>b.size()) return;
        v.level=rd32(b,o);v.donor=rd32(b,o+4);o+=8;
        std::memcpy(v.digest,&b[o],32);o+=32;
        v.table_size=rd32(b,o);v.vb_size=rd32(b,o+4);v.first=rd32(b,o+8);
        const uint32_t n=rd32(b,o+12);o+=16;
        if(v.table_size<entries_at || v.table_size>table_buffer || n>256 || o+n*20+4>b.size()) return;
        for(uint32_t i=0;i<n;++i) {
            Entry e;e.id=rd32(b,o);std::memcpy(e.bytes,&b[o+4],16);o+=20;
            if((e.id>>12)>=groups || (e.id&0xfff)>0x7ff) return;
            v.entries.push_back(e);
        }
        const uint32_t s=rd32(b,o);o+=4;
        uint32_t next=v.first;
        for(uint32_t i=0;i<s;++i) {
            if(o+8>b.size()) return;
            Sample m;m.index=rd32(b,o);const uint32_t size=rd32(b,o+4);o+=8;
            if(m.index!=next++ || m.index>=sample_limit || !size || (size&15) || size>0x10000 || o+size>b.size()) return;
            m.bytes.assign(b.begin()+o,b.begin()+o+size);o+=size;
            v.samples.push_back(std::move(m));
        }
        out.push_back(std::move(v));
    }
    if(o!=b.size()) return;
    pack=std::move(out);
    std::fprintf(stderr,"[TTK sound] hidden-level sound pack %s (%zu levels)\n",path.c_str(),pack.size());
}

bool loader_ok() {
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={
        {0x8006a2c0u,0x354u,"b58d0212804cebdc02f12db2d41bdcf5e29304f7b05803cd2a202df137fecb66"},
        {0x80068b20u,0x70u,"61776b9fe8f6fa5cbc564b0b5efe4670415fc3c6a14bb8e6296ade8c7f368c32"},
        {0x8006b3acu,0x14u,"5ef794a0c238faf277133023c1392ed767fecc2b20730e5b2e7a34bb407e9a2d"},
    };
    static std::array<std::vector<uint32_t>,3> words;
    return ttk::code_identity(guards,words,psx_mod_read_word,g_psx_ram);
}

bool table_is_disc(const Level& v,uint32_t table) {
    std::vector<uint8_t> bytes;
    bytes.reserve(v.table_size);
    for(uint32_t a=0;a<12;++a) bytes.push_back(psx_mod_read_byte(table+a));
    for(uint32_t a=entries_at;a<v.table_size;++a) bytes.push_back(psx_mod_read_byte(table+a));
    uint8_t digest[32];
    psx_sha256_compute(bytes.data(),bytes.size(),digest);
    return !std::memcmp(digest,v.digest,32);
}

// The game's VB upload (0x8006a3fc, right after this hook) moves whole DMA
// blocks, so it writes past the VB's end into the first added sample (True
// Grit: 48 bytes; that sample looped as a steady beep under the pause menu,
// user 2026-11-26). The samples are written again at Duke's first update
// after the bank load, once the upload is done.
struct Pending {uint32_t at;const std::vector<uint8_t>* bytes;};
std::vector<Pending> pending;uint32_t pending_level=~0u;
void after_upload(CPUState* cpu,uint32_t) {
    if(pending.empty() || cpu->gpr[4]!=0x800d7198u) return;
    if(psx_mod_read_word(0x800c39c8u)==pending_level)
        for(const auto& p:pending) spu_ram_poke(p.at,p.bytes->data(),uint32_t(p.bytes->size()));
    pending.clear();
}
void bank_addresses(CPUState* cpu,uint32_t) {
    static const bool trace=std::getenv("DNTTK_HIDDEN_SOUND_TRACE")!=nullptr;
    if(trace) std::fprintf(stderr,"[TTK sound] bank load ra=%08x level=%u vh=%08x modernized=%d\n",
                           cpu->gpr[31],cpu->gpr[16],cpu->gpr[4],ttk::input_modernized());
    if(cpu->gpr[31]!=0x8006a5b0u || !ttk::input_modernized()) return;
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_HIDDEN_SOUND");return !t || std::strcmp(t,"0");}();
    if(!enabled) return;
    std::call_once(pack_once,load_pack);
    const uint32_t level=cpu->gpr[16],vh=cpu->gpr[4];
    const Level* v=nullptr;
    for(const auto& l:pack) if(l.level==level) v=&l;
    if(!v || !loader_ok()) return;
    const uint32_t table=psx_mod_read_word(table_word);
    if(table<0x80010000u || table>0x801fe000u || vh<0x80010000u || vh>0x801f0000u) return;
    if(psx_mod_read_word(table)!=v->table_size || psx_mod_read_word(table+8)!=v->vb_size || !table_is_disc(*v,table)) {
        std::fprintf(stderr,"[TTK sound] level %u: sound table is not the disc's, left alone\n",level);
        return;
    }
    const uint32_t vh_size=psx_mod_read_word(table+4);
    if(psx_mod_read_word(vh)!=0x56414270u || vh_size<0x800u || vh_size>0x10000u) return;   // "pBAV"
    const uint32_t sizes=vh+vh_size-0x200u;
    uint32_t own=0;
    for(uint32_t k=1;k<=v->first;++k) own+=uint32_t(psx_mod_read_half(sizes+2*k))*8u;
    if(psx_mod_read_half(sizes) || own!=v->vb_size) return;
    uint32_t end=spu_bank+v->vb_size;
    for(const auto& s:v->samples) end+=uint32_t(s.bytes.size());
    if(end>spu_shipped_end) return;
    // A missing gadget group goes after the level's own entries.
    uint32_t gadget_new=0;
    for(const auto& e:v->entries) if((e.id>>12)==gadget) ++gadget_new;
    const bool make_gadget=!psx_mod_read_word(table+12+8*gadget);
    if(make_gadget && v->table_size+16*gadget_new>table_buffer) return;
    uint32_t written=0,made=0;
    for(const auto& e:v->entries) {
        const uint32_t g=e.id>>12,i=e.id&0x7ff;
        uint32_t at;
        if(g==gadget && make_gadget) {
            if(i!=made) return;   // the pack lists the whole group in order
            at=table+v->table_size+16*made++;
        } else {
            if(i>=psx_mod_read_word(table+12+8*g)) continue;
            at=psx_mod_read_word(table+16+8*g)+16*i;
        }
        for(uint32_t b=0;b<16;++b) psx_mod_write_byte(at+b,e.bytes[b]);
        ++written;
    }
    if(make_gadget && made) {
        psx_mod_write_word(table+12+8*gadget,made);
        psx_mod_write_word(table+16+8*gadget,table+v->table_size);
    }
    // Sizes first; each sample lands where 0x8006a3ac will put its address.
    for(const auto& s:v->samples) psx_mod_write_half(sizes+2*(s.index+1),uint16_t(s.bytes.size()/8));
    pending.clear();
    for(const auto& s:v->samples) {
        uint32_t at=spu_bank;
        for(uint32_t k=0;k<=s.index;++k) at+=uint32_t(psx_mod_read_half(sizes+2*k))*8u;
        spu_ram_poke(at,s.bytes.data(),uint32_t(s.bytes.size()));
        pending.push_back({at,&s.bytes});
    }
    pending_level=level;
    std::fprintf(stderr,"[TTK sound] level %u: %u entries and %zu samples from level %u (D61B)\n",
                 level,written,v->samples.size(),v->donor);
}
}

TTK_REGISTER(register_ttk_hidden_level_sound, TTK_RANK_HIDDEN_LEVEL_SOUND) {
    psx_mod_register_function_entry_plugin("ttk.hidden.level.sound",0x8006a3acu,bank_addresses);
    psx_mod_register_function_entry_plugin("ttk.hidden.level.sound",0x8005a210u,after_upload);
}
