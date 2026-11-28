// D36N: first-person viewmodel motion (Modernized only).
//
// The drawn weapon and both display arms are one camera-relative assembly:
// the right hand draws at weapon_matrix and the arm chains hang from it
// (prepare_display_matrix, display_arm_matrix). This layer adds deliberate,
// physically motivated motion to that assembly, as one pose about the grip in
// the level frame, the same convention as the D53G idle gesture: pitch muzzle
// up, yaw muzzle right, roll top left (degrees); x right, y down, z forward
// (view units). The original animation keeps firing, reloads, throws and
// draws; gameplay (aim, hits, events, timers) never sees this pose.
//
// Layers (research: documentation/572-d36n-viewmodel-motion.md):
//  - look sway: the gun trails the view when turning or looking (a damped
//    spring toward a lag proportional to the view's angular velocity, soft
//    clamped), with a little roll into the lag and a settle on stopping;
//  - inertia: one mass on a spring held by the view, driven by Duke's own
//    velocity changes. Starting, stopping and strafing lag the gun; a jump's
//    take-off lags it down, free fall floats it up, a landing dips it in
//    proportion to the impact, all from the same model;
//  - bob: a footstep figure-8 on the D17V view bob's phase (one dip per step,
//    one side sway per stride), deeper when running;
//  - run carry: running, the gun settles a little lower and canted in; any
//    shot cuts it at once and it returns after a pause;
//  - breath: standing still, a slow faint rise and fall.
// Time is the game's frame step (300 units a second), so pause holds it. The
// pose is computed on each live update and published with the render state;
// redraw workers blend it between ticks like the idle gesture.
//
// OPTIONS > GAMEPLAY > WEAPON MOTION (DNTTK_WEAPON_MOTION): on (default),
// subtle (half), off. Developer: DNTTK_FP_MOTION_PARTS=sway,inertia,bob,carry,
// breath (a list; default all), DNTTK_FP_MOTION_LOG=1 (one line per update).

static double weapon_motion_scale() {
    static const char* parsed=nullptr; static double v=1.0; static bool have=false;
    const char* t=ttk_live_env("DNTTK_WEAPON_MOTION");
    if(have && t==parsed) return v;
    have=true; parsed=t;
    if(!t || !*t || !std::strcmp(t,"on")) v=1.0;
    else if(!std::strcmp(t,"off") || !std::strcmp(t,"0")) v=0.0;
    else if(!std::strcmp(t,"subtle")) v=0.5;
    else {char* end=nullptr; const double d=std::strtod(t,&end); v=end && !*end && d>=0 && d<=3 ? d : 1.0;}
    return v;
}
enum MotionPart : unsigned {motion_sway=1,motion_inertia=2,motion_bob=4,motion_carry=8,motion_breath=16};
static unsigned motion_parts() {
    static const unsigned parts=[]{
        const char* t=std::getenv("DNTTK_FP_MOTION_PARTS");
        if(!t) return 31u;
        unsigned p=0;
        if(std::strstr(t,"sway")) p|=motion_sway;
        if(std::strstr(t,"inertia")) p|=motion_inertia;
        if(std::strstr(t,"bob")) p|=motion_bob;
        if(std::strstr(t,"carry")) p|=motion_carry;
        if(std::strstr(t,"breath")) p|=motion_breath;
        return p;
    }();
    return parts;
}

