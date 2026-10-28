// D17AD: repeat the native loading backdrop into the widescreen margins.
// Keep the original FT4, texture window and spinner unchanged. Insert two
// translated copies before the texture-window reset in the native DMA chain.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "live_env.h"
#include "code_identity.h"

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
namespace {
constexpr uint32_t backdrop=0x800d24e8u, quad=backdrop+12;
constexpr uint32_t region_bytes=80, regions=4;
uint32_t arena=0;
void reserve() {if(!arena) arena=psx_mod_alloc_gpu_dma_memory(region_bytes*regions+16,16);}
void extend(CPUState* cpu,uint32_t) {
    // Last native AddPrim: both the quad and reset are complete and linked.
    if(cpu->gpr[31]!=0x80022270u || cpu->gpr[5]!=backdrop ||
       cpu->gpr[4]!=0x7ff || !ttk::input_modernized()) return;
    const int m=psx_mod_widescreen_x_margin();
    // An adaptive 4:3 frame may be held while the window grows. Keep its
    // side tiles outside the clip rectangle so that cached image can widen.
    const char* wide=ttk_live_env("DNTTK_WIDESCREEN");
    const bool adaptive=wide && !std::strcmp(wide,"auto");
    if(m<0 || (m==0 && !adaptive) || m>512 || !arena) return;
    if(psx_mod_read_word(backdrop+4)!=0xe2084318u ||
       psx_mod_read_word(backdrop+0x38)!=0xe2000000u ||
       psx_mod_read_byte(quad+3)!=9 || psx_mod_read_byte(quad+7)!=0x2c ||
       (psx_mod_read_word(quad)&0xffffff)!=(backdrop+0x34u)%0x1000000u) return;
    for(unsigned i=0;i<4;++i) {
        if((int16_t)psx_mod_read_half(quad+8+8*i)!=(i%2?256:-256) ||
           (int16_t)psx_mod_read_half(quad+10+8*i)!=(i/2?120:-120) ||
           psx_mod_read_byte(quad+12+8*i)!=(i%2?255:0) ||
           psx_mod_read_byte(quad+13+8*i)!=(i/2?255:0)) return;
    }
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={{0x800220f4u,0x19cu,"3daceb419526e21889e8c5b7a26f09f187388d83a4b9cb5fd2bb1a2351a0b902"}};
    static std::array<std::vector<uint32_t>,1> expected;
    static ttk::IdentityMemo memo;
    const auto frame=ttk::input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen))
        memo.set(frame,g_dirty_ram_code_gen,ttk::code_identity(guards,expected,psx_mod_read_word,g_psx_ram));
    if(!memo.ok) return;
    // Mirror the region cursor in saved/replayed memory, so loading an older
    // frame cannot overwrite the region still referenced by its DMA chain.
    const uint32_t mirror=arena+region_bytes*regions;
    const uint32_t region=(psx_mod_read_word(mirror)+1)%regions;
    psx_mod_write_word(mirror,region);
    const uint32_t left=arena+region*region_bytes,right=left+40;
    for(uint32_t dst:{left,right}) {
        for(unsigned o=0;o<40;o+=4) psx_mod_write_word(dst+o,psx_mod_read_word(quad+o));
        for(unsigned i=0;i<4;++i) {
            const int x=(int16_t)psx_mod_read_half(quad+8+8*i)+(dst==left?-512:512);
            psx_mod_write_half(dst+8+8*i,uint16_t(x));
        }
    }
    psx_mod_write_word(left,0x09000000u|(right&0xffffff));
    psx_mod_write_word(quad,0x09000000u|(left&0xffffff));
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_loading_widescreen) {
    reserve();
    psx_mod_register_function_entry_plugin("ttk.loading.widescreen",0x8002bc18u,extend);
}
