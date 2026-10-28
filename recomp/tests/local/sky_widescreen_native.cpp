// D17R: the sky's fill, backdrop and clip rectangle across the widescreen
// margins. Loads the owned EXE (the sky routine identity guard) and drives the
// registered hook with the original call sites. Usage: ttk-sky-test EXE
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <vector>
static uint8_t ram[0x200000];
static std::map<uint32_t,PSXModFunctionEntryCallback> hooks;
static int32_t margin=0;
static bool modern=true;
static uint16_t projection=256;
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){ram[a&0x1fffff]=v;ram[(a+1)&0x1fffff]=v>>8;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0x1fffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){hooks[a]=cb;return 1;}
extern "C" int32_t psx_mod_widescreen_x_margin(void){return margin;}
namespace ttk {uint64_t input_host_frame(){static uint64_t f=0;return ++f;} bool input_modernized(){return modern;}}

static int16_t h(uint32_t a){return (int16_t)psx_mod_read_half(a);}
static void sh(uint32_t a,int v){psx_mod_write_half(a,(uint16_t)(int16_t)v);}
static void call(uint32_t address,uint32_t ra,uint32_t a0,uint32_t a1,uint32_t a2=0) {
    CPUState cpu{};cpu.gte_ctrl[26]=projection;cpu.gpr[31]=ra;cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;
    hooks.at(address)(&cpu,address);
}
constexpr uint32_t packet=0x80100000u,matrix=0x801ffe00u,context=0x800d67a8u,camera=0x800d6eb0u;
static void fill_packet() {
    // F4 from the root rectangle: (-256,-120) (256,-120) (-256,40) (256,40).
    sh(packet+8,-256);sh(packet+0xa,-120);sh(packet+0xc,256);sh(packet+0xe,-120);
    sh(packet+0x10,-256);sh(packet+0x12,40);sh(packet+0x14,256);sh(packet+0x16,40);
}
static void backdrop_matrix() {
    // Yaw 30 degrees: rows (3547,0,-2048) (0,4096,0) (2048,0,3547), eye translation.
    const int m[9]={3547,0,-2048,0,4096,0,2048,0,3547};
    for(int i=0;i<9;++i) sh(matrix+2*i,m[i]);
    psx_mod_write_word(matrix+0x14,19542);
}
static void clip() {sh(context+0x18,-256);sh(context+0x1a,256);sh(context+0x1c,-120);sh(context+0x1e,120);}
static void reset() {fill_packet();backdrop_matrix();clip();}
static void run_all() {
    call(0x8002bc18u,0x80038bb4u,0x7ff,packet);
    call(0x800292a0u,0x80038c68u,camera,matrix);
    call(0x80010000u,0x80038c94u,0,0,context);
}
static bool untouched() {
    return h(packet+8)==-256 && h(packet+0xc)==256 && h(packet+0x10)==-256 && h(packet+0x14)==256 &&
        h(matrix)==3547 && h(matrix+4)==-2048 && h(context+0x18)==-256 && h(context+0x1a)==256;
}

