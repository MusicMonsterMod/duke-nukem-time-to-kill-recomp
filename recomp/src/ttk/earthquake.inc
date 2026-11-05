// D44A: Duke Nukem 3D's earthquake (documentation/397-d44a-earthquakes.md;
// research note 396). One global countdown in Duke 3D tics (30 a second): a
// source sets it, never adds; while it runs every image moves the eye +-256 Build z (here +-quake_bob units, the same share of
// the eye height) flipping each tic, and turns it 8/2048 of a turn right for
// two tics of four (EDuke32 G_DrawRooms: z += 256 - ((t & 1) << 9),
// ang += (2 - (t & 2)) << 2). Full strength to the last tic, map-wide, no
// fade, no easing. The eye only: frame_replay.cpp applies the offset to the
// camera of each image it composes or redraws and takes it back after, so
// aim, movement, collision and the camera's own smoothing never see it; in
// first person Duke's own draw (arms, weapon, legs) keeps the steady eye.
//
// Sources: the native blast shake request 0x80028a2c(camera, position,
// strength, time) from the TNT barrel blast 0x80097090 (return 0x800973e0)
// and the explosive vest 0x80063f0c (return 0x80064170; its only caller is
// the vest's fuse countdown 0x80064fa8, which beeps while +0x1f0 runs down,
// so ordinary deaths never reach it) start 16 tics (one Duke 3D DETONATE)
// and their native random jitter is replaced: the
// request is pointed at a scratch copy of the camera. Rockets and grenades
// (0x800702bc), pipe bombs (0x800707cc) and dynamite (0x80070f7c) keep the
// native shake (no earthquake for weapons in Duke 3D). Console
// "earthquake <seconds>". Modernized with EARTHQUAKES on only.
static constexpr double quake_bob=21;       // 797-unit standing eye / 38 (Duke 3D: 256 of 38<<8)
static constexpr double quake_yaw=8*tau/2048; // 1.41 degrees
static constexpr unsigned quake_detonation=16;
struct QuakeView {bool on=false,eye=false;double dy=0,yaw=0;unsigned tic=0;};
static QuakeView quake_view;                // this image's offset (RenderState carries it to workers)
static double quake_left;                   // Duke 3D g_earthquakeTime, fractional below 30 fps
static bool quake_shown;                    // the starting tic has been composed
static uint32_t quake_loads;
static std::atomic<int> quake_request{-1};  // console, in tics x 1000
static uint32_t quake_scratch;
static uint64_t quake_starts,quake_barrels,quake_vests,quake_images,quake_rumbles;
static bool earthquake_enabled() {
    const char* t=ttk_live_env("DNTTK_EARTHQUAKES");
    return input_modernized() && !(t && !std::strcmp(t,"off"));
}
static bool earthquake_trace() {static const bool v=std::getenv("DNTTK_QUAKE_TRACE")!=nullptr;return v;}
static void earthquake_start(double tics,const char* why) {
    quake_left=tics;quake_shown=false;quake_loads=psx_mod_savestate_loads();++quake_starts;
    if(earthquake_trace())std::fprintf(stderr,"[TTK quake] start %.2f tics (%s) field %llu\n",tics,why,(unsigned long long)input_host_frame());
}
// Entry of 0x80028a2c.
static void earthquake_native_shake(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    const bool barrel=ra==0x800973e0u,vest=ra==0x80064170u;
    if(earthquake_trace() && gameplay_context())
        std::fprintf(stderr,"[TTK quake] native shake ra=%08x strength=%d time=%d%s%u\n",ra,(int)cpu->gpr[6],(int)cpu->gpr[7],
            vest?" vest type ":"",vest?psx_mod_read_half(cpu->gpr[20]+0x2c):0u);
    // The two call sites are main executable code: each is a jal 0x80028a2c.
    if((!barrel && !vest) || psx_mod_read_word(ra-8)!=0x0c00a28bu ||
       !earthquake_enabled() || !gameplay_context() || cpu->gpr[4]!=camera)return;
    if(!quake_scratch)quake_scratch=psx_mod_alloc_guest_memory(0xb0,16);
    if(!quake_scratch)return;
    for(uint32_t i=0;i<0xb0;i+=4)psx_mod_write_word(quake_scratch+i,psx_mod_read_word(camera+i));
    cpu->gpr[4]=quake_scratch;
    ++(barrel?quake_barrels:quake_vests);
    earthquake_start(quake_detonation,barrel?"barrel":"vest");
}
// The native pad rumble latch 0x8001e97c(port, strength): raise the pad's
// motor byte when rumble is enabled (0x800c3f9c & 0x30) and the pad has one.
static void earthquake_rumble(unsigned strength) {
    const unsigned port=psx_mod_read_byte(player+0x233);
    if(port>=4 || (psx_mod_read_half(0x800c3f9c)&0x30)!=0x30)return;
    const uint32_t pad=0x800d1610+0x22c*port;
    if(!psx_mod_read_byte(pad+0x216) || psx_mod_read_byte(pad+0x228)>=strength)return;
    psx_mod_write_byte(pad+0x228,(uint8_t)strength);++quake_rumbles;
}
// Live gameplay composition entry (once a game frame; not while paused):
// the countdown, the pad and this image's offset. A start composes its first
// tic before the countdown moves.
void earthquake_compose() {
    quake_view={};
    if(!gameplay_context())return;
    const int request=quake_request.exchange(-1);
    if(request>=0) {
        if(request==0) {quake_left=0;if(earthquake_trace())std::fprintf(stderr,"[TTK quake] stopped (console)\n");}
        else earthquake_start(request/1000.0,"console");
    }
    if(quake_left<=0)return;
    if(quake_loads!=psx_mod_savestate_loads() || !earthquake_enabled()) {quake_left=0;return;}
    if(quake_shown) {
        // The frame step 0x800d21fc: 10 at 30 fps (one tic), 15 at 20.
        const unsigned step=std::min(30u,psx_mod_read_word(0x800d21fc));
        quake_left=std::max(0.0,quake_left-step/10.0);
        if(quake_left<=0)return;
    }
    quake_shown=true;
    // Duke 3D shakes only on the ground (on_ground); here it keeps going in
    // jumps, falls, water, flight and climbs (user, 2026-11-05).
    const unsigned t=(unsigned)std::ceil(quake_left-1e-6);
    // EDuke32 rumbles both motors at t << 8 of 65535 while the eye shakes.
    earthquake_rumble(std::min(255u,t));
    quake_view.on=true;quake_view.tic=t;
    quake_view.dy=(t&1)?-quake_bob:quake_bob;   // game Y grows downward, as Build z
    quake_view.yaw=(t&2)?0.0:quake_yaw;
    quake_view.eye=first_person_view_live() && fp_blend>=0.999;
    ++quake_images;
    if(earthquake_trace())std::fprintf(stderr,"[TTK quake] image tic %u dy %+.0f yaw %.2f%s field %llu\n",t,quake_view.dy,quake_view.yaw*360/tau,
        quake_view.eye?" eye":"",(unsigned long long)input_host_frame());
}
bool earthquake_image(double& dy,double& yaw,bool& eye) {
    dy=quake_view.dy;yaw=quake_view.yaw;eye=quake_view.eye;
    return quake_view.on;
}
// Backtick console: "earthquake <seconds>" (decimals allowed), "earthquake"
// alone one detonation (16 tics), "earthquake 0" stops one. Sets, never adds.
bool earthquake_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    if(std::strncmp(line,"earthquake",10) || (line[10] && line[10]!=' '))return false;
    if(!input_modernized()) {say("Earthquake: Modernized profile only (Vanilla is unchanged)");return true;}
    if(psx_mod_read_half(0x800bcbb0)!=1 || psx_mod_read_half(0x800ce1b0)!=2) {say("Earthquake: start or load a game first");return true;}
    const char* arg=line+10;
    while(*arg==' ')++arg;
    double seconds=quake_detonation/30.0;
    if(*arg) {
        char* end=nullptr;seconds=std::strtod(arg,&end);
        while(end && *end==' ')++end;
        if(end==arg || (end && *end) || !(seconds>=0) || seconds>600) {say("Earthquake: earthquake <seconds> (0 to 600; earthquake alone is one blast)");return true;}
    }
    if(!earthquake_enabled()) say("Earthquake: EARTHQUAKES is off in OPTIONS > GAMEPLAY");
    const int tics_k=(int)std::lround(seconds*30000);
    quake_request.store(tics_k);
    char reply[80];
    if(tics_k==0)std::snprintf(reply,sizeof reply,"Earthquake stopped");
    else std::snprintf(reply,sizeof reply,"Earthquake: %.2f s (%d tics)",seconds,(int)std::lround(seconds*30));
    say(reply);
    close=tics_k>0;
    return true;
}
const char* earthquake_json() {
    static char b[256];
    std::snprintf(b,sizeof b,"{\"enabled\":%s,\"left\":%.2f,\"tic\":%u,\"on\":%s,\"starts\":%llu,\"barrels\":%llu,\"vests\":%llu,\"images\":%llu,\"rumbles\":%llu}",
        earthquake_enabled()?"true":"false",quake_left,quake_view.tic,quake_view.on?"true":"false",(unsigned long long)quake_starts,(unsigned long long)quake_barrels,
        (unsigned long long)quake_vests,(unsigned long long)quake_images,(unsigned long long)quake_rumbles);
    return b;
}
