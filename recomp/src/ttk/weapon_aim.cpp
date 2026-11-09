#include <atomic>
#include <chrono>
// SLUS-00583 / LEVEL00 weapon adapter. Runs before the original dispatch,
// preserving muzzle, spread, projectile integration and damage handling.
#include "weapon_aim.h"
#include "aim_weapons.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "live_env.h"
#include "code_identity.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
extern "C" {extern int g_precise_mode,g_ls_mode,g_psx_call_bail;}
namespace ttk {
constexpr uint32_t player=0x800d7198, camera=0x800d6eb0;
struct Guard {uint32_t address,size;const char* digest;};
#include "aim_guards.inc"
static unsigned shots, rejected, queries, lease_misses;
static char last_lease_miss[96]="none";
static int last_kind, last_weapon;
static double last_origin[3],last_target[3],last_direction[3];
static bool busy;
static unsigned hidden_markers, assisted_shots;
static bool last_assisted;
static unsigned close_retractions;
// D10G1: shots bent toward the view because the crosshair point was close.
static unsigned point_blank_shots;
static double last_bend;
static uint32_t aim_vector, aim_origin, beam_origins;
// One record per projectile pool slot (100 x 0x60 at 0x800da978): the beam's
// fixed origin (+0, read by the updater through projectile +0x18), a tag
// (+12) and, for D49, the view the shot was aimed from: eye (+16) and
// forward x4096 (+28). Redraw workers read the record from guest memory.
constexpr uint32_t beam_record=48, beam_slots=100, beam_tag=0x44343953u;
void aim_render_prepare() {
    // Populate contents only when used; reserve stable host pointers before
    // replay workers fork so first shots cannot force a synchronous restart.
    if(!aim_vector) aim_vector=psx_mod_alloc_guest_memory(16,16);
    if(!aim_origin) aim_origin=psx_mod_alloc_guest_memory(16,16);
    if(!beam_origins) beam_origins=psx_mod_alloc_guest_memory(beam_slots*beam_record,16);
}
static uint32_t beam_shot_sp;
static uint64_t beam_epoch;
static bool beam_pending;
static unsigned view_beams;
static bool view_mode() {
    if(!input_modern_controls())return false;
    const char* mode=ttk_live_env("DNTTK_WEAPON_AIM");
    return mode && !std::strcmp(mode,"view");
}
static bool supported(unsigned weapon) {return view_weapon_supported(weapon);}
// Jumps, falls, flight and the original swim states own a camera/input lease,
// never the ground movement lease. Reuse their authenticated state for shots
// and beam completion without widening any locomotion writes (D07F: jumps
// and falls). D08J6A: of the attached states, only an armed settled climb.
static bool shot_ready() {return movement_ready() || airborne_shot_ready() || jetpack_input_ready() || swim_weapon_ready() || climb_shot_ready();}
extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
static uint64_t aim_identity_calls, aim_identity_checks;
static bool identity() {
    static thread_local std::array<std::vector<uint32_t>,sizeof aim_guards/sizeof aim_guards[0]> expected;
    static thread_local IdentityMemo memo;
    ++aim_identity_calls;
    const uint64_t frame=input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen)) {
        memo.set(frame,g_dirty_ram_code_gen,code_identity(aim_guards,expected,psx_mod_read_word,g_psx_ram));
        ++aim_identity_checks;
    }
    return memo.ok;
}
static void read_vector(uint32_t a,double* v) {for(int i=0;i<3;++i)v[i]=(int32_t)psx_mod_read_word(a+4*i);}
static void write_vector(uint32_t a,const double* v) {for(int i=0;i<3;++i)psx_mod_write_word(a+4*i,(int32_t)std::lround(v[i]));}
static bool normal(double* v) {
    double length=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(!std::isfinite(length)||length<1)return false;
    for(int i=0;i<3;++i)v[i]/=length;
    return true;
}
// The original segment query consumes two padded s32 XYZ vectors, room seed,
// ignored actor and output pointers. Query-only: no damage or projectile spawn.
// Keep nested CPU/GTE and scratch RAM independent of the original call.
// test: the projectile hit-test word the original sweep (0x8006dd2c) passes
// from projectile +0 to the actor test 0x80076330 (stack argument 5; argument
// 6, projectile +0x2e, stays 0). 0 is the coarse actor test; bit 2 selects the
// model-box test 0x80075f0c for actors with a +0x40 model box and no +0x44
// sphere list.
static bool trace(CPUState* source,const double* from,const double* to,double* hit,int& kind,uint32_t* actor=nullptr,uint32_t ignore=player,uint32_t test=0) {
    uint32_t top=source->gpr[29];
    if(top<0x801f4000 || top>0x801ffff0)return false;
    if(top&3)return false;
    // Word copies (D10F1: the camera lens runs several queries per update).
    uint32_t saved[1024],scratch[256];
    for(unsigned i=0;i<1024;++i)saved[i]=psx_mod_read_word(top-4096+4*i);
    for(unsigned i=0;i<256;++i)scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState cpu=*source;uint32_t sp=top-0x100;
    cpu.pc=0;cpu.gpr[29]=sp;cpu.gpr[31]=0x800000fc;
    cpu.gpr[4]=ignore;cpu.gpr[5]=sp+0x30;
    cpu.gpr[6]=(int8_t)psx_mod_read_byte(player+0x2e);cpu.gpr[7]=sp+0x50;
    for(unsigned i=0;i<0x80;i+=4)psx_mod_write_word(sp+i,0);
    psx_mod_write_word(sp+0x18,test);
    write_vector(sp+0x30,from);write_vector(sp+0x40,to);
    psx_mod_write_word(sp+0x10,sp+0x60);psx_mod_write_word(sp+0x14,sp+0x70);
    psx_mod_write_word(sp+0x50,0xffffffff);
    psx_mod_host_call_begin(); // no savestate inside this call
    psx_dispatch_call(&cpu,0x8006d980,0x800000fc);
    psx_mod_host_call_end();
    kind=cpu.gpr[2];int room=(int32_t)psx_mod_read_word(sp+0x50);
    if(std::getenv("DNTTK_AIM_TRACE"))std::fprintf(stderr,"ttk-aim-query kind=%d room=%d pc=%08x from=%.0f,%.0f,%.0f to=%.0f,%.0f,%.0f\n",kind,room,cpu.pc,from[0],from[1],from[2],to[0],to[1],to[2]);
    uint32_t raw=(uint32_t)kind;
    // Original return contract: 0 miss, 1 world, otherwise actor pointer.
    // A world boundary hit can legitimately report room -1.
    bool hit_actor=raw>=0x80010000u && raw<=0x801ffffcu && !(raw&3);
    bool ok=cpu.pc==0 && !g_psx_call_bail && (kind==0 || kind==1 || hit_actor) && room>=-1 && room<128 && (kind || room>=0);
    if(ok) {if(kind)read_vector(sp+0x60,hit);else std::copy(to,to+3,hit);}
    for(unsigned i=0;i<1024;++i)psx_mod_write_word(top-4096+4*i,saved[i]);
    for(unsigned i=0;i<256;++i)psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    if(actor)*actor=ok && hit_actor?raw:0;
    kind=kind==0?0:kind==1?1:2;
    ++queries;return ok;
}
// D10H2: Duke's rounds are hit-tested with the projectile word +0 = 3 (the
// launcher 0x800718ac sets it when the shooter is type 0x20 and the projectile
// type's table word, 0x800C4BC0 + 64 * type, has bit 4: pistol 0, shotgun 1,
// Gatling 7, RPG 8, Freezer 13, all of Duke's view-aimed bullets). The
// crosshair query, the D10H3 auto aim and the original-lock checks ask
// "would the round meet this actor", so they use the same test: the coarse
// one (word 0) met a running pig cop's edge that the round's model-box test
// then missed. DNTTK_ROUND_TEST=0 keeps the coarse test (developer A/B).
static uint32_t round_test() {
    static const int on=[]{const char* v=std::getenv("DNTTK_ROUND_TEST");return !v || std::strcmp(v,"0");}();
    return on?3u:0u;
}
bool view_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind) {
    return player_identity_ready() && identity() && trace(cpu,from,to,hit,kind);
}
bool camera_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind,uint32_t ignore,uint32_t* actor) {
    return player_identity_ready() && identity() && trace(cpu,from,to,hit,kind,actor,ignore?ignore:player);
}
// Assistance follows only an existing original target lock close to the view.
// Independently recheck camera AND physical-muzzle visibility against that exact
// actor. Failure leaves the free-view target intact; visibility settings do not
// participate. Original spread/integration/damage still own the projectile.
static bool assist(CPUState* cpu,const double* eye,const double* muzzle,const double* forward,double* target) {
    const char* mode=ttk_live_env("DNTTK_AIM_ASSIST");
    if(!mode || std::strcmp(mode,"original-lock"))return false;
    uint32_t actor=psx_mod_read_word(player+0x288);
    if(!psx_mod_read_word(player+0x2b0) || actor<0x800d0000 || actor>0x801ff000 ||
        (actor&3) || actor==player || (psx_mod_read_word(actor)&2))return false;
    double point[3],direction[3],hit[3];read_vector(actor+0xec,point);
    for(int i=0;i<3;++i)direction[i]=point[i]-eye[i];
    double distance=std::sqrt(direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2]);
    if(distance<1 || distance>16384 || !normal(direction))return false;
    double dot=0;for(int i=0;i<3;++i)dot+=direction[i]*forward[i];
    if(dot<std::cos(6.0*3.141592653589793/180.0))return false;
    int kind=0;uint32_t found=0;
    if(!trace(cpu,eye,point,hit,kind,&found,player,round_test()) || found!=actor)return false;
    if(!trace(cpu,muzzle,point,hit,kind,&found,player,round_test()) || found!=actor)return false;
    std::copy(point,point+3,target);return true;
}
// D07D: ownership is proven at the enqueue alone. The marker routine's only
// caller (EXE and all 30 unique overlays) is 0x80035434, which passes the actor
// in S4; the routine never saves or writes S4, and its frame holds that return
// address at sp+0x40. No entry lease, camera link, map or gameplay-state gate:
// the dot is cosmetic, so any Modernized frame that draws it for Duke hides it,
// including interpreted slices where an entry hook may not fire.
// D10G1 point-blank aim. The muzzle sits well below and beside the screen ray
// (about 300 units below it with the boom compressed), so a crosshair point a
// few hundred units ahead sent rounds up to 70 degrees off the view. Aim at a
// point at the crosshair point's depth, between the muzzle's own view-parallel
// ray and the crosshair point: point_blank_lift of the way up inside
// point_blank_near, rising to the crosshair point itself at point_blank_far.
// Never more than point_blank_max off the view. Both steps are continuous, so
// sweeping the crosshair never visibly switches.
static constexpr double point_blank_near=128,point_blank_far=512;
static constexpr double point_blank_lift_default=0.35,point_blank_max_default=35;
static std::atomic<double> point_blank_lift{-1},point_blank_max{point_blank_max_default};
static double lift_setting() {
    double lift=point_blank_lift.load(std::memory_order_relaxed);
    if(lift>=0)return lift;
    lift=point_blank_lift_default;
    if(const char* v=std::getenv("DNTTK_POINT_BLANK_LIFT")) {char* end;double d=std::strtod(v,&end);if(end!=v && d>=0 && d<=1)lift=d;}
    if(const char* v=std::getenv("DNTTK_POINT_BLANK_MAX")) {char* end;double d=std::strtod(v,&end);if(end!=v && d>=5 && d<=45)point_blank_max.store(d,std::memory_order_relaxed);}
    point_blank_lift.store(lift,std::memory_order_relaxed);
    return lift;
}
// DNTTK_POINT_BLANK=0 keeps the pre-D10G1 point aim (developer A/B).
static bool point_blank_enabled() {const char* v=std::getenv("DNTTK_POINT_BLANK");return !v || std::strcmp(v,"0");}
static bool unit(double* v) {
    double length=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(!std::isfinite(length)||length<1e-6)return false;
    for(int i=0;i<3;++i)v[i]/=length;
    return true;
}
static void point_blank(const double* forward,const double* muzzle,const double* target,double convergence,double* dir) {
    double t=std::clamp((convergence-point_blank_near)/(point_blank_far-point_blank_near),0.0,1.0);
    t=t*t*(3-2*t);
    const double lift=lift_setting(),up=lift+(1-lift)*t;
    double aim[3];
    for(int i=0;i<3;++i) {
        const double level=muzzle[i]+forward[i]*convergence;
        aim[i]=level+(target[i]-level)*up-muzzle[i];
    }
    if(!unit(aim))return;
    double dot=0;for(int i=0;i<3;++i)dot+=aim[i]*forward[i];
    const double limit=point_blank_max.load(std::memory_order_relaxed)*3.141592653589793/180;
    if(dot<std::cos(limit)) {
        double side[3];for(int i=0;i<3;++i)side[i]=aim[i]-forward[i]*dot;
        if(!unit(side))return;
        for(int i=0;i<3;++i)aim[i]=forward[i]*std::cos(limit)+side[i]*std::sin(limit);
    }
    std::copy(aim,aim+3,dir);
}
// D10G3: the camera the shot reads (camera+0x0c/+0x14) is the previous game
// update's; at 120/180 Hz the screen shows the late view: rotation about 34 ms
// newer, eye at the presented world time (about 64 ms of Duke's motion older).
// Aim from the view the last present showed, so the round goes where the
// crosshair is on screen. Fallback to the game camera: no late-view present in
// the last 60 ms (60 Hz, game-owned camera), or one that disagrees (cut).
// DNTTK_SHOT_VIEW=0 or console "shotview off" keeps the game camera.
static std::atomic<int> shot_view_override{-1};
static bool shot_view_enabled() {
    const int o=shot_view_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=std::getenv("DNTTK_SHOT_VIEW");return !v || std::strcmp(v,"0");
}
static unsigned screen_view_shots,screen_view_refused;
static bool last_screen_view;
static double last_view_shift,last_view_turn,last_view_age;
static void screen_view(double* eye,double* forward) {
    last_screen_view=false;last_view_shift=last_view_turn=0;last_view_age=-1;
    double e[3],f[3],age=0;
    if(!shot_view_enabled() || !shot_view(e,f,age))return;
    last_view_age=age;
    double shift=0,dot=0,a=0,b=0;
    for(int i=0;i<3;++i) {shift+=(e[i]-eye[i])*(e[i]-eye[i]);dot+=f[i]*forward[i];a+=f[i]*f[i];b+=forward[i]*forward[i];}
    shift=std::sqrt(shift);
    if(a<1 || b<1)return;
    const double turn=std::acos(std::clamp(dot/std::sqrt(a*b),-1.0,1.0))*180/3.141592653589793;
    // The late view may lead a flick by tens of degrees (its own limit is 100).
    if(age>60 || shift>1536 || turn>100) {++screen_view_refused;return;}
    std::copy(e,e+3,eye);std::copy(f,f+3,forward);
    last_screen_view=true;last_view_shift=shift;last_view_turn=turn;++screen_view_shots;
}
// D10H3 AUTO AIM (DNTTK_AIM_ASSIST): EDuke32's autoaim, A_FindTargetSprite in
// player.cpp. HITSCAN (hitscan, the Modernized default; EDuke32 cl_autoaim 2):
// the bullet guns, Desert Eagle (4), Combat Shotgun (5), Buffalo Rifle (6) and
// Gatling (7, upgraded 28). EDUKE32 (eduke32; cl_autoaim 1): every view-aimed
// weapon except the throws (EDuke32 never aims a pipe bomb): also the Crossbow
// (3), RPG (8, 29), Freezer (11), Flamethrower (9, 27) and Energy Weapon (10,
// whose D22D beam then locks the auto aimed enemy). The nearest enemy in the rooms'
// targetable lists (the D22D beam lock's set, which also holds dancers and
// other civilians, props and Duke's own type): only types with the hostile
// flag 0x10000000 (*0x800D2660 + 28*type, word 0: pig cops, flyers, turrets,
// class 27) and never AI class 8 (+0x15, type byte 6: every civilian, all with
// 0x40000000 and without the hostile flag). EDuke32 likewise aims only at
// A_CheckEnemySprite actors. Inside a horizontal cone of
// AUTO_AIM_ANGLE 48/2048 of a turn either side of the view (about 8.4 degrees),
// nearest by depth along the view, with EDuke32's vertical "on screen" test
// (|160 * slope - horizon| < 100: slopes within 0.625 of the view's), seen from
// the eye and reached from the muzzle (both traces must meet that actor). The
// round then goes exactly at its aim point (+0xec, the original lock's point).
// Apex and cone follow the shown view, with each actor where the shown image
// drew it (D10G4); depth from Duke's body, more than duke_aim_min (EDuke32's
// 512 is under Duke 3D's eye height). The original's spread still applies, as
// EDuke32 adds its spread after autoaim. An enemy under the crosshair is tried
// first, so unlike EDuke32 the player can pick the farther of two; it too gets
// the exact aim point, which lands where D10G4's rewound point on a running
// enemy still misses (D10H2). Console: "autoaim [cone 1..256|default]"
// (2048ths, session only).
static constexpr int duke_aim_cone_default=48;
static constexpr double duke_aim_min=256,duke_aim_range=16384,duke_aim_screen=100.0/160.0;
static std::atomic<int> duke_aim_cone{duke_aim_cone_default};
static unsigned duke_aim_shots,duke_aim_searches,duke_aim_misses;
static uint32_t last_duke_aim_actor;
static double last_duke_aim_angle;
static int last_duke_aim_candidates;
// 0 none, 1 HITSCAN, 2 EDUKE32 (all).
static int duke_aim_mode() {
    const char* v=ttk_live_env("DNTTK_AIM_ASSIST");
    return !v ? 0 : !std::strcmp(v,"hitscan") ? 1 : !std::strcmp(v,"eduke32") ? 2 : 0;
}
// Never an NPC: the type must be hostile and neither it nor the actor class 8.
static bool duke_aim_enemy(uint32_t actor) {
    const uint32_t types=psx_mod_read_word(0x800d2660);
    const unsigned type=psx_mod_read_half(actor+0x2c);
    if(types<0x80010000u || types>0x801f0000u || type>=4096)return false;
    return (psx_mod_read_word(types+28*type)&0x10000000u) && psx_mod_read_byte(types+28*type+6)!=8 && psx_mod_read_byte(actor+0x15)!=8;
}
static bool duke_aim_hitscan(unsigned weapon) {return weapon==4 || weapon==5 || weapon==6 || weapon==7 || weapon==28;}
// D10G4 shoot what you see: at 120/180 Hz the shown image draws actors at
// the presented world time, about 90 ms old, while the screen-ray query tests
// them where they are now. For that one query, move each actor the shown view
// drew back to where it drew it (position +4 and the world joint
// translations at +0x3c that the hit tests read), then restore every word.
// An actor hit aims the round at the same point on that actor now; the
// cover checks and the round itself meet current positions. Only with the
// D10G3 screen view (its eye and world time agree). DNTTK_SHOOT_SEEN=0 or
// console "shootseen off" tests actors where they are now (developer A/B).
static std::atomic<int> shoot_seen_override{-1};
static bool shoot_seen_enabled() {
    const int o=shoot_seen_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=std::getenv("DNTTK_SHOOT_SEEN");return !v || std::strcmp(v,"0");
}
static constexpr int seen_max=24,seen_words=seen_max*(1+48)*3;
struct SeenRewind {
    int n=0,nwords=0;
    uint32_t actor[seen_max];double shift[seen_max][3];
    uint32_t addr[seen_words],old[seen_words];
};
static unsigned seen_shots,seen_rewinds;
static double last_seen_shift;
static int last_seen_actors;
static bool ram_word(uint32_t a,uint32_t n) {return a>=0x80010000u && a+n<=0x80200000u && !(a&3);}
static void seen_rewind(SeenRewind& r) {
    r.n=r.nwords=0;
    uint32_t obj[seen_max];double pos[seen_max][3];
    const int n=shot_view_poses(obj,pos,seen_max);
    const uint32_t held=psx_mod_read_word(player+0x290),climbed=psx_mod_read_word(player+0x17c);
    uint32_t mats_seen[seen_max];int nmats=0;
    for(int i=0;i<n;++i) {
        const uint32_t a=obj[i];
        if(a==player || a==held || a==climbed || !ram_word(a,0x44))continue;
        double now[3],d[3],len=0;read_vector(a+4,now);
        for(int k=0;k<3;++k) {d[k]=std::round(now[k]-pos[i][k]);len+=d[k]*d[k];}
        // Still, or a jump the replay would not blend either (respawn, slot reuse).
        if(len<1 || len>1536.0*1536.0)continue;
        const uint32_t mats=psx_mod_read_word(a+0x3c),model=psx_mod_read_word(a+0x40);
        int joints=0;
        if(ram_word(model,4) && ram_word(mats,0x20)) {
            joints=psx_mod_read_byte(model);
            if(joints<0 || joints>48 || !ram_word(mats,0x20u*joints))joints=0;
            for(int k=0;k<nmats;++k) if(mats_seen[k]==mats)joints=0;   // never shift a shared block twice
            if(joints)mats_seen[nmats++]=mats;
        }
        auto shift=[&](uint32_t at){
            for(int k=0;k<3;++k) {
                r.addr[r.nwords]=at+4*k;r.old[r.nwords]=psx_mod_read_word(at+4*k);
                psx_mod_write_word(at+4*k,(uint32_t)((int32_t)r.old[r.nwords]-(int32_t)d[k]));++r.nwords;
            }
        };
        shift(a+4);
        for(int j=0;j<joints;++j)shift(mats+0x20u*j+0x14);
        r.actor[r.n]=a;std::copy(d,d+3,r.shift[r.n]);++r.n;
    }
}
static void seen_restore(SeenRewind& r) {
    for(int i=r.nwords-1;i>=0;--i)psx_mod_write_word(r.addr[i],r.old[i]);
    r.nwords=0;
}
static bool duke_aim(CPUState* cpu,const double* eye,const double* forward,const double* muzzle,const SeenRewind& seen,uint32_t prefer,double* target) {
    ++duke_aim_searches;last_duke_aim_actor=0;last_duke_aim_angle=0;last_duke_aim_candidates=0;
    const double h=std::hypot(forward[0],forward[2]);
    if(h<1e-6)return false;
    const double fx=forward[0]/h,fz=forward[2]/h,view_slope=forward[1]/h;
    const double cone=duke_aim_cone.load(std::memory_order_relaxed)*3.141592653589793/1024;
    const uint32_t rooms=psx_mod_read_word(0x800da454),count=psx_mod_read_word(0x800da47c);
    if(rooms<0x80010000u || rooms>0x801f0000u || count>256)return false;
    double body[3];read_vector(player+4,body);
    struct Candidate {uint32_t actor;double depth,angle;};
    Candidate found[33];int n=0;
    if(prefer && duke_aim_enemy(prefer))found[n++]={prefer,-1,0};
    for(unsigned room=0;room<count && n<32;++room) {
        uint32_t node=psx_mod_read_word(rooms+0x58*room+0x4c);
        for(unsigned i=0;node && i<64 && n<32;++i,node=psx_mod_read_word(node+4)) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3))break;
            const uint32_t actor=psx_mod_read_word(node+8);
            if(actor<0x80010000u || actor>0x801ffc00u || (actor&3) || actor==player || (psx_mod_read_word(actor)&2))continue;
            bool twice=false;for(int k=0;k<n;++k)twice|=found[k].actor==actor;
            if(twice || !duke_aim_enemy(actor))continue;
            double point[3];read_vector(actor+0xec,point);
            for(int k=0;k<seen.n;++k) if(seen.actor[k]==actor) for(int j=0;j<3;++j)point[j]-=seen.shift[k][j];
            const double vx=point[0]-eye[0],vz=point[2]-eye[2],along=vx*fx+vz*fz;
            const double depth=(point[0]-body[0])*fx+(point[2]-body[2])*fz;
            if(along<=0 || depth<=duke_aim_min || depth>duke_aim_range)continue;
            const double angle=std::atan2(std::abs(vx*fz-vz*fx),along);
            if(angle>cone)continue;
            const double slope=(point[1]-eye[1])/std::hypot(vx,vz);
            if(std::abs(slope-view_slope)>=duke_aim_screen)continue;
            found[n++]={actor,depth,angle};
        }
    }
    last_duke_aim_candidates=n;
    std::sort(found,found+n,[](const Candidate& a,const Candidate& b){return a.depth<b.depth;});   // the crosshair's enemy (-1) first
    // Nearest first; at most four pairs of traces per shot.
    for(int i=0;i<n && i<4;++i) {
        double point[3],hit[3];read_vector(found[i].actor+0xec,point);
        int kind=0;uint32_t met=0;
        if(!trace(cpu,eye,point,hit,kind,&met,player,round_test()) || met!=found[i].actor)continue;
        if(!trace(cpu,muzzle,point,hit,kind,&met,player,round_test()) || met!=found[i].actor)continue;
        std::copy(point,point+3,target);
        last_duke_aim_actor=found[i].actor;last_duke_aim_angle=found[i].angle*180/3.141592653589793;
        return true;
    }
    if(n)++duke_aim_misses;
    return false;
}
static void marker_hook(CPUState* cpu,uint32_t address) {
    if(address!=0x8002bc18 || cpu->gpr[31]!=0x80033db0 || cpu->gpr[20]!=player)return;
    const char* visible=ttk_live_env("DNTTK_RED_DOT");
    if(!visible || std::strcmp(visible,"0") || !input_modernized() || g_ls_mode || g_psx_call_bail || busy)return;
    const uint32_t sp=cpu->gpr[29],primitive=cpu->gpr[5];
    if(sp<0x801f4000 || sp>0x801fffb0 || (sp&3) ||
        psx_mod_read_word(sp+0x40)!=0x8003543c ||
        cpu->gpr[4]!=0 || primitive!=cpu->gpr[16] || primitive<0x80010000 || primitive>0x801fffd8 ||
        (primitive&3) || psx_mod_read_byte(primitive+3)!=9 ||
        (psx_mod_read_byte(primitive+7)&0xfc)!=0x2c || !identity())return;
    // Collapse only this textured quad to zero area before original OT enqueue.
    // No aim fields, selection flags, saved game option or global texture changes.
    for(uint32_t offset:{8u,16u,24u,32u})psx_mod_write_word(primitive+offset,0);
    ++hidden_markers;
}
static bool diagnostics() {const char* v=std::getenv("DNTTK_AIM_TRACE");return v && !std::strcmp(v,"1");}
// D10H2 diagnostics: the actor the last shot met under the crosshair (or the
// nearest moving drawn actor), logged with each flight step of the round.
static uint32_t diag_actor;
static void observe(CPUState* cpu,uint32_t address) {
    if(!diagnostics() || busy || !input_snapshot(Context::Gameplay).active)return;
    uint32_t a=cpu->gpr[4];
    if(address==0x800718ac && a==player) {
        double p[3],d[3];read_vector(cpu->gpr[5],p);read_vector(cpu->gpr[6],d);
        std::fprintf(stderr,"ttk-launch type=%u ra=%08x origin=%.0f,%.0f,%.0f direction=%.0f,%.0f,%.0f\n",cpu->gpr[7],cpu->gpr[31],p[0],p[1],p[2],d[0],d[1],d[2]);
    } else if(address==0x800702bc && a>=0x800da978 && a<0x800dcd78 &&
              (a-0x800da978)%0x60==0 && cpu->gpr[7]==player &&
              cpu->gpr[6]>=0x80010000 && cpu->gpr[6]<=0x801ffff0) {
        double p[3];read_vector(cpu->gpr[6],p);
        std::fprintf(stderr,"ttk-explosion projectile=%08x type=%u ra=%08x point=%.0f,%.0f,%.0f\n",a,psx_mod_read_byte(a+0x30),cpu->gpr[31],p[0],p[1],p[2]);
    } else if(address==0x8006f1b8 && a>=0x800da978 && a<0x800dcd78 && (a-0x800da978)%0x60==0 && psx_mod_read_word(a+0x10)==player) {
        double p[3];read_vector(cpu->gpr[5],p);
        std::fprintf(stderr,"ttk-impact projectile=%08x type=%u ra=%08x point=%.0f,%.0f,%.0f\n",a,psx_mod_read_byte(a+0x30),cpu->gpr[31],p[0],p[1],p[2]);
    } else if(address==0x8006dd2c && a>=0x800da978 && a<0x800dcd78 && (a-0x800da978)%0x60==0 && psx_mod_read_word(a+0x10)==player) {
        double p[3],q[3],d[3];read_vector(a+0x14,p);read_vector(a+4,q);read_vector(a+0x34,d);
        double t[3]={0,0,0};if(ram_word(diag_actor,0x10))read_vector(diag_actor+4,t);
        std::fprintf(stderr,"ttk-flight projectile=%08x type=%u ra=%08x from=%.0f,%.0f,%.0f to=%.0f,%.0f,%.0f direction=%.0f,%.0f,%.0f tgt=%08x,%.0f,%.0f,%.0f test=%04x,%04x\n",a,psx_mod_read_byte(a+0x30),cpu->gpr[31],p[0],p[1],p[2],q[0],q[1],q[2],d[0],d[1],d[2],diag_actor,t[0],t[1],t[2],psx_mod_read_half(a),psx_mod_read_half(a+0x2e));
    }
}
// D22D: the beam updater 0x800733C8 damages only the actor in projectile +0x14,
// through the type's handler table 0x800C5ECC, and only while no wall or other
// actor stands in between; a beam without one never damages. The constructor
// 0x80073220 fills +0x14 from the original acquisition 0x800587C8: actors in
// a room's targetable list (room table *0x800DA454, 0x58 bytes, count
// *0x800DA47C, list head +0x4C, node +4 next / +8 actor), closer than the
// range at 0x800C3B58 (squared) and in a cone round Duke's aim (0x80 base,
// 0x200 with the upgrade's bit 8 = several beams).
static bool targetable(uint32_t actor) {
    if(actor<0x80010000u || actor>0x801ffc00u || (actor&3) || actor==player || (psx_mod_read_word(actor)&2))return false;
    const uint32_t rooms=psx_mod_read_word(0x800da454),count=psx_mod_read_word(0x800da47c);
    const int room=(int8_t)psx_mod_read_byte(actor+0x2e);
    if(rooms<0x80010000u || rooms>0x801f0000u || count>256 || room<0 || unsigned(room)>=count)return false;
    uint32_t node=psx_mod_read_word(rooms+0x58*room+0x4c);
    for(unsigned i=0;node && i<64;++i) {
        if(node<0x80010000u || node>0x801ffff0u || (node&3))return false;
        if(psx_mod_read_word(node+8)==actor)return true;
        node=psx_mod_read_word(node+4);
    }
    return false;
}
static bool beam_reach(uint32_t actor) {
    double a[3],b[3],d=0;read_vector(player+4,a);read_vector(actor+4,b);
    for(int i=0;i<3;++i)d+=(a[i]-b[i])*(a[i]-b[i]);
    return d<(double)(int32_t)psx_mod_read_word(0x800c3b58);
}
static uint32_t beam_target, beam_displaced;
static double beam_eye[3], beam_forward[3];
static unsigned beam_locks, beam_extra_locks;
// The upgrade's extra beams keep the original's other locks within its own
// 45-degree cone, measured round the view instead of Duke's aim vector.
static bool beam_extra_ok(uint32_t actor) {
    if(!targetable(actor) || !beam_reach(actor))return false;
    double d[3];read_vector(actor+0xec,d);
    for(int i=0;i<3;++i)d[i]-=beam_eye[i];
    if(!normal(d))return false;
    double dot=0;for(int i=0;i<3;++i)dot+=d[i]*beam_forward[i];
    return dot>=std::cos(45.0*3.141592653589793/180.0);
}
// D49: at 120/180 Hz the late first-person view turns the world to the newest
// mouse look while the world itself is drawn about 90 ms back, and Duke's arms
// and gun are drawn to match the view (D14H). A beam is a world segment from
// the muzzle at its shot, so it trailed the gun by that delay plus its own
// life while turning or moving. Redraw workers move Duke's own view-aimed
// beams by the view change since their shot (frame_replay.cpp), so they leave
// the gun where it points. Drawing only: the updater, hits and locks keep the
// beam's real line. DNTTK_BEAM_FOLLOW=0 or console "beamfollow off" keeps the
// world beam.
static std::atomic<int> beam_follow_override{-1};
bool aim_beam_follow_enabled() {
    const int o=beam_follow_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=std::getenv("DNTTK_BEAM_FOLLOW");return !v || std::strcmp(v,"0");
}
bool aim_beam_view(uint32_t object,uint32_t& record,double eye[3],double fwd[3]) {
    if(!beam_origins || object<0x800da978u || object>=0x800da978u+beam_slots*0x60u || (object-0x800da978u)%0x60u)return false;
    record=beam_origins+beam_record*((object-0x800da978u)/0x60u);
    if(psx_mod_read_word(object+0x18)!=record || psx_mod_read_word(record+12)!=beam_tag)return false;
    read_vector(record+16,eye);read_vector(record+28,fwd);
    for(int i=0;i<3;++i)fwd[i]/=4096;
    return true;
}
bool beam_follow_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"beamfollow") && std::strncmp(line,"beamfollow ",11))return false;
    close=false;
    if(line[10]) {
        const char* value=line+11;
        if(!std::strcmp(value,"on"))beam_follow_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))beam_follow_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))beam_follow_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: beamfollow [on|off|default] (on: energy beams leave the gun where it points at high refresh; session only)");return true;}
        close=true;
    }
    char text[192];std::snprintf(text,sizeof text,"Beam follow %s: energy beams are drawn %s (session only). Default on.",
        aim_beam_follow_enabled()?"on":"off",aim_beam_follow_enabled()?"from the gun where it points":"where they were fired in the world");
    say(text);return true;
}
// D49C: the beam draw 0x80072868 itself advances the beam's look each time it
// runs (wave phase +0x40, stripe parity +0x58), once per game frame; redraws
// reload the frame's memory, so every redraw between two game frames repeated
// the same look. frame_replay.cpp advances it to the redraw's time instead.
static std::atomic<int> beam_anim_override{-1};
bool aim_beam_anim_enabled() {
    const int o=beam_anim_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=std::getenv("DNTTK_BEAM_ANIM");return !v || std::strcmp(v,"0");
}
bool beam_anim_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"beamanim") && std::strncmp(line,"beamanim ",9))return false;
    close=false;
    if(line[8]) {
        const char* value=line+9;
        if(!std::strcmp(value,"on"))beam_anim_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))beam_anim_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))beam_anim_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: beamanim [on|off|default] (on: energy beams animate at the display rate at high refresh; session only)");return true;}
        close=true;
    }
    char text[192];std::snprintf(text,sizeof text,"Beam animation %s: energy beams animate %s (session only). Default on.",
        aim_beam_anim_enabled()?"on":"off",aim_beam_anim_enabled()?"at the display rate":"once per game frame");
    say(text);return true;
}
// At the beam constructor's verified post-allocation list-advance call, keep
// the original damage/line collision updater. The first beam locks onto the
// targetable actor under the crosshair (D22D) instead of the original
// acquisition, or none (a miss). Each projectile owns a safe physical origin
// for its short visual lifetime.
static bool beam_extra_pending;
static void beam_finish(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const uint32_t object=cpu->gpr[16],index=cpu->gpr[17];
    const bool first=beam_pending && index==0, extra=beam_extra_pending && index>0 && index<3;
    if((!first && !extra) || cpu->gpr[31]!=0x80073380 || cpu->gpr[29]!=beam_shot_sp-0x110 ||
       cpu->gpr[5]!=cpu->gpr[29]+0x10 || cpu->gpr[23]!=player ||
       !f.active || f.epoch!=beam_epoch ||
       !input_modernized() || !view_mode() || busy || g_precise_mode || g_ls_mode || g_psx_call_bail ||
       object<0x800da978 || object>=0x800dcd78 || (object-0x800da978)%0x60 ||
       psx_mod_read_word(object+0x10)!=player || psx_mod_read_byte(object+0x30)!=15 ||
       psx_mod_read_word(object+0x18)!=player+0x80c || !shot_ready() || !identity())return;
    beam_pending=false;
    if(!beam_origins)beam_origins=psx_mod_alloc_guest_memory(beam_slots*beam_record,16);
    if(!beam_origins) {beam_extra_pending=false;return;}
    const auto origin=beam_origins+beam_record*((object-0x800da978)/0x60);
    write_vector(origin,last_origin);
    // D49: the view this beam was aimed from (the D10G3 shown view).
    double fwd[3];for(int i=0;i<3;++i)fwd[i]=beam_forward[i]*4096;
    psx_mod_write_word(origin+12,beam_tag);write_vector(origin+16,beam_eye);write_vector(origin+28,fwd);
    const uint32_t original=psx_mod_read_word(object+0x14),count=cpu->gpr[20];
    uint32_t lock=0;
    if(first) {
        lock=targetable(beam_target) && beam_reach(beam_target)?beam_target:0;
        beam_displaced=original;beam_target=lock;
        // Only the upgrade (several acquired targets) keeps its extra beams.
        beam_extra_pending=count>1;
        if(!beam_extra_pending)cpu->gpr[20]=1;
        ++view_beams;if(lock)++beam_locks;
    } else {
        // An extra lock on the crosshair actor takes the original first lock
        // instead, so no actor is hit twice by one shot.
        lock=original==beam_target?beam_displaced:original;
        if(lock==beam_target || !beam_extra_ok(lock))lock=0;
        if(lock)++beam_extra_locks;
    }
    psx_mod_write_word(object+0x14,lock);psx_mod_write_word(object+0x18,origin);
    if(diagnostics())std::fprintf(stderr,"ttk-view-beam projectile=%08x index=%u count=%u lock=%08x original=%08x origin=%.0f,%.0f,%.0f\n",object,index,count,lock,original,last_origin[0],last_origin[1],last_origin[2]);
}
// D10H2: the round meets the actor where its own update put it. Duke's gun
// call runs in his update; other actors update after it and before the
// projectiles are swept (0x8006dd2c, the same game update), so an enemy that
// updates after Duke has taken one more step (a running pig cop about 60-100
// units) when the round is first swept. When a view-aimed round was aimed at
// an actor (the crosshair's, the D10H3 auto aimed or the original lock), its
// first sweep shifts the round's end point by that actor's step since the gun
// call, scaled along the round's line, so the round goes through the same
// point on the actor that the crosshair met. Speed, spread round the line,
// damage and every later sweep are the original's. Only that update, only a
// step of 1-512 units, only rounds that start at this shot's origin.
// DNTTK_ROUND_FOLLOW=0 or console "roundfollow off" keeps the plain round.
static std::atomic<int> round_follow_override{-1};
static bool round_follow_enabled() {
    const int o=round_follow_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=std::getenv("DNTTK_ROUND_FOLLOW");return !v || std::strcmp(v,"0");
}
struct RoundFollow {bool armed=false;uint32_t actor=0;int32_t origin[3]={0,0,0};double pos[3]={0,0,0},target[3]={0,0,0};};
static RoundFollow follow;
static unsigned follow_arms,follow_shifts;
static double last_follow_shift;
static void follow_arm(uint32_t actor,const double* origin,const double* target) {
    follow.armed=false;
    if(!round_follow_enabled() || !ram_word(actor,0x100) || actor==player || (psx_mod_read_word(actor)&2))return;
    follow.actor=actor;read_vector(actor+4,follow.pos);std::copy(target,target+3,follow.target);
    for(int i=0;i<3;++i)follow.origin[i]=(int32_t)std::lround(origin[i]);
    follow.armed=true;++follow_arms;
}
static void follow_flight(CPUState* cpu) {
    const uint32_t a=cpu->gpr[4];
    if(!follow.armed || a<0x800da978 || a>=0x800dcd78 || (a-0x800da978)%0x60 || psx_mod_read_word(a+0x10)!=player)return;
    for(int i=0;i<3;++i) if((int32_t)psx_mod_read_word(a+0x14+4*i)!=follow.origin[i])return;   // not this shot's round, or not its first sweep
    const uint32_t actor=follow.actor;
    if(!ram_word(actor,0x100) || (psx_mod_read_word(actor)&2))return;
    double now[3],d[3],step=0;read_vector(actor+4,now);
    for(int i=0;i<3;++i) {d[i]=now[i]-follow.pos[i];step+=d[i]*d[i];}
    step=std::sqrt(step);
    if(step<1 || step>512)return;
    double from[3],to[3],dir[3],reach=0,span=0,length=0;
    read_vector(a+0x14,from);read_vector(a+4,to);read_vector(a+0x34,dir);
    for(int i=0;i<3;++i) {reach+=(follow.target[i]-from[i])*(follow.target[i]-from[i]);span+=(to[i]-from[i])*(to[i]-from[i]);length+=dir[i]*dir[i];}
    reach=std::sqrt(reach);span=std::sqrt(span);length=std::sqrt(length);
    if(reach<64 || span<1 || length<1)return;
    double u[3];for(int i=0;i<3;++i)u[i]=to[i]+d[i]*span/reach-from[i];
    if(!unit(u))return;
    double end[3],v[3];for(int i=0;i<3;++i) {end[i]=from[i]+u[i]*span;v[i]=u[i]*length;}
    write_vector(a+4,end);write_vector(a+0x34,v);
    ++follow_shifts;last_follow_shift=step;
    if(diagnostics())std::fprintf(stderr,"ttk-round-follow projectile=%08x actor=%08x step=%.0f,%.0f,%.0f\n",a,actor,d[0],d[1],d[2]);
}
// D49F: in first person the Laser Gatling (record 28) launches its bolts
// from the drawn barrel. The gun is shown in its own presentation pose
// (D12/D36), so the third-person skeleton's muzzle the round started at
// (player +0xbc) sat 400-800 units under and beside the drawn barrel, and the
// bolt's line ran from there to the crosshair point. D49E draws the bolt's
// first tick from the drawn barrel to its first position, so the shot left
// the barrel along one line and then flew along another: a bend about one
// tick (2000 units) out. Now the round itself starts at the drawn barrel tip
// (Duke's draw records it in the view frame, D49 first_person_drawn_muzzle,
// the muzzle flash's point), placed in the world under the view on screen
// (D10G3, or the game camera at 60 Hz), and aims at the crosshair point as
// before, so it flies one straight line from the barrel to where the
// crosshair is. Gameplay: the hit still lands at the crosshair point; only
// the line toward it starts elsewhere, so cover between the barrel and the
// target now decides what the bolt meets, as it is drawn. The usual wall
// guard (body to origin) keeps a barrel pushed through a wall on Duke's
// side, and an eye-to-barrel check keeps it out of floors and ceilings when
// looking steeply. Modernized, first person, the Laser Gatling only (the
// other guns' rounds are not drawn; rockets and fireballs keep the accepted
// D49B/D10H8 feel). DNTTK_BOLT_LAUNCH=0 or console "boltlaunch off" keeps the
// skeleton's muzzle (A/B).
static std::atomic<int> bolt_launch_override{-1};
static bool bolt_launch_enabled() {
    const int o=bolt_launch_override.load(std::memory_order_relaxed);
    if(o>=0)return o!=0;
    const char* v=ttk_live_env("DNTTK_BOLT_LAUNCH");return !(v && v[0]=='0' && !v[1]);
}
bool aim_bolt_launch_enabled() {return bolt_launch_enabled();}
static bool barrel_launch_record(unsigned weapon) {return weapon==28;}
static unsigned barrel_launches,barrel_refused,barrel_guarded,barrel_steps;
static double last_barrel_offset,last_barrel_step;
// The drawn barrel tip in the world: the eye and view rows the shot aims from
// (the shown view, or the game camera when the shot uses it).
static bool drawn_barrel(const double* eye,bool screen,double* out) {
    double vm[3],rows[9];
    if(!first_person_drawn_muzzle(vm))return false;
    if(screen) {if(!shot_view_rows(rows))return false;}
    else for(int r=0;r<3;++r) {
        double row[3]={0,0,0};
        for(int k=0;k<3;++k)row[k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k));
        if(!normal(row))return false;
        for(int k=0;k<3;++k)rows[3*r+k]=row[k];
    }
    // In front of the eye and within reach of the held gun (its presentation
    // anchor sits past the near cull, about 1050 units ahead).
    if(vm[2]<16 || vm[0]*vm[0]+vm[1]*vm[1]+vm[2]*vm[2]>2048.0*2048.0)return false;
    for(int k=0;k<3;++k)out[k]=eye[k]+rows[k]*vm[0]+rows[3+k]*vm[1]+rows[6+k]*vm[2];
    return true;
}
bool bolt_launch_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"boltlaunch") && std::strncmp(line,"boltlaunch ",11))return false;
    close=false;
    if(line[10]) {
        const char* value=line+11;
        if(!std::strcmp(value,"on"))bolt_launch_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))bolt_launch_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))bolt_launch_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: boltlaunch [on|off|default] (on: first-person Laser Gatling bolts fly from the drawn barrel; session only)");return true;}
        close=true;
    }
    char text[192];std::snprintf(text,sizeof text,"Bolt launch %s: first-person Laser Gatling bolts fly from %s (session only). Default on.",
        bolt_launch_enabled()?"on":"off",bolt_launch_enabled()?"the drawn barrel":"Duke's skeleton muzzle");
    say(text);return true;
}
static void aim_hook_body(CPUState* cpu,uint32_t address) {
    if(address==0x8001ca4c) {beam_finish(cpu);return;}
    if(address==0x8006dd2c && input_modernized() && !g_ls_mode && !g_psx_call_bail)follow_flight(cpu);
    marker_hook(cpu,address);
    if(input_modernized())observe(cpu,address);
    // D10H4: every player gun call, whatever the aim mode.
    if(address==0x8003c500 && cpu->gpr[4]==player && (cpu->gpr[31]==0x8004f7b4 || cpu->gpr[31]==0x8004fa88))weapon_feel_shot();
    if(g_precise_mode || g_ls_mode || g_psx_call_bail || busy || !input_modernized() || !view_mode() || input_snapshot(Context::Gameplay).held[original_aim])return;
    if(address!=0x8003c500 || cpu->gpr[4]!=player || !supported(cpu->gpr[7]))return;
    if(!shot_ready()) {
        // D07E: a supported player shot left to the original aim.
        ++lease_misses;std::snprintf(last_lease_miss,sizeof last_lease_miss,"%s",movement_lease_miss());
        if(diagnostics())std::fprintf(stderr,"ttk-aim-lease-miss weapon=%u ra=%08x why=%s\n",cpu->gpr[7],cpu->gpr[31],last_lease_miss);
        return;
    }
    if(!identity())return;
    uint32_t sp=cpu->gpr[29],origin=cpu->gpr[5],ra=cpu->gpr[31];
    if(sp<0x801f4000 || sp>0x801fff00)return;
    const bool gun=(ra==0x8004f7b4 || ra==0x8004fa88) && !thrown_weapon(cpu->gpr[7]) && cpu->gpr[7]!=9 && cpu->gpr[7]!=10 && cpu->gpr[7]!=27 &&
        origin==sp+0x40 && cpu->gpr[6]==player+0x144;
    // Charged throwing handler: direction is a stack copy, speed/fuse remain
    // original stack arguments. Never accept a guessed caller or pointer.
    const bool thrown=ra==0x8004e7e4 && thrown_weapon(cpu->gpr[7]) &&
        origin==sp+0x30 && cpu->gpr[6]==sp+0x20;
    const bool continuous=(ra==0x8003cac8 || ra==0x80058b80) &&
        (cpu->gpr[7]==9 || cpu->gpr[7]==10 || cpu->gpr[7]==27) && origin==player+0x80c && cpu->gpr[6]==player+0x144;
    if(!gun && !thrown && !continuous)return;
    beam_pending=beam_extra_pending=false;
    busy=true;
    double eye[3],forward[3],target[3],muzzle[3],body[3],safe[3],ray_start[3];
    read_vector(camera+0x14,eye);read_vector(continuous?player+0x80c:player+0xbc,muzzle);
    for(int i=0;i<3;++i)forward[i]=(int16_t)psx_mod_read_half(camera+12+2*i);
    screen_view(eye,forward);
    const bool free_look=free_look_aim(eye,forward);
    if(free_look)last_screen_view=false;
    bool ok=normal(forward);int kind=0;
    for(int i=0;i<3;++i)target[i]=eye[i]+forward[i]*16384;
    // D49F: the drawn barrel, before the cover guard and the aim below.
    bool barrel=false;
    double native_muzzle[3];std::copy(muzzle,muzzle+3,native_muzzle);
    if(ok && gun && !free_look && barrel_launch_record(cpu->gpr[7]) && bolt_launch_enabled() && first_person_view_live()) {
        double q[3],d=0;
        if(drawn_barrel(eye,last_screen_view,q)) {
            for(int i=0;i<3;++i)d+=(q[i]-muzzle[i])*(q[i]-muzzle[i]);
            d=std::sqrt(d);
            if(d<2048) {std::copy(q,q+3,muzzle);barrel=true;last_barrel_offset=d;last_barrel_step=0;}
        }
        if(!barrel)++barrel_refused;
    }
    read_vector(player+4,body);body[1]=muzzle[1];
    // The screen ray begins at Duke's depth. Geometry behind the character
    // (between the follow camera and Duke) must not turn a shot backwards.
    // The physical projectile and body-to-muzzle check still enforce all cover.
    double depth=0;for(int i=0;i<3;++i)depth+=(body[i]-eye[i])*forward[i];
    for(int i=0;i<3;++i)ray_start[i]=eye[i]+forward[i]*std::clamp(depth,0.0,16000.0);
    uint32_t view_actor=0;
    static SeenRewind seen;
    seen.n=0;last_seen_shift=0;last_seen_actors=0;
    if(ok && last_screen_view && shoot_seen_enabled())seen_rewind(seen);
    ok=ok && trace(cpu,ray_start,target,target,kind,&view_actor,player,round_test());
    seen_restore(seen);last_seen_actors=seen.n;
    // Diagnostics: what the same ray meets with every actor where it is now.
    int now_kind=-1;uint32_t now_actor=0;
    // The moving actor nearest the ray: its aim point's miss distance where drawn and now.
    double near_drawn=-1,near_now=-1,near_moved=0;uint32_t near_actor=0;
    if(seen.n && diagnostics()) {
        double hit[3],to[3];for(int k=0;k<3;++k)to[k]=eye[k]+forward[k]*16384;if(!trace(cpu,ray_start,to,hit,now_kind,&now_actor,player,round_test()))now_kind=-1;
        for(int i=0;i<seen.n;++i) {
            double moved=0;for(int k=0;k<3;++k)moved+=seen.shift[i][k]*seen.shift[i][k];
            if(moved<64*64 || (psx_mod_read_word(seen.actor[i])&2))continue;   // dead bodies are not shootable
            // Ground-plane miss of the position (+4), the field the hit tests start from.
            double now[3],was[3];read_vector(seen.actor[i]+4,now);for(int k=0;k<3;++k)was[k]=now[k]-seen.shift[i][k];
            auto miss=[&](const double* q){const double vx=q[0]-eye[0],vz=q[2]-eye[2],h=std::hypot(forward[0],forward[2]);
                if(h<1e-6)return 1e9;const double fx=forward[0]/h,fz=forward[2]/h,t=vx*fx+vz*fz;return t>0?std::abs(vx*fz-vz*fx):1e9;};
            const double d=miss(was);
            if(near_drawn<0 || d<near_drawn) {near_drawn=d;near_now=miss(now);near_moved=std::sqrt(moved);near_actor=seen.actor[i];}
        }
    }
    if(seen.n)++seen_rewinds;
    if(ok && kind==2) for(int i=0;i<seen.n;++i) if(seen.actor[i]==view_actor) {
        // The same point on the actor where it stands now.
        for(int k=0;k<3;++k)target[k]+=seen.shift[i][k];
        last_seen_shift=std::sqrt(seen.shift[i][0]*seen.shift[i][0]+seen.shift[i][1]*seen.shift[i][1]+seen.shift[i][2]*seen.shift[i][2]);
        ++seen_shots;break;
    }
    // Keep the physical origin. Guard a muzzle protruding through thin cover
    // with a body-to-muzzle segment before committing a new direction.
    int cover=0;
    if(ok && barrel) {
        // Looking steeply down or up, the barrel can sit past a floor or
        // ceiling the level body-to-muzzle segment never meets.
        int floor_kind=0;double hit[3];
        if(trace(cpu,eye,muzzle,hit,floor_kind) && floor_kind==1) {
            double back[3];for(int i=0;i<3;++i)back[i]=eye[i]-hit[i];
            if(normal(back)) {for(int i=0;i<3;++i)muzzle[i]=hit[i]+back[i]*8;body[1]=muzzle[1];++barrel_guarded;}
            else {std::copy(native_muzzle,native_muzzle+3,muzzle);body[1]=muzzle[1];barrel=false;}
        }
    }
    ok=ok && trace(cpu,body,muzzle,safe,cover);
    if(barrel && cover)++barrel_guarded;
    if(ok && cover==2) {
        // An intersecting actor is not a wall. Start on Duke's side of the
        // entire overlap, so the original sweep enters the enemy from outside.
        std::copy(body,body+3,muzzle);++close_retractions;
    } else if(ok && cover) {
        double back[3];for(int i=0;i<3;++i)back[i]=body[i]-safe[i];
        if(normal(back))for(int i=0;i<3;++i)muzzle[i]=safe[i]+back[i]*8;
        else ok=false;
    }
    last_assisted=ok && !cover && assist(cpu,eye,muzzle,forward,target);
    bool duke_aimed=false;
    const int duke_mode=duke_aim_mode();
    if(ok && !cover && duke_mode && (duke_mode==2 ? gun || continuous : gun && duke_aim_hitscan(cpu->gpr[7])))
        duke_aimed=last_assisted=duke_aim(cpu,eye,forward,muzzle,seen,kind==2 && targetable(view_actor)?view_actor:0,target);
    if(duke_aimed)++duke_aim_shots;
    double convergence=0;for(int i=0;i<3;++i)convergence+=(target[i]-muzzle[i])*forward[i];
    if(ok && convergence<64) {
        // A close screen-ray hit can be behind a long/offset animated muzzle.
        // Retract, never advance through the target or manufacture damage.
        std::copy(body,body+3,muzzle);++close_retractions;
        double remaining=0;for(int i=0;i<3;++i)remaining+=(target[i]-muzzle[i])*forward[i];
        if(remaining<64)for(int i=0;i<3;++i)target[i]+=forward[i]*(64-remaining);
    }
    if(ok && barrel && cover!=2 && convergence>=64) {
        // D49F: the barrel sits about 1050 units ahead of the eye and to the
        // side, so a close target, or a floor ahead when looking down, puts
        // its line to the crosshair point far off the view: a bolt leaving
        // the gun sideways, and from just above a floor a grazing line whose
        // impact lands 30-90 units past the point. Step the origin back
        // toward the eye (which reaches the point) until the line is within
        // barrel_max_angle of the view and meets no wall short of the point.
        // Such shots arrive within one game tick, where D49E still draws the
        // bolt from the drawn barrel. Actors in the way end the check: the
        // bolt meets them, as drawn.
        static constexpr double barrel_max_angle=20;
        auto reaches=[&](const double* from) {
            double u[3];for(int i=0;i<3;++i)u[i]=target[i]-from[i];
            if(!normal(u))return true;
            double c=0;for(int i=0;i<3;++i)c+=u[i]*forward[i];
            if(c<std::cos(barrel_max_angle*3.141592653589793/180))return false;
            double hit[3];int k=0;
            // A miss into open sky ends outside every room (room -1), which
            // the query reports as not ok: nothing stands in the way.
            if(!trace(cpu,from,target,hit,k))return k==0;
            double d=0;for(int i=0;i<3;++i)d+=(hit[i]-target[i])*(hit[i]-target[i]);
            return k!=1 || d<=24.0*24.0;
        };
        if(!reaches(muzzle)) {
            const double from[3]={muzzle[0],muzzle[1],muzzle[2]};
            for(double t:{0.25,0.5,0.75,1.0}) {
                double q[3];for(int i=0;i<3;++i)q[i]=from[i]+(eye[i]-from[i])*t;
                if(t==1.0 || reaches(q)) {std::copy(q,q+3,muzzle);last_barrel_step=t;break;}
            }
            ++barrel_steps;
        }
    }
    double dir[3];for(int i=0;i<3;++i)dir[i]=target[i]-muzzle[i];
    ok=ok && normal(dir);
    // D10G1: a point close ahead. A targetable actor under the crosshair
    // (an enemy) or an assisted lock keeps the exact point, so the round meets
    // it; walls and untargetable props bend. Thrown weapons keep their
    // original lob; first person (the eye near the gun) keeps the point aim.
    last_bend=0;
    if(ok && !thrown && !last_assisted && !(kind==2 && targetable(view_actor)) && !first_person_view_live() && point_blank_enabled()) {
        double before[3];std::copy(dir,dir+3,before);
        double remaining=0;for(int i=0;i<3;++i)remaining+=(target[i]-muzzle[i])*forward[i];
        point_blank(forward,muzzle,target,remaining,dir);
        double dot=0;for(int i=0;i<3;++i)dot+=before[i]*dir[i];
        last_bend=std::acos(std::clamp(dot,-1.0,1.0))*180/3.141592653589793;
        if(last_bend>0.05)++point_blank_shots;else last_bend=0;
    }
    if(ok) {
        // Host enhancement argument storage; never alias the caller matrix,
        // persistent auto-aim state or actor heading.
        if(!aim_vector) aim_vector=psx_mod_alloc_guest_memory(16,16);
        if(!aim_vector) {++rejected;busy=false;return;}
        uint32_t vector=aim_vector;
        for(int i=0;i<3;++i) {last_origin[i]=muzzle[i];last_target[i]=target[i];last_direction[i]=dir[i]*4096;}
        if(continuous) {
            if(!aim_origin)aim_origin=psx_mod_alloc_guest_memory(16,16);
            if(!aim_origin) {++rejected;busy=false;return;}
            origin=aim_origin;cpu->gpr[5]=origin;
        }
        write_vector(origin,muzzle);write_vector(vector,last_direction);cpu->gpr[6]=vector;
        if(continuous && cpu->gpr[7]==10) {
            const auto& f=input_snapshot(Context::Gameplay);
            beam_shot_sp=sp;beam_epoch=f.epoch;beam_pending=true;
            // D22D: the beam's lock is the actor under the crosshair, or the
            // assisted original lock once both of its cover checks passed;
            // D10H3 EDUKE32: the auto aimed enemy.
            beam_target=duke_aimed?last_duke_aim_actor:last_assisted?psx_mod_read_word(player+0x288):view_actor;
            std::copy(eye,eye+3,beam_eye);std::copy(forward,forward+3,beam_forward);
        }
        if(barrel)++barrel_launches;
        if(barrel && diagnostics())std::fprintf(stderr,"ttk-barrel-launch weapon=%u origin=%.0f,%.0f,%.0f native=%.0f,%.0f,%.0f offset=%.0f cover=%d step=%.2f view=%s\n",cpu->gpr[7],muzzle[0],muzzle[1],muzzle[2],native_muzzle[0],native_muzzle[1],native_muzzle[2],last_barrel_offset,cover,last_barrel_step,last_screen_view?"screen":"game");
        if(gun)follow_arm(duke_aimed?last_duke_aim_actor:last_assisted?psx_mod_read_word(player+0x288):kind==2?view_actor:0,muzzle,target);
        ++shots;if(last_assisted)++assisted_shots;last_kind=cover?4:(last_assisted?2:kind);last_weapon=cpu->gpr[7];
        int check_kind=-1;uint32_t check_actor=0;double tpos[3]={0,0,0};
        if(diagnostics()) {
            // D10H2: what the round's own line and test (muzzle along the final
            // direction) meets with every actor where it is now.
            double to[3],hit[3];for(int k=0;k<3;++k)to[k]=muzzle[k]+dir[k]*16384;
            if(!trace(cpu,muzzle,to,hit,check_kind,&check_actor,player,3))check_kind=-1;
            diag_actor=kind==2?view_actor:near_actor;
            if(ram_word(diag_actor,0x10))read_vector(diag_actor+4,tpos);
            std::fprintf(stderr,"ttk-aim-check actor=%08x pos=%.0f,%.0f,%.0f check=%d,%08x origin=%.0f,%.0f,%.0f\n",diag_actor,tpos[0],tpos[1],tpos[2],check_kind,check_kind==2?check_actor:0,muzzle[0],muzzle[1],muzzle[2]);
        }
        if(diagnostics())std::fprintf(stderr,"ttk-aim-shot weapon=%d target=%.0f,%.0f,%.0f direction=%.0f,%.0f,%.0f cover=%d bend=%.1f view=%s eye=%.0f,%.0f,%.0f forward=%.4f,%.4f,%.4f shift=%.0f turn=%.2f age=%.1f seen=%d actor=%08x moved=%.0f now=%d,%08x near=%08x,%.0f,%.0f,%.0f autoaim=%08x,%.2f,%d\n",last_weapon,target[0],target[1],target[2],last_direction[0],last_direction[1],last_direction[2],cover,last_bend,
            last_screen_view?"screen":"game",eye[0],eye[1],eye[2],forward[0],forward[1],forward[2],last_view_shift,last_view_turn,last_view_age,last_seen_actors,kind==2?view_actor:0,last_seen_shift,now_kind,now_kind==2?now_actor:0,near_actor,near_drawn,near_now,near_moved,duke_aimed?last_duke_aim_actor:0,last_duke_aim_angle,last_duke_aim_candidates);
    } else ++rejected;
    busy=false;
}
static void hook(CPUState* cpu,uint32_t address) {
    if(!frame_trace_on()) {aim_hook_body(cpu,address);return;}
    const auto t0=std::chrono::steady_clock::now();aim_hook_body(cpu,address);
    frame_trace_account(address|0x1u,(long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count());
}
// D10G1 tuning: "pointblank [lift [max]|default]", session only. lift: the
// fraction of the way from the muzzle's level up to the crosshair point that
// a close round meets the surface; max: degrees off the view (5-45).
bool point_blank_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"pointblank") && std::strncmp(line,"pointblank ",11))return false;
    close=false;
    if(line[10]) {
        const char* value=line+11;double lift=point_blank_lift_default,max=point_blank_max_default;
        int used=0;const int n=std::sscanf(value,"%lf %n",&lift,&used);
        if(n==1 && value[used])std::sscanf(value+used,"%lf",&max);
        else if(n!=1 && std::strcmp(value,"default"))lift=-1;
        if(lift<0 || lift>1 || max<5 || max>45) {say("Usage: pointblank [lift 0..1 [max 5..45]|default] (0 muzzle level, 1 crosshair; session only)");return true;}
        point_blank_lift.store(lift,std::memory_order_relaxed);point_blank_max.store(max,std::memory_order_relaxed);close=true;
    }
    char text[192];std::snprintf(text,sizeof text,
        "Point-blank lift %.2f, max %.0f degrees (0 muzzle level, 1 crosshair point; session only). Default %.2f, %.0f.",
        lift_setting(),point_blank_max.load(std::memory_order_relaxed),point_blank_lift_default,point_blank_max_default);
    say(text);return true;
}
bool shot_view_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"shotview") && std::strncmp(line,"shotview ",9))return false;
    close=false;
    if(line[8]) {
        const char* value=line+9;
        if(!std::strcmp(value,"on"))shot_view_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))shot_view_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))shot_view_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: shotview [on|off|default] (on: shots aim from the view on screen; session only)");return true;}
        close=true;
    }
    char text[192];std::snprintf(text,sizeof text,"Shot view %s: shots aim from the %s (session only). Default on.",
        shot_view_enabled()?"on":"off",shot_view_enabled()?"view on screen":"previous game update's camera");
    say(text);return true;
}
bool shoot_seen_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"shootseen") && std::strncmp(line,"shootseen ",10))return false;
    close=false;
    if(line[9]) {
        const char* value=line+10;
        if(!std::strcmp(value,"on"))shoot_seen_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))shoot_seen_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))shoot_seen_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: shootseen [on|off|default] (on: the crosshair hits enemies where they are drawn; session only)");return true;}
        close=true;
    }
    char text[192];std::snprintf(text,sizeof text,"Shoot what you see %s: the crosshair tests enemies %s (session only). Default on.",
        shoot_seen_enabled()?"on":"off",shoot_seen_enabled()?"where they are drawn":"where they are now");
    say(text);return true;
}
bool round_follow_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"roundfollow") && std::strncmp(line,"roundfollow ",12))return false;
    close=false;
    if(line[11]) {
        const char* value=line+12;
        if(!std::strcmp(value,"on"))round_follow_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off"))round_follow_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default"))round_follow_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: roundfollow [on|off|default] (on: a round aimed at an enemy meets it after the enemy's own update; session only)");return true;}
        close=true;
    }
    char text[224];std::snprintf(text,sizeof text,"Round follow %s: a round aimed at an enemy %s (session only). Default on. Rounds shifted %u.",
        round_follow_enabled()?"on":"off",round_follow_enabled()?"meets the point the crosshair met after the enemy's own update":"flies its line unchanged",follow_shifts);
    say(text);return true;
}
bool auto_aim_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"autoaim") && std::strncmp(line,"autoaim ",8))return false;
    close=false;
    if(line[7]) {
        const char* value=line+8;int cone=0;char tail=0;
        if(!std::strcmp(value,"default"))cone=duke_aim_cone_default;
        else if(std::sscanf(value,"%d%c",&cone,&tail)!=1 || cone<1 || cone>256) {say("Usage: autoaim [cone 1..256|default] (2048ths of a turn either side; EDuke32 48; session only)");return true;}
        duke_aim_cone.store(cone,std::memory_order_relaxed);close=true;
    }
    const int cone=duke_aim_cone.load(std::memory_order_relaxed);
    char text[224];std::snprintf(text,sizeof text,"Auto aim %s: EDuke32 cone %d/2048 either side (%.1f degrees; session only). Default %d (EDuke32). Shots aimed %u.",
        duke_aim_mode()==2?"EDUKE32 (every weapon but throws)":duke_aim_mode()?"HITSCAN (bullet guns)":"off or original (OPTIONS > ADVANCED > COMBAT)",cone,cone*360.0/2048,duke_aim_cone_default,duke_aim_shots);
    say(text);return true;
}
// D10H2 lab fixture (diagnostic only; console "testmove", session only): move
// one actor sideways across the view on a triangle wave, one step per game
// update, on the game thread at Duke's animation runner entry. Unlike debug
// port RAM writes (one byte per request, applied at interrupt safe points),
// its position never changes between the shot's query and the round's sweep
// of the same update, the same as an enemy that moves in its own update. Its
// health is kept at 30000; every drop is logged (ttk-testmove-damage). Span 0
// leaves the position to the game (health and damage log only).
static uint32_t move_actor;
static double move_base[3],move_side[2],move_speed,move_span;
static unsigned move_tick;
static int move_hp;
static bool move_late,move_pending;
// Every update and every sweep of Duke's rounds: log a health drop, refill.
static bool move_watch() {
    const uint32_t a=move_actor;
    if(!ram_word(a,0x100) || (psx_mod_read_word(a)&2)) {move_actor=0;std::fprintf(stderr,"ttk-testmove stop actor=%08x\n",a);return false;}
    const int hp=(int16_t)psx_mod_read_half(a+0x32);
    if(hp<move_hp)std::fprintf(stderr,"ttk-testmove-damage actor=%08x hp=%d->%d tick=%u\n",a,move_hp,hp,move_tick);
    move_hp=hp;
    if(hp<20000) {psx_mod_write_half(a+0x32,30000);move_hp=30000;}
    return true;
}
static void move_step() {
    const uint32_t a=move_actor;
    double o=0;
    if(move_speed>0 && move_span>0) {
        const double ph=std::fmod(move_tick*move_speed/30.0,4*move_span);
        o=ph<2*move_span?ph-move_span:3*move_span-ph;
    }
    ++move_tick;
    if(move_span<=0)return;   // span 0: health and damage log only, the game moves it
    psx_mod_write_word(a+4,(uint32_t)(int32_t)std::lround(move_base[0]+move_side[0]*o));
    psx_mod_write_word(a+12,(uint32_t)(int32_t)std::lround(move_base[2]+move_side[1]*o));
}
// "late": the step is taken at the first sweep of one of Duke's rounds after
// his update, as an enemy that updates after Duke (else at the next update).
static void move_hook(CPUState* cpu,uint32_t address) {
    if(g_ls_mode || g_psx_call_bail)return;
    if(address==0x8005a210 && cpu->gpr[4]==player) {
        follow.armed=false;   // D10H2: a shot's rounds are first swept in its own update
        if(!move_actor || !move_watch())return;
        if(!move_late)move_step();
        else {if(move_pending)move_step();move_pending=true;}
    } else if(address==0x8006dd2c && move_actor) {
        const uint32_t a=cpu->gpr[4];
        if(a<0x800da978 || a>=0x800dcd78 || (a-0x800da978)%0x60 || psx_mod_read_word(a+0x10)!=player || !move_watch())return;
        if(move_late && move_pending) {move_pending=false;move_step();}
    }
}
bool test_move_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"testmove") && std::strncmp(line,"testmove ",9))return false;
    close=false;unsigned actor=0;double speed=0,span=700;char text[192],late[8]="";
    if(!std::strcmp(line,"testmove off")) {move_actor=0;say("Test mover off.");return true;}
    if(std::sscanf(line+8," %x %lf %lf %7s",&actor,&speed,&span,late)<2 || (late[0] && std::strcmp(late,"late")) || !ram_word(actor,0x100) || speed<0 || speed>8000 || span<0 || span>4000) {
        say("Usage: testmove <actor hex> <units/s> [span [late]] | testmove off (lab fixture: slide an actor across the view, session only)");return true;
    }
    const double fx=(int16_t)psx_mod_read_half(camera+12),fz=(int16_t)psx_mod_read_half(camera+16),h=std::hypot(fx,fz);
    if(h<1) {say("No camera.");return true;}
    read_vector(actor+4,move_base);move_side[0]=fz/h;move_side[1]=-fx/h;
    move_speed=speed;move_span=span;move_tick=0;move_late=late[0]!=0;move_pending=false;move_hp=(int16_t)psx_mod_read_half(actor+0x32);move_actor=actor;
    std::snprintf(text,sizeof text,"Test mover: actor %08x, %.0f u/s, span %.0f%s.",actor,speed,span,move_late?", late":"");say(text);close=true;return true;
}
const char* aim_debug_json() {
    static char out[3072];
    std::snprintf(out,sizeof out,"{\"bolt_launch\":%s,\"barrel_launches\":%u,\"barrel_refused\":%u,\"barrel_guarded\":%u,\"barrel_steps\":%u,\"last_barrel_offset\":%.0f,\"round_test\":%u,\"round_follow\":%s,\"follow_arms\":%u,\"follow_shifts\":%u,\"last_follow_shift\":%.0f,\"auto_aim\":\"%s\",\"duke_aim_cone\":%d,\"duke_aim_shots\":%u,\"duke_aim_searches\":%u,\"duke_aim_misses\":%u,\"last_duke_aim_actor\":\"%08x\",\"last_duke_aim_angle\":%.2f,\"last_duke_aim_candidates\":%d,\"shoot_seen\":%s,\"seen_shots\":%u,\"seen_rewinds\":%u,\"last_seen_actors\":%d,\"last_seen_shift\":%.0f,\"shot_view\":%s,\"screen_view_shots\":%u,\"screen_view_refused\":%u,\"last_screen_view\":%s,\"last_view_shift\":%.0f,\"last_view_turn\":%.2f,\"last_view_age\":%.1f,\"point_blank_shots\":%u,\"point_blank_lift\":%.2f,\"last_bend\":%.1f,\"beam_locks\":%u,\"beam_extra_locks\":%u,\"close_retractions\":%u,\"assisted_shots\":%u,\"hidden_markers\":%u,\"last_assisted\":%s,\"reticle\":%s,\"shots\":%u,\"rejected\":%u,\"queries\":%u,\"lease_misses\":%u,\"last_lease_miss\":\"%s\",\"identity_calls\":%llu,\"identity_checks\":%llu,\"weapon\":%d,\"hit_kind\":%d,\"origin\":[%.0f,%.0f,%.0f],\"target\":[%.0f,%.0f,%.0f],\"direction\":[%.0f,%.0f,%.0f]}",bolt_launch_enabled()?"true":"false",barrel_launches,barrel_refused,barrel_guarded,barrel_steps,last_barrel_offset,round_test(),round_follow_enabled()?"true":"false",follow_arms,follow_shifts,last_follow_shift,ttk_live_env("DNTTK_AIM_ASSIST")?ttk_live_env("DNTTK_AIM_ASSIST"):"off",duke_aim_cone.load(std::memory_order_relaxed),duke_aim_shots,duke_aim_searches,duke_aim_misses,last_duke_aim_actor,last_duke_aim_angle,last_duke_aim_candidates,shoot_seen_enabled()?"true":"false",seen_shots,seen_rewinds,last_seen_actors,last_seen_shift,shot_view_enabled()?"true":"false",screen_view_shots,screen_view_refused,last_screen_view?"true":"false",last_view_shift,last_view_turn,last_view_age,point_blank_shots,lift_setting(),last_bend,beam_locks,beam_extra_locks,close_retractions,assisted_shots,hidden_markers,last_assisted?"true":"false",ttk_aim_reticle()?"true":"false",shots,rejected,queries,lease_misses,last_lease_miss,(unsigned long long)aim_identity_calls,(unsigned long long)aim_identity_checks,last_weapon,last_kind,last_origin[0],last_origin[1],last_origin[2],last_target[0],last_target[1],last_target[2],last_direction[0],last_direction[1],last_direction[2]);return out;
}
}
static int crosshair_forced=-1; // -1 follow env; 0/1 override (I key, EDuke-style)
// D09B: the selected native marker owns the display even between its poses.
// Keep the crosshair preference so switching RED DOT off restores it.
static bool native_marker_selected() {
    const char* visible=ttk_live_env("DNTTK_RED_DOT");
    return ttk::input_modernized() && visible && !std::strcmp(visible,"1");
}
extern "C" int ttk_aim_crosshair_enabled(void) {
    if(native_marker_selected())return 0;
    if(crosshair_forced>=0)return crosshair_forced;
    const char* visible=ttk_live_env("DNTTK_CROSSHAIR");
    return !visible || std::strcmp(visible,"0");
}
extern "C" void ttk_aim_reset_crosshair_override(void) {crosshair_forced=-1;}
extern "C" void ttk_aim_toggle_crosshair(void) {
    if(native_marker_selected())return;
    crosshair_forced=ttk_aim_crosshair_enabled()?0:1;
}
extern "C" int ttk_aim_reticle() {
    return !ttk::free_look_active() && ttk_aim_crosshair_enabled() && !g_precise_mode && !g_ls_mode && !g_psx_call_bail &&
        ttk::view_mode() && !ttk::input_snapshot(ttk::Context::Gameplay).held[ttk::original_aim] &&
        (ttk::shot_ready() || ttk::locomotion_input_ready()) && (psx_mod_read_byte(ttk::player+0x3b8)==2) &&
        ttk::supported(psx_mod_read_byte(ttk::player+0x3b9)) && ttk::identity();
}
// Modernized crosshair: the project's own hand-drawn art, recomp/assets/ui/crosshair.png
// (9x9, ARGB; D24A, it replaced the EDuke32 CROSSHAIR tile 2523). Kept equal to the PNG by
// tests/local/test_ui_art.py.
static const uint32_t k_crosshair[81]={
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0xff80ff00u,0xff80ff00u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0xff80ff00u,0xff80ff00u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xff80ff00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
};
extern "C" int ttk_aim_crosshair_pixels(const uint32_t* p){return p==k_crosshair;}
extern "C" int ttk_aim_crosshair_image(const uint32_t** pixels,int* width,int* height) {
    if(!pixels||!width||!height||!ttk_aim_reticle())return 0;
    *pixels=k_crosshair;*width=9;*height=9;return 1;
}
PSX_MOD_CONSTRUCTOR(register_ttk_aim) {for(uint32_t a:{0x8005a210u,0x8006dd2cu})psx_mod_register_function_entry_plugin("ttk.weapon.testmove",a,ttk::move_hook);for(uint32_t a:{0x800702bcu,0x8001ca4cu,0x8002bc18u,0x8003c500u,0x800718acu,0x8006dd2cu,0x8006f1b8u})psx_mod_register_function_entry_plugin("ttk.weapon.aim",a,ttk::hook);}