// Tuning (see note 572 for the reasoning and the references behind each).
struct MotionTuning {
    // Look sway: lag seconds of angular velocity, soft limit, spring.
    // Near-critical damping: the lag settles without a wobble (Source's
    // follower, Doom 3's capped average; none of the references bounce).
    double look_lag=0.022, look_max=3.2, look_hz=3.6, look_zeta=0.9, look_input=0.05;
    double look_shift=1.6;      // view units of trailing per degree of lag
    double look_roll=0.55;      // roll per degree of yaw lag
    // Inertia (side and forward): gain on Duke's velocity changes, spring, limits.
    double inertia_gain[3]={0.0035,0,0.0030}, inertia_hz=3.0, inertia_zeta=0.7;
    double inertia_max[3]={9,0,10};    // soft limits of the drawn offset
    // Vertical: in the air the gun is offset by vertical speed (lagging low
    // rising, floating up falling); a landing dips it, quick down, slower back.
    double air_gain=0.0032, air_max=6, air_tau=0.08;
    double land_gain=0.0065, land_max=14, land_down=0.15, land_up=0.30, land_pitch=0.25;
    double inertia_pitch=0.30;  // muzzle degrees per unit of vertical offset
    double inertia_roll=0.35;   // roll degrees per unit of side lag
    double strafe_roll=2.0;     // degrees at running strafe speed
    // Bob, at walking amount 1 (running reaches 2): view units and degrees.
    // Duke 3D's arc: lowest crossing the centre, rising to each side.
    double bob_x=4.5, bob_y=4.0, bob_roll=1.2, bob_yaw=0.9, bob_pitch=0.5, bob_lag=0.35, bob_fire=0.35;
    // Run carry, at full weight.
    double carry_pitch=-1.5, carry_yaw=-1.0, carry_roll=2.5, carry_x=-3, carry_y=5, carry_z=-3;
    double carry_in=0.22, carry_out=0.05, carry_hold=0.45;
    // Breath: seconds per breath, amplitudes.
    double breath_period=4.6, breath_pitch=0.30, breath_y=0.9, breath_roll=0.12;
    // Shotgun rack: gun cant at the pull (degrees, units) and the hand's stroke.
    double pump_roll=7, pump_pitch=2.5, pump_x=-6, pump_y=-3, pump_stroke=34;
};
static const MotionTuning motion_tuning{};

struct MotionSpring {double x=0,v=0;};
// Semi-implicit Euler in substeps short enough for the stiffest spring.
static void spring_step(MotionSpring& s,double target,double hz,double zeta,double dt,double accel=0) {
    const double w=2*3.14159265358979*hz;
    const int n=std::max(1,int(std::ceil(dt/0.002)));
    const double h=dt/n;
    for(int i=0;i<n;++i) {
        s.v+=(w*w*(target-s.x)-2*zeta*w*s.v+accel)*h;
        s.x+=s.v*h;
    }
}

struct MotionState {
    bool valid=false;
    double fwd[3]{},right[3]{},up[3]{};   // previous camera axes (world)
    double root[3]{};                     // previous root
    double vel[3]{};                      // smoothed root velocity, units/s, level frame (x right, y down, z forward)
    double speed=0;                       // smoothed ground speed, units/s
    MotionSpring look[2];                 // yaw, pitch lag (degrees)
    MotionSpring body[3];                 // inertia offset (view units)
    double bob_amount=0, stride=0, own_phase=0, last_phase=0;
    double carry=0, since_fire=10;
    double breath_t=0, breath_w=0;
    double weight=0;                      // eligibility (mode, slot), eased
    double strafe=0;                      // smoothed strafe roll weight
    double turn_rate=0, lift_rate=0;      // low-passed view angular velocity
    double air=0;                         // vertical offset in the air
    double land_t=-1, land_depth=0, land_now=0;   // landing envelope (seconds since, depth, value)
    double fall=0;                        // fastest fall this airborne spell
    bool was_air=false;
    double fire_w=0;                      // eased firing weight
    unsigned slot=32;
    uint64_t updates=0, resets=0, landings=0;
    double last_impact=0;
};
static MotionState motion;
// The pose (6), then the Shotgun's pump slide (0..1) and support-grip hold weight.
static constexpr int motion_values=8;
static double motion_pose_live[motion_values];
static double override_motion_pose[motion_values];

static void motion_reset() {
    const auto updates=motion.updates,resets=motion.resets+1,landings=motion.landings;
    motion={};motion.updates=updates;motion.resets=resets;motion.landings=landings;
    std::memset(motion_pose_live,0,sizeof motion_pose_live);
}

