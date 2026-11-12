// D17 high refresh rate (Modernized). TTK simulates at 30 fps (20 when a frame
// overruns; delta 5 per field, flip 0x8001fcbc). Presenting faster must never
// run its logic faster, so in-between images are drawn by replaying the game's
// own view composition on a frozen copy of the machine (render_replay.h), in
// worker processes on other cores (render_worker.c):
//
//   - at the entry of the view composition 0x80026164 (called from the frame
//     function 0x8002666c, return 0x800268f4) the guest memory and CPU state
//     are published to a worker slot;
//   - the previous frame's image is then queued for redraw at a few
//     interpolation fractions between it and this frame;
//   - a worker loads the slot, runs the composition and the frame's drawing
//     tail up to the flip, then the chain close 0x8001fba0 and the buffer swap
//     and submission 0x8001fa78, recording the GP0 stream;
//   - at each display deadline the provider picks the finished redraw that
//     matches the time since the displayed image appeared, and the runtime
//     draws its stream into saved/restored VRAM and presents it.
//
// The live guest never sees a replay. DNTTK_FRAME_RATE selects the present
// rate: auto (every display refresh up to 240, evenly divided above), display (the monitor's refresh), 30, 60 (the original presentation,
// no replays), 120, 144, 165, 180, 240 or unlimited. Vanilla always runs 60.
// DNTTK_FRAME_INTERP=off disables the redraws (developer, documentation/80).
#include "cpu_state.h"
#include "mod_plugins.h"
#include "game_menu.h"
#include "pc_input.h"
#include "modern_controls.h"
#include "weapon_aim.h"
#include "near_clip.h"
#include "screen_flash.h"
#include "live_env.h"
#include "sky_render.h"
#include "psx_sdl.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#ifdef __linux__
#include <malloc.h>
#include <x86intrin.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#endif
#include <vector>
#include <unordered_map>
#include "render_replay.h"
#include "mod_memory.h"
#include "lamp_bake_map.h"
#include <cstring>
#include <string>

extern "C" {
extern int g_psx_call_bail;
int overlay_loader_prepare_address(uint32_t addr);
}

