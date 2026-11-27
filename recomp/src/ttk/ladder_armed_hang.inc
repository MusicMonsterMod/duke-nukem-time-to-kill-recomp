// D08J11 - a one-armed full hang on a ladder while the sidearm is out.
// Included from modern_controls.cpp after swim.inc, before climb_aim.inc.
// User, 2026-11-27: "rather than hang, he immediately pulls up. this can look
// jarring if you start shooting because he's doing half a pull up while
// shooting enemies! ... during the hang phase, duke's position reverts to a
// full hang with one arm if the pistol is drawn ... maybe we build out 3
// poses and i choose the best one of the 3."
//
// After a catch below the rungs (D08J10) the legs hang (ladder_dangle) while
// the upper body keeps the climb's rest, 186 or 187 (an armed climb rests,
// D08J6A). In both the off arm (rot 10-12, channel 2) is bent: in 186 its
// hand grips 358 above the pelvis (shoulder 172 above it), in 187 it holds a
// rung at the hip (58 above), measured with recomp/analysis/d08j11/hands.py.
// The original's own one-handed full hang is the armed ledge hang 149: hands
// 425 above the pelvis, the arm at full stretch.
//
// Modernized, LADDER CLIMBING MODERN, third person, the D08J6B aim lease
// (climb_aim_ready: sidearm out, view aiming), while the dangle lasts (a leg
// still short of the lowest rung), weight eased in over armed_hang_ramp_ms and
// out with the stow:
//  * the off arm takes 186's high grip (its channel-2 frame 0) as the clip
//    pose, so from 187 the free hand moves up onto the rung the gun hand let
//    go of;
//  * the whole model is offset in the build's root (as the D08J6F swing):
//    `drop` down and `forward` toward the ladder along Duke's heading. The
//    D08J6B off-hand IK keeps the hand on its grip (the offset is taken back
//    out of the contact), so the arm straightens and the body hangs below it.
//    Duke's position, collision and camera never change;
//  * `side` turns the body about the grip's vertical toward the aim (that
//    share of the view's yaw right of the heading, within -armed_hang_side_left
//    and armed_hang_side_right), through the D08J6F swing; the chest turn and
//    the gun arc take the rest.
// Three candidates were built; the user chose 2, side (2026-11-27: "hangpose
// 2 is the one, yeah i like it. lets lock it in"), the default. The others
// stay switchable live for tuning with the console (`hangpose 1|2|3|off`) or
// DNTTK_LADDER_ARMED_HANG=hang|side|pull|off:
//  1 hang  - hanging straight under the grip hand, arm at full stretch;
//  2 side  - the same hang, the body turned side-on toward the aim;
//  3 pull  - a slight one-arm pull: elbow bent, chest raised near the rungs.
// `hangpose tune <drop> <forward> <side>` overrides the current candidate's
// numbers for the session (developer tuning; `hangpose tune off` clears it).
// Unarmed, first person, LADDER CLIMBING CLASSIC and Vanilla never reach it
// (the dangle and the lease are Modernized and modern-climbing only).
static bool climb_aim_ready();
static constexpr double armed_hang_ramp_ms=220;
static constexpr double armed_hang_side_left=30*tau/360,armed_hang_side_right=75*tau/360;
struct ArmedHangShape {const char* name;double drop,forward,side;};
static constexpr ArmedHangShape armed_hang_shapes[3]={
    {"hang",60,70,0},
    {"side",60,60,.6},
    {"pull",25,90,0}};
