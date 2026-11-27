// D08J10 - LADDER CLIMBING (OPTIONS > ADVANCED > MOVEMENT; Modernized
// DNTTK_LADDER_CLIMBING modern, the default, or classic; Vanilla always
// classic). The original takes about a second to get Duke onto a ladder before
// the climb starts, reaching with his hands until his feet find the rungs.
// Measured on the first alley ladder (LEVEL00 object type 46 at 9225, -10239,
// 13769), at the game's 30 player updates a second:
//  * from the floor, W + E: the ground mount 185 (mode 8 after 0), 32 updates
//    (1.07 s) for 266 units of rise; the climb 188/189 then rises 219-299 a
//    step every 11 updates;
//  * a jump with E: the catch 147/148 (mode 3, 16-17 updates; in 148 the body
//    swings down about 230 as the hands take the weight and back up) and the
//    feet transfer 156 (11-12 updates), 0.93 s before the climb;
//  * a catch below a high bottom hangs in 207 (it waits for W) and W plays 210
//    into 156.
// MODERN enters the ladder through Duke's own hand-over-hand climb instead
// (the user, 2026-11-26, after a 2.5x speed-up of those clips: "it's just the
// low rungs anim speeded up ... more like his normal hand climb style like
// muscling himself up"):
//  * the ground mount: on 185's first update Duke is attached as the catches
//    attach him (mode 3, box +0x180 = 0, sub-state +0x1c4 = 0) in the climb
//    step 188 at his standing height; the step lifts him onto the rungs;
//  * a catch: 147/148 play at their own pace until the swing bottoms out (the
//    root stops going down) and hand over to 188 from that low point, so he
//    pulls himself up out of the catch; 156 and 210, if reached, start 188 at
//    once.
// The game interpolates from the frame shown into 188's first frame. The host
// eases Duke onto the climbing line (200 from the panel, facing it) over the
// step's opening hold (3 updates), from where he stood, where the original
// snapped him onto the ladder's centre at the start of 185 and onto the line at
// its end (the rest pose 186 snaps the same way, so the entry never uses it).
// Third candidate (user, 2026-11-27: "the top part of his body is great ...
// hes miming as if he's on the rungs when hes not"): during that ease the
// ground mount's legs are the host clip from ttk_ladder_entry.py (the right
// foot planted where he stood, the left stepping onto the bottom rung), then
// 188's legs take over, which keep that foot on the rung. The
// climb then belongs to the original handler. Not changed: the step-off at the
// bottom (185 from mode 3), the D08U top mount (156 under its host blend), the
// hang 207 and the pole, chain and wall clips (a plain ladder attachment only).
// Developer A/B: DNTTK_LADDER_SETTLE (updates, default 3), DNTTK_LADDER_CATCH_MAX
// (the latest catch update to hand over, default 10).
static long ladder_env_long(const char* name,long fallback,long lo,long hi) {
    const char* t=std::getenv(name);if(!t || !*t)return fallback;
    const long v=std::strtol(t,nullptr,10);return v<lo||v>hi?fallback:v;
}
static bool ladder_climbing_modern() {
    const char* v=ttk_live_env("DNTTK_LADDER_CLIMBING");
    return input_modernized() && !(v && !std::strcmp(v,"classic"));
}
static constexpr unsigned ladder_entry_clip=188;
static unsigned ladder_last_mode;
static struct {bool active;uint32_t object;double sx,sz,ax,az;int syaw,ayaw;unsigned ticks,updates;double dy,done;} ladder_settle;
static struct {uint32_t object;int32_t y,y0;unsigned ticks;unsigned clip;} ladder_catch;
static unsigned ladder_ground_entries,ladder_catch_entries,ladder_transfer_entries;
// The ground mount's legs (ttk_ladder_entry.py, host extra clip 61): keys
// 0..ladder_entry_keys, one per player update from the conversion, while the
// ease brings Duke onto the line (the right foot stays planted, the left steps
// onto the bottom rung); then 188's live legs take over across
// ladder_entry_handback updates.
static constexpr unsigned ladder_entry_leg_clip=61,ladder_entry_keys=3,ladder_entry_handback=4;
static struct {bool active;unsigned u;} ladder_legs;
static unsigned ladder_leg_writes;
// Where Duke stood at the last update on the floor: the original 185 places
// him on the ladder's centre line 275 from the panel at once (up to about 200
// sideways after an angled approach); the ease starts from here instead.
static struct {bool valid;int32_t x,z;uint16_t yaw;} ladder_floor_pose;
// The climbing line on Duke's side of a plain ladder panel.
static bool ladder_line(uint32_t object,double& x,double& z,int& yaw) {
    int16_t b[8];
    if(!ladder_box(object,b))return false;
    const double a=(psx_mod_read_half(object+0x1c)&4095)*tau/4096;
    const double nx=std::sin(a),nz=std::cos(a),tx=std::cos(a),tz=-std::sin(a);
    const double lx=(b[0]+b[4])/2.0,lz=(b[2]+b[6])/2.0;
    const double cx=(int32_t)psx_mod_read_word(object+4)+lx*tx+lz*nx;
    const double cz=(int32_t)psx_mod_read_word(object+12)+lx*tz+lz*nz;
    const double px=(int32_t)psx_mod_read_word(player+4),pz=(int32_t)psx_mod_read_word(player+12);
    const double side=(px-cx)*nx+(pz-cz)*nz>0?1:-1;
    x=cx+side*200*nx;z=cz+side*200*nz;
    yaw=int(std::lround(std::atan2(-side*nx,-side*nz)*4096/tau))&4095;
    return true;
}
// x, z and heading are written outright; the height change dy is added in
// steps on top of the climb's own root motion.
static void ladder_settle_write(double e) {
    auto& m=ladder_settle;
    const double step=std::lround(m.dy*e)-m.done;
    if(step) {psx_mod_write_word(player+8,(int32_t)psx_mod_read_word(player+8)+int32_t(step));m.done+=step;}
    psx_mod_write_word(player+4,(int32_t)std::lround(m.sx+(m.ax-m.sx)*e));
    psx_mod_write_word(player+12,(int32_t)std::lround(m.sz+(m.az-m.sz)*e));
    const int turn=((m.ayaw-m.syaw+2048)&4095)-2048;
    const int heading=int(std::lround(m.syaw+turn*e))&4095;
    psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x24,heading);
    for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
}
// Attach Duke in the climb step where he is and start the ease to the line
// (and, from a catch, up by dy to where his hands keep their rung).
static bool ladder_enter_climb(uint32_t object,double dy=0) {
    double x,z;int yaw;
    if(!ladder_line(object,x,z,yaw))return false;
    ladder_settle={true,object,(double)(int32_t)psx_mod_read_word(player+4),(double)(int32_t)psx_mod_read_word(player+12),
        x,z,int(psx_mod_read_half(player+0x1c)&4095),yaw,0,unsigned(ladder_env_long("DNTTK_LADDER_SETTLE",ladder_entry_keys,1,30)),dy,0};
    psx_mod_write_word(player+0x17c,object);psx_mod_write_half(player+0x180,0);psx_mod_write_word(player+0x1c4,0);
    psx_mod_write_byte(player+0x22c,3);psx_mod_write_byte(player+0x22d,3);
    psx_mod_write_half(player+0x60,ladder_entry_clip);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-ladder-entry object=%08x from=%.0f,%.0f to=%.0f,%.0f yaw=%d\n",object,ladder_settle.sx,ladder_settle.sz,x,z,yaw);
    return true;
}
// The ground mount: back to where Duke stood (the original 185 has just
// placed him on the ladder's centre line), then into the climb.
static void ladder_ground_convert(uint32_t object,const decltype(ladder_floor_pose)& floor_pose) {
    if(floor_pose.valid && std::hypot(double(floor_pose.x-(int32_t)psx_mod_read_word(player+4)),
                                      double(floor_pose.z-(int32_t)psx_mod_read_word(player+12)))<400) {
        psx_mod_write_word(player+4,floor_pose.x);psx_mod_write_word(player+12,floor_pose.z);
        psx_mod_write_half(player+0x1c,floor_pose.yaw);psx_mod_write_half(player+0x24,floor_pose.yaw);
    }
    if(ladder_enter_climb(object)) {++ladder_ground_entries;ladder_last_mode=3;ladder_legs={true,0};}
}
// After a catch the hands climb with 188 while the body hangs (user,
// 2026-11-27: "the legs can dangle or sway naturally until they are high
// enough for him to put his feet on the rungs"; then "the exact hang that
// duke should do is when you mantle something but leave it idle ... this hang
// while idle, and a slight sway while climbing"). Below the hands Duke takes
// the original ledge hang 149 (legs clip 249, shared with the bottom hang 207):
// its pelvis and legs, with their own slow idle (four frames, 150 ticks each,
// about 1.8 s). The climb clips roll the pelvis 10-12 degrees (187, 188/189),
// which on their own turned the hanging legs to one side; while hanging the
// pelvis is the hang's, and the chest is turned back by the same amount, so
// the climbing upper body keeps its world pose exactly. While a climb step
// runs (188/189) a share of the climb's own pelvis roll comes through as the
// sway (ladder_sway_share, eased over ladder_sway_ms). Each leg joins 188's
// once the toe 188/189 would place is on or above the ladder's lowest rung
// (toe height bottom - 234, measured), over ladder_dangle_ramp updates; the
// pelvis goes back to the climb's as the legs do. Toe heights below the
// pelvis per frame of 186-189's legs, left (ROT 4) and right (ROT 8), from
// ttk_anim.py poses (recomp/analysis/d08j10/feet.py table).
static constexpr int16_t ladder_toe_below[4][4][2]={
    {{292,535},{292,536},{291,536},{289,540}},   // 186
    {{555,352},{557,349},{557,348},{557,347}},   // 187
    {{289,535},{435,455},{555,313},{555,313}},   // 188
    {{555,313},{454,407},{283,526},{283,526}}};  // 189
