// D08A27: extend only the native toxic-gas FT4 to the live viewport.
// Resident 0x8002fc38 (sole caller 0x800354cc, exposed camera owner) computes
// the native vertical boundary and animated material 5, then hardcodes X to
// +/-256. Change the completed packet, leaving all simulation and material
// fields intact. Replay workers execute the same hook with their live margin.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "code_identity.h"
#include <cstdlib>

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;

namespace {
void gas_widescreen(CPUState* cpu,uint32_t) {
    if(cpu->gpr[31]!=0x8002ffc0u || cpu->gpr[19]!=5 ||
       !ttk::input_modernized() || psx_mod_read_word(0x800c27bcu)!=1) return;
    static const bool disabled=[] {const char* s=std::getenv("DNTTK_GAS_WIDE");
        return s && !std::strcmp(s,"0");}();
    const int32_t m=psx_mod_widescreen_x_margin();
    if(disabled || m<=0 || m>512) return;
    const uint32_t p=cpu->gpr[5];
    if(p<0x80010000u || p>0x801fffd8u || (p&3) ||
       psx_mod_read_byte(p+3)!=9 || psx_mod_read_byte(p+7)!=0x2e) return;
    for(uint32_t o:{8u,24u})
        if((int16_t)psx_mod_read_half(p+o)!=-256 ||
           (int16_t)psx_mod_read_half(p+o+8)!=256) return;
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={
        {0x8002fc38u,0x3b4u,"4b8b7820898b618e07f406e4e42c5090d7650962a469a23f2b03d509e49fd51f"},
        {0x800354a0u,0x34u,"4c62604065e9200cfa73c3198352791638e763031273d916428fbfb28404b700"},
    };
    static std::array<std::vector<uint32_t>,2> expected;
    static ttk::IdentityMemo memo;
    const auto frame=ttk::input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen))
        memo.set(frame,g_dirty_ram_code_gen,ttk::code_identity(guards,expected,psx_mod_read_word,g_psx_ram));
    if(!memo.ok) return;
    for(uint32_t o:{8u,24u}) {
        psx_mod_write_half(p+o,uint16_t(-256-m));
        psx_mod_write_half(p+o+8,uint16_t(256+m));
    }
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_gas_widescreen) {
    psx_mod_register_function_entry_plugin("ttk.gas.widescreen",0x8002bc18u,gas_widescreen);
}
