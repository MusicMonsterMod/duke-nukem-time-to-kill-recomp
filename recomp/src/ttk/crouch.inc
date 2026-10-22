// Original crouch enter/reverse animation 176; idle 178; movement 181..184.
// Own only requests started on supported ground. Never synthesize Triangle's turn.
static bool crouch_owned, crouch_boost;
static uint32_t crouch_sp;
static unsigned crouch_entries,crouch_exits,crouch_blocked;
// D08K7: still kneeling (178) with crouch released, the stand-up refused or not
// yet allowed. crouch_walk.inc keeps the directions (never the original roll)
// and stance_request stands Duke as soon as there is room.
static bool crouch_stand_pending;
static bool crouch_stand_room(CPUState* cpu);   // crouch_hull.inc
static bool crouch_pose(unsigned animation) {return animation==176 || (animation>=178 && animation<=184);}
// D08K11: water alone does not imply swimming. Keep the original depth
// boundary and native modes; never claim the deep-water animation 176 dip.
static bool crouch_water_supported() {
    if(!in_water_early())return true;
    const char* option=ttk_live_env("DNTTK_CROUCH");
    return input_modernized() && (!option || std::strcmp(option,"original")) &&
           shallow_land_water_early();
}
static bool crouch_ground_state(bool landed=false) {
    const unsigned mode=psx_mod_read_byte(player+0x22c),previous=psx_mod_read_byte(player+0x22d);
    if(mode==0 && (previous==0 || (landed && previous==9)))return true;
    // Native mid-depth wading is mode 1; crossing its boundary can leave
    // 0/1 or 1/0 for an update. Keep both native bytes untouched.
    return mode<=1 && previous<=1 && in_water_early();
}
static void stance_request(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    crouch_stand_pending=false;
    if(!player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       !crouch_ground_state()) {crouch_owned=false;return;}
    // Original crouch and swimming retain their existing ownership.
    if(!crouch_water_supported()) {
        // Leaving the grounded water band must release the host's idle pose:
        // native water transitions deliberately exclude crouch clips 176..184.
        // Play the ordinary stand-up, then let the native head/depth checks
        // choose swimming. Never force mode 4/5 or clear the submersion mask.
        const unsigned a=psx_mod_read_half(player+0x60);
        if(crouch_owned && (a==176 || a==178)) {
            if(!crouch_stand_room(cpu)) {crouch_stand_pending=true;return;}
            psx_mod_write_half(player+0x60,176);psx_mod_write_half(player+0x68,0);
            psx_mod_write_half(player+0x6a,1);
        }
        crouch_owned=false;return;
    }
    unsigned animation=psx_mod_read_half(player+0x60),flags=psx_mod_read_word(player+0x224);
    if(crouch_owned && !crouch_pose(animation))crouch_owned=false;
    bool held=f.active && f.held[crouch];
    if(held && !crouch_owned && movement_ready() && !(flags&0x200002c5) && !f.held[original_aim] && !f.held[interact]) {
        psx_mod_write_half(player+0x60,176);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        crouch_owned=true;++crouch_entries;
    } else if(crouch_owned && !held && animation!=176 && !(flags&0x200002c5)) {
        // The 0x370 standing clearance of 7926c over the floor Duke is on: the
        // room's, or a walkable object's top (D08K7: on the slot 10 dumpster any
        // object contact refused the stand-up forever).
        if(!crouch_stand_room(cpu)) {++crouch_blocked;crouch_stand_pending=true;return;}
        psx_mod_write_half(player+0x60,176);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,1);++crouch_exits;
    }
    crouch_stand_pending=crouch_owned && !held && psx_mod_read_half(player+0x60)==178;
    crouch_sp=cpu->gpr[29];crouch_boost=false;
}
static void stance_advance(CPUState* cpu) {
    if(!crouch_owned || crouch_boost || psx_mod_read_half(player+0x60)!=176 ||
       cpu->gpr[4]!=player+0x60 || cpu->gpr[5]!=player || cpu->gpr[31]!=0x8005a3d4 ||
       cpu->gpr[6]!=crouch_sp-0x30 || !player_identity_ready())return;
    int delta=(int16_t)psx_mod_read_half(cpu->gpr[6]);
    if(delta>0 && delta<=4096) {psx_mod_write_half(cpu->gpr[6],delta*4);crouch_boost=true;}
}

// Native crouch idle (548a4) forcibly stands in the 512..640 water band.
// For an owned Modern crouch only, take its existing normal-input branch.
// The resident guard already covers 5484c..5492c. Restore at the very next
// hook, before any identity check; no water, state or collision data changes.
static constexpr uint32_t crouch_water_branch=0x800548e8;
static void crouch_water_restore() {
    if(psx_mod_read_word(crouch_water_branch)==0x10000008)
        psx_mod_write_code_word(crouch_water_branch,0x14400008);
}
static bool crouch_water_dispatch(CPUState* cpu,uint32_t ra) {
    if(cpu->gpr[4]!=player || ra!=0x8004b634 ||
       psx_mod_read_half(player+0x60)!=178 || !crouch_owned ||
       !crouch_ground_state() || !in_water_early() || !crouch_water_supported() ||
       int32_t(psx_mod_read_word(player+0x1c8))-int32_t(psx_mod_read_word(player+0x834))<0x200 ||
       !player_identity_ready() || g_precise_mode || g_ls_mode || g_psx_call_bail ||
       psx_mod_read_word(crouch_water_branch)!=0x14400008)return false;
    psx_mod_write_code_word(crouch_water_branch,0x10000008);
    return true;
}
