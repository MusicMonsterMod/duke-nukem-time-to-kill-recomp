// D08J7C: the one-handed armed pull-up. While armed_pullup_active() the
// original plays the two-handed pull-up 140 on every channel but the gun arm
// (which keeps the sidearm block), so the off hand and chest pulled up as if
// both hands held the lip. Host clip onehand_clip (ttk_onehand_pullup.py, the
// same 10 frames as 140) holds a one-handed upper body: the chest leaned onto
// the left arm, the left hand on the lip, the head on the ledge and the gun arm
// in a high ready that comes down into the sidearm pose by armed_pullup_late.
// Revision 3: shots are allowed the whole way up; while the original aim owns
// the chest (+0x224 bit 0x100, firing) the gun arm leaves the high ready for
// its channel's aimed sidearm pose over onehand_aim_ms and comes back after.
// Written here, at 0x800411b8 after every channel blend (as crouch_walk_pose),
// sampled at channel 0's own (previous, current, t) so it keeps 140's timing
// (the D08J7B 1.6x included). Legs and root stay on 140: Duke ends where he
// does today. The gun arm hands back to its channel between frames
// armed_pullup_late-1 and armed_pullup_late (the shots that open there aim
// from the game's own pose); while the original aim build owns the chest
// (+0x224 bit 0x100) the chest and head are left to it. Ramps in and out over
// onehand_ramp_ms of real time (the same at 60/120/180).
// DNTTK_ONEHAND_PULLUP=0 keeps the D08J7C prototype pose (developer A/B).
static constexpr unsigned onehand_clip=41;
static constexpr double onehand_ramp_ms=140,onehand_aim_ms=70;
static double onehand_gun_weight=1;
static constexpr unsigned onehand_ids[8]={9,16,10,11,12,13,14,15};
static CrouchQuat onehand_last[8];
static double onehand_last_ms=-1;
// Channel 0's position on 140 (frames from the clip's first, t), false off it.
static bool onehand_position(uint32_t& kp,uint32_t& kc,double& t) {
    const int clip=int16_t(psx_mod_read_half(0x800c2d64+8*140));
    if(clip<0 || clip>=2048)return false;
    const uint32_t first=psx_mod_read_half(0x800d8328+4*clip),count=psx_mod_read_half(0x800d8328+4*clip+2);
    const uint32_t pf=psx_mod_read_half(player+0x64),cf=psx_mod_read_half(player+0x66);
    if(pf<first || cf<first || pf>=first+count || cf>=first+count)return false;
    uint32_t hf,hn;
    if(!ttk::anim_extra_clip(onehand_clip,hf,hn) || hn!=count+1)return false;
    const uint32_t ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*cf)>>16,left=psx_mod_read_half(player+0x62);
    t=ticks ? std::clamp(double(int32_t(ticks)-int32_t(left))/double(ticks),0.0,1.0) : 1.0;
    // The host clip's frames follow a leading copy (ttk_crouch_walk.host_frames).
    kp=hf+1+(pf-first);kc=hf+1+(cf-first);
    onehand_phase=double(cf-first)-1.0+t;
    if(pf==cf)onehand_phase=double(cf-first);
    return true;
}
static void onehand_pullup_pose(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634)return;
    static const int enabled=ttk_env_switch("DNTTK_ONEHAND_PULLUP");
    const double now=input_clock_ms();
    const double dt=onehand_last_ms<0 ? 0 : std::clamp(now-onehand_last_ms,0.0,100.0);
    onehand_last_ms=now;
    const uint32_t desc=duke_model();
    uint32_t kp=0,kc=0;double t=0;
    const bool on=enabled!=0 && desc && armed_pullup_active() && onehand_position(kp,kc,t);
    onehand_weight=std::clamp(onehand_weight+(on?1:-1)*dt/onehand_ramp_ms,0.0,1.0);
    if(on && dt==0)onehand_weight=std::max(onehand_weight,1e-3);
    if(!desc || onehand_weight<=0) {onehand_weight=0;onehand_owned=false;return;}
    std::array<std::array<int32_t,4>,32> a{},b{};uint32_t ma=0,mb=0;
    if(on && (!crouch_frame_quats(kp,a,ma) || !crouch_frame_quats(kc,b,mb)))return;
    if(!on && !onehand_owned)return;
    // The gun arm hands back to its channel over the frame before the late phase.
    const bool aimed=psx_mod_read_word(player+0x224)&0x100;
    if(!onehand_owned)onehand_gun_weight=aimed?0:1;
    onehand_gun_weight=std::clamp(onehand_gun_weight+(aimed?-1:1)*dt/onehand_aim_ms,0.0,1.0);
    const double gun=std::clamp(double(armed_pullup_late)-onehand_phase,0.0,1.0)*onehand_gun_weight;
    for(unsigned i=0;i<8;++i) {
        const unsigned j=onehand_ids[i];
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r)continue;
        if(on) {
            auto q=[&](const std::array<std::array<int32_t,4>,32>& v,uint32_t m,const std::array<std::array<int32_t,4>,32>& o,uint32_t om) {
                const auto& s=(m>>j&1) ? v[j] : o[j];
                return (m|om)>>j&1 ? crouch_qnorm({double(s[0]),double(s[1]),double(s[2]),double(s[3])}) : crouch_record_q(r);
            };
            onehand_last[i]=crouch_slerp(q(a,ma,b,mb),q(b,mb,a,ma),t);
        }
        double w=onehand_weight;
        if(j>=13)w*=on ? gun : 0;
        if(aimed && (j==9 || j==16))w=0;
        if(w<=0)continue;
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),onehand_last[i],w));
    }
    onehand_owned=true;
    ++onehand_writes;
}
