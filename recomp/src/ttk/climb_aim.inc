// D08J6B: the hang-and-aim pose on climbs, third person (note 377 stage B).
// Included from modern_controls.cpp after swim.inc. D08J6A draws the sidearm
// on a ladder, chain/pole or climbing wall. The original keeps the climb clip
// on the legs, the off-hand arm and the chest (channels 0, 2, 3) and puts the
// sidearm on the gun arm (channel 1), so the off hand already holds the
// climb. What it lacks: its aim build (0x80097c04) in modes 3/6/7 aims only
// the head (0x80097a44) and, in the aim poses 8/11, the gun shoulder
// (0x800978f8, the direction as the bone's Y axis), both along Duke's own
// heading +0x124, which faces the ladder, chain or wall; the chest never
// turns. Shots already follow the view (D08J6A), so the arm and the round
// disagreed.
//
// Modernized, view aiming, third person, sidearm out on a mode 3 climb (and
// D08J6D's hangs):
// - the chest turns toward the view within climb_chest_max (a yaw about the
//   world vertical at its composition, 0x800b42ec);
// - the off-hand shoulder keeps its climb-clip world orientation (the inverse
//   turn), so the arm and hand stay where the clip holds the rung, shifted
//   only by the turned shoulder's position;
// - the head looks toward the view within climb_head_max, firing or not;
// - while the original aims (firing), the head look-at and the one-arm aim
//   take the view direction clamped to the gun arm's arc (right of the
//   heading up to climb_gun_right, across the body to climb_gun_left). The
//   look-at's direction argument is replaced at its hook; the one-arm aim has
//   no entry hook (codegen-fixed), so Duke's aim vector +0x124 (set from the
//   heading, 0x8003fcc8) holds the direction only for the gun shoulder's
//   build and is restored at the next joint's composition (its forearm), or
//   at the first restore point after the build.
// Weight ramps over climb_aim_ramp_ms of real time (the same at 60/120/180),
// out again for the stow (upper 7). No game memory but the build's own
// scratch matrices and the aim direction arguments is written. D08J6D: the
// same pose in the settled ledge hang (mode 6, off hand on the lip) and under
// the ceiling monkey bars (mode 7, off hand on the bar); the original keeps
// the same channel split there. First person is D08J6C.
// DNTTK_CLIMB_AIM=0 keeps the D08J6A pose (developer A/B).
static constexpr double climb_aim_ramp_ms=160;
static double climb_chest_max=50*tau/360,climb_head_max=70*tau/360,
    climb_gun_right=110*tau/360,climb_gun_left=60*tau/360,climb_pitch_max=70*tau/360;
