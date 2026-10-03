// D11B near-clip math: GTE-exact projection, safety classification and DPCS.
#include "near_clip.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
static uint8_t ram[0x200000];
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t,uint32_t){return 0x801e0000;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){ram[a&0x1fffff]=v;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0x1fffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t,PSXModFunctionEntryCallback){return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_replay_active(void){return 0;}
namespace ttk {bool frame_trace_on(){return false;} void frame_trace_account(uint32_t,long){}}
namespace ttk {bool first_person_view_live(){return false;} bool widescreen_near_clip_live(){return false;} bool first_person_duke_drawing(){return false;} bool first_person_actor_drawing(){return false;} bool first_person_weapon_drawing(){return false;} uint64_t input_host_frame(){return 0;}}
static int16_t sx(uint32_t p){return (int16_t)p;} static int16_t sy(uint32_t p){return (int16_t)(p>>16);}
int main() {
    ttk::NearGte g{};
    for(int i=0;i<3;++i) g.r[i][i]=4096;
    g.h=256;g.fc[0]=16*200;g.fc[1]=0;g.fc[2]=16*50;
    // Ordinary vertex: q = 256/1000 in 16.16, SX = floor(100*q) = 25, SY = 12.
    auto a=ttk::near_project(g,100,50,1000);
    assert(a.safe && a.sz==1000 && sx(a.sxy)==25 && sy(a.sxy)==12);
    // Divide saturation (2*SZ <= H) and a vertex behind the eye are unsafe.
    assert(!ttk::near_project(g,10,10,128).safe);
    assert(!ttk::near_project(g,10,10,-500).safe);
    assert(ttk::near_project(g,10,10,-500).sz==0);
    assert(ttk::near_project(g,10,10,129).safe);
    // Far off to the side: exact GTE value saturates at -0x400/0x3ff, so unsafe.
    auto side=ttk::near_project(g,6000,0,1000);
    assert(!side.safe && sx(side.sxy)==0x3ff);
    // Translation and view position.
    g.tr[2]=2000;auto t=ttk::near_project(g,0,0,0);
    assert(t.safe && t.sz==2000 && t.view[2]==2000);
    // DPCS: IR0 0 keeps the color, 4096 reaches the far color (FC/16), code byte kept.
    assert(ttk::near_dpcs(g,0x34102030u,0)==0x34102030u);
    const uint32_t f=ttk::near_dpcs(g,0x34102030u,4096);
    assert((f&0xff)==200 && ((f>>8)&0xff)==0 && ((f>>16)&0xff)==50 && (f>>24)==0x34);
    std::puts("ttk-near-test PASS");
}
