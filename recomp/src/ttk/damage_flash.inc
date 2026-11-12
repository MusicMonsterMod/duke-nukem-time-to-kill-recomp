// D39E: Duke3D-style damage flashes from Duke's own damage handler
// 0x800a40a8 (a0 victim, a1 hit position, a2 amount, a3 type, stack +0x10
// the attacker or source object; note 370, D39E section). The handler ends a
// hit before any health change when god mode (0x800c3cc6), +0x8a0
// invincibility, victim word bit 0x10000 or 0x20000000, the original steroid
// shield, a worn Bio Mask (gas) or a zero amount cancels it, but its armour
// step (0x800a3d84) runs before the god-mode test. So a hit counts when it
// took health, armour while that gate was open, or Duke's life, or (type 7,
// the Freezer, 0x800a43c4) froze him: that hit takes nothing itself, so the
// freeze is the hit. Compared after the call returns (at the next handler entry or the next player
// update), never guessed from the arguments.
// DNTTK_DAMAGE_TRACE=1 prints every Duke hit and what it drew (survey).
struct DamageHit {bool pending=false;unsigned type=0;int amount=0;uint32_t source=0,ra=0;int projectile=-1;
    int16_t health=0,armour=0;bool gate=false,frozen=false;uint64_t loads=0;};