extern "C" uint64_t s_frame_count;
extern "C" const char* ttk_frontend_input_json();
extern "C" const char* ttk_game_menu_json();
extern "C" double ttk_frontend_press_ms();
extern "C" uint64_t ttk_frontend_press_field();
extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
namespace ttk {
const char* frame_replay_debug_json();
namespace {
const char* status_json() {return frame_replay_debug_json();}
bool bolt_barrel_enabled();   // D49E (bolt_barrel.inc)
constexpr uint32_t composition=0x80026164, composition_ret=0x800268f4;
constexpr uint32_t flip=0x8001fcbc, chain_close=0x8001fba0, submit=0x8001fa78;
constexpr uint32_t sentinel=0x800000fc;
// D17T: the frontend (game mode 0x800bcbb0 == 0: title, attract demo, main
// menu) draws the same view composition from its own tick 0x80022324 (return
// 0x800234d4), and its flip waits at least 6 fields (10 images a second;
// gameplay waits 2). Its in-betweens come from the same replay. The guest's
// pace is never changed: the attract demo is recorded per-frame input and
// desyncs at 30 Hz (Duke stuck on a wall, scene cut early). The frontend's
// flip closes the drawing chain itself (return 0x8001fce0).
// DNTTK_FRONTEND_INTERP=off (developer) keeps the original 10 Hz images.
constexpr uint32_t frontend_ret=0x800234d4, frontend_close_ret=0x8001fce0, game_mode=0x800bcbb0;
uint64_t frontend_field=0, frontend_captures=0, frontend_resets=0;
double frontend_interval_ms=6000.0/59.94;
bool frontend_enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_FRONTEND_INTERP");return !(t && !std::strcmp(t,"off"));}();return v;}
bool frontend_composition(const CPUState* cpu) {
    return frontend_enabled() && cpu->gpr[31]==frontend_ret && psx_mod_read_half(game_mode)==0;
}
// A frontend image within the last 15 fields (the flip waits 6).
bool frontend_live() {
    return frontend_field && psx_mod_read_half(game_mode)==0 && s_frame_count-frontend_field<=15;
}
// D17U: loading screens. While the CD loads, the blocking load loop 0x8001c440
// (return 0x8001c49c), the frontend tick's load branch (0x8002343c) and the
// level start 0x800257c8 (0x80025818) draw the backdrop 0x800220f4, then the
// spinning logo 0x80025554, then flip: 10 images a second in mode 0, 30 in
// gameplay loads. The logo is a GTE model turned about Y by the angle
// 0x800beaf8 (12 bits), which the draw advances by the frame's field delta
// 0x800d21fc (5 per field) before drawing. These images are captured at the
// logo's entry and redrawn by the workers like frontend images, with the angle
// interpolated; the guest, its CD reads and its pace are untouched.
// DNTTK_LOADING_INTERP=off (developer) keeps the original images.
constexpr uint32_t logo_draw=0x80025554, logo_angle=0x800beaf8, field_delta=0x800d21fc;
uint64_t loading_field=0, loading_captures=0, loading_resets=0;
double loading_interval_ms=6000.0/59.94;
bool loading_enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_LOADING_INTERP");return !(t && !std::strcmp(t,"off"));}();return v;}
bool loading_image(const CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    return loading_enabled() && (ra==0x8001c49cu || ra==0x8002343cu || ra==0x80025818u);
}
// The angle the logo draw will show (it adds the delta first).
uint16_t drawn_logo_angle() {return (uint16_t)((psx_mod_read_half(logo_angle)+psx_mod_read_half(field_delta))&0xfff);}
// A loading image within the last 15 fields, in any game mode.
bool loading_live() {return loading_field && s_frame_count-loading_field<=15;}
bool replay_context() {return replay_gameplay_context() || frontend_live() || loading_live();}
// D19C: the frontend tick (one per 6 fields; the attract demo runs behind the
// title menu, so its pace stays) handles menu input in its state handler
// (state 0x800d24e4, 7 = main menu; cursor 0x800d24e6) before the composition.
// Each in-between is therefore drawn from the newest image of its pair, its
// world blended back toward the older image's poses: menu text, highlight and
// screen changes reach the screen one image interval (about 100 ms) sooner,
// while the world keeps the same interpolated motion.
// DNTTK_FRONTEND_NEWEST=off (developer) draws from the older image (D17T).
// D19E: the same for loading images in the frontend (the main menu over the
// spinning logo while a demo scene loads): drawn from the newer image with
// the logo angle interpolated, so the menu reaches the screen one image
// sooner there too. DNTTK_LOADING_NEWEST=off (developer) keeps D17U's base.
constexpr uint32_t frontend_state=0x800d24e4, menu_cursor=0x800d24e6;
// D19O: the frontend's demo sub-state (2 = the demo scene runs, 0/1 = it
// loads) and the field of the last menu image captured.
constexpr uint32_t demo_load=0x800d24e2;
uint64_t menu_field=0;
bool frontend_newest() {static const bool v=[]{const char* t=std::getenv("DNTTK_FRONTEND_NEWEST");return !(t && !std::strcmp(t,"off"));}();return v;}
bool loading_newest() {static const bool v=[]{const char* t=std::getenv("DNTTK_LOADING_NEWEST");return !(t && !std::strcmp(t,"off"));}();return v;}
// Diagnostics: the frontend state and cursor an image shows, and per menu
// change the time from the press to the guest image and to the screen.
// D19E: also the options sub-screen 0x800d24d2 and, for sub-menus with their
// own cursor, a halfword named by DNTTK_MENU_WATCH (developer, hex address).
uint32_t menu_signature() {
    static const uint32_t watch=[]{const char* e=std::getenv("DNTTK_MENU_WATCH");return e?(uint32_t)std::strtoul(e,nullptr,16):0u;}();
    const uint32_t extra=(uint32_t)psx_mod_read_half(0x800d24d2)<<8 ^ (watch ? (uint32_t)psx_mod_read_half(watch)<<10 : 0u);
    return ((uint32_t)psx_mod_read_half(frontend_state)<<16|psx_mod_read_half(menu_cursor))^extra;
}
struct MenuLatency {double press_ms=0, guest_ms=-1, screen_ms=-1; uint32_t from=0, to=0, fields=0; bool loading=false;};
MenuLatency menu_latency[32]; unsigned menu_latency_n=0;
uint32_t captured_menu=0, shown_menu=0;
const char* menu_latency_json() {
    static char buf[3600];
    int n=std::snprintf(buf,sizeof buf,"[");
    const unsigned first=menu_latency_n>32?menu_latency_n-32:0;
    for(unsigned i=first;i<menu_latency_n && n<(int)sizeof buf-100;++i) {
        const MenuLatency& l=menu_latency[i%32];
        n+=std::snprintf(buf+n,sizeof buf-n,"%s{\"from\":\"%x\",\"to\":\"%x\",\"fields\":%u,\"guest_ms\":%.1f,\"screen_ms\":%.1f,\"loading\":%d}",i>first?",":"",l.from,l.to,l.fields,l.guest_ms,l.screen_ms,(int)l.loading);
    }
    std::snprintf(buf+n,sizeof buf-n,"]");
    return buf;
}
constexpr uint32_t room_walk=0x80039dd0;   // eye room: (target pos, eye, target room, previous room)
uint64_t room_walks=0;                     // worker-side count (diagnostic)
// Optional worker timing, separate from the emulation-thread sample profile.
bool quality_profile() {static const bool on=std::getenv("DNTTK_REPLAY_PROFILE")!=nullptr;return on;}
uint64_t quality_actor_start=0,quality_actor_ticks=0;
unsigned quality_actor_count=0;
void quality_actor_end(CPUState*,uint32_t) {
    if(quality_actor_start) {quality_actor_ticks+=SDL_GetPerformanceCounter()-quality_actor_start;quality_actor_start=0;}
}
// max_jobs: per game frame, the planned in-betweens, or with the late camera
// one redraw per present. Unlimited initially schedules at 1000 Hz: a
// 4-field frame plus the lead horizon can exceed the old 15-entry limit.
// 96 entries cover that window while pacing settles; worker jobs remain 16.
extern "C" void render_worker_stats(char* out,size_t cap);
constexpr int slots=8, max_steps=8, max_jobs=96;   // slots <= RW_SLOTS (render_worker.c)

int32_t target_hz=60;   // 60 = original presentation (replay off)
bool replay_on=false;
bool interp_replay=false;
int steps=1;            // redraws per game frame (follows the rate; DNTTK_INTERP_STEPS)
bool steps_fixed=false; // DNTTK_INTERP_STEPS given
bool budget_fixed=false;
double budget_ms=1.8;   // main-thread redraw budget per guest field (ms)
double seen_hz=0.0;     // presentation rate reported by the runtime
double seen_interval=2.0/59.94;   // game frame interval (EMA of the flip interval)

// ---- Interpolation state ---------------------------------------------------
// Camera 0x800d6eb0 (stride 0xb0): the render derives the eye from the Q12
// rotation +0 (nine halfwords), the anchor +0x64..+0x6c and the distance +0x42
// (0x80039c7c), and walks portals from room +0x90. Actors (Duke 0x800d7198 and
// every actor the live frame draws through 0x800348d8): +0x3c points at the
// world joint matrices (PsyQ MATRIX, 0x20 bytes: Q12 3x3 + translation), +0x40
// at the model whose byte 0 is the joint count. Blending those matrices moves
// and animates actors between game frames; the camera fields move the eye.
constexpr uint32_t camera_base=0x800d6eb0, duke=0x800d7198;
constexpr int max_actors=64, max_joints=48;
struct Mat {int16_t m[9]; int16_t pad; int32_t t[3];};
static_assert(sizeof(Mat)==0x20,"PsyQ MATRIX");
struct ActorState {uint32_t actor=0, mats=0, model=0; int joints=0; Mat mat[max_joints];};
// D52: base is the camera's address (camera_base, or the game-owned camera
// the composition draws from: security monitors, scripted views).
struct CameraState {int16_t rot[9]; int16_t distance; int32_t anchor[3]; uint32_t room;
                   int16_t view[9]; int32_t view_t[3]; int32_t pos[3]; uint32_t base;};
// D17AY: object positions (+4..+12) and headings (+0x1c, 4096 a turn) of Duke,
// the objects he holds or climbs (+0x290, +0x17c) and the drawn actors. Game
// ticks update them; host draw hooks read them through draw_object_pose, which
// in a redraw returns the in-between value instead of the last tick.
constexpr int max_poses=24;
struct ObjectPose {uint32_t obj; int32_t pos[3]; uint16_t yaw, pad;};
struct InterpState {
    CameraState camera{};
    bool has_pivot=false; double pivot[3]{};   // orbit pivot (late camera, third person)
    int nactors=0;
    ActorState actor[max_actors];
    int nposes=0;
    ObjectPose pose[max_poses];
};
uint32_t drawn[max_actors]; int ndrawn=0;      // actors drawn by the last live frame
uint32_t drawn_kind[max_actors];
extern uint32_t drawing_kind[max_actors];
uint32_t drawing[max_actors]; int ndrawing=0;  // the live frame in progress

bool ram_ok(uint32_t a,uint32_t n) {return a>=0x80010000u && a+n<=0x80200000u && !(a&3);}
Mat read_mat(uint32_t a) {
    Mat m{};
    for(int i=0;i<9;++i) m.m[i]=(int16_t)psx_mod_read_half(a+2*i);
    for(int i=0;i<3;++i) m.t[i]=(int32_t)psx_mod_read_word(a+0x14+4*i);
    return m;
}
void read_camera(CameraState& c,uint32_t base=camera_base) {
    c.base=base;
    for(int i=0;i<9;++i) c.rot[i]=(int16_t)psx_mod_read_half(base+2*i);
    c.distance=(int16_t)psx_mod_read_half(base+0x42);
    for(int i=0;i<3;++i) c.anchor[i]=(int32_t)psx_mod_read_word(base+0x64+4*i);
    c.room=psx_mod_read_word(base+0x90);
    for(int i=0;i<9;++i) c.view[i]=(int16_t)psx_mod_read_half(base+0x20+2*i);
    for(int i=0;i<3;++i) c.view_t[i]=(int32_t)psx_mod_read_word(base+0x34+4*i);
    for(int i=0;i<3;++i) c.pos[i]=(int32_t)psx_mod_read_word(base+0x14+4*i);
}
// D52: the camera a redraw may write (the player's or a game-owned one).
bool known_camera(uint32_t base) {
    return base==camera_base || base==camera_base+0xb0u || base==0x800d7010u || base==0x800d7010u+0xb0u || base==0x800da488u;
}
bool read_actor(uint32_t actor,ActorState& a) {
    if(!ram_ok(actor,0x44)) return false;
    a.actor=actor;
    a.mats=psx_mod_read_word(actor+0x3c);
    a.model=psx_mod_read_word(actor+0x40);
    if(!ram_ok(a.model,4) || !ram_ok(a.mats,0x20)) return false;
    a.joints=psx_mod_read_byte(a.model);
    if(a.joints<=0 || a.joints>max_joints || !ram_ok(a.mats,0x20u*a.joints)) return false;
    for(int j=0;j<a.joints;++j) a.mat[j]=read_mat(a.mats+0x20u*j);
    return true;
}
void capture_state(InterpState& st) {
    // D52: the camera this frame is drawn from. Before, only the player's
    // camera was captured, so a game-owned view (security monitors) kept its
    // 30 fps steps in every redraw, and redraws of the newer and older frame
    // lists (D10H8) alternated between its two positions.
    read_camera(st.camera,composition_camera());
    st.has_pivot=late_pivot(st.pivot);
    st.nactors=0;
    auto add=[&](uint32_t actor){
        for(int i=0;i<st.nactors;++i) if(st.actor[i].actor==actor) return;
        if(st.nactors<max_actors && read_actor(actor,st.actor[st.nactors])) ++st.nactors;
    };
    add(duke);
    for(int i=0;i<ndrawn;++i) add(drawn[i]);
    st.nposes=0;
    auto pose=[&](uint32_t obj){
        if(!ram_ok(obj,0x20) || st.nposes>=max_poses) return;
        for(int i=0;i<st.nposes;++i) if(st.pose[i].obj==obj) return;
        ObjectPose& o=st.pose[st.nposes++];
        o.obj=obj; o.yaw=psx_mod_read_half(obj+0x1c); o.pad=0;
        for(int i=0;i<3;++i) o.pos[i]=(int32_t)psx_mod_read_word(obj+4+4*i);
    };
    pose(duke); pose(psx_mod_read_word(duke+0x290)); pose(psx_mod_read_word(duke+0x17c));
    for(int i=0;i<ndrawn;++i) pose(drawn[i]);
}

// Rotation part of a Q12 matrix to a unit quaternion and back (row major).
struct Quat {double w,x,y,z;};
Quat to_quat(const double r[9]) {
    Quat q;
    const double tr=r[0]+r[4]+r[8];
    if(tr>0) {double s=std::sqrt(tr+1.0)*2; q={0.25*s,(r[7]-r[5])/s,(r[2]-r[6])/s,(r[3]-r[1])/s};}
    else if(r[0]>r[4] && r[0]>r[8]) {double s=std::sqrt(1.0+r[0]-r[4]-r[8])*2; q={(r[7]-r[5])/s,0.25*s,(r[1]+r[3])/s,(r[2]+r[6])/s};}
    else if(r[4]>r[8]) {double s=std::sqrt(1.0+r[4]-r[0]-r[8])*2; q={(r[2]-r[6])/s,(r[1]+r[3])/s,0.25*s,(r[5]+r[7])/s};}
    else {double s=std::sqrt(1.0+r[8]-r[0]-r[4])*2; q={(r[3]-r[1])/s,(r[2]+r[6])/s,(r[5]+r[7])/s,0.25*s};}
    const double n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
    return {q.w/n,q.x/n,q.y/n,q.z/n};
}
void from_quat(const Quat& q,double r[9]) {
    const double w=q.w,x=q.x,y=q.y,z=q.z;
    r[0]=1-2*(y*y+z*z); r[1]=2*(x*y-z*w);   r[2]=2*(x*z+y*w);
    r[3]=2*(x*y+z*w);   r[4]=1-2*(x*x+z*z); r[5]=2*(y*z-x*w);
    r[6]=2*(x*z-y*w);   r[7]=2*(y*z+x*w);   r[8]=1-2*(x*x+y*y);
}
// Blend rotation (with per-column scale) and translation; false = discontinuity.
bool blend_rot(const int16_t a[9],const int16_t b[9],double alpha,int16_t out[9]) {
    double ra[9],rb[9],sa[3],sb[3];
    for(int c=0;c<3;++c) {
        sa[c]=std::sqrt((double)a[c]*a[c]+(double)a[3+c]*a[3+c]+(double)a[6+c]*a[6+c]);
        sb[c]=std::sqrt((double)b[c]*b[c]+(double)b[3+c]*b[3+c]+(double)b[6+c]*b[6+c]);
        if(sa[c]<256 || sb[c]<256) return false;
        for(int r=0;r<3;++r) {ra[3*r+c]=a[3*r+c]/sa[c];rb[3*r+c]=b[3*r+c]/sb[c];}
    }
    Quat qa=to_quat(ra),qb=to_quat(rb);
    double dot=qa.w*qb.w+qa.x*qb.x+qa.y*qb.y+qa.z*qb.z;
    if(dot<0) {qb={-qb.w,-qb.x,-qb.y,-qb.z};dot=-dot;}
    if(dot<0.5) return false;                    // over 120 degrees: a cut, not motion
    double wa=1-alpha,wb=alpha;
    if(dot<0.9995) {
        const double th=std::acos(std::min(dot,1.0)),st=std::sin(th);
        wa=std::sin((1-alpha)*th)/st; wb=std::sin(alpha*th)/st;
    }
    Quat q{wa*qa.w+wb*qb.w,wa*qa.x+wb*qb.x,wa*qa.y+wb*qb.y,wa*qa.z+wb*qb.z};
    const double n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
    q={q.w/n,q.x/n,q.y/n,q.z/n};
    double r[9];from_quat(q,r);
    // Endpoint residuals (what the quaternion round trip loses of each Q12
    // matrix), added back by weight: the blend is then exact at alpha 0 and 1
    // and continuous with the real images on either side.
    double fa[9],fb[9];from_quat(qa,fa);from_quat(qb,fb);
    for(int c=0;c<3;++c) {
        const double sc=sa[c]+(sb[c]-sa[c])*alpha;
        for(int row=0;row<3;++row) {
            const int i=3*row+c;
            const double res=(1-alpha)*(a[i]-fa[i]*sa[c])+alpha*(b[i]-fb[i]*sb[c]);
            out[i]=(int16_t)std::lround(std::clamp(r[i]*sc+res,-32768.0,32767.0));
        }
    }
    return true;
}
int32_t lerp_i(int32_t a,int32_t b,double alpha) {return (int32_t)std::lround(a+(double)(b-a)*alpha);}
// D10H8: past the newest pose (alpha > 1) a position runs on at the speed
// both of the last two game steps agree on (minmod, per axis): steady motion
// continues, while a stop, a reversal or a one-step jump (a stair step, a
// collision push-out) holds at the newest pose instead of overshooting.
// Rotations hold at the newest pose. ext_ms is the time past it.
struct Extrap {bool on=false; double ext_ms=0, sab=1, sza=1;} extrap;
int32_t lerp_x(int32_t a,int32_t b,double alpha,const int32_t* z) {
    if(alpha<=1.0) return lerp_i(a,b,alpha);
    if(!extrap.on || !z) return b;
    const double v1=(double)(b-a)/extrap.sab, v0=(double)(a-*z)/extrap.sza;
    const double v=v1*v0<=0 ? 0.0 : std::fabs(v1)<std::fabs(v0) ? v1 : v0;
    return (int32_t)std::lround(b+v*extrap.ext_ms);
}
// The view matrix (camera +0x20) is the camera rotation with its rows scaled
// (row 0 by the horizontal aspect, 1.6). Blend it as a rotation with row
// scales: lerping its elements made in-between views skew and shrink, and the
// first-person weapon (placed from the slerped rotation +0) drifted against it.
bool blend_view(const int16_t a[9],const int16_t b[9],double alpha,int16_t out[9]) {
    double ra[9],rb[9],sa[3],sb[3];
    for(int r=0;r<3;++r) {
        sa[r]=std::sqrt((double)a[3*r]*a[3*r]+(double)a[3*r+1]*a[3*r+1]+(double)a[3*r+2]*a[3*r+2]);
        sb[r]=std::sqrt((double)b[3*r]*b[3*r]+(double)b[3*r+1]*b[3*r+1]+(double)b[3*r+2]*b[3*r+2]);
        if(sa[r]<256 || sb[r]<256) return false;
        for(int c=0;c<3;++c) {ra[3*r+c]=a[3*r+c]/sa[r];rb[3*r+c]=b[3*r+c]/sb[r];}
    }
    Quat qa=to_quat(ra),qb=to_quat(rb);
    double dot=qa.w*qb.w+qa.x*qb.x+qa.y*qb.y+qa.z*qb.z;
    if(dot<0) {qb={-qb.w,-qb.x,-qb.y,-qb.z};dot=-dot;}
    if(dot<0.5) return false;
    double wa=1-alpha,wb=alpha;
    if(dot<0.9995) {
        const double th=std::acos(std::min(dot,1.0)),st=std::sin(th);
        wa=std::sin((1-alpha)*th)/st; wb=std::sin(alpha*th)/st;
    }
    Quat q{wa*qa.w+wb*qb.w,wa*qa.x+wb*qb.x,wa*qa.y+wb*qb.y,wa*qa.z+wb*qb.z};
    const double n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
    q={q.w/n,q.x/n,q.y/n,q.z/n};
    double m[9];from_quat(q,m);
    double fa[9],fb[9];from_quat(qa,fa);from_quat(qb,fb);   // endpoint residuals, as blend_rot
    for(int r=0;r<3;++r) {
        const double sc=sa[r]+(sb[r]-sa[r])*alpha;
        for(int c=0;c<3;++c) {
            const int i=3*r+c;
            const double res=(1-alpha)*(a[i]-fa[i]*sa[r])+alpha*(b[i]-fb[i]*sb[r]);
            out[i]=(int16_t)std::lround(std::clamp(m[i]*sc+res,-32768.0,32767.0));
        }
    }
    return true;
}

// One published game frame.
struct Frame {
    uint64_t serial=0, flips=0;
    double time_ms=0; // pose time on the host presentation clock
    int slot=-1;
    bool valid=false;
    int jobs[max_jobs];      // worker job, -1 when not submitted yet or released
    float alpha[max_jobs];
    bool failed[max_jobs];
    bool submitted[max_jobs];
    double look_time[max_jobs]; // host clock ms used for this camera sample
    float late_yaw[max_jobs];   // diagnostic: late view yaw used (degrees), 999 none
    double target[max_jobs];    // late camera: the present (ms) this redraw is for
    double submit_ms[max_jobs]; // late camera: when it was submitted (ms), 0 once seen ready
    // D10G3: the camera this redraw shows (eye, forward x4096) when the late
    // view owned it; the shot reads it once the redraw is presented.
    bool view_ok[max_jobs];
    double view_eye[max_jobs][3], view_fwd[max_jobs][3];
    int16_t view_rot[max_jobs][9];   // D49F: the whole view rotation (rows)
    int njobs=0;
    InterpState state;
    uint8_t render[16384];  // modern_controls draw-time state (render_state_save; D41M: 16 KB with the lamp levels)
    NearClipFrameState near{};   // near clip frame accounting at composition entry
    // Transforms the live composition loaded through 0x800292a0 (camera,
    // matrix), keyed by drawn object and call index within its draw.
    struct Xf {uint32_t obj; uint16_t seq, pad; Mat m;};
    static constexpr int max_xf=512;
    Xf xf[max_xf];
    int nxf=0;
    bool xf_done=false;
    bool frontend=false;   // D17T: a frontend image (no mouse look, longer interval)
    bool loading=false;    // D17U: a loading screen image (also frontend: game-owned view)
    uint16_t logo=0;       // D17U: logo angle this image shows
    uint32_t menu=0;       // D19C: frontend state and cursor (diagnostic)
    bool menu_mode=false;  // D19E: captured in game mode 0 (frontend menus can be drawn)
    uint32_t shows[max_jobs]; // D19C: menu signature of the image each redraw is drawn from
    FlashState flash;      // D39B: the screen flash at composition
    uint32_t duke_word=0;  // D39E trace: Duke's word 0 at composition (bit 0x20000 frozen)
};
Frame frames[slots];
extern double late_lead_ms;
// D17T: the world delay when the newest image is a frontend one, else 0.
double newest_frontend() {
    const Frame* n=nullptr;
    for(const auto& f:frames) if(f.valid && (!n || f.serial>n->serial)) n=&f;
    if(!n || !n->frontend) return 0.0;
    static const double fixed=[]{const char* e=std::getenv("DNTTK_FRONTEND_DELAY_MS");return e?std::clamp(std::atof(e),60.0,250.0):0.0;}();
    const double interval=n->loading ? loading_interval_ms : frontend_interval_ms;
    return fixed>0.0 ? fixed : std::clamp(interval+late_lead_ms+8.0,90.0,200.0);
}
double response_slack=0,response_pose_lag=0,response_oldest_age=0;
uint64_t response_future_evictions=0;
uint64_t serial=0;
uint64_t captures=0,publishes=0,submits=0,presented=0,reused=0,no_job=0,failures=0;

// Parameters shipped with a job (worker side reads them).
struct Params {
    uint32_t magic;
    float alpha;
    uint32_t flags;          // 1 = camera fields present, 4 = loading image (logo angle), 8 = body camera, 16 = D49 beams follow the view, 32 = D49C beam animation (beam_t), 64 = D49E bolts at the barrel
    uint32_t nmats;          // matrix patches following the header
    CameraState camera;
    uint32_t render_len;     // draw-time host state after the patches
    uint32_t nxf;            // transform substitutions after the render state
    NearClipFrameState near; // D17A: the live frame's near clip accounting
    int32_t logo;            // D17U: logo angle to show (flag 4)
    int16_t body_rot[9],body_view[9]; // D14H: Duke's own draw (flag 8)
    uint64_t serial;         // D17AY: base frame (present trace)
    float beam_t;            // D49C: redraw time in game frames past the drawn frame's list (flag 32)
    uint32_t nposes;         // D17AY: in-between object poses
    ObjectPose pose[max_poses];
};
struct MatPatch {uint32_t addr; Mat mat;};
constexpr uint32_t params_max=60000;
uint64_t game_camera_blends=0;   // D52: in-betweens of a game-owned camera
uint64_t blended_poses=0,blended_mats=0,skipped_actors=0,camera_cuts=0,pending_flips=0,blended_xf=0,skipped_objects=0,substituted=0,sub_misses=0;
Frame* rec_frame=nullptr;      // live frame whose transforms are being recorded
uint32_t xf_obj=0; uint16_t xf_seq=0;
uint32_t scratch=0;            // mod memory ring for substituted matrices
constexpr int scratch_n=64;

// Build the job parameters for base frame a toward frame b at alpha.
uint32_t build_params(const InterpState& a,const InterpState& b,double alpha,uint8_t* out,const InterpState* z=nullptr) {
    Params p{};
    p.magic=0x54544b32u;
    p.alpha=(float)alpha;
    const double ar=std::min(alpha,1.0);   // D10H8: rotations hold past the newest pose
    // Camera: a jump in the anchor (cut, teleport) keeps the base camera.
    CameraState c=a.camera;
    long dx=(long)b.camera.anchor[0]-a.camera.anchor[0],dy=(long)b.camera.anchor[1]-a.camera.anchor[1],dz=(long)b.camera.anchor[2]-a.camera.anchor[2];
    // D52: a switch between the player's and a game-owned camera is a cut.
    const bool cut=dx*dx+dy*dy+dz*dz>(long)2048*2048 || a.camera.base!=b.camera.base;
    if(!cut && blend_rot(a.camera.rot,b.camera.rot,ar,c.rot)) {
        for(int i=0;i<3;++i) c.anchor[i]=lerp_x(a.camera.anchor[i],b.camera.anchor[i],alpha,z?&z->camera.anchor[i]:nullptr);
        c.distance=(int16_t)lerp_i(a.camera.distance,b.camera.distance,ar);
        // Through a doorway the in-between eye's room is found in the worker
        // by the game's own portal walk (flag 2); until then keep the base room.
        c.room=a.camera.room;
        // Developer check DNTTK_ROOM_WALK=always|off.
        static const int walk_mode=[]{const char* t=std::getenv("DNTTK_ROOM_WALK");return !t?1:(!std::strcmp(t,"always")||!std::strcmp(t,"probe"))?2:!std::strcmp(t,"off")?0:1;}();
        // D52: the walk starts from Duke, so only for the player's camera.
        if(a.camera.base==camera_base && (walk_mode==2 || (walk_mode==1 && a.camera.room!=b.camera.room))) p.flags|=2;
        if(walk_mode==0) c.room=alpha<0.5 ? a.camera.room : b.camera.room;
        if(!blend_view(a.camera.view,b.camera.view,ar,c.view))
            for(int i=0;i<9;++i) c.view[i]=(int16_t)lerp_i(a.camera.view[i],b.camera.view[i],ar);
        for(int i=0;i<3;++i) c.view_t[i]=lerp_x(a.camera.view_t[i],b.camera.view_t[i],alpha,z?&z->camera.view_t[i]:nullptr);
        for(int i=0;i<3;++i) c.pos[i]=lerp_x(a.camera.pos[i],b.camera.pos[i],alpha,z?&z->camera.pos[i]:nullptr);
        p.camera=c;
        p.flags|=1;
        game_camera_blends+=c.base!=camera_base;
    } else ++camera_cuts;
    // D17AY: objects present in both frames; a jump (teleport, respawn,
    // reused slot) keeps the base pose.
    for(int i=0;i<a.nposes && p.nposes<(uint32_t)max_poses;++i) {
        const ObjectPose& x=a.pose[i];
        const ObjectPose* y=nullptr;
        for(int k=0;k<b.nposes;++k) if(b.pose[k].obj==x.obj) {y=&b.pose[k];break;}
        ObjectPose o=x;
        if(y) {
            long dx=(long)y->pos[0]-x.pos[0],dy=(long)y->pos[1]-x.pos[1],dz=(long)y->pos[2]-x.pos[2];
            if(dx*dx+dy*dy+dz*dz<=(long)1536*1536) {
                const ObjectPose* w=nullptr;
                if(z && alpha>1.0) for(int k=0;k<z->nposes;++k) if(z->pose[k].obj==x.obj) {w=&z->pose[k];break;}
                for(int k=0;k<3;++k) o.pos[k]=lerp_x(x.pos[k],y->pos[k],alpha,w?&w->pos[k]:nullptr);
                const int turn=(((int)(y->yaw&0xfff)-(int)(x.yaw&0xfff)+2048)%4096+4096)%4096-2048;
                o.yaw=(uint16_t)((x.yaw&0xf000)|(((int)(x.yaw&0xfff)+(int)std::lround(turn*ar))&0xfff));
                ++blended_poses;
            }
        }
        p.pose[p.nposes++]=o;
    }
    uint32_t used=sizeof(Params);
    for(int i=0;i<a.nactors;++i) {
        const ActorState& x=a.actor[i];
        const ActorState* y=nullptr;
        for(int k=0;k<b.nactors;++k) if(b.actor[k].actor==x.actor) {y=&b.actor[k];break;}
        if(!y || y->mats!=x.mats || y->model!=x.model || y->joints!=x.joints) {++skipped_actors;continue;}
        MatPatch patch[max_joints];
        bool ok=true;
        const ActorState* w=nullptr;
        if(z && alpha>1.0) for(int k=0;k<z->nactors;++k) if(z->actor[k].actor==x.actor && z->actor[k].mats==x.mats && z->actor[k].joints==x.joints) {w=&z->actor[k];break;}
        for(int j=0;j<x.joints && ok;++j) {
            const Mat& m0=x.mat[j];const Mat& m1=y->mat[j];
            long tx=(long)m1.t[0]-m0.t[0],ty=(long)m1.t[1]-m0.t[1],tz=(long)m1.t[2]-m0.t[2];
            if(tx*tx+ty*ty+tz*tz>(long)1536*1536) {ok=false;break;}
            patch[j].addr=x.mats+0x20u*j;
            patch[j].mat=m0;
            if(!blend_rot(m0.m,m1.m,ar,patch[j].mat.m)) {ok=false;break;}
            for(int k=0;k<3;++k) patch[j].mat.t[k]=lerp_x(m0.t[k],m1.t[k],alpha,w?&w->mat[j].t[k]:nullptr);
        }
        if(!ok) {++skipped_actors;continue;}
        const uint32_t bytes=sizeof(MatPatch)*(uint32_t)x.joints;
        if(used+bytes>params_max) break;
        std::memcpy(out+used,patch,bytes);
        used+=bytes;
        p.nmats+=(uint32_t)x.joints;
        blended_mats+=(uint32_t)x.joints;
    }
    std::memcpy(out,&p,sizeof p);
    return used;
}
// Interpolated transforms for every object (except Duke, whose joint
// matrices are patched in RAM) drawn with the same call count in both frames.
uint32_t append_xf(const Frame& a,const Frame& b,double alpha,uint8_t* out,uint32_t used,const Frame* z=nullptr) {
    Params p; std::memcpy(&p,out,sizeof p);
    const double ar=std::min(alpha,1.0);
    int i=0;
    while(i<a.nxf) {
        const uint32_t obj=a.xf[i].obj;
        int n=0; while(i+n<a.nxf && a.xf[i+n].obj==obj) ++n;
        int j=-1;
        for(int k=0;k<b.nxf;++k) if(b.xf[k].obj==obj) {j=k;break;}
        int nb=0; if(j>=0) while(j+nb<b.nxf && b.xf[j+nb].obj==obj) ++nb;
        bool ok=j>=0 && nb==n && obj!=duke;
        int jz=-1;   // D10H8: the same object two frames back (extrapolation)
        if(ok && z && alpha>1.0) {
            for(int k=0;k<z->nxf;++k) if(z->xf[k].obj==obj) {jz=k;break;}
            int nz=0; if(jz>=0) while(jz+nz<z->nxf && z->xf[jz+nz].obj==obj) ++nz;
            if(nz!=n) jz=-1;
        }
        Frame::Xf blend[Frame::max_xf];
        for(int k=0;k<n && ok;++k) {
            const Mat& m0=a.xf[i+k].m; const Mat& m1=b.xf[j+k].m;
            long tx=(long)m1.t[0]-m0.t[0],ty=(long)m1.t[1]-m0.t[1],tz=(long)m1.t[2]-m0.t[2];
            if(tx*tx+ty*ty+tz*tz>(long)1536*1536) {ok=false;break;}
            blend[k]=a.xf[i+k];
            if(!blend_rot(m0.m,m1.m,ar,blend[k].m.m)) {ok=false;break;}
            for(int c=0;c<3;++c) blend[k].m.t[c]=lerp_x(m0.t[c],m1.t[c],alpha,jz>=0?&z->xf[jz+k].m.t[c]:nullptr);
        }
        if(ok && used+n*sizeof(Frame::Xf)<=params_max) {
            std::memcpy(out+used,blend,n*sizeof(Frame::Xf));
            used+=n*(uint32_t)sizeof(Frame::Xf); p.nxf+=(uint32_t)n; blended_xf+=(uint64_t)n;
        } else if(obj!=duke) ++skipped_objects;
        i+=n;
    }
    std::memcpy(out,&p,sizeof p);
    return used;
}
uint32_t append_render(const Frame& base,uint8_t* out,uint32_t used) {
    const uint32_t n=(uint32_t)render_state_size();
    if(n>sizeof base.render || used+n>params_max) return used;
    std::memcpy(out+used,base.render,n);
    Params p; std::memcpy(&p,out,sizeof p); p.render_len=n; std::memcpy(out,&p,sizeof p);
    return used+n;
}

int32_t parse_rate(const char* text) {
    if(!text || !*text || !std::strcmp(text,"auto")) return -2;
    if(!std::strcmp(text,"display")) return 0;
    if(!std::strcmp(text,"unlimited")) return -1;
    char* end=nullptr;
    const long v=std::strtol(text,&end,10);
    if(end && !*end && v>=30 && v<=1000) return (int32_t)v;
    return 60;
}

Frame* frame_by_serial(uint64_t s) {
    for(auto& f:frames) if(f.valid && f.serial==s) return &f;
    return nullptr;
}

bool late_camera();
// Continuous presentation: interpolate timestamped pose samples,
// never predict which VRAM band the next VBlank will expose.
bool continuous_timeline() {
    static const bool on=[]{const char* e=std::getenv("DNTTK_PRESENT_TIMELINE");return !(e && e[0]=='0');}();
    return on && late_camera();
}
double newest_frontend();
// D10H5: gameplay world delay, adaptive. A present at t shows the world at
// t - delay, interpolated between frames n-1 and n, so the pair must be
// complete (frame n composed) when its redraw is submitted, a lead before t:
// delay >= (n ready - n-1 pose time) + lead. world_delay_ms follows the p99
// of that pair age over the last 128 frames (about 4 s) plus the lead and
// half a present (50..90 ms; 90 was the fixed value), slewed so the world clock
// only runs 10% slower or 3% faster, never jumps. DNTTK_WORLD_DELAY_MS
// fixes it (50..130).
double world_delay_ms=90.0;
bool world_delay_fixed=false;
double pair_age_samples[128]{}; unsigned pair_age_count=0;
double pair_age_p99=0, world_delay_wanted=90.0;
uint64_t timeline_starved=0, timeline_presents=0;
// D10H7 diagnostics, gameplay submits (last 1024): the actual lead
// (target - submit) and the delay the submit needed (target - newest
// complete pose; starved when the world delay is below it).
double submit_ahead_samples[1024]{}, submit_need_samples[1024]{}; unsigned submit_sample_count=0;
// D10H7: submits are never early (target - submit <= lead; the provider
// ticks about once per present), so the present term is tail margin on the
// p99 estimate, not submit granularity. Half a present kept starved presents
// at the D10H5 level (0.03%, none at 0); DNTTK_WORLD_DELAY_MARGIN scales it
// (1 = D10H5, 0 = no margin: 0.12% starved).
double world_delay_margin_scale=0.5;
// D10H8: first-person world draw on the present timeline (gameplay only;
// third person keeps pair). Default newer: the newer frame's display list
// (its new hits, smoke, HUD and weapon frame appear one game frame sooner),
// with the same blended poses as the pair, like EDuke32 drawing its newest
// sprites at interpolated positions. DNTTK_WORLD_DRAW=pair is the D10H5/D10H7
// draw (the older list); step draws the newest complete frame unblended
// (moving things step at the game rate). DNTTK_WORLD_EXTRAP_MS (default 16,
// 0..50) lets the world time run that far past the newest pose (positions
// limited by lerp_x) and lowers the delay target by the same amount.
int world_draw=-1;          // 0 pair, 1 newer, 2 step
double world_extrap_ms=0;
uint64_t timeline_extrapolated=0;
double world_delay_pair=90.0;   // the D10H5 target without extrapolation
void world_draw_init();
// First person only (third person keeps the D10H5 pair blend).
double world_fp_extrap() {world_draw_init(); return world_draw>=1 && late_fp_blend()>=0.999 ? world_extrap_ms : 0.0;}
// The extrapolation a present may use: what the world delay is below the
// pair-only target, at most the setting, so after a switch to third person
// the world does not hold while the delay slews back up.
double world_extrap_allow() {
    world_draw_init();
    if(world_draw<1) return 0.0;
    if(world_delay_fixed) return world_fp_extrap();
    return std::clamp(world_delay_pair-world_delay_ms,0.0,world_extrap_ms);
}
void world_draw_init() {
    if(world_draw>=0) return;
    world_draw=1;
    if(const char* e=std::getenv("DNTTK_WORLD_DRAW")) world_draw=!std::strcmp(e,"pair")?0:!std::strcmp(e,"step")?2:1;
    world_extrap_ms=16.0;
    if(const char* e=std::getenv("DNTTK_WORLD_EXTRAP_MS")) world_extrap_ms=std::clamp(std::atof(e),0.0,50.0);
    if(world_draw==2) world_extrap_ms=0;
}
void note_submit(double ahead,double need) {
    submit_ahead_samples[submit_sample_count%1024]=ahead;
    submit_need_samples[submit_sample_count++%1024]=need;
}
double sample_pct(const double* v,unsigned count,int pct) {
    const unsigned n=std::min(count,1024u);
    if(!n) return 0;
    static double sorted[1024];std::copy(v,v+n,sorted);std::sort(sorted,sorted+n);
    return sorted[(n-1)*pct/1000];
}
void note_pair_age(double ms) {
    pair_age_samples[pair_age_count++%128]=std::clamp(ms,0.0,200.0);
    const unsigned n=std::min(pair_age_count,128u);
    double sorted[128];std::copy(pair_age_samples,pair_age_samples+n,sorted);
    std::sort(sorted,sorted+n);
    pair_age_p99=sorted[(n-1)*99/100];
}
void update_world_delay(double now_ms,double period_ms) {
    static double last=0;
    const double dt=last ? std::clamp(now_ms-last,0.0,50.0) : 0.0;
    last=now_ms;
    if(world_delay_fixed || pair_age_count<8) return;
    world_draw_init();
    world_delay_pair=std::clamp(pair_age_p99+late_lead_ms+period_ms*world_delay_margin_scale,50.0,90.0);
    world_delay_wanted=world_delay_pair-world_fp_extrap();
    world_delay_ms+=std::clamp(world_delay_wanted-world_delay_ms,-dt*0.03,dt*0.10);
}
double timeline_delay() {
    static const bool init=[]{
        if(const char* e=std::getenv("DNTTK_WORLD_DELAY_MS")) {world_delay_ms=std::clamp(std::atof(e),50.0,130.0);world_delay_fixed=true;}
        if(const char* e=std::getenv("DNTTK_WORLD_DELAY_MARGIN")) world_delay_margin_scale=std::clamp(std::atof(e),0.0,2.0);
        return true;}();
    (void)init;
    // D17T: frontend images are 6 fields apart. The pair around a present
    // must be complete (the next image composed and its redraw submitted a
    // lead before the present), so the delay covers one interval plus the
    // lead and a margin: about 130 ms instead of 90.
    const double fe=newest_frontend();
    return fe>0.0 ? fe : world_delay_ms;
}
bool pair_starved=false;   // the last timeline_pair: world time past its pair
double pair_end_ms=0;      // the last timeline_pair: newest complete pose time
Frame* timeline_pair(double present_ms, Frame*& next, double& alpha) {
    const double world_ms=present_ms-timeline_delay();
    Frame* best=nullptr; Frame* oldest=nullptr;
    next=nullptr; pair_end_ms=-1e30;
    for(auto& f:frames) if(f.valid && f.xf_done) {
        Frame* n=frame_by_serial(f.serial+1);
        if(!n || !n->xf_done || n->time_ms<=f.time_ms) continue;
        pair_end_ms=std::max(pair_end_ms,n->time_ms);
        if(!oldest || f.time_ms<oldest->time_ms) oldest=&f;
        if(f.time_ms<=world_ms && (!best || f.time_ms>best->time_ms)) best=&f;
    }
    if(!best) best=oldest;
    if(!best) return nullptr;
    world_draw_init();
    next=frame_by_serial(best->serial+1);
    const bool game=!best->frontend && !next->frontend;
    if(game && world_draw==2) {
        // D10H8 step: the newest complete frame, as it is.
        for(auto& f:frames) if(f.valid && f.xf_done) {
            Frame* n=frame_by_serial(f.serial+1);
            if(n && n->xf_done && n->time_ms>next->time_ms && !n->frontend) {best=&f;next=n;}
        }
        pair_starved=false;alpha=1.0;
        return best;
    }
    const double extrap=game ? world_extrap_allow() : 0.0;
    const double span=next->time_ms-best->time_ms;
    pair_starved=world_ms>next->time_ms+extrap;
    alpha=std::clamp((world_ms-best->time_ms)/span,0.0,1.0+extrap/span);
    return best;
}
bool timeline_clock_reset=true;
void reset_timeline(bool keep_shown=false);
void release_jobs(Frame& f) {
    for(int i=0;i<f.njobs;++i) if(f.jobs[i]>=0) psx_mod_replay_release(f.jobs[i]);
    f.njobs=0;
}
bool late_camera();
extern int pace_div;
int submit_job(Frame& base,const Frame& next,double alpha,bool late,double look_ms=-1);

// Queue redraws of the previous frame (the image the player sees next),
// blended toward the frame just captured.
uint8_t param_buffer[params_max];
bool interp_off=false;   // DNTTK_REPLAY_TEST=alpha0 / interp_off: plain redraws
bool interp_full=false;  // DNTTK_REPLAY_TEST=alpha1: redraw fully at the next frame
// One in-between per display refresh of a 30 fps game frame: 120 Hz -> 3,
// 144/165 -> 4, 180 -> 5, 240 -> 7. The redraw budget grows with it; the
// slack floor still keeps every redraw out of the time the game needs.
void update_steps() {
    if(steps_fixed) return;
    double hz=target_hz>0 ? (double)target_hz : seen_hz;
    if(hz<=0.0) return;
    // Late camera pacing presents every pace_div-th refresh: plan one
    // in-between per present. Planned per refresh, only the first third (or
    // half) of a frame's in-betweens were ever shown, so moving things went
    // 0 to 0.33 of the way and then jumped (2026-10-02 dancer "pop").
    // seen_hz already includes the runtime divisor (display/Unlimited).
    if(late_camera() && target_hz>0) hz/=pace_div;
    // In-betweens per game frame from the measured frame interval: at 20 fps
    // (busy scenes) 180 Hz shows nine images per game frame, not six.
    steps=std::clamp((int)std::lround(hz*seen_interval)-1,1,max_steps);
    // Late camera: one more redraw per game frame (alpha 0) and none of the
    // presents is a free real image, so the cap is higher; the slack floor and
    // the next-present check still keep redraws out of the game's own time.
    const int jobs=steps+(late_camera()?1:0);
    if(!budget_fixed) budget_ms=std::min(1.2+0.6*jobs*(2.0/59.94)/seen_interval,late_camera()?8.0:4.0);
}
struct QueueEntry {uint32_t serial; float ms;};
QueueEntry queue_ring[32]; unsigned queue_n=0;
uint64_t seen_last_flip=0, seen_frequency=0;
// D17B late camera (Modernized, default on; DNTTK_LATE_CAMERA=0 off). Every
// presented image is a redraw made for that present (submit_due) whose view
// rotation is the mouse look a fixed lead before it (late_view);
// the eye, Duke and every object stay interpolated between game frames, and
// gameplay keeps its own 30 Hz aim. A redraw at alpha 0 replaces the real
// image, which would otherwise swing the view back to the older game camera at
// every game frame. Before, the in-betweens blended two game cameras, so
// turning reached the screen a game frame or more late (heavier at 20 fps).
bool late_camera() {
    // Default on, with adaptive pacing (late_pace): where fresh images cannot
    // keep up with every refresh, presents drop to every 2nd or 3rd refresh,
    // evenly, instead of repeating images (the 2026-10-02 playtest judder).
    // DNTTK_LATE_CAMERA=0 restores the blend of two game cameras.
    static const bool v=[]{const char* t=std::getenv("DNTTK_LATE_CAMERA");return !(t && t[0]=='0' && !t[1]);}();
    return v && !interp_off && !interp_full;
}
// Submit a job this long before its present. Adaptive: the measured time from
// submission to a finished worker redraw (smoothed, x1.25 + 2 ms, 10..40 ms),
// so light scenes keep the mouse latency low and heavy ones (the dancers at
// 4x: worker redraws past 16 ms) stop repeating images (2026-10-03).
// DNTTK_LATE_LEAD_MS fixes it.
double late_lead_ms=24.0, late_ready_ms=16.0;
double ready_samples[64]{}; unsigned ready_count=0;
void note_ready(double ms) {
    ready_samples[ready_count++%64]=std::clamp(ms,0.0,60.0);
    const unsigned n=std::min(ready_count,64u);
    double sorted[64];std::copy(ready_samples,ready_samples+n,sorted);
    std::sort(sorted,sorted+n);
    // Deadlines need the tail, not the average: dancer redraws have a
    // 12 ms centre but regularly take 20+ ms. Keep a bounded p99 window.
    late_ready_ms=sorted[(n-1)*99/100];
}
bool late_lead_fixed=false;
uint64_t late_applied=0, late_skipped=0, late_jit=0;
// Late camera pacing: per window of 90 presents, the share that had to
// repeat an image or fall back to the real one. Above 10% (visible judder;
// fresh images cannot keep up, for example the opening street at 4x and CPU
// 100%) presents go to every 2nd, then 3rd refresh, evenly. Stepping back up
// needs a long clean run (under 2%): 4 windows, doubled every time the faster
// rate fails again within 3 windows (at most 32), so a scene settles on one
// steady rate instead of switching back and forth, which itself read as
// jerky (2026-10-03).
int pace_div=1, pace_clean=0, pace_backoff=1; uint64_t pace_presents=0, pace_bad=0, pace_changes=0, pace_windows=0, pace_down_window=0; uint64_t emu_sheds=0;
double pace_last_ratio=0; uint64_t pace_mid=0;
// D23H load shedding (replay_load_window): divisor steps it owns, its backoff.
int shed_steps=0, shed_backoff=1, shed_bad=0, shed_clean=0, shed_since_probe=1000, shed_full_clean=0;
double shed_feed_ms=0;  // D17AR3 part 4: redraw feed cost in the second that shed
bool shed_audio=false;  // D50: the last shed was for the audio ring
uint64_t shed_probes=0, shed_failed_probes=0;  // D23H: windows between 2% and 10%
// D23H: every present-rate change goes to the session log with its reason,
// and late_pace prints a summary each minute (presents/s, rate, backoff).
void pace_log(const char* why,double ratio,int backoff) {
    static unsigned n=0; if(++n>200) return;
    std::fprintf(stderr,"[TTK pace] presents every %d refresh%s (%s, repeats %.1f%%, backoff %d)\n",pace_div,pace_div>1?"es":"",why,ratio*100.0,backoff);
}
// D23I: with each minute line, where the resident memory sits, so a long
// session's growth names its owner in the log. Read on a short-lived thread:
// /proc/self/smaps takes a few milliseconds. Static = the executable's tables
// (.data/.bss, e.g. the GPU host-vertex tables, a high-water mark capped at
// 108 MiB); heap = malloc's main arena, with the bytes malloc has handed out (allocated, not all resident);
// shared = GL driver and replay-worker shared mappings; anon = other private
// anonymous memory (malloc's other arenas, large blocks, thread stacks);
// files = mapped libraries and files.
void memory_minute(unsigned minute) {
#ifndef __linux__
    (void)minute;
#else
    std::thread([minute]{
        FILE* f=std::fopen("/proc/self/smaps","r"); if(!f) return;
        char line[512], exe[256]={0};
        { const ssize_t n=readlink("/proc/self/exe",exe,sizeof exe-1); if(n>0) exe[n]=0; }
        unsigned long long kb[6]={0,0,0,0,0,0};   // static heap shared anon files total
        int cls=4; unsigned long long last_end=0, exe_end=0;
        while(std::fgets(line,sizeof line,f)) {
            unsigned long long lo,hi; char perm[8]={0};
            if(std::sscanf(line,"%llx-%llx %7s",&lo,&hi,perm)==3) {
                const char* name=line; for(int i=0;i<5 && name;++i) {name=std::strchr(name,' '); if(name) while(*name==' ') ++name;}
                std::string nm=name?std::string(name):std::string(); while(!nm.empty() && (nm.back()=='\n' || nm.back()==' ')) nm.pop_back();
                if(exe[0] && nm==exe) {cls=perm[1]=='w'?0:4; exe_end=hi;}
                else if(nm.empty()) cls=(lo==exe_end && lo==last_end)?0:perm[3]=='s'?2:3;
                else if(nm=="[heap]") cls=1;
                else if(perm[3]=='s' || nm.rfind("/dev/",0)==0 || nm.rfind("/memfd:",0)==0) cls=2;
                else if(nm[0]=='[') cls=3;
                else cls=4;
                if(cls==0 && nm.empty()) exe_end=hi;
                last_end=hi; continue;
            }
            unsigned long long v;
            if(std::sscanf(line,"Rss: %llu kB",&v)==1) {kb[cls]+=v; kb[5]+=v;}
        }
        std::fclose(f);
        const struct mallinfo2 mi=mallinfo2();
        std::fprintf(stderr,"[TTK memory] minute %u: rss %llu MB = static %llu, heap %llu (malloc allocated %zu), shared %llu, anon %llu, files %llu\n",
            minute,kb[5]/1024,kb[0]/1024,kb[1]/1024,(mi.uordblks+mi.hblkhd)>>20,kb[2]/1024,kb[3]/1024,kb[4]/1024);
    }).detach();
#endif
}
void pace_minute() {
    using clk=std::chrono::steady_clock;
    static clk::time_point t0; static uint64_t n0=0, w0=0, m0=0, c0=0; static uint64_t presents=0;
    ++presents; const auto now=clk::now();
    if(!t0.time_since_epoch().count()) {t0=now;n0=presents;w0=pace_windows;m0=pace_mid;c0=pace_changes;return;}
    const double dt=std::chrono::duration<double>(now-t0).count();
    if(dt<60.0) return;
    long rss_kb=0; if(FILE* f=std::fopen("/proc/self/statm","r")) {long a=0,b=0; if(std::fscanf(f,"%ld %ld",&a,&b)==2) rss_kb=b*4; std::fclose(f);}
    std::fprintf(stderr,"[TTK pace] minute: %.1f presents/s, every %d, backoff %d/%d, %llu windows (%llu mixed), %llu changes, rss %ld MB\n",
        (presents-n0)/dt,pace_div,pace_backoff,shed_backoff,(unsigned long long)(pace_windows-w0),(unsigned long long)(pace_mid-m0),(unsigned long long)(pace_changes-c0),rss_kb/1024);
    static unsigned minutes=0; memory_minute(++minutes);
    t0=now;n0=presents;w0=pace_windows;m0=pace_mid;c0=pace_changes;
}
void late_pace(bool bad) {
    static const bool on=[]{const char* t=std::getenv("DNTTK_LATE_PACING");return !(t && t[0]=='0' && !t[1]);}();
    if(!on) return;
    ++pace_presents; if(bad) ++pace_bad;
    if(pace_presents<90) return;
    const double ratio=(double)pace_bad/(double)pace_presents;
    pace_presents=pace_bad=0; ++pace_windows; pace_last_ratio=ratio;
    if(ratio>0.10 && pace_div<(target_hz==-1 ? 16 : 3)) {
        if(pace_down_window && pace_windows-pace_down_window<=3) pace_backoff=std::min(pace_backoff*2,32);
        ++pace_div;pace_clean=0;psx_mod_set_present_divisor(pace_div);++pace_changes;pace_log("repeats",ratio,pace_backoff);
    } else if(ratio<0.02 && pace_div>1) {
        if(++pace_clean>=4*pace_backoff) {--pace_div;pace_clean=0;pace_down_window=pace_windows;psx_mod_set_present_divisor(pace_div);++pace_changes;pace_log("recovered",ratio,pace_backoff);}
    } else {if(ratio>=0.02) ++pace_mid; pace_clean=0;}
}
float last_late_yaw=999.0f;
// Rotate the interpolated camera to the late view about the eye (first person)
// or the orbit pivot (third person). False keeps the interpolated camera.
bool apply_late(const InterpState& a,const InterpState& b,double alpha,Params& p,double look_ms) {
    double ly=0,lp=0;
    // D52: the mouse never turns a game-owned camera (security monitors).
    if(!(p.flags&1) || p.camera.base!=camera_base || !late_view(ly,lp,look_ms)) return false;
    CameraState& c=p.camera;
    double ra[9];
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) n+=(double)c.rot[3*r+k]*c.rot[3*r+k];
        n=std::sqrt(n); if(n<1024) return false;
        for(int k=0;k<3;++k) ra[3*r+k]=c.rot[3*r+k]/n;
    }
    static const double test_yaw=[]{const char* t=std::getenv("DNTTK_LATE_TEST_YAW");return t?std::atof(t)*3.14159265358979/180:0.0;}();
    ly+=test_yaw;   // developer check: every redraw turned by this many degrees
    // D17AU (developer): DNTTK_LATE_TEST_YAW_FILE names a file holding degrees
    // added to every redraw from now on, so a still scene can be panned along
    // the same path in every run.
    static const char* yaw_file=std::getenv("DNTTK_LATE_TEST_YAW_FILE");
    if(yaw_file) if(FILE* y=std::fopen(yaw_file,"r")) {double d=0;if(std::fscanf(y,"%lf",&d)==1) ly+=d*3.14159265358979/180;std::fclose(y);}
    last_late_yaw=(float)(ly*180/3.14159265358979);
    const double sy=std::sin(ly),cy=std::cos(ly),sp=std::sin(lp),cp=std::cos(lp);
    const double rl[9]={cy,0,-sy,-sy*sp,cp,-cy*sp,sy*cp,sp,cy*cp};
    // Sanity check against a cut the lease did not report. A fast mouse flick
    // can lead the blended game camera by tens of degrees; it must not fall
    // back to the blend for some in-betweens (the view would snap back).
    if(ra[6]*rl[6]+ra[7]*rl[7]+ra[8]*rl[8]<std::cos(100*3.14159265358979/180)) return false;
    double pv[3];
    const bool eye=late_fp_blend()>=0.999;
    if(eye) for(int i=0;i<3;++i) pv[i]=c.pos[i];
    else if(a.has_pivot && b.has_pivot) for(int i=0;i<3;++i) pv[i]=a.pivot[i]+(b.pivot[i]-a.pivot[i])*alpha;
    else return false;
    // World point q -> pv + Rl^T Ra (q - pv): fixed relative to the view.
    auto move=[&](int32_t q[3]) {
        double d[3],v[3];
        for(int i=0;i<3;++i) d[i]=q[i]-pv[i];
        for(int r=0;r<3;++r) v[r]=ra[3*r]*d[0]+ra[3*r+1]*d[1]+ra[3*r+2]*d[2];
        for(int i=0;i<3;++i) q[i]=(int32_t)std::lround(pv[i]+rl[i]*v[0]+rl[3+i]*v[1]+rl[6+i]*v[2]);
    };
    move(c.anchor);
    if(!eye) {move(c.pos);move(c.view_t);p.flags|=2;}
    double vs[3];
    for(int r=0;r<3;++r) {double n=0; for(int k=0;k<3;++k) n+=(double)c.view[3*r+k]*c.view[3*r+k]; vs[r]=std::sqrt(n);}
    for(int i=0;i<9;++i) {
        c.rot[i]=(int16_t)std::lround(rl[i]*4096);
        c.view[i]=(int16_t)std::lround(std::clamp(rl[i]*vs[i/3],-32768.0,32767.0));
    }
    // D14H: in first person Duke faces the view yaw at every game frame, but
    // his skeleton is interpolated between game frames while the late view
    // turns at the present rate, so on fast turns his body trailed the view
    // and caught up at the next game frame. Draw Duke alone through the
    // interpolated yaw at the late pitch, about the same eye: identical to
    // turning his whole body to the late yaw. Gameplay heading, aim and the
    // world are untouched. DNTTK_LATE_BODY=0 (developer) keeps the old draw.
    static const bool body=[]{const char* t=std::getenv("DNTTK_LATE_BODY");return !(t && t[0]=='0' && !t[1]);}();
    if(eye && body) {
        const double gy=std::atan2(ra[6],ra[8]),by=std::sin(gy),bx=std::cos(gy);
        const double rb[9]={bx,0,-by,-by*sp,cp,-bx*sp,by*cp,sp,bx*cp};
        for(int i=0;i<9;++i) {
            p.body_rot[i]=(int16_t)std::lround(rb[i]*4096);
            p.body_view[i]=(int16_t)std::lround(std::clamp(rb[i]*vs[i/3],-32768.0,32767.0));
        }
        p.flags|=8;
    }
    return true;
}
// Build and submit one redraw job of base toward next at alpha.
uint32_t submitted_menu=0;   // D19C: menu signature of the image the last job draws from
int submit_job(Frame& base,const Frame& next,double alpha,bool late,double look_ms) {
    submitted_menu=base.menu;
    // Callers store this job at base.njobs.
    const int view_at=base.njobs<max_jobs ? base.njobs : -1;
    if(view_at>=0) base.view_ok[view_at]=false;
    if(base.loading) {
        // D17U: the shortest way round between the two images' angles.
        const double a=interp_off ? 0.0 : interp_full ? 1.0 : alpha;
        const int turn=(((int)next.logo-(int)base.logo+2048)%4096+4096)%4096-2048;
        // D19E: in the frontend, draw from the newer image (menu text and
        // highlight one image sooner); the angle is the same interpolation.
        const Frame& drawn=next.loading && base.menu_mode && next.menu_mode && loading_newest() ? next : base;
        Params p{};
        p.magic=0x54544b32u; p.alpha=(float)a; p.flags=4; p.near=drawn.near;
        p.logo=((int)base.logo+(int)std::lround(turn*a))&0xfff;
        std::memcpy(param_buffer,&p,sizeof p);
        const uint32_t len=append_render(drawn,param_buffer,sizeof p);
        // Tagged with the base serial (released with the base frame).
        const int job=psx_mod_replay_submit(drawn.slot,param_buffer,len,base.serial);
        if(job>=0) ++submits;
        submitted_menu=drawn.menu;
        return job;
    }
    if(base.frontend && next.frontend && !next.loading && frontend_newest()) {
        // D19C: draw from the newer image, blended back toward the older one.
        const double a=1.0-(interp_off ? 0.0 : interp_full ? 1.0 : alpha);
        uint32_t len=build_params(next.state,base.state,a,param_buffer);
        {Params p; std::memcpy(&p,param_buffer,sizeof p); p.near=next.near; std::memcpy(param_buffer,&p,sizeof p);}
        len=append_render(next,param_buffer,len);
        len=append_xf(next,base,a,param_buffer,len);
        // Tagged with the base serial: the job belongs to the base frame's
        // list (released with it, before the newer frame's slot is reused).
        const int job=psx_mod_replay_submit(next.slot,param_buffer,len,base.serial);
        if(job>=0) ++submits;
        submitted_menu=next.menu;
        return job;
    }
    submitted_menu=base.menu;
    // D10H8: the newer frame's display list (world_draw newer/step), with the
    // same poses, camera and draw-time blends as the base list.
    world_draw_init();
    const bool fp=late_fp_blend()>=0.999;
    const bool newer=late && fp && world_draw>=1 && &next!=&base && !base.frontend && !next.frontend && next.serial==base.serial+1;
    const Frame& drawn=newer ? next : base;
    const Frame* back=alpha>1.0 ? frame_by_serial(base.serial-1) : nullptr;
    if(back && (back->frontend || !back->xf_done)) back=nullptr;
    extrap.on=back!=nullptr;
    if(back) {extrap.sab=std::max(next.time_ms-base.time_ms,1.0);extrap.sza=std::max(base.time_ms-back->time_ms,1.0);extrap.ext_ms=(alpha-1.0)*extrap.sab;}
    uint32_t len=build_params(base.state,next.state,interp_off ? 0.0 : interp_full ? 1.0 : alpha,param_buffer,back?&back->state:nullptr);
    {
        Params p; std::memcpy(&p,param_buffer,sizeof p);
        p.near=drawn.near; p.serial=base.serial;
        // D49C: the drawn list's beams were drawn at its own game frame; the
        // redraw shows the world at alpha past base (newer list: alpha - 1).
        if(aim_beam_anim_enabled()) {
            const double ra=interp_off ? 0.0 : interp_full ? 1.0 : alpha;
            p.beam_t=(float)(newer ? ra-1.0 : ra); p.flags|=32;
        }
        // D10H8: an extrapolated eye can leave the base room; walk it.
        if(alpha>1.0) p.flags|=2;
        if(bolt_barrel_enabled()) p.flags|=64;   // D49E
        last_late_yaw=999.0f;
        if(late) {
            if(apply_late(base.state,next.state,alpha,p,look_ms)) {
                ++late_applied;
                if((p.flags&8) && aim_beam_follow_enabled()) p.flags|=16;
                if(view_at>=0) {
                    base.view_ok[view_at]=true;
                    for(int i=0;i<3;++i) {base.view_eye[view_at][i]=p.camera.pos[i];base.view_fwd[view_at][i]=p.camera.rot[6+i];}
                    for(int i=0;i<9;++i) base.view_rot[view_at][i]=p.camera.rot[i];
                }
            } else ++late_skipped;
        }
        std::memcpy(param_buffer,&p,sizeof p);
    }
    {
        const uint32_t at=len;len=append_render(drawn,param_buffer,len);
        const double ra=interp_off ? 0.0 : interp_full ? 1.0 : alpha;
        if(len-at==render_state_size()) {
            if(newer) render_state_blend_newer(param_buffer+at,base.render,ra);
            else render_state_blend_mask(param_buffer+at,next.render,ra);
        }
    }
    len=append_xf(base,next,interp_off ? 0.0 : interp_full ? 1.0 : alpha,param_buffer,len,back);
    extrap.on=false;
    // Tagged with the base serial: released with the base frame, which is
    // older, so the newer frame's slot is still held (oldest slot reused first).
    const int job=psx_mod_replay_submit(drawn.slot,param_buffer,len,base.serial);
    if(job>=0) ++submits;
    return job;
}

