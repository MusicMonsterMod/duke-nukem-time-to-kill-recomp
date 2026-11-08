// Collision fixture checks for the actual adapter; not retail geometry proof.
#include "weapon_aim.h"
#include "level_select.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cstdio>
extern "C" {int g_precise_mode=0,g_ls_mode=0,g_psx_call_bail=0;}
static unsigned char ram[0x200000],scratch[1024];
static PSXModFunctionEntryCallback callback;
static bool ready=true,modern=true,cover=false,jet_ready=false,swim_ready=false,air_ready=false,climb_ready=false;
static unsigned writes;
static bool actor_visible, block_actor, block_muzzle, close_actor, behind_cover, ray_actor;
static unsigned char& byte(uint32_t a) {return (a&0x1ffffc00)==0x1f800000?scratch[a&1023]:ram[a&0x1fffff];}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return byte(a);}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return byte(a)|(byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
static void pokeb(uint32_t a){byte(a)^=1;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){byte(a)=v;++writes;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){psx_mod_write_byte(a,v);psx_mod_write_byte(a+1,v>>8);}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){psx_mod_write_half(a,v);psx_mod_write_half(a+2,v>>16);}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t n,uint32_t){static uint32_t next=0x801e0000;auto a=next;next+=(n+15)&~15;return a;}
extern "C" void psx_mod_host_call_begin(void){}
extern "C" void psx_mod_host_call_end(void){}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){if(a==0x8003c500)callback=cb;return 1;}
namespace ttk {bool auto_aim_console_command(const char*,void (*)(const char*),bool&);bool round_follow_console_command(const char*,void (*)(const char*),bool&);}
namespace ttk {bool free_look_active(){return false;}bool free_look_aim(double*,double*){return false;}static InputFrame f;const InputFrame& input_snapshot(Context){return f;}uint64_t input_host_frame(){return f.sequence;}bool input_modernized(){return modern;}bool input_modern_controls(){return modern;}bool movement_ready(){return ready;}const char* movement_lease_miss(){return "test";}bool locomotion_input_ready(){return false;}bool jetpack_input_ready(){return modern && jet_ready;}bool swim_weapon_ready(){return modern && swim_ready;}bool airborne_shot_ready(){return modern && air_ready;}bool climb_shot_ready(){return modern && climb_ready;}bool player_identity_ready(){return ready;}void weapon_feel_shot(){}static bool fp_view;bool first_person_view_live(){return fp_view;}bool frame_trace_on(){return false;}void frame_trace_account(uint32_t,long){}
// D10G3: the view the last present showed.
static bool sv_ok;static double sv_eye[3],sv_fwd[3],sv_age;bool shot_view(double* e,double* f,double& age){if(!sv_ok)return false;for(int i=0;i<3;++i){e[i]=sv_eye[i];f[i]=sv_fwd[i];}age=sv_age;return true;}
// D10G4: where that view drew the fixture actor.
static int sp_n;static uint32_t sp_obj;static double sp_pos[3];int shot_view_poses(uint32_t* o,double (*q)[3],int max){if(!sv_ok||!sp_n||max<1)return 0;o[0]=sp_obj;for(int i=0;i<3;++i)q[0][i]=sp_pos[i];return 1;}}
// D10G4 fixture: an actor (sphere radius 150) at its position in RAM; the
// query records the joint translation it saw.
static constexpr uint32_t seen_actor=0x800e1000,seen_model=0x800e1100,seen_mats=0x800e1200;
static bool seen_on;static double seen_query_mat_x=1e9;
// D10H2: the hit-test word (stack +0x18) of the last screen-ray and cover queries.
static uint32_t ray_test=0xffffffff,cover_test=0xffffffff;
extern "C" void psx_dispatch_call(CPUState* c,uint32_t address,uint32_t ret) {
    assert(address==0x8006d980 && ret==0x800000fc && c->gpr[4]==0x800d7198);
    uint32_t a=c->gpr[5],sp=c->gpr[29];
    double from[3],to[3],t=1;
    for(int i=0;i<3;++i){from[i]=(int32_t)psx_mod_read_word(a+4*i);to[i]=(int32_t)psx_mod_read_word(a+16+4*i);}
    if(to[2]==16384)ray_test=psx_mod_read_word(sp+0x18);
    else if(from[0]==0 && to[0]==100)cover_test=psx_mod_read_word(sp+0x18);
    // Infinite forward wall, plus a thin wall between actor and protruding muzzle.
    if(to[2]>1000 && from[2]<1000)t=(1000-from[2])/(to[2]-from[2]);
    if(cover && from[0]<50 && to[0]>50)t=std::fmin(t,(50-from[0])/(to[0]-from[0]));
    uint32_t close_hit=0;
    if(behind_cover && from[2]<-500 && to[2]>-500)t=std::fmin(t,(-500-from[2])/(to[2]-from[2]));
    if(close_actor) {
        // Sphere: the offset muzzle's entry is deeper than the screen ray's
        // front face. Clamping to that entry previously produced a backwards shot.
        double d[3],q[3],aa=0,bb=0,cc=-200*200;
        for(int i=0;i<3;++i){d[i]=to[i]-from[i];q[i]=from[i]-(i==2?500:0);aa+=d[i]*d[i];bb+=2*q[i]*d[i];cc+=q[i]*q[i];}
        double disc=bb*bb-4*aa*cc;
        if(aa>0 && disc>=0){double near=(-bb-std::sqrt(disc))/(2*aa);if(near>=0 && near<t){t=near;close_hit=0x800e0000;}}
    }
    if(ray_actor && to[2]==16384){t=500.0/16384;close_hit=0x800e0000;}
    if(seen_on && to[2]==16384) {
        double d[3],q[3],aa=0,bb=0,cc=-150*150;
        for(int i=0;i<3;++i){d[i]=to[i]-from[i];q[i]=from[i]-(int32_t)psx_mod_read_word(seen_actor+4+4*i);aa+=d[i]*d[i];bb+=2*q[i]*d[i];cc+=q[i]*q[i];}
        seen_query_mat_x=(int32_t)psx_mod_read_word(seen_mats+0x14);
        double disc=bb*bb-4*aa*cc;
        if(aa>0 && disc>=0){double near=(-bb-std::sqrt(disc))/(2*aa);if(near>=0 && near<t){t=near;close_hit=seen_actor;}}
    }
    c->gpr[2]=close_hit?close_hit:actor_visible && to[2]==500 ? ((block_actor || (block_muzzle && from[0]==100))?1:0x800e0000) : (t<1?1:0);psx_mod_write_word(c->gpr[7],2);
    for(int i=0;i<3;++i)psx_mod_write_word(psx_mod_read_word(sp+16)+4*i,std::lround(from[i]+t*(to[i]-from[i])));
    c->gte_data[0]=0xdeadbeef; // copying CPU must isolate query temporaries
}
int main(int argc,char**argv) {
    assert(argc==2);std::ifstream f(argv[1],std::ios::binary);f.seekg(2048);f.read((char*)ram+0x10000,0xbb000);assert(f.gcount()==0xbb000);
    constexpr uint32_t p=0x800d7198,cam=0x800d6eb0,sp=0x801ffe50;
    setenv("DNTTK_WEAPON_AIM","view",1);
    CPUState c{};c.gpr[29]=sp;c.gpr[31]=0x8004f7b4;c.gpr[4]=p;c.gpr[5]=sp+0x40;c.gpr[6]=p+0x144;c.gpr[7]=4;
    auto seed=[&](){c.gpr[6]=p+0x144;psx_mod_write_word(sp+0x40,100);psx_mod_write_word(sp+0x44,0);psx_mod_write_word(sp+0x48,0);psx_mod_write_word(p+0xbc,100);psx_mod_write_word(p+0xc0,0);psx_mod_write_word(p+0xc4,0);};
    for(int i=0;i<3;++i){psx_mod_write_word(p+4+4*i,0);psx_mod_write_word(cam+0x14+4*i,0);psx_mod_write_half(cam+12+2*i,i==2?4096:0);}
    psx_mod_write_byte(p+0x2e,2);seed();
    modern=false;writes=0;callback(&c,0x8003c500);assert(writes==0);modern=true;
    ttk::f.held[ttk::original_aim]=true;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    g_precise_mode=1;callback(&c,0x8003c500);assert(writes==0);g_precise_mode=0;
    ready=false;callback(&c,0x8003c500);assert(writes==0);ready=true;
    c.gpr[4]=p+0x8a4;callback(&c,0x8003c500);assert(writes==0);c.gpr[4]=p;
    for(unsigned weapon:{9u,10u,13u,14u}){c.gpr[7]=weapon;callback(&c,0x8003c500);assert(writes==0);}c.gpr[7]=4;
    pokeb(0x8003c500);callback(&c,0x8003c500);assert(writes==0);pokeb(0x8003c500);
    unsigned char stack_before[4096],scratch_before[1024];
    std::memcpy(stack_before,ram+(sp&0x1fffff)-4096,4096);std::memcpy(scratch_before,scratch,1024);
    callback(&c,0x8003c500);
    assert(!std::memcmp(stack_before,ram+(sp&0x1fffff)-4096,4096));assert(!std::memcmp(scratch_before,scratch,1024));
    assert(c.gpr[6]==0x801e0000 && c.gte_data[0]==0);
    int x=(int32_t)psx_mod_read_word(c.gpr[6]),z=psx_mod_read_word(c.gpr[6]+8);
    assert(x<0 && std::abs(100+1000.0*x/z)<1); // parallax converges at wall
    assert(psx_mod_read_word(sp+0x40)==100); // muzzle offset preserved
    seed();psx_mod_write_word(sp+0x40,500);callback(&c,0x8003c500);
    assert(psx_mod_read_word(sp+0x40)==100); // precision-camera origin cannot teleport the muzzle
    seed();cover=true;callback(&c,0x8003c500);
    assert(psx_mod_read_word(sp+0x40)==42); // retract from near cover, never teleport forward
    // D10G1 (lift 0.35, cap 35 degrees by default): a muzzle 300 below and 100 beside the screen ray. With the wall
    // 200 ahead the round leaves almost along the view (the point itself is
    // 58 degrees off); at 1000 ahead it still converges on the point.
    cover=false;
    auto off_view=[&](int eye_z){seed();psx_mod_write_word(cam+0x1c,eye_z);
        psx_mod_write_word(p+0xc0,300);psx_mod_write_word(p+0xc4,eye_z);
        callback(&c,0x8003c500);
        double d[3],n=0;for(int i=0;i<3;++i){d[i]=(int32_t)psx_mod_read_word(c.gpr[6]+4*i);n+=d[i]*d[i];}
        assert(c.gpr[6]==0x801e0000 && d[0]<=0 && d[1]<=0);
        return std::acos(d[2]/std::sqrt(n))*180/3.141592653589793;};
    {
        bool closed=false;auto quiet=[](const char*){};
        assert(ttk::point_blank_console_command("pointblank 0",quiet,closed) && closed);
        const double level=off_view(800);
        assert(ttk::point_blank_console_command("pointblank 2",quiet,closed) && !closed); // refused
        assert(ttk::point_blank_console_command("pointblank 0.25 60",quiet,closed) && !closed); // refused
        assert(ttk::point_blank_console_command("pointblank 0.5 40",quiet,closed) && closed);
        const double wide=off_view(800);
        assert(ttk::point_blank_console_command("pointblank default",quiet,closed) && closed);
        const double close=off_view(800),far=off_view(0);
        setenv("DNTTK_POINT_BLANK","0",1);const double close_off=off_view(800);unsetenv("DNTTK_POINT_BLANK");
        ttk::fp_view=true;assert(std::abs(off_view(800)-close_off)<0.01);ttk::fp_view=false; // first person unchanged
        std::printf("D10G1 round off the view: wall 200 ahead %.1f (lift 0 %.1f, point aim %.1f), 1000 ahead %.1f\n",close,level,close_off,far);
        std::fflush(stdout);assert(level<9 && close>level && close<=35.05 && wide>close && wide<=40.05 && close_off>55 && std::abs(far-17.6)<0.3);
        double last=0;
        for(int z=900;z>=0;z-=10){const double a=off_view(z);if(z%100==0)std::printf(" %d:%.1f",1000-z,a);assert(a<=35.05 && (z==900 || std::abs(a-last)<3));last=a;}std::puts("");
    }
    psx_mod_write_word(cam+0x1c,0);psx_mod_write_word(p+0xc4,0);
    std::puts("PASS: D10G1 point-blank rounds stay within 35 degrees of the view, continuous with distance");
    // D10G3: game camera eye 0,0,0 looking +z, muzzle 100 to the side, wall
    // 1000 ahead. A fresh shown view aims from the screen eye and forward; a
    // stale (over 60 ms), cut (over 1536 away) or switched-off one does not.
    {
        auto slope=[&](){seed();callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
            return (double)(int32_t)psx_mod_read_word(c.gpr[6])/(int32_t)psx_mod_read_word(c.gpr[6]+8);};
        auto view=[&](double ex,double fx,double age){ttk::sv_ok=true;ttk::sv_eye[0]=ex;ttk::sv_eye[1]=ttk::sv_eye[2]=0;
            ttk::sv_fwd[0]=fx;ttk::sv_fwd[1]=0;ttk::sv_fwd[2]=std::sqrt(4096.0*4096-fx*fx);ttk::sv_age=age;};
        const double game=slope();
        view(300,0,5);const double shifted=slope();
        view(0,2048,5);const double turned=slope();
        view(300,0,80);const double stale=slope();
        view(2000,0,5);const double cut=slope();
        bool closed=false;auto quiet=[](const char*){};
        view(300,0,5);
        assert(ttk::shot_view_console_command("shotview off",quiet,closed) && closed);const double off=slope();
        assert(ttk::shot_view_console_command("shotview maybe",quiet,closed) && !closed); // refused
        assert(ttk::shot_view_console_command("shotview default",quiet,closed) && closed);const double back=slope();
        setenv("DNTTK_SHOT_VIEW","0",1);const double env_off=slope();unsetenv("DNTTK_SHOT_VIEW");
        ttk::sv_ok=false;const double none=slope();
        std::printf("D10G3 x/z: game %.3f, screen eye +300 %.3f, screen turned 30 %.3f, stale %.3f, cut %.3f, off %.3f\n",game,shifted,turned,stale,cut,off);
        std::fflush(stdout);
        assert(std::abs(game+0.1)<0.01 && std::abs(shifted-0.2)<0.01 && std::abs(turned-(1000*std::tan(3.141592653589793/6)-100)/1000)<0.01);
        assert(stale==game && cut==game && off==game && env_off==game && none==game && back==shifted);
    }
    std::puts("PASS: D10G3 shots aim from the view on screen; stale, cut or switched-off views keep the game camera");
    // D10G4: an actor 800 ahead now 300 to the side, drawn on the crosshair
    // (shown at x 0). The screen ray meets it where drawn (z 650) and the
    // round aims at the same point on it now (x 300); its words are restored.
    {
        psx_mod_write_byte(seen_model,1);psx_mod_write_word(seen_actor+0x3c,seen_mats);psx_mod_write_word(seen_actor+0x40,seen_model);
        auto place=[&](int x){psx_mod_write_word(seen_actor+4,x);psx_mod_write_word(seen_actor+12,800);
            psx_mod_write_word(seen_mats+0x14,x);psx_mod_write_word(seen_mats+0x1c,800);};
        auto slope=[&](){seed();callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
            return (double)(int32_t)psx_mod_read_word(c.gpr[6])/(int32_t)psx_mod_read_word(c.gpr[6]+8);};
        seen_on=true;ttk::sv_ok=true;for(int i=0;i<3;++i){ttk::sv_eye[i]=0;ttk::sv_fwd[i]=i==2?4096:0;}ttk::sv_age=5;
        ttk::sp_n=1;ttk::sp_obj=seen_actor;ttk::sp_pos[0]=ttk::sp_pos[1]=0;ttk::sp_pos[2]=800;
        place(300);const double seen=slope();const double mat_during=seen_query_mat_x;
        const bool restored=(int32_t)psx_mod_read_word(seen_actor+4)==300 && (int32_t)psx_mod_read_word(seen_mats+0x14)==300 &&
            (int32_t)psx_mod_read_word(seen_actor+12)==800;
        bool closed=false;auto quiet=[](const char*){};
        assert(ttk::shoot_seen_console_command("shootseen off",quiet,closed) && closed);const double off=slope();
        assert(ttk::shoot_seen_console_command("shootseen sideways",quiet,closed) && !closed); // refused
        assert(ttk::shoot_seen_console_command("shootseen default",quiet,closed) && closed);const double back=slope();
        setenv("DNTTK_SHOOT_SEEN","0",1);const double env_off=slope();unsetenv("DNTTK_SHOOT_SEEN");
        ttk::sv_ok=false;const double game=slope();ttk::sv_ok=true;          // no screen view: no rewind
        ttk::sv_age=80;const double stale=slope();ttk::sv_age=5;
        ttk::sp_pos[0]=-2000;const double jump=slope();ttk::sp_pos[0]=0;     // over 1536: not rewound
        place(0);const double still=slope();
        std::printf("D10G4 x/z: drawn on the crosshair, moved 300 %.3f (query saw joint x %.0f), off %.3f, game camera %.3f, still %.3f\n",seen,mat_during,off,game,still);
        std::fflush(stdout);
        assert(restored && mat_during==0 && std::abs(seen-200.0/650)<0.01 && back==seen);
        assert(std::abs(off+0.1)<0.01 && env_off==off && game==off && stale==off && jump==off && std::abs(still+100.0/650)<0.01);
        seen_on=false;ttk::sv_ok=false;ttk::sp_n=0;place(0);
    }
    std::puts("PASS: D10G4 the crosshair meets actors where drawn; the round aims at them now and every word is restored");
    // D10H2: the screen ray uses the rounds' own actor test (word 3), the
    // body-to-muzzle cover check the coarse one (0). A round aimed at the
    // fixture actor (drawn on the crosshair, now 300 aside: target 300,0,650)
    // meets it after the actor's own update stepped it 60 further: its first
    // sweep goes through 360,0,650 (x/z 0.4 from the muzzle at 100,0,0).
    {
        psx_mod_write_byte(seen_model,1);psx_mod_write_word(seen_actor+0x3c,seen_mats);psx_mod_write_word(seen_actor+0x40,seen_model);
        auto place=[&](int x){psx_mod_write_word(seen_actor+4,x);psx_mod_write_word(seen_actor+12,800);
            psx_mod_write_word(seen_mats+0x14,x);psx_mod_write_word(seen_mats+0x1c,800);};
        seen_on=true;ttk::sv_ok=true;for(int i=0;i<3;++i){ttk::sv_eye[i]=0;ttk::sv_fwd[i]=i==2?4096:0;}ttk::sv_age=5;
        ttk::sp_n=1;ttk::sp_obj=seen_actor;ttk::sp_pos[0]=ttk::sp_pos[1]=0;ttk::sp_pos[2]=800;
        constexpr uint32_t round=0x800da978+0x60*3;
        // Shoot, then build the round as the launcher and integrator would:
        // +0x10 owner, +0x14 origin, +4 end of the first sweep, +0x34 direction.
        auto sweep=[&](int step,bool first=true,uint32_t owner=0x800d7198,bool dead=false,bool vanilla=false){
            place(300);seed();callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
            double d[3],n=0;for(int i=0;i<3;++i){d[i]=(int32_t)psx_mod_read_word(c.gpr[6]+4*i);n+=d[i]*d[i];}n=std::sqrt(n);
            const double from[3]={first?100.0:101.0,0,0};
            for(int i=0;i<3;++i){psx_mod_write_word(round+0x14+4*i,std::lround(from[i]));psx_mod_write_word(round+4+4*i,std::lround(from[i]+d[i]/n*3500));psx_mod_write_word(round+0x34+4*i,std::lround(d[i]));}
            psx_mod_write_word(round+0x10,owner);
            place(300+step);if(dead)psx_mod_write_word(seen_actor,2);
            CPUState r=c;r.gpr[4]=round;modern=!vanilla;callback(&r,0x8006dd2c);modern=true;psx_mod_write_word(seen_actor,0);
            const double ex=(int32_t)psx_mod_read_word(round+4)-from[0],ez=(int32_t)psx_mod_read_word(round+12)-from[2];
            const double vx=(int32_t)psx_mod_read_word(round+0x34),vz=(int32_t)psx_mod_read_word(round+0x3c);
            assert(std::abs(ex/ez-vx/vz)<0.002 && std::abs(std::hypot(ex,ez)-3500)<2); // end and direction agree; speed kept
            return vx/vz;};
        const double moved=sweep(60),still=sweep(0),later=sweep(60,false),other=sweep(60,true,0x800e2000),dead=sweep(60,true,0x800d7198,true),jump=sweep(600),vanilla=sweep(60,true,0x800d7198,false,true);
        bool closed=false;auto quiet=[](const char*){};
        assert(ttk::round_follow_console_command("roundfollow off",quiet,closed) && closed);const double off=sweep(60);
        assert(ttk::round_follow_console_command("roundfollow maybe",quiet,closed) && !closed); // refused
        assert(ttk::round_follow_console_command("roundfollow default",quiet,closed) && closed);const double back=sweep(60);
        std::printf("D10H2 x/z: query test %u, cover test %u; first sweep after a 60 step %.3f, still %.3f, later sweep %.3f, other owner %.3f, dead %.3f, 600 jump %.3f, Vanilla %.3f, off %.3f\n",
            ray_test,cover_test,moved,still,later,other,dead,jump,vanilla,off);
        std::fflush(stdout);
        assert(ray_test==3 && cover_test==0);
        assert(std::abs(moved-0.4)<0.005 && back==moved && std::abs(still-200.0/650)<0.005);
        for(double v:{later,other,dead,jump,vanilla,off})assert(std::abs(v-still)<0.005);
        seen_on=false;ttk::sv_ok=false;ttk::sp_n=0;place(0);psx_mod_write_word(round+0x10,0);
    }
    std::puts("PASS: D10H2 aim queries use the rounds' own actor test; a round aimed at an actor meets it after the actor's own update");
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);
    psx_mod_write_word(p+0x224,0);assert(ttk_aim_reticle()); // walking is still armed
    // Native marker selection suppresses the modern reticle, including the
    // hotkey, without destroying either saved crosshair preference.
    setenv("DNTTK_RED_DOT","1",1);
    assert(!ttk_aim_crosshair_enabled() && !ttk_aim_reticle());
    ttk_aim_toggle_crosshair();assert(!ttk_aim_reticle());
    setenv("DNTTK_RED_DOT","0",1);assert(ttk_aim_reticle());
    setenv("DNTTK_CROSSHAIR","0",1);setenv("DNTTK_RED_DOT","1",1);
    ttk_aim_toggle_crosshair();
    setenv("DNTTK_RED_DOT","0",1);assert(!ttk_aim_crosshair_enabled());
    setenv("DNTTK_CROSSHAIR","1",1);
    ttk_aim_toggle_crosshair();assert(!ttk_aim_crosshair_enabled());
    ttk_aim_reset_crosshair_override();assert(ttk_aim_crosshair_enabled());
    ttk::f.held[ttk::original_aim]=true;assert(!ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    psx_mod_write_byte(p+0x3b8,0);assert(!ttk_aim_reticle());
    psx_mod_write_byte(p+0x3b8,1);assert(!ttk_aim_reticle()); // held inventory object
    // D08Q3: flight has no ground lease. Both gun callers still converge
    // on the camera ray, including vertical aim, with independent reticle
    // preference and unchanged guard/original-aim rejection.
    ready=false;jet_ready=true;cover=false;
    psx_mod_write_byte(p+0x3b8,2);
    for(unsigned weapon:{4u,5u,8u,11u,28u,29u}) {
        c.gpr[7]=weapon;c.gpr[31]=weapon==4?0x8004f7b4:0x8004fa88;
        psx_mod_write_byte(p+0x3b9,weapon);
        for(int pitch:{-2048,0,2048}) {
            psx_mod_write_half(cam+14,pitch);seed();callback(&c,0x8003c500);
            assert(c.gpr[6]==0x801e0000 && ttk_aim_reticle());
            const int vy=(int32_t)psx_mod_read_word(c.gpr[6]+4);
            assert(pitch==0?vy==0:vy*pitch>0);
        }
    }
    psx_mod_write_half(cam+14,0);c.gpr[7]=4;c.gpr[31]=0x8004f7b4;
    setenv("DNTTK_CROSSHAIR","0",1);assert(!ttk_aim_reticle());
    seed();callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
    setenv("DNTTK_CROSSHAIR","1",1);
    ttk::f.held[ttk::original_aim]=true;seed();writes=0;callback(&c,0x8003c500);
    assert(writes==0 && !ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    pokeb(0x8003c500);seed();writes=0;callback(&c,0x8003c500);
    assert(writes==0 && !ttk_aim_reticle());pokeb(0x8003c500);
    modern=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());modern=true;
    jet_ready=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());
    ready=true;psx_mod_write_byte(p+0x3b9,4);
    std::puts("PASS: D08Q3 flight shots, vertical aim, crosshair preference and guarded fallback");
    // D07F: a jump or fall holds only the camera-only lease. Its shots (gun
    // callers, pitch down and up) still converge on the camera ray.
    ready=false;air_ready=true;
    for(unsigned weapon:{4u,5u,8u}) {
        c.gpr[7]=weapon;c.gpr[31]=weapon==4?0x8004f7b4:0x8004fa88;
        psx_mod_write_byte(p+0x3b9,weapon);
        for(int pitch:{-2048,2048}) {
            psx_mod_write_half(cam+14,pitch);seed();callback(&c,0x8003c500);
            assert(c.gpr[6]==0x801e0000 && ttk_aim_reticle());
            assert((int32_t)psx_mod_read_word(c.gpr[6]+4)*pitch>0);
        }
    }
    air_ready=false;psx_mod_write_half(cam+14,0);c.gpr[7]=4;c.gpr[31]=0x8004f7b4;
    seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());
    ready=true;psx_mod_write_byte(p+0x3b9,4);
    std::puts("PASS: D07F airborne shots converge on the view ray");
    // D08O1: the original swim states (surface/underwater) hold their own
    // lease, not the ground one; shots converge on the view the same way.
    ready=false;swim_ready=true;
    for(int pitch:{-2048,0,2048}) {
        psx_mod_write_half(cam+14,pitch);seed();callback(&c,0x8003c500);
        assert(c.gpr[6]==0x801e0000 && ttk_aim_reticle());
        const int vy=(int32_t)psx_mod_read_word(c.gpr[6]+4);
        assert(pitch==0?vy==0:vy*pitch>0);
    }
    psx_mod_write_half(cam+14,0);
    modern=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());modern=true;
    swim_ready=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());
    ready=true;
    std::puts("PASS: D08O1 swim shots, vertical aim and guarded fallback");
    // D08J6A: an armed settled climb holds its own lease; sidearm shots
    // converge on the view, never without it or in Vanilla.
    ready=false;climb_ready=true;
    for(int pitch:{-2048,0,2048}) {
        psx_mod_write_half(cam+14,pitch);seed();callback(&c,0x8003c500);
        assert(c.gpr[6]==0x801e0000 && ttk_aim_reticle());
        const int vy=(int32_t)psx_mod_read_word(c.gpr[6]+4);
        assert(pitch==0?vy==0:vy*pitch>0);
    }
    psx_mod_write_half(cam+14,0);
    modern=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());modern=true;
    climb_ready=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());
    ready=true;
    std::puts("PASS: D08J6A climb sidearm shots, vertical aim and guarded fallback");
    // The ordinary shotgun/rifle/Gatling/RPG handler has a distinct call site.
    for(unsigned weapon:{5u,6u,7u,8u,11u}) {
        c.gpr[31]=0x8004fa88;c.gpr[7]=weapon;seed();callback(&c,0x8003c500);
        assert(c.gpr[6]==0x801e0000);
    }
    c.gpr[31]=0x8004fa84;seed();writes=0;callback(&c,0x8003c500);assert(writes==0);
    c.gpr[31]=0x8004fa88;pokeb(0x8004fa80);seed();writes=0;callback(&c,0x8003c500);assert(writes==0);pokeb(0x8004fa80);
    c.gpr[31]=0x8004f7b4;c.gpr[7]=4;
    seed();c.gpr[31]=0x8004e7e4;c.gpr[7]=12;c.gpr[5]=sp+0x30;c.gpr[6]=sp+0x20;
    psx_mod_write_word(sp+0x10,321);psx_mod_write_word(sp+0x14,99);
    callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
    assert(psx_mod_read_word(sp+0x10)==321 && psx_mod_read_word(sp+0x14)==99);
    c.gpr[5]=sp+0x40;c.gpr[6]=sp+0x20;writes=0;callback(&c,0x8003c500);assert(writes==0);
    c.gpr[31]=0x8004f7b4;c.gpr[7]=4;seed();
    // Independent marker controls cannot alter supported free-view shot vectors.
    cover=false;psx_mod_write_byte(p+0x3b8,2);
    setenv("DNTTK_CROSSHAIR","0",1);assert(!ttk_aim_reticle());
    seed();callback(&c,0x8003c500);int free_x=(int32_t)psx_mod_read_word(c.gpr[6]);
    setenv("DNTTK_RED_DOT","0",1);seed();callback(&c,0x8003c500);
    assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    setenv("DNTTK_AIM_ASSIST","original-lock",1);
    psx_mod_write_word(p+0x288,0x800e0000);psx_mod_write_word(p+0x2b0,1);
    psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e00f0,0);psx_mod_write_word(0x800e00f4,500);
    actor_visible=true;seed();callback(&c,0x8003c500);
    int assist_x=(int32_t)psx_mod_read_word(c.gpr[6]);
    assert(assist_x<free_x && std::abs(100+500.0*assist_x/(int32_t)psx_mod_read_word(c.gpr[6]+8)-20)<1);
    setenv("DNTTK_CROSSHAIR","1",1);setenv("DNTTK_RED_DOT","1",1);
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==assist_x);
    block_actor=true;seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    block_actor=false;block_muzzle=true;seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    block_muzzle=false;psx_mod_write_word(0x800e00ec,300); // outside six-degree cone
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e0000,2); // dead target
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    // Point-blank overlap must begin behind the whole actor, then hit forwards.
    unsetenv("DNTTK_AIM_ASSIST");actor_visible=false;close_actor=true;seed();
    psx_mod_write_word(cam+0x1c,-1000);psx_mod_write_word(p+0xc4,450);
    callback(&c,0x8003c500);
    std::printf("Point-blank sphere: forward direction=%d, origin=(%d,%d)\n",
        (int32_t)psx_mod_read_word(c.gpr[6]+8),(int32_t)psx_mod_read_word(sp+0x40),(int32_t)psx_mod_read_word(sp+0x48));std::fflush(stdout);
    assert((int32_t)psx_mod_read_word(c.gpr[6]+8)>4000);
    assert(psx_mod_read_word(sp+0x40)==0 && psx_mod_read_word(sp+0x48)==0);
    // Camera obstruction behind Duke cannot reverse a forward shot.
    close_actor=false;behind_cover=true;seed();callback(&c,0x8003c500);
    assert((int32_t)psx_mod_read_word(c.gpr[6]+8)>0);
    behind_cover=false;psx_mod_write_word(cam+0x1c,0);
    // Only the guarded marker's pending quad may collapse; aim fields untouched.
    // D07D: ownership comes from the enqueue frame alone (S4 actor, marker
    // frame RA), with no entry lease, camera link or gameplay gate.
    CPUState marker{};marker.gpr[29]=sp-0x48;marker.gpr[31]=0x80033db0;marker.gpr[20]=p;
    marker.gpr[5]=marker.gpr[16]=0x801d0000;psx_mod_write_word(sp-0x48+0x40,0x8003543c);
    psx_mod_write_byte(0x801d0003,9);psx_mod_write_byte(0x801d0007,0x2c);
    auto quad=[&](){for(unsigned offset:{8u,16u,24u,32u})psx_mod_write_word(0x801d0000+offset,0x00640064);};
    auto hidden=[&](){for(unsigned offset:{8u,16u,24u,32u})if(psx_mod_read_word(0x801d0000+offset))return false;return true;};
    setenv("DNTTK_RED_DOT","1",1);quad();writes=0;callback(&marker,0x8002bc18);assert(writes==0 && !hidden());
    setenv("DNTTK_RED_DOT","0",1);ready=false;g_precise_mode=1;
    callback(&marker,0x8002bc18);assert(hidden());
    assert(psx_mod_read_word(p+0x288)==0x800e0000 && psx_mod_read_word(p+0x2b0)==1);
    ready=true;g_precise_mode=0;
    marker.gpr[20]=0x800e0000;quad();writes=0;callback(&marker,0x8002bc18);assert(writes==0); // another actor
    marker.gpr[20]=p;psx_mod_write_word(sp-0x48+0x40,0x80035440);writes=0;callback(&marker,0x8002bc18);assert(writes==0); // foreign frame
    psx_mod_write_word(sp-0x48+0x40,0x8003543c);g_ls_mode=1;writes=0;callback(&marker,0x8002bc18);assert(writes==0);
    g_ls_mode=0;modern=false;callback(&marker,0x8002bc18);assert(writes==0); // Vanilla keeps the dot
    modern=true;pokeb(0x80033b00);writes=0;callback(&marker,0x8002bc18);assert(writes==0); // changed marker code
    pokeb(0x80033b00);callback(&marker,0x8002bc18);assert(hidden());
    // Authenticated energy constructor completion: preserve physical origin.
    // D22D: the first beam locks onto the targetable actor under the crosshair
    // (the updater damages only +0x14), never the original acquisition.
    ready=false;jet_ready=true; // beam completion must also accept flight
    psx_mod_write_word(0x800e0000,1);psx_mod_write_byte(0x800e002e,1);
    psx_mod_write_word(0x800e0004,20);psx_mod_write_word(0x800e0008,0);psx_mod_write_word(0x800e000c,500);
    psx_mod_write_word(0x800da454,0x801c0000);psx_mod_write_word(0x800da47c,4);
    psx_mod_write_word(0x801c0058+0x4c,0x801c1000);psx_mod_write_word(0x801c1004,0);psx_mod_write_word(0x801c1008,0x800e0000);
    psx_mod_write_word(0x800c3b58,36000000);
    c.gpr[31]=0x80058b80;c.gpr[7]=10;c.gpr[5]=p+0x80c;c.gpr[6]=p+0x144;
    psx_mod_write_word(p+0x80c,100);psx_mod_write_word(p+0x810,0);psx_mod_write_word(p+0x814,0);
    ttk::f.active=true;
    CPUState beam{};beam.gpr[29]=sp-0x110;beam.gpr[5]=beam.gpr[29]+0x10;beam.gpr[16]=0x800da978;beam.gpr[23]=p;
    // shot: fire with the crosshair ray ending on the actor or not; object: the
    // original constructor's projectile with its own lock and acquisition count.
    auto shot=[&](bool on_actor){ray_actor=on_actor;c.gpr[5]=p+0x80c;c.gpr[6]=p+0x144;callback(&c,0x8003c500);ray_actor=false;
        assert(c.gpr[5]!=p+0x80c && psx_mod_read_word(p+0x80c)==100);};
    auto object=[&](unsigned index,uint32_t lock,unsigned count){beam.gpr[17]=index;beam.gpr[20]=count;
        psx_mod_write_word(0x800da988,p);psx_mod_write_byte(0x800da9a8,15);
        psx_mod_write_word(0x800da990,p+0x80c);psx_mod_write_word(0x800da98c,lock);};
    auto finish=[&](){beam.gpr[31]=0x80073380;callback(&beam,0x8001ca4c);return psx_mod_read_word(0x800da98c);};
    shot(true);object(0,0x800e1000,1);
    writes=0;beam.gpr[31]=0x8007337c;callback(&beam,0x8001ca4c);assert(writes==0); // foreign list advance
    assert(finish()==0x800e0000 && psx_mod_read_word(0x800da990)!=p+0x80c && beam.gpr[20]==1);
    {   // D49: the beam's record carries the view it was aimed from (redraws
        // move the drawn beam by the view change since); other projectiles,
        // or a slot whose origin is not its record, have none.
        uint32_t rec=0;double eye[3],fwd[3];
        assert(ttk::aim_beam_view(0x800da978,rec,eye,fwd) && rec==psx_mod_read_word(0x800da990));
        const double n=std::sqrt(fwd[0]*fwd[0]+fwd[1]*fwd[1]+fwd[2]*fwd[2]);assert(std::fabs(n-1)<0.01);
        assert(!ttk::aim_beam_view(0x800da978+0x60,rec,eye,fwd) && !ttk::aim_beam_view(0x800da978+8,rec,eye,fwd));
        const uint32_t was=psx_mod_read_word(0x800da990);psx_mod_write_word(0x800da990,p+0x80c);
        assert(!ttk::aim_beam_view(0x800da978,rec,eye,fwd));psx_mod_write_word(0x800da990,was);
        assert(ttk::aim_beam_follow_enabled());
    }
    object(0,0,1);writes=0;callback(&beam,0x8001ca4c);assert(writes==0); // consumed lease
    shot(false);object(0,0x800e0000,1);assert(finish()==0); // a miss: no auto-lock
    psx_mod_write_word(0x800e0000,3);shot(true);object(0,0x800e0000,1);assert(finish()==0); // dead
    psx_mod_write_word(0x800e0000,1);
    psx_mod_write_word(0x800c3b58,400*400);shot(true);object(0,0x800e0000,1);assert(finish()==0); // out of reach
    psx_mod_write_word(0x800c3b58,36000000);
    psx_mod_write_word(0x801c1008,0x800e1000);shot(true);object(0,0x800e0000,1);assert(finish()==0); // not targetable
    psx_mod_write_word(0x801c1008,0x800e0000);
    // Upgrade (several acquired targets): the crosshair beam first, then the
    // original's extra locks inside 45 degrees of the view; the crosshair
    // actor's extra lock becomes the displaced original first lock.
    shot(false);object(0,0x800e0000,3);assert(finish()==0 && beam.gpr[20]==3);
    object(1,0x800e0000,3);assert(finish()==0x800e0000);
    psx_mod_write_word(0x800e00ec,4000);psx_mod_write_word(0x800e00f4,1000); // 76 degrees off the view
    object(2,0x800e0000,3);assert(finish()==0);
    psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e00f4,500);
    shot(true);object(0,0x800e1000,2);assert(finish()==0x800e0000 && beam.gpr[20]==2);
    object(1,0x800e0000,2);assert(finish()==0); // displaced 0x800e1000 is not targetable: no double hit
    object(3,0x800e0000,3);writes=0;callback(&beam,0x8001ca4c);assert(writes==0); // never a fourth beam
    std::puts("PASS: D22D energy beam locks the crosshair actor; misses, dead, reach, list and upgrade extras; D49 shot view record");
    // D10H3 AUTO AIM EDUKE32: the bullet weapons go exactly at the nearest
    // hostile targetable actor (the room list above; never an NPC) inside
    // 48/2048 of a turn either side of the view, seen from the eye and reached
    // from the muzzle. Type table: 57 a pig cop (hostile flag, class 6), 922 a
    // dancer (class 8, no hostile flag), 540 a prop (class 9, no hostile flag).
    ready=true;jet_ready=false;ttk::f.active=false;
    c.gpr[31]=0x8004f7b4;c.gpr[5]=sp+0x40;c.gpr[6]=p+0x144;c.gpr[7]=4;
    auto fire=[&](unsigned weapon){c.gpr[7]=weapon;c.gpr[5]=sp+0x40;seed();callback(&c,0x8003c500);c.gpr[7]=4;
        const double x=(int32_t)psx_mod_read_word(c.gpr[6]),z=(int32_t)psx_mod_read_word(c.gpr[6]+8);return 100+500.0*x/z;};
    auto aimed=[](double at){return std::abs(at-20)<1;};
    psx_mod_write_word(0x800e0000,1);psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e00f0,0);psx_mod_write_word(0x800e00f4,500);
    psx_mod_write_word(0x800d2660,0x801b0000);
    psx_mod_write_word(0x801b0000+28*57,0x50208001);psx_mod_write_byte(0x801b0000+28*57+6,6);
    psx_mod_write_word(0x801b0000+28*922,0x40628001);psx_mod_write_byte(0x801b0000+28*922+6,8);
    psx_mod_write_word(0x801b0000+28*540,0x00218003);psx_mod_write_byte(0x801b0000+28*540+6,9);
    psx_mod_write_half(0x800e002c,57);psx_mod_write_byte(0x800e0015,6);
    setenv("DNTTK_AIM_ASSIST","off",1);actor_visible=true;assert(!aimed(fire(4)));
    setenv("DNTTK_AIM_ASSIST","duke3d",1);assert(!aimed(fire(4))); // the first build's name is not a mode
    // EDUKE32: every view-aimed gun call (the throws never reach this path).
    setenv("DNTTK_AIM_ASSIST","eduke32",1);
    for(unsigned weapon:{3u,4u,5u,6u,7u,8u,11u,28u,29u})assert(aimed(fire(weapon)));
    for(unsigned weapon:{1u,12u,14u})assert(!aimed(fire(weapon)));
    // HITSCAN: the bullet guns, Buffalo Rifle included.
    setenv("DNTTK_AIM_ASSIST","hitscan",1);
    for(unsigned weapon:{4u,5u,6u,7u,28u})assert(aimed(fire(weapon)));
    for(unsigned weapon:{3u,8u,29u,11u})assert(!aimed(fire(weapon))); // Crossbow and projectiles
    block_actor=true;assert(!aimed(fire(4)));block_actor=false;            // not seen from the eye
    block_muzzle=true;assert(!aimed(fire(4)));block_muzzle=false;          // muzzle behind cover
    psx_mod_write_word(0x800e0000,3);assert(!aimed(fire(4)));psx_mod_write_word(0x800e0000,1); // dead
    psx_mod_write_word(0x801c1008,0x800e1000);assert(!aimed(fire(4)));psx_mod_write_word(0x801c1008,0x800e0000); // not targetable
    psx_mod_write_word(0x800e00f0,400);assert(!aimed(fire(4)));psx_mod_write_word(0x800e00f0,0); // off screen vertically
    psx_mod_write_half(0x800e002c,922);psx_mod_write_byte(0x800e0015,8);assert(!aimed(fire(4))); // a dancer: never
    ray_actor=true;assert(!aimed(fire(4)));ray_actor=false;                                   // not even under the crosshair
    psx_mod_write_half(0x800e002c,540);psx_mod_write_byte(0x800e0015,9);assert(!aimed(fire(4))); // a prop
    psx_mod_write_half(0x800e002c,57);psx_mod_write_byte(0x800e0015,8);assert(!aimed(fire(4)));  // an NPC-class actor
    psx_mod_write_byte(0x800e0015,6);assert(aimed(fire(4)));
    psx_mod_write_word(0x800e00ec,100); // 11.3 degrees: outside EDuke32's 8.4
    {double at=fire(4);assert(std::abs(at-100)>=1);}
    bool shut=false;auto quiet=[](const char*){};
    assert(ttk::auto_aim_console_command("autoaim 70",quiet,shut) && shut);   // 12.3 degrees
    {double at=fire(4);assert(std::abs(at-100)<1);}
    assert(ttk::auto_aim_console_command("autoaim default",quiet,shut));
    assert(ttk::auto_aim_console_command("autoaim 999",quiet,shut) && !shut);
    {double at=fire(4);assert(std::abs(at-100)>=1);}
    psx_mod_write_word(0x800e00ec,20);
    modern=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0);modern=true; // Vanilla
    actor_visible=false;unsetenv("DNTTK_AIM_ASSIST");
    std::puts("PASS: D10H3 EDuke32 auto aim: EDUKE32 every gun, HITSCAN the bullet guns, never NPCs or props, cone, eye and muzzle checks, dead, list, vertical, console, Vanilla");
    setenv("DNTTK_WEAPON_AIM","original",1);seed();writes=0;callback(&c,0x8003c500);assert(writes==0);
    std::puts("weapon aim guards, parallax, muzzle obstruction and original fallback passed");
}
