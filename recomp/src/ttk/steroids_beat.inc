// D08A10 (experimental) steroids heartbeat: while portable steroids run, a
// short game sound plays at Duke on Duke 3D's rhythm (PLAYER.C: DUKE_HARTBEAT
// every 8 game tics at 30 Hz = 266.7 ms, 225 bpm; the user's reference video
// measures 268 ms). Remove this file and its three hook-ups to revert.
//
// The beat runs on the steroids timer itself: the drain 0x800414a0 takes the
// frame step 0x800d21fc (5 per field, 300 a second) from player+0x366, so a
// beat falls each time the amount crosses a multiple of 80 (80/300 s). The
// rhythm is game time at any frame rate, and it stops by itself when the
// original ends the effect (timer out, damage cut 0x800a4154, the level reset).
// Checked from the authenticated player update (0x8005a210 from 0x80041b34),
// which runs in every Duke state (ground, swim, jetpack, ladders), never in
// render replays.
//
// Sound: 0x0001, the click the walk/run toggle makes (Shift in Modernized):
// the player update flips player+0x224 bit 1 and calls the original
// non-positional sound call 0x8006bbd8(1) (0x800418cc, 0x80041934, 0x80041994,
// 0x800419c8). The beat makes the same call on a private stack.
// id = bank << 12 | index (0x8006bc4c). "sfx <id>" (debug) plays a sound at
// Duke through the positional call 0x8006b73c(id, Duke+4, 0x800, 0).
// DNTTK_STEROID_BEAT=off silences it; =<id> (hex or decimal) tries another.
static constexpr unsigned beat_units=80;
static int beat_sound() {  // -1: off
    static const int id=[]{
        const char* v=std::getenv("DNTTK_STEROID_BEAT");
        if(!v || !*v) return 0x0001;
        if(!std::strcmp(v,"off")) return -1;
        return (int)std::strtoul(v,nullptr,0);
    }();
    return id;
}
static int beat_bucket=-1,beat_start,beat_last;
static uint64_t beat_loads;
static unsigned beats,beat_skips,beat_resyncs,beat_restarts;
// An original sound call on a private stack: positional at Duke (0x8006b73c,
// the pickup tail's args) or the plain call 0x8006bbd8(id).
static bool duke_sound(CPUState* cpu,unsigned id,bool at_duke=true) {
    const uint32_t top=cpu->gpr[29];
    if(id>0xfffff || guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState call=*cpu;call.pc=0;call.gpr[29]=top-0x100;call.gpr[31]=0x800000fc;
    call.gpr[4]=id;call.gpr[5]=player+4;call.gpr[6]=0x800;call.gpr[7]=0;
    guest_busy=true;psx_dispatch_call(&call,at_duke?0x8006b73c:0x8006bbd8,0x800000fc);guest_busy=false;
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return true;
}
// Debug: the last sound calls (id, caller) and "sfx <id>" to audition one.
struct SoundCall {uint32_t id,ra,frame;};
static SoundCall sound_log[32];
static std::atomic<unsigned> sound_log_next;
static std::atomic<int> sfx_request{-1};
static void sound_logged(CPUState* cpu) {
    const unsigned i=sound_log_next.fetch_add(1)%32;
    sound_log[i]={cpu->gpr[4],cpu->gpr[31],(uint32_t)input_host_frame()};
}
static void steroids_beat(CPUState* cpu) {
    const int sfx=sfx_request.exchange(-1);
    if(sfx>=0) duke_sound(cpu,(unsigned)sfx);
    const uint64_t loads=psx_mod_savestate_loads();
    // Any running steroids (a used dose, a pickup while one runs, dnhyper) while
    // Duke is alive; the original profile keeps the original's silence.
    const bool running=portable_steroids() && (psx_mod_read_half(player+0x364)&2) &&
        (int16_t)psx_mod_read_half(player+0x366)>0 && (int16_t)psx_mod_read_half(player+0x32)>0 && player_identity_ready();
    if(!running || loads!=beat_loads || beat_sound()<0) {beat_bucket=-1;beat_loads=loads;return;}
    // Beats count from the amount the dose started at, so the second beat
    // also falls a full 80 units after the first.
    const int amount=(int16_t)psx_mod_read_half(player+0x366);
    // Taken (or refreshed by a pickup, or dnhyper again): the first beat at
    // once, as Duke 3D. Only the drain lowers the amount between checks, so
    // any rise is a restart (D08A15). Comparing with the start instead missed
    // a refresh to the full amount: the drain runs before this check, so a
    // full start and a full refresh both read 9000 - step, never above it,
    // and the old count held the next beat back for as long as the old run
    // had lasted.
    const bool restarted=beat_bucket>=0 && amount>beat_last;
    beat_last=amount;
    if(beat_bucket<0 || restarted) {
        if(restarted) ++beat_restarts;
        beat_start=amount;beat_bucket=0;
        if(duke_sound(cpu,(unsigned)beat_sound(),false)) ++beats;
        return;
    }
    const int bucket=(beat_start-amount)/(int)beat_units;
    if(bucket<=beat_bucket) return;
    // One crossing per update; a longer drop (a damage cut, a hitch) beats once.
    if(bucket-beat_bucket>1) ++beat_skips;
    beat_bucket=bucket;
    if(duke_sound(cpu,(unsigned)beat_sound(),false)) ++beats; else ++beat_resyncs;
}
static const char* steroids_beat_debug_json() {
    static char buffer[1400];
    int used=std::snprintf(buffer,sizeof buffer,"{\"sound\":%d,\"beats\":%u,\"skips\":%u,\"refused\":%u,\"restarts\":%u,\"bucket\":%d,\"log\":[",
        beat_sound(),beats,beat_skips,beat_resyncs,beat_restarts,beat_bucket);
    const unsigned next=sound_log_next.load();
    for(unsigned k=0;k<32 && k<next && used<(int)sizeof buffer-40;++k) {
        const SoundCall& c=sound_log[(next-1-k)%32];
        used+=std::snprintf(buffer+used,sizeof buffer-used,"%s[%u,%u,%u]",k?",":"",c.id,c.ra,c.frame);
    }
    std::snprintf(buffer+used,sizeof buffer-used,"]}");
    return buffer;
}
bool beat_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    if(std::strncmp(line,"sfx ",4))return false;
    if(!input_modernized()) {say("sfx: Modernized profile only");return true;}
    char* end=nullptr;
    const unsigned long id=std::strtoul(line+4,&end,0);
    if(end==line+4 || *end || id>0xfffff) {say("sfx <id>: bank<<12|index, e.g. sfx 0x100f");return true;}
    sfx_request.store((int)id);
    char reply[48];std::snprintf(reply,sizeof reply,"Playing sound 0x%lx at Duke",id);
    say(reply);
    close=true;
    return true;
}
