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
//   walking pace;
// - channel 0 (player+0x60) keeps animation 178 but plays the crouch-walk clips
//   (ttk-anim.pack extra clips 1-4: forward, backward, +x and -x strafe, solved
//   by ttk_crouch_walk.py): the host sets its previous/current frames and the
//   ticks left each update from a gait phase that advances with the distance
//   moved, so the planted feet match the travel at any speed. The clips' long
//   frames (30000 ticks) keep the original stepper from advancing them (it
//   would loop within animation 178's clip, i.e. walk on past the extra clips;
//   a padding clip follows them in the pack);
// - held still, Duke keeps the same low stance (extra clip 5, both feet
//   planted) instead of the original kneel, so stopping and starting never pop
//   between the kneel and the walk; every clip change blends over 6 updates;
// - the pelvis follows the clip's height (300 + 10 cos(4 pi phase) over the
//   feet), eased from the kneel's 282 on entry.
// The arms and torso (channels 1-3) stay with the original: crouch idle, aim,
// fire. Release stands up through crouch.inc where the headroom allows.
// Clip design (ttk_crouch_walk.py): a captured crouch walk retargeted onto
// Duke (assets/anim/crouch-walk-mocap.json, CC-BY 4.0), pelvis about 300 with
// the capture's bob (carried per frame) and torso lean, 640 units a cycle.
static constexpr double crouch_walk_cycle=640.0, crouch_strafe_cycle=380.0;   // ttk_crouch_walk.py CYCLE, STRAFE_CYCLE
static constexpr double crouch_walk_pelvis=300.0, crouch_walk_bob=10.0, crouch_kneel_pelvis=282.0;
static constexpr unsigned crouch_walk_ticks=30000, crouch_walk_ramp=6, crouch_idle_clip=5;
// The crouch moves at the Modernized walking pace (user, 2026-10-22: "locked to
// the same speed as walking").
static double crouch_walk_step() {return modern_pace_step(false);}
static std::atomic<bool> crouch_walk_owning{false};
// Host state: which extra clip channel 0 shows (0 = none: the original's own
// kneel), the gait phase, the frame a blend starts from and its remaining ramp.
static unsigned crouch_walk_clip,crouch_walk_ramp_left,crouch_walk_from_frame;
static double crouch_walk_phase,crouch_walk_height;
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
static bool crouch_walk_eligible() {
    uint32_t first,count;
    const unsigned animation=psx_mod_read_half(player+0x60);
    return input_modernized() && crouch_walk_modern() && crouch_owned && (animation==176 || animation==178) &&
           player_identity_ready() && !(psx_mod_read_word(player)&0x20002) &&
           psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 && !in_water_early() &&
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
// The frame channel 0 shows now (the blend's new start when the clip changes).
static uint32_t crouch_walk_frame_now() {return psx_mod_read_half(player+0x66);}
// Not in control (not eligible, another animation, a savestate load, focus
// lost...) but channel 0 still on a crouch-walk frame: hand it back to the
// first frame of the current animation's own legs clip with that frame's
// ticks. Left alone, the host frames' 30000 ticks would hold Duke static for
// two minutes (user report, 2026-10-22: "duke's entire animation is broken and
// becomes static").
static unsigned crouch_walk_releases,crouch_walk_upper_fixes;
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
// upper channels never stay on a crouch-walk frame, and never own a leg joint
// (no original upper track animates one).
static constexpr uint32_t crouch_leg_joints=0x1ffu|(1u<<21);   // ROT 0-8, 21
static void crouch_channels_sanitize() {
    uint32_t lo,hi;
    if(!crouch_extra_range(lo,hi))return;
    for(unsigned ch=1;ch<4;++ch) {
        const uint32_t c=player+0x60+0x14*ch;
        const uint32_t pf=psx_mod_read_half(c+4),cf=psx_mod_read_half(c+6);
        if((pf>=lo && pf<hi) || (cf>=lo && cf<hi)) {crouch_channel_restart(ch);++crouch_walk_upper_fixes;}
        for(uint32_t m:{c+0xc,c+0x10}) {
            const uint32_t v=psx_mod_read_word(m);
            if(v&crouch_leg_joints) psx_mod_write_word(m,v&~crouch_leg_joints);
        }
    }
}
static void crouch_walk_update(CPUState* cpu) {
    crouch_channels_sanitize();
    const auto& f=input_snapshot(Context::Gameplay);
    const bool eligible=crouch_walk_eligible() && f.active && f.held[crouch];
    crouch_walk_owning.store(eligible);
    if(!eligible || psx_mod_read_half(player+0x60)!=178) {
        if(crouch_walk_clip) ++crouch_walk_stops;
        crouch_walk_clip=0;crouch_pose_valid=false;
        crouch_walk_release();
        return;
    }
    const bool moving=f.move_x || f.move_y;
    // Clip: the squat idle while still, otherwise the dominant input axis
    // (Duke faces the view: +x is his right).
    const unsigned clip=!moving ? crouch_idle_clip :
        std::fabs(f.move_y)>=std::fabs(f.move_x) ? (f.move_y>0?1u:2u) : (f.move_x>0?3u:4u);
    uint32_t first,count;
    if(!crouch_walk_frames(clip,first,count))return;
    if(clip!=crouch_walk_clip) {
        if(!crouch_walk_clip) {crouch_walk_phase=0;crouch_walk_height=crouch_kneel_pelvis;++crouch_walk_starts;}
        crouch_walk_from_frame=crouch_walk_frame_now();
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
        HullBlock b=crouch_probe(cpu,dx,dz,step);
        if(b!=hull_clear && dx && dz) {
            mx=dx;mz=0;b=crouch_probe(cpu,mx,mz,step);
            if(b!=hull_clear) {mx=0;mz=dz;b=crouch_probe(cpu,mx,mz,step);}
        }
        double moved=0;
        if(b==hull_clear) {
            psx_mod_write_word(player+4,psx_mod_read_word(player+4)+uint32_t(mx));
            psx_mod_write_word(player+0xc,psx_mod_read_word(player+0xc)+uint32_t(mz));
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
    if(crouch_walk_ramp_left) {
        const double ramp=1.0-double(crouch_walk_ramp_left)/double(crouch_walk_ramp+1);
        crouch_walk_channel(crouch_walk_from_frame,first+(moving?(k+1)%count:0),ramp);
        if(--crouch_walk_ramp_left==0 && !moving) crouch_walk_channel(first,first,0);
    } else if(moving) crouch_walk_channel(first+k,first+(k+1)%count,t);
    else crouch_walk_channel(first,first,0);
    // The clip's own pelvis height (its bob), eased only while blending in.
    const double ha=crouch_frame_pelvis(crouch_pose_from),hb=crouch_frame_pelvis(crouch_pose_to);
    if(ha>0 && hb>0) target=ha+(hb-ha)*crouch_pose_t;
    else if(hb>0) target=hb;
    else if(moving) target+=crouch_walk_bob*std::cos(4*3.141592653589793*crouch_walk_phase);
    crouch_walk_pelvis_height(crouch_walk_ramp_left ? crouch_walk_height+(target-crouch_walk_height)*0.5 : target);
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
// 0x800411b8 (player update, last call before the model build): the crouch
// walk's pose onto the legs and pelvis records, slerped as 0x8002a0c8 does.
static void crouch_walk_pose(CPUState* cpu) {
    if(cpu->gpr[4]==player && cpu->gpr[31]==0x80042634) crouch_channels_sanitize();
    if(!crouch_pose_valid || !crouch_walk_owning.load() || cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634 ||
       psx_mod_read_half(player+0x60)!=178)return;
    const uint32_t desc=duke_model();
    if(!desc)return;
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
}