static DamageHit damage_hit;
static unsigned damage_flashes,damage_cancelled,damage_kept;
// damage_frame_lost (cheats.inc): what this game frame's hits took so far;
// the pellets of one shotgun blast are separate handler calls, judged
// together (reset at each player update).
static bool damage_trace() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_DAMAGE_TRACE");return t && t[0]=='1';}();
    return v;
}
// The cancel tests the handler makes after its armour step (0x800a44ec).
static bool damage_gate_open() {
    return !psx_mod_read_half(0x800c3cc6) && !(psx_mod_read_word(player)&0x10000) && !psx_mod_read_half(player+0x8a0);
}
static void damage_resolve() {
    if(!damage_hit.pending) return;
    const DamageHit h=damage_hit;
    damage_hit.pending=false;
    if(h.loads!=psx_mod_savestate_loads()) return;
    const int16_t health=(int16_t)psx_mod_read_half(player+0x32),armour=(int16_t)psx_mod_read_half(player+0x234);
    const int health_lost=std::max(0,h.health-health),armour_lost=h.gate?std::max(0,h.armour-armour):0;
    const bool died=h.health>0 && health<=0;
    const bool froze=h.gate && !h.frozen && (psx_mod_read_word(player)&0x20000);
    const int lost=health_lost+armour_lost;
    if(lost>0) damage_frame_lost+=lost;
    const DamageFlash f=damage_flash_for(h.type,lost>0?damage_frame_lost:0,died,froze,h.source?psx_mod_read_half(h.source+0x2c):0,h.projectile);
    const FlashState now=screen_flash_read();
    const bool drawn=f.intensity>0 && (uint32_t)f.intensity*flash_tic>=now.level;
    if(drawn) {screen_flash(f.intensity,f.r,f.g,f.b);++damage_flashes;}
    else if(f.intensity>0) ++damage_kept;          // a stronger flash is still running
    else ++damage_cancelled;
    char note[96];
    std::snprintf(note,sizeof note,"type %u lost %d %s %d %d %d %d%s",h.type,lost,f.kind,f.intensity,f.r,f.g,f.b,drawn?"":" (not drawn)");
    screen_flash_damage_note(note);
    if(damage_trace())
        std::fprintf(stderr,"[TTK damage] type=%u amount=%d source=%08x(id %u) proj=%d ra=%08x health %d->%d armour %d->%d gate=%d died=%d -> %s palfrom %d %d %d %d%s\n",
            h.type,h.amount,h.source,h.source?psx_mod_read_half(h.source+0x2c):0,h.projectile,h.ra,h.health,health,h.armour,armour,h.gate,died,
            f.kind,f.intensity,f.r,f.g,f.b,drawn?"":f.intensity?" (kept a stronger flash)":" (no flash)");
}
// Entry of 0x800a40a8, after the D08A13 steroid pass (so a Modernized hit
// under running steroids is a normal hit, and the original shield is not).
static void damage_flash_entry(CPUState* cpu) {
    damage_resolve();  // a previous hit: its call has returned
    // D42: the source of the latest hit on Duke (the death view turns to it).
    if(cpu->gpr[4]==player && input_modernized()) {
        const uint32_t at=cpu->gpr[29]>=0x80000000u && cpu->gpr[29]<0x801ffff0u ? psx_mod_read_word(cpu->gpr[29]+0x10) : 0;
        last_hit_source=at>=0x80010000u && at<=0x801fffa0u && !(at&3) ? at : 0;last_hit_ms=input_clock_ms();
    }
    if(cpu->gpr[4]!=player || !input_modernized() || screen_flash_classic() || !player_identity_ready()) return;
    const uint32_t sp=cpu->gpr[29],ra=cpu->gpr[31];
    DamageHit& h=damage_hit;
    h={};
    h.pending=true;h.type=cpu->gpr[7];h.amount=(int)cpu->gpr[6];h.ra=ra;h.loads=psx_mod_savestate_loads();
    if(sp>=0x80000000u && sp<0x801ffff0u) h.source=psx_mod_read_word(sp+0x10);
    if(h.source<0x80010000u || h.source>0x801fffa0u || (h.source&3)) h.source=0;
    // Projectile hits (0x8006feb4, 0x80073848): s1 is the projectile, its
    // byte +0x30 the row of the table 0x800c4bc0 the type came from.
    const uint32_t s1=cpu->gpr[17];
    if((ra==0x8006febcu || ra==0x80073850u) && s1>=0x80010000u && s1<=0x801fffa0u) h.projectile=psx_mod_read_byte(s1+0x30);
    h.health=(int16_t)psx_mod_read_half(player+0x32);h.armour=(int16_t)psx_mod_read_half(player+0x234);
    h.gate=damage_gate_open();h.frozen=psx_mod_read_word(player)&0x20000;
}
// Developer (DNTTK_TEST_INPUT hurt=type,amount): one hit on Duke through the
// original handler itself, as cheat_call runs an original function (private
// stack and scratchpad), with the hit at Duke's position and no attacker
// object (stack +0x10..0x1c zero). Armour, health, death, pain sound, rumble
// and the god-mode, invincibility and Bio Mask gates are the original's; the
// entry hooks (steroids, Bio Mask, this flash) see it as a real hit.
static unsigned damage_test_hits;
static void damage_test_hurt(CPUState* cpu) {
    int type=0,amount=0;
    if(!screen_flash_take_hurt(type,amount) || !player_identity_ready() || psx_mod_read_half(0x800bcbb0)!=1) return;
    CPUState copy=*cpu;
    const uint32_t top=copy.gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    uint8_t stack[4096],scratch[1024];
    for(unsigned i=0;i<sizeof stack;++i)stack[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i)scratch[i]=psx_mod_read_byte(0x1f800000+i);
    copy.pc=0;copy.gpr[29]=top-256;copy.gpr[31]=0x800000fc;
    copy.gpr[4]=player;copy.gpr[5]=player+4;copy.gpr[6]=(uint32_t)amount;copy.gpr[7]=(uint32_t)type;
    for(unsigned i=0;i<4;++i)psx_mod_write_word(copy.gpr[29]+0x10+4*i,0);
    guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&copy,0x800a40a8,0x800000fc);psx_mod_host_call_end();guest_busy=false;
    for(unsigned i=0;i<sizeof stack;++i)psx_mod_write_byte(top-4096+i,stack[i]);
    for(unsigned i=0;i<sizeof scratch;++i)psx_mod_write_byte(0x1f800000+i,scratch[i]);
    ++damage_test_hits;
    damage_resolve();
}
// D39E: frozen blue vision on the present tint (was D35J5's half-blend PS1
// quads, first person only; those stay Classic's until D39G). While Duke is
// frozen (frozen_eye_state: alive, bit 0x20000, timer +0x214 running) each
// game frame gives EDuke32's palfrom 16 0 0 24 again, a steady hold in
// either view; the thaw clears it at once. It never cuts a stronger
// flash short (a hit while frozen shows, then the blue returns under it).
static unsigned frozen_holds,thaw_predicted;
// The thaw happens inside this frame's update, after this hook: the drain at
// 0x8004157c takes (13653 x frame delta 0x800d21fc) >> 12 from +0x214 (twice
// that with the jetpack on, +0x358 bits 0 and 1) and clears the frozen bit
// once that covers what is left; a scripted player (bit 0x20000000) skips it.
// This frame's picture is then already thawed, so its blue must end here,
// not a frame later (D39E: the blue lingered while Duke moved).
static bool thaws_this_frame() {
    if(psx_mod_read_word(player)&0x20000000u) return false;
    int32_t drain=(int32_t)((13653*(int64_t)(int32_t)psx_mod_read_word(0x800d21fc))>>12);
    if((psx_mod_read_half(player+0x358)&3)==3) drain*=2;
    return drain>=(int32_t)psx_mod_read_word(player+0x214);
}
// The session log notes each freeze and thaw, with why a hold was skipped,
// so a player report can be read back from the log.
static void frozen_tint_update() {
    static bool was=false; static unsigned held_at_start=0,skipped=0; static const char* why="";
    const uint32_t flags=psx_mod_read_word(player);
    const bool frozen=frozen_eye_state();
    const bool flagged=(flags&0x20000)!=0;
    // Thawed (or dead): the frozen blue ends at once, as in EDuke32. Only our
    // own hold is cleared; any other flash keeps fading.
    static bool held=false;
    if(held && !frozen) {
        const FlashState now=screen_flash_read();
        if(now.r==damage_frozen.r && now.g==damage_frozen.g && now.b==damage_frozen.b &&
           now.level<=(uint32_t)damage_frozen.intensity*flash_tic) screen_flash(0);
        held=false;
    }
    if(flagged!=was) {
        if(flagged) {
            std::fprintf(stderr,"[TTK flash] frozen: flags %08x timer %d health %d option %s renderer %s modern %d play %d\n",flags,
                (int32_t)psx_mod_read_word(player+0x214),(int16_t)psx_mod_read_half(player+0x32),flash_option_name(screen_flash_option()),
                screen_flash_classic()?"classic":"polymorph",input_modernized()?1:0,psx_mod_read_half(0x800bcbb0));
            held_at_start=frozen_holds;skipped=0;why="";
        } else
            std::fprintf(stderr,"[TTK flash] thawed: %u blue holds, %u skipped (%s), cleared with the thaw frame %u\n",frozen_holds-held_at_start,skipped,why,thaw_predicted);
        was=flagged;
    }
    const char* reason=!input_modernized()?"vanilla":screen_flash_classic()?"classic":!player_identity_ready()?"identity":
        psx_mod_read_half(0x800bcbb0)!=1?"not in play":!frozen?"frozen check":nullptr;
    if(!reason && thaws_this_frame()) {
        // End our blue now, with the frame that shows Duke thawed.
        const FlashState now=screen_flash_read();
        if(held && now.r==damage_frozen.r && now.g==damage_frozen.g && now.b==damage_frozen.b &&
           now.level<=(uint32_t)damage_frozen.intensity*flash_tic) screen_flash(0);
        held=false;++thaw_predicted;return;
    }
    if(!reason) {
        const FlashState now=screen_flash_read();
        if(now.level>(uint32_t)damage_frozen.intensity*flash_tic) reason="stronger flash";
        else {screen_flash(damage_frozen.intensity,damage_frozen.r,damage_frozen.g,damage_frozen.b);++frozen_holds;held=true;return;}
    }
    if(flagged) {++skipped;why=reason;}
}
