#include <chrono>
// SLUS-00583 / LEVEL00 weapon adapter. Runs before the original dispatch,
// preserving muzzle, spread, projectile integration and damage handling.
#include "weapon_aim.h"
#include "aim_weapons.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
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
static unsigned shots, rejected, queries;
static int last_kind, last_weapon;
static double last_origin[3],last_target[3],last_direction[3];
static bool busy;
static unsigned hidden_markers, assisted_shots;
static bool last_assisted;
static unsigned close_retractions;
static uint32_t aim_vector, aim_origin, beam_origins;
void aim_render_prepare() {
    // Populate contents only when used; reserve stable host pointers before
    // replay workers fork so first shots cannot force a synchronous restart.
    if(!aim_vector) aim_vector=psx_mod_alloc_guest_memory(16,16);
    if(!aim_origin) aim_origin=psx_mod_alloc_guest_memory(16,16);
    if(!beam_origins) beam_origins=psx_mod_alloc_guest_memory(96*16,16);
}
static uint32_t beam_shot_sp;
static uint64_t beam_sequence, beam_epoch;
static bool beam_pending;
static unsigned view_beams;
static bool view_mode() {
    const char* mode=std::getenv("DNTTK_WEAPON_AIM");
    return mode && !std::strcmp(mode,"view");
}
static bool supported(unsigned weapon) {return view_weapon_supported(weapon);}
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
static bool trace(CPUState* source,const double* from,const double* to,double* hit,int& kind,uint32_t* actor=nullptr) {
    uint32_t top=source->gpr[29];
    if(top<0x801f4000 || top>0x801ffff0)return false;
    uint8_t saved[4096],scratch[1024];
    for(unsigned i=0;i<sizeof saved;++i)saved[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i)scratch[i]=psx_mod_read_byte(0x1f800000+i);
    CPUState cpu=*source;uint32_t sp=top-0x100;
    cpu.pc=0;cpu.gpr[29]=sp;cpu.gpr[31]=0x800000fc;
    cpu.gpr[4]=player;cpu.gpr[5]=sp+0x30;
    cpu.gpr[6]=(int8_t)psx_mod_read_byte(player+0x2e);cpu.gpr[7]=sp+0x50;
    for(unsigned i=0;i<0x80;i+=4)psx_mod_write_word(sp+i,0);
    write_vector(sp+0x30,from);write_vector(sp+0x40,to);
    psx_mod_write_word(sp+0x10,sp+0x60);psx_mod_write_word(sp+0x14,sp+0x70);
    psx_mod_write_word(sp+0x50,0xffffffff);
    psx_dispatch_call(&cpu,0x8006d980,0x800000fc);
    kind=cpu.gpr[2];int room=(int32_t)psx_mod_read_word(sp+0x50);
    if(std::getenv("DNTTK_AIM_TRACE"))std::fprintf(stderr,"ttk-aim-query kind=%d room=%d pc=%08x from=%.0f,%.0f,%.0f to=%.0f,%.0f,%.0f\n",kind,room,cpu.pc,from[0],from[1],from[2],to[0],to[1],to[2]);
    uint32_t raw=(uint32_t)kind;
    // Original return contract: 0 miss, 1 world, otherwise actor pointer.
    // A world boundary hit can legitimately report room -1.
    bool hit_actor=raw>=0x80010000u && raw<=0x801ffffcu && !(raw&3);
    bool ok=cpu.pc==0 && !g_psx_call_bail && (kind==0 || kind==1 || hit_actor) && room>=-1 && room<128 && (kind || room>=0);
    if(ok) {if(kind)read_vector(sp+0x60,hit);else std::copy(to,to+3,hit);}
    for(unsigned i=0;i<sizeof saved;++i)psx_mod_write_byte(top-4096+i,saved[i]);
    for(unsigned i=0;i<sizeof scratch;++i)psx_mod_write_byte(0x1f800000+i,scratch[i]);
    if(actor)*actor=ok && hit_actor?raw:0;
    kind=kind==0?0:kind==1?1:2;
    ++queries;return ok;
}
bool view_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind) {
    return player_identity_ready() && identity() && trace(cpu,from,to,hit,kind);
}
// Assistance follows only an existing original target lock close to the view.
// Independently recheck camera AND physical-muzzle visibility against that exact
// actor. Failure leaves the free-view target intact; visibility settings do not
// participate. Original spread/integration/damage still own the projectile.
static bool assist(CPUState* cpu,const double* eye,const double* muzzle,const double* forward,double* target) {
    const char* mode=std::getenv("DNTTK_AIM_ASSIST");
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
    if(!trace(cpu,eye,point,hit,kind,&found) || found!=actor)return false;
    if(!trace(cpu,muzzle,point,hit,kind,&found) || found!=actor)return false;
    std::copy(point,point+3,target);return true;
}
// D07D: ownership is proven at the enqueue alone. The marker routine's only
// caller (EXE and all 30 unique overlays) is 0x80035434, which passes the actor
// in S4; the routine never saves or writes S4, and its frame holds that return
// address at sp+0x40. No entry lease, camera link, map or gameplay-state gate:
// the dot is cosmetic, so any Modernized frame that draws it for Duke hides it,
// including interpreted slices where an entry hook may not fire.
static void marker_hook(CPUState* cpu,uint32_t address) {
    if(address!=0x8002bc18 || cpu->gpr[31]!=0x80033db0 || cpu->gpr[20]!=player)return;
    const char* visible=std::getenv("DNTTK_RED_DOT");
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
        std::fprintf(stderr,"ttk-flight projectile=%08x type=%u ra=%08x from=%.0f,%.0f,%.0f to=%.0f,%.0f,%.0f direction=%.0f,%.0f,%.0f\n",a,psx_mod_read_byte(a+0x30),cpu->gpr[31],p[0],p[1],p[2],q[0],q[1],q[2],d[0],d[1],d[2]);
    }
}
// At the beam constructor's verified post-allocation list-advance call, keep
// the original damage/line collision updater but detach its implicit actor lock.
// Each projectile owns a safe physical origin for its short visual lifetime.
static void beam_finish(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const uint32_t object=cpu->gpr[16];
    if(!beam_pending || cpu->gpr[31]!=0x80073380 || cpu->gpr[29]!=beam_shot_sp-0x110 ||
       cpu->gpr[5]!=cpu->gpr[29]+0x10 || cpu->gpr[23]!=player || cpu->gpr[17]!=0 ||
       !f.active || f.sequence!=beam_sequence || f.epoch!=beam_epoch ||
       !input_modernized() || !view_mode() || busy || g_precise_mode || g_ls_mode || g_psx_call_bail ||
       object<0x800da978 || object>=0x800dcd78 || (object-0x800da978)%0x60 ||
       psx_mod_read_word(object+0x10)!=player || psx_mod_read_byte(object+0x30)!=15 ||
       psx_mod_read_word(object+0x18)!=player+0x80c || !movement_ready() || !identity())return;
    beam_pending=false;
    if(!beam_origins)beam_origins=psx_mod_alloc_guest_memory(96*16,16);
    if(!beam_origins)return;
    const auto origin=beam_origins+16*((object-0x800da978)/0x60);
    write_vector(origin,last_origin);
    psx_mod_write_word(object+0x14,0);psx_mod_write_word(object+0x18,origin);
    // Free view emits the original no-acquisition single beam. Assistance has
    // already passed both cover checks and adjusted the direction if requested.
    cpu->gpr[20]=1;++view_beams;
    if(diagnostics())std::fprintf(stderr,"ttk-view-beam projectile=%08x origin=%.0f,%.0f,%.0f\n",object,last_origin[0],last_origin[1],last_origin[2]);
}
static void aim_hook_body(CPUState* cpu,uint32_t address) {
    if(address==0x8001ca4c) {beam_finish(cpu);return;}
    marker_hook(cpu,address);
    if(input_modernized())observe(cpu,address);
    if(g_precise_mode || g_ls_mode || g_psx_call_bail || busy || !input_modernized() || !view_mode() || input_snapshot(Context::Gameplay).held[original_aim])return;
    if(address!=0x8003c500 || cpu->gpr[4]!=player ||
       !supported(cpu->gpr[7]) || !movement_ready() || !identity())return;
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
    beam_pending=false;
    busy=true;
    double eye[3],forward[3],target[3],muzzle[3],body[3],safe[3],ray_start[3];
    read_vector(camera+0x14,eye);read_vector(continuous?player+0x80c:player+0xbc,muzzle);
    for(int i=0;i<3;++i)forward[i]=(int16_t)psx_mod_read_half(camera+12+2*i);
    bool ok=normal(forward);int kind=0;
    for(int i=0;i<3;++i)target[i]=eye[i]+forward[i]*16384;
    read_vector(player+4,body);body[1]=muzzle[1];
    // The screen ray begins at Duke's depth. Geometry behind the character
    // (between the follow camera and Duke) must not turn a shot backwards.
    // The physical projectile and body-to-muzzle check still enforce all cover.
    double depth=0;for(int i=0;i<3;++i)depth+=(body[i]-eye[i])*forward[i];
    for(int i=0;i<3;++i)ray_start[i]=eye[i]+forward[i]*std::clamp(depth,0.0,16000.0);
    ok=ok && trace(cpu,ray_start,target,target,kind);
    // Keep the physical origin. Guard a muzzle protruding through thin cover
    // with a body-to-muzzle segment before committing a new direction.
    int cover=0;
    ok=ok && trace(cpu,body,muzzle,safe,cover);
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
    double convergence=0;for(int i=0;i<3;++i)convergence+=(target[i]-muzzle[i])*forward[i];
    if(ok && convergence<64) {
        // A close screen-ray hit can be behind a long/offset animated muzzle.
        // Retract, never advance through the target or manufacture damage.
        std::copy(body,body+3,muzzle);++close_retractions;
        double remaining=0;for(int i=0;i<3;++i)remaining+=(target[i]-muzzle[i])*forward[i];
        if(remaining<64)for(int i=0;i<3;++i)target[i]+=forward[i]*(64-remaining);
    }
    double dir[3];for(int i=0;i<3;++i)dir[i]=target[i]-muzzle[i];
    ok=ok && normal(dir);
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
            beam_shot_sp=sp;beam_sequence=f.sequence;beam_epoch=f.epoch;beam_pending=true;
        }
        ++shots;if(last_assisted)++assisted_shots;last_kind=cover?4:(last_assisted?2:kind);last_weapon=cpu->gpr[7];
        if(diagnostics())std::fprintf(stderr,"ttk-aim-shot weapon=%d target=%.0f,%.0f,%.0f direction=%.0f,%.0f,%.0f cover=%d\n",last_weapon,target[0],target[1],target[2],last_direction[0],last_direction[1],last_direction[2],cover);
    } else ++rejected;
    busy=false;
}
static void hook(CPUState* cpu,uint32_t address) {
    if(!frame_trace_on()) {aim_hook_body(cpu,address);return;}
    const auto t0=std::chrono::steady_clock::now();aim_hook_body(cpu,address);
    frame_trace_account(address|0x1u,(long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count());
}
const char* aim_debug_json() {
    static char out[1024];
    std::snprintf(out,sizeof out,"{\"close_retractions\":%u,\"assisted_shots\":%u,\"hidden_markers\":%u,\"last_assisted\":%s,\"reticle\":%s,\"shots\":%u,\"rejected\":%u,\"queries\":%u,\"identity_calls\":%llu,\"identity_checks\":%llu,\"weapon\":%d,\"hit_kind\":%d,\"origin\":[%.0f,%.0f,%.0f],\"target\":[%.0f,%.0f,%.0f],\"direction\":[%.0f,%.0f,%.0f]}",close_retractions,assisted_shots,hidden_markers,last_assisted?"true":"false",ttk_aim_reticle()?"true":"false",shots,rejected,queries,(unsigned long long)aim_identity_calls,(unsigned long long)aim_identity_checks,last_weapon,last_kind,last_origin[0],last_origin[1],last_origin[2],last_target[0],last_target[1],last_target[2],last_direction[0],last_direction[1],last_direction[2]);return out;
}
}
static int crosshair_forced=-1; // -1 follow env; 0/1 override (I key, EDuke-style)
extern "C" int ttk_aim_crosshair_enabled(void) {
    if(crosshair_forced>=0)return crosshair_forced;
    const char* visible=std::getenv("DNTTK_CROSSHAIR");
    return !visible || std::strcmp(visible,"0");
}
extern "C" void ttk_aim_toggle_crosshair(void) {
    crosshair_forced=ttk_aim_crosshair_enabled()?0:1;
}
extern "C" int ttk_aim_reticle() {
    return ttk_aim_crosshair_enabled() && !g_precise_mode && !g_ls_mode && !g_psx_call_bail &&
        ttk::view_mode() && !ttk::input_snapshot(ttk::Context::Gameplay).held[ttk::original_aim] &&
        (ttk::movement_ready() || ttk::locomotion_input_ready()) && (psx_mod_read_byte(ttk::player+0x3b8)==2) &&
        ttk::supported(psx_mod_read_byte(ttk::player+0x3b9)) && ttk::identity();
}
// Exact EDuke32 CROSSHAIR tile (research/inv/tile2523.png) — yellow 9×9 with open center.
static const uint32_t k_crosshair[81]={
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0xfffcfc00u,0xfffcfc00u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0xfffcfc00u,0xfffcfc00u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
    0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xfffcfc00u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,
};
extern "C" int ttk_aim_crosshair_pixels(const uint32_t* p){return p==k_crosshair;}
extern "C" int ttk_aim_crosshair_image(const uint32_t** pixels,int* width,int* height) {
    if(!pixels||!width||!height||!ttk_aim_reticle())return 0;
    *pixels=k_crosshair;*width=9;*height=9;return 1;
}
PSX_MOD_CONSTRUCTOR(register_ttk_aim) {for(uint32_t a:{0x800702bcu,0x8001ca4cu,0x8002bc18u,0x8003c500u,0x800718acu,0x8006dd2cu,0x8006f1b8u})psx_mod_register_function_entry_plugin("ttk.weapon.aim",a,ttk::hook);}
