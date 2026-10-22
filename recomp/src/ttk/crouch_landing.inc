// D08K9: consume held crouch in the same update as an ordinary dry landing.
// The original selector still owns collision, sound, damage and the landing
// choice. Intercept only its soft-landing animation at the existing lower
// channel initializer, before that animation can be initialized or displayed.
static bool crouch_landing_armed,crouch_landing_started;
static uint64_t crouch_landing_epoch;
static uint32_t crouch_landing_sp;
static unsigned crouch_landings;
static bool crouch_landing_input() {
    const auto& f=input_snapshot(Context::Gameplay);
    uint32_t first,count;
    return f.active && f.held[crouch] && !f.held[original_aim] && !f.held[interact] &&
        independent_camera() && crouch_walk_modern() &&
        player_identity_ready() && !(psx_mod_read_word(player)&0x20002) &&
        !(psx_mod_read_word(player+0x224)&0x202002c1) && !in_water_early() &&
        crouch_walk_frames(1,first,count) && crouch_walk_frames(crouch_idle_clip,first,count);
}
static void crouch_landing_track(CPUState* cpu) {
    const unsigned a=psx_mod_read_half(player+0x60);
    crouch_landing_started=false;
    crouch_landing_armed=psx_mod_read_byte(player+0x22c)==9 &&
        (a==97 || a==98 || a==103 || a==104 || a==107 || a==108) && crouch_landing_input();
    crouch_landing_epoch=input_snapshot(Context::Gameplay).epoch;
    crouch_landing_sp=cpu->gpr[29];
}
static void crouch_landing_start(CPUState* cpu,uint32_t ra) {
    if(!crouch_landing_armed || cpu->gpr[4]!=player || ra!=0x8005a4d0 ||
       cpu->gpr[29]!=crouch_landing_sp-0x40 ||
       input_snapshot(Context::Gameplay).epoch!=crouch_landing_epoch)return;
    // Consume the request once, including hard landings, water and interrupts.
    crouch_landing_armed=false;
    const unsigned a=psx_mod_read_half(player+0x60);
    if((a!=105 && a!=78 && a!=84 && a!=85 && a!=96) ||
       psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=9 ||
       !crouch_landing_input())return;
    // Capture the last displayed flight pose for the short landing blend.
    const uint32_t desc=duke_model();
    crouch_display_mask=0;
    if(desc)for(unsigned j=0;j<32;++j) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(r) {crouch_display_pose[j]=crouch_record_q(r);crouch_display_mask|=1u<<j;}
    }
    crouch_walk_height=std::clamp(double(int32_t(psx_mod_read_word(player+0x1c8)))-
        double(int32_t(psx_mod_read_word(player+8))),crouch_kneel_pelvis,600.0);
    crouch_owned=true;crouch_stand_pending=false;crouch_boost=false;
    psx_mod_write_half(player+0x60,178);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    // This bit requests the selector's running continuation; crouch owns it now.
    psx_mod_write_word(player+0x228,psx_mod_read_word(player+0x228)&~8u);
    crouch_landing_started=true;++crouch_entries;++crouch_landings;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))std::fprintf(stderr,"ttk-crouch landing %u -> 178 height=%.0f\n",a,crouch_walk_height);
}
static void crouch_landing_finish(CPUState* cpu,uint32_t ra) {
    if(!crouch_landing_started || cpu->gpr[4]!=player || ra!=0x8005a4f8 ||
       cpu->gpr[29]!=crouch_landing_sp-0x40 ||
       input_snapshot(Context::Gameplay).epoch!=crouch_landing_epoch)return;
    crouch_landing_started=false;
    // The initializer and the original clip setup have now run. Install the
    // host pose and take the first collision-checked crouch step this update.
    crouch_walk_update(cpu,true);
}
