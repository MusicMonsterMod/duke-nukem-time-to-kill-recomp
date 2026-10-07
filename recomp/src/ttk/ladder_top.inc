// D08U top-of-ladder mount (Modernized). Original rules, verified on the
// slot-12 sewer ladder and the first alley ladder
// (documentation/72-d08u-ladder-top.md):
//  * A plain ladder is an object whose type flags have 0x200 (0x100 and 0x400
//    are other climb families). Its collision box (0x80073ae8, index 0) is a
//    flat panel: x is the width, y the height, z (thickness) is zero; object
//    +0x1c is its yaw. Both faces are climbable.
//  * The original only mounts a plain ladder from below (0x80051cf0 -> 185)
//    or by catching it in the air (0x80047480..: +0x17c = ladder, +0x180 = box,
//    mode 3, hang-to-ladder transfer 156 -> climb 186). There is no top mount:
//    from the platform above, the only original way down is a jump.
//  * Once attached, the original ladder handler owns everything: 186/188/189
//    climb 200 units off the panel, 190 exits at the top onto the platform and,
//    with Down + Cross held, 185 steps off onto the floor at the bottom.
// Modernized contract: E at the top of a plain ladder (on the platform, within
// reach of the panel, its top 300..800 units below Duke's feet) starts the
// original transfer 156 with the ladder attached exactly as the airborne catch
// does. Over the 12 player updates of 156 the host blends Duke from where he
// stands to the climbing line on the far side of the panel, turning him to
// face it; then the original 186 takes over. S on a plain ladder also holds
// Cross, so Duke climbs all the way down and steps off instead of stopping at
// the lowest rung. E-owned stow and redraw follow the D08J rules.
static constexpr uint32_t object_ladder=0x200u;
static constexpr unsigned ladder_mount_default_updates=12;
// Diagnostics only: DNTTK_LADDER_MOUNT_UPDATES lengthens the blend so that
// the original 156 ends first, as it does when the game drops frames.
static unsigned ladder_mount_updates() {
    static const unsigned n=[]{const char* v=std::getenv("DNTTK_LADDER_MOUNT_UPDATES");
        const long x=v?std::atol(v):0;return x>=4 && x<=60?unsigned(x):ladder_mount_default_updates;}();
    return n;
}
static unsigned ladder_mount_finishes;
struct LadderTop { uint32_t object; double x,y,z; int yaw; };
static struct {
    bool active;
    uint32_t object;
    double sx,sy,sz,ax,ay,az;
    int syaw,ayaw;
    unsigned ticks;
} mount;
static uint64_t ladder_request_frame;
static unsigned ladder_mounts, ladder_mount_starts, ladder_mount_aborts;
static bool plain_ladder(uint32_t flags) {return (flags&object_ladder) && !(flags&0x500u);}
// 0x80073ae8 for box index 0: the object's box list (+0x44) or its model's
// single box (+0x40, +4). Eight halfwords: x, y, z, pad minimum and maximum.
static bool ladder_box(uint32_t object,int16_t* box) {
    uint32_t base;
    const uint32_t list=psx_mod_read_word(object+0x44);
    if(list) {
        if(list<0x80010000u || list>=0x80200000u || (list&3))return false;
        base=psx_mod_read_word(list+8);
    } else {
        if(object_type_flags(object)&0x2000u)return false;
        base=psx_mod_read_word(object+0x40);
        if(base)base+=4;
    }
    if(base<0x80010000u || base>=0x80200000u || (base&1))return false;
    for(int i=0;i<8;++i)box[i]=(int16_t)psx_mod_read_half(base+2*i);
    return true;
}
// A plain ladder in Duke's cell list (the list 0x80077f28 walks) whose top is
// just below the floor he stands on, within reach of its panel.
static bool ladder_top_find(LadderTop& out) {
    const double px=(int32_t)psx_mod_read_word(player+4),py=(int32_t)psx_mod_read_word(player+8),
        pz=(int32_t)psx_mod_read_word(player+12);
    uint32_t node=psx_mod_read_word(player+0x220);
    for(int guard=0;node && guard<64;++guard) {
        if(node<0x80010000u || node>=0x80200000u || (node&3))return false;
        const uint32_t object=psx_mod_read_word(node+8);
        node=psx_mod_read_word(node+4);
        if(!plain_ladder(object_type_flags(object)))continue;
        int16_t b[8];
        if(!ladder_box(object,b))continue;
        const double half=(b[4]-b[0])/2.0;
        if(half<64 || half>1024 || std::abs(b[6]-b[2])>64)continue;
        const double a=(psx_mod_read_half(object+0x1c)&4095)*tau/4096;
        // RotMatrixY: local z is the panel normal, local x its width.
        const double nx=std::sin(a),nz=std::cos(a),tx=std::cos(a),tz=-std::sin(a);
        const double lx=(b[0]+b[4])/2.0,lz=(b[2]+b[6])/2.0;
        const double cx=(int32_t)psx_mod_read_word(object+4)+lx*tx+lz*nx;
        const double cz=(int32_t)psx_mod_read_word(object+12)+lx*tz+lz*nz;
        const double top=(int32_t)psx_mod_read_word(object+8)+std::min(b[1],b[5]);
        const double rx=px-cx,rz=pz-cz;
        const double normal=rx*nx+rz*nz,lateral=rx*tx+rz*tz,above=top-py;
        if(above<300 || above>800 || std::abs(lateral)>half+120 ||
           std::abs(normal)<40 || std::abs(normal)>420)continue;
        // Climb on the far face, facing the panel (toward the platform).
        const double side=normal>0?-1:1;
        out.object=object;
        out.x=cx+side*200*nx;out.z=cz+side*200*nz;out.y=top+320;
        out.yaw=int(std::lround(std::atan2(-side*nx,-side*nz)*4096/tau))&4095;
        return true;
    }
    return false;
}
// Settled normal ground under a live modern lease: never airborne, in water,
// on the jetpack, grabbing, in a mantle or already attached.
static bool ladder_top_ground_ready() {
    if(!(movement_ready() || locomotion_input_ready()))return false;
    const unsigned animation=psx_mod_read_half(player+0x60);
    return psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 &&
        land_gait_anim(animation) && !in_water_early() && !jetpack_flying_early() &&
        !(psx_mod_read_word(player+0x224)&0x202002c1u) && !(psx_mod_read_word(player)&0x20002);
}
bool ladder_top_available() {
    LadderTop target;
    return ladder_top_ground_ready() && ladder_top_find(target);
}
void ladder_top_request() {ladder_request_frame=input_host_frame();}
// Camera-only lease widening while the host blends the mount.
static bool ladder_mount_early() {
    return mount.active && psx_mod_read_half(player+0x60)==156 && psx_mod_read_byte(player+0x22c)==3 &&
        psx_mod_read_word(player+0x17c)==mount.object;
}
// Duke is on a plain ladder (mount transfer, climb, exit or bottom step-off).
bool ladder_descent_ready() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    return traversal_input_ready() && psx_mod_read_byte(player+0x22c)==3 &&
        (animation==156 || (animation>=185 && animation<=190)) &&
        plain_ladder(object_type_flags(psx_mod_read_word(player+0x17c)));
}
// D08U1: where a plain ladder ends above the floor (0x8007ded0 finds no ladder
// 500 below Duke), the original leaves the lowest rung through 156 and 211
// into a hang from the bottom rung (207). There Square lets go and Duke drops
// to the floor, Up climbs back; Down only flips 211/207 with Duke's body in the
// floor. 1 = on the way into the hang, 2 = hanging (207).
int ladder_bottom_hang() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if((animation!=207 && animation!=211) || !traversal_input_ready() ||
       psx_mod_read_byte(player+0x22c)!=3 || psx_mod_read_byte(player+0x22d)!=3 ||
       !plain_ladder(object_type_flags(psx_mod_read_word(player+0x17c))))return 0;
    return animation==207?2:1;
}
// D08U1: the same decision, taken cleanly. While Duke is on a plain ladder
// (186..189), the player update asks the original's own probe 0x8007ded0 (no
// ladder and no floor 500 below Duke) before the ladder case runs.
//  * Stepping down (188/189 with the descent flag +0x6a): when the probe is
//    true, the case at 0x80044128 would start 156 -> 211 -> 207 in this very
//    update (it reads +0x6a, not Down, so a tapped S that is already released
//    gets there too). The flag is cleared, so the original takes its own
//    stop-at-this-rung branch instead.
//  * On that last rung, S sends Square, the original let-go from the climbing
//    pose (fall 108, landing), instead of Down.
// A ladder that reaches its floor never trips the probe and keeps the
// original 185 step-off.
static bool ladder_end_open;
static constexpr uint32_t ladder_end_margin=96;
// The rung where Duke was stopped: he settles a little higher (rest 186/187),
// where the probe no longer sees the end, so it stays the last rung until he
// climbs up from it or leaves the ladder.
static struct {bool on; uint32_t object; int32_t y;} ladder_last_rung;
static unsigned ladder_end_probes, ladder_end_hits, ladder_end_stops;
static bool ladder_climb_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    return animation>=186 && animation<=189 && psx_mod_read_byte(player+0x22c)==3 &&
        psx_mod_read_byte(player+0x22d)==3 &&
        plain_ladder(object_type_flags(psx_mod_read_word(player+0x17c)));
}
static void ladder_end_update(CPUState* cpu) {
    ladder_end_open=false;
    if(ladder_last_rung.on && (!ladder_climb_early() || psx_mod_read_word(player+0x17c)!=ladder_last_rung.object ||
       (int32_t)psx_mod_read_word(player+8)<ladder_last_rung.y-150))ladder_last_rung.on=false;
    // Session-log diagnostics: one line per ladder pose change (mode 3/8),
    // with the keys held and the original buttons sent (Up/Down/Cross/Square).
    {
        static unsigned last_animation;
        const unsigned animation=psx_mod_read_half(player+0x60),mode=psx_mod_read_byte(player+0x22c);
        if(input_modernized() && (mode==3 || mode==8 || last_animation) && animation!=last_animation) {
            const auto& g=input_snapshot(Context::Gameplay);const uint16_t pad=~input_last_pad();
            std::fprintf(stderr,"[TTK ladder] anim=%u mode=%u/%u y=%d obj=%08x keys=%s%s%s pad=%s%s%s%s end=%d\n",animation,mode,
                psx_mod_read_byte(player+0x22d),(int32_t)psx_mod_read_word(player+8),psx_mod_read_word(player+0x17c),
                g.held[move_forward]?"W":"",g.held[move_back]?"S":"",g.held[interact]?"E":"",
                (pad&16)?"U":"",(pad&64)?"D":"",(pad&16384)?"X":"",(pad&32768)?"Q":"",ladder_last_rung.on);
            last_animation=(mode==3 || mode==8)?animation:0;
        }
    }
    if(!input_modernized() || !input_snapshot(Context::Gameplay).active || !ladder_climb_early() ||
       !traversal_input_ready() || !gameplay_context() || !identity())return;
    // 0x8007dd50 rewrites the touch fields; the original recomputes them at
    // its own call, so the probe leaves them as they were.
    const uint32_t touch=psx_mod_read_word(player+0x174);
    const uint16_t touch_part=psx_mod_read_half(player+0x178);
    // The handler moves Duke (20-40 units per update) before its own probe,
    // so ask a little lower than he stands now. A whole step (300) is too far:
    // on a ladder that reaches its floor the point falls below the floor.
    const uint32_t y=psx_mod_read_word(player+8);
    psx_mod_write_word(player+8,y+ladder_end_margin);
    int32_t open=0;
    const bool called=original_call(cpu,0x8007ded0,0,&open);
    psx_mod_write_word(player+8,y);
    psx_mod_write_word(player+0x174,touch);psx_mod_write_half(player+0x178,touch_part);
    if(!called)return;
    ++ladder_end_probes;
    if(!open) {ladder_end_open=ladder_last_rung.on;return;}
    ladder_end_open=true;++ladder_end_hits;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if((animation==188 || animation==189) && psx_mod_read_half(player+0x6a)) {
        psx_mod_write_half(player+0x6a,0);++ladder_end_stops;
        ladder_last_rung={true,psx_mod_read_word(player+0x17c),(int32_t)y};
    }
}
// D08U2: an enemy standing at the top of a ladder. Resting on the top rung
// (186/187) with Up, the original ladder handler (0x80043fd0..) asks
// 0x8007d65c(player, 0, 1) whether the ladder ends here (1), then the exit
// test 0x8007d240, and starts the top exit 190 only when it passes. The exit
// test first casts a short segment forward from Duke at the ladder top
// (0x8007cde8): any object in his cell list without type flag 8 that it meets
// refuses the exit. A living character there (hostile AI class 6 or NPC class
// 8, object +0x15) therefore leaves Duke on the top rung for as long as it
// stands there. Modernized: when such characters are the only thing the
// segment meets, the host runs the original exit test with them moved out of
// the way for the length of the call and, if it passes, starts the original
// exit 190 exactly as the handler does (+0x60 = 190, +0x68 = +0x6a = 0). Walls,
// crates and other objects still refuse the exit; Vanilla is unchanged.
static unsigned ladder_actor_exits,ladder_actor_checks;
static bool ladder_actor(uint32_t object) {
    if(object<0x80010000u || object>=0x801ffd00u || (object&3))return false;
    const unsigned kind=psx_mod_read_byte(object+0x15);
    if(kind!=6 && kind!=8)return false;
    const uint32_t types=psx_mod_read_word(0x800d2660);
    const unsigned type=psx_mod_read_half(object+0x2c);
    if(type>=1024 || psx_mod_read_byte(types+28*type+6)!=kind)return false;
    return !(psx_mod_read_word(object)&2) && (int16_t)psx_mod_read_half(object+0x32)>0;
}
static void ladder_actor_exit(CPUState* cpu) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(mount.active || (animation!=186 && animation!=187) || !ladder_climb_early() ||
       !input_modernized() || !input_snapshot(Context::Gameplay).active || !traversal_input_ready() ||
       !gameplay_context() || !identity() || (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x200241u) || !((~input_last_pad())&16) ||
       psx_mod_read_half(player+0x180)!=0)return;
    const uint32_t ladder=psx_mod_read_word(player+0x17c);
    int16_t box[8];
    if(!ladder_box(ladder,box))return;
    // The characters in Duke's cell list, before anything is called.
    uint32_t actors[16];unsigned count=0;
    uint32_t node=psx_mod_read_word(player+0x220);
    for(int guard=0;node && guard<64;++guard) {
        if(node<0x80010000u || node>=0x80200000u || (node&3))return;
        const uint32_t object=psx_mod_read_word(node+8);
        node=psx_mod_read_word(node+4);
        if(ladder_actor(object) && count<16)actors[count++]=object;
    }
    if(!count)return;
    // 0x8007d65c and 0x8007d240 rewrite the touch fields; the handler calls
    // them again in this update, so a refused attempt leaves them as they were.
    const uint32_t touch=psx_mod_read_word(player+0x174);
    const uint16_t touch_part=psx_mod_read_half(player+0x178);
    auto restore_touch=[&]{psx_mod_write_word(player+0x174,touch);psx_mod_write_half(player+0x178,touch_part);};
    int32_t top_reached=0;
    if(!original_call(cpu,0x8007d65c,0,&top_reached,1) || top_reached!=1) {restore_touch();return;}
    const uint32_t top=(uint32_t)((int32_t)psx_mod_read_word(ladder+8)+box[1]);
    int32_t blocked=0;
    if(!original_call(cpu,0x8007cde8,top,&blocked) || !blocked) {restore_touch();return;}
    ++ladder_actor_checks;
    // Far above the level for the length of the calls only.
    int32_t heights[16];
    for(unsigned i=0;i<count;++i) {
        heights[i]=(int32_t)psx_mod_read_word(actors[i]+8);
        psx_mod_write_word(actors[i]+8,(uint32_t)(heights[i]-0x40000));
    }
    int32_t still=1,exit=0;
    const bool clear=original_call(cpu,0x8007cde8,top,&still) && !still;
    const bool passed=clear && original_call(cpu,0x8007d240,0,&exit) && exit;
    for(unsigned i=0;i<count;++i)psx_mod_write_word(actors[i]+8,(uint32_t)heights[i]);
    if(!passed) {
        // 0x8007d240 attaches the landing only when it passes.
        if(!clear || psx_mod_read_word(player+0x17c)==ladder)restore_touch();
        return;
    }
    psx_mod_write_half(player+0x60,190);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    ++ladder_actor_exits;
    std::fprintf(stderr,"[TTK ladder] top exit past %u character(s) at the ladder top (first %08x type %u)\n",
        count,actors[0],psx_mod_read_half(actors[0]+0x2c));
}
// Duke rests on the ladder while the host finishes the mount blend: no Up or
// Down may start a step from the wrong height.
bool ladder_mount_finishing() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    return mount.active && (animation==186 || animation==187);
}
bool ladder_end_below() {return ladder_end_open && ladder_climb_early() && traversal_input_ready();}
// The original ladder exits (190 at the top, 185 stepping off at the bottom)
// play out by themselves; a held S must not reach them as Down, which would
// start an original backstep (82) the moment Duke lands.
bool ladder_exit_ready() {
    const unsigned animation=psx_mod_read_half(player+0x60),previous=psx_mod_read_byte(player+0x22d);
    return traversal_input_ready() && psx_mod_read_byte(player+0x22c)==8 &&
        (animation==185 || animation==190) && (previous==3 || previous==8);
}
static void ladder_mount_write(double e) {
    psx_mod_write_word(player+4,(int32_t)std::lround(mount.sx+(mount.ax-mount.sx)*e));
    psx_mod_write_word(player+8,(int32_t)std::lround(mount.sy+(mount.ay-mount.sy)*e));
    psx_mod_write_word(player+12,(int32_t)std::lround(mount.sz+(mount.az-mount.sz)*e));
    const int turn=((mount.ayaw-mount.syaw+2048)&4095)-2048;
    const int heading=int(std::lround(mount.syaw+turn*e))&4095;
    psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x24,heading);
    for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
}
// Player update (0x8005a210 from 0x80041b34), before the original handler
// advances 156 and applies its own small root motion.
static void ladder_top_update(CPUState* cpu) {
    ladder_end_update(cpu);
    ladder_actor_exit(cpu);
    if(mount.active) {
        const unsigned animation=psx_mod_read_half(player+0x60);
        const bool attached=psx_mod_read_byte(player+0x22c)==3 && psx_mod_read_word(player+0x17c)==mount.object &&
            animation>=186 && animation<=189;
        const bool unfinished=mount.ticks<ladder_mount_updates();
        if((psx_mod_read_word(player)&0x20002) || !gameplay_context() || !identity() ||
           (!ladder_mount_early() && !(attached && unfinished))) {
            // The original moved on: 186 means Duke is on the ladder.
            mount.active=false;
            if(attached)++ladder_mounts;
            else ++ladder_mount_aborts;
            return;
        }
        // D08U1: when the game drops frames the original 156 advances several
        // frames per player update and ends before the blend does; Duke was
        // then left above the climbing line, where S plays the step poses in
        // place. Finish the blend while he rests (186/187); a step that has
        // already started (188/189) gets the rest at once.
        if(animation==188 || animation==189) {
            ladder_mount_write(1);mount.active=false;++ladder_mounts;++ladder_mount_finishes;return;
        }
        ++mount.ticks;
        const double u=std::min(1.0,double(mount.ticks)/ladder_mount_updates());
        ladder_mount_write(u*u*(3-2*u));
        if(attached && mount.ticks>=ladder_mount_updates()) {mount.active=false;++ladder_mounts;++ladder_mount_finishes;}
        return;
    }
    if(!ladder_request_frame || input_host_frame()-ladder_request_frame>20)return;
    ladder_request_frame=0;
    LadderTop target;
    if(!ladder_top_ground_ready() || psx_mod_read_byte(player+0x3b8)!=0 || !ladder_top_find(target))return;
    mount.active=true;mount.object=target.object;mount.ticks=0;
    mount.sx=(int32_t)psx_mod_read_word(player+4);mount.sy=(int32_t)psx_mod_read_word(player+8);
    mount.sz=(int32_t)psx_mod_read_word(player+12);mount.syaw=psx_mod_read_half(player+0x1c)&4095;
    mount.ax=target.x;mount.ay=target.y;mount.az=target.z;mount.ayaw=target.yaw;
    // Exactly the airborne catch's attachment (0x800475c0..0x8004765c).
    psx_mod_write_word(player+0x17c,target.object);psx_mod_write_half(player+0x180,0);
    psx_mod_write_word(player+0x1c4,0);
    psx_mod_write_byte(player+0x22c,3);psx_mod_write_byte(player+0x22d,3);
    psx_mod_write_half(player+0x60,156);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    ladder_mount_write(0);
    ++ladder_mount_starts;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-ladder-top object=%08x from=%.0f,%.0f,%.0f yaw=%d to=%.0f,%.0f,%.0f yaw=%d\n",
            target.object,mount.sx,mount.sy,mount.sz,mount.syaw,target.x,target.y,target.z,target.yaw);
}
// D08J1 overhead ladder leap (Modernized). A plain ladder whose bottom rung
// hangs high on a wall cannot be mounted from the floor (0x80051cf0 only
// starts 185 at the feet); walking or running into it stops at the wall (94).
// The original catches it in the air: with E held the reach (armed at
// takeoff since D08X) attaches it from a running, walking or standing jump
// (147/148 -> 210 -> climb). Measured on the user's UI slot-6 ladder (bottom
// about 1040 above the floor, documentation/79-d08j1-ladder-leap.md): running
// jumps caught from 500..2000 units out, walking jumps from 600..1000 (400
// and 1300 met the wall low and bounced, 107), a standing jump at the wall
// caught. Contract: with E held, on settled ground, holstered and heading at
// such a ladder, the host presses the original jump once when Duke is inside
// the catching window for his gait. The original decides the catch.
static constexpr double leap_rise_min=300,leap_rise_max=1400;
// Off-centre approaches: a leap starts up to leap_side beyond the panel's
// half width; in flight Duke eases sideways (leap_step per update, along the
// panel only) until he is leap_inset inside it, where the original catches.
static constexpr double leap_side=180,leap_inset=60,leap_step=70;
static constexpr unsigned leap_updates=45;
static unsigned ladder_leaps,ladder_leap_nudges;
static struct { double cx,cz,tx,tz,half; } leap_found;
static struct { bool active; double cx,cz,tx,tz,half; unsigned ticks; bool airborne; } leap;
// Panel distance at which the jump is pressed: running, walking, standing.
static double ladder_leap_distance(unsigned animation) {
    if(animation>=76 && animation<=79)return 750;
    if(animation>=72 && animation<=75)return 800;
    if(animation==63)return 450;
    return 0;
}
bool ladder_leap_ready() {
    if(!ladder_top_ground_ready() || psx_mod_read_byte(player+0x3b8)!=0)return false;
    const double reach=ladder_leap_distance(psx_mod_read_half(player+0x60));
    if(reach<=0)return false;
    const double px=(int32_t)psx_mod_read_word(player+4),pz=(int32_t)psx_mod_read_word(player+12);
    const double feet=(int32_t)psx_mod_read_word(player+0x1c8);
    const double heading=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
    const double fx=std::sin(heading),fz=std::cos(heading);
    uint32_t node=psx_mod_read_word(player+0x220);
    for(int guard=0;node && guard<64;++guard) {
        if(node<0x80010000u || node>=0x80200000u || (node&3))return false;
        const uint32_t object=psx_mod_read_word(node+8);
        node=psx_mod_read_word(node+4);
        if(!plain_ladder(object_type_flags(object)))continue;
        int16_t b[8];
        if(!ladder_box(object,b))continue;
        const double half=(b[4]-b[0])/2.0;
        if(half<64 || half>1024 || std::abs(b[6]-b[2])>64)continue;
        const double a=(psx_mod_read_half(object+0x1c)&4095)*tau/4096;
        const double nx=std::sin(a),nz=std::cos(a),tx=std::cos(a),tz=-std::sin(a);
        const double lx=(b[0]+b[4])/2.0,lz=(b[2]+b[6])/2.0;
        const double cx=(int32_t)psx_mod_read_word(object+4)+lx*tx+lz*nx;
        const double cz=(int32_t)psx_mod_read_word(object+12)+lx*tz+lz*nz;
        const double bottom=(int32_t)psx_mod_read_word(object+8)+std::max(b[1],b[5]);
        const double rx=px-cx,rz=pz-cz;
        const double normal=rx*nx+rz*nz,lateral=rx*tx+rz*tz,rise=feet-bottom;
        if(rise<leap_rise_min || rise>leap_rise_max || std::abs(lateral)>half+leap_side ||
           std::abs(normal)>reach)continue;
        // Heading at the panel within about 50 degrees.
        const double side=normal>0?-1:1;
        if(fx*side*nx+fz*side*nz<0.64)continue;
        leap_found={cx,cz,tx,tz,half};
        return true;
    }
    return false;
}
void ladder_leap_note() {
    ++ladder_leaps;
    leap={true,leap_found.cx,leap_found.cz,leap_found.tx,leap_found.tz,leap_found.half,0,false};
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-ladder-leap anim=%u xyz=%d,%d,%d\n",psx_mod_read_half(player+0x60),
            (int32_t)psx_mod_read_word(player+4),(int32_t)psx_mod_read_word(player+8),(int32_t)psx_mod_read_word(player+12));
}
// The host's leap is under way (takeoff or flight): E's reach stays held, so a
// tap of E catches like a held E.
bool ladder_leap_active() {return leap.active;}
// Player update: while the host's leap is in the air, ease Duke along the
// panel into its width. Ends on the catch (mode 3), landing, Vanilla, state or
// identity loss, or after leap_updates.
static void ladder_leap_update() {
    if(!leap.active)return;
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(++leap.ticks>leap_updates || mode==3 || (mode!=0 && mode!=9) || (mode==0 && leap.airborne) ||
       (psx_mod_read_word(player)&0x20002) || !gameplay_context() || !identity() || !input_modernized()) {
        leap.active=false;return;
    }
    if(mode!=9)return;
    leap.airborne=true;
    const double px=(int32_t)psx_mod_read_word(player+4),pz=(int32_t)psx_mod_read_word(player+12);
    const double lateral=(px-leap.cx)*leap.tx+(pz-leap.cz)*leap.tz;
    const double excess=std::abs(lateral)-(leap.half-leap_inset);
    if(excess<=0)return;
    const double move=std::min(leap_step,excess)*(lateral>0?-1:1);
    psx_mod_write_word(player+4,(int32_t)std::lround(px+move*leap.tx));
    psx_mod_write_word(player+12,(int32_t)std::lround(pz+move*leap.tz));
    ++ladder_leap_nudges;
}