// Per-slot shaping. The Energy Weapon's support shoulder shows if the gun
// rises above ready (D53G), so it only moves down; the RPG fills the lower
// view, so its rotations are halved.
static void motion_slot_shape(unsigned slot,double* p) {
    if(slot==8) {for(int i=0;i<3;++i) p[i]*=0.5; p[3]*=0.7; p[4]*=0.7;}
    if(slot==10) {
        p[0]=std::min(p[0],0.0); p[4]=std::max(p[4],0.0);
        // D36O: the D36N limit on its muzzle-down, muzzle-left, top-left and
        // rightward motion is gone: the housing side it protected was a folded
        // quad the renderer now draws whole (near_clip.cpp, note 573).
    }
    // Two-handed weapons: a lift brings the bare support forearm into the
    // eye (D53G rest at 0.6), so upward travel is limited.
    const bool support=slot==3 || (slot>=5 && slot<=11);
    if(support && slot!=10) {p[4]=std::max(p[4],-6.0); p[0]=std::min(p[0],3.0);}
}

// D36N Shotgun pump (display slot 5). The native third-person pump (upper
// clip 26, keys 79-90, after fire 24 and 25) lifts the support fist off the
// gun toward the top of the receiver, which from the eye read as "a weird
// rubbing thing" (user, D08A53). The support chain is held on its ready grip
// (clip 20 key 0) and the rack is drawn instead: the hand pulls back along
// the fore-end, holds, drives forward, and the gun cants so the stroke reads.
// Progress 0..1 through clip 26 from its key and the key's countdown.
struct PumpTrack {unsigned key=0; unsigned span=1;};
static PumpTrack pump_track;
static double grip_hold, pump_slide_now, pump_cant_now;   // live
static double shotgun_pump_progress() {
    const unsigned anim=psx_mod_read_half(player+0x74),timer=psx_mod_read_half(player+0x76),key=psx_mod_read_half(player+0x78);
    if(anim==24 || anim==25) {pump_track={};return 0;}
    if(anim!=26 || key<79 || key>90) {pump_track={};return -1;}
    if(key!=pump_track.key) {pump_track.key=key;pump_track.span=std::max(1u,timer);}
    pump_track.span=std::max(pump_track.span,timer);
    const double frac=1-std::clamp(double(timer)/pump_track.span,0.0,1.0);
    return std::clamp((key-79+frac)/11.0,0.0,1.0);
}
static double smoothstep(double a,double b,double x) {const double t=std::clamp((x-a)/(b-a),0.0,1.0);return t*t*(3-2*t);}
// Pump slide 0..1 (hand back along the wood): quick pull, short hold, drive home.
static double pump_slide(double p) {return p<0 ? 0 : smoothstep(0.02,0.38,p)*(1-smoothstep(0.52,0.86,p));}
// Gun cant through the rack, 0..1, peaking with the pull.
static double pump_cant(double p) {return p<0 ? 0 : std::pow(std::sin(3.14159265358979*std::clamp(p/0.92,0.0,1.0)),2);}
static bool shotgun_hold_clip(unsigned anim) {return anim==18 || anim==19 || anim==20 || (anim>=23 && anim<=26);}

// The Shotgun grip and rack: a pose fix, not a motion preference, so it runs
// at any WEAPON MOTION setting (Modernized; Vanilla has no first person).
static void shotgun_update(unsigned slot,double dt) {
    static const int grip_on=ttk_env_switch("DNTTK_FP_SHOTGUN_GRIP");   // developer A/B (0: the D08A53 grip)
    if(slot!=5 || !input_modernized() || !grip_on) {grip_hold=pump_slide_now=pump_cant_now=0;pump_track={};return;}
    const double pp=shotgun_pump_progress();
    pump_slide_now=pump_slide(pp);pump_cant_now=pump_cant(pp);
    const bool hold=shotgun_hold_clip(psx_mod_read_half(player+0x74));
    grip_hold+=((hold?1.0:0.0)-grip_hold)*(dt>0 ? 1-std::exp(-dt/0.06) : 0.0);
}
static void shotgun_compose(double* pose) {
    const MotionTuning& T=motion_tuning;const double c=pump_cant_now;
    pose[2]+=T.pump_roll*c;pose[0]+=T.pump_pitch*c;pose[3]+=T.pump_x*c;pose[4]+=T.pump_y*c;
    pose[6]=pump_slide_now;pose[7]=grip_hold;
}

