// D26K: time in the backtick console is not player inactivity. While the
// console holds input (D26J: an active frame with no new input), the guest
// pad is neutral in run mode (Caps Lock or Shift as the speed modifier), so
// the native idle routine 0x80053038 counted player+0x240 down from 3000 by
// the frame step 0x800d21fc and, at zero, ran its armed expiry: the original
// holster 0x8004d4a4, then the fidgets 64-69. (In walk mode the Walk bit kept
// 0x8001e070 reporting activity and the timer at 3000 until D53B, below.)
// While the console holds, the countdown keeps the value it had when the
// console opened; a reset (activity, 3000) still applies and is kept from
// then on. Closing resumes the countdown from the kept value, so the original
// idle holster comes after the same remaining time as without the console.
// A holster or fidget already under way when the console opens finishes as
// the original's. Checked from the authenticated player update (0x8005a210
// from 0x80041b34), which runs every update, never in render replays.
// D53B: watching a security camera is not inactivity either. During the view
// Duke +0x224 has the script bit 0x20000000 (the alternate camera, D52), the
// pad is neutral and the same countdown ran out about every 10 s, so Duke
// played his waiting fidgets 64-69 and their lines while the player watched.
// The same hold applies for the whole view; leaving the camera resumes the
// original countdown from the kept value (the use press had reset it to 3000).
// The view moves the game camera off Duke, so it cannot use
// player_identity_ready() (camera +0xa4 == Duke): Modernized, game mode and
// the authenticated code identity are required instead. The input epoch
// advances every update during the view (the console keeps one), so the view
// hold is latched for the continuous view rather than per epoch. Vanilla
// keeps the original idle lines.
// D53B (2026-11-21, user: "duke moves around too so we need to make sure he
// actually snaps out of the idle state the second the user interacts"): in
// Modernized walk mode the injected Walk bit was pad activity to 0x8001e070,
// so Duke never idled at all. The idle is now owned here, with the original's
// timer, fidgets, lines and holster:
// - the pad lets go of the Walk bit after 0.5 s of no player activity with
//   Duke standing (idle_walk_release, pc_input), so the countdown runs;
// - any player activity (pc_input's activity serial: keys and buttons but
//   the Walk modifier, movement, mouse or stick look) resets it to 3000, also
//   in run mode, so looking around counts;
// - activity during a fidget 64-69 ends it at once: at the player update entry
//   standing idle 63 restarts at frame 0 the way the original starts any
//   animation (the fidget's lower-body path never reaches 0x800493a4, where
//   kick_convert works), and the same update dispatches the input. A line
//   already playing finishes (user choice);
// - the original idle holster stays; the next activity draws the weapon it
//   put away (+0x3ba) at once, through the original weapon request (+0x224
//   bit 4) at the 2x draw rate (user: "keep the holster but redraw at once");
//   with no activity the weapon comes back by itself after the fidget (user:
//   "he should always reholster the weapon if he lowers it"), and the fidget
//   follows the holster after about a second instead of a whole idle period.
// IDLE ANIMATIONS (D53B part three; DNTTK_IDLE_ANIMATIONS): ON is the above in
// third person; in first person, where a fidget cannot be seen and a lowered
// weapon only looks wrong (user, 2026-11-21), ON plays as VOICE ONLY. VOICE
// ONLY keeps the native countdown full and runs the same cadence here: on the
// idle beat where the original plays fidget 66 (counter +0x232 == 0, every
// third beat), Duke says one of its four lines (143, 329, 330, 331, the
// lower-body event 0x80048478) through the original say 0x8006cad8, with no
// animation and no holster. OFF has no idle at all. DUKE TALK OFF (sound
// menu, as EDuke32's) makes VOICE ONLY an OFF, and plays every speech line
// silently (concurrent_music.cpp).
static bool console_idle_held, console_idle_view;
static uint64_t console_idle_epoch;
static unsigned console_idle_value, console_idle_keeps, console_idle_view_keeps;
static uint64_t idle_activity_seen;
static bool idle_holster_expect, idle_holstered, idle_draw_queued, idle_draw_boost;
static unsigned idle_weapon, idle_draw_updates, idle_expect_updates, idle_clock=3000, idle_lines;
static bool idle_fidget_seen;
static uint32_t idle_rng=0x2545f491u;
enum class IdleMode {Full,Voice,Off,FirstPerson};
static unsigned idle_resets, idle_snaps, idle_holsters, idle_draws, idle_draw_drops;
static bool idle_fidget(unsigned a) {return a>=64 && a<=69;}
// D53G: first-person idle gestures (user, 2026-11-21: "you cant actually see
// him crack his knuckles or whatever in first person ... if we could provide
// first person specific poses for the idle animations"). IDLE ANIMATIONS ON in
// first person keeps the VOICE ONLY cadence (native countdown full, no
// holster, the fidget-66 beat's line) and, on each idle beat with a weapon
// ready, plays one of these on the first-person weapon hand. A key is a pose
// over the display grip in the level frame: pitch (degrees, muzzle up), yaw
// (degrees, muzzle right), roll (degrees, top left), then x right, y down,
// z forward in view units (as weapon_slot_offset). The display arms and the
// support hand are posed from the weapon matrix, so they follow. Display
// only: the game's hand, aim, events and timers are untouched. Any player
// activity eases the gesture out over fp_idle_fade_s (the D53B snap-out).
struct FpIdleKey {double t; double v[6];};
struct FpIdleGesture {const char* name; double length; unsigned keys; FpIdleKey key[7];};
static constexpr double fp_idle_fade_s=0.1;
static const FpIdleGesture fp_idle_gestures[]={
    // Look it over: turned to show its side, brought in, turned to the other side and back.
    {"inspect",3.4,5,{{0,{0,0,0,0,0,0}},{0.6,{8,-26,32,-70,-40,-60}},{1.6,{6,-20,-18,-62,-36,-60}},
                     {2.5,{8,-24,26,-70,-40,-60}},{3.4,{0,0,0,0,0,0}}}},
    // Heft: dipped and bounced in the hand, as if feeling its weight.
    {"heft",2.0,6,{{0,{0,0,0,0,0,0}},{0.35,{-9,0,4,0,40,-10}},{0.7,{6,0,-2,0,-14,0}},
                  {1.0,{-5,0,3,0,22,-4}},{1.35,{2,0,-1,0,-4,0}},{2.0,{0,0,0,0,0,0}}}},
    // Rest: muzzle tipped up and pulled back, held a moment, then back to ready.
    {"rest",3.0,5,{{0,{0,0,0,0,0,0}},{0.6,{36,-6,-12,-30,-60,-50}},{1.5,{32,-4,-9,-28,-54,-50}},
                  {2.3,{35,-6,-11,-30,-58,-50}},{3.0,{0,0,0,0,0,0}}}},
};
static constexpr int fp_idle_gesture_count=int(sizeof fp_idle_gestures/sizeof fp_idle_gestures[0]);
// The energy weapon (display slot 10) is held toward the middle with the
// support hand on its front, so lifting the muzzle or the gun raised the
// support upper arm until its cut shoulder end showed bottom left (user,
// 2026-11-22: "can see exposed shoulder joint looking disembodied"). Its
// inspect turns the gun while dropping it a little, and its rest lowers the
// gun instead of tipping it up; the arm then runs off the bottom of the view.
// Its heft only bounces below ready. Same lengths as the shared gestures.
static const FpIdleGesture fp_idle_energy[]={
    {"inspect",3.4,5,{{0,{0,0,0,0,0,0}},{0.6,{-4,-26,32,10,20,0}},{1.6,{-2,-20,-18,8,16,0}},
                     {2.5,{-4,-24,26,10,20,0}},{3.4,{0,0,0,0,0,0}}}},
    {"heft",2.0,6,{{0,{0,0,0,0,0,0}},{0.35,{-9,0,4,0,40,-10}},{0.7,{-1,0,-2,0,8,0}},
                  {1.0,{-5,0,3,0,26,-4}},{1.35,{0,0,-1,0,6,0}},{2.0,{0,0,0,0,0,0}}}},
    {"rest",3.0,5,{{0,{0,0,0,0,0,0}},{0.6,{-20,-6,-12,10,30,-20}},{1.5,{-18,-4,-9,10,28,-20}},
                  {2.3,{-20,-6,-11,10,30,-20}},{3.0,{0,0,0,0,0,0}}}},
};
static_assert(sizeof fp_idle_energy==sizeof fp_idle_gestures,"one energy variant per gesture");
static unsigned fp_idle_slot() {
    const unsigned slot=psx_mod_read_byte(player+0x3b9);
    return slot==27?9:slot==28?7:slot==29?8:slot;
}
static int fp_idle_variant=-1, fp_idle_previous=-1;
static double fp_idle_t, fp_idle_weight;
static bool fp_idle_fading;
static unsigned fp_idle_weapon, fp_idle_starts, fp_idle_ends, fp_idle_cuts;
// Catmull-Rom through the keys, flat at both ends.
static void fp_idle_sample(int variant,double t,double* out) {
    for(int i=0;i<6;++i)out[i]=0;
    if(variant<0 || variant>=fp_idle_gesture_count)return;
    const FpIdleGesture& g=fp_idle_slot()==10 ? fp_idle_energy[variant] : fp_idle_gestures[variant];
    if(t<=0 || t>=g.length)return;
    unsigned k=0;
    while(k+2<g.keys && t>g.key[k+1].t)++k;
    const FpIdleKey& a=g.key[k];const FpIdleKey& b=g.key[k+1];
    const double h=b.t-a.t,s=std::clamp((t-a.t)/h,0.0,1.0),s2=s*s,s3=s2*s;
    for(int i=0;i<6;++i) {
        const double ma=k>0 ? (b.v[i]-g.key[k-1].v[i])/(b.t-g.key[k-1].t)*h : 0;
        const double mb=k+2<g.keys ? (g.key[k+2].v[i]-a.v[i])/(g.key[k+2].t-a.t)*h : 0;
        out[i]=(2*s3-3*s2+1)*a.v[i]+(s3-2*s2+s)*ma+(-2*s3+3*s2)*b.v[i]+(s3-s2)*mb;
    }
}
// Developer tuning only (session, never saved): the console's
// "fpidle <gesture> <seconds>" holds that pose whenever the weapon is drawn
// in first person, "fpidle <gesture>" plays one at the next update, "fpidle
// off" clears the hold. DNTTK_FP_IDLE=0 turns the gestures off (A/B).
static std::atomic<int> fp_idle_hold_variant{-1}, fp_idle_play_request{-1};
static std::atomic<double> fp_idle_hold_t{0};
static double fp_idle_raw[6];static std::atomic<bool> fp_idle_raw_on{false};
static bool fp_idle_freeze(int& variant,double& t) {
    variant=fp_idle_hold_variant.load(std::memory_order_relaxed);t=fp_idle_hold_t.load(std::memory_order_relaxed);
    return variant>=0 && variant<fp_idle_gesture_count;
}
// Per-weapon amplitude (first-person display slot, as weapon_view_slot):
// the RPG already fills the lower view at ready, so its gestures stay small;
// rest on a two-handed weapon (support slots 3, 5-11) lifts the support
// forearm into the eye, so it rises less there.
static double fp_idle_amplitude(int variant) {
    const unsigned slot=fp_idle_slot();
    if(slot==8) return 0.35;
    if(variant==2 && slot!=10 && (slot==3 || (slot>=5 && slot<=11))) return 0.6; // energy: its own lowered rest
    return 1;
}
static void fp_idle_pose_live(double* out) {
    int v;double t;
    if(fp_idle_raw_on.load(std::memory_order_relaxed)) {std::memcpy(out,fp_idle_raw,sizeof fp_idle_raw);return;}
    double k=fp_idle_weight;
    if(fp_idle_freeze(v,t)) {fp_idle_sample(v,t,out);k=1;}
    else {fp_idle_sample(fp_idle_variant,fp_idle_t,out);v=fp_idle_variant;}
    if(v<0) return;
    k*=fp_idle_amplitude(v);
    for(int i=0;i<6;++i)out[i]*=k;
}
static void fp_idle_cut() {if(fp_idle_variant>=0)++fp_idle_cuts;fp_idle_variant=-1;fp_idle_weight=0;fp_idle_fading=false;}
static void fp_idle_interrupt() {if(fp_idle_variant>=0 && !fp_idle_fading){fp_idle_fading=true;++fp_idle_ends;}}
// A first-person gesture needs a drawn weapon at its ready pose (not the
// boot, a held item or an upper action) and Duke standing on the ground.
static bool fp_idle_ready(unsigned lower,unsigned upper,unsigned equipment,bool ground) {
    static const int enabled=ttk_env_switch("DNTTK_FP_IDLE");
    if(!enabled || !input_modernized() || equipment!=2 || lower!=63 || !ground) return false;
    const unsigned w=psx_mod_read_byte(player+0x3b9);
    if(w==0 || w>=32 || (psx_mod_read_word(player+0x224)&0x2cd)) return false;
    return upper==5 || upper==20 || upper==29 || upper==35 || upper==39;
}
static void fp_idle_start() {
    unsigned pick=(idle_rng>>20)%unsigned(fp_idle_gesture_count);
    if(int(pick)==fp_idle_previous)pick=(pick+1)%unsigned(fp_idle_gesture_count);
    fp_idle_variant=fp_idle_previous=int(pick);fp_idle_t=0;fp_idle_weight=1;fp_idle_fading=false;
    fp_idle_weapon=psx_mod_read_byte(player+0x3b9);++fp_idle_starts;
}
static void fp_idle_advance(bool ready,double dt) {
    if(fp_idle_variant<0)return;
    // Another weapon, or an upper action (a shot, a reload): gone at once, so
    // a shot always leaves the aimed gun; other activity eases out.
    const unsigned upper=psx_mod_read_half(player+0x74);
    if(psx_mod_read_byte(player+0x3b8)!=2 || psx_mod_read_byte(player+0x3b9)!=fp_idle_weapon ||
       !(upper==5 || upper==20 || upper==29 || upper==35 || upper==39)) {fp_idle_cut();return;}
    if(!ready)fp_idle_interrupt();
    if(fp_idle_fading) {
        fp_idle_weight-=dt/fp_idle_fade_s;
        if(fp_idle_weight<=0)fp_idle_cut();
        return;
    }
    fp_idle_t+=dt;
    if(fp_idle_t>=fp_idle_gestures[fp_idle_variant].length) {fp_idle_variant=-1;fp_idle_weight=0;}
}
static bool idle_hold_watching() {
    return input_modernized() && gameplay_context() && identity() &&
        (psx_mod_read_word(player+0x224)&0x20000000u)!=0;
}
// Unset (a launch without run.py) is the default: ON, DUKE TALK ON.
static bool live_is(const char* name,const char* value) {const char* v=ttk_live_env(name);return v && !std::strcmp(v,value);}
bool duke_talk_muted() {return input_modernized() && live_is("DNTTK_DUKE_TALK","off");}
static IdleMode idle_mode(const InputFrame& f) {
    IdleMode m=live_is("DNTTK_IDLE_ANIMATIONS","off")?IdleMode::Off:live_is("DNTTK_IDLE_ANIMATIONS","voice")?IdleMode::Voice:IdleMode::Full;
    if(m==IdleMode::Full && f.first_person) m=IdleMode::FirstPerson; // the global VIEW: FIRST PERSON (D53G)
    if(m==IdleMode::Voice && duke_talk_muted()) m=IdleMode::Off;
    return m;
}
// The original say (0x8006cad8), on a saved stack as damage_flash.inc calls.
static bool idle_say(CPUState* cpu,unsigned line) {
    CPUState copy=*cpu;
    const uint32_t top=copy.gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    uint8_t stack[4096],scratch[1024];
    for(unsigned i=0;i<sizeof stack;++i)stack[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i)scratch[i]=psx_mod_read_byte(0x1f800000+i);
    copy.pc=0;copy.gpr[29]=top-256;copy.gpr[31]=0x800000fc;copy.gpr[4]=line;
    for(unsigned i=0;i<4;++i)psx_mod_write_word(copy.gpr[29]+0x10+4*i,0);
    guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&copy,0x8006cad8,0x800000fc);psx_mod_host_call_end();guest_busy=false;
    for(unsigned i=0;i<sizeof stack;++i)psx_mod_write_byte(top-4096+i,stack[i]);
    for(unsigned i=0;i<sizeof scratch;++i)psx_mod_write_byte(0x1f800000+i,scratch[i]);
    return copy.pc==0 && !g_psx_call_bail;
}
bool idle_walk_release() {
    if(!input_modernized() || input_idle_ms()<500) return false;
    const unsigned a=psx_mod_read_half(player+0x60);
    return psx_mod_read_byte(player+0x22c)==0 && (a==63 || idle_fidget(a));
}
static bool idle_weapon_command(const InputFrame& f) {
    for(unsigned i=0;i<f.command_count;++i) {
        const int c=f.commands[i];
        if((c>=weapon_previous && c<=weapon_last) || (c>=weapon_group_1 && c<=weapon_group_0)) return true;
    }
    return false;
}
// End a fidget the player interrupted; upper follows when it plays the same
// fidget (unarmed).
static void idle_end_fidget(unsigned lower) {
    psx_mod_write_half(player+0x60,63);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    if(psx_mod_read_half(player+0x74)==lower) {
        psx_mod_write_half(player+0x74,63);psx_mod_write_half(player+0x7c,0);psx_mod_write_half(player+0x7e,0);
    }
    ++idle_snaps;
}
static void console_idle_hold(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const bool view=idle_hold_watching();
    if(view || (input_console_holding() && player_identity_ready())) {
        fp_idle_cut();
        const unsigned timer=psx_mod_read_half(player+0x240);
        const bool same=view ? console_idle_view : (!console_idle_view && console_idle_epoch==f.epoch);
        if(console_idle_held && same && timer<console_idle_value) {
            psx_mod_write_half(player+0x240,console_idle_value);
            ++(view ? console_idle_view_keeps : console_idle_keeps);return;
        }
        console_idle_held=true;console_idle_view=view;console_idle_epoch=f.epoch;console_idle_value=timer;
        return;
    }
    console_idle_held=false;
    const uint64_t serial=input_activity_serial();
    const bool active=f.active && serial!=idle_activity_seen;
    idle_activity_seen=serial;
    if(!player_identity_ready()) {idle_holster_expect=idle_holstered=idle_draw_queued=idle_draw_boost=idle_fidget_seen=false;idle_clock=3000;fp_idle_cut();return;}
    const IdleMode mode=idle_mode(f);
    const unsigned timer=psx_mod_read_half(player+0x240),lower=psx_mod_read_half(player+0x60),
        upper=psx_mod_read_half(player+0x74),equipment=psx_mod_read_byte(player+0x3b8);
    const bool ground=psx_mod_read_byte(player+0x22c)==0;
    if(idle_draw_boost && ((equipment==2 && (upper==5 || upper==20 || upper==29 || upper==35 || upper==39)) ||
       ++idle_draw_updates>360)) idle_draw_boost=false;
    const unsigned step=psx_mod_read_word(0x800d21fc);
    // Fire (or a weapon key) cuts it in this update, before the shot's
    // first frame draws; other activity eases it out.
    if(active && (f.held[fire] || idle_weapon_command(f))) fp_idle_cut();
    fp_idle_advance(mode==IdleMode::FirstPerson && !active && fp_idle_ready(lower,upper,equipment,ground),step/300.0);
    if(const int want=fp_idle_play_request.exchange(-1);want>=0 && mode==IdleMode::FirstPerson && fp_idle_ready(lower,upper,equipment,ground)) {
        fp_idle_start();fp_idle_variant=fp_idle_previous=want;
    }
    if(active) {
        if(timer<3000) {psx_mod_write_half(player+0x240,3000);++idle_resets;}
        idle_clock=3000;idle_holster_expect=false;
        if(idle_fidget(lower) && ground) idle_end_fidget(lower);
        if(idle_holstered) {
            idle_holstered=false;idle_fidget_seen=false;
            if(!f.held[holster] && !idle_weapon_command(f)) {idle_draw_queued=true;idle_draw_boost=true;}
        }
    } else if(mode==IdleMode::Full) {
        if(equipment!=0 && timer && timer<=psx_mod_read_word(0x800d21fc))
            {idle_holster_expect=true;idle_expect_updates=0;} // the original's countdown runs out in this update
    } else {
        // VOICE ONLY / OFF / ON in first person: the native countdown never
        // runs out; the same cadence runs here while Duke stands idle.
        if(idle_fidget(lower) && ground) idle_end_fidget(lower);
        if(timer<3000) psx_mod_write_half(player+0x240,3000);
        if(mode==IdleMode::Off || lower!=63 || !ground) idle_clock=3000;
        else if(idle_clock>step) idle_clock-=step;
        else {
            idle_clock=3000;
            const unsigned beat=psx_mod_read_byte(player+0x232);
            idle_rng=idle_rng*1103515245u+12345u;
            // D53G: ON in first person, a gesture on every beat the weapon is ready.
            if(mode==IdleMode::FirstPerson && fp_idle_variant<0 && fp_idle_ready(lower,upper,equipment,ground)) fp_idle_start();
            if(beat==0 && !(mode==IdleMode::FirstPerson && duke_talk_muted())) {
                static const unsigned lines[4]={0x8f,0x149,0x14a,0x14b};
                if(idle_say(cpu,lines[(idle_rng>>16)&3]))++idle_lines;
            }
            psx_mod_write_byte(player+0x232,beat+1>=3?0:beat+1);
        }
    }
    // The idle holster (upper 6) puts the weapon away within a few updates;
    // the fidget then follows after about a second.
    if(idle_holster_expect && equipment==0) {
        idle_holster_expect=false;idle_holstered=true;idle_fidget_seen=false;
        idle_weapon=psx_mod_read_byte(player+0x3ba);++idle_holsters;
        if(psx_mod_read_half(player+0x240)>300) psx_mod_write_half(player+0x240,300);
    } else if(idle_holster_expect && ++idle_expect_updates>60) idle_holster_expect=false;
    if(idle_holstered && equipment!=0) idle_holstered=false; // drawn another way
    if(idle_holstered) {
        // The weapon comes back after the fidget, or at once when the idle is
        // not ON in third person (a view or option change).
        if(idle_fidget(lower)) idle_fidget_seen=true;
        else if(idle_fidget_seen || mode!=IdleMode::Full) {idle_holstered=false;idle_fidget_seen=false;idle_draw_queued=true;}
    }
    if(!idle_draw_queued) return;
    const uint32_t flags=psx_mod_read_word(player+0x224);
    const unsigned w=idle_weapon;
    if(equipment!=0 || (flags&4) || f.held[holster] || idle_weapon_command(f) ||
       base_weapon(w)<1 || base_weapon(w)>14 || !usable_weapon(base_weapon(w))) {
        idle_draw_queued=false;idle_draw_boost=false;++idle_draw_drops;return;
    }
    if((flags&0x2cd) || upper==6 || upper>=280 || idle_fidget(upper)) return; // the holster, the fidget or another upper action is still running
    idle_draw_queued=false;
    psx_mod_write_byte(player+0x3ba,w);
    psx_mod_write_word(player+0x224,flags|4);++selections;++idle_draws;
    idle_draw_updates=0;
}
static const char* idle_json() {
    static char buffer[640];
    std::snprintf(buffer,sizeof buffer,"{\"fp_gesture\":\"%s\",\"fp_t\":%.2f,\"fp_weight\":%.2f,\"fp_starts\":%u,\"fp_ends\":%u,\"fp_cuts\":%u,\"mode\":\"%s\",\"lines\":%u,\"clock\":%u,\"timer\":%u,\"idle_ms\":%.0f,\"walk_release\":%s,\"resets\":%u,\"snaps\":%u,\"holsters\":%u,\"draws\":%u,\"draw_drops\":%u,\"holstered\":%s,\"draw_queued\":%s,\"draw_boost\":%s,\"console_keeps\":%u,\"view_keeps\":%u}",
        fp_idle_variant>=0?fp_idle_gestures[fp_idle_variant].name:"none",fp_idle_t,fp_idle_weight,fp_idle_starts,fp_idle_ends,fp_idle_cuts,
        live_is("DNTTK_IDLE_ANIMATIONS","off")?"off":live_is("DNTTK_IDLE_ANIMATIONS","voice")?"voice":"on",idle_lines,idle_clock,
        psx_mod_read_half(player+0x240),input_idle_ms(),idle_walk_release()?"true":"false",idle_resets,idle_snaps,idle_holsters,idle_draws,idle_draw_drops,
        idle_holstered?"true":"false",idle_draw_queued?"true":"false",idle_draw_boost?"true":"false",console_idle_keeps,console_idle_view_keeps);
    return buffer;
}
bool fp_idle_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"fpidle") && std::strncmp(line,"fpidle ",7))return false;
    close=false;
    char name[32]={};double t=-1;
    const int n=line[6]?std::sscanf(line+7,"%31s %lf",name,&t):0;
    int v=-1;
    for(int i=0;i<fp_idle_gesture_count;++i) if(n>=1 && !std::strcmp(name,fp_idle_gestures[i].name))v=i;
    if(n>=1 && !std::strcmp(name,"pose")) {
        double r[6];
        if(std::sscanf(line+12,"%lf %lf %lf %lf %lf %lf",&r[0],&r[1],&r[2],&r[3],&r[4],&r[5])!=6) {say("Usage: fpidle pose pitch yaw roll x y z");return true;}
        std::memcpy(fp_idle_raw,r,sizeof r);fp_idle_raw_on.store(true);say("Holding a raw first-person idle pose.");return true;
    }
    if(n>=1 && !std::strcmp(name,"off")) {fp_idle_raw_on.store(false);fp_idle_hold_variant.store(-1);say("First-person idle pose hold off.");return true;}
    if(v<0) {say("Usage: fpidle [inspect|heft|rest] [seconds] (developer: hold a D53G pose, or play one now; fpidle off)");return true;}
    if(n==2) {fp_idle_hold_t.store(t);fp_idle_hold_variant.store(v);}
    else fp_idle_play_request.store(v);
    char text[160];std::snprintf(text,sizeof text,n==2?"Holding first-person idle %s at %.2f s (session only).":"Playing first-person idle %s.",fp_idle_gestures[v].name,t);
    say(text);close=n!=2;return true;
}
