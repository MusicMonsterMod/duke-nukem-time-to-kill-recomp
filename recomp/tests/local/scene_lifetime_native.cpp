#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
extern "C" {int g_precise_mode=0,g_ls_mode=0,g_psx_call_bail=0;}
static unsigned char ram[0x200000],baseline[0x200000];
static PSXModFunctionEntryCallback callback;
static unsigned writes;
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){ram[a&0x1fffff]=v;++writes;}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){psx_mod_write_byte(a,v);psx_mod_write_byte(a+1,v>>8);}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){psx_mod_write_half(a,v);psx_mod_write_half(a+2,v>>16);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){assert(a==0x8002b9f4);callback=cb;return 1;}
// Execute the owned, byte-authenticated no-callback release routines as the
// reference, including branch delay slots. This subset rejects every opcode
// outside the two routines rather than becoming another general emulator.
static void original_release(uint32_t head) {
    uint32_t r[32]{};r[4]=head;r[29]=0x801fff00;r[31]=0x800000fc;
    uint32_t pc=0x8001c83c,pending=0;
    for(unsigned steps=0;pc!=0x800000fc;++steps) {
        assert(steps<50000);
        uint32_t ins=psx_mod_read_word(pc),op=ins>>26,rs=(ins>>21)&31,rt=(ins>>16)&31,rd=(ins>>11)&31;
        int32_t imm=int16_t(ins);uint32_t branch=0,next=pending?pending:pc+4;pending=0;
        switch(op) {
        case 0: if(!ins)break;
            if((ins&63)==0x21)r[rd]=r[rs]+r[rt];
            else if((ins&63)==8)branch=r[rs];else assert(false);break;
        case 2:branch=((pc+4)&0xf0000000)|((ins&0x3ffffff)<<2);break;
        case 3:r[31]=pc+8;branch=((pc+4)&0xf0000000)|((ins&0x3ffffff)<<2);break;
        case 4:if(r[rs]==r[rt])branch=pc+4+imm*4;break;
        case 5:if(r[rs]!=r[rt])branch=pc+4+imm*4;break;
        case 9:r[rt]=r[rs]+imm;break;
        case 15:r[rt]=(ins&65535)<<16;break;
        case 35:r[rt]=psx_mod_read_word(r[rs]+imm);break;
        case 37:r[rt]=psx_mod_read_half(r[rs]+imm);break;
        case 41:psx_mod_write_half(r[rs]+imm,r[rt]);break;
        case 43:psx_mod_write_word(r[rs]+imm,r[rt]);break;
        default:assert(false);
        }
        r[0]=0;pending=branch;pc=next;
    }
}
int main(int argc,char**argv) {
    assert(argc==2);
    std::ifstream f(argv[1],std::ios::binary);f.read((char*)baseline,sizeof baseline);assert(f.gcount()==sizeof baseline);
    // The owned crash is a real list/allocator fixture, not proof that its
    // already-corrupt object contents can be resumed. Reuse happens AFTER reset.
    CPUState c{};c.gpr[4]=1;c.gpr[31]=0x80025474;c.gpr[29]=0x801fff80;
    auto reset=[](){std::memcpy(ram,baseline,sizeof ram);writes=0;};
    reset();
    unsigned before=psx_mod_read_half(0x800d141a),n=0;
    for(uint32_t h:{0x800c5694u,0x800c5698u})for(uint32_t p=psx_mod_read_word(h);p;p=psx_mod_read_word(p+4)){assert(++n<=1024);}
    assert(n>0);
    original_release(0x800c5694);original_release(0x800c5698);
    // Original calls leave their saved-register words below SP; the host hook
    // deliberately does not touch that stack memory.
    std::memcpy(ram+0x1ffec0,baseline+0x1ffec0,0x40);
    std::vector<unsigned char> reference(ram,ram+sizeof ram);
    reset();auto cpu_before=c;callback(&c,0x8002b9f4);
    assert(!std::memcmp(ram,reference.data(),sizeof ram));
    assert(writes && !std::memcmp(&c,&cpu_before,sizeof c));
    assert(!psx_mod_read_word(0x800c5694) && !psx_mod_read_word(0x800c5698));
    assert(psx_mod_read_half(0x800d141a)==before+n);
    unsigned actual=0;for(uint32_t p=psx_mod_read_word(0x800ce408);p;p=psx_mod_read_word(p+4)){assert(++actual<=1024);assert(!psx_mod_read_word(p+8));}
    assert(actual==before+n);
    assert(!std::memcmp(ram+0xd7198,baseline+0xd7198,0x8a4)); // player unchanged
    writes=0;callback(&c,0x8002b9f4);assert(!writes); // ordinary already-cleared reset
    reset();c.gpr[31]=0x80025478;callback(&c,0x8002b9f4);assert(!writes);c.gpr[31]=0x80025474;
    reset();c.gpr[4]=0;callback(&c,0x8002b9f4);assert(!writes);c.gpr[4]=1;
    reset();ram[0x1c83c]^=1;callback(&c,0x8002b9f4);assert(!writes);
    reset();auto first=psx_mod_read_word(0x800c5694);psx_mod_write_word(first+4,first);writes=0;callback(&c,0x8002b9f4);assert(!writes);
    reset();psx_mod_write_word(0x800c5698,psx_mod_read_word(0x800c5694));writes=0;callback(&c,0x8002b9f4);assert(!writes);
    reset();psx_mod_write_half(0x800d141a,before+1);writes=0;callback(&c,0x8002b9f4);assert(!writes);
    reset();g_psx_call_bail=1;callback(&c,0x8002b9f4);assert(!writes);g_psx_call_bail=0;
    std::printf("PASS: captured %u-node scene list retired before heap reuse; original-MIPS RAM equivalence, free-pool accounting, CPU/player preservation, repeat and rejection cases\n",n);
}
