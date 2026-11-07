// Stuck-weapon guard (Modernized, 2026-11-07). User, UI slot 7: "that strange
// bug where i cant shoot or change weapons again".
//
// The state: equipment +0x3b8 = 2 (a weapon drawn, +0x3b9) while the upper
// track +0x74 plays the legs' own clip (63 idle, 72-79 walk / run, the same
// block as +0x60) on a pose without the weapon bit (table 0x800c2824 bit 8).
// That is a holstered Duke's free upper body. With equipment 2 the original
// never takes it back: the weapon state handlers only run from weapon-owned
// poses, so fire, the number keys and the wheel all do nothing, the upper
// keeps following the legs, and a savestate keeps the state.
//
// The cause found: the D08Z8 sidearm loan (sidearm_mantle.inc), fixed there.
// Host code that lends or hands off the upper track (D08Z2 mantles, D08J7
// hangs, D08J9 doors, D08Z8 loans) can leave the same state if a restore is
// ever skipped, so this guard is the backstop: on settled ground, after the
// state has lasted a few player updates with nothing in progress, Duke is put
// back to a holstered state (equipment 0, the upper on the legs as it already
// is) and the same weapon is requested through the original's own draw
// (+0x3ba, +0x224 |= 4), the way a number key draws from holstered. Each
// repair is logged with its context.
static unsigned weapon_pose_bad_updates,weapon_pose_repairs;
static unsigned weapon_pose_last_lower,weapon_pose_last_weapon;
static uint64_t weapon_pose_loads;
static bool weapon_pose_stuck() {
    if(psx_mod_read_byte(player+0x3b8)!=2)return false;
    const unsigned weapon=psx_mod_read_byte(player+0x3b9);
    if(weapon==0 || weapon>14)return false;
    const unsigned lower=psx_mod_read_half(player+0x60),upper=psx_mod_read_half(player+0x74);
    if(upper!=lower || upper>=280 || !land_gait_anim(lower))return false;
    if(psx_mod_read_word(0x800c2824+4*upper)&8)return false;   // a weapon-owned pose
    if(psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0)return false;
    if(psx_mod_read_byte(player+0x27d)!=0)return false;          // a use in progress
    if(psx_mod_read_word(player+0x224)&0x2cdu)return false;       // a request, a use, a hold
    if((int16_t)psx_mod_read_half(player+0x32)<=0)return false;
    if(psx_mod_read_word(player)&0x20002)return false;
    // Host handoffs still open restore the upper themselves.
    if(mantle_upper_pending || hang_drop_pending || sidearm_loan || door_window || sidearm_climb_live)return false;
    return true;
}
// Once per player update (0x8005a210 entry), after the traversal helpers.
static void weapon_pose_guard() {
    const uint64_t loads=psx_mod_savestate_loads();
    if(loads!=weapon_pose_loads) {weapon_pose_loads=loads;weapon_pose_bad_updates=0;}
    if(!input_modernized() || !gameplay_context() || !player_identity_ready() || psx_mod_read_byte(player+0x233)!=0 ||
       !weapon_pose_stuck()) {weapon_pose_bad_updates=0;return;}
    if(++weapon_pose_bad_updates<6)return;   // about 0.2 s
    weapon_pose_bad_updates=0;
    const unsigned weapon=psx_mod_read_byte(player+0x3b9);
    weapon_pose_last_lower=psx_mod_read_half(player+0x60);weapon_pose_last_weapon=weapon;
    psx_mod_write_byte(player+0x3b8,0);
    psx_mod_write_byte(player+0x3ba,weapon);
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|4u);
    ++weapon_pose_repairs;
    std::fprintf(stderr,"[TTK weapon] weapon pose repaired (%u): weapon %u drawn with the upper on the legs' clip %u; redrawn "
        "(sidearm loans %u, loan repairs %u, mantle handoffs %u/%u, hang drops %u, door uses %u)\n",
        weapon_pose_repairs,weapon,weapon_pose_last_lower,sidearm_loans,sidearm_loan_repairs,
        mantle_upper_handoffs,mantle_upper_restores,hang_drop_handoffs,door_uses);
}
static const char* weapon_pose_debug_json() {
    static char buffer[128];
    std::snprintf(buffer,sizeof buffer,"{\"repairs\":%u,\"bad_updates\":%u,\"last_lower\":%u,\"last_weapon\":%u,\"stuck\":%s}",
        weapon_pose_repairs,weapon_pose_bad_updates,weapon_pose_last_lower,weapon_pose_last_weapon,weapon_pose_stuck()?"true":"false");
    return buffer;
}