void queue(Frame& base,const Frame& next) {
    {
        QueueEntry& q=queue_ring[queue_n++%32];
        q.serial=(uint32_t)base.serial;
        q.ms=seen_last_flip && seen_frequency ? (float)((double)(SDL_GetPerformanceCounter()-seen_last_flip)*1000.0/(double)seen_frequency) : -1.0f;
    }
    release_jobs(base);
    update_steps();
    // Late camera: no plan; submit_due makes one redraw per coming present.
    if(late_camera()) return;
    for(int i=1;i<=steps && base.njobs<max_jobs;++i) {
        const double alpha=(double)i/(double)(steps+1);
        const int n=base.njobs;
        base.alpha[n]=(float)alpha; base.failed[n]=false; base.jobs[n]=-1; base.submitted[n]=false;
        ++base.njobs;
        const int job=submit_job(base,next,alpha,false);
        if(job<0) {--base.njobs;break;}
        base.jobs[n]=job; base.submitted[n]=true;
    }
}
// The game has issued its next display flip (0x8001fa78: buffer index
// 0x800bdbd0, DISPENV 0x800d1cfc + index*0x74) that the presenter has not
// shown yet: the image on screen changes at the next VBlank.
bool guest_flip_pending() {
    const uint32_t index=psx_mod_read_word(0x800bdbd0);
    const int guest_y=index<8 ? (int16_t)psx_mod_read_half(0x800d1cfc+index*0x74+2) : -1;
    const int shown_y=psx_mod_present_display_y();
    return guest_y>=0 && shown_y>=0 && guest_y!=shown_y;
}
uint64_t ahead_hist[8];   // diagnostic: per due present, newest serial - shown serial (x2, +1 while composing)
void submit_due(const PSXModPresentInfo* info,Frame* shown) {
    // Late camera, one redraw per present: about late_lead_ms before each
    // coming present, submit a redraw with the newest mouse look and the
    // positions at that present's own time in the game frame on screen
    // (alpha = elapsed / interval, held at 1 while the next image is late).
    // A fixed plan of in-betweens per game frame could not follow frames of
    // 2, 3 or 4 fields: it either ran out (repeats, then a catch-up) or, with
    // extra redraws at the frame's end, held positions and wasted work
    // (2026-10-02/03 playtests).
    if(!late_camera() || !info->frequency || !info->last_flip || !shown || info->present_hz<=0.0) return;
    const double to_ms=1000.0/(double)info->frequency;
    const double now_ms=(double)info->now*to_ms;
    const double flip_ms=(double)info->last_flip*to_ms;
    const double interval_ms=(info->flip_interval>0.02 && info->flip_interval<0.15 ? info->flip_interval : seen_interval)*1000.0;
    const double period_ms=1000.0/info->present_hz;
    // When the image changes: a display change happens only at a VBlank. Once
    // the game has issued its flip it is the next VBlank after now (the flip
    // is issued 15-20 ms before it shows in a 2-field frame); before that it
    // cannot be earlier than that VBlank and is predicted one frame interval
    // after the last change.
    const bool pending=guest_flip_pending();
    const double field_ms=1000.0/59.94;
    double vblank_ms=flip_ms+field_ms*std::ceil((now_ms-flip_ms)/field_ms);
    if(vblank_ms<=now_ms) vblank_ms+=field_ms;
    // A VBlank that passed under 2 ms ago with the flip pending is the change
    // the presenter has not registered yet.
    if(pending && vblank_ms-field_ms>flip_ms+1.0 && now_ms-(vblank_ms-field_ms)<2.0) vblank_ms-=field_ms;
    const double end_ms=pending ? vblank_ms : std::max(flip_ms+interval_ms,vblank_ms);
    Frame* next=frame_by_serial(shown->serial+1);
    // Worker latency: first time a submitted redraw is seen finished.
    for(Frame* f:{shown,next}) {
        if(!f) continue;
        for(int i=0;i<f->njobs;++i) {
            uint64_t tag=0;
            if(f->submit_ms[i]<=0.0 || f->jobs[i]<0 || psx_mod_replay_job_state(f->jobs[i],&tag)!=3 || tag!=f->serial) continue;
            note_ready(now_ms-f->submit_ms[i]);
            f->submit_ms[i]=0.0;
        }
    }
    if(!late_lead_fixed) {
        // D17AW: on the render thread an image is ready its drawing time after
        // the worker finishes; the lead covers both.
        const double wanted=std::clamp(late_ready_ms+psx_mod_replay_render_latency_ms()+3.0,10.0,40.0);
        // Slew with elapsed time so the mouse sampling time cannot jump
        // backwards when a slow job enters the readiness window.
        static double last_adjust=0;
        const double change=last_adjust?std::clamp((now_ms-last_adjust)*0.1,0.0,1.0):0;
        late_lead_ms+=std::clamp(wanted-late_lead_ms,-change,change);
        last_adjust=now_ms;
    }
    for(double t=(double)info->next_deadline*to_ms; t-now_ms<=late_lead_ms; t+=period_ms) {
        if(t<now_ms-period_ms*0.5) continue;
        for(int k=0;k<2;++k) {
            Frame* f=k?next:shown;
            if(!f) continue;
            // Shown frame: before the change; past a predicted change only
            // when the present is close (6 ms) and the game still has not
            // flipped, so the image really is late. Next frame: from the
            // change (a quarter present early when only predicted).
            if(k==0 && t>=end_ms && (pending || t-now_ms>6.0)) continue;
            if(k==1 && t<end_ms-(pending?0.0:period_ms*0.25)) continue;
            Frame* to=frame_by_serial(f->serial+1);
            if(!to) continue;
            bool have=false;
            for(int i=0;i<f->njobs;++i) if(std::fabs(f->target[i]-t)<period_ms*0.5) {have=true;break;}
            if(have || f->njobs>=max_jobs) continue;
            const double base=k?end_ms:flip_ms;
            const double alpha=std::clamp((t-base)/interval_ms,0.0,1.0);
            // The look as it was a fixed lead before this present: every
            // present is then exactly that far behind the mouse, whenever its
            // redraw happens to be submitted (DNTTK_LATE_SAMPLE=0: now).
            static const bool sample=[]{const char* e=std::getenv("DNTTK_LATE_SAMPLE");return !(e && e[0]=='0' && !e[1]);}();
            const double look_ms=sample ? std::min(t-late_lead_ms,now_ms) : now_ms;
            const int job=submit_job(*f,*to,alpha,!f->frontend,look_ms);
            if(job<0) return;   // no free worker job: try again at the next tick
            const int n=f->njobs++;
            f->alpha[n]=(float)alpha; f->failed[n]=false; f->jobs[n]=job; f->submitted[n]=true; f->shows[n]=submitted_menu;
            f->look_time[n]=look_ms; f->late_yaw[n]=last_late_yaw; f->target[n]=t; f->submit_ms[n]=now_ms; ++late_jit;
            static const bool log=std::getenv("DNTTK_LATE_LOG")!=nullptr;
            if(log) std::fprintf(stderr,"[late] serial %llu k %d t %+.1f now %+.1f end %+.1f alpha %.2f pending %d n %d\n",
                (unsigned long long)f->serial,k,t-flip_ms,now_ms-flip_ms,end_ms-flip_ms,alpha,pending?1:0,n);
        }
    }
}

