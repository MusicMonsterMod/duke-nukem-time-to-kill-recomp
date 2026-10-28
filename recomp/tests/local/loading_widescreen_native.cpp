// D17AD: drive the registered hook against the owned executable.
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
static uint8_t ram[0x1000000];
static std::map<uint32_t,PSXModFunctionEntryCallback> hooks;
static int32_t margin=0;
static bool modern=true;
static const char* wide="16:9";
extern "C" const char* ttk_live_env(const char*){return wide;}
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0xffffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){ram[a&0xffffff]=v;ram[(a+1)&0xffffff]=v>>8;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0xffffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){hooks[a]=cb;return 1;}
extern "C" int32_t psx_mod_widescreen_x_margin(void){return margin;}
namespace ttk {uint64_t input_host_frame(){static uint64_t f=0;return ++f;} bool input_modernized(){return modern;}}

extern "C" uint32_t psx_mod_alloc_gpu_dma_memory(uint32_t n,uint32_t){assert(n==336);return 0x80e00000;}
static constexpr uint32_t q=0x800d24f4;
static void packet() {
    psx_mod_write_word(q,0x090d251c);
    psx_mod_write_word(q+4,0x2c808080);
    psx_mod_write_word(q-8,0xe2084318);
    psx_mod_write_word(q+44,0xe2000000);
    for(unsigned i=0;i<4;++i) {
        psx_mod_write_half(q+8+8*i,uint16_t(i%2?256:-256));
        psx_mod_write_half(q+10+8*i,uint16_t(i/2?120:-120));
        psx_mod_write_word(q+12+8*i,(i%2?255:0)|(i/2?255<<8:0)|0xabcd0000);
    }
}
int main(int argc,char** argv) {
    assert(argc==2);
    std::ifstream file(argv[1],std::ios::binary);
    std::vector<unsigned char> exe((std::istreambuf_iterator<char>(file)),{});
    assert(exe.size()>0x800 && !std::memcmp(exe.data(),"PS-X EXE",8));
    uint32_t base;std::memcpy(&base,exe.data()+0x18,4);
    std::memcpy(ram+(base&0xffffff),exe.data()+0x800,exe.size()-0x800);
    CPUState cpu{};cpu.gpr[31]=0x80022270;cpu.gpr[5]=q-12;cpu.gpr[4]=0x7ff;
    auto invoke=[&]{hooks.at(0x8002bc18)(&cpu,0x8002bc18);};
    for(int m:{1,51,85,192,256,512}) {
        margin=m;packet();invoke();
        const uint32_t l=psx_mod_read_word(q)&0xffffff,r=psx_mod_read_word(l)&0xffffff;
        assert(l>=0xe00000 && l<0xe00140 && r==l+40);
        assert(psx_mod_read_word(r)==0x090d251c);
        for(unsigned o=4;o<40;o+=4) {
            if(o==8||o==16||o==24||o==32) {
                assert((int16_t)psx_mod_read_half(l+o)==(int16_t)psx_mod_read_half(q+o)-512);
                assert((int16_t)psx_mod_read_half(r+o)==(int16_t)psx_mod_read_half(q+o)+512);
                assert(psx_mod_read_half(l+o+2)==psx_mod_read_half(q+o+2));
                assert(psx_mod_read_half(r+o+2)==psx_mod_read_half(q+o+2));
            } else {assert(psx_mod_read_word(l+o)==psx_mod_read_word(q+o));assert(psx_mod_read_word(r+o)==psx_mod_read_word(q+o));}
        }
        invoke();assert((psx_mod_read_word(q)&0xffffff)==l);
    }
    margin=0;wide="auto";packet();invoke();assert(psx_mod_read_word(q)!=0x090d251c);wide="16:9";
    auto unchanged=[&]{std::vector<uint8_t> before(ram,ram+sizeof ram);invoke();assert(!std::memcmp(before.data(),ram,sizeof ram));};
    packet();modern=false;unchanged();modern=true;
    for(int m:{-1,0,513}) {margin=m;unchanged();}margin=85;
    cpu.gpr[31]^=4;unchanged();cpu.gpr[31]^=4;
    cpu.gpr[5]^=4;unchanged();cpu.gpr[5]^=4;
    cpu.gpr[4]=0;unchanged();cpu.gpr[4]=0x7ff;
    for(unsigned o:{0u,3u,7u,8u,10u,12u,13u,16u,24u,32u}) {packet();ram[(q+o)&0xffffff]^=1;unchanged();}
    packet();uint32_t w=psx_mod_read_word(0x800220f4);psx_mod_write_word(0x800220f4,w^1);++g_dirty_ram_code_gen;unchanged();
    psx_mod_write_word(0x800220f4,w);++g_dirty_ram_code_gen;invoke();assert(psx_mod_read_word(q)!=0x090d251c);
    std::puts("PASS loading backdrop tiling, material preservation, chain order, ring reuse and exclusion gates");
}
