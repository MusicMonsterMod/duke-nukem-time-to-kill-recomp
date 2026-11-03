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
static unsigned pickup_flashes;
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
    screen_flash(f.intensity,f.r,f.g,f.b);++pickup_flashes;
    char note[64];
    std::snprintf(note,sizeof note,"type %u %s %d %d %d %d",type,f.kind,f.intensity,f.r,f.g,f.b);
    screen_flash_pickup_note(note);
    if(damage_trace()) std::fprintf(stderr,"[TTK pickup] type=%u -> %s palfrom %d %d %d %d\n",type,f.kind,f.intensity,f.r,f.g,f.b);
}