enum Kind {gameplay_image=0, frontend_image=1, loading_image_kind=2};
void capture(CPUState* cpu,int kind);
// D17AR3 part 2: redraw-only live frames. With the late camera every
// gameplay present is a worker redraw (a missing one repeats the last), so
// the live frame's own drawing is never shown, yet it cost the emulation
// thread its host world pass and all of its GPU drawing. While engaged, a
// live gameplay frame selects its host world as before but emits no pieces
// (host_visibility.inc), and the GPU DMA lists it sends after its flip drop
// their drawing primitives (psx_mod_live_draw_skip_arm); state, VRAM
// uploads and copies, the guest's own timing and everything the game reads
// are unchanged. The presenter never shows such an image: it keeps the screen
// as it is, and savestate thumbnails come from the last redraw shown. Engaged
// only in gameplay images with the late camera and live workers, after 8
// presents in a row showed a redraw; 3 presents in a row without one (workers
// stalled or restarting; D50: and at least 250 ms since the last one), a
// failed redraw or a savestate load draw the next live frames again. Frontend,
// loading, menus, pause, split screen, 60 Hz and Vanilla are never engaged.
// DNTTK_REDRAW_ONLY=0 (developer) or console "redrawonly off" disables it;
// D40: without either, the live renderer decides (Classic: off).
std::atomic<int> redraw_only_override{-1};
bool redraw_only_switch() {
    static const int v=ttk_env_switch("DNTTK_REDRAW_ONLY");
    const int o=redraw_only_override.load(std::memory_order_relaxed);
    return o>=0 ? o!=0 : v>=0 ? v!=0 : !ttk_world_classic();
}
bool live_redraw_only=false, redraw_only_engaged=false, redraw_only_submit=false;
unsigned ro_good=0, ro_miss_run=0;
// D50: when the last fresh redraw was shown (presenter, below). A short gap
// in the workers' redraws (the energy beam's draw code makes every redraw
// about a third dearer) no longer turns live drawing back on at once: the
// emulation thread then drew every list itself and, on the player's machine,
// stalled 60-100 ms (audio underruns, game slowdown). Measured (note 415):
// with the energy beam the render thread's GL work doubles (flat/textured
// batch ping-pong) and redraws arrive 70-130 ms apart now and then, while
// the workers keep running. Only a gap of 3 presents and ro_tolerance_ms
// (250 ms, the workers' own stuck-job deadline RW_JOB_DEADLINE_MS) draws
// live frames again; meanwhile the screen repeats its last image, and the
// pace governor sheds presents if repeats persist.
// DNTTK_REDRAW_ONLY_TOLERANCE_MS (developer, 0 restores the 3-present rule).
std::chrono::steady_clock::time_point ro_last_good;
// D50 diagnostics: why presents had no redraw (reported with each switch-off).
// miss classes: 0 no job aimed at this present, 1 job queued in a worker,
// 2 worker finished (drawing or not picked), 3 job failed; loop counts: no
// timeline pair for a deadline, no free worker job.
uint64_t ro_miss_cls[4], ro_nopair, ro_nojob, ro_miss_queued, ro_miss_running;
double ro_miss_age_max, ro_miss_age_sum; uint64_t ro_miss_age_n;
uint64_t ro_miss_rt_pending, ro_miss_rt_gone, ro_miss_free, ro_miss_wfail;
double ro_tolerance_ms() {
    static const double v=[]{const char* t=std::getenv("DNTTK_REDRAW_ONLY_TOLERANCE_MS");const double n=t?std::atof(t):250.0;return n>=0 && n<=1000 ? n : 250.0;}();
    return v;
}
bool ro_workers_behind() {
    if(ro_miss_run<3) return false;
    const double gap=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-ro_last_good).count();
    return gap>=ro_tolerance_ms();
}
uint64_t ro_failures_seen=0, ro_frames=0, ro_engages=0, ro_drops=0;
void redraw_only_trace(const char* what) {
    static const bool on=std::getenv("DNTTK_REDRAW_ONLY_TRACE")!=nullptr;
    if(!on) return;
    uint64_t st[11];psx_mod_live_draw_skip_stats(st);
    std::fprintf(stderr,"[TTK redraw-only] %s field=%llu lists=%llu/%llu skipped=%llu flips=%llu\n",what,(unsigned long long)s_frame_count,
        (unsigned long long)st[9],(unsigned long long)st[10],(unsigned long long)st[0],(unsigned long long)psx_mod_present_flips());
}
void redraw_only_decide(bool gameplay) {
    const bool want=gameplay && redraw_only_switch() && late_camera() && continuous_timeline() &&
        replay_gameplay_context() && psx_mod_replay_workers_active();
    // A savestate load replaces VRAM under any older redraw: draw the first
    // frames after it again until fresh redraws are shown.
    static uint32_t load_epoch=0;
    const bool fail_redraw=failures!=ro_failures_seen,fail_load=psx_mod_savestate_loads()!=load_epoch;
    const bool failed=fail_redraw || fail_load;
    ro_failures_seen=failures;load_epoch=psx_mod_savestate_loads();
    if(failed) ro_good=0;
    if(!want || failed || ro_workers_behind()) {
        if(redraw_only_engaged) {
            ++ro_drops;
            // D50: why the live frames draw again (the emulation thread then
            // draws every list itself; the player's stalls, note 415).
            static unsigned logged;
            static uint64_t seen_cls[4],seen_np,seen_nj;
            if(logged<60) std::fprintf(stderr,"[TTK redraw-only] misses since last: no job %llu, in worker %llu, worker done %llu, failed %llu; deadlines without a pair %llu, no free job %llu\n",
                (unsigned long long)(ro_miss_cls[0]-seen_cls[0]),(unsigned long long)(ro_miss_cls[1]-seen_cls[1]),(unsigned long long)(ro_miss_cls[2]-seen_cls[2]),(unsigned long long)(ro_miss_cls[3]-seen_cls[3]),
                (unsigned long long)(ro_nopair-seen_np),(unsigned long long)(ro_nojob-seen_nj));
            for(int k=0;k<4;++k) seen_cls[k]=ro_miss_cls[k];seen_np=ro_nopair;seen_nj=ro_nojob;
            if(logged<60) {char w[256];render_worker_stats(w,sizeof w);
                std::fprintf(stderr,"[TTK redraw-only] in-worker misses: render thread drawing %llu, image gone %llu, job freed %llu, worker failed %llu, queued %llu, running %llu, job age at its present mean %.1f max %.1f ms; lead %.1f, ready p99 %.1f ms; workers %s\n",
                (unsigned long long)ro_miss_rt_pending,(unsigned long long)ro_miss_rt_gone,(unsigned long long)ro_miss_free,(unsigned long long)ro_miss_wfail,
                (unsigned long long)ro_miss_queued,(unsigned long long)ro_miss_running,ro_miss_age_n?ro_miss_age_sum/ro_miss_age_n:0.0,ro_miss_age_max,late_lead_ms,late_ready_ms,w);}
            ro_miss_queued=ro_miss_running=ro_miss_rt_pending=ro_miss_rt_gone=ro_miss_free=ro_miss_wfail=0;ro_miss_age_max=ro_miss_age_sum=0;ro_miss_age_n=0;
            if(++logged<=60) std::fprintf(stderr,"[TTK redraw-only] off: %s (field %llu, %u presents without a redraw)\n",
                !want?(!gameplay?"not a gameplay image":!psx_mod_replay_workers_active()?"workers inactive":"conditions"):
                fail_redraw?"a redraw failed":fail_load?"savestate load":"workers behind",
                (unsigned long long)s_frame_count,ro_miss_run);
        }
        redraw_only_engaged=false;
    } else if(!redraw_only_engaged && ro_good>=8) {
        redraw_only_engaged=true;++ro_engages;
    }
    live_redraw_only=redraw_only_submit=redraw_only_engaged;
    if(live_redraw_only) ++ro_frames;
    redraw_only_trace(live_redraw_only?"compose engaged":"compose");
}
// Every flip (0x8001fcbc, gameplay, frontend and loading alike) is a frame
// boundary: the frame's lists are sent after it, several of them, chained
// from the GPU's completion interrupt until the next frame. They skip only
// when the composition before this flip was engaged, so a frame drawn
// without one (pause, menus, loading) never inherits it.
// D40: a renderer switch (OPTIONS, console) waits for the next flip, so a
// frame is drawn wholly by one renderer: the frame just composed keeps the
// renderer it was built with (its redraws carry it in RenderState), the
// next one is drawn by the new one. Polymorph draws its redraws on the render
// thread (D17AW); Classic keeps the single-thread path.
std::atomic<int> world_request{-1};
bool render_thread_wanted() {
    const char* t=std::getenv("DNTTK_RENDER_THREAD");
    return !ttk_world_classic() && !(t && t[0]=='0' && !t[1]);
}
uint64_t world_switches=0;
void world_renderer_apply() {
    const int q=world_request.exchange(-1);
    if(q<0 || q==ttk_world_classic()) return;
    ttk_world_classic_set(q);
    // Polymorph's present tint stops on Classic (D39G: no flash there), and
    // does not come back with a switch to Polymorph.
    if(q) screen_flash(0);
    psx_mod_set_render_thread(render_thread_wanted());
    ++world_switches;
    std::fprintf(stderr,"[TTK renderer] %s (switch %llu, field %llu)\n",q?"classic":"polymorph",
        (unsigned long long)world_switches,(unsigned long long)s_frame_count);
}
void redraw_only_flip(CPUState*,uint32_t) {
    if(!psx_mod_replay_active()) world_renderer_apply();
    near_clip_flip();
    if(psx_mod_replay_active()) return;
    psx_mod_live_draw_skip_arm(redraw_only_submit);
    if(redraw_only_submit) redraw_only_trace("flip armed");
    redraw_only_submit=false;
}
void composition_hook(CPUState* cpu,uint32_t address) {
    if(address==composition && !psx_mod_replay_active()) {
        live_redraw_only=redraw_only_submit=false;
        psx_mod_live_draw_skip_enable(replay_on && interp_replay && redraw_only_switch());
    }
    if(address!=composition || !replay_on || !interp_replay || psx_mod_replay_active()) return;
    const bool fe=frontend_composition(cpu);
    if(!fe && (cpu->gpr[31]!=composition_ret || psx_mod_read_word(0x800c27bc)!=1)) return;
    if(continuous_timeline() && !fe && !replay_gameplay_context()) return;
    redraw_only_decide(!fe);
    capture(cpu,fe ? frontend_image : gameplay_image);
}
void impact_tags_scan();
void capture(CPUState* cpu,int kind) {
    ++captures;
    static uint64_t load_epoch=~uint64_t(0),field0=0,last_field=0;
    static double host0=0;
    static int last_kind=gameplay_image;
    static bool last_menu=false;
    const uint64_t field=s_frame_count, epoch=psx_mod_savestate_loads();
    const double host_now=1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency();
    // D17T/D17U: never blend across a switch between gameplay, frontend and
    // loading images, or across a gap in frontend or loading images (a demo
    // scene loading, a load starting).
    const bool switched=kind!=last_kind || (kind!=gameplay_image && field-last_field>15);
    double& interval_ms=kind==loading_image_kind ? loading_interval_ms : frontend_interval_ms;
    if(kind!=gameplay_image && !switched && field>last_field) interval_ms+=((double)(field-last_field)*(1000.0/59.94)-interval_ms)*0.25;
    // A load starts at the mode's pace: 6 fields in the frontend, 2 in gameplay.
    if(switched && kind==loading_image_kind) loading_interval_ms=(psx_mod_read_half(game_mode)==0 ? 6.0 : 2.0)*1000.0/59.94;
    if(switched && kind==frontend_image) ++frontend_resets;
    if(switched && kind==loading_image_kind) ++loading_resets;
    last_kind=kind; last_field=field;
    if(kind==frontend_image) ++frontend_captures;
    if(kind==loading_image_kind) ++loading_captures;
    // D19O: menu images (game mode 0) keep the last redraw on screen through
    // a reset; see menu_alone.
    const bool menu=kind!=gameplay_image && psx_mod_read_half(game_mode)==0;
    if(epoch!=load_epoch || field<field0 || timeline_clock_reset || switched || std::fabs(host_now-(host0+(double)(field-field0)*(1000.0/59.94)))>100.0) {
        reset_timeline(menu && last_menu && epoch==load_epoch); load_epoch=epoch; field0=field;
        host0=host_now; timeline_clock_reset=false;
    }
    last_menu=menu;
    if(menu) menu_field=field;
    // Follow sustained host/guest clock phase error gradually. Keep pose
    // intervals monotonic and avoid turning a startup stall into a permanent
    // extra delay larger than the retained snapshot history.
    if(continuous_timeline()) {
        const double error=host_now-(host0+(double)(field-field0)*(1000.0/59.94));
        host0+=std::clamp(error*.05,-1.0,1.0);
    }
    const double pose_ms=host0+(double)(field-field0)*(1000.0/59.94);
    // Reuse the oldest slot; its redraws are no longer on screen.
    Frame* f=nullptr;
    for(auto& c:frames) if(!c.valid) {f=&c;break;}
    if(!f) {
        f=&frames[0];
        for(auto& c:frames) if(c.serial<f->serial) f=&c;
    }
    if(f->valid && continuous_timeline()) for(int i=0;i<f->njobs;++i)
        if(f->target[i]>host_now) {++response_future_evictions;break;}
    release_jobs(*f);
    f->valid=false;
    const int slot=(int)(f-frames);
    // D49A: tag new beam impact glows and sparks before the workers see them.
    if(kind==gameplay_image) impact_tags_scan();
    if(!psx_mod_replay_publish(slot,cpu)) return;
    ++publishes;
    f->serial=++serial; f->time_ms=pose_ms;
    // The previous frame's flip (0x8001fa78: buffer index 0x800bdbd0, then
    // PutDispEnv of DISPENV 0x800d1cfc + index*0x74) is seen by the presenter
    // only at the next VBlank. If it is still pending, count it now: this
    // frame's image is shown two display changes after that flip.
    {
        const uint32_t index=psx_mod_read_word(0x800bdbd0);
        const int guest_y=index<8 ? (int16_t)psx_mod_read_half(0x800d1cfc+index*0x74+2) : -1;
        const int shown_y=psx_mod_present_display_y();
        f->flips=psx_mod_present_flips()+((guest_y>=0 && shown_y>=0 && guest_y!=shown_y) ? 1u : 0u);
        if(guest_y>=0 && shown_y>=0 && guest_y!=shown_y) ++pending_flips;
    }
    f->slot=slot;
    f->flash=screen_flash_read();
    f->duke_word=psx_mod_read_word(0x800d7198);
    f->valid=true;
    f->frontend=kind!=gameplay_image;
    f->loading=kind==loading_image_kind;
    f->logo=f->loading ? drawn_logo_angle() : 0;
    f->menu_mode=psx_mod_read_half(game_mode)==0;
    f->menu=kind==frontend_image || (f->loading && f->menu_mode) ? menu_signature() : 0;
    if(f->menu && f->menu!=captured_menu) {
        // D19C diagnostics: a new menu image. Pair it with a press made since
        // the previous change (within 1 s).
        const double press=ttk_frontend_press_ms();
        MenuLatency& last=menu_latency[(menu_latency_n+31)%32];
        if(captured_menu && press>0 && host_now-press<3000.0 && (!menu_latency_n || press>last.press_ms)) {
            MenuLatency& e=menu_latency[menu_latency_n++%32];
            e=MenuLatency{}; e.press_ms=press; e.guest_ms=host_now-press; e.from=captured_menu; e.to=f->menu; e.fields=(uint32_t)(field-ttk_frontend_press_field()); e.loading=f->loading;
        }
        captured_menu=f->menu;
    }
    if(f->loading) {f->state.nactors=0;f->state.has_pivot=false;read_camera(f->state.camera);}
    else capture_state(f->state);
    static const bool pose_trace=std::getenv("DNTTK_POSE_TRACE")!=nullptr;
    if(pose_trace) std::fprintf(stderr,
        "[pose] serial=%llu field=%llu pose_ms=%.3f host_ms=%.3f eye=%d,%d,%d\n",
        (unsigned long long)f->serial,(unsigned long long)field,pose_ms,host_now,
        f->state.camera.pos[0],f->state.camera.pos[1],f->state.camera.pos[2]);
    static const bool nostate=std::getenv("DNTTK_REPLAY_NOSTATE")!=nullptr;
    if(!nostate && render_state_size()<=sizeof f->render) render_state_save(f->render);
    else if(!nostate) {   // D41M: a state that no longer fits loses every redraw's lights, say so
        static bool told;if(!told) {told=true;std::fprintf(stderr,"[TTK redraw] draw-time state %zu bytes does not fit %zu: redraws run without it\n",render_state_size(),sizeof f->render);}
    }
    f->near=near_clip_frame_state();
    // Record this frame's transforms; redraws of the previous frame are queued
    // when this composition ends (composition_end), with both sets complete.
    f->nxf=0; f->xf_done=false;
    rec_frame=f;
}