int main(int argc,char** argv) {
    if(argc<2) {std::fprintf(stderr,"usage: %s SLUS_005.83\n",argv[0]);return 2;}
    std::ifstream file(argv[1],std::ios::binary);
    const std::vector<unsigned char> exe((std::istreambuf_iterator<char>(file)),{});
    assert(exe.size()>0x800 && std::memcmp(exe.data(),"PS-X EXE",8)==0);
    uint32_t base=0;std::memcpy(&base,exe.data()+0x18,4);
    std::memcpy(ram+(base&0x1fffff),exe.data()+0x800,exe.size()-0x800);
    assert(hooks.count(0x8002bc18u) && hooks.count(0x800292a0u) && hooks.count(0x80010000u));

    // 4:3 (Vanilla, widescreen off): nothing changes.
    margin=0;reset();run_all();assert(untouched());

    // 16:9: margin 85 native pixels.
    margin=85;reset();run_all();
    assert(h(packet+8)==-341 && h(packet+0xc)==341 && h(packet+0x10)==-341 && h(packet+0x14)==341);
    assert(h(packet+0xa)==-120 && h(packet+0x12)==40 && h(packet+0x16)==40);  // y untouched
    // Row 0 (local X) scaled by 341/256, rounded away from zero; rows 1-2 kept.
    assert(h(matrix)==4725 && h(matrix+2)==0 && h(matrix+4)==-2728);
    assert(h(matrix+6)==0 && h(matrix+8)==4096 && h(matrix+12)==2048 && h(matrix+16)==3547);
    assert(psx_mod_read_word(matrix+0x14)==19542u);
    assert(h(context+0x18)==-341 && h(context+0x1a)==341 && h(context+0x1c)==-120 && h(context+0x1e)==120);
    // Already widened rectangles are not widened twice.
    call(0x8002bc18u,0x80038bb4u,0x7ff,packet);call(0x80010000u,0x80038e44u,0,0,context);
    assert(h(packet+8)==-341 && h(context+0x18)==-341);
    // Every sky draw call widens the clip rectangle.
    for(uint32_t ra:{0x80038c94u,0x80038cf0u,0x80038da8u,0x80038e44u}) {
        clip();call(0x80010000u,ra,0,0,context);assert(h(context+0x18)==-341 && h(context+0x1a)==341);
    }
    // 21:9: margin 192.
    margin=192;reset();run_all();
    assert(h(packet+8)==-448 && h(context+0x1a)==448 && h(matrix)==6208);

    // D14D: actual H, including a replay frame's projection, expands only
    // the partial backdrop. Screen-space fill/clip bounds do not depend on H.
    for(int m:{0,85,192}) for(int hp:{160,191,256,277,386}) {
        margin=m;projection=hp;reset();run_all();
        const int d=hp<256?hp:256;
        assert(h(matrix)==(3547*(256+m)+d-1)/d);
        assert(h(matrix+4)==(-2048*(256+m)+1-d)/d);
        assert(h(matrix+8)==4096 && h(matrix+12)==2048 && h(matrix+16)==3547);
        assert(h(packet+8)==-256-m && h(context+0x1a)==256+m);
    }
    // Invalid H and Vanilla retain the accepted aspect-only correction.
    margin=0;projection=0;reset();run_all();assert(untouched());
    projection=160;modern=false;reset();run_all();assert(untouched());modern=true;
    projection=256;

    // Other callers, cameras and contexts are left alone.
    margin=85;reset();
    call(0x8002bc18u,0x80033db0u,0x7ff,packet);
    call(0x800292a0u,0x80038d7cu,camera,matrix);   // slow cloud layer: full 360 degrees
    call(0x800292a0u,0x80038c68u,0x800d7010u,matrix);
    call(0x80010000u,0x80032288u,0,0,context);
    call(0x80010000u,0x80038c94u,0,0,0x800d6900u);
    assert(untouched());

    // Split screen keeps its own viewports.
    psx_mod_write_word(0x800c27bcu,2);reset();run_all();assert(untouched());
    psx_mod_write_word(0x800c27bcu,1);

    // Changed sky code: no widening.
    const uint32_t word=psx_mod_read_word(0x80038b08u);
    psx_mod_write_word(0x80038b08u,word^1);++g_dirty_ram_code_gen;
    reset();run_all();assert(untouched());
    psx_mod_write_word(0x80038b08u,word);++g_dirty_ram_code_gen;
    reset();run_all();assert(h(packet+8)==-341);

    // The new 4:3 FOV path must retain identity and split-screen guards.
    margin=0;projection=160;
    psx_mod_write_word(0x800c27bcu,2);reset();run_all();assert(untouched());
    psx_mod_write_word(0x800c27bcu,1);
    psx_mod_write_word(0x80038b08u,word^1);++g_dirty_ram_code_gen;
    reset();run_all();assert(untouched());
    psx_mod_write_word(0x80038b08u,word);++g_dirty_ram_code_gen;
    reset();run_all();assert(h(matrix)==5676);
    projection=256;

    // D17R2: sky omitted by the portal walk, with a loaded backdrop. Only
    // the eventual sky return changes; far limits and room data stay intact.
    CPUState cpu{};cpu.gpr[31]=0x8002e6dc;cpu.gpr[4]=camera;cpu.gpr[29]=0x801ffe00;
    const uint32_t flag=cpu.gpr[29]+0x28;
    psx_mod_write_word(0x800da464,1);
    psx_mod_write_word(context+0x74,24576);
    auto background=[&](){hooks.at(0x8002ffec)(&cpu,0x8002ffec);};
    for(int m:{0,85,192}) {
        margin=m;psx_mod_write_word(flag,0);background();
        assert(psx_mod_read_word(flag)==1 && psx_mod_read_word(context+0x74)==24576);
    }
    psx_mod_write_word(flag,0);modern=false;background();assert(!psx_mod_read_word(flag));modern=true;
    psx_mod_write_word(0x800c27bc,2);background();assert(!psx_mod_read_word(flag));
    psx_mod_write_word(0x800c27bc,1);
    cpu.gpr[31]=0x80026278;background();assert(!psx_mod_read_word(flag));cpu.gpr[31]=0x8002e6dc;
    cpu.gpr[4]=camera+0x160;background();assert(!psx_mod_read_word(flag));cpu.gpr[4]=camera;
    cpu.gpr[29]=0x801fffe0;background();assert(!psx_mod_read_word(flag));cpu.gpr[29]=0x801ffe00;
    // Empty sky: never enable one. Each actual sky component can request it.
    const uint32_t assets[]={0x800da468,0x800da464,0x800da344,0x800da45c,0x800da46c};
    for(auto a:assets) psx_mod_write_word(a,0);
    background();assert(!psx_mod_read_word(flag));
    for(auto a:assets) {
        psx_mod_write_word(a,1);background();assert(psx_mod_read_word(flag)==1);
        psx_mod_write_word(a,0);psx_mod_write_word(flag,0);
    }
    psx_mod_write_word(0x800da464,1);
    for(uint32_t a:{0x8002e830u,0x800265b4u,0x80038b08u}) {
        const uint32_t original=psx_mod_read_word(a);
        psx_mod_write_word(a,original^1);++g_dirty_ram_code_gen;
        background();assert(!psx_mod_read_word(flag));
        psx_mod_write_word(a,original);++g_dirty_ram_code_gen;
        background();assert(psx_mod_read_word(flag)==1);psx_mod_write_word(flag,0);
    }
    psx_mod_write_word(flag,7);background();assert(psx_mod_read_word(flag)==7);
    std::puts("sky widescreen: ok");
    return 0;
}
