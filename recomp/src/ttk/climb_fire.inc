// D08J6A: firing while climbing (note 377); D08J7A: and from ledge hangs (note 378). Included in the guarded controls
// adapter after shortcuts.inc. The original climb states already stop, draw,
// fire and stow: the climb dispatcher 0x80043eb8 refuses to move while the
// upper animation carries table flag 8 (a weapon pose), a weapon request or a
// Circle tap draws while attached, Cross fires real rounds and a second Circle
// tap stows. Modernized adds the controls: fire draws the sidearm, climbing
// input stows, shots follow the camera (weapon_aim.cpp).
//
// Uncharted/Tomb Raider rule: one hand stays on the climb, so only the
// one-handed sidearm (slot 4) comes out. Another selected weapon is swapped
// for it and given back once Duke holsters, or on the ground after letting go
// armed.
static constexpr unsigned climb_sidearm=4;
static constexpr unsigned climb_no_swap=255;
static unsigned climb_swap_from=climb_no_swap;
static unsigned climb_fire_draws,climb_swaps,climb_restores,climb_ground_restores;
// Ladder rungs 186-189, chain/pole 192-195, climbing wall 198-205: the
// clips the climb dispatcher runs for a settled attachment (mounts, catches
// and exits excluded).
static bool climb_clip(unsigned a) {return (a>=186 && a<=189) || (a>=192 && a<=195) || (a>=198 && a<=205);}
static bool attached_weapon_owner() {
    return input_modernized() && gameplay_context() &&
       !(psx_mod_read_word(player)&0x20002) && !(psx_mod_read_word(player+0x224)&0x20000000) &&
       psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera;
}
// D08J7A (note 378): the settled ledge hang (149, mode 6) gets the same
// lease. The original already draws there on Circle or the weapon request,
// fires with one hand on the lip and stows; armed, it refuses the shimmy,
// pull-up and drop (its moves wait for an upper body without a weapon pose).
bool hang_weapon_ready() {
    return attached_weapon_owner() && psx_mod_read_byte(player+0x22c)==6 && psx_mod_read_byte(player+0x22d)==6 &&
        psx_mod_read_half(player+0x60)==149 && identity();
}
bool climb_weapon_ready() {
    if(!attached_weapon_owner())return false;
    if(psx_mod_read_byte(player+0x22c)==6)return hang_weapon_ready();
    return psx_mod_read_byte(player+0x22c)==3 && psx_mod_read_byte(player+0x22d)==3 &&
        climb_clip(psx_mod_read_half(player+0x60)) && psx_mod_read_word(player+0x17c) && identity();
}
// Holstered on a climb with a loaded sidearm: held fire stops the climb (the
// host sends no climb directions) and must never reach the climb handlers as
// unarmed Cross (Down + Cross steps off a ladder, D08U).
bool climb_fire_hold_ready() {
    return climb_weapon_ready() && psx_mod_read_byte(player+0x3b8)==0 && usable_weapon(climb_sidearm);
}
// The draw starts at once, also inside a climb step: with no direction sent
// the step ends at its rest, where the flag-8 refusal holds Duke.
static bool climb_fire_draw_ready() {
    const unsigned a=psx_mod_read_half(player+0x60);
    return climb_fire_hold_ready() && psx_mod_read_half(player+0x74)==a &&
        !(psx_mod_read_word(player+0x224)&0x2cd);
}
// Armed on a climb in the weapon's settled idle, or the one-arm aim hold (8)
// that lingers after the last shot: climbing input may spend one Circle tap
// to stow (otherwise the same idle set as the E stow).
bool climb_stow_ready() {
    const unsigned upper=psx_mod_read_half(player+0x74);
    return climb_weapon_ready() && psx_mod_read_byte(player+0x3b8)==2 &&
        !(psx_mod_read_word(player+0x224)&4) &&
        (upper==5 || upper==8 || upper==20 || upper==29 || upper==35 || upper==39);
}
bool climb_shot_ready() {return climb_weapon_ready() && psx_mod_read_byte(player+0x3b8)==2;}
// The draw request and the swap's return while still attached: at the climb
// dispatcher entry (Duke from 0x8004b60c), and once per player update in a
// ledge hang (hang_fire_update).
static void climb_fire_request() {
    if(!interaction_alive()) {climb_swap_from=climb_no_swap;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    const uint32_t flags=psx_mod_read_word(player+0x224);
    if(f.held[fire] && !f.held[holster] && !f.held[interact] && climb_fire_draw_ready()) {
        const unsigned selected=psx_mod_read_byte(player+0x3ba);
        if(base_weapon(selected)!=climb_sidearm) {
            if(climb_swap_from==climb_no_swap)climb_swap_from=selected;
            ++climb_swaps;
        }
        psx_mod_write_byte(player+0x3ba,climb_sidearm);
        psx_mod_write_word(player+0x224,flags|4);++selections;++climb_fire_draws;
        return;
    }
    const unsigned a=psx_mod_read_half(player+0x60);
    if(climb_swap_from!=climb_no_swap && climb_weapon_ready() && psx_mod_read_byte(player+0x3b8)==0 &&
       !(flags&0x2cd) && psx_mod_read_half(player+0x74)==a) {
        psx_mod_write_byte(player+0x3ba,climb_swap_from);
        climb_swap_from=climb_no_swap;++climb_restores;
    }
}
static void climb_fire_dispatch(CPUState* cpu) {
    if(cpu->gpr[4]==player && cpu->gpr[31]==0x8004b60cu)climb_fire_request();
}
// D08J7A: S or Space in an armed ledge hang lets go with the weapon out, as
// Space does on ladders. For one player update the upper track takes the hang
// pose (the original then releases through Down, 108); at the next update the
// kept weapon block (+0x74, 0x14 bytes) comes back, in the fall or, if the
// original refused, still hanging. Same handoff as the D08Z2 mantles.
static uint32_t hang_drop_block[0x14/4];
static bool hang_drop_pending;
static uint64_t hang_drop_loads;
static unsigned hang_drop_handoffs,hang_drop_restores;
bool hang_drop_armed_ready(bool drop) {
    const unsigned upper=psx_mod_read_half(player+0x74);
    return drop && hang_weapon_ready() && psx_mod_read_byte(player+0x3b8)==2 &&
        !(psx_mod_read_word(player+0x224)&0x2cd) &&
        (upper==5 || upper==8 || upper==20 || upper==29 || upper==35 || upper==39);
}
static void hang_fire_update() {
    if(hang_drop_pending) {
        hang_drop_pending=false;
        const unsigned upper=psx_mod_read_half(player+0x74);
        if(psx_mod_savestate_loads()!=hang_drop_loads || !player_identity_ready() ||
           psx_mod_read_byte(player+0x3b8)!=2 || upper>=280 || (psx_mod_read_word(0x800c2824+4*upper)&8))return;
        for(unsigned i=0;i<std::size(hang_drop_block);++i)psx_mod_write_word(player+0x74+4*i,hang_drop_block[i]);
        ++hang_drop_restores;
        return;
    }
    if(!hang_weapon_ready())return;
    climb_fire_request();
    const auto& f=input_snapshot(Context::Gameplay);
    const bool drop=f.active && !f.held[fire] && !f.held[holster] && !f.held[interact] && !f.held[move_forward] &&
        (f.held[jump] || f.held[move_back]);
    if(!hang_drop_armed_ready(drop))return;
    for(unsigned i=0;i<std::size(hang_drop_block);++i)hang_drop_block[i]=psx_mod_read_word(player+0x74+4*i);
    hang_drop_loads=psx_mod_savestate_loads();hang_drop_pending=true;
    psx_mod_write_half(player+0x74,149);
    psx_mod_write_half(player+0x7c,0);psx_mod_write_half(player+0x7e,0);
    ++hang_drop_handoffs;
}
// Ground weapon selection (select_weapon): Duke let go with the sidearm out.
// Once settled, the original request switches back to the swapped weapon; a
// holstered Duke just gets its selection back.
static void climb_fire_ground_restore(const InputFrame& f,unsigned equipment,uint32_t flags) {
    if(climb_swap_from==climb_no_swap)return;
    if(!interaction_alive()) {climb_swap_from=climb_no_swap;return;}
    if(!movement_ready() || f.held[fire] || f.held[original_aim] || (flags&0x2cd))return;
    const unsigned from=climb_swap_from;
    if(equipment==0) {
        psx_mod_write_byte(player+0x3ba,from);
        climb_swap_from=climb_no_swap;++climb_restores;return;
    }
    const unsigned upper=psx_mod_read_half(player+0x74);
    if(equipment!=2 || base_weapon(psx_mod_read_byte(player+0x3b9))!=climb_sidearm) {climb_swap_from=climb_no_swap;return;}
    if(!(upper==5 || upper==20 || upper==29 || upper==35 || upper==39))return;
    if(base_weapon(from)>14) {climb_swap_from=climb_no_swap;return;}
    // D08J7A: the original can report a weapon unusable for a moment (seen
    // landing on the dumpster after an armed hang drop); wait for it. A
    // holster or another selection ends the wait above.
    if(!usable_weapon(base_weapon(from)))return;
    climb_swap_from=climb_no_swap;
    psx_mod_write_byte(player+0x3ba,from);
    psx_mod_write_word(player+0x224,flags|4);++selections;++climb_ground_restores;
}
static const char* climb_fire_debug_json() {
    static char buffer[320];
    std::snprintf(buffer,sizeof buffer,"{\"ready\":%s,\"hang\":%s,\"hold\":%s,\"stow\":%s,\"shot\":%s,\"swap_from\":%d,\"draws\":%u,\"swaps\":%u,\"restores\":%u,\"ground_restores\":%u,\"hang_drops\":%u,\"hang_drop_restores\":%u}",
        climb_weapon_ready()?"true":"false",hang_weapon_ready()?"true":"false",climb_fire_hold_ready()?"true":"false",climb_stow_ready()?"true":"false",
        climb_shot_ready()?"true":"false",climb_swap_from==climb_no_swap?-1:(int)climb_swap_from,
        climb_fire_draws,climb_swaps,climb_restores,climb_ground_restores,hang_drop_handoffs,hang_drop_restores);
    return buffer;
}