// Worker process: draw a published frame (memory and CPU already loaded).
const uint8_t* sub_table=nullptr; uint32_t sub_n=0, sub_cursor=0;
// D14H: the late and body cameras for this redraw (replay_body_camera).
struct BodyCamera {bool valid=false,body=false; int16_t rot[2][9],view[2][9];} body_camera;
// D17AY: this redraw's in-between object poses and its place in the timeline.
struct DrawPoses {bool valid=false,tick=false; uint64_t serial=0; float alpha=0; uint32_t n=0; ObjectPose pose[max_poses];} draw_poses;
// D44A: the earthquake (earthquake.inc) in the camera of the image being
// drawn: the eye moved by dy and the view turned by yaw about the eye. cam[0]
// is what every draw sees; in first person Duke's own draw (arms, weapon,
// legs) keeps cam[1], the steady camera, as Duke 3D draws its weapon over the
// shaking world. A redraw takes its base frame's offset (render state): the
// tic's offset holds between game frames, never blended. Live, the steady
// camera comes back when the composition ends (or at the next camera update),
// in each field nothing else has changed since.
struct QuakeCamera {bool valid=false,eye=false,duke=false,live=false; CameraState cam[2];} quake_cam;
uint64_t quake_live=0,quake_redraws=0,quake_restores=0,quake_kept=0;
void write_camera(const CameraState& c,bool rotation) {
    if(rotation) for(int i=0;i<9;++i) {
        psx_mod_write_half(camera_base+2*i,(uint16_t)c.rot[i]);
        psx_mod_write_half(camera_base+0x20+2*i,(uint16_t)c.view[i]);
    }
    for(int i=0;i<3;++i) {
        psx_mod_write_word(camera_base+0x14+4*i,(uint32_t)c.pos[i]);
        psx_mod_write_word(camera_base+0x34+4*i,(uint32_t)c.view_t[i]);
        psx_mod_write_word(camera_base+0x64+4*i,(uint32_t)c.anchor[i]);
    }
}
// Yaw about the world's vertical (+Z toward +X is a turn right), then the eye
// (+0x14, the view translation +0x34 and the anchor +0x64 with it) down by dy.
void quake_turn(int16_t row[3],double c,double s) {
    const double x=row[0],z=row[2];
    row[0]=(int16_t)std::lround(std::clamp(x*c+z*s,-32768.0,32767.0));
    row[2]=(int16_t)std::lround(std::clamp(z*c-x*s,-32768.0,32767.0));
}
void quake_shake(CameraState& q,double dy,double yaw) {
    const double c=std::cos(yaw),s=std::sin(yaw);
    if(yaw!=0) {
        for(int r=0;r<3;++r) {quake_turn(q.rot+3*r,c,s);quake_turn(q.view+3*r,c,s);}
        const double ax=q.anchor[0]-q.pos[0],az=q.anchor[2]-q.pos[2];
        q.anchor[0]=q.pos[0]+(int32_t)std::lround(ax*c+az*s);
        q.anchor[2]=q.pos[2]+(int32_t)std::lround(az*c-ax*s);
    }
    const int32_t d=(int32_t)std::lround(dy);
    q.pos[1]+=d; q.view_t[1]+=d; q.anchor[1]+=d;
}
void quake_begin(bool live) {
    quake_cam={};
    double dy=0,yaw=0; bool eye=false;
    if(!earthquake_image(dy,yaw,eye)) return;
    read_camera(quake_cam.cam[1]);
    quake_cam.cam[0]=quake_cam.cam[1];
    quake_shake(quake_cam.cam[0],dy,yaw);
    write_camera(quake_cam.cam[0],true);
    // A late first-person redraw switches the rotation per draw itself
    // (replay_body_camera): its world rotation turns, Duke's stays.
    if(body_camera.valid) {
        std::memcpy(body_camera.rot[0],quake_cam.cam[0].rot,sizeof body_camera.rot[0]);
        std::memcpy(body_camera.view[0],quake_cam.cam[0].view,sizeof body_camera.view[0]);
    }
    quake_cam.valid=true; quake_cam.eye=eye; quake_cam.live=live;
    ++(live ? quake_live : quake_redraws);
}
void quake_end() {
    if(!quake_cam.valid) return;
    if(quake_cam.live) {
        CameraState now; read_camera(now);
        const CameraState& w=quake_cam.cam[quake_cam.duke];
        const CameraState& c=quake_cam.cam[1];
        const bool rot=!std::memcmp(now.rot,w.rot,sizeof now.rot) && !std::memcmp(now.view,w.view,sizeof now.view);
        // The composition derives the eye (+0x14) and the view translation
        // (+0x34) again from the anchor (0x80039c7c, +-1 rounding): within 2
        // units of the shaken eye they are ours, and get the steady eye back.
        auto near=[&](const int32_t v[3]) {for(int i=0;i<3;++i) if(std::abs(v[i]-w.pos[i])>2) return false; return true;};
        const bool pos=!std::memcmp(now.pos,w.pos,sizeof now.pos) || near(now.pos);
        const bool vt=!std::memcmp(now.view_t,w.view_t,sizeof now.view_t) || near(now.view_t);
        const bool an=!std::memcmp(now.anchor,w.anchor,sizeof now.anchor);
        if(rot) for(int i=0;i<9;++i) {psx_mod_write_half(camera_base+2*i,(uint16_t)c.rot[i]);psx_mod_write_half(camera_base+0x20+2*i,(uint16_t)c.view[i]);}
        for(int i=0;i<3;++i) {
            if(pos) psx_mod_write_word(camera_base+0x14+4*i,(uint32_t)c.pos[i]);
            if(vt) psx_mod_write_word(camera_base+0x34+4*i,(uint32_t)c.pos[i]);
            if(an) psx_mod_write_word(camera_base+0x64+4*i,(uint32_t)c.anchor[i]);
        }
        ++((rot && pos && vt && an) ? quake_restores : quake_kept);
        static const bool trace=std::getenv("DNTTK_QUAKE_TRACE")!=nullptr;
        if(trace && !(rot && pos && vt && an)) {
            std::fprintf(stderr,"[TTK quake] end kept rot=%d pos=%d vt=%d an=%d duke=%d now pos %d %d %d vt %d %d %d an %d %d %d rot0 %d %d %d | wrote pos %d %d %d vt %d %d %d an %d %d %d rot0 %d %d %d\n",rot,pos,vt,an,quake_cam.duke,
                now.pos[0],now.pos[1],now.pos[2],now.view_t[0],now.view_t[1],now.view_t[2],now.anchor[0],now.anchor[1],now.anchor[2],now.rot[0],now.rot[1],now.rot[2],
                w.pos[0],w.pos[1],w.pos[2],w.view_t[0],w.view_t[1],w.view_t[2],w.anchor[0],w.anchor[1],w.anchor[2],w.rot[0],w.rot[1],w.rot[2]);
        }
    }
    quake_cam={};
}
// D49: in a late first-person redraw Duke and his gun are drawn as if turned
// to the late view (D14H body camera), but the world, Duke's beams with it,
// is drawn about 90 ms back under that view: a beam fired while turning
// stayed where the crosshair had been. Each of Duke's view-aimed beams
// (weapon_aim.cpp records the view it was aimed from) is moved in this
// worker's copy only, by the change from that view to the one drawn: q ->
// eye + Rl^T Rs (q - shot eye). It then leaves the gun the way it did on
// screen at its shot, with Duke's motion since. Origin and start always; the
// end too unless the beam is locked on an actor (it stays on the actor); its
// angles then point from the moved origin to the end.
// Gameplay, hits and the beam's real line are untouched.
void beam_rows(const double f[3],double r[9]) {
    const double h=std::hypot(f[0],f[2]);
    const double sy=h>1e-9?f[0]/h:0.0,cy=h>1e-9?f[2]/h:1.0,sp=std::clamp(f[1],-1.0,1.0),cp=h;
    const double m[9]={cy,0,-sy,-sy*sp,cp,-cy*sp,sy*cp,sp,cy*cp};
    std::memcpy(r,m,sizeof m);
}
// The beam's angles (+0x24 yaw, +0x26 pitch, 4096 a turn; the updater's
// matrix 0x8002b1b4, direction (sin y cos p, -sin p, cos y cos p)) orient its
// drawn streak: point them from the origin record to the end, as the updater
// built them.
void beam_aim_angles(uint32_t a,uint32_t rec) {
    double w[3],n=0;
    for(int k=0;k<3;++k) {w[k]=(double)(int32_t)psx_mod_read_word(a+0x34+4*k)-(double)(int32_t)psx_mod_read_word(rec+4*k);n+=w[k]*w[k];}
    if(n<=1.0) return;
    n=std::sqrt(n);
    const double k=2048.0/3.14159265358979;
    auto unit=[&](double x){long u=std::lround(x*k)%4096; if(u>=2048) u-=4096; if(u<-2048) u+=4096; return (uint16_t)(int16_t)u;};
    psx_mod_write_half(a+0x24,unit(std::atan2(w[0],w[2])));
    psx_mod_write_half(a+0x26,unit(-std::asin(std::clamp(w[1]/n,-1.0,1.0))));
}
void beams_follow_view(const CameraState& c) {
    double rl[9];
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) n+=(double)c.rot[3*r+k]*c.rot[3*r+k];
        n=std::sqrt(n); if(n<1024) return;
        for(int k=0;k<3;++k) rl[3*r+k]=c.rot[3*r+k]/n;
    }
    static const bool trace=std::getenv("DNTTK_BEAM_FOLLOW_TRACE")!=nullptr;
    for(uint32_t i=0;i<100;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(!psx_mod_read_half(a) || psx_mod_read_byte(a+0x30)!=15 || psx_mod_read_word(a+0x10)!=duke) continue;
        uint32_t rec=0; double eye[3],fwd[3];
        if(!aim_beam_view(a,rec,eye,fwd)) continue;
        double rs[9]; beam_rows(fwd,rs);
        auto move=[&](uint32_t at) {
            double d[3],v[3];
            for(int k=0;k<3;++k) d[k]=(double)(int32_t)psx_mod_read_word(at+4*k)-eye[k];
            for(int r=0;r<3;++r) v[r]=rs[3*r]*d[0]+rs[3*r+1]*d[1]+rs[3*r+2]*d[2];
            for(int k=0;k<3;++k) psx_mod_write_word(at+4*k,(uint32_t)(int32_t)std::lround(c.pos[k]+rl[k]*v[0]+rl[3+k]*v[1]+rl[6+k]*v[2]));
        };
        const int16_t yaw0=(int16_t)psx_mod_read_half(a+0x24);
        move(rec); move(a+4);
        const bool locked=psx_mod_read_word(a+0x14)!=0;
        if(!locked) move(a+0x34);
        beam_aim_angles(a,rec);
        if(trace) std::fprintf(stderr,"[TTK beam follow] %08x lock %d eye %.0f,%.0f,%.0f -> %d,%d,%d fwd %.3f,%.3f,%.3f -> %.3f,%.3f,%.3f yaw %d -> %d\n",a,locked?1:0,eye[0],eye[1],eye[2],
            c.pos[0],c.pos[1],c.pos[2],fwd[0],fwd[1],fwd[2],rl[6],rl[7],rl[8],yaw0,(int16_t)psx_mod_read_half(a+0x24));
    }
}
// D49 part 2: the beam's drawn root on the drawn barrel. The first-person gun
// is shown in its own presentation pose (ready pose, kick, per-weapon offsets)
// with the body camera, so a beam moved with the view still started beside
// the barrel. At the list step after Duke's draw (first_person.inc, the world
// camera just restored; projectiles are drawn after the actors), every one of
// Duke's view-aimed beams gets its origin on the barrel tip Duke's draw
// recorded (view frame) under the world camera, and its angles pointed from
// it to the end. Worker copy only.
bool beam_follow_job=false;
void beam_roots_at_barrel() {
    if(!psx_mod_replay_active() || !beam_follow_job || !body_camera.valid || body_camera.body) return;
    static const bool trace=std::getenv("DNTTK_BEAM_FOLLOW_TRACE")!=nullptr;
    double vm[3];
    if(!first_person_drawn_muzzle(vm)) {if(trace) std::fprintf(stderr,"[TTK beam root] no barrel\n"); return;}
    double rl[9],e[3];
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) {const double x=(int16_t)psx_mod_read_half(camera_base+2*(3*r+k)); n+=x*x;}
        n=std::sqrt(n); if(n<1024) return;
        for(int k=0;k<3;++k) rl[3*r+k]=(int16_t)psx_mod_read_half(camera_base+2*(3*r+k))/n;
    }
    for(int k=0;k<3;++k) e[k]=(int32_t)psx_mod_read_word(camera_base+0x14+4*k);
    double q[3];
    for(int k=0;k<3;++k) q[k]=e[k]+rl[k]*vm[0]+rl[3+k]*vm[1]+rl[6+k]*vm[2];
    for(uint32_t i=0;i<100;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(!psx_mod_read_half(a) || psx_mod_read_byte(a+0x30)!=15 || psx_mod_read_word(a+0x10)!=duke) continue;
        uint32_t rec=0; double eye[3],fwd[3];
        if(!aim_beam_view(a,rec,eye,fwd)) continue;
        if(trace) std::fprintf(stderr,"[TTK beam root] %08x from %d,%d,%d to %.0f,%.0f,%.0f view %.0f,%.0f,%.0f\n",a,(int32_t)psx_mod_read_word(rec),(int32_t)psx_mod_read_word(rec+4),(int32_t)psx_mod_read_word(rec+8),q[0],q[1],q[2],vm[0],vm[1],vm[2]);
        for(int k=0;k<3;++k) {psx_mod_write_word(rec+4*k,(uint32_t)(int32_t)std::lround(q[k]));psx_mod_write_word(a+4+4*k,(uint32_t)(int32_t)std::lround(q[k]));}
        beam_aim_angles(a,rec);
    }
}
// D49C: the energy beam's look moves with the redraw's clock. The beam draw
// 0x80072868 bends each of its +0x1c strands into a wave over N segments
// (short 0x800c3b3c): strand phase from +0x40, advanced 2*(4096/N) per strand
// each draw, and alternates its stripe colours by the draw count +0x58. The
// game draws once per game frame and every redraw reloads that frame's
// memory, so between two game frames the wave and stripes stood still (about
// 30 steps a second under 120/180 Hz redraws). Here, in the worker's copy
// only, each beam's phase is moved by beam_t game frames of its own per-draw
// advance, taken the short way round a 4096 turn (2454 = -1642 at N 10 with
// 3 strands: the wave seen at the game's rate flows from the gun), and the
// draw count by whole frames on the same clock. Every beam, any owner.
uint64_t beam_anims=0;
void beams_animate(double t) {
    const int n=(int16_t)psx_mod_read_half(0x800c3b3cu);
    if(n<=0 || n>256 || !(t>-2.0 && t<2.0)) return;
    const int seg=(int16_t)(4096/n);
    for(uint32_t i=0;i<100;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(!psx_mod_read_half(a) || psx_mod_read_byte(a+0x30)!=15) continue;
        const int32_t strands=(int32_t)psx_mod_read_word(a+0x1c);
        if(strands<=0 || strands>64) continue;
        auto turn=[](long d){d%=4096; if(d>=2048) d-=4096; if(d<-2048) d+=4096; return d;};
        const long d40=turn((long)strands*2*seg), d5a=turn((long)strands*(4096/strands));
        static const bool trace=std::getenv("DNTTK_BEAM_ANIM_TRACE")!=nullptr;
        if(trace) std::fprintf(stderr,"[TTK beam anim] %08x t %.3f phase %u -> %u count %d -> %d\n",a,t,
            psx_mod_read_half(a+0x40),(uint32_t)((psx_mod_read_word(a+0x40)+(uint32_t)(int32_t)std::lround(t*d40))&0xffff),
            (int16_t)psx_mod_read_half(a+0x58),(int16_t)psx_mod_read_half(a+0x58)+(int)std::floor(t));
        psx_mod_write_word(a+0x40,psx_mod_read_word(a+0x40)+(uint32_t)(int32_t)std::lround(t*d40));
        psx_mod_write_half(a+0x5a,(uint16_t)(psx_mod_read_half(a+0x5a)+(int16_t)std::lround(t*d5a)));
        psx_mod_write_half(a+0x58,(uint16_t)(psx_mod_read_half(a+0x58)+(int16_t)std::floor(t)));
        ++beam_anims;
    }
}
#include "impact_follow.inc"
#include "bolt_barrel.inc"
#include "redraw_check.inc"
int worker_draw(CPUState* source,const void* params,uint32_t len) {
    // D23N1: one clock for everything this job draws from the time (live_env.h).
    // The lamp bake table is held the same way (lamp_bake_map.h).
    struct ClockHold {ClockHold(){const uint32_t t=(uint32_t)SDL_GetTicks();ttk_draw_clock_hold(t?t:1u);lamp_bake_pin(true);} ~ClockHold(){ttk_draw_clock_hold(0);lamp_bake_pin(false);}} clock_hold;
    const uint64_t quality_start=quality_profile()?SDL_GetPerformanceCounter():0;
    quality_actor_start=quality_actor_ticks=quality_actor_count=0;
    const uint64_t hits0=substituted,miss0=sub_misses;
    if(len<sizeof(Params)) return 0;
    Params p;
    std::memcpy(&p,params,sizeof p);
    bolt_barrel_job=(p.flags&64)!=0;   // D49E
    if(p.magic!=0x54544b32u || sizeof(Params)+(uint64_t)p.nmats*sizeof(MatPatch)>len) return 0;
    // Developer mask DNTTK_INTERP_CAM: 1 rotation, 2 anchor/distance/room,
    // 4 view matrix +0x20/+0x34, 8 eye position +0x14 (default all).
    static const unsigned cam_mask=[]{const char* t=std::getenv("DNTTK_INTERP_CAM");return t?(unsigned)std::strtoul(t,nullptr,0):15u;}();
    // D52: the camera the base frame was drawn from (a game-owned view keeps
    // the player's camera untouched); only the player's gets the body camera
    // and the room walk from Duke.
    const uint32_t cb=p.camera.base;
    if((p.flags&1) && !known_camera(cb)) p.flags&=~1u;
    const bool own=cb==camera_base;
    if(!own) p.flags&=~(2u|8u|16u);
    if(p.flags&1) {
        if(cam_mask&1) for(int i=0;i<9;++i) psx_mod_write_half(cb+2*i,(uint16_t)p.camera.rot[i]);
        if(cam_mask&2) {
            psx_mod_write_half(cb+0x42,(uint16_t)p.camera.distance);
            for(int i=0;i<3;++i) psx_mod_write_word(cb+0x64+4*i,(uint32_t)p.camera.anchor[i]);
            psx_mod_write_word(cb+0x90,p.camera.room);
        }
        if(cam_mask&4) {
            for(int i=0;i<9;++i) psx_mod_write_half(cb+0x20+2*i,(uint16_t)p.camera.view[i]);
            for(int i=0;i<3;++i) psx_mod_write_word(cb+0x34+4*i,(uint32_t)p.camera.view_t[i]);
        }
        if(cam_mask&8) for(int i=0;i<3;++i) psx_mod_write_word(cb+0x14+4*i,(uint32_t)p.camera.pos[i]);
        body_camera={}; beam_follow_job=false;
        if((p.flags&8) && (cam_mask&5)==5) {
            body_camera.valid=true;
            std::memcpy(body_camera.rot[0],p.camera.rot,sizeof p.body_rot);std::memcpy(body_camera.view[0],p.camera.view,sizeof p.body_view);
            std::memcpy(body_camera.rot[1],p.body_rot,sizeof p.body_rot);std::memcpy(body_camera.view[1],p.body_view,sizeof p.body_view);
            if(p.flags&16) {beams_follow_view(p.camera);impacts_follow_view(p.camera);}
            beam_follow_job=(p.flags&16)!=0; first_person_muzzle_reset();
        }
        // The camera update (0x8003ade4) stores the eye's room from
        // 0x80039dd0(target position, eye, target room, previous room), which
        // walks portals from Duke to the eye. Between two frames in different
        // rooms, run it for the in-between eye: portal visibility then starts
        // in the room the eye is really in (no views through door frames).
        if((p.flags&2) && (cam_mask&10)==10) {
            CPUState eye=*source;
            eye.pc=0; eye.gpr[31]=sentinel;
            eye.gpr[4]=camera_base; eye.gpr[5]=camera_base+0x14;
            psx_dispatch_call(&eye,0x80039c7c,sentinel);
            if(eye.pc!=0 || g_psx_call_bail) {g_psx_call_bail=0;return 0;}
            CPUState walk=*source;
            walk.pc=0; walk.gpr[31]=sentinel;
            walk.gpr[4]=duke+4;
            walk.gpr[5]=camera_base+0x14;
            walk.gpr[6]=(uint32_t)(int32_t)(int8_t)psx_mod_read_byte(duke+0x2e);
            walk.gpr[7]=p.camera.room;
            psx_dispatch_call(&walk,room_walk,sentinel);
            static const bool probe=[]{const char* t=std::getenv("DNTTK_ROOM_WALK");return t && !std::strcmp(t,"probe");}();
            if(walk.pc==0 && !g_psx_call_bail) {psx_mod_write_word(camera_base+0x90,probe ? walk.gpr[2]+1 : walk.gpr[2]);++room_walks;}
            g_psx_call_bail=0;
        }
    }
    // D17AY developer comparison: DNTTK_DRAW_POSE=tick keeps every draw-time
    // reader on the last game tick (the behaviour before D17AY).
    static const bool pose_tick=[]{const char* t=std::getenv("DNTTK_DRAW_POSE");return t && !std::strcmp(t,"tick");}();
    draw_poses={};
    draw_poses.valid=true; draw_poses.tick=pose_tick; draw_poses.serial=p.serial; draw_poses.alpha=p.alpha;
    draw_poses.n=std::min<uint32_t>(p.nposes,max_poses); std::memcpy(draw_poses.pose,p.pose,sizeof p.pose);
    static const bool no_actors=std::getenv("DNTTK_INTERP_NO_ACTORS")!=nullptr;
    if(no_actors) p.nmats=0;
    const uint8_t* patches=(const uint8_t*)params+sizeof(Params);
    const uint32_t render_at=sizeof(Params)+p.nmats*(uint32_t)sizeof(MatPatch);
    if(p.render_len && p.render_len==render_state_size() && render_at+p.render_len<=len)
        render_state_load((const uint8_t*)params+render_at);
    // The near clip's packet budget and held weapon packets are counted from the
    // frame's first renderer call. A worker's own copy is left over from its
    // previous job (another frame or alpha), so in-betweens could run out of
    // budget and drop polygons beside the eye that the live frame clipped: start
    // where the live frame did (D17A). DNTTK_REPLAY_NEAR_STATE=0 (developer)
    // keeps the worker's leftover state for comparison.
    static const bool near_state=[]{const char* t=std::getenv("DNTTK_REPLAY_NEAR_STATE");return !(t && t[0]=='0' && !t[1]);}();
    if(near_state) near_clip_load_frame_state(p.near);
    sub_table=nullptr; sub_n=0; sub_cursor=0;
    // Substituted matrices live in mod memory allocated here, in the worker,
    // after the image load (which resets the allocator to the live process's
    // watermark): backing Expansion 1 in the live process is guest-visible.
    scratch=psx_mod_alloc_guest_memory(0x20*scratch_n,16);
    const uint32_t xf_at=render_at+p.render_len;
    if(p.nxf && xf_at+(uint64_t)p.nxf*sizeof(Frame::Xf)<=len) {
        sub_table=(const uint8_t*)params+xf_at; sub_n=p.nxf;
    }
    for(uint32_t i=0;i<p.nmats;++i) {
        MatPatch m;
        std::memcpy(&m,patches+i*sizeof(MatPatch),sizeof m);
        for(int k=0;k<9;++k) psx_mod_write_half(m.addr+2*k,(uint16_t)m.mat.m[k]);
        for(int k=0;k<3;++k) psx_mod_write_word(m.addr+0x14+4*k,(uint32_t)m.mat.t[k]);
    }
    uint64_t nc0[8]; near_clip_counters(nc0);
    CPUState cpu=*source;
    cpu.pc=0;
    // The published state is the composition's entry: its return address is
    // the gameplay frame function's (0x800268f4) or the frontend tick's
    // (0x800234d4, D17T). Composition, then that caller's drawing tail, until
    // it calls the flip. D17U: a loading image starts at the logo draw, whose
    // backdrop is already in the drawing chain; the draw adds the field delta.
    if(p.flags&4) psx_mod_write_half(logo_angle,(uint16_t)((p.logo-(int)psx_mod_read_half(field_delta))&0xfff));
    if(!(p.flags&4)) quake_begin(false);
    if((p.flags&32) && !(p.flags&4)) beams_animate(p.beam_t);
    n1::begin(cpu,p);   // D23N1 developer check (DNTTK_N1_CHECK)
    psx_dispatch_call(&cpu,(p.flags&4) ? logo_draw : composition,flip);
    n1::at_flip(cpu);
    bool ok=cpu.pc==0 && !g_psx_call_bail;
    if(ok) {
        CPUState tail=cpu;
        tail.pc=0; tail.gpr[31]=sentinel;
        psx_dispatch_call(&tail,chain_close,sentinel);
        ok=tail.pc==0 && !g_psx_call_bail;
        if(ok) {
            tail.pc=0; tail.gpr[31]=sentinel;
            psx_dispatch_call(&tail,submit,sentinel);
            ok=tail.pc==0 && !g_psx_call_bail;
        }
    }
    n1::finish(ok);
    g_psx_call_bail=0;
    sub_table=nullptr; sub_n=0; body_camera={}; draw_poses={}; quake_cam={};
    static const bool log=std::getenv("DNTTK_REPLAY_LOG")!=nullptr;
    if(log) {
        uint64_t nc1[8]; near_clip_counters(nc1);
        uint64_t wc[4]; near_clip_water_counters(wc);
        std::fprintf(stderr,"[replay near] alpha=%.3f wtaken=%llu wtri=%llu otaken=%llu otri=%llu fallback=%llu budget=%llu overflow=%llu refused=%llu first=%08x\n",
            p.alpha,(unsigned long long)(nc1[0]-nc0[0]),(unsigned long long)(nc1[1]-nc0[1]),(unsigned long long)(nc1[2]-nc0[2]),
            (unsigned long long)(nc1[3]-nc0[3]),(unsigned long long)(nc1[4]-nc0[4]),(unsigned long long)(nc1[5]-nc0[5]),
            (unsigned long long)(nc1[6]-nc0[6]),(unsigned long long)(nc1[7]-nc0[7]),p.near.first_cursor);
        std::fprintf(stderr,"[replay water] wseen=%llu wtaken=%llu wguard=%llu wgrid=%llu\n",
            (unsigned long long)wc[0],(unsigned long long)wc[1],(unsigned long long)wc[2],(unsigned long long)wc[3]);
        std::fprintf(stderr,"[replay job] alpha=%.3f flags=%u req_pos=%d,%d,%d req_vt=%d,%d,%d rot0=%d,%d,%d after_pos=%d,%d,%d after_vt=%d,%d,%d after_rot0=%d,%d,%d mats=%u xf=%u ok=%d\n",
            p.alpha,p.flags,p.camera.pos[0],p.camera.pos[1],p.camera.pos[2],p.camera.view_t[0],p.camera.view_t[1],p.camera.view_t[2],
            p.camera.rot[0],p.camera.rot[1],p.camera.rot[2],
            (int32_t)psx_mod_read_word(camera_base+0x14),(int32_t)psx_mod_read_word(camera_base+0x18),(int32_t)psx_mod_read_word(camera_base+0x1c),
            (int32_t)psx_mod_read_word(camera_base+0x34),(int32_t)psx_mod_read_word(camera_base+0x38),(int32_t)psx_mod_read_word(camera_base+0x3c),
            (int16_t)psx_mod_read_half(camera_base),(int16_t)psx_mod_read_half(camera_base+2),(int16_t)psx_mod_read_half(camera_base+4),
            p.nmats,p.nxf,ok?1:0);
    }
    if(quality_start) {
        quality_actor_end(nullptr,0);
        const double ms=1000.0/SDL_GetPerformanceFrequency();
        std::fprintf(stderr,"[quality worker] total_ms=%.3f actor_ms=%.3f actors=%u rooms=%u xf_hit=%llu xf_miss=%llu\n",
            (SDL_GetPerformanceCounter()-quality_start)*ms,quality_actor_ticks*ms,quality_actor_count,
            psx_mod_read_word(0x800d68a8),(unsigned long long)(substituted-hits0),(unsigned long long)(sub_misses-miss0));
    }
    return ok ? 1 : 0;
}

