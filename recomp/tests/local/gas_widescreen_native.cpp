// D08A27: drive the registered hook against the owned executable.
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
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){ram[a&0x1fffff]=v;ram[(a+1)&0x1fffff]=v>>8;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0x1fffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){hooks[a]=cb;return 1;}
extern "C" int32_t psx_mod_widescreen_x_margin(void){return margin;}
namespace ttk {uint64_t input_host_frame(){static uint64_t f=0;return ++f;} bool input_modernized(){return modern;}}

static void packet() {
    for(unsigned i=0;i<40;++i)ram[0x100000+i]=uint8_t(17+i);
    ram[0x100003]=9;ram[0x100007]=0x2e;
    for(unsigned o:{8u,24u}) {psx_mod_write_half(0x80100000+o,uint16_t(-256));psx_mod_write_half(0x80100008+o,256);}
}
int main(int argc,char** argv) {
    assert(argc==2);
    std::ifstream file(argv[1],std::ios::binary);
    std::vector<unsigned char> exe((std::istreambuf_iterator<char>(file)),{});
    assert(exe.size()>0x800 && !std::memcmp(exe.data(),"PS-X EXE",8));
    uint32_t base;std::memcpy(&base,exe.data()+0x18,4);
    std::memcpy(ram+(base&0x1fffff),exe.data()+0x800,exe.size()-0x800);
    psx_mod_write_word(0x800c27bc,1);
    CPUState cpu{};cpu.gpr[31]=0x8002ffc0;cpu.gpr[19]=5;cpu.gpr[5]=0x80100000;
    auto invoke=[&]{hooks.at(0x8002bc18)(&cpu,0x8002bc18);};
    for(int m:{0,85,192,256,512}) {
        margin=m;packet();uint8_t expected[40];std::memcpy(expected,ram+0x100000,40);
        for(unsigned o:{8u,24u}) {
            uint16_t l=uint16_t(-256-m),r=uint16_t(256+m);
            std::memcpy(expected+o,&l,2);std::memcpy(expected+o+8,&r,2);
        }
        invoke();assert(!std::memcmp(expected,ram+0x100000,40));
        invoke();assert(!std::memcmp(expected,ram+0x100000,40));
    }
    margin=85;
    auto unchanged=[&]{uint8_t before[40];std::memcpy(before,ram+0x100000,40);invoke();assert(!std::memcmp(before,ram+0x100000,40));};
    packet();modern=false;unchanged();modern=true;
    for(int n:{0,2}) {psx_mod_write_word(0x800c27bc,n);unchanged();}psx_mod_write_word(0x800c27bc,1);
    cpu.gpr[31]=0x80038bb4;unchanged();cpu.gpr[31]=0x8002ffc0;
    cpu.gpr[19]=4;unchanged();cpu.gpr[19]=5;
    for(int m:{-1,513}) {margin=m;unchanged();}margin=85;
    for(unsigned o:{3u,7u,8u,16u,24u,32u}) {packet();ram[0x100000+o]^=1;unchanged();}
    for(uint32_t p:{0x80000000u,0x801fffdcu,0x80100001u}) {packet();cpu.gpr[5]=p;unchanged();}cpu.gpr[5]=0x80100000;
    for(uint32_t a:{0x8002fc38u,0x800354ccu}) {
        packet();uint32_t w=psx_mod_read_word(a);psx_mod_write_word(a,w^1);++g_dirty_ram_code_gen;unchanged();
        psx_mod_write_word(a,w);++g_dirty_ram_code_gen;invoke();
        assert((int16_t)psx_mod_read_half(0x80100008)==-341);
    }
    std::puts("PASS gas packet coverage, byte preservation, identity and exclusion gates");
}
