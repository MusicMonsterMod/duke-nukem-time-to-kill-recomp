// D08Q — modern jetpack flight controls (host layer over the original handler).
// Included from modern_controls.cpp after swim.inc.
//
// Original SLUS-00583 (read-only research, documentation/57-jetpack-controls.md):
//   • 8004aaf8 (called every player update from the 800c2754[3] pre-state
//     step) enters mode 10 / anim 163 when Square is pressed with the jetpack
//     equipped and on (+0x358 & 3 == 3), fuel +0x35a >= dt, not in water.
//   • 8004ade0 is the mode-10 handler (8004b594 dispatch). It rotates the
//     thrust velocity +0x1e4/+0x1e8/+0x1ec into body space (+0x1c heading),
//     reads the pad tables: Square = lift, Up/Down = forward/back thrust,
//     L2/R2 = strafe (half the forward gain), a hover button (800d1cf0 word)
//     toggles the hover lock +0x224 & 0x08000000, then rotates back, leans the
//     model (+0x87c..+0x880), integrates (8003ea58), and applies the root
//     (8004ac08: result 3 from the floor probe lands Duke). Any held input
//     sets +0x224 bit 31 (no gravity this tick); no input → gravity → 164
//     fall pose. Fuel drains only while thrusting (s2). With the hover lock
//     set the handler zeroes +0x1f8/+0x1e8 every tick and writes the vertical
//     root +0xfe = +0x860 + bob(+0x864) − body Y, so Y follows +0x860.
//   • Fuel out / jetpack off → 8004b39c: anim 108 fall, 8003ff6c(player,1).
//
// Modernized (this file + pc_input.cpp + state()):
//   • Camera-only lease in mode 10: mouse orbit; body heading = view yaw
//     (face_view).
//   • J while flying → original item toggle (select_weapon), the handler then
//     cuts out to the 108 fall; the camera lease survives that fall.
// Modern/EDuke32 own a smoothly eased velocity. Native pads retain flame,
// lean, firing and fuel behavior; predicted native thrust is cancelled before
// integration. D08Q8 resolves the integrated step with authenticated native
// collision sweeps, including stable floor/ceiling clearance and small steps.
// No alternating vertical nudge or cached-floor height correction is needed.
// Classic retains its burst physics and landing; it shares clearance checks.
// Vanilla never enters these adapters.

// Speeds in bv units (bv * dt / 1024 = units per update). Measured from the
// original (post-handler trace, Classic): horizontal flight reaches ~16000
// (still accelerating at walls; ~18000 seen), Square's climb caps at -11832.
// Descent ~ the old Modern Ctrl rate (D08Q1, ~490 per 12 frames here).
static constexpr float k_jet_speed = 16000.f;
static constexpr float k_jet_climb = 11800.f;
static constexpr float k_jet_descend = 6000.f;
// Ease time constants (dt units, ~ms): horizontal starts/stops over a few
// updates; vertical stops quickly so a released climb barely overshoots.
static constexpr float k_jet_ease_ms = 50.f;
static constexpr float k_jet_ease_vertical_ms = 15.f;
// Height hold: bv per unit of error (closes ~60% of it in 0.1 s), capped.
static constexpr float k_jet_hold_gain = 6.f;
static constexpr float k_jet_hold_max = 600.f;
// Vertical speed (bv) under which a released climb/descent is captured.
static constexpr float k_jet_capture_speed = 300.f;
static unsigned jet_hover_engages, jet_descents, jet_updates, jet_classic_updates, jet_hold_captures, jet_root_nudges;
static bool jet_was_flying, jet_hold_valid, jet_flight_live;
static bool jet_air_height_hold;
static int32_t jet_hold_y;
static float jet_vel[3];
// The original's thrust velocity +0x1e4..+0x1ec after the previous update
// (jet_post_update), world space; it changes slowly, so it predicts this
// update's thrust for the cancellation below.
static int32_t jet_thrust_prev[3];
static bool jet_pads_fed;
// Vertical root (units) planned this update, and the learned offset between
// the root the handler applies and the plan.
static float jet_root_planned, jet_root_bias;
static uint64_t jet_epoch;