// Image cache (runtime entries 0..5): redraws drawn ahead of their present.
// Runtime image cache entries (RP_CACHE): room for the shown frame's and the
// next frame's in-betweens at 240 Hz (7 each), so preparing the next frame
// never evicts an image still to be shown (that thrash redrew images twice).
constexpr int cache_entries=16;
uint64_t shown_serial=0;    // frame on screen at the last due present
struct Cached {uint64_t serial=0; int index=-1; uint64_t used=0;};
Cached cache[cache_entries];
// D39B: each cached redraw's screen flash, mixed at the redraw's own alpha
// between the two frames it blends, so the fade follows the same clock as
// the motion it is drawn over (and a repeated image keeps its tint).
struct CachedTint {bool on=false; float rgbw[4]{}; uint64_t serial=0; double alpha=0; uint32_t duke_word=0;};
CachedTint cache_tint[cache_entries];
uint64_t cache_clock=0, prefetched=0, governed=0, deferred=0, late_repeats=0;
int last_shown_cache=-1;   // cache entry of the last present (late camera keeps it)
double slack_floor_ms=0.5;

uint64_t menu_kept=0, menu_alone_jobs=0, menu_held=0;   // D19O diagnostics
// D19O (menu_alone below). DNTTK_MENU_ALONE=off (developer) restores the
// real-image fallback after a reset.
bool menu_alone_on() {static const bool v=[]{const char* t=std::getenv("DNTTK_MENU_ALONE");return !(t && !std::strcmp(t,"off"));}();return v;}
void reset_timeline(bool keep_shown) {
    if(!continuous_timeline()) return;
    timeline_clock_reset=true;
    for(auto& f:frames) {release_jobs(f);f.valid=false;}
    for(auto& c:cache) c=Cached{};
    rec_frame=nullptr;
    // The cache entry's image stays valid (cache_victim never reuses it).
    if(keep_shown && last_shown_cache>=0 && menu_alone_on()) ++menu_kept;
    else last_shown_cache=-1;
}
// D19O: D19C/D19E redraws are drawn from the newest captured image; the real
// image shows the same menu change one to two guest images later (measured:
// a cursor move 22 presents later at 120). After a reset (a host stall, a
// switch between the demo and the loading logo) no pair exists until the
// next image, about 100 ms, and falling back to the real image stepped the
// screen back (13 presents of PRESS START after the main menu had appeared).
// Until then the newest menu image is redrawn on its own; before its first
// redraw is ready the last redraw stays (reset_timeline keep_shown).
// D19O: while a demo scene loads behind the title menu, the logo images can
// stop for more than 15 fields (the blocking load handing back to the menu,
// long load ticks). The presenter then left the replay and showed the real
// image for about 15 presents: the logo stepped back and the menu came late.
// Within such a gap (up to 5 s, game mode 0, scene still loading) the last
// redraw is held instead; a movie, a game start or the demo running again
// ends it.
bool menu_hold() {
    return menu_alone_on() && last_shown_cache>=0 && psx_mod_replay_cache_valid(last_shown_cache) &&
        psx_mod_read_half(game_mode)==0 && psx_mod_read_half(demo_load)!=2 && menu_field && s_frame_count-menu_field<=300;
}
Frame* menu_alone() {
    if(!menu_alone_on()) return nullptr;
    Frame* n=nullptr;
    for(auto& f:frames) if(f.valid && f.xf_done && (!n || f.serial>n->serial)) n=&f;
    return n && n->frontend && n->menu_mode && (n->loading ? loading_newest() : frontend_newest()) ? n : nullptr;
}
int cache_find(uint64_t frame,int index) {
    for(int c=0;c<cache_entries;++c)
        if(cache[c].serial==frame && cache[c].index==index && psx_mod_replay_cache_valid(c)) return c;
    return -1;
}
// D17AW: the entry whose redraw is queued on the render thread, not drawn yet.
int cache_pending_find(uint64_t frame,int index) {
    for(int c=0;c<cache_entries;++c)
        if(cache[c].serial==frame && cache[c].index==index && psx_mod_replay_cache_pending(c)) return c;
    return -1;
}
int cache_victim() {
    // An empty entry, else an image of a frame already off screen, else the
    // least recently used.
    int best=-1;
    for(int c=0;c<cache_entries;++c) {
        if(c==last_shown_cache) continue;
        if(psx_mod_replay_cache_pending(c)) continue;   // D17AW: being drawn on the render thread
        if(!psx_mod_replay_cache_valid(c) || !frame_by_serial(cache[c].serial)) return c;
        if(cache[c].serial<shown_serial && (best<0 || cache[c].used<cache[best].used)) best=c;
    }
    if(best>=0) return best;
    best=last_shown_cache==0?1:0;
    for(int c=0;c<cache_entries;++c) if(c!=last_shown_cache && !psx_mod_replay_cache_pending(c) && cache[c].used<cache[best].used) best=c;
    return best;
}
// Main-thread budget for drawing redraws: a token bucket refilled at
// budget_ms per guest field (DNTTK_REPLAY_BUDGET_MS), so redraws can never
// take the time the game itself needs, wherever the tick happens.
double tokens_ms=0.0;
double feed_ms=1.5;     // recent main-thread cost of one redraw (EMA)
double window_feed_sum=0; uint64_t window_feed_n=0;  // this second's redraw feeds (replay_load_window)
uint64_t bucket_t=0;
bool budget_ok(const PSXModPresentInfo* info) {
    if(!info->frequency) return false;
    if(bucket_t) {
        const double fields=(double)(info->now-bucket_t)/(double)info->frequency*59.94;
        tokens_ms=std::min(tokens_ms+fields*budget_ms,budget_ms*2.0);
    }
    bucket_t=info->now;
    return tokens_ms>=budget_ms*0.5;
}
double due_wait_ms=4.0;   // D17AW: DNTTK_RT_DUE_WAIT_MS
// D17AW: redraws are drawn on the render thread. Queueing one costs the
// emulation thread almost nothing, so the slack and budget governors (which
// kept feeds out of the game's own time) do not hold them back.
bool rt_async() {return psx_mod_replay_render_async()!=0;}
bool render(Frame& f,int index,const PSXModPresentInfo* info,bool due=false) {
    uint64_t tag=0;
    if(f.failed[index] || f.jobs[index]<0 || psx_mod_replay_job_state(f.jobs[index],&tag)!=3 || tag!=f.serial) return false;
    // The budget governs preparation ahead of time. A late-camera image that
    // is finished in a worker and wanted by this very present is drawn anyway
    // (one feed, about 1 ms): refusing it froze the view for a present and
    // then caught up, the jerk at the dancers (2026-10-02).
    static const bool due_free=[]{const char* t=std::getenv("DNTTK_LATE_DUE_FREE");return !(t && t[0]=='0' && !t[1]);}();
    if(!(due && due_free && late_camera()) && !rt_async() && !budget_ok(info)) {++governed;return false;}
    if(due) budget_ok(info);   // keep the bucket's clock current
    if(f.submit_ms[index]>0.0) {
        note_ready((double)info->now*1000.0/(double)info->frequency-f.submit_ms[index]);
        f.submit_ms[index]=0.0;
    }
    const int c=cache_victim();
    cache[c]={0,-1,0};
    const uint64_t t0=SDL_GetPerformanceCounter();
    const bool ok=psx_mod_replay_render_job(f.jobs[index],c);
    const double ms=(double)(SDL_GetPerformanceCounter()-t0)*1000.0/(double)info->frequency;
    tokens_ms-=ms;
    feed_ms+=(ms-feed_ms)*0.2;
    window_feed_sum+=ms;++window_feed_n;
    if(!ok) {++failures;f.failed[index]=true;return false;}
    cache[c]={f.serial,index,++cache_clock};
    {
        const Frame* next=frame_by_serial(f.serial+1);
        CachedTint& t=cache_tint[c];
        t.on=flash_mix(f.flash,next?next->flash:f.flash,std::min(f.alpha[index],1.0f),screen_flash_steps(),t.rgbw);
        t.serial=f.serial;t.alpha=f.alpha[index];t.duke_word=f.duke_word;
    }
    // D17AW: on the render thread the image is only queued. A present due now
    // waits for it a little, as the single-thread path drew it right here.
    if(due && psx_mod_replay_cache_pending(c)) psx_mod_replay_wait_cache(c,due_wait_ms);
    // The image is cached: free the worker job for the next submissions.
    psx_mod_replay_release(f.jobs[index]); f.jobs[index]=-1;
    ++presented;
    return true;
}

bool late_camera();
Frame* shown_frame(uint64_t flips) {
    // Late camera: the in-betweens shown while game image k is on screen blend
    // k-1 toward k (one game frame later than the blend toward k+1), because
    // frame k+1 is often composed only after image k appears (a fresh game:
    // 5-15 ms after), too late to prepare k's in-betweens. Both frames of the
    // pair are then known a frame ahead; mouse rotation stays immediate.
    static const int lag=[]{const char* t=std::getenv("DNTTK_REPLAY_LAG");return t?std::atoi(t):-1;}();
    const int use=lag>=0 ? lag : late_camera() ? 3 : 2;
    for(auto& f:frames) if(f.valid && f.flips+(uint64_t)use==flips) return &f;
    return nullptr;
}

// Prepare the next redraws while the emulation thread has spare time.
void prefetch(const PSXModPresentInfo* info) {
    if(!rt_async() && info->slack_ms<slack_floor_ms) {++governed;return;}
    // Never start a preparation that would run past the next present.
    if(info->next_deadline && info->frequency) {
        const double to_next=info->next_deadline>info->now ? (double)(info->next_deadline-info->now)*1000.0/(double)info->frequency : 0.0;
        if(to_next<feed_ms*1.2) {++deferred;return;}
    }
    Frame* now=shown_frame(info->flips);
    Frame* next=now ? frame_by_serial(now->serial+1) : nullptr;
    // Late camera: a redraw is for one present. Feed the one for the coming
    // present (or a later one); a redraw whose present has passed will never
    // be shown, so free its worker job instead of spending main-thread time
    // on it (that starved the wanted images at 4x, 2026-10-03).
    const bool late=late_camera() && info->present_hz>0.0 && info->frequency;
    const double next_ms=late ? (double)info->next_deadline*1000.0/(double)info->frequency : 0.0;
    const double half_ms=late ? 500.0/info->present_hz : 0.0;
    for(Frame* f:{now,next}) {
        if(!f) continue;
        for(int i=0;i<f->njobs;++i) {
            if(late && f->target[i]<next_ms-half_ms) {
                if(f->jobs[i]>=0 && cache_find(f->serial,i)<0) {psx_mod_replay_release(f->jobs[i]);f->jobs[i]=-1;}
                continue;
            }
            if(cache_find(f->serial,i)>=0 || f->failed[i] || f->jobs[i]<0) continue;
            uint64_t tag=0;
            if(psx_mod_replay_job_state(f->jobs[i],&tag)!=3 || tag!=f->serial) continue;
            if(render(*f,i,info)) ++prefetched;
            return;   // one per call: keep each preparation short
        }
    }
}

// Present trace (diagnostic, always on, 128 entries): per due present the
// time since the last flip, the wanted alpha, the index shown (-1: the real
// image), how many of the frame's redraws were ready in a worker and how many
// were cached, and whether the shown one was drawn at its present.
struct TraceEntry {float ms,alpha; int8_t shown,njobs,ready,cached,drawn; uint32_t serial; float yaw; double t; float pa; float age;};
TraceEntry trace_ring[128]; unsigned trace_n=0;
// D10G3: the late view the last present showed (shot_view, weapon_aim.cpp).
// Presents and the game update run on the emulation thread.
// D10G4: with it, where that image drew each object (build_params' blend of
// the base frame toward the next at the redraw's alpha).
struct ShownView {bool ok=false; double eye[3]{}, fwd[3]{}; int16_t rot[9]{}; std::chrono::steady_clock::time_point at{}; int nposes=0; ObjectPose pose[max_poses];} shown_view_state;
void shown_poses(const Frame& f,int shown) {
    const Frame* n=frame_by_serial(f.serial+1);
    const double alpha=interp_off ? 0.0 : interp_full ? 1.0 : f.alpha[shown];
    shown_view_state.nposes=0;
    // D10H8: past the newest pose, the same limited extrapolation as the draw.
    const Frame* back=alpha>1.0 && n ? frame_by_serial(f.serial-1) : nullptr;
    if(back && (back->frontend || !back->xf_done)) back=nullptr;
    extrap.on=back!=nullptr;
    if(back) {extrap.sab=std::max(n->time_ms-f.time_ms,1.0);extrap.sza=std::max(f.time_ms-back->time_ms,1.0);extrap.ext_ms=(alpha-1.0)*extrap.sab;}
    for(int i=0;i<f.state.nposes;++i) {
        const ObjectPose& x=f.state.pose[i];
        ObjectPose o=x;
        if(n) for(int k=0;k<n->state.nposes;++k) if(n->state.pose[k].obj==x.obj) {
            const ObjectPose& y=n->state.pose[k];
            const ObjectPose* w=nullptr;
            if(back) for(int j=0;j<back->state.nposes;++j) if(back->state.pose[j].obj==x.obj) {w=&back->state.pose[j];break;}
            long dx=(long)y.pos[0]-x.pos[0],dy=(long)y.pos[1]-x.pos[1],dz=(long)y.pos[2]-x.pos[2];
            if(dx*dx+dy*dy+dz*dz<=(long)1536*1536) for(int c=0;c<3;++c) o.pos[c]=lerp_x(x.pos[c],y.pos[c],alpha,w?&w->pos[c]:nullptr);
            break;
        }
        shown_view_state.pose[shown_view_state.nposes++]=o;
    }
    extrap.on=false;
}
void trace(const PSXModPresentInfo* info,double alpha,const Frame* f,int shown,bool drawn) {
    if(f && shown>=0) {
        shown_view_state.ok=f->view_ok[shown];
        if(shown_view_state.ok) {
            for(int i=0;i<3;++i) {shown_view_state.eye[i]=f->view_eye[shown][i];shown_view_state.fwd[i]=f->view_fwd[shown][i];}
            for(int i=0;i<9;++i) shown_view_state.rot[i]=f->view_rot[shown][i];
            shown_poses(*f,shown);
        }
        shown_view_state.at=std::chrono::steady_clock::now();
    } else if(shown==-1) shown_view_state.ok=false;
    else if(shown_view_state.ok) shown_view_state.at=std::chrono::steady_clock::now();   // the same image again
    TraceEntry& e=trace_ring[trace_n++%128];
    e.ms=info->last_flip && info->frequency ? (float)((double)(info->now-info->last_flip)*1000.0/(double)info->frequency) : -1.0f;
    e.t=info->frequency ? (double)info->now/(double)info->frequency : 0.0;
    e.alpha=(float)alpha; e.shown=(int8_t)shown; e.drawn=drawn; e.serial=f?(uint32_t)f->serial:0;
    e.yaw=f && shown>=0 ? f->late_yaw[shown] : 999.0f;
    e.pa=f && shown>=0 ? f->alpha[shown] : -1.0f;
    static double last_look=0;
    if(f && shown>=0) last_look=late_camera() && f->late_yaw[shown]<900 ? f->look_time[shown] : 0;
    else if(shown==-1) last_look=0;
    e.age=last_look>0 ? (float)(e.t*1000-last_look) : -1.0f;
    e.njobs=f?(int8_t)f->njobs:0; e.ready=0; e.cached=0;
    // D19C diagnostics: the first present showing a new menu image.
    if(f && shown>=0 && f->frontend && f->menu_mode) {
        const uint32_t m=f->shows[shown];
        if(m!=shown_menu && menu_latency_n) {
            MenuLatency& l=menu_latency[(menu_latency_n-1)%32];
            if(l.screen_ms<0 && l.to==m) l.screen_ms=e.t*1000.0-l.press_ms;
        }
        shown_menu=m;
    }
    if(f) for(int i=0;i<f->njobs;++i) {
        uint64_t tag=0;
        if(f->jobs[i]>=0 && psx_mod_replay_job_state(f->jobs[i],&tag)==3 && tag==f->serial) ++e.ready;
        if(cache_find(f->serial,i)>=0) ++e.cached;
    }
}
// A frame published with flip count c is on screen from flip c+lag (its image
// is drawn after its own flip and displayed at the next one).
int timeline_provider(const PSXModPresentInfo* info) {
    static bool suspended=false;
    if(!replay_context()) {
        if(info->due && menu_hold()) {++menu_held;trace(info,0,nullptr,-2,false);return last_shown_cache+1;}
        if(!info->due && menu_hold()) return 0;
        suspended=true;return 0;
    }
    if(suspended) {reset_timeline();suspended=false;return 0;}
    if(!info->frequency || info->present_hz<=0) return 0;
    const double now_ms=1000.0*info->now/info->frequency;
    const double period=1000.0/info->present_hz;
    response_slack=info->slack_ms;
    double newest=-1e30,oldest=1e30;
    for(auto& f:frames) if(f.valid) {newest=std::max(newest,f.time_ms);oldest=std::min(oldest,f.time_ms);}
    response_pose_lag=now_ms-newest;response_oldest_age=now_ms-oldest;
    for(auto& f:frames) if(f.valid) for(int i=0;i<f.njobs;++i) {
        uint64_t tag=0;
        if(f.submit_ms[i]>0 && f.jobs[i]>=0 && psx_mod_replay_job_state(f.jobs[i],&tag)==3 && tag==f.serial) {
            note_ready(now_ms-f.submit_ms[i]);f.submit_ms[i]=0;
        }
    }
    static double adjust_ms=0;
    if(!late_lead_fixed) {
        // D17AW: plus the render thread's drawing time (0 without it).
        const double wanted=std::clamp(late_ready_ms+psx_mod_replay_render_latency_ms()+3.0,6.0,40.0);
        const double step=adjust_ms?std::clamp((now_ms-adjust_ms)*.1,0.0,1.0):0;
        late_lead_ms+=std::clamp(wanted-late_lead_ms,-step,step);
    }
    adjust_ms=now_ms;
    update_world_delay(now_ms,period);
    for(double t=1000.0*info->next_deadline/info->frequency;t-now_ms<=late_lead_ms;t+=period) {
        Frame* next=nullptr;double alpha=0;Frame* f=timeline_pair(t,next,alpha);
        if(!f && (f=menu_alone())) {next=f;alpha=0;}
        if(!f) {++ro_nopair;continue;}
        if(f->njobs>=max_jobs) continue;
        // A new capture can move this deadline into the next snapshot pair.
        // Its already-submitted job still owns the deadline: do not submit and
        // feed the same presentation twice just because the pair changed.
        bool have=false;
        for(const auto& owner:frames) if(owner.valid) {
            for(int i=0;i<owner.njobs;++i)
                if(std::fabs(owner.target[i]-t)<period*.4) {have=true;break;}
            if(have) break;
        }
        if(have) continue;
        if(!f->frontend) {++timeline_presents; if(pair_starved) ++timeline_starved; if(alpha>1.0) ++timeline_extrapolated; if(next!=f) {
            note_submit(t-now_ms,t-pair_end_ms);
            static const bool pose_trace=std::getenv("DNTTK_POSE_TRACE")!=nullptr;
            if(pose_trace) std::fprintf(stderr,"[submit] host_ms=%.3f t=%.3f lead=%.2f delay=%.2f need=%.2f period=%.3f\n",now_ms,t,late_lead_ms,timeline_delay(),t-pair_end_ms,period);
        }}
        const double look_ms=std::min(t-late_lead_ms,now_ms);
        const int job=submit_job(*f,*next,alpha,!f->frontend,look_ms);
        if(job<0) {++ro_nojob;break;}
        if(next==f) ++menu_alone_jobs;
        const int i=f->njobs++;
        f->alpha[i]=(float)alpha;f->jobs[i]=job;f->failed[i]=false;f->submitted[i]=true;
        f->shows[i]=submitted_menu;
        f->target[i]=t;f->look_time[i]=look_ms;f->late_yaw[i]=last_late_yaw;f->submit_ms[i]=now_ms;++late_jit;
    }
    if(!info->due) {
        const double next_ms=1000.0*info->next_deadline/info->frequency;
        Frame* best=nullptr;int pick=-1;double at=1e30;
        for(auto& f:frames) if(f.valid) for(int i=0;i<f.njobs;++i) {
            // Keep a just-late result for the next present. Dropping every
            // unfinished job immediately after its deadline censored the slow
            // samples from the readiness estimate: a newly expensive view
            // could repeat one image for 100+ ms while all its redraws were
            // thrown away. Bound the grace to two presents to avoid backlog.
            if(f.target[i]<now_ms-period*2) {
                if(f.jobs[i]>=0 && cache_find(f.serial,i)<0) {
                    if(f.submit_ms[i]>0) {
                        note_ready(now_ms-f.submit_ms[i]);
                        f.submit_ms[i]=0;
                    }
                    psx_mod_replay_release(f.jobs[i]);f.jobs[i]=-1;
                }
                continue;
            }
            if(f.jobs[i]<0 || cache_find(f.serial,i)>=0 || f.failed[i]) continue;
            uint64_t tag=0;
            if(f.target[i]<at && psx_mod_replay_job_state(f.jobs[i],&tag)==3 && tag==f.serial) {best=&f;pick=i;at=f.target[i];}
        }
        if(best && (rt_async() || info->slack_ms>=slack_floor_ms)) {
            if(at<=next_ms+period*.5 || next_ms-now_ms>=feed_ms*1.2)
                if(render(*best,pick,info)) ++prefetched;
        }
        return 0;
    }
    Frame* best=nullptr;int pick=-1;double at=-1;
    for(auto& f:frames) if(f.valid) for(int i=0;i<f.njobs;++i) {
        if(f.target[i]>now_ms+period*.25 || f.target[i]<=at) continue;
        uint64_t tag=0;
        if(cache_find(f.serial,i)>=0 || cache_pending_find(f.serial,i)>=0 ||
           (f.jobs[i]>=0 && !f.failed[i] && psx_mod_replay_job_state(f.jobs[i],&tag)==3 && tag==f.serial)) {best=&f;pick=i;at=f.target[i];}
    }
    if(best) {
        int c=cache_find(best->serial,pick);bool drawn=false;
        // D17AW: queued on the render thread and not finished yet. The
        // single-thread path drew it right here; wait for it a little.
        if(c<0) {
            const int p=cache_pending_find(best->serial,pick);
            if(p>=0 && psx_mod_replay_wait_cache(p,due_wait_ms)) {c=cache_find(best->serial,pick);drawn=true;}
        }
        if(c<0 && (rt_async() || info->slack_ms>=slack_floor_ms) && render(*best,pick,info,true)) {c=cache_find(best->serial,pick);drawn=true;}
        if(c>=0) {
            if(target_hz==-1) late_pace(c==last_shown_cache);
            shown_serial=best->serial;cache[c].used=++cache_clock;++reused;
            ++ro_good;ro_miss_run=0;ro_last_good=std::chrono::steady_clock::now();
            trace(info,best->alpha[pick],best,pick,drawn);last_shown_cache=c;return c+1;
        }
    }
    {
        int cls=0;
        for(auto& f:frames) if(f.valid) for(int i=0;i<f.njobs;++i) if(std::fabs(f.target[i]-now_ms)<period*.75) {
            uint64_t tag=0;const int st=f.jobs[i]>=0?psx_mod_replay_job_state(f.jobs[i],&tag):-1;
            cls=std::max(cls,f.failed[i]?3:st==3?2:1);
            if(!f.failed[i] && f.jobs[i]<0) {
                const int pc=cache_pending_find(f.serial,i);
                if(pc>=0) ++ro_miss_rt_pending; else if(cache_find(f.serial,i)<0) ++ro_miss_rt_gone;
            } else if(!f.failed[i] && st==0) ++ro_miss_free;
            else if(!f.failed[i] && st==4) ++ro_miss_wfail;
            if(!f.failed[i] && (st==1 || st==2)) {
                if(st==1) ++ro_miss_queued; else ++ro_miss_running;
                const double age=f.submit_ms[i]>0?now_ms-f.submit_ms[i]:-1;
                if(age>=0) {ro_miss_age_max=std::max(ro_miss_age_max,age);ro_miss_age_sum+=age;++ro_miss_age_n;}
            }
        }
        ++ro_miss_cls[cls];
    }
    ro_good=0;++ro_miss_run;
    if(last_shown_cache>=0 && psx_mod_replay_cache_valid(last_shown_cache)) {
        ++late_repeats;if(target_hz==-1) late_pace(true);
        trace(info,0,nullptr,-2,false);return last_shown_cache+1;
    }
    ++no_job;trace(info,0,nullptr,-1,false);return 0;
}
int provider(const PSXModPresentInfo* info,void*) {
    if(!interp_replay || !input_modernized() || !info) return 0;
    // D34B: native split screen retains alternate viewports in live VRAM.
    // Never display a stale single-view/loading replay over the running match.
    static bool split_screen=false;
    const bool split=psx_mod_read_word(0x800c27bc)==2 && psx_mod_read_half(game_mode)!=0;
    if(split!=split_screen) {reset_timeline();frontend_field=loading_field=0;split_screen=split;}
    if(split) return 0;
    if(info->due) pace_minute();
    if(info->present_hz>0.0) seen_hz=info->present_hz;
    seen_last_flip=info->last_flip; seen_frequency=info->frequency;
    if(info->flip_interval>0.02 && info->flip_interval<0.15) seen_interval+=(info->flip_interval-seen_interval)*0.05;
    static const bool nofeed=std::getenv("DNTTK_REPLAY_NOFEED")!=nullptr;
    if(nofeed) return 0;
    if(continuous_timeline()) return timeline_provider(info);
    submit_due(info,shown_frame(info->flips));
    if(!info->due) {prefetch(info);return 0;}
    Frame* shown=shown_frame(info->flips);
    if(!shown || !info->frequency || !info->last_flip) {++no_job;return 0;}
    shown_serial=shown->serial;
    const double interval=info->flip_interval>0.02 && info->flip_interval<0.15 ? info->flip_interval : 2.0/59.94;
    const double alpha=(double)(info->now-info->last_flip)/(double)info->frequency/interval;
    // Developer check: DNTTK_REPLAY_TEST=alpha0 always shows the first redraw
    // (which, without interpolation patches, must equal the real image).
    static const bool test0=[]{const char* t=std::getenv("DNTTK_REPLAY_TEST");return t && !std::strcmp(t,"alpha0");}();
    const double now_ms=(double)info->now*1000.0/(double)info->frequency;
    const double half_ms=info->present_hz>0.0 ? 500.0/info->present_hz : 2.0;
    {const int a=(int)std::min<uint64_t>(serial-shown->serial,3)*2+(rec_frame?1:0); ++ahead_hist[a];}
    for(int i=shown->njobs-1;i>=0;--i) {
        // Late camera: the redraw made for this present, else the newest
        // earlier one that is ready.
        if(late_camera() && !test0) {if(shown->target[i]>now_ms+half_ms) continue;}
        else if(!test0 && shown->alpha[i]>alpha+1e-6) continue;
        if(test0 && i!=0) continue;
        int c=cache_find(shown->serial,i);
        static const bool nodue=std::getenv("DNTTK_REPLAY_NODUE")!=nullptr;
        bool drawn=false;
        if(c<0 && !nodue) {   // D17AW: queued on the render thread
            const int p=cache_pending_find(shown->serial,i);
            if(p>=0 && psx_mod_replay_wait_cache(p,due_wait_ms)) {c=cache_find(shown->serial,i);drawn=true;}
        }
        if(c<0 && !nodue && (rt_async() || info->slack_ms>=slack_floor_ms) && render(*shown,i,info,true)) {c=cache_find(shown->serial,i);drawn=true;}
        if(c>=0) {
            // The same image twice in a row is a repeat (pacing counts it).
            if(late_camera() && !test0) late_pace(c==last_shown_cache);
            cache[c].used=++cache_clock;++reused;trace(info,alpha,shown,i,drawn);last_shown_cache=c;
            return c+1;
        }
    }
    // Late camera: the real image carries the older game camera, so showing it
    // would swing the view back; repeat the last presented redraw instead.
    if(late_camera() && last_shown_cache>=0 && psx_mod_replay_cache_valid(last_shown_cache) && cache[last_shown_cache].serial+1>=shown->serial) {
        ++late_repeats;late_pace(true);trace(info,alpha,shown,-2,false);return last_shown_cache+1;
    }
    trace(info,alpha,shown,-1,false);
    if(late_camera()) late_pace(true);
    last_shown_cache=-1;
    ++no_job;
    return 0;
}

