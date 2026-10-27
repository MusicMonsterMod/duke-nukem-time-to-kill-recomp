// D08K4: Modernized crouch walking (documentation/187-d08k4-crouch-walking.md).
// CROUCH MODERN (default): held crouch plus a direction walks low instead of
// the original roll (181-184). ORIGINAL keeps the roll; Vanilla never gets here.
//
// While Duke holds the stationary crouch (lower animation 178, crouch.inc's
// stance) and a direction is held, the host owns his legs and position:
// - the direction bits never reach the original (pc_input.cpp masks them via
//   crouch_walk_input_ready()), so its crouch handler never starts a roll;
// - Duke faces the view, and moves camera-relative through the D08K2 crouch
//   probe (crouch hull, walls, objects, edges, steps) at the Modernized
//   walking/running pace selected by the configured speed action;
// - channel 0 (player+0x60) keeps animation 178 but plays the crouch-walk clips
//   (ttk-anim.pack extra clips 1-4: forward, backward, +x and -x strafe, solved
//   by ttk_crouch_walk.py): the host sets its previous/current frames and the
//   ticks left each update from a gait phase that advances with the distance
//   moved, so the planted feet match the travel at any speed. The clips' long
//   frames (30000 ticks) keep the original stepper from advancing them (it
//   would loop within animation 178's clip, i.e. walk on past the extra clips;
//   a padding clip follows them in the pack);
// - held still, extra clip 5 holds the original one-knee kneel. Clip changes
//   blend from the last displayed pose, including interrupted transitions;
// - the pelvis follows the clip's height (300 + 10 cos(4 pi phase) over the
//   feet), eased from the kneel's 282 on entry.
// The arms and torso (channels 1-3) stay with the original: crouch idle, aim,
// fire. Release stands up through crouch.inc where the headroom allows;
// until then the crouch walk keeps the directions (D08K7).
// Clip design (ttk_crouch_walk.py): a captured crouch walk retargeted onto
// Duke (assets/anim/crouch-walk-mocap.json, CC-BY 4.0), pelvis about 300 with
// the capture's bob (carried per frame) and torso lean, 640 units a cycle.
static constexpr double crouch_walk_cycle=640.0, crouch_strafe_cycle=380.0;   // ttk_crouch_walk.py CYCLE, STRAFE_CYCLE
static constexpr double crouch_walk_pelvis=300.0, crouch_walk_bob=10.0, crouch_kneel_pelvis=282.0;
static constexpr unsigned crouch_walk_ticks=30000, crouch_walk_ramp=6, crouch_idle_clip=5;
// D08K12: share standing locomotion's configured speed modifier / RUN MODE.
// Only travel changes: the crouch hull and distance-driven gait keep ownership.
// Original swimming's forced shallow-water run must not override this stance.
static double crouch_walk_step() {return modern_pace_step(input_running(false));}
static std::atomic<bool> crouch_walk_owning{false};
// Host state: which extra clip channel 0 shows (0 = none: the original's own
// kneel), the gait phase and the remaining pose transition.
static unsigned crouch_walk_clip,crouch_walk_ramp_left;
static double crouch_walk_phase,crouch_walk_height,crouch_transition_height;
static std::array<std::array<double,4>,32> crouch_display_pose{},crouch_transition_pose{};
static uint32_t crouch_display_mask,crouch_transition_mask;
static double crouch_transition_t=1.0;
static double crouch_fire_weight=0.0;
static unsigned crouch_walk_starts,crouch_walk_updates,crouch_walk_stops,crouch_walk_blocked;
static bool crouch_walk_modern() {
    const char* v=ttk_live_env("DNTTK_CROUCH");
    return !v || std::strcmp(v,"original");
}
// pc_input.cpp: withhold every direction from the original while the crouch
// walk owns the stance (held crouch, crouched, Modern, clips loaded).
bool crouch_walk_input_ready() {return crouch_walk_owning.load();}
// A host clip's frames after its leading sentinel (see ttk_crouch_walk.py
// host_clip: the original blend hangs going straight to a clip's first frame).
static bool crouch_walk_frames(unsigned clip,uint32_t& first,uint32_t& count) {
    if(!ttk::anim_extra_clip(clip,first,count) || count<2) return false;
    ++first;--count;return true;
}
static bool crouch_walk_eligible(bool landed=false) {
    uint32_t first,count;
    const unsigned animation=psx_mod_read_half(player+0x60);
    return input_modernized() && crouch_walk_modern() && crouch_owned && (animation==176 || animation==178) &&
           player_identity_ready() && !(psx_mod_read_word(player)&0x20002) &&
           crouch_ground_state(landed) && crouch_water_supported() &&
           crouch_walk_frames(1,first,count) && crouch_walk_frames(crouch_idle_clip,first,count);
}
// The pose last given to channel 0, applied again to Duke's joints right
// before his model is built (crouch_walk_pose): with a weapon drawn the
// original armed path resets the legs after the blend, and the pelvis
// rotation (the lean) is never taken from the channel.
static uint32_t crouch_pose_from,crouch_pose_to;static double crouch_pose_t;static bool crouch_pose_valid;
static void crouch_walk_channel(uint32_t from,uint32_t to,double t) {
    if(t<0)t=0;if(t>0.98)t=0.98;
    crouch_pose_from=from;crouch_pose_to=to;crouch_pose_t=t;crouch_pose_valid=true;
    const uint32_t c=player+0x60;
    psx_mod_write_half(c+4,uint16_t(from));
    psx_mod_write_half(c+6,uint16_t(to));
    psx_mod_write_half(c+2,uint16_t(std::lround(crouch_walk_ticks*(1.0-t))));
    psx_mod_write_half(c+0xa,0);
}
static void crouch_walk_pelvis_height(double height) {
    crouch_walk_height=height;
    psx_mod_write_word(player+8,psx_mod_read_word(player+0x1c8)-uint32_t(int32_t(std::lround(height))));
}
// Pelvis height a host frame carries (its root command y), 0 when none.
static double crouch_frame_pelvis(uint32_t frame) {
    const uint32_t frm=psx_mod_read_word(0x800d8324),rot=psx_mod_read_word(0x800d8320);
    const uint32_t w1=psx_mod_read_word(frm+8*frame+4),begin=w1&0xffff,count=w1>>16;
    for(uint32_t c=begin;c<begin+count && c<begin+32;++c) {
        const uint32_t a=psx_mod_read_word(rot+8*c);
        if((a>>29)&1) {const uint32_t y=(a>>14)&0x3fff;return double(int32_t(y&0x2000?y-0x4000:y));}
    }
    return 0;
}
// Not in control (not eligible, another animation, a savestate load, focus
// lost...) but channel 0 still on a crouch-walk frame: hand it back to the
// first frame of the current animation's own legs clip with that frame's
// ticks. Left alone, the host frames' 30000 ticks would hold Duke static for
// two minutes (user report, 2026-10-22: "duke's entire animation is broken and
// becomes static").
static unsigned crouch_walk_releases,crouch_walk_upper_fixes,crouch_mask_fixes;
// Put channel ch (0-3) on the first frame of its animation's own clip.
static bool crouch_channel_restart(unsigned ch) {
    const uint32_t c=player+0x60+0x14*ch;
    const unsigned animation=psx_mod_read_half(c);
    const unsigned clip=animation<336 ? psx_mod_read_half(0x800c2d64+8*animation+2*ch) : 0xffff;
    if(clip>=2048)return false;
    const uint32_t rec=psx_mod_read_word(0x800d8328+4*clip),first=rec&0xffff;
    if(!(rec>>16) || first==0xffff)return false;
    const uint32_t ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*first)>>16;
    psx_mod_write_half(c+4,uint16_t(first));psx_mod_write_half(c+6,uint16_t(first));
    psx_mod_write_half(c+2,uint16_t(ticks?ticks:30));
    return true;
}
static bool crouch_extra_range(uint32_t& lo,uint32_t& hi) {
    uint32_t lo_count,pad,pad_count;
    if(!ttk::anim_extra_clip(1,lo,lo_count) || !ttk::anim_extra_clip(99,pad,pad_count))return false;
    hi=pad+pad_count;return true;
}
static void crouch_walk_release() {
    uint32_t lo,hi;
    if(!crouch_extra_range(lo,hi))return;
    const uint32_t c=player+0x60;
    const uint32_t pf=psx_mod_read_half(c+4),cf=psx_mod_read_half(c+6);
    if(!((pf>=lo && pf<hi) || (cf>=lo && cf<hi)))return;
    if(!crouch_channel_restart(0)) {
        // No legs clip for this animation: the kneel's.
        const uint32_t rec=psx_mod_read_word(0x800d8328+4*psx_mod_read_half(0x800c2d64+8*178)),first=rec&0xffff;
        psx_mod_write_half(c+4,uint16_t(first));psx_mod_write_half(c+6,uint16_t(first));psx_mod_write_half(c+2,150);
    }
    ++crouch_walk_releases;
}
// Animation 178 keeps its upper channels (2, 3) in step with the legs by frame
// offset, so while channel 0 plays a crouch-walk frame the game moves them onto
// crouch-walk frames too: they then play leg joints and their joint masks
// (+0xc / +0x10, only ever OR-ed) keep the leg bits, and from then on, in
// every animation, the blend's fade (0x80059324) set the legs to identity:
// straight, static legs after crouching (user video 2026-10-22). Every update:
// upper channels never stay on a crouch-walk frame, and their masks keep only
// the joints the channel's own original clips animate (scan of every clip in
// DB00-09, D08K6): right arm 13-15, left arm 10-12, chest/head 9, 16-18, 20,
// 22, 23. Armed, animation 178's frame-offset sync also walked channels 2 and
// 3 onto neighbouring clips (the right arm's, the chest's), so they kept the
// right arm and chest bits and every later animation faded both arms to
// identity: arms hanging straight down after crouching (user report D08K6).
static constexpr uint32_t crouch_channel_joints[4]={0,0xe000u,0x1c00u,
    (1u<<9)|(1u<<16)|(1u<<17)|(1u<<18)|(1u<<20)|(1u<<22)|(1u<<23)};
