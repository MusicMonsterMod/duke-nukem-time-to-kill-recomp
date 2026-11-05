// D08J8C: doors never stop the shooting. Included in the guarded controls
// adapter after climb_fire.inc. Armed Cross at a kick-open door (kind 20,
// user UI slot 11) starts the native use action: the stage machine
// 0x800513e4 (+0x27d) installs pose 116 (clip 600, keys 60/50/50/50/50/50)
// with the committed-action bit 0x40, and the door opens when the strike key
// (key 3, event 0x800) ends. Until the pose ends (about 0.95 s) the fire
// request (0x80058120) is refused: 116 carries upper-table bit 0x2000 and
// +0x224 & 0x241 holds 0x40. Measured: 13 rounds held from the press
// started 1.24 s after it.
// Revision 3 (user: "i want the door to open in the normal way, but duke is
// just able to still shoot"): the kick plays at the original speed to its
// end; with a weapon out the host owns the fire through it (weapon_feel.inc,
// door_fire_owned: the weapon's own fire track on the gun arm, as in the
// mantles). The revision 1 6x kick and early end are gone. Holstered kicks
// and every other use action stay original; Vanilla unchanged. Revision 2
// below: E, not left click, opens doors.
static unsigned door_kicks;
static bool armed_door_kick() {
    return input_modernized() && gameplay_context() && psx_mod_read_half(player+0x60)==116 &&
        psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 &&
        psx_mod_read_byte(player+0x3b8)==2 && psx_mod_read_byte(player+0x27d)==5 &&
        (psx_mod_read_word(player+0x224)&0x40) && !(psx_mod_read_word(player)&0x20002) && player_identity_ready();
}
// Revision 2 (user, after the first build): left click should never open a
// door, E should, and Duke should be able to shoot straight away after E.
// Modernized E with a weapon out holsters first (an armed Cross with nothing
// to use would fire), so the door went holstered: about 0.94 s of kick, then
// the draw. Only the game's own finder knows whether a door is there, so:
// - E, armed on the ground: a probe sends armed Cross for up to
//   door_probe_limit player updates. If the finder starts a use (stage
//   +0x27d set) on a door, the armed kick above runs (no holster). If the
//   game instead raised a fire request (+0x224 0x100) the request and the
//   upper block (+0x74, 0x14 bytes, kept at the probe's first update) are put
//   back before any round can leave (the raise 15 alone takes about three
//   updates), and the original E (holster, then use) follows; also when
//   another object's use started, which is ended at its first stage.
// - E while fire is held: the finder needs a fresh Cross, so the probe
//   first releases Cross for one player update (state 4), then presses it;
//   held fire keeps its shots (no request is put back), and only a door use
//   is kept.
// - Fire, armed, with no E probe: a door use is ended at its first stage
//   (before stage 4's use callback), so the round leaves instead.
// A door is an object whose type metadata selects the kick pose: kind (type
// table 0x800d2660, 28-byte records, byte 4) whose jump-table entry
// (0x80015850, kinds 2-18) is 0x800515e0 (kind 5, pose 116; note 237).
static constexpr unsigned door_probe_limit=4;
static std::atomic<int> door_probe{0};   // 0 idle, 1 probing, 2 armed door use, 3 refused, 4 Cross gap
static bool door_probe_firing;
static std::atomic<unsigned> door_probe_starts{0};
static unsigned door_probe_expiries,door_probe_updates,door_probe_uses,door_probe_reverts,door_probe_refusals,door_fire_cancels;
static uint32_t door_probe_block[0x14/4];
static bool door_probe_saved;
static unsigned door_probe_residue;   // updates a revert keeps being applied
static uint64_t door_probe_loads;
static bool door_object(uint32_t object) {
    if(object<0x80010000u || object>=0x801fffc0u)return false;
    const uint32_t table=psx_mod_read_word(0x800d2660);
    if(table<0x80010000u || table>=0x80200000u)return false;
    const unsigned type=psx_mod_read_half(object+0x2c);
    if(type>=4096)return false;
    const unsigned kind=psx_mod_read_byte(table+28*type+4);
    return kind>=2 && kind<2+0x11 && psx_mod_read_word(0x80015850+4*(kind-2))==0x800515e0u;
}
static bool door_owner() {
    return input_modernized() && gameplay_context() && psx_mod_read_byte(player+0x22c)==0 &&
        psx_mod_read_byte(player+0x22d)==0 && psx_mod_read_byte(player+0x3b8)==2 &&
        !(psx_mod_read_word(player)&0x20002) && player_identity_ready();
}
// Only at a door Duke is touching (+0x174, the contact object: the door
// while he stands at it, 0 at crates, walls and ledges; measured), so E
// elsewhere keeps the original stow with no probe.
static bool door_touched() {return door_object(psx_mod_read_word(player+0x174));}
// Input thread, where E would stow: armed on the ground, idle (the E stow's
// own readiness), at a door, no use running.
bool door_probe_ready() {
    return door_probe.load()==0 && interaction_holster_ready() && door_owner() && door_touched() && psx_mod_read_byte(player+0x27d)==0;
}
// Input thread, E pressed while fire is held.
bool door_probe_firing_ready() {return door_probe.load()==0 && door_owner() && door_touched() && psx_mod_read_byte(player+0x27d)==0;}
void door_probe_begin(bool firing) {door_probe_saved=false;door_probe_firing=firing;door_probe.store(firing?4:1);++door_probe_starts;}
int door_probe_state() {return door_probe.load();}
void door_probe_end() {int s=door_probe.load();if(s==3)door_probe.store(0);}
static void door_use_cancel() {
    psx_mod_write_byte(player+0x27d,0);psx_mod_write_byte(player+0x27c,0);psx_mod_write_byte(player+0x27e,0);
    psx_mod_write_word(player+0x290,0);
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x40u);
}
static void door_probe_revert() {
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x100u);
    if(psx_mod_savestate_loads()==door_probe_loads)
        for(unsigned i=0;i<std::size(door_probe_block);++i)psx_mod_write_word(player+0x74+4*i,door_probe_block[i]);
}
static void door_probe_update() {
    // The pad the game already sampled may still carry the probe's Cross.
    if(door_probe_residue) {
        --door_probe_residue;
        if((psx_mod_read_word(player+0x224)&0x100) && !input_snapshot(Context::Gameplay).held[fire] && door_owner())door_probe_revert();
    }
    const int state=door_probe.load();
    if(state==2) {if(psx_mod_read_byte(player+0x27d)==0 || !door_owner())door_probe.store(0);return;}
    if(state==4) {door_probe_updates=0;door_probe.store(door_owner()?1:3);return;}
    if(state!=1)return;
    if(!door_owner()) {door_probe.store(3);return;}
    if(!door_probe_saved) {
        for(unsigned i=0;i<std::size(door_probe_block);++i)door_probe_block[i]=psx_mod_read_word(player+0x74+4*i);
        door_probe_saved=true;door_probe_updates=0;door_probe_loads=psx_mod_savestate_loads();
    }
    const unsigned stage=psx_mod_read_byte(player+0x27d);
    const uint32_t flags=psx_mod_read_word(player+0x224);
    if(stage) {
        if(door_object(psx_mod_read_word(player+0x290))) {door_probe.store(2);++door_probe_uses;return;}
        door_use_cancel();door_probe.store(3);++door_probe_refusals;return;
    }
    if((flags&0x100) && !door_probe_firing) {
        door_probe_revert();door_probe_residue=2;
        door_probe.store(3);++door_probe_reverts;return;
    }
    if(++door_probe_updates>door_probe_limit) {door_probe.store(3);++door_probe_expiries;}
}
// Fire (not E) started a door use: end it at its first stage.
static void door_fire_cancel() {
    const unsigned stage=psx_mod_read_byte(player+0x27d);
    if(door_probe.load()!=0 || stage<1 || stage>4 || !(psx_mod_read_word(player+0x224)&0x40) || !door_owner())return;
    const auto& f=input_snapshot(Context::Gameplay);
    if(!f.held[fire] || f.held[interact] || !door_object(psx_mod_read_word(player+0x290)))return;
    door_use_cancel();++door_fire_cancels;
}
// Once per player update: the E probe and the fire cancel.
static bool door_kick_seen;
static void door_fire_update() {
    door_probe_update();
    door_fire_cancel();
    const bool kick=armed_door_kick();
    if(kick && !door_kick_seen)++door_kicks;
    door_kick_seen=kick;
}
static const char* door_fire_debug_json() {
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"kicks\":%u,\"probe\":%d,\"probe_starts\":%u,\"probe_expiries\":%u,\"probe_uses\":%u,\"probe_reverts\":%u,\"probe_refusals\":%u,\"fire_cancels\":%u}",
        door_kicks,door_probe.load(),door_probe_starts.load(),door_probe_expiries,door_probe_uses,door_probe_reverts,door_probe_refusals,door_fire_cancels);
    return buffer;
}