// D39B: Polymorph's present tint (Modernized; Classic and Vanilla never).
// A replay image carries its own tint; anything else shows the live record.
int present_tint(int slot,float rgbw[4],void*) {
    if(screen_flash_classic() || !input_modernized()) return 0;
    bool on=false;
    if(slot>=1 && slot<=cache_entries) {
        const CachedTint& t=cache_tint[slot-1];
        if((on=t.on)) std::memcpy(rgbw,t.rgbw,sizeof t.rgbw);
    } else on=screen_flash_live(rgbw);
    on=on && flash_option_rgbw(screen_flash_option(),rgbw);   // D39C: SCREEN FLASHES
    screen_flash_note(slot,rgbw,on);
    // D39E developer trace: per present, the game frame its picture is
    // composed from, the blend toward the next, Duke frozen in that frame, the tint.
    static const bool trace=[]{const char* t=std::getenv("DNTTK_FLASH_TRACE");return t && t[0]=='1';}();
    if(trace && slot>=1 && slot<=cache_entries) {
        const CachedTint& t=cache_tint[slot-1];
        std::fprintf(stderr,"[TTK tint] frame %llu alpha %.3f frozen %d w %.3f\n",(unsigned long long)t.serial,t.alpha,
            (t.duke_word&0x20000u)?1:0,on?rgbw[3]:0.f);
    }
    return on;
}

bool modernized_session=false;   // D19G1: FRAME RATE can start replay later
bool slack_fixed=false;
// The present rate's own settings, at a launch and on a FRAME RATE change.
void apply_rate(int32_t hz) {
    target_hz=hz;
    // In-between images above 60 by default (off: DNTTK_FRAME_INTERP=off);
    // 30 shows each game image once and never redraws.
    const char* interp=std::getenv("DNTTK_FRAME_INTERP");
    interp_replay=!(interp && !std::strcmp(interp,"off")) && (hz<0 || hz==0 || hz>60);
    // Past idle time is not a present deadline. With the optimized rendering
    // path, fixed-rate camera images must not be withheld by that stale EMA.
    // Unlimited retains the guard so overload feeds its adaptive cadence.
    if(!slack_fixed) slack_floor_ms=continuous_timeline() && hz!=-1 ? 0.0 : 0.5;
    pace_div=1;pace_clean=0;pace_backoff=1;
    psx_mod_set_present_divisor(1);
}

void activate() {
    const char* mode=std::getenv("DNTTK_INPUT_MODE");
    if(!mode || std::strcmp(mode,"modernized")) return;
    modernized_session=true;
    if(const char* t=std::getenv("DNTTK_INTERP_STEPS")) {steps=std::clamp(std::atoi(t),1,max_steps);steps_fixed=true;}
    if(const char* t=std::getenv("DNTTK_REPLAY_TEST")) {
        interp_off=!std::strcmp(t,"alpha0") || !std::strcmp(t,"interp_off");
        interp_full=!std::strcmp(t,"alpha1");
    }
    if(const char* t=std::getenv("DNTTK_REPLAY_SLACK_MS")) {slack_floor_ms=std::clamp(std::atof(t),0.0,16.0);slack_fixed=true;}
    if(const char* t=std::getenv("DNTTK_LATE_LEAD_MS")) {late_lead_ms=std::clamp(std::atof(t),2.0,40.0);late_lead_fixed=true;}
    if(const char* t=std::getenv("DNTTK_REPLAY_BUDGET_MS")) {budget_ms=std::clamp(std::atof(t),0.5,16.0);budget_fixed=true;}
    if(const char* t=std::getenv("DNTTK_RT_DUE_WAIT_MS")) due_wait_ms=std::clamp(std::atof(t),0.0,8.0);
    psx_mod_set_replay_status(status_json);
    psx_mod_set_present_tint(present_tint,nullptr);
    // D17AW: Polymorph draws its redraws on the host render thread, so the
    // emulation thread only emulates. Classic keeps the single-thread path;
    // DNTTK_RENDER_THREAD=0 (developer) does too.
    psx_mod_set_render_thread(render_thread_wanted());
    const int32_t hz=parse_rate(std::getenv("DNTTK_FRAME_RATE"));
    if(hz==60) return;
    apply_rate(hz);
    replay_on=psx_mod_set_present_replay(provider,nullptr,target_hz)!=0;
}

// Workers fork from a running game, after the window and renderer exist.
bool workers_started=false;
void start_workers_once() {
    if(workers_started || !replay_on || !interp_replay || !replay_context()) return;
    // The game initializes this math routine before gameplay, but may not call
    // it until the opening has begun. Load its exact cached variant now so a
    // first use does not force all render workers to refork during camera motion.
    (void)overlay_loader_prepare_address(0x800B3D9Cu);
    near_clip_prepare();
    render_state_prepare();
    aim_render_prepare();
    impact_follow_prepare();
    workers_started=true;
    psx_mod_replay_workers_visual_timing(1);
    int workers=3;
    if(const char* t=std::getenv("DNTTK_REPLAY_WORKERS")) workers=std::clamp(std::atoi(t),1,8);
    if(psx_mod_replay_workers_start(workers,worker_draw)<=0) interp_replay=false;
}
void composition_entry(CPUState* cpu,uint32_t address) {
    // D19: our OPTIONS screens are drawn first, so this image's capture has them.
    if(!psx_mod_replay_active() && address==composition) game_menu_frame(cpu);
    if(!psx_mod_replay_active() && address==composition && replay_on && interp_replay && frontend_composition(cpu))
        frontend_field=s_frame_count;
    if(!psx_mod_replay_active()) start_workers_once();
    // The live frame that just ended drew these actors.
    if(!psx_mod_replay_active() && address==composition) {
        std::memcpy(drawn,drawing,sizeof(uint32_t)*(size_t)ndrawing);
        std::memcpy(drawn_kind,drawing_kind,sizeof(uint32_t)*(size_t)ndrawing);
        ndrawn=ndrawing;
        ndrawing=0;
    }
    // D44A: this image's earthquake offset is fixed before the capture (its
    // render state carries it to the redraws) and shown after.
    const bool quake=!psx_mod_replay_active() && address==composition && cpu->gpr[31]==composition_ret;
    if(quake) {quake_end();earthquake_compose();}
    composition_hook(cpu,address);
    if(quake) quake_begin(true);
}
// D17U: a loading screen's logo draw (live frames only).
void logo_entry(CPUState* cpu,uint32_t address) {
    // D19: the menu over the loading logo (once per image; see game_menu.cpp).
    if(!psx_mod_replay_active() && address==logo_draw) game_menu_frame(cpu);
    if(address!=logo_draw || psx_mod_replay_active() || !replay_on || !interp_replay || !loading_image(cpu)) return;
    // Never a second image in a tick that already captured its composition.
    if(rec_frame && rec_frame->valid && !rec_frame->loading) return;
    loading_field=s_frame_count;
    start_workers_once();
    capture(cpu,loading_image_kind);
}
// The object-list loop draws each object through table 0x800c0bf8 by its kind
// (byte +0x14): 0x800632b0, 0x80031d10, 0x80032e78, 0x800348d8 (Duke and
// matrix actors), 0x80031c14, all called (camera, object, ...).
uint32_t drawing_kind[max_actors];
void actor_draw(CPUState* cpu,uint32_t address) {
    // D52: objects drawn through the composition's camera (a game-owned view
    // too); a mirror's stacked camera stays out.
    if(!interp_replay || cpu->gpr[4]!=composition_camera()) return;
    xf_obj=cpu->gpr[5]; xf_seq=0;
    if(psx_mod_replay_active()) {
        if(quality_profile()) {quality_actor_end(nullptr,0);quality_actor_start=SDL_GetPerformanceCounter();++quality_actor_count;}
        return;
    }
    const uint32_t actor=cpu->gpr[5];
    for(int i=0;i<ndrawing;++i) if(drawing[i]==actor) return;
    if(ndrawing<max_actors) {drawing_kind[ndrawing]=address;drawing[ndrawing++]=actor;}
}
// 0x800292a0(camera, matrix): one transform load inside an object's draw.
// Live frames record it; a worker's redraw substitutes the interpolated one.
void transform_load(CPUState* cpu,uint32_t) {
    if(!interp_replay || cpu->gpr[4]!=composition_camera()) return;
    const uint32_t sky_ra=cpu->gpr[31];
    // D52: the sky is built around whichever camera draws it.
    const bool sky=sky_transform_call(camera_base,sky_ra);
    static SkyRenderIdentity sky_identity;
    static const bool sky_native=[]{const char* t=std::getenv("DNTTK_SKY_CAMERA");return !(t && !std::strcmp(t,"0"));}();
    static const bool sky_trace=std::getenv("DNTTK_SKY_TRACE")!=nullptr;
    if(sky && sky_native && sky_identity.valid(input_host_frame(),g_dirty_ram_code_gen,g_psx_ram)) {
        // xf_obj still names the last world object after its draw returns.
        // Recording the sky under that key made workers replace its freshly
        // calculated eye with an older world-space translation. With late
        // mouse input that displacement can exceed the cloud band's radius.
        // Let the original routine construct BOTH rotation and translation
        // from this worker's camera and original sky timer. Never interpolate
        // the camera-facing backdrop as an independent world object either.
        static unsigned logs=0;
        if(sky_trace && psx_mod_replay_active() && logs++<120) {
            const Mat m=read_mat(cpu->gpr[5]);
            std::fprintf(stderr,"[sky-native] ra=%08x offset=%d,%d,%d phase=%u\n",sky_ra,
                m.t[0]-(int)psx_mod_read_word(camera_base+0x14),
                m.t[1]-(int)psx_mod_read_word(camera_base+0x18),
                m.t[2]-(int)psx_mod_read_word(camera_base+0x1c),psx_mod_read_word(0x800c0c64));
        }
        return;
    }
    if(!xf_obj) return;
    const uint16_t seq=xf_seq++;
    const uint32_t m=cpu->gpr[5];
    if(!psx_mod_replay_active()) {
        if(!rec_frame || rec_frame->xf_done || rec_frame->nxf>=Frame::max_xf) return;
        if(!(m>=0x80010000u && m+0x20u<=0x80200000u) && !((m&0x1fffffffu)>=0x1f000000u)) return;
        rec_frame->xf[rec_frame->nxf++]={xf_obj,seq,0,read_mat(m)};
        return;
    }
    if(!sub_table || !scratch || xf_obj==duke) return;
    for(uint32_t k=0;k<sub_n;++k) {
        const uint32_t i=(sub_cursor+k)%sub_n;
        Frame::Xf x; std::memcpy(&x,sub_table+i*sizeof(Frame::Xf),sizeof x);
        if(x.obj!=xf_obj || x.seq!=seq) continue;
        static unsigned sky_logs=0;
        if(sky && sky_trace && sky_logs<120) {
            const Mat now=read_mat(m);
            if(std::abs(now.t[0]-x.m.t[0])+std::abs(now.t[1]-x.m.t[1])+std::abs(now.t[2]-x.m.t[2])>10) {
                ++sky_logs;
                std::fprintf(stderr,"[sky-xf] ra=%08x obj=%08x seq=%u now=%d,%d,%d sub=%d,%d,%d eye=%d,%d,%d\n",sky_ra,xf_obj,seq,now.t[0],now.t[1],now.t[2],x.m.t[0],x.m.t[1],x.m.t[2],(int)psx_mod_read_word(camera_base+0x14),(int)psx_mod_read_word(camera_base+0x18),(int)psx_mod_read_word(camera_base+0x1c));
            }
        }
        const uint32_t dst=scratch+0x20u*(i%scratch_n);
        for(int k=0;k<9;++k) psx_mod_write_half(dst+2*k,(uint16_t)x.m.m[k]);
        psx_mod_write_half(dst+18,0);
        for(int k=0;k<3;++k) psx_mod_write_word(dst+0x14+4*k,(uint32_t)x.m.t[k]);
        cpu->gpr[5]=dst;
        sub_cursor=(i+1)%sub_n;
        ++substituted;
        return;
    }
    ++sub_misses;
}
// 0x8001fba0 called right after the live composition (return 0x80026904):
// this frame's transforms are complete; redraw the previous frame toward it.
void composition_end(CPUState* cpu,uint32_t) {
    if(!psx_mod_replay_active() && cpu->gpr[31]==0x80026904u) quake_end();
    if(!interp_replay || psx_mod_replay_active() || !rec_frame) return;
    if(cpu->gpr[31]!=(rec_frame->frontend ? frontend_close_ret : 0x80026904u)) return;
    rec_frame->xf_done=true;
    static const bool pose_trace=std::getenv("DNTTK_POSE_TRACE")!=nullptr;
    if(pose_trace) std::fprintf(stderr,"[pose-ready] serial=%llu host_ms=%.3f\n",
        (unsigned long long)rec_frame->serial,
        1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency());
    xf_obj=0;
    if(Frame* prev=frame_by_serial(rec_frame->serial-1)) {
        if(!rec_frame->frontend && !prev->frontend) note_pair_age(1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency()-prev->time_ms);
        if(prev->xf_done) queue(*prev,*rec_frame);
    }
    rec_frame=nullptr;
}
} // namespace
// D14H: Duke's own actor draw in a late first-person redraw uses the body
// camera; every other draw (world, reflection, other actors) the late one.
void replay_quake_camera(bool duke) {
    if(!quake_cam.valid || !quake_cam.eye || quake_cam.duke==duke) return;
    quake_cam.duke=duke;
    write_camera(quake_cam.cam[duke],!body_camera.valid);
}
void replay_quake_restore() {if(!psx_mod_replay_active()) quake_end();}
void replay_beam_roots() {beam_roots_at_barrel();}
void replay_body_camera(bool duke) {
    if(!body_camera.valid || body_camera.body==duke) return;
    body_camera.body=duke;
    for(int i=0;i<9;++i) {
        psx_mod_write_half(camera_base+2*i,(uint16_t)body_camera.rot[duke][i]);
        psx_mod_write_half(camera_base+0x20+2*i,(uint16_t)body_camera.view[duke][i]);
    }
}