static bool jet_trace() {
    static const bool enabled=std::getenv("DNTTK_JET_TRACE")!=nullptr;
    return enabled;
}
// Altitude is a player target; only a fresh collision may change it.
static int32_t jet_hold_target(int32_t) {return jet_hold_y;}
bool jetpack_input_ready() {
    if(!input_modernized() || !gameplay_context() || !independent_camera())return false;
    if(!input_snapshot(Context::Gameplay).active)return false;
    if(!jetpack_flying_early())return false;
    if((psx_mod_read_word(player)&0x20002) || (psx_mod_read_word(player+0x224)&0x20000200u))return false;
    if(psx_mod_read_word(camera+0xa4)!=player || psx_mod_read_word(player+0x7d4)!=camera)return false;
    return lease_ready(true) && identity();
}
// Same controls as Modern (lease, mouse, WASD); only the flight physics differ.
bool jetpack_classic_input_ready() { return jetpack_classic() && jetpack_input_ready(); }
// D08S: keep native item capacity/HUD/save units. One full pack is 3660
// 60 Hz input fields (61 seconds), independent of presentation rate and the
// original variable animation dt. A sub-unit remainder is host-only.
static uint64_t jet_fuel_frame, jet_fuel_epoch, jet_fuel_fraction;
static int jet_fuel_expected=-1, jet_fuel_after, jet_fuel_seed;
static bool jet_fuel_pending;
// J's short, accelerating lift follows the reference's ten 30 Hz updates.
// Move through native velocity/collision, never by teleporting the root.
static constexpr float k_jet_start_lift=640.f;
static constexpr uint64_t k_jet_start_fields=20;
static bool jet_lift_active;
static uint64_t jet_lift_frame,jet_lift_epoch;
static int32_t jet_lift_y;
static bool jet_instant_ready() {
    const auto& f=input_snapshot(Context::Gameplay);
    return input_modernized() && !jetpack_classic() && f.active && gameplay_context() &&
        independent_camera() && player_identity_ready() && lease_ready(true) &&
        psx_mod_read_byte(player+0x233)==0 && (int16_t)psx_mod_read_half(player+0x32)>0 &&
        !(psx_mod_read_word(player)&0x20002) &&
        psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera;
}
static void jet_eduke_pose(unsigned animation,unsigned mode) {
    psx_mod_write_half(player+0x60,animation);
    psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    psx_mod_write_byte(player+0x22c,mode);psx_mod_write_byte(player+0x22d,mode);
    for(unsigned axis=0;axis<3;++axis) {
        psx_mod_write_word(player+0x1e4+4*axis,0);
        psx_mod_write_word(player+0x1f4+4*axis,0);
        psx_mod_write_half(player+0xfc+2*axis,0);
    }
    jet_lift_active=false;jet_air_height_hold=false;
    jet_flight_live=false;jet_hold_valid=false;jet_root_bias=0;
    takeoff_valid=false;flight_valid=false;delayed_takeoff=false;fall_velocity_pending=false;
}
static bool jet_eduke_off(CPUState* cpu,bool fall=true) {
    // Original close stops the flight loop sound and clears its flags.
    if(!original_call(cpu,0x8003ff6c,1))return false;
    psx_mod_write_half(player+0x84c,1);psx_mod_write_half(player+0x84e,0);
    if(!original_call(cpu,0x8004001c,0))return false; // close mesh endpoint
    if(fall) {
        psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x8e000000u);
        jet_eduke_pose(108,9);
    }
    jet_fall_grace=fall;jet_was_flying=false;
    jet_epoch=input_snapshot(Context::Gameplay).epoch;
    jet_fuel_frame=0;jet_fuel_pending=false;
    return true;
}
static bool jet_instant_toggle(CPUState* cpu) {
    if(!jet_instant_ready())return false;
    if(jetpack_flying_early()) {
        if(!jet_eduke_off(cpu))return false;
        input_notice("JETPACK OFF");return true;
    }
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    const unsigned flags=psx_mod_read_word(player+0x224);
    const unsigned owned=psx_mod_read_half(player+0x358);
    // A fresh J can reverse this pack's own airborne close immediately.
    // Other pending item transitions retain ownership.
    const bool closing=mode==9 && (owned&0x8003)==0x8001 &&
        (flags&0x10000000u) && psx_mod_read_word(player+0x884)==1;
    // Only normal ground/jump/fall ownership. Never steal a climb, swim,
    // scripted interaction, death, or pending item/weapon transition.
    if((mode!=0 && mode!=9) || (flags&0x2000028cu) || !(owned&1) ||
       ((owned&0x8000) && !closing) || (int16_t)psx_mod_read_half(player+0x35a)<=0 ||
       ((int32_t)psx_mod_read_word(player+0x834)!=0x40000000 &&
        (int32_t)psx_mod_read_word(player+8)>=(int32_t)psx_mod_read_word(player+0x834)))return false;
    if(guest_busy || cpu->gpr[29]<0x801f4000 || cpu->gpr[29]>0x801ffff0 ||
       g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    psx_mod_write_half(player+0x358,owned|0x8002);
    if(!original_call(cpu,0x800402d0,1))return false;
    // Finish the original opening mesh update immediately (its endpoint is 5).
    psx_mod_write_half(player+0x84c,4);psx_mod_write_half(player+0x84e,0);
    if(!original_call(cpu,0x8004001c,0))return false;
    psx_mod_write_word(player,psx_mod_read_word(player)&~0x20u);
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x88000041u);
    jet_eduke_pose(163,10); // authenticated native entry's lower-body setup
    jet_fall_grace=false;
    // Ground starts lift off; airborne starts arrest vertical motion and
    // capture the current height without replaying the ground launch boost.
    jet_lift_active=mode==0;jet_lift_y=(int32_t)psx_mod_read_word(player+8);
    jet_air_height_hold=mode==9;
    if(jet_air_height_hold) {jet_hold_y=jet_lift_y;jet_hold_valid=true;}
    jet_lift_frame=input_host_frame();jet_lift_epoch=input_snapshot(Context::Gameplay).epoch;
    jet_fuel_frame=input_host_frame();jet_fuel_epoch=input_snapshot(Context::Gameplay).epoch;
    jet_fuel_expected=(int16_t)psx_mod_read_half(player+0x35a);
    input_notice("JETPACK ON");return true;
}
// Player-update entry (8005a210 hook), before the pre-state step and the
// mode-10 handler read heading, pads and velocities for this update.
static void jetpack_update(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const bool flying=jetpack_flying_early();
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(jet_was_flying && !flying) {
        // Cut-out in the air: original 108 fall (mode byte 10 on the cut-out
        // update, then 9). Landing (mode 0), any other state or a fresh
        // capture epoch ends the grace.
        // J clears the 358 bit before the handler swaps the pose, so the
        // cut-out update can still show the flight anim.
        jet_fall_grace=(animation==108 || jetpack_anim(animation)) && (mode==9 || mode==10) && input_modernized();
    } else if(jet_fall_grace && (animation!=108 || (mode!=9 && mode!=10) || !f.active || f.epoch!=jet_epoch)) {
        jet_fall_grace=false;
    }
    jet_was_flying=flying;
    jet_epoch=f.epoch;
    if(!flying || !jetpack_input_ready() || jetpack_classic()) {
        // A new flight (or lease) starts from the original's own motion.
        if(jet_trace() && jet_flight_live)std::fprintf(stderr,"ttk-jet reset flying=%d ready=%d anim=%u mode=%u flags=%08x refusal=%s\n",
            flying,jetpack_input_ready(),animation,mode,psx_mod_read_word(player+0x224),lease_refusal_reason());
        jet_lift_active=false;
        if(!flying)jet_air_height_hold=false;
        if(jetpack_eduke32())jet_fuel_frame=0;
        jet_flight_live=false;
        jet_hold_valid=false;
        jet_thrust_prev[0]=jet_thrust_prev[1]=jet_thrust_prev[2]=0;
        jet_root_bias=0.f;
        if(flying && jetpack_input_ready()) {
            // D08R Classic: modern controls (mouse facing, pad bridge),
            // original burst physics.
            ++jet_updates;++jet_classic_updates;
            if(!f.held[original_aim])face_view();
            if(jet_trace())std::fprintf(stderr,"ttk-jetc dt=%u xyz=%d,%d,%d\n",psx_mod_read_word(0x800d21fc),
                (int32_t)psx_mod_read_word(player+4),(int32_t)psx_mod_read_word(player+8),(int32_t)psx_mod_read_word(player+12));
        }
        return;
    }
    if(jetpack_eduke32()) {
        const int fuel=(int16_t)psx_mod_read_half(player+0x35a);
        const int cap=(int16_t)psx_mod_read_half(0x800c2716);
        const uint64_t frame=input_host_frame();
        if(f.epoch!=jet_fuel_epoch || fuel!=jet_fuel_expected || frame<jet_fuel_frame)
            jet_fuel_fraction=0;
        const uint64_t elapsed=jet_fuel_frame && f.epoch==jet_fuel_epoch && frame>=jet_fuel_frame ? frame-jet_fuel_frame : 0;
        jet_fuel_frame=frame;jet_fuel_epoch=f.epoch;
        if(cap>0 && !psx_mod_read_half(0x800c3cc4) && !psx_mod_read_half(0x800c3cc6))
            jet_fuel_fraction+=elapsed*(unsigned)cap;
        const int drain=(int)(jet_fuel_fraction/3660);jet_fuel_fraction%=3660;
        jet_fuel_after=std::max(0,fuel-drain);jet_fuel_expected=jet_fuel_after;
        if(!jet_fuel_after) {
            psx_mod_write_half(player+0x35a,0);jet_eduke_off(cpu);return;
        }
        // The original handler must not cut out on its dt-based threshold.
        // Restore our save/HUD units at the same update's post-handler hook.
        jet_fuel_seed=std::max(jet_fuel_after,2*(int)psx_mod_read_word(0x800d21fc)+1);
        psx_mod_write_half(player+0x35a,jet_fuel_seed);
        jet_fuel_pending=true;
    }
    ++jet_updates;
    if(!f.held[original_aim])face_view();
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    if(!jet_flight_live) {
        // Take over from the takeoff: carry its current motion, then ease.
        for(int axis=0;axis<3;++axis)jet_vel[axis]=(float)(int32_t)psx_mod_read_word(player+0x1f4+4*axis);
        jet_flight_live=true;
    }
    const float dt=(float)std::clamp<uint32_t>(psx_mod_read_word(0x800d21fc),1u,60u);
    const float ease=1.f-std::exp(-dt/k_jet_ease_ms);
    const float ease_vertical=1.f-std::exp(-dt/k_jet_ease_vertical_ms);
    // Horizontal: WASD and Up / Down, camera-relative (heading = view).
    float forward=std::clamp(f.move_y+f.arrow_y,-1.f,1.f),side=std::clamp(f.move_x,-1.f,1.f);
    const float magnitude=std::hypot(forward,side);
    if(magnitude>1.f) {forward/=magnitude;side/=magnitude;}
    const float yaw=(float)psx_mod_read_half(player+0x1c)*tau/4096.f; // +Z = 0, +X = 1024
    const float want_x=k_jet_speed*(forward*std::sin(yaw)+side*std::cos(yaw));
    const float want_z=k_jet_speed*(forward*std::cos(yaw)-side*std::sin(yaw));
    // Vertical (Y grows downward).
    const bool ascend=f.held[jump] && !f.held[crouch];
    const bool descend=f.held[crouch] && !f.held[jump];
    float want_y=0.f;
    if(ascend) {
        jet_air_height_hold=true;
        want_y=-k_jet_climb;jet_hold_valid=false;
    } else if(descend) {
        jet_air_height_hold=true;
        want_y=k_jet_descend;jet_hold_valid=false;++jet_descents;
    } else if(!jet_hold_valid) {
        // Ease the climb/descent out, then hold where it settled.
        if(std::fabs(jet_vel[1])<k_jet_capture_speed) {
            jet_hold_y=y;jet_hold_valid=true;++jet_hold_captures;
        }
    }
    if(jet_hold_valid && !ascend && !descend) {
        want_y=std::clamp((float)(jet_hold_target(y)-y)*k_jet_hold_gain,-k_jet_hold_max,k_jet_hold_max);
    }
    bool starting=false;
    if(jet_lift_active) {
        const uint64_t frame=input_host_frame();
        const uint64_t age=frame>=jet_lift_frame ? frame-jet_lift_frame : k_jet_start_fields+1;
        if(ascend || descend || f.epoch!=jet_lift_epoch || age>k_jet_start_fields) {
            jet_lift_active=false;
            if(!ascend && !descend) {
                jet_hold_y=y;jet_hold_valid=true;jet_vel[1]=0;want_y=0;
            }
        } else {
            const float phase=(float)age/(float)k_jet_start_fields;
            const float target=jet_lift_y-k_jet_start_lift*(5.f*phase*phase+1.5f*phase)/6.5f;
            want_y=std::clamp(target-y,-128.f,0.f)*1024.f/dt;
            jet_hold_valid=false;starting=true;
        }
    }
    jet_vel[0]+=(want_x-jet_vel[0])*ease;
    jet_vel[2]+=(want_z-jet_vel[2])*ease;
    // Released: end the ease once under ~1 unit per update; the original's
    // root shift floors, so a residue of -1 would still creep a unit each update.
    if(magnitude==0.f && std::hypot(jet_vel[0],jet_vel[2])*dt<1024.f)jet_vel[0]=jet_vel[2]=0.f;
    // The hold is already a smooth controller; climbs and descents ease.
    jet_vel[1]=starting || (jet_hold_valid && !ascend && !descend) ? want_y : jet_vel[1]+(want_y-jet_vel[1])*ease_vertical;
    if(magnitude==0.f && !ascend && !descend)++jet_hover_engages;
    // Host-owned flight: the original's hover lock stays off. Moving or
    // climbing, input_pad feeds the original its own pads (flame, lean poses,
    // thrust state, fuel drain); its thrust v then lands on top of bv
    // (8003ea58: bv += v*dt, root = (bv*dt + v*dt/2) >> 10), so write the
    // target minus the thrust predicted from the previous update.
    if(psx_mod_read_word(player+0x224)&0x08000000u)psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x08000000u);
    jet_pads_fed=magnitude>0.f || ascend;
    int32_t bv[3];
    for(int axis=0;axis<3;++axis)bv[axis]=(int32_t)std::lround(jet_vel[axis]);
    const int32_t step_dt=(int32_t)dt;
    float vertical=jet_vel[1]*dt/1024.f; // planned vertical root, units
    jet_root_planned=vertical;
    bv[1]=(int32_t)std::lround((vertical-jet_root_bias)*1024.f/dt);
    for(int axis=0;axis<3;++axis) {
        if(jet_pads_fed)bv[axis]-=jet_thrust_prev[axis]*step_dt+jet_thrust_prev[axis]/2;
        psx_mod_write_word(player+0x1f4+4*axis,(uint32_t)bv[axis]);
        psx_mod_write_word(player+0x1e4+4*axis,0); // the handler sets this update's thrust
    }
    // Fuel: the original drains dt per update while it thrusts (fed pads) or
    // hovers; hovering and Ctrl alone feed it nothing, so drain for it (its
    // fuel < dt check then cuts out).
    if(!jet_pads_fed && !psx_mod_read_half(0x800c3cc4)) {
        const int16_t fuel=(int16_t)psx_mod_read_half(player+0x35a);
        psx_mod_write_half(player+0x35a,(uint16_t)std::max<int32_t>(0,fuel-(int32_t)dt));
    }
    if(jet_trace())std::fprintf(stderr,"ttk-jet dt=%.0f anim=%u st=%u/%u flags=%08x xyz=%d,%d,%d h=%u foot=%d floor=%d vel=%.0f/%.0f/%.0f want=%.0f/%.0f/%.0f fuel=%d asc=%d desc=%d hold=%d/%d\n",
        dt,animation,mode,psx_mod_read_byte(player+0x22d),psx_mod_read_word(player+0x224),
        (int32_t)psx_mod_read_word(player+4),y,(int32_t)psx_mod_read_word(player+12),psx_mod_read_half(player+0x1c),
        (int32_t)psx_mod_read_word(player+0x10),(int32_t)psx_mod_read_word(player+0x1c8),
        jet_vel[0],jet_vel[1],jet_vel[2],want_x,want_y,want_z,(int16_t)psx_mod_read_half(player+0x35a),ascend,descend,jet_hold_valid,jet_hold_y);
}
// D08Q8: resolve the integrated root before the native axis-separated apply.
// The native sweep remains the collision oracle. Every trial restores actor
// state; only a proven clear path is committed. A fixed flight envelope avoids
// the animated feet changing the contact height on every thrust pose.
struct JetSurface {
    bool valid=false;
    uint64_t epoch=0;
    int x=0,y=0,z=0,kind=0,nx=0,ny=0,nz=0,px=0,py=0,pz=0;
    double remainder=0;
};
static JetSurface jet_surface;
static void jet_clearance_step(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x8004b570 || guest_busy ||
       !jetpack_input_ready())return;
    const bool classic=jetpack_classic();
    uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    auto restore=[&]{for(unsigned i=0;i<0x8a4/4;++i)
        if(psx_mod_read_word(player+4*i)!=saved[i])psx_mod_write_word(player+4*i,saved[i]);};
    const int32_t x=(int32_t)saved[1],y=(int32_t)saved[2],z=(int32_t)saved[3];
    // 8003964c(player,15): the native sweep reads a cached world-space
    // head point. Translate that point with each isolated trial as well.
    const uint32_t layout=saved[0x40/4],matrices=saved[0x3c/4];
    if(layout<0x80010000u || layout>=0x801ffff0u || matrices<0x80010000u || matrices>=0x801fe000u)return;
    const unsigned head_index=psx_mod_read_byte(layout+0x24+15);
    if(head_index>=64)return;
    const uint32_t head=matrices+32*head_index+0x14;
    const uint32_t head_saved[3]={psx_mod_read_word(head),psx_mod_read_word(head+4),psx_mod_read_word(head+8)};
    const int dx=(int16_t)psx_mod_read_half(player+0xfc);
    const int dz=(int16_t)psx_mod_read_half(player+0x100);
    const int dy=classic ? (int16_t)psx_mod_read_half(player+0xfe) : (int)std::lround(jet_root_planned);
    int hit=0,nx=0,ny=0,nz=0,px=0,py=0,pz=0;bool failed=false;
    JetSurface contact_surface;
    auto sweep=[&](int ox,int oy,int oz,int sx,int sy,int sz) {
        restore();
        psx_mod_write_word(player+4,x+ox);psx_mod_write_word(player+8,y+oy-32);
        psx_mod_write_word(player+12,z+oz);
        psx_mod_write_word(player+0x10,y+oy+624);
        psx_mod_write_half(player+0xfc,(uint16_t)sx);
        psx_mod_write_half(player+0xfe,(uint16_t)(sy?sy:1));
        psx_mod_write_half(player+0x100,(uint16_t)sz);
        psx_mod_write_word(head,x+ox);
        psx_mod_write_word(head+4,y+oy-384);
        psx_mod_write_word(head+8,z+oz);
        int32_t result=-1;const bool ok=original_call(cpu,0x8007a98c,0,&result);
        for(unsigned axis=0;axis<3;++axis)psx_mod_write_word(head+4*axis,head_saved[axis]);
        hit=result;nx=(int32_t)psx_mod_read_word(player+0x194);ny=(int32_t)psx_mod_read_word(player+0x198);nz=(int32_t)psx_mod_read_word(player+0x19c);
        px=(int32_t)psx_mod_read_word(player+0x184);py=(int32_t)psx_mod_read_word(player+0x188);pz=(int32_t)psx_mod_read_word(player+0x18c);
        restore();if(!ok)failed=true;
        // Native vertical apply accepts result 1: an adjacent vertical wall
        // does not obstruct travel parallel to it. Keep that contract for
        // vertical/stationary probes only; horizontal wall crossings still
        // require a clear sweep.
        return ok && (result==0 || (result==1 && ny==0 && !sx && !sz));
    };
    // Validate both the swept route and the complete destination envelope.
    auto clear=[&](int ox,int oy,int oz,int sx,int sy,int sz) {
        const bool path=sweep(ox,oy,oz,sx,sy,sz);
        const bool separating=!sx && !sz && ((hit==3 && sy>0) || (hit==2 && sy<0));
        return (path || separating) && sweep(ox+sx,oy+sy,oz+sz,0,0,0);
    };
    auto diagonal=[&](int sx,int sy,int sz) {
        const int collision[]={hit,nx,ny,nz,px,py,pz};
        if(clear(0,0,0,sx,sy,sz))return true;
        // A failed alternate route must not replace the original contact
        // plane with an unrelated side-wall result.
        hit=collision[0];nx=collision[1];ny=collision[2];nz=collision[3];
        px=collision[4];py=collision[5];pz=collision[6];return false;
    };
    // Plane intersection supplies the clearance target in one calculation.
    // Keep the query count bounded: probe-heavy binary searches themselves
    // consume guest work and stretch the game's animation timestep.
    auto boundary=[&](int tx,int tz,int& target) {
        if(std::abs(ny)<512 || (hit!=2 && hit!=3))return false;
        const double surface=py-((double)nx*(x+tx-px)+(double)nz*(z+tz-pz))/ny;
        const double radius=275.0*(saved[0xb0/4]&0xffffu)/4096.0;
        const double reach=(tx || tz) ? radius*std::hypot((double)nx,(double)nz)/std::abs(ny) : 0.0;
        target=(int)std::lround(surface+(hit==3?reach:-reach))+(hit==3 ? 416 : -656)-y;
        if(jet_trace() && (tx || tz))std::fprintf(stderr,"ttk-jplane hit=%d p=%d,%d,%d n=%d,%d,%d radius=%.0f target=%d\n",hit,px,py,pz,nx,ny,nz,radius,target);
        contact_surface={true,input_snapshot(Context::Gameplay).epoch,0,0,0,hit,nx,ny,nz,px,py,pz};
        return std::abs(target)<32768;
    };
    // A freshly opened pack starts with a zero lift root and native feet
    // still on the ground. Preserve the original upward-flight departure
    // (the rising sweep shortens its foot envelope by 256) until airborne.
    // Otherwise the zero-root native follow-up can immediately land it again.
    if(!classic && !dx && !dz && dy==0 && !clear(0,0,0,0,0,0) && hit==2 && !failed) {
        restore();psx_mod_write_half(player+0xfe,(uint16_t)-2);
        jet_hold_y=y-2;jet_hold_valid=true;return;
    }
    int vy=dy,mx=dx,mz=dz;bool contact=false,following=false;
    const auto& flight_input=input_snapshot(Context::Gameplay);
    // Once touching a slope, carry its tangent into ordinary WASD flight.
    // Waiting for another penetration produces alternating level travel and
    // height corrections. Confirm the plane locally every update: never
    // extrapolate a remembered surface across a ledge, save load or teleport.
    if(!classic && (mx || mz) && !flight_input.held[jump] &&
       !flight_input.held[crouch] && !jet_lift_active && jet_surface.valid &&
       jet_surface.epoch==flight_input.epoch && jet_surface.x==x &&
       jet_surface.y==y && jet_surface.z==z) {
        const auto previous=jet_surface;
        double exact=-((double)previous.nx*mx+(double)previous.nz*mz)/previous.ny+previous.remainder;
        int tangent=(int)std::lround(exact);
        int tx=mx,tz=mz;
        if(std::abs(tangent)>48) {
            tx=tx*48/std::abs(tangent);tz=tz*48/std::abs(tangent);
            exact=-((double)previous.nx*tx+(double)previous.nz*tz)/previous.ny+previous.remainder;
            tangent=(int)std::lround(exact);
        }
        sweep(tx,tangent,tz,0,previous.kind==3?-160:160,0);
        const double plane_error=(double)previous.nx*(px-previous.px)+
            (double)previous.ny*(py-previous.py)+(double)previous.nz*(pz-previous.pz);
        if(!failed && hit==previous.kind && nx==previous.nx && ny==previous.ny && nz==previous.nz &&
           std::abs(plane_error)<=8.0*std::abs(previous.ny)) {
            vy=tangent;mx=tx;mz=tz;following=true;contact=true;contact_surface=previous;
            // Retain sub-unit travel so rounding cannot slowly drift back
            // into the plane and trigger another corrective jump.
            contact_surface.remainder=exact-tangent;
        }
    }
    jet_surface.valid=false;
    if(vy && !following && !clear(0,0,0,0,vy,0)) {
        if(jet_trace())std::fprintf(stderr,"ttk-jvertical hit=%d p=%d,%d,%d n=%d,%d,%d root=%d,%d\n",hit,px,py,pz,nx,ny,nz,y,vy);
        if(hit==2 && (classic || dy<0)) {
            restore();
            // The native thrust can reverse Modern's integrated root during
            // departure; use the requested rising root on this fallback too.
            if(!classic)psx_mod_write_half(player+0xfe,(uint16_t)std::min(-2,dy));
            return; // native takeoff / Classic landing
        }
        int target=0;
        if(boundary(0,0,target)) {
            vy=dy<0 ? std::clamp(target,dy,0) : std::clamp(target,0,dy);
            if(vy && !clear(0,0,0,0,vy,0))vy=0;
        } else vy=0;
        contact=true;
    }
    // Follow the actual diagonal route, not a vertical-then-horizontal elbow.
    // The elbow can intersect a slope even when the intended route fits.
    if((mx || mz) && !clear(0,vy,0,mx,0,mz) && !diagonal(mx,vy,mz)) {
        int target=0;const int normal_y=ny;
        bool found=false,corrected=false;
        if(boundary(mx,mz,target) && std::abs(target)<=384) {
            const int next=std::clamp(target,-48,48);
            // Scale the whole route before validating it. Testing the full
            // horizontal step at a partial height creates stop/start flight.
            int tx=mx,tz=mz;
            if(std::abs(target)>48) {tx=tx*48/std::abs(target);tz=tz*48/std::abs(target);}
            if((clear(0,0,0,0,next,0) && clear(0,next,0,tx,0,tz)) ||
               clear(0,0,0,tx,next,tz)) {
                vy=next;mx=tx;mz=tz;contact=true;found=true;corrected=true;
            } else if(clear(0,0,0,0,next,0)) {
                vy=next;contact=true;corrected=true;
            }
        } else {
            // A vertical stair face has no height component in its normal.
            // A bounded clear route above/below it permits a gradual step;
            // both paths must fit, so full walls and narrow gaps stay solid.
            const int direction=normal_y>512?1:-1;
            for(int sign:{direction,-direction}) {
                const int trial=sign*320;
                if(clear(0,0,0,0,trial,0) && clear(0,trial,0,mx,0,mz)) {
                    vy=sign*32;contact=true;corrected=true;
                    found=clear(0,vy,0,mx,0,mz);
                    if(!found)mx=mz=0;
                    break;
                }
            }
        }
        if(!found) {
            mx=mz=0;
            // Tangent motion belongs to horizontal progress. A blocking wall
            // must not turn ordinary WASD into an unrequested vertical climb.
            if(following && !corrected) {vy=0;contact_surface.valid=false;}
        }
    }
    if(failed) {restore();return;}
    restore();
    psx_mod_write_word(player+4,x+mx);psx_mod_write_word(player+8,y+vy);psx_mod_write_word(player+12,z+mz);
    // Native apply now sees a stationary root. Horizontal travel no longer
    // requires alternating vertical nudges, including at a flat ceiling.
    for(unsigned axis=0;axis<3;++axis)psx_mod_write_half(player+0xfc+2*axis,0);
    if(contact && !classic) {
        if(contact_surface.valid) {
            jet_surface=contact_surface;jet_surface.x=x+mx;jet_surface.y=y+vy;jet_surface.z=z+mz;
        }
        jet_vel[1]=0;jet_hold_y=y+vy;jet_hold_valid=true;jet_air_height_hold=true;
        jet_lift_active=false;
    }
    jet_root_bias=0;
    if(jet_trace())std::fprintf(stderr,"ttk-jclear xyz=%d,%d,%d root=%d,%d,%d applied=%d,%d,%d contact=%d follow=%d\n",
        x,y,z,dx,dy,dz,mx,vy,mz,contact,following);
}
// After the mode-10 handler (0x80058120 hook): remember the original's thrust
// for the next update's cancellation, and show the jetpack flame (drawn from
// +0x224 & 0x82000000: both native thrust/render bits are required) while
// Duke hovers or descends on the host.
static void jet_post_update(CPUState* cpu) {
    if(jet_fuel_pending) {
        jet_fuel_pending=false;
        if(input_modernized() && jetpack_eduke32() && player_identity_ready()) {
            // Preserve a native pickup or god-mode top-up during this update.
            const int current=(int16_t)psx_mod_read_half(player+0x35a);
            if(current>jet_fuel_seed) {
                jet_fuel_after=current;jet_fuel_fraction=0;jet_fuel_expected=current;
            }
            psx_mod_write_half(player+0x35a,jet_fuel_after);
            // Native collision may land or submerge Duke. Keep that result,
            // close the pack, and avoid leaving a ground-only Space restart.
            if(psx_mod_read_byte(player+0x22c)!=10 && (psx_mod_read_half(player+0x358)&2))
                jet_eduke_off(cpu,false);
        }
    }
    if(!jet_flight_live || !jetpack_flying_early() || !jetpack_input_ready() || jetpack_classic())return;
    for(int axis=0;axis<3;++axis)jet_thrust_prev[axis]=jet_pads_fed ? (int32_t)psx_mod_read_word(player+0x1e4+4*axis) : 0;
    jet_root_bias=0; // D08Q8 uses the exact planned root at the collision hook.
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|0x82000000u);
    if(jet_trace())std::fprintf(stderr,"ttk-jpost anim=%u fed=%d xyz=%d,%d,%d v=%d/%d/%d root=%d/%d/%d\n",psx_mod_read_half(player+0x60),jet_pads_fed,
        (int32_t)psx_mod_read_word(player+4),(int32_t)psx_mod_read_word(player+8),(int32_t)psx_mod_read_word(player+12),
        jet_thrust_prev[0],jet_thrust_prev[1],jet_thrust_prev[2],
        (int16_t)psx_mod_read_half(player+0xfc),(int16_t)psx_mod_read_half(player+0xfe),(int16_t)psx_mod_read_half(player+0x100));
}
