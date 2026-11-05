// D10H4 WEAPON FEEL (Modernized, DNTTK_WEAPON_FEEL): RETRO (the default)
// gives the guns with a Duke Nukem 3D analog a close simulation of EDuke32's
// press response (named RETRO, not EDuke32, until it matches: user,
// 2026-11-05); MODERN keeps the original timing. Both parts serve one press's first shot:
//  - Latch: the press reaches the game for at least two player updates. The
//    game samples the pad once per update (about 30 Hz), so a click shorter
//    than that can fall between two samples; the original's own fire request
//    (+0x224 0x100) then holds it until the fire track's shot test.
//  - Earlier shot: until that press's round leaves (0x8003c500 from the gun
//    call, weapon_aim.cpp), the raise runs 4x and the fire track 2x. Desert
//    Eagle 15 / 16, Combat Shotgun 24 / 25, Gatling and Freezer 33 / 34, RPG
//    37 (no raise). Measured from rest at 180 Hz the original takes about
//    200 / 350 / 285 / 190 ms; EDuke32 58 / 123 / 88 / 108 (note 389).
// D10H6 held fire: the original repeats whenever the looping fire track
// passes its fire-event keyframe (flag 8), at about 11 units per update, so
// the Desert Eagle's 70-unit loop fired after 6 or 7 updates (200 / 233 ms)
// in an uneven pattern. EDuke32 fires every 5 tics. The upper delta is the
// game clock (0x800d21fc: 10 per two-field update, 15 per three-field one,
// 300 per second) times Duke's animation rate (+0xb0). While fire is held
// after a shot, the fire track is now placed on the game clock instead: at
// the share of its loop that the time since the shot is of EDuke32's period,
// reaching the fire event exactly when the period is up (overshoot carries
// to the next shot). Desert Eagle 5 tics (6/s), Gatling 3 (10/s), Freezer 4
// (7.5/s); an EDuke32 tic is 10 clock units. Running or swimming, the Desert Eagle's
// fire track is 10 (weapon_feel_fire_on), paced the same way.
// The shotgun (pump) and RPG already repeat faster than Duke 3D: native.
// Ammo, damage, spread, the TTK-only weapons (Crossbow, Buffalo Rifle,
// Flamethrower), the Energy Weapon and throws stay original.
static std::atomic<uint64_t> feel_press_serial{0}, feel_press_seen{0}, feel_updates{0}, feel_latch_until{0};
static uint64_t feel_presses, feel_latches, feel_boosted_shots, feel_shots, feel_boost_updates, feel_expired;
static bool feel_pending;          // a latched press whose round has not left yet
static unsigned feel_pending_updates;
static uint64_t feel_last_shot=0;  // player update of the last gun call
static unsigned feel_last_weapon;
static uint64_t feel_cadence_updates;
static unsigned feel_last_gap;     // updates between the last two shots
static bool feel_paced;            // the last cadence update reached its shot
static int feel_clock;             // game clock since the last held shot
static int feel_last_clock;        // clock units between the last two paced shots
static int feel_step;              // the clock of the last paced update
static bool weapon_feel_retro() {
    const char* v=ttk_live_env("DNTTK_WEAPON_FEEL");
    return input_modernized() && (!v || std::strcmp(v,"modern"));
}
static double weapon_feel_factor(const char* name,double fallback) {
    const char* v=ttk_live_env(name);char* end=nullptr;
    const double x=v?std::strtod(v,&end):fallback;
    return v && end!=v && std::isfinite(x) ? std::clamp(x,1.0,8.0) : fallback;
}
// Upper fire track and raise of the armed weapon (+0x3b9); 0 when it has no
// Duke 3D analog.
static void weapon_feel_tracks(unsigned weapon,unsigned& raise,unsigned& fire) {
    raise=fire=0;
    switch(weapon) {
    case 4: raise=15;fire=16;break;                  // Desert Eagle: pistol
    case 5: raise=24;fire=25;break;                  // Combat Shotgun: shotgun
    case 7: case 28: case 11: raise=33;fire=34;break; // Gatling: chaingun; Freezer: freezethrower
    case 8: case 29: fire=37;break;                  // RPG
    }
}
// The fire track playing on the upper body (+0x74), or 0. Running (forward,
// back or sideways) or swimming, the Desert Eagle fires from track 10 instead of 16: keys
// 57 (0, 20), 58 (8, 20), 59 (1, 30), the same 70-unit loop with its fire
// event on the second key.
static unsigned weapon_feel_fire_on(unsigned weapon,unsigned upper) {
    unsigned raise,fire;weapon_feel_tracks(weapon,raise,fire);
    return upper==fire || (weapon==4 && upper==10) ? upper : 0;
}
// D10H6: EDuke32's repeat in game updates (30 per second, its tics).
static unsigned weapon_feel_repeat(unsigned weapon) {
    switch(weapon) {
    case 4: return 5;                 // pistol: 6 per second
    case 7: case 28: return 3;        // chaingun: 10 per second
    case 11: return 4;                // freezethrower: 7.5 per second
    }
    return 0;
}
// The looping upper animation's keys, as the upper advancer 0x80059db0 reads
// them (variant 1): animation 0x800c2d64, key range 0x800d8328, keys
// *0x800d8324 (flags, duration; the key table lives above main RAM, at
// 0x80faf380 in the player build). The track's active key is +6 with +2 left.
struct FeelLoop {int first=0,count=0,event=-1,total=0;int duration[16]={};};
static bool weapon_feel_keys(unsigned anim,FeelLoop& loop) {
    const int index=(int16_t)psx_mod_read_half(0x800c2d64+anim*8+2);
    if(index<0 || index>=4096)return false;
    loop.first=(int16_t)psx_mod_read_half(0x800d8328+4*index);loop.count=(int16_t)psx_mod_read_half(0x800d8328+4*index+2);
    const uint32_t keys=psx_mod_read_word(0x800d8324);
    if(loop.first<0 || loop.count<1 || loop.count>16 || keys<0x80010000u || keys>0x80fffff0u || (keys&3))return false;
    int events=0;loop.total=0;
    for(int i=0;i<loop.count;++i) {
        const uint32_t key=keys+8*(uint32_t)(loop.first+i);
        if(key>0x80fffff8u)return false;
        const int duration=(int16_t)psx_mod_read_half(key+2);
        if(duration<1 || duration>1000)return false;
        loop.duration[i]=duration;loop.total+=duration;
        if(psx_mod_read_half(key)&8) {loop.event=i;++events;}
    }
    return events<=1;
}
static bool weapon_feel_loop(unsigned anim,FeelLoop& loop) {
    return weapon_feel_keys(anim,loop) && loop.event>=0;   // one fire event per loop
}
// Units the playing upper track has left up to the end of key `last` (its
// last key when -1), or -1 when it is past it.
static int weapon_feel_rest(const FeelLoop& loop,int last=-1) {
    const int active=(int16_t)psx_mod_read_half(player+0x7a)-loop.first,left=(int16_t)psx_mod_read_half(player+0x76);
    if(last<0)last=loop.count-1;
    if(active<0 || active>last || left<0 || left>loop.duration[active])return -1;
    int units=left;
    for(int i=active+1;i<=last;++i)units+=loop.duration[i];
    return units;
}
// Units from a fire track's start to the end of its fire key.
static int weapon_feel_to_event(const FeelLoop& loop) {
    int units=0;
    for(int i=0;i<=loop.event;++i)units+=loop.duration[i];
    return units;
}
// Units the track has run since the loop's fire event (0..total), or -1.
static int weapon_feel_since_event(const FeelLoop& loop) {
    const int active=(int16_t)psx_mod_read_half(player+0x7a)-loop.first,left=(int16_t)psx_mod_read_half(player+0x76);
    if(active<0 || active>=loop.count || left<0 || left>loop.duration[active])return -1;
    int units=loop.duration[active]-left;
    for(int i=(loop.event+1)%loop.count;i!=active;i=(i+1)%loop.count)units+=loop.duration[i];
    return active==loop.event && left==0 ? loop.total : units;
}
static bool weapon_feel_armed() {
    if(!weapon_feel_retro() || !player_identity_ready() || psx_mod_read_byte(player+0x3b8)!=2)return false;
    unsigned raise,fire;weapon_feel_tracks(psx_mod_read_byte(player+0x3b9),raise,fire);
    return fire!=0;
}
// Input thread: a fire press edge in captured Modernized gameplay.
void weapon_feel_press() {if(weapon_feel_retro())feel_press_serial.fetch_add(1);}
// Input thread: hold the fire pad while a press is unseen by the game or latched.
bool weapon_feel_latched_fire() {
    const uint64_t serial=feel_press_serial.load();
    return weapon_feel_retro() && (serial!=feel_press_seen.load() || feel_updates.load()<feel_latch_until.load());
}
// Gun call 0x8003c500 for the player (weapon_aim.cpp), in any aim mode.
void weapon_feel_shot() {
    ++feel_shots;
    if(feel_pending) {feel_pending=false;++feel_boosted_shots;}
    const uint64_t now=feel_updates.load();
    feel_last_gap=(unsigned)std::min<uint64_t>(now-feel_last_shot,999);
    feel_last_shot=now;feel_last_weapon=psx_mod_read_byte(player+0x3b9);
    // A paced shot keeps the overshoot of the update that crossed the period,
    // so the average rate holds; lateness from a slow update is not owed
    // (no catch-up burst). Any other shot starts the clock afresh.
    const int period=10*(int)weapon_feel_repeat(feel_last_weapon);
    feel_last_clock=feel_paced ? feel_clock : 0;
    const int over=feel_clock-period;
    feel_clock=feel_paced && period && over>=0 && over<feel_step ? over : 0;
    feel_paced=false;
    static const bool trace=[]{const char* v=std::getenv("DNTTK_AIM_TRACE");return v && !std::strcmp(v,"1");}();
    if(trace)std::fprintf(stderr,"ttk-feel-shot weapon=%u gap=%u clock=%d upper=%u\n",feel_last_weapon,feel_last_gap,feel_last_clock,psx_mod_read_half(player+0x74));
}
// D10H6 fire CPU: a shot can only fall on a game update, and at CPU SPEED 150
// the game slips to three-field (50 ms) updates under fire, so held gaps ran
// 148-199 ms (note 391). While fire is held on a RETRO weapon, and for three
// updates after, the runtime overclock (psx_cycles.c) runs at
// DNTTK_WEAPON_FEEL_CPU (200 by default; never below the player's CPU SPEED;
// run.py --fire-cpu off passes 100). Walking, running and swimming cover the
// same distance at either speed (measured; note 391).
extern "C" uint32_t g_psx_overclock_q16;
static uint64_t feel_cpu_until, feel_cpu_updates;
static bool feel_cpu_on;
static void weapon_feel_cpu(uint64_t updates,bool fire_held) {
    static const uint32_t base=g_psx_overclock_q16;   // the player's CPU SPEED
    if(fire_held && weapon_feel_armed())feel_cpu_until=updates+3;
    feel_cpu_on=updates<feel_cpu_until;
    const char* v=ttk_live_env("DNTTK_WEAPON_FEEL_CPU");
    const long percent=std::clamp(v?std::strtol(v,nullptr,10):200L,100L,400L);
    g_psx_overclock_q16=feel_cpu_on ? std::min(base,(uint32_t)(6553600ull/(uint64_t)percent)) : base;
    if(feel_cpu_on)++feel_cpu_updates;
}
// D10H6 wall bump: a run into a wall plays the stumble 94/95 (about 20
// fields; 0x800537f4..0x800538d8), whose upper pose drops the gun (tracks 9,
// 5, then the raise 15) so no shot leaves for about 500 ms. EDuke32 just
// stops. With a RETRO gun up, the bump stores the original's own stop
// instead (idle 63, as 0x800537ec..0x800539dc does); its thud (this call,
// 0x8006b270 from 0x800538c4), rumble and noise stay. Landings keep 94/95.
static uint64_t feel_bumps_stopped;
static void weapon_feel_bump(CPUState* cpu) {
    if(cpu->gpr[18]!=player+0x60 || !weapon_feel_armed())return;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(animation!=94 && animation!=95)return;
    psx_mod_write_half(player+0x60,63);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    ++feel_bumps_stopped;
}
// Authenticated player update (0x8005a210).
static void weapon_feel_update(bool fire_held) {
    const uint64_t updates=feel_updates.fetch_add(1)+1;
    weapon_feel_cpu(updates,fire_held);
    const uint64_t serial=feel_press_serial.load();
    if(serial!=feel_press_seen.load()) {
        feel_press_seen.store(serial);++feel_presses;
        if(weapon_feel_armed()) {
            feel_latch_until.store(updates+2);++feel_latches;
            feel_pending=true;feel_pending_updates=0;
        }
    }
    // A press the original refuses (pump, reload, a hit reaction, a switch)
    // stops waiting after about a second of game updates.
    if(feel_pending && (!weapon_feel_armed() || ++feel_pending_updates>30)) {feel_pending=false;++feel_expired;}
}
// Upper track (0x80059db0 from 0x8005a5a8): this update's delta (unchanged when none).
static int weapon_feel_delta(unsigned upper,int delta,bool fire_held) {
    if(!weapon_feel_armed())return delta;
    if(!(movement_ready() || airborne_shot_ready() || jetpack_input_ready() || swim_weapon_ready()))return delta;
    const unsigned weapon=psx_mod_read_byte(player+0x3b9);
    unsigned raise,fire;weapon_feel_tracks(weapon,raise,fire);
    if(feel_pending) {
        // The advancer carries into the next track in the same call, so the
        // first update on the raise runs it out and on through the fire key:
        // the round leaves on the update after the game saw the press, as
        // EDuke32's does on the tic after. The Desert Eagle's fire track may
        // be 16 or, running, 10: the later fire key of the two.
        FeelLoop track,shot;
        if(raise && upper==raise) {
            const int scaled=(int)std::lround(delta*weapon_feel_factor("DNTTK_WEAPON_FEEL_RAISE",4));
            if(!weapon_feel_keys(raise,track) || !weapon_feel_loop(fire,shot))return scaled;
            int to_event=weapon_feel_to_event(shot);
            if(weapon==4 && weapon_feel_loop(10,shot))to_event=std::max(to_event,weapon_feel_to_event(shot));
            const int rest=weapon_feel_rest(track);
            return rest<0 ? scaled : std::max(scaled,rest+to_event+1);
        }
        if(weapon_feel_fire_on(weapon,upper)) {
            const int scaled=(int)std::lround(delta*weapon_feel_factor("DNTTK_WEAPON_FEEL_FIRE",2));
            if(!weapon_feel_loop(upper,shot))return scaled;
            const int rest=weapon_feel_rest(shot,shot.event);
            return rest<0 ? scaled : std::max(scaled,rest+1);
        }
        return delta;
    }
    // D10H6: held fire after a shot of this weapon, on the game clock.
    const int period=10*(int)weapon_feel_repeat(weapon);
    const int clock=(int)psx_mod_read_word(0x800d21fc);
    const uint64_t since=feel_updates.load()-feel_last_shot;
    if(!period || !fire_held || weapon!=feel_last_weapon || since<1 || since>8 || clock<1 || clock>60)
        {feel_paced=false;return delta;}
    // The Freezer leaves its fire loop for one update of the gun-up pose (32)
    // after each shot; that time counts too.
    if(upper==32 && fire==34) {feel_clock+=clock;feel_paced=false;return delta;}
    if(!weapon_feel_fire_on(weapon,upper)) {feel_paced=false;return delta;}
    FeelLoop loop;
    if(!weapon_feel_loop(upper,loop))return delta;
    const int at=weapon_feel_since_event(loop);
    if(at<0)return delta;
    feel_clock+=clock;feel_step=clock;++feel_cadence_updates;
    feel_paced=feel_clock>=period;
    int target=feel_paced ? loop.total : loop.total*feel_clock/period;
    // The Freezer's loop ends after the key that follows the shot; finish that
    // key at once so the gun-up update and the restart fit in its 4 tics.
    const int after=loop.total-loop.duration[loop.event];
    if(fire==34 && weapon==11 && at<after)target=std::max(target,after);
    return std::max(0,target-at);
}

static const char* weapon_feel_debug_json() {
    static char buffer[480];
    std::snprintf(buffer,sizeof buffer,"{\"profile\":\"%s\",\"presses\":%llu,\"latches\":%llu,\"shots\":%llu,\"boosted_shots\":%llu,\"boost_updates\":%llu,\"expired\":%llu,\"pending\":%s,\"latched\":%s,\"cadence_updates\":%llu,\"last_gap\":%u,\"cpu_boost\":%s,\"cpu_boost_updates\":%llu,\"bumps_stopped\":%llu}",
        weapon_feel_retro()?"retro":"modern",(unsigned long long)feel_presses,(unsigned long long)feel_latches,(unsigned long long)feel_shots,
        (unsigned long long)feel_boosted_shots,(unsigned long long)feel_boost_updates,(unsigned long long)feel_expired,
        feel_pending?"true":"false",weapon_feel_latched_fire()?"true":"false",(unsigned long long)feel_cadence_updates,feel_last_gap,
        feel_cpu_on?"true":"false",(unsigned long long)feel_cpu_updates,(unsigned long long)feel_bumps_stopped);
    return buffer;
}
