// D17R: the original sky across the D14 widescreen columns. See note 172.
//
// The sky routine 0x800388e4 runs after the screen-mode call 0x8001fc44 has
// put the root rectangle 0x800d2210 back to the 4:3 (-256,-120,512,240), so
// three of its parts stop at the original picture's edges:
//  - the flat sky-colour fill, an F4 polygon from the root rectangle drawn by
//    0x8002bc18 (ra 0x80038bb4): black corners above the horizon;
//  - the camera-facing backdrop (selector 0x800da464, 0x800292a0 ra
//    0x80038c68), authored X -416..416 for the original field of view: a
//    slanted edge with the clouds over black past it;
//  - the object renderer's rectangle test (context 0x800d67a8 +0x18..+0x1e,
//    flag 0x80, copied from 0x800d22a0) rejects any polygon wholly outside
//    -256..256, so cloud and horizon pieces in the margins are dropped.
// Each part is widened by the live margin m: the fill and the clip rectangle
// to -256-m..256+m, the backdrop's local X by (256+m)/256 (it is built facing
// the camera's look direction, so its X axis is the view's horizontal right
// axis: its screen edges move out by that factor and its vertical gradient is
// unchanged). The routine, its timer, textures, the
// D17O camera-relative matrices and 4:3 (m=0, Vanilla) are untouched.
// DNTTK_SKY_WIDE=0 (developer) keeps the 4:3 sky.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "sky_render.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;

namespace ttk {
namespace {
constexpr uint32_t camera=0x800d6eb0u,context=0x800d67a8u,players=0x800c27bcu;

int32_t margin() {
    static const bool off=[]{const char* t=std::getenv("DNTTK_SKY_WIDE");return t && !std::strcmp(t,"0");}();
    if(off) return 0;
    const int32_t m=psx_mod_widescreen_x_margin();
    if(m<=0 || m>512 || psx_mod_read_word(players)>=2) return 0;
    static SkyRenderIdentity identity;
    return identity.valid(input_host_frame(),g_dirty_ram_code_gen,g_psx_ram) ? m : 0;
}

// Two x halves of one rectangle, widened only when they are the exact 4:3 pair.
void widen(uint32_t left,uint32_t right,int32_t m) {
    if((int16_t)psx_mod_read_half(left)!=-256 || (int16_t)psx_mod_read_half(right)!=256) return;
    psx_mod_write_half(left,(uint16_t)(int16_t)(-256-m));
    psx_mod_write_half(right,(uint16_t)(int16_t)(256+m));
}

void hook(CPUState* cpu,uint32_t address) {
    const uint32_t ra=cpu->gpr[31];
    if(address==0x8002bc18u) {
        if(ra!=0x80038bb4u) return;
        const int32_t m=margin();if(!m) return;
        // F4 packet: x0 +8, x1 +0xc, x2 +0x10, x3 +0x14 (left, right, left, right).
        const uint32_t p=cpu->gpr[5];
        if(p<0x80010000u || p>0x801fffe8u) return;
        widen(p+8,p+0xc,m);
        widen(p+0x10,p+0x14,m);
    } else if(address==0x800292a0u) {
        if(ra!=0x80038c68u || cpu->gpr[4]!=camera) return;
        const int32_t m=margin();if(!m) return;
        // Guest-stack MATRIX. The trace showed the model's forward axis in its
        // row 2 (the camera's look direction), so local X is row 0: m00 +0,
        // m01 +2, m02 +4. Scaling a column instead skews it off the view's yaw.
        const uint32_t mat=cpu->gpr[5];
        if(mat<0x80010000u || mat>0x801fffe0u) return;
        for(uint32_t o:{0u,2u,4u}) {
            const int32_t v=(int16_t)psx_mod_read_half(mat+o);
            const int32_t s=(v*(256+m)+(v<0?-255:255))/256;  // round away from zero
            psx_mod_write_half(mat+o,(uint16_t)(int16_t)(s<-32768?-32768:s>32767?32767:s));
        }
    } else if(address==0x80010000u) {
        if(cpu->gpr[6]!=context) return;
        if(ra!=0x80038c94u && ra!=0x80038cf0u && ra!=0x80038da8u && ra!=0x80038e44u) return;
        const int32_t m=margin();if(!m) return;
        widen(context+0x18,context+0x1a,m);
    }
}
}
}

PSX_MOD_CONSTRUCTOR(register_ttk_sky_widescreen) {
    for(uint32_t address:{0x8002bc18u,0x800292a0u,0x80010000u})
        psx_mod_register_function_entry_plugin("ttk.sky.widescreen",address,ttk::hook);
}
