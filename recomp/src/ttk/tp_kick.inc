// D12E1 third-person quick kick (documentation/382-d12e1-third-person-quick-kick.md).
//
// KICK QUICK (Modernized default, DNTTK_TP_KICK; ORIGINAL keeps 112..115):
// in third person, Q (or a held attack with the Boot selected, or an original
// Boot request, converted at 0x800493a4 as D12A does) plays a fast MMA-style
// roundhouse with the left leg on Duke's pelvis and legs only, with any
// weapon, standing, walking, running or in a jump (as the eye-view kick). The
// arms, chest, head and weapon stay on the original animation and aim, so
// firing works throughout.
// - Pose: host clip 51 (ttk_quick_kick.py, measured from the user's reference
//   roundhouse), 16 keys at tp_kick_key_ms, written onto the pelvis and leg
//   joint records at 0x800411b8 after every channel blend (as
//   crouch_walk_pose). The hips turn over; the chest follows them only by
//   tp_chest_follow, so the torso and weapon stay near the aim (firing aims
//   the chest exactly). In the air the support leg keeps the jump's pose. Blends
//   in from the displayed pose over the first 50 ms and back to the live
//   locomotion from the regain point (350 ms) to 450 ms.
// - Timing (host time, the D12A cadence): hits on 133..333 ms through the
//   D12A sphere (0x800a979c, radius 96, type 10, the same damage per contact),
//   aimed from Duke's hip along the view within the foot's reach; whoosh (the
//   kick sound 0x1000) at the snap; D12C impact on the first real contact with
//   a pitch step that never repeats twice in a row. Chains: a Q during a kick
//   queues the next one, which starts at 350 ms (the same leg); a held Q
//   repeats.
// - On a real hit only: the strike holds 50 ms (taken from the recovery, the
//   450 ms total and 350 ms chain point never move), the original spark burst
//   0x8006d594(6, contact, 0xff) at the contact, and a camera punch along the
//   kick (orbit target offset; the view direction and aim never move).
// - Movement: ground travel is capped at the walking pace for 0..350 ms (the
//   original walk/run handlers' displacement, modern_controls.cpp), and a 35
//   unit step forward on the snap through the standing hull probe (on the ground).
// First person keeps D12A (kick.inc); Vanilla never gets here.
static constexpr unsigned tp_kick_clip=51,tp_kick_keys=16;
static constexpr double tp_kick_key_ms[tp_kick_keys]={0,40,67,92,110,125,138,150,200,283,317,350,375,400,425,450};   // ttk_quick_kick.py KEY_MS
static constexpr double tp_kick_total=450,tp_kick_regain=350,tp_kick_ramp_in=50,tp_kick_snap=125,tp_chest_follow=0.35;
static constexpr double tp_kick_hit_first=133,tp_kick_hit_last=333,tp_kick_pause=50,tp_kick_pause_latest=283;
static constexpr double tp_kick_cancel_fade=80;
static constexpr double tp_kick_step=35,tp_kick_reach=460,tp_kick_height=60,tp_kick_side=-10;   // the left leg, near the middle
static constexpr double tp_punch_ms=140,tp_punch_push=14,tp_punch_shake=6;
static constexpr unsigned tp_kick_written[7]={0,1,2,3,5,6,7};   // toes keep the live pose
static constexpr uint32_t tp_spark_routine=0x8006d594;
static constexpr int tp_impact_steps[5]={-4,-2,0,2,4};
static bool tp_active,tp_queued,tp_sounded,tp_stepped,tp_impacted,tp_paused,tp_from_valid,tp_shown;
static unsigned tp_impact_step=2;
static bool tp_air;   // this update: Duke is in a jump or fall
static double tp_start_ms,tp_hit_ms,tp_cancel_ms=-1,tp_punch_start=-1e9;
static double tp_punch_dir[3];
static CrouchQuat tp_from[9],tp_last[9];
static uint32_t tp_point,tp_spark_point;
static uint64_t tp_kicks,tp_chains,tp_converts,tp_fire_starts,tp_cancels,tp_hit_calls,tp_hits,tp_walls,tp_sounds,tp_sparks,tp_steps,tp_step_blocks,tp_pose_writes,tp_paces,tp_suppressed,tp_surface_points;
static bool tp_kick_quick() {
    const char* v=ttk_live_env("DNTTK_TP_KICK");
    return input_modernized() && (!v || std::strcmp(v,"original"));
}
// The third-person view on the ground (idle 63, the walks/runs 72..79; a kick
// already running also through a landing pose) or in a jump or fall (mode 9,
// the standing jump 96/97: the states that keep the view aim for shots, D07F).
static bool tp_kick_air() {
    if(jetpack_camera_only() || push_state_early() || mantle_state_early() || ladder_mount_early() || climb_state_early()) return false;
    const unsigned mode=psx_mod_read_byte(player+0x22c),previous=psx_mod_read_byte(player+0x22d),lower=psx_mod_read_half(player+0x60);
    return mode==9 || (mode==0 && (previous==9 || lower==96 || lower==97));
}
static unsigned tp_refuse_reason;
static bool tp_kick_ready() {
    tp_refuse_reason=1;
    if(!tp_kick_quick() || !independent_camera() || fp_blend>0 || !player_identity_ready()) return false;
    tp_refuse_reason=2;
    if(psx_mod_read_word(player)&0x20002u) return false;
    tp_refuse_reason=3;
    uint32_t first,count;
    if(!ttk::anim_extra_clip(tp_kick_clip,first,count) || count!=tp_kick_keys+1) return false;
    tp_refuse_reason=0;
    if(tp_kick_air()) return true;
    tp_refuse_reason=4;
    if(psx_mod_read_byte(player+0x22c)!=0) return false;
    const unsigned lower=psx_mod_read_half(player+0x60);
    if(tp_active && jump_animation(lower)) {tp_refuse_reason=0;return true;}
    tp_refuse_reason=5;
    const bool ground=psx_mod_read_byte(player+0x22d)==0 && (lower==63 || (lower>=72 && lower<=79));
    if(ground) tp_refuse_reason=0;
    return ground;
}
// Developer tuning only: DNTTK_TP_KICK_FREEZE=0..449 holds every kick at that
// time (the pose stays until the kick is cancelled, e.g. by the view switch).
static double tp_kick_freeze() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_TP_KICK_FREEZE");char* end=nullptr;
        const double v=text?std::strtod(text,&end):-1;
        return text && end && !*end && v>=0 && v<tp_kick_total ? v : -1.0;
    }();
    return value;
}
static double tp_kick_elapsed() {return tp_kick_freeze()>=0 ? tp_kick_freeze() : input_clock_ms()-tp_start_ms;}
// Clip time: the strike holds tp_kick_pause on a hit; the recovery after it
// plays faster so the kick still ends at tp_kick_total.
static double tp_kick_clip_ms(double t) {
    if(!tp_paused || t<tp_hit_ms) return t;
    if(t<tp_hit_ms+tp_kick_pause) return tp_hit_ms;
    return tp_hit_ms+(t-tp_hit_ms-tp_kick_pause)*(tp_kick_total-tp_hit_ms)/(tp_kick_total-tp_hit_ms-tp_kick_pause);
}
static void tp_kick_start() {
    tp_from_valid=tp_shown;
    if(tp_shown) for(unsigned i=0;i<9;++i) tp_from[i]=tp_last[i];
    tp_active=true;tp_queued=false;tp_sounded=tp_stepped=tp_impacted=tp_paused=false;
    tp_start_ms=input_clock_ms();tp_cancel_ms=-1;++tp_kicks;
}
static uint64_t tp_requests,tp_refusals;
static bool tp_kick_request(bool queue) {
    ++tp_requests;
    if(!tp_kick_ready()) {++tp_refusals;return false;}
    if(tp_active) {if(queue) tp_queued=true;return true;}
    tp_kick_start();
    return true;
}
static void tp_kick_cancel() {
    if(!tp_active) return;
    tp_active=tp_queued=false;tp_cancel_ms=input_clock_ms();++tp_cancels;
}
// modern_controls.cpp walk/run handlers: travel capped at the walking pace
// from the press to the regain point.
static bool tp_kick_slowing() {return tp_active && tp_kick_elapsed()<tp_kick_regain;}
static double tp_kick_pace(double length,unsigned dt) {
    if(!tp_kick_slowing()) return length;
    const double cap=modern_walk_pace*dt/1024.0;
    if(length>cap) ++tp_paces;
    return std::min(length,cap);
}
// orbit_constraint: the camera punch, a world offset of the orbit target.
static void tp_kick_camera_offset(double* out) {
    out[0]=out[1]=out[2]=0;
    const double t=input_clock_ms()-tp_punch_start;
    if(t<0 || t>=tp_punch_ms || fp_blend>0) return;
    // A push along the kick over the first 33 ms, back over the next 33,
    // then a decaying side-to-side shake.
    const double push=t<33 ? t/33 : t<67 ? 1-(t-33)/34 : 0;
    const double decay=1-t/tp_punch_ms;
    const double shake=tp_punch_shake*decay*decay*std::sin(t*0.09);
    const double side[3]={tp_punch_dir[2],0,-tp_punch_dir[0]};
    for(int i=0;i<3;++i) out[i]=tp_punch_dir[i]*tp_punch_push*push+side[i]*shake;
}
// Duke's heading (game yaw: +z 0, +x 1024) as a unit vector.
static void tp_kick_heading(double* forward) {
    const double a=(psx_mod_read_half(player+0x1c)&4095)*6.283185307179586/4096;
    forward[0]=std::sin(a);forward[1]=0;forward[2]=std::cos(a);
}
// Once per player update (0x80058120, ra 0x80041c44), after kick_update.
static void tp_kick_update(CPUState* cpu) {
    if(!tp_point) tp_point=psx_mod_alloc_guest_memory(16,16);
    if(!tp_spark_point) tp_spark_point=psx_mod_alloc_guest_memory(16,16);
    const auto& f=input_snapshot(Context::Gameplay);
    if(tp_active && !tp_kick_ready()) {tp_kick_cancel();return;}
    tp_air=tp_kick_air();
    if(tp_active) {
        const double t=tp_kick_elapsed();
        const bool again=tp_queued || f.held[quick_kick] || (f.held[fire] && boot_selected());
        if((again && t>=tp_kick_regain) || t>=tp_kick_total) {
            tp_active=false;
            if(again && tp_kick_request(false)) ++tp_chains;
        }
    }
    if(!tp_active && f.held[fire] && boot_selected() && tp_kick_request(false)) ++tp_fire_starts;
    if(!tp_active) return;
    const double t=tp_kick_elapsed();
    double forward[3];tp_kick_heading(forward);
    if(!tp_sounded && t>=tp_kick_snap) {
        tp_sounded=true;
        if(kick_call(cpu,kick_sound_routine,{kick_sound,player+4,0x800,0,0,0})) ++tp_sounds;
    }
    if(!tp_stepped && t>=tp_kick_snap && !tp_air) {
        // The original 115 steps into its kick: a short step through the
        // standing hull (walls, objects, edges, steps); refused, Duke stays.
        tp_stepped=true;
        int32_t step=0;
        const int32_t dx=int32_t(std::lround(forward[0]*tp_kick_step)),dz=int32_t(std::lround(forward[2]*tp_kick_step));
        if(crouch_probe(cpu,dx,dz,step,0x370)==hull_clear && std::abs(step)<=16) {
            psx_mod_write_word(player+4,psx_mod_read_word(player+4)+uint32_t(dx));
            psx_mod_write_word(player+0xc,psx_mod_read_word(player+0xc)+uint32_t(dz));
            ++tp_steps;
        } else ++tp_step_blocks;
    }
    if(t<tp_kick_hit_first || t>tp_kick_hit_last || !tp_point) return;
    // The posed ankle (ttk_quick_kick.py: about 460 ahead of the pelvis at its
    // height, near the middle at the strike), pulled to the aim: the view ray from
    // the hip, where it meets a surface or an actor within the foot's reach.
    double hip[3],view[3],point[3],end[3],hit[3];
    const double side=tp_kick_side;
    for(int i=0;i<3;++i) {
        hip[i]=(int32_t)psx_mod_read_word(player+4+4*i);
        view[i]=(int16_t)psx_mod_read_half(camera+2*(6+i))/4096.0;
    }
    const double right[3]={forward[2],0,-forward[0]};
    for(int i=0;i<3;++i) point[i]=hip[i]+forward[i]*tp_kick_reach+right[i]*side;
    point[1]-=tp_kick_height;
    for(int i=0;i<3;++i) end[i]=hip[i]+view[i]*kick_trace_length;
    int kind=0;
    if(view_segment_query(cpu,hip,end,hit,kind) && kind &&
       std::hypot(hit[0]-hip[0],hit[2]-hip[2])<=kick_horizontal_reach && hit[1]<hip[1]+impact_floor_margin*4) {
        for(int i=0;i<3;++i) point[i]=hit[i]-view[i]*kick_surface_backoff;
        ++tp_surface_points;
    }
    bool wall=false;
    if(!tp_impacted) wall=kick_wall_reached(cpu,forward);
    for(int i=0;i<3;++i) psx_mod_write_word(tp_point+4*i,(uint32_t)(int32_t)std::lround(point[i]));
    psx_mod_write_word(tp_point+12,0);
    const uint32_t delta=psx_mod_read_word(0x800d21fc);
    const uint32_t damage=delta*5*((psx_mod_read_half(player+0x364)&2)?16:4);
    bool struck=false;
    if(kick_call(cpu,kick_damage_routine,{tp_point,(uint32_t)kick_radius,(uint32_t)kick_type,damage,player,0},&struck)) ++tp_hit_calls;
    if(tp_impacted || !(struck || wall)) return;
    tp_impacted=true;++(struck?tp_hits:tp_walls);
    // Never the same impact pitch twice in a row.
    tp_impact_step=(tp_impact_step+1+unsigned(std::lround(t))%4)%5;
    impact_pitch_offset=tp_impact_steps[tp_impact_step];
    kick_impact(cpu,struck);
    impact_pitch_offset=0;
    // The spark: at the sphere for an actor, at the wall where the foot meets it.
    double spark[3];
    for(int i=0;i<3;++i) spark[i]=point[i];
    if(!struck) {
        double from[3],to[3];
        for(int i=0;i<3;++i) {from[i]=hip[i];to[i]=hip[i]+forward[i]*impact_reach;}
        from[1]=to[1]=hip[1]-tp_kick_height;
        int k=0;
        if(view_segment_query(cpu,from,to,hit,k) && k==1) for(int i=0;i<3;++i) spark[i]=hit[i]-forward[i]*8;
    }
    if(tp_spark_point) {
        for(int i=0;i<3;++i) psx_mod_write_word(tp_spark_point+4*i,(uint32_t)(int32_t)std::lround(spark[i]));
        psx_mod_write_word(tp_spark_point+12,0);
        if(kick_call(cpu,tp_spark_routine,{6,tp_spark_point,0xff,0,0,0})) ++tp_sparks;
    }
    if(t<=tp_kick_pause_latest) {tp_paused=true;tp_hit_ms=t;}
    for(int i=0;i<3;++i) tp_punch_dir[i]=forward[i];
    tp_punch_start=input_clock_ms();
}
// 0x800493a4 (lower-body initializer): an original Boot request in third
// person (QUICK) restarts idle 63 as kick_convert does; Q or a held attack
// kicks, Action alone does nothing (as in the eye view).
static void tp_kick_convert(CPUState* cpu,uint32_t ra) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(cpu->gpr[4]!=player || (ra!=0x8005a3a0 && ra!=0x8005a490 && ra!=0x8005a4d0) ||
       animation<112 || animation>115 || psx_mod_read_half(player+0x68)!=0 || fp_blend>0 || !tp_kick_quick() || !player_identity_ready()) return;
    psx_mod_write_half(player+0x60,63);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    const auto& f=input_snapshot(Context::Gameplay);
    if((f.held[fire] || f.held[quick_kick]) && tp_kick_request(true)) ++tp_converts;
    else ++tp_suppressed;
}
// One key pose of a kick clip, by key index.
static bool tp_kick_key(unsigned clip,unsigned key,std::array<std::array<int32_t,4>,32>& q,uint32_t& mask) {
    uint32_t first,count;
    if(!ttk::anim_extra_clip(clip,first,count) || count!=tp_kick_keys+1 || key>=tp_kick_keys) return false;
    return crouch_frame_quats(first+1+key,q,mask);
}
// The clip pose at clip time c (ms) for joint ids 0..8.
static bool tp_kick_clip_pose(double c,CrouchQuat (&out)[9]) {
    c=std::clamp(c,0.0,tp_kick_total);
    unsigned k=0;
    while(k+2<tp_kick_keys && c>=tp_kick_key_ms[k+1]) ++k;
    // Dense keys sampled from the reference: linear between them keeps its arcs.
    const double u=std::clamp((c-tp_kick_key_ms[k])/(tp_kick_key_ms[k+1]-tp_kick_key_ms[k]),0.0,1.0);
    std::array<std::array<int32_t,4>,32> a{},b{};uint32_t ma=0,mb=0;
    const unsigned clip=tp_kick_clip;
    if(!tp_kick_key(clip,k,a,ma) || !tp_kick_key(clip,k+1,b,mb)) return false;
    for(unsigned j:tp_kick_written) {
        if(!(ma>>j&1) || !(mb>>j&1)) return false;
        out[j]=crouch_slerp(crouch_qnorm({double(a[j][0]),double(a[j][1]),double(a[j][2]),double(a[j][3])}),
                            crouch_qnorm({double(b[j][0]),double(b[j][1]),double(b[j][2]),double(b[j][3])}),u);
    }
    return true;
}
// 0x800411b8 (player update, last call before the model build).
static void tp_kick_pose(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80042634) return;
    const uint32_t desc=duke_model();
    const double now=input_clock_ms();
    const bool fading=!tp_active && tp_cancel_ms>=0 && now-tp_cancel_ms<tp_kick_cancel_fade && tp_shown;
    if(!desc || (!tp_active && !fading)) {tp_shown=false;return;}
    CrouchQuat target[9];
    double w;
    if(tp_active) {
        const double t=tp_kick_elapsed();
        if(!tp_kick_clip_pose(tp_kick_clip_ms(t),target)) {tp_shown=false;return;}
        const double in=std::clamp(t/tp_kick_ramp_in,0.0,1.0),out=std::clamp((t-tp_kick_regain)/(tp_kick_total-tp_kick_regain),0.0,1.0);
        // Ramp in from the pose shown last (a chained kick) or the live one.
        if(tp_from_valid && in<1) for(unsigned j:tp_kick_written) target[j]=crouch_slerp(tp_from[j],target[j],in*in*(3-2*in));
        w=(tp_from_valid ? 1.0 : in*in*(3-2*in))*(1-out*out*(3-2*out));
    } else {
        for(unsigned j:tp_kick_written) target[j]=tp_last[j];
        const double u=1-(now-tp_cancel_ms)/tp_kick_cancel_fade;
        w=u*u*(3-2*u);
    }
    const uint32_t pelvis_r=crouch_joint_record(desc,0),chest_r=crouch_joint_record(desc,9);
    if(!pelvis_r || !chest_r) {tp_shown=false;return;}
    const CrouchQuat pelvis_live=crouch_record_q(pelvis_r),chest_live=crouch_record_q(chest_r);
    for(unsigned j:tp_kick_written) {
        const uint32_t r=crouch_joint_record(desc,j);
        if(!r) continue;
        if(tp_air && j>=1 && j<=3) {tp_last[j]=crouch_record_q(r);continue;}   // the jump's support leg
        const CrouchQuat q=crouch_slerp(crouch_record_q(r),target[j],w);
        crouch_record_write(r,q);
        tp_last[j]=q;
    }
    // The chest follows the hips' turn by tp_chest_follow only: kept is its
    // world rotation (world = local (x) parent, game order), followed its local.
    const CrouchQuat pelvis_now=crouch_record_q(pelvis_r);
    const CrouchQuat kept=crouch_qnorm(crouch_qmul(crouch_qmul(chest_live,pelvis_live),crouch_qinv(pelvis_now)));
    crouch_record_write(chest_r,crouch_slerp(kept,chest_live,tp_chest_follow));
    tp_shown=true;++tp_pose_writes;
}
static const char* tp_kick_debug_json() {
    static char buffer[900];
    std::snprintf(buffer,sizeof buffer,"{\"quick\":%s,\"ready\":%s,\"active\":%s,\"air\":%u,\"ms\":%.0f,\"kicks\":%llu,\"chains\":%llu,\"converts\":%llu,\"fire_starts\":%llu,\"suppressed\":%llu,\"cancels\":%llu,\"hit_calls\":%llu,\"hits\":%llu,\"walls\":%llu,\"surface_points\":%llu,\"sounds\":%llu,\"sparks\":%llu,\"steps\":%llu,\"step_blocks\":%llu,\"paces\":%llu,\"pose_writes\":%llu,\"paused\":%s,\"requests\":%llu,\"refusals\":%llu,\"reason\":%u}",
        tp_kick_quick()?"true":"false",tp_kick_ready()?"true":"false",tp_active?"true":"false",tp_air?1u:0u,tp_active?tp_kick_elapsed():0.0,
        (unsigned long long)tp_kicks,(unsigned long long)tp_chains,(unsigned long long)tp_converts,(unsigned long long)tp_fire_starts,(unsigned long long)tp_suppressed,
        (unsigned long long)tp_cancels,(unsigned long long)tp_hit_calls,(unsigned long long)tp_hits,(unsigned long long)tp_walls,(unsigned long long)tp_surface_points,
        (unsigned long long)tp_sounds,(unsigned long long)tp_sparks,(unsigned long long)tp_steps,(unsigned long long)tp_step_blocks,(unsigned long long)tp_paces,
        (unsigned long long)tp_pose_writes,tp_paused?"true":"false",(unsigned long long)tp_requests,(unsigned long long)tp_refusals,tp_refuse_reason);
    return buffer;
}
