// D53C: no reach into a Duke 3D security TV.
//
// The original use of a security monitor (LEVEL00 type 858, subtypes 39-41,
// D35K) lines Duke up 275 units from the monitor's collision face, the TTK
// panel on the wall (0x80051890 -> 0x8007ec4c with 0x113: a ray to the face,
// then Duke's position is set in one update), stows the weapon and plays the
// class-2 use 277, whose reach puts the hand on that panel; the monitor's
// event 3 (alternate camera, 0x8003b36c) is keyed inside 277. D53's Duke 3D
// TV stands 140 in front of the panel with its glass at 125, so the reach
// went about 130 units through the glass, in both views (user, UI slot 10).
// From the lined-up spot Duke's shoulder is already 46 units from the glass,
// so the touch cannot be kept in front of it, and lining him up further back
// pops him backwards (the line-up is a snap). So, as the user offered, Duke
// makes no reach at a TV: while 277 plays for a monitor drawn as the TV, the
// chest, neck and arms are held in the unarmed idle (animation 63's first
// frame of each upper channel), ramped in and out over real time. Written
// here, at 0x800411b8 after every channel blend (as onehand_pullup_pose), so
// third person, first person (the D36H5 contact rig reads these joints),
// reflections and redraws all show the same pose. Legs, root, the line-up,
// 277's timing, the event, the camera, the weapon stow/return and every
// other use stay the original's. The TTK look (flat panel), Classic and
// Vanilla keep the reach. Developer A/B: DNTTK_MONITOR_USE=native.
static constexpr unsigned monitor_use_anim=277,monitor_idle_anim=63;
static constexpr unsigned monitor_use_ids[8]={9,16,10,11,12,13,14,15};
static constexpr double monitor_use_ramp_in_ms=60,monitor_use_ramp_out_ms=150;
static double monitor_use_weight=0,monitor_use_last_ms=-1;
static CrouchQuat monitor_use_last[8];
static bool monitor_use_owned=false;
static uint64_t monitor_use_writes=0,monitor_use_uses=0;
// The use in progress (stages 1..8, committed) of a monitor drawn as the TV,
// while 277 plays on the legs channel.
static bool monitor_use_active() {
    if(!input_modernized() || !ttk::security_tv_object)return false;
    if(psx_mod_read_half(player+0x60)!=monitor_use_anim)return false;
    const unsigned stage=psx_mod_read_byte(player+0x27d);
    if(stage<1 || stage>8 || !(psx_mod_read_word(player+0x224)&0x40))return false;
    if(psx_mod_read_byte(player+0x22c)!=0)return false;
    return ttk::security_tv_object(psx_mod_read_word(player+0x290));
}
// The unarmed idle's first frame on the upper channel that owns joint j
// (1: right arm 13-15, 2: left arm 10-12, 3: chest and neck).
static bool monitor_idle_quat(unsigned j,CrouchQuat& out) {
    const unsigned ch=j>=13 && j<=15 ? 1 : j>=10 && j<=12 ? 2 : 3;
    const int clip=int16_t(psx_mod_read_half(0x800c2d64+8*monitor_idle_anim+2*ch));
    if(clip<0 || clip>=2048)return false;
    const uint32_t first=psx_mod_read_half(0x800d8328+4*clip),count=psx_mod_read_half(0x800d8328+4*clip+2);
    if(first==0xffff || !count)return false;
    std::array<std::array<int32_t,4>,32> q{};uint32_t mask=0;
    if(!crouch_frame_quats(first,q,mask) || !(mask>>j&1))return false;
    out=crouch_qnorm({double(q[j][0]),double(q[j][1]),double(q[j][2]),double(q[j][3])});
    return true;
}
static void monitor_use_pose(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634)return;
    static const bool native=[]{const char* v=std::getenv("DNTTK_MONITOR_USE");return v && !std::strcmp(v,"native");}();
    const double now=input_clock_ms();
    const double dt=monitor_use_last_ms<0 ? 0 : std::clamp(now-monitor_use_last_ms,0.0,100.0);
    monitor_use_last_ms=now;
    const uint32_t desc=duke_model();
    const bool on=!native && desc && monitor_use_active();
    if(on && !monitor_use_owned)++monitor_use_uses;
    monitor_use_weight=std::clamp(monitor_use_weight+(on ? dt/monitor_use_ramp_in_ms : -dt/monitor_use_ramp_out_ms),0.0,1.0);
    if(on && dt==0)monitor_use_weight=std::max(monitor_use_weight,1e-3);
    if(!desc || monitor_use_weight<=0 || (!on && !monitor_use_owned)) {monitor_use_weight=0;monitor_use_owned=false;return;}
    const double w=monitor_use_weight*monitor_use_weight*(3-2*monitor_use_weight);
    for(unsigned i=0;i<8;++i) {
        const unsigned j=monitor_use_ids[i];
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r)continue;
        if(on && !monitor_idle_quat(j,monitor_use_last[i]))monitor_use_last[i]=crouch_record_q(r);
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),monitor_use_last[i],w));
    }
    monitor_use_owned=on || monitor_use_weight>0;
    ++monitor_use_writes;
}
static const char* monitor_use_debug_json() {
    static char buffer[160];
    std::snprintf(buffer,sizeof buffer,"{\"active\":%s,\"weight\":%.3f,\"uses\":%llu,\"writes\":%llu}",
        monitor_use_owned?"true":"false",monitor_use_weight,(unsigned long long)monitor_use_uses,(unsigned long long)monitor_use_writes);
    return buffer;
}
