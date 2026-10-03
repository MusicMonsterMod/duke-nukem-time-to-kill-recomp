// Original crouch enter/reverse animation 176; idle 178; movement 181..184.
// Own only requests started on supported ground. Never synthesize Triangle's turn.
static bool crouch_owned, crouch_boost;
static uint32_t crouch_sp;
static unsigned crouch_entries,crouch_exits,crouch_blocked;
static bool crouch_pose(unsigned animation) {return animation==176 || (animation>=178 && animation<=184);}
static void stance_request(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(!player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0) {crouch_owned=false;return;}
    // Anim 176 is also the original water dip. Never start crouch ownership in water.
    if(in_water_early()) {crouch_owned=false;return;}
    unsigned animation=psx_mod_read_half(player+0x60),flags=psx_mod_read_word(player+0x224);
    if(crouch_owned && !crouch_pose(animation))crouch_owned=false;
    bool held=f.active && f.held[crouch];
    if(held && !crouch_owned && movement_ready() && !(flags&0x200002c5) && !f.held[original_aim] && !f.held[interact]) {
        psx_mod_write_half(player+0x60,176);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        crouch_owned=true;++crouch_entries;
    } else if(crouch_owned && !held && animation!=176 && !(flags&0x200002c5)) {
        // Original room floor/ceiling query at the current position. Same 0x370
        // standing clearance used by 7926c. Refuse object-relative ownership.
        int32_t clearance=0;
        if(psx_mod_read_word(player+0x174) || !original_call(cpu,0x8007765c,player+4,&clearance) || clearance<0x370) {
            ++crouch_blocked;return;
        }
        psx_mod_write_half(player+0x60,176);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,1);++crouch_exits;
    }
    crouch_sp=cpu->gpr[29];crouch_boost=false;
}
static void stance_advance(CPUState* cpu) {
    if(!crouch_owned || crouch_boost || psx_mod_read_half(player+0x60)!=176 ||
       cpu->gpr[4]!=player+0x60 || cpu->gpr[5]!=player || cpu->gpr[31]!=0x8005a3d4 ||
       cpu->gpr[6]!=crouch_sp-0x30 || !player_identity_ready())return;
    int delta=(int16_t)psx_mod_read_half(cpu->gpr[6]);
    if(delta>0 && delta<=4096) {psx_mod_write_half(cpu->gpr[6],delta*4);crouch_boost=true;}
}
