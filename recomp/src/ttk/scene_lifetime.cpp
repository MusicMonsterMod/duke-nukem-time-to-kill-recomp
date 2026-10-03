// SLUS-00583 scene lifetime: retire leftover scene lists BEFORE heap reuse.
// This implements the no-callback list release at 8001C83C/8001C6AC.
// It never suppresses an object update or repairs an object's handler index.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include <array>
#include <vector>
#include <cstdio>
extern "C" { extern int g_precise_mode,g_ls_mode,g_psx_call_bail; }
namespace ttk {
namespace {
constexpr uint32_t pool=0x800ce418, nodes=1024, stride=12;
constexpr uint32_t free_head=0x800ce408, free_count=0x800d141a;
constexpr uint32_t heads[]={0x800c5694,0x800c5698};
struct Guard {uint32_t address,size;const char* digest;};
const Guard guards[]={
    {0x8002b9f4u,0xbcu,"0ded3b1a4b711cd8fca908e92bf34644d3b3565e887008956fc3e3713a8eee46"},
    {0x8001c83cu,0x84u,"5f5dc9cea05a4e94f5fde1c8c0199c87c01c0ae3115cc712b8bd3df04fbcf53d"},
    {0x8001c6acu,0x28u,"7a7c38780ebd980a40c7bb34ecd87eb3228692285c5d2395a5c0a34e858d3ac2"},
    {0x8001c5ccu,0x64u,"2a01283fb7b50d7648a78391932fd4ba58af7efe55eaafefe395b75d310588b9"},
};
bool node_index(uint32_t p,unsigned& i) {
    if(p<pool || p>=pool+nodes*stride || (p-pool)%stride)return false;
    i=(p-pool)/stride;return true;
}
void reset_hook(CPUState* cpu,uint32_t address) {
    if(address!=0x8002b9f4 || g_precise_mode || g_ls_mode || g_psx_call_bail || cpu->gpr[4]!=1)return;
    const uint32_t ra=cpu->gpr[31];
    bool caller=false;
    for(uint32_t r:{0x8001b510u,0x8001b8a0u,0x800222a8u,0x80025474u,0x800270d0u,0x80028594u})caller|=ra==r;
    if(!caller || psx_mod_read_word(ra-8)!=0x0c00ae7d)return;
    if(!psx_mod_read_word(heads[0]) && !psx_mod_read_word(heads[1]))return;
    static thread_local std::array<std::vector<uint32_t>,4> expected;
    if(!code_identity(guards,expected))return;
    // Validate the complete free chain and both lists before changing any RAM.
    // Other lists can own the remaining nodes; shared/cyclic/foreign nodes fail.
    std::array<bool,nodes> seen{};
    uint32_t free=psx_mod_read_word(free_head);
    unsigned count=0;
    for(uint32_t p=free;p;p=psx_mod_read_word(p+4)) {
        unsigned i;
        if(!node_index(p,i) || seen[i] || psx_mod_read_word(p+8))return;
        seen[i]=true;++count;
    }
    if(count!=psx_mod_read_half(free_count))return;
    std::vector<uint32_t> release;
    for(uint32_t head:heads) {
        uint32_t first=psx_mod_read_word(head),previous=0,last=0;
        for(uint32_t p=first;p;p=psx_mod_read_word(p+4)) {
            unsigned i;
            if(!node_index(p,i) || seen[i] || !psx_mod_read_word(p+8))return;
            if(previous && psx_mod_read_word(p)!=previous)return;
            seen[i]=true;release.push_back(p);previous=last=p;
        }
        if(first && psx_mod_read_word(first)!=last)return;
    }
    if(release.empty())return;
    for(uint32_t p:release) {
        psx_mod_write_word(p+8,0);
        psx_mod_write_word(p+4,free);
        free=p;++count;
    }
    psx_mod_write_word(free_head,free);
    psx_mod_write_half(free_count,count);
    for(uint32_t head:heads)psx_mod_write_word(head,0);
    std::fprintf(stderr,"[TTK scene] Released %zu leftover scene-list nodes before heap reset (caller %08x)\n",release.size(),ra);
}
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_scene_lifetime) {
    psx_mod_register_function_entry_plugin("ttk.scene.lifetime",0x8002b9f4,ttk::reset_hook);
}
