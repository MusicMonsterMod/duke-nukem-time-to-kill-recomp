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
// rate: display (the monitor's refresh), 30, 60 (the original presentation,
// no replays), 120, 144, 165, 180, 240 or unlimited. Vanilla always runs 60.
// DNTTK_FRAME_INTERP=off disables the redraws (developer, documentation/80).
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "modern_controls.h"
#include "weapon_aim.h"
#include "near_clip.h"
#include "sky_render.h"
#include "psx_sdl.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
extern int g_psx_call_bail;
int overlay_loader_prepare_address(uint32_t addr);
}

extern "C" uint64_t s_frame_count;
extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
namespace ttk {
const char* frame_replay_debug_json();
namespace {
const char* status_json() {return frame_replay_debug_json();}
constexpr uint32_t composition=0x80026164, composition_ret=0x800268f4;
constexpr uint32_t flip=0x8001fcbc, chain_close=0x8001fba0, submit=0x8001fa78;
constexpr uint32_t sentinel=0x800000fc;
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
struct CameraState {int16_t rot[9]; int16_t distance; int32_t anchor[3]; uint32_t room;
                   int16_t view[9]; int32_t view_t[3]; int32_t pos[3];};
struct InterpState {
    CameraState camera{};
    bool has_pivot=false; double pivot[3]{};   // orbit pivot (late camera, third person)
    int nactors=0;
    ActorState actor[max_actors];
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
void read_camera(CameraState& c) {
    for(int i=0;i<9;++i) c.rot[i]=(int16_t)psx_mod_read_half(camera_base+2*i);
    c.distance=(int16_t)psx_mod_read_half(camera_base+0x42);
    for(int i=0;i<3;++i) c.anchor[i]=(int32_t)psx_mod_read_word(camera_base+0x64+4*i);
    c.room=psx_mod_read_word(camera_base+0x90);
    for(int i=0;i<9;++i) c.view[i]=(int16_t)psx_mod_read_half(camera_base+0x20+2*i);
    for(int i=0;i<3;++i) c.view_t[i]=(int32_t)psx_mod_read_word(camera_base+0x34+4*i);
    for(int i=0;i<3;++i) c.pos[i]=(int32_t)psx_mod_read_word(camera_base+0x14+4*i);
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
    read_camera(st.camera);
    st.has_pivot=late_pivot(st.pivot);
    st.nactors=0;
    auto add=[&](uint32_t actor){
        for(int i=0;i<st.nactors;++i) if(st.actor[i].actor==actor) return;
        if(st.nactors<max_actors && read_actor(actor,st.actor[st.nactors])) ++st.nactors;
    };
    add(duke);
    for(int i=0;i<ndrawn;++i) add(drawn[i]);
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
    int njobs=0;
    InterpState state;
    uint8_t render[8192];   // modern_controls draw-time state (render_state_save)
    NearClipFrameState near{};   // near clip frame accounting at composition entry
    // Transforms the live composition loaded through 0x800292a0 (camera,
    // matrix), keyed by drawn object and call index within its draw.
    struct Xf {uint32_t obj; uint16_t seq, pad; Mat m;};
    static constexpr int max_xf=512;
    Xf xf[max_xf];
    int nxf=0;
    bool xf_done=false;
};
Frame frames[slots];
double response_slack=0,response_pose_lag=0,response_oldest_age=0;
uint64_t response_future_evictions=0;
uint64_t serial=0;
uint64_t captures=0,publishes=0,submits=0,presented=0,reused=0,no_job=0,failures=0;

// Parameters shipped with a job (worker side reads them).
struct Params {
    uint32_t magic;
    float alpha;
    uint32_t flags;          // 1 = camera fields present
    uint32_t nmats;          // matrix patches following the header
    CameraState camera;
    uint32_t render_len;     // draw-time host state after the patches
    uint32_t nxf;            // transform substitutions after the render state
    NearClipFrameState near; // D17A: the live frame's near clip accounting
};
struct MatPatch {uint32_t addr; Mat mat;};
constexpr uint32_t params_max=60000;
uint64_t blended_mats=0,skipped_actors=0,camera_cuts=0,pending_flips=0,blended_xf=0,skipped_objects=0,substituted=0,sub_misses=0;
Frame* rec_frame=nullptr;      // live frame whose transforms are being recorded
uint32_t xf_obj=0; uint16_t xf_seq=0;
uint32_t scratch=0;            // mod memory ring for substituted matrices
constexpr int scratch_n=64;

// Build the job parameters for base frame a toward frame b at alpha.
uint32_t build_params(const InterpState& a,const InterpState& b,double alpha,uint8_t* out) {
    Params p{};
    p.magic=0x54544b32u;
    p.alpha=(float)alpha;
    // Camera: a jump in the anchor (cut, teleport) keeps the base camera.
    CameraState c=a.camera;
    long dx=(long)b.camera.anchor[0]-a.camera.anchor[0],dy=(long)b.camera.anchor[1]-a.camera.anchor[1],dz=(long)b.camera.anchor[2]-a.camera.anchor[2];
    const bool cut=dx*dx+dy*dy+dz*dz>(long)2048*2048;
    if(!cut && blend_rot(a.camera.rot,b.camera.rot,alpha,c.rot)) {
        for(int i=0;i<3;++i) c.anchor[i]=lerp_i(a.camera.anchor[i],b.camera.anchor[i],alpha);
        c.distance=(int16_t)lerp_i(a.camera.distance,b.camera.distance,alpha);
        // Through a doorway the in-between eye's room is found in the worker
        // by the game's own portal walk (flag 2); until then keep the base room.
        c.room=a.camera.room;
        // Developer check DNTTK_ROOM_WALK=always|off.
        static const int walk_mode=[]{const char* t=std::getenv("DNTTK_ROOM_WALK");return !t?1:(!std::strcmp(t,"always")||!std::strcmp(t,"probe"))?2:!std::strcmp(t,"off")?0:1;}();
        if(walk_mode==2 || (walk_mode==1 && a.camera.room!=b.camera.room)) p.flags|=2;
        if(walk_mode==0) c.room=alpha<0.5 ? a.camera.room : b.camera.room;
        if(!blend_view(a.camera.view,b.camera.view,alpha,c.view))
            for(int i=0;i<9;++i) c.view[i]=(int16_t)lerp_i(a.camera.view[i],b.camera.view[i],alpha);
        for(int i=0;i<3;++i) c.view_t[i]=lerp_i(a.camera.view_t[i],b.camera.view_t[i],alpha);
        for(int i=0;i<3;++i) c.pos[i]=lerp_i(a.camera.pos[i],b.camera.pos[i],alpha);
        p.camera=c;
        p.flags|=1;
    } else ++camera_cuts;
    uint32_t used=sizeof(Params);
    for(int i=0;i<a.nactors;++i) {
        const ActorState& x=a.actor[i];
        const ActorState* y=nullptr;
        for(int k=0;k<b.nactors;++k) if(b.actor[k].actor==x.actor) {y=&b.actor[k];break;}
        if(!y || y->mats!=x.mats || y->model!=x.model || y->joints!=x.joints) {++skipped_actors;continue;}
        MatPatch patch[max_joints];
        bool ok=true;
        for(int j=0;j<x.joints && ok;++j) {
            const Mat& m0=x.mat[j];const Mat& m1=y->mat[j];
            long tx=(long)m1.t[0]-m0.t[0],ty=(long)m1.t[1]-m0.t[1],tz=(long)m1.t[2]-m0.t[2];
            if(tx*tx+ty*ty+tz*tz>(long)1536*1536) {ok=false;break;}
            patch[j].addr=x.mats+0x20u*j;
            patch[j].mat=m0;
            if(!blend_rot(m0.m,m1.m,alpha,patch[j].mat.m)) {ok=false;break;}
            for(int k=0;k<3;++k) patch[j].mat.t[k]=lerp_i(m0.t[k],m1.t[k],alpha);
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
uint32_t append_xf(const Frame& a,const Frame& b,double alpha,uint8_t* out,uint32_t used) {
    Params p; std::memcpy(&p,out,sizeof p);
    int i=0;
    while(i<a.nxf) {
        const uint32_t obj=a.xf[i].obj;
        int n=0; while(i+n<a.nxf && a.xf[i+n].obj==obj) ++n;
        int j=-1;
        for(int k=0;k<b.nxf;++k) if(b.xf[k].obj==obj) {j=k;break;}
        int nb=0; if(j>=0) while(j+nb<b.nxf && b.xf[j+nb].obj==obj) ++nb;
        bool ok=j>=0 && nb==n && obj!=duke;
        Frame::Xf blend[Frame::max_xf];
        for(int k=0;k<n && ok;++k) {
            const Mat& m0=a.xf[i+k].m; const Mat& m1=b.xf[j+k].m;
            long tx=(long)m1.t[0]-m0.t[0],ty=(long)m1.t[1]-m0.t[1],tz=(long)m1.t[2]-m0.t[2];
            if(tx*tx+ty*ty+tz*tz>(long)1536*1536) {ok=false;break;}
            blend[k]=a.xf[i+k];
            if(!blend_rot(m0.m,m1.m,alpha,blend[k].m.m)) {ok=false;break;}
            for(int c=0;c<3;++c) blend[k].m.t[c]=lerp_i(m0.t[c],m1.t[c],alpha);
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
    if(!text || !*text) return 60;
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
double timeline_delay() {
    static const double ms=[]{const char* e=std::getenv("DNTTK_WORLD_DELAY_MS");return e?std::clamp(std::atof(e),50.0,130.0):90.0;}();
    return ms;
}
Frame* timeline_pair(double present_ms, Frame*& next, double& alpha) {
    const double world_ms=present_ms-timeline_delay();
    Frame* best=nullptr; Frame* oldest=nullptr;
    next=nullptr;
    for(auto& f:frames) if(f.valid && f.xf_done) {
        Frame* n=frame_by_serial(f.serial+1);
        if(!n || !n->xf_done || n->time_ms<=f.time_ms) continue;
        if(!oldest || f.time_ms<oldest->time_ms) oldest=&f;
        if(f.time_ms<=world_ms && (!best || f.time_ms>best->time_ms)) best=&f;
    }
    if(!best) best=oldest;
    if(!best) return nullptr;
    next=frame_by_serial(best->serial+1);
    alpha=std::clamp((world_ms-best->time_ms)/(next->time_ms-best->time_ms),0.0,1.0);
    return best;
}
bool timeline_clock_reset=true;
void reset_timeline();
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
uint64_t shed_probes=0, shed_failed_probes=0;  // D23H: windows between 2% and 10%
// D23H: every present-rate change goes to the session log with its reason,
// and late_pace prints a summary each minute (presents/s, rate, backoff).
void pace_log(const char* why,double ratio,int backoff) {
    static unsigned n=0; if(++n>200) return;
    std::fprintf(stderr,"[TTK pace] presents every %d refresh%s (%s, repeats %.1f%%, backoff %d)\n",pace_div,pace_div>1?"es":"",why,ratio*100.0,backoff);
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
    t0=now;n0=presents;w0=pace_windows;m0=pace_mid;c0=pace_changes;
}
void late_pace(bool bad) {
    static const bool on=[]{const char* t=std::getenv("DNTTK_LATE_PACING");return !(t && t[0]=='0' && !t[1]);}();
    if(!on) return;
    ++pace_presents; if(bad) ++pace_bad;
    if(pace_presents<90) return;
    const double ratio=(double)pace_bad/(double)pace_presents;
    pace_presents=pace_bad=0; ++pace_windows; pace_last_ratio=ratio;
    if(ratio>0.10 && pace_div<(target_hz<0 ? 16 : 3)) {
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
    if(!(p.flags&1) || !late_view(ly,lp,look_ms)) return false;
    CameraState& c=p.camera;
    double ra[9];
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) n+=(double)c.rot[3*r+k]*c.rot[3*r+k];
        n=std::sqrt(n); if(n<1024) return false;
        for(int k=0;k<3;++k) ra[3*r+k]=c.rot[3*r+k]/n;
    }
    static const double test_yaw=[]{const char* t=std::getenv("DNTTK_LATE_TEST_YAW");return t?std::atof(t)*3.14159265358979/180:0.0;}();
    ly+=test_yaw;   // developer check: every redraw turned by this many degrees
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
    return true;
}
// Build and submit one redraw job of base toward next at alpha.
int submit_job(Frame& base,const Frame& next,double alpha,bool late,double look_ms) {
    uint32_t len=build_params(base.state,next.state,interp_off ? 0.0 : interp_full ? 1.0 : alpha,param_buffer);
    {
        Params p; std::memcpy(&p,param_buffer,sizeof p);
        p.near=base.near;
        last_late_yaw=999.0f;
        if(late) {if(apply_late(base.state,next.state,alpha,p,look_ms)) ++late_applied; else ++late_skipped;}
        std::memcpy(param_buffer,&p,sizeof p);
    }
    len=append_render(base,param_buffer,len);
    len=append_xf(base,next,interp_off ? 0.0 : interp_full ? 1.0 : alpha,param_buffer,len);
    const int job=psx_mod_replay_submit(base.slot,param_buffer,len,base.serial);
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
        const double wanted=std::clamp(late_ready_ms+3.0,10.0,40.0);
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
            const int job=submit_job(*f,*to,alpha,true,look_ms);
            if(job<0) return;   // no free worker job: try again at the next tick
            const int n=f->njobs++;
            f->alpha[n]=(float)alpha; f->failed[n]=false; f->jobs[n]=job; f->submitted[n]=true;
            f->look_time[n]=look_ms; f->late_yaw[n]=last_late_yaw; f->target[n]=t; f->submit_ms[n]=now_ms; ++late_jit;
            static const bool log=std::getenv("DNTTK_LATE_LOG")!=nullptr;
            if(log) std::fprintf(stderr,"[late] serial %llu k %d t %+.1f now %+.1f end %+.1f alpha %.2f pending %d n %d\n",
                (unsigned long long)f->serial,k,t-flip_ms,now_ms-flip_ms,end_ms-flip_ms,alpha,pending?1:0,n);
        }
    }
}

void composition_hook(CPUState* cpu,uint32_t address) {
    if(address!=composition || !replay_on || !interp_replay || psx_mod_replay_active()) return;
    if(cpu->gpr[31]!=composition_ret || psx_mod_read_word(0x800c27bc)!=1) return;
    if(continuous_timeline() && !replay_gameplay_context()) return;
    ++captures;
    static uint64_t load_epoch=~uint64_t(0),field0=0;
    static double host0=0;
    const uint64_t field=s_frame_count, epoch=psx_mod_savestate_loads();
    const double host_now=1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency();
    if(epoch!=load_epoch || field<field0 || timeline_clock_reset || std::fabs(host_now-(host0+(double)(field-field0)*(1000.0/59.94)))>100.0) {
        reset_timeline(); load_epoch=epoch; field0=field;
        host0=host_now; timeline_clock_reset=false;
    }
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
    f->valid=true;
    capture_state(f->state);
    static const bool pose_trace=std::getenv("DNTTK_POSE_TRACE")!=nullptr;
    if(pose_trace) std::fprintf(stderr,
        "[pose] serial=%llu field=%llu pose_ms=%.3f host_ms=%.3f eye=%d,%d,%d\n",
        (unsigned long long)f->serial,(unsigned long long)field,pose_ms,host_now,
        f->state.camera.pos[0],f->state.camera.pos[1],f->state.camera.pos[2]);
    static const bool nostate=std::getenv("DNTTK_REPLAY_NOSTATE")!=nullptr;
    if(!nostate && render_state_size()<=sizeof f->render) render_state_save(f->render);
    f->near=near_clip_frame_state();
    // Record this frame's transforms; redraws of the previous frame are queued
    // when this composition ends (composition_end), with both sets complete.
    f->nxf=0; f->xf_done=false;
    rec_frame=f;
}

// Worker process: draw a published frame (memory and CPU already loaded).
const uint8_t* sub_table=nullptr; uint32_t sub_n=0, sub_cursor=0;
int worker_draw(CPUState* source,const void* params,uint32_t len) {
    const uint64_t quality_start=quality_profile()?SDL_GetPerformanceCounter():0;
    quality_actor_start=quality_actor_ticks=quality_actor_count=0;
    const uint64_t hits0=substituted,miss0=sub_misses;
    if(len<sizeof(Params)) return 0;
    Params p;
    std::memcpy(&p,params,sizeof p);
    if(p.magic!=0x54544b32u || sizeof(Params)+(uint64_t)p.nmats*sizeof(MatPatch)>len) return 0;
    // Developer mask DNTTK_INTERP_CAM: 1 rotation, 2 anchor/distance/room,
    // 4 view matrix +0x20/+0x34, 8 eye position +0x14 (default all).
    static const unsigned cam_mask=[]{const char* t=std::getenv("DNTTK_INTERP_CAM");return t?(unsigned)std::strtoul(t,nullptr,0):15u;}();
    if(p.flags&1) {
        if(cam_mask&1) for(int i=0;i<9;++i) psx_mod_write_half(camera_base+2*i,(uint16_t)p.camera.rot[i]);
        if(cam_mask&2) {
            psx_mod_write_half(camera_base+0x42,(uint16_t)p.camera.distance);
            for(int i=0;i<3;++i) psx_mod_write_word(camera_base+0x64+4*i,(uint32_t)p.camera.anchor[i]);
            psx_mod_write_word(camera_base+0x90,p.camera.room);
        }
        if(cam_mask&4) {
            for(int i=0;i<9;++i) psx_mod_write_half(camera_base+0x20+2*i,(uint16_t)p.camera.view[i]);
            for(int i=0;i<3;++i) psx_mod_write_word(camera_base+0x34+4*i,(uint32_t)p.camera.view_t[i]);
        }
        if(cam_mask&8) for(int i=0;i<3;++i) psx_mod_write_word(camera_base+0x14+4*i,(uint32_t)p.camera.pos[i]);
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
    cpu.gpr[31]=composition_ret;
    // Composition, then the frame function's drawing tail, until it calls the flip.
    psx_dispatch_call(&cpu,composition,flip);
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
    g_psx_call_bail=0;
    sub_table=nullptr; sub_n=0;
    static const bool log=std::getenv("DNTTK_REPLAY_LOG")!=nullptr;
    if(log) {
        uint64_t nc1[8]; near_clip_counters(nc1);
        std::fprintf(stderr,"[replay near] alpha=%.3f wtaken=%llu wtri=%llu otaken=%llu otri=%llu fallback=%llu budget=%llu overflow=%llu refused=%llu first=%08x\n",
            p.alpha,(unsigned long long)(nc1[0]-nc0[0]),(unsigned long long)(nc1[1]-nc0[1]),(unsigned long long)(nc1[2]-nc0[2]),
            (unsigned long long)(nc1[3]-nc0[3]),(unsigned long long)(nc1[4]-nc0[4]),(unsigned long long)(nc1[5]-nc0[5]),
            (unsigned long long)(nc1[6]-nc0[6]),(unsigned long long)(nc1[7]-nc0[7]),p.near.first_cursor);
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
uint64_t cache_clock=0, prefetched=0, governed=0, deferred=0, late_repeats=0;
int last_shown_cache=-1;   // cache entry of the last present (late camera keeps it)
double slack_floor_ms=0.5;

void reset_timeline() {
    if(!continuous_timeline()) return;
    timeline_clock_reset=true;
    for(auto& f:frames) {release_jobs(f);f.valid=false;}
    for(auto& c:cache) c=Cached{};
    rec_frame=nullptr;last_shown_cache=-1;
}
int cache_find(uint64_t frame,int index) {
    for(int c=0;c<cache_entries;++c)
        if(cache[c].serial==frame && cache[c].index==index && psx_mod_replay_cache_valid(c)) return c;
    return -1;
}
int cache_victim() {
    // An empty entry, else an image of a frame already off screen, else the
    // least recently used.
    int best=-1;
    for(int c=0;c<cache_entries;++c) {
        if(c==last_shown_cache) continue;
        if(!psx_mod_replay_cache_valid(c) || !frame_by_serial(cache[c].serial)) return c;
        if(cache[c].serial<shown_serial && (best<0 || cache[c].used<cache[best].used)) best=c;
    }
    if(best>=0) return best;
    best=last_shown_cache==0?1:0;
    for(int c=0;c<cache_entries;++c) if(c!=last_shown_cache && cache[c].used<cache[best].used) best=c;
    return best;
}
// Main-thread budget for drawing redraws: a token bucket refilled at
// budget_ms per guest field (DNTTK_REPLAY_BUDGET_MS), so redraws can never
// take the time the game itself needs, wherever the tick happens.
double tokens_ms=0.0;
double feed_ms=1.5;     // recent main-thread cost of one redraw (EMA)
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
bool render(Frame& f,int index,const PSXModPresentInfo* info,bool due=false) {
    uint64_t tag=0;
    if(f.failed[index] || f.jobs[index]<0 || psx_mod_replay_job_state(f.jobs[index],&tag)!=3 || tag!=f.serial) return false;
    // The budget governs preparation ahead of time. A late-camera image that
    // is finished in a worker and wanted by this very present is drawn anyway
    // (one feed, about 1 ms): refusing it froze the view for a present and
    // then caught up, the jerk at the dancers (2026-10-02).
    static const bool due_free=[]{const char* t=std::getenv("DNTTK_LATE_DUE_FREE");return !(t && t[0]=='0' && !t[1]);}();
    if(!(due && due_free && late_camera()) && !budget_ok(info)) {++governed;return false;}
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
    if(!ok) {++failures;f.failed[index]=true;return false;}
    cache[c]={f.serial,index,++cache_clock};
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
    if(info->slack_ms<slack_floor_ms) {++governed;return;}
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
void trace(const PSXModPresentInfo* info,double alpha,const Frame* f,int shown,bool drawn) {
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
    if(!replay_gameplay_context()) {suspended=true;return 0;}
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
        const double wanted=std::clamp(late_ready_ms+3.0,6.0,40.0);
        const double step=adjust_ms?std::clamp((now_ms-adjust_ms)*.1,0.0,1.0):0;
        late_lead_ms+=std::clamp(wanted-late_lead_ms,-step,step);
    }
    adjust_ms=now_ms;
    for(double t=1000.0*info->next_deadline/info->frequency;t-now_ms<=late_lead_ms;t+=period) {
        Frame* next=nullptr;double alpha=0;Frame* f=timeline_pair(t,next,alpha);
        if(!f || f->njobs>=max_jobs) continue;
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
        const double look_ms=std::min(t-late_lead_ms,now_ms);
        const int job=submit_job(*f,*next,alpha,true,look_ms);
        if(job<0) break;
        const int i=f->njobs++;
        f->alpha[i]=(float)alpha;f->jobs[i]=job;f->failed[i]=false;f->submitted[i]=true;
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
        if(best && info->slack_ms>=slack_floor_ms) {
            if(at<=next_ms+period*.5 || next_ms-now_ms>=feed_ms*1.2)
                if(render(*best,pick,info)) ++prefetched;
        }
        return 0;
    }
    Frame* best=nullptr;int pick=-1;double at=-1;
    for(auto& f:frames) if(f.valid) for(int i=0;i<f.njobs;++i) {
        if(f.target[i]>now_ms+period*.25 || f.target[i]<=at) continue;
        uint64_t tag=0;
        if(cache_find(f.serial,i)>=0 || (f.jobs[i]>=0 && !f.failed[i] && psx_mod_replay_job_state(f.jobs[i],&tag)==3 && tag==f.serial)) {best=&f;pick=i;at=f.target[i];}
    }
    if(best) {
        int c=cache_find(best->serial,pick);bool drawn=false;
        if(c<0 && info->slack_ms>=slack_floor_ms && render(*best,pick,info,true)) {c=cache_find(best->serial,pick);drawn=true;}
        if(c>=0) {
            if(target_hz<0) late_pace(c==last_shown_cache);
            shown_serial=best->serial;cache[c].used=++cache_clock;++reused;
            trace(info,best->alpha[pick],best,pick,drawn);last_shown_cache=c;return c+1;
        }
    }
    if(last_shown_cache>=0 && psx_mod_replay_cache_valid(last_shown_cache)) {
        ++late_repeats;if(target_hz<0) late_pace(true);
        trace(info,0,nullptr,-2,false);return last_shown_cache+1;
    }
    ++no_job;trace(info,0,nullptr,-1,false);return 0;
}
int provider(const PSXModPresentInfo* info,void*) {
    if(!interp_replay || !input_modernized() || !info) return 0;
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
        if(c<0 && !nodue && info->slack_ms>=slack_floor_ms && render(*shown,i,info,true)) {c=cache_find(shown->serial,i);drawn=true;}
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

void activate() {
    const char* mode=std::getenv("DNTTK_INPUT_MODE");
    if(!mode || std::strcmp(mode,"modernized")) return;
    target_hz=parse_rate(std::getenv("DNTTK_FRAME_RATE"));
    if(target_hz==60) return;
    // In-between images above 60 by default (off: DNTTK_FRAME_INTERP=off);
    // 30 shows each game image once and never redraws.
    const char* interp=std::getenv("DNTTK_FRAME_INTERP");
    interp_replay=!(interp && !std::strcmp(interp,"off")) && (target_hz<0 || target_hz==0 || target_hz>60);
    if(const char* t=std::getenv("DNTTK_INTERP_STEPS")) {steps=std::clamp(std::atoi(t),1,max_steps);steps_fixed=true;}
    if(const char* t=std::getenv("DNTTK_REPLAY_TEST")) {
        interp_off=!std::strcmp(t,"alpha0") || !std::strcmp(t,"interp_off");
        interp_full=!std::strcmp(t,"alpha1");
    }
    // Past idle time is not a present deadline. With the optimized rendering
    // path, fixed-rate camera images must not be withheld by that stale EMA.
    // Unlimited retains the guard so overload feeds its adaptive cadence.
    if(continuous_timeline() && target_hz>=0) slack_floor_ms=0.0;
    if(const char* t=std::getenv("DNTTK_REPLAY_SLACK_MS")) slack_floor_ms=std::clamp(std::atof(t),0.0,16.0);
    if(const char* t=std::getenv("DNTTK_LATE_LEAD_MS")) {late_lead_ms=std::clamp(std::atof(t),2.0,40.0);late_lead_fixed=true;}
    if(const char* t=std::getenv("DNTTK_REPLAY_BUDGET_MS")) {budget_ms=std::clamp(std::atof(t),0.5,16.0);budget_fixed=true;}
    replay_on=psx_mod_set_present_replay(provider,nullptr,target_hz)!=0;
    psx_mod_set_replay_status(status_json);
}

// Workers fork from a running game, after the window and renderer exist.
bool workers_started=false;
void start_workers_once() {
    if(workers_started || !replay_on || !interp_replay || !replay_gameplay_context()) return;
    // The game initializes this math routine before gameplay, but may not call
    // it until the opening has begun. Load its exact cached variant now so a
    // first use does not force all render workers to refork during camera motion.
    (void)overlay_loader_prepare_address(0x800B3D9Cu);
    near_clip_prepare();
    render_state_prepare();
    aim_render_prepare();
    workers_started=true;
    psx_mod_replay_workers_visual_timing(1);
    int workers=3;
    if(const char* t=std::getenv("DNTTK_REPLAY_WORKERS")) workers=std::clamp(std::atoi(t),1,8);
    if(psx_mod_replay_workers_start(workers,worker_draw)<=0) interp_replay=false;
}
void composition_entry(CPUState* cpu,uint32_t address) {
    if(!psx_mod_replay_active()) start_workers_once();
    // The live frame that just ended drew these actors.
    if(!psx_mod_replay_active() && address==composition) {
        std::memcpy(drawn,drawing,sizeof(uint32_t)*(size_t)ndrawing);
        std::memcpy(drawn_kind,drawing_kind,sizeof(uint32_t)*(size_t)ndrawing);
        ndrawn=ndrawing;
        ndrawing=0;
    }
    composition_hook(cpu,address);
}
// The object-list loop draws each object through table 0x800c0bf8 by its kind
// (byte +0x14): 0x800632b0, 0x80031d10, 0x80032e78, 0x800348d8 (Duke and
// matrix actors), 0x80031c14, all called (camera, object, ...).
uint32_t drawing_kind[max_actors];
void actor_draw(CPUState* cpu,uint32_t address) {
    if(!interp_replay || cpu->gpr[4]!=camera_base) return;
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
    if(!interp_replay || cpu->gpr[4]!=camera_base) return;
    const uint32_t sky_ra=cpu->gpr[31];
    const bool sky=sky_transform_call(cpu->gpr[4],sky_ra);
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
    if(!interp_replay || psx_mod_replay_active() || cpu->gpr[31]!=0x80026904u || !rec_frame) return;
    rec_frame->xf_done=true;
    static const bool pose_trace=std::getenv("DNTTK_POSE_TRACE")!=nullptr;
    if(pose_trace) std::fprintf(stderr,"[pose-ready] serial=%llu host_ms=%.3f\n",
        (unsigned long long)rec_frame->serial,
        1000.0*SDL_GetPerformanceCounter()/SDL_GetPerformanceFrequency());
    xf_obj=0;
    if(Frame* prev=frame_by_serial(rec_frame->serial-1)) if(prev->xf_done) queue(*prev,*rec_frame);
    rec_frame=nullptr;
}
} // namespace

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
int replay_load_window(int window) {
    if(!replay_on || !interp_replay || !late_camera()) {shed_bad=shed_clean=0;return window==1 ? -1 : 0;}
    shed_steps=std::min(shed_steps,pace_div-1);  // Unlimited's late_pace shares the divisor
    ++shed_since_probe;
    if(window==2) {shed_bad=0;shed_clean=0;return 0;}  // not a gameplay second (menu, load)
    if(window==1) {
        shed_clean=0;shed_full_clean=0;
        if(++shed_bad<2) return 0;
        shed_bad=0;
        if(pace_div>=(target_hz<0 ? 16 : 3)) return -1;
        const bool failed_probe=shed_since_probe<=10;
        if(failed_probe) {shed_backoff=std::min(shed_backoff*2,16);++shed_failed_probes;}
        ++pace_div;++shed_steps;pace_clean=0;pace_presents=pace_bad=0;psx_mod_set_present_divisor(pace_div);++pace_changes;++emu_sheds;
        shed_since_probe=1000;
        pace_log(failed_probe?"emulation behind, faster rate failed":"emulation behind",0,shed_backoff);
        return 1;
    }
    shed_bad=0;
    if(shed_steps>0) {
        if(++shed_clean>=5*shed_backoff) {
            --pace_div;--shed_steps;shed_clean=0;shed_since_probe=0;++shed_probes;
            pace_clean=0;pace_presents=pace_bad=0;psx_mod_set_present_divisor(pace_div);++pace_changes;
            pace_log("emulation keeping up, trying faster",0,shed_backoff);
        }
    } else if(pace_div==1 && shed_backoff>1 && ++shed_full_clean>=60) {shed_backoff/=2;shed_full_clean=0;}
    return 0;
}

const char* frame_replay_debug_json() {
    static char buffer[16384];
    int n=std::snprintf(buffer,sizeof buffer,"{\"on\":%s,\"interp\":%s,\"target_hz\":%d,\"steps\":%d,\"captures\":%llu,\"publishes\":%llu,"
        "\"submits\":%llu,\"rendered\":%llu,\"shown\":%llu,\"no_job\":%llu,\"failures\":%llu,\"serial\":%llu,\"prefetched\":%llu,\"governed\":%llu,\"deferred\":%llu,\"feed_ms\":%.2f,\"budget_ms\":%.2f,\"blended_mats\":%llu,\"skipped_actors\":%llu,\"camera_cuts\":%llu,\"drawn\":%d,\"pending_flips\":%llu,\"blended_xf\":%llu,\"skipped_objects\":%llu,\"substituted\":%llu,\"sub_misses\":%llu,\"late\":%s,\"late_applied\":%llu,\"late_skipped\":%llu,\"late_jit\":%llu,\"late_repeats\":%llu,\"lead_ms\":%.1f,\"ready_ms\":%.1f,\"pace_div\":%d,\"pace_backoff\":%d,\"pace_clean\":%d,\"pace_ratio\":%.3f,\"pace_mid\":%llu,\"shed_steps\":%d,\"shed_backoff\":%d,\"shed_probes\":%llu,\"shed_failed_probes\":%llu,\"pace_changes\":%llu,\"emu_sheds\":%llu,\"ahead\":[%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu]}",
        replay_on?"true":"false",interp_replay?"true":"false",(int)target_hz,steps,(unsigned long long)captures,
        (unsigned long long)publishes,(unsigned long long)submits,(unsigned long long)presented,(unsigned long long)reused,
        (unsigned long long)no_job,(unsigned long long)failures,(unsigned long long)serial,
        (unsigned long long)prefetched,(unsigned long long)governed,(unsigned long long)deferred,feed_ms,budget_ms,(unsigned long long)blended_mats,
        (unsigned long long)skipped_actors,(unsigned long long)camera_cuts,ndrawn,(unsigned long long)pending_flips,(unsigned long long)blended_xf,(unsigned long long)skipped_objects,
        (unsigned long long)substituted,(unsigned long long)sub_misses,late_camera()?"true":"false",
        (unsigned long long)late_applied,(unsigned long long)late_skipped,(unsigned long long)late_jit,(unsigned long long)late_repeats,late_lead_ms,late_ready_ms,pace_div,pace_backoff,pace_clean,pace_last_ratio,(unsigned long long)pace_mid,shed_steps,shed_backoff,(unsigned long long)shed_probes,(unsigned long long)shed_failed_probes,(unsigned long long)pace_changes,(unsigned long long)emu_sheds,
        (unsigned long long)ahead_hist[0],(unsigned long long)ahead_hist[1],(unsigned long long)ahead_hist[2],(unsigned long long)ahead_hist[3],
        (unsigned long long)ahead_hist[4],(unsigned long long)ahead_hist[5],(unsigned long long)ahead_hist[6],(unsigned long long)ahead_hist[7]);
    // Append the present trace: "ms/alpha/shown/ready/cached/njobs[d]/serial/yaw/t/planned alpha".
    if(n>1 && n<(int)sizeof buffer-64) {
        n-=1;
        n+=std::snprintf(buffer+n,sizeof buffer-n,",\"timeline\":{\"slack_ms\":%.2f,\"pose_lag_ms\":%.2f,\"oldest_age_ms\":%.2f,\"future_evictions\":%llu},\"trace\":\"",response_slack,response_pose_lag,response_oldest_age,(unsigned long long)response_future_evictions);
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
} // namespace ttk

PSX_MOD_CONSTRUCTOR(register_ttk_frame_replay) {
    psx_mod_register_activation_plugin("ttk.frame_rate",ttk::activate);
    psx_mod_register_function_entry_plugin("ttk.frame.replay",ttk::composition,ttk::composition_entry);
    for(uint32_t a:{0x800632b0u,0x80031d10u,0x80032e78u,0x800348d8u,0x80031c14u})
        psx_mod_register_function_entry_plugin("ttk.frame.replay",a,ttk::actor_draw);
    psx_mod_register_function_entry_plugin("ttk.frame.replay.profile",0x8001ca4cu,ttk::quality_actor_end);
    psx_mod_register_function_entry_plugin("ttk.frame.replay.xf",0x800292a0u,ttk::transform_load);
    psx_mod_register_function_entry_plugin("ttk.frame.replay",0x8001fba0u,ttk::composition_end);
}
