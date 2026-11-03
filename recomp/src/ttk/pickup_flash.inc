// D39F: pickup flashes on the D39 present tint (note 370, D39F
// section). Modernized and Polymorph only, as every D39 flash. It rides the
// game's own sound calls (hooks that already exist; a new entry hook would
// change the generated code).
//
// Pickups: the dispatcher 0x80081a48 (s1 Duke, s3 the touched object, its
// type at +0x2c) plays a taken pickup's sound in its common tail,
// 0x8006bbd8(s2) at ra 0x80082d3c, only when the pickup was accepted (s0 set;
// a full item or a weapon Duke cannot carry more of stays put and plays
// nothing). The powerup cases ending at 0x800828d0 (steroids among them) play
// their own 0x8006b73c at ra 0x800828d8 first; one of them clears s2
// (0x800828ac) and reaches the tail silently, steroids keep s2 and play both.
// One flash per dispatcher call (pickup_calls, counted at its entry). Duke3D's
// palfrom replaces whatever is fading (the measured rule), so a pickup shows
// even over a hit's red, as in EDuke32.
// DNTTK_DAMAGE_TRACE=1 also prints each pickup (survey).
static unsigned pickup_flashes,pickup_tint_skips;
static bool powerup_tint_running();
static uint64_t pickup_calls,pickup_flashed_call;
static void pickup_flash_begin() {++pickup_calls;}   // dispatcher entry
static bool flash_events_ready() {
    return input_modernized() && !screen_flash_classic() && player_identity_ready() && psx_mod_read_half(0x800bcbb0)==1;
}
// 0x8006bbd8 at ra 0x80082d3c, or 0x8006b73c at ra 0x800828d8 (s0 set).
static void pickup_flash_sound(CPUState* cpu) {
    const uint32_t object=cpu->gpr[19];
    if(cpu->gpr[17]!=player || cpu->gpr[16]!=1 || object<0x80010000u || object>0x801fffa0u ||
       pickup_flashed_call==pickup_calls || !flash_events_ready()) return;
    pickup_flashed_call=pickup_calls;
    const unsigned type=psx_mod_read_half(object+0x2c);
    const DamageFlash f=pickup_flash_for(type);
    // D36M: no pickup flash while a power-up tint runs (user, 2026-11-03:
    // over the tint it "doesn't quite look right"), the coin itself included:
    // its tint starts instead. Hits still flash.
    if(powerup_tint_running()) {
        ++pickup_tint_skips;
        if(damage_trace()) std::fprintf(stderr,"[TTK pickup] type=%u -> none (power-up tint)\n",type);
        return;
    }
    screen_flash(f.intensity,f.r,f.g,f.b);++pickup_flashes;
    char note[64];
    std::snprintf(note,sizeof note,"type %u %s %d %d %d %d",type,f.kind,f.intensity,f.r,f.g,f.b);
    screen_flash_pickup_note(note);
    if(damage_trace()) std::fprintf(stderr,"[TTK pickup] type=%u -> %s palfrom %d %d %d %d\n",type,f.kind,f.intensity,f.r,f.g,f.b);
}
// D36M: the power-up tint (screen_flash.h powerup_tint_for). Each game
// frame (flash_frame, before the frozen blue so that one wins) a running coin
// timer (gadget_hud.inc: +0x8a0 invincibility, +0x89c invisibility, +0x89e
// Double Duke) holds its light tint, unless a stronger flash is still fading.
// Duke's update drains the timers after this hook, so a timer this frame's
// step uses up has already ended: the tint ends with the frame that shows it.
// Ending clears only our own hold; Vanilla and Classic draw nothing.
static unsigned powerup_holds;
static DamageFlash powerup_held;
static bool powerup_running(uint32_t timer) {
    const int32_t left=(int16_t)psx_mod_read_half(player+timer);
    const int32_t step=(int16_t)psx_mod_read_half(0x800d21fc);
    return left>0 && left>std::max<int32_t>(step,0);
}
static bool powerup_tint_running() {
    return (int16_t)psx_mod_read_half(player+0x8a0)>0 || (int16_t)psx_mod_read_half(player+0x89c)>0 ||
        (int16_t)psx_mod_read_half(player+0x89e)>0;
}
static void powerup_tint_update() {
    const bool alive=(int16_t)psx_mod_read_half(player+0x32)>0;
    const DamageFlash want=flash_events_ready() && alive?
        powerup_tint_for(powerup_running(0x8a0),powerup_running(0x89c),powerup_running(0x89e)):DamageFlash{};
    const FlashState now=screen_flash_read();
    const bool ours=powerup_held.intensity && now.r==powerup_held.r && now.g==powerup_held.g && now.b==powerup_held.b &&
        now.level<=(uint32_t)powerup_held.intensity*flash_tic;
    if(!want.intensity) {
        if(ours) screen_flash(0);
        powerup_held={};return;
    }
    if(!ours && now.level>(uint32_t)want.intensity*flash_tic) return;   // a stronger flash is fading
    screen_flash(want.intensity,want.r,want.g,want.b);
    powerup_held=want;++powerup_holds;
}
// D36M: Duke's update (0x800412a4, flash_frame) stops once he is dead, so
// the record froze with whatever it held: a power-up tint or the killing
// hit's red stayed on the death screen (user report). On a game frame without
// a player update while Duke is dead, run the fade and the power-up check
// here: the red fades as usual and a power-up tint ends at once. Called from
// the main loop's HUD call (0x8002e850 at ra 0x800265d4), an existing hook;
// never in a redraw (replay re-runs guest code, it must not change state).
static unsigned dead_flash_frames;
static void flash_dead_frame() {
    static uint64_t seen=~0ull;
    const uint64_t updates=screen_flash_update_count();
    const bool ran=updates!=seen;
    seen=updates;
    if(ran || psx_mod_replay_active() || !input_modernized() || psx_mod_read_half(0x800bcbb0)!=1 ||
       psx_mod_read_half(0x800be568) || psx_mod_read_half(0x800d2540) ||
       !(psx_mod_read_word(player)&2) || (int16_t)psx_mod_read_half(player+0x32)>0) return;   // dead (game_menu duke_dead)
    screen_flash_update();
    seen=screen_flash_update_count();
    powerup_tint_update();
    ++dead_flash_frames;
}
