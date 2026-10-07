// D08J5 climbing down poles and chains (Modernized). Original rules, verified
// on the user's slot-3 chain (documentation/125-d08j5-chain-descent.md):
//  * Poles and chains are objects whose type flags have 0x400 (not 0x100 or
//    0x200). Their box (0x80073ae8, index 0) is a thin column. The airborne
//    catch (0x800475c0..) attaches one with the transfer 154, then the
//    hang-climb 192..195 holds Duke 225 units from its axis along his
//    heading: every update the original places him from his yaw, so his
//    heading alone decides which side of the chain he is on.
//  * There is no top mount. From a platform beside a chain's top the only
//    original ways down are a jump (which overshoots a chain hanging past the
//    edge) or the drop.
//  * Down at the rest (192/193, 0x8004421c) or while stepping down (194/195,
//    0x80044398) asks 0x8007d65c(player, 1, ..): 4 means a floor within 360
//    units below Duke's lowest bone, and the handler steps him off onto it
//    (191). The test is signed and the floor comes from the level query
//    0x8007cf28 around Duke, which can return the platform the chain hangs
//    beside, up to about 1,000 units above him. Near the top on the platform
//    side, Down therefore lifts Duke back onto the platform.
//  * Up at the top exits onto the platform (196); Square lets go.
// Modernized contract:
//  * E at the top of a pole or chain (its top 300..800 units below the floor
//    Duke stands on, its axis within reach) starts the original 154 with the
//    object attached exactly as the airborne catch does. Over the player
//    updates of 154 the host lowers Duke to the top of the chain and turns
//    his heading half a turn, so the original swings him round to the far
//    side, facing back toward the platform. Then the original hang-climb owns
//    him.
//  * Climbing down never steps Duke off onto a floor above his feet: in the
//    down probe the host drops such a floor before the original's 360 test,
//    so the descent goes on. A floor below still ends in the original 191.
//  * S resting at a chain's end with nothing to step onto lets go (Square),
//    as does Ctrl anywhere on a pole or chain (pc_input.cpp). Vanilla is
//    unchanged.
static constexpr uint32_t object_pole=0x400u;
static constexpr double pole_reach=640;
// Duke's feet this far below the chain's top as the mount ends; 154's own
// root motion settles him about 130 higher.
static constexpr double pole_hang_depth=400;
static constexpr unsigned pole_mount_updates=12;
// A floor more than this above Duke's feet is not a step-off for Down.
static constexpr int32_t pole_floor_above=100;
static constexpr unsigned pole_stall_updates=8;
static unsigned pole_mount_starts,pole_mounts,pole_mount_aborts,pole_floor_rejects,pole_stall_drops;
static struct {
    bool active;
    uint32_t object;
    double ax,az,sy,ay;
    int syaw;
    unsigned ticks;
} pole_mount;
static struct {unsigned count; int32_t y; bool stalled;} pole_stall;
static bool pole_object(uint32_t flags) {return (flags&object_pole) && !(flags&0x300u);}
struct PoleAxis {double x,z,top,bottom;};
static bool pole_axis(uint32_t object,PoleAxis& out) {
    int16_t b[8];
    if(!ladder_box(object,b))return false;
    // A column, not a flat panel (ladders and climbing walls have no depth).
    if(b[4]-b[0]<32 || b[6]-b[2]<32 || b[4]-b[0]>512 || b[6]-b[2]>512)return false;
    const double a=(psx_mod_read_half(object+0x1c)&4095)*tau/4096;
    const double nx=std::sin(a),nz=std::cos(a),tx=std::cos(a),tz=-std::sin(a);
    const double lx=(b[0]+b[4])/2.0,lz=(b[2]+b[6])/2.0;
    out.x=(int32_t)psx_mod_read_word(object+4)+lx*tx+lz*nx;
    out.z=(int32_t)psx_mod_read_word(object+12)+lx*tz+lz*nz;
    const double y=(int32_t)psx_mod_read_word(object+8);
    out.top=y+std::min(b[1],b[5]);out.bottom=y+std::max(b[1],b[5]);
    return out.bottom-out.top>=1024;
}
struct PoleTop {uint32_t object; double x,z,y; int yaw;};
// The nearest pole or chain in Duke's cell list whose top is just below the
// floor he stands on, within reach of its axis.
static bool pole_top_find(PoleTop& out) {
    const double px=(int32_t)psx_mod_read_word(player+4),py=(int32_t)psx_mod_read_word(player+8),
        pz=(int32_t)psx_mod_read_word(player+12);
    double best=pole_reach+1;
    uint32_t node=psx_mod_read_word(player+0x220);
    for(int guard=0;node && guard<64;++guard) {
        if(node<0x80010000u || node>=0x80200000u || (node&3))return false;
        const uint32_t object=psx_mod_read_word(node+8);
        node=psx_mod_read_word(node+4);
        if(!pole_object(object_type_flags(object)))continue;
        PoleAxis axis;
        if(!pole_axis(object,axis))continue;
        const double rx=axis.x-px,rz=axis.z-pz,reach=std::hypot(rx,rz),below=axis.top-py;
        if(below<300 || below>800 || reach<120 || reach>=best)continue;
        best=reach;
        out.object=object;out.x=axis.x;out.z=axis.z;out.y=axis.top+pole_hang_depth;
        out.yaw=int(std::lround(std::atan2(rx,rz)*4096/tau))&4095;
    }
    return best<=pole_reach;
}
static void pole_mount_write(double e) {
    const int heading=int(std::lround(pole_mount.syaw+2048*e))&4095;
    const double h=heading*tau/4096;
    psx_mod_write_word(player+4,(int32_t)std::lround(pole_mount.ax-225*std::sin(h)));
    psx_mod_write_word(player+8,(int32_t)std::lround(pole_mount.sy+(pole_mount.ay-pole_mount.sy)*e));
    psx_mod_write_word(player+12,(int32_t)std::lround(pole_mount.az-225*std::cos(h)));
    psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x24,heading);
    for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
}
// Called with a fresh E request on settled ground (ladder_top_update).
static bool pole_mount_start() {
    PoleTop target;
    if(!pole_top_find(target))return false;
    pole_mount={true,target.object,target.x,target.z,(double)(int32_t)psx_mod_read_word(player+8),target.y,target.yaw,0};
    // Exactly the airborne catch's attachment for a 0x400 object.
    psx_mod_write_word(player+0x17c,target.object);psx_mod_write_half(player+0x180,0);
    psx_mod_write_word(player+0x1c4,0);
    psx_mod_write_byte(player+0x22c,3);psx_mod_write_byte(player+0x22d,3);
    psx_mod_write_half(player+0x60,154);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    pole_mount_write(0);
    ++pole_mount_starts;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-pole-top object=%08x axis=%.0f,%.0f from_y=%.0f to_y=%.0f yaw=%d\n",
            target.object,target.x,target.z,pole_mount.sy,target.y,target.yaw);
    return true;
}
static bool pole_rest_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    return (animation==192 || animation==193) && psx_mod_read_byte(player+0x22c)==3 &&
        pole_object(object_type_flags(psx_mod_read_word(player+0x17c)));
}
// Player update (0x8005a210), before the original 154 / hang-climb handler.
static void pole_climb_update() {
    if(pole_mount.active) {
        const unsigned animation=psx_mod_read_half(player+0x60);
        const bool attached=psx_mod_read_byte(player+0x22c)==3 && psx_mod_read_word(player+0x17c)==pole_mount.object;
        if((psx_mod_read_word(player)&0x20002) || !gameplay_context() || !identity() || !input_modernized() ||
           !attached || (animation!=154 && animation!=192 && animation!=193)) {
            pole_mount.active=false;
            if(attached && animation>=192 && animation<=195)++pole_mounts;
            else ++pole_mount_aborts;
            return;
        }
        ++pole_mount.ticks;
        const double u=std::min(1.0,double(pole_mount.ticks)/pole_mount_updates);
        pole_mount_write(u*u*(3-2*u));
        if(pole_mount.ticks>=pole_mount_updates && animation!=154) {pole_mount.active=false;++pole_mounts;}
    }
    // S held at a rest that the original does not leave: the end of the chain.
    const auto& f=input_snapshot(Context::Gameplay);
    const bool waiting=!pole_mount.active && input_modernized() && f.active && traversal_state_ready() &&
        pole_rest_early() && f.held[move_back] && !f.held[move_forward];
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    if(!waiting) pole_stall={0,y,false};
    else {
        if(pole_stall.count && std::abs(y-pole_stall.y)>8) pole_stall.count=0;
        if(!pole_stall.count) pole_stall.y=y;
        if(++pole_stall.count==pole_stall_updates) {
            pole_stall.stalled=true;++pole_stall_drops;
            std::fprintf(stderr,"[TTK pole] S at the end of %08x: let go\n",psx_mod_read_word(player+0x17c));
        }
    }
}
// Entry of 0x8003964c from the down probe (ra 0x8007d7fc): 0x8007d65c has
// stored the level floor at its sp+0x4c and reads it at 0x8007d8d4 for the
// 360 step-off test. Only for the hang-climb's own calls (its saved ra) on a
// pole or chain, direction down (s3 = 1).
void pole_probe_floor(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[19]!=1 || !input_modernized())return;
    const uint32_t sp=cpu->gpr[29];
    if(sp<0x80010100u || sp>0x801fff00u || (sp&3))return;
    const uint32_t caller=psx_mod_read_word(sp+0x64);
    if(caller!=0x80044340u && caller!=0x80044434u)return;
    if(!pole_object(object_type_flags(psx_mod_read_word(player+0x17c))) || !identity())return;
    const int32_t floor=(int32_t)psx_mod_read_word(sp+0x4c),y=(int32_t)psx_mod_read_word(player+8);
    if(floor>=y-pole_floor_above)return;
    psx_mod_write_word(sp+0x4c,(uint32_t)(y+0x8000));
    if(!pole_floor_rejects || std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"[TTK pole] climbing down: floor %d above Duke (%d) is not a step-off\n",floor,y);
    ++pole_floor_rejects;
}
bool pole_mount_finishing() {return pole_mount.active && pole_rest_early();}
bool pole_let_go_ready() {return pole_stall.stalled && pole_sidestep_ready();}
// D08U / D08J5: E at the top of a plain ladder or of a pole or chain.
bool ladder_top_available() {
    if(!ladder_top_ground_ready())return false;
    LadderTop ladder;
    PoleTop pole;
    return ladder_top_find(ladder) || pole_top_find(pole);
}
