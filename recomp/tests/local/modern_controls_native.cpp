#include "tight_camera.h"
static bool renderer_available=true;
extern "C" int psx_mod_present_replay_available(void){return renderer_available;}
#include <cstdint>
#include <cstdlib>
#include <string_view>
extern "C" int pgxp_mesh_geometry(void) {const char* s=std::getenv("DNTTK_GEOMETRY_PRECISION");return s && std::string_view(s)=="corrected";}
extern "C" int pgxp_mesh_textures(void) {const char* s=std::getenv("DNTTK_TEXTURE_PRECISION");return s && std::string_view(s)=="corrected";}
extern "C" void pgxp_mesh_register_boundary(void) {}
extern "C" void pgxp_invalidate_all(void) {}
extern "C" {int g_pgxp_mesh_active=0;}
extern "C" int pgxp_mesh_vertex(uint32_t,uint32_t,int32_t*,int32_t*,uint16_t*) {return 0;}
extern "C" void gte_nclip_culling_set(int) {}
extern "C" void gte_nclip_culling_stats(uint64_t* checks,uint64_t* flips) {*checks=*flips=0;}
// Owned-data integration harness. Models call contracts, not terrain/animation.
#include "modern_controls.h"
#include "level_select.h"
#include "control_math.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <array>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <iterator>
#include <algorithm>
static uint8_t ram[0x200000];
// The runtime runs every plugin registered at an address, in registration order.
struct HookChain {
    std::vector<PSXModFunctionEntryCallback> list;
    void operator()(CPUState* cpu,uint32_t address) const {for(auto hook:list)hook(cpu,address);}
};
static auto& hooks() {static std::map<uint32_t,HookChain> value;return value;}
static unsigned writes, code_writes;
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t,uint32_t){return 0x801e0000;}
extern "C" uint32_t psx_mod_alloc_gpu_dma_memory(uint32_t,uint32_t){return 0;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a) {return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a) {return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a) {return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
// Runtime RAM image + RAM-code generation (memory.c). The runtime bumps the
// generation on clean->dirty page transitions and executable-range marks;
// this mock bumps on every store, the conservative model.
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
// A guest store into guarded code: the runtime marks the page (generation bump).
static void poke(uint32_t offset) {ram[offset]^=1;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v) {ram[a&0x1fffff]=v;++writes;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v) {ram[a&0x1fffff]=v;ram[(a+1)&0x1fffff]=v>>8;++writes;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v) {psx_mod_write_half(a,v);psx_mod_write_half(a+2,v>>16);}
extern "C" void psx_mod_write_code_word(uint32_t a,uint32_t v) {++code_writes;psx_mod_write_word(a,v);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb) {hooks()[a].list.push_back(cb);return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_depth(uint32_t,uint32_t,int32_t,int32_t,float,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_coverage(uint32_t,uint32_t,int32_t,int32_t,float,float,float,float,float){return 1;}
static PSXModActivationCallback activation;static unsigned aspect_num,aspect_den;
extern "C" int psx_mod_register_activation_plugin(const char*,PSXModActivationCallback cb) {activation=cb;return 1;}
extern "C" int psx_mod_set_fixed_display_aspect(uint32_t n,uint32_t d) {aspect_num=n;aspect_den=d;return 1;}
static unsigned adaptive_num,adaptive_den;static int32_t ws_margin;
extern "C" int psx_mod_set_adaptive_display_aspect(uint32_t n,uint32_t d) {adaptive_num=n;adaptive_den=d;return 1;}
extern "C" int32_t psx_mod_widescreen_x_margin(void) {return ws_margin;}
extern "C" {int g_precise_mode=0,g_ls_mode=0,g_psx_call_bail=0;}
static int clearance_result=2048;
static uint32_t hull_object;static int hull_ray_hit,hull_steep;static std::vector<int32_t> hull_ray_y;   // D08K2
static int32_t hull_object_top;   // D08K7: the step 0x80077748 reports onto the contact's top
static int ladder_end_result=0;static unsigned ladder_end_calls;
static unsigned kicks, edge_launches, crouch_falls;
static bool terrain_nested_hook;static unsigned terrain_nested_calls;
static int terrain_result=3,terrain_floor=-1000;
static uint32_t terrain_reference;
static unsigned ledge_probe_calls; static bool ledge_probe_pass;
static std::vector<uint32_t> cheat_calls;
static uint32_t enemy_restore_actor;
// D08X lift: the stubbed acquisition catches only when Duke is at or above lift_catch_y.
static int32_t lift_catch_y=-100000;static unsigned lineups;static int lineup_ok=1;static std::vector<int32_t> lift_calls;
// D08Y drop: the stub also catches when Duke is lowered to drop_catch_y or below,
// as drop_mode with the ledge top drop_top; held_seen records the held bit.
static int32_t drop_catch_y=100000,drop_top=0,lookahead_drop=-100000;static unsigned drop_mode=6;static std::vector<int> held_seen;
// D12A quick kick: damage sphere and sound calls (address, a0..a3, two stack words, point).
struct KickCall {uint32_t address,a[4],stack[2];int32_t point[3];};
static std::vector<KickCall> kick_calls;
// D12C: the stubbed sphere strikes something (it stores its attacker argument
// at sp+0x10 of its own 0x80-byte frame, as 0x800a979c does on a hit).
static bool sphere_strike;
static std::vector<std::array<uint32_t,4>> steroid_sounds;

// D08Y: the runtime CPU overclock lease (renewed from the player update).
static unsigned overclock_renewals;extern "C" void psx_overclock_renew(void){++overclock_renewals;}
// D08Z1: the isolated flight sweep 0x8007a98c reports `jw_result` with the
// normal (jw_nx, 0, jw_nz) for its first `jw_hits` calls, then a clear path.
static int jw_result,jw_hits,jw_sweeps;static int32_t jw_nx,jw_nz;
extern "C" void psx_dispatch_call(CPUState* cpu,uint32_t address,uint32_t) {
    if(address==0x8007d240) {
        const uint32_t p=0x800d7198;++ledge_probe_calls;
        assert(psx_mod_read_word(p+0x17c)==0 && psx_mod_read_word(p+0x174)==0);
        psx_mod_write_word(p+0x184,1234); // query scratch must roll back on refusal
        psx_mod_write_word(p+0x1c8,uint32_t(-11264));
        cpu->gpr[2]=ledge_probe_pass;cpu->pc=0;return;
    }
    if(address==0x8003f060) {
        assert(cpu->gpr[4]==0x800d7198 && cpu->gpr[5]==1);++crouch_falls;
        psx_mod_write_half(0x800d7198+0x60,108);psx_mod_write_byte(0x800d7198+0x22c,9);
        psx_mod_write_word(0x800d7198+0x1f8,0);cpu->pc=0;return;
    }
    if(address==0x80095cb0) {cpu->pc=0;return;} // D08T3 room membership
    if(address==0x8007a98c) {
        const bool hit=++jw_sweeps<=jw_hits;
        psx_mod_write_word(0x800d7198+0x194,uint32_t(jw_nx));psx_mod_write_word(0x800d7198+0x198,0);
        psx_mod_write_word(0x800d7198+0x19c,uint32_t(jw_nz));
        cpu->gpr[2]=hit?uint32_t(jw_result):0;cpu->pc=0;return;
    }
    if(address==0x8003ebf4) {cpu->pc=0;return;}   // D08Z1: its isolated integration
    if(address==0x8003d738 || address==0x8003d7bc || address==0x8003d840) {
        cheat_calls.push_back(address);cpu->pc=0;return;
    }
    if(address==0x80098d70 || address==0x80098f3c) {
        auto row=cpu->gpr[4];cheat_calls.push_back(address);
        if(address==0x80098d70){psx_mod_write_word(row+4,0);psx_mod_write_word(row+20,0);}
        else {psx_mod_write_word(row+4,enemy_restore_actor);psx_mod_write_word(row+20,1);}
        cpu->pc=0;return;
    }
    if(address==0x8003e2d0) {++edge_launches;cpu->pc=0;return;}
    if(address==0x8006b73c) { // D08A4: the pickup sound R plays at Duke
        steroid_sounds.push_back({cpu->gpr[4],cpu->gpr[5],cpu->gpr[6],cpu->gpr[7]});
        if(cpu->gpr[4]==0x2007) { // D12C impact: a sound object (id +0x5c, table volume +0x6a) and its handle
            kick_calls.push_back({0x8006b73c,{cpu->gpr[4],cpu->gpr[5],cpu->gpr[6],cpu->gpr[7]},{},{}});
            psx_mod_write_word(0x801e1000+0x5c,0x2007);psx_mod_write_half(0x801e1000+0x6a,5192);
            cpu->gpr[2]=0x05000000u|(0x801e1000u&0xffffffu);
        }
        cpu->pc=0;return;
    }
    if(address==0x8007ec4c) {++lineups;cpu->gpr[2]=lineup_ok;cpu->pc=0;return;}   // D08X mantle line-up
    if(address==0x80055208) {
        const uint32_t p=0x800d7198;const int32_t y=int32_t(psx_mod_read_word(p+8));lift_calls.push_back(y);
        const uint32_t held=psx_mod_read_word(0x800d1b50+4*psx_mod_read_byte(p+0x233));
        held_seen.push_back(held>=0x80010000u && held<0x80200000u ? int(psx_mod_read_word(held)&1) : -1);
        if(y<=lift_catch_y){psx_mod_write_byte(p+0x22c,6);psx_mod_write_half(p+0x60,148);cpu->gpr[2]=1;}
        else if(y>=drop_catch_y){psx_mod_write_byte(p+0x22c,drop_mode);psx_mod_write_half(p+0x60,148);psx_mod_write_word(p+0x1c8,uint32_t(drop_top));
            psx_mod_write_half(p+0x24,1024);psx_mod_write_word(p+4,111);psx_mod_write_word(p+12,222);psx_mod_write_word(p+0x17c,0x5678);cpu->gpr[2]=1;}
        else {psx_mod_write_word(p+0x174,0x1234);psx_mod_write_word(p+0x1c4,999);cpu->gpr[2]=0;}
        cpu->pc=0;return;
    }
    if(address==0x800a979c || address==0x8006b270) {
        KickCall k{address,{cpu->gpr[4],cpu->gpr[5],cpu->gpr[6],cpu->gpr[7]},
                   {psx_mod_read_word(cpu->gpr[29]+0x10),psx_mod_read_word(cpu->gpr[29]+0x14)},{}};
        if(address==0x800a979c) for(int i=0;i<3;++i) k.point[i]=(int32_t)psx_mod_read_word(cpu->gpr[4]+4*i);
        if(address==0x800a979c && sphere_strike) psx_mod_write_word(cpu->gpr[29]-0x80+0x10,k.stack[0]);
        kick_calls.push_back(k);cpu->pc=0;return;
    }
    if(address==0x8007ded0) { // D08U1 ladder-end probe; it clobbers the touch fields like 0x8007dd50
        ++ladder_end_calls;psx_mod_write_word(0x800d7198+0x174,0);psx_mod_write_half(0x800d7198+0x178,0xffff);
        cpu->gpr[2]=ladder_end_result;cpu->pc=0;return;
    }
    // D08K2 crouch probe: object probe, head ray (records the ray height), slope test.
    if(address==0x800780b4) {psx_mod_write_word(0x800d7198+0x174,hull_object);cpu->pc=0;return;}
    if(address==0x800774cc) {
        hull_ray_y.push_back(int32_t(psx_mod_read_word(cpu->gpr[5]+4)));hull_ray_y.push_back(int32_t(psx_mod_read_word(cpu->gpr[6]+4)));
        cpu->gpr[2]=hull_ray_hit;cpu->pc=0;return;
    }
    if(address==0x80077438) {assert(cpu->gpr[4]==0x800d7198+0x1a4);cpu->gpr[2]=hull_steep;cpu->pc=0;return;}
    if(address==0x80077748) {assert(psx_mod_read_word(0x800d7198+0x174));psx_mod_write_word(0x800d7198+0x1c4,uint32_t(hull_object_top));cpu->pc=0;return;}
    if(address==0x8003ff6c) {
        psx_mod_write_half(0x800d7198+0x358,psx_mod_read_half(0x800d7198+0x358)&~2u);
        psx_mod_write_word(0x800d7198+0x224,psx_mod_read_word(0x800d7198+0x224)|0x10000000);
        cpu->pc=0;return;
    }
    if(address==0x8004001c) {
        constexpr uint32_t p=0x800d7198;
        const bool on=psx_mod_read_half(p+0x358)&2;
        psx_mod_write_half(p+0x84c,on?5:0);
        psx_mod_write_half(p+0x358,psx_mod_read_half(p+0x358)&~0x8000u);
        psx_mod_write_word(p+0x224,psx_mod_read_word(p+0x224)&~0x10001000u);
        cpu->pc=0;return;
    }
    assert(address==0x8007926c || address==0x800797f4 || address==0x800402d0 || address==0x8007765c || address==0x800517a4);
    if(address==0x800517a4)++kicks;
    if(address==0x8007926c || address==0x800797f4) {
        if(terrain_nested_hook) {
            CPUState nested=*cpu;nested.gpr[29]-=0x158;nested.gpr[31]=0x800794b8;
            hooks()[0x800780b4](&nested,0x800780b4);++terrain_nested_calls;
        }
        terrain_reference=psx_mod_read_word(0x800d7198+0x1c8);
        psx_mod_write_word(0x800d7198+0x1c4,terrain_floor);cpu->gpr[2]=terrain_result;
    } else {
        if(address==0x8007765c && lookahead_drop!=-100000)psx_mod_write_word(0x800d7198+0x1c4,uint32_t(lookahead_drop));
        cpu->gpr[2]=clearance_result;
    }
    cpu->pc=0;
}
namespace ttk {bool input_live_look(uint64_t&,double&,double&,double){return false;} void input_state_loaded(){}}
// D08K9: opt-in clip availability for landing ownership fixtures.
static bool landing_clips;
namespace ttk {bool anim_extra_clip(unsigned clip,uint32_t& first,uint32_t& count){
    if(!landing_clips)return false;first=10000+clip*10;count=3;return true;
}}
// D23E: no high-refresh redraws in the harness; the overclock lease pauses.
namespace ttk {int replay_load_window(int window){return window==1 ? -1 : 0;}}
extern "C" void ttk_fast_timing_renew(void) {}
static uint32_t save_loads;
extern "C" uint32_t psx_mod_savestate_loads(void){return save_loads;}
extern "C" int psx_mod_replay_active(void){return 0;}
namespace ttk {
const char* aim_debug_json(){return "{}";}
// D12A: the crosshair segment query (weapon_aim.cpp in the player build).
static bool segment_hit;static double segment_point[3];static int segment_kind=2;
bool view_segment_query(CPUState*,const double*,const double* to,double* hit,int& kind) {
    kind=segment_hit?segment_kind:0;for(int i=0;i<3;++i)hit[i]=segment_hit?segment_point[i]:to[i];return true;
}
static InputFrame input;
static Cheat pending_cheat=Cheat::None;
Cheat input_take_cheat(){auto c=pending_cheat;pending_cheat=Cheat::None;return c;}
void input_notice(const char*){}
void input_request_interaction(){}
void inventory_update(unsigned,const uint16_t*,const int16_t*,const uint16_t*,bool){}
void mission_update(unsigned,const uint16_t*){}
bool mission_open;bool mission_visible(){return mission_open;}void mission_close(){mission_open=false;}int mission_browsed_item(){return -1;}
static bool modern;
static bool running, jump_pending, auto_stow;
bool input_auto_stow_pending(){return auto_stow;}
bool input_jump_pending(){return jump_pending;}
bool input_take_jump(){bool result=jump_pending;jump_pending=false;return result;}
bool input_take_swim_redraw(){return false;}
static bool want_capture;
static unsigned capture_offers;
void input_offer_gameplay_capture() {++capture_offers;}
void input_ack_commands(uint64_t) {}
bool input_wants_initial_capture() {return want_capture;}
bool input_running(){return running;}
static bool grab_owns;
bool input_push_grab_owns_cross(){return grab_owns;}
static bool reach_held;
bool input_airborne_reach_held(){return reach_held;}
bool input_modernized(){return modern;}bool input_modern_controls(){const char* s=std::getenv("DNTTK_CONTROL_SCHEME");return modern && (!s || std::strcmp(s,"legacy"));}
const InputFrame& input_snapshot(Context){return input;}
static uint16_t test_pad=0xffff;
uint16_t input_last_pad(){return test_pad;}
uint64_t input_host_frame(){return input.sequence;}
static double clock_ms=1000;
double input_clock_ms(){return clock_ms;}
}
static void call(uint32_t address,uint32_t a0,uint32_t a1,uint32_t ra,uint32_t sp=0x801f0000,uint32_t a2=0) {
    CPUState cpu{};cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;cpu.gpr[31]=ra;cpu.gpr[29]=sp;
    hooks().at(address)(&cpu,address);
}
int main(int argc,char** argv) {
    setenv("DNTTK_DRAW_DISTANCE","extended",1);
    assert(argc>=3 && argc<=5);
    std::ifstream exe(argv[1],std::ios::binary);exe.seekg(2048);exe.read((char*)ram+0x10000,0xbb000);assert(exe.gcount()==0xbb000);
    std::ifstream overlay(argv[2],std::ios::binary);overlay.read((char*)ram+0xca968,9668);assert(overlay.gcount()==9668);
    constexpr uint32_t p=0x800d7198,c=0x800d6eb0;
    psx_mod_write_half(0x800bcbb0,1);
    psx_mod_write_word(c+0xa4,p);psx_mod_write_word(p+0x7d4,c);
    psx_mod_write_half(p+0x60,63);
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096);
    ttk::input.active=true;ttk::input.sequence=1;ttk::input.epoch=1;ttk::input.move_x=1;
    writes=0;call(0x8003ade4,c,p,0x80025ee8);call(0x80053500,p,0,0x80048664);assert(writes==0); // Vanilla
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    call(0x80048410,p,0,0x8004b634,0x801fff00);call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
    assert(code_writes==0 && psx_mod_read_word(0x800cc57c)==0x24020008);
    ttk::modern=true;
    psx_mod_write_word(0x800be570,1);
    call(0x8003ade4,c,p,0x80025ee8);assert(code_writes==0);
    psx_mod_write_word(0x800be570,0);
    call(0x8003ade4,c,p,0x80025ee4);assert(code_writes==0);
    const auto secret_flags=psx_mod_read_word(0x800dd878);
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    assert(code_writes==2 && psx_mod_read_word(0x800cc57c)==0x24020000 &&
           psx_mod_read_word(0x800cc580)==0x30a30400);
    assert(psx_mod_read_word(0x800dd878)==secret_flags); // no invented conversation
    call(0x8003ade4,c,p,0x80025ee8);assert(code_writes==2); // idempotent
    // D07E: the lease lasts one game frame. Slow updates (5 fields, an upgraded
    // energy stream) widen it to 6; a pause longer than the cap resets it to 4.
    {
        auto camera=[&](unsigned fields){ttk::input.sequence+=fields;call(0x8003ade4,c,p,0x80025ee8);};
        for(int i=0;i<4;++i)camera(5);
        ttk::input.sequence+=5;assert(ttk::movement_ready());
        ttk::input.sequence+=1;assert(ttk::movement_ready());
        ttk::input.sequence+=1;assert(!ttk::movement_ready());
        camera(30);ttk::input.sequence+=5;assert(!ttk::movement_ready());
        camera(30);for(int i=0;i<4;++i)camera(3); // a stall interval is not remembered
        ttk::input.sequence+=4;assert(ttk::movement_ready());
        ttk::input.sequence+=1;assert(!ttk::movement_ready());
        camera(1);assert(ttk::movement_ready());
        std::puts("PASS: D07E lease lasts one game frame, also in slow scenes");
    }
    if(const char* scheme=std::getenv("DNTTK_JETPACK");scheme && !std::strcmp(scheme,"eduke32")) {
        auto camera=[&](){call(0x8003ade4,c,p,0x80025ee8);};
        auto poll=[&](){call(0x80058120,p,0,0x80041c44,0x801fff00);};
        auto toggle=[&](){ttk::input.commands[0]=ttk::jetpack;ttk::input.command_count=1;++ttk::input.command_serial;camera();poll();ttk::input.command_count=0;};
        auto update=[&](unsigned fields){ttk::input.sequence+=fields;camera();call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);poll();};
        ttk::input.move_x=0;ttk::input.move_y=0;
        psx_mod_write_half(p+0x32,10000);psx_mod_write_half(p+0x358,1);psx_mod_write_half(p+0x35a,9000);
        psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x224,0);
        psx_mod_write_word(0x800d21fc,10);psx_mod_write_half(0x800c3cc4,0);psx_mod_write_half(0x800c3cc6,0);
        toggle();assert(psx_mod_read_byte(p+0x22c)==10 && psx_mod_read_half(p+0x60)==163 && psx_mod_read_half(p+0x84c)==5);
        update(2);assert((int32_t)psx_mod_read_word(p+0x1f8)<0); // J alone starts lift, no Space
        // This hook-only fixture does not integrate the preceding lift.
        psx_mod_write_word(p+0x1c8,psx_mod_read_word(p+0x10)+1000);
        ttk::input.held[ttk::crouch]=true;update(2);
        assert((int32_t)psx_mod_read_word(p+0x1f8)>0); // Ctrl cancels startup lift immediately
        ttk::input.held[ttk::crouch]=false;
        for(int i=0;i<28;++i)update(2);
        assert(psx_mod_read_half(p+0x35a)==8853); // one second, independent of animation dt
        toggle();assert(psx_mod_read_byte(p+0x22c)==9 && psx_mod_read_half(p+0x60)==108 && psx_mod_read_half(p+0x84c)==0);
        const auto fuel=psx_mod_read_half(p+0x35a);update(2);assert(psx_mod_read_half(p+0x35a)==fuel);
        psx_mod_write_word(p+0x224,0x41);toggle();assert(psx_mod_read_byte(p+0x22c)==10); // airborne flags
        psx_mod_write_half(p+0x35a,9000);
        ++ttk::input.epoch;update(2);const auto restored=psx_mod_read_half(p+0x35a);
        assert(restored==9000); // new capture/load never bills an old interval
        for(int i=0;i<1829;++i)update(2);
        assert(psx_mod_read_byte(p+0x22c)==10 && psx_mod_read_half(p+0x35a)>0);
        update(2);assert(psx_mod_read_half(p+0x35a)==0 && psx_mod_read_byte(p+0x22c)==9);
        toggle();assert(psx_mod_read_byte(p+0x22c)==9); // empty cannot activate
        psx_mod_write_half(p+0x35a,9000);psx_mod_write_word(p+0x834,0);psx_mod_write_word(p+8,10);
        toggle();assert(psx_mod_read_byte(p+0x22c)==9); // water veto
        psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_half(p+0x32,0);
        toggle();assert(psx_mod_read_byte(p+0x22c)==9); // death veto
        psx_mod_write_half(p+0x32,10000);poke(0x4ade0);toggle();assert(psx_mod_read_byte(p+0x22c)==9);poke(0x4ade0);
        ttk::modern=false;toggle();assert(psx_mod_read_byte(p+0x22c)==9);ttk::modern=true;
        toggle();assert(psx_mod_read_byte(p+0x22c)==10);
        psx_mod_write_half(0x800c3cc6,1);const auto god_fuel=psx_mod_read_half(p+0x35a);
        for(int i=0;i<30;++i)update(2);
        assert(psx_mod_read_half(p+0x35a)==god_fuel);
        psx_mod_write_half(0x800c3cc6,0);
        psx_mod_write_half(p+0x35a,4000);update(2);
        ttk::input.sequence+=2;camera();call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        psx_mod_write_half(p+0x35a,9000);poll();assert(psx_mod_read_half(p+0x35a)==9000); // pickup during native handler
        psx_mod_write_half(0x800be568,1);ttk::input.sequence+=180;camera();
        psx_mod_write_half(0x800be568,0);update(2);assert(psx_mod_read_half(p+0x35a)==9000); // no pause debt
        // A native landing closes the pack without replacing the ground pose.
        ttk::input.sequence+=2;camera();call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_half(p+0x60,105);poll();
        assert(psx_mod_read_byte(p+0x22c)==0 && psx_mod_read_half(p+0x60)==105 && !(psx_mod_read_half(p+0x358)&2));
        std::puts("PASS: D08S instant ground/air toggles, 61-second fuel, god mode, landing and safety gates");return 0;
    }
    // D08Q3: actual flight eligibility retains overlay, camera, capture and
    // alive checks, and never enables ground movement/air-steering writes.
    {
        setenv("DNTTK_WEAPON_AIM","view",1);
        psx_mod_write_half(p+0x358,3);psx_mod_write_byte(p+0x22c,10);
        psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);
        for(unsigned anim=163;anim<=170;++anim) {
            psx_mod_write_half(p+0x60,anim);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
            assert(ttk::jetpack_input_ready() && ttk::view_aim_input_ready());
            assert(!ttk::movement_ready() && !ttk::locomotion_input_ready() && !ttk::airborne_input_ready());
        }
        // D08Q6 Modern flight model: the host writes the flight velocity
        // (+0x1f4..+0x1fc) before every update with the hover lock off;
        // WASD eases to a camera-relative speed; while moving, a vertical
        // step under 2 units alternates +-2 (8004ac08 drops horizontal motion
        // on a 0 vertical root); the height is captured and held (floor cap);
        // Space/Ctrl climb/descend; fed pads' thrust (read after the handler)
        // is cancelled next update; the flame flag is set after the handler;
        // fuel drains on the host only when no pads are fed.
        {
            auto update=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
            auto post=[&]{call(0x80058120,p,0,0x80041c44,0x801fff00);};
            auto bv=[&](int axis){return (int32_t)psx_mod_read_word(p+0x1f4+4*axis);};
            auto y=[&](int32_t v,int32_t clearance){psx_mod_write_word(p+8,(uint32_t)v);psx_mod_write_word(p+0x10,(uint32_t)(v+600));psx_mod_write_word(p+0x1c8,(uint32_t)(v+600+clearance));};
            const uint32_t saved_flags=psx_mod_read_word(p+0x224);const uint16_t saved_fuel=psx_mod_read_half(p+0x35a);
            psx_mod_write_half(p+0x60,164);psx_mod_write_word(p+0x224,0x08000000u);
            const uint16_t saved_selected=psx_mod_read_half(0x800c3f94);
            for(int axis=0;axis<3;++axis){psx_mod_write_word(p+0x1f4+4*axis,0);psx_mod_write_word(p+0x1e4+4*axis,77);}
            psx_mod_write_half(0x800c3cc4,0);psx_mod_write_half(p+0x35a,5000);psx_mod_write_half(p+0xfe,0);
            ttk::input.move_x=0;ttk::input.move_y=0;ttk::input.arrow_y=0;
            psx_mod_write_word(0x800d21fc,10); // dt
            y(-10700,900);update();
            assert(!(psx_mod_read_word(p+0x224)&0x08000000u)); // hover lock off
            for(int axis=0;axis<3;++axis)assert(psx_mod_read_word(p+0x1e4+4*axis)==0);
            assert(bv(0)==0 && bv(1)==0 && bv(2)==0 && psx_mod_read_half(p+0x35a)==4990); // held; idle fuel -dt
            post();assert((psx_mod_read_word(p+0x224)&0x82000000u)==0x82000000u); // both native flame-render bits
            // W: camera-relative (heading = view), eases in; +-2 vertical steps.
            ttk::input.move_y=1;update();
            const double yaw=psx_mod_read_half(p+0x1c)*6.283185307179586/4096;
            assert(std::abs(bv(0))+std::abs(bv(2))>1000);
            assert(bv(1)==205 || bv(1)==-205);
            const int32_t first_vertical=bv(1);update();assert(bv(1)==-first_vertical);
            assert(psx_mod_read_half(p+0x35a)==4990); // fed pads: the original drains
            for(int i=0;i<60;++i)update();
            assert(std::abs(bv(0)-std::lround(16000*std::sin(yaw)))<=2 && std::abs(bv(2)-std::lround(16000*std::cos(yaw)))<=2);
            // The handler's thrust (read after it) is cancelled next update.
            psx_mod_write_word(p+0x1e4,20);psx_mod_write_word(p+0x1e8,(uint32_t)-30);psx_mod_write_word(p+0x1ec,40);post();
            const int32_t x_before=bv(0);update();
            assert(std::abs(bv(0)-(x_before-(20*10+10)))<=2);
            // Model the handler's applied vertical root as the plan (no thrust).
            auto applied=[&]{psx_mod_write_half(p+0xfe,(uint16_t)(int16_t)((bv(1)*10)>>10));};
            ttk::input.move_y=0;for(int i=0;i<60;++i){update();applied();post();}
            assert(bv(0)==0 && bv(2)==0); // eased to a stop, no creep
            // Held height: 20 units above it steers down; floor cap rides higher ground.
            psx_mod_write_half(p+0xfe,0);
            y(-10720,920);update();assert(bv(1)>60 && bv(1)<400); // steer down (plus any learned offset)
            y(-10600,800);update();assert(bv(1)<-400);
            y(-10700,300);update();assert(bv(1)<-400); // 212 below the approach height
            // Space climbs (eases), release re-captures once settled; Ctrl descends.
            y(-10700,900);ttk::input.held[ttk::jump]=true;update();assert(bv(1)<0);
            for(int i=0;i<30;++i)update();assert(std::abs(bv(1)+11800)<=300);
            ttk::input.held[ttk::jump]=false;y(-11000,1200);for(int i=0;i<40;++i)update();
            assert(std::abs(bv(1))<=120); // captured at -11000 (plus any learned offset)
            ttk::input.held[ttk::crouch]=true;for(int i=0;i<30;++i)update();assert(std::abs(bv(1)-6000)<=300);
            // D11F follow-up: floor approach keeps the running jetpack in
            // flight. Ctrl's positive step is bounded; release does not apply
            // the old 512-unit automatic clearance rise.
            y(-11000,64);psx_mod_write_half(p+0xfe,0);
            jw_result=2;jw_hits=1000;jw_sweeps=0;
            const auto query_scratch=psx_mod_read_word(p+0x194);
            // The contact guard uses a -2-unit root to keep native horizontal
            // collision active, plus the learned sub-unit root correction.
            update();assert(bv(1)>=-350 && bv(1)<=120 && psx_mod_read_byte(p+0x22c)==10);
            assert(jw_sweeps>0 && psx_mod_read_word(p+0x194)==query_scratch);
            // Leaving the platform must allow descent despite stale +1c8.
            jw_hits=0;update();assert(bv(1)>1000);
            // Releasing Ctrl below the stale platform must hold the new
            // height, not pull Duke back up toward that cached floor.
            ttk::input.held[ttk::crouch]=false;
            for(int i=0;i<40;++i)update();assert(std::abs(bv(1))<=120);
            ttk::input.held[ttk::crouch]=true;
            // A newly encountered platform is detected even with stale low floor.
            y(-11000,5000);jw_hits=1000;update();assert(bv(1)>=-350 && bv(1)<=120);
            post();assert((psx_mod_read_word(p+0x224)&0x82000000u)==0x82000000u);
            // The ordinary draw consumes 0x02000000 before the mirror draw.
            psx_mod_write_word(p+0x224,psx_mod_read_word(p+0x224)&~0x02000000u);
            call(0x800348d8,0x801ffe58,p,0x8003816c);
            assert((psx_mod_read_word(p+0x224)&0x82000000u)==0x82000000u);
            psx_mod_write_word(p+0x224,psx_mod_read_word(p+0x224)&~0x02000000u);
            call(0x800348d8,0x801ffe58,0x800d8000,0x8003816c);
            assert(!(psx_mod_read_word(p+0x224)&0x02000000u));
            ttk::modern=false;call(0x800348d8,0x801ffe58,p,0x8003816c);
            assert(!(psx_mod_read_word(p+0x224)&0x02000000u));ttk::modern=true;
            ttk::input.held[ttk::crouch]=false;
            for(int i=0;i<40;++i)update();
            assert(bv(1)>=-350 && bv(1)<=120 && psx_mod_read_byte(p+0x22c)==10);
            jw_hits=0;
            ttk::input.held[ttk::jump]=true;update();assert(bv(1)<0);
            ttk::input.held[ttk::jump]=false;
            psx_mod_write_half(0x800c3cc4,1);const uint16_t f0=psx_mod_read_half(p+0x35a);update();assert(psx_mod_read_half(p+0x35a)==f0); // original skip
            psx_mod_write_half(0x800c3cc4,0);
            for(int axis=0;axis<3;++axis){psx_mod_write_word(p+0x1f4+4*axis,0);psx_mod_write_word(p+0x1e4+4*axis,0);}
            psx_mod_write_word(p+0x224,saved_flags);psx_mod_write_half(p+0x35a,saved_fuel);
            psx_mod_write_half(0x800c3f94,saved_selected); // post() ran the flight shortcuts
            std::puts("PASS: D08Q6 flight model: lock off, eased camera-relative speed, +-2 vertical step, thrust cancel, flame, stop without creep, hold/floor cap, Space/Ctrl, fuel");
        }
        ttk::input.active=false;assert(!ttk::jetpack_input_ready());ttk::input.active=true;
        ttk::modern=false;assert(!ttk::jetpack_input_ready());ttk::modern=true;
        psx_mod_write_word(p,2);assert(!ttk::jetpack_input_ready());psx_mod_write_word(p,0);
        psx_mod_write_word(c+0xa4,0);assert(!ttk::jetpack_input_ready());psx_mod_write_word(c+0xa4,p);
        poke(0x4ade0);assert(!ttk::jetpack_input_ready());poke(0x4ade0);
        psx_mod_write_half(p+0x358,1);assert(!ttk::jetpack_input_ready());
        psx_mod_write_half(p+0x358,0);psx_mod_write_byte(p+0x22c,0);psx_mod_write_half(p+0x60,63);
        psx_mod_write_byte(p+0x3b8,0);psx_mod_write_byte(p+0x3b9,0);
        ttk::input.sequence=1;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        unsetenv("DNTTK_WEAPON_AIM");
        std::puts("PASS: D08Q3 flight lease, all flight animations, guards and ground handoff");
    }
    // D08T1 0x80051cf0: only Grab's own Cross is kept from climbing a pushable
    // object; E's Cross climbs it like any climbable. 0x80051890: E's Cross never
    // reaches the original idle grab of a pushable-only object; Grab's does.
    // Only this tick's bit, only for Duke from the original callers; never
    // Vanilla, uncaptured or plain climbables.
    {
        const uint32_t table=0x801e8000,object=0x801e9000,word=0x800d147c;
        const uint32_t saved[]={psx_mod_read_word(0x800d2660),psx_mod_read_word(0x800d1b50),psx_mod_read_word(word)};
        const unsigned saved_player=psx_mod_read_byte(p+0x233);
        psx_mod_write_word(0x800d2660,table);psx_mod_write_half(object+0x2c,165);
        psx_mod_write_word(table+28*165,0x081090e2);
        psx_mod_write_byte(p+0x233,0);psx_mod_write_word(0x800d1b50,word);
        psx_mod_write_word(p+0x174,object);
        auto cross=[&](uint32_t ra,uint32_t actor){psx_mod_write_word(word,3);call(0x80051cf0,actor,0,ra);return psx_mod_read_word(word);};
        assert(cross(0x8005384c,p)==3); // E's Cross climbs the dumpster
        ttk::grab_owns=true;
        assert(cross(0x8005384c,p)==2 && cross(0x80052544,p)==2 && cross(0x8005349c,p)==2);
        assert(cross(0x80053850,p)==3 && cross(0x8005384c,p+0x8a4)==3);
        psx_mod_write_word(table+28*165,0xe2);assert(cross(0x8005384c,p)==3); // climbable only
        psx_mod_write_word(table+28*165,0x081090e2);
        ttk::input.active=false;assert(cross(0x8005384c,p)==3);ttk::input.active=true;
        ttk::modern=false;assert(cross(0x8005384c,p)==3);ttk::modern=true;
        ttk::grab_owns=false;
        auto idle=[&](uint32_t ra,uint32_t actor){psx_mod_write_word(word,1);call(0x80051890,actor,0,ra);return psx_mod_read_word(word);};
        assert(idle(0x800467fc,p)==0 && idle(0x80052c1c,p)==0); // E never grabs
        ttk::grab_owns=true;assert(idle(0x80052c1c,p)==1);ttk::grab_owns=false; // Grab does
        assert(idle(0x80052c20,p)==1 && idle(0x80052c1c,p+0x8a4)==1);
        psx_mod_write_word(table+28*165,0x08400000);assert(idle(0x80052c1c,p)==1); // also a switch
        psx_mod_write_word(table+28*165,0xe2);assert(idle(0x80052c1c,p)==1); // not pushable
        psx_mod_write_word(table+28*165,0x081090e2);
        ttk::input.active=false;assert(idle(0x80052c1c,p)==1);ttk::input.active=true;
        ttk::modern=false;assert(idle(0x80052c1c,p)==1);ttk::modern=true;
        // D08T2: the table holds 1062 types (0x7428 bytes); the medieval
        // Duke-symbol block is type 924. 1061 is the last valid type.
        psx_mod_write_half(object+0x2c,924);psx_mod_write_word(table+28*924,0x081890e2);
        assert(idle(0x80052c1c,p)==0);ttk::grab_owns=true;assert(idle(0x80052c1c,p)==1);ttk::grab_owns=false;
        psx_mod_write_half(object+0x2c,1061);psx_mod_write_word(table+28*1061,0x081890e2);
        assert(idle(0x80052c1c,p)==0);
        psx_mod_write_half(object+0x2c,1062);assert(idle(0x80052c1c,p)==1); // outside the table
        psx_mod_write_half(object+0x2c,165);
        // D08T4: touching is offered only where the original grab can start:
        // bit 0 of byte 4 of Duke's floor record (*(player+0x48), unaligned).
        {
            const uint32_t saved_floor=psx_mod_read_word(p+0x48),floor=0x801ea003;
            psx_mod_write_half(p+0x60,63);psx_mod_write_word(p+0x48,floor);
            psx_mod_write_byte(floor+4,0x01);assert(ttk::push_contact_ready());
            psx_mod_write_byte(floor+4,0x00);assert(!ttk::push_contact_ready()); // level 5 street
            psx_mod_write_byte(floor+4,0xfe);assert(!ttk::push_contact_ready());
            psx_mod_write_byte(floor+4,0x01);psx_mod_write_word(p+0x48,0);assert(!ttk::push_contact_ready());
            psx_mod_write_word(p+0x48,floor);psx_mod_write_word(table+28*165,0xe2);assert(!ttk::push_contact_ready());
            psx_mod_write_word(table+28*165,0x081090e2);psx_mod_write_word(p+0x48,saved_floor);
        }
        // Grab 121 remembering the object: camera-only lease, never locomotion.
        psx_mod_write_word(p+0x290,object);psx_mod_write_half(p+0x60,121);
        assert(ttk::push_grab_ready() && !ttk::locomotion_input_ready() && !ttk::movement_ready());
        // D08T3: arbitrary short movement, neutral hold, support and wall
        // refusal, release and actor/profile isolation, using real callbacks.
        {
            const auto saved_push_epoch=ttk::input.epoch;
            const uint32_t saved_push_ticks=psx_mod_read_word(0x800d21fc);
            uint8_t before[0x8a4];for(unsigned i=0;i<sizeof before;++i)before[i]=psx_mod_read_byte(p+i);
            const uint32_t list=0x801ea100,box=0x801ea120;
            psx_mod_write_word(object+0x44,list);psx_mod_write_word(list+8,box);
            const int16_t bounds[]={-512,-512,-512,0,512,512,512,0};
            for(int i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(bounds[i]));
            psx_mod_write_word(object+4,2000);psx_mod_write_word(object+8,0);psx_mod_write_word(object+12,2000);
            psx_mod_write_word(p+0x220,0);psx_mod_write_half(p+0x68,3);
            psx_mod_write_half(p+0x1c,1024);psx_mod_write_word(0x800d21fc,10);
            psx_mod_write_word(p+0x1c8,512);psx_mod_write_word(p+0x1c4,77);
            ttk::grab_owns=true;lookahead_drop=0;clearance_result=2048;
            auto prepare=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
            auto move=[&]{call(0x80048410,p,0,0x8004b634,0x801fff00);};
            // W is already down while grab finishes between the two hooks.
            psx_mod_write_half(p+0x68,1);ttk::input.move_y=1;prepare();
            psx_mod_write_half(p+0x68,3);
            if(!ttk::push_pad(0,1)){ttk::input.move_x=1;ttk::input.move_y=0;}
            const int x0=int32_t(psx_mod_read_word(object+4));
            move();const int x1=int32_t(psx_mod_read_word(object+4));assert(std::abs(x1-x0)==30);
            assert(psx_mod_read_word(p+0x1c4)==77 && psx_mod_read_word(p+0x1c8)==512);
            const float mx=ttk::input.move_x,my=ttk::input.move_y;
            ttk::input.move_x=ttk::input.move_y=0;move();assert(int32_t(psx_mod_read_word(object+4))==x1);
            ttk::input.move_x=mx;ttk::input.move_y=my;lookahead_drop=512;move();assert(int32_t(psx_mod_read_word(object+4))==x1);
            lookahead_drop=0;hull_ray_hit=1;move();assert(int32_t(psx_mod_read_word(object+4))==x1);hull_ray_hit=0;
            ttk::input.move_x=-mx;ttk::input.move_y=-my;move();assert(int32_t(psx_mod_read_word(object+4))==x0);
            // A zero-depth ladder panel, rotated across the travel axis, must
            // stop the block while allowing it to retreat without regrabbing.
            const uint32_t ladder=0x801ea200,ladder_list=0x801ea300,ladder_box=0x801ea320,node=0x801ea400;
            psx_mod_write_half(ladder+0x2c,46);psx_mod_write_word(table+28*46,0x108212);
            psx_mod_write_word(ladder+0x44,ladder_list);psx_mod_write_word(ladder_list+8,ladder_box);
            const int16_t panel[]={-189,-1023,0,0,189,1023,0,0};
            for(int i=0;i<8;++i)psx_mod_write_half(ladder_box+2*i,uint16_t(panel[i]));
            psx_mod_write_half(ladder+0x1c,1024);psx_mod_write_word(ladder+8,0);psx_mod_write_word(ladder+12,2000);
            const int direction=x1>x0?1:-1;
            psx_mod_write_word(ladder+4,x0+direction*(512+24));
            psx_mod_write_word(node+4,0);psx_mod_write_word(node+8,ladder);
            const uint32_t saved_rooms=psx_mod_read_word(0x800da454),saved_count=psx_mod_read_word(0x800da47c);
            psx_mod_write_word(0x800da454,0x801ea500);psx_mod_write_word(0x800da47c,1);
            psx_mod_write_word(0x801ea548,node);psx_mod_write_word(p+0x220,0);
            ttk::input.move_x=mx;ttk::input.move_y=my;move();
            const int stopped=int32_t(psx_mod_read_word(object+4));
            assert(std::abs(stopped-x0)<30);move();assert(int32_t(psx_mod_read_word(object+4))==stopped);
            ttk::input.move_x=-mx;ttk::input.move_y=-my;move();
            assert(int32_t(psx_mod_read_word(object+4))==stopped-direction*30);
            psx_mod_write_word(0x800da454,saved_rooms);psx_mod_write_word(0x800da47c,saved_count);
            // D08T5: slot-7 boundary is exact even for a partial final step.
            // Old saves beyond it can retreat; another lane/type is unaffected.
            const uint32_t oy=psx_mod_read_word(object+8),oz=psx_mod_read_word(object+12);
            const uint16_t angle=psx_mod_read_half(object+0x1c);
            psx_mod_write_word(p+0x220,0);
            psx_mod_write_word(object+8,uint32_t(-9728));psx_mod_write_word(object+12,13829);
            psx_mod_write_half(object+0x1c,2048);
            ttk::input.move_x=mx*direction;ttk::input.move_y=my*direction;
            psx_mod_write_word(object+4,7287);move();assert(psx_mod_read_word(object+4)==7292);
            move();move();assert(psx_mod_read_word(object+4)==7292);
            ttk::input.move_x=-mx*direction;ttk::input.move_y=-my*direction;
            move();assert(psx_mod_read_word(object+4)==7262);
            psx_mod_write_word(object+4,8418);move();assert(psx_mod_read_word(object+4)==8388);
            ttk::input.move_x=mx*direction;ttk::input.move_y=my*direction;
            move();assert(psx_mod_read_word(object+4)==8388);
            psx_mod_write_word(object+12,2000);move();assert(psx_mod_read_word(object+4)==8418);
            psx_mod_write_word(object+12,13829);psx_mod_write_half(object+0x2c,924);
            move();assert(psx_mod_read_word(object+4)==8448);psx_mod_write_half(object+0x2c,165);
            psx_mod_write_word(object+8,oy);psx_mod_write_word(object+12,oz);psx_mod_write_half(object+0x1c,angle);
            std::puts("PASS: D08T5 exact alley limit, held stop, retreat from old save, other lane/type");
            // D08T7: a boundary load can acquire without any previous host
            // grab. Eligibility and original alignment are separate from travel.
            {
                uint32_t keep[0x8a4/4];for(unsigned i=0;i<0x8a4/4;++i)keep[i]=psx_mod_read_word(p+4*i);
                const uint32_t floor=0x801ea003;
                psx_mod_write_word(p+0x48,floor);psx_mod_write_byte(floor+4,0xfe);
                psx_mod_write_word(p+0x174,object);psx_mod_write_word(p+0x17c,0);
                psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);
                psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x24,1024);
                psx_mod_write_word(p+0x220,0);psx_mod_write_word(0x800da47c,0);
                psx_mod_write_word(object+4,7292);psx_mod_write_word(object+8,uint32_t(-9728));
                psx_mod_write_word(object+12,13829);psx_mod_write_half(object+0x1c,2048);
                assert(ttk::push_contact_ready());
                auto acquire=[&]{psx_mod_write_word(word,1);call(0x80051890,p,0,0x80052c1c,0x801fff00);};
                auto refused=[&]{acquire();assert(psx_mod_read_half(p+0x60)==63 && psx_mod_read_byte(floor+4)==0xfe);};
                lineup_ok=0;refused();lineup_ok=1;
                hull_ray_hit=1;refused();hull_ray_hit=0;
                lookahead_drop=512;refused();lookahead_drop=0;
                psx_mod_write_half(p+0x24,0);refused();psx_mod_write_half(p+0x24,1024);
                psx_mod_write_word(p+0x17c,object);refused();psx_mod_write_word(p+0x17c,0);
                psx_mod_write_word(p+0x224,0x40);refused();psx_mod_write_word(p+0x224,0);
                ttk::modern=false;refused();ttk::modern=true;
                ttk::input.active=false;refused();ttk::input.active=true;
                acquire();assert(psx_mod_read_half(p+0x60)==121 && psx_mod_read_word(p+0x290)==object);
                assert(psx_mod_read_byte(floor+4)==0xfe && psx_mod_read_word(object+4)==7292);
                psx_mod_write_half(p+0x60,63);psx_mod_write_word(object+4,8418);
                acquire();assert(psx_mod_read_half(p+0x60)==121); // old beyond-limit save
                psx_mod_write_half(p+0x60,63);psx_mod_write_word(object+12,13830);
                assert(!ttk::push_contact_ready());refused();
                for(unsigned i=0;i<0x8a4/4;++i)psx_mod_write_word(p+4*i,keep[i]);
                std::puts("PASS: D08T7 fresh boundary acquisition, old save, line-up/support/wall/axis/state/profile refusal, unchanged floor");
            }

            psx_mod_write_word(p+0x220,0);psx_mod_write_word(object+4,x0);
            psx_mod_write_half(p+0x60,120);++ttk::input.epoch;ttk::grab_owns=false;prepare();
            assert(psx_mod_read_half(p+0x60)==121); // capture/pause epoch must end the old clip
            ttk::grab_owns=false;prepare();move();assert(int32_t(psx_mod_read_word(object+4))==x0);
            ttk::modern=false;ttk::grab_owns=true;prepare();move();assert(int32_t(psx_mod_read_word(object+4))==x0);
            ttk::modern=true;ttk::grab_owns=false;ttk::input.move_x=ttk::input.move_y=0;lookahead_drop=-100000;
            ttk::input.epoch=saved_push_epoch;psx_mod_write_word(0x800d21fc,saved_push_ticks);
            for(unsigned i=0;i<sizeof before;++i)psx_mod_write_byte(p+i,before[i]);
            std::puts("PASS: D08T3 held-direction grab handoff, flat ladder, stop/reverse, held neutral, support/wall refusal, query restoration and Vanilla");
        }
        psx_mod_write_half(p+0x60,63);psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x290,0);
        assert(!ttk::push_grab_ready() && ttk::movement_ready());
        psx_mod_write_word(0x800d2660,saved[0]);psx_mod_write_word(0x800d1b50,saved[1]);psx_mod_write_word(word,saved[2]);
        psx_mod_write_byte(p+0x233,saved_player);
    }
    // D08T8: only mismatched object support in a settled ledge hang retries
    // the original clearance probe. A refusal restores all query state.
    {
        const auto memory=std::vector<uint8_t>(ram,ram+sizeof ram);
        const auto input=ttk::input;
        const uint32_t object=0x801e9000,table=0x801e8000,list=0x801ea100,box=list+32;
        psx_mod_write_word(0x800d2660,table);psx_mod_write_word(table+28*165,0x081090e2);
        psx_mod_write_half(object+0x2c,165);psx_mod_write_word(object+8,uint32_t(-9728));
        psx_mod_write_word(object+0x44,list);psx_mod_write_word(list+8,box);
        const int16_t bounds[]={-512,-512,-512,0,512,512,512,0};
        for(unsigned i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(bounds[i]));
        ttk::input={};ttk::input.active=true;ttk::input.held[ttk::move_forward]=true;
        psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);
        psx_mod_write_byte(p+0x22c,6);psx_mod_write_byte(p+0x22d,6);
        psx_mod_write_half(p+0x60,149);psx_mod_write_half(p+0x74,149);
        psx_mod_write_word(p+0x174,object);psx_mod_write_word(p+0x17c,object);psx_mod_write_half(p+0x180,0);
        psx_mod_write_word(p+0x1c8,uint32_t(-11264));psx_mod_write_word(p+0x184,777);
        auto update=[&]{++ttk::input.sequence;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
        ttk::test_pad=0xffef;assert(ttk::ledge_hang_ready());ledge_probe_pass=false;auto n=ledge_probe_calls;update();
        assert(ledge_probe_calls==n+1 && psx_mod_read_half(p+0x60)==149);
        assert(psx_mod_read_word(p+0x17c)==object && psx_mod_read_word(p+0x174)==object && psx_mod_read_word(p+0x184)==777);
        auto refuses=[&]{auto before=ledge_probe_calls;update();assert(ledge_probe_calls==before && psx_mod_read_half(p+0x60)==149);};
        ttk::test_pad=0xffbf;refuses();ttk::test_pad=0xffef;
        ttk::input.held[ttk::jump]=true;refuses();ttk::input.held[ttk::jump]=false;
        ttk::input.held[ttk::move_back]=true;refuses();ttk::input.held[ttk::move_back]=false;
        ttk::input.held[ttk::move_forward]=false;refuses();ttk::input.held[ttk::move_forward]=true;
        ttk::input.active=false;refuses();ttk::input.active=true;
        ttk::modern=false;refuses();ttk::modern=true;
        psx_mod_write_byte(p+0x22c,3);assert(!ttk::ledge_hang_ready());refuses();
        psx_mod_write_byte(p+0x22c,7);assert(!ttk::ledge_hang_ready());refuses();psx_mod_write_byte(p+0x22c,6);
        psx_mod_write_word(p+0x1c8,uint32_t(-10240));refuses();psx_mod_write_word(p+0x1c8,uint32_t(-11264));
        psx_mod_write_half(p+0x180,1);refuses();psx_mod_write_half(p+0x180,0);
        psx_mod_write_word(p+0x224,0x40);refuses();psx_mod_write_word(p+0x224,0);
        poke(0x7d240);assert(!ttk::ledge_hang_ready());refuses();poke(0x7d240);
        ledge_probe_pass=true;update();assert(psx_mod_read_half(p+0x60)==140 && psx_mod_read_word(p+0x17c)==0);
        assert(psx_mod_read_half(p+0x68)==0 && psx_mod_read_half(p+0x6a)==0);
        std::copy(memory.begin(),memory.end(),ram);++g_dirty_ram_code_gen;ttk::input=input;ttk::test_pad=0xffff;
        std::puts("PASS: D08T8 support mismatch, clearance refusal restoration, original pull-up and input/state/profile/code guards");
    }
    // D08U top-of-ladder mount: the slot-12 sewer ladder (type 308, a 378 x 6142
    // panel at x -4136, yaw 1024, top 507 below the walkway). E's request at the
    // walkway edge attaches it exactly as the airborne catch does (156, mode 3,
    // +0x17c/+0x180) and blends Duke onto the far-side climbing line.
    {
        const uint32_t table=0x801e8000,object=0x801e9100,list=0x801e9200,box=0x801e9300,node=0x801e9400;
        const uint32_t saved_table=psx_mod_read_word(0x800d2660);
        const auto saved_sequence=ttk::input.sequence;
        psx_mod_write_word(0x800d2660,table);psx_mod_write_word(table+28*308,0x108202);
        psx_mod_write_half(object+0x2c,308);psx_mod_write_half(object+0x1c,1024);
        psx_mod_write_word(object+4,uint32_t(-4136));psx_mod_write_word(object+8,uint32_t(-2047));psx_mod_write_word(object+12,76328);
        psx_mod_write_word(object+0x44,list);psx_mod_write_word(list+8,box);
        const int16_t panel[]={-189,-3071,0,189,189,3071,0,3076};
        for(int i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(panel[i]));
        psx_mod_write_word(node+8,object);psx_mod_write_word(node+4,0);psx_mod_write_word(p+0x220,node);
        auto place=[&](int x,int y,int z){psx_mod_write_word(p+4,uint32_t(x));psx_mod_write_word(p+8,uint32_t(y));psx_mod_write_word(p+12,uint32_t(z));};
        auto camera=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        auto update=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
        auto s32=[](uint32_t a){return int32_t(psx_mod_read_word(a));};
        place(-3798,-5625,76329);psx_mod_write_half(p+0x1c,3002);psx_mod_write_byte(p+0x3b8,0);
        camera();assert(ttk::movement_ready() && ttk::ladder_top_available());
        place(-3798+800,-5625,76329);assert(!ttk::ladder_top_available()); // out of reach
        place(-3798,-5625,76329+700);assert(!ttk::ladder_top_available()); // past its side
        place(-3798,-5200,76329);assert(!ttk::ladder_top_available()); // not at its top
        place(-3798,-5625,76329);
        psx_mod_write_word(table+28*308,0x8402);assert(!ttk::ladder_top_available()); // climbing wall
        psx_mod_write_word(table+28*308,0x108202);
        psx_mod_write_half(p+0x60,108);assert(!ttk::ladder_top_available());psx_mod_write_half(p+0x60,63);
        // Armed: the request is ignored (E stows first in pc_input).
        psx_mod_write_byte(p+0x3b8,2);ttk::ladder_top_request();update();
        assert(psx_mod_read_half(p+0x60)==63 && psx_mod_read_byte(p+0x22c)==0);
        psx_mod_write_byte(p+0x3b8,0);
        ttk::ladder_top_request();update();
        assert(psx_mod_read_half(p+0x60)==156 && psx_mod_read_byte(p+0x22c)==3 && psx_mod_read_byte(p+0x22d)==3);
        assert(psx_mod_read_word(p+0x17c)==object && psx_mod_read_half(p+0x180)==0 && s32(p+4)==-3798);
        for(int i=0;i<12;++i) {
            camera();assert(ttk::traversal_camera_ready() && !ttk::locomotion_input_ready());update();
        }
        assert(s32(p+4)==-4336 && s32(p+8)==-5118+320 && s32(p+12)==76328 && psx_mod_read_half(p+0x1c)==1024);
        psx_mod_write_half(p+0x60,186);update(); // the original transfer ended: attached
        assert(psx_mod_read_half(p+0x60)==186 && std::strstr(ttk::controls_debug_json(),"\"mounts\":1"));
        // D08J3: the mouse camera stays on through the attached climb (mode 3).
        camera();assert(ttk::traversal_camera_ready() && !ttk::locomotion_input_ready() && !ttk::airborne_shot_ready());
        // D08U1: the original's ladder-end probe while stepping down. Open below
        // (ladder ends above the floor): the descent flag is cleared so the
        // original stops at this rung; touch fields survive the probe.
        psx_mod_write_word(p+0x174,0x801e1234);psx_mod_write_half(p+0x178,7);
        psx_mod_write_half(p+0x60,188);psx_mod_write_half(p+0x6a,1);ladder_end_result=0;update();
        assert(ladder_end_calls>0 && psx_mod_read_half(p+0x6a)==1 && !ttk::ladder_end_below());
        ladder_end_result=1;update();
        assert(psx_mod_read_half(p+0x6a)==0 && std::strstr(ttk::controls_debug_json(),"\"end_stops\":1"));
        assert(psx_mod_read_word(p+0x174)==0x801e1234 && psx_mod_read_half(p+0x178)==7);
        psx_mod_write_half(p+0x60,186);psx_mod_write_half(p+0x6a,1);update(); // resting: no flag write
        assert(psx_mod_read_half(p+0x6a)==1);
        const unsigned calls=ladder_end_calls;ttk::modern=false;psx_mod_write_half(p+0x60,188);update();
        assert(ladder_end_calls==calls && psx_mod_read_half(p+0x6a)==1);ttk::modern=true; // Vanilla untouched
        ladder_end_result=0;psx_mod_write_half(p+0x60,186);psx_mod_write_half(p+0x6a,0);psx_mod_write_word(p+0x174,0);
        // Vanilla never mounts.
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);place(-3798,-5625,76329);
        ttk::modern=false;ttk::ladder_top_request();update();assert(psx_mod_read_half(p+0x60)==63);ttk::modern=true;
        ttk::input.sequence+=25;camera();update();assert(psx_mod_read_half(p+0x60)==63); // a stale request expires
        psx_mod_write_word(p+0x220,0);psx_mod_write_word(p+0x17c,0);psx_mod_write_word(0x800d2660,saved_table);
        place(0,0,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
        ttk::input.sequence=saved_sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        std::puts("PASS: D08U top-of-ladder mount (reach/side/top/family/state gates, armed and Vanilla refusal, catch-equivalent attach, 12-update blend onto the climbing line, camera-only lease, 186 handoff, request expiry, D08U1 ladder-end stop)");
    }
    // D08J5 pole/chain top mount: the user's slot-3 chain (type 842, flags
    // 0x100412, a 138-wide column from Y -3072 to 1024 at (-4049, 31248)),
    // its top 509 below the platform. E at the edge attaches it as the airborne
    // catch does (154) and swings Duke half a turn round it while lowering him.
    {
        const uint32_t table=0x801e8000,object=0x801e9100,list=0x801e9200,box=0x801e9300,node=0x801e9400;
        const uint32_t saved_table=psx_mod_read_word(0x800d2660);
        const auto saved_sequence=ttk::input.sequence;
        psx_mod_write_word(0x800d2660,table);psx_mod_write_word(table+28*842,0x100412);
        psx_mod_write_half(object+0x2c,842);psx_mod_write_half(object+0x1c,0);
        psx_mod_write_word(object+4,uint32_t(-4049));psx_mod_write_word(object+8,uint32_t(-1024));psx_mod_write_word(object+12,31248);
        psx_mod_write_word(object+0x44,list);psx_mod_write_word(list+8,box);
        const int16_t column[]={-69,-2048,-69,97,69,2048,69,2050};
        for(int i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(column[i]));
        psx_mod_write_word(node+8,object);psx_mod_write_word(node+4,0);psx_mod_write_word(p+0x220,node);
        auto place=[&](int x,int y,int z){psx_mod_write_word(p+4,uint32_t(x));psx_mod_write_word(p+8,uint32_t(y));psx_mod_write_word(p+12,uint32_t(z));};
        auto camera=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        auto update=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
        auto s32=[](uint32_t a){return int32_t(psx_mod_read_word(a));};
        place(-4386,-3581,31122);psx_mod_write_half(p+0x1c,797);psx_mod_write_byte(p+0x3b8,0);
        camera();assert(ttk::movement_ready() && ttk::ladder_top_available());
        place(-4386-400,-3581,31122);assert(!ttk::ladder_top_available()); // out of reach
        place(-4386,-3200,31122);assert(!ttk::ladder_top_available()); // not at its top
        place(-4386,-3581,31122);
        psx_mod_write_word(table+28*842,0x100512);assert(!ttk::ladder_top_available()); // 0x100 family
        psx_mod_write_word(table+28*842,0x100412);
        ttk::modern=false;ttk::ladder_top_request();update();assert(psx_mod_read_half(p+0x60)==63);ttk::modern=true; // Vanilla
        ttk::ladder_top_request();update();
        assert(psx_mod_read_half(p+0x60)==154 && psx_mod_read_byte(p+0x22c)==3 && psx_mod_read_byte(p+0x22d)==3);
        assert(psx_mod_read_word(p+0x17c)==object && psx_mod_read_half(p+0x180)==0 && s32(p+0x1c4)==0);
        const int start=psx_mod_read_half(p+0x1c);
        assert(std::abs(std::hypot(s32(p+4)+4049.0,s32(p+12)-31248.0)-225)<2); // the original's hang radius
        for(int i=0;i<12;++i) {
            camera();assert(ttk::traversal_camera_ready() && !ttk::locomotion_input_ready());update();
        }
        // Facing the chain from the edge, then half a turn.
        const int facing=int(std::lround(std::atan2(-4049+4386.0,31248-31122.0)*4096/(2*M_PI)))&4095;
        assert(std::abs(start-facing)<64 && psx_mod_read_half(p+0x1c)==((facing+2048)&4095) && s32(p+8)==-3072+400);
        assert(s32(p+4)>-4049); // the far side, facing the platform
        psx_mod_write_half(p+0x60,192);update();
        assert(std::strstr(ttk::controls_debug_json(),"\"pole\":{\"mount_starts\":1,\"mounts\":1"));
        // Down probe (0x8007d65c from the hang-climb, s3 = 1): a floor above
        // Duke's feet is replaced by one far below; a floor below is kept.
        auto probe=[&](uint32_t caller,unsigned dir,int32_t floor){
            const uint32_t sp=0x801f0000;psx_mod_write_word(sp+0x64,caller);psx_mod_write_word(sp+0x4c,uint32_t(floor));
            CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[19]=dir;cpu.gpr[29]=sp;cpu.gpr[31]=0x8007d7fc;
            hooks().at(0x8003964c)(&cpu,0x8003964c);return s32(sp+0x4c);};
        assert(probe(0x80044340,1,-3072)==-2672+0x8000 && probe(0x80044434,1,-3072)==-2672+0x8000);
        assert(probe(0x80044434,1,510)==510 && probe(0x80044434,0,-3072)==-3072 && probe(0x80044999,1,-3072)==-3072);
        ttk::modern=false;assert(probe(0x80044434,1,-3072)==-3072);ttk::modern=true;
        // S resting at the end (the original does not leave 192): after 8 updates, let go.
        ttk::input.held[ttk::move_back]=true;
        for(int i=0;i<7;++i){update();assert(!ttk::pole_let_go_ready());}
        update();assert(ttk::pole_let_go_ready());
        psx_mod_write_half(p+0x60,195);update();assert(!ttk::pole_let_go_ready()); // stepping down again
        ttk::input.held[ttk::move_back]=false;
        // The original top exit and step-off keep the camera-only lease.
        psx_mod_write_byte(p+0x22c,8);psx_mod_write_half(p+0x60,191);camera();assert(ttk::traversal_camera_ready() && ttk::ladder_exit_ready());
        psx_mod_write_half(p+0x60,196);camera();assert(ttk::ladder_exit_ready());
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        psx_mod_write_word(p+0x220,0);psx_mod_write_word(p+0x17c,0);psx_mod_write_word(0x800d2660,saved_table);
        place(0,0,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
        ttk::input.sequence=saved_sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        std::puts("PASS: D08J5 pole/chain top mount (reach/top/family gates, Vanilla refusal, catch-equivalent 154 attach, half-turn swing to the far side, down probe ignores floors above, S at the end lets go, exits keep the lease)");
    }
    // D08J1 overhead ladder leap: the user's slot-6 ladder (type 46, a 378 x
    // 2046 panel at z 80918, yaw 0, bottom about 1040 above the floor).
    {
        const uint32_t table=0x801e8000,object=0x801e9100,list=0x801e9200,box=0x801e9300,node=0x801e9400;
        const uint32_t saved_table=psx_mod_read_word(0x800d2660);
        const auto saved_sequence=ttk::input.sequence;
        const uint32_t saved_floor=psx_mod_read_word(p+0x1c8);
        psx_mod_write_word(0x800d2660,table);psx_mod_write_word(table+28*46,0x108212);
        psx_mod_write_half(object+0x2c,46);psx_mod_write_half(object+0x1c,0);
        psx_mod_write_word(object+4,24577);psx_mod_write_word(object+8,uint32_t(-8203));psx_mod_write_word(object+12,80918);
        psx_mod_write_word(object+0x44,list);psx_mod_write_word(list+8,box);
        const int16_t panel[]={-189,-1023,0,189,189,1023,0,1040};
        for(int i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(panel[i]));
        psx_mod_write_word(node+8,object);psx_mod_write_word(node+4,0);psx_mod_write_word(p+0x220,node);
        auto place=[&](int x,int z){psx_mod_write_word(p+4,uint32_t(x));psx_mod_write_word(p+8,uint32_t(-6650));psx_mod_write_word(p+12,uint32_t(z));};
        auto camera=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        auto gait=[&](unsigned a){psx_mod_write_half(p+0x60,a);};
        psx_mod_write_word(p+0x1c8,uint32_t(-6143));psx_mod_write_half(p+0x1c,2048);psx_mod_write_byte(p+0x3b8,0);
        place(24442,80918+700);gait(63);camera();assert(ttk::movement_ready() && !ttk::ladder_top_available());
        assert(!ttk::ladder_leap_ready()); // standing: only at the wall
        gait(76);assert(ttk::ladder_leap_ready());gait(72);assert(ttk::ladder_leap_ready());
        place(24442,80918+900);gait(76);assert(!ttk::ladder_leap_ready()); // not yet in the window
        place(24442,80918+245);gait(63);assert(ttk::ladder_leap_ready()); // stopped at the wall
        psx_mod_write_half(p+0x1c,0);assert(!ttk::ladder_leap_ready()); // facing away
        psx_mod_write_half(p+0x1c,2048+700);assert(!ttk::ladder_leap_ready()); // beyond 50 degrees
        psx_mod_write_half(p+0x1c,2048+400);assert(ttk::ladder_leap_ready());psx_mod_write_half(p+0x1c,2048);
        place(24577+189+200,80918+245);assert(!ttk::ladder_leap_ready()); // past its side (half + 180)
        place(24577-220,80918+245);assert(ttk::ladder_leap_ready());
        place(24442,80918+245);
        psx_mod_write_byte(p+0x3b8,2);assert(!ttk::ladder_leap_ready());psx_mod_write_byte(p+0x3b8,0); // armed: stow first
        psx_mod_write_word(p+0x1c8,uint32_t(-7000));assert(!ttk::ladder_leap_ready()); // bottom within reach of the floor
        psx_mod_write_word(p+0x1c8,uint32_t(-5600));assert(!ttk::ladder_leap_ready()); // too high to catch
        psx_mod_write_word(p+0x1c8,uint32_t(-6143));
        psx_mod_write_word(table+28*46,0x108412);assert(!ttk::ladder_leap_ready()); // climbing wall family
        psx_mod_write_word(table+28*46,0x108212);
        gait(108);assert(!ttk::ladder_leap_ready());gait(63); // airborne
        // Off-centre leap: 160 beyond the half width starts, and in flight Duke
        // eases along the panel (70 per update) to 60 inside it, never off it.
        place(24577-189-160,80918+245);assert(ttk::ladder_leap_ready());
        place(24577-189-200,80918+245);assert(!ttk::ladder_leap_ready());
        place(24577-189-160,80918+245);assert(ttk::ladder_leap_ready());ttk::ladder_leap_note();
        auto update=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
        gait(97);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        const int x0=int32_t(psx_mod_read_word(p+4));
        update();assert(int32_t(psx_mod_read_word(p+4))==x0+70 && int32_t(psx_mod_read_word(p+12))==80918+245);
        update();assert(int32_t(psx_mod_read_word(p+4))==x0+140);
        update();assert(int32_t(psx_mod_read_word(p+4))==x0+210);
        update();assert(int32_t(psx_mod_read_word(p+4))==24577-129);
        update();assert(int32_t(psx_mod_read_word(p+4))==24577-129);
        assert(std::strstr(ttk::controls_debug_json(),"\"leap_nudges\":4"));
        psx_mod_write_byte(p+0x22c,3);update(); // caught: the assist ends
        psx_mod_write_byte(p+0x22c,9);place(24577-300,80918+245);update();assert(int32_t(psx_mod_read_word(p+4))==24577-300);
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);gait(63);place(24442,80918+245);
        ttk::modern=false;assert(!ttk::ladder_leap_ready());ttk::modern=true;
        psx_mod_write_word(p+0x220,0);psx_mod_write_word(0x800d2660,saved_table);psx_mod_write_word(p+0x1c8,saved_floor);
        psx_mod_write_word(p+4,0);psx_mod_write_word(p+8,0);psx_mod_write_word(p+12,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
        ttk::input.sequence=saved_sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        std::puts("PASS: D08J1 overhead ladder leap (gait windows, wall stop, facing, side, armed, rise limits, family, airborne and Vanilla refusal; in-flight side assist to the panel, ends on the catch)");
    }
    // D08X: E held through an original jump (98/103/104, mode 9) arms the
    // original reach at once (+0x224 |= 0x800000); never without E, with
    // precision aim, over original blocking bits, outside Duke's own ballistic
    // call, in other animations or in Vanilla.
    {
        const auto saved_anim=psx_mod_read_half(p+0x60);
        const auto saved_flags=psx_mod_read_word(p+0x224);
        psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);psx_mod_write_word(p+0x224,0);
        auto arm=[&](unsigned anim,uint32_t ra=0x80055934){psx_mod_write_half(p+0x60,anim);psx_mod_write_word(p+0x224,0);
            call(0x8003ebf4,p,0,ra);return psx_mod_read_word(p+0x224)&0x800000u;};
        ttk::reach_held=false;assert(!arm(98) && !arm(104));
        ttk::reach_held=true;assert(arm(98) && arm(103) && arm(104));
        assert(!arm(97) && !arm(108) && !arm(109) && !arm(105));
        assert(!arm(98,0x80055930));
        ttk::input.held[ttk::original_aim]=true;assert(!arm(98));ttk::input.held[ttk::original_aim]=false;
        psx_mod_write_half(p+0x60,98);psx_mod_write_word(p+0x224,0x40);call(0x8003ebf4,p,0,0x80055934);
        assert(psx_mod_read_word(p+0x224)==0x40);
        ttk::input.active=false;assert(!arm(98));ttk::input.active=true;
        ttk::modern=false;assert(!arm(98));ttk::modern=true;
        ttk::reach_held=false;
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);psx_mod_write_half(p+0x60,saved_anim);
        psx_mod_write_word(p+0x224,saved_flags);
        std::puts("PASS: D08X hold-E reach arm (98/103/104 only, E required, precision aim/blocking bits/caller/capture/Vanilla refusals)");
    }
    // D08Z1: before Duke's own ballistic update the host sweeps the flight in
    // isolation; a wall (result 1) leaves the motion along it plus a small
    // outward drift, a ceiling while rising (result 3) stops the rise, a corner
    // still in the way after three passes stops the horizontal travel. Clear
    // paths, falls into a ceiling, other callers, a short guest stack and
    // Vanilla are untouched.
    {
        const auto saved_anim=psx_mod_read_half(p+0x60);
        psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);psx_mod_write_half(p+0x60,107);
        auto v=[&](unsigned o){return int32_t(psx_mod_read_word(p+o));};
        auto fly=[&](int32_t vx,int32_t vy,int32_t vz,int result,int hits,int32_t nx,int32_t nz,uint32_t ra=0x80055934,uint32_t sp=0x801fff00){
            psx_mod_write_word(p+0x1f4,uint32_t(vx));psx_mod_write_word(p+0x1f8,uint32_t(vy));psx_mod_write_word(p+0x1fc,uint32_t(vz));
            jw_result=result;jw_hits=hits;jw_nx=nx;jw_nz=nz;jw_sweeps=0;
            call(0x8003ebf4,p,0,ra,sp);};
        fly(0,-3000,10000,1,1,0,-4096);   // head-on: stops at the wall, keeps the arc
        assert(v(0x1f4)==0 && v(0x1f8)==-3000 && v(0x1fc)==-128 && jw_sweeps==2);
        assert(std::strstr(ttk::controls_debug_json(),"\"jump_walls\":{\"slide\":true,\"slides\":1,") &&
               std::strstr(ttk::controls_debug_json(),"\"age\":0}"));
        fly(10000,500,10000,1,1,0,-4096);  // 45 degrees: keeps the motion along it
        assert(v(0x1f4)==10000 && v(0x1f8)==500 && v(0x1fc)==-128);
        fly(-7000,0,7000,1,1,2896,-2896);  // a diagonal wall
        assert(std::abs(v(0x1f4)-91)<=2 && std::abs(v(0x1fc)+91)<=2);
        fly(10000,0,0,1,1,0,-4096);       // along it and still touching: pushed off
        assert(v(0x1f4)==10000 && v(0x1fc)==-256 && std::strstr(ttk::controls_debug_json(),"\"nudges\":1,"));
        fly(0,0,10000,1,3,0,-4096);       // a convex corner: turned 15 degrees toward the slide clears it
        assert(jw_sweeps==4 && std::abs(v(0x1f4)+2500)<=3 && std::abs(v(0x1fc)-9330)<=3 &&
               std::strstr(ttk::controls_debug_json(),"\"turns\":1,"));
        fly(0,-500,10000,1,99,0,-4096);   // nothing clears: pushed straight off it, never into it
        assert(jw_sweeps==17 && v(0x1f4)==0 && v(0x1fc)==-1200 && v(0x1f8)==-500 && std::strstr(ttk::controls_debug_json(),"\"stops\":1,"));
        // Safety net: the original bounce's rerun of the integration (ra
        // 0x8003ef68, its caller the wall case 0x800559f8 at sp+0x24) gets the
        // pre-pass velocity minus the part into the wall, a push off it, the
        // update's own vertical speed and timer; the 107 it writes is undone
        // at the contact sound call that follows it.
        {
            psx_mod_write_half(p+0x60,103);psx_mod_write_half(p+0x68,7);psx_mod_write_half(p+0x6a,9);
            psx_mod_write_word(p+0x200,1234);
            fly(3000,-2000,8000,0,0,0,0);   // clear pre-pass: remembered as this update's velocity
            psx_mod_write_word(p+0x194,0);psx_mod_write_word(p+0x19c,uint32_t(-4096));
            psx_mod_write_word(p+0x1f4,uint32_t(-1500));psx_mod_write_word(p+0x1f8,0);psx_mod_write_word(p+0x1fc,uint32_t(-4000));
            psx_mod_write_word(p+0x200,1300);
            psx_mod_write_word(0x801ffe00+0x24,0x800559f8);
            call(0x8003ebf4,p,0,0x8003ef68,0x801ffe00);
            assert(v(0x1f4)==3000 && v(0x1fc)==-300 && v(0x1f8)==-2000 && v(0x200)==1234);
            psx_mod_write_half(p+0x60,107);psx_mod_write_half(p+0x68,0);psx_mod_write_half(p+0x6a,0);
            // D08Z2: not at an animation runner (the update has started 107 on
            // its tracks by then), only at the contact sound right after the write.
            call(0x80059db0,p+0x74,p,0x8005a5a8,0x801fff00);assert(psx_mod_read_half(p+0x60)==107);
            call(0x8006bbd8,0x2008,0,0x80055a2c,0x801fff00);assert(psx_mod_read_half(p+0x60)==107);
            call(0x8006bbd8,0x2008,0,0x80055a10,0x801fff00);
            assert(psx_mod_read_half(p+0x60)==103 && psx_mod_read_half(p+0x68)==7 && psx_mod_read_half(p+0x6a)==9);
            psx_mod_write_half(p+0x60,107);call(0x8006bbd8,0x2008,0,0x80055a10,0x801fff00);
            assert(psx_mod_read_half(p+0x60)==107);   // once only
            // Only once per pre-pass, only from the wall case.
            psx_mod_write_word(p+0x1f4,uint32_t(-1500));call(0x8003ebf4,p,0,0x8003ef68,0x801ffe00);assert(v(0x1f4)==-1500);
            fly(3000,-2000,8000,0,0,0,0);
            psx_mod_write_word(p+0x1f4,uint32_t(-1500));psx_mod_write_word(0x801ffe00+0x24,0x80055ae4);   // the steep-ceiling bounce
            call(0x8003ebf4,p,0,0x8003ef68,0x801ffe00);assert(v(0x1f4)==-1500);
            assert(std::strstr(ttk::controls_debug_json(),"\"catches\":1,"));
            psx_mod_write_half(p+0x60,107);
        }
        fly(5000,-6000,0,3,1,0,0);         // ceiling while rising
        assert(v(0x1f4)==5000 && v(0x1f8)==0 && v(0x1fc)==0 && std::strstr(ttk::controls_debug_json(),"\"ceilings\":1,"));
        fly(5000,6000,0,3,1,0,0);          // falling: the original's own landing/ceiling rules
        assert(v(0x1f4)==5000 && v(0x1f8)==6000 && jw_sweeps==1);
        fly(4000,100,4000,0,0,0,0);        // clear
        assert(v(0x1f4)==4000 && v(0x1fc)==4000 && jw_sweeps==1);
        fly(0,0,10000,1,1,0,-4096,0x80055930);assert(v(0x1fc)==10000 && jw_sweeps==0);              // other caller
        fly(0,0,10000,1,1,0,-4096,0x80055934,0x801f0000);assert(v(0x1fc)==10000 && jw_sweeps==0);  // no guest stack
        psx_mod_write_byte(p+0x22c,0);fly(0,0,10000,1,1,0,-4096);assert(v(0x1fc)==10000 && jw_sweeps==0);
        psx_mod_write_byte(p+0x22c,9);
        ttk::modern=false;fly(0,0,10000,1,1,0,-4096);assert(v(0x1fc)==10000 && jw_sweeps==0);ttk::modern=true;
        for(unsigned o:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(p+o,0);
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);psx_mod_write_half(p+0x60,saved_anim);
        std::puts("PASS: D08Z1 jump wall slide (head-on, angled, diagonal, touching push-off, convex corner turn, push-off when nothing clears, bounce safety net and 107 undo, rising ceiling; falls, clear paths, caller, stack, mode and Vanilla untouched)");
    }
    // D08X boxes room: an E bounce (107) off a climbable crate whose top is in
    // mantle range above Duke's feet becomes the matching original mantle.
    {
        const uint32_t table=0x801e8000,crate=0x801e9500,list=0x801e9600,box=0x801e9700;
        const uint32_t saved_table=psx_mod_read_word(0x800d2660);
        psx_mod_write_word(0x800d2660,table);psx_mod_write_word(table+28*20,0x2090c2);
        psx_mod_write_half(crate+0x2c,20);psx_mod_write_word(crate+4,uint32_t(15937));psx_mod_write_word(crate+8,uint32_t(-6702));
        psx_mod_write_word(crate+12,73187);psx_mod_write_word(crate+0x44,list);psx_mod_write_word(list+8,box);
        const int16_t cube[]={-506,-514,-511,726,514,510,513,889};
        for(int i=0;i<8;++i)psx_mod_write_half(box+2*i,uint16_t(cube[i]));
        auto update=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
        auto bounce=[&](int y,int heading=1024){
            psx_mod_write_half(p+0x60,107);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
            psx_mod_write_word(p+4,uint32_t(15100));psx_mod_write_word(p+8,uint32_t(y));psx_mod_write_word(p+12,73748);
            psx_mod_write_half(p+0x1c,uint16_t(heading));psx_mod_write_word(p+0x174,crate);psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);
            psx_mod_write_word(p+0x1c8,uint32_t(y+507));psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);update(); // leave any old bounce
            psx_mod_write_half(p+0x60,107);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);update();
            return psx_mod_read_half(p+0x60);};
        ttk::reach_held=true;
        assert(bounce(-7103)==136 && psx_mod_read_byte(p+0x22c)==0);
        assert(int32_t(psx_mod_read_word(p+0x1c8))==-7103+507 && int32_t(psx_mod_read_word(p+0x1c4))==-7216-(-7103+507));
        assert(bounce(-7000)==137);
        assert(bounce(-6650)==139);  // top 1073 above the feet: the full climb after the line-up
        lineup_ok=0;assert(bounce(-7103)==107);lineup_ok=1; // the original line-up refuses: no mantle
        assert(bounce(-7700)==107);  // already level with the top
        ttk::reach_held=false;assert(bounce(-7103)==107);ttk::reach_held=true;
        assert(bounce(-7103,3072)==107); // facing away
        psx_mod_write_word(table+28*20,0x2);assert(bounce(-7103)==107);psx_mod_write_word(table+28*20,0x2090c2);
        ttk::modern=false;assert(bounce(-7103)==107);ttk::modern=true;
        ttk::reach_held=false;psx_mod_write_word(p+0x174,0);psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        psx_mod_write_word(p+0x1c8,0);psx_mod_write_word(p+0x1c4,0);psx_mod_write_word(0x800d2660,saved_table);
        psx_mod_write_word(p+4,0);psx_mod_write_word(p+8,0);psx_mod_write_word(p+12,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        // The original rewrites bit 0x40 of the upper-body flag entries at run
        // time (object/ledge hang on 149/152/153, the pole side probe
        // 0x800439e4 on the current animation: 196 at a chain top exit, D08V1)
        // and BONUS.OVR the low half of 265: game state, never an identity
        // loss. Any other bit of the table is still authenticated.
        const auto seq_before=ttk::input.sequence;
        for(uint32_t a:{0x800c2a78u,0x800c2a84u,0x800c2a88u,0x800c2b34u,0x800c2a74u,0x800c2824u,0x800c2c80u}) {
            const uint32_t v=psx_mod_read_word(a);psx_mod_write_word(a,v^0x40);
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
            psx_mod_write_word(a,v);
        }
        {
            const uint32_t v=psx_mod_read_word(0x800c2c48);psx_mod_write_half(0x800c2c48,1);
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
            psx_mod_write_half(0x800c2c4a,1);
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::movement_ready());
            psx_mod_write_word(0x800c2c48,v);
        }
        for(uint32_t a:{0x800c2a74u,0x800c2b34u}) for(uint32_t bit:{0x1u,0x20u,0x80u,0x10000u}) {
            const uint32_t kept=psx_mod_read_word(a);psx_mod_write_word(a,kept^bit);
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::movement_ready());
            // The verdict is rechecked every frame: restored bytes recover.
            psx_mod_write_word(a,kept);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        }
        ttk::input.sequence=seq_before;call(0x8003ade4,c,p,0x80025ee8);
        // Lifted reach retry: misses restore height and probe fields; a catch at
        // +320 keeps the original hang, starts at the real height and eases up.
        {
            auto reach=[&]{psx_mod_write_half(p+0x60,109);psx_mod_write_half(p+0x74,109);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
                psx_mod_write_word(p+8,uint32_t(-7000));psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x1c4,0);
                psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);lift_calls.clear();};
            // A reach session restarts the retry schedule; an update without E
            // ends the session.
            auto reset=[&]{const bool h=ttk::reach_held;ttk::reach_held=false;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);ttk::reach_held=h;lift_calls.clear();};
            ttk::reach_held=true;reset();reach();lift_catch_y=-100000;
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            // Budgeted: four core tries (turned at the original height, then
            // lifted 160, 320) and one lifted turned try per update; no vertical
            // velocity here, so no D08Y lowered tries.
            assert((lift_calls==std::vector<int32_t>{-7000,-7000,-7160,-7320,-7160}));
            assert(psx_mod_read_half(p+0x1c)==0 && psx_mod_read_half(p+0x24)==0);
            assert(int32_t(psx_mod_read_word(p+8))==-7000 && psx_mod_read_word(p+0x174)==0 && psx_mod_read_word(p+0x1c4)==0);
            // The next updates continue round the schedule: 480 and the wide turns.
            reach();call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            assert((lift_calls==std::vector<int32_t>{-7480,-7000,-7000,-7000,-7160}));
            reset();reach();lift_catch_y=-7300;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            assert(lift_calls.size()==4 && lift_calls[3]==-7320 && psx_mod_read_half(p+0x60)==148 && psx_mod_read_byte(p+0x22c)==6);
            assert(int32_t(psx_mod_read_word(p+8))==-7000);
            // Flush settle: entered from flight (previous mode 9) the hang root
            // eases to the ledge top (+0x1c8) + 456 and then stays.
            psx_mod_write_byte(p+0x22d,9);psx_mod_write_word(p+0x1c8,uint32_t(-7500));
            for(int i=0;i<12;++i)call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            assert(int32_t(psx_mod_read_word(p+8))==-7500+456);
            psx_mod_write_word(p+0x1c8,0);
            ttk::reach_held=false;reach();call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(lift_calls.empty());
            ttk::reach_held=true;reach();ttk::input.held[ttk::original_aim]=true;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            assert(lift_calls.empty());ttk::input.held[ttk::original_aim]=false;
            reach();psx_mod_write_byte(p+0x3b8,2);call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(lift_calls.empty());
            ttk::reach_held=false;lift_catch_y=-100000;
            psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x74,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
            psx_mod_write_word(p+8,0);psx_mod_write_byte(p+0x3b8,0);
            call(0x8003ade4,c,p,0x80025ee8);
        }
        // D08Y gap jumps. E reach (109): a lowered catch of a ledge 0x60..0x4c0
        // above the feet becomes the original height mantle at the real height;
        // a lower lip, a non-ledge catch and a miss leave Duke untouched.
        {
            auto reach=[&]{psx_mod_write_half(p+0x60,109);psx_mod_write_half(p+0x74,109);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
                psx_mod_write_word(p+4,0);psx_mod_write_word(p+12,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
                psx_mod_write_word(p+8,uint32_t(-7000));psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x1c4,0);psx_mod_write_word(p+0x1c8,0);
                psx_mod_write_word(p+0x17c,0);psx_mod_write_word(p+0x1f8,900);
                psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);lift_calls.clear();held_seen.clear();};
            auto update=[&]{call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
            auto reset=[&]{const bool h=ttk::reach_held;ttk::reach_held=false;update();ttk::reach_held=h;lift_calls.clear();};
            ttk::reach_held=true;reset();reach();drop_catch_y=-6760;drop_mode=6;
            // The test's standing offset is whatever was learned; read it back
            // from the mantle's floor (+0x1c8 = feet) and check the height band.
            drop_top=-7000+507-300;update();
            // Falling: the fourth core try is lowered 240 and catches.
            assert(lift_calls.size()==4 && lift_calls[3]==-6760 && psx_mod_read_byte(p+0x22c)==0 && psx_mod_read_byte(p+0x22d)==0);
            const int32_t feet=int32_t(psx_mod_read_word(p+0x1c8)),rel=int32_t(psx_mod_read_word(p+0x1c4));
            assert(rel==drop_top-feet && rel<-0x60 && rel>=-0x4c0);
            const unsigned anim=psx_mod_read_half(p+0x60);
            assert(anim==(rel>=-0x1bf?134u:rel>=-0x23f?135u:rel>=-0x2bf?136u:137u));
            assert(int32_t(psx_mod_read_word(p+8))==-7000 && psx_mod_read_word(p+0x1f8)==0);
            // Squared to the wall at once; the mantle starts where he is and
            // glides to the catch point (111, 222) over its first updates.
            assert(psx_mod_read_half(p+0x1c)==1024 && psx_mod_read_half(p+0x24)==1024);
            const double first=std::hypot(int32_t(psx_mod_read_word(p+4)),int32_t(psx_mod_read_word(p+12)));
            assert(first>0 && first<=100);   // one glide step, not the ~248 snap
            for(int i=0;i<8;++i)update();
            assert(std::hypot(int32_t(psx_mod_read_word(p+4))-111.0,int32_t(psx_mod_read_word(p+12))-222.0)<=8);
            assert(psx_mod_read_half(p+0x60)==anim);
            assert(psx_mod_read_word(p+0x17c)==0);   // the catch's hang object is not kept
            // A lip under 0x60: no mantle, everything restored.
            reset();reach();drop_top=feet-0x40;update();
            assert(lift_calls.size()==5 && lift_calls[3]==-6760 && psx_mod_read_half(p+0x60)==109 && psx_mod_read_byte(p+0x22c)==9);
            assert(int32_t(psx_mod_read_word(p+8))==-7000 && psx_mod_read_half(p+0x24)==0 && psx_mod_read_word(p+4)==0 && psx_mod_read_word(p+0x1f8)==900);
            // A ladder or object catch (mode 3/7) is never turned into a mantle.
            for(unsigned m:{3u,7u}) {
                reset();reach();drop_mode=m;drop_top=feet-300;update();
                assert(psx_mod_read_half(p+0x60)==109 && psx_mod_read_byte(p+0x22c)==9 && psx_mod_read_word(p+0x17c)==0 && psx_mod_read_word(p+12)==0);
            }
            drop_mode=6;
            // Rising (velocity up): only the D08X retries run.
            reset();reach();psx_mod_write_word(p+0x1f8,uint32_t(-900));drop_top=feet-300;update();
            assert(lift_calls.size()==5 && psx_mod_read_half(p+0x60)==109);
            for(int32_t y:lift_calls)assert(y<=-7000);
            // Falling, three updates cover every core try (all four lowered
            // heights and all lifts) without a catch.
            reset();drop_catch_y=100000;std::vector<int32_t> seen;
            for(int i=0;i<3;++i){reach();update();seen.insert(seen.end(),lift_calls.begin(),lift_calls.end());}
            for(int32_t y:{-6880,-6760,-6640,-6520,-7160,-7320,-7480})assert(std::count(seen.begin(),seen.end(),y)>=1);
            assert(seen.size()==15 && psx_mod_read_half(p+0x60)==109);
            drop_catch_y=-6760;
            // Armed, precision aim, no E, Vanilla: no lowered retries at all.
            reach();psx_mod_write_byte(p+0x3b8,2);update();assert(lift_calls.empty());
            reach();ttk::input.held[ttk::original_aim]=true;update();assert(lift_calls.empty());ttk::input.held[ttk::original_aim]=false;
            ttk::reach_held=false;reach();update();assert(lift_calls.empty());
            ttk::modern=false;ttk::reach_held=true;reach();update();assert(lift_calls.empty());ttk::modern=true;ttk::reach_held=false;
            // Ledge forgiveness, no E, armed: a running jump (104) that bounces
            // (107) off a lip at most 0x100 above the feet starts mantle 134;
            // the held bit is presented to the isolated call only.
            const uint32_t held=0x80100000;psx_mod_write_word(0x800d1b50+4*psx_mod_read_byte(p+0x233),held);psx_mod_write_word(held,0);
            auto bounce_at=[&](int32_t lip){reach();psx_mod_write_byte(p+0x3b8,2);psx_mod_write_half(p+0x60,104);update();
                assert(lift_calls.empty());drop_top=feet-lip;drop_catch_y=-6600;psx_mod_write_half(p+0x60,107);update();};
            bounce_at(0x80);
            assert(psx_mod_read_half(p+0x60)==134 && psx_mod_read_byte(p+0x22c)==0 && int32_t(psx_mod_read_word(p+0x1c4))==-0x80);
            assert(psx_mod_read_word(held)==0 && !held_seen.empty() && held_seen.back()==1);
            assert(psx_mod_read_byte(p+0x3b8)==2 && int32_t(psx_mod_read_word(p+8))==-7000);
            // D08Z2: a weapon-owned upper pose (20, table bit 8) is handed to
            // the mantle (upper = 134, restarted) and its whole block put back
            // once the mantle ends; a full-body upper (109 above) is not touched.
            assert(psx_mod_read_half(p+0x74)==109);
            assert(psx_mod_read_word(0x800c2824+4*20)&8);
            reach();psx_mod_write_byte(p+0x3b8,2);psx_mod_write_half(p+0x60,104);update();
            const uint16_t upper_block[10]={20,7,3,4,1,0,511,32,238,0};
            for(int i=0;i<10;++i)psx_mod_write_half(p+0x74+2*i,upper_block[i]);
            drop_top=feet-0x80;drop_catch_y=-6600;psx_mod_write_half(p+0x60,107);update();
            assert(psx_mod_read_half(p+0x60)==134 && psx_mod_read_half(p+0x74)==134 && psx_mod_read_half(p+0x7c)==0 && psx_mod_read_half(p+0x7e)==0);
            update();assert(psx_mod_read_half(p+0x74)==134);   // still mantling
            psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x74,63);update();
            for(int i=0;i<10;++i)assert(psx_mod_read_half(p+0x74+2*i)==upper_block[i]);
            // Restored once only.
            psx_mod_write_half(p+0x74,63);update();assert(psx_mod_read_half(p+0x74)==63);
            // Taller than 0x100: the original bounce stays.
            bounce_at(0x180);assert(psx_mod_read_half(p+0x60)==107 && psx_mod_read_byte(p+0x22c)==9 && psx_mod_read_word(held)==0);
            // Only right after a jump: a bounce without a preceding 98/103/104 is left alone.
            reach();psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);update();
            psx_mod_write_byte(p+0x22c,9);psx_mod_write_half(p+0x60,107);drop_top=feet-0x80;update();assert(psx_mod_read_half(p+0x60)==107);
            ttk::modern=false;bounce_at(0x80);assert(psx_mod_read_half(p+0x60)==107);ttk::modern=true;
            drop_catch_y=100000;psx_mod_write_word(0x800d1b50+4*psx_mod_read_byte(p+0x233),0);
            psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x74,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
            psx_mod_write_word(p+8,0);psx_mod_write_word(p+4,0);psx_mod_write_word(p+12,0);psx_mod_write_word(p+0x1c8,0);psx_mod_write_word(p+0x1c4,0);
            psx_mod_write_word(p+0x1f8,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);psx_mod_write_byte(p+0x3b8,0);
            call(0x8003ade4,c,p,0x80025ee8);
            std::puts("PASS: D08Y gap jumps (lowered E catch -> height mantle, lip/mode/armed/Vanilla refusals; no-E bounce off a low lip -> 134, held bit restored; D08Z2 armed upper pose handed to the mantle and restored)");
        }
        std::puts("PASS: D08X crate mantle (E bounce off a climbable object in mantle range -> original 134..138; height, facing, E, flags and Vanilla refusals)");
    }
    // D14: the widescreen activation plugin selects the profile's aspect only in
    // Modernized; Vanilla, unset and off stay 4:3. auto is adaptive up to 21:9.
    {
        assert(activation);
        auto run=[&](const char* mode,const char* wide){aspect_num=aspect_den=adaptive_num=adaptive_den=0;
            if(mode)setenv("DNTTK_INPUT_MODE",mode,1);else unsetenv("DNTTK_INPUT_MODE");
            if(wide)setenv("DNTTK_WIDESCREEN",wide,1);else unsetenv("DNTTK_WIDESCREEN");
            activation();return aspect_num*100+aspect_den;};
        assert(run("modernized","16:9")==1609 && run("modernized","21:9")==2109 && run("modernized","16:10")==1610);
        assert(run("modernized",nullptr)==0 && run("modernized","off")==0 && run("modernized","16:9x")==0 && run("modernized","wide")==0);
        assert(run("modernized","auto")==1609 && adaptive_num==21 && adaptive_den==9);
        assert(run("vanilla","16:9")==0 && run("vanilla","auto")==0 && adaptive_num==0 && run(nullptr,"16:9")==0);
        unsetenv("DNTTK_INPUT_MODE");unsetenv("DNTTK_WIDESCREEN");
        std::puts("PASS: D14 widescreen activation (Modernized 16:9/16:10/21:9/auto; Vanilla, unset and off stay 4:3)");
    }
    // D14: the root view rectangle widens by the margin at the camera render's
    // projection load only; HUD layout x moves out while the status bar draws
    // and is restored by the screen-mode call after it (and by the next render).
    {
        constexpr uint32_t rect=0x800d2210,hud=0x800dd778;
        auto set_rect=[&]{psx_mod_write_half(rect,uint16_t(-256));psx_mod_write_half(rect+2,uint16_t(-120));psx_mod_write_half(rect+4,512);psx_mod_write_half(rect+6,240);};
        auto rx=[&]{return int16_t(psx_mod_read_half(rect));};auto rw=[&]{return int16_t(psx_mod_read_half(rect+4));};
        const int16_t layout[][2]={{-246,89},{-246,89},{169,89},{169,89},{-165,89},{-165,89},{-246,69},{0,0},{169,69},{169,69},{82,89},{0,0},{169,-105}};
        for(unsigned i=0;i<16;++i){psx_mod_write_half(hud+4*i,i<13?uint16_t(layout[i][0]):0);psx_mod_write_half(hud+4*i+2,i<13?uint16_t(layout[i][1]):0);}
        auto hx=[&](unsigned i){return int16_t(psx_mod_read_half(hud+4*i));};
        auto hud_intact=[&]{for(unsigned i=0;i<13;++i) if(hx(i)!=layout[i][0]) return false;return true;};
        psx_mod_write_word(0x800c27bc,1);
        ws_margin=0;set_rect();call(0x800b4d9c,386,0,0x8002e4d0);assert(rx()==-256 && rw()==512);   // 4:3
        call(0x8008ba30,0,0,0x80026590);assert(hud_intact());
        ws_margin=85;call(0x800b4d9c,386,0,0x80012345);assert(rx()==-256 && rw()==512);         // other caller
        ttk::modern=false;call(0x800b4d9c,386,0,0x8002e4d0);assert(rx()==-256);ttk::modern=true; // Vanilla
        call(0x800b4d9c,386,0,0x8002e4d0);assert(rx()==-341 && rw()==682);
        call(0x800b4d9c,386,0,0x8002e4d0);assert(rx()==-341 && rw()==682);                       // never twice
        call(0x8008ba30,0,0,0x80026590);
        assert(hx(0)==-331 && hx(2)==254 && hx(4)==-250 && hx(6)==-331 && hx(7)==0 && hx(10)==167 && hx(12)==254);
        assert(int16_t(psx_mod_read_half(hud+2))==89);
        call(0x8001fc44,2,0,0x800265ac);assert(hud_intact());
        call(0x8008ba30,0,0,0x80026590);assert(!hud_intact());call(0x800b4d9c,386,0,0x8002e4d0);assert(hud_intact());
        call(0x8008ba30,0,0,0x80099999);assert(hud_intact());                                    // other caller
        psx_mod_write_word(0x800c27bc,2);call(0x8008ba30,0,0,0x80026590);assert(hud_intact());   // split screen
        psx_mod_write_word(0x800c27bc,1);
        ws_margin=192;set_rect();call(0x800b4d9c,386,0,0x8002e4d0);assert(rx()==-448 && rw()==896); // 21:9
        call(0x8008ba30,0,0,0x80026590);assert(hx(0)==-438 && hx(2)==361);call(0x8001fc44,2,0,0x800265ac);assert(hud_intact());
        assert(ttk::widescreen_near_clip_live());ttk::modern=false;assert(!ttk::widescreen_near_clip_live());ttk::modern=true;
        ws_margin=0;assert(!ttk::widescreen_near_clip_live());set_rect();
        std::puts("PASS: D14 view rectangle and HUD corners (margin-sized, render/status-bar callers only, restored, 4:3/Vanilla/split screen untouched)");
    }
    // D23A identity memo: the verdict is reused within one host frame and
    // RAM-code generation; either changing forces the full compare. The
    // patched apartment pair passes the exact path (memcmp misses it).
    {
        assert(ttk::movement_ready());
        const uint32_t kept=psx_mod_read_word(0x80048410);
        ram[0x48410]^=0x40; // bypass the mock's generation bump
        assert(ttk::movement_ready()); // memo: same frame, same generation
        ++g_dirty_ram_code_gen;assert(!ttk::movement_ready()); // generation moved
        ram[0x48410]^=0x40;assert(psx_mod_read_word(0x80048410)==kept);
        assert(!ttk::movement_ready()); // negative verdict memoised too
        ++ttk::input.sequence;assert(ttk::movement_ready()); // new frame re-verifies
        ram[0x48410]^=0x40;assert(ttk::movement_ready());
        ++ttk::input.sequence;assert(!ttk::movement_ready()); // new frame catches it
        ram[0x48410]^=0x40;++ttk::input.sequence;assert(ttk::movement_ready());
        assert(psx_mod_read_word(0x800cc57c)==0x24020000); // patch still present: fast path fell back
        ttk::input.sequence=1;assert(ttk::movement_ready());
    }
    psx_mod_write_word(0x800cc580,0x30a30408); // half-patched pair fails identity
    assert(!ttk::movement_ready());
    call(0x8003ade4,c,p,0x80025ee8);assert(code_writes==2);
    psx_mod_write_word(0x800cc57c,0x24020008); // original overlay reloaded
    call(0x8003ade4,c,p,0x80025ee8);assert(code_writes==4 && ttk::movement_ready());
    // The LEVEL00 zone script writes a hit position into the overlay's trailing
    // scratch vector (0x800ccf1c..0x800ccf2b) on the flooded-corridor ledge.
    // That is level state, not code: identity must survive it. The last code
    // or table word before it must still be guarded.
    psx_mod_write_word(0x800ccf1c,0x000075a3);psx_mod_write_word(0x800ccf20,0xffffe687);
    psx_mod_write_word(0x800ccf24,0x0001a87e);psx_mod_write_word(0x800ccf28,0x12345678);
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    const auto last_table_word=psx_mod_read_word(0x800ccf18);
    psx_mod_write_word(0x800ccf18,last_table_word^1);assert(!ttk::movement_ready());
    psx_mod_write_word(0x800ccf18,last_table_word);
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    // D22A: the first map's portal loads LEVEL01.OVR over the same base. An
    // authenticated LEVEL01 keeps the leases; its own scratch tail is level
    // state; an unknown tag or changed body fails closed; the LEVEL00 apartment
    // patch never touches it. Optional fourth argument: the owned LEVEL01.OVR.
    if(argc>=4) {
        std::vector<uint8_t> first(ram+0xca968,ram+0xca968+12196);
        std::ifstream level01(argv[3],std::ios::binary);
        level01.read((char*)ram+0xca968,12196);assert(level01.gcount()==12196);++g_dirty_ram_code_gen;
        assert(psx_mod_read_word(0x800ca968)==4);
        const unsigned patches=code_writes;
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready() && code_writes==patches);
        psx_mod_write_word(0x800cd8fc,0x000075a3);psx_mod_write_word(0x800cd908,0x12345678);
        assert(ttk::movement_ready());
        const auto level01_last=psx_mod_read_word(0x800cd8f8);
        psx_mod_write_word(0x800cd8f8,level01_last^1);assert(!ttk::movement_ready());
        psx_mod_write_word(0x800cd8f8,level01_last);assert(ttk::movement_ready());
        psx_mod_write_word(0x800ca968,5);assert(!ttk::movement_ready()); // LEVEL02 tag, LEVEL01 body
        psx_mod_write_word(0x800ca968,3);assert(!ttk::movement_ready()); // LEVEL00 tag, LEVEL01 body
        psx_mod_write_word(0x800ca968,0x12);assert(!ttk::movement_ready()); // no such level
        psx_mod_write_word(0x800ca968,4);assert(ttk::movement_ready());
        call(0x8003ade4,c,p,0x80025ee8);assert(code_writes==patches);
        std::copy(first.begin(),first.end(),ram+0xca968);++g_dirty_ram_code_gen;
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        std::puts("PASS: D22A LEVEL01 overlay identity, scratch tail, unknown/mismatched tags, no apartment patch, LEVEL00 restore");
    }
    // D22B: every level the original level select offers. Optional fifth
    // argument: a directory of the owned LEVELxx.OVR files. Each authenticates
    // by its own tag, tolerates writes to its scratch tail (the bytes after
    // `body`), refuses a change to its last authenticated word and never gets
    // the LEVEL00 apartment patch. Unlisted overlays (LEVEL04/13/14/30) and a
    // level's tag over another level's body fail closed.
    if(argc==5) {
        struct Level {unsigned number;uint32_t size,body;};
        static const Level levels[]={{0,9668,9652},{1,12196,12180},{2,10780,10780},{3,8808,8792},{5,14148,14144},
            {6,8504,8504},{7,8660,8660},{8,3700,3700},{9,12832,12812},{10,8266,8264},{11,7704,7704},{12,13496,13496},
            {21,720,720},{22,720,720},{23,4012,4012},{24,724,724},{25,2336,2336},{26,4784,4784},{27,5712,5708},
            {28,4416,4416},{29,6356,6356}};
        constexpr uint32_t span=14148; // LEVEL05, the largest
        std::vector<uint8_t> first(ram+0xca968,ram+0xca968+span);
        auto load=[&](unsigned number,uint32_t size) {
            char path[512];std::snprintf(path,sizeof path,"%s/LEVEL%02u.OVR",argv[4],number);
            std::ifstream file(path,std::ios::binary);
            std::fill(ram+0xca968,ram+0xca968+span,0);
            file.read((char*)ram+0xca968,size);assert(uint32_t(file.gcount())==size);++g_dirty_ram_code_gen;
        };
        // The lease needs a camera update within four host frames.
        auto tick=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        uint32_t previous_tag=0;
        for(const auto& level:levels) {
            load(level.number,level.size);
            const uint32_t tag=psx_mod_read_word(0x800ca968);
            const unsigned patches=code_writes;
            tick();
            assert(ttk::movement_ready() && ttk::locomotion_input_ready());
            if(level.number==0)assert(code_writes==patches+2); // LEVEL00 alone: apartment pair
            else assert(code_writes==patches);
            for(uint32_t a=level.body;a<level.size;++a)ram[0xca968+a]^=0x5a;
            tick();assert(ttk::movement_ready());
            const uint32_t last=0x800ca968+level.body-4;
            if(level.number!=0) { // LEVEL00's last body word follows its own test above
                psx_mod_write_word(last,psx_mod_read_word(last)^1);tick();assert(!ttk::movement_ready());
                psx_mod_write_word(last,psx_mod_read_word(last)^1);tick();assert(ttk::movement_ready());
            }
            if(previous_tag) {
                psx_mod_write_word(0x800ca968,previous_tag);tick();assert(!ttk::movement_ready());
                psx_mod_write_word(0x800ca968,tag);tick();assert(ttk::movement_ready());
            }
            // D08T7 mobility is level/placement data, not just a type flag.
            // In particular LEVEL05's type-165 scenery remains fixed.
            {
                const uint32_t object=0x801e9000,table=0x801e8000,floor=0x801ea003;
                const uint32_t old_table=psx_mod_read_word(0x800d2660),old_contact=psx_mod_read_word(p+0x174),old_floor=psx_mod_read_word(p+0x48);
                psx_mod_write_word(0x800d2660,table);psx_mod_write_word(p+0x174,object);psx_mod_write_word(p+0x48,floor);
                psx_mod_write_byte(floor+4,0);psx_mod_write_word(table+28*165,0x081090e2);psx_mod_write_word(table+28*924,0x081890e2);
                psx_mod_write_half(object+0x2c,165);psx_mod_write_word(object+8,uint32_t(-9728));
                psx_mod_write_word(object+12,13829);psx_mod_write_half(object+0x1c,2048);
                assert(ttk::push_contact_ready()==(level.number==0));
                psx_mod_write_half(object+0x2c,924);psx_mod_write_word(object+8,5632);psx_mod_write_half(object+0x1c,3072);
                for(int z:{33268,32244,30196}) {psx_mod_write_word(object+12,z);assert(ttk::push_contact_ready()==(level.number==6));}
                psx_mod_write_word(object+12,30197);assert(!ttk::push_contact_ready());
                psx_mod_write_word(0x800d2660,old_table);psx_mod_write_word(p+0x174,old_contact);psx_mod_write_word(p+0x48,old_floor);
            }
            previous_tag=tag;
            if(level.number==0) { // restore the original pair for the next level
                psx_mod_write_word(0x800cc57c,0x24020008);psx_mod_write_word(0x800cc580,0x30a30408);
            }
        }
        for(unsigned number:{4u,13u,14u,30u}) {
            char path[512];std::snprintf(path,sizeof path,"%s/LEVEL%02u.OVR",argv[4],number);
            std::ifstream sized(path,std::ios::binary|std::ios::ate);const auto size=uint32_t(sized.tellg());
            load(number,size);tick();assert(!ttk::movement_ready());
        }
        std::copy(first.begin(),first.end(),ram+0xca968);++g_dirty_ram_code_gen;
        tick();assert(ttk::movement_ready());
        std::puts("PASS: D22B all 21 selectable level overlays: own tag and body, scratch tails, last body word, cross-level tags, unlisted levels refused, apartment patch LEVEL00 only");
    }
    // D22B dodge rolls (PIG FACTORY trace): each observed animation/mode pair
    // keeps the camera-only lease; the original owns the motion (no locomotion
    // lease); other modes and Vanilla stay out. (The live route checks that
    // first person stays active.)
    {
        auto camera=[&]{++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        camera();assert(ttk::movement_ready());
        static const unsigned rolls[][3]={{157,9,9},{157,0,0},{158,0,9},{158,0,0},{159,0,0},{160,9,0},{161,0,0},{161,0,9},{162,0,0}};
        for(const auto& r:rolls) {
            psx_mod_write_half(p+0x60,r[0]);psx_mod_write_byte(p+0x22c,r[1]);psx_mod_write_byte(p+0x22d,r[2]);
            camera();
            assert(ttk::traversal_camera_ready() && !ttk::locomotion_input_ready() && !ttk::movement_ready());
        }
        psx_mod_write_byte(p+0x22c,4);psx_mod_write_byte(p+0x22d,4);camera();assert(!ttk::traversal_camera_ready());
        psx_mod_write_half(p+0x60,163);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();
        assert(!ttk::traversal_camera_ready());
        psx_mod_write_half(p+0x60,158);ttk::modern=false;camera();assert(!ttk::traversal_camera_ready());ttk::modern=true;
        psx_mod_write_half(p+0x60,63);camera();assert(ttk::movement_ready());
        // Steep-slope slide (FAMILY JEWELS trace): 145 (2/9), 99 (9/9), the
        // 105 landing it leads to, then idle; a plain 105 without a slide is not.
        static const unsigned slide[][3]={{145,2,9},{143,2,2},{99,9,2},{99,9,9},{105,0,9},{105,0,0}};
        for(const auto& r:slide) {
            psx_mod_write_half(p+0x60,r[0]);psx_mod_write_byte(p+0x22c,r[1]);psx_mod_write_byte(p+0x22d,r[2]);
            camera();assert(ttk::traversal_camera_ready() && !ttk::locomotion_input_ready() && !ttk::movement_ready());
        }
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();
        assert(ttk::movement_ready() && !ttk::traversal_camera_ready());
        psx_mod_write_half(p+0x60,105);camera();assert(!ttk::traversal_camera_ready()); // latch cleared
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,2);camera();
        assert(ttk::traversal_camera_ready()); // leaving the slope as 108
        // The original's own back-steps and strafes are taken over by the full
        // lease; their landing frames keep the camera; mid-water forms only in water.
        for(unsigned a:{82u,83u,84u,85u,88u,89u,90u,91u}) {
            psx_mod_write_half(p+0x60,a);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();
            assert(ttk::movement_ready());
            psx_mod_write_byte(p+0x22d,9);camera();assert(!ttk::movement_ready() && ttk::locomotion_input_ready());
        }
        for(unsigned a:{94u,95u,106u}) { // landing poses straight after a fall keep the camera
            psx_mod_write_half(p+0x60,a);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,9);camera();
            assert(ttk::locomotion_input_ready() && !ttk::movement_ready());
            psx_mod_write_byte(p+0x22d,0);camera();assert(ttk::locomotion_input_ready() && !ttk::movement_ready());
        }
        for(unsigned a:{86u,87u,92u,93u}) {
            psx_mod_write_half(p+0x60,a);psx_mod_write_byte(p+0x22d,0);camera();assert(!ttk::movement_ready());
        }
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();assert(ttk::movement_ready());
        // Third person: a Space-only (unowned) jump keeps the camera-only lease
        // from a live lease through takeoff, flight and landing, never after a gap.
        assert(!ttk::input.first_person);
        uint32_t saved_cam[5];
        for(int i=0;i<5;++i)saved_cam[i]=psx_mod_read_word(c+(i<2?0x4c+4*i:0x64+4*(i-2)));
        psx_mod_write_word(c+0x50,p+0x7bc);psx_mod_write_word(c+0x4c,p+0x7bc); // a valid orbit
        psx_mod_write_word(c+0x64,0);psx_mod_write_word(c+0x68,0);psx_mod_write_word(c+0x6c,-3000);
        camera();assert(ttk::movement_ready());
        static const unsigned jump[][3]={{96,0,0},{97,9,0},{97,9,9},{105,0,9},{105,0,0}};
        for(const auto& r:jump) {
            psx_mod_write_half(p+0x60,r[0]);psx_mod_write_byte(p+0x22c,r[1]);psx_mod_write_byte(p+0x22d,r[2]);
            camera();assert(ttk::locomotion_input_ready() && !ttk::movement_ready());
            assert(ttk::airborne_shot_ready()==(r[1]==9 || r[2]==9 || r[0]==96)); // D07F: view-aimed air shots
        }
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();
        assert(ttk::movement_ready() && !ttk::airborne_shot_ready());
        ttk::input.sequence+=8;psx_mod_write_half(p+0x60,97);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::locomotion_input_ready() && !ttk::airborne_shot_ready());
        for(int i=0;i<5;++i)psx_mod_write_word(c+(i<2?0x4c+4*i:0x64+4*(i-2)),saved_cam[i]);
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);camera();camera();assert(ttk::movement_ready());
        std::puts("PASS: D22B third-person unowned jump camera and D07F air shots; dodge rolls 157..162 and slope slides (mode 2, 99/108, landing) keep the camera lease; original back-steps/strafes taken over; swim/other/Vanilla excluded");
    }
    for(unsigned flags : {0u,8u,0x400u,0x408u,0x10008u,0x10400u}) {
        const bool original=(flags&0x408)==8;
        const bool modernized=(flags&(psx_mod_read_word(0x800cc580)&65535))==
                              (psx_mod_read_word(0x800cc57c)&65535);
        assert(modernized==!(flags&0x400));
        if(flags&8)assert(modernized==original);
    }
    std::puts("PASS: apartment code identity, Vanilla exclusion, exact patch pair, reload, one-shot condition and untouched dialogue flags");
    psx_mod_write_half(0x800bcbb0,0);assert(!ttk::movement_ready() && !ttk::player_identity_ready());
    psx_mod_write_half(0x800bcbb0,1);psx_mod_write_half(0x800be568,1);assert(!ttk::movement_ready());
    psx_mod_write_half(0x800be568,0);psx_mod_write_half(0x800d2540,1);assert(!ttk::movement_ready());
    psx_mod_write_half(0x800d2540,0);
    // Fire draw excludes Boot, active interaction, menus and traversal.
    psx_mod_write_half(p+0x74,63);psx_mod_write_byte(p+0x3ba,4);
    assert(ttk::fire_draw_ready());
    psx_mod_write_byte(p+0x3ba,0);assert(!ttk::fire_draw_ready());
    psx_mod_write_byte(p+0x3ba,4);psx_mod_write_word(p+0x224,4);assert(!ttk::fire_draw_ready());
    psx_mod_write_word(p+0x224,0);
    // Transition speed advances only the upper equip/stow delta once; never fire.
    CPUState transition{};transition.gpr[29]=0x801effc0;transition.gpr[31]=0x8005a5a8;
    transition.gpr[4]=p+0x74;transition.gpr[5]=p;transition.gpr[6]=0x801effd2;transition.gpr[7]=1;
    for(unsigned upper : {6u,7u,13u,21u,22u,30u,31u,36u,40u}) {
        call(0x8005a210,p,0x800c2754,0x80041b34);
        psx_mod_write_half(p+0x74,upper);psx_mod_write_half(0x801effd2,1024);
        hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==1536);
        hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==1536);
    }
    call(0x8005a210,p,0x800c2754,0x80041b34);
    psx_mod_write_half(p+0x74,8);psx_mod_write_half(0x801effd2,1024);
    hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==1024);
    psx_mod_write_half(p+0x74,6);transition.gpr[31]=0x8005a3d4;
    hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==1024);
    psx_mod_write_half(p+0x74,63);psx_mod_write_byte(p+0x3ba,0);
    // D08Z: the manual jump style plays the lower-body preparation 96 three
    // times faster, once per player update; assisted and other tracks unchanged.
    {
        CPUState lower{};lower.gpr[29]=0x801effc0;lower.gpr[31]=0x8005a3d4;
        lower.gpr[4]=p+0x60;lower.gpr[5]=p;lower.gpr[6]=0x801effd0;lower.gpr[7]=0;
        auto run=[&](unsigned animation){call(0x8005a210,p,0x800c2754,0x80041b34);psx_mod_write_half(p+0x60,animation);
            psx_mod_write_half(0x801effd0,400);hooks().at(0x80059db0)(&lower,0x80059db0);return psx_mod_read_half(0x801effd0);};
        assert(run(96)==400);
        setenv("DNTTK_JUMP","manual",1);
        assert(run(96)==1200);
        hooks().at(0x80059db0)(&lower,0x80059db0);assert(psx_mod_read_half(0x801effd0)==1200);
        assert(run(63)==400 && run(97)==400);
        lower.gpr[7]=1;assert(run(96)==400);lower.gpr[7]=0;
        lower.gpr[31]=0x8005a5a8;assert(run(96)==400);lower.gpr[31]=0x8005a3d4;
        psx_mod_write_byte(p+0x22c,9);assert(run(96)==400);psx_mod_write_byte(p+0x22c,0);
        ttk::modern=false;assert(run(96)==400);ttk::modern=true;
        unsetenv("DNTTK_JUMP");psx_mod_write_half(p+0x60,63);
        std::puts("PASS: D08Z quick takeoff (manual only, 96 lower-body delta x3 once per update)");
    }
    // Idle clearance must use requested backpedal, before a walk/run callback.
    uint32_t idle_sp=0x801ef000;
    ttk::input.move_x=0;ttk::input.move_y=-1;
    psx_mod_write_word(idle_sp+0x54,0x80052228);
    psx_mod_write_word(idle_sp+0x10,0);psx_mod_write_word(idle_sp+0x18,275);
    call(0x800780b4,p,idle_sp+0x10,0x800794b8,idle_sp);
    assert((int32_t)psx_mod_read_word(idle_sp+0x18)==-275);
    assert((int32_t)psx_mod_read_word(idle_sp+0x38)==-275);
    psx_mod_write_word(idle_sp+0x54,0x8005222c);writes=0;
    call(0x800780b4,p,idle_sp+0x10,0x800794b8,idle_sp);assert(writes==0);
    psx_mod_write_word(idle_sp+0x54,0x80052228);
    ttk::input.held[ttk::original_aim]=true;writes=0;
    call(0x800780b4,p,idle_sp+0x10,0x800794b8,idle_sp);assert(writes==0);
    ttk::input.held[ttk::original_aim]=false;
    poke(0x52220);call(0x800780b4,p,idle_sp+0x10,0x800794b8,idle_sp);assert(writes==0);poke(0x52220);
    ttk::input.move_x=1;ttk::input.move_y=0;
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,100);
    writes=0;call(0x80053500,p+0x8a4,0,0x80048664);assert(writes==0); // other actor
    call(0x80053500,p,0,0x80048664,0x20);assert(writes==0); // malformed stack
    call(0x80053500,p,0,0x80048660);assert(writes==0); // wrong caller
    call(0x80053500,p,0,0x80048664);
    assert(psx_mod_read_half(p+0xfc)==100 && psx_mod_read_half(p+0x100)==0);
    assert(psx_mod_read_half(p+0x1c)==0); // movement never changes facing
    // Backpedal/strafe momentum is latched once for the verified running jump.
    psx_mod_write_half(p+0x60,104);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,1000);psx_mod_write_word(p+0x1f8,-500);
    writes=0;call(0x8003ebf4,p,0,0x80055930);assert(writes==0);
    call(0x8003ebf4,p,0,0x80055934);
    assert(psx_mod_read_word(p+0x1f4)==1000 && psx_mod_read_word(p+0x1fc)==0);
    assert((int32_t)psx_mod_read_word(p+0x1f8)==-500); // vertical velocity untouched
    psx_mod_write_word(p+0x1f4,-300);writes=0;call(0x8003ebf4,p,0,0x80055934);
    assert(writes==0 && (int32_t)psx_mod_read_word(p+0x1f4)==-300); // collision response retained
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    call(0x8003ade4,c,p,0x80025ee8);
    // Opposite run-cycle takeoff (103) must receive exactly the same correction.
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,100);
    call(0x80053500,p,0,0x80048664);
    psx_mod_write_half(p+0x60,103);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,1000);
    call(0x8003ebf4,p,0,0x80055934);call(0x8003ade4,c,p,0x80025ee8);
    assert(psx_mod_read_word(p+0x1f4)==1000 && psx_mod_read_word(p+0x1fc)==0);
    assert(ttk::locomotion_input_ready() && !ttk::movement_ready());
    ++ttk::input.epoch;assert(!ttk::locomotion_input_ready());--ttk::input.epoch;
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    call(0x8003ade4,c,p,0x80025ee8);
    // Numbered selection requests original equipment transition, never grants a weapon.
    psx_mod_write_half(p+0x74,5);ttk::input.command_serial=1;ttk::input.command_count=1;ttk::input.commands[0]=ttk::weapon_group_3;
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);psx_mod_write_word(p+0x224,0);
    psx_mod_write_half(p+0x2c4+5*4,0);writes=0;call(0x80058120,p,0,0x80041c44);assert(writes==0);
    ttk::input.command_serial=2;psx_mod_write_half(p+0x2c4+5*4,1);psx_mod_write_half(p+0x2c6+5*4,20);
    call(0x80058120,p,0,0x80041c44);
    assert(psx_mod_read_byte(p+0x3ba)==5 && psx_mod_read_byte(p+0x3b9)==4 && (psx_mod_read_word(p+0x224)&4));
    psx_mod_write_word(p+0x224,0);psx_mod_write_byte(p+0x3b8,0);ttk::input.command_count=0;
    // Redraw requires settled normal ownership and matching holstered upper pose.
    psx_mod_write_half(p+0x74,63);assert(ttk::interaction_restore_ready());
    psx_mod_write_half(p+0x74,190);assert(!ttk::interaction_restore_ready());
    psx_mod_write_half(p+0x74,63);psx_mod_write_word(p+0x224,4);assert(!ttk::interaction_restore_ready());
    psx_mod_write_word(p+0x224,0);psx_mod_write_word(p,2);assert(!ttk::interaction_alive() && !ttk::interaction_restore_ready());psx_mod_write_word(p,0);
    // Original attached traversal gets input only; movement/camera hooks stay out.
    for(auto mode : {3,6,7,8}) {
        unsigned animation=mode==8?140:149;
        psx_mod_write_half(p+0x60,animation);
        psx_mod_write_byte(p+0x22c,mode);psx_mod_write_byte(p+0x22d,mode);
        assert(ttk::traversal_input_ready() && ttk::interaction_ready() && !ttk::movement_ready() && !ttk::interaction_restore_ready());
        writes=0;call(0x80048410,p,0,0x8004b634);assert(writes==0);
        ttk::want_capture=true;ttk::input.active=false;
        const auto offered=ttk::capture_offers;
        call(0x8005a210,p,0x800c2754,0x80041b30);assert(ttk::capture_offers==offered);
        psx_mod_write_half(0x800be568,1);
        call(0x8005a210,p,0x800c2754,0x80041b34);assert(ttk::capture_offers==offered);
        psx_mod_write_half(0x800be568,0);writes=0;
        call(0x8005a210,p,0x800c2754,0x80041b34);assert(ttk::capture_offers==offered+1);
        assert(!ttk::traversal_input_ready() && writes==0); // offer is not permission to write
        ttk::want_capture=false;
        call(0x8005a210,p,0x800c2754,0x80041b34);assert(ttk::capture_offers==offered+1);
        ttk::input.active=true;
        psx_mod_write_byte(p+0x3b8,2);assert(!ttk::traversal_input_ready());psx_mod_write_byte(p+0x3b8,0);
        psx_mod_write_half(p+0x60,213);assert(!ttk::traversal_input_ready());
    }
    psx_mod_write_half(p+0x60,149);psx_mod_write_byte(p+0x22c,3);psx_mod_write_byte(p+0x22d,3);
    // Attached animation bytes can remain present while menus/scripts suspend
    // gameplay. They do not authorize WASD/E input outside live player control.
    for (uint32_t gate : {0x800bcbb0u,0x800be568u,0x800d2540u}) {
        const auto original=psx_mod_read_half(gate);
        psx_mod_write_half(gate,gate==0x800bcbb0u?0:1);
        assert(!ttk::traversal_input_ready() && !ttk::interaction_ready());
        assert(ttk::interaction_alive()==(gate!=0x800bcbb0u));
        assert(!ttk::interaction_restore_ready());
        psx_mod_write_half(gate,original);
        assert(ttk::traversal_input_ready());
    }
    poke(0x43eb8);assert(!ttk::traversal_input_ready());poke(0x43eb8);
    psx_mod_write_word(p,2);assert(!ttk::traversal_input_ready());psx_mod_write_word(p,0);
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    call(0x8003ade4,c,p,0x80025ee8);
    // Ordinary gait changes use original paired animations, never stop/traversal.
    psx_mod_write_half(p+0x60,72);ttk::running=true;
    call(0x80048410,p,0,0x8004b634);assert(psx_mod_read_half(p+0x60)==76);
    ttk::running=false;call(0x80048410,p,0,0x8004b634);assert(psx_mod_read_half(p+0x60)==72);
    psx_mod_write_half(p+0x60,73);ttk::running=true;writes=0;
    call(0x80048410,p,0,0x8004b634);assert(writes==0);ttk::running=false;
    // D08Z3: a held direction ends the landing recovery 105 into walk/run
    // (frame 0) at the ground dispatcher; a jump press with no direction
    // ends it into the stance 63. Nothing pressed, the first landing update
    // (previous mode 9), the hard landing 106, another caller, crouch,
    // precision aim, landing=original and Vanilla leave it alone.
    {
        auto landing=[&](unsigned anim,unsigned previous){
            psx_mod_write_half(p+0x60,anim);psx_mod_write_half(p+0x68,3);psx_mod_write_half(p+0x6a,40);
            psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,previous);writes=0;
        };
        const float mx=ttk::input.move_x,my=ttk::input.move_y;
        ttk::input.move_x=0;ttk::input.move_y=1;
        landing(105,0);call(0x80048410,p,0,0x8004b634);
        assert(psx_mod_read_half(p+0x60)==72 && psx_mod_read_half(p+0x68)==0 && psx_mod_read_half(p+0x6a)==0);
        ttk::running=true;landing(105,0);call(0x80048410,p,0,0x8004b634);assert(psx_mod_read_half(p+0x60)==76);ttk::running=false;
        ttk::input.move_x=-1;ttk::input.move_y=0;landing(105,0);call(0x80048410,p,0,0x8004b634);assert(psx_mod_read_half(p+0x60)==72);
        ttk::input.move_x=0;ttk::input.move_y=0;
        landing(105,0);call(0x80048410,p,0,0x8004b634);assert(writes==0 && psx_mod_read_half(p+0x60)==105);
        ttk::jump_pending=true;landing(105,0);call(0x80048410,p,0,0x8004b634);
        assert(psx_mod_read_half(p+0x60)==63 && psx_mod_read_half(p+0x6a)==0 && ttk::jump_pending);ttk::jump_pending=false;
        ttk::input.move_y=1;
        landing(105,9);call(0x80048410,p,0,0x8004b634);assert(writes==0);
        landing(106,0);call(0x80048410,p,0,0x8004b634);assert(writes==0);
        landing(105,0);call(0x80048410,p,0,0x8004b630);assert(writes==0);
        ttk::input.held[ttk::crouch]=true;landing(105,0);call(0x80048410,p,0,0x8004b634);assert(writes==0);ttk::input.held[ttk::crouch]=false;
        ttk::input.held[ttk::original_aim]=true;landing(105,0);call(0x80048410,p,0,0x8004b634);assert(writes==0);ttk::input.held[ttk::original_aim]=false;
        setenv("DNTTK_LANDING","original",1);landing(105,0);call(0x80048410,p,0,0x8004b634);assert(writes==0);unsetenv("DNTTK_LANDING");
        ttk::modern=false;landing(105,0);call(0x80048410,p,0,0x8004b634);assert(writes==0);ttk::modern=true;
        landing(105,0);call(0x80048410,p,0,0x8004b634);assert(psx_mod_read_half(p+0x60)==72);
        ttk::input.move_x=mx;ttk::input.move_y=my;
        psx_mod_write_half(p+0x68,0);psx_mod_write_half(p+0x6a,0);psx_mod_write_byte(p+0x22d,0);
        std::puts("PASS: D08Z3 landing recovery (direction to walk/run, jump to stance; idle, first update, 106, caller, crouch, aim, original and Vanilla untouched)");
    }
    // D08K9: the real initializer hook changes only a soft landing reached
    // from held crouch in this same authenticated player update. Damage and
    // water have already been decided by the original selector.
    {
        const auto saved=ttk::input;
        landing_clips=true;
        auto land=[&](unsigned next,int reject=0) {
            ttk::modern=true;ttk::input=saved;ttk::input.active=true;
            ttk::input.held[ttk::crouch]=true;ttk::input.held[ttk::original_aim]=false;
            ttk::input.held[ttk::interact]=false;
            psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,2);
            psx_mod_write_word(p+0x834,0x40000000);
            psx_mod_write_half(p+0x60,104);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
            if(reject==1)ttk::input.held[ttk::crouch]=false; // only pressed after landing
            if(reject==2)psx_mod_write_byte(p+0x22c,8);    // traversal, not a flight
            if(reject==3)setenv("DNTTK_CROUCH","original",1);
            if(reject==4)setenv("DNTTK_LANDING","original",1);
            if(reject==5)landing_clips=false;
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            psx_mod_write_half(p+0x60,next);psx_mod_write_half(p+0x68,0);
            psx_mod_write_byte(p+0x22c,next==127?5:0);psx_mod_write_byte(p+0x22d,9);
            ttk::input.held[ttk::crouch]=reject!=6;        // released before landing
            if(reject==7)++ttk::input.epoch;             // load/focus invalidates request
            if(reject==8)ttk::modern=false;
            if(reject==9)psx_mod_write_word(p+0x834,psx_mod_read_word(p+0x1c8)-800); // deep water
            if(reject==14 || reject==15) {
                psx_mod_write_word(p+0x834,psx_mod_read_word(p+0x1c8)-128);
                if(reject==15)setenv("DNTTK_SWIMMING","original",1);
            }
            if(reject==10)ttk::input.held[ttk::original_aim]=true;
            if(reject==11)poke(0x54c04);
            call(0x800493a4,p,0,reject==12?0x8005a3a0:0x8005a4d0,
                 reject==13?0x801fefb0:0x801fefc0);
            const unsigned result=psx_mod_read_half(p+0x60);
            if(reject==11)poke(0x54c04);
            unsetenv("DNTTK_SWIMMING");unsetenv("DNTTK_CROUCH");unsetenv("DNTTK_LANDING");landing_clips=true;ttk::modern=true;
            return result;
        };
        for(unsigned a:{105u,78u,84u,85u,96u})assert(land(a)==178);
        for(unsigned a:{106u,127u,227u,158u,161u,134u})assert(land(a)==a);
        for(int reject=1;reject<=15;++reject) {const unsigned result=land(105,reject);const unsigned expected=(reject==4 || reject==14)?178u:105u;if(result!=expected)std::fprintf(stderr,"D08K9 rejection %d returned %u\n",reject,result);assert(result==expected);}
        landing_clips=false;ttk::input=saved;
        psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x224,0);
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        std::puts("PASS: D08K9 same-update soft crouch landing; hard landing/death/water/traversal, release/late press, epoch, Original crouch, missing clips, code/caller/stack and Vanilla exclusions; crouch landing independent of legacy landing option");
    }
    // D08Z5: a fall started from the slope slide handler (0x8003f060, ra
    // 0x800546c4/dc/ec, mode 2) gets no launch: the table vector its flight
    // start rotates (0x80066e34, ra 0x8003e488, a1 = +0x1f4) is zeroed.
    // Other fall callers, other rotations, other modes, wall_backflip=original
    // and Vanilla keep the launch; one fall start arms one rotation only.
    {
        const uint8_t mode=psx_mod_read_byte(p+0x22c);
        auto launch=[&](uint32_t fall_ra,uint32_t rotate_ra=0x8003e488,uint32_t vector=0){
            if(!vector)vector=p+0x1f4;
            psx_mod_write_byte(p+0x22c,2);psx_mod_write_half(p+0x60,145);
            call(0x8003f060,p,0,fall_ra);
            psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1f8,0);psx_mod_write_word(p+0x1fc,uint32_t(-3040));
            call(0x80066e34,0x801effe0,vector,rotate_ra);
            return int32_t(psx_mod_read_word(p+0x1fc));
        };
        for(uint32_t ra:{0x800546c4u,0x800546dcu,0x800546ecu}) assert(launch(ra)==0 && psx_mod_read_word(p+0x1f4)==0);
        assert(std::strstr(ttk::controls_debug_json(),"\"wall_backflip\":{\"modern\":true,\"slides\":0,\"stops\":3,\"jumps\":0,\"climbs\":0,\"lifts\":0}"));
        assert(launch(0x80043f44)==-3040);                    // another fall start
        assert(launch(0x800546dc,0x8007f2e0)==-3040);         // another rotation (0x8007f224's own)
        call(0x80066e34,0x801effe0,p+0x1f4,0x8003e488);assert(psx_mod_read_word(p+0x1fc)==0); // then this fall's flight start
        psx_mod_write_word(p+0x1fc,uint32_t(-3040));
        call(0x80066e34,0x801effe0,p+0x1f4,0x8003e488);assert(int32_t(psx_mod_read_word(p+0x1fc))==-3040); // armed once
        assert(launch(0x800546dc,0x8003e488,p+0x200)==-3040); // another vector
        psx_mod_write_byte(p+0x22c,9);call(0x8003f060,p,0,0x800546dc);
        psx_mod_write_word(p+0x1fc,uint32_t(-3040));call(0x80066e34,0x801effe0,p+0x1f4,0x8003e488);
        assert(int32_t(psx_mod_read_word(p+0x1fc))==-3040);  // not from mode 2
        setenv("DNTTK_WALL_BACKFLIP","original",1);assert(launch(0x800546dc)==-3040);unsetenv("DNTTK_WALL_BACKFLIP");
        ttk::modern=false;assert(launch(0x800546dc)==-3040);ttk::modern=true;
        // The slide start after the mode 2 store is counted (diagnostics only).
        writes=0;call(0x8005446c,p,0,0x80055fc0);call(0x8005446c,p,0,0x800547fc);assert(writes==0);
        // The slide's jump branch (its facing test, ra 0x800547fc) picked 99/100,
        // the backflip: Modern starts 98 instead, in mode 2 or after the fall
        // start's 9. Only that branch's next initialization (0x800493a4 from the
        // lower-body update), only 99/100; a standing jump's 99, original and
        // Vanilla keep it.
        auto start=[&](unsigned anim,uint8_t m,bool armed,uint32_t ra=0x8005a3a0){
            if(armed)call(0x8005446c,p,0,0x800547fc);
            psx_mod_write_byte(p+0x22c,m);psx_mod_write_half(p+0x60,anim);psx_mod_write_half(p+0x68,0);psx_mod_write_half(p+0x23e,anim);
            call(0x800493a4,p,0,ra);
            return unsigned(psx_mod_read_half(p+0x60));
        };
        assert(start(99,2,false)==98 && psx_mod_read_half(p+0x23e)==98); // armed by the call above
        assert(start(100,9,true,0x8005a4d0)==98);
        assert(start(99,9,false)==99);                          // a standing jump (Down at 0x8005492c)
        assert(start(98,2,true)==98 && start(99,2,false)==99);  // facing down the slope: 98, then disarmed
        assert(start(99,2,true,0x8004b5bc)==99);                // another initializer caller
        assert(start(99,2,false)==98);                          // ... leaves it armed for the update's own
        setenv("DNTTK_WALL_BACKFLIP","original",1);assert(start(99,2,true)==99);unsetenv("DNTTK_WALL_BACKFLIP");
        ttk::modern=false;assert(start(100,2,true)==100);ttk::modern=true;
        call(0x800493a4,p,0,0x8005a3a0);
        // The 98 it starts rises and drifts up the slope only as fast as the arc
        // still leaves the face (k 0.25: uphill * tan(face) = rise / 4), on its
        // first airborne update (0x8003ebf4 from 0x80055934). From the bottom of
        // the face (mode 9, the fall's velocity) it gets the slope jump's rise.
        auto climb=[&](int32_t vy,uint32_t ra=0x80055934){
            assert(start(99,2,true)==98);
            psx_mod_write_byte(p+0x22c,9);psx_mod_write_half(p+0x2bc,64453);psx_mod_write_half(p+0x2be,uint16_t(-586));
            psx_mod_write_word(p+0x1f4,5952);psx_mod_write_word(p+0x1f8,uint32_t(vy));psx_mod_write_word(p+0x1fc,808);
            call(0x8003ebf4,p,0,ra);
        };
        auto near=[&](uint32_t at,int32_t want){return std::abs(int32_t(psx_mod_read_word(p+at))-want)<=2;};
        climb(-6917);assert(near(0x1f4,1370) && near(0x1f8,-6917) && near(0x1fc,124));
        climb(0);assert(near(0x1f4,1370) && near(0x1f8,-6917) && near(0x1fc,124));
        climb(-6917,0x80055a00);assert(near(0x1f4,5952) && near(0x1fc,808));   // another caller
        call(0x8003ebf4,p,0,0x80055934);assert(near(0x1f4,1370));               // ... the jump's own update still does
        psx_mod_write_word(p+0x1f4,5952);call(0x8003ebf4,p,0,0x80055934);
        psx_mod_write_word(p+0x1f8,uint32_t(-6917));psx_mod_write_word(p+0x1f4,5952);
        call(0x8003ebf4,p,0,0x80055934);assert(near(0x1f4,5952));               // once per jump
        psx_mod_write_word(p+0x1f8,0);psx_mod_write_byte(p+0x22c,mode);psx_mod_write_half(p+0x60,63);
        assert(std::strstr(ttk::controls_debug_json(),"\"wall_backflip\":{\"modern\":true,\"slides\":1,\"stops\":4,\"jumps\":6,\"climbs\":3,"));
        psx_mod_write_word(p+0x1fc,0);psx_mod_write_byte(p+0x22c,mode);psx_mod_write_half(p+0x60,63);
        std::puts("PASS: D08Z5 wall backflip (no launch off a slope slide, its jump starts 98 not 99/100 and climbs the slope; other falls, rotations, jumps, original and Vanilla keep theirs)");
    }
    psx_mod_write_half(p+0x60,63);
    uint32_t sp=0x801f0000-0x30-0x58;
    psx_mod_write_word(sp+0x54,0x80053548);psx_mod_write_word(sp+0x10,0);psx_mod_write_word(sp+0x18,275);
    call(0x800780b4,p,sp+0x10,0x800794b8,sp);
    assert(psx_mod_read_word(sp+0x10)==275 && psx_mod_read_word(sp+0x18)==0);
    assert(psx_mod_read_word(sp+0x30)==275 && psx_mod_read_word(sp+0x38)==0);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,25);
    call(0x80053404,p,0,0x80048620);
    assert(psx_mod_read_half(p+0xfc)==25 && psx_mod_read_half(p+0x100)==0);
    sp=0x801f0000-0x20-0x58;
    psx_mod_write_word(sp+0x54,0x80053424);psx_mod_write_word(sp+0x10,0);psx_mod_write_word(sp+0x18,275);
    call(0x800780b4,p,sp+0x10,0x800794b8,sp);
    assert(psx_mod_read_word(sp+0x10)==275 && psx_mod_read_word(sp+0x18)==0);
    ttk::input.move_x=0;
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,20);
    call(0x80054218,p,63,0x800487c4,0x801f0000,4);
    assert(psx_mod_read_half(p+0xfc)==7 && psx_mod_read_half(p+0x100)==0);
    sp=0x801f0000-0x20-0x58;
    psx_mod_write_word(sp+0x54,0x80054240);psx_mod_write_word(sp+0x10,0);psx_mod_write_word(sp+0x18,275);
    call(0x800780b4,p,sp+0x10,0x800794b8,sp);
    assert(psx_mod_read_word(sp+0x10)==275 && psx_mod_read_word(sp+0x18)==0);
    ttk::input.move_x=1;
    poke(0xca968);writes=0;call(0x80053500,p,0,0x80048664);assert(writes==0);poke(0xca968);
    poke(0x53500);writes=0;call(0x80053500,p,0,0x80048664);assert(writes==0);poke(0x53500);
    ram[(p+0x22c)&0x1fffff]=9;writes=0;call(0x80053500,p,0,0x80048664);assert(writes==0);
    ram[(p+0x22c)&0x1fffff]=0;psx_mod_write_word(p+0x224,0x20000000);writes=0;
    call(0x8003ade4,c,p,0x80025ee8);assert(writes==0 && !ttk::movement_ready());
    psx_mod_write_word(p+0x224,0);
    psx_mod_write_word(c+0x50,p+0x7bc);psx_mod_write_word(c+0x4c,p+0x7bc);
    psx_mod_write_word(c+0x64,0);psx_mod_write_word(c+0x68,0);psx_mod_write_word(c+0x6c,-3000);
    call(0x8003ade4,c,p,0x80025ee8);
    ttk::input.total_x=100;ttk::input.total_y=-50;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);
    sp=0x801f0000-0xb8;
    writes=0;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp+4);assert(writes==0);
    call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    assert(writes>0 && (int16_t)psx_mod_read_half(c+12)>0 && (int16_t)psx_mod_read_half(c+14)<0);
    assert(psx_mod_read_word(p+4)==0 && psx_mod_read_word(p+12)==0); // no position commits
    // Final look-at conversion follows requested view even when the original
    // collision-derived matrix points elsewhere. No camera position writes.
    CPUState orientation{};orientation.gpr[29]=0x801f0000-0xb8-0xa8;
    orientation.gpr[18]=c;orientation.gpr[31]=0x80029220;
    orientation.gpr[4]=orientation.gpr[29]+0x18;orientation.gpr[5]=orientation.gpr[29]+0x68;
    const auto requested_x=psx_mod_read_half(c+12);
    writes=0;hooks().at(0x8002a7fc)(&orientation,0x8002a7fc);
    assert(writes==11 && psx_mod_read_half(orientation.gpr[4]+12)==requested_x);
    assert(psx_mod_read_word(0x800c3c48)==768); // D10B temporary minimum restored
    assert(psx_mod_read_word(c+0x64)==0 && (int32_t)psx_mod_read_word(c+0x6c)==-3000);
    orientation.gpr[31]=0x80029218;writes=0;
    hooks().at(0x8002a7fc)(&orientation,0x8002a7fc);assert(writes==0);
    orientation.gpr[31]=0x80029220;orientation.gpr[18]=p;
    hooks().at(0x8002a7fc)(&orientation,0x8002a7fc);assert(writes==0);
    orientation.gpr[18]=c;poke(0x28f2c);
    hooks().at(0x8002a7fc)(&orientation,0x8002a7fc);assert(writes==0);poke(0x28f2c);
    orientation.gpr[31]=0x8002920c;orientation.gpr[4]=c;orientation.gpr[5]=c+0x54;
    hooks().at(0x8002a7fc)(&orientation,0x8002a7fc);assert(writes==9);
    // Wall bump keeps camera responsive but cannot move or rotate the actor pose.
    for (int animation: {94,95}) {
        psx_mod_write_half(p+0x60,animation);ttk::input.total_x+=20;++ttk::input.sequence;
        call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::movement_ready());
        auto before=psx_mod_read_half(c+12);writes=0;
        call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
        assert(writes>0 && psx_mod_read_half(c+12)!=before);
        writes=0;call(0x80053500,p,0,0x80048664);assert(writes==0);
    }
    psx_mod_write_half(p+0x60,63);
    auto yaw_component=psx_mod_read_half(c+12);
    call(0x8003ade4,c,p,0x80025ee8);call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    assert(psx_mod_read_half(c+12)==yaw_component); // same mouse sample once
    // D17B: expensive scenes can take more than four guest fields. Keep
    // accumulated mouse rotation even if the game's stored camera is older.
    for(unsigned gap:{5u,8u,12u}) {
        const double before=std::atan2((int16_t)psx_mod_read_half(c+12),
                                      (int16_t)psx_mod_read_half(c+16));
        psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096);
        ttk::input.total_x+=20;ttk::input.sequence+=gap;
        call(0x8003ade4,c,p,0x80025ee8);
        call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
        const double after=std::atan2((int16_t)psx_mod_read_half(c+12),
                                     (int16_t)psx_mod_read_half(c+16));
        assert(std::abs(after-before-2.4*3.141592653589793/180)<0.002);
    }
    std::puts("PASS: camera preserves mouse rotation across slow game updates");
    ttk::input.active=false;writes=0;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    assert(writes>0 && !ttk::movement_ready()); // presentation survives input release
    ttk::input.active=true;
    call(0x8002a038,0,1,0x8003af40,sp); // complete the preceding camera solve
    psx_mod_write_word(p,2);writes=0;call(0x8003ade4,c,p,0x80025ee8);call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);assert(writes==0);
    psx_mod_write_word(p,0);psx_mod_write_half(p+0x60,218);writes=0;call(0x8003ade4,c,p,0x80025ee8);assert(writes==0 && !ttk::movement_ready());
    psx_mod_write_half(p+0x60,63);
    setenv("DNTTK_CAMERA_MODE","original",1);
    call(0x8003ade4,c,p,0x80025ee8);writes=0;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);assert(writes==0);
    setenv("DNTTK_CAMERA_MODE","independent",1);setenv("DNTTK_MOUSE_INVERT_Y","1",1);setenv("DNTTK_MOUSE_SENSITIVITY","0.24",1);
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+14,0);psx_mod_write_half(c+16,4096);
    ++ttk::input.epoch;ttk::input.total_x=ttk::input.total_y=0;
    call(0x8003ade4,c,p,0x80025ee8);
    ttk::input.total_y=-50;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    assert((int16_t)psx_mod_read_half(c+14)>800); // inverted 12 degree pitch
    // A loaded world with a wall-displaced boom seeds visible +X, not anchor's +Z.
    ++save_loads;
    psx_mod_write_half(c+12,4096);psx_mod_write_half(c+14,0);psx_mod_write_half(c+16,0);
    ++ttk::input.epoch;ttk::input.total_x=ttk::input.total_y=0;
    call(0x8003ade4,c,p,0x80025ee8);call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    assert(psx_mod_read_half(c+12)==4096 && psx_mod_read_half(c+16)==0);
    setenv("DNTTK_WEAPON_AIM","view",1);
    psx_mod_write_half(c+12,4096);psx_mod_write_half(c+14,0);psx_mod_write_half(c+16,0);
    psx_mod_write_half(p+0x1e,123);psx_mod_write_half(p+0x26,456);
    call(0x80058120,p,0,0x80041c44);
    assert(psx_mod_read_half(p+0x1c)==1024 && psx_mod_read_half(p+0x24)==1024);
    assert(psx_mod_read_half(p+0x1e)==123 && psx_mod_read_half(p+0x26)==456);
    // Holstered and supported armed follow view even with backwards/sideways intent.
    ttk::input.move_y=-1;ttk::input.move_x=1;
    ram[(p+0x3b8)&0x1fffff]=2;psx_mod_write_word(p+0x224,2);ram[(p+0x3b9)&0x1fffff]=4;
    psx_mod_write_half(c+12,-4096);
    call(0x80058120,p,0,0x80041c44);assert(psx_mod_read_half(p+0x1c)==3072);
    auto refuse=[&](){writes=0;call(0x80058120,p,0,0x80041c44);assert(writes==0);};
    ttk::modern=false;refuse();ttk::modern=true;
    ttk::input.active=false;refuse();ttk::input.active=true;
    ttk::input.held[ttk::original_aim]=true;refuse();ttk::input.held[ttk::original_aim]=false;
    // D07E: original weapon aiming faces the view too; the arm stays original.
    setenv("DNTTK_WEAPON_AIM","original",1);psx_mod_write_half(c+12,4096);
    call(0x80058120,p,0,0x80041c44);assert(psx_mod_read_half(p+0x1c)==1024 && psx_mod_read_half(p+0x24)==1024);
    {
        CPUState arm{};arm.gpr[29]=0x801f0000;arm.gpr[31]=0x80097f28;
        arm.gpr[4]=0x1f800060;arm.gpr[5]=p+0x124;arm.gpr[18]=p;writes=0;
        hooks().at(0x80097a44)(&arm,0x80097a44);assert(writes==0 && arm.gpr[5]==p+0x124);
    }
    ttk::input.held[ttk::original_aim]=true;refuse();ttk::input.held[ttk::original_aim]=false;
    setenv("DNTTK_WEAPON_AIM","off",1);refuse();
    setenv("DNTTK_WEAPON_AIM","view",1);psx_mod_write_half(c+12,-4096);
    call(0x80058120,p,0,0x80041c44);assert(psx_mod_read_half(p+0x1c)==3072);
    setenv("DNTTK_CAMERA_MODE","original",1);refuse();setenv("DNTTK_CAMERA_MODE","independent",1);
    ram[(p+0x3b9)&0x1fffff]=30;refuse();ram[(p+0x3b9)&0x1fffff]=4;
    poke(0xca968);refuse();poke(0xca968);
    poke(0x58120);refuse();poke(0x58120);
    psx_mod_write_word(p,2);refuse();psx_mod_write_word(p,0);
    psx_mod_write_word(p+0x224,0x82);refuse();psx_mod_write_word(p+0x224,2);
    writes=0;call(0x80058120,p+0x8a4,0,0x80041c44);assert(writes==0);
    call(0x80058120,p,0,0x80041c40);assert(writes==0);
    CPUState arm{};arm.gpr[29]=0x801f0000;arm.gpr[31]=0x80097f28;
    arm.gpr[4]=0x1f800060;arm.gpr[5]=p+0x124;arm.gpr[18]=p;
    psx_mod_write_half(c+14,-2048);psx_mod_write_word(p+0x124,12345);
    hooks().at(0x80097a44)(&arm,0x80097a44);
    assert(arm.gpr[5]==0x801e0000 && (int32_t)psx_mod_read_word(arm.gpr[5]+4)==-2048);
    assert(psx_mod_read_word(p+0x124)==12345); // persistent aiming untouched
    arm.gpr[5]=p+0x124;arm.gpr[18]=p+0x8a4;writes=0;
    hooks().at(0x80097a44)(&arm,0x80097a44);assert(writes==0 && arm.gpr[5]==p+0x124);
    arm.gpr[18]=p;ttk::input.held[ttk::original_aim]=true;
    hooks().at(0x80097a44)(&arm,0x80097a44);assert(writes==0);
    ttk::input.held[ttk::original_aim]=false;ram[(p+0x3b8)&0x1fffff]=0;psx_mod_write_word(p+0x224,0);writes=0;
    hooks().at(0x80097a44)(&arm,0x80097a44);assert(writes==0);
    std::puts("PASS: view facing, holstered/armed, independent pitch, presentation argument and fallback guards");
    // Respawn/temporary original ownership must not ratchet the boom inward.
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x224,0);ttk::input.active=true;
    psx_mod_write_word(p,2);call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_word(c+0x64,0);psx_mod_write_word(c+0x68,0);psx_mod_write_word(c+0x6c,-300);
    psx_mod_write_word(p,0);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    sp=0x801f0000-0xb8;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
    double bx=(int32_t)psx_mod_read_word(sp+0x18),by=(int32_t)psx_mod_read_word(sp+0x1c),bz=(int32_t)psx_mod_read_word(sp+0x20)-300;
    assert(std::abs(std::sqrt(bx*bx+by*by+bz*bz)-3000)<2);
    // Preserve input across the state-first landing seam, without gait-write permission.
    ttk::input.move_x=1;ttk::input.move_y=0;ttk::input.sequence+=10;call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,160);call(0x80053500,p,0,0x80048664);
    ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,20);psx_mod_write_half(p+0xfe,123);
    call(0x80053500,p,0,0x80048664);
    double smoothed=std::hypot((int16_t)psx_mod_read_half(p+0xfc),(int16_t)psx_mod_read_half(p+0x100));
    assert(smoothed>85 && smoothed<95 && psx_mod_read_half(p+0xfe)==123);
    psx_mod_write_half(p+0x60,103);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,1000);call(0x8003ebf4,p,0,0x80055934);
    call(0x8003ade4,c,p,0x80025ee8);psx_mod_write_byte(p+0x22c,0);
    assert(ttk::locomotion_input_ready() && !ttk::movement_ready());
    // Fresh normal-camera intent is enough for a short-run-up jump, even when
    // the gait has not produced a movement callback. Never redirect twice.
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    ++ttk::input.epoch;++ttk::input.sequence;ttk::input.move_x=1;ttk::input.move_y=0;
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096);
    call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0x60,104);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,2500);psx_mod_write_word(p+0x1f8,-1000);
    call(0x8003ebf4,p,0,0x80055934);
    assert((int32_t)psx_mod_read_word(p+0x1f4)==2500 && psx_mod_read_word(p+0x1fc)==0 && (int32_t)psx_mod_read_word(p+0x1f8)==-1000);
    writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
    // The original reach animation keeps the already-owned flight lease and E.
    psx_mod_write_byte(p+0x3b8,0);psx_mod_write_half(p+0x60,109);
    ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    assert(ttk::airborne_input_ready() && ttk::interaction_ready() && !ttk::movement_ready());
    writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0); // never reinject velocity
    ++ttk::input.epoch;assert(!ttk::airborne_input_ready());
    psx_mod_write_half(p+0x60,78);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);ttk::running=true;
    uint32_t js=0x801ee000;
    psx_mod_write_word(js+0x44,0x8005398c);
    for(int i=0;i<3;++i) {psx_mod_write_word(js+0x10+4*i,1024);psx_mod_write_word(js+0x20+4*i,999);}
    std::vector<uint8_t> player_before(ram+(p&0x1fffff),ram+(p&0x1fffff)+0x840);
    call(0x800780b4,p,js+0x10,0x80078c78,js);
    assert(!std::memcmp(player_before.data(),ram+(p&0x1fffff),player_before.size()));
    for(int i=0;i<3;++i) {assert(psx_mod_read_word(js+0x10+4*i)==0);assert(psx_mod_read_word(js+0x20+4*i)==psx_mod_read_word(p+4+4*i));}
    writes=0;ttk::running=false;call(0x800780b4,p,js+0x10,0x80078c78,js);assert(writes==0);
    ttk::running=true;psx_mod_write_word(js+0x44,0x80053990);writes=0;
    call(0x800780b4,p,js+0x10,0x80078c78,js);assert(writes==0);
    psx_mod_write_word(js+0x44,0x8005398c);poke(0x78c0c);writes=0;
    call(0x800780b4,p,js+0x10,0x80078c78,js);assert(writes==0);poke(0x78c0c);
    // D08Y: a large drop at the 1024-ahead endpoint (sp+0x20) leaves the
    // original look-ahead alone (it queues the jump for the edge); the probe's
    // player fields are restored. A small drop still jumps now.
    // (original_call needs a stack inside the guest's, so use one here)
    const uint32_t ks=0x801fe000;psx_mod_write_word(ks+0x44,0x8005398c);
    for(int i=0;i<3;++i) {psx_mod_write_word(ks+0x10+4*i,1024);psx_mod_write_word(ks+0x20+4*i,999);}
    psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x1c4,77);lookahead_drop=2048;writes=0;
    call(0x800780b4,p,ks+0x10,0x80078c78,ks);
    for(int i=0;i<3;++i) {assert(psx_mod_read_word(ks+0x10+4*i)==1024);assert(psx_mod_read_word(ks+0x20+4*i)==999);}
    assert(psx_mod_read_word(p+0x1c4)==77);
    lookahead_drop=700;call(0x800780b4,p,ks+0x10,0x80078c78,ks);
    for(int i=0;i<3;++i) assert(psx_mod_read_word(ks+0x10+4*i)==0);
    assert(psx_mod_read_word(p+0x1c4)==77);lookahead_drop=-100000;
    // D08Z: the manual style jumps now before a large drop as well.
    setenv("DNTTK_JUMP","manual",1);
    for(int i=0;i<3;++i) {psx_mod_write_word(ks+0x10+4*i,1024);psx_mod_write_word(ks+0x20+4*i,999);}
    lookahead_drop=2048;call(0x800780b4,p,ks+0x10,0x80078c78,ks);
    for(int i=0;i<3;++i) {assert(psx_mod_read_word(ks+0x10+4*i)==0);assert(psx_mod_read_word(ks+0x20+4*i)==psx_mod_read_word(p+4+4*i));}
    lookahead_drop=-100000;
    // D08Z air steering: an owned flight bends toward camera-relative WASD at
    // up to the takeoff speed; no input, precision aim or assisted: no writes.
    {
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        ++ttk::input.epoch;++ttk::input.sequence;ttk::input.move_x=1;ttk::input.move_y=0;
        psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096);
        call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_half(p+0x60,104);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,0);psx_mod_write_half(p+0xba,11);
        psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,10000);psx_mod_write_word(p+0x1f8,-1000);
        call(0x8003ebf4,p,0,0x80055934);
        assert((int32_t)psx_mod_read_word(p+0x1f4)==10000 && psx_mod_read_word(p+0x1fc)==0);
        ttk::input.move_x=0;ttk::input.move_y=1;
        call(0x8003ebf4,p,0,0x80055934);
        const int32_t vx=psx_mod_read_word(p+0x1f4),vz=psx_mod_read_word(p+0x1fc);
        assert(vz>900 && vx<10000 && std::hypot((double)vx,(double)vz)<=10001 && (int32_t)psx_mod_read_word(p+0x1f8)==-1000);
        // Repeated steering converges on the request without exceeding the cap.
        for(int i=0;i<40;++i) call(0x8003ebf4,p,0,0x80055934);
        assert(std::abs((int32_t)psx_mod_read_word(p+0x1f4))<50 && std::abs((int32_t)psx_mod_read_word(p+0x1fc)-10000)<50);
        ttk::input.move_y=0;writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
        ttk::input.move_x=-1;ttk::input.held[ttk::original_aim]=true;writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
        ttk::input.held[ttk::original_aim]=false;
        // The E reach 109 keeps steering in manual only.
        psx_mod_write_half(p+0x60,109);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
        writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes>0);
        unsetenv("DNTTK_JUMP");
        writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
        psx_mod_write_half(p+0x60,104);writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
        // Pure step: a reversal at the cap never gains speed.
        auto v=ttk::air_steer({3000,0},{-1,0},3000,420);assert(v.x==2580 && v.z==0);
        v=ttk::air_steer({3000,0},{0,1},3000,420);assert(std::hypot(v.x,v.z)<3000 && v.z>0 && v.x<3000);
        v=ttk::air_steer({0,2900},{0,1},3000,420);assert(v.x==0 && v.z==3000);
        psx_mod_write_half(p+0x60,78);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
        std::puts("PASS: D08Z manual jump (look-ahead jumps now before gaps, bounded air steering incl. reach, assisted unchanged)");
    }
    // D08A: all role groups, availability, upgraded ammo, remote and success history.
    ttk::input={};ttk::input.active=true;ttk::input.sequence=10000;ttk::input.epoch=100;
    psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);
    psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x74,5);
    psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);
    call(0x8003ade4,c,p,0x80025ee8);
    for(unsigned i=0;i<30;++i){psx_mod_write_half(p+0x2c4+4*i,1);psx_mod_write_half(p+0x2c6+4*i,20);}
    auto request=[&](int action){psx_mod_write_word(p+0x224,0);ttk::input.commands[0]=action;
        ttk::input.command_count=1;++ttk::input.command_serial;call(0x80058120,p,0,0x80041c44);};
    const unsigned expected[]={0,4,5,7,8,12,10,9,6,11};
    for(unsigned i=0;i<10;++i) {psx_mod_write_byte(p+0x3b9,i==1?5:4);request(ttk::weapon_group_1+i);
        assert(psx_mod_read_byte(p+0x3ba)==expected[i]);}
    psx_mod_write_byte(p+0x3b9,12);request(ttk::weapon_group_6);assert(psx_mod_read_byte(p+0x3ba)==14);
    psx_mod_write_byte(p+0x3b9,14);request(ttk::weapon_group_6);assert(psx_mod_read_byte(p+0x3ba)==13);
    psx_mod_write_byte(p+0x3b9,4);psx_mod_write_half(p+0x2c6+12*4,0);psx_mod_write_word(p+0x254,1);
    request(ttk::weapon_group_6);assert(psx_mod_read_byte(p+0x3ba)==12);
    psx_mod_write_byte(p+0x3b8,1);psx_mod_write_byte(p+0x3b9,0);psx_mod_write_half(p+0x74,39);
    request(ttk::weapon_group_2);assert(psx_mod_read_byte(p+0x3ba)==4); // leave the original detonator equipment state
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);psx_mod_write_half(p+0x74,5);

    psx_mod_write_word(p+0x254,0);request(ttk::weapon_group_6);assert(psx_mod_read_byte(p+0x3ba)==14);
    psx_mod_write_half(p+0x2c4+7*4,9);psx_mod_write_half(p+0x2c6+7*4,0);
    request(ttk::weapon_group_4);assert(psx_mod_read_byte(p+0x3ba)==7);
    psx_mod_write_half(p+0x2c6+28*4,0);psx_mod_write_byte(p+0x3ba,4);
    request(ttk::weapon_group_4);assert(psx_mod_read_byte(p+0x3ba)==4 && !(psx_mod_read_word(p+0x224)&4));
    psx_mod_write_byte(p+0x3b9,5);request(ttk::weapon_last);assert(psx_mod_read_byte(p+0x3ba)==4);
    psx_mod_write_byte(p+0x3b9,7);psx_mod_write_half(p+0x74,30);request(ttk::weapon_group_2);
    psx_mod_write_byte(p+0x3b9,5);psx_mod_write_half(p+0x74,5);request(ttk::weapon_last);
    assert(psx_mod_read_byte(p+0x3ba)==4); // interrupted draw must not become history

    psx_mod_write_half(p+0x74,42);psx_mod_write_byte(p+0x3ba,5);
    request(ttk::weapon_group_2);assert(psx_mod_read_byte(p+0x3ba)==5); // charged throw refused
    psx_mod_write_half(p+0x74,5);ttk::input.held[ttk::fire]=true;
    request(ttk::weapon_group_2);assert(psx_mod_read_byte(p+0x3ba)==5);ttk::input.held[ttk::fire]=false;
    // No medkit ownership means no request. An owned dose uses original item equipment.
    psx_mod_write_half(p+0x368,0);request(ttk::medkit);assert(!(psx_mod_read_word(p+0x224)&12));
    psx_mod_write_half(p+0x368,1);psx_mod_write_half(p+0x36a,100);
    request(ttk::medkit);assert(psx_mod_read_byte(p+0x3bb)==5 && (psx_mod_read_word(p+0x224)&12)==12);
    // Shared original menu selection, cycle-only immutability, zero/one/many,
    // active empty toggles, and depletion. No synthetic animation acceptance.
    assert(psx_mod_read_half(0x800c3f94)==5);
    for(unsigned id:{1u,2u,3u}){psx_mod_write_half(p+0x354+4*id,1);psx_mod_write_half(p+0x356+4*id,40);}
    uint8_t items_before[24];std::memcpy(items_before,ram+(p&0x1fffff)+0x354,24);
    for(unsigned id:{1u,2u,3u,5u,1u}) {request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==id);}
    request(ttk::item_previous);assert(psx_mod_read_half(0x800c3f94)==5);
    assert(!std::memcmp(items_before,ram+(p&0x1fffff)+0x354,24));
    psx_mod_write_half(0x800c3f94,3);request(ttk::item_previous);assert(psx_mod_read_half(0x800c3f94)==2);
    psx_mod_write_half(p+0x35e,0);request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==1); // depletion first falls back to medkit, then cycles
    for(unsigned id:{1u,2u,3u})psx_mod_write_half(p+0x354+4*id,0);
    request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==5);
    request(ttk::item_previous);assert(psx_mod_read_half(0x800c3f94)==5);
    psx_mod_write_half(p+0x368,0);request(ttk::item_use);assert(psx_mod_read_half(0x800c3f94)==0 && !(psx_mod_read_word(p+0x224)&12));
    psx_mod_write_half(p+0x358,3);psx_mod_write_half(p+0x35a,0);
    request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==1); // active can be switched off
    psx_mod_write_half(p+0x358,0);
    {
        // D08A4: held steroids (bit 0 with an amount, the original's own owned
        // item) join the cycle after the medkit; R, and U on them, set the
        // running bit (the original menu's toggle) and play the pickup sound,
        // with no equipment request. Running or none held: R does nothing.
        psx_mod_write_half(p+0x368,1);psx_mod_write_half(p+0x36a,100);psx_mod_write_half(0x800c3f94,5);
        psx_mod_write_half(p+0x364,1);psx_mod_write_half(p+0x366,9000);
        const uint16_t health=psx_mod_read_half(p+0x32);psx_mod_write_half(p+0x32,10000);
        // A stack in range for the original sound call.
        auto press=[&](int action){psx_mod_write_word(p+0x224,0);ttk::input.commands[0]=action;
            ttk::input.command_count=1;++ttk::input.command_serial;call(0x80058120,p,0,0x80041c44,0x801ff000);};
        request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==4 && psx_mod_read_half(p+0x364)==1);
        request(ttk::item_previous);assert(psx_mod_read_half(0x800c3f94)==5);
        steroid_sounds.clear();
        press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==3 && psx_mod_read_half(p+0x366)==9000);
        assert(!(psx_mod_read_word(p+0x224)&12) && steroid_sounds.size()==1 && steroid_sounds[0][0]==0x100f &&
               steroid_sounds[0][1]==p+4 && steroid_sounds[0][2]==0x800 && steroid_sounds[0][3]==uint32_t(-10));
        press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==3 && steroid_sounds.size()==1); // running
        psx_mod_write_half(p+0x364,1);psx_mod_write_half(0x800c3f94,4);
        press(ttk::item_use);assert(psx_mod_read_half(p+0x364)==3 && !(psx_mod_read_word(p+0x224)&12));
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);
        press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==0 && steroid_sounds.size()==2);
        psx_mod_write_half(p+0x364,1);psx_mod_write_half(p+0x366,0);  // owned but empty
        press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==1);
        psx_mod_write_half(p+0x366,9000);psx_mod_write_half(p+0x32,0);  // dead
        press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==1);psx_mod_write_half(p+0x32,10000);
        ttk::modern=false;press(ttk::steroids);assert(psx_mod_read_half(p+0x364)==1);ttk::modern=true;
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);psx_mod_write_half(p+0x368,0);psx_mod_write_half(p+0x32,health);
        std::puts("PASS: D08A4 held steroids: cycle, R and U use (running bit, sound, no request), running/none/empty/dead/Vanilla refused");
    }
    request(ttk::quick_kick);assert(!(psx_mod_read_word(p+0x224)&12));
    assert(kicks==0);psx_mod_write_half(p+0x74,63);psx_mod_write_byte(p+0x3b8,0);psx_mod_write_byte(p+0x3ba,0);
    ttk::input.commands[0]=ttk::quick_kick;++ttk::input.command_serial;
    call(0x80058120,p,0,0x80041c44,0x801ff000);assert(kicks==1);
    psx_mod_write_half(p+0x74,42);++ttk::input.command_serial;
    call(0x80058120,p,0,0x80041c44,0x801ff000);assert(kicks==1);
    psx_mod_write_half(p+0x74,63);
    psx_mod_write_byte(p+0x3ba,4);++ttk::input.command_serial;
    call(0x80058120,p,0,0x80041c44,0x801ff000);assert(kicks==1);
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_half(p+0x74,5);
    {
        // D08Q5: in flight (mode 10) weapon groups, the wheel and X reach the
        // original request; items other than the pack, the kick, fire held,
        // draws and Vanilla stay refused.
        psx_mod_write_half(p+0x358,3);psx_mod_write_half(p+0x35a,9000);
        psx_mod_write_byte(p+0x22c,10);psx_mod_write_byte(p+0x22d,10);psx_mod_write_half(p+0x60,164);
        psx_mod_write_byte(p+0x3b9,4);psx_mod_write_byte(p+0x3ba,4);
        ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
        assert(ttk::jetpack_input_ready() && !ttk::movement_ready());
        request(ttk::weapon_group_3);assert(psx_mod_read_byte(p+0x3ba)==5 && (psx_mod_read_word(p+0x224)&4));
        psx_mod_write_byte(p+0x3ba,4);request(ttk::weapon_next);assert(psx_mod_read_byte(p+0x3ba)==5);
        psx_mod_write_byte(p+0x3b9,5);psx_mod_write_byte(p+0x3ba,5);request(ttk::weapon_previous);
        assert(psx_mod_read_byte(p+0x3ba)==4);
        psx_mod_write_byte(p+0x3ba,5);psx_mod_write_half(p+0x368,1);psx_mod_write_half(p+0x36a,100);
        request(ttk::medkit);assert(!(psx_mod_read_word(p+0x224)&12) && psx_mod_read_byte(p+0x3ba)==5);
        request(ttk::quick_kick);assert(kicks==1 && !(psx_mod_read_word(p+0x224)&12));
        ttk::input.held[ttk::fire]=true;request(ttk::weapon_group_5);assert(psx_mod_read_byte(p+0x3ba)==5);
        ttk::input.held[ttk::fire]=false;
        psx_mod_write_half(p+0x74,22);request(ttk::weapon_group_5);assert(psx_mod_read_byte(p+0x3ba)==5);
        psx_mod_write_half(p+0x74,5);
        ttk::modern=false;request(ttk::weapon_group_5);assert(psx_mod_read_byte(p+0x3ba)==5);ttk::modern=true;
        request(ttk::weapon_group_5);assert(psx_mod_read_byte(p+0x3ba)==8);
        psx_mod_write_half(p+0x358,0);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x3b9,4);psx_mod_write_byte(p+0x3ba,4);
        psx_mod_write_word(p+0x224,0);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
        assert(ttk::movement_ready());
        std::puts("PASS: D08Q5 flight weapon groups, wheel and refusals (items, kick, fire, draw, Vanilla)");
    }

    // Stance request has no Triangle/turn write; original animation delta is accelerated once.
    ttk::input.held[ttk::crouch]=true;ttk::input.command_count=0;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
    assert(psx_mod_read_half(p+0x60)==176 && psx_mod_read_half(p+0x6a)==0);
    CPUState stance{};stance.gpr[4]=p+0x60;stance.gpr[5]=p;stance.gpr[6]=0x801fefd0;
    stance.gpr[29]=0x801fefc0;stance.gpr[31]=0x8005a3d4;psx_mod_write_half(0x801fefd0,30);
    hooks().at(0x80059db0)(&stance,0x80059db0);assert(psx_mod_read_half(0x801fefd0)==120);
    hooks().at(0x80059db0)(&stance,0x80059db0);assert(psx_mod_read_half(0x801fefd0)==120);
    psx_mod_write_half(p+0x60,178);ttk::input.held[ttk::crouch]=false;clearance_result=500;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);assert(psx_mod_read_half(p+0x60)==178);
    clearance_result=2048;ttk::input.active=false;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
    assert(psx_mod_read_half(p+0x60)==176 && psx_mod_read_half(p+0x6a)==1); // focus release also stands
    // D08K11: shallow water is grounded crouch territory, including a body
    // below the plane. Native swim/deep water and Original crouch stay out.
    {
        const auto saved_water=psx_mod_read_word(p+0x834),saved_flag=psx_mod_read_word(p+0x20c);
        const auto saved_y=psx_mod_read_word(p+8),saved_floor=psx_mod_read_word(p+0x1c8);
        for(int test=0;test<8;++test) {
            ttk::input.active=true;ttk::input.held[ttk::crouch]=false;
            psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
            psx_mod_write_word(p+0x224,0);psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x20c,0);
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            const int depth=test==1?512:test==2?640:test==3?641:128;
            psx_mod_write_word(p+0x834,-depth);psx_mod_write_word(p+0x1c8,0);
            psx_mod_write_word(p+8,-282);psx_mod_write_word(p+0x20c,0x3b800);
            if(test==1 || test==2) {psx_mod_write_byte(p+0x22c,1);psx_mod_write_byte(p+0x22d,1);}
            if(test==4)psx_mod_write_byte(p+0x22c,4);
            if(test==5)psx_mod_write_byte(p+0x22c,5);
            if(test==6)setenv("DNTTK_CROUCH","original",1);
            if(test==7)ttk::modern=false;
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
            ttk::input.held[ttk::crouch]=true;
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            assert(psx_mod_read_half(p+0x60)==(test<3?176:63));
            assert(psx_mod_read_word(p+0x20c)==0x3b800); // never force submersion
            assert(psx_mod_read_byte(p+0x22c)==(test==4?4:test==5?5:(test==1 || test==2)?1:0));
            if(test<3) {
                psx_mod_write_half(p+0x60,178);
                call(0x80048410,p,0,0x8004b634,0x801ff000);
                assert(psx_mod_read_word(0x800548e8)==(test?0x10000008u:0x14400008u));
                // The next hook restores before checking identity, even in Vanilla.
                ttk::modern=false;call(0x80058120,p,0,0x80041c44,0x801ff000);
                assert(psx_mod_read_word(0x800548e8)==0x14400008u);
                ttk::modern=true;
                // A held crouch crossing into deep water plays the normal
                // exit so native swim checks are no longer excluded by 178.
                psx_mod_write_word(p+0x834,-800);clearance_result=500;
                call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
                assert(psx_mod_read_half(p+0x60)==178); // keep clearance
                clearance_result=2048;
                call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
                assert(psx_mod_read_half(p+0x60)==176 && psx_mod_read_half(p+0x6a)==1);
                assert(psx_mod_read_word(p+0x20c)==0x3b800);
            }
            ttk::modern=true;unsetenv("DNTTK_CROUCH");
        }
        psx_mod_write_word(p+0x834,saved_water);psx_mod_write_word(p+0x20c,saved_flag);
        psx_mod_write_word(p+8,saved_y);psx_mod_write_word(p+0x1c8,saved_floor);
        ttk::input.held[ttk::crouch]=false;ttk::input.active=false;
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);
        std::puts("PASS: D08K11 shallow/mid water crouch, submerged body, deep/swim/Original/Vanilla exclusions, native water mask preserved");
    }
    // D08K2 crouch probe via the debug mover (crouchgo): a crouched Duke (178)
    // steps by (20, 0) per update when the crouch hull fits, with the head ray
    // at feet - 0x2c0; each refusal reason leaves him where he was.
    {
        bool close=false;
        const auto say=[](const char*){};
        uint32_t keep[5];for(int i=0;i<4;++i)keep[i]=psx_mod_read_word(p+4+4*i);keep[4]=psx_mod_read_word(p+0x1c8);
        assert(ttk::crouch_console_command("crouchgo 20 0",say,close) && close);
        // An unowned 178 with crouch released: crouch.inc neither requests nor
        // releases (one update at 63 drops the earlier fixture's ownership).
        ttk::input.held[ttk::crouch]=false;psx_mod_write_half(p+0x60,63);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);psx_mod_write_half(p+0x60,178);
        psx_mod_write_word(p+0x1c8,uint32_t(-1000));psx_mod_write_word(p+8,uint32_t(-1500));
        auto step=[&](int clearance,int32_t floor_step,uint32_t object,int ray,int steep){
            clearance_result=clearance;lookahead_drop=floor_step;hull_object=object;hull_ray_hit=ray;hull_steep=steep;hull_ray_y.clear();
            const int32_t x0=int32_t(psx_mod_read_word(p+4));
            if(psx_mod_read_half(p+0x60)!=63)psx_mod_write_half(p+0x60,178);
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            const int32_t moved=int32_t(psx_mod_read_word(p+4))-x0;
            return moved;
        };
        assert(step(0x2e0,0,0,0,0)==20);                     // fits crouched (704) but not standing (880)
        assert(hull_ray_y.size()==2 && hull_ray_y[0]==-1000-0x2c0 && hull_ray_y[1]==-1000-0x2c0);
        assert(step(0x2b0,0,0,0,0)==0);                      // too low even crouched
        assert(step(0xffff,int32_t(0xffff0000u),0,0,0)==0);  // no floor: wall
        assert(step(2048,0x180,0,0,0)==0);                   // drop over 0x100: edge
        assert(step(2048,-0x180,0,0,0)==0);                  // step up over 0x100
        // D08K7: objects as 0x8007926c judges them. A fake object (type 5) and
        // type table; flag 0x1000 marks a walkable top (the dumpsters).
        constexpr uint32_t obj=0x801c0000,types=0x801c0100;
        psx_mod_write_word(0x800d2660,types);psx_mod_write_half(obj+0x2c,5);
        const int32_t duke_x=int32_t(psx_mod_read_word(p+4));
        auto place=[&](int32_t dx,uint32_t flags,int32_t top){
            psx_mod_write_word(obj+4,uint32_t(duke_x+dx));psx_mod_write_word(obj+0xc,psx_mod_read_word(p+0xc));
            psx_mod_write_word(types+28*5,flags);hull_object_top=top;
        };
        place(100,0x1000,-0x180);
        assert(step(2048,0,obj,0,0)==0);                     // taller than a step: in the way
        assert(psx_mod_read_word(p+0x174)==0);               // restored after the refusal
        place(-100,0x1000,-0x180);
        assert(step(2048,0,obj,0,0)==20);                    // already overlapping it: stepping away is allowed
        place(0,0x1000,-0x80);
        assert(step(0x2e0,0x400,obj,0,0)==0);                // walkable top: headroom from it (0x2e0-0x400-0x80) too low
        hull_ray_y.clear();
        const int32_t top_y=int32_t(psx_mod_read_word(p+8));
        assert(step(0x800,0x400,obj,0,0)==20);               // walkable top is the floor: up 0x80 onto it
        assert(int32_t(psx_mod_read_word(p+8))==top_y-0x80 && psx_mod_read_word(p+0x174)==obj);
        assert(hull_ray_y.size()==2 && hull_ray_y[1]==-1000-0x80-0x2c0);
        psx_mod_write_word(p+8,uint32_t(top_y));psx_mod_write_word(p+0x1c8,uint32_t(-1000));
        place(0,0x1000,0);
        assert(step(0x800,0x180,obj,0,0)==20);               // its top at feet level over a 0x180 drop: no edge
        place(0,0x1000,0x180);
        assert(step(0x800,0x180,obj,0,0)==0);                // off its top over the drop: edge
        place(0,0,-0x80);
        assert(step(2048,0,obj,0,0)==20);                    // not walkable: the room decides, contact cleared
        assert(psx_mod_read_word(p+0x174)==0 && int32_t(psx_mod_read_word(p+0x1c8))==-1000);
        // Stand-up on a walkable top: 0x370 from the top to the room's ceiling
        // (the slot 10 dumpster: refused on any contact before).
        assert(ttk::crouch_console_command("crouchwalk off",say,close) && !close);
        place(0,0x1000,0);
        auto stand=[&](int clearance,int32_t room_step,uint32_t object=0x801c0000){
            clearance_result=clearance;lookahead_drop=room_step;hull_object=object;
            ttk::input.active=true;++ttk::input.sequence;ttk::input.command_count=0;
            ttk::input.held[ttk::crouch]=true;psx_mod_write_half(p+0x60,63);
            call(0x8003ade4,c,p,0x80025ee8);
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);      // owned crouch (176)
            assert(psx_mod_read_half(p+0x60)==176);
            psx_mod_write_half(p+0x60,178);ttk::input.held[ttk::crouch]=false;
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            return psx_mod_read_half(p+0x60)==176 && psx_mod_read_half(p+0x6a)==1;
        };
        assert(!stand(0x600,0x400));                         // 0x200 over the top: stays down
        assert(psx_mod_read_word(p+0x174)==0);               // query fields restored
        assert(stand(0x800,0x400));                          // 0x400 over the top: stands
        place(0,0x1000,-0x180);
        assert(!stand(0x2000,0));                            // overlapping something taller than a step
        assert(ttk::crouch_console_command("crouchstand block",say,close));
        assert(!stand(0x2000,0,0));                          // debug block
        assert(ttk::crouch_console_command("crouchstand free",say,close));
        assert(stand(0x2000,0,0));
        ttk::input.active=false;
        psx_mod_write_word(0x800d2660,0);hull_object=0;hull_object_top=0;
        for(int i=0;i<4;++i)psx_mod_write_word(p+4+4*i,keep[i]);psx_mod_write_word(p+0x1c8,keep[4]);
        psx_mod_write_word(p+0x1c8,uint32_t(-1000));psx_mod_write_word(p+8,uint32_t(-1500));
        assert(ttk::crouch_console_command("crouchgo 20 0",say,close) && close);
        ttk::input.held[ttk::crouch]=false;psx_mod_write_half(p+0x60,63);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);psx_mod_write_half(p+0x60,178);
        assert(step(2048,0,0,1,0)==0);                       // head ray hit
        assert(step(2048,-0x80,0,0,1)==0);                   // steep step up
        const int32_t y0=int32_t(psx_mod_read_word(p+8));
        assert(step(2048,0x40,0,0,0)==20);                   // step down follows the floor
        assert(int32_t(psx_mod_read_word(p+8))==y0+0x40 && int32_t(psx_mod_read_word(p+0x1c8))==-1000+0x40);
        psx_mod_write_half(p+0x60,63);assert(step(2048,0,0,0,0)==0);psx_mod_write_half(p+0x60,178);   // only while crouched (178)
        assert(ttk::crouch_console_command("crouchwalk off",say,close) && !close);
        psx_mod_write_half(p+0x60,178);assert(step(2048,0,0,0,0)==0);  // off
        clearance_result=2048;lookahead_drop=-100000;hull_object=0;hull_ray_hit=hull_steep=0;
        ttk::input.held[ttk::crouch]=false;psx_mod_write_half(p+0x60,63);
        for(int i=0;i<4;++i)psx_mod_write_word(p+4+4*i,keep[i]);psx_mod_write_word(p+0x1c8,keep[4]);
        std::puts("PASS: D08K2 crouch probe (crouch-only clearance passes, head ray at feet-0x2c0, low/wall/edge/step/object/ray/slope refusals restore state, floor follow, 178 only, off); D08K7 objects as the original walk judges them (tall refused, step away, walkable top as floor, other objects ignored) and stand-up on a walkable top");
    }
    // Distance clamps/smoothing, wall compression independence and no FOV mutation.
    ttk::input.active=true;ttk::input.held[ttk::crouch]=false;psx_mod_write_half(p+0x60,63);
    psx_mod_write_word(c+0x50,p+0x7bc);psx_mod_write_word(c+0x4c,p+0x7bc);
    auto metric=[](const char* key){auto text=std::strstr(ttk::controls_debug_json(),key);assert(text);return std::strtod(text+std::strlen(key),nullptr);};
    ttk::input.distance_total=100;++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    assert(metric("\"preferred_radius\":")==768);
    for(unsigned i=0;i<40;++i){++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);}
    assert(std::abs(metric("\"radius\":")-768)<1);
    ttk::input.distance_total=-100;++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    assert(metric("\"preferred_radius\":")==6144 && metric("\"radius\":")<6144);
    psx_mod_write_word(c+0x64,0);psx_mod_write_word(c+0x68,0);psx_mod_write_word(c+0x6c,10);
    ++ttk::input.epoch;ttk::input.distance_total=0;++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
    assert(metric("\"preferred_radius\":")==6144); // solved wall boom cannot become preference
    // D08L1: real crouch mover crosses only a collision-checked drop, keeps
    // departure feet height and starts original falling without upward speed.
    {
        const auto saved=ttk::input;
        const uint32_t saved_x=psx_mod_read_word(p+4),saved_z=psx_mod_read_word(p+12);
        for(int reject=0;reject<6;++reject) {
            ttk::input=saved;ttk::input.active=true;ttk::input.move_x=0;ttk::input.move_y=1;
            ttk::input.held[ttk::crouch]=false;ttk::input.held[ttk::interact]=false;
            ttk::input.held[ttk::original_aim]=false;
            psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);psx_mod_write_word(p+0x20c,0);
            psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_half(p+0x60,63);
            psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
            psx_mod_write_half(p+0xba,11);psx_mod_write_word(p+0x1c8,uint32_t(-1000));
            psx_mod_write_word(p+8,uint32_t(-1282));
            lookahead_drop=0;clearance_result=2048;hull_object=0;hull_ray_hit=0;
            call(0x8003ade4,c,p,0x80025ee8);
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            ttk::input.held[ttk::crouch]=reject!=5;landing_clips=true;
            if(reject==5) { // native forced kneel, no modern crouch request
                psx_mod_write_half(p+0x60,178);clearance_result=0x2e0;ttk::input.move_y=0;
            }
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            if(reject==5){assert(ttk::crouch_walk_input_ready());ttk::input.held[ttk::crouch]=true;}
            ttk::input.move_y=1;
            psx_mod_write_half(p+0x60,178);
            lookahead_drop=1024;clearance_result=1024+0x2e0;hull_ray_y.clear();
            if(reject==1)clearance_result=1024+0x2b0; // room below fits, departure does not
            if(reject==2)hull_ray_hit=1;
            if(reject==3)lookahead_drop=-384;
            if(reject==4){lookahead_drop=-65536;clearance_result=0xffff;}
            const auto before=crouch_falls;const auto feet=psx_mod_read_word(p+0x1c8);
            call(0x8005a210,p,0x800c2754,0x80041b34,0x801ff000);
            assert(crouch_falls==before+(reject==0 || reject==5));
            assert(psx_mod_read_word(p+0x1c8)==feet);
            if(reject==0 || reject==5) {
                assert(psx_mod_read_half(p+0x60)==108 && psx_mod_read_byte(p+0x22c)==9);
                assert(psx_mod_read_word(p+0x1f8)==0 && !ttk::crouch_walk_input_ready());
                assert(hull_ray_y.size()==2 && hull_ray_y[0]==hull_ray_y[1]);
            }
        }
        landing_clips=false;hull_ray_hit=0;lookahead_drop=-100000;clearance_result=2048;
        ttk::input=saved;psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);
        psx_mod_write_half(p+0xba,0);
        psx_mod_write_word(p+4,saved_x);psx_mod_write_word(p+12,saved_z);
        std::puts("PASS: D08L1 crouch walk-off uses original fall; departure clearance, wall ray, tall step and missing-floor refusals");
    }
    // D10 recenter: V swings yaw behind Duke's heading and pitch to the first
    // (original) follow pitch; mouse motion cancels a swing. Shoulder offsets
    // only the requested eye along the view's right row before the original solve.
    {
        const double quarter=6.2831853071795864769/4;
        psx_mod_write_half(p+0x1c,1024); // Duke faces +X
        auto frame=[&](){++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);};
        const double yaw0=metric("\"yaw\":");const uint16_t heading0=psx_mod_read_half(p+0x1c);
        const uint32_t camera_x0=psx_mod_read_word(c+0x64),camera_z0=psx_mod_read_word(c+0x6c);
        const double before=metric("\"recenters\":");
        ttk::input.recenter_total=1;frame();
        assert(std::strstr(ttk::controls_debug_json(),"\"recentering\":true"));
        for(unsigned i=0;i<60;++i)frame();
        assert(std::abs(metric("\"yaw\":")-quarter)<1e-6);
        assert(std::abs(metric("\"pitch\":")-metric("\"rest_pitch\":"))<1e-6);
        assert(metric("\"recenters\":")==before+1);
        // D10C look keys: Look Up turns the pitch up (negative) at 150 deg/s
        // while held and eases back to the rest pitch on release; Lock Look
        // Down turns it at 80 deg/s and keeps it; a Look tap unlocks it.
        {
            const double deg=6.2831853071795864769/360,rest=metric("\"rest_pitch\":");
            auto timed=[&](unsigned n){for(unsigned i=0;i<n;++i){ttk::clock_ms+=50;frame();}};
            using ttk::Action;
            ttk::input.held[ttk::look_up]=true;timed(4);   // the first update takes the key
            assert(std::abs(metric("\"pitch\":")-ttk::clamp_pitch(rest-150*deg*0.15))<1e-6);
            assert(std::strstr(ttk::controls_debug_json(),"\"hold\":-1"));
            ttk::input.held[ttk::look_up]=false;timed(1);
            assert(std::strstr(ttk::controls_debug_json(),"\"returning\":true"));
            timed(20);
            assert(std::abs(metric("\"pitch\":")-rest)<1e-6 && metric("\"returns\":")==1);
            ttk::input.held[ttk::lock_look_down]=true;timed(3);
            ttk::input.held[ttk::lock_look_down]=false;timed(20);
            const double locked=ttk::clamp_pitch(rest+80*deg*0.15); // the release update ends the hold
            assert(std::abs(metric("\"pitch\":")-locked)<1e-6);  // stays
            // A tap released before the next camera update is only a counted press.
            ttk::input.look_press_total+=1;timed(1);
            assert(std::strstr(ttk::controls_debug_json(),"\"taps\":1"));
            timed(20);
            assert(std::abs(metric("\"pitch\":")-rest)<1e-6 && metric("\"returns\":")==2);
            // Mouse motion during the ease back keeps the view where it is.
            ttk::input.held[ttk::look_down]=true;timed(3);ttk::input.held[ttk::look_down]=false;timed(1);
            ttk::input.total_y+=1;timed(1);
            assert(std::strstr(ttk::controls_debug_json(),"\"returning\":false"));
            const double kept=metric("\"pitch\":");timed(10);
            assert(std::abs(metric("\"pitch\":")-kept)<1e-9 && kept>rest+0.05);
            ttk::input.recenter_total+=1;for(unsigned i=0;i<60;++i)frame();
        }
        // D10D keyboard turning: Duke 3D rates, 5 build units per 30 Hz tic for
        // the first 1/8 s, then 15 (30 running); right turns yaw positive.
        {
            const double unit=30*6.2831853071795864769/2048;
            auto timed=[&](unsigned n){for(unsigned i=0;i<n;++i){ttk::clock_ms+=50;frame();}};
            auto held_angle=[&](double t,bool run){return t<0.125?5*unit*t:5*unit*0.125+15*unit*(run?2:1)*(t-0.125);};
            const double yaw0=metric("\"yaw\":");
            ttk::running=false;ttk::input.held[ttk::turn_right]=true;timed(4); // the first update takes the key
            assert(std::abs(ttk::wrap_yaw(metric("\"yaw\":")-yaw0-held_angle(0.15,false)))<1e-6);
            assert(std::strstr(ttk::controls_debug_json(),"\"key_turn\":{\"hold\":1,\"running\":false,\"turns\":1}"));
            ttk::input.held[ttk::turn_right]=false;timed(1);              // the release update ends the hold
            const double turned=metric("\"yaw\":");timed(5);
            assert(std::abs(metric("\"yaw\":")-turned)<1e-9);
            assert(std::abs(ttk::wrap_yaw(turned-yaw0-held_angle(0.2,false)))<1e-6);
            // Running doubles the rate after the slow start; left turns back.
            ttk::running=true;ttk::input.held[ttk::turn_left]=true;timed(7);
            assert(std::abs(ttk::wrap_yaw(turned-metric("\"yaw\":")-held_angle(0.3,true)))<1e-6);
            // The mouse adds on top of a held turn key.
            const double before_mouse=metric("\"yaw\":");
            ttk::input.total_x+=10;timed(1);
            assert(std::abs(ttk::wrap_yaw(before_mouse-metric("\"yaw\":")-(held_angle(0.35,true)-held_angle(0.3,true))+10*metric("\"mouse_deg\":")*6.2831853071795864769/360))<1e-4);
            ttk::input.held[ttk::turn_left]=false;ttk::running=false;timed(1);
            // A turn key stops a V swing; pitch is untouched by turning.
            ttk::input.recenter_total+=1;ttk::input.held[ttk::turn_right]=true;timed(1);
            assert(std::strstr(ttk::controls_debug_json(),"\"recentering\":false"));
            const double pitch0=metric("\"pitch\":");
            ttk::input.held[ttk::turn_right]=false;timed(2);
            assert(std::abs(metric("\"pitch\":")-pitch0)<1e-9);
            ttk::input.recenter_total+=1;for(unsigned i=0;i<60;++i)frame();
        }
        psx_mod_write_half(p+0x1c,0);ttk::input.recenter_total=2;frame();
        ttk::input.total_x+=40;frame();
        assert(std::strstr(ttk::controls_debug_json(),"\"recentering\":false"));
        const double moved=metric("\"yaw\":");assert(std::abs(moved)>0.01 && std::abs(moved-quarter)>0.01);
        // Face +X again and settle so the right row is (0,0,-1).
        ttk::input.recenter_total=3;psx_mod_write_half(p+0x1c,1024);
        for(unsigned i=0;i<60;++i)frame();
        char state[]="/tmp/ttk-d10-camera-XXXXXX";const int fd=mkstemp(state);assert(fd>=0);close(fd);std::remove(state);
        setenv("DNTTK_CAMERA_STATE_FILE",state,1);
        ttk::input.shoulder=1;
        for(unsigned i=0;i<60;++i)frame();
        const double radius_now=metric("\"radius\":"),offset=metric("\"shoulder_offset\":");
        assert(std::abs(offset-std::clamp(0.22*radius_now,192.0,640.0))<1);
        sp=0x801f0000-0xb8;
        call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
        const int32_t z=(int32_t)psx_mod_read_word(sp+0x18+8)+(int32_t)psx_mod_read_word(c+0x6c);
        const int32_t x=(int32_t)psx_mod_read_word(sp+0x18)+(int32_t)psx_mod_read_word(c+0x64);
        const int32_t tz=(int32_t)psx_mod_read_word(p+0x7bc+8),tx=(int32_t)psx_mod_read_word(p+0x7bc);
        assert(std::abs((tz-z)-offset)<2);             // eye moved to Duke's right
        assert(x<tx);                                  // and still behind him along +X
        assert(psx_mod_read_word(p+4)==0 && psx_mod_read_word(p+12)==0); // no position commits
        std::ifstream saved(state);std::string text((std::istreambuf_iterator<char>(saved)),std::istreambuf_iterator<char>());
        assert(text=="{\"camera_distance\": 6144, \"shoulder\": \"right\", \"view\": \"third\"}\n");
        std::remove(state);ttk::input.shoulder=-1;frame();
        assert(!std::ifstream(state)); // no write until the side has settled for 30 frames
        for(unsigned i=0;i<31;++i)frame();
        std::ifstream again(state);std::string left((std::istreambuf_iterator<char>(again)),std::istreambuf_iterator<char>());
        assert(left.find("\"left\"")!=std::string::npos);
        std::remove(state);unsetenv("DNTTK_CAMERA_STATE_FILE");
        ttk::input.shoulder=0;for(unsigned i=0;i<60;++i)frame();
        assert(std::abs(metric("\"shoulder_offset\":"))<1);
        // D10B: verified constraint scope, collision-first inward movement,
        // outward hysteresis, preserved preference and native constant restore.
        {
            const double preference=metric("\"preferred_radius\":");
            frame();sp=0x801f0000-0xb8;
            call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
            assert(psx_mod_read_word(0x800c3c48)==256);
            const double y=metric("\"yaw\":"),pitch=metric("\"pitch\":");
            const double forward[]={std::sin(y)*std::cos(pitch),std::sin(pitch),std::cos(y)*std::cos(pitch)};
            for(int i=0;i<3;++i) {
                double target=(int32_t)psx_mod_read_word(p+0x7bc+4*i);
                if(i==1)target+=(int32_t)psx_mod_read_word(c+0x88);
                psx_mod_write_word(sp+0x28+4*i,(int32_t)std::lround(target-forward[i]*2000));
            }
            psx_mod_write_word(sp+0x20,1500);
            auto follow=[&](uint32_t ra,int delta) {
                CPUState cpu{};cpu.gpr[4]=delta;cpu.gpr[5]=4;cpu.gpr[29]=sp;cpu.gpr[31]=ra;
                hooks().at(0x8002a038)(&cpu,0x8002a038);return cpu;
            };
            assert(follow(0x8003af40,100).gpr[5]==1);
            assert(psx_mod_read_word(0x800c3c48)==768);
            assert(follow(0x8003af80,1500).gpr[5]==1);
            assert(follow(0x8003af80,-100).gpr[4]==0);
            for(int i=0;i<10;++i)frame();
            assert(follow(0x8003af80,-100).gpr[5]>1);
            assert(follow(0x80012345,-100).gpr[5]==4);
            assert(metric("\"preferred_radius\":")==preference);
            // End at a clear solve, so later camera fixtures have no obstruction.
            psx_mod_write_word(sp+0x20,-5000);follow(0x8003af40,0);
            call(0x800348d8,c,p,0x8003769c);
            constexpr uint32_t ctx=0x800d67a8;
            const uint32_t oldflags=psx_mod_read_word(p),old224=psx_mod_read_word(p+0x224);
            const uint16_t effects[]={psx_mod_read_half(p+0x89c),psx_mod_read_half(p+0x89e),psx_mod_read_half(p+0x8a0)};
            psx_mod_write_word(p,oldflags&~0x6040u);psx_mod_write_word(p+0x224,old224&~0x200000u);
            for(int i=0;i<3;++i)psx_mod_write_half(p+0x89c+2*i,0);
            auto native_fade=[&] {psx_mod_write_word(ctx+0x48,0x02000000);psx_mod_write_word(ctx+0x4c,0x600000);};
            CPUState draw{};
            call(0x8001ca4c,0,0,0x80012345); // nested list iteration cannot end Duke
            assert(ttk::third_person_duke_drawing());
            native_fade();ttk::third_person_mesh_begin(&draw);
            assert(psx_mod_read_word(ctx+0x48)==0 && psx_mod_read_word(ctx+0x4c)==0);
            call(0x8001ca4c,0,0,0x800376b4);
            assert(psx_mod_read_word(ctx+0x48)==0x02000000);
            call(0x800348d8,c,p,0x8003769c);
            {
                uint32_t eye[3];
                for(int i=0;i<3;++i) {
                    eye[i]=psx_mod_read_word(c+0x14+4*i);
                    const int root=(int32_t)psx_mod_read_word(p+4+4*i)-(i==1?640:0);
                    psx_mod_write_word(c+0x14+4*i,root-(int32_t)std::lround(forward[i]*1000));
                }
                const double opacity=ttk::third_person_body_coverage();assert(opacity>0 && opacity<1);
                std::vector<uint8_t> image(ttk::render_state_size());ttk::render_state_save(image.data());
                const pid_t child=fork();assert(child>=0);
                if(child==0) {
                    renderer_available=false;ttk::render_state_load(image.data());
                    assert(ttk::third_person_camera_live());
                    assert(std::abs(ttk::third_person_body_coverage()-opacity)<1e-9);
                    _exit(0);
                }
                int status=0;assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
                for(int i=0;i<3;++i)psx_mod_write_word(c+0x14+4*i,eye[i]);
            }
            psx_mod_write_half(p+0x89c,1);native_fade();ttk::third_person_mesh_begin(&draw);
            assert(psx_mod_read_word(ctx+0x48)==0x02000000);
            assert(ttk::third_person_body_coverage()==1);
            call(0x8001ca4c,0,0,0x800376b4);
            psx_mod_write_word(p,oldflags);psx_mod_write_word(p+0x224,old224);
            for(int i=0;i<3;++i)psx_mod_write_half(p+0x89c+2*i,effects[i]);
            psx_mod_write_word(ctx+0x48,0);psx_mod_write_word(ctx+0x4c,0);
            std::puts("PASS: D10B scoped minimum, inward solve, outward hold/recovery and saved-distance isolation");
        }
        // D11 first-person: the eye follows the neck joint; the anchor sits half a
        // projection distance ahead of it. The original minimum boom is relaxed
        // only while the eye view blends in, and the head joint is flagged only
        // between Duke's draw and the next object-list step.
        {
            const uint32_t desc=0x801d5820,mats=0x801d6000,record=desc+0x44+0x28*9;
            psx_mod_write_word(p+0x40,desc);psx_mod_write_word(p+0x3c,mats);
            // D11D/D12B: joint -> part per costume (records byte 1; the part ->
            // joint table at +0x24), as dumped from the first map and LEVEL01.
            static constexpr uint8_t first_map[19]={0x00,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x14,0x10,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x15};
            static constexpr uint8_t cowboy[19]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x15,0x09,0x0d,0x0e,0x0f,0x14,0x10,0x0a,0x0b,0x0c};
            auto layout=[&](const uint8_t* parts){
                for(unsigned i=0;i<0x20;++i) psx_mod_write_byte(desc+0x24+i,0xff);
                for(unsigned j=0;j<19;++j) {
                    psx_mod_write_byte(desc+0x24+parts[j],j);psx_mod_write_byte(desc+0x44+0x28*j+1,parts[j]);
                    psx_mod_write_byte(desc+0x44+0x28*j+2,parts[j]==0x10 || parts[j]==0x15 ? 3 : 2);
                    psx_mod_write_byte(desc+0x44+0x28*j,0);
                }
            };
            psx_mod_write_byte(desc,19);layout(first_map);
            const int32_t root[3]={1000,-500,2000},neck[3]={1010,-690,2005};
            int32_t saved_root[3];
            for(int i=0;i<3;++i) {
                saved_root[i]=psx_mod_read_word(p+4+4*i);
                psx_mod_write_word(p+4+4*i,root[i]);psx_mod_write_word(mats+0x20*9+0x14+4*i,neck[i]);
            }
            psx_mod_write_half(c+0x42,386);
            assert(psx_mod_read_word(0x800c3c48)==768);
            ttk::input.first_person=true;
            for(unsigned i=0;i<40;++i)frame();
            assert(metric("\"blend\":")==1);
            sp=0x801f0000-0xb8;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
            assert(psx_mod_read_word(0x800c3c48)==0);
            // The relaxed boom never outlives the camera update (save states).
            call(0x8002a038,0,1,0x8003af40,sp);assert(psx_mod_read_word(0x800c3c48)==768);
            // The render's per-frame H load gets the eye view's shorter distance;
            // camera[+0x42] itself is never written.
            {
                CPUState cpu{};cpu.gpr[4]=386;cpu.gpr[31]=0x8002e4d0;cpu.gpr[29]=0x801f0000;
                hooks().at(0x800b4d9c)(&cpu,0x800b4d9c);assert(cpu.gpr[4]==256);
                cpu.gpr[4]=386;cpu.gpr[31]=0x800255dc;hooks().at(0x800b4d9c)(&cpu,0x800b4d9c);assert(cpu.gpr[4]==386);
            }
            double forward[3],anchor[3];
            for(int i=0;i<3;++i) {
                forward[i]=(int16_t)psx_mod_read_half(c+12+2*i)/4096.0;
                anchor[i]=(int32_t)psx_mod_read_word(sp+0x18+4*i)+(int32_t)psx_mod_read_word(c+0x64+4*i);
            }
            const double flat=std::hypot(forward[0],forward[2]);
            (void)flat;
            const double eye[3]={(double)neck[0],neck[1]-96.0,(double)neck[2]};
            // The anchor is half the camera's projection distance ahead of the eye.
            assert((int16_t)psx_mod_read_half(c+0x42)==386);
            for(int i=0;i<3;++i) assert(std::abs(anchor[i]-(eye[i]+forward[i]*193))<3);
            assert(psx_mod_read_word(p+4)==(uint32_t)root[0]); // no position commits
            call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1);
            call(0x8001ca4c,0x800d0000,0,0x800376b4);assert(!(psx_mod_read_byte(record)&1));
            // D11C: a stale save-state hide in first person is adopted for this
            // draw and released at the list step like our own.
            psx_mod_write_byte(record,1);
            call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1);
            call(0x8001ca4c,0x800d0000,0,0x800376b4);assert(!(psx_mod_read_byte(record)&1));
            call(0x800348d8,c,0x800d8000,0x8003769c);assert(!(psx_mod_read_byte(record)&1)); // other actors
            call(0x800348d8,c,p,0x80037718);assert(!(psx_mod_read_byte(record)&1));          // other draw loop
            // D11D: the cowboy costume's neck is joint 15 (joint 9 sits at the
            // hip). The eye follows joint 15 and joint 15 is hidden; a stale bit
            // on joint 9 from an older build is reclaimed.
            {
                layout(cowboy);
                const uint32_t record15=desc+0x44+0x28*15;
                int32_t saved9[3];
                for(int i=0;i<3;++i) {
                    saved9[i]=psx_mod_read_word(mats+0x20*9+0x14+4*i);
                    psx_mod_write_word(mats+0x20*9+0x14+4*i,root[i]+(i==1?89:0));
                    psx_mod_write_word(mats+0x20*15+0x14+4*i,neck[i]);
                }
                psx_mod_write_byte(record,1);
                for(unsigned i=0;i<40;++i)frame();
                call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
                for(int i=0;i<3;++i) {
                    const double a=(int32_t)psx_mod_read_word(sp+0x18+4*i)+(int32_t)psx_mod_read_word(c+0x64+4*i);
                    assert(std::abs(a-(eye[i]+forward[i]*193))<3);
                }
                call(0x8002a038,0,1,0x8003af40,sp);
                call(0x800348d8,c,p,0x8003769c);
                assert(!(psx_mod_read_byte(record)&1) && (psx_mod_read_byte(record15)&1));
                call(0x8001ca4c,0x800d0000,0,0x800376b4);assert(!(psx_mod_read_byte(record15)&1));
                for(int i=0;i<3;++i) psx_mod_write_word(mats+0x20*9+0x14+4*i,saved9[i]);
                layout(first_map);
                for(unsigned i=0;i<40;++i)frame();
            }
            // D12 first-person weapon: only the draw loop's hand transform and its
            // attached draws get the private matrix; arm joints get one at the eye;
            // Duke's joint matrices are never written.
            {
                auto hook=[&](uint32_t address,uint32_t a0,uint32_t a1,uint32_t ra,uint32_t joint,uint32_t a3=0,uint32_t sp2=0x801f0000){
                    CPUState cpu{};cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[7]=a3;cpu.gpr[20]=p;cpu.gpr[23]=joint;
                    cpu.gpr[31]=ra;cpu.gpr[29]=sp2;hooks().at(address)(&cpu,address);return cpu;
                };
                psx_mod_write_byte(desc+0x33,7);
                for(unsigned j=0;j<19;++j) for(int i=0;i<9;++i) psx_mod_write_half(mats+0x20*j+2*i,i%4==0?4096:0);
                for(int i=0;i<3;++i) psx_mod_write_word(mats+0x20*7+0x14+4*i,neck[i]+(i==2?250:120));
                const uint8_t saved_state=psx_mod_read_byte(p+0x3b8),saved_slot=psx_mod_read_byte(p+0x3b9);
                psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);
                std::vector<uint8_t> before(ram+(mats&0x1fffff),ram+(mats&0x1fffff)+0x20*19);
                const uint32_t hand=mats+0x20*7;
                call(0x800348d8,c,p,0x8003769c);
                auto h=hook(0x800292a0,c,hand,0x80034c6c,7);
                assert(h.gpr[5]!=hand && h.gpr[5]>=0x801e0000 && ttk::first_person_weapon_drawing());
                const uint32_t vm=h.gpr[5];
                assert(hook(0x80033e40,hand,0,0x8003531c,7).gpr[4]==vm);          // weapon mesh
                assert(hook(0x80033f5c,hand,4,0x80035250,7).gpr[4]==vm);          // held item
                assert(hook(0x80033e40,hand,0,0x80035000,7).gpr[4]==hand);        // other caller
                // Muzzle flash: placed from the same matrix, so it stays on the weapon.
                assert(hook(0x800341e4,hand,4,0x80035348,7,0x801d7000).gpr[4]==vm);
                // The flash's own transform (another caller) is left alone.
                assert(hook(0x800292a0,c,0x801f0240,0x8003437c,7,0,0x801f0200).gpr[5]==0x801f0240);
                // Arm joints go to a matrix at the eye (culled by the loop); others untouched.
                auto arm=hook(0x800292a0,c,mats+0x20*3,0x80034c6c,3);
                assert(arm.gpr[5]!=mats+0x20*3 && !ttk::first_person_weapon_drawing());
                for(int i=0;i<3;++i) assert(psx_mod_read_word(arm.gpr[5]+0x14+4*i)==psx_mod_read_word(c+0x14+4*i));
                assert(hook(0x800292a0,c,mats+0x20*9,0x80034c6c,9).gpr[5]==mats+0x20*9);
                assert(hook(0x800292a0,c,hand,0x80034c6c,7,0).gpr[5]!=hand);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                assert(!ttk::first_person_weapon_drawing());
                // Outside Duke's draw, holstered, or another actor: untouched.
                assert(hook(0x800292a0,c,hand,0x80034c6c,7).gpr[5]==hand);
                call(0x800348d8,c,p,0x8003769c);psx_mod_write_byte(p+0x3b8,0);
                assert(hook(0x800292a0,c,hand,0x80034c6c,7).gpr[5]==hand);
                assert(hook(0x800292a0,c,mats+0x20*3,0x80034c6c,3).gpr[5]==mats+0x20*3);
                psx_mod_write_byte(p+0x3b8,2);
                {CPUState cpu{};cpu.gpr[4]=c;cpu.gpr[5]=hand;cpu.gpr[20]=0x800d8000;cpu.gpr[23]=7;cpu.gpr[31]=0x80034c6c;cpu.gpr[29]=0x801f0000;
                 hooks().at(0x800292a0)(&cpu,0x800292a0);assert(cpu.gpr[5]==hand);}
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                assert(std::equal(before.begin(),before.end(),ram+(mats&0x1fffff)));
                // D12D: resolved upgrade records must have the complete base
                // viewmodel transform and mesh scale, before and after firing.
                // Exercise the real draw hooks, leaving the guest record intact.
                {
                    auto framing=[&](unsigned slot,bool flash) {
                        psx_mod_write_byte(p+0x3b9,slot);
                        call(0x800348d8,c,p,0x8003769c);
                        for(unsigned i=0;i<40;++i)hook(0x800292a0,c,hand,0x80034c6c,7);
                        if(flash) {
                            assert(hook(0x800341e4,hand,4,0x80035348,7).gpr[4]==vm);
                            hook(0x800292a0,c,hand,0x80034c6c,7);
                        }
                        std::array<uint8_t,32> matrix{};
                        std::copy(ram+(vm&0x1fffff),ram+(vm&0x1fffff)+32,matrix.begin());
                        const auto mesh=hook(0x80033e40,hand,0,0x8003531c,7,4096);
                        assert(psx_mod_read_byte(p+0x3b9)==slot);
                        call(0x8001ca4c,0x800d0000,0,0x800376b4);
                        return std::make_pair(matrix,mesh.gpr[7]);
                    };
                    for(bool flash:{false,true}) {
                        assert(framing(9,flash)==framing(27,flash));
                        assert(framing(7,flash)==framing(28,flash));
                        assert(framing(8,flash)==framing(29,flash));
                    }
                    assert(std::equal(before.begin(),before.end(),ram+(mats&0x1fffff)));
                    psx_mod_write_byte(p+0x3b9,4);
                }
                // D08K10: crouch display comes from the arm channel, never the
                // hunch-adjusted world hand. A synthetic arm checks independence
                // from that matrix and that animated channel motion still plays.
                {
                    const std::vector<uint8_t> saved_ram(ram,ram+sizeof ram);
                    const auto saved_input=ttk::input;
                    const uint32_t frm=0x801d7000,rot=0x801d7100;
                    psx_mod_write_word(0x800d8324,frm);psx_mod_write_word(0x800d8320,rot);
                    psx_mod_write_word(frm,100u<<16);psx_mod_write_word(frm+4,3u<<16);
                    for(unsigned j=13;j<=15;++j) {
                        const unsigned index=psx_mod_read_byte(desc+0x24+j);
                        const uint32_t rec=desc+0x44+0x28*index;
                        psx_mod_write_byte(rec+2,j-10);
                        for(unsigned i=0;i<3;++i)psx_mod_write_word(rec+0x14+4*i,i==j-13?100:0);
                        psx_mod_write_word(rot+8*(j-13),0);
                        psx_mod_write_word(rot+8*(j-13)+4,(j<<27)|(4095u<<14));
                    }
                    psx_mod_write_half(p+0x74,39);psx_mod_write_half(p+0x76,100);
                    psx_mod_write_half(p+0x78,0);psx_mod_write_half(p+0x7a,0);
                    ttk::input.held[ttk::crouch]=true;ttk::input.held[ttk::interact]=false;
                    ttk::input.held[ttk::original_aim]=false;
                    psx_mod_write_half(p+0x60,63);psx_mod_write_word(p+0x224,0);
                    psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
                    call(0x8003ade4,c,p,0x80025ee8);
                    call(0x8005a210,p,0x800c2754,0x80041b34);
                    assert(psx_mod_read_half(p+0x60)==176);
                    auto displayed=[&]() {
                        call(0x800348d8,c,p,0x8003769c);
                        const auto result=hook(0x800292a0,c,hand,0x80034c6c,7);
                        assert(result.gpr[5]!=hand);
                        std::array<uint8_t,32> bytes{};
                        std::copy(ram+(result.gpr[5]&0x1fffff),ram+(result.gpr[5]&0x1fffff)+32,bytes.begin());
                        call(0x8001ca4c,0x800d0000,0,0x800376b4);
                        return bytes;
                    };
                    for(unsigned slot:{1u,2u,3u,12u,13u,14u,32u}) {
                        psx_mod_write_byte(p+0x3b8,slot==32?1:2);
                        psx_mod_write_byte(p+0x3b9,slot==32?12:slot);
                        const auto neutral=displayed();
                        psx_mod_write_word(hand+0x14,psx_mod_read_word(hand+0x14)+600);
                        assert(displayed()==neutral);
                        // Original crouch and standing retain D12's live path.
                        setenv("DNTTK_CROUCH","original",1);assert(displayed()!=neutral);
                        unsetenv("DNTTK_CROUCH");assert(displayed()==neutral);
                        psx_mod_write_half(p+0x60,63);assert(displayed()!=neutral);
                        psx_mod_write_half(p+0x60,178);assert(displayed()==neutral);
                        // Rotate the shoulder in its animation frame, as a throw
                        // does. Its display must animate even though the world
                        // hand has not changed.
                        psx_mod_write_word(rot,2896);
                        psx_mod_write_word(rot+4,(13u<<27)|(2896u<<14));
                        assert(displayed()!=neutral);
                        psx_mod_write_word(rot,0);psx_mod_write_word(rot+4,(13u<<27)|(4095u<<14));
                        assert(displayed()==neutral);
                    }
                    // D11E uses the same arm animation in water, even with
                    // Original crouch selected. Worker snapshots must carry
                    // swim eligibility in both directions independently of it.
                    {
                        const auto water=psx_mod_read_word(p+0x834),flag=psx_mod_read_word(p+0x20c),floor=psx_mod_read_word(p+0x1c8);
                        setenv("DNTTK_CROUCH","original",1);
                        psx_mod_write_word(p+0x834,root[1]-198);psx_mod_write_word(p+0x20c,1);psx_mod_write_word(p+0x1c8,root[1]+2000);
                        psx_mod_write_byte(p+0x22c,5);psx_mod_write_half(p+0x60,128);
                        for(unsigned slot:{3u,12u,32u}) {
                            psx_mod_write_byte(p+0x3b8,slot==32?1:2);psx_mod_write_byte(p+0x3b9,slot==32?12:slot);
                            const auto corrected=displayed();
                            psx_mod_write_word(hand+0x14,psx_mod_read_word(hand+0x14)+600);assert(displayed()==corrected);
                            std::vector<uint8_t> active(ttk::render_state_size()),inactive(active.size());
                            ttk::render_state_save(active.data());
                            psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x20c,0);psx_mod_write_byte(p+0x22c,0);
                            const auto original=displayed();assert(original!=corrected);ttk::render_state_save(inactive.data());
                            const pid_t child=fork();assert(child>=0);
                            if(child==0) {
                                ttk::render_state_load(active.data());assert(displayed()==corrected);
                                ttk::render_state_load(inactive.data());assert(displayed()==original);
                                ttk::render_state_load(active.data());assert(displayed()==corrected);_exit(0);
                            }
                            int status=0;assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
                            psx_mod_write_word(p+0x834,root[1]-198);psx_mod_write_word(p+0x20c,1);psx_mod_write_byte(p+0x22c,5);
                            psx_mod_write_word(rot,2896);psx_mod_write_word(rot+4,(13u<<27)|(2896u<<14));assert(displayed()!=corrected);
                            psx_mod_write_word(rot,0);psx_mod_write_word(rot+4,(13u<<27)|(4095u<<14));assert(displayed()==corrected);
                        }
                        psx_mod_write_word(p+0x834,water);psx_mod_write_word(p+0x20c,flag);psx_mod_write_word(p+0x1c8,floor);
                        psx_mod_write_byte(p+0x22c,0);psx_mod_write_half(p+0x60,178);unsetenv("DNTTK_CROUCH");
                        std::puts("PASS: D11E animated swim weapons ignore compensated world hand, retain animation, publish worker eligibility and restore ground framing");
                    }
                    // D12's seeded guns keep exactly their existing matrices.
                    for(unsigned slot=4;slot<=11;++slot) {
                        psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,slot);
                        const auto seeded=displayed();
                        setenv("DNTTK_CROUCH","original",1);assert(displayed()==seeded);
                        unsetenv("DNTTK_CROUCH");assert(displayed()==seeded);
                    }
                    psx_mod_write_byte(p+0x3b9,1);
                    // A replay worker redraws with fork-time stance statics.
                    // Publish eligibility with each frame, including disabling it
                    // again for Original/standing frames in the same worker.
                    {
                        const auto corrected=displayed();
                        std::vector<uint8_t> active(ttk::render_state_size()),inactive(active.size());
                        ttk::render_state_save(active.data());
                        setenv("DNTTK_CROUCH","original",1);
                        const auto original=displayed();ttk::render_state_save(inactive.data());
                        unsetenv("DNTTK_CROUCH");assert(original!=corrected);
                        const pid_t child=fork();assert(child>=0);
                        if(child==0) {
                            // Simulate a worker forked before crouch began.
                            ttk::input.held[ttk::crouch]=false;psx_mod_write_half(p+0x60,63);
                            call(0x8005a210,p,0x800c2754,0x80041b34);
                            psx_mod_write_half(p+0x60,178);
                            assert(displayed()==original);
                            ttk::render_state_load(active.data());assert(displayed()==corrected);
                            ttk::render_state_load(inactive.data());assert(displayed()==original);
                            ttk::render_state_load(active.data());assert(displayed()==corrected);
                            _exit(0);
                        }
                        int status=0;assert(waitpid(child,&status,0)==child);
                        assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
                    }
                    // Missing hand data falls back instead of inventing a pose.
                    const auto neutral=displayed();psx_mod_write_word(frm+4,2u<<16);
                    assert(displayed()!=neutral);
                    std::copy(saved_ram.begin(),saved_ram.end(),ram);++g_dirty_ram_code_gen;
                    ttk::input=saved_input;
                    psx_mod_write_half(p+0x60,63);
                    call(0x8005a210,p,0x800c2754,0x80041b34);
                    std::puts("PASS: D08K10 crouched viewmodel (six weapons and held item, channel motion, live-matrix independence, Original/standing and missing-data fallback)");
                }
                psx_mod_write_byte(p+0x3b8,saved_state);psx_mod_write_byte(p+0x3b9,saved_slot);
                ttk::input.first_person=false;for(unsigned i=0;i<40;++i)frame();
                // Third person and Vanilla: upgraded records keep the original
                // hand, attached mesh scale and muzzle-flash transform.
                for(bool modern:{true,false}) {
                    ttk::modern=modern;
                    for(unsigned slot:{4u,5u,10u,27u,28u,29u}) {
                        call(0x800348d8,c,p,0x8003769c);
                        psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,slot);
                        assert(hook(0x800292a0,c,hand,0x80034c6c,7).gpr[5]==hand);
                        const auto mesh=hook(0x80033e40,hand,0,0x8003531c,7,4096);
                        assert(mesh.gpr[4]==hand && mesh.gpr[7]==4096);
                        assert(hook(0x800341e4,hand,4,0x80035348,7).gpr[4]==hand);
                        call(0x8001ca4c,0x800d0000,0,0x800376b4);
                    }
                }
                ttk::modern=true;
                psx_mod_write_byte(p+0x3b8,saved_state);psx_mod_write_byte(p+0x3b9,saved_slot);
                ttk::input.first_person=true;for(unsigned i=0;i<40;++i)frame();
                std::puts("PASS: D12D upgraded first-person framing (base-equivalent pose/recoil/scale, held record preserved, third-person and Vanilla exclusion)");
            }
            // D12A quick kick: an original boot request (112..115, not yet
            // initialized) becomes the quick kick at the lower-body initializer;
            // its sphere runs along the view on each player update while the leg
            // is out; the right leg joints get private matrices; Duke's own
            // joint matrices are never written.
            {
                auto hook=[&](uint32_t address,uint32_t a0,uint32_t a1,uint32_t ra,uint32_t joint){
                    CPUState cpu{};cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[20]=p;cpu.gpr[23]=joint;
                    cpu.gpr[31]=ra;cpu.gpr[29]=0x801f0000;hooks().at(address)(&cpu,address);return cpu;
                };
                auto update=[&](){call(0x80058120,p,0,0x80041c44,0x801fff00);};
                const uint32_t saved_delta=psx_mod_read_word(0x800d21fc);psx_mod_write_word(0x800d21fc,10);
                const uint16_t saved_power=psx_mod_read_half(p+0x364);psx_mod_write_half(p+0x364,0);
                std::vector<uint8_t> before(ram+(mats&0x1fffff),ram+(mats&0x1fffff)+0x20*19);
                const double converts=metric("\"converts\":");
                // Initialized (track set), another caller, or another actor: untouched.
                psx_mod_write_half(p+0x60,114);psx_mod_write_half(p+0x68,1);
                call(0x800493a4,p,0,0x8005a490,0x801fff00);assert(psx_mod_read_half(p+0x60)==114);
                psx_mod_write_half(p+0x68,0);
                call(0x800493a4,p,0,0x8005a000,0x801fff00);assert(psx_mod_read_half(p+0x60)==114);
                call(0x800493a4,0x800d8000,0,0x8005a490,0x801fff00);assert(psx_mod_read_half(p+0x60)==114);
                assert(metric("\"converts\":")==converts);
                // The request keeps the eye view's lease until its initializer.
                frame();assert(metric("\"blend\":")==1);
                // From E (Action, no attack held): idle restarts, no kick.
                const double suppressed=metric("\"suppressed\":");
                call(0x800493a4,p,0,0x8005a490,0x801fff00);
                assert(psx_mod_read_half(p+0x60)==63 && psx_mod_read_half(p+0x68)==0);
                assert(metric("\"suppressed\":")==suppressed+1 && metric("\"converts\":")==converts);
                assert(std::strstr(ttk::controls_debug_json(),"\"kick\":{\"active\":false"));
                // With the attack held: the quick kick.
                psx_mod_write_half(p+0x60,113);psx_mod_write_half(p+0x68,0);frame();
                ttk::input.held[ttk::fire]=true;
                call(0x800493a4,p,0,0x8005a490,0x801fff00);
                ttk::input.held[ttk::fire]=false;
                assert(psx_mod_read_half(p+0x60)==63 && psx_mod_read_half(p+0x68)==0 && metric("\"converts\":")==converts+1);
                kick_calls.clear();
                for(unsigned i=0;i<6;++i){frame();update();}
                assert(kick_calls.empty());                         // chamber: no sound or hit yet
                frame();update();
                assert(kick_calls.size()==1 && kick_calls[0].address==0x8006b270 &&
                       kick_calls[0].a[0]==0x1000 && kick_calls[0].a[1]==p+4 && kick_calls[0].a[2]==0x800);
                frame();update();assert(kick_calls.size()==2);       // hit window opens
                const KickCall& k=kick_calls[1];
                assert(k.address==0x800a979c && k.a[1]==96 && k.a[2]==10 && k.a[3]==200 && k.stack[0]==p && k.stack[1]==0);
                for(int i=0;i<3;++i) {
                    const double forward=(int16_t)psx_mod_read_half(c+12+2*i)/4096.0;
                    assert(std::abs(k.point[i]-((int32_t)psx_mod_read_word(c+0x14+4*i)+forward*340))<2);
                }
                // Where the crosshair ray meets something within the foot's
                // reach, the sphere goes just short of that point; a farther
                // hit leaves the default point along the view.
                {
                    double eye[3],fwd[3];
                    for(int i=0;i<3;++i){fwd[i]=(int16_t)psx_mod_read_half(c+12+2*i)/4096.0;eye[i]=(int32_t)psx_mod_read_word(c+0x14+4*i);}
                    const double flat=std::hypot(fwd[0],fwd[2]);assert(flat>0.3);
                    for(int i=0;i<3;++i)ttk::segment_point[i]=eye[i]+fwd[i]*200/flat;
                    ttk::segment_hit=true;const size_t n=kick_calls.size();
                    frame();update();assert(kick_calls.size()==n+1);
                    for(int i=0;i<3;++i)assert(std::abs(kick_calls.back().point[i]-(ttk::segment_point[i]-fwd[i]*24))<2);
                    for(int i=0;i<3;++i)ttk::segment_point[i]=eye[i]+fwd[i]*900/flat;
                    frame();update();assert(kick_calls.size()==n+2);
                    for(int i=0;i<3;++i)assert(std::abs(kick_calls.back().point[i]-(eye[i]+fwd[i]*340))<2);
                    ttk::segment_hit=false;
                }
                // The leg is drawn in front of the eye during the kick.
                call(0x800348d8,c,p,0x8003769c);
                auto leg=hook(0x800292a0,c,mats+0x20*16,0x80034c6c,16);
                assert(leg.gpr[5]!=mats+0x20*16 && ttk::first_person_weapon_drawing());
                // The thigh is drawn end for end from the knee (its hip origin
                // is inside the loop's near cull).
                {
                    auto thigh=hook(0x800292a0,c,mats+0x20*14,0x80034c6c,14);
                    assert(thigh.gpr[5]!=mats+0x20*14 && ttk::first_person_weapon_drawing());
                    const uint32_t tm=thigh.gpr[5];int32_t thigh_origin[3];
                    for(int i=0;i<3;++i)thigh_origin[i]=(int32_t)psx_mod_read_word(tm+0x14+4*i);
                    auto knee=hook(0x800292a0,c,mats+0x20*15,0x80034c6c,15);
                    // Same place as the knee, nudged down the screen (view y, camera row 1) only.
                    double off[3],along[3]={0,0,0};
                    for(int i=0;i<3;++i)off[i]=thigh_origin[i]-(int32_t)psx_mod_read_word(knee.gpr[5]+0x14+4*i);
                    for(int r=0;r<3;++r)for(int i=0;i<3;++i)along[r]+=(int16_t)psx_mod_read_half(c+2*(3*r+i))/4096.0*off[i];
                    assert(std::abs(along[0])<=2 && along[1]>10 && std::abs(along[2])<=2);
                }
                assert(hook(0x800292a0,c,mats+0x20*12,0x80034c6c,12).gpr[5]==mats+0x20*12); // left leg
                // D12B: the cowboy costume's right leg is joints 5..8; its joints
                // 14 and 15 (part 0x14, the neck) are left alone.
                layout(cowboy);
                for(unsigned j=5;j<=8;++j) assert(hook(0x800292a0,c,mats+0x20*j,0x80034c6c,j).gpr[5]!=mats+0x20*j);
                for(unsigned j=14;j<=15;++j) assert(hook(0x800292a0,c,mats+0x20*j,0x80034c6c,j).gpr[5]==mats+0x20*j);
                layout(first_map);
                assert(!ttk::first_person_weapon_drawing());
                assert(hook(0x800292a0,c,mats+0x20*9,0x80034c6c,9).gpr[5]==mats+0x20*9);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                for(unsigned i=0;i<12;++i){frame();update();}
                const size_t hits=std::count_if(kick_calls.begin(),kick_calls.end(),[](const KickCall& x){return x.address==0x800a979c;});
                assert(hits>=4 && hits<=13);                         // each update in frames 8..20
                kick_calls.clear();
                for(unsigned i=0;i<10;++i){frame();update();}
                assert(kick_calls.empty());                         // finished
                call(0x800348d8,c,p,0x8003769c);
                assert(hook(0x800292a0,c,mats+0x20*16,0x80034c6c,16).gpr[5]==mats+0x20*16);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                assert(std::equal(before.begin(),before.end(),ram+(mats&0x1fffff)));
                // Boot selected: a held attack starts the kick (moving too).
                {
                    const uint32_t eq=psx_mod_read_word(p+0x3b8);psx_mod_write_word(p+0x3b8,eq&~0x00ff00ffu);
                    const double starts=metric("\"fire_starts\":");
                    ttk::input.held[ttk::fire]=true;frame();update();
                    assert(metric("\"fire_starts\":")==starts+1 && std::strstr(ttk::controls_debug_json(),"\"kick\":{\"active\":true"));
                    for(unsigned i=0;i<30;++i){frame();update();}
                    assert(metric("\"fire_starts\":")==starts+2);   // still held: the next kick follows
                    ttk::input.held[ttk::fire]=false;for(unsigned i=0;i<30;++i){frame();update();}
                    assert(std::strstr(ttk::controls_debug_json(),"\"kick\":{\"active\":false"));
                    // A weapon selected: holding fire does not kick.
                    psx_mod_write_word(p+0x3b8,(eq&~0x00ff00ffu)|0x00040002u);
                    ttk::input.held[ttk::fire]=true;frame();update();assert(metric("\"fire_starts\":")==starts+2);
                    ttk::input.held[ttk::fire]=false;psx_mod_write_word(p+0x3b8,eq);kick_calls.clear();
                }
                // D12C: a quick kick plays the wall-bump thud 0x2007 once, at the
                // first contact the sphere reports or a wall (world) where the
                // boot reaches; nothing for empty air or an actor the ray sees
                // but the sphere does not touch.
                auto thuds=[&](){return std::count_if(kick_calls.begin(),kick_calls.end(),[&](const KickCall& x){
                    return x.address==0x8006b73c && x.a[0]==0x2007 && x.a[1]==p+4 && x.a[2]==0x800 && x.a[3]==48;});};
                auto spheres=[&](){return std::count_if(kick_calls.begin(),kick_calls.end(),[](const KickCall& x){return x.address==0x800a979c;});};
                auto quick=[&](unsigned frames){
                    psx_mod_write_half(p+0x60,113);psx_mod_write_half(p+0x68,0);ttk::input.held[ttk::fire]=true;
                    call(0x800493a4,p,0,0x8005a490,0x801fff00);ttk::input.held[ttk::fire]=false;kick_calls.clear();
                    for(unsigned i=0;i<frames;++i){frame();update();}
                };
                {
                    const double played=metric("\"played\":");
                    quick(30);assert(spheres()>=4 && thuds()==0);                  // empty air
                    ttk::segment_hit=true;ttk::segment_kind=2;
                    for(int i=0;i<3;++i)ttk::segment_point[i]=(int32_t)psx_mod_read_word(c+0x14+4*i)+(int16_t)psx_mod_read_half(c+12+2*i)/4096.0*200;
                    quick(30);assert(thuds()==0);                                  // an actor in view, not struck
                    ttk::segment_hit=false;sphere_strike=true;quick(30);sphere_strike=false;
                    assert(thuds()==1 && spheres()>=4);                           // struck: once, at the first contact
                    assert(psx_mod_read_half(0x801e1000+0x6a)==3*5192);            // that sound at three times its table volume
                    {
                        size_t first_sphere=kick_calls.size(),thud=kick_calls.size();
                        for(size_t i=0;i<kick_calls.size();++i){
                            if(kick_calls[i].address==0x800a979c && first_sphere==kick_calls.size())first_sphere=i;
                            if(kick_calls[i].address==0x8006b73c && kick_calls[i].a[0]==0x2007)thud=i;
                        }
                        assert(thud==first_sphere+1);
                    }
                    ttk::segment_hit=true;ttk::segment_kind=1;quick(30);assert(thuds()==1);   // a wall in reach
                    ttk::segment_hit=false;ttk::segment_kind=2;
                    assert(metric("\"played\":")==played+2);
                    setenv("DNTTK_KICK_IMPACT","off",1);                          // read once: still on
                    sphere_strike=true;quick(30);sphere_strike=false;assert(thuds()==1);unsetenv("DNTTK_KICK_IMPACT");
                }
                // Third person: the original request plays as before.
                ttk::input.first_person=false;for(unsigned i=0;i<40;++i)frame();
                psx_mod_write_half(p+0x60,112);psx_mod_write_half(p+0x68,0);
                call(0x800493a4,p,0,0x8005a490,0x801fff00);assert(psx_mod_read_half(p+0x60)==112);
                // D12C: the original kick's sphere call (ra 0x80049098) is made by
                // the host with the same arguments; the original call is left a
                // radius that touches nothing. One thud per kick animation.
                {
                    constexpr uint32_t foot=0x801e0000,ssp=0x801fe000;
                    psx_mod_write_word(foot,100);psx_mod_write_word(foot+4,-200);psx_mod_write_word(foot+8,300);
                    auto sphere=[&](uint32_t ra){
                        CPUState cpu{};cpu.gpr[4]=foot;cpu.gpr[5]=96;cpu.gpr[6]=10;cpu.gpr[7]=200;cpu.gpr[29]=ssp;cpu.gpr[31]=ra;
                        psx_mod_write_word(ssp+0x10,p);psx_mod_write_word(ssp+0x14,0);
                        hooks().at(0x800a979c)(&cpu,0x800a979c);return cpu;
                    };
                    kick_calls.clear();
                    auto miss=sphere(0x80049098);
                    assert(miss.gpr[5]==0xf0000000u && spheres()==1 && thuds()==0);
                    const KickCall& k=kick_calls[0];
                    assert(k.a[0]==foot && k.a[1]==96 && k.a[2]==10 && k.a[3]==200 && k.stack[0]==p && k.stack[1]==0);
                    sphere_strike=true;
                    sphere(0x80049098);sphere(0x80049098);assert(spheres()==3 && thuds()==1);
                    update();psx_mod_write_half(p+0x60,115);sphere(0x80049098);assert(thuds()==2); // next kick
                    psx_mod_write_half(p+0x60,63);update();psx_mod_write_half(p+0x60,115);
                    sphere(0x80049098);assert(thuds()==3);
                    kick_calls.clear();
                    assert(sphere(0x800a0000).gpr[5]==96 && kick_calls.empty());   // another caller
                    ttk::modern=false;
                    assert(sphere(0x80049098).gpr[5]==96 && kick_calls.empty());   // Vanilla
                    ttk::modern=true;sphere_strike=false;
                    psx_mod_write_half(p+0x60,112);
                }
                psx_mod_write_half(p+0x60,63);
                psx_mod_write_word(0x800d21fc,saved_delta);psx_mod_write_half(p+0x364,saved_power);
                ttk::input.first_person=true;for(unsigned i=0;i<40;++i)frame();
            }
            ttk::input.first_person=false;
            for(unsigned i=0;i<40;++i)frame();
            assert(metric("\"blend\":")==0);
            call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
            assert(psx_mod_read_word(0x800c3c48)==256 && (int16_t)psx_mod_read_half(c+0x42)==386);
            call(0x8002a038,0,1,0x8003af40,sp);assert(psx_mod_read_word(0x800c3c48)==768);
            call(0x800348d8,c,p,0x8003769c);assert(!(psx_mod_read_byte(record)&1));
            // D11C: a head hide captured by a save state (bit 0 set, host owns
            // nothing after the load) is reclaimed at Duke's next draw in third
            // person, keeping the record's other flag bits; other actors and
            // draw loops leave it alone.
            {
                const double reclaims0=metric("\"head_reclaims\":");
                psx_mod_write_byte(record,0x11);
                call(0x800348d8,c,0x800d8000,0x8003769c);assert(psx_mod_read_byte(record)==0x11);
                call(0x800348d8,c,p,0x80037718);assert(psx_mod_read_byte(record)==0x11);
                call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)==0x10);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);assert(psx_mod_read_byte(record)==0x10);
                assert(metric("\"head_reclaims\":")==reclaims0+1);
                psx_mod_write_byte(record,0);
            }
            // A relaxed word captured by a save state is repaired, not adopted.
            psx_mod_write_word(0x800c3c48,0);frame();
            assert(psx_mod_read_word(0x800c3c48)==768);
            // D26B: console/focus/input epochs and guest pause retain the
            // eye, orientation and head policy without granting movement.
            ttk::input.first_person=true;for(unsigned i=0;i<40;++i)frame();
            const double owned_yaw=metric("\"yaw\":"),owned_pitch=metric("\"pitch\":");
            for(unsigned paused:{0u,1u}) {
                ttk::input.active=false;++ttk::input.epoch;
                ttk::input.total_x=ttk::input.total_y=0;
                psx_mod_write_half(0x800be568,paused);
                ttk::input.sequence+=50; // paused redraw before another camera update
                call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                for(unsigned i=0;i<8;++i)frame();
                assert(metric("\"blend\":")==1 && !ttk::movement_ready());
                assert(metric("\"yaw\":")==owned_yaw && metric("\"pitch\":")==owned_pitch);
                call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
                assert(psx_mod_read_word(0x800c3c48)==0);
                call(0x8002a038,0,1,0x8003af40,sp);
                call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);assert(!(psx_mod_read_byte(record)&1));
                CPUState cpu{};cpu.gpr[4]=386;cpu.gpr[31]=0x8002e4d0;cpu.gpr[29]=0x801f0000;
                hooks().at(0x800b4d9c)(&cpu,0x800b4d9c);assert(cpu.gpr[4]==256);
            }
            psx_mod_write_half(0x800be568,0);
            ttk::input.active=true;++ttk::input.epoch;frame();
            assert(metric("\"blend\":")==1 && ttk::movement_ready());
            assert(metric("\"yaw\":")==owned_yaw);
            // Load a different stored heading while input is suspended: adopt
            // that world's direction, but immediately use our selected view.
            ttk::input.active=false;
            call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1);
            ++save_loads;psx_mod_write_byte(record,0x11);
            call(0x800b4d9c,386,0,0x8002e4d0); // load resumes inside a draw
            assert(psx_mod_read_byte(record)==0x11 && metric("\"blend\":")==0);
            psx_mod_write_half(c+12,4096);psx_mod_write_half(c+14,0);psx_mod_write_half(c+16,0);
            frame();assert(metric("\"blend\":")==1);
            assert(std::abs(metric("\"yaw\":")-quarter)<1e-6);
            // A load may start in an unowned jump; camera acquisition must
            // not depend on input/flight state from before that save.
            for(unsigned a:{96u,97u,98u,103u,104u,105u,109u}) {
                ++save_loads;psx_mod_write_half(p+0x60,a);
                psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
                ++ttk::input.epoch;frame();
                assert(metric("\"blend\":")==1 && !ttk::movement_ready());
            }
            psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);frame();
            // Explicit Legacy owns the original camera; the first-person
            // preference is dormant and returns when Modern is selected.
            setenv("DNTTK_CONTROL_SCHEME","legacy",1);frame();
            assert(metric("\"blend\":")==0 && ttk::input.first_person);
            unsetenv("DNTTK_CONTROL_SCHEME");frame();assert(metric("\"blend\":")==1);
            // Unknown code and an actual camera-owner change still revoke
            // presentation, even though the request itself remains first.
            poke(0x43eb8);frame();assert(metric("\"blend\":")==0);poke(0x43eb8);
            frame();assert(metric("\"blend\":")==1);
            psx_mod_write_word(c+0xa4,0);frame();assert(metric("\"blend\":")==0);
            psx_mod_write_word(c+0xa4,p);frame();assert(metric("\"blend\":")==1);
            // Same-level teardown cannot inherit the old orbit either.
            call(0x8002b9f4,0,0,0x80025474);assert(metric("\"blend\":")==1);
            call(0x8002b9f4,1,0,0x80025478);assert(metric("\"blend\":")==1);
            call(0x8002b9f4,1,0,0x80025474);assert(metric("\"blend\":")==0);
            frame();assert(metric("\"blend\":")==1 && ttk::input.first_person);
            std::puts("PASS: D26B view ownership across input suspension, pause, capture epochs, load, explicit scheme and camera ownership; movement and identity gates retained");
            // D11E: swimming is a presentation state, independent of active
            // input and of the pitched/turning neck. Keep original water and
            // velocity fields untouched, including a load directly underwater.
            {
                const auto water=psx_mod_read_word(p+0x834),flag=psx_mod_read_word(p+0x20c),floor=psx_mod_read_word(p+0x1c8);
                auto solved_eye=[&](int axis) {
                    call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
                    const double v=(int32_t)psx_mod_read_word(sp+0x18+4*axis)+(int32_t)psx_mod_read_word(c+0x64+4*axis)-
                        (int16_t)psx_mod_read_half(c+12+2*axis)/4096.0*193;
                    call(0x8002a038,0,1,0x8003af40,sp);return v;
                };
                ttk::input.active=false;
                psx_mod_write_word(p+0x834,root[1]-198);psx_mod_write_word(p+0x20c,1);
                psx_mod_write_word(p+0x1c8,root[1]+2000);
                for(const uint8_t* parts:{first_map,cowboy}) {
                    layout(parts);const unsigned head=psx_mod_read_byte(desc+0x34);
                    for(unsigned mode:{0u,1u,4u,5u}) {
                        psx_mod_write_byte(p+0x22c,mode);psx_mod_write_byte(p+0x22d,mode);
                        psx_mod_write_half(p+0x60,mode==4?122:mode==5?128:176);
                        ++save_loads;frame();
                        assert(metric("\"blend\":")==1 && !ttk::movement_ready());
                        const uint32_t water_flag=psx_mod_read_word(p+0x20c),velocity=psx_mod_read_word(p+0x1f8);
                        for(int pose:{-400,0,450}) {
                            for(int i=0;i<3;++i)psx_mod_write_word(mats+0x20*head+0x14+4*i,root[i]+pose);
                            for(int i=0;i<24;++i)frame();
                            assert(std::abs(solved_eye(0)-root[0])<2 && std::abs(solved_eye(2)-root[2])<2);
                            assert(std::abs(solved_eye(1)-(root[1]-294))<2);
                            assert(psx_mod_read_word(p+0x20c)==water_flag && psx_mod_read_word(p+0x1f8)==velocity);
                            for(int i=0;i<3;++i)assert((int32_t)psx_mod_read_word(p+4+4*i)==root[i]);
                        }
                        const auto flags=psx_mod_read_word(p+0x224);
                        psx_mod_write_word(p+0x224,flags|0x200);frame();assert(metric("\"blend\":")==((mode==4 || mode==5)?1:0) && !ttk::movement_ready());
                        psx_mod_write_word(p+0x224,flags|0x20000000);frame();assert(metric("\"blend\":")==0);
                        psx_mod_write_word(p+0x224,flags);frame();assert(metric("\"blend\":")==1);
                        psx_mod_write_half(0x800be568,1);frame();assert(metric("\"blend\":")==1);
                        psx_mod_write_half(0x800be568,0);
                        ttk::input.first_person=false;for(int i=0;i<40;++i)frame();assert(metric("\"blend\":")==0);
                        ttk::input.first_person=true;++save_loads;frame();assert(metric("\"blend\":")==1);
                    }
                }
                // A water-to-mantle handoff still has previous mode 4/5 and
                // may retain the water flag. Yield before the first climb frame.
                for(unsigned previous:{4u,5u}) {
                    psx_mod_write_byte(p+0x22c,8);psx_mod_write_byte(p+0x22d,previous);psx_mod_write_half(p+0x60,134);
                    ++save_loads;frame();assert(metric("\"blend\":")==0);
                    assert(std::strstr(ttk::controls_debug_json(),"\"reason\":\"traversal\""));
                }
                layout(first_map);
                for(int i=0;i<3;++i)psx_mod_write_word(mats+0x20*9+0x14+4*i,neck[i]);
                psx_mod_write_word(p+0x834,water);psx_mod_write_word(p+0x20c,flag);psx_mod_write_word(p+0x1c8,floor);
                psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);psx_mod_write_half(p+0x60,63);
                ++save_loads;frame();
                std::puts("PASS: D11E surface/underwater eye, two costume layouts, pose independence, suspended input/pause, loaded view, third-person switch and no water/velocity/position writes");
            }

            // D11F: camera-only flight keeps the selected eye with no action
            // lease. Lean animation cannot shift it into a wall or the pack.
            {
                const auto pack=psx_mod_read_half(p+0x358);
                const auto water=psx_mod_read_word(p+0x20c);
                psx_mod_write_word(p+0x20c,0);
                psx_mod_write_half(p+0x358,3);
                ttk::input.active=false;
                for(const char* scheme:{"modern","classic","eduke32"}) {
                    setenv("DNTTK_JETPACK",scheme,1);
                    for(const uint8_t* parts:{first_map,cowboy}) {
                        layout(parts);const unsigned head=psx_mod_read_byte(desc+0x34);
                        for(unsigned animation=163;animation<=170;++animation) {
                            psx_mod_write_byte(p+0x22c,10);psx_mod_write_byte(p+0x22d,0);
                            psx_mod_write_half(p+0x60,animation);
                            ++save_loads;frame();
                            assert(metric("\"blend\":")==1 && !ttk::movement_ready());
                            const auto velocity=psx_mod_read_word(p+0x1f8);
                            for(int pose:{-400,450}) {
                                for(int i=0;i<3;++i)psx_mod_write_word(mats+0x20*head+0x14+4*i,root[i]+pose);
                                for(int i=0;i<24;++i)frame();
                                call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
                                for(int i=0;i<3;++i) {
                                    const double eye=(int32_t)psx_mod_read_word(sp+0x18+4*i)+(int32_t)psx_mod_read_word(c+0x64+4*i)-
                                        (int16_t)psx_mod_read_half(c+12+2*i)/4096.0*193;
                                    assert(std::abs(eye-(root[i]-(i==1?294:0)))<2);
                                    assert((int32_t)psx_mod_read_word(p+4+4*i)==root[i]);
                                }
                                call(0x8002a038,0,1,0x8003af40,sp);
                                assert(psx_mod_read_word(p+0x1f8)==velocity && psx_mod_read_half(p+0x358)==3);
                            }
                        }
                    }
                }
                // Native off/fuel-out clears the pack one tick before the
                // flight animation changes; landing retains previous mode 10.
                psx_mod_write_half(p+0x358,1);frame();
                assert(metric("\"blend\":")==1 && !ttk::jetpack_input_ready());
                psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,10);
                psx_mod_write_half(p+0x60,108);frame();assert(metric("\"blend\":")==1);
                for(unsigned animation:{63u,105u,106u}) {
                    psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,10);
                    psx_mod_write_half(p+0x60,animation);frame();assert(metric("\"blend\":")==1);
                }
                psx_mod_write_half(p+0x358,3);psx_mod_write_byte(p+0x22c,10);
                psx_mod_write_half(p+0x60,164);frame();
                unsetenv("DNTTK_JETPACK");
                psx_mod_write_word(p,2);frame();assert(metric("\"blend\":")==0);
                psx_mod_write_word(p,0);frame();assert(metric("\"blend\":")==1);
                ttk::modern=false;frame();assert(!ttk::first_person_view_live());
                ttk::modern=true;frame();assert(metric("\"blend\":")==1);
                ttk::input.first_person=false;for(int i=0;i<40;++i)frame();assert(metric("\"blend\":")==0);
                ttk::input.first_person=true;frame();
                layout(first_map);
                for(int i=0;i<3;++i)psx_mod_write_word(mats+0x20*9+0x14+4*i,neck[i]);
                psx_mod_write_half(p+0x358,pack);psx_mod_write_word(p+0x20c,water);
                psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);psx_mod_write_half(p+0x60,63);
                ++save_loads;frame();assert(metric("\"blend\":")==1);
                std::puts("PASS: D11F flight eye across schemes, poses and costumes; loaded/suspended presentation, death, Vanilla, third-person and ground handoff");
            }

            ttk::input.active=true;ttk::input.first_person=false;++ttk::input.epoch;
            for(unsigned i=0;i<40;++i)frame();
            for(int i=0;i<3;++i)psx_mod_write_word(p+4+4*i,saved_root[i]);
        }
        // Leave the view where the following cases expect it.
        psx_mod_write_half(p+0x1c,(uint16_t)std::lround(yaw0*4096/6.2831853071795864769)&4095);
        ttk::input.recenter_total=4;for(unsigned i=0;i<60;++i)frame();
        call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
        psx_mod_write_half(p+0x1c,heading0);psx_mod_write_word(c+0x64,camera_x0);psx_mod_write_word(c+0x6c,camera_z0);
    }
    // Standing-jump preparation retains a bounded direction lease, then redirects
    // original animation 98 velocity once. No forced state, height or position.
    // Start an independent fixture facing +Z, not an input-recapture reseed.
    ++save_loads;psx_mod_write_half(c+12,0);psx_mod_write_half(c+14,0);psx_mod_write_half(c+16,4096);
    auto ground=[&](){
        ++ttk::input.epoch;++ttk::input.sequence;ttk::input.active=true;
        ttk::input.held[ttk::original_aim]=false;ttk::input.held[ttk::crouch]=false;
        psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);
        psx_mod_write_half(p+0x60,63);psx_mod_write_half(p+0x68,0);
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_half(p+0x60,96);
    };
    for(unsigned animation:{63u,72u,73u,74u,75u,76u,78u}) {
        ground();psx_mod_write_half(p+0x60,animation);ttk::input.move_x=1;ttk::input.move_y=0;
        psx_mod_write_half(p+0x358,0);ttk::jump_pending=true;
        call(0x8005a210,p,0x800c2754,0x80041b34);
        if(animation<76) {
            assert(psx_mod_read_half(p+0x60)==96 && !ttk::jump_pending);
            assert(psx_mod_read_half(p+0x68)==0 && psx_mod_read_half(p+0x6a)==0);
        } else assert(psx_mod_read_half(p+0x60)==animation && ttk::jump_pending);
    }
    // D08Z6 first press during grounded wall-stop preparation. These poses
    // have a camera-only lease; waiting for idle would expire a short tap.
    for(unsigned animation:{94u,95u}) for(int reject=0;reject<7;++reject) {
        setenv("DNTTK_JUMP",reject==1?"assisted":"manual",1);
        ground();psx_mod_write_half(p+0x60,animation);ttk::input.move_x=0;ttk::input.move_y=1;
        ttk::running=reject!=2;ttk::jump_pending=true;
        psx_mod_write_half(p+0x358,0);psx_mod_write_word(p+0x20c,0);
        if(reject==3)psx_mod_write_byte(p+0x22d,9);
        if(reject==4)ttk::input.held[ttk::crouch]=true;
        if(reject==5)ttk::input.held[ttk::original_aim]=true;
        call(0x8003ade4,c,p,0x80025ee8);
        if(reject==6)++ttk::input.epoch;
        call(0x8005a210,p,0x800c2754,0x80041b34);
        assert(psx_mod_read_half(p+0x60)==(reject?animation:96));
        assert(ttk::jump_pending==bool(reject));
    }
    unsetenv("DNTTK_JUMP");ttk::running=false;ttk::jump_pending=false;
    std::puts("PASS: D08Z6 first buffered press during grounded run stop; assisted/walk/landing/crouch/aim/epoch exclusions");
    ground();psx_mod_write_half(p+0x60,72);ttk::input.move_x=1;
    psx_mod_write_half(p+0x358,3);psx_mod_write_half(p+0x35a,100);psx_mod_write_word(0x800d21fc,1);
    ttk::jump_pending=true;call(0x8005a210,p,0x800c2754,0x80041b34);
    assert(psx_mod_read_half(p+0x60)==72 && ttk::jump_pending); // original jet thrust
    psx_mod_write_half(p+0x358,0);ttk::jump_pending=false;
    // A fresh run-start press clears only original stride jump suppression.
    for(int reject=0;reject<6;++reject) {
        ground();psx_mod_write_half(p+0x60,76);ttk::input.move_y=1;
        ttk::jump_pending=true;psx_mod_write_word(p+0x228,0x108);
        uint32_t caller=0x80048664;
        if(reject==1)ttk::jump_pending=false;
        if(reject==2)ttk::input.held[ttk::crouch]=true;
        if(reject==3)ttk::input.held[ttk::original_aim]=true;
        if(reject==4)caller+=4;
        if(reject==5)++ttk::input.epoch;
        call(0x80053500,p,0,caller);
        assert(psx_mod_read_word(p+0x228)==(reject?0x108:0x100));
        assert(psx_mod_read_half(p+0x60)==76 && ttk::jump_pending==(reject!=1));
    }
    ttk::jump_pending=false;
    for(auto direction:std::vector<std::pair<float,float>>{{0,1},{0,-1},{1,0},{-1,0},{.707107f,.707107f},{-.707107f,-.707107f}}) {
        ground();ttk::input.move_x=direction.first;ttk::input.move_y=direction.second;
        // Camera observes new 96 before its initializer, clearing normal lease.
        // D22B: the camera-only jump lease continues it in third person too.
        call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::movement_ready() && !ttk::directional_takeoff_ready());
        writes=0;call(0x800493a4,p,0,0x8005a3a0);assert(writes==0);
        call(0x8003ade4,c,p,0x80025ee8);
        assert(ttk::directional_takeoff_ready() && ttk::locomotion_input_ready() && !ttk::movement_ready());
        // Releasing both direction and jump cannot change the accepted takeoff.
        ttk::input.move_x=ttk::input.move_y=0;
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::directional_takeoff_ready());
        psx_mod_write_word(js+0x44,0x800549c4);
        psx_mod_write_word(js+0x10,0);psx_mod_write_word(js+0x18,375);
        writes=0;call(0x800780b4,p,js+0x10,0x8007985c,js);
        assert(writes>0 && std::abs((int32_t)psx_mod_read_word(js+0x10)-375*direction.first)<1);
        assert(std::abs((int32_t)psx_mod_read_word(js+0x18)-375*direction.second)<1);
        assert(psx_mod_read_word(js+0x30)==psx_mod_read_word(p+4)+psx_mod_read_word(js+0x10));
        psx_mod_write_half(p+0x60,98);psx_mod_write_byte(p+0x22c,9);
        psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,1000);psx_mod_write_word(p+0x1f8,-900);
        call(0x8003ebf4,p,0,0x80055934);
        assert(std::abs((int32_t)psx_mod_read_word(p+0x1f4)-1000*direction.first)<1);
        assert(std::abs((int32_t)psx_mod_read_word(p+0x1fc)-1000*direction.second)<1);
        assert((int32_t)psx_mod_read_word(p+0x1f8)==-900);
        writes=0;call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::airborne_input_ready());
        unsigned saved_equipment=psx_mod_read_byte(p+0x3b8),saved_upper=psx_mod_read_half(p+0x74);
        psx_mod_write_byte(p+0x3b8,2);psx_mod_write_half(p+0x74,7);
        assert(!ttk::interaction_holster_ready());
        psx_mod_write_half(p+0x74,5);psx_mod_write_word(p+0x224,4);assert(!ttk::interaction_holster_ready());
        psx_mod_write_word(p+0x224,0);assert(ttk::interaction_holster_ready());
        psx_mod_write_byte(p+0x3b8,0);psx_mod_write_half(p+0x74,1);assert(!ttk::interaction_ready());
        psx_mod_write_half(p+0x74,104);psx_mod_write_word(p+0x224,4);assert(!ttk::interaction_ready());
        psx_mod_write_word(p+0x224,0);assert(ttk::interaction_ready());
        psx_mod_write_byte(p+0x3b8,saved_equipment);psx_mod_write_half(p+0x74,saved_upper);
        transition.gpr[31]=0x8005a5a8;
        for(unsigned upper:{0u,1u,6u,21u,30u,36u,40u}) {
            psx_mod_write_byte(p+0x3b8,upper<2?0:2);
            ttk::auto_stow=true;call(0x8005a210,p,0x800c2754,0x80041b34);
            psx_mod_write_half(p+0x74,upper);psx_mod_write_half(0x801effd2,1024);
            hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==4096);
            hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==4096);
            ttk::auto_stow=false;call(0x8005a210,p,0x800c2754,0x80041b34);
            psx_mod_write_half(0x801effd2,1024);
            hooks().at(0x80059db0)(&transition,0x80059db0);assert(psx_mod_read_half(0x801effd2)==1024);
        }

    }
    // Space-first: neutral 96 keeps ownership, then picks up direction without
    // restarting preparation or supplying a phantom Forward while neutral.
    for(auto direction:std::vector<std::pair<float,float>>{{0,1},{0,-1},{1,0},{-1,0},{.707107f,.707107f},{-.707107f,-.707107f}}) {
        ground();ttk::input.move_x=ttk::input.move_y=0;
        call(0x800493a4,p,0,0x8005a4d0);call(0x8003ade4,c,p,0x80025ee8);
        assert(ttk::locomotion_input_ready() && !ttk::directional_takeoff_ready());
        psx_mod_write_half(p+0x68,1);psx_mod_write_half(p+0x62,7);
        ttk::input.sequence+=3;call(0x8003ade4,c,p,0x80025ee8);
        ttk::input.move_x=direction.first;ttk::input.move_y=direction.second;
        writes=0;call(0x8005a210,p,0x800c2754,0x80041b34);
        assert(writes==0 && ttk::directional_takeoff_ready());
        assert(psx_mod_read_half(p+0x68)==1 && psx_mod_read_half(p+0x62)==7);
        ttk::input.move_x=ttk::input.move_y=0;
        psx_mod_write_half(p+0x60,98);psx_mod_write_byte(p+0x22c,9);
        psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,1000);psx_mod_write_word(p+0x1f8,-900);
        call(0x8003ebf4,p,0,0x80055934);
        assert(std::abs((int32_t)psx_mod_read_word(p+0x1f4)-1000*direction.first)<1);
        assert(std::abs((int32_t)psx_mod_read_word(p+0x1fc)-1000*direction.second)<1);
        assert((int32_t)psx_mod_read_word(p+0x1f8)==-900);
    }
    ground();ttk::input.move_x=ttk::input.move_y=0;
    call(0x800493a4,p,0,0x8005a4d0);call(0x8003ade4,c,p,0x80025ee8);
    ++ttk::input.epoch;ttk::input.move_y=1;
    call(0x8005a210,p,0x800c2754,0x80041b34);assert(!ttk::directional_takeoff_ready());
    ground();ttk::input.move_x=ttk::input.move_y=0;
    call(0x800493a4,p,0,0x8005a4d0);call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0x60,97);psx_mod_write_byte(p+0x22c,9);ttk::input.move_y=1;
    writes=0;call(0x8005a210,p,0x800c2754,0x80041b34);call(0x8003ebf4,p,0,0x80055bc4);
    assert(writes==0 && !ttk::directional_takeoff_ready()); // no midair reinjection
    for(int rejection=0;rejection<10;++rejection) {
        ground();ttk::input.move_x=1;ttk::input.move_y=0;
        uint32_t actor=p,caller=0x8005a3a0;
        if(rejection==0)ttk::input.move_x=0;
        if(rejection==1)ttk::input.active=false;
        if(rejection==2)ttk::input.held[ttk::original_aim]=true;
        if(rejection==3)ttk::input.held[ttk::crouch]=true;
        if(rejection==4)++ttk::input.epoch;
        if(rejection==5)ttk::input.sequence+=5;
        if(rejection==6)psx_mod_write_byte(p+0x22c,3);
        if(rejection==7)actor=p+0x8a4;
        if(rejection==8)caller+=4;
        if(rejection==9)poke(0x5492c);
        call(0x800493a4,actor,0,caller);assert(!ttk::directional_takeoff_ready());
        if(rejection==9)poke(0x5492c);
    }
    ground();ttk::input.move_x=1;call(0x800493a4,p,0,0x8005a3a0);
    ttk::input.sequence+=61;assert(!ttk::directional_takeoff_ready());
    // A refused directional takeoff may continue forward only once ascending
    // clearance accepts it. Preserve vertical velocity, support and position.
    ground();ttk::input.move_x=0;ttk::input.move_y=1;
    call(0x800493a4,p,0,0x8005a3a0);call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0x60,97);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1f8,-4000);psx_mod_write_word(p+0x1c4,123);
    psx_mod_write_word(p+0x1c8,567);psx_mod_write_word(p+8,-12000);psx_mod_write_word(p+0x10,-11500);
    terrain_result=3;terrain_floor=100;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    assert(psx_mod_read_half(p+0x60)==97 && psx_mod_read_word(p+0x1c4)==123);
    assert((int32_t)terrain_reference==-11500 && psx_mod_read_word(p+0x1c8)==567);
    terrain_result=0;terrain_floor=-20;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(psx_mod_read_half(p+0x60)==97);
    terrain_floor=100;
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(psx_mod_read_half(p+0x60)==98);
    psx_mod_write_word(p+0x1f8,-9000); // model original initializer
    psx_mod_write_word(p+0x1fc,6000);psx_mod_write_word(p+0x1f4,0);
    call(0x8003ebf4,p,0,0x80055934);assert((int32_t)psx_mod_read_word(p+0x1f8)==-4000);
    ground();ttk::input.move_x=0;ttk::input.move_y=1;
    call(0x800493a4,p,0,0x8005a3a0);call(0x8003ade4,c,p,0x80025ee8);
    psx_mod_write_half(p+0x60,97);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1f8,-9000);psx_mod_write_word(p+0x1c8,-11264);
    psx_mod_write_word(p+0x1c4,-383);psx_mod_write_word(p+8,-11775);psx_mod_write_word(p+0x10,-11380);
    terrain_result=0;terrain_floor=100; // head-height query alone would falsely accept
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(psx_mod_read_half(p+0x60)==97);
    psx_mod_write_word(p+0x10,-11648);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);assert(psx_mod_read_half(p+0x60)==98);
    // D08Z6: clearance, not proximity, permits forward acceleration. Manual
    // running builds speed through the ordinary steering and sweep path;
    // walking/assisted keep the standing cap, and release adds no impulse.
    for(int variant=0;variant<3;++variant) {
        setenv("DNTTK_JUMP",variant==2?"assisted":"manual",1);
        ground();ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=variant!=1;
        psx_mod_write_word(p+0x20c,0);
        call(0x800493a4,p,0,0x8005a3a0);call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_half(p+0x60,97);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1f8,-4000);psx_mod_write_word(p+0x1c4,0);
        psx_mod_write_word(p+0x10,-11500);psx_mod_write_half(p+0xba,11);
        terrain_result=3;terrain_floor=100;
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        assert(psx_mod_read_half(p+0x60)==97);
        terrain_result=0;
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        assert(psx_mod_read_half(p+0x60)==98);
        psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,2734);
        psx_mod_write_word(p+0x1f8,-9000);
        const uint32_t xyz[]={psx_mod_read_word(p+4),psx_mod_read_word(p+8),psx_mod_read_word(p+12)};
        call(0x8003ebf4,p,0,0x80055934);
        assert((int32_t)psx_mod_read_word(p+0x1f8)==-4000);
        assert((int32_t)psx_mod_read_word(p+0x1fc)==(variant==0?4134:2734));
        for(int i=0;i<12;++i)call(0x8003ebf4,p,0,0x80055934);
        assert((int32_t)psx_mod_read_word(p+0x1fc)==(variant==0?10000:2734));
        for(int i=0;i<3;++i)assert(psx_mod_read_word(p+4+4*i)==xyz[i]);
        ttk::input.move_y=0;psx_mod_write_word(p+0x1fc,0);writes=0;
        call(0x8003ebf4,p,0,0x80055934);assert(writes==0);
    }
    unsetenv("DNTTK_JUMP");ttk::running=false;
    std::puts("PASS: D08Z6 cleared running jump builds bounded horizontal speed; vertical/position, release, walking and assisted preserved");
    // Only the original clear drop result, in the small-drop range, selects
    // the original run/fall dispatcher for a walking-speed step.
    for(auto check:std::vector<std::pair<int,int>>{{6,384},{6,900},{3,384},{6,200}}) {
        ground();psx_mod_write_half(p+0x60,72);ttk::input.move_y=1;ttk::input.move_x=0;
        ttk::input.held[ttk::jump]=false;ttk::running=false;
        call(0x8003ade4,c,p,0x80025ee8);
        terrain_result=check.first;terrain_floor=check.second;
        psx_mod_write_word(p+0x1c4,123);
        call(0x80048410,p,0,0x8004b634,0x801fff00);
        assert(psx_mod_read_half(p+0x60)==(check.first==6 && check.second>256?76:72));
        assert(psx_mod_read_word(p+0x1c4)==123);
    }
    for(int height:{384,900}) {
        ground();psx_mod_write_half(p+0x60,72);ttk::input.move_y=1;ttk::input.move_x=0;
        call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1c4,height);psx_mod_write_word(p+0x20c,0);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        call(0x8003ade4,c,p,0x80025ee8);
        assert(ttk::short_fall_input_ready()==(height==384));
        if(height==384) {
            psx_mod_write_word(p+0x1f4,3058);psx_mod_write_word(p+0x1fc,0);
            psx_mod_write_word(p+0x1f8,456);
            call(0x8003ebf4,p,0,0x80055934);
            auto vx=(int32_t)psx_mod_read_word(p+0x1f4),vz=(int32_t)psx_mod_read_word(p+0x1fc);
            assert(std::abs(std::hypot(vx,vz)-2048)<1 && psx_mod_read_word(p+0x1f8)==456);
            psx_mod_write_word(p+0x1f4,0);psx_mod_write_word(p+0x1fc,0);
            call(0x8003ebf4,p,0,0x80055934); // collision must keep its stopped velocity
            assert(psx_mod_read_word(p+0x1f4)==0 && psx_mod_read_word(p+0x1fc)==0);
        }
        ttk::input.active=false;assert(!ttk::short_fall_input_ready());ttk::input.active=true;
        psx_mod_write_half(p+0x60,74);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        // D08L1: a later walking ground stride also permits a large walk-off.
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1c4,900);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        call(0x8003ade4,c,p,0x80025ee8);assert(ttk::short_fall_input_ready());
    }
    // A running furniture fall inherits actual ground speed, once. Original
    // collision can subsequently stop it; the velocity is never reinjected.
    ground();ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=true;
    psx_mod_write_half(p+0x60,76);psx_mod_write_half(p+0xba,11);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,88);
    call(0x8003ade4,c,p,0x80025ee8);call(0x80053500,p,0,0x80048664);
    const double approach=psx_mod_read_half(p+0x100)*1024.0/11;
    psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1c4,384);psx_mod_write_word(p+0x20c,0);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    psx_mod_write_word(p+0x1f4,3058);psx_mod_write_word(p+0x1fc,0);psx_mod_write_word(p+0x1f8,456);
    call(0x8003ebf4,p,0,0x80055934);
    assert(std::abs((int32_t)psx_mod_read_word(p+0x1fc)-approach)<2 && psx_mod_read_word(p+0x1f8)==456);
    psx_mod_write_word(p+0x1fc,0);call(0x8003ebf4,p,0,0x80055934);assert(psx_mod_read_word(p+0x1fc)==0);
    psx_mod_write_half(p+0xba,0);ttk::running=false;
    std::puts("PASS: ascending clearance reference, retained vertical impulse, bounded small drops and expired landing lease");
    // D08L: running off a larger ledge straight from a ground stride departs
    // at ground speed, capped below the running jump. D08L1 caps a walking
    // departure at 2048; a fall following a jump (103) keeps original 3058.
    for(int variant=0;variant<4;++variant) {
        ground();ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=variant!=1;
        psx_mod_write_half(p+0x60,76);psx_mod_write_half(p+0xba,11);
        psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,variant==3?200:88);
        call(0x8003ade4,c,p,0x80025ee8);call(0x80053500,p,0,0x80048664);
        const double run_off=psx_mod_read_half(p+0x100)*1024.0/11;
        if(variant==2)psx_mod_write_half(p+0x60,103);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1c4,2048);psx_mod_write_word(p+0x20c,0);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1f4,3058);psx_mod_write_word(p+0x1fc,0);psx_mod_write_word(p+0x1f8,456);
        call(0x8003ebf4,p,0,0x80055934);
        const auto vz=(int32_t)psx_mod_read_word(p+0x1fc),vx=(int32_t)psx_mod_read_word(p+0x1f4);
        if(variant==0)assert(std::abs(vz-run_off)<2 && vx==0);
        else if(variant==3)assert(run_off>10000 && vz==10000);
        else if(variant==1)assert(vx==0 && vz==2048);
        else assert(vx==3058 && vz==0);
        assert(psx_mod_read_word(p+0x1f8)==456);
        psx_mod_write_word(p+0x1fc,0);call(0x8003ebf4,p,0,0x80055934);assert(psx_mod_read_word(p+0x1fc)==0);
        psx_mod_write_half(p+0x60,74);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    }
    // A spiked stride estimate (run start) is capped below the running jump.
    ground();ttk::running=true;psx_mod_write_half(p+0x60,76);psx_mod_write_half(p+0xba,11);psx_mod_write_half(p+0x100,200);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00); // retire the previous landing
    call(0x8003ade4,c,p,0x80025ee8);call(0x80053500,p,0,0x80048664);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
    psx_mod_write_word(p+0x1c4,384);psx_mod_write_word(p+0x20c,0);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    psx_mod_write_word(p+0x1f4,3058);psx_mod_write_word(p+0x1fc,0);
    call(0x8003ebf4,p,0,0x80055934);assert(psx_mod_read_word(p+0x1fc)==10000);
    psx_mod_write_half(p+0x60,74);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
    psx_mod_write_half(p+0xba,0);ttk::running=false;
    std::puts("PASS: running large-ledge run-off keeps capped ground speed; walking capped at 2048 and post-jump falls unchanged; spiked strides capped");
    terrain_result=3;terrain_floor=-1000;
    // Original grant dispatch, flags, unsupported-context rejection, and
    // hostile-only hide/show ownership. Guest routines are modeled separately.
    auto cheat=[&](ttk::Cheat command){ttk::pending_cheat=command;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);};
    ground();psx_mod_write_half(p+0x60,63);ttk::input.move_x=ttk::input.move_y=0;
    call(0x8003ade4,c,p,0x80025ee8);psx_mod_write_word(0x800c27bc,1);
    // D08Q2: the reported Continue state strands only jetpack pending at the
    // closed/off endpoint. Exercise the actual selection hook, including the
    // negative cases that must keep original item transitions/restrictions.
    {
        uint8_t saved_player[0x8a4];std::memcpy(saved_player,ram+(p&0x1fffff),sizeof saved_player);
        ttk::input.command_count=0;
        psx_mod_write_byte(p+0x3b8,2);psx_mod_write_half(p+0x74,5);
        psx_mod_write_word(p+0x834,0x40000000);
        // (Steroids, item 4, are not held here: held ones would follow the medkit, D08A4.)
        for(unsigned i:{1u,2u,3u,5u}){psx_mod_write_half(p+0x354+4*i,1);psx_mod_write_half(p+0x356+4*i,100);}
        auto stranded=[&](){
            ttk::modern=true;ttk::input.active=true;
            psx_mod_write_word(p,0);psx_mod_write_half(p+0x32,6250);
            psx_mod_write_word(p+0x224,0);psx_mod_write_half(p+0x358,0x8001);
            psx_mod_write_half(p+0x35a,9000);psx_mod_write_half(p+0x84c,0);
            psx_mod_write_word(p+0x884,1);psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        };
        auto poll=[&](){call(0x80058120,p,0,0x80041c44);};
        stranded();poll();assert(psx_mod_read_half(p+0x358)==1 && psx_mod_read_half(p+0x35a)==9000);
        // Picker and activation use the repaired original record, without a cheat.
        psx_mod_write_half(0x800c3f94,5);request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==1);
        ttk::input.commands[0]=ttk::jetpack;++ttk::input.command_serial;
        call(0x80058120,p,0,0x80041c44,0x801fff00);assert(psx_mod_read_half(p+0x358)==3 && psx_mod_read_byte(p+0x22c)==10);
        const uint32_t start_y=psx_mod_read_word(p+8);
        ttk::input.held[ttk::jump]=false;ttk::input.held[ttk::crouch]=false;
        ttk::input.sequence+=2;call(0x8003ade4,c,p,0x80025ee8);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        assert((int32_t)psx_mod_read_word(p+0x1f8)<0 && psx_mod_read_word(p+8)==start_y); // native collision applies lift

        // Modern J also acquires a free fall, through the same guarded startup.
        psx_mod_write_half(p+0x358,1);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_half(p+0x60,108);psx_mod_write_word(p+0x224,0x41);
        ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);
        ttk::input.commands[0]=ttk::jetpack;++ttk::input.command_serial;
        call(0x80058120,p,0,0x80041c44,0x801fff00);
        assert(psx_mod_read_byte(p+0x22c)==10 && psx_mod_read_half(p+0x358)==3);
        // No ground-start boost or automatic clearance climb on a low air start.
        const uint32_t air_y=psx_mod_read_word(p+8);
        psx_mod_write_word(p+0x10,1000);psx_mod_write_word(p+0x1c8,1100);
        psx_mod_write_word(0x800d21fc,10);
        ttk::input.move_x=0;ttk::input.move_y=0;ttk::input.arrow_y=0;
        ttk::input.sequence+=2;call(0x8003ade4,c,p,0x80025ee8);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        assert(psx_mod_read_word(p+8)==air_y && (int32_t)psx_mod_read_word(p+0x1f8)==0);
        // Rapid off/on: the native close is still pending while Duke falls.
        psx_mod_write_half(p+0x358,0x8001);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_half(p+0x60,108);psx_mod_write_word(p+0x224,0x10000002);
        psx_mod_write_word(p+0x884,1);psx_mod_write_half(p+0x84c,2);
        ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);++ttk::input.command_serial;
        call(0x80058120,p,0,0x80041c44,0x801fff00);
        assert(psx_mod_read_byte(p+0x22c)==10 && psx_mod_read_half(p+0x358)==3 && psx_mod_read_half(p+0x84c)==5);

        for(unsigned flag:{0x1000u,0x10000000u,4u,8u}) {
            stranded();psx_mod_write_word(p+0x224,flag);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        }
        for(unsigned phase:{1u,4u,5u}) {
            stranded();psx_mod_write_half(p+0x84c,phase);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        }
        for(unsigned flags:{0u,1u,3u,0x8000u,0x8003u}) {
            stranded();psx_mod_write_half(p+0x358,flags);poll();assert(psx_mod_read_half(p+0x358)==flags);
        }
        stranded();psx_mod_write_word(p+0x884,2);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();psx_mod_write_word(p,2);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();psx_mod_write_half(p+0x32,0);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();psx_mod_write_byte(p+0x22c,10);poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();ttk::input.active=false;poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();ttk::modern=false;poll();assert(psx_mod_read_half(p+0x358)==0x8001);
        stranded();poke(0x4001c);poll();assert(psx_mod_read_half(p+0x358)==0x8001);poke(0x4001c);
        // Empty fuel is still unusable after housekeeping; unrelated bits survive.
        stranded();psx_mod_write_half(p+0x358,0x8041);psx_mod_write_half(p+0x35a,0);poll();
        assert(psx_mod_read_half(p+0x358)==0x41 && psx_mod_read_half(p+0x35a)==0);
        request(ttk::jetpack);assert(psx_mod_read_half(p+0x358)==0x41);
        std::memcpy(ram+(p&0x1fffff),saved_player,sizeof saved_player);++g_dirty_ram_code_gen;
        ttk::input.command_count=0;
        std::puts("PASS: D08Q2 stranded jetpack repair, picker/J, empty fuel, transition/death/Vanilla/code guards");
    }
    cheat_calls.clear();cheat(ttk::Cheat::Stuff);
    assert((cheat_calls==std::vector<uint32_t>{0x8003d7bc,0x8003d738,0x8003d840}));
    {
        // D08G4: the inventory cheats set full armour (10000, never lower) and
        // stop running steroids, leaving the dose held; the grant is stubbed, so
        // the flags stand in for what it leaves (bit 0, full amount).
        const uint16_t armour=psx_mod_read_half(p+0x234);
        for(auto c:{ttk::Cheat::Stuff,ttk::Cheat::Items,ttk::Cheat::Inventory}) {
            psx_mod_write_half(p+0x234,2500);psx_mod_write_half(p+0x364,3);psx_mod_write_half(p+0x366,9000);
            cheat(c);assert(psx_mod_read_half(p+0x234)==10000 && psx_mod_read_half(p+0x364)==1 && psx_mod_read_half(p+0x366)==9000);
        }
        psx_mod_write_half(p+0x364,1);cheat(ttk::Cheat::Inventory);assert(psx_mod_read_half(p+0x364)==1);  // held stays held
        psx_mod_write_half(p+0x234,2500);psx_mod_write_half(p+0x364,3);
        cheat(ttk::Cheat::Keys);assert(psx_mod_read_half(p+0x234)==2500 && psx_mod_read_half(p+0x364)==3);
        cheat(ttk::Cheat::Weapons);assert(psx_mod_read_half(p+0x234)==2500);
        psx_mod_write_half(p+0x234,armour);psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);
        std::puts("PASS: D08G4 dnstuff/dnitems/dninventory give full armour and stop running steroids (dose held); dnkeys/dnweapons unchanged");
    }
    psx_mod_write_half(0x800c3cc6,0);cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==1);
    cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==0);
    {
        // D08Q4: god mode raises health to the type maximum (never lowers an
        // Atomic Health surplus) and keeps an owned jetpack at the item cap.
        const uint32_t saved_types=psx_mod_read_word(0x800d2660),most=0x801e8000+28*psx_mod_read_half(p+0x2c)+0xc;
        const uint16_t saved_most=psx_mod_read_half(most),saved_health=psx_mod_read_half(p+0x32);
        const uint16_t saved_jet=psx_mod_read_half(p+0x358),saved_fuel=psx_mod_read_half(p+0x35a);
        psx_mod_write_word(0x800d2660,0x801e8000);psx_mod_write_half(most,10000);
        assert(psx_mod_read_half(0x800c2716)==9000);
        psx_mod_write_half(p+0x32,3000);psx_mod_write_half(p+0x358,1);psx_mod_write_half(p+0x35a,100);
        cheat(ttk::Cheat::God);assert(psx_mod_read_half(p+0x32)==10000 && psx_mod_read_half(p+0x35a)==9000);
        psx_mod_write_half(p+0x32,15000);psx_mod_write_half(p+0x35a,8983);cheat(ttk::Cheat::None);
        assert(psx_mod_read_half(p+0x32)==15000 && psx_mod_read_half(p+0x35a)==9000);
        psx_mod_write_half(p+0x358,0);psx_mod_write_half(p+0x35a,0);cheat(ttk::Cheat::None); // pack lost while on: given back
        assert(psx_mod_read_half(p+0x358)==1 && psx_mod_read_half(p+0x35a)==9000);
        psx_mod_write_half(p+0x358,1);cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==0);
        psx_mod_write_half(p+0x32,3000);psx_mod_write_half(p+0x35a,100);cheat(ttk::Cheat::None);
        assert(psx_mod_read_half(p+0x32)==3000 && psx_mod_read_half(p+0x35a)==100);
        // Turning god mode on gives a missing jetpack (owned bit, full fuel);
        // the jetpack's on bit is untouched and turning it off keeps the pack.
        psx_mod_write_half(p+0x358,0);psx_mod_write_half(p+0x35a,0);
        cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==1 && psx_mod_read_half(p+0x358)==1 && psx_mod_read_half(p+0x35a)==9000);
        cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==0 && psx_mod_read_half(p+0x358)==1);
        // D08G3: upgrade bit on weapons 4,5,7,8,9,10 and the persistent mask only.
        uint16_t saved_weapons[36];for(unsigned i=0;i<36;++i)saved_weapons[i]=psx_mod_read_half(p+0x2c4+4*i);
        const uint8_t saved_mask=psx_mod_read_byte(p+0x85f);
        uint16_t saved_ammo[36];for(unsigned i=0;i<36;++i)saved_ammo[i]=psx_mod_read_half(p+0x2c6+4*i);
        for(unsigned i=0;i<36;++i){psx_mod_write_half(p+0x2c4+4*i,i==7||i==9?1:0);psx_mod_write_half(p+0x2c6+4*i,0);}
        psx_mod_write_half(p+0x2c6+4*7,30000);psx_mod_write_half(p+0x2c6+4*9,5);psx_mod_write_half(p+0x2c6+4*27,7);
        psx_mod_write_byte(p+0x85f,0x40);
        cheat(ttk::Cheat::Upgrade);
        for(unsigned i=0;i<36;++i) {
            const bool up=i==4||i==5||i==7||i==8||i==9||i==10,owned=i==7||i==9||i==28||i==27;
            assert(psx_mod_read_half(p+0x2c4+4*i)==((owned?1:0)|(up?8:0)));
        }
        // Owned Gatling carries ammo to 28 up to its capacity; a larger
        // upgraded stock (27) is kept; the unowned RPG gives 29 nothing.
        assert(psx_mod_read_half(p+0x2c6+4*28)==psx_mod_read_half(0x800c4594+44*28));
        assert(psx_mod_read_half(p+0x2c6+4*27)==7 && psx_mod_read_half(p+0x2c6+4*29)==0);
        assert(psx_mod_read_half(p+0x2c6+4*7)==30000 && psx_mod_read_byte(p+0x85f)==0x7f);
        for(unsigned i=0;i<36;++i)psx_mod_write_half(p+0x2c6+4*i,saved_ammo[i]);
        for(unsigned i=0;i<36;++i)psx_mod_write_half(p+0x2c4+4*i,saved_weapons[i]);psx_mod_write_byte(p+0x85f,saved_mask);
        psx_mod_write_half(most,saved_most);psx_mod_write_word(0x800d2660,saved_types);psx_mod_write_half(p+0x32,saved_health);
        psx_mod_write_half(p+0x358,saved_jet);psx_mod_write_half(p+0x35a,saved_fuel);
        std::puts("PASS: D08Q4 god mode health/jetpack pin and jetpack grant, Atomic surplus kept, off resumes; D08G3 upgrade bits and mask");
    }
    psx_mod_write_word(0x800c27bc,2);cheat_calls.clear();cheat(ttk::Cheat::Stuff);assert(cheat_calls.empty());
    psx_mod_write_word(0x800c27bc,1);ttk::input.active=false;cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==0);ttk::input.active=true;
    constexpr uint32_t table=0x801e1000,types=0x801e8000,enemy=0x801c0000,npc=0x801c0400;
    psx_mod_write_word(0x800de720,table);psx_mod_write_word(0x800d2660,types);psx_mod_write_word(0x800c56a0,5);
    psx_mod_write_byte(types+57*28+6,6);psx_mod_write_byte(types+167*28+6,8);
    for(unsigned i=0;i<5;++i){for(unsigned j=0;j<48;++j)psx_mod_write_byte(table+i*48+j,0);psx_mod_write_word(table+i*48,57);}
    psx_mod_write_word(table+4,enemy);psx_mod_write_word(table+20,1);
    psx_mod_write_word(enemy,1);psx_mod_write_half(enemy+0x2c,57);psx_mod_write_half(enemy+0x32,1234);psx_mod_write_byte(enemy+0x15,6);
    psx_mod_write_word(table+48,167);psx_mod_write_word(table+52,npc);psx_mod_write_word(table+68,1);
    psx_mod_write_word(table+96+20,2); // dead stays dead
    psx_mod_write_word(table+192+20,16); // scripted-disabled stays disabled
    enemy_restore_actor=enemy;
    cheat(ttk::Cheat::Monsters);
    assert(psx_mod_read_word(table+4)==0 && psx_mod_read_word(table+20)==16);
    assert(psx_mod_read_word(table+52)==npc && psx_mod_read_word(table+68)==1);
    assert(psx_mod_read_word(table+96+20)==2 && psx_mod_read_word(table+144+20)==16);
    cheat(ttk::Cheat::Monsters);
    assert(psx_mod_read_word(table+4)==enemy && psx_mod_read_half(enemy+0x32)==1234);
    assert(psx_mod_read_word(table+144+20)==0 && psx_mod_read_word(table+192+20)==16);
    // Scene teardown clears host ownership; no writes into a recycled table.
    cheat(ttk::Cheat::Monsters);call(0x8002b9f4,1,0,0x80025474);
    writes=0;cheat(ttk::Cheat::Monsters); // all rows already inactive/dead/disabled
    assert(psx_mod_read_word(table+20)==16);
    // Concealed apartment pickup is denied by dispatch, never by moving/deleting
    // its object or changing the player's inventory. Exposed/other rooms fall through.
    ground();psx_mod_write_word(0x800c27bc,1);psx_mod_write_word(0x800dd878,0);
    constexpr uint32_t bed=0x801d6004,pickup=0x801dad64;
    psx_mod_write_word(0x800d257c,bed);psx_mod_write_half(bed+0x2c,5);
    psx_mod_write_byte(bed+0x34,11);psx_mod_write_byte(bed+0x2e,8);
    psx_mod_write_half(pickup+0x2c,58);psx_mod_write_byte(pickup+0x34,0);
    psx_mod_write_byte(pickup+0x15,1);psx_mod_write_byte(pickup+0x2e,8);
    auto pickup_call=[&](uint32_t caller=0x8007fe6c){
        CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[5]=pickup;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=caller;
        hooks().at(0x80081a48)(&cpu,0x80081a48);return cpu.gpr[5];
    };
    const auto old_inventory=psx_mod_read_word(p+0x2c4+12*4);
    auto denied=pickup_call();assert(denied!=pickup && psx_mod_read_half(denied+0x2c)==0xffff);
    assert(psx_mod_read_half(pickup+0x2c)==58 && psx_mod_read_word(p+0x2c4+12*4)==old_inventory);
    psx_mod_write_word(0x800dd878,0x400);assert(pickup_call()==pickup);
    psx_mod_write_word(0x800dd878,0);psx_mod_write_byte(pickup+0x2e,51);assert(pickup_call()==pickup);
    psx_mod_write_byte(pickup+0x2e,8);assert(pickup_call(0x8007fe68)==pickup);
    ttk::modern=false;assert(pickup_call()==pickup);ttk::modern=true;
    ttk::input.active=false;assert(pickup_call()==pickup);ttk::input.active=true;
    auto pickup_code=psx_mod_read_word(0x80081a48);psx_mod_write_word(0x80081a48,pickup_code^1);
    assert(pickup_call()==pickup);psx_mod_write_word(0x80081a48,pickup_code);
    constexpr uint32_t sw=0x801d64e4,query_sp=0x801ffd00;
    psx_mod_write_word(0x800d25b0,sw);psx_mod_write_half(sw+0x2c,595);psx_mod_write_byte(sw+0x34,24);
    psx_mod_write_byte(sw+0x2e,psx_mod_read_byte(p+0x2e));
    psx_mod_write_word(query_sp+0x84,0x80077fa4);psx_mod_write_word(query_sp+0xb0,0x800518c0);
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096);
    for(unsigned j=0;j<3;++j)psx_mod_write_word(query_sp+0x58+4*j,psx_mod_read_word(p+4+4*j));
    psx_mod_write_word(query_sp+0x58,psx_mod_read_word(p+4)+100);
    psx_mod_write_word(query_sp+0x60,psx_mod_read_word(p+12)+400);
    auto switch_query=[&](){
        CPUState cpu{};cpu.gpr[4]=query_sp+0x18;cpu.gpr[5]=sw;cpu.gpr[6]=query_sp+0x38;
        cpu.gpr[7]=query_sp+0x48;cpu.gpr[20]=p+0x1c;cpu.gpr[29]=query_sp;cpu.gpr[31]=0x80077e18;
        writes=0;hooks().at(0x80076330)(&cpu,0x80076330);return writes;
    };
    assert(switch_query()>0 && psx_mod_read_word(query_sp+0x28)>psx_mod_read_word(query_sp+0x58));
    psx_mod_write_word(query_sp+0x60,psx_mod_read_word(p+12)+900);assert(!switch_query());
    psx_mod_write_word(query_sp+0x60,psx_mod_read_word(p+12)-400);assert(!switch_query());
    psx_mod_write_word(query_sp+0x60,psx_mod_read_word(p+12)+400);
    ttk::modern=false;assert(!switch_query());ttk::modern=true;
    psx_mod_write_word(query_sp+0xb0,0x800518bc);assert(!switch_query());
    {
        // D08A4: a steroids pickup (type 638) is held instead of run. The
        // original case writes the full amount and bit 1; at its sound call
        // (ra 0x800828d8) bit 1 goes back off and bit 0 on. Held: the next
        // pickup is left on the ground. Running: the original refresh.
        constexpr uint32_t pills=0x801dae00;psx_mod_write_half(pills+0x2c,638);
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);
        auto touch=[&](uint32_t caller=0x8007fe6c){
            CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[5]=pills;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=caller;
            hooks().at(0x80081a48)(&cpu,0x80081a48);return cpu.gpr[5];
        };
        auto original_case=[&](uint32_t sound_ra=0x800828d8){
            psx_mod_write_half(p+0x366,psx_mod_read_half(0x800c2722));psx_mod_write_half(p+0x364,psx_mod_read_half(p+0x364)|2);
            CPUState cpu{};cpu.gpr[17]=p;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=sound_ra;
            hooks().at(0x8006b73c)(&cpu,0x8006b73c);
        };
        assert(psx_mod_read_half(0x800c2722)==9000);
        assert(touch()==pills);original_case();
        assert(psx_mod_read_half(p+0x364)==1 && psx_mod_read_half(p+0x366)==9000);
        auto left=touch();assert(left!=pills && psx_mod_read_half(left+0x2c)==0xffff && psx_mod_read_half(p+0x364)==1);
        psx_mod_write_half(p+0x364,3);psx_mod_write_half(p+0x366,100);
        assert(touch()==pills);original_case();assert(psx_mod_read_half(p+0x364)==1 && psx_mod_read_half(p+0x366)==9000);  // D08A15: run stops, held
        psx_mod_write_half(p+0x364,2);psx_mod_write_half(p+0x366,100);  // dnhyper run
        assert(touch()==pills);original_case();assert(psx_mod_read_half(p+0x364)==1 && psx_mod_read_half(p+0x366)==9000);
        psx_mod_write_half(p+0x364,0);
        assert(touch(0x8007fe68)==pills);original_case();assert(psx_mod_read_half(p+0x364)==2); // other caller
        psx_mod_write_half(p+0x364,0);
        assert(touch()==pills);original_case(0x800936e8);assert(psx_mod_read_half(p+0x364)==2); // other sound call
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(pills+0x2c,580);
        assert(touch()==pills);original_case();assert(psx_mod_read_half(p+0x364)==2); // not steroids
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(pills+0x2c,638);
        ttk::modern=false;assert(touch()==pills);original_case();assert(psx_mod_read_half(p+0x364)==2);ttk::modern=true;
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);
        std::puts("PASS: D08A4 steroids pickup held at the original sound call; held leaves it, running stops and is held full (D08A15); caller/sound/type/Vanilla fallbacks");
    }
    {
        // D08A13: at Duke's damage handler (0x800a40a8) running steroids lose
        // bit 1 for that call only, so the original's shield (0x800a4154: take
        // 1500, no damage) is skipped and the timer is left alone; the next
        // hook (Duke's update 0x800412a4 here) puts the bit back.
        auto damage=[&](uint32_t victim){CPUState cpu{};cpu.gpr[4]=victim;cpu.gpr[29]=0x801fff00;hooks().at(0x800a40a8)(&cpu,0x800a40a8);};
        auto update=[&]{CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[29]=0x801fff00;hooks().at(0x800412a4)(&cpu,0x800412a4);};
        auto flags=[&]{return psx_mod_read_half(p+0x364);};
        psx_mod_write_half(p+0x364,3);psx_mod_write_half(p+0x366,4000);
        damage(p);assert(flags()==1 && psx_mod_read_half(p+0x366)==4000);
        assert(std::strstr(ttk::controls_debug_json(),"\"damage_hidden\":true"));
        damage(p);assert(flags()==1);  // a second hit: off again for its own call
        update();assert(flags()==3 && psx_mod_read_half(p+0x366)==4000);
        assert(std::strstr(ttk::controls_debug_json(),"\"damage_hidden\":false"));
        psx_mod_write_half(p+0x364,2);damage(p);assert(flags()==0);update();assert(flags()==2);  // dnhyper
        damage(p);psx_mod_write_half(p+0x366,0);update();assert(flags()==0);  // ended meanwhile: stays off
        psx_mod_write_half(p+0x366,4000);psx_mod_write_half(p+0x364,3);
        damage(0x801d0000);assert(flags()==3);  // another victim
        psx_mod_write_half(p+0x364,1);damage(p);assert(flags()==1);update();assert(flags()==1);  // held, not running
        psx_mod_write_half(p+0x364,3);ttk::modern=false;damage(p);assert(flags()==3);ttk::modern=true;  // Vanilla
        psx_mod_write_half(p+0x364,0);psx_mod_write_half(p+0x366,0);
        std::puts("PASS: D08A13 damage handler skips the steroid shield (timer untouched, bit 1 back at the next hook); dnhyper, expiry, other victim, held, Vanilla");
    }
    {
        // D08A14: while Duke is dead (word 0 bit 1, health 0) the next hook
        // clears steroids, running or held; alive, Vanilla and other bits stay.
        // Any shared hook will do (hook_body); 0x8001fc44 is the view composition's.
        auto update=[&]{CPUState cpu{};cpu.gpr[29]=0x801fff00;cpu.gpr[31]=0x800265a4;hooks().at(0x8001fc44)(&cpu,0x8001fc44);};
        auto flags=[&]{return psx_mod_read_half(p+0x364);};
        const uint32_t word0=psx_mod_read_word(p);const uint16_t health=psx_mod_read_half(p+0x32);
        psx_mod_write_half(p+0x32,700);psx_mod_write_half(p+0x364,3);psx_mod_write_half(p+0x366,4000);
        update();assert(flags()==3 && psx_mod_read_half(p+0x366)==4000);  // alive
        psx_mod_write_word(p,word0|2);psx_mod_write_half(p+0x32,0);
        ttk::modern=false;update();assert(flags()==3);ttk::modern=true;  // Vanilla keeps them
        update();assert(flags()==0 && psx_mod_read_half(p+0x366)==0);  // running dose
        psx_mod_write_half(p+0x364,2);psx_mod_write_half(p+0x366,4000);update();assert(flags()==0);  // dnhyper
        psx_mod_write_half(p+0x364,0x8001);psx_mod_write_half(p+0x366,9000);update();  // held: lost, other bits kept
        assert(flags()==0x8000 && psx_mod_read_half(p+0x366)==0);
        assert(std::strstr(ttk::controls_debug_json(),"\"death_clears\":3"));
        psx_mod_write_word(p,word0);psx_mod_write_half(p+0x32,health);psx_mod_write_half(p+0x364,0);
        std::puts("PASS: D08A14 death clears steroids (dose, dnhyper, held); alive and Vanilla keep them");
    }
    {
        // D08A12: a drawn dynamite (+0x3b8 = 0x0e02) stays unlit at the
        // held-throwable handler 0x8004dea4: fuse held full (0x800c4ed8),
        // this call's sparks (ra 0x8004e138) dark and sizzle (ra 0x8004e154)
        // refused; a request or the released holster button stows it like
        // the Holy Hand Grenade. Cross down lights it, and the original runs
        // from then on until the stick leaves the hand.
        const uint32_t cross_slot=psx_mod_read_word(0x800d1b50),holster_slot=psx_mod_read_word(0x800d1a90);
        const uint32_t cross=0x801e2000,holster=0x801e2010;
        psx_mod_write_word(0x800d1b50,cross);psx_mod_write_word(0x800d1a90,holster);
        const uint8_t index=psx_mod_read_byte(p+0x233);psx_mod_write_byte(p+0x233,0);
        const uint16_t lower=psx_mod_read_half(p+0x60);psx_mod_write_half(p+0x60,63);
        const uint32_t flags0=psx_mod_read_word(p+0x224);
        const uint16_t full=psx_mod_read_half(0x800c4ed8);assert(full==4000);
        auto handler=[&](uint32_t who){CPUState cpu{};cpu.gpr[4]=who;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=0x8004f608;hooks().at(0x8004dea4)(&cpu,0x8004dea4);};
        auto sparks=[&](uint32_t ra){CPUState cpu{};cpu.gpr[4]=6;cpu.gpr[6]=0xff;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=ra;hooks().at(0x8006d594)(&cpu,0x8006d594);return cpu.gpr[6];};
        auto sizzle=[&](uint32_t ra){CPUState cpu{};cpu.gpr[4]=0x101d;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=ra;hooks().at(0x8006b270)(&cpu,0x8006b270);return cpu.gpr[4];};
        auto other_hook=[&]{CPUState cpu{};cpu.gpr[29]=0x801fff00;cpu.gpr[31]=0x800265a4;hooks().at(0x8001fc44)(&cpu,0x8001fc44);};
        auto stick=[&](uint32_t flags,uint32_t cross_word,uint32_t holster_word){
            psx_mod_write_half(p+0x3b8,0x0e02);psx_mod_write_word(p+0x224,flags);psx_mod_write_word(p+0x250,1234);
            psx_mod_write_word(cross,cross_word);psx_mod_write_word(holster,holster_word);
            psx_mod_write_half(p+0x74,39);psx_mod_write_half(p+0x7c,5);psx_mod_write_half(p+0x7e,0);writes=0;
        };
        auto stowed=[&]{return psx_mod_read_half(p+0x74)==0x28 && psx_mod_read_half(p+0x7c)==0 && psx_mod_read_half(p+0x7e)==1;};
        // Unlit and idle: fuse full, quiet, no stow; only the handler's own calls are quieted.
        stick(0,0,0);handler(p);
        assert(psx_mod_read_word(p+0x250)==full && psx_mod_read_half(p+0x74)==39);
        assert(sparks(0x8004e138)==0 && sizzle(0x8004e154)==0xffffffffu);
        assert(sparks(0x8004e6d0)==0xff && sizzle(0x8004e0d8)==0x101d);
        // A weapon request or a released holster button stows it (anim 40 backwards).
        stick(4,0,0);handler(p);assert(stowed() && psx_mod_read_word(p+0x250)==full);
        stick(0,0,2);handler(p);assert(stowed());
        stick(0x200,0,2);handler(p);assert(psx_mod_read_half(p+0x74)==39);  // 0x4241 blocks the button, as at 0x8004e1c0
        stick(0,0,3);handler(p);assert(psx_mod_read_half(p+0x74)==39);      // still held: not yet
        // Cross down where 0x8004dc74 would charge lights it: full fuse from the press, original from then on.
        stick(4,1,0);handler(p);
        assert(psx_mod_read_word(p+0x250)==full && psx_mod_read_half(p+0x74)==39);
        assert(sparks(0x8004e138)==0xff && sizzle(0x8004e154)==0x101d);
        assert(std::strstr(ttk::controls_debug_json(),"\"lit\":true"));
        psx_mod_write_word(p+0x250,1234);psx_mod_write_word(cross,0);handler(p);
        assert(psx_mod_read_word(p+0x250)==1234 && psx_mod_read_half(p+0x74)==39);  // burning, no stow
        other_hook();handler(p);assert(psx_mod_read_word(p+0x250)==1234);         // still in hand: still lit
        // Stowed or exploded (the hand empties outside a throw): the next stick
        // starts unlit and none is queued.
        psx_mod_write_half(p+0x3b8,0);other_hook();
        assert(std::strstr(ttk::controls_debug_json(),"\"lit\":false,\"redraw\":false"));
        // Thrown (the hand empties during throw animations 41-46): the next stick is queued.
        stick(0,0,0);other_hook();psx_mod_write_half(p+0x74,41);psx_mod_write_half(p+0x3b8,0);other_hook();
        assert(std::strstr(ttk::controls_debug_json(),"\"redraw\":true"));
        setenv("DNTTK_DYNAMITE","original",1);stick(0,0,0);other_hook();psx_mod_write_half(p+0x74,42);psx_mod_write_half(p+0x3b8,0);other_hook();
        assert(std::strstr(ttk::controls_debug_json(),"\"redraw\":false"));unsetenv("DNTTK_DYNAMITE");
        // A charge already running (+0x224 bit 8) lights it even with Cross up.
        stick(0x100,0,0);handler(p);assert(sparks(0x8004e138)==0xff);psx_mod_write_half(p+0x3b8,0);other_hook();
        stick(0,0,0);handler(p);assert(psx_mod_read_word(p+0x250)==full);
        // Cross in the charge's reset state (+0x224 bit 5) or with 0x241 set does not light it.
        stick(0x20,1,0);handler(p);assert(sparks(0x8004e138)==0);
        stick(0x40,1,0);handler(p);assert(sparks(0x8004e138)==0);
        // Other actors, other weapons, Original and Vanilla: untouched.
        stick(4,0,0);handler(0x801d0000);assert(writes==0);
        stick(4,0,0);psx_mod_write_half(p+0x3b8,0x0d02);writes=0;handler(p);assert(writes==0);
        setenv("DNTTK_DYNAMITE","original",1);stick(4,0,0);handler(p);assert(writes==0 && sparks(0x8004e138)==0xff);unsetenv("DNTTK_DYNAMITE");
        ttk::modern=false;stick(4,0,0);handler(p);assert(writes==0 && sizzle(0x8004e154)==0x101d);ttk::modern=true;
        psx_mod_write_half(p+0x3b8,0);psx_mod_write_word(p+0x224,flags0);psx_mod_write_word(p+0x250,0);
        psx_mod_write_half(p+0x74,0);psx_mod_write_half(p+0x7c,0);psx_mod_write_half(p+0x7e,0);
        psx_mod_write_half(p+0x60,lower);psx_mod_write_byte(p+0x233,index);
        psx_mod_write_word(0x800d1b50,cross_slot);psx_mod_write_word(0x800d1a90,holster_slot);
        other_hook();
        std::puts("PASS: D08A12 dynamite unlit until Cross (fuse full, sparks dark, sizzle refused), request/holster stow, lit runs original until it leaves the hand; reset/0x241, other actor/weapon, Original and Vanilla untouched");
    }
    {
        // D08A9: a gadget the dispatcher grants (bit 0, full amount) or
        // refills becomes the selection when its caller resumes (0x8001ca4c,
        // ra 0x8007fe78); keys, full items and other callers leave it, and a
        // gadget mid-activation makes it wait for the next item poll.
        constexpr uint32_t box=0x801dae00;psx_mod_write_half(box+0x2c,560);
        for(unsigned i=1;i<=5;++i){psx_mod_write_half(p+0x354+4*i,0);psx_mod_write_half(p+0x356+4*i,0);}
        psx_mod_write_half(p+0x368,1);psx_mod_write_half(p+0x36a,100);psx_mod_write_half(0x800c3f94,5);
        const uint16_t health=psx_mod_read_half(p+0x32);psx_mod_write_half(p+0x32,10000);
        auto poll=[&]{psx_mod_write_word(p+0x224,0);call(0x80058120,p,0,0x80041c44,0x801ff000);};
        poll();assert(psx_mod_read_half(0x800c3f94)==5);
        auto pickup=[&](unsigned item,uint16_t amount,uint32_t ra=0x8007fe78,uint32_t caller=0x8007fe6c){
            CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[5]=box;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=caller;
            hooks().at(0x80081a48)(&cpu,0x80081a48);
            if(item){psx_mod_write_half(p+0x354+4*item,psx_mod_read_half(p+0x354+4*item)|1);psx_mod_write_half(p+0x356+4*item,amount);}
            CPUState after{};after.gpr[29]=0x801fff00;after.gpr[31]=ra;hooks().at(0x8001ca4c)(&after,0x8001ca4c);
            return psx_mod_read_half(0x800c3f94);
        };
        assert(pickup(2,13500)==2);                       // new Bio Mask
        assert(pickup(0,0)==2);                           // a key: nothing granted
        assert(pickup(5,10000)==5);                       // medkit refill (100 -> full)
        assert(pickup(2,13500)==5);                       // full Bio Mask: the original refuses it
        assert(pickup(1,9000,0x800376b4)==5);             // other return: not a pickup
        psx_mod_write_half(p+0x358,0);
        assert(pickup(1,9000,0x8007fe78,0x8007fe68)==5);  // other caller
        psx_mod_write_half(p+0x358,0);
        psx_mod_write_half(p+0x35c,0x8001);               // Bio Mask mid-activation
        assert(pickup(3,18000)==5);poll();assert(psx_mod_read_half(0x800c3f94)==5);
        psx_mod_write_half(p+0x35c,1);poll();assert(psx_mod_read_half(0x800c3f94)==3);
        ttk::modern=false;assert(pickup(1,9000)==3);ttk::modern=true;
        for(unsigned i=1;i<=5;++i){psx_mod_write_half(p+0x354+4*i,0);psx_mod_write_half(p+0x356+4*i,0);}
        psx_mod_write_half(p+0x32,health);
        std::puts("PASS: D08A9 picked-up gadget selected (grant, refill), keys/full/other return/other caller/Vanilla keep it, waits for an activation");
    }
    std::puts("PASS: bounded switch query, model-centre ray, reach/cone/parent/Vanilla fallbacks");
    std::puts("PASS: concealed pickup, exposed/other-room/Vanilla/actor/caller/code fallbacks; inventory preserved");
    std::puts("PASS: typed cheat grants/toggles, context guards, hostile/NPC separation, health restoration and scene ownership reset");
    std::puts("PASS: standing takeoff, six directions, release latch, clearance rotation, once-only impulse and interruption guards");
    for(int rejection=0;rejection<7;++rejection) {
        ground();psx_mod_write_half(p+0x60,76);ttk::input.move_y=1;ttk::input.move_x=0;
        call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);
        psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x20c,0);
        psx_mod_write_word(p+0x1c4,383);psx_mod_write_word(p+0x1f8,100);
        terrain_result=0;terrain_floor=383;clearance_result=2048;
        ttk::jump_pending=true;unsigned before=edge_launches;
        if(rejection==1)ttk::input.sequence+=7;
        if(rejection==2)++ttk::input.epoch;
        if(rejection==3)psx_mod_write_word(p+0x1c4,2048);
        if(rejection==4)clearance_result=500;
        if(rejection==5)terrain_result=1;
        if(rejection==6)psx_mod_write_word(p+0x1f8,-100);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        if(edge_launches!=before+(rejection==0))std::fprintf(stderr,"edge fixture rejection=%d launches=%u before=%u controls=%s pending=%d\n",rejection,edge_launches,before,ttk::controls_debug_json(),ttk::jump_pending);
        assert(edge_launches==before+(rejection==0));
        if(!rejection) {
            assert(!ttk::jump_pending && psx_mod_read_half(p+0x60)==98);
            ttk::jump_pending=true;call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
            assert(edge_launches==before+1); // no second impulse
        }
    }
    // Crystal-2 turret wade: depth 0x100, water flagged, walk/turn clips.
    // Original 8005201c only free-swims at ≥0x281; anim 70 is a turn.
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x224,0);psx_mod_write_word(p+0x20c,0x33000);
    psx_mod_write_word(p+0x834,(uint32_t)-6144);psx_mod_write_word(p+0x1c8,(uint32_t)-5888);
    ttk::input.active=true;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready() && !ttk::swim_input_ready());
    psx_mod_write_half(p+0x60,70);++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);
    assert(ttk::movement_ready() && ttk::locomotion_input_ready() && !ttk::swim_input_ready());
    psx_mod_write_half(p+0x60,71);++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);
    assert(ttk::movement_ready() && !ttk::swim_input_ready());
    ttk::want_capture=true;const auto wade_offers=ttk::capture_offers;
    call(0x8005a210,p,0x800c2754,0x80041b34);
    assert(ttk::capture_offers==wade_offers+1); // Escape recapture without walk gait
    psx_mod_write_half(0x800be568,1);call(0x8005a210,p,0x800c2754,0x80041b34);
    assert(ttk::capture_offers==wade_offers+1);
    psx_mod_write_half(0x800be568,0);ttk::want_capture=false;
    ttk::running=true;ttk::input.move_x=1;ttk::input.move_y=0;
    psx_mod_write_half(p+0x60,71);
    call(0x80048410,p,0,0x8004b634);
    assert(psx_mod_read_half(p+0x60)==76); // tank turn becomes land run
    ttk::running=false;ttk::input.move_x=0;
    psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x20c,0);
    psx_mod_write_half(p+0x60,63);++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    std::puts("PASS: turret-wade land lease, anim-70/71 run, Escape recapture");
    // D08AC: authenticated mid-water gaits use the land dispatcher without
    // removing the water mask or changing floor/position/velocity. Native
    // swimming, Original, Vanilla, foreign callers and changed code stay out.
    {
        const auto saved=ttk::input;
        uint8_t body[0x8a4];for(unsigned i=0;i<sizeof body;++i)body[i]=psx_mod_read_byte(p+i);
        for(int test=0;test<8;++test) {
            for(unsigned i=0;i<sizeof body;++i)psx_mod_write_byte(p+i,body[i]);
            ttk::input=saved;ttk::input.active=true;ttk::input.move_x=0;ttk::input.move_y=1;
            ttk::input.held[ttk::crouch]=false;ttk::input.held[ttk::original_aim]=false;
            psx_mod_write_word(p+0x224,0);psx_mod_write_word(p,0);
            psx_mod_write_half(p+0x60,80);psx_mod_write_half(p+0x68,0);
            psx_mod_write_byte(p+0x22c,test==2?4:test==3?5:1);psx_mod_write_byte(p+0x22d,1);
            psx_mod_write_word(p+0x834,-512);psx_mod_write_word(p+0x1c8,test==1?288:0);
            psx_mod_write_word(p+0x20c,0x3b800);
            if(test==4)setenv("DNTTK_SWIMMING","original",1);
            if(test==5)ttk::modern=false;
            if(test==7)poke(0x5492c);
            call(0x800493a4,p,0,test==6?0x8005a3a4:0x8005a3a0);
            assert(psx_mod_read_half(p+0x60)==(test==0?(ttk::input_running()?76:72):80));
            assert(psx_mod_read_byte(p+0x22c)==(test==0?0:test==2?4:test==3?5:1));
            assert(psx_mod_read_word(p+0x20c)==0x3b800 && int32_t(psx_mod_read_word(p+0x834))==-512);
            for(unsigned o:{4u,8u,12u,0x1f4u,0x1f8u,0x1fcu})
                for(unsigned i=0;i<4;++i)assert(psx_mod_read_byte(p+o+i)==body[o+i]);
            if(test==7)poke(0x5492c);
            ttk::modern=true;unsetenv("DNTTK_SWIMMING");
        }
        ttk::input.active=true;ttk::input.move_y=1;
        ttk::input.held[ttk::original_aim]=ttk::input.held[ttk::crouch]=false;
        psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);
        psx_mod_write_word(p+0x834,-512);psx_mod_write_word(p+0x1c8,0);
        psx_mod_write_word(p+0x20c,0x3b800);
        psx_mod_write_half(p+0x60,72);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        call(0x8003ade4,c,p,0x80025ee8);
        call(0x80048410,p,0,0x8004b634,0x801ff000);
        assert(psx_mod_read_word(0x8004866c)==0x10000332);
        assert(psx_mod_read_word(0x80054964)==0);
        psx_mod_write_word(0x801feffc,0x8004b634);
        call(0x800780b4,0,0,0,0x801fefc0); // nested hook restores, checks, reapplies
        assert(psx_mod_read_word(0x8004866c)==0x10000332);
        ttk::modern=false;call(0x80058120,p,0,0x80041c44,0x801ff010);
        assert(psx_mod_read_word(0x8004866c)==0x10400332);
        assert(psx_mod_read_word(0x80054964)==0x10400060);
        ttk::modern=true;
        psx_mod_write_word(p+0x838,640);
        for(int depth:{640,641}) {
            psx_mod_write_word(p+0x834,-depth);psx_mod_write_half(p+0x60,98);
            psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
            call(0x80055e80,p,0,0x8005a4f8,0x801ff000);
            assert(psx_mod_read_word(0x80056240)==(depth==640?0x1000005cu:0x1040005cu));
            call(0x80058120,p,0,0x80041c44,0x801ff010);
            assert(psx_mod_read_word(0x80056240)==0x1040005c);
        }
        psx_mod_write_word(p+0x834,-640);psx_mod_write_half(p+0x60,98);
        psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        call(0x80048410,p,0,0x8004b634,0x801ff000);
        assert(psx_mod_read_word(0x80054cbc)==0x1000000a);
        assert(ttk::player_identity_ready()); // collision/aim plugins see owned code
        psx_mod_write_code_word(0x80054964,0); // known substitution, but not owned in this scope
        assert(!ttk::player_identity_ready());
        psx_mod_write_code_word(0x80054964,0x10400060);
        poke(0x54c04);assert(!ttk::player_identity_ready());poke(0x54c04);
        assert(ttk::player_identity_ready());
        // A real floor/water change during collision must release the shallow
        // landing rule before the native selector reads the volume depth.
        psx_mod_write_word(0x801feffc,0x8004b634);psx_mod_write_word(p+0x834,-800);
        call(0x8006b270,0x3001,p+4,0x80054cb0,0x801fef80);
        assert(psx_mod_read_word(0x80054cbc)==0x1440000a);
        assert(psx_mod_read_word(p+0x20c)==0x3b800);
        // D08AE: cached support at the surface must not suppress native
        // deep entry. Also release an already armed shallow scope when the
        // native query refreshes only the volume (the support stays stale).
        for(unsigned target:{0x80055e80u,0x80048410u})for(int volume:{640,641,3072,5120}) {
            const bool transition=target==0x80055e80;
            const uint32_t branch=transition?0x80056240:0x80054cbc;
            const uint32_t original=transition?0x1040005c:0x1440000a;
            const uint32_t patched=transition?0x1000005c:0x1000000a;
            const uint32_t caller=transition?0x8005a4f8:0x8004b634;
            psx_mod_write_word(p+0x834,0);psx_mod_write_word(p+0x1c8,0);
            psx_mod_write_word(p+0x838,volume);psx_mod_write_half(p+0x60,98);
            psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
            call(target,p,0,caller,0x801ff000);
            assert(psx_mod_read_word(branch)==(volume==640?patched:original));
            psx_mod_write_word(0x801feffc,caller);
            psx_mod_write_word(p+0x838,5120);
            call(0x8006b270,0x3001,p+4,0x80054cb0,0x801fef80);
            assert(psx_mod_read_word(branch)==original);
            assert(psx_mod_read_word(p+0x20c)==0x3b800);
            call(0x80058120,p,0,0x80041c44,0x801ff010);
        }
        // Grounded shelf policy still depends on solid support, even if the
        // enclosing water volume is deep; only flight protection changed.
        psx_mod_write_word(p+0x838,5120);
        // D08AD: a walking terrain probe runs before the dispatcher's stack
        // exists. Re-entering the outer hook must arm its scope afterwards.
        psx_mod_write_word(p+0x834,-512);psx_mod_write_half(p+0x60,72);
        psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        ttk::running=false;ttk::input.move_y=1;ttk::input.move_x=0;
        ttk::input.held[ttk::jump]=false;
        call(0x8003ade4,c,p,0x80025ee8);
        psx_mod_write_word(0x801feffc,p);
        terrain_nested_hook=true;terrain_nested_calls=0;
        call(0x80048410,p,0,0x8004b634,0x801ff000);
        terrain_nested_hook=false;assert(terrain_nested_calls>0);
        assert(psx_mod_read_word(0x800485e4)==0x10000354);
        psx_mod_write_word(0x801feffc,0x8004b634);
        call(0x80053404,p,0,0x800485dc,0x801fefd8);
        assert(psx_mod_read_word(0x800485e4)==0x10000354);
        call(0x80058120,p,0,0x80041c44,0x801ff010);
        // The completion selector has a different saved-RA offset (-8).
        // Preserve the native phase/frame data: no host phase reconstruction.
        for(unsigned a:{72u,74u,76u,78u})for(int test=0;test<7;++test) {
            ttk::input=saved;ttk::input.active=true;ttk::input.move_y=1;
            ttk::input.held[ttk::original_aim]=ttk::input.held[ttk::crouch]=false;
            psx_mod_write_word(p,0);psx_mod_write_word(p+0x224,0);
            psx_mod_write_word(p+0x834,test==1?-641:-512);
            psx_mod_write_word(p+0x1c8,0);psx_mod_write_word(p+0x20c,0x3b800);
            psx_mod_write_byte(p+0x22c,test==2?4:0);
            psx_mod_write_half(p+0x60,a);psx_mod_write_half(p+0x68,2);
            psx_mod_write_half(p+0x64,117);psx_mod_write_half(p+0x66,118);
            if(test==3)setenv("DNTTK_SWIMMING","original",1);
            if(test==4)ttk::modern=false;
            if(test==6)poke(0x4b874);
            const auto original=psx_mod_read_word(0x8004b874);
            call(0x8004b644,p,0,test==5?0x8005a474:0x8005a470,0x801ff000);
            assert(psx_mod_read_word(0x8004b874)==(test==0?0x1000000au:original));
            assert(psx_mod_read_word(0x8004bb04)==(test==0?0x1000000au:0x1040000au));
            assert(psx_mod_read_half(p+0x60)==a && psx_mod_read_half(p+0x68)==2);
            assert(psx_mod_read_half(p+0x64)==117 && psx_mod_read_half(p+0x66)==118);
            assert(psx_mod_read_word(p+0x20c)==0x3b800);
            if(test==0) {
                psx_mod_write_word(0x801feff8,0x8005a470);
                call(0x800780b4,0,0,0,0x801fefc0);
                assert(psx_mod_read_word(0x8004b874)==0x1000000a);
            }
            call(0x80058120,p,0,0x80041c44,0x801ff010);
            assert(psx_mod_read_word(0x8004b874)==original);
            if(test==6)poke(0x4b874);
            ttk::modern=true;unsetenv("DNTTK_SWIMMING");
        }
        std::puts("PASS: D08AD post-probe dispatcher scope and native stride completion; phase preservation and exclusions");
        for(unsigned i=0;i<sizeof body;++i)psx_mod_write_byte(p+i,body[i]);
        ttk::input=saved;
        std::puts("PASS: D08AC shared mid-water gait; water identity/physics preserved; deep/swim/Original/Vanilla/caller/code exclusions");
    }
    // Mid-depth wade (flooded corridor ledge): depth 0x200, mode 1, clip 81.
    // The wade handler's root is retargeted to camera-relative WASD at land
    // pace; camera-only lease keeps mouse look; no direction keeps the clip.
    psx_mod_write_half(p+0x60,81);psx_mod_write_byte(p+0x22c,1);psx_mod_write_byte(p+0x22d,1);
    psx_mod_write_word(p+0x224,2);psx_mod_write_word(p+0x20c,0x7fc91);
    psx_mod_write_word(p+0x834,(uint32_t)-6144);psx_mod_write_word(p+0x1c8,(uint32_t)-5632);
    ttk::input.active=true;ttk::input.move_x=0;ttk::input.move_y=0;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);
    assert(ttk::movement_ready() && ttk::locomotion_input_ready() && !ttk::swim_input_ready());
    assert(ttk::mid_wade_input_ready());
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096); // view +Z
    psx_mod_write_half(p+0xfc,(uint16_t)(int16_t)-22);psx_mod_write_half(p+0x100,0); // clip root, body-relative world
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==-22 && (int16_t)psx_mod_read_half(p+0x100)==0); // idle: untouched
    ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=true;
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0x100)==78 && (int16_t)psx_mod_read_half(p+0xfc)==0); // W: view forward, 22 * run step/28 lifted to the 78..117 band
    ttk::input.move_x=1;ttk::input.move_y=0;
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==78 && (int16_t)psx_mod_read_half(p+0x100)==0); // D: strafe right (+X)
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,30); // mid clip tick: 30 * run step/28
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==105);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,6); // short clip tick: floor 0.8 * run step
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==78);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,44); // long clip tick: ceiling 1.2 * run step
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==117);
    ttk::running=false;psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22); // D08AA: land walk pace, band 17..25
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==17);ttk::running=true;
    // D08AB SWIMMING original: the pre-D08AA band (walk 2x22 in 40..60, run 4x22
    // in 80..120) and the forced run in shallow / mid-depth water.
    assert(!ttk::wade_full_speed_ready());
    setenv("DNTTK_SWIMMING","original",1);assert(ttk::wade_full_speed_ready());
    ttk::running=false;psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==44);
    ttk::running=true;psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==88);
    assert(std::strstr(ttk::controls_debug_json(),"\"swimming\":\"original\""));
    ttk::modern=false;assert(!ttk::wade_full_speed_ready());ttk::modern=true;
    unsetenv("DNTTK_SWIMMING");assert(std::strstr(ttk::controls_debug_json(),"\"swimming\":\"modern\""));
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x80048664,0x801fff00); // foreign caller: untouched
    assert((int16_t)psx_mod_read_half(p+0xfc)==0 && (int16_t)psx_mod_read_half(p+0x100)==22);
    psx_mod_write_byte(p+0x22c,0);psx_mod_write_half(p+0x60,76); // land run in the same water: not the wade handler
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==0 && (int16_t)psx_mod_read_half(p+0x100)==22);
    {
        // D08AA: surface strokes and underwater thrust at the shared walk/run
        // pace, from the swim tail's rotation call (0x80066e34, ra 0x80046ec8).
        const uint32_t angles=0x801fff30,v=p+0x1f4;
        auto stroke=[&](unsigned mode,unsigned anim,int32_t x,int32_t z,int16_t pitch=0,uint32_t ra=0x80046ec8) {
            psx_mod_write_byte(p+0x22c,mode);psx_mod_write_byte(p+0x22d,mode);psx_mod_write_half(p+0x60,anim);
            psx_mod_write_word(v,x);psx_mod_write_word(v+4,0);psx_mod_write_word(v+8,z);psx_mod_write_half(angles+2,pitch);
            CPUState cpu{};cpu.gpr[4]=angles;cpu.gpr[5]=v;cpu.gpr[31]=ra;
            for(auto& cb:hooks().at(0x80066e34).list)cb(&cpu,0x80066e34);
        };
        auto vx=[&]{return (int32_t)psx_mod_read_word(v);};
        auto vz=[&]{return (int32_t)psx_mod_read_word(v+8);};
        ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=false;
        stroke(4,125,0,0x1333);assert(vz()==2030);          // surface forward: walk
        ttk::running=true;stroke(4,125,0,0x1333);assert(vz()==9520); // Shift: run
        stroke(4,125,0,0x1333/2);assert(std::abs(vz()-4760)<=1); // pad pressure share kept
        ttk::input.move_y=-1;stroke(4,126,0,-0xaaa);assert(vz()==-9520); // back
        ttk::input.move_x=-1;ttk::input.move_y=0;stroke(4,123,-0x444,0);assert(vx()==-9520 && vz()==0); // strafe
        ttk::input.move_x=0;ttk::input.move_y=1;
        stroke(5,128,0,0x149f,-300);assert(vz()==9520 && (int16_t)psx_mod_read_half(angles+2)==-300); // view-pitched thrust
        ttk::running=false;stroke(5,129,0,0x149f);assert(vz()==2030);
        ttk::running=true;ttk::input.held[ttk::jump]=true;
        stroke(5,128,0,0x149f,640); // Space: vertical kept, horizontal = pace
        {
            const double was=0x149f*std::sin(640*ttk::tau/4096),pitch=(int16_t)psx_mod_read_half(angles+2)*ttk::tau/4096;
            assert(std::abs(vz()*std::sin(pitch)-was)<30 && std::abs(vz()*std::cos(pitch)-9520)<30);
        }
        ttk::input.held[ttk::jump]=false;
        stroke(5,133,0,0x149f);assert(vz()==0x149f);        // dive 133: original
        stroke(4,125,0,0x1333,0,0x80046000);assert(vz()==0x1333); // foreign caller
        ttk::input.move_y=0;stroke(4,125,0,0x1333);assert(vz()==0x1333); // no direction
        ttk::input.move_y=1;ttk::modern=false;stroke(4,125,0,0x1333);assert(vz()==0x1333);ttk::modern=true; // Vanilla
        // D08AB SWIMMING original: strokes and thrust keep their fixed speeds; live.
        setenv("DNTTK_SWIMMING","original",1);
        stroke(4,125,0,0x1333);assert(vz()==0x1333);
        ttk::input.move_x=-1;ttk::input.move_y=0;stroke(4,123,-0x444,0);assert(vx()==-0x444);
        ttk::input.move_x=0;ttk::input.move_y=1;stroke(5,128,0,0x149f,-300);assert(vz()==0x149f && (int16_t)psx_mod_read_half(angles+2)==-300);
        unsetenv("DNTTK_SWIMMING");stroke(4,125,0,0x1333);assert(vz()==9520);
        // D08AF: bracket the native integrator and pre-collision sweep.
        // Simulate its output; the blend may change only horizontal root.
        auto plunge=[&](int vy,bool release=false,uint32_t caller=0x8004b5bc,
                        bool stale=false,unsigned mode=5,unsigned anim=128,uint32_t flags=0x04000000) {
            psx_mod_write_word(p,(psx_mod_read_word(p)&~0x04000020u)|flags);
            psx_mod_write_byte(p+0x22c,mode);psx_mod_write_half(p+0x60,anim);
            psx_mod_write_word(p+0x224,0);psx_mod_write_word(v+4,vy?vy:1100);
            psx_mod_write_half(p+0xba,17);
            ttk::input.move_y=1;
            CPUState cpu{};cpu.gpr[4]=p;cpu.gpr[29]=0x801fff00;cpu.gpr[31]=caller;
            hooks().at(0x800455bc)(&cpu,0x800455bc);
            psx_mod_write_word(v,500);psx_mod_write_word(v+4,vy);psx_mod_write_word(v+8,2000);
            psx_mod_write_half(p+0xfc,8);psx_mod_write_half(p+0xfe,31);psx_mod_write_half(p+0x100,32);
            if(release)ttk::input.move_y=0;
            if(stale)++ttk::input.epoch;
            cpu.gpr[31]=0x8007a9f8;cpu.gpr[29]-=0xb8;
            const unsigned before=writes;
            hooks().at(0x8003964c)(&cpu,0x8003964c);
            assert(psx_mod_read_word(v)==500 && (int32_t)psx_mod_read_word(v+4)==vy && psx_mod_read_word(v+8)==2000);
            assert(psx_mod_read_half(p+0xfe)==31); // native plunge depth untouched
            const bool changed=writes!=before;
            const unsigned after=writes;
            hooks().at(0x8003964c)(&cpu,0x8003964c);assert(writes==after); // consumed once
            if(stale)--ttk::input.epoch;
            return changed;
        };
        assert(!plunge(16000));assert(!plunge(8192)); // initial resistance unchanged
        assert(plunge(2048));
        const int partial_x=(int16_t)psx_mod_read_half(p+0xfc),partial_z=(int16_t)psx_mod_read_half(p+0x100);
        assert(plunge(0)); // native stop reaches the existing swim pace
        const int full_x=(int16_t)psx_mod_read_half(p+0xfc),full_z=(int16_t)psx_mod_read_half(p+0x100);
        const double expected=9520*std::cos((int16_t)psx_mod_read_half(p+0x1e)*ttk::tau/4096)*17/1024;
        assert(std::abs(std::hypot(full_x,full_z)-expected)<1);
        assert(std::abs(partial_x-full_x)<=std::abs(8-full_x));
        assert(std::abs(partial_z-full_z)<=std::abs(32-full_z));
        assert(!plunge(2048,true)); // release retains native damping
        assert(!plunge(2048,false,0x80046d78)); // foreign caller
        assert(!plunge(2048,false,0x8004b5bc,true)); // load/context epoch
        assert(!plunge(2048,false,0x8004b5bc,false,1,80)); // shallow water
        assert(!plunge(2048,false,0x8004b5bc,false,4,125)); // surface
        assert(!plunge(2048,false,0x8004b5bc,false,5,133)); // explicit dive
        assert(!plunge(2048,false,0x8004b5bc,false,5,128,0)); // ordinary stroke
        ttk::modern=false;assert(!plunge(2048));ttk::modern=true;
        setenv("DNTTK_SWIMMING","original",1);assert(!plunge(2048));unsetenv("DNTTK_SWIMMING");
        poke(0x3e4b0);assert(!plunge(2048));poke(0x3e4b0);assert(plunge(2048));
        psx_mod_write_word(p,psx_mod_read_word(p)&~0x04000000u);
        std::puts("PASS: D08AF plunge tail, native vertical/velocity preservation, release, caller, epoch, Original, Vanilla and changed-code gates");
        psx_mod_write_word(v,0);psx_mod_write_word(v+8,0);
    }
    ttk::running=false;ttk::input.move_x=0;ttk::input.move_y=0;
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x20c,0);
    ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    std::puts("PASS: mid-depth wade handler root retarget, idle/foreign/land exclusions; D08AA surface/underwater pace; D08AB original swimming");
    // Isolated F7 UI slot 2: overlay already patched, waist-deep, anim 63.
    psx_mod_write_word(0x800cc57c,0x24020000);
    psx_mod_write_word(0x800cc580,0x30a30400);
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x224,2);psx_mod_write_word(p+0x20c,0x33000);
    psx_mod_write_word(p+0x834,(uint32_t)-6144);psx_mod_write_word(p+0x1c8,(uint32_t)-5888);
    ttk::input.active=true;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready() && !ttk::swim_input_ready());
    ttk::want_capture=true;const auto slot2_offers=ttk::capture_offers;
    call(0x8005a210,p,0x800c2754,0x80041b34);
    assert(ttk::capture_offers==slot2_offers+1);
    ttk::want_capture=false;
    std::puts("PASS: F7 slot-2 patched overlay identity, wade lease, recapture offer");
    // D17Y: widen only a direct, open upper room boundary. Downstream
    // windows retain their authored aperture, as do indoor/short portals.
    {
        setenv("DNTTK_DRAW_DISTANCE","extended",1);
        constexpr uint32_t table=0x80180000,portals=0x80181000,entry=0x800d68b0,rect=0x800d2210;
        auto setup=[&] {
            psx_mod_write_word(0x800da454,table);psx_mod_write_word(c+0x90,0);
            psx_mod_write_word(table+4,1024);psx_mod_write_word(table+0x14,1024);
            psx_mod_write_byte(table+0x30,1);psx_mod_write_word(table+0x38,portals);
            psx_mod_write_word(table+88+12,8);psx_mod_write_word(table+88+0x14,uint32_t(-2048));
            psx_mod_write_half(portals+14,1);
            for(unsigned j=0;j<4;++j) psx_mod_write_half(portals+j*8+2,(j&1)?5120:0);
            psx_mod_write_half(rect,uint16_t(-256));psx_mod_write_half(rect+2,uint16_t(-120));
            psx_mod_write_half(rect+4,512);psx_mod_write_half(rect+6,240);
            psx_mod_write_word(0x800d68a8,1);
            psx_mod_write_half(entry,uint16_t(-100));psx_mod_write_half(entry+2,uint16_t(-80));
            psx_mod_write_half(entry+4,200);psx_mod_write_half(entry+6,160);psx_mod_write_word(entry+8,1);
            psx_mod_write_word(0x800da450,1000);psx_mod_write_word(0x800da34c,2000);
            psx_mod_write_word(0x800d67a8+0x74,1000+0x13d0);
        };
        auto top=[&]{return int16_t(psx_mod_read_half(entry+2));};
        auto run=[&]{call(0x80062b48,0,0,0x8002e688);};
        setup();run();assert(top()==-120);run();assert(top()==-120);
        setup();psx_mod_write_word(entry+8,2);run();assert(top()==-82); // indirect
        setup();psx_mod_write_word(table+88+12,0);run();assert(top()==-82); // indoors
        setup();psx_mod_write_word(table+88+0x14,1024);run();assert(top()==-82);
        setup();for(unsigned j=0;j<4;j+=2)psx_mod_write_half(portals+j*8+2,64);run();assert(top()==-82);
        setup();psx_mod_write_word(table+0x38,0x801ffff0);run();assert(top()==-82);
        setup();ttk::modern=false;run();assert(top()==-80);ttk::modern=true;
        setup();call(0x80062b48,0,0,0x80012345);assert(top()==-80);
        std::puts("PASS: D17Y open upper portal, downstream/indoor/height/bounds/caller and Vanilla guards");
    }
    assert(ttk::camera_body_coverage(650,0)==0);
    assert(ttk::camera_body_coverage(1700,0)==1);
    assert(ttk::camera_body_coverage(650,660)==1);
    for(int d=650;d<1700;++d) {
        const double a=ttk::camera_body_coverage(d,0),b=ttk::camera_body_coverage(d+1,0);
        assert(a>=0 && b<=1 && b>=a && b-a<0.002);
        assert(ttk::camera_body_coverage(d,400)==ttk::camera_body_coverage(d,-400));
    }
    std::puts("PASS: D10B proximity coverage bounds, continuity, monotonicity and shoulder symmetry");
    std::puts("PASS: distance clamps, convergence, epoch recovery and wall-independent preference; D10 recenter, shoulder offset and camera side file");
    std::puts("PASS: all semantic groups, upgrades, empty remote, history, unsafe attack/item guards, crouch delta and clearance fixtures");
    std::puts("PASS: immediate takeoff support probe, guarded caller/code, owned airborne reach lease");
    std::puts("PASS: respawn boom, landing seam and pre-collision stride smoothing");
    std::puts("PASS: camera constraints, duplicate look, capture and original-camera gates");
    std::puts("PASS: Vanilla, actor/caller/overlay/code/state guards; movement/probe alignment; independent facing");
    std::puts("PASS: D08T1 pushable masks (D08T4 floor gate, D08T2 types up to 1061, grab-only climb mask, E-only idle grab mask; caller/actor/flags/Vanilla/capture) and camera-only grab lease");
    std::puts("PASS: D12A first-person quick kick (boot request conversion from the attack only (E suppressed) and its guards, lease, sound, view-aimed hit sphere at the crosshair surface within reach, held attack with Boot, right-leg viewmodel with reversed thigh, third person untouched); D12C kick impact (thud (one octave up, three times its volume) once at the first sphere contact or a reachable wall, silent otherwise; third-person sphere made by the host, original neutralized, once per kick; Vanilla and other callers untouched)");
}
