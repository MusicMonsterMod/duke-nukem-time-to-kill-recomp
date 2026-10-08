// D08A4 portable steroids (Modernized, EDuke32 style; DNTTK_STEROIDS=original
// keeps the original activate-on-pickup). A steroids pickup is kept as an
// owned item; R (or Enter / U with steroids selected in the switcher) uses it.
//
// The original still has a portable steroids item; only its pickup skips it:
// - Item 4 is steroids: flags player+0x364, amount/timer +0x366, full amount
//   0x800c2722 (9000). The item table types it as a timed toggle (3), like the
//   Bio Mask and goggles.
// - Owned (bit 0) with an amount is a held item: the inventory grant 0x8003d738
//   (dninventory) gives exactly that, and the original Select inventory lists it
//   (0x80088134) and uses it the way it uses every timed toggle (0x80089494:
//   flags ^= 2). The use routine 0x800402d0 then only clears its pending bit.
// - While bit 1 is on, 0x800414a0 drains the amount by the frame step 0x800d21fc
//   and clears bits 0-1 at 0; the kick (0x80049058), the status bar (0x8008bd94)
//   and the damage path 0x800a4154 (which takes 1500 and can end it) read bit 1.
// - The level-end snapshot 0x80083348 (the carry record 0x800dd030 the memory
//   card save writes) keeps items 0-6 as bit 0 and the amount, so a held item
//   is saved like the jetpack.
// - Pickup: dispatcher 0x80081a48 (called only from 0x8007fe64) case 0x800827e8
//   for type 638 sets the full amount and bit 1 (it activates), puts message 110
//   (STEROIDS) into 0x800dcd90[player 0x233]; the shared tail 0x800828d0 plays
//   sound 0x100f at Duke through 0x8006b73c (ra 0x800828d8) and the dispatcher
//   returns 1, so the caller removes the pickup.
//
// Pickup: at dispatcher entry a steroids touch is noted. At the tail's sound
// call, still inside the same pickup, bit 1 goes back off and bit 0 on: the
// pickup is held at the full amount the original wrote. Message, sound and
// removal stay the original's. While steroids already run, the pickup is left
// to the original, which refreshes them; while one is held, it stays where it
// is, as in Duke 3D (GAME.CON: ifpinventory GET_STEROIDS).
// Use: the original menu's toggle (bit 1 on, the held amount drains), the
// pickup sound and the Duke 3D quote 12 wording "USED STEROIDS".
static bool portable_steroids() {
    static const bool original=[]{const char* v=std::getenv("DNTTK_STEROIDS");return v && !std::strcmp(v,"original");}();
    return input_modernized() && !original;
}
// D08A13: Duke's damage handler 0x800a40a8 (class table 0x800c5ed4; a0 the
// victim player, a1 attacker, a2 damage, a3 type) checks +0x364 bit 1 at
// 0x800a4154, before difficulty scaling, armour or health: running steroids
// take 1500 from +0x366 (or end, bits 0-1 cleared, at 1500 or less) and the
// handler returns 0 with no damage at all. The original steroids are a shield
// that each hit spends. In Modernized portable steroids are their own timed
// effect (user rule: steroids and armour are independent), so for Duke's
// damage call, and only for it, bit 1 is off: the hit takes the original's
// normal path (armour, health, pain, death and its return value) and the
// timer is untouched. Nothing else in the handler reads +0x364, and every
// other reader (the drain inside Duke's update 0x800412a4, the kick in
// 0x80048410, the status bar 0x8008ba30, the heartbeat) runs from a hooked
// function, whose entry puts bit 1 back first.
// DNTTK_STEROID_SHIELD=original keeps the original shield (diagnostics).
static bool steroid_damage_hidden;
static uint64_t steroid_damage_loads;
static unsigned steroid_damage_passes;
static bool steroid_shield_original() {
    static const bool keep=[]{const char* v=std::getenv("DNTTK_STEROID_SHIELD");return v && !std::strcmp(v,"original");}();
    return keep;
}
// The flags as the effect stands, with bit 1 while a damage call has it off.
static unsigned steroid_flags() {
    return psx_mod_read_half(player+0x364)|(steroid_damage_hidden?2u:0u);
}
static void steroids_damage_restore() {
    if(!steroid_damage_hidden) return;
    steroid_damage_hidden=false;
    if(steroid_damage_loads!=psx_mod_savestate_loads() || (int16_t)psx_mod_read_half(player+0x366)<=0) return;
    psx_mod_write_half(player+0x364,(uint16_t)(psx_mod_read_half(player+0x364)|2));
}
// Held and not running: the one dose R can take.
static unsigned steroid_doses() {
    if(!portable_steroids()) return 0;
    return (steroid_flags()&3)==1 && (int16_t)psx_mod_read_half(player+0x366)>0;
}
// Held or running: steroids stay in the switcher and the HUD box (D08A8), like
// a jetpack that is on. Running is bit 1 alone (D08A11): dnhyper and the
// original's own activations set it without the owned bit, and the drain
// 0x800414a0 clears bits 0-1 when it ends, so nothing is left held.
static bool steroids_owned() {
    if(!portable_steroids()) return false;
    return (steroid_flags()&3) && (int16_t)psx_mod_read_half(player+0x366)>0;
}
static bool steroids_running() {
    return steroids_owned() && (steroid_flags()&2);
}
// Entry of Duke's damage handler 0x800a40a8 (see steroid_flags above).
static void steroids_damage_entry(CPUState* cpu) {
    steroids_damage_restore();  // a second hit: the previous call has returned
    if(cpu->gpr[4]!=player || steroid_shield_original() || !steroids_running()) return;
    psx_mod_write_half(player+0x364,(uint16_t)(psx_mod_read_half(player+0x364)&~2u));
    steroid_damage_hidden=true;steroid_damage_loads=psx_mod_savestate_loads();++steroid_damage_passes;
}
// D08A8: running steroids count down in their own HUD box (gadget_hud.inc).
// The status bar's element 2 (0x8008bcf4..0x8008be00) shows the steroids'
// percent with the armour icon while +0x364 bit 1 is on (and so shows even
// with no armour); it reads that bit only there. So for the status bar draw,
// and only for it, bit 1 is off: element 2 is the original armour element.
// It goes back on at the first hook after the status bar returns
// (0x80026590): 0x8001fc44 or 0x8002e850, which every path of the view
// composition 0x80026164 reaches before anything else runs (and at the next
// status bar entry and 0x800b4d9c, in case). Replay workers replay the whole
// composition, so they restore it the same way.
// Bit 1 is the only mark of running steroids (bit 0 may be off), so the hide
// remembers it; a savestate loaded in between keeps the loaded flags.
static bool steroid_hud_hidden;
static uint64_t steroid_hud_loads;
static unsigned steroid_hud_hides;
static void steroids_hud_restore() {
    if(!steroid_hud_hidden) return;
    steroid_hud_hidden=false;
    if(steroid_hud_loads!=psx_mod_savestate_loads() || (int16_t)psx_mod_read_half(player+0x366)<=0) return;
    psx_mod_write_half(player+0x364,(uint16_t)(psx_mod_read_half(player+0x364)|2));
}
static void steroids_hud_hide() {
    if(steroid_hud_hidden || !steroids_running()) return;
    psx_mod_write_half(player+0x364,(uint16_t)(psx_mod_read_half(player+0x364)&~2u));
    steroid_hud_hidden=true;steroid_hud_loads=psx_mod_savestate_loads();++steroid_hud_hides;
}
struct SteroidPickup {bool pending;uint16_t flags;};
static SteroidPickup steroid_pickup;
static uint32_t steroid_left_object;
static unsigned steroid_stores,steroid_full_leaves,steroid_refreshes,steroid_uses,steroid_refusals,steroid_late_settles;
static bool steroid_object(uint32_t object) {
    return !(object&3) && object>=0x80010000 && object<=0x801fffa0 && psx_mod_read_half(object+0x2c)==638;
}
// Dispatcher entry (a0 Duke, a1 the touched object).
static void steroids_pickup(CPUState* cpu) {
    steroid_pickup.pending=false;
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x8007fe6c || !portable_steroids() || !player_identity_ready() ||
       psx_mod_read_byte(player+0x233)!=0 || !steroid_object(cpu->gpr[5])) return;
    const unsigned flags=psx_mod_read_half(player+0x364);
    if(flags&2) {++steroid_refreshes;return;}  // running: the original refresh
    if(steroid_doses()) {
        // A blank object the dispatcher has no case for: nothing is picked up.
        if(!steroid_left_object) steroid_left_object=psx_mod_alloc_guest_memory(0x60,16);
        if(!steroid_left_object) return;
        psx_mod_write_half(steroid_left_object+0x2c,0xffff);
        cpu->gpr[5]=steroid_left_object;++steroid_full_leaves;
        return;
    }
    steroid_pickup={true,(uint16_t)flags};
}
// Hold the pickup instead of running it.
static void steroids_settle() {
    if(!steroid_pickup.pending) return;
    steroid_pickup.pending=false;
    const unsigned flags=psx_mod_read_half(player+0x364);
    psx_mod_write_half(player+0x364,(uint16_t)((flags&~2u)|1u));
    psx_mod_write_half(player+0x366,psx_mod_read_half(0x800c2722));
    ++steroid_stores;
}
// Entry of 0x8006b73c from the pickup tail (ra 0x800828d8; s1 is Duke).
static void steroids_pickup_sound(CPUState* cpu) {
    if(steroid_pickup.pending && cpu->gpr[17]==player) steroids_settle();
}
// A call the pickup tail did not reach (it always does): settle on the next
// weapon/item poll so a held dose is never lost to the effect.
static void steroids_poll() {
    if(steroid_pickup.pending) {++steroid_late_settles;steroids_settle();}
}
static bool steroids_use(CPUState* cpu) {
    if(!steroid_doses()) return false;  // none held (or running): nothing happens
    if((int16_t)psx_mod_read_half(player+0x32)<=0 || !player_identity_ready()) {++steroid_refusals;return false;}
    psx_mod_write_half(player+0x364,(uint16_t)(psx_mod_read_half(player+0x364)|2));
    ++steroid_uses;
    input_notice("USED STEROIDS");
    // The pickup sound at Duke, as the pickup tail plays it, on a private stack.
    const uint32_t top=cpu->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return true;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState call=*cpu;call.pc=0;call.gpr[29]=top-0x100;call.gpr[31]=0x800000fc;
    call.gpr[4]=0x100f;call.gpr[5]=player+4;call.gpr[6]=0x800;call.gpr[7]=(uint32_t)-10;
    guest_busy=true;psx_dispatch_call(&call,0x8006b73c,0x800000fc);guest_busy=false;
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return true;
}
static const char* steroids_debug_json() {
    static char buffer[400];
    std::snprintf(buffer,sizeof buffer,"{\"portable\":%s,\"doses\":%u,\"flags\":%u,\"timer\":%d,\"stores\":%u,\"full_leaves\":%u,\"refreshes\":%u,\"uses\":%u,\"refusals\":%u,\"late_settles\":%u,\"hud_hides\":%u,\"hud_hidden\":%s,\"shield\":\"%s\",\"damage_passes\":%u,\"damage_hidden\":%s}",
        portable_steroids()?"true":"false",steroid_doses(),psx_mod_read_half(player+0x364),(int16_t)psx_mod_read_half(player+0x366),
        steroid_stores,steroid_full_leaves,steroid_refreshes,steroid_uses,steroid_refusals,steroid_late_settles,
        steroid_hud_hides,steroid_hud_hidden?"true":"false",steroid_shield_original()?"original":"independent",
        steroid_damage_passes,steroid_damage_hidden?"true":"false");
    return buffer;
}