static double climb_aim_weight,climb_aim_last_ms=-1;
static bool climb_aim_set;
// The turn this build applies (radians, right positive) and the clamped aim.
static double climb_chest_yaw,climb_head_yaw,climb_aim_dir[3];
static double climb_view_delta;
static uint32_t climb_aim_vector;
static int32_t climb_saved_aim[3];
// Off-hand IK progress in the current build (0 idle, 1 shoulder solved, 2 forearm).
static bool climb_rf_valid;
static int climb_ik_stage;
static bool climb_aim_lent;
static unsigned climb_aim_lends,climb_aim_returns;
static void climb_aim_return() {
    if(!climb_aim_lent)return;
    climb_aim_lent=false;
    for(int i=0;i<3;++i)psx_mod_write_word(player+0x124+4*i,(uint32_t)climb_saved_aim[i]);
    ++climb_aim_returns;
}
static unsigned climb_aim_builds,climb_chest_turns,climb_offhand_holds,climb_head_turns,climb_look_aims;
// Developer tuning, degrees: DNTTK_CLIMB_AIM_LIMITS=chest,head,gun_right,gun_left,pitch.
static void climb_aim_limits() {
    static bool read;
    if(read)return;
    read=true;
    const char* v=std::getenv("DNTTK_CLIMB_AIM_LIMITS");
    double l[5];
    if(v && std::sscanf(v,"%lf,%lf,%lf,%lf,%lf",&l[0],&l[1],&l[2],&l[3],&l[4])==5) {
        climb_chest_max=l[0]*tau/360;climb_head_max=l[1]*tau/360;climb_gun_right=l[2]*tau/360;
        climb_gun_left=l[3]*tau/360;climb_pitch_max=l[4]*tau/360;
    }
}
// Sidearm out on a climb, in a ledge hang or under a ceiling (D08J6D) under
// view aiming (the shots' own rule, climb_shot_ready, plus the presentation
// conditions of the ground aim).
static bool climb_aim_ready() {
    const char* mode=ttk_live_env("DNTTK_WEAPON_AIM");
    return fp_blend==0 && climb_weapon_ready() &&
        psx_mod_read_byte(player+0x3b8)==2 && base_weapon(psx_mod_read_byte(player+0x3b9))==climb_sidearm &&
        independent_camera() && mode && !std::strcmp(mode,"view") &&
        !input_snapshot(Context::Gameplay).held[original_aim] && !(psx_mod_read_word(player+0x224)&0x80);
}
static void climb_view_forward(double* d) {
    for(int i=0;i<3;++i)d[i]=(int16_t)psx_mod_read_half(camera+12+2*i);
    if(free_look.active) {double eye[3];free_look_aim(eye,d);}
    const double n=std::sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
    if(n>0)for(int i=0;i<3;++i)d[i]/=n;
}
// 0x800411b8 entry (ra 0x80042634), the last call before the build choice.
static void climb_aim_begin(CPUState* cpu,uint32_t ra) {
    climb_aim_return();climb_aim_set=false;
    if(cpu->gpr[4]!=player || ra!=0x80042634)return;
    static const int enabled=ttk_env_switch("DNTTK_CLIMB_AIM");
    climb_aim_limits();
    const double now=input_clock_ms();
    const double dt=climb_aim_last_ms<0 ? 0 : std::clamp(now-climb_aim_last_ms,0.0,100.0);
    climb_aim_last_ms=now;
    const bool ready=enabled!=0 && climb_aim_ready() && duke_model();
    climb_ik_stage=0;
    if(!ready) {climb_aim_weight=0;climb_rf_valid=false;return;}
    const bool stowing=psx_mod_read_half(player+0x74)==7;
    climb_aim_weight=std::clamp(climb_aim_weight+(stowing?-1:1)*dt/climb_aim_ramp_ms,0.0,1.0);
    if(climb_aim_weight<=0)return;
    double v[3];climb_view_forward(v);
    const double flat=std::sqrt(v[0]*v[0]+v[2]*v[2]);
    if(flat<1e-3)return;
    // Game yaw: +Z 0, +X 1024; positive turns toward +X, Duke's right.
    const double heading=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
    const double delta=wrap_yaw(std::atan2(v[0],v[2])-heading);
    climb_view_delta=delta;
    const double w=climb_aim_weight;
    climb_chest_yaw=std::clamp(delta,-climb_chest_max,climb_chest_max)*w;
    climb_head_yaw=std::clamp(delta,-climb_head_max,climb_head_max)*w;
    const double gun=heading+std::clamp(delta,-climb_gun_left,climb_gun_right);
    const double pitch=std::clamp(std::atan2(v[1],flat),-climb_pitch_max,climb_pitch_max);
    climb_aim_dir[0]=std::sin(gun)*std::cos(pitch);
    climb_aim_dir[1]=std::sin(pitch);
    climb_aim_dir[2]=std::cos(gun)*std::cos(pitch);
    climb_aim_set=true;
    ++climb_aim_builds;
    // The one-arm aim (aim build, poses 8/11) reads +0x124 for the gun shoulder.
    const unsigned upper=psx_mod_read_half(player+0x74);
    if((psx_mod_read_word(player+0x224)&0x200100) && (upper==8 || upper==11)) {
        double n=0;
        for(int i=0;i<3;++i){climb_saved_aim[i]=(int32_t)psx_mod_read_word(player+0x124+4*i);n+=double(climb_saved_aim[i])*climb_saved_aim[i];}
        n=std::sqrt(n);
        if(n<16)return;
        for(int i=0;i<3;++i)psx_mod_write_word(player+0x124+4*i,(uint32_t)(int32_t)std::lround(climb_aim_dir[i]*n));
        climb_aim_lent=true;++climb_aim_lends;
    }
}
// Scratch matrices (row-major Q12 rotation, int32 translation at +0x14).
using ClimbM=std::array<std::array<double,3>,3>;
using ClimbV=contact_arm::Vec;
static ClimbM climb_m_read(uint32_t a) {
    ClimbM m{};
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)m[i][j]=(int16_t)psx_mod_read_half(a+2*(3*i+j))/4096.0;
    return m;
}
static void climb_m_write(uint32_t a,const ClimbM& m) {
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)
        psx_mod_write_half(a+2*(3*i+j),(uint16_t)(int16_t)std::lround(std::clamp(m[i][j],-1.0,1.0)*4096));
}
static ClimbV climb_t_read(uint32_t a) {
    return {double((int32_t)psx_mod_read_word(a)),double((int32_t)psx_mod_read_word(a+4)),double((int32_t)psx_mod_read_word(a+8))};
}
static ClimbM climb_mul(const ClimbM& a,const ClimbM& b) {
    ClimbM m{};
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)for(int k=0;k<3;++k)m[i][j]+=a[i][k]*b[k][j];
    return m;
}
static ClimbM climb_tr(const ClimbM& a) {
    ClimbM m{};
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)m[i][j]=a[j][i];
    return m;
}
static ClimbV climb_mv(const ClimbM& m,ClimbV v) {
    return {m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z,m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z,m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z};
}
// Ry maps +Z to (sin, 0, cos): a turn about the world vertical.
static ClimbM climb_yaw_m(double yaw) {
    const double c=std::cos(yaw),s=std::sin(yaw);
    return ClimbM{{{c,0,s},{0,1,0},{-s,0,c}}};
}
// The smallest rotation taking direction a to direction b (Rodrigues).
static ClimbM climb_swing(ClimbV a,ClimbV b) {
    const double la=a.length(),lb=b.length();
    ClimbM id{{{1,0,0},{0,1,0},{0,0,1}}};
    if(la<1e-6 || lb<1e-6)return id;
    a=a*(1/la);b=b*(1/lb);
    const ClimbV k{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
    const double s=k.length(),c=a.dot(b);
    if(s<1e-9)return id;
    const ClimbV n=k*(1/s);
    const ClimbM K{{{0,-n.z,n.y},{n.z,0,-n.x},{-n.y,n.x,0}}};
    const ClimbM K2=climb_mul(K,K);
    ClimbM r=id;
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)r[i][j]+=s*K[i][j]+(1-c)*K2[i][j];
    return r;
}
// Off-hand arm hold, solved across its three compositions in one build.
// The forearm's local rotation comes from the previous build (the climb
// rests while armed); without it the shoulder only keeps its orientation.
static ClimbM climb_rf,climb_ws_orig,climb_ws_new,climb_wf_orig,climb_wf_new;
static ClimbV climb_elbow,climb_target;
static unsigned climb_ik_solves;
static double climb_ik_reach;
static bool climb_arm_records(uint32_t model,int& fore,int& hand) {
    fore=duke_part_joint(model,part_left_shoulder+1);hand=duke_part_joint(model,part_left_shoulder+2);
    return fore>=0 && hand>=0 && psx_mod_read_byte(model+0x44+0x28*fore+2)==4 && psx_mod_read_byte(model+0x44+0x28*hand+2)==5;
}
static void climb_offhand_shoulder(uint32_t model,uint32_t parent,uint32_t local) {
    climb_ik_stage=0;
    const ClimbM Pn=climb_m_read(parent),Pc=climb_mul(climb_tr(climb_yaw_m(climb_chest_yaw)),Pn),Ls=climb_m_read(local);
    const ClimbM Ws=climb_mul(Pc,Ls);
    int fore,hand;
    if(!climb_rf_valid || !climb_arm_records(model,fore,hand)) {climb_m_write(local,climb_mul(climb_tr(Pn),Ws));return;}
    const ClimbV tc=climb_t_read(parent+0x14),ts=climb_t_read(local+0x14);
    const ClimbV tf=climb_t_read(model+0x44+0x28*fore+0x14),th=climb_t_read(model+0x44+0x28*hand+0x14);
    const ClimbV so=tc+climb_mv(Pc,ts),sn=tc+climb_mv(Pn,ts);
    const ClimbV eo=so+climb_mv(Ws,tf),ho=eo+climb_mv(climb_mul(Ws,climb_rf),th);
    contact_arm::Pose pose;
    if(!contact_arm::solve(sn,ho,eo+(sn-so),eo-so,tf.length(),th.length(),pose)) {climb_m_write(local,climb_mul(climb_tr(Pn),Ws));return;}
    const ClimbV upper=pose.elbow-sn;
    climb_elbow=sn+upper*(tf.length()/std::max(1.0,upper.length()));
    climb_target=ho;climb_ik_reach=pose.upper_scale;
    climb_ws_orig=Ws;
    climb_ws_new=climb_mul(climb_swing(climb_mv(Ws,tf),climb_elbow-sn),Ws);
    climb_m_write(local,climb_mul(climb_tr(Pn),climb_ws_new));
    climb_ik_stage=1;
}
// 0x800b42ec entry (P, L, out), after swim_aim_arm; the first test must be cheap.
static void climb_aim_arm(CPUState* cpu) {
    if(!climb_aim_set || (cpu->gpr[31]!=0x8009899cu && cpu->gpr[31]!=0x800980d0u) || cpu->gpr[21]!=player)return;
    const uint32_t model=duke_model(),record=cpu->gpr[cpu->gpr[31]==0x8009899cu?16:17];
    const uint32_t parent=cpu->gpr[4],local=cpu->gpr[5],out=cpu->gpr[6];
    if(!model || record<model+0x44 || (record-(model+0x44))%0x28 || local<0x1f800000u || local>0x1f8003e0u)return;
    const int index=(int)((record-(model+0x44))/0x28);
    // The gun shoulder (aimed, never composed) is built: give +0x124 back.
    if(climb_aim_lent && index>psx_mod_read_byte(model+0x31))climb_aim_return();
    const int chest=duke_part_joint(model,part_spine),left=duke_part_joint(model,part_left_shoulder),
        head=duke_part_joint(model,part_neck);
    if(chest<0 || psx_mod_read_byte(model+0x44+0x28*chest+2)!=2)return;
    if(index==chest) {
        if(parent!=0x1f800020u || out!=0x1f800040u)return;
        climb_m_write(local,climb_mul(climb_mul(climb_tr(climb_m_read(parent)),climb_mul(climb_yaw_m(climb_chest_yaw),climb_m_read(parent))),climb_m_read(local)));
        ++climb_chest_turns;
        return;
    }
    int fore,hand;
    if(climb_arm_records(model,fore,hand) && (index==fore || index==hand)) {
        if(index==fore) {
            if(parent!=0x1f800060u || out!=0x1f800080u)return;
            const ClimbM Lf=climb_m_read(local);
            climb_rf=Lf;climb_rf_valid=true;
            if(climb_ik_stage!=1)return;
            // The forearm swings onto the hand's contact; the hand keeps its grip.
            climb_wf_orig=climb_mul(climb_ws_orig,Lf);
            const ClimbM base=climb_mul(climb_ws_new,Lf);
            const ClimbV th=climb_t_read(model+0x44+0x28*hand+0x14);
            climb_wf_new=climb_mul(climb_swing(climb_mv(base,th),climb_target-climb_elbow),base);
            climb_m_write(local,climb_mul(climb_tr(climb_ws_new),climb_wf_new));
            climb_ik_stage=2;
        } else {
            if(climb_ik_stage!=2 || parent!=0x1f800080u || out!=0x1f8000a0u)return;
            climb_m_write(local,climb_mul(climb_mul(climb_tr(climb_wf_new),climb_wf_orig),climb_m_read(local)));
            climb_ik_stage=0;++climb_ik_solves;
        }
        return;
    }
    // The off hand and the head hang from the (turned) chest, at depth 3.
    if((index!=left && index!=head) || index<chest || psx_mod_read_byte(record+2)!=3 ||
       parent!=0x1f800040u || out!=0x1f800060u)return;
    if(index==left) {climb_offhand_shoulder(model,parent,local);++climb_offhand_holds;}
    else {
        const ClimbM Pn=climb_m_read(parent);
        climb_m_write(local,climb_mul(climb_mul(climb_tr(Pn),climb_mul(climb_yaw_m(climb_head_yaw-climb_chest_yaw),Pn)),climb_m_read(local)));
        ++climb_head_turns;
    }
}
// The aim build's look-at (0x80097a44, the head) and one-arm aim (0x800978f8,
// the gun shoulder in poses 8/11): the clamped view direction replaces
// Duke's heading +0x124. Never the persistent aim state.
static bool climb_aim_direction(CPUState* cpu) {
    if(!climb_aim_set || cpu->gpr[18]!=player || cpu->gpr[5]!=player+0x124)return false;
    if(!climb_aim_vector)climb_aim_vector=psx_mod_alloc_guest_memory(16,16);
    if(!climb_aim_vector)return false;
    for(int i=0;i<3;++i)psx_mod_write_word(climb_aim_vector+4*i,(uint32_t)(int32_t)std::lround(climb_aim_dir[i]*4096));
    cpu->gpr[5]=climb_aim_vector;
    return true;
}
static const char* climb_aim_debug_json() {
    static char buffer[640];
    std::snprintf(buffer,sizeof buffer,"{\"set\":%s,\"weight\":%.3f,\"delta\":%.3f,\"chest_yaw\":%.3f,\"head_yaw\":%.3f,\"dir\":[%.3f,%.3f,%.3f],\"builds\":%u,\"chest_turns\":%u,\"offhand_holds\":%u,\"head_turns\":%u,\"look_aims\":%u,\"aim_lends\":%u,\"aim_returns\":%u,\"lent\":%s,\"ik_solves\":%u,\"ik_reach\":%.3f,\"ik_elbow\":[%.0f,%.0f,%.0f],\"ik_target\":[%.0f,%.0f,%.0f]}",
        climb_aim_set?"true":"false",climb_aim_weight,climb_view_delta,climb_chest_yaw,climb_head_yaw,climb_aim_dir[0],climb_aim_dir[1],climb_aim_dir[2],
        climb_aim_builds,climb_chest_turns,climb_offhand_holds,climb_head_turns,climb_look_aims,climb_aim_lends,climb_aim_returns,climb_aim_lent?"true":"false",climb_ik_solves,climb_ik_reach,climb_elbow.x,climb_elbow.y,climb_elbow.z,climb_target.x,climb_target.y,climb_target.z);
    return buffer;
}
