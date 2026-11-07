// D08Z8 revision 2: the sidearm climbs without being put away (Modernized).
// User (2026-11-07): "make this really unintrusive when you mantle up. if its
// already the desert eagle, duke should just be able to continuously shoot."
//
// The original mantle is the ground obstacle action 0x80051cf0, called by the
// walk / run handlers (ra 0x80052544, 0x8005349c, 0x8005384c) while Duke walks
// into something. It needs Cross this tick (action word bit 0) or +0x224 &
// 0x241, and refuses while the upper track holds a weapon-owned pose (upper
// table 0x800c2824 bit 8: every armed pose, 5 / 16 / 20 ...). Past that it
// already knows armed climbs: with equipment 2 and a weapon whose table entry
// (0x800c4590 + 44 * weapon) has bit 1 (the Desert Eagle) it starts the low
// mantles (+0x1c4 >= -0x2ff) armed and refuses higher ones (+0x224 |= 0x20).
// So an armed Duke never mantles; E stowed first (pc_input.cpp), and held
// fire had to wait for the upper body to settle before that stow (rev 1).
//
// Now, with the climb sidearm out (climb_fire.inc) on the ground, E asks here
// first, as D08J9 asks the door finder, and the E stow waits for the answer:
// - At the obstacle action's entry, while the question is open, Duke is lent
//   to the original as it sees a holstered Duke: equipment 0, the upper track
//   on the lower animation (as a free upper body plays it), Cross this tick.
//   The original's own action then decides everything (line-up, the mantle
//   134..139 from the height, objects, ladders), and the walk handler gets its
//   own answer.
// - The loan ends at this update's fire request entry (0x80058120, ra
//   0x80041c44): the weapon (equipment 2, its upper pose) is put back. If the
//   action started a mantle, E is answered (no stow) and the shooting never
//   stops: D08J8 fires the sidearm through 134..139 (trav_fire_drive), the
//   original from 140. Not firing, the upper track takes the mantle as the
//   D08Z2 armed step-up does (mantle_upper_handoff) and the weapon pose comes
//   back when the climb ends.
// - Anything else (no movement input, a ladder ahead or below, two refused
//   obstacle actions, or no contact within 0.4 s of a tap / 0.67 s held) answers "no":
//   E stows and goes on as before (revision 1 then covers held fire).
// No savestate is taken or loaded while the loan is open.
static std::atomic<int> sidearm_request{0};   // 0 idle, 1 asked, 2 mantled, 3 no mantle
static unsigned sidearm_asks,sidearm_mantles,sidearm_misses,sidearm_loans,sidearm_handoffs,sidearm_expiries;
static unsigned sidearm_reaches,sidearm_reach_hangs;
static unsigned sidearm_loan_repairs;   // non-mantle loans whose upper followed the legs
// sidearm_climb_live (ledge_reach.inc): the armed climb in progress.
static unsigned sidearm_wait;                 // player updates since the question
static bool sidearm_loan;
static uint8_t sidearm_loan_equip;
static uint16_t sidearm_loan_upper,sidearm_loan_lower;
// Stuck-weapon fix (2026-11-07): the weapon's whole upper block (+0x74, 0x14
// bytes). Lent as holstered, a free upper body follows the legs: when the
// walk handler changed the lower clip inside the loan (walking into a wall,
// 74 -> 63), the upper copied it, the id no longer matched and the old close
// put equipment 2 back over an upper on the legs' clip. That upper then
// followed the legs for good: no weapon state handler, no shot, no switch.
static uint32_t sidearm_loan_block[0x14/4];
static uint32_t sidearm_loan_word,sidearm_loan_pad;
static constexpr unsigned sidearm_tap_updates=12,sidearm_hold_updates=20;   // 0.4 s / 0.67 s
static unsigned sidearm_refusals;   // obstacle actions in a row that did not mantle
static bool sidearm_owner() {
    return door_owner() && base_weapon(psx_mod_read_byte(player+0x3b9))==climb_sidearm &&
        psx_mod_read_byte(player+0x27d)==0 && !(psx_mod_read_word(player+0x224)&0x241u) &&
        psx_mod_read_half(player+0x74)<280;
}
// Input thread.
bool sidearm_mantle_ready() {return sidearm_request.load()==0 && sidearm_owner();}
void sidearm_mantle_ask() {sidearm_request.store(1);++sidearm_asks;}
int sidearm_mantle_state() {return sidearm_request.load();}
void sidearm_mantle_end() {int s=sidearm_request.load();if(s!=1)sidearm_request.store(0);}
void sidearm_mantle_cancel() {sidearm_request.store(0);}
static bool sidearm_climb(unsigned a) {return a>=134 && a<=139;}
// Fire request entry (ra 0x80041c44), and the next player update as a backstop.
static void sidearm_loan_close() {
    if(!sidearm_loan)return;
    sidearm_loan=false;psx_mod_host_call_end();
    if(sidearm_loan_word)psx_mod_write_word(sidearm_loan_word,(psx_mod_read_word(sidearm_loan_word)&~1u)|(sidearm_loan_pad&1u));
    const unsigned lower=psx_mod_read_half(player+0x60);
    const bool climbed=sidearm_climb(lower) && lower!=sidearm_loan_lower;
    psx_mod_write_byte(player+0x3b8,sidearm_loan_equip);
    // A mantle: the upper id only (the original never touched the upper block
    // when it started one). Anything else: the weapon's whole block, as lent.
    if(climbed) {
        if(psx_mod_read_half(player+0x74)==sidearm_loan_lower)psx_mod_write_half(player+0x74,sidearm_loan_upper);
    } else {
        if(psx_mod_read_half(player+0x74)!=sidearm_loan_lower)++sidearm_loan_repairs;
        for(unsigned i=0;i<std::size(sidearm_loan_block);++i)psx_mod_write_word(player+0x74+4*i,sidearm_loan_block[i]);
    }
    if(!climbed) {
        // Walking into something the original would not climb (a wall, a
        // switch): two refusals in a row answer "no", so E goes on at once.
        if(sidearm_request.load()==1 && ++sidearm_refusals>=2) {sidearm_request.store(3);++sidearm_misses;}
        return;
    }
    sidearm_refusals=0;
    ++sidearm_mantles;sidearm_request.store(2);
    // This climb's sidearm, as if D08J8 had drawn it: an armed 139 that ends
    // in the ledge hang pulls up one-handed (trav_hang, D08J7C) as W / E ask.
    trav_drawn=true;sidearm_climb_live=true;
    const auto& f=input_snapshot(Context::Gameplay);
    if(!(f.active && f.held[fire])) {mantle_upper_handoff(lower);++sidearm_handoffs;}
}
// 0x80051cf0 entry from the walk / run handlers.
static void sidearm_obstacle_action(CPUState* cpu,uint32_t ra) {
    sidearm_loan_close();
    if(sidearm_request.load()!=1 || cpu->gpr[4]!=player ||
       (ra!=0x80052544 && ra!=0x8005349c && ra!=0x8005384c) || !sidearm_owner())return;
    // A ladder or other mount in contact needs the weapon away: the usual E.
    const uint32_t contact=psx_mod_read_word(player+0x174);
    if(contact && (object_type_flags(contact)&0x700u)) {sidearm_request.store(3);++sidearm_misses;return;}
    const uint32_t word=door_action_word();
    if(!word)return;
    sidearm_loan_equip=psx_mod_read_byte(player+0x3b8);
    sidearm_loan_upper=psx_mod_read_half(player+0x74);
    for(unsigned i=0;i<std::size(sidearm_loan_block);++i)sidearm_loan_block[i]=psx_mod_read_word(player+0x74+4*i);
    sidearm_loan_lower=psx_mod_read_half(player+0x60);
    sidearm_loan_word=word;sidearm_loan_pad=psx_mod_read_word(word);
    psx_mod_write_byte(player+0x3b8,0);
    psx_mod_write_half(player+0x74,sidearm_loan_lower);
    psx_mod_write_word(word,sidearm_loan_pad|1u);
    sidearm_loan=true;++sidearm_loans;psx_mod_host_call_begin();
}
// Once per player update (0x8005a210 entry): the question's time.
static void sidearm_swap_update();
static void weapon_loop_stop(CPUState* cpu) {
    const unsigned from=weapon_loop_stop_from;
    if(!from)return;
    weapon_loop_stop_from=0;
    const uint32_t handle=psx_mod_read_word(player+0x260);
    if(!handle || !player_identity_ready())return;
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x800u);
    psx_mod_write_word(player+0x81c,0);
    original_call(cpu,0x8006af44,0,nullptr,-1,handle);
    psx_mod_write_word(player+0x260,0);
    if(base_weapon(from)==7)original_call(cpu,0x8006b270,player+4,nullptr,0x800,0x1011);   // Gatling spin-down
    ++weapon_loop_stops;
}
static void sidearm_mantle_update(CPUState* cpu) {
    sidearm_loan_close();
    sidearm_swap_update();
    weapon_loop_stop(cpu);
    if(sidearm_climb_live) {
        const unsigned mode=psx_mod_read_byte(player+0x22c);
        const unsigned a=psx_mod_read_half(player+0x60);
        if(hang_weapon_ready()) {trav_hang=true;sidearm_climb_live=false;++sidearm_reach_hangs;}
        else if(mode!=9 && mode!=6 && mode!=8 && !(a>=134 && a<=140)) sidearm_climb_live=false;
    }
    if(sidearm_request.load()!=1) {sidearm_wait=0;sidearm_refusals=0;return;}
    // A ladder to leap for or to climb down needs the weapon away: the usual E.
    if(ladder_top_available() || ladder_leap_geometry()) {sidearm_request.store(3);++sidearm_misses;sidearm_wait=0;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    const bool held=f.active && f.held[interact];
    // The original climbs only what Duke walks into: standing still, E stows at once.
    const bool moving=f.active && (f.move_x!=0 || f.move_y!=0);
    if(!sidearm_owner() || !moving || ++sidearm_wait>(held?sidearm_hold_updates:sidearm_tap_updates)) {
        sidearm_request.store(3);++sidearm_expiries;sidearm_wait=0;
    }
}
// D08Z8: a weapon out that can climb as the sidearm (the sidearm itself, or
// another weapon with sidearm ammo: revision 5 swaps on a catch), while Duke
// takes off or flies (mode 9). In a jump E then keeps fire on the pad.
static bool sidearm_climb_weapon() {
    return sidearm_drawn() || (psx_mod_read_byte(player+0x3b8)==2 && usable_weapon(climb_sidearm));
}
bool sidearm_weapon_out() {return input_modernized() && player_identity_ready() && sidearm_climb_weapon();}
bool sidearm_in_flight() {
    return input_modernized() && player_identity_ready() && sidearm_climb_weapon() &&
        (psx_mod_read_byte(player+0x22c)==9 || psx_mod_read_half(player+0x60)==96);
}
// Revision 4: jump grabs of world ledges with the sidearm firing. The reach
// 0x800557f8 (first in the airborne handler 0x80055904) switches the jump to
// the reach 109 only when Cross is held this tick, the D08X reach is armed
// (+0x224 0x800000, none of 0x241) and the upper pose lacks table bit 8, and
// from then on runs the ledge acquisition 0x80055208 every update (grab 147 /
// 148, hang 149, pull-up 140). Held fire is Cross, so with the sidearm firing
// only the weapon pose stops it. Right after this update's reach (the
// ballistic hook, ra 0x80055934, from ledge_reach_arm's site) the same
// transition is made for the armed sidearm; the original then catches (or
// not) on its own. The weapon stays out: 109, 147, 148 and 140 carry no fire
// refusal (0x2000), so the shots go on, and the hang / pull-up are D08J7's
// armed ones: reaching the hang marks it as the climb's sidearm (trav_hang),
// so W or E held pulls up one-handed (D08J7C). Without fire the E stow
// runs in flight as before and the original reach grabs unarmed.
// Revision 5 (user: "any other weapon"): with another weapon firing, a jump at
// a ledge swaps to the sidearm as D08J8 does on climbs, but only for a ledge
// that is really there: each update the original acquisition 0x80055208 is
// tried on a saved Duke (as the reach calls it from 109) and every byte is put
// back. Where it would catch, the jump enters the reach 109 exactly as the
// reach does, and the original's own next reach update catches; at that grab
// (147 / 148 / 149) the sidearm takes the weapon's place (climb_swap_from,
// SIDEARM RETURN as on every climb) and is drawn in the hand (upper 6, the
// D08J8 draw). No catch: the weapon stays; a reach that lands empty swaps
// nothing.
static unsigned sidearm_swap_catches,sidearm_swap_probes,sidearm_swap_reaches;
static bool sidearm_swap_pending;
static uint32_t sidearm_probe_saved[0x8a4/4];
static void sidearm_swap_reach(CPUState* cpu) {
    if(sidearm_swap_pending || !usable_weapon(climb_sidearm) ||
       base_weapon(psx_mod_read_byte(player+0x3b9))==climb_sidearm)return;
    for(unsigned i=0;i<0x8a4/4;++i)sidearm_probe_saved[i]=psx_mod_read_word(player+4*i);
    const uint32_t flags=psx_mod_read_word(player+0x224);
    psx_mod_write_half(player+0x60,109);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    psx_mod_write_word(player+0x224,flags&~reach_armed);
    int32_t caught=0;++sidearm_swap_probes;
    const bool hit=original_call(cpu,0x80055208,0,&caught) && (caught&0xffff);
    for(unsigned i=0;i<0x8a4/4;++i)
        if(psx_mod_read_word(player+4*i)!=sidearm_probe_saved[i])psx_mod_write_word(player+4*i,sidearm_probe_saved[i]);
    if(!hit)return;
    psx_mod_write_half(player+0x60,109);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    psx_mod_write_word(player+0x224,flags&~reach_armed);
    sidearm_swap_pending=true;++sidearm_swap_reaches;
}
// Once per player update: the grab that the probe found.
static void sidearm_swap_update() {
    if(!sidearm_swap_pending)return;
    const unsigned a=psx_mod_read_half(player+0x60),mode=psx_mod_read_byte(player+0x22c);
    if(a!=109 && a!=147 && a!=148 && a!=149) {sidearm_swap_pending=false;return;}   // landed, bounced
    if(a==109 || psx_mod_read_byte(player+0x3b8)!=2 || !player_identity_ready())return;
    sidearm_swap_pending=false;
    (void)mode;
    const unsigned selected=psx_mod_read_byte(player+0x3ba);
    if(base_weapon(selected)==climb_sidearm || !usable_weapon(climb_sidearm))return;
    sidearm_swap_in();++sidearm_swap_catches;
}
static void sidearm_reach(CPUState* cpu,const InputFrame& f) {
    if(!input_modernized() || !f.active || !f.held[fire] || f.held[original_aim] ||
       psx_mod_read_byte(player+0x3b8)!=2 || !input_airborne_reach_held() || !player_identity_ready())return;
    const unsigned animation=psx_mod_read_half(player+0x60),upper=psx_mod_read_half(player+0x74);
    if((animation!=98 && animation!=103 && animation!=104) || upper>=280 ||
       !(psx_mod_read_word(0x800c2824+4*upper)&8))return;   // a free upper body: the original's own reach
    const uint32_t flags=psx_mod_read_word(player+0x224);
    if((flags&(reach_armed|0x241u))!=reach_armed)return;
    const uint32_t word=door_action_word();
    if(!word || !(psx_mod_read_word(word)&1))return;         // Cross this tick, as the reach needs
    if(!sidearm_drawn()) {sidearm_swap_reach(cpu);return;}
    psx_mod_write_half(player+0x60,109);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    psx_mod_write_word(player+0x224,flags&~reach_armed);
    sidearm_climb_live=true;++sidearm_reaches;
}
static const char* sidearm_mantle_debug_json() {
    static char buffer[320];
    std::snprintf(buffer,sizeof buffer,"{\"state\":%d,\"asks\":%u,\"loans\":%u,\"mantles\":%u,\"handoffs\":%u,\"misses\":%u,\"expiries\":%u,\"air_armed\":%u,\"reaches\":%u,\"reach_hangs\":%u,\"swap_probes\":%u,\"swap_reaches\":%u,\"swap_catches\":%u,\"loop_stops\":%u,\"loan_repairs\":%u}",
        sidearm_request.load(),sidearm_asks,sidearm_loans,sidearm_mantles,sidearm_handoffs,sidearm_misses,sidearm_expiries,armed_air_mantles,sidearm_reaches,sidearm_reach_hangs,sidearm_swap_probes,sidearm_swap_reaches,sidearm_swap_catches,weapon_loop_stops,sidearm_loan_repairs);
    return buffer;
}