static std::atomic<int> armed_hang_tune_on{0},armed_hang_tune_drop{0},armed_hang_tune_forward{0},armed_hang_tune_side{0};
// trim: what the fit took off the candidate's drop (debug).
static struct {double w,last_ms;int shape;double drop,forward,side,trim;double t[3];} armed_hang={0,-1,0,0,0,0,0,{0,0,0}};
// Written by the D08J6B off-hand solve (climb_aim.inc): the drop at which the
// hand is at full reach from the shoulder (the arm straight, never stretched).
static double climb_ik_fit,armed_hang_fit=1e9;static bool climb_ik_fit_valid;
static unsigned armed_hang_builds,armed_hang_arm_writes;
// 0 off, 1..3 the candidate (2, side, when unset: the user's choice).
static int armed_hang_shape() {
    const char* v=ttk_live_env("DNTTK_LADDER_ARMED_HANG");
    if(!v || !*v)return 2;
    if(!std::strcmp(v,"off") || !std::strcmp(v,"0"))return 0;
    for(int i=0;i<3;++i)
        if(!std::strcmp(v,armed_hang_shapes[i].name) || (v[0]=='1'+i && !v[1]))return i+1;
    return 2;
}
// The body offset the build applies (actor-relative world axes, down +y).
static bool armed_hang_offset(double* t) {
    if(armed_hang.w<=0)return false;
    for(int i=0;i<3;++i)t[i]=armed_hang.t[i];
    return true;
}
// At 0x800411b8 (ra 0x80042634), after ladder_dangle_pose, before
// climb_aim_begin: the weight, the shape and the off arm's grip pose.
static void armed_hang_pose(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634)return;
    auto& h=armed_hang;
    const double now=input_clock_ms();
    const double dt=h.last_ms<0?0:std::clamp(now-h.last_ms,0.0,100.0);
    h.last_ms=now;
    const unsigned a=psx_mod_read_half(player+0x60);
    const int shape=armed_hang_shape();
    // Still hanging on this ladder below the rungs; else it ends at once.
    const bool hanging=ladder_dangle.active && psx_mod_read_byte(player+0x22c)==3 && a>=186 && a<=189;
    if(!hanging || !shape || fp_blend>0) {h.w=0;h.shape=0;h.trim=0;armed_hang_fit=1e9;climb_ik_fit_valid=false;for(double& v:h.t)v=0;return;}
    // Eases in once the sidearm is out and settled, out through an equip or
    // stow track (upper 6/7: the W stow runs 6, then the weapon state goes 0
    // and the lease with it). Past the lease the build keeps applying the
    // fading offset without the off-hand IK (climb_aim_arm), so it never snaps.
    const unsigned upper=psx_mod_read_half(player+0x74);
    const bool armed=climb_aim_ready() && upper!=6 && upper!=7;
    h.w=std::clamp(h.w+(armed?1:-1)*dt/armed_hang_ramp_ms,0.0,1.0);
    if(h.w<=0) {h.trim=0;armed_hang_fit=1e9;for(double& v:h.t)v=0;return;}
    h.shape=shape;
    const ArmedHangShape& s=armed_hang_shapes[shape-1];
    h.drop=s.drop;h.forward=s.forward;h.side=s.side;
    if(armed_hang_tune_on.load()) {
        h.drop=armed_hang_tune_drop.load();h.forward=armed_hang_tune_forward.load();h.side=armed_hang_tune_side.load()/100.0;
    }
    const double e=h.w*h.w*(3-2*h.w);
    // The arm stops at full stretch: drop no further than the hand reaches.
    if(climb_ik_fit_valid)armed_hang_fit=climb_ik_fit;
    climb_ik_fit_valid=false;
    const double drop=std::min(h.drop*e,armed_hang_fit);
    h.trim=h.drop*e-drop;
    // Game yaw: +Z 0, +X 1024; forward is (sin, 0, cos).
    const double heading=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
    h.t[0]=std::sin(heading)*h.forward*e;h.t[1]=drop;h.t[2]=std::cos(heading)*h.forward*e;
    ++armed_hang_builds;
    // The off arm onto 186's high grip (channel 2, frame 0).
    const uint32_t desc=duke_model();
    const int clip=int16_t(psx_mod_read_half(0x800c2d64+8*186+4));
    if(!desc || clip<0 || clip>=2048)return;
    const uint32_t first=psx_mod_read_half(0x800d8328+4*clip);
    std::array<std::array<int32_t,4>,32> q{};uint32_t mask=0;
    if(!crouch_frame_quats(first,q,mask))return;
    for(unsigned j=10;j<=12;++j) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r || !(mask>>j&1))continue;
        const CrouchQuat target=crouch_qnorm({double(q[j][0]),double(q[j][1]),double(q[j][2]),double(q[j][3])});
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),target,e));
    }
    ++armed_hang_arm_writes;
}
// The grip the off hand holds (actor-relative, as the D08J6B solve's
// contact): it follows the posed hand while the hang eases in, is frozen in
// the world once fully in (firing and the rest idle no longer slide it along
// or off the rung; Duke never moves while armed), and eases back to the posed
// hand on the way out.
static bool armed_hang_grip_frozen;static double armed_hang_grip_world[3];
static unsigned armed_hang_grip_freezes;
static contact_arm::Vec armed_hang_grip(contact_arm::Vec hand,contact_arm::Vec actor) {
    auto& g=armed_hang_grip_world;
    if(armed_hang.w<=0) {armed_hang_grip_frozen=false;return hand;}
    if(armed_hang_grip_frozen && std::hypot(std::hypot(g[0]-actor.x-hand.x,g[1]-actor.y-hand.y),g[2]-actor.z-hand.z)>200)
        armed_hang_grip_frozen=false;   // moved or re-posed: follow the hand again
    if(!armed_hang_grip_frozen) {
        g[0]=actor.x+hand.x;g[1]=actor.y+hand.y;g[2]=actor.z+hand.z;
        if(armed_hang.w>=1) {armed_hang_grip_frozen=true;++armed_hang_grip_freezes;}
        return hand;
    }
    const double e=armed_hang.w*armed_hang.w*(3-2*armed_hang.w);
    const contact_arm::Vec held{g[0]-actor.x,g[1]-actor.y,g[2]-actor.z};
    return hand+(held-hand)*e;
}
// The swing toward the aim for `side` (radians, Duke's gun side positive),
// combined with the D08J6F swing law's target.
static double armed_hang_swing(double delta,double law_target) {
    if(armed_hang.w<=0 || armed_hang.side<=0)return law_target;
    const double e=armed_hang.w*armed_hang.w*(3-2*armed_hang.w);
    const double own=std::clamp(armed_hang.side*delta,-armed_hang_side_left,armed_hang_side_right)*e;
    return law_target>0?std::max(law_target,own):own;
}
bool armed_hang_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"hangpose") && std::strncmp(line,"hangpose ",9))return false;
    close=false;
    const char* arg=line[8]?line+9:"";
    char text[200];
    if(!std::strncmp(arg,"tune",4)) {
        double d,f,s;
        if(!std::strcmp(arg,"tune off")) {armed_hang_tune_on.store(0);say("Armed ladder hang: tuning off.");return true;}
        if(std::sscanf(arg+4,"%lf %lf %lf",&d,&f,&s)!=3) {say("Usage: hangpose tune <drop> <forward> <side> | hangpose tune off");return true;}
        armed_hang_tune_drop.store(int(std::lround(std::clamp(d,-200.0,300.0))));
        armed_hang_tune_forward.store(int(std::lround(std::clamp(f,-200.0,200.0))));
        armed_hang_tune_side.store(int(std::lround(std::clamp(s,0.0,1.0)*100)));
        armed_hang_tune_on.store(1);
        std::snprintf(text,sizeof text,"Armed ladder hang tuned: drop %d, forward %d, side %.2f (session).",armed_hang_tune_drop.load(),armed_hang_tune_forward.load(),armed_hang_tune_side.load()/100.0);
        say(text);return true;
    }
    // Literals only: the live env keeps the pointer.
    if(!std::strcmp(arg,"1") || !std::strcmp(arg,"hang"))ttk_live_env_set("DNTTK_LADDER_ARMED_HANG","hang");
    else if(!std::strcmp(arg,"2") || !std::strcmp(arg,"side"))ttk_live_env_set("DNTTK_LADDER_ARMED_HANG","side");
    else if(!std::strcmp(arg,"3") || !std::strcmp(arg,"pull"))ttk_live_env_set("DNTTK_LADDER_ARMED_HANG","pull");
    else if(!std::strcmp(arg,"off") || !std::strcmp(arg,"0"))ttk_live_env_set("DNTTK_LADDER_ARMED_HANG","off");
    else if(*arg) {say("Usage: hangpose [1 hang|2 side|3 pull|off] | hangpose tune <drop> <forward> <side>");return true;}
    const int shape=armed_hang_shape();
    if(!shape)std::snprintf(text,sizeof text,"Armed ladder hang: off (the D08J10 half pull-up).");
    else {
        const ArmedHangShape& s=armed_hang_shapes[shape-1];
        std::snprintf(text,sizeof text,"Armed ladder hang: %d %s (drop %.0f, forward %.0f, side %.2f)%s.",shape,s.name,s.drop,s.forward,s.side,
            armed_hang_tune_on.load()?", tuning on":"");
    }
    say(text);
    return true;
}
static const char* armed_hang_debug_json() {
    static char buffer[320];
    std::snprintf(buffer,sizeof buffer,"{\"shape\":%d,\"w\":%.3f,\"drop\":%.0f,\"forward\":%.0f,\"side\":%.2f,\"trim\":%.1f,\"t\":[%.1f,%.1f,%.1f],\"builds\":%u,\"arm_writes\":%u,\"grip_frozen\":%s,\"grip_freezes\":%u}",
        armed_hang.shape,armed_hang.w,armed_hang.drop,armed_hang.forward,armed_hang.side,armed_hang.trim,armed_hang.t[0],armed_hang.t[1],armed_hang.t[2],armed_hang_builds,armed_hang_arm_writes,armed_hang_grip_frozen?"true":"false",armed_hang_grip_freezes);
    return buffer;
}
