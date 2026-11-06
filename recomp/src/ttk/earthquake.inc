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
//
// D44B scripted level events (documentation/399-d44b-scripted-earthquakes.md):
// TIME TO KILL's subway train crash starts 48 tics (Duke 3D's SE 2 length).
//
// D44C the overlays' own shakes (documentation/400-d44c-overlay-earthquakes.md):
// the robot turrets that drive around OBEY OR DIE and PIG FACTORY (types
// 621-623, blown up by a level copy of the vest's blast) and the boss
// landings of WING'ED DEATH and MOLOCH call 0x80028a2c from their level
// overlay and start 16 tics like the barrels; MINER 69ER's tremor rooms hold
// an earthquake while Duke is inside, their native jitter masked.
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
static uint64_t quake_starts,quake_barrels,quake_vests,quake_images,quake_rumbles,quake_trains,quake_robots,quake_bosses,quake_tremors,quake_tremor_masks;
static constexpr unsigned quake_train_crash=48;
static bool earthquake_enabled() {
    const char* t=ttk_live_env("DNTTK_EARTHQUAKES");
    return input_modernized() && !(t && !std::strcmp(t,"off"));
}
static bool earthquake_trace() {static const bool v=std::getenv("DNTTK_QUAKE_TRACE")!=nullptr;return v;}
static void earthquake_start(double tics,const char* why) {
    quake_left=tics;quake_shown=false;quake_loads=psx_mod_savestate_loads();++quake_starts;
    if(earthquake_trace())std::fprintf(stderr,"[TTK quake] start %.2f tics (%s) field %llu\n",tics,why,(unsigned long long)input_host_frame());
}
// D44C: level overlay calls of 0x80028a2c (return addresses inside the
// overlay at 0x800ca968), each valid only under its authenticated level tag
// (identity(): the whole overlay body matches the owned disc).
struct QuakeOverlaySource {uint32_t tag,ra;bool boss;};
static constexpr QuakeOverlaySource quake_overlay_sources[]={
    {0x8,0x800cbcf0u,false},  // LEVEL05 OBEY OR DIE: the robot turrets, types 621-623 (0x800cbaa0)
    {0xc,0x800cb4acu,false},  // LEVEL09 PIG FACTORY: the same code (0x800cb4a4)
    {0x1a,0x800cb704u,true},  // LEVEL28 WING'ED DEATH: the boss lands (animation 0x143)
    {0x1b,0x800cb7fcu,true},  // LEVEL29 MOLOCH: landing, state flag +0xb8 0x100
    {0x1b,0x800cb964u,true},  // LEVEL29 MOLOCH: stomp, state flag +0xb8 2
};
static const QuakeOverlaySource* earthquake_overlay_source(uint32_t ra) {
    if(ra<0x800ca968u)return nullptr;
    for(const auto& s:quake_overlay_sources)
        if(s.ra==ra)return identity() && identity_tag==s.tag?&s:nullptr;
    return nullptr;
}
// Entry of 0x80028a2c.
static void earthquake_native_shake(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    const bool barrel=ra==0x800973e0u,vest=ra==0x80064170u;
    if(earthquake_trace() && gameplay_context())
        std::fprintf(stderr,"[TTK quake] native shake ra=%08x strength=%d time=%d%s%u\n",ra,(int)cpu->gpr[6],(int)cpu->gpr[7],
            vest?" vest type ":"",vest?psx_mod_read_half(cpu->gpr[20]+0x2c):0u);
    // Every call site is a jal 0x80028a2c: the main executable's two, or an
    // authenticated overlay's.
    const QuakeOverlaySource* overlay=barrel || vest?nullptr:earthquake_overlay_source(ra);
    if((!barrel && !vest && !overlay) || psx_mod_read_word(ra-8)!=0x0c00a28bu ||
       !earthquake_enabled() || !gameplay_context() || cpu->gpr[4]!=camera)return;
    if(!quake_scratch)quake_scratch=psx_mod_alloc_guest_memory(0xb0,16);
    if(!quake_scratch)return;
    for(uint32_t i=0;i<0xb0;i+=4)psx_mod_write_word(quake_scratch+i,psx_mod_read_word(camera+i));
    cpu->gpr[4]=quake_scratch;
    if(overlay) {
        // Started only when under 16 tics are left: a repeat on the first
        // tic (a boss state that asks twice) must not hold t still (the bob
        // flips on t), and a longer earthquake is not cut short.
        ++(overlay->boss?quake_bosses:quake_robots);
        if(quake_left<=quake_detonation-1)earthquake_start(quake_detonation,overlay->boss?"boss":"robot turret");
        return;
    }
    ++(barrel?quake_barrels:quake_vests);
    earthquake_start(quake_detonation,barrel?"barrel":"vest");
}
// D44C: MINER 69ER (LEVEL02) Duke's per-frame room hook 0x800cc954 holds the
// camera jitter at strength 2 (rooms 28 and 35), 4 (36) or 6 (38) with time
// 300 while Duke (+0x2e, his room) is inside, and latches the pad at 25, 65
// or 100. Here it is an earthquake that keeps running while he stays.
static unsigned earthquake_tremor_strength() {
    if(!identity() || identity_tag!=0x5)return 0;
    switch((int8_t)psx_mod_read_byte(player+0x2e)) {
    case 28: case 35: return 2;
    case 36: return 4;
    case 38: return 6;
    }
    return 0;
}
static void earthquake_tremor() {
    if(!earthquake_enabled() || !earthquake_tremor_strength())return;
    if(quake_left<=0 || quake_loads!=psx_mod_savestate_loads()) {++quake_tremors;earthquake_start(quake_detonation,"tremor");}
    // Topped up by an even number of tics, so the pattern runs on unbroken.
    else if(quake_left<quake_detonation)quake_left+=quake_detonation;
}
// Camera update entry (0x8003ade4, Duke's camera): the tremor's own jitter,
// at or below the room strength, is taken out before 0x80028bdc plays it.
// A stronger shake (a rocket in the room) plays as usual. The overlay's pad
// latch is left alone.
static void earthquake_tremor_mask() {
    if(!earthquake_enabled() || !gameplay_context())return;
    const unsigned strength=earthquake_tremor_strength();
    const int16_t now=(int16_t)psx_mod_read_half(camera+0x48);
    if(!strength || now<=0 || now>(int)strength)return;
    psx_mod_write_half(camera+0x48,0);psx_mod_write_half(camera+0x4a,0);++quake_tremor_masks;
}
// D44B: TIME TO KILL's subway train, object type 77 (LEVEL00 only; first_map()
// authenticates the whole overlay). The overlay's handler switches on the
// train's +0x35 through the jump table 0x800caa2c: 0 start (sound 0xa002),
// 1 speed doubling to 256 a frame, 2 at speed, 3 the crash (set at
// 0x800cca2c once X drops below 8001: the speed halves each frame, the train
// slews and throws debris through 0x8006d6ac), 4 stopped. The step from 2 to
// 3 is the impact. The train relinks between rooms as it runs, so it is
// found by walking every room's object list (table 0x800da454, 88 bytes a
// room, list +0x48, nodes +4 next, +8 object).
static uint32_t quake_train;
static unsigned quake_train_state=~0u;
static uint32_t quake_train_loads;
static uint32_t earthquake_find_train() {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=std::min(256u,psx_mod_read_word(0x800da47c));
    if(!table)return 0;
    for(uint32_t room=0;room<rooms;++room) {
        uint32_t node=psx_mod_read_word(table+room*88+0x48);
        for(unsigned k=0;node && k<256;++k,node=psx_mod_read_word(node+4)) {
            const uint32_t o=psx_mod_read_word(node+8);
            if(o && psx_mod_read_half(o+0x2c)==77 && psx_mod_read_byte(o+0x14)==1)return o;
        }
    }
    return 0;
}
static void earthquake_scripted() {
    earthquake_tremor();
    if(!first_map() || quake_train_loads!=psx_mod_savestate_loads()) {
        quake_train=0;quake_train_state=~0u;quake_train_loads=psx_mod_savestate_loads();
        if(!first_map())return;
    }
    if(quake_train && (psx_mod_read_half(quake_train+0x2c)!=77 || psx_mod_read_byte(quake_train+0x14)!=1)) {quake_train=0;quake_train_state=~0u;}
    if(!quake_train) {
        static uint64_t next_search;
        const uint64_t frame=input_host_frame();
        if(frame<next_search)return;
        next_search=frame+60;
        if(!(quake_train=earthquake_find_train()))return;
    }
    const unsigned state=psx_mod_read_byte(quake_train+0x35);
    if(quake_train_state==2 && state==3 && earthquake_enabled()) {++quake_trains;earthquake_start(quake_train_crash,"train");}
    quake_train_state=state;
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
    earthquake_scripted();
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
    static char b[384];
    std::snprintf(b,sizeof b,"{\"enabled\":%s,\"left\":%.2f,\"tic\":%u,\"on\":%s,\"starts\":%llu,\"barrels\":%llu,\"vests\":%llu,\"images\":%llu,\"rumbles\":%llu,\"trains\":%llu,\"robots\":%llu,\"bosses\":%llu,\"tremors\":%llu,\"tremor_masks\":%llu}",
        earthquake_enabled()?"true":"false",quake_left,quake_view.tic,quake_view.on?"true":"false",(unsigned long long)quake_starts,(unsigned long long)quake_barrels,
        (unsigned long long)quake_vests,(unsigned long long)quake_images,(unsigned long long)quake_rumbles,(unsigned long long)quake_trains,
        (unsigned long long)quake_robots,(unsigned long long)quake_bosses,(unsigned long long)quake_tremors,(unsigned long long)quake_tremor_masks);
    return b;
}
