// D12A first-person quick kick (Modernized eye view only).
//
// The original boot attack 0x800517a4 (Boot selected: Q, Action at idle, or a
// held attack) picks full-body animation 112..115 at random (rand() & 3).
// Several of them spin Duke round or lean him back, and the eye view has no
// lease through them, so the view cut to the third-person camera. Their hit is
// the dispatcher case 0x80049044, run on every player update of 112..115:
// 0x800a979c(point 7 of the model (joint 16, the right ankle), radius 96,
// type 10, [0x800d21fc] * 20 (* 80 with steroids, player+0x364 bit 1),
// attacker Duke, 0) - a sphere that damages the first object whose box it
// touches, through that object's class handler (hit reaction and push).
//
// In the eye view the kick is instead a short Duke 3D style quick kick:
// - Q starts it with any weapon, standing, moving, crouched or in a jump; the
//   weapon stays drawn. An original request (Boot selected) is turned into it
//   at the lower-body initializer 0x800493a4, before 112..115 is set up, by
//   restarting idle 63 exactly as the original transition does.
// - Duke's own right leg meshes (parts 5..8: hip, knee, ankle, toe; joints
//   14..17 on the first map, 5..8 in the cowboy costume, D12B) are drawn in
//   front of the eye with the current costume's meshes,
//   posed from the straight front kick 115 (chamber and extension, recorded in
//   this build), and drawn on top like the D12 weapon.
// - The same original sphere, with the same radius and damage, is placed along
//   the view (crosshair) direction on every player update while the leg is out
//   (about four updates), as the original does for the foot on every update of
//   112..115; the struck object's own hit reaction pushes it.
// - The original kick sound (0x8006b270 id 0x1000, from the 0x800 event of
//   112..115) plays as the leg snaps out.
// Third person and Vanilla keep the original kick.
static constexpr unsigned leg_joints=4;
// Leg index (0 hip .. 3 toe) of one of Duke's joints, or -1.
static int kick_leg_index(uint32_t desc,unsigned joint) {
    const int part=duke_joint_part(desc,joint);
    return part>=(int)part_right_hip && part<(int)(part_right_hip+leg_joints) ? part-(int)part_right_hip : -1;
}
struct LegKey { double q[4]; double p[3]; };
// Right leg world rotation (quaternion w,x,y,z) and position from the root, in
// Duke's level frame (x right, y down, z ahead), from 115 on the first street.
static constexpr LegKey leg_keys[3][leg_joints]={
    {{{0.9794,0.1609,-0.0216,-0.1201},{31.7,47.0,-12.6}},{{0.9835,-0.1337,0.0146,-0.1213},{79.7,241.0,95.1}},{{0.9999,-0.0008,0.0115,0.0073},{143.6,474.0,1.9}},{{0.9999,-0.0008,0.0115,0.0073},{142.5,505.0,69.1}}}, // rest (115 f0)
    {{{0.6510,0.7383,-0.0916,-0.1505},{41.6,39.0,-10.8}},{{0.9836,-0.0376,0.0586,-0.1663},{42.5,-26.0,208.7}},{{0.9860,0.0866,0.0441,-0.1356},{126.9,214.0,158.9}},{{0.9860,0.0866,0.0441,-0.1356},{137.7,232.0,230.3}}}, // chamber (115 f40)
    {{{0.7043,0.7018,0.0178,-0.1058},{29.2,47.0,-18.4}},{{0.7057,0.7003,0.0182,-0.1058},{63.2,8.0,203.3}},{{0.6243,0.7779,-0.0459,-0.0551},{116.3,31.0,456.5}},{{0.6243,0.7779,-0.0459,-0.0551},{104.2,-41.0,472.9}}}, // extend (115 f49)
};
// Host frames (60 Hz): rest -> chamber, chamber -> extend, hold, extend ->
// chamber, chamber -> rest. The hit window covers the snap and the hold.
static constexpr unsigned kick_chamber=7, kick_extend=11, kick_hold=17, kick_retract=21, kick_frames=27;
static constexpr unsigned kick_hit_first=8, kick_hit_last=20;
static constexpr uint32_t kick_damage_routine=0x800a979c, kick_sound_routine=0x8006b270, kick_sound=0x1000;
static constexpr int kick_radius=96, kick_type=10;
// Crosshair ray for the hit: traced this far; a hit counts when it is within
// the original foot's horizontal reach from the eye (115 extends the ankle
// about 455 ahead of the root) and the sphere is placed this far short of it.
static constexpr double kick_trace_length=1200, kick_horizontal_reach=480, kick_surface_backoff=24;
static bool kick_active, kick_sounded, kick_queued;
static uint64_t kick_start;
static uint32_t kick_point, leg_matrices;
static uint64_t kicks, kick_converts, kick_hit_calls, kick_sounds, kick_leg_draws, kick_cancels, kick_surface_points, kick_fire_starts, kick_thigh_draws, kick_queues, kick_repeats, kick_suppressed;
// Display: the leg's root-relative pose plus an offset from the eye (view
// units: x right, y down, z ahead), turned 12 degrees in about the hip, so the
// leg rises from the lower left and the boot lands just under the crosshair,
// clear of a drawn weapon (tuned 2026-09-30 on the first street).
// Developer tuning only (not saved): DNTTK_FP_KICK=x,y,z offset,
// DNTTK_FP_KICK_TILT=pitch,yaw,roll (degrees, as the weapon tilt),
// DNTTK_FP_KICK_REACH=100..600.
static const double* kick_anchor() {
    static double value[3]={-170,80,160};
    static const bool parsed=[]{
        const char* text=std::getenv("DNTTK_FP_KICK");double v[3];
        if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
            for(int i=0;i<3;++i) value[i]=std::clamp(v[i],-1000.0,1000.0);
        return true;
    }();
    (void)parsed;return value;
}
static const double* kick_tilt() {
    static double value[3]={0,12,0};
    static const bool parsed=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_TILT");double v[3];
        if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
            for(int i=0;i<3;++i) value[i]=std::clamp(v[i],-180.0,180.0);
        return true;
    }();
    (void)parsed;return value;
}
static double kick_reach() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_REACH");char* end=nullptr;
        const double v=text?std::strtod(text,&end):0;
        return text && end && !*end && v>=100 && v<=600 ? v : 340.0;
    }();
    return value;
}
static bool kick_eye_ready() {return fp_blend>=0.5 && orbit_valid && fp_supported && identity();}
static void kick_cancel() {if(kick_active) ++kick_cancels;kick_active=false;kick_queued=false;}
// Q in the eye view, a held attack with the Boot, or a converted original
// request. One kick at a time; a Q during a kick queues the next one, so
// rapid presses chain back to back as a held attack with the Boot does.
static bool kick_request(bool queue) {
    if(!kick_eye_ready()) return false;
    if(kick_active) {if(queue && !kick_queued) {kick_queued=true;++kick_queues;} return true;}
    kick_active=true;kick_sounded=false;kick_queued=false;kick_start=input_host_frame();++kicks;
    return true;
}
// Developer tuning only: DNTTK_FP_KICK_FREEZE=0..26 holds every kick at that
// frame (the leg stays up until the view leaves first person).
static int kick_freeze() {
    static const int value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_FREEZE");char* end=nullptr;
        const long v=text?std::strtol(text,&end,10):-1;
        return text && end && !*end && v>=0 && v<(long)kick_frames ? (int)v : -1;
    }();
    return value;
}
static unsigned kick_frame() {
    if(kick_freeze()>=0) return (unsigned)kick_freeze();
    if(render_override) return override_kick_frame;   // D17 worker: the live frame's value
    const uint64_t now=input_host_frame();
    return now>=kick_start ? (unsigned)std::min<uint64_t>(now-kick_start,kick_frames) : 0;
}
// A guest call with up to four register arguments and two stack arguments, on
// an isolated stack as original_call does.
static bool kick_call(CPUState* source,uint32_t address,const uint32_t (&args)[6]) {
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    uint8_t saved[4096],scratch[1024];
    for(unsigned i=0;i<sizeof saved;++i) saved[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i) scratch[i]=psx_mod_read_byte(0x1f800000+i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
    for(int i=0;i<4;++i) cpu.gpr[4+i]=args[i];
    psx_mod_write_word(cpu.gpr[29]+0x10,args[4]);psx_mod_write_word(cpu.gpr[29]+0x14,args[5]);
    guest_busy=true;psx_dispatch_call(&cpu,address,0x800000fc);guest_busy=false;
    for(unsigned i=0;i<sizeof saved;++i) psx_mod_write_byte(top-4096+i,saved[i]);
    for(unsigned i=0;i<sizeof scratch;++i) psx_mod_write_byte(0x1f800000+i,scratch[i]);
    return cpu.pc==0 && !g_psx_call_bail;
}
// Once per player update (0x80058120, ra 0x80041c44): sound and hits.
static bool boot_selected() {return !(psx_mod_read_word(player+0x3b8)&0x00ff00ffu);}
static void kick_update(CPUState* cpu) {
    // Allocate the kick's guest scratch up front: an allocation changes the
    // mod-memory watermark, which reforks the D17 redraw workers, and doing
    // that at the first kick dropped the in-between frames during it.
    if(!kick_point) kick_point=psx_mod_alloc_guest_memory(16,16);
    if(!leg_matrices) leg_matrices=psx_mod_alloc_guest_memory(0x20*leg_joints,16);
    const auto& f=input_snapshot(Context::Gameplay);
    if(kick_active && !kick_eye_ready()) {kick_cancel();return;}
    if(kick_active && kick_frame()>=kick_frames) {
        // Chain: a queued Q, or Q held (repeats like a held attack).
        const bool again=kick_queued || f.held[quick_kick];
        kick_active=false;kick_queued=false;
        if(again && kick_request()) ++kick_repeats;
    }
    // Boot selected (equipment and selection 0, the mask 0x800517a4 checks):
    // a held attack kicks while moving too. The original only kicks from a
    // standing idle; that request is still converted in kick_convert.
    if(!kick_active && f.held[fire] && boot_selected() && kick_request()) ++kick_fire_starts;
    if(!kick_active) return;
    const unsigned frame=kick_frame();
    if(!kick_sounded && frame>=kick_chamber) {
        kick_sounded=true;
        if(kick_call(cpu,kick_sound_routine,{kick_sound,player+4,0x800,0,0,0})) ++kick_sounds;
    }
    if(frame<kick_hit_first || frame>kick_hit_last) return;
    if(!kick_point) kick_point=psx_mod_alloc_guest_memory(16,16);
    if(!kick_point) return;
    // Along the view (camera row 2, the pitched forward axis). Where the
    // crosshair ray meets a surface or an actor within the foot's reach, the
    // sphere goes there (just short of it), so a low object that fills the
    // crosshair is kicked where it is, not above it.
    double eye[3],forward[3],point[3],end[3],hit[3];
    for(int i=0;i<3;++i) {
        forward[i]=(int16_t)psx_mod_read_half(camera+2*(6+i))/4096.0;
        eye[i]=(int32_t)psx_mod_read_word(camera+0x14+4*i);
        point[i]=eye[i]+forward[i]*kick_reach();
        end[i]=eye[i]+forward[i]*kick_trace_length;
    }
    int kind=0;
    if(view_segment_query(cpu,eye,end,hit,kind) && kind &&
       std::hypot(hit[0]-eye[0],hit[2]-eye[2])<=kick_horizontal_reach) {
        for(int i=0;i<3;++i) point[i]=hit[i]-forward[i]*kick_surface_backoff;
        ++kick_surface_points;
    }
    for(int i=0;i<3;++i) psx_mod_write_word(kick_point+4*i,(uint32_t)(int32_t)std::lround(point[i]));
    psx_mod_write_word(kick_point+12,0);
    const uint32_t delta=psx_mod_read_word(0x800d21fc);
    const uint32_t damage=delta*5*((psx_mod_read_half(player+0x364)&2)?16:4);
    if(kick_call(cpu,kick_damage_routine,{kick_point,(uint32_t)kick_radius,(uint32_t)kick_type,damage,player,0})) ++kick_hit_calls;
}
// 0x800493a4 entry from the lower-body update (ra 0x8005a3a0/0x8005a490/0x8005a4d0):
// an original boot request (112..115, not yet initialized) in the eye view.
// Cross is both the attack and Action: the original also kicks from E (Action)
// at idle with the Boot selected. In the eye view only Q and the attack kick
// (user playtest 2026-09-30), so a request made without the attack held
// restarts idle with no kick.
static void kick_convert(CPUState* cpu,uint32_t ra) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(cpu->gpr[4]!=player || (ra!=0x8005a3a0 && ra!=0x8005a490 && ra!=0x8005a4d0) ||
       animation<112 || animation>115 || psx_mod_read_half(player+0x68)!=0 || !kick_eye_ready()) return;
    if(input_snapshot(Context::Gameplay).held[fire]) {kick_request();++kick_converts;}
    else ++kick_suppressed;
    psx_mod_write_half(player+0x60,63);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
}
static void slerp(const double* a,const double* b,double t,double* out) {
    double d=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3],s=1;
    if(d<0) {d=-d;s=-1;}
    double wa=1-t,wb=t;
    if(d<0.9995) {const double th=std::acos(d),sn=std::sin(th);wa=std::sin((1-t)*th)/sn;wb=std::sin(t*th)/sn;}
    double n=0;
    for(int i=0;i<4;++i) {out[i]=wa*a[i]+wb*s*b[i];n+=out[i]*out[i];}
    n=std::sqrt(n);for(int i=0;i<4;++i) out[i]/=n;
}
// Pose at the current kick frame: two keys and the eased blend between them.
static void kick_pose(unsigned frame,unsigned& from,unsigned& to,double& t) {
    auto ease=[](double x){x=std::clamp(x,0.0,1.0);return x*x*(3-2*x);};
    if(frame<kick_chamber) {from=0;to=1;t=ease(frame/(double)kick_chamber);}
    else if(frame<kick_extend) {from=1;to=2;t=ease((frame-kick_chamber)/(double)(kick_extend-kick_chamber));}
    else if(frame<kick_hold) {from=to=2;t=0;}
    else if(frame<kick_retract) {from=2;to=1;t=ease((frame-kick_hold)/(double)(kick_retract-kick_hold));}
    else {from=1;to=0;t=ease((frame-kick_retract)/(double)(kick_frames-kick_retract));}
}
// 0x800292a0 from the joint loop (ra 0x80034c6c) for a right-leg joint while
// the kick plays: a private matrix placing that joint in front of the eye.
// Developer switch: DNTTK_FP_KICK_THIGH=0 leaves the thigh to the loop's near cull.
static bool kick_thigh_enabled() {
    static const bool value=[]{const char* t=std::getenv("DNTTK_FP_KICK_THIGH");return !(t && t[0]=='0' && !t[1]);}();
    return value;
}
// The thigh alone sits lower on screen by this fraction of the screen height
// (240 lines), measured at its knee-end depth (user playtests 2026-09-30:
// 10% first, 13% chosen).
// Developer override: DNTTK_FP_KICK_THIGH_DROP=0..0.5.
static double kick_thigh_drop() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_FP_KICK_THIGH_DROP");char* end=nullptr;
        const double v=text?std::strtod(text,&end):-1;
        return text && end && !*end && v>=0 && v<=0.5 ? v : 0.13;
    }();
    return value;
}
// A leg joint's level-frame rotation and root-relative position at a kick frame.
static void leg_pose(unsigned leg,unsigned frame,double (&lh)[3][3],double* p) {
    unsigned from,to;double t;
    kick_pose(frame,from,to,t);
    const LegKey& a=leg_keys[from][leg];const LegKey& b=leg_keys[to][leg];
    double q[4];
    slerp(a.q,b.q,t,q);quat_mat(q,lh);
    for(int i=0;i<3;++i) p[i]=a.p[i]+(b.p[i]-a.p[i])*t;
    // Tilt about the hip in the level frame, as the weapon tilt.
    const double* tilt=kick_tilt();
    if(tilt[0] || tilt[1] || tilt[2]) {
        const double d=3.14159265358979/180,pt=-tilt[0]*d,y=tilt[1]*d,r=tilt[2]*d;
        const double rx[3][3]={{1,0,0},{0,std::cos(pt),-std::sin(pt)},{0,std::sin(pt),std::cos(pt)}};
        const double ry[3][3]={{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}};
        const double rz[3][3]={{std::cos(r),-std::sin(r),0},{std::sin(r),std::cos(r),0},{0,0,1}};
        double m[3][3];mat_mul(ry,rx,m);mat_mul(m,rz,m);mat_mul(m,lh,lh);
        double hip[3],rel[3];
        for(int i=0;i<3;++i) {hip[i]=leg_keys[from][0].p[i]+(leg_keys[to][0].p[i]-leg_keys[from][0].p[i])*t;rel[i]=p[i]-hip[i];}
        for(int i=0;i<3;++i) p[i]=hip[i]+m[i][0]*rel[0]+m[i][1]*rel[1]+m[i][2]*rel[2];
    }
}
static bool kick_leg_joint(CPUState* cpu) {
    if(!kick_active || cpu->gpr[4]!=camera || cpu->gpr[20]!=player) return false;
    const unsigned joint=cpu->gpr[23];
    const uint32_t mats=psx_mod_read_word(player+0x3c),desc=psx_mod_read_word(player+0x40);
    if(mats<0x80010000 || mats>0x801fff00 || desc!=duke_model() || !desc ||
       cpu->gpr[5]!=mats+0x20*joint) return false;
    const int leg=kick_leg_index(desc,joint);
    if(leg<0) return false;
    if(!leg_matrices) leg_matrices=psx_mod_alloc_guest_memory(0x20*leg_joints,16);
    if(!leg_matrices) return false;
    const uint32_t out_m=leg_matrices+0x20*leg;
    const unsigned frame=kick_frame();
    double p[3],lh[3][3];
    leg_pose(leg,frame,lh,p);
    if(leg==0 && kick_thigh_enabled()) {
        // The thigh's origin (the hip) is nearer than the loop's near cull
        // (GTE H) from the eye, so the loop would skip the whole thigh. Draw
        // it end for end instead: origin at the knee (which passes the cull),
        // turned 180 degrees about an axis across the thigh, so the same mesh
        // runs from the knee back toward the hip and off the bottom edge.
        double knee[3],kl[3][3];
        leg_pose(1,frame,kl,knee);
        double d[3],u[3],n=0;
        for(int i=0;i<3;++i) d[i]=knee[i]-p[i];
        for(int i=0;i<3;++i) {u[i]=lh[0][i]*d[0]+lh[1][i]*d[1]+lh[2][i]*d[2];n+=u[i]*u[i];} // thigh axis, local
        n=std::sqrt(n);
        if(n>1) {
            for(double& v:u) v/=n;
            // Across the thigh: local x unless the thigh runs along it.
            double a[3]={1,0,0};
            if(std::abs(u[0])>0.9) {a[0]=0;a[1]=1;}
            double w[3]={u[1]*a[2]-u[2]*a[1],u[2]*a[0]-u[0]*a[2],u[0]*a[1]-u[1]*a[0]},wn=0;
            for(double v:w) wn+=v*v;
            wn=std::sqrt(wn);for(double& v:w) v/=wn;
            double flip[3][3];
            for(int r=0;r<3;++r) for(int k=0;k<3;++k) flip[r][k]=2*w[r]*w[k]-(r==k);
            mat_mul(lh,flip,lh);
            for(int i=0;i<3;++i) p[i]=knee[i];
            // Screen lines to view units at this depth: y = lines * z / H (H 256).
            const double depth=p[2]+kick_anchor()[2];
            if(depth>0) p[1]+=kick_thigh_drop()*240*depth/256;
            ++kick_thigh_draws;
        }
    }
    double c[3][3];
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) c[r][k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k))/4096.0;
    // Shown pitched with the view, like the weapon: stored lh^T C.
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) {
        const double v=lh[0][r]*c[0][k]+lh[1][r]*c[1][k]+lh[2][r]*c[2][k];
        psx_mod_write_half(out_m+2*(3*r+k),(uint16_t)(int16_t)std::clamp(std::lround(v*4096),-32768L,32767L));
    }
    psx_mod_write_half(out_m+0x12,0);
    const double* anchor=kick_anchor();
    for(int i=0;i<3;++i) {
        double w=(int32_t)psx_mod_read_word(camera+0x14+4*i);
        for(int k=0;k<3;++k) w+=c[k][i]*(p[k]+anchor[k]);
        psx_mod_write_word(out_m+0x14+4*i,(uint32_t)(int32_t)std::lround(w));
    }
    fade_clear(cpu);
    cpu->gpr[5]=out_m;weapon_viewmodel=true;++kick_leg_draws;
    return true;
}
static const char* kick_debug_json() {
    static char buffer[560];
    std::snprintf(buffer,sizeof buffer,"{\"active\":%s,\"frame\":%u,\"kicks\":%llu,\"converts\":%llu,\"fire_starts\":%llu,\"queues\":%llu,\"repeats\":%llu,\"suppressed\":%llu,\"hits\":%llu,\"surface_points\":%llu,\"sounds\":%llu,\"leg_draws\":%llu,\"thigh_draws\":%llu,\"cancels\":%llu,\"reach\":%.0f}",
        kick_active?"true":"false",kick_active?kick_frame():0u,(unsigned long long)kicks,(unsigned long long)kick_converts,(unsigned long long)kick_fire_starts,
        (unsigned long long)kick_queues,(unsigned long long)kick_repeats,(unsigned long long)kick_suppressed,(unsigned long long)kick_hit_calls,(unsigned long long)kick_surface_points,(unsigned long long)kick_sounds,(unsigned long long)kick_leg_draws,(unsigned long long)kick_thigh_draws,(unsigned long long)kick_cancels,kick_reach());
    return buffer;
}