// D17AY: an object's position (+4..+12) and heading (+0x1c, 4096 a turn) as
// this draw shows it. A redraw between two game ticks gets the in-between
// value captured with the frames (true); a live draw, or an object neither
// frame recorded, reads guest memory (false). yaw may be null.
bool replay_redraw_active() {return draw_poses.valid;}
bool shot_view(double eye[3],double forward[3],double& age_ms) {
    if(!shown_view_state.ok) return false;
    age_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-shown_view_state.at).count();
    for(int i=0;i<3;++i) {eye[i]=shown_view_state.eye[i];forward[i]=shown_view_state.fwd[i];}
    return true;
}
// D49F: the rows of the view shot_view returned (right, down, forward), each
// normalized as the D49 beam roots normalize the draw's camera rows.
bool shot_view_rows(double rows[9]) {
    if(!shown_view_state.ok) return false;
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) n+=(double)shown_view_state.rot[3*r+k]*shown_view_state.rot[3*r+k];
        n=std::sqrt(n); if(n<1024) return false;
        for(int k=0;k<3;++k) rows[3*r+k]=shown_view_state.rot[3*r+k]/n;
    }
    return true;
}
int shot_view_poses(uint32_t obj[],double pos[][3],int max) {
    if(!shown_view_state.ok) return 0;
    const int n=std::min(max,shown_view_state.nposes);
    for(int i=0;i<n;++i) {obj[i]=shown_view_state.pose[i].obj;for(int k=0;k<3;++k) pos[i][k]=shown_view_state.pose[i].pos[k];}
    return n;
}
// D17AY diagnostic: a trace reference reads the in-between pose even under
// DNTTK_DRAW_POSE=tick, so the old and new readers are measured alike.
bool present_trace_reference=false;
bool draw_object_pose(uint32_t obj,double pos[3],double* yaw) {
    if(!draw_poses.tick || present_trace_reference) for(uint32_t i=0;i<draw_poses.n;++i) if(draw_poses.pose[i].obj==obj) {
        const ObjectPose& o=draw_poses.pose[i];
        for(int k=0;k<3;++k) pos[k]=o.pos[k];
        if(yaw) *yaw=o.yaw&0xfff;
        return true;
    }
    for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(obj+4+4*k);
    if(yaw) *yaw=psx_mod_read_half(obj+0x1c)&0xfff;
    return false;
}
// D17AY developer diagnostic, DNTTK_PRESENT_TRACE=1: a presented world point
// relative to the view of the draw that shows it (camera eye +0x14 and view
// rows +0x20 as loaded for this draw), one stderr line per redraw with its
// base frame and fraction. A point that steps at the game tick against the
// interpolated view jumps at each new base frame while moving evenly within
// one (recomp/tools/local/present_trace.py measures that).
bool present_trace_enabled() {static const bool on=std::getenv("DNTTK_PRESENT_TRACE")!=nullptr;return on;}
void present_trace(const char* name,const double world[3]) {
    if(!present_trace_enabled() || !draw_poses.valid) return;
    double d[3],v[3];
    for(int i=0;i<3;++i) d[i]=world[i]-(int32_t)psx_mod_read_word(camera_base+0x14+4*i);
    for(int r=0;r<3;++r) {
        v[r]=0;
        for(int k=0;k<3;++k) v[r]+=(int16_t)psx_mod_read_half(camera_base+0x20+2*(3*r+k))/4096.0*d[k];
    }
    std::fprintf(stderr,"[present] s=%llu a=%.4f body=%d n=%s v=%.2f,%.2f,%.2f\n",(unsigned long long)draw_poses.serial,
        draw_poses.alpha,body_camera.valid && body_camera.body ? 1 : 0,name,v[0],v[1],v[2]);
}

// A presented scalar (an opacity, a blend), logged as v=value,0,1.
void present_trace_value(const char* name,double value) {
    if(!present_trace_enabled() || !draw_poses.valid) return;
    std::fprintf(stderr,"[present] s=%llu a=%.4f body=%d n=%s v=%.5f,0,1\n",(unsigned long long)draw_poses.serial,
        draw_poses.alpha,body_camera.valid && body_camera.body ? 1 : 0,name,value);
}

// D35D1: while Duke's body camera is current in a late redraw, the late
// camera's rotation, so a contact fixed to the world (the push grip) can be
// placed in the body view where the late camera sees the object.
bool replay_late_rotation(int16_t rot[9]) {
    if(!body_camera.valid || !body_camera.body) return false;
    std::memcpy(rot,body_camera.rot[0],sizeof body_camera.rot[0]);
    return true;
}

// D19G: FRAME RATE confirmed in the in-game OPTIONS. D19G1: from any launch
// rate. 60 stops replay presentation and returns to the original present path
// (as a 60 launch); another rate starts it, the render workers at the next
// composition. False when it cannot apply now (software renderer).
bool frame_rate_live() {return modernized_session && (replay_on || psx_mod_present_replay_available());}
bool frame_rate_set(const char* value) {
    if(!modernized_session) return false;
    const int32_t hz=parse_rate(value);
    if(hz==60) {
        if(replay_on && !psx_mod_request_present_replay(nullptr,nullptr,60)) return false;
        replay_on=interp_replay=false;
        target_hz=60;
        reset_timeline();
    } else if(!replay_on) {
        if(!psx_mod_request_present_replay(provider,nullptr,hz)) return false;
        reset_timeline();
        apply_rate(hz);
        replay_on=true;
    } else {
        if(!psx_mod_request_present_replay_rate(hz)) return false;
        apply_rate(hz);
    }
    std::fprintf(stderr,"[TTK replay] frame rate %s\n",value?value:"60");
    return true;
}

// D23E/D23H: emulation load shedding. Late-camera redraws run on the
// emulation thread at every present (D23E: about 1.4 ms each, 19% of the
// thread at 120 Hz in the busy western town), so when the game falls behind
// real time presents go to every 2nd, then 3rd refresh, evenly, before the
// overclock safety net gives up the CPU speed.
//
// D23H: one controller owns both directions, judged once per second of
// gameplay by overclock_lease (modern_controls.cpp), in every frame-rate mode.
// Before, a shed could only be undone by late_pace, which the default present
// timeline runs at Unlimited only, so at 120 Hz or Match Display one savestate
// save (about 90 ms) or F7 menu visit kept 60 or 40 presents until restart.
// - Shed only when two seconds in a row fall behind: a one-off hitch (save,
//   load, menu, worker start) is a single bad second and sheds nothing.
// - After 5 s x backoff clean seconds, try the next faster rate. If it falls
//   behind again within 10 s the probe failed: back down, backoff doubles (at
//   most 16: one probe every 80 s in a scene that cannot hold it).
// - Backoff halves after each clean minute at the full rate, so a session's
//   history does not keep the game slower than the scene needs.
// DNTTK_REPLAY_SHED=0 (developer) never sheds, to measure a scene at full rate.
static bool shed_enabled() {static const bool v=[]{const char* t=std::getenv("DNTTK_REPLAY_SHED");return !(t && t[0]=='0' && !t[1]);}();return v;}
uint64_t audio_sheds=0;
static void shed_step(double feed,const char* why) {
    const bool failed_probe=shed_since_probe<=10;
    if(failed_probe) {shed_backoff=std::min(shed_backoff*2,16);++shed_failed_probes;}
    ++pace_div;++shed_steps;pace_clean=0;pace_presents=pace_bad=0;psx_mod_set_present_divisor(pace_div);++pace_changes;++emu_sheds;
    shed_feed_ms=feed;shed_audio=false;
    shed_since_probe=1000;
    pace_log(failed_probe?"emulation behind, faster rate failed":why,0,shed_backoff);
}
int replay_load_window(int window) {
    if(!shed_enabled()) window=window==1 ? 2 : window;
    if(!replay_on || !interp_replay || !late_camera()) {shed_bad=shed_clean=0;return window==1 ? -1 : 0;}
    shed_steps=std::min(shed_steps,pace_div-1);  // Unlimited's late_pace shares the divisor
    ++shed_since_probe;
    const double feed=window_feed_n>=10 ? window_feed_sum/(double)window_feed_n : 0.0;
    window_feed_sum=0;window_feed_n=0;
    // Not a gameplay second (menu, pause, load): not judged. D50: it no
    // longer wipes the clean run either; a pause is no evidence of a slow
    // scene (a pause menu held the 11:26 session at 90 for 2 minutes).
    if(window==2) {shed_bad=0;return 0;}
    if(window==1) {
        shed_clean=0;shed_full_clean=0;
        // D17AR3 part 4: a step up that falls behind in its first seconds goes
        // back after one slow second, so a probe costs the game less speed.
        if(++shed_bad<(shed_since_probe<=10 ? 1 : 2)) return 0;
        shed_bad=0;
        if(pace_div>=(target_hz==-1 ? 16 : 3)) return -1;
        shed_step(feed,"emulation behind");
        return 1;
    }
    shed_bad=0;
    if(shed_steps>0) {
        // D17AR3 part 4: step back up once the view's redraws are clearly
        // cheaper than when it shed (turned away from a heavy vista), or
        // after a long clean run. Probing the same heavy view slowed the game
        // for a second each time (slot 10's jetpack view at 180 Hz).
        const bool lighter=feed>0.0 && shed_feed_ms>0.0 && feed<shed_feed_ms*0.75;
        // D50: a shed for the audio ring steps back up after 5 clean seconds,
        // like a lighter view. The ring refills only slowly (the output's
        // rate correction, at most 0.5%), so its level is no guide; a scene
        // that still cannot keep up drains it again and sheds again (and the
        // failed probe doubles the backoff).
        if(++shed_clean>=(lighter || shed_audio ? 5 : 30)*shed_backoff) {
            --pace_div;--shed_steps;shed_clean=0;shed_since_probe=0;++shed_probes;
            pace_clean=0;pace_presents=pace_bad=0;psx_mod_set_present_divisor(pace_div);++pace_changes;
            pace_log("emulation keeping up, trying faster",0,shed_backoff);
        }
    } else if(pace_div==1 && shed_backoff>1 && ++shed_full_clean>=60) {shed_backoff/=2;shed_full_clean=0;}
    return 0;
}

// D17AQ: the audio ring is draining (overclock_lease saw it under a third
// of its fill target). The emulation has been behind long enough to be heard, so
// shed one step now instead of waiting for two slow seconds; the 3 s after
// a savestate load are not exempt. Slot 10's jetpack vista at 180 Hz ran at
// 44-50 fields/s through that window and the 180 ms ring went dry: crackle.
// The second's feed so far is the shed's reference for stepping back up.
int replay_audio_low() {
    if(!shed_enabled() || !replay_on || !interp_replay || !late_camera()) return 0;
    shed_steps=std::min(shed_steps,pace_div-1);
    if(pace_div>=(target_hz==-1 ? 16 : 3)) return 0;
    const double feed=window_feed_n ? window_feed_sum/(double)window_feed_n : shed_feed_ms;
    shed_bad=shed_clean=shed_full_clean=0;
    shed_step(feed,"audio running low");shed_audio=true;
    ++audio_sheds;
    return 1;
}

// D17AR3: the live frame in progress skips its drawing (host_visibility.inc).
uint64_t ro_drops_total() {return ro_drops;}  // D50 hitch log
bool live_frame_redraw_only() {return live_redraw_only && !psx_mod_replay_active();}
const char* redraw_only_json() {
    static char buffer[384];uint64_t st[11];psx_mod_live_draw_skip_stats(st);
    std::snprintf(buffer,sizeof buffer,"{\"kept\":[%llu,%llu,%llu,%llu,%llu],\"switch\":%s,\"engaged\":%s,\"frames\":%llu,\"engages\":%llu,\"drops\":%llu,\"lists\":%llu,\"prims\":%llu,\"held\":%llu,\"blind\":%d,\"good\":%u}",
        (unsigned long long)st[4],(unsigned long long)st[5],(unsigned long long)st[6],(unsigned long long)st[7],(unsigned long long)st[8],
        redraw_only_switch()?"true":"false",redraw_only_engaged?"true":"false",(unsigned long long)ro_frames,(unsigned long long)ro_engages,(unsigned long long)ro_drops,
        (unsigned long long)st[0],(unsigned long long)st[1],(unsigned long long)st[2],psx_mod_live_draw_blind(),ro_good);
    return buffer;
}
bool redraw_only_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"redrawonly") && std::strncmp(line,"redrawonly ",11)) return false;
    close=false;
    if(line[10]) {
        const char* v=line+11;
        if(!std::strcmp(v,"on")) redraw_only_override=1;
        else if(!std::strcmp(v,"off")) redraw_only_override=0;
        else if(!std::strcmp(v,"default")) redraw_only_override=-1;
        else {say("Usage: redrawonly [on|off|default] (developer, session only)");return true;}
    }
    say(redraw_only_switch()?"Redraw-only live frames on (developer).":"Redraw-only live frames off.");
    return true;
}
// D40: the world renderer in game, as EDuke32's setrendermode. Applies at the
// next flip; the OPTIONS row saves the choice, the console does not.
bool world_renderer_request(bool classic) {
    if(!modernized_session) return false;
    world_request.store(classic?1:0);
    return true;
}
const char* world_renderer_json() {
    static char buffer[192];
    const int q=world_request.load();
    std::snprintf(buffer,sizeof buffer,"{\"renderer\":\"%s\",\"pending\":\"%s\",\"switches\":%llu,\"render_thread\":%s}",
        ttk_world_classic()?"classic":"polymorph",q<0?"":q?"classic":"polymorph",(unsigned long long)world_switches,
        psx_mod_replay_render_async()?"true":"false");
    return buffer;
}
bool renderer_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"renderer") && std::strncmp(line,"renderer ",9)) return false;
    close=false;
    if(line[8]) {
        const char* v=line+9;
        const bool classic=!std::strcmp(v,"classic");
        if(!classic && std::strcmp(v,"polymorph")) {say("Usage: renderer [polymorph|classic] (this session; OPTIONS > DISPLAY saves it)");return true;}
        if(!world_renderer_request(classic)) {say("The renderer switch needs the Modernized profile.");return true;}
        say(classic?"Renderer: Classic.":"Renderer: Polymorph.");
        return true;
    }
    say(ttk_world_classic()?"Renderer: Classic.":"Renderer: Polymorph.");
    return true;
}
extern "C" void gl_renderer_present_tint_diag(uint64_t* asked,uint64_t* drawn,float last[4]);
const char* quake_json() {
    static char b[512];
    std::snprintf(b,sizeof b,"{\"controls\":%s,\"live\":%llu,\"redraws\":%llu,\"restores\":%llu,\"kept\":%llu}",earthquake_json(),
        (unsigned long long)quake_live,(unsigned long long)quake_redraws,(unsigned long long)quake_restores,(unsigned long long)quake_kept);
    return b;
}
const char* impact_json() {
    static char b[256];
    std::snprintf(b,sizeof b,"{\"on\":%s,\"glows\":%llu,\"sparks\":%llu,\"glow_misses\":%llu,\"bolt_barrel\":%s,\"bolts_at_barrel\":%llu}",impact_follow_enabled()?"true":"false",
        (unsigned long long)impact_glows,(unsigned long long)impact_sparks,(unsigned long long)impact_glow_misses,bolt_barrel_enabled()?"true":"false",(unsigned long long)bolt_moves);
    return b;
}
const char* frame_replay_debug_json() {
    static char buffer[98304];
    uint64_t tint_asked=0,tint_drawn=0; float tint_last[4]{};
    gl_renderer_present_tint_diag(&tint_asked,&tint_drawn,tint_last);
    int n=std::snprintf(buffer,sizeof buffer,"{\"renderer\":%s,\"flash\":%s,\"quake\":%s,\"impact\":%s,\"on\":%s,\"interp\":%s,\"target_hz\":%d,\"steps\":%d,\"captures\":%llu,\"publishes\":%llu,"
        "\"submits\":%llu,\"rendered\":%llu,\"shown\":%llu,\"no_job\":%llu,\"failures\":%llu,\"serial\":%llu,\"prefetched\":%llu,\"governed\":%llu,\"deferred\":%llu,\"feed_ms\":%.2f,\"budget_ms\":%.2f,\"blended_mats\":%llu,\"blended_poses\":%llu,\"skipped_actors\":%llu,\"camera_cuts\":%llu,\"game_camera_blends\":%llu,\"drawn\":%d,\"pending_flips\":%llu,\"blended_xf\":%llu,\"skipped_objects\":%llu,\"substituted\":%llu,\"sub_misses\":%llu,\"late\":%s,\"late_applied\":%llu,\"late_skipped\":%llu,\"late_jit\":%llu,\"late_repeats\":%llu,\"lead_ms\":%.1f,\"ready_ms\":%.1f,\"pace_div\":%d,\"pace_backoff\":%d,\"pace_clean\":%d,\"pace_ratio\":%.3f,\"pace_mid\":%llu,\"shed_steps\":%d,\"shed_backoff\":%d,\"shed_probes\":%llu,\"shed_failed_probes\":%llu,\"pace_changes\":%llu,\"emu_sheds\":%llu,\"audio_sheds\":%llu,\"ahead\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu]}",
        world_renderer_json(),screen_flash_json(tint_asked,tint_drawn,tint_last),quake_json(),impact_json(),replay_on?"true":"false",interp_replay?"true":"false",(int)target_hz,steps,(unsigned long long)captures,
        (unsigned long long)publishes,(unsigned long long)submits,(unsigned long long)presented,(unsigned long long)reused,
        (unsigned long long)no_job,(unsigned long long)failures,(unsigned long long)serial,
        (unsigned long long)prefetched,(unsigned long long)governed,(unsigned long long)deferred,feed_ms,budget_ms,(unsigned long long)blended_mats,(unsigned long long)blended_poses,
        (unsigned long long)skipped_actors,(unsigned long long)camera_cuts,(unsigned long long)game_camera_blends,ndrawn,(unsigned long long)pending_flips,(unsigned long long)blended_xf,(unsigned long long)skipped_objects,
        (unsigned long long)substituted,(unsigned long long)sub_misses,late_camera()?"true":"false",
        (unsigned long long)late_applied,(unsigned long long)late_skipped,(unsigned long long)late_jit,(unsigned long long)late_repeats,late_lead_ms,late_ready_ms,pace_div,pace_backoff,pace_clean,pace_last_ratio,(unsigned long long)pace_mid,shed_steps,shed_backoff,(unsigned long long)shed_probes,(unsigned long long)shed_failed_probes,(unsigned long long)pace_changes,(unsigned long long)emu_sheds,(unsigned long long)audio_sheds,
        (unsigned long long)ahead_hist[0],(unsigned long long)ahead_hist[1],(unsigned long long)ahead_hist[2],(unsigned long long)ahead_hist[3],
        (unsigned long long)ahead_hist[4],(unsigned long long)ahead_hist[5],(unsigned long long)ahead_hist[6],(unsigned long long)ahead_hist[7]);
    // Append the present trace: "ms/alpha/shown/ready/cached/njobs[d]/serial/yaw/t/planned alpha".
    if(n>1 && n<(int)sizeof buffer-64) {
        n-=1;
        n+=std::snprintf(buffer+n,sizeof buffer-n,",\"redraw_only\":%s",redraw_only_json());
        n+=std::snprintf(buffer+n,sizeof buffer-n,",\"timeline\":{\"slack_ms\":%.2f,\"pose_lag_ms\":%.2f,\"oldest_age_ms\":%.2f,\"future_evictions\":%llu,\"world_delay_ms\":%.1f,\"world_delay_wanted_ms\":%.1f,\"world_delay_fixed\":%s,\"pair_age_p99_ms\":%.1f,\"presents\":%llu,\"starved\":%llu,\"margin_scale\":%.2f,\"submit_samples\":%u,\"ahead_ms\":[%.2f,%.2f,%.2f,%.2f],\"need_ms\":[%.2f,%.2f,%.2f,%.2f],\"world_draw\":%d,\"extrap_ms\":%.1f,\"extrapolated\":%llu},\"frontend\":{\"live\":%s,\"captures\":%llu,\"resets\":%llu,\"interval_ms\":%.1f,\"delay_ms\":%.1f,\"input\":%s,\"newest\":%s,\"kept\":%llu,\"alone\":%llu,\"held\":%llu,\"game_menu\":%s,\"menu\":%s},\"loading\":{\"live\":%s,\"captures\":%llu,\"resets\":%llu,\"interval_ms\":%.1f},\"trace\":\"",response_slack,response_pose_lag,response_oldest_age,(unsigned long long)response_future_evictions,world_delay_ms,world_delay_wanted,world_delay_fixed?"true":"false",pair_age_p99,(unsigned long long)timeline_presents,(unsigned long long)timeline_starved,
            world_delay_margin_scale,submit_sample_count,
            sample_pct(submit_ahead_samples,submit_sample_count,0),sample_pct(submit_ahead_samples,submit_sample_count,500),sample_pct(submit_ahead_samples,submit_sample_count,990),sample_pct(submit_ahead_samples,submit_sample_count,1000),
            sample_pct(submit_need_samples,submit_sample_count,500),sample_pct(submit_need_samples,submit_sample_count,990),sample_pct(submit_need_samples,submit_sample_count,999),sample_pct(submit_need_samples,submit_sample_count,1000),world_draw,world_extrap_ms,(unsigned long long)timeline_extrapolated,
            frontend_live()?"true":"false",(unsigned long long)frontend_captures,(unsigned long long)frontend_resets,frontend_interval_ms,newest_frontend(),ttk_frontend_input_json(),frontend_newest()?"true":"false",(unsigned long long)menu_kept,(unsigned long long)menu_alone_jobs,(unsigned long long)menu_held,ttk_game_menu_json(),menu_latency_json(),
            loading_live()?"true":"false",(unsigned long long)loading_captures,(unsigned long long)loading_resets,loading_interval_ms);
        const unsigned first=trace_n>128?trace_n-128:0;
        for(unsigned i=first;i<trace_n && n<(int)sizeof buffer-200;++i) {
            const TraceEntry& e=trace_ring[i%128];
            n+=std::snprintf(buffer+n,sizeof buffer-n,"%.1f/%.2f/%d/%d/%d/%d%s/%u/%.4f/%.6f/%.2f/%.2f ",e.ms,e.alpha,e.shown,e.ready,e.cached,e.njobs,e.drawn?"d":"",e.serial,e.yaw,e.t,e.pa,e.age);
        }
        n+=std::snprintf(buffer+n,sizeof buffer-n,"\",\"queued\":\"");
        const unsigned qf=queue_n>32?queue_n-32:0;
        for(unsigned i=qf;i<queue_n && n<(int)sizeof buffer-40;++i)
            n+=std::snprintf(buffer+n,sizeof buffer-n,"%u@%.1f ",queue_ring[i%32].serial,queue_ring[i%32].ms);
        std::snprintf(buffer+n,sizeof buffer-n,"\"}");
    }
    return buffer;
}
// D49E: "boltbarrel [on|off|default]", session-only A/B of the Laser
// Gatling's bolts drawn from the drawn barrel versus the original launch point.
bool bolt_barrel_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"boltbarrel") && std::strncmp(line,"boltbarrel ",11)) return false;
    close=false;
    if(line[10]) {
        const char* value=line+11;
        if(!std::strcmp(value,"on")) bolt_barrel_override.store(1,std::memory_order_relaxed);
        else if(!std::strcmp(value,"off")) bolt_barrel_override.store(0,std::memory_order_relaxed);
        else if(!std::strcmp(value,"default")) bolt_barrel_override.store(-1,std::memory_order_relaxed);
        else {say("Usage: boltbarrel [on|off|default] (on: Laser Gatling bolts leave the drawn barrel; session only)");return true;}
        close=true;
    }
    char text[160];std::snprintf(text,sizeof text,"Bolt barrel %s: Laser Gatling bolts are drawn %s (session only). Default on.",
        bolt_barrel_enabled()?"on":"off",bolt_barrel_enabled()?"from the drawn barrel":"from the original launch point");
    say(text);return true;
}
} // namespace ttk

PSX_MOD_CONSTRUCTOR(register_ttk_frame_replay) {
    psx_mod_register_activation_plugin("ttk.frame_rate",ttk::activate);
    psx_mod_register_function_entry_plugin("ttk.frame.replay",ttk::composition,ttk::composition_entry);
    psx_mod_register_function_entry_plugin("ttk.frame.replay",ttk::logo_draw,ttk::logo_entry);
    for(uint32_t a:{0x800632b0u,0x80031d10u,0x80032e78u,0x800348d8u,0x80031c14u})
        psx_mod_register_function_entry_plugin("ttk.frame.replay",a,ttk::actor_draw);
    psx_mod_register_function_entry_plugin("ttk.frame.replay.profile",0x8001ca4cu,ttk::quality_actor_end);
    // D49E: after the transform's record/substitute step, so the bolt's drawn
    // streak is changed in this draw's own copy only.
    psx_mod_register_function_entry_plugin("ttk.frame.replay.xf",0x800292a0u,[](CPUState* cpu,uint32_t fn){ttk::transform_load(cpu,fn);ttk::bolt_at_barrel(cpu);});
    psx_mod_register_function_entry_plugin("ttk.frame.replay",0x8001fba0u,ttk::composition_end);
    psx_mod_register_function_entry_plugin("ttk.frame.replay",ttk::flip,ttk::redraw_only_flip);
}