static constexpr double ladder_rung0=234,ladder_rung_reach=30,ladder_dangle_ramp=4,ladder_dangle_in=3;
static constexpr unsigned ladder_hang_anim=149;
static constexpr double ladder_hang_period_ms=1800,ladder_sway_ms=300,ladder_sway_share=0.35;
static struct {bool active;uint32_t object;int32_t bottom;unsigned u;double w[2];bool from_set;CrouchQuat from[9];double sway,last_ms;} ladder_dangle;
static unsigned ladder_dangle_starts,ladder_dangle_writes;
static void ladder_dangle_start(uint32_t object) {
    int16_t b[8];
    if(!ladder_box(object,b)){ladder_dangle.active=false;return;}
    ladder_dangle={true,object,(int32_t)psx_mod_read_word(object+8)+std::max<int32_t>(b[1],b[5]),0,{0,0},false,{},0,-1};
    ++ladder_dangle_starts;
}
// Channel 0's keyframe position on 186..189: (frame, next frame, t).
static bool ladder_climb_phase(unsigned a,unsigned& kp,unsigned& kc,double& t) {
    if(a<186 || a>189)return false;
    const int clip=int16_t(psx_mod_read_half(0x800c2d64+8*a));
    if(clip<0 || clip>=2048)return false;
    const uint32_t first=psx_mod_read_half(0x800d8328+4*clip),count=psx_mod_read_half(0x800d8328+4*clip+2);
    const uint32_t pf=psx_mod_read_half(player+0x64),cf=psx_mod_read_half(player+0x66);
    if(cf<first || cf>=first+count || count>4)return false;
    kc=cf-first;kp=(pf>=first && pf<first+count)?pf-first:kc;
    const uint32_t ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*cf)>>16,left=psx_mod_read_half(player+0x62);
    t=ticks ? std::clamp(double(int32_t(ticks)-int32_t(left))/double(ticks),0.0,1.0) : 1.0;
    return true;
}
static void ladder_dangle_update(unsigned a,unsigned mode) {
    auto& d=ladder_dangle;
    if(!d.active)return;
    if(mode!=3 || psx_mod_read_word(player+0x17c)!=d.object || a<186 || a>189 || !identity()) {d.active=false;return;}
    ++d.u;
    unsigned kp,kc;double t;
    if(!ladder_climb_phase(a,kp,kc,t))return;
    const double root=(int32_t)psx_mod_read_word(player+8);
    for(int leg=0;leg<2;++leg) {
        const auto& row=ladder_toe_below[a-186];
        const double toe=root+row[kp][leg]+(row[kc][leg]-row[kp][leg])*t;
        // Down is +y: the foot reaches once it is no lower than the lowest rung.
        if(d.w[leg]>0 || toe<=d.bottom-ladder_rung0+ladder_rung_reach)d.w[leg]=std::min(1.0,d.w[leg]+1/ladder_dangle_ramp);
    }
    if(d.w[0]>=1 && d.w[1]>=1)d.active=false;
}
static CrouchQuat ladder_q(const std::array<int32_t,4>& v) {return crouch_qnorm({double(v[0]),double(v[1]),double(v[2]),double(v[3])});}
// At 0x800411b8: the hang's pelvis and legs while a leg cannot reach a rung.
static void ladder_dangle_pose() {
    auto& d=ladder_dangle;
    if(!d.active)return;
    const uint32_t desc=duke_model();
    const int clip=int16_t(psx_mod_read_half(0x800c2d64+8*ladder_hang_anim));
    if(!desc || clip<0 || clip>=2048)return;
    const uint32_t first=psx_mod_read_half(0x800d8328+4*clip),count=psx_mod_read_half(0x800d8328+4*clip+2);
    if(!count || count>16)return;
    const double now=input_clock_ms();
    const double phase=std::fmod(now,ladder_hang_period_ms)/ladder_hang_period_ms*count;
    const unsigned k=unsigned(phase)%count;const double f=phase-std::floor(phase);
    std::array<std::array<int32_t,4>,32> qa{},qb{};uint32_t ma=0,mb=0;
    if(!crouch_frame_quats(first+k,qa,ma) || !crouch_frame_quats(first+(k+1)%count,qb,mb))return;
    const uint32_t pelvis_r=crouch_joint_record(desc,0),chest_r=crouch_joint_record(desc,9);
    if(!pelvis_r || !chest_r)return;
    const unsigned anim=psx_mod_read_half(player+0x60);
    const double dt=d.last_ms<0?0:std::clamp(now-d.last_ms,0.0,100.0);
    d.last_ms=now;
    d.sway=std::clamp(d.sway+((anim==188 || anim==189)?1:-1)*dt/ladder_sway_ms,0.0,1.0);
    const double es=d.sway*d.sway*(3-2*d.sway);
    if(!d.from_set) {
        for(unsigned j=0;j<=8;++j){const uint32_t r=crouch_joint_record(desc,j);d.from[j]=r?crouch_record_q(r):CrouchQuat{0,0,0,1};}
        d.from_set=true;
    }
    const double in=std::min(1.0,d.u/ladder_dangle_in),ein=in*in*(3-2*in);
    auto hang=[&](unsigned j){
        const auto& sa=(ma>>j&1)?qa[j]:qb[j];const auto& sb=(mb>>j&1)?qb[j]:qa[j];
        return crouch_slerp(ladder_q(sa),ladder_q(sb),f);};
    // Pelvis: the hang's, with part of the climb's roll as the sway; back to
    // the climb's as the legs reach the rungs.
    const CrouchQuat pelvis_live=crouch_record_q(pelvis_r),chest_live=crouch_record_q(chest_r);
    const double reached=(d.w[0]+d.w[1])/2,pkeep=1-reached*reached*(3-2*reached);
    if((ma|mb)&1) {
        const CrouchQuat target=crouch_slerp(d.from[0],crouch_slerp(hang(0),pelvis_live,ladder_sway_share*es),ein);
        const CrouchQuat pelvis_now=crouch_slerp(pelvis_live,target,pkeep);
        crouch_record_write(pelvis_r,pelvis_now);
        // The chest keeps its world rotation (world = local (x) parent, game order).
        crouch_record_write(chest_r,crouch_qnorm(crouch_qmul(crouch_qmul(chest_live,pelvis_live),crouch_qinv(pelvis_now))));
    }
    for(unsigned j=1;j<=8;++j) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r || !((ma|mb)>>j&1))continue;
        const double v=d.w[j<=4?0:1],keep=1-v*v*(3-2*v);
        if(keep<=0)continue;
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),crouch_slerp(d.from[j],hang(j),ein),keep));
    }
    ++ladder_dangle_writes;
}
// The original sets 185 after the animation update, later in the same player
// update, so the hook below would only see it one update later, after a
// frame drawn at the original's placement. Checked again at 0x800411b8
// (before the model build): converted there, that frame never shows.
static unsigned ladder_late_converts;
static void ladder_entry_late(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634 || ladder_last_mode!=0)return;
    if(psx_mod_read_half(player+0x60)!=185 || psx_mod_read_byte(player+0x22c)!=8)return;
    if(!ladder_climbing_modern() || !player_identity_ready() || mount.active)return;
    const uint32_t object=psx_mod_read_word(player+0x17c);
    if(!plain_ladder(object_type_flags(object)))return;
    ladder_ground_convert(object,ladder_floor_pose);++ladder_late_converts;
}
// Once per player update, at the animation update 0x8005a210 (after the
// game's handler for this update, before the animation runner). The handler
// has already moved the mode history on (+0x22d reads 8 from the ground
// mount's first update), so the mode seen at the last update tells the ground
// mount (after mode 0) from the step-off (after mode 3).
static void ladder_entry_update() {
    const unsigned a=psx_mod_read_half(player+0x60),mode=psx_mod_read_byte(player+0x22c),last=ladder_last_mode;
    ladder_last_mode=mode;
    const auto floor_pose=ladder_floor_pose;
    ladder_floor_pose={mode==0,(int32_t)psx_mod_read_word(player+4),(int32_t)psx_mod_read_word(player+12),uint16_t(psx_mod_read_half(player+0x1c)&4095)};
    // A conversion is shown at key 0 in its own update; each later update is
    // the next key, in step with the ease's ticks.
    if(ladder_legs.active) {
        if(++ladder_legs.u>ladder_entry_keys+ladder_entry_handback || mode!=3 || a<186 || a>189)ladder_legs.active=false;
    }
    ladder_dangle_update(a,mode);
    auto& m=ladder_settle;
    if(m.active) {
        const bool attached=mode==3 && psx_mod_read_word(player+0x17c)==m.object && a>=186 && a<=189;
        // A rest pose (186/187: the climb was refused, near the top or under
        // an enemy there) is placed on the line by the original handler.
        if(!attached || a==186 || a==187 || !identity()) {m.active=false;return;}
        ++m.ticks;
        const double u=std::min(1.0,double(m.ticks)/m.updates);
        ladder_settle_write(u*u*(3-2*u));
        if(m.ticks>=m.updates)m.active=false;
        return;
    }
    const uint32_t object=psx_mod_read_word(player+0x17c);
    const bool catching=mode==3 && (a==147 || a==148);
    if(!catching)ladder_catch={0,0,0,0,0};
    if(!ladder_climbing_modern() || !player_identity_ready() || mount.active ||
       !plain_ladder(object_type_flags(object)))return;
    if(a==185 && mode==8 && last==0) {ladder_ground_convert(object,floor_pose);return;}
    // The feet transfer rises about 85 before the climb starts (148 and 147
    // routes, measured): the climb starts that much higher.
    if(mode==3 && (a==156 || a==210)) {
        if(ladder_enter_climb(object,-85)) {++ladder_transfer_entries;ladder_dangle_start(object);}
        return;
    }
    if(!catching)return;
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    if(ladder_catch.object!=object) {ladder_catch={object,y,y,0,a};return;}
    ++ladder_catch.ticks;
    // Down is +y. The swing has bottomed out once the root stops going down.
    const bool rising=ladder_catch.ticks>=2 && y<=ladder_catch.y;
    ladder_catch.y=y;
    if(rising || ladder_catch.ticks>=unsigned(ladder_env_long("DNTTK_LADDER_CATCH_MAX",10,1,40))) {
        // Where the original's climb starts, from the catch height (classic,
        // measured): 102 below it after the running catch 148, 67 above it
        // after the standing catch 147. Only ever up, at most 300.
        const double target=ladder_catch.y0+(ladder_catch.clip==147?-67:102);
        if(ladder_enter_climb(object,std::clamp(target-y,-300.0,0.0))) {++ladder_catch_entries;ladder_dangle_start(object);}
        ladder_catch={0,0,0,0,0};
    }
}
// At 0x800411b8 (ra 0x80042634, after every channel blend), as the other
// host poses: the ground mount's legs over 188's (joints 1-8 only; pelvis,
// root and the upper body stay the game's).
static void ladder_entry_pose(CPUState* cpu) {
    ladder_entry_late(cpu);
    if(cpu->gpr[4]==player && cpu->gpr[31]==0x80042634)ladder_dangle_pose();
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634 || !ladder_legs.active)return;
    const uint32_t desc=duke_model();
    uint32_t first,count;
    if(!desc || !ttk::anim_extra_clip(ladder_entry_leg_clip,first,count) || count!=ladder_entry_keys+2)return;
    const unsigned u=ladder_legs.u,key=std::min(u,ladder_entry_keys);
    std::array<std::array<int32_t,4>,32> q{};uint32_t mask=0;
    if(!crouch_frame_quats(first+1+key,q,mask))return;
    double w=1;
    if(u>ladder_entry_keys) {const double v=double(u-ladder_entry_keys)/ladder_entry_handback;w=1-v*v*(3-2*v);}
    if(w<=0)return;
    for(unsigned j=1;j<=8;++j) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r || !(mask>>j&1))continue;
        const CrouchQuat target=crouch_qnorm({double(q[j][0]),double(q[j][1]),double(q[j][2]),double(q[j][3])});
        crouch_record_write(r,crouch_slerp(crouch_record_q(r),target,w));
    }
    ++ladder_leg_writes;
}
static const char* ladder_grab_debug_json() {
    static char buffer[320];
    std::snprintf(buffer,sizeof buffer,"{\"modern\":%s,\"ground\":%u,\"catch\":%u,\"transfer\":%u,\"settle\":%s,\"legs\":%s,\"leg_writes\":%u,\"late\":%u,\"dangle\":%s,\"dangle_w\":[%.2f,%.2f],\"dangles\":%u,\"dangle_writes\":%u,\"sway\":%.2f}",
        ladder_climbing_modern()?"true":"false",ladder_ground_entries,ladder_catch_entries,ladder_transfer_entries,
        ladder_settle.active?"true":"false",ladder_legs.active?"true":"false",ladder_leg_writes,ladder_late_converts,ladder_dangle.active?"true":"false",ladder_dangle.w[0],ladder_dangle.w[1],ladder_dangle_starts,ladder_dangle_writes,ladder_dangle.sway);
    return buffer;
}
