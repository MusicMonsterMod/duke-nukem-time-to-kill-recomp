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
static void stance_request(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    crouch_stand_pending=false;
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
