// D08J9: doors and shooting (Modernized; replaces the D08J8C door probe and
// its host-driven fire tracks). Included in the guarded controls adapter after
// climb_fire.inc. User (UI slot 10): "we're supposed to be able to shoot
// completely normally when opening doors"; "im totally opening to rewriting
// open-door code".
//
// The original (measured and read from the executable):
// - The Action press 0x80051890 runs the finder 0x80077f28 (the object in
//   front of Duke, +0x174/+0x178) and, on a fresh Cross (the action word
//   *(0x800d1b50[+0x233]) is a shift register: bit 0 this tick, bit 1 the
//   last), with none of +0x224 & 0x241, starts a use: +0x27c mode, +0x27d
//   stage 1, +0x290 the object, +0x224 |= 0x40. With a weapon out the lower
//   idle handler still reaches it (0x80052c14), so an armed Cross at a door
//   kicks it instead of firing.
// - The selector 0x800513e4 installs the kick 116 at stage 4 -> 5 (class 5 of
//   the type table 0x800d2660, branch 0x800515e0); the door opens on 116's
//   strike key (event 0x800) and stage 6/8 completes. The weapon stays drawn
//   and its upper tracks keep running (the shotgun's pump 26 goes on to its
//   gun-up hold 23 as usual).
// - The fire request 0x80058120 (once per update, between 0x80057230 and the
//   target update 0x80058404) clears +0x224 0x100 when the lower clip's table
//   entry (0x800c2824) has bit 0x2000, as 116 does, or with 0x40 committed.
//   So the weapon could not fire through the kick.
// D08J8C worked around that: E sent armed Cross through the pad ("probe"),
// put back fire requests the probe raised, cut Cross for a gap while firing,
// and drove each weapon's fire track itself through the kick. That missed
// doors whose contact (+0x174) is only written by the finder (slot 10: E did
// nothing), skipped the shotgun's gun-up hold 23 (no shots for the whole kick)
// and replaced the weapons' own cycle (pump, rate) with the host's loop.
//
// Now:
// - E, armed on the ground: the host asks the game's own finder for the
//   object in front of Duke and, at a door, makes the original Action press
//   with a fresh Cross presented for that one call only. Nothing is sent
//   through the pad, so fire and E never share Cross. No door: E does what
//   it did (stow, then the use).
// - Through an armed door use the weapon is the original's. Its gates are
//   the committed bit 0x40 (every weapon state handler, 0x241 masks) and,
//   on the lower clip, table bits 0x20000 (the raise from idle and the other
//   weapon state handlers, 0x8004dc90..0x80051344, and the target lock) and
//   0x2000 (only the fire request, 0x80041b58/0x80041bcc/0x80058134). The
//   kick 116 has both bits (0x22441). For the animation runner of each
//   player update (0x8005a210 entry, where the weapon state handlers run, to
//   the fire request's entry 0x80058120, after the heading handler) 0x40 is
//   clear and 116's entry lacks 0x20000; both are put back there. The fire
//   request's 0x2000 refusal is answered at the next runner entry: the host
//   makes the request's own decision without the kick (+0x224 0x100 when
//   Cross is held, the upper track has table bit 8 and no 0x2000, and none
//   of 0x201 is set). The use stage machine, the door's own callback and
//   every reader outside that window see the original state. Raise, fire,
//   pump, reload, ammo and rate are the weapon's own; D10H6 pacing and
//   view-aimed rounds apply as on the ground.
// - Left click never opens a door: at a door, an armed original Action press
//   sees Cross as held since the last tick (bit 1), so it starts no use and
//   the round leaves.
// Holstered doors, every other use action and Vanilla are unchanged.
static bool door_object(uint32_t object) {
    if(object<0x80010000u || object>=0x801fffc0u || (object&3))return false;
    const uint32_t table=psx_mod_read_word(0x800d2660);
    if(table<0x80010000u || table>=0x80200000u)return false;
    const unsigned type=psx_mod_read_half(object+0x2c);
    if(type>=4096)return false;
    const unsigned kind=psx_mod_read_byte(table+28*type+4);
    return kind>=2 && kind<2+0x11 && psx_mod_read_word(0x80015850+4*(kind-2))==0x800515e0u;
}
// Armed, on the ground, Duke's own update.
static bool door_owner() {
    return input_modernized() && gameplay_context() && psx_mod_read_byte(player+0x22c)==0 &&
        psx_mod_read_byte(player+0x22d)==0 && psx_mod_read_byte(player+0x3b8)==2 &&
        !(psx_mod_read_word(player)&0x20002) && player_identity_ready();
}
// A use of a door in progress (stages 1..8), weapon out. Inside the weapon's
// window (below) 0x40 is lent out.
static bool door_window;
static bool armed_door_use() {
    const unsigned stage=psx_mod_read_byte(player+0x27d);
    return stage>=1 && stage<=8 && (door_window || (psx_mod_read_word(player+0x224)&0x40)) && door_owner() &&
        door_object(psx_mod_read_word(player+0x290));
}
static bool armed_door_kick() {return armed_door_use() && psx_mod_read_half(player+0x60)==116;}
// The action word (Cross), or 0.
static uint32_t door_action_word() {
    const uint32_t word=psx_mod_read_word(0x800d1b50+4*psx_mod_read_byte(player+0x233));
    return word<0x80010000u || word>=0x80200000u || (word&3) ? 0 : word;
}
static std::atomic<int> door_request{0};   // 0 idle, 1 asked, 2 door use started, 3 no door
static unsigned door_requests,door_uses,door_misses,door_refusals,door_kicks,door_gates,door_clicks;
// Input thread: E pressed, weapon out on the ground, no use running.
bool door_use_ready() {
    return door_request.load()==0 && door_owner() && psx_mod_read_byte(player+0x27d)==0 &&
        !(psx_mod_read_word(player+0x224)&0x241);
}
void door_use_request() {door_request.store(1);++door_requests;}
int door_use_state() {return door_request.load();}
void door_use_end() {int s=door_request.load();if(s==2 || s==3)door_request.store(0);}
// The original's own calls, with no savestate inside (the pad word is lent).
static bool door_call(CPUState* cpu,uint32_t address) {
    psx_mod_host_call_begin();
    const bool ok=hull_call(cpu,address,player,nullptr);
    psx_mod_host_call_end();
    return ok;
}
// Player update (0x8005a210 entry): answer E.
static void door_use_answer(CPUState* cpu) {
    if(door_request.load()!=1)return;
    if(!door_owner() || psx_mod_read_byte(player+0x27d) || (psx_mod_read_word(player+0x224)&0x241)) {door_request.store(3);return;}
    const uint32_t contact=psx_mod_read_word(player+0x174);
    const uint16_t contact_side=psx_mod_read_half(player+0x178);
    if(!door_call(cpu,0x80077f28) || !door_object(psx_mod_read_word(player+0x174))) {
        psx_mod_write_word(player+0x174,contact);psx_mod_write_half(player+0x178,contact_side);
        ++door_misses;door_request.store(3);return;
    }
    const uint32_t object=psx_mod_read_word(player+0x174);
    const uint32_t word=door_action_word();
    if(!word) {door_request.store(3);return;}
    const uint32_t pad=psx_mod_read_word(word);
    psx_mod_write_word(word,(pad|1u)&~2u);
    const bool called=door_call(cpu,0x80051890);
    psx_mod_write_word(word,pad);
    if(called && psx_mod_read_byte(player+0x27d)==1 && psx_mod_read_word(player+0x290)==object) {++door_uses;door_request.store(2);return;}
    // Facing or the door's own answer refused it (the original then sets 0x20).
    ++door_refusals;door_request.store(3);
}
// 0x80051890 entry from the lower idle handlers (ra 0x800467fc, 0x80052c1c):
// left click at a door, weapon out, is a shot.
static void door_click_action(CPUState* cpu,uint32_t ra) {
    if(cpu->gpr[4]!=player || (ra!=0x800467fc && ra!=0x80052c1c) || !door_owner())return;
    const uint32_t word=door_action_word();
    if(!word)return;
    const uint32_t pad=psx_mod_read_word(word);
    if((pad&3)!=1)return;
    const uint32_t contact=psx_mod_read_word(player+0x174);
    const uint16_t contact_side=psx_mod_read_half(player+0x178);
    const bool found=door_call(cpu,0x80077f28) && door_object(psx_mod_read_word(player+0x174));
    psx_mod_write_word(player+0x174,contact);psx_mod_write_half(player+0x178,contact_side);
    if(!found)return;
    psx_mod_write_word(word,pad|2u);++door_clicks;
}
// The fire request 0x80058120 through an armed door use, without the kick.
static void door_fire_request() {
    if(!armed_door_use())return;
    const uint32_t word=door_action_word();
    const uint32_t flags=psx_mod_read_word(player+0x224);
    const unsigned upper=psx_mod_read_half(player+0x74);
    if(!word || !(psx_mod_read_word(word)&1) || (flags&0x301) || upper>=280)return;
    const uint32_t table=psx_mod_read_word(0x800c2824+4*upper);
    if((table&0x2000) || !(table&8))return;
    psx_mod_write_word(player+0x224,flags|0x100);++door_gates;
}
// The weapon's window through the runner (see above).
static constexpr uint32_t door_kick_entry=0x800c2824+4*116, door_kick_flags=0x22441;
// No savestate is taken or loaded while it is open (psx_mod_host_call_begin
// holds both), so a saved game never keeps the lent state.
static unsigned door_windows;
static void door_window_close() {
    if(!door_window)return;
    door_window=false;psx_mod_host_call_end();
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|0x40u);
    if(psx_mod_read_word(door_kick_entry)==(door_kick_flags&~0x20000u))psx_mod_write_word(door_kick_entry,door_kick_flags);
}
static void door_window_open() {
    // 0x200000 takes the update past the fire request (0x80041b40), where the
    // window would close.
    if(door_window || !armed_door_use() || (psx_mod_read_word(player+0x224)&0x200000))return;
    if(psx_mod_read_word(door_kick_entry)!=door_kick_flags)return;
    psx_mod_write_word(door_kick_entry,door_kick_flags&~0x20000u);
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x40u);
    door_window=true;++door_windows;psx_mod_host_call_begin();
}
// Fire request entry (0x80058120, ra 0x80041c44).
static void door_fire_window_end() {door_window_close();}
// Once per player update (0x8005a210 entry, ra 0x80041b34).
static bool door_kick_seen;
static void door_use_update(CPUState* cpu) {
    door_window_close();
    door_use_answer(cpu);
    door_fire_request();
    door_window_open();
    const bool kick=armed_door_kick();
    if(kick && !door_kick_seen)++door_kicks;
    door_kick_seen=kick;
}
static const char* door_use_debug_json() {
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"request\":%d,\"requests\":%u,\"uses\":%u,\"misses\":%u,\"refusals\":%u,\"kicks\":%u,\"requests_fire\":%u,\"windows\":%u,\"clicks\":%u,\"armed_use\":%s}",
        door_request.load(),door_requests,door_uses,door_misses,door_refusals,door_kicks,door_gates,door_windows,door_clicks,armed_door_use()?"true":"false");
    return buffer;
}
