// Owned-data integration harness. Models call contracts, not terrain/animation.
#include "modern_controls.h"
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
#include <vector>
#include <unistd.h>
#include <string>
#include <iterator>
#include <algorithm>
static uint8_t ram[0x200000];
static auto& hooks() {static std::map<uint32_t,PSXModFunctionEntryCallback> value;return value;}
static unsigned writes, code_writes;
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t,uint32_t){return 0x801e0000;}
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
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb) {hooks()[a]=cb;return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_depth(uint32_t,uint32_t,int32_t,int32_t,float,float,float,float){return 1;}
static PSXModActivationCallback activation;static unsigned aspect_num,aspect_den;
extern "C" int psx_mod_register_activation_plugin(const char*,PSXModActivationCallback cb) {activation=cb;return 1;}
extern "C" int psx_mod_set_fixed_display_aspect(uint32_t n,uint32_t d) {aspect_num=n;aspect_den=d;return 1;}
static unsigned adaptive_num,adaptive_den;static int32_t ws_margin;
extern "C" int psx_mod_set_adaptive_display_aspect(uint32_t n,uint32_t d) {adaptive_num=n;adaptive_den=d;return 1;}
extern "C" int32_t psx_mod_widescreen_x_margin(void) {return ws_margin;}
extern "C" {int g_precise_mode=0,g_ls_mode=0,g_psx_call_bail=0;}
static int clearance_result=2048;
static unsigned kicks, edge_launches;
static int terrain_result=3,terrain_floor=-1000;
static uint32_t terrain_reference;
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

// D08Y: the runtime CPU overclock lease (renewed from the player update).
static unsigned overclock_renewals;extern "C" void psx_overclock_renew(void){++overclock_renewals;}
extern "C" void psx_dispatch_call(CPUState* cpu,uint32_t address,uint32_t) {
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
        kick_calls.push_back(k);cpu->pc=0;return;
    }
    assert(address==0x8007926c || address==0x800797f4 || address==0x800402d0 || address==0x8007765c || address==0x800517a4);
    if(address==0x800517a4)++kicks;
    if(address==0x8007926c || address==0x800797f4) {
        terrain_reference=psx_mod_read_word(0x800d7198+0x1c8);
        psx_mod_write_word(0x800d7198+0x1c4,terrain_floor);cpu->gpr[2]=terrain_result;
    } else {
        if(address==0x8007765c && lookahead_drop!=-100000)psx_mod_write_word(0x800d7198+0x1c4,uint32_t(lookahead_drop));
        cpu->gpr[2]=clearance_result;
    }
    cpu->pc=0;
}
namespace ttk {bool input_live_look(uint64_t&,double&,double&,double){return false;} void input_state_loaded(){}}
extern "C" uint32_t psx_mod_savestate_loads(void){return 0;}
namespace ttk {
const char* aim_debug_json(){return "{}";}
// D12A: the crosshair segment query (weapon_aim.cpp in the player build).
static bool segment_hit;static double segment_point[3];
bool view_segment_query(CPUState*,const double*,const double* to,double* hit,int& kind) {
    kind=segment_hit?2:0;for(int i=0;i<3;++i)hit[i]=segment_hit?segment_point[i]:to[i];return true;
}
static InputFrame input;
static Cheat pending_cheat=Cheat::None;
Cheat input_take_cheat(){auto c=pending_cheat;pending_cheat=Cheat::None;return c;}
void input_notice(const char*){}
void inventory_update(unsigned,const uint16_t*,const int16_t*,const uint16_t*,bool){}
static bool modern;
static bool running, jump_pending, auto_stow;
bool input_auto_stow_pending(){return auto_stow;}
bool input_jump_pending(){return jump_pending;}
bool input_take_jump(){bool result=jump_pending;jump_pending=false;return result;}
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
bool input_modernized(){return modern;}
const InputFrame& input_snapshot(Context){return input;}
uint64_t input_host_frame(){return input.sequence;}
}
static void call(uint32_t address,uint32_t a0,uint32_t a1,uint32_t ra,uint32_t sp=0x801f0000,uint32_t a2=0) {
    CPUState cpu{};cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;cpu.gpr[31]=ra;cpu.gpr[29]=sp;
    hooks().at(address)(&cpu,address);
}
int main(int argc,char** argv) {
    assert(argc==3);
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
        // Grab 121 remembering the object: camera-only lease, never locomotion.
        psx_mod_write_word(p+0x290,object);psx_mod_write_half(p+0x60,121);
        assert(ttk::push_grab_ready() && !ttk::locomotion_input_ready() && !ttk::movement_ready());
        psx_mod_write_half(p+0x60,63);psx_mod_write_word(p+0x174,0);psx_mod_write_word(p+0x290,0);
        assert(!ttk::push_grab_ready() && ttk::movement_ready());
        psx_mod_write_word(0x800d2660,saved[0]);psx_mod_write_word(0x800d1b50,saved[1]);psx_mod_write_word(word,saved[2]);
        psx_mod_write_byte(p+0x233,saved_player);
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
        camera();assert(!ttk::traversal_camera_ready());
        // Vanilla never mounts.
        psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);place(-3798,-5625,76329);
        ttk::modern=false;ttk::ladder_top_request();update();assert(psx_mod_read_half(p+0x60)==63);ttk::modern=true;
        ttk::input.sequence+=25;camera();update();assert(psx_mod_read_half(p+0x60)==63); // a stale request expires
        psx_mod_write_word(p+0x220,0);psx_mod_write_word(p+0x17c,0);psx_mod_write_word(0x800d2660,saved_table);
        place(0,0,0);psx_mod_write_half(p+0x1c,0);psx_mod_write_half(p+0x24,0);
        ttk::input.sequence=saved_sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
        std::puts("PASS: D08U top-of-ladder mount (reach/side/top/family/state gates, armed and Vanilla refusal, catch-equivalent attach, 12-update blend onto the climbing line, camera-only lease, 186 handoff, request expiry)");
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
        // The original object hang clears bit 0x40 of the 149/152/153 flag
        // entries; that is game state and must not drop the identity guard.
        const auto seq_before=ttk::input.sequence;
        for(uint32_t a:{0x800c2a78u,0x800c2a84u,0x800c2a88u}) {
            const uint32_t v=psx_mod_read_word(a);psx_mod_write_word(a,v^0x40);
            ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
            psx_mod_write_word(a,v);
        }
        const uint32_t kept=psx_mod_read_word(0x800c2a74);psx_mod_write_word(0x800c2a74,kept^0x40);
        ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::movement_ready());
        psx_mod_write_word(0x800c2a74,kept);++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
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
            std::puts("PASS: D08Y gap jumps (lowered E catch -> height mantle, lip/mode/armed/Vanilla refusals; no-E bounce off a low lip -> 134, held bit restored)");
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
    assert(writes==9 && psx_mod_read_half(orientation.gpr[4]+12)==requested_x);
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
    ttk::input.active=false;writes=0;call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);assert(writes==0);
    ttk::input.active=true;
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
    // Recapture with a wall-displaced boom seeds visible +X, not anchor's +Z.
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
    setenv("DNTTK_WEAPON_AIM","original",1);refuse();setenv("DNTTK_WEAPON_AIM","view",1);
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
        // D11 first-person: the eye follows the neck joint; the anchor sits half a
        // projection distance ahead of it. The original minimum boom is relaxed
        // only while the eye view blends in, and the head joint is flagged only
        // between Duke's draw and the next object-list step.
        {
            const uint32_t desc=0x801d5820,mats=0x801d6000,record=desc+0x44+0x28*9;
            psx_mod_write_word(p+0x40,desc);psx_mod_write_word(p+0x3c,mats);
            psx_mod_write_byte(desc,19);psx_mod_write_byte(record+2,3);psx_mod_write_byte(record,0);
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
                psx_mod_write_byte(p+0x3b8,saved_state);psx_mod_write_byte(p+0x3b9,saved_slot);
                ttk::input.first_person=false;for(unsigned i=0;i<40;++i)frame();
                // Third person: the hand keeps its own matrix.
                call(0x800348d8,c,p,0x8003769c);psx_mod_write_byte(p+0x3b8,2);
                assert(hook(0x800292a0,c,hand,0x80034c6c,7).gpr[5]==hand);
                call(0x8001ca4c,0x800d0000,0,0x800376b4);
                psx_mod_write_byte(p+0x3b8,saved_state);
                ttk::input.first_person=true;for(unsigned i=0;i<40;++i)frame();
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
                // Third person: the original request plays as before.
                ttk::input.first_person=false;for(unsigned i=0;i<40;++i)frame();
                psx_mod_write_half(p+0x60,112);psx_mod_write_half(p+0x68,0);
                call(0x800493a4,p,0,0x8005a490,0x801fff00);assert(psx_mod_read_half(p+0x60)==112);
                psx_mod_write_half(p+0x60,63);
                psx_mod_write_word(0x800d21fc,saved_delta);psx_mod_write_half(p+0x364,saved_power);
                ttk::input.first_person=true;for(unsigned i=0;i<40;++i)frame();
            }
            ttk::input.first_person=false;
            for(unsigned i=0;i<40;++i)frame();
            assert(metric("\"blend\":")==0);
            call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);
            assert(psx_mod_read_word(0x800c3c48)==768 && (int16_t)psx_mod_read_half(c+0x42)==386);
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
            // Losing the lease mid-blend restores the boom and head at once.
            ttk::input.first_person=true;for(unsigned i=0;i<3;++i)frame();
            call(0x8003aa48,c,sp+0x18,0x8003aeb0,sp);assert(psx_mod_read_word(0x800c3c48)<768);
            call(0x800348d8,c,p,0x8003769c);assert(psx_mod_read_byte(record)&1 || metric("\"blend\":")<0.5);
            ttk::input.active=false;frame();
            assert(!(psx_mod_read_byte(record)&1));
            assert(psx_mod_read_word(0x800c3c48)==768 && metric("\"blend\":")==0);
            {
                CPUState cpu{};cpu.gpr[4]=386;cpu.gpr[31]=0x8002e4d0;cpu.gpr[29]=0x801f0000;
                hooks().at(0x800b4d9c)(&cpu,0x800b4d9c);assert(cpu.gpr[4]==386); // third person untouched
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
        call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::locomotion_input_ready());
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
    // Only the original clear drop result, in the small-drop range, selects
    // the original run/fall dispatcher for a walking-speed step.
    for(auto check:std::vector<std::pair<int,int>>{{6,384},{6,900},{3,384},{6,200}}) {
        ground();psx_mod_write_half(p+0x60,72);ttk::input.move_y=1;ttk::input.move_x=0;
        ttk::input.held[ttk::jump]=false;ttk::running=false;
        call(0x8003ade4,c,p,0x80025ee8);
        terrain_result=check.first;terrain_floor=check.second;
        psx_mod_write_word(p+0x1c4,123);
        call(0x80048410,p,0,0x8004b634,0x801fff00);
        assert(psx_mod_read_half(p+0x60)==(check.first==6 && check.second==384?76:72));
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
        // A later large fall must not inherit an old furniture landing lease.
        psx_mod_write_half(p+0x60,108);psx_mod_write_byte(p+0x22c,9);psx_mod_write_byte(p+0x22d,9);
        psx_mod_write_word(p+0x1c4,900);
        call(0x8005a210,p,0x800c2754,0x80041b34,0x801fff00);
        call(0x8003ade4,c,p,0x80025ee8);assert(!ttk::short_fall_input_ready());
    }
    // A running furniture fall inherits actual ground speed, once. Original
    // collision can subsequently stop it, and large falls never receive it.
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
    // at ground speed, capped below the running jump. Walking and a fall that
    // follows a jump (103) keep the original 3058.
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
    std::puts("PASS: running large-ledge run-off keeps capped ground speed; walking and post-jump falls unchanged; spiked strides capped");
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
        for(unsigned i=1;i<=5;++i){psx_mod_write_half(p+0x354+4*i,1);psx_mod_write_half(p+0x356+4*i,100);}
        auto stranded=[&](){
            ttk::modern=true;ttk::input.active=true;
            psx_mod_write_word(p,0);psx_mod_write_half(p+0x32,6250);
            psx_mod_write_word(p+0x224,0);psx_mod_write_half(p+0x358,0x8001);
            psx_mod_write_half(p+0x35a,9000);psx_mod_write_half(p+0x84c,0);
            psx_mod_write_word(p+0x884,1);psx_mod_write_byte(p+0x22c,0);psx_mod_write_byte(p+0x22d,0);
        };
        auto poll=[&](){call(0x80058120,p,0,0x80041c44);};
        stranded();poll();assert(psx_mod_read_half(p+0x358)==1 && psx_mod_read_half(p+0x35a)==9000);
        // Picker and activation use the repaired original record, without a cheat.
        psx_mod_write_half(0x800c3f94,5);request(ttk::item_next);assert(psx_mod_read_half(0x800c3f94)==1);
        ttk::input.commands[0]=ttk::jetpack;++ttk::input.command_serial;
        call(0x80058120,p,0,0x80041c44,0x801fff00);assert(psx_mod_read_half(p+0x358)==0x8003);
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
    psx_mod_write_half(0x800c3cc6,0);cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==1);
    cheat(ttk::Cheat::God);assert(psx_mod_read_half(0x800c3cc6)==0);
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
    assert(ttk::wade_full_speed_ready());
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
    call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready() && !ttk::wade_full_speed_ready());
    std::puts("PASS: turret-wade land lease, anim-70/71 run, Escape recapture");
    // Mid-depth wade (flooded corridor ledge): depth 0x200, mode 1, clip 81.
    // The wade handler's root is retargeted to camera-relative WASD at land
    // pace; camera-only lease keeps mouse look; no direction keeps the clip.
    psx_mod_write_half(p+0x60,81);psx_mod_write_byte(p+0x22c,1);psx_mod_write_byte(p+0x22d,1);
    psx_mod_write_word(p+0x224,2);psx_mod_write_word(p+0x20c,0x7fc91);
    psx_mod_write_word(p+0x834,(uint32_t)-6144);psx_mod_write_word(p+0x1c8,(uint32_t)-5632);
    ttk::input.active=true;ttk::input.move_x=0;ttk::input.move_y=0;++ttk::input.sequence;
    call(0x8003ade4,c,p,0x80025ee8);
    assert(!ttk::movement_ready() && ttk::locomotion_input_ready() && !ttk::swim_input_ready());
    assert(ttk::wade_full_speed_ready() && ttk::mid_wade_input_ready());
    psx_mod_write_half(c+12,0);psx_mod_write_half(c+16,4096); // view +Z
    psx_mod_write_half(p+0xfc,(uint16_t)(int16_t)-22);psx_mod_write_half(p+0x100,0); // clip root, body-relative world
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==-22 && (int16_t)psx_mod_read_half(p+0x100)==0); // idle: untouched
    ttk::input.move_x=0;ttk::input.move_y=1;ttk::running=true;
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0x100)==88 && (int16_t)psx_mod_read_half(p+0xfc)==0); // W: view forward, 22*4 in the 80..120 band
    ttk::input.move_x=1;ttk::input.move_y=0;
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==88 && (int16_t)psx_mod_read_half(p+0x100)==0); // D: strafe right (+X)
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,6); // short clip tick: floor 80
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==80);
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,44); // long clip tick: ceiling 120
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==120);
    ttk::running=false;psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22); // walk band 40..60
    call(0x800539f8,p,4,0x8004870c,0x801fff00);assert((int16_t)psx_mod_read_half(p+0xfc)==44);ttk::running=true;
    psx_mod_write_half(p+0xfc,0);psx_mod_write_half(p+0x100,22);
    call(0x800539f8,p,4,0x80048664,0x801fff00); // foreign caller: untouched
    assert((int16_t)psx_mod_read_half(p+0xfc)==0 && (int16_t)psx_mod_read_half(p+0x100)==22);
    psx_mod_write_byte(p+0x22c,0);psx_mod_write_half(p+0x60,76); // land run in the same water: not the wade handler
    call(0x800539f8,p,4,0x8004870c,0x801fff00);
    assert((int16_t)psx_mod_read_half(p+0xfc)==0 && (int16_t)psx_mod_read_half(p+0x100)==22);
    ttk::running=false;ttk::input.move_x=0;ttk::input.move_y=0;
    psx_mod_write_half(p+0x60,63);psx_mod_write_byte(p+0x22d,0);
    psx_mod_write_word(p+0x834,0x40000000);psx_mod_write_word(p+0x20c,0);
    ++ttk::input.sequence;call(0x8003ade4,c,p,0x80025ee8);assert(ttk::movement_ready());
    std::puts("PASS: mid-depth wade handler root retarget, idle/foreign/land exclusions");
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
    std::puts("PASS: distance clamps, convergence, epoch recovery and wall-independent preference; D10 recenter, shoulder offset and camera side file");
    std::puts("PASS: all semantic groups, upgrades, empty remote, history, unsafe attack/item guards, crouch delta and clearance fixtures");
    std::puts("PASS: immediate takeoff support probe, guarded caller/code, owned airborne reach lease");
    std::puts("PASS: respawn boom, landing seam and pre-collision stride smoothing");
    std::puts("PASS: camera constraints, duplicate look, capture and original-camera gates");
    std::puts("PASS: Vanilla, actor/caller/overlay/code/state guards; movement/probe alignment; independent facing");
    std::puts("PASS: D08T1 pushable masks (grab-only climb mask, E-only idle grab mask; caller/actor/flags/Vanilla/capture) and camera-only grab lease");
    std::puts("PASS: D12A first-person quick kick (boot request conversion from the attack only (E suppressed) and its guards, lease, sound, view-aimed hit sphere at the crosshair surface within reach, held attack with Boot, right-leg viewmodel with reversed thigh, third person untouched)");
}
