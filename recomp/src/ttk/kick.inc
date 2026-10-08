// D12A first-person quick kick (Modernized eye view only).
//
// The original boot attack 0x800517a4 (Boot selected: Q, Action at idle, or a
// held attack) picks full-body animation 112..115 at random (rand() & 3).
// Several of them spin Duke round or lean him back, and the eye view has no
// lease through them, so the view cut to the third-person camera. Their hit is
// the dispatcher case 0x80049044, run on every player update of 112..115:
// 0x800a979c(point 7 of the model (joint 16, the right ankle), radius 96,
// type 10, [0x800d21fc] * 20 (* 80 with steroids, player+0x364 bit 1),
// attacker Duke, 0) - a sphere that damages the first object whose box it
// touches, through that object's class handler (hit reaction and push).
//
// In the eye view the kick is instead a short Duke 3D style quick kick:
// - Q starts it with any weapon, standing, moving, crouched or in a jump; the
//   weapon stays drawn. An original request (Boot selected) is turned into it
//   at the lower-body initializer 0x800493a4, before 112..115 is set up, by
//   restarting idle 63 exactly as the original transition does.
// - Duke's own right leg meshes (parts 5..8: hip, knee, ankle, toe; joints
//   14..17 on the first map, 5..8 in the cowboy costume, D12B) are drawn in
//   front of the eye with the current costume's meshes,
//   posed from the straight front kick 115 (chamber and extension, recorded in
//   this build), and drawn on top like the D12 weapon.
// - The same original sphere, with the same radius and damage, is placed along
//   the view (crosshair) direction on every player update while the leg is out
//   (about four updates), as the original does for the foot on every update of
//   112..115; the struck object's own hit reaction pushes it.
// - The original kick sound (0x8006b270 id 0x1000, from the 0x800 event of
//   112..115) plays as the leg snaps out.
// Third person and Vanilla keep the original kick.
//
// D12C impact: a kick that connects plays the game's own wall-bump thud once,
// at the first contact; a kick that touches nothing stays silent.
// - The sound is 0x2007 at Duke with 0x800, as the
//   wall bump (0x800537f4..0x800538bc: bump animation 94/95, pad rumble,
//   0x8006b270(0x2007, Duke+4, 0x800); no vocal in that call; landing uses the
//   same id). Bank 2 entry 7: SPU sample 0x1BAA0 on the first street. It is
//   played one octave up (see impact_pitch).
// - Actor/object contact is the sphere's own hit result (kick_call `struck`).
// - Wall contact: the original sphere ignores level geometry, so a wall is the
//   original segment query 0x8006d980 reporting the world (kind 1) where the
//   boot reaches: the D12A crosshair trace in the eye view, and in third
//   person the foot's path out from Duke's root at the foot's height. Floors
//   and ceilings do not count.
// - Third person (Modernized): the original kick's sphere call (case
//   0x80049044, ra 0x80049098) is made by the host with the same arguments to
//   learn the result, and the original call is given a radius that can touch
//   nothing, so damage is applied once, by the same routine.
// Vanilla keeps the original silence. DNTTK_KICK_IMPACT=off silences it (and
// leaves the original third-person call alone), DNTTK_KICK_IMPACT=<id> plays
// another sound (diagnostics).
static constexpr unsigned leg_joints=4;
// Leg index (0 hip .. 3 toe) of one of Duke's joints, or -1.
static int kick_leg_index(uint32_t desc,unsigned joint) {
    const int part=duke_joint_part(desc,joint);
    return part>=(int)part_right_hip && part<(int)(part_right_hip+leg_joints) ? part-(int)part_right_hip : -1;
}
struct LegKey { double q[4]; double p[3]; };
// Right leg world rotation (quaternion w,x,y,z) and position from the root, in
// Duke's level frame (x right, y down, z ahead), from 115 on the first street.
static constexpr LegKey leg_keys[3][leg_joints]={
    {{{0.9794,0.1609,-0.0216,-0.1201},{31.7,47.0,-12.6}},{{0.9835,-0.1337,0.0146,-0.1213},{79.7,241.0,95.1}},{{0.9999,-0.0008,0.0115,0.0073},{143.6,474.0,1.9}},{{0.9999,-0.0008,0.0115,0.0073},{142.5,505.0,69.1}}}, // rest (115 f0)
    {{{0.6510,0.7383,-0.0916,-0.1505},{41.6,39.0,-10.8}},{{0.9836,-0.0376,0.0586,-0.1663},{42.5,-26.0,208.7}},{{0.9860,0.0866,0.0441,-0.1356},{126.9,214.0,158.9}},{{0.9860,0.0866,0.0441,-0.1356},{137.7,232.0,230.3}}}, // chamber (115 f40)
    {{{0.7043,0.7018,0.0178,-0.1058},{29.2,47.0,-18.4}},{{0.7057,0.7003,0.0182,-0.1058},{63.2,8.0,203.3}},{{0.6243,0.7779,-0.0459,-0.0551},{116.3,31.0,456.5}},{{0.6243,0.7779,-0.0459,-0.0551},{104.2,-41.0,472.9}}}, // extend (115 f49)
};
// Host frames (60 Hz): rest -> chamber, chamber -> extend, hold, extend ->
// chamber, chamber -> rest. The hit window covers the snap and the hold.
static constexpr unsigned kick_chamber=7, kick_extend=11, kick_hold=17, kick_retract=21, kick_frames=27;
static constexpr unsigned kick_hit_first=8, kick_hit_last=20;
static constexpr uint32_t kick_damage_routine=0x800a979c, kick_sound_routine=0x8006b270, kick_sound=0x1000;
static constexpr int kick_radius=96, kick_type=10;
// Crosshair ray for the hit: traced this far; a hit counts when it is within
// the original foot's horizontal reach from the eye (115 extends the ankle
// about 455 ahead of the root) and the sphere is placed this far short of it.
static constexpr double kick_trace_length=1200, kick_horizontal_reach=480, kick_surface_backoff=24;
static constexpr uint32_t impact_sound=0x2007;
// Wall contact: hits lower than this above Duke's root are the floor (y is
// down); a hit above the eye in the eye view is the ceiling.
static constexpr double impact_floor_margin=48;
// The original call's radius while the host makes it: the sphere's box test
// needs a point within (box - radius), so a radius of -2^28 rejects every box
// (and its squared radius wraps to 0, which the distance gate rejects first).
static constexpr uint32_t sphere_reject_radius=0xf0000000u;
// 0x8006b73c is 0x8006b270 with the pitch byte (sound +0x3e, read signed at
// 0x80068740) as its fourth argument: 0x40 is 0x8006b270's own value (the
// table pitch + 4 and a random 0..7 fine spread); any other value is the
// table pitch + value, no spread. Measured for 0x2007: SPU pitch 0x398 + 23
// per step, so 0x40 plays at 0x3F4..0x3F8 and 48 at 0x7E8, one octave up
// (user choice 2026-10-08: the original thud is below 150 Hz and inaudible
// on most speakers). DNTTK_KICK_IMPACT_PITCH=-64..63 overrides it.
static constexpr uint32_t impact_sound_routine=0x8006b73c;
static constexpr int impact_octave_up=48;
static constexpr uint32_t impact_stop_routine=0x80068900;
static uint64_t impact_replaced;
static int impact_pitch() {
    static const int value=[]{
        const char* text=std::getenv("DNTTK_KICK_IMPACT_PITCH");char* end=nullptr;
        const long v=text?std::strtol(text,&end,0):impact_octave_up;
        return text && end && !*end && v>=-64 && v<=63 ? (int)v : impact_octave_up;
    }();
    return value;
}
// Volume multiplier for the impact's own sound (user: twice, then three
// times, 2026-10-08); DNTTK_KICK_IMPACT_GAIN=1..4.
static constexpr uint32_t impact_default_gain=3;
static uint32_t impact_volume;
static uint32_t impact_gain() {
    static const uint32_t value=[]{
        const char* text=std::getenv("DNTTK_KICK_IMPACT_GAIN");char* end=nullptr;
        const long v=text?std::strtol(text,&end,10):impact_default_gain;
        return text && end && !*end && v>=1 && v<=4 ? (uint32_t)v : impact_default_gain;
    }();
    return value;
}
static bool kick_impacted, orig_impacted;
static unsigned orig_kick_anim;
static uint64_t impacts, impact_objects, impact_walls, impact_orig_calls, impact_failed;
static int impact_id() {
    static const int value=[]{
        const char* text=std::getenv("DNTTK_KICK_IMPACT");
        if(!text) return (int)impact_sound;
        if(!std::strcmp(text,"off")) return -1;
        char* end=nullptr;const long v=std::strtol(text,&end,0);
        return end && !*end && v>=0 && v<0x8000 ? (int)v : (int)impact_sound;
    }();
    return value;
}
static bool kick_active, kick_sounded, kick_queued;
static uint64_t kick_start;
static uint32_t kick_point, leg_matrices;
static uint64_t kicks, kick_converts, kick_hit_calls, kick_sounds, kick_leg_draws, kick_cancels, kick_surface_points, kick_fire_starts, kick_thigh_draws, kick_queues, kick_repeats, kick_suppressed;
// Display: the leg's root-relative pose plus an offset from the eye (view
// units: x right, y down, z ahead), turned 12 degrees in about the hip, so the
// leg rises from the lower left and the boot lands just under the crosshair,
// clear of a drawn weapon (tuned 2026-09-30 on the first street).
// Developer tuning only (not saved): DNTTK_FP_KICK=x,y,z offset,
// DNTTK_FP_KICK_TILT=pitch,yaw,roll (degrees, as the weapon tilt),
// DNTTK_FP_KICK_REACH=100..600.
static const double* kick_anchor() {
    static double value[3]={-170,80,160};
    static const bool parsed=[]{
        const char* text=std::getenv("DNTTK_FP_KICK");double v[3];
        if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
            for(int i=0;i<3;++i) value[i]=std::clamp(v[i],-1000.0,1000.0);
        return true;
    }();
    (void)parsed;return value;
}
static const double* kick_tilt() {
    static double value[3]={0,12,0};
    static const bool parsed=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_TILT");double v[3];
        if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
            for(int i=0;i<3;++i) value[i]=std::clamp(v[i],-180.0,180.0);
        return true;
    }();
    (void)parsed;return value;
}
static double kick_reach() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_REACH");char* end=nullptr;
        const double v=text?std::strtod(text,&end):0;
        return text && end && !*end && v>=100 && v<=600 ? v : 340.0;
    }();
    return value;
}
static bool kick_eye_ready() {return fp_blend>=0.5 && orbit_valid && fp_supported && identity();}
static void kick_cancel() {if(kick_active) ++kick_cancels;kick_active=false;kick_queued=false;}
// Q in the eye view, a held attack with the Boot, or a converted original
// request. One kick at a time; a Q during a kick queues the next one, so
// rapid presses chain back to back as a held attack with the Boot does.
static bool kick_request(bool queue) {
    if(!kick_eye_ready()) return false;
    if(kick_active) {if(queue && !kick_queued) {kick_queued=true;++kick_queues;} return true;}
    kick_active=true;kick_sounded=false;kick_queued=false;kick_impacted=false;kick_start=input_host_frame();++kicks;
    return true;
}
// Developer tuning only: DNTTK_FP_KICK_FREEZE=0..26 holds every kick at that
// frame (the leg stays up until the view leaves first person).
static int kick_freeze() {
    static const int value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_FREEZE");char* end=nullptr;
        const long v=text?std::strtol(text,&end,10):-1;
        return text && end && !*end && v>=0 && v<(long)kick_frames ? (int)v : -1;
    }();
    return value;
}
static unsigned kick_frame() {
    if(kick_freeze()>=0) return (unsigned)kick_freeze();
    if(render_override) return override_kick_frame;   // D17 worker: the live frame's value
    const uint64_t now=input_host_frame();
    return now>=kick_start ? (unsigned)std::min<uint64_t>(now-kick_start,kick_frames) : 0;
}
// A guest call with up to four register arguments and two stack arguments, on
// an isolated stack as original_call does.
// D12C: with `struck`, the call is the damage sphere 0x800a979c and reports
// whether it struck something. The sphere returns no result, but only on a
// hit does it store its fifth argument (the attacker, never 0) at sp+0x10 of
// its own 0x80-byte frame (0x800a9b8c), just before calling the struck
// object's class handler (0x800a9bac); nothing else writes that slot (callees
// keep to their own frames). The routine is SHA-guarded.
static constexpr uint32_t sphere_frame=0x80, sphere_attacker_slot=0x10;
static bool kick_call(CPUState* source,uint32_t address,const uint32_t (&args)[6],bool* struck=nullptr,uint32_t* result=nullptr) {
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    uint8_t saved[4096],scratch[1024];
    for(unsigned i=0;i<sizeof saved;++i) saved[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i) scratch[i]=psx_mod_read_byte(0x1f800000+i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
    for(int i=0;i<4;++i) cpu.gpr[4+i]=args[i];
    psx_mod_write_word(cpu.gpr[29]+0x10,args[4]);psx_mod_write_word(cpu.gpr[29]+0x14,args[5]);
    const uint32_t slot=cpu.gpr[29]-sphere_frame+sphere_attacker_slot;
    if(struck) psx_mod_write_word(slot,0);
    guest_busy=true;psx_dispatch_call(&cpu,address,0x800000fc);guest_busy=false;
    if(struck) *struck=cpu.pc==0 && !g_psx_call_bail && args[4] && psx_mod_read_word(slot)==args[4];
    if(result) *result=cpu.pc==0 && !g_psx_call_bail ? cpu.gpr[2] : 0;
    for(unsigned i=0;i<sizeof saved;++i) psx_mod_write_byte(top-4096+i,saved[i]);
    for(unsigned i=0;i<sizeof scratch;++i) psx_mod_write_byte(0x1f800000+i,scratch[i]);
    return cpu.pc==0 && !g_psx_call_bail;
}
// D12C: the wall-bump thud at Duke, once per connecting kick.
static void kick_impact(CPUState* cpu,bool object) {
    ++(object?impact_objects:impact_walls);
    if(impact_id()<0) return;
    // The routine refuses a sound whose id is already playing (0x8006b7b0,
    // table flag 4), and Duke walking into a wall plays the same 0x2007
    // quietly (the bump; landings too), so the first kick at a wall was often
    // silent (user report 2026-10-08). A 0x2007 still playing is stopped first
    // with the game's stop (0x80068900, what 0x8006af44 does for a handle,
    // under its checks: sound system state 7, sound active).
    if(psx_mod_read_word(0x800c39b4)==7)
        for(uint32_t node=psx_mod_read_word(0x800da5b8),n=0;node && n<64;++n) {
            if(node<0x80010000u || node>0x801fff80u) break;
            const uint32_t next=psx_mod_read_word(node);
            if(psx_mod_read_word(node+0x5c)==(uint32_t)impact_id() && psx_mod_read_half(node+8)) {
                if(kick_call(cpu,impact_stop_routine,{node,0,0,0,0,0})) ++impact_replaced;
                break;
            }
            if(next==node) break;
            node=next;
        }
    uint32_t handle=0;
    if(!kick_call(cpu,impact_sound_routine,{(uint32_t)impact_id(),player+4,0x800,(uint32_t)impact_pitch(),0,0},nullptr,&handle)) {++impact_failed;return;}
    ++impacts;
    // Louder (user request 2026-10-08: double its volume). The call returns a
    // handle, the sound's address in its low 24 bits (0x80067e74); +0x5c is
    // its id and +0x6a its volume from the sound table (copied at
    // 0x8006b850), which the update ramps the voice to (0x80068ddc) or sets
    // outright (0x80069004) before the positional mix. Only this sound's
    // copy is scaled, capped at the SPU maximum 0x3fff.
    const uint32_t sound=0x80000000u|(handle&0xffffffu);
    if(!handle || sound<0x80010000u || sound>0x801fff80u || psx_mod_read_word(sound+0x5c)!=(uint32_t)impact_id()) return;
    const uint32_t volume=psx_mod_read_half(sound+0x6a);
    impact_volume=volume;
    psx_mod_write_half(sound+0x6a,(uint16_t)std::min<uint32_t>(0x3fff,volume*impact_gain()));
}
// Wall contact in the eye view: the original segment query from Duke's body
// at boot height out along the view's heading to the boot's reach.
static constexpr double impact_boot_height=150, impact_reach=kick_horizontal_reach;
static bool kick_wall_reached(CPUState* cpu,const double* forward) {
    const double n=std::hypot(forward[0],forward[2]);
    if(n<0.2) return false;
    double from[3],to[3],hit[3];int kind=0;
    for(int i=0;i<3;++i) from[i]=(int32_t)psx_mod_read_word(player+4+4*i);
    from[1]-=impact_boot_height;
    to[0]=from[0]+forward[0]/n*impact_reach;to[1]=from[1];to[2]=from[2]+forward[2]/n*impact_reach;
    return view_segment_query(cpu,from,to,hit,kind) && kind==1;
}
// Once per player update (0x80058120, ra 0x80041c44): sound and hits.
static bool boot_selected() {return !(psx_mod_read_word(player+0x3b8)&0x00ff00ffu);}
static void kick_update(CPUState* cpu) {
    // Allocate the kick's guest scratch up front: an allocation changes the
    // mod-memory watermark, which reforks the D17 redraw workers, and doing
    // that at the first kick dropped the in-between frames during it.
    if(!kick_point) kick_point=psx_mod_alloc_guest_memory(16,16);
    if(!leg_matrices) leg_matrices=psx_mod_alloc_guest_memory(0x20*leg_joints,16);
    // Original kick (third person): a new impact for each kick animation.
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(animation<112 || animation>115) {orig_impacted=false;orig_kick_anim=0;}
    const auto& f=input_snapshot(Context::Gameplay);
    if(kick_active && !kick_eye_ready()) {kick_cancel();return;}
    if(kick_active && kick_frame()>=kick_frames) {
        // Chain: a queued Q, or Q held (repeats like a held attack).
        const bool again=kick_queued || f.held[quick_kick];
        kick_active=false;kick_queued=false;
        if(again && kick_request()) ++kick_repeats;
    }
    // Boot selected (equipment and selection 0, the mask 0x800517a4 checks):
    // a held attack kicks while moving too. The original only kicks from a
    // standing idle; that request is still converted in kick_convert.
    if(!kick_active && f.held[fire] && boot_selected() && kick_request()) ++kick_fire_starts;
    if(!kick_active) return;
    const unsigned frame=kick_frame();
    if(!kick_sounded && frame>=kick_chamber) {
        kick_sounded=true;
        if(kick_call(cpu,kick_sound_routine,{kick_sound,player+4,0x800,0,0,0})) ++kick_sounds;
    }
    if(frame<kick_hit_first || frame>kick_hit_last) return;
    if(!kick_point) kick_point=psx_mod_alloc_guest_memory(16,16);
    if(!kick_point) return;
    // Along the view (camera row 2, the pitched forward axis). Where the
    // crosshair ray meets a surface or an actor within the foot's reach, the
    // sphere goes there (just short of it), so a low object that fills the
    // crosshair is kicked where it is, not above it.
    double eye[3],forward[3],point[3],end[3],hit[3];
    for(int i=0;i<3;++i) {
        forward[i]=(int16_t)psx_mod_read_half(camera+2*(6+i))/4096.0;
        eye[i]=(int32_t)psx_mod_read_word(camera+0x14+4*i);
        point[i]=eye[i]+forward[i]*kick_reach();
        end[i]=eye[i]+forward[i]*kick_trace_length;
    }
    int kind=0;bool wall=false;
    if(view_segment_query(cpu,eye,end,hit,kind) && kind &&
       std::hypot(hit[0]-eye[0],hit[2]-eye[2])<=kick_horizontal_reach) {
        for(int i=0;i<3;++i) point[i]=hit[i]-forward[i]*kick_surface_backoff;
        ++kick_surface_points;
    }
    if(!kick_impacted) wall=kick_wall_reached(cpu,forward);
    for(int i=0;i<3;++i) psx_mod_write_word(kick_point+4*i,(uint32_t)(int32_t)std::lround(point[i]));
    psx_mod_write_word(kick_point+12,0);
    const uint32_t delta=psx_mod_read_word(0x800d21fc);
    const uint32_t damage=delta*5*((psx_mod_read_half(player+0x364)&2)?16:4);
    bool struck=false;
    if(kick_call(cpu,kick_damage_routine,{kick_point,(uint32_t)kick_radius,(uint32_t)kick_type,damage,player,0},&struck)) ++kick_hit_calls;
    if(!kick_impacted && (struck || wall)) {kick_impacted=true;kick_impact(cpu,struck);}
}
// D12C: entry of the damage sphere 0x800a979c from the original kick's case
// (ra 0x80049098) for Duke, Modernized. The host makes the same call to learn
// whether the foot struck something, then the original call touches nothing.
static void kick_sphere_entry(CPUState* cpu) {
    if(cpu->gpr[31]!=0x80049098u || impact_id()<0 || guest_busy || render_override || cpu->gpr[5]!=(uint32_t)kick_radius ||
       cpu->gpr[6]!=(uint32_t)kick_type || !player_identity_ready()) return;
    const uint32_t sp=cpu->gpr[29];
    if(sp<0x801f4000 || sp>0x801ffff0 || psx_mod_read_word(sp+0x10)!=player) return;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(animation<112 || animation>115) return;
    if(animation!=orig_kick_anim) {orig_kick_anim=animation;orig_impacted=false;}
    const uint32_t foot=cpu->gpr[4];
    if(foot<0x80010000 || foot>0x801ffff0) return;
    bool struck=false;
    if(!kick_call(cpu,kick_damage_routine,{foot,cpu->gpr[5],cpu->gpr[6],cpu->gpr[7],player,psx_mod_read_word(sp+0x14)},&struck)) return;
    cpu->gpr[5]=sphere_reject_radius;++impact_orig_calls;
    if(orig_impacted) return;
    bool wall=false;
    if(!struck) {
        // The foot's path out from Duke's root, at the foot's height.
        double from[3],to[3],hit[3];int kind=0;
        for(int i=0;i<3;++i) to[i]=(int32_t)psx_mod_read_word(foot+4*i);
        from[0]=(int32_t)psx_mod_read_word(player+4);from[1]=to[1];from[2]=(int32_t)psx_mod_read_word(player+12);
        const double root=(int32_t)psx_mod_read_word(player+8);
        wall=to[1]<root-impact_floor_margin && std::hypot(to[0]-from[0],to[2]-from[2])>16 &&
             view_segment_query(cpu,from,to,hit,kind) && kind==1;
    }
    if(struck || wall) {orig_impacted=true;kick_impact(cpu,struck);}
}
// 0x800493a4 entry from the lower-body update (ra 0x8005a3a0/0x8005a490/0x8005a4d0):
// an original boot request (112..115, not yet initialized) in the eye view.
// Cross is both the attack and Action: the original also kicks from E (Action)
// at idle with the Boot selected. In the eye view only Q and the attack kick
// (user playtest 2026-09-30), so a request made without the attack held
// restarts idle with no kick.
static void kick_convert(CPUState* cpu,uint32_t ra) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(cpu->gpr[4]!=player || (ra!=0x8005a3a0 && ra!=0x8005a490 && ra!=0x8005a4d0) ||
       animation<112 || animation>115 || psx_mod_read_half(player+0x68)!=0 || !kick_eye_ready()) return;
    if(input_snapshot(Context::Gameplay).held[fire]) {kick_request();++kick_converts;}
    else ++kick_suppressed;
    psx_mod_write_half(player+0x60,63);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
}
static void slerp(const double* a,const double* b,double t,double* out) {
    double d=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3],s=1;
    if(d<0) {d=-d;s=-1;}
    double wa=1-t,wb=t;
    if(d<0.9995) {const double th=std::acos(d),sn=std::sin(th);wa=std::sin((1-t)*th)/sn;wb=std::sin(t*th)/sn;}
    double n=0;
    for(int i=0;i<4;++i) {out[i]=wa*a[i]+wb*s*b[i];n+=out[i]*out[i];}
    n=std::sqrt(n);for(int i=0;i<4;++i) out[i]/=n;
}
// Pose at the current kick frame: two keys and the eased blend between them.
static void kick_pose(unsigned frame,unsigned& from,unsigned& to,double& t) {
    auto ease=[](double x){x=std::clamp(x,0.0,1.0);return x*x*(3-2*x);};
    if(frame<kick_chamber) {from=0;to=1;t=ease(frame/(double)kick_chamber);}
    else if(frame<kick_extend) {from=1;to=2;t=ease((frame-kick_chamber)/(double)(kick_extend-kick_chamber));}
    else if(frame<kick_hold) {from=to=2;t=0;}
    else if(frame<kick_retract) {from=2;to=1;t=ease((frame-kick_hold)/(double)(kick_retract-kick_hold));}
    else {from=1;to=0;t=ease((frame-kick_retract)/(double)(kick_frames-kick_retract));}
}
// 0x800292a0 from the joint loop (ra 0x80034c6c) for a right-leg joint while
// the kick plays: a private matrix placing that joint in front of the eye.
// Developer switch: DNTTK_FP_KICK_THIGH=0 leaves the thigh to the loop's near cull.
static bool kick_thigh_enabled() {
    static const bool value=[]{const char* t=std::getenv("DNTTK_FP_KICK_THIGH");return !(t && t[0]=='0' && !t[1]);}();
    return value;
}
// The thigh alone sits lower on screen by this fraction of the screen height
// (240 lines), measured at its knee-end depth (user playtests 2026-09-30:
// 10% first, 13% chosen).
// Developer override: DNTTK_FP_KICK_THIGH_DROP=0..0.5.
static double kick_thigh_drop() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_THIGH_DROP");char* end=nullptr;
        const double v=text?std::strtod(text,&end):-1;
        return text && end && !*end && v>=0 && v<=0.5 ? v : 0.13;
    }();
    return value;
}
// A leg joint's level-frame rotation and root-relative position at a kick frame.
static void leg_pose(unsigned leg,unsigned frame,double (&lh)[3][3],double* p) {
    unsigned from,to;double t;
    kick_pose(frame,from,to,t);
    const LegKey& a=leg_keys[from][leg];const LegKey& b=leg_keys[to][leg];
    double q[4];
    slerp(a.q,b.q,t,q);quat_mat(q,lh);
    for(int i=0;i<3;++i) p[i]=a.p[i]+(b.p[i]-a.p[i])*t;
    // Tilt about the hip in the level frame, as the weapon tilt.
    const double* tilt=kick_tilt();
    if(tilt[0] || tilt[1] || tilt[2]) {
        const double d=3.14159265358979/180,pt=-tilt[0]*d,y=tilt[1]*d,r=tilt[2]*d;
        const double rx[3][3]={{1,0,0},{0,std::cos(pt),-std::sin(pt)},{0,std::sin(pt),std::cos(pt)}};
        const double ry[3][3]={{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}};
        const double rz[3][3]={{std::cos(r),-std::sin(r),0},{std::sin(r),std::cos(r),0},{0,0,1}};
        double m[3][3];mat_mul(ry,rx,m);mat_mul(m,rz,m);mat_mul(m,lh,lh);
        double hip[3],rel[3];
        for(int i=0;i<3;++i) {hip[i]=leg_keys[from][0].p[i]+(leg_keys[to][0].p[i]-leg_keys[from][0].p[i])*t;rel[i]=p[i]-hip[i];}
        for(int i=0;i<3;++i) p[i]=hip[i]+m[i][0]*rel[0]+m[i][1]*rel[1]+m[i][2]*rel[2];
    }
}
static bool kick_leg_joint(CPUState* cpu) {
    if(!kick_active || cpu->gpr[4]!=camera || cpu->gpr[20]!=player) return false;
    const unsigned joint=cpu->gpr[23];
    const uint32_t mats=psx_mod_read_word(player+0x3c),desc=psx_mod_read_word(player+0x40);
    if(mats<0x80010000 || mats>0x801fff00 || desc!=duke_model() || !desc ||
       cpu->gpr[5]!=mats+0x20*joint) return false;
    const int leg=kick_leg_index(desc,joint);
    if(leg<0) return false;
    if(!leg_matrices) leg_matrices=psx_mod_alloc_guest_memory(0x20*leg_joints,16);
    if(!leg_matrices) return false;
    const uint32_t out_m=leg_matrices+0x20*leg;
    const unsigned frame=kick_frame();
    double p[3],lh[3][3];
    leg_pose(leg,frame,lh,p);
    if(leg==0 && kick_thigh_enabled()) {
        // The thigh's origin (the hip) is nearer than the loop's near cull
        // (GTE H) from the eye, so the loop would skip the whole thigh. Draw
        // it end for end instead: origin at the knee (which passes the cull),
        // turned 180 degrees about an axis across the thigh, so the same mesh
        // runs from the knee back toward the hip and off the bottom edge.
        double knee[3],kl[3][3];
        leg_pose(1,frame,kl,knee);
        double d[3],u[3],n=0;
        for(int i=0;i<3;++i) d[i]=knee[i]-p[i];
        for(int i=0;i<3;++i) {u[i]=lh[0][i]*d[0]+lh[1][i]*d[1]+lh[2][i]*d[2];n+=u[i]*u[i];} // thigh axis, local
        n=std::sqrt(n);
        if(n>1) {
            for(double& v:u) v/=n;
            // Across the thigh: local x unless the thigh runs along it.
            double a[3]={1,0,0};
            if(std::abs(u[0])>0.9) {a[0]=0;a[1]=1;}
            double w[3]={u[1]*a[2]-u[2]*a[1],u[2]*a[0]-u[0]*a[2],u[0]*a[1]-u[1]*a[0]},wn=0;
            for(double v:w) wn+=v*v;
            wn=std::sqrt(wn);for(double& v:w) v/=wn;
            double flip[3][3];
            for(int r=0;r<3;++r) for(int k=0;k<3;++k) flip[r][k]=2*w[r]*w[k]-(r==k);
            mat_mul(lh,flip,lh);
            for(int i=0;i<3;++i) p[i]=knee[i];
            // Screen lines to view units at this depth: y = lines * z / H (H 256).
            const double depth=p[2]+kick_anchor()[2];
            if(depth>0) p[1]+=kick_thigh_drop()*240*depth/256;
            ++kick_thigh_draws;
        }
    }
    double c[3][3];
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) c[r][k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k))/4096.0;
    // Shown pitched with the view, like the weapon: stored lh^T C.
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) {
        const double v=lh[0][r]*c[0][k]+lh[1][r]*c[1][k]+lh[2][r]*c[2][k];
        psx_mod_write_half(out_m+2*(3*r+k),(uint16_t)(int16_t)std::clamp(std::lround(v*4096),-32768L,32767L));
    }
    psx_mod_write_half(out_m+0x12,0);
    const double* anchor=kick_anchor();
    for(int i=0;i<3;++i) {
        double w=(int32_t)psx_mod_read_word(camera+0x14+4*i);
        for(int k=0;k<3;++k) w+=c[k][i]*(p[k]+anchor[k]);
        psx_mod_write_word(out_m+0x14+4*i,(uint32_t)(int32_t)std::lround(w));
    }
    fade_clear(cpu);
    cpu->gpr[5]=out_m;weapon_viewmodel=true;++kick_leg_draws;
    return true;
}
static const char* kick_debug_json() {
    static char buffer[760];
    std::snprintf(buffer,sizeof buffer,"{\"active\":%s,\"frame\":%u,\"kicks\":%llu,\"converts\":%llu,\"fire_starts\":%llu,\"queues\":%llu,\"repeats\":%llu,\"suppressed\":%llu,\"hits\":%llu,\"surface_points\":%llu,\"sounds\":%llu,\"leg_draws\":%llu,\"thigh_draws\":%llu,\"cancels\":%llu,\"reach\":%.0f,\"impact\":{\"sound\":%d,\"played\":%llu,\"objects\":%llu,\"walls\":%llu,\"original_calls\":%llu,\"failed\":%llu,\"gain\":%u,\"table_volume\":%u,\"replaced\":%llu}}",
        kick_active?"true":"false",kick_active?kick_frame():0u,(unsigned long long)kicks,(unsigned long long)kick_converts,(unsigned long long)kick_fire_starts,
        (unsigned long long)kick_queues,(unsigned long long)kick_repeats,(unsigned long long)kick_suppressed,(unsigned long long)kick_hit_calls,(unsigned long long)kick_surface_points,(unsigned long long)kick_sounds,(unsigned long long)kick_leg_draws,(unsigned long long)kick_thigh_draws,(unsigned long long)kick_cancels,kick_reach(),
        impact_id(),(unsigned long long)impacts,(unsigned long long)impact_objects,(unsigned long long)impact_walls,(unsigned long long)impact_orig_calls,(unsigned long long)impact_failed,impact_gain(),impact_volume,(unsigned long long)impact_replaced);
    return buffer;
}