static void crouch_channels_sanitize() {
    uint32_t lo,hi;
    if(!crouch_extra_range(lo,hi))return;
    for(unsigned ch=1;ch<4;++ch) {
        const uint32_t c=player+0x60+0x14*ch;
        const uint32_t pf=psx_mod_read_half(c+4),cf=psx_mod_read_half(c+6);
        if((pf>=lo && pf<hi) || (cf>=lo && cf<hi)) {crouch_channel_restart(ch);++crouch_walk_upper_fixes;}
        for(uint32_t m:{c+0xc,c+0x10}) {
            const uint32_t v=psx_mod_read_word(m);
            if(v&~crouch_channel_joints[ch]) {psx_mod_write_word(m,v&crouch_channel_joints[ch]);++crouch_mask_fixes;}
        }
    }
}
static void crouch_walk_update(CPUState* cpu,bool landed=false) {
    crouch_channels_sanitize();
    const auto& f=input_snapshot(Context::Gameplay);
    // D08K7: also while a released crouch waits for room to stand (directions
    // crouch walk; the original would roll).
    const bool eligible=crouch_walk_eligible(landed) && f.active && (f.held[crouch] || crouch_stand_pending);
    crouch_walk_owning.store(eligible);
    if(!eligible || psx_mod_read_half(player+0x60)!=178) {
        if(crouch_walk_clip) ++crouch_walk_stops;
        crouch_walk_clip=0;crouch_pose_valid=false;
        crouch_display_mask=crouch_transition_mask=0;crouch_transition_t=1.0;crouch_fire_weight=0.0;
        crouch_walk_release();
        return;
    }
    const bool moving=f.move_x || f.move_y;
    // Clip: the one-knee idle while still, otherwise the dominant input axis
    // (Duke faces the view: +x is his right).
    const unsigned clip=!moving ? crouch_idle_clip :
        std::fabs(f.move_y)>=std::fabs(f.move_x) ? (f.move_y>0?1u:2u) : (f.move_x>0?3u:4u);
    uint32_t first,count;
    if(!crouch_walk_frames(clip,first,count))return;
    if(clip!=crouch_walk_clip) {
        if(!crouch_walk_clip) {crouch_walk_phase=0;if(!landed)crouch_walk_height=crouch_kneel_pelvis;++crouch_walk_starts;}
        crouch_transition_pose=crouch_display_pose;
        crouch_transition_mask=crouch_display_mask;
        crouch_transition_height=crouch_walk_height;
        crouch_walk_ramp_left=crouch_walk_ramp;
        crouch_walk_clip=clip;
    }
    double target=crouch_walk_pelvis;
    face_view();
    if(moving) {
        // Move through the crouch probe, sliding along a blocked axis.
        const auto dir=movement(view_yaw(),f.move_x,f.move_y);
        const double mag=std::min(1.0,double(std::hypot(f.move_x,f.move_y)));
        const double step_len=crouch_walk_step()*mag;
        int32_t dx=int32_t(std::lround(dir.x*step_len)),dz=int32_t(std::lround(dir.z*step_len));
        int32_t step=0,mx=dx,mz=dz;
        HullBlock b=crouch_probe(cpu,dx,dz,step,crouch_hull,true);
        if(b!=hull_clear && dx && dz) {
            mx=dx;mz=0;b=crouch_probe(cpu,mx,mz,step,crouch_hull,true);
            if(b!=hull_clear) {mx=0;mz=dz;b=crouch_probe(cpu,mx,mz,step,crouch_hull,true);}
        }
        double moved=0;
        if(b==hull_clear) {
            psx_mod_write_word(player+4,psx_mod_read_word(player+4)+uint32_t(mx));
            psx_mod_write_word(player+0xc,psx_mod_read_word(player+0xc)+uint32_t(mz));
            if(step>crouch_step_down && (mx || mz)) {
                // The destination lost support. Keep the departure height and
                // let the original fall own gravity, swept collision and landing.
                // Never snap down to the probed floor or launch a jump.
                crouch_walk_release();
                if(original_call(cpu,0x8003f060,1)) {
                    const unsigned dt=psx_mod_read_half(player+0xba);
                    const double speed=dt ? std::min(2048.0,std::hypot(double(mx),double(mz))*1024.0/dt) : 0;
                    const double len=std::hypot(double(mx),double(mz));
                    psx_mod_write_word(player+0x1f4,int32_t(std::lround(mx*speed/len)));
                    psx_mod_write_word(player+0x1fc,int32_t(std::lround(mz*speed/len)));
                    crouch_owned=crouch_stand_pending=false;
                    crouch_walk_owning.store(false);crouch_walk_clip=0;crouch_pose_valid=false;
                    crouch_display_mask=crouch_transition_mask=0;crouch_fire_weight=0;
                    ++hull_moves;
                    return;
                }
                // Guest calls may be unavailable during a precise replay.
                psx_mod_write_word(player+4,psx_mod_read_word(player+4)-uint32_t(mx));
                psx_mod_write_word(player+0xc,psx_mod_read_word(player+0xc)-uint32_t(mz));
                ++crouch_walk_blocked;
                return;
            }
            psx_mod_write_word(player+0x1c8,psx_mod_read_word(player+0x1c8)+uint32_t(step));
            moved=std::hypot(double(mx),double(mz));
            ++hull_moves;
        } else ++crouch_walk_blocked;
        crouch_walk_phase=std::fmod(crouch_walk_phase+moved/(clip>=3?crouch_strafe_cycle:crouch_walk_cycle),1.0);
    }
    // Channel 0: a blend into the new clip first, then the gait (or the idle).
    const double idx=crouch_walk_phase*count;
    const unsigned k=moving ? unsigned(idx)%count : 0;
    const double t=moving ? idx-std::floor(idx) : 0.0;
    crouch_transition_t=1.0;
    if(crouch_walk_ramp_left) {
        const double ramp=1.0-double(crouch_walk_ramp_left)/double(crouch_walk_ramp+1);
        crouch_transition_t=ramp*ramp*(3.0-2.0*ramp);
        crouch_walk_channel(first+k,first+(moving?(k+1)%count:0),t);
        if(--crouch_walk_ramp_left==0 && !moving) crouch_walk_channel(first,first,0);
    } else if(moving) crouch_walk_channel(first+k,first+(k+1)%count,t);
    else crouch_walk_channel(first,first,0);
    // The clip's own pelvis height (its bob), eased only while blending in.
    const double ha=crouch_frame_pelvis(crouch_pose_from),hb=crouch_frame_pelvis(crouch_pose_to);
    if(ha>0 && hb>0) target=ha+(hb-ha)*crouch_pose_t;
    else if(hb>0) target=hb;
    else if(moving) target+=crouch_walk_bob*std::cos(4*3.141592653589793*crouch_walk_phase);
    crouch_walk_pelvis_height(crouch_transition_height+(target-crouch_transition_height)*crouch_transition_t);
    const double lean_target=moving ? 1.0 : 0.0;
    crouch_fire_weight+=std::clamp(lean_target-crouch_fire_weight,-1.0/crouch_walk_ramp,1.0/crouch_walk_ramp);
    ++crouch_walk_updates;
}
// Joint quaternions of one frame (merged tables): ROT joint id -> Q12 x,y,z,w.
static int32_t crouch_frame_height;   // pelvis height a host frame carries (root command y), 0 = none
static bool crouch_frame_quats(uint32_t frame,std::array<std::array<int32_t,4>,32>& q,uint32_t& mask) {
    crouch_frame_height=0;
    const uint32_t frm=psx_mod_read_word(0x800d8324),rot=psx_mod_read_word(0x800d8320);
    const uint32_t w1=psx_mod_read_word(frm+8*frame+4),begin=w1&0xffff,count=w1>>16;
    if(count>32)return false;
    mask=0;
    for(uint32_t c=begin;c<begin+count;++c) {
        const uint32_t a=psx_mod_read_word(rot+8*c),b=psx_mod_read_word(rot+8*c+4);
        if((a>>29)&1) {         // root translation: the clip's pelvis height in y
            const uint32_t y=(a>>14)&0x3fff;crouch_frame_height=int32_t(y&0x2000?y-0x4000:y);continue;
        }
        const unsigned j=b>>27;
        auto sx=[](uint32_t v,int bits){v&=(1u<<bits)-1;return int32_t(v&(1u<<(bits-1))?v-(1u<<bits):v);};
        q[j]={sx(a,14),sx(a>>14,14),sx(b,14),sx(b>>14,13)};mask|=1u<<j;
    }
    return true;
}
// D08K6 upper body: the chest leans over the knees, the head looks ahead and,
// where no weapon owns them, the arms carry the fists up front (host clips
// 11-15, the same frames as the legs clips 1-5, ttk_crouch_walk.py). Written
// here, after every channel blend, so the original's churn on the upper
// channels under animation 178 (they follow channel 0 by frame offset and
// crouch_channels_sanitize puts them back) never reaches the model: that
// churn moved the chest and left arm by 2-26 degrees every few updates with a
// weapon drawn (the left-arm shake, user report 2026-10-22). An arm that a
// weapon pose owns (its channel not on 178) keeps the world orientation the
// pose was made for (upright pelvis, the pose's own chest):
// A' = A (x) C (x) P'^-1 (x) C'^-1 in game quaternion order. The override
// ramps in and out over crouch_walk_ramp updates.
using CrouchQuat=std::array<double,4>;   // game order x, y, z, w (unit)
// Chest lean while firing crouched (degrees from the view look-at): the walk's
// 62 less a slight raise.
static constexpr double crouch_fire_lean=50.0;
static bool crouch_fire_tilting;
// 0x80097a44 (the original aim's look-at) for the chest while firing crouched:
// the view direction d tilted down by crouch_fire_lean about the look-at's
// right axis r = (d.z, 0, -d.x); u = d x r points down (y is down).
static bool crouch_fire_direction(int32_t dir[3]) {
    if(!crouch_fire_tilting)return false;
    double d[3]={double(dir[0]),double(dir[1]),double(dir[2])};
    const double n=std::sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
    double r[3]={d[2],0,-d[0]};
    const double rn=std::sqrt(r[0]*r[0]+r[2]*r[2]);
    if(n<1 || rn<1)return false;
    for(double& v:d)v/=n;
    for(double& v:r)v/=rn;
    const double u[3]={d[1]*r[2]-d[2]*r[1],d[2]*r[0]-d[0]*r[2],d[0]*r[1]-d[1]*r[0]};
    const double a=crouch_fire_weight*crouch_fire_lean*3.141592653589793/180.0,c=std::cos(a),s=std::sin(a);
    for(int i=0;i<3;++i)dir[i]=int32_t(std::lround((d[i]*c+u[i]*s)*4096.0));
    return true;
}
static constexpr unsigned crouch_upper_ids[8]={9,16,10,11,12,13,14,15};
static double crouch_upper_weight;
static CrouchQuat crouch_upper_last[8];
static uint32_t crouch_upper_last_mask;
static unsigned crouch_upper_writes,crouch_upper_compensated,crouch_upper_aims;
static CrouchQuat crouch_qmul(const CrouchQuat& a,const CrouchQuat& b) {
    return {a[3]*b[0]+a[0]*b[3]+a[1]*b[2]-a[2]*b[1], a[3]*b[1]-a[0]*b[2]+a[1]*b[3]+a[2]*b[0],
            a[3]*b[2]+a[0]*b[1]-a[1]*b[0]+a[2]*b[3], a[3]*b[3]-a[0]*b[0]-a[1]*b[1]-a[2]*b[2]};
}
static CrouchQuat crouch_qinv(const CrouchQuat& a) {return {-a[0],-a[1],-a[2],a[3]};}
static CrouchQuat crouch_qnorm(CrouchQuat a) {
    const double n=std::sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]+a[3]*a[3]);
    if(n<1e-9) return {0,0,0,1};
    for(double& v:a)v/=n;
    return a;
}
static CrouchQuat crouch_slerp(const CrouchQuat& a,CrouchQuat b,double t) {
    double c=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
    if(c<0){for(double& v:b)v=-v;c=-c;}
    double wa=1-t,wb=t;
    if(1-c>=256/4096.0){const double th=std::acos(std::min(c,1.0)),s=std::sin(th);wa=std::sin((1-t)*th)/s;wb=std::sin(t*th)/s;}
    return crouch_qnorm({a[0]*wa+b[0]*wb,a[1]*wa+b[1]*wb,a[2]*wa+b[2]*wb,a[3]*wa+b[3]*wb});
}
static uint32_t crouch_joint_record(uint32_t desc,unsigned rot_id) {
    const unsigned record=psx_mod_read_byte(desc+0x24+rot_id);
    return record<duke_joints ? desc+0x44+0x28*record+4 : 0;
}
static CrouchQuat crouch_record_q(uint32_t r) {
    CrouchQuat q;
    for(int i=0;i<4;++i)q[i]=double(int32_t(psx_mod_read_word(r+4*i)));
    return crouch_qnorm(q);
}
static void crouch_record_write(uint32_t r,const CrouchQuat& q) {
    for(int i=0;i<4;++i)psx_mod_write_word(r+4*i,uint32_t(int32_t(std::lround(q[i]*4096.0))));
}
// The upper clip frame matching a legs clip frame (0 = none: an original frame).
static uint32_t crouch_upper_frame(uint32_t legs_frame) {
    for(unsigned c=1;c<=crouch_idle_clip;++c) {
        uint32_t f,n,uf,un;
        if(!ttk::anim_extra_clip(c,f,n) || legs_frame<f || legs_frame>=f+n)continue;
        return ttk::anim_extra_clip(c+10,uf,un) && un==n ? uf+(legs_frame-f) : 0;
    }
    return 0;
}
// A channel's current pose from its own frames: previous -> current frame,
// t = (ticks - left) / ticks of the current frame, as 0x80059004 blends.
static bool crouch_channel_pose(unsigned ch,std::array<CrouchQuat,32>& out,uint32_t& mask) {
    const uint32_t c=player+0x60+0x14*ch;
    const uint32_t pf=psx_mod_read_half(c+4),cf=psx_mod_read_half(c+6);
    if(pf==0xffff || cf==0xffff)return false;
    std::array<std::array<int32_t,4>,32> a{},b{};uint32_t ma=0,mb=0;
    if(!crouch_frame_quats(pf,a,ma) || !crouch_frame_quats(cf,b,mb))return false;
    const uint32_t ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*cf)>>16,left=psx_mod_read_half(c+2);
    const double t=ticks ? std::clamp(double(int32_t(ticks)-int32_t(left))/double(ticks),0.0,1.0) : 1.0;
    mask=ma|mb;
    for(unsigned j=0;j<32;++j) {
        if(!(mask>>j&1))continue;
        auto v=[&](const std::array<std::array<int32_t,4>,32>& x,uint32_t m,const std::array<std::array<int32_t,4>,32>& other) {
            const auto& s=(m>>j&1) ? x[j] : other[j];
            return crouch_qnorm({double(s[0]),double(s[1]),double(s[2]),double(s[3])});
        };
        out[j]=crouch_slerp(v(a,ma,b),v(b,mb,a),t);
    }
    return true;
}
// Upper body onto the joint records: ours where owning, else blended back to
// the game's over the ramp.
static void crouch_walk_upper(uint32_t desc,bool owning) {
    // Firing: the original sets +0x224 bit 0x100 and its aim build
    // (0x80097c04, 0x80097a44 with the D07A view direction) replaces the chest's
    // world rotation with a look-at of the view; the arms and head hang from
    // that. Our lean and weapon-arm compensation assume the leaned chest, so on
    // top of the aimed chest the gun pointed 50-58 degrees up ("shoots directly
    // up", user D08K6). While the bit is set the upper joints get their own
    // channels' pose (fresh, never the records), and, while the crouch walk
    // owns Duke, the chest's look-at is tilted down by crouch_fire_lean
    // (crouch_fire_direction, called from the 0x80097a44 hook) so he stays
    // hunched instead of snapping upright (user: "a slight raise, if any ... like
    // how gracefully we accomplished the swimming animation"); both shoulders are
    // turned back by the same tilt, so the arms and gun keep the original aim
    // (the swim fire pose D08O2A does the same with the shoulders). The head
    // has its own look-at and keeps looking at the view.
    crouch_fire_tilting=false;
    if(psx_mod_read_word(player+0x224)&0x100) {
        if(owning || crouch_upper_weight>0) {
            std::array<CrouchQuat,32> q[4]{};uint32_t m[4]{};
            for(unsigned ch=1;ch<4;++ch) if(!crouch_channel_pose(ch,q[ch],m[ch])) m[ch]=0;
            // M(q') = A_tilt^T M(q): q' = q (x) (sin(-lean/2), 0, 0, cos(lean/2)).
            const double h=-crouch_fire_weight*crouch_fire_lean*3.141592653589793/360.0;
            const CrouchQuat untilt{std::sin(h),0,0,std::cos(h)};
            for(unsigned j:crouch_upper_ids) {
                const unsigned ch=j>=13 && j<=15 ? 1 : j>=10 && j<=12 ? 2 : 3;
                const uint32_t r=crouch_joint_record(desc,j);
                if(!r || !(m[ch]>>j&1))continue;
                crouch_record_write(r,owning && (j==10 || j==13) ? crouch_qnorm(crouch_qmul(q[ch][j],untilt)) : q[ch][j]);
            }
            crouch_fire_tilting=owning;
            ++crouch_upper_aims;
        }
        // Keep the lean's weight while the crouch walk owns Duke: after the shot
        // the chest returns to the lean directly, not from upright.
        if(!owning) crouch_upper_weight=0;
        crouch_upper_last_mask=0;
        return;
    }
    const double step=1.0/double(crouch_walk_ramp);
    crouch_upper_weight=owning ? std::min(1.0,crouch_upper_weight+step) : std::max(0.0,crouch_upper_weight-step);
    if(crouch_upper_weight<=0 && !owning) {crouch_upper_last_mask=0;return;}
    std::array<std::array<int32_t,4>,32> a{},b{};uint32_t ma=0,mb=0;
    if(owning) {
        const uint32_t fa=crouch_upper_frame(crouch_pose_from),fb=crouch_upper_frame(crouch_pose_to);
        if(fa && !crouch_frame_quats(fa,a,ma))ma=0;
        if(fb && !crouch_frame_quats(fb,b,mb))mb=0;
        if(!ma && !mb)return;
    }
    // Arms: ours while their channel follows the legs (animation 178).
    const bool right_ours=psx_mod_read_half(player+0x74)==178,left_ours=psx_mod_read_half(player+0x88)==178;
    const uint32_t pelvis_r=crouch_joint_record(desc,0),chest_r=crouch_joint_record(desc,9);
    if(!pelvis_r || !chest_r)return;
    uint32_t mask=0;
    for(unsigned i=0;i<8;++i) {
        const unsigned j=crouch_upper_ids[i];
        const bool arm=j>=10 && j<=15,ours=!arm || (j<=12 ? left_ours : right_ours);
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r || !ours)continue;
        CrouchQuat target;
        if(owning) {
            const CrouchQuat game=crouch_record_q(r);
            auto q=[&](const std::array<std::array<int32_t,4>,32>& v,uint32_t m) {
                return (m>>j&1) ? crouch_qnorm({double(v[j][0]),double(v[j][1]),double(v[j][2]),double(v[j][3])}) : game;
            };
            target=crouch_slerp(q(a,ma),q(b,mb),crouch_pose_t);
            crouch_upper_last[i]=target;
        } else if(crouch_upper_last_mask>>i&1) target=crouch_upper_last[i];
        else continue;
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),target,crouch_upper_weight));
        mask|=1u<<i;
    }
    if(owning) crouch_upper_last_mask=mask;
    // Weapon-owned arms: the pose comes from the arm's own channel frames (the
    // blend 0x80059004 makes), never from the records: while Duke holds still
    // the channel's frame does not change and the records can still hold our
    // previous update's values, so correcting them compounded (the right
    // forearm and hand went to identity, lab D08K6). The shoulder keeps the
    // world orientation the pose was made for (an upright pelvis and channel
    // 3's own chest frame) under our pelvis and chest.
    CrouchQuat chest_ref{0,0,0,1};
    {
        std::array<CrouchQuat,32> q{};uint32_t m=0;
        if(crouch_channel_pose(3,q,m) && (m>>9&1)) chest_ref=q[9];
    }
    const CrouchQuat pelvis_now=crouch_record_q(pelvis_r),chest_now=crouch_record_q(chest_r);
    const CrouchQuat fix=crouch_qmul(crouch_qmul(chest_ref,crouch_qinv(pelvis_now)),crouch_qinv(chest_now));
    for(unsigned ch:{1u,2u}) {
        if(ch==1 ? right_ours : left_ours)continue;
        std::array<CrouchQuat,32> q{};uint32_t m=0;
        if(!crouch_channel_pose(ch,q,m))continue;
        const unsigned first=ch==1 ? 13 : 10;
        for(unsigned j=first;j<first+3;++j) {
            const uint32_t r=crouch_joint_record(desc,j);
            if(!r || !(m>>j&1))continue;
            crouch_record_write(r,j==first ? crouch_qnorm(crouch_qmul(q[j],fix)) : q[j]);
        }
        ++crouch_upper_compensated;
    }
    ++crouch_upper_writes;
}
// 0x800411b8 (player update, last call before the model build): the crouch
// walk's pose onto the legs and pelvis records, slerped as 0x8002a0c8 does.
static void crouch_walk_pose(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634)return;
    crouch_fire_tilting=false;
    crouch_channels_sanitize();
    const uint32_t desc=duke_model();
    if(!desc)return;
    const bool owning=crouch_pose_valid && crouch_walk_owning.load() && psx_mod_read_half(player+0x60)==178;
    if(!owning) {crouch_walk_upper(desc,false);return;}
    std::array<std::array<int32_t,4>,32> a{},b{};uint32_t ma=0,mb=0;
    if(!crouch_frame_quats(crouch_pose_from,a,ma) || !crouch_frame_quats(crouch_pose_to,b,mb))return;
    const double t=crouch_pose_t;
    for(unsigned j=0;j<32;++j) {
        if(!((ma|mb)>>j&1))continue;
        const unsigned record=psx_mod_read_byte(desc+0x24+j);
        if(record>=duke_joints)continue;
        double qa[4],qb[4];
        for(int i=0;i<4;++i){qa[i]=(ma>>j&1)?a[j][i]:(i==3?4096:0);qb[i]=(mb>>j&1)?b[j][i]:(i==3?4096:0);}
        double c=(qa[0]*qb[0]+qa[1]*qb[1]+qa[2]*qb[2]+qa[3]*qb[3])/(4096.0*4096.0);
        if(c<0){for(double& v:qb)v=-v;c=-c;}
        double wa=1-t,wb=t;
        if(1-c>=256/4096.0){const double th=std::acos(std::min(c,1.0)),s=std::sin(th);wa=std::sin((1-t)*th)/s;wb=std::sin(t*th)/s;}
        const uint32_t r=desc+0x44+0x28*record+4;
        for(int i=0;i<4;++i)psx_mod_write_word(r+4*i,uint32_t(int32_t(std::lround(qa[i]*wa+qb[i]*wb))));
    }
    // Blend from the pose actually displayed, not the old clip's next frame.
    // Save the result each update so a rapid stop/restart stays continuous.
    for(unsigned j=0;j<9;++j) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r)continue;
        auto q=crouch_record_q(r);
        if(crouch_transition_mask>>j&1)q=crouch_slerp(crouch_transition_pose[j],q,crouch_transition_t);
        crouch_record_write(r,q);crouch_display_pose[j]=q;crouch_display_mask|=1u<<j;
    }
    crouch_walk_upper(desc,true);
    for(unsigned j:crouch_upper_ids) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r)continue;
        auto q=crouch_record_q(r);
        // Aimed shoulders must retain the exact chest-tilt compensation.
        if(!(psx_mod_read_word(player+0x224)&0x100) && (crouch_transition_mask>>j&1))
            q=crouch_slerp(crouch_transition_pose[j],q,crouch_transition_t);
        crouch_record_write(r,q);crouch_display_pose[j]=q;crouch_display_mask|=1u<<j;
    }
}