// One live update: c is this draw's (orthonormalized) camera, rows right,
// down, forward in world axes.
static void motion_update(const double c[3][3]) {
    const MotionTuning& T=motion_tuning;
    const double scale=input_modernized() ? weapon_motion_scale() : 0.0;
    const unsigned parts=motion_parts();
    const unsigned step=psx_mod_read_word(0x800d21fc);
    const double dt=std::clamp(step/300.0,0.0,0.1);
    ++motion.updates;
    const unsigned slot=weapon_view_slot();
    shotgun_update(slot,dt);
    if(scale<=0 || slot>=32) {motion_reset();shotgun_compose(motion_pose_live);return;}
    double root[3];
    for(int i=0;i<3;++i) root[i]=(int32_t)psx_mod_read_word(player+4+4*i);
    // Height from the original camera's pivot when it rides Duke (the eye's
    // own reference, D17C): level through idle and gait, it follows steps,
    // jumps and falls; the root's height wobbles with the idle clip.
    if(psx_mod_read_word(camera+0x50)==player+0x7bc) root[1]=(int32_t)psx_mod_read_word(player+0x7c0);
    // Level frame: right and forward without pitch, game Y down.
    const double rl=std::hypot(c[0][0],c[0][2]),fl=std::hypot(c[2][0],c[2][2]);
    if(rl<0.5 || fl<0.1) {motion_reset();shotgun_compose(motion_pose_live);return;}
    const double lr[3]={c[0][0]/rl,0,c[0][2]/rl},lf[3]={c[2][0]/fl,0,c[2][2]/fl};
    if(!motion.valid || slot!=motion.slot) {
        // A new weapon starts from rest (the draw animation carries it in).
        const double w=motion.valid ? motion.weight : 0;
        motion_reset();motion.weight=w;motion.valid=true;motion.slot=slot;
        std::memcpy(motion.fwd,c[2],sizeof motion.fwd);std::memcpy(motion.right,c[0],sizeof motion.right);
        for(int i=0;i<3;++i) motion.up[i]=-c[1][i];
        std::memcpy(motion.root,root,sizeof root);
        shotgun_compose(motion_pose_live);
        return;
    }
    if(dt<=0) return;   // paused: hold the pose
    // View angular velocity, degrees a second: the new forward against the
    // previous right and up axes (positive: turning right, looking up).
    double turn=0,lift=0;
    for(int i=0;i<3;++i) {turn+=c[2][i]*motion.right[i];lift+=c[2][i]*motion.up[i];}
    const double rad=57.29577951308232;
    double w_turn=std::asin(std::clamp(turn,-1.0,1.0))*rad/dt,w_lift=std::asin(std::clamp(lift,-1.0,1.0))*rad/dt;
    // A camera cut (respawn, a scripted view, a mode change) is not a turn.
    if(std::abs(w_turn)*dt>45 || std::abs(w_lift)*dt>45) {w_turn=w_lift=0;}
    std::memcpy(motion.fwd,c[2],sizeof motion.fwd);std::memcpy(motion.right,c[0],sizeof motion.right);
    for(int i=0;i<3;++i) motion.up[i]=-c[1][i];
    // Body velocity in the level frame.
    double d[3];for(int i=0;i<3;++i) d[i]=root[i]-motion.root[i];
    std::memcpy(motion.root,root,sizeof root);
    const bool teleport=std::hypot(d[0],d[2])>400 || std::abs(d[1])>600;
    double v_new[3]={0,0,0};
    if(!teleport) {
        v_new[0]=(d[0]*lr[0]+d[2]*lr[2])/dt;
        v_new[1]=d[1]/dt;
        v_new[2]=(d[0]*lf[0]+d[2]*lf[2])/dt;
    }
    // The root surges with the stride (D17V): low-pass the horizontal
    // velocity (about 0.1 s); vertical motion is ballistic, so lightly.
    // On the ground a stair step lifts the root in one update: smooth it more.
    const bool on_ground=psx_mod_read_byte(player+0x22c)==0;
    const double kh=1-std::exp(-dt/0.10),kv=1-std::exp(-dt/(on_ground?0.08:0.03));
    double dv[3];
    for(int i=0;i<3;++i) {
        const double k=i==1?kv:kh;
        const double before=motion.vel[i];
        motion.vel[i]+=(v_new[i]-motion.vel[i])*k;
        dv[i]=teleport ? 0 : motion.vel[i]-before;
    }
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    const bool ground=mode==0, air=mode==9;
    // Eligible: standing or jumping with a drawn weapon, no crouch/swim/climb
    // path, no one-handed raise; otherwise the layer eases away.
    const bool eligible=(ground || air) && !crouch_weapon_active() && !swim_weapon_view_active() &&
        onehand_view().raise<=0 && (fp_policy.action==PerspectiveAction::None || fp_policy.action==PerspectiveAction::Ground ||
        fp_policy.action==PerspectiveAction::Fall);
    motion.weight+=((eligible?1.0:0.0)-motion.weight)*(1-std::exp(-dt/0.12));
    const double ground_speed=std::hypot(motion.vel[0],motion.vel[2]);
    motion.speed=ground ? ground_speed : motion.speed;
    const double walk=600;   // D17V: bob_walk_speed (10 per vblank) in units a second
    // Look sway.
    const double soft=T.look_max;
    auto soft_clamp=[&](double x) {return soft*std::tanh(x/soft);};
    const bool sway=parts&motion_sway;
    // The view moves in per-update steps of the mouse: filter the rate first.
    const double ki=1-std::exp(-dt/T.look_input);
    motion.turn_rate+=(w_turn-motion.turn_rate)*ki;motion.lift_rate+=(w_lift-motion.lift_rate)*ki;
    spring_step(motion.look[0],sway ? soft_clamp(-motion.turn_rate*T.look_lag) : 0,T.look_hz,T.look_zeta,dt);
    spring_step(motion.look[1],sway ? soft_clamp(-motion.lift_rate*T.look_lag) : 0,T.look_hz,T.look_zeta,dt);
    // Inertia: the gun is a mass held by a spring to the view; Duke's
    // velocity change this update is an impulse against it (clamped, so a
    // hard landing reads strong without throwing the gun off screen).
    const bool inertia=parts&motion_inertia;
    // An impulse J sets the spring moving at J*omega, so its first swing
    // peaks near 0.6 J (zeta 0.55): J is about the drawn travel in units.
    const double omega=2*3.14159265358979*T.inertia_hz;
    for(int i=0;i<3;i+=2) {
        const double impulse=inertia ? std::clamp(-dv[i]*T.inertia_gain[i],-40.0,40.0) : 0;
        motion.body[i].v+=impulse*omega;
        spring_step(motion.body[i],0,T.inertia_hz,T.inertia_zeta,dt);
    }
    // Vertical. In the air: an offset from vertical speed (game Y down, so a
    // rise, negative, lowers the gun, positive offset). The fastest fall is
    // the landing's impact.
    if(air) motion.fall=std::max(motion.fall,motion.vel[1]);
    const double air_target=inertia && air ? std::clamp(-motion.vel[1]*T.air_gain,-T.air_max,T.air_max) : 0;
    motion.air+=(air_target-motion.air)*(1-std::exp(-dt/T.air_tau));
    if(ground && motion.was_air) {
        const double depth=inertia ? std::min(T.land_max,std::max(0.0,motion.fall-200)*T.land_gain) : 0;
        if(depth>0.5) {
            // A landing during the last one's dip continues from where it is.
            const double now=motion.land_now,deep=std::max(depth,now),r=std::clamp(now/deep,0.0,1.0);
            motion.land_depth=deep;motion.land_t=(1-std::sqrt(1-r))*T.land_down;
            ++motion.landings;motion.last_impact=motion.fall;
        }
        motion.fall=0;
    }
    motion.was_air=air;
    double land=0;
    if(motion.land_t>=0) {
        // Ease out to the bottom, ease in-out back up.
        const double t=motion.land_t;
        if(t<T.land_down) {const double u=t/T.land_down;land=1-(1-u)*(1-u);}
        else if(t<T.land_down+T.land_up) {const double u=(t-T.land_down)/T.land_up;land=1-u*u*(3-2*u);}
        else motion.land_t=-1;
        land*=motion.land_depth;
        if(motion.land_t>=0) motion.land_t+=dt;
    }
    motion.land_now=land;
    const double vertical=motion.air+land;
    // Strafe tilt from the smoothed side velocity.
    const double strafe_target=inertia && ground ? std::clamp(motion.vel[0]/(walk*4),-1.0,1.0) : 0;
    motion.strafe+=(strafe_target-motion.strafe)*(1-std::exp(-dt/0.15));
    // Bob: the D17V view bob's phase when it runs (the eye and the gun step
    // together), else our own at the same tempo.
    const bool bob=parts&motion_bob;
    const double bob_target=bob && ground && ground_speed>30 ?
        std::min(2.0,std::pow(ground_speed/walk,0.43))*std::min(1.0,ground_speed/180) : 0;
    motion.bob_amount+=(bob_target-motion.bob_amount)*(1-std::exp(-dt/(bob_target>motion.bob_amount?0.18:0.12)));
    if(motion.bob_amount<0.003 && bob_target==0) motion.bob_amount=0;
    double phase;
    if(view_bob_scale()>0 && bob_amount>0) phase=bob_phase;
    else {
        if(motion.bob_amount>0) motion.own_phase=std::fmod(motion.own_phase+bob_rate*60*dt,3.14159265358979);
        phase=motion.own_phase;
    }
    if(phase+1.0<motion.last_phase) motion.stride=motion.stride>0.5 ? 0 : 1;   // a step: the other foot
    motion.last_phase=phase;
    // Run carry, cut by any shot.
    const bool firing=kick>0.05 || input_snapshot(Context::Gameplay).held[fire];
    motion.since_fire=firing ? 0 : motion.since_fire+dt;
    motion.fire_w+=((firing?1.0:0.0)-motion.fire_w)*(1-std::exp(-dt/(firing?0.04:0.25)));
    const bool carry_on=(parts&motion_carry) && ground && ground_speed>walk*2.5 && motion.since_fire>T.carry_hold;
    const double carry_tau=carry_on ? T.carry_in : T.carry_out;
    motion.carry+=((carry_on?1.0:0.0)-motion.carry)*(1-std::exp(-dt/carry_tau));
    // Breath, standing still with no gesture playing.
    const bool still=ground && ground_speed<20 && std::abs(w_turn)<5 && std::abs(w_lift)<5;
    const double breath_target=(parts&motion_breath) && still ? std::max(0.0,1-fp_idle_weight) : 0;
    motion.breath_w+=(breath_target-motion.breath_w)*(1-std::exp(-dt/(breath_target>motion.breath_w?1.2:0.25)));
    motion.breath_t=std::fmod(motion.breath_t+dt,T.breath_period);
    // Compose.
    double p[6]={0,0,0,0,0,0};
    // Look: yaw and pitch lag, the gun trailing in x/y, a little roll.
    const double ly=motion.look[0].x,lp=motion.look[1].x;
    p[1]+=ly; p[0]+=lp; p[2]-=ly*T.look_roll;
    p[3]+=ly*T.look_shift; p[4]-=lp*T.look_shift;
    // Inertia offsets and their rotations.
    double body[3];
    for(int i=0;i<3;i+=2) body[i]=T.inertia_max[i]*std::tanh(motion.body[i].x/T.inertia_max[i]);
    body[1]=0;
    p[3]+=body[0]; p[4]+=vertical; p[5]+=body[2];
    p[0]-=vertical*T.inertia_pitch+land*T.land_pitch; p[2]+=body[0]*T.inertia_roll;
    p[2]+=motion.strafe*T.strafe_roll;
    // Bob: one side sway per stride, one dip per step, lowest as the gun
    // crosses the centre (Duke 3D's arc, the 2:1 ratio of every reference);
    // the gun lags the step by bob_lag radians. Firing eases it down so the
    // muzzle holds while shots leave.
    if(motion.bob_amount>0) {
        const double a=motion.bob_amount*(1-(1-T.bob_fire)*motion.fire_w);
        const double s=phase-T.bob_lag, theta=s+3.14159265358979*motion.stride;
        const double dip=std::cos(s)*std::cos(s), side=std::sin(theta);
        p[3]+=T.bob_x*a*side; p[4]+=T.bob_y*a*dip;
        p[2]+=T.bob_roll*a*side; p[1]+=T.bob_yaw*a*side; p[0]-=T.bob_pitch*a*dip;
    }
    if(motion.carry>0) {
        const double k=motion.carry;
        p[0]+=T.carry_pitch*k; p[1]+=T.carry_yaw*k; p[2]+=T.carry_roll*k;
        p[3]+=T.carry_x*k; p[4]+=T.carry_y*k; p[5]+=T.carry_z*k;
    }
    if(motion.breath_w>0) {
        const double b=std::sin(motion.breath_t/T.breath_period*2*3.14159265358979)*motion.breath_w;
        p[0]+=T.breath_pitch*b; p[4]-=T.breath_y*b; p[2]+=T.breath_roll*b;
    }
    motion_slot_shape(slot,p);
    const double k=scale*motion.weight;
    for(int i=0;i<6;++i) motion_pose_live[i]=p[i]*k;
    shotgun_compose(motion_pose_live);
    static const bool log=std::getenv("DNTTK_FP_MOTION_LOG")!=nullptr;
    if(log) std::fprintf(stderr,"[TTK fp motion] dt %.4f mode %u slot %u w %.2f turn %.0f lift %.0f vel %.0f %.0f %.0f bob %.2f ph %.2f st %.0f carry %.2f breath %.2f pose %.2f %.2f %.2f %.1f %.1f %.1f\n",
        dt,mode,slot,motion.weight,w_turn,w_lift,motion.vel[0],motion.vel[1],motion.vel[2],motion.bob_amount,phase,motion.stride,motion.carry,motion.breath_w,
        motion_pose_live[0],motion_pose_live[1],motion_pose_live[2],motion_pose_live[3],motion_pose_live[4],motion_pose_live[5]);
}
static void motion_view(double* out) {
    std::memcpy(out,render_override ? override_motion_pose : motion_pose_live,6*sizeof(double));
}
static double motion_value(int i) {return render_override ? override_motion_pose[i] : motion_pose_live[i];}
static const char* motion_debug_json() {
    static char buf[2600];
    std::snprintf(buf,sizeof buf,"{\"rolls\":\"%s\",\"scale\":%.2f,\"updates\":%llu,\"resets\":%llu,\"landings\":%llu,\"impact\":%.0f,\"weight\":%.3f,\"bob\":%.3f,\"carry\":%.3f,\"breath\":%.3f,\"land\":%.3f,\"slide\":%.3f,\"grip\":%.3f,\"subunit\":%.0f,\"subunit_limited\":%llu,\"pose\":[%.3f,%.3f,%.3f,%.3f,%.3f,%.3f],\"muzzle_scale\":%.2f,\"muzzle_flashes\":%llu,\"muzzle_ok\":%s,\"muzzle_tip\":[%.1f,%.1f,%.1f],\"attached_locks\":%llu}",
        arm_roll_json(),input_modernized() ? weapon_motion_scale() : 0.0,(unsigned long long)motion.updates,(unsigned long long)motion.resets,(unsigned long long)motion.landings,motion.last_impact,motion.weight,motion.bob_amount,motion.carry,motion.breath_w,motion.land_now,
        motion_pose_live[6],motion_pose_live[7],view_subunit_frame,(unsigned long long)view_subunit_limited,
        motion_pose_live[0],motion_pose_live[1],motion_pose_live[2],motion_pose_live[3],motion_pose_live[4],motion_pose_live[5],
        drawn_muzzle_scale(),(unsigned long long)muzzle_scaled_flashes,fp_muzzle_ok?"true":"false",fp_muzzle_view[0],fp_muzzle_view[1],fp_muzzle_view[2],(unsigned long long)attached_locks);
    return buf;
}
