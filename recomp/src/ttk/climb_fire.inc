// D08J6A: firing while climbing (note 377); D08J7A: and from ledge hangs (note 378);
// D08J6D: and under climbable ceilings. Included in the guarded controls
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
// armed or pulling up armed, once the player has been off the trigger for
// 0.75 s (D08J7C).
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
// D08J6D: the ceiling monkey bars (mode 7 under a climbable object, D08J4).
// The original draws there on Circle, fires with one hand on the bar, stows,
// and lets go armed through Square; armed, it refuses the hand-over-hand
// travel (150/151). Crates (0xc0) are let go at once (D08X) and never get it.
static bool ceiling_attached() {
    const unsigned a=psx_mod_read_half(player+0x60);
    return attached_weapon_owner() && psx_mod_read_byte(player+0x22c)==7 && psx_mod_read_byte(player+0x22d)==7 &&
        a>=149 && a<=151 && (object_type_flags(psx_mod_read_word(player+0x17c))&object_climbable)==0x80u && identity();
}
bool ceiling_weapon_ready() {return ceiling_attached() && psx_mod_read_half(player+0x60)==149;}
bool climb_weapon_ready() {
    if(!attached_weapon_owner())return false;
    if(psx_mod_read_byte(player+0x22c)==6)return hang_weapon_ready();
    if(psx_mod_read_byte(player+0x22c)==7)return ceiling_weapon_ready();
    return psx_mod_read_byte(player+0x22c)==3 && psx_mod_read_byte(player+0x22d)==3 &&
        climb_clip(psx_mod_read_half(player+0x60)) && psx_mod_read_word(player+0x17c) && identity();
}
// Holstered on a climb with a loaded sidearm: held fire stops the climb (the
// host sends no climb directions) and must never reach the climb handlers as
// unarmed Cross (Down + Cross steps off a ladder, D08U).
// On the ceiling also during a travel step: held fire ends it at the next
// hang (ceiling_hang.inc sends no Up), where the draw starts.
bool climb_fire_hold_ready() {
    return (climb_weapon_ready() || ceiling_attached()) && psx_mod_read_byte(player+0x3b8)==0 && usable_weapon(climb_sidearm);
}
// The draw starts at once, also inside a climb step: with no direction sent
// the step ends at its rest, where the flag-8 refusal holds Duke.
static bool climb_fire_draw_ready() {
    const unsigned a=psx_mod_read_half(player+0x60);
    return climb_weapon_ready() && climb_fire_hold_ready() && psx_mod_read_half(player+0x74)==a &&
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
bool armed_pullup_shot_ready();
bool climb_shot_ready() {return (climb_weapon_ready() && psx_mod_read_byte(player+0x3b8)==2) || armed_pullup_shot_ready();}
// The draw request and the swap's return while still attached: at the climb
// dispatcher entry (Duke from 0x8004b60c), and once per player update in a
// ledge hang or under a ceiling (hang_fire_update).
static void climb_fire_request() {
    if(!interaction_alive()) {climb_swap_from=climb_no_swap;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    const uint32_t flags=psx_mod_read_word(player+0x224);
    // D08J7B: W held in a hang climbs on (the draw then waits for the ground).
    const bool climbing_on=psx_mod_read_byte(player+0x22c)==6 && f.held[move_forward];
    if(f.held[fire] && !f.held[holster] && !f.held[interact] && !climbing_on && climb_fire_draw_ready()) {
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
// D08J7C: W in a ledge hang with the sidearm out pulls up one-handed, firing
// or not: the same handoff starts the pull-up 140, and the gun arm (upper
// track, channel 1) keeps the sidearm's pose while the body and the off hand
// (channels 0, 2, 3) play the pull-up. Any sidearm pose but a draw or stow.
static bool sidearm_pose(unsigned upper) {
    return upper<280 && (psx_mod_read_word(0x800c2824+4*upper)&8) && upper!=6 && upper!=7;
}
// Input: original Up while W is held in a hang with the sidearm out. The
// original's pull-up check (0x8004728c) refuses a weapon pose, so Up does
// nothing until the handoff update gives it the hang pose.
bool hang_pullup_input_ready(bool pull) {
    return pull && hang_weapon_ready() && psx_mod_read_byte(player+0x3b8)==2 &&
        base_weapon(psx_mod_read_byte(player+0x3b9))==climb_sidearm;
}
static bool hang_pullup_armed_ready(bool pull) {
    return hang_pullup_input_ready(pull) && !(psx_mod_read_word(player+0x224)&0x2cd) &&
        sidearm_pose(psx_mod_read_half(player+0x74));
}
// The armed pull-up lasts from the handoff until the lower track leaves 140.
// Its first frame is kept at the restore update. The late phase is the knee
// on the lip to standing: pull-up frame 6 of 10 on (measured on the D35G
// alley platform: chest over the lip between frames 4 and 5, knee 5 and 6);
// revision 2 refused shots before it, revision 3 allows them throughout.
static constexpr unsigned armed_pullup_late=6;
static bool armed_pullup;
static unsigned armed_pullup_first,armed_pullups,armed_pullup_restores;
// The one-handed pull-up pose (onehand_pullup.inc): its weight, 140 position
// (frames from the first, fractional) and writes, for the debug report.
static double onehand_weight,onehand_phase;
// D08J7D: the first-person viewmodel's high ready (0 aimed, 1 raised beside
// the head), the clip's gun-arm weight as onehand_pullup_pose last wrote it.
static double onehand_view_raise;
static bool onehand_owned;
static unsigned onehand_writes;
bool hang_pullup_handoff() {return hang_drop_pending && armed_pullup && hang_weapon_ready();}
static bool armed_pullup_active() {
    return armed_pullup && !hang_drop_pending && input_modernized() && psx_mod_read_half(player+0x60)==140 &&
        psx_mod_read_byte(player+0x22c)==8 && psx_mod_read_byte(player+0x3b8)==2 && player_identity_ready();
}
static bool armed_pullup_late_phase() {
    return (unsigned)(psx_mod_read_half(player+0x66)-armed_pullup_first)>=armed_pullup_late;
}
// D08J7C revision 3: with one hand on the lip Duke can shoot the whole way up
// (user: "duke actually never needs to stop shooting"). Shots are view-aimed
// from the real muzzle; the aim's body-to-muzzle check keeps a muzzle behind
// the lip on Duke's side, so a shot into the ledge face hits it, over the lip
// or along the wall it goes. Only the handoff update stays masked (the hang's
// Cross must not reach the game while the original starts 140). The late
// phase now only times the pose's gun-arm handback (onehand_pullup.inc).
bool armed_pullup_shot_ready() {return armed_pullup_active() && attached_weapon_owner();}
bool armed_pullup_fire_masked() {return armed_pullup && hang_drop_pending;}
// The original aim's look-at (0x80097a44) takes the view direction while the
// armed pull-up fires, as presentation_ready() gives it on the ground, so the
// chest and pistol turn to the crosshair the shots go to (climbs and hangs
// otherwise keep the original direction). View aiming only.
static bool armed_pullup_view_aim() {
    const char* mode=ttk_live_env("DNTTK_WEAPON_AIM");
    return armed_pullup_shot_ready() && independent_camera() && mode && !std::strcmp(mode,"view") &&
        !input_snapshot(Context::Gameplay).held[original_aim] && !(psx_mod_read_word(player+0x224)&0x80);
}
static void hang_fire_update() {
    if(hang_drop_pending) {
        hang_drop_pending=false;
        const unsigned upper=psx_mod_read_half(player+0x74);
        if(psx_mod_savestate_loads()!=hang_drop_loads || !player_identity_ready() ||
           psx_mod_read_byte(player+0x3b8)!=2 || upper>=280 || (psx_mod_read_word(0x800c2824+4*upper)&8)) {
            armed_pullup=false;return;
        }
        for(unsigned i=0;i<std::size(hang_drop_block);++i)psx_mod_write_word(player+0x74+4*i,hang_drop_block[i]);
        if(!armed_pullup) {++hang_drop_restores;return;}
        ++armed_pullup_restores;
        // Refused (still hanging): no pull-up to track.
        if(psx_mod_read_half(player+0x60)!=140)armed_pullup=false;
        else armed_pullup_first=psx_mod_read_half(player+0x66);
        return;
    }
    if(armed_pullup && !armed_pullup_active())armed_pullup=false;
    if(ceiling_weapon_ready()) {climb_fire_request();return;}
    if(!hang_weapon_ready())return;
    climb_fire_request();
    const auto& f=input_snapshot(Context::Gameplay);
    const bool pull=f.active && f.held[move_forward] && !f.held[move_back] && !f.held[holster] && !f.held[interact];
    const bool drop=f.active && !f.held[fire] && !f.held[holster] && !f.held[interact] && !f.held[move_forward] &&
        (f.held[jump] || f.held[move_back]);
    const bool pullup=hang_pullup_armed_ready(pull);
    if(!pullup && !hang_drop_armed_ready(drop))return;
    for(unsigned i=0;i<std::size(hang_drop_block);++i)hang_drop_block[i]=psx_mod_read_word(player+0x74+4*i);
    hang_drop_loads=psx_mod_savestate_loads();hang_drop_pending=true;
    psx_mod_write_half(player+0x74,149);
    psx_mod_write_half(player+0x7c,0);psx_mod_write_half(player+0x7e,0);
    armed_pullup=pullup;
    if(pullup)++armed_pullups; else ++hang_drop_handoffs;
}
// Ground weapon selection (select_weapon): Duke let go with the sidearm out.
// Once settled, the original request switches back to the swapped weapon; a
// holstered Duke just gets its selection back.
// D08J7C: one rule for the armed pull-up, the D08J7A armed drop and the
// D08J6A armed let-go: Duke keeps the sidearm while the player keeps shooting;
// the swapped weapon comes back after sidearm_return_ms off the trigger (real
// time, the same at every frame rate), so taps never start the swap between
// them. The player's own weapon choice cancels it (climb_swap_cancel).
static constexpr double sidearm_return_ms=750;
static double sidearm_fire_ms;
static unsigned sidearm_return_waits,sidearm_return_cancels;
static void climb_swap_cancel() {
    if(climb_swap_from==climb_no_swap)return;
    climb_swap_from=climb_no_swap;++sidearm_return_cancels;
}
// OPTIONS > SIDEARM RETURN: NEVER (the default) keeps the sidearm on the
// ground (the swap is forgotten; on a climb a holster still gives the
// selection back). AUTO runs the timer above.
static bool sidearm_return_auto() {
    const char* v=ttk_live_env("DNTTK_SIDEARM_RETURN");
    return v && !std::strcmp(v,"auto");
}
static void climb_fire_ground_restore(const InputFrame& f,unsigned equipment,uint32_t flags) {
    if(climb_swap_from==climb_no_swap)return;
    if(!interaction_alive()) {climb_swap_from=climb_no_swap;return;}
    if(!sidearm_return_auto() && movement_ready()) {climb_swap_cancel();return;}
    const double now=input_clock_ms();
    if(f.held[fire])sidearm_fire_ms=now;
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
    if(now-sidearm_fire_ms<sidearm_return_ms) {++sidearm_return_waits;return;}
    // D08J7A: the original can report a weapon unusable for a moment (seen
    // landing on the dumpster after an armed hang drop); wait for it. A
    // holster or another selection ends the wait above.
    if(!usable_weapon(base_weapon(from)))return;
    climb_swap_from=climb_no_swap;
    psx_mod_write_byte(player+0x3ba,from);
    psx_mod_write_word(player+0x224,flags|4);++selections;++climb_ground_restores;
}
static const char* climb_fire_debug_json() {
    static char buffer[512];
    std::snprintf(buffer,sizeof buffer,"{\"ready\":%s,\"hang\":%s,\"ceiling\":%s,\"hold\":%s,\"stow\":%s,\"shot\":%s,\"swap_from\":%d,\"draws\":%u,\"swaps\":%u,\"restores\":%u,\"ground_restores\":%u,\"hang_drops\":%u,\"hang_drop_restores\":%u,\"armed_pullup\":%s,\"armed_pullup_late\":%s,\"armed_pullups\":%u,\"armed_pullup_restores\":%u,\"return_waits\":%u,\"return_cancels\":%u,\"onehand_weight\":%.3f,\"onehand_phase\":%.3f,\"onehand_writes\":%u}",
        climb_weapon_ready()?"true":"false",hang_weapon_ready()?"true":"false",ceiling_weapon_ready()?"true":"false",climb_fire_hold_ready()?"true":"false",climb_stow_ready()?"true":"false",
        climb_shot_ready()?"true":"false",climb_swap_from==climb_no_swap?-1:(int)climb_swap_from,
        climb_fire_draws,climb_swaps,climb_restores,climb_ground_restores,hang_drop_handoffs,hang_drop_restores,
        armed_pullup?"true":"false",armed_pullup_shot_ready()?"true":"false",armed_pullups,armed_pullup_restores,sidearm_return_waits,sidearm_return_cancels,onehand_weight,onehand_phase,onehand_writes);
    return buffer;
}
// D08J7B (note 378 part B): fire never interrupts a climb over a lip. Fire
// held or clicked during the climb up the face (139), the pull-up (140), a
// height mantle (134..138) or a hang Duke is climbing on from (149 with W
// held) queues the weapon he had before the E stow; the original weapon
// request then draws it on the first ground frame, at the D08J6A 2x rate.
// The request waits for the pull-up: the original serves it at once in a
// ledge hang, which would stop a platform climb in its brief hang. Modernized also plays the pull-up 140 1.6x faster (1.74 s ->
// about 1.09 s): only the runner's time, so its events, root motion and end
// position stay the original's.
static constexpr unsigned pullup_scale_num=8,pullup_scale_den=5;
static bool mantle_draw_queued,mantle_draw_boost,pullup_boosted[4];
static unsigned mantle_draw_weapon,mantle_draw_updates;
static unsigned mantle_draw_queues,mantle_draw_requests,mantle_draw_drops,pullup_boosts;
static bool mantle_clip(unsigned a) {return a>=134 && a<=140;}
// Duke climbing on: a mantle clip, or the hang between the two climbs of a
// platform route while W is held (the original pulls up from it).
static bool mantle_climbing(const InputFrame& f) {
    const unsigned a=psx_mod_read_half(player+0x60),mode=psx_mod_read_byte(player+0x22c);
    return (mantle_clip(a) && (mode==8 || mode==0)) || (a==149 && mode==6 && f.held[move_forward]);
}
bool mantle_fire_cross_masked() {return input_modernized() && (mantle_draw_queued || mantle_draw_boost) && psx_mod_read_byte(player+0x3b8)!=2;}
static void mantle_fire_update() {
    for(bool& b:pullup_boosted)b=false;
    if(!interaction_alive() || !gameplay_context()) {mantle_draw_queued=mantle_draw_boost=false;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    const unsigned equipment=psx_mod_read_byte(player+0x3b8),upper=psx_mod_read_half(player+0x74);
    const uint32_t flags=psx_mod_read_word(player+0x224);
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mantle_draw_boost && ((equipment==2 && (upper==5 || upper==20 || upper==29 || upper==35 || upper==39)) ||
       (mode==0 && ++mantle_draw_updates>360)))mantle_draw_boost=false;
    if(!f.active)return;
    const bool climbing=mantle_climbing(f);
    if(!mantle_draw_queued && climbing && f.held[fire] && !f.held[holster] && equipment==0 && !(flags&4) &&
       player_identity_ready()) {
        const unsigned w=climb_swap_from!=climb_no_swap?climb_swap_from:psx_mod_read_byte(player+0x3ba);
        if(base_weapon(w)>=1 && base_weapon(w)<=14 && usable_weapon(base_weapon(w))) {
            mantle_draw_queued=true;mantle_draw_weapon=w;++mantle_draw_queues;
        }
    }
    // In the pull-up (no hang follows it) the original keeps the request
    // and serves it on the first ground frame, before the upper track blends
    // out of the mantle; a height mantle (134..138) is served on the ground.
    const bool pullup=psx_mod_read_half(player+0x60)==140 && mode==8;
    if(!mantle_draw_queued || (climbing && !pullup))return;
    // A holster, a weapon of its own or anything but the ground ends it.
    if(f.held[holster] || equipment!=0 || (flags&4) || (mode!=0 && !pullup) || !player_identity_ready()) {
        mantle_draw_queued=false;++mantle_draw_drops;return;
    }
    if((flags&0x2cd) || upper>=280)return;
    climb_swap_from=climb_no_swap;mantle_draw_queued=false;
    psx_mod_write_byte(player+0x3ba,mantle_draw_weapon);
    psx_mod_write_word(player+0x224,flags|4);++selections;++mantle_draw_requests;
    mantle_draw_boost=true;mantle_draw_updates=0;
}
// Animation runner 0x80059db0, once per player update and channel: the legs
// from the player update (ra 0x8005a3d4, delta at sp+0x10 of that frame, a3 0)
// and, when they play the pull-up too, channels 1-3 (right arm, left arm,
// chest/head: ra 0x8005a5a8, 0x8005a75c, 0x8005a7bc; sp+0x12/0x14/0x16, a3 1-3).
// D36H10: every channel's time is scaled. Channels 2 and 3 run their own
// clock and are then put on channel 1's frames by offset (0x8005a11c); left at
// the original rate they kept a stale previous frame whenever channel 1 crossed
// into the next one first, so the chest and left arm jumped up to two frames
// back and forth on every frame of 140: the body rocked about 200 units in
// 66 ms, seen in first person as vibrating arms (water exit, UI slot 12).
// DNTTK_PULLUP_SYNC=off scales only channels 0 and 1, as D08J7B (developer A/B).
static void pullup_advance(CPUState* cpu,uint32_t ra) {
    if(!input_modernized() || psx_mod_read_half(player+0x60)!=140 || psx_mod_read_byte(player+0x22c)!=8 ||
       cpu->gpr[5]!=player || !player_identity_ready())return;
    static const bool sync_off=[]{const char* t=std::getenv("DNTTK_PULLUP_SYNC");return t && !std::strcmp(t,"off");}();
    static constexpr uint32_t returns[4]={0x8005a3d4,0x8005a5a8,0x8005a75c,0x8005a7bc};
    const unsigned ch=cpu->gpr[7];
    if(ch>3 || (sync_off && ch>1) || pullup_boosted[ch] || ra!=returns[ch] || cpu->gpr[4]!=player+0x60+0x14*ch ||
       cpu->gpr[6]!=weapon_update_sp-0x30+2*ch || psx_mod_read_half(player+0x60+0x14*ch)!=140)return;
    const int delta=(int16_t)psx_mod_read_half(cpu->gpr[6]);
    if(delta<=0 || delta>4096)return;
    psx_mod_write_half(cpu->gpr[6],(delta*pullup_scale_num+pullup_scale_den/2)/pullup_scale_den);
    pullup_boosted[ch]=true;
    if(ch==0)++pullup_boosts;
}
static const char* mantle_fire_debug_json() {
    static char buffer[192];
    std::snprintf(buffer,sizeof buffer,"{\"queued\":%s,\"weapon\":%u,\"boost\":%s,\"queues\":%u,\"requests\":%u,\"drops\":%u,\"pullup_boosts\":%u}",
        mantle_draw_queued?"true":"false",mantle_draw_weapon,mantle_draw_boost?"true":"false",
        mantle_draw_queues,mantle_draw_requests,mantle_draw_drops,pullup_boosts);
    return buffer;
}
