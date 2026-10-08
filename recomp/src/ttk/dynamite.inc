// D08A12 modern dynamite (DNTTK_DYNAMITE=modern, the Modernized default; the
// launcher passes original for Vanilla). documentation/138-d08a12-modern-dynamite.md.
//
// Original: drawing the dynamite lights it. The draw completion (anim 40 end,
// 0x8004e39c, table 0x80014d20, case 0x8004e5ac) sets equipment 2 / weapon 14
// and the fuse +0x250 = 4000 (half 0x800c4ed8). The held-throwable handler
// 0x8004dea4 (anim 39 and two other upper-body states; table 0x80014ce8) runs
// case 0x8004e018 for it: the fuse drops by about 3.33 x the frame delta
// (0x800d21fc); at 0 it explodes in Duke's hand (0x80070f7c, ammo -1, empty
// hand); otherwise sparks 0x8006d594(6,pos,0xff), the sizzle loop
// 0x8006b270(0x101d) into +0x260 and the throw check 0x8004dc74 (Cross: charge
// +0x248/+0x24c up to 700, release or full charge throws). Unlike the other
// throwables (0x8004e180/0x8004e1d4) it never looks at a weapon request
// (+0x224 bit 2) or the holster button (0x800d1a90), so a selected stick has
// to be thrown or explode. The throw (0x8004e6a0) hands the remaining fuse to
// the thrown object.
//
// Modern: a drawn stick is unlit. At the handler's entry the fuse is held full
// and this call's sparks (brightness 0) and sizzle (sound id -1, which
// 0x8006b270 refuses, so +0x260 stays 0) are kept quiet; a weapon request or
// the holster button stows it exactly as 0x8004e180 does for the Holy Hand
// Grenade (anim 40 backwards). Cross down where 0x8004dc74 would charge lights
// it: from then on the original runs untouched (full fuse from that press,
// sparks, sizzle, charge, throw; a hold kept past the fuse explodes in hand as
// before). Damage, radius, ammo and the throw are the original's.
static bool dynamite_modern() {
    if(!input_modernized()) return false;
    const char* v=std::getenv("DNTTK_DYNAMITE");
    return !v || std::strcmp(v,"original");
}
static bool dynamite_lit,dynamite_quiet,dynamite_seen; // dynamite_redraw(s): shortcuts.inc
static uint32_t dynamite_loads;
static unsigned dynamite_holds,dynamite_lights,dynamite_stows,dynamite_quiet_sparks,dynamite_quiet_sizzles;
static bool dynamite_in_hand() {return psx_mod_read_half(player+0x3b8)==0x0e02;}
// Any hook: the lit stick has left the hand (thrown, exploded, stowed) or a
// savestate replaced the game; the next stick drawn starts unlit. A throw
// (its end, table 0x80014d70, empties the hand during animations 41-46)
// queues the next stick, as in Duke 3D (dynamite_redraw_due).
static void dynamite_track() {
    const uint32_t loads=psx_mod_savestate_loads();
    if(loads!=dynamite_loads) {dynamite_loads=loads;dynamite_lit=dynamite_redraw=dynamite_seen=false;}
    if(dynamite_in_hand()) {dynamite_seen=true;return;}
    if(dynamite_seen) {
        const unsigned upper=psx_mod_read_half(player+0x74);
        dynamite_redraw=dynamite_modern() && psx_mod_read_byte(player+0x3b8)==0 && upper>=41 && upper<=46;
        dynamite_lit=dynamite_seen=false;
    }
}
// select_weapon: after a throw the original leaves Duke empty-handed. Draw
// the next stick through the same request as the 6 key once the hand is
// empty and no other request is in flight; another weapon, another request
// or no sticks left drop it.
static bool dynamite_redraw_due(unsigned equipment,uint32_t flags) {
    if(!dynamite_redraw) return false;
    if(!dynamite_modern() || equipment!=0 || (flags&4) || psx_mod_read_byte(player+0x3ba)!=14 ||
       !(psx_mod_read_half(player+0x2c4+4*14)&1) || (int16_t)psx_mod_read_half(player+0x2c6+4*14)<=0) {
        dynamite_redraw=false;return false;
    }
    return true;
}
static uint32_t dynamite_button(uint32_t table) {
    const uint32_t word=psx_mod_read_word(table+4*psx_mod_read_byte(player+0x233));
    return word>=0x80010000u && word<0x80200000u ? psx_mod_read_word(word) : 0;
}
// Entry of 0x8004dea4 (a0 = the player).
static void dynamite_entry(CPUState* cpu) {
    dynamite_quiet=false;
    if(cpu->gpr[4]!=player || !dynamite_modern()) return;
    dynamite_track();
    // Code identity only: a scripted camera owning the view must not light it.
    if(dynamite_lit || !dynamite_in_hand() || !identity()) return;
    const uint32_t flags=psx_mod_read_word(player+0x224);
    // 0x8004dc74's charge gates: its reset state (upper flag 0x20000 of the
    // lower animation, +0x224 bit 5), Cross held with 0x241 clear, and the
    // throw's own 0x4000.
    const unsigned lower=psx_mod_read_half(player+0x60);
    const bool reset=lower<0x200 && (psx_mod_read_word(0x800c2824+4*lower)&0x20000);
    // +0x224 bit 8 is set while a throw charge runs (observed live): a charge
    // a press already started counts as lit too.
    if((flags&0x100) || (!reset && !(flags&0x20) && (dynamite_button(0x800d1b50)&1) && !(flags&0x4241))) {
        psx_mod_write_word(player+0x250,psx_mod_read_half(0x800c4ed8));
        dynamite_lit=true;++dynamite_lights;
        return;
    }
    psx_mod_write_word(player+0x250,psx_mod_read_half(0x800c4ed8));
    dynamite_quiet=true;++dynamite_holds;
    // 0x8004e180: a request, or the holster button released with 0x4241 clear.
    if((flags&4) || ((dynamite_button(0x800d1a90)&3)==2 && !(flags&0x4241))) {
        psx_mod_write_half(player+0x74,0x28);psx_mod_write_half(player+0x7c,0);psx_mod_write_half(player+0x7e,1);
        ++dynamite_stows;
    }
}
// 0x8006d594 from case 0x8004e018 (ra 0x8004e138): the fuse sparks.
static void dynamite_sparks(CPUState* cpu) {
    if(dynamite_quiet && cpu->gpr[31]==0x8004e138u) {cpu->gpr[6]=0;++dynamite_quiet_sparks;}
}
// 0x8006b270 from case 0x8004e018 (ra 0x8004e154): the sizzle loop.
static void dynamite_sizzle(CPUState* cpu) {
    if(dynamite_quiet && cpu->gpr[31]==0x8004e154u) {cpu->gpr[4]=0xffffffffu;++dynamite_quiet_sizzles;}
}
static const char* dynamite_debug_json() {
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"modern\":%s,\"lit\":%s,\"redraw\":%s,\"redraws\":%u,\"holds\":%u,\"lights\":%u,\"stows\":%u,\"quiet_sparks\":%u,\"quiet_sizzles\":%u}",
        dynamite_modern()?"true":"false",dynamite_lit?"true":"false",dynamite_redraw?"true":"false",dynamite_redraws,dynamite_holds,dynamite_lights,dynamite_stows,dynamite_quiet_sparks,dynamite_quiet_sizzles);
    return buffer;
}
