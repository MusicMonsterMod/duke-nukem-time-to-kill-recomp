// D08AC: use the existing land gaits and takeoffs in grounded water.
// No water-mask, surface, volume, position or velocity writes. The original
// water transition/effect routines still run, including deep-water handoff.
static void grounded_water_gait() {
    const auto& f=input_snapshot(Context::Gameplay);
    if(!grounded_water_modern() || !f.active || f.held[original_aim] || f.held[crouch] ||
       !independent_camera() || !player_identity_ready() ||
       (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x202002c1))return;
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    const unsigned previous=psx_mod_read_byte(player+0x22d);
    const unsigned a=psx_mod_read_half(player+0x60);
    if(mode>1 || (previous>1 && previous!=9) ||
       !((a==63 || (a>=72 && a<=79)) || a==70 || a==71 || a==80 || a==81 || a==96 || a==105))return;
    // Mode 1 only selects the mid-wade gait. Native depth/head checks still
    // choose 4/5 when actually swimming; do not convert those states.
    if(mode==1)psx_mod_write_byte(player+0x22c,0);
    if(previous==1)psx_mod_write_byte(player+0x22d,0);
    if(a==80 || a==81) {
        const unsigned next=!(f.move_x || f.move_y)?63:input_running()?76:72;
        psx_mod_write_half(player+0x60,next);
        psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    }
}
// Native dispatcher post-processing replaces even a just-selected run jump
// with wade 80/81 at depth 512..640. Bypass only those four rewrites, plus
// preparation's depth-only directional veto. Collision and effects still run.
// All words are inside existing resident guards. Restore before controls hooks'
// identity checks and reapply only while this authenticated dispatcher frame
// is on the stack. Never hold a patch across a player update or load.
struct GroundWaterPatch { uint32_t address,original,replacement; };
static constexpr GroundWaterPatch ground_water_patches[]={
    {0x800485e4,0x10400354,0x10000354},
    {0x80048628,0x10400343,0x10000343},
    {0x8004866c,0x10400332,0x10000332},
    {0x800486b0,0x10400321,0x10000321},
    {0x80054964,0x10400060,0x00000000},
    {0x80056240,0x1040005c,0x1000005c},
    {0x80054cbc,0x1440000a,0x1000000a},
};
static uint32_t ground_water_dispatch_sp;
static uint64_t ground_water_dispatch_epoch;
static uint32_t ground_water_patch_mask;
static unsigned ground_water_kind;
static void grounded_water_restore() {
    if(!ground_water_patch_mask)return;
    for(unsigned i=0;i<std::size(ground_water_patches);++i) {
        const auto& p=ground_water_patches[i];
        if((ground_water_patch_mask&(1u<<i)) && psx_mod_read_word(p.address)==p.replacement)
            psx_mod_write_code_word(p.address,p.original);
    }
    ground_water_patch_mask=0;
}
static void grounded_water_dispatch(CPUState* cpu,uint32_t ra) {
    ground_water_dispatch_sp=0;
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || ra!=0x8004b634 || !grounded_water_modern() ||
       !f.active || f.held[original_aim] || !independent_camera() ||
       (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x202002c1) || !player_identity_ready())return;
    const unsigned a=psx_mod_read_half(player+0x60);
    const bool flight=psx_mod_read_byte(player+0x22c)==9 && a>=97 && a<=108;
    if(!((a>=72 && a<=79) || (a==96 && takeoff_owned()) || flight))return;
    ground_water_dispatch_sp=cpu->gpr[29];ground_water_dispatch_epoch=f.epoch;
    ground_water_kind=flight?3:1;
}
// Keep descending flights in the grounded band on the normal landing path.
// The native transition otherwise treats a dipped neck as surface swimming.
// Patch before function entry so the runtime selects its guarded interpreter;
// recheck the refreshed water depth at the final neck lookup. Deep entry and
// existing surface/underwater modes retain native physics and effects.
static void grounded_water_transition(CPUState* cpu,uint32_t ra) {
    ground_water_dispatch_sp=0;
    if(cpu->gpr[4]!=player || ra!=0x8005a4f8 || !grounded_water_modern() ||
       psx_mod_read_byte(player+0x22c)!=9 || !input_snapshot(Context::Gameplay).active ||
       !independent_camera() || !player_identity_ready())return;
    ground_water_dispatch_sp=cpu->gpr[29];
    ground_water_dispatch_epoch=input_snapshot(Context::Gameplay).epoch;ground_water_kind=2;
}
static void grounded_water_query(CPUState* cpu,uint32_t address) {
    if(!ground_water_dispatch_sp)return;
    const uint32_t sp=cpu->gpr[29];
    const uint32_t target=ground_water_kind==2?0x80055e80:0x80048410;
    const uint32_t caller=ground_water_kind==2?0x8005a4f8:0x8004b634;
    const unsigned frame_size=ground_water_kind==2?0x50:0x28;
    const bool entry=address==target && sp==ground_water_dispatch_sp;
    if(!input_modernized() || input_snapshot(Context::Gameplay).epoch!=ground_water_dispatch_epoch ||
       sp<0x80010100 || sp>0x801ffff0 ||
       (!entry && (sp>ground_water_dispatch_sp-frame_size ||
                    psx_mod_read_word(ground_water_dispatch_sp-4)!=caller)) ||
       g_precise_mode || g_ls_mode || g_psx_call_bail || !identity() ||
       (ground_water_kind>=2 && !grounded_water_modern())) {
        ground_water_dispatch_sp=0;return;
    }
    for(const auto& p:ground_water_patches)
        if(psx_mod_read_word(p.address)!=p.original) {ground_water_dispatch_sp=0;return;}
    for(unsigned i=0;i<std::size(ground_water_patches);++i) {
        const auto& p=ground_water_patches[i];
        if((ground_water_kind==1 && p.address!=0x80056240 && p.address!=0x80054cbc) ||
           (ground_water_kind==2 && p.address==0x80056240) ||
           (ground_water_kind==3 && p.address==0x80054cbc)) {
            psx_mod_write_code_word(p.address,p.replacement);
            ground_water_patch_mask|=1u<<i;
        }
    }
}

// Other registered plugins (wall collision, aiming, sound) can check identity
// inside the native dispatch. Recognize only our exact owned substitutions;
// every other byte and every unowned patch still has to match the disc guard.
static uint32_t grounded_water_identity_word(uint32_t address) {
    const uint32_t word=psx_mod_read_word(address);
    if(ground_water_patch_mask)for(unsigned i=0;i<std::size(ground_water_patches);++i) {
        const auto& p=ground_water_patches[i];
        if((ground_water_patch_mask&(1u<<i)) && address==p.address && word==p.replacement)return p.original;
    }
    return word;
}
