// D11 first-person prototype (Modernized, independent camera only).
//
// The eye follows Duke's current neck joint. The normal-camera update, its
// four-ray obstruction solve, room and height queries all still run; only the
// requested anchor changes, as for the D06 orbit. Original details it relies on:
// - 0x80039c7c places the camera at anchor - forward*camera[+0x42]/2 (the
//   projection distance, 386 = 160/tan(22.5 deg) from the 45 degree angle at
//   camera[+0x40]), so the anchor is set that far ahead of the eye.
// - 0x8003aa48 pushes any camera closer than the word at 0x800c3c48 (768)
//   back out to that distance. Only that routine reads the word. It is relaxed
//   at that call while the eye view is blending in and restored before the
//   same camera update ends, so no save state or later frame sees it.
// - 0x8003ade4 then eases the camera toward the solved anchor per view axis:
//   0x8002a038(delta, rate) returns delta/rate, with rates from the state
//   table at 0x800c0d4c (2/5/4 on the ground). A trailing eye would put the
//   view behind Duke's neck while running, so the eye view scales those rates
//   toward 1 (no lag) as it blends in. Only the three calls from that update.
// - The world renderer flags vertices nearer than GTE H/2 (0x8002f1e0), and
//   a wall Duke faces at the original H (386) drops out to black. The render
//   loads GTE H from camera[+0x42] each frame via 0x800b4d9c (from 0x8002e4c8);
//   the eye view passes a shorter one there (256, about 64 degrees wide) and
//   never writes camera[+0x42]. Walls met at a steep angle can still lose the
//   polygons that reach beside or behind the eye: the renderer rejects them
//   rather than clipping, and moving the eye does not avoid it.
// - 0x800348d8 draws an actor joint by joint and skips a joint whose record
//   flag byte has bit 0 set. The head joint is flagged only between Duke's
//   draw entry and the object-list step that follows it.
// Unsupported states without the orbit lease return to the third-person orbit.
static constexpr uint32_t min_boom_word=0x800c3c48, min_boom_original=768;
// Eye above the neck pivot, in game units. It stays over the neck (not ahead
// of it) to keep the most room between the eye and a wall Duke touches.
static constexpr double eye_height=96;
static constexpr int default_eye_projection=256;
// Developer tuning only (not a saved preference): DNTTK_FP_PROJECTION=160..386.
static int eye_projection() {
    static const int value=[]{
        const char* text=std::getenv("DNTTK_FP_PROJECTION");
        char* end=nullptr;const long v=text?std::strtol(text,&end,10):0;
        return text && end && !*end && v>=160 && v<=386 ? (int)v : default_eye_projection;
    }();
    return value;
}
static double eye_offset[3];
static bool eye_offset_valid;
static ActionPerspective fp_policy;
static bool min_boom_written;
static uint32_t head_flag, backpack_flag;
static uint64_t fp_updates, fp_fallbacks, head_hides, projections;
static int last_projection;

static int head_joint() {
    const uint32_t desc=duke_model();
    const int joint=desc ? duke_part_joint(desc,part_neck) : -1;
    // Record byte 2 is the hierarchy level; the neck is a level-3 child of the spine.
    return joint>=0 && psx_mod_read_byte(desc+0x44+0x28*joint+2)==3 ? joint : -1;
}
static uint32_t head_record() {
    const int joint=head_joint();
    return joint>=0 ? psx_mod_read_word(player+0x40)+0x44+0x28*joint : 0;
}
static bool neck_origin(double* xyz) {
    const uint32_t mats=psx_mod_read_word(player+0x3c);
    const int joint=head_joint();
    if(joint<0 || mats<0x80010000 || mats>0x801fff00) return false;
    for(int i=0;i<3;++i) xyz[i]=(int32_t)psx_mod_read_word(mats+0x20*joint+0x14+4*i);
    return true;
}
// Restores the executable's constant, so a value that was ever captured
// relaxed (a save state or debug read mid-update) cannot become the original.
static void min_boom_restore() {
    const uint32_t value=psx_mod_read_word(min_boom_word);
    if(!min_boom_written && value>=min_boom_original) return;
    if(value<=min_boom_original) psx_mod_write_word(min_boom_word,min_boom_original);
    min_boom_written=false;
}
static void head_restore() {
    for(uint32_t* flag:{&head_flag,&backpack_flag}) {
        if(*flag) psx_mod_write_byte(*flag,psx_mod_read_byte(*flag)&~1u);
        *flag=0;
    }
}
// 0x800b4d9c(H) entry from the render's per-frame load of camera[+0x42].
static void first_person_projection(CPUState* cpu,uint32_t ra) {
    const int original=(int16_t)psx_mod_read_half(camera+0x42);
    if(ra!=0x8002e4d0 || fp_blend<=0 || !orbit_valid || (int32_t)cpu->gpr[4]!=original ||
       original<128 || original>1024) return;
    last_projection=(int)std::lround(original+(eye_projection()-original)*fp_blend);
    cpu->gpr[4]=(uint32_t)last_projection;
    ++projections;
}
static void first_person_release(PerspectiveReason reason) {
    if(fp_blend>0) ++fp_fallbacks;
    fp_blend=0;eye_offset_valid=false;
    PerspectiveFacts facts{};facts.selected=input_snapshot(Context::Gameplay).first_person;
    facts.denial=reason;fp_policy=resolve_perspective(facts);
    min_boom_restore();head_restore();
}
// Presentation only: never use the action lease here (pause/focus retain the
// selected view). The normal-camera owner/identity has already been checked.
static bool first_person_swimming() {
    return in_water_early() && !shallow_land_water_early();
}
// D35E: ladder-exclusive exits can already be attached to the landing.
// Shared grab/transfer clips require the actual plain-ladder owner; a pole,
// chain or ledge with the same clip must retain its own perspective policy.
static bool first_person_ladder() {
    const unsigned a=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), prev=psx_mod_read_byte(player+0x22d);
    if(mode==9 && prev==3 && a==108 && fp_policy.action==PerspectiveAction::Ladder)return true;
    if(mode==8 && (prev==3 || prev==8) && (a==185 || a==190))return true;
    if(!plain_ladder(object_type_flags(psx_mod_read_word(player+0x17c))))return false;
    if(mode==8 && prev==0 && a==185)return true; // ground mount before mode history catches up
    return climb_state_early() && (a==147 || a==148 || a==156 ||
        (a>=185 && a<=190) || a==207 || a==210 || a==211);
}
// D35F: pole-class ownership, not scenery names, distinguishes shared catches.
static bool first_person_pole() {
    const unsigned a=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c),prev=psx_mod_read_byte(player+0x22d);
    if(mode==9 && prev==3 && a==108 && fp_policy.action==PerspectiveAction::Pole)return true;
    if(mode==8 && (prev==3 || prev==8) && (a==191 || a==196))return true;
    if(!pole_object(object_type_flags(psx_mod_read_word(player+0x17c))))return false;
    return climb_state_early() && (a==147 || a==148 || a==154 || (a>=191 && a<=196));
}
// D35I: wall-owned catches/transfers and the exclusive mount/exit clips.
static bool first_person_wall() {
    const unsigned a=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c),prev=psx_mod_read_byte(player+0x22d);
    if(mode==9 && prev==3 && a==108 && fp_policy.action==PerspectiveAction::Wall)return true;
    if(mode==8 && (prev==3 || prev==8) && (a==197 || a==206))return true;
    if((object_type_flags(psx_mod_read_word(player+0x17c))&0x700u)!=0x100u)return false;
    if(mode==8 && prev==0 && a==197)return true;
    return climb_state_early() && (a==147 || a==148 || a==155 || (a>=197 && a<=206));
}
// D35H: the D35G eye already covers overhead hangs. Split their setting
// by the attached object's climbable flags; crates and bare ledges stay Mantle.
// This predicate authorizes presentation only, never movement or attachment.
static bool first_person_overhead() {
    const unsigned a=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c),prev=psx_mod_read_byte(player+0x22d);
    const bool overhead=(object_type_flags(psx_mod_read_word(player+0x17c))&object_climbable)==0x80u;
    if(mode==9 && prev==7 && a==108)
        return overhead || fp_policy.action==PerspectiveAction::Overhead;
    return overhead && ((mode==7 && a>=149 && a<=151) || (mode==6 && (a==147 || a==148)));
}
// D35G: ledge ownership is distinct from the attached ladder/pole set.
// Water exit owns its early 8/4 or 8/5 tick before swim flags clear.
static bool first_person_mantle() {
    const unsigned a=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c),prev=psx_mod_read_byte(player+0x22d);
    // Native takeoff installs the clip before its dispatcher sets mode 8.
    if(mode==0 && (prev==0 || prev==9) && a>=134 && a<=142)return true;
    if(mode==8 && a>=134 && a<=142)
        return prev==0 || prev==4 || prev==5 || prev==6 || prev==7 || prev==8 || prev==9;
    if(mode==6 && a>=147 && a<=153)
        return prev==0 || prev==6 || prev==7 || prev==8 || prev==9;
    if(mode==7 && a>=149 && a<=153)return prev==6 || prev==7 || prev==9;
    return mode==9 && a==108 && (prev==6 || prev==7 || prev==8);
}
// Once per verified normal-camera update, after orbit_begin.
static void first_person_update(const InputFrame& f,unsigned ticks,bool acquiring=false) {
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    const unsigned animation=psx_mod_read_half(player+0x60);
    PerspectiveFacts facts{};
    facts.selected=f.first_person;facts.active=f.active;
    facts.use=use_interaction_eye();
    facts.mirror=mirror_interaction_eye();
    // Water exit takes ownership before the lingering water flag clears.
    facts.water_exit=mode==8 && (previous==4 || previous==5) && animation>=134 && animation<=142;
    facts.mantle=first_person_mantle();facts.overhead=first_person_overhead();
    facts.wall=first_person_wall();facts.ladder=first_person_ladder();facts.pole=first_person_pole();
    facts.push=push_state_early();facts.traversal=traversal_camera_early();
    facts.committed=committed_move_early();facts.fall=unowned_fall_early();
    facts.swim=first_person_swimming();facts.jet=mode==10 && jetpack_anim(animation);
    fp_policy=resolve_perspective(facts);
    if(fp_policy.action==PerspectiveAction::Use)
        fp_policy=resolve_perspective(facts,use_view());
    if(fp_policy.action==PerspectiveAction::Mirror)
        fp_policy=resolve_perspective(facts,mirror_view());
    // Only the resolved swimming owner gets this preference. Lingering water
    // flags must not apply it to a mantle, fall or another owning action.
    if(fp_policy.action==PerspectiveAction::Swimming)
        fp_policy=resolve_perspective(facts,swimming_view());
    if(fp_policy.action==PerspectiveAction::Jetpack)
        fp_policy=resolve_perspective(facts,jetpack_view());
    if(fp_policy.action==PerspectiveAction::Push)
        fp_policy=resolve_perspective(facts,push_view());
    if(fp_policy.action==PerspectiveAction::Ladder)
        fp_policy=resolve_perspective(facts,ladder_view());
    if(fp_policy.action==PerspectiveAction::Pole)
        fp_policy=resolve_perspective(facts,pole_view());
    if(fp_policy.action==PerspectiveAction::Mantle || fp_policy.action==PerspectiveAction::WaterExit)
        fp_policy=resolve_perspective(facts,mantle_view());
    if(fp_policy.action==PerspectiveAction::Wall)
        fp_policy=resolve_perspective(facts,wall_view());
    if(fp_policy.action==PerspectiveAction::Overhead)
        fp_policy=resolve_perspective(facts,overhead_view());
    const bool wanted=fp_policy.effective;
    // A newly authenticated world starts in the selected view. Never blend
    // from an unrelated old save/camera or flash a third-person load frame.
    if(acquiring)fp_blend=wanted?1.0:0.0;
    if(!wanted && fp_blend==0) {eye_offset_valid=false;return;}
    if(ticks) {
        const double blend=1-std::pow(0.7,ticks);
        fp_blend+=((wanted?1.0:0.0)-fp_blend)*blend;
        if(std::abs(fp_blend-(wanted?1.0:0.0))<0.01) fp_blend=wanted?1:0;
    }
    if(fp_blend==0) {eye_offset_valid=false;min_boom_restore();}
}
// D17C: a stable eye, and view bob as its own deliberate camera motion.
// Duke's idle and walk animations move his root (player+8) up and down (idle
// about 18 units, walking about 55) and sway the neck a few units; the eye rode
// on both, and the game's camera code turns the view toward the anchor from
// that eye (0x8002a524), so the whole room seemed to breathe while standing
// still. The eye is now placed on the original camera's pivot (player+0x7bc):
// x/z equal the root, y stays level through idle and gait and follows real
// height changes (steps, crouching, jumping, falling) at once. The head offset
// on top is smoothed over about half a second (large changes, such as a crouch
// moving the head, follow quickly). View bob is then added on purpose (D17V).
// DNTTK_VIEW_BOB: off, subtle, on (default), strong, or a scale (0..3).
// D19: parsed again when the OPTIONS screens change it.
static double view_bob_scale() {
    static const char* parsed=nullptr; static double v=1.0; static bool have=false;
    const char* t=ttk_live_env("DNTTK_VIEW_BOB");
    if(have && t==parsed) return v;
    have=true; parsed=t;
    if(!t || !*t || !std::strcmp(t,"on")) v=1.0;
    else if(!std::strcmp(t,"off") || !std::strcmp(t,"0")) v=0.0;
    else if(!std::strcmp(t,"subtle")) v=0.5;
    else if(!std::strcmp(t,"strong")) v=2.0;
    else {char* end=nullptr; const double d=std::strtod(t,&end); v=end && !*end && d>=0 && d<=3 ? d : 1.0;}
    return v;
}
// D17V: Duke 3D's view bob (EDuke32 P_ProcessInput): while Duke moves on the
// ground its phase advances a fixed 52/2048 of a turn per 30 Hz tic, and the
// eye sinks by |sin(phase)| times a depth proportional to speed, so walking
// and running share one tempo (about 1.5 dips a second) and running only
// dips deeper; no sideways sway. The D17C bob advanced its phase by distance,
// so running (5 times walking speed) dipped about 9 times a second, close to
// TTK's 30 Hz game frames, and per-frame root steps made it jitter.
// Time here is the host vblank sequence. Speed is the velocity of a glide
// that follows Duke's root: the root surges with the stride (running 57..155
// units per game frame, about 10 Hz), while Duke 3D's camera moves at an even
// speed. The glide carries a low-passed velocity, is pulled back toward the
// root, and is bounded to bob_lead ahead of it along the motion (so the eye
// never pushes far toward a wall Duke runs at) and bob_lag behind or beside
// it; when Duke stops it zeroes its velocity and settles back onto the root.
// Depth: bob_depth at walking speed (bob_walk_speed per vblank), growing as
// speed^0.43 to twice that at running speed (about 50), easing out in the air.
// View bob off keeps the plain D17C eye (no glide, no bob).
static constexpr double bob_depth=16, bob_walk_speed=10, bob_rate=26.0/2048*2*3.14159265358979;
static constexpr double bob_lead=16, bob_lag=64;
static double eye_rel[3], bob_phase, bob_amount, bob_last[3], bob_dip, bob_glide[2], bob_velocity[2];
static uint64_t bob_frame;
static unsigned bob_still;
// D11E: a swimming head pitches through the whole stroke (over 400 units of
// vertical pose change). Keep the eye on the body, with the floating neck's
// measured height (about 198) plus the existing 96-unit eye offset. Surface
// mode uses the water plane so the idle stroke cannot bob across it.
static constexpr double swim_eye_above_root=294;
// D11F: flight has the same measured upright neck height (about 198).
// Anchor above the root so native flight lean cannot move the eye sideways.
static constexpr double jet_eye_above_root=294;
static bool eye_was_root_anchored;
static double eye_reference_offset;

static bool eye_rel_valid, bob_last_valid;
uint64_t bob_updates;
// Requested anchor for the eye view; false when the neck joint is unavailable.
static bool first_person_anchor(const double* forward,double* anchor) {
    double neck[3];
    if(!neck_origin(neck)) return false;
    double root[3],pivot_y=0;
    for(int i=0;i<3;++i) root[i]=(int32_t)psx_mod_read_word(player+4+4*i);
    const bool pivoted=psx_mod_read_word(camera+0x50)==player+0x7bc;
    pivot_y=pivoted ? (int32_t)psx_mod_read_word(player+0x7c0) : root[1];
    const bool mantling=(fp_policy.action==PerspectiveAction::Mantle || fp_policy.action==PerspectiveAction::Overhead || fp_policy.action==PerspectiveAction::WaterExit) && fp_policy.effective;
    const bool swimming=first_person_swimming() && !mantling;
    const bool jetting=jetpack_flying_early();
    const bool pushing=fp_policy.action==PerspectiveAction::Push && fp_policy.effective;
    const bool using_object=(fp_policy.action==PerspectiveAction::Use || fp_policy.action==PerspectiveAction::Mirror) && fp_policy.effective;
    const bool fidget=idle_fidget_eye();
    const bool climbing=(fp_policy.action==PerspectiveAction::Wall || fp_policy.action==PerspectiveAction::Ladder || fp_policy.action==PerspectiveAction::Pole) && fp_policy.effective;
    const bool root_anchored=swimming || jetting || pushing || climbing || mantling || using_object || fidget;
    if(root_anchored) pivot_y=root[1];
    // Rebase the previous offset when switching between root and land pivot;
    // real body travel stays immediate, only the posture difference is eased.
    if(eye_rel_valid && eye_offset_valid && root_anchored!=eye_was_root_anchored)
        eye_rel[1]+=eye_reference_offset-(pivot_y-root[1]);
    eye_reference_offset=pivot_y-root[1];eye_was_root_anchored=root_anchored;
    // Eye relative to its references: x/z to the root, y to the pivot.
    // Game Y grows downward.
    double target[3]={neck[0]-root[0],neck[1]-eye_height-pivot_y,neck[2]-root[2]};
    if(swimming) {
        target[0]=target[2]=0;
        target[1]=psx_mod_read_byte(player+0x22c)==4 ?
            (int32_t)psx_mod_read_word(player+0x834)-eye_height-root[1] : -swim_eye_above_root;
    }
    if(jetting) {
        target[0]=target[2]=0;
        target[1]=-jet_eye_above_root;
    }
    // D35D: the grab leans the animated neck toward the object. Keep the
    // eye over the body at standing height; hands retain their native pose.
    // D35E uses the same stable upright offset: the native ladder root is
    // 200 units off its panel, so looking around cannot put the eye through it.
    // D35G: follow native root travel over the lip, without the animated
    // neck lean or a stale swimming plane pulling the eye into the ledge.
    // Idle gestures and committed use animations may lean or turn the neck
    // sharply. Keep a comfortable upright eye while native arms/events run.
    if(pushing || climbing || mantling || using_object || fidget) {target[0]=target[2]=0;target[1]=-294;}
    // D15: the slow centring drift was still feeding idle neck animation into
    // the camera after locomotion stopped. Hold within the existing dead band
    // while stationary; actual travel, posture changes beyond the band and
    // pivot height changes keep their existing response. No extra camera state.
    const bool hold_centre=pgxp_mesh_geometry() && bob_last_valid &&
        root[0]==bob_last[0] && root[2]==bob_last[2];
    for(int i=0;i<3;++i) {
        // Dead band: idle breathing and gait stay inside it and are held
        // (a very slow drift keeps the moving eye centred); beyond it the eye follows
        // the excess promptly.
        const double d=target[i]-eye_rel[i],band=i==1?64.0:24.0;
        const double step=std::abs(d)>band ? (d-std::copysign(band,d))*0.5 : (hold_centre?0.0:d*0.004);
        // Swim/flight offsets converge completely, including surface clearance;
        // the land dead band would retain the previous posture indefinitely.
        const double anchored_step=d*0.5;
        eye_rel[i]=eye_rel_valid && eye_offset_valid ? eye_rel[i]+(root_anchored?anchored_step:step) : target[i];
        eye_offset[i]=eye_rel[i];
    }
    const bool was_valid=eye_offset_valid;
    eye_rel_valid=eye_offset_valid=true;
    // Deliberate view bob and glide (D17V).
    const double scale=root_anchored?0.0:view_bob_scale();
    const uint64_t frame=input_host_frame();
    const bool advanced=frame>bob_frame;
    if(scale>0 && bob_last_valid && was_valid && advanced) {
        const double frames=(double)std::min<uint64_t>(frame-bob_frame,8);
        const double d[2]={root[0]-bob_last[0],root[2]-bob_last[2]};
        const double dist=std::hypot(d[0],d[1]);
        if(dist>=400) {bob_glide[0]=bob_glide[1]=bob_velocity[0]=bob_velocity[1]=0;}   // a teleport is not a step
        else {
            // One still update can be a stride's pause; two are a stop.
            bob_still=dist==0 ? bob_still+1 : 0;
            if(bob_still>=2) bob_velocity[0]=bob_velocity[1]=0;
            else if(dist>0)
                for(int i=0;i<2;++i) bob_velocity[i]+=(d[i]/frames-bob_velocity[i])*(1-std::pow(0.9,frames));
            // Glide offset from the root: advance by the velocity, less the
            // root's own step, then pull back toward the root.
            const double pull=std::pow(0.85,frames);
            for(int i=0;i<2;++i) bob_glide[i]=(bob_glide[i]+bob_velocity[i]*frames-d[i])*pull;
            const double v=std::hypot(bob_velocity[0],bob_velocity[1]);
            if(v>1e-6) {
                const double u[2]={bob_velocity[0]/v,bob_velocity[1]/v};
                const double along=std::clamp(bob_glide[0]*u[0]+bob_glide[1]*u[1],-bob_lag,bob_lead);
                const double side=std::clamp(-bob_glide[0]*u[1]+bob_glide[1]*u[0],-bob_lag,bob_lag);
                bob_glide[0]=along*u[0]-side*u[1];bob_glide[1]=along*u[1]+side*u[0];
            } else if(const double m=std::hypot(bob_glide[0],bob_glide[1]); m>bob_lag)
                for(double& g:bob_glide) g*=bob_lag/m;
        }
        // Ground locomotion only (mode 0); jumps and falls ease the dip out.
        const double v=std::hypot(bob_velocity[0],bob_velocity[1]);
        const bool ground=psx_mod_read_byte(player+0x22c)==0;
        const double target=ground && v>0.5 ?
            std::min(2.0,std::pow(v/bob_walk_speed,0.43))*std::min(1.0,v/3) : 0;
        bob_amount+=(target-bob_amount)*(1-std::pow(0.87,frames));
        if(bob_amount<0.005 && target==0) bob_amount=0;
        // Like Duke 3D, the phase holds while standing, so the next walk
        // starts from the same point at zero depth.
        if(target>0 || bob_amount>0)
            bob_phase=std::fmod(bob_phase+bob_rate*frames,3.14159265358979);
    } else if(scale<=0 || !bob_last_valid || !was_valid) {
        bob_amount=bob_glide[0]=bob_glide[1]=bob_velocity[0]=bob_velocity[1]=0;bob_still=0;
    }
    if(advanced || !bob_last_valid) {
        for(int i=0;i<3;++i) bob_last[i]=root[i];
        bob_frame=frame;
    }
    bob_last_valid=true;
    bob_dip=scale>0 ? bob_depth*bob_amount*scale*std::sin(bob_phase) : 0;   // game Y grows downward
    if(std::getenv("DNTTK_BOB_LOG"))
        std::fprintf(stderr,"[TTK bob] frame %llu root %.0f %.0f %.0f speed %.2f amount %.3f phase %.3f dip %.2f glide %.1f %.1f\n",
            (unsigned long long)frame,root[0],root[1],root[2],std::hypot(bob_velocity[0],bob_velocity[1]),bob_amount,bob_phase,bob_dip,bob_glide[0],bob_glide[1]);
    ++bob_updates;
    const double half=std::max(0,(int)(int16_t)psx_mod_read_half(camera+0x42))/2.0;
    double eye[3]={root[0]+eye_rel[0]+bob_glide[0],pivot_y+eye_rel[1]+bob_dip,root[2]+eye_rel[2]+bob_glide[1]};
    // D35F: the native root circles 225 units from the attachment axis.
    // Keep the eye over that root, independent of animated neck sway or look.
    // Wider pole boxes may need extra radial clearance; presentation only.
    if(fp_policy.action==PerspectiveAction::Pole && fp_policy.effective &&
       psx_mod_read_byte(player+0x22c)==3) {
        const uint32_t object=psx_mod_read_word(player+0x17c);
        PoleAxis axis;int16_t box[8];
        if(pole_object(object_type_flags(object)) && pole_axis(object,axis) && ladder_box(object,box)) {
            const double dx=eye[0]-axis.x,dz=eye[2]-axis.z,r=std::hypot(dx,dz);
            const double clearance=std::hypot((box[4]-box[0])/2.0,(box[6]-box[2])/2.0)+32;
            if(r>1 && r<clearance) {
                eye[0]=axis.x+dx*clearance/r;eye[2]=axis.z+dz*clearance/r;
            }
        }
    }
    for(int i=0;i<3;++i) anchor[i]=eye[i]+forward[i]*half;
    ++fp_updates;
    return true;
}
// 0x8003aa48 entry: relax the original minimum boom while the eye blends in.
// Restored at the camera update's next hooks (0x8002a038, 0x8002a7fc).
static void first_person_constraint() {
    if(fp_blend<=0) {min_boom_restore();return;}
    // Anything above the original is not ours to change.
    if(psx_mod_read_word(min_boom_word)>min_boom_original) return;
    psx_mod_write_word(min_boom_word,(uint32_t)std::lround(min_boom_original*(1-fp_blend)));
    min_boom_written=true;
}
// 0x8002a038 entry from the normal camera's follow easing (0x8003ade4 frame).
static uint64_t fp_follows;
static void first_person_follow(CPUState* cpu,uint32_t ra,uint32_t sp) {
    if(fp_blend<=0 || !orbit_valid || (ra!=0x8003af40 && ra!=0x8003af60 && ra!=0x8003af80) ||
       sp!=orbit_sp-0xb8) return;
    const int32_t rate=(int32_t)cpu->gpr[5];
    if(rate<=1 || rate>64) return;
    cpu->gpr[5]=(uint32_t)std::max<int32_t>(1,(int32_t)std::lround(rate*(1-fp_blend)));
    ++fp_follows;
}
// 0x800348d8 entry for Duke (duke) under the normal camera (eye), and the list
// step after it.
// D11C: a save state captured between Duke's draw entry and that list step
// keeps bit 0 set, and after a load nothing owns it. The original never sets
// bit 0 on a joint record (its only joint flag writes set bit 4, 0x8009d974 and
// 0x800a33c8; no overlay writes the flag byte), so an unowned bit here is a
// stale hide of ours and is cleared, in either view.
static uint64_t head_reclaims;
static void first_person_draw(bool duke,bool eye) {
    head_restore();
    if(!duke) return;
    // Any joint: builds before D11D hid joint 9, which in the cowboy model is
    // not the head, so a save state from them can carry a stale bit there.
    if(const uint32_t desc=duke_model())
        for(unsigned j=0;j<duke_joints;++j) {
            const uint32_t r=desc+0x44+0x28*j;
            if(psx_mod_read_byte(r)&1) {psx_mod_write_byte(r,psx_mod_read_byte(r)&~1u);++head_reclaims;}
        }
    const uint32_t record=head_record();
    if(!record) return;
    if(!eye || fp_blend<0.5 || !orbit_valid) return;
    psx_mod_write_byte(record,psx_mod_read_byte(record)|1);
    head_flag=record;++head_hides;
    // D36K: the pitched swim body carries the back-mounted pack through the
    // eye. Hide only its semantic joint in this primary swimming draw. The
    // native pose, attachment and third-person/reflection draws stay intact.
    if(fp_policy.effective && fp_policy.action==PerspectiveAction::Swimming)
        if(const uint32_t desc=duke_model()) {
            const int joint=duke_part_joint(desc,part_backpack);
            if(joint>=0) {
                backpack_flag=desc+0x44+0x28*joint;
                psx_mod_write_byte(backpack_flag,psx_mod_read_byte(backpack_flag)|1);
            }
        }
}

// D12 first-person weapon: Duke's own right hand joint and the original
// weapon mesh, drawn in front of the eye. 0x800348d8 draws each joint after
// 0x800292a0(camera, joint matrix) loads the GTE transform (camera+0x20 times
// the joint matrix) and returns the joint depth; a joint nearer than GTE H is
// skipped with everything attached to it. In the eye view the hand (model
// record byte +0x33, joint 7) is within about 100 of the eye, so neither it
// nor the weapon was drawn. After the hand's own mesh the same loop draws, for
// that joint only:
// - 0x80033e40(hand matrix, weapon mesh, player+0x238, player+0x856) when
//   player+0x3b8 is 2 (weapon), the mesh from the slot table 0x800c4590;
// - 0x800341e4(hand matrix, slot, camera, muzzle offset) when player word 0
//   has bit 3: the muzzle flash, placed from the hand matrix;
// - 0x80033f5c(hand matrix, slot, camera) when player+0x3b8 is 1 (held item).
// Each of those calls, and the hand's 0x800292a0 call, gets a private copy of
// the live hand matrix instead: its animation (draw, recoil, reload) in a
// level view frame, pushed forward past the near cull, with the view pitch
// applied. Duke's joint matrices are never written.
static constexpr uint32_t hand_record_byte=0x33;
static uint32_t weapon_matrix;
// Duke's draw, from its entry (0x800348d8, ra 0x8003769c) to the list step
// after it (0x8001ca4c, ra 0x800376b4). duke_drawing is not usable here: the
// draw's own callees reach 0x8001ca4c before the joint loop.
static bool weapon_duke_draw, weapon_live;
static bool display_hand_done;
// From the hand joint's transform to the next joint's: the hand mesh and its
// attached draws, which the near-clip object path draws on top (D12).
static LimbPresentation limb_draw;
static uint64_t weapon_hands, weapon_draws, weapon_flashes, weapon_items;
// Placement in the level view frame (x right, y down, z ahead). The pistol's
// firing pose puts the hand at about (84, 120, 257) from the eye; that pose is
// drawn at the anchor, and the animation's motion about it is scaled by the
// motion factor, so the drawn-weapon rest pose (hand at the shoulder, gun
// raised) and recoil stay visible without filling the view.
// Developer tuning only (not saved): DNTTK_FP_WEAPON=x,y,z anchor and
// DNTTK_FP_WEAPON_MOTION=0..1.
static constexpr double weapon_reference[3]={84,120,257};
// Per-weapon display offsets (presentation only), view units as above. Slot 7
// holds its grip far right and low. Slot 8 (key 5) is the twin cannon
// launcher: framed like Duke 3D's Devastator (user reference, 2026-09-29),
// low and tilted muzzle-up so the two cannons rise from the bottom corners and
// the block joining them stays a strip at the bottom edge. The hand joint must
// stay deeper than the loop's near cull (GTE H, 256), which limits how close
// the weapon can sit; a 1.2 scale widens it instead.
// Playtest passes (user, 2026-09-29): the shotgun (slot 5, key 3) turns in a
// little so barrel and pump show, muzzle raised toward the crosshair; the
// gatling (slot 7, key 4) sits at the bottom edge, muzzle raised toward the
// crosshair; the double barrel (slot 6, key 9) gets a slight raise;
// the throwing blades (slots 1 and 2) lean forward so they read as blades;
// the pistol (slot 4) and knife (slot 1) sit low enough that the glove's cut
// wrist is below the screen edge, so no floating hand shows;
// the energy weapon (slot 10, key 7) and freezer (slot 11, key 0) are scaled
// up and tilted so more of the barrel and tip show (they cannot come closer:
// the hand joint would fall inside the loop's near cull).
static constexpr double weapon_slot_offset[32][3]={
    {},{-60,60,0},{},{},{-10,65,0},{30,10,0},{},{110,80,290},{-20,240,-100},{},{0,-20,0},{0,-20,0},
};
// Per-weapon display tilt (degrees: pitch about the level x axis, negative
// raises a forward-pointing muzzle and lowers the grip; yaw, positive turns
// the muzzle right; roll),
// applied about the grip in the level frame. Developer override:
// DNTTK_FP_WEAPON_TILT<n>=pitch,yaw,roll.
static constexpr double weapon_slot_tilt[32][3]={{},{-30,-20,40},{-40,0,0},{},{-10,0,0},{-16,-15,6},{-6,0,0},{-12,-26,16},{42,0,0},{},{-12,0,0},{-12,-10,0}};
static const double* slot_tilt(unsigned slot) {
    static double table[32][3];
    static const bool parsed=[]{
        for(unsigned s=0;s<32;++s) {
            for(int i=0;i<3;++i) table[s][i]=weapon_slot_tilt[s][i];
            char name[32];std::snprintf(name,sizeof name,"DNTTK_FP_WEAPON_TILT%u",s);
            const char* text=std::getenv(name);double v[3];
            if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
                for(int i=0;i<3;++i) table[s][i]=std::clamp(v[i],-180.0,180.0);
        }
        return true;
    }();
    (void)parsed;return table[slot];
}
// Per-weapon display scale for the weapon mesh (the original draw's own scale
// argument, a3 of 0x80033e40, 4096 = 1). The eye-view copy draws with the
// mesh's stored vertex colors, so scale does not change its shading.
// Developer override: DNTTK_FP_WEAPON_SCALE<n>=factor.
static constexpr double weapon_slot_scale[32]={0,0,0,0,0,0,0,0,1.2,0,1.4,1.4};
static double slot_scale(unsigned slot) {
    static double table[32];
    static const bool parsed=[]{
        for(unsigned s=0;s<32;++s) {
            table[s]=weapon_slot_scale[s]>0?weapon_slot_scale[s]:1;
            char name[32];std::snprintf(name,sizeof name,"DNTTK_FP_WEAPON_SCALE%u",s);
            const char* text=std::getenv(name);char* end=nullptr;
            const double v=text?std::strtod(text,&end):0;
            if(text && end && !*end && v>=0.25 && v<=4) table[s]=v;
        }
        return true;
    }();
    (void)parsed;return slot<32?table[slot]:1;
}
// Developer tuning only (not saved): DNTTK_FP_WEAPON_SLOT<n>=x,y,z replaces a
// slot's offset.
static const double* slot_offset(unsigned slot) {
    static double table[32][3];
    static const bool parsed=[]{
        for(unsigned s=0;s<32;++s) {
            for(int i=0;i<3;++i) table[s][i]=weapon_slot_offset[s][i];
            char name[32];std::snprintf(name,sizeof name,"DNTTK_FP_WEAPON_SLOT%u",s);
            const char* text=std::getenv(name);double v[3];
            if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
                for(int i=0;i<3;++i) table[s][i]=std::clamp(v[i],-2000.0,2000.0);
        }
        return true;
    }();
    (void)parsed;return table[slot];
}
static const double* weapon_anchor() {
    static double value[3]={110,150,520};
    static const bool parsed=[]{
        const char* text=std::getenv("DNTTK_FP_WEAPON");double v[3];
        if(text && std::sscanf(text,"%lf,%lf,%lf",&v[0],&v[1],&v[2])==3)
            for(int i=0;i<3;++i) value[i]=std::clamp(v[i],i==2?128.0:-1000.0,2000.0);
        return true;
    }();
    (void)parsed;return value;
}
static double weapon_motion() {
    static const double value=[]{
        const char* text=std::getenv("DNTTK_FP_WEAPON_MOTION");char* end=nullptr;
        const double v=text?std::strtod(text,&end):0;
        return text && end && !*end && v>=0 && v<=1 ? v : 0.5;
    }();
    return value;
}
static bool weapon_view_enabled() {
    static const bool value=[]{const char* t=std::getenv("DNTTK_FP_WEAPON_VIEW");return !(t && t[0]=='0' && !t[1]);}();
    return value;
}
static uint32_t hand_matrix() {
    const uint32_t desc=psx_mod_read_word(player+0x40),mats=psx_mod_read_word(player+0x3c);
    if(desc<0x80010000 || desc>0x801fff00 || mats<0x80010000 || mats>0x801fff00 ||
       psx_mod_read_byte(desc)!=duke_joints) return 0;
    const unsigned hand=psx_mod_read_byte(desc+hand_record_byte);
    return hand<duke_joints ? mats+0x20*hand : 0;
}
static void fade_restore(CPUState* cpu);
static void fade_clear(CPUState* cpu);
static bool kick_leg_joint(CPUState* cpu);
// End a verified draw scope with restoration; world/replay replacement instead
// discards ownership without restoring bytes into a different RAM image.
static void limb_scope_end(CPUState* cpu) {fade_restore(cpu);limb_draw={};}
static void limb_scope_forget() {limb_draw={};weapon_live=false;weapon_duke_draw=false;display_hand_done=false;}
static void limb_claim(CPUState* cpu,LimbOwner owner,int part,bool bridge=false) {
    limb_draw=limb_presentation(owner,part,bridge);
    if(limb_draw.light==LimbLight::CorrectProximityFade) fade_clear(cpu);
}
static void weapon_release() {weapon_live=false;limb_scope_end(nullptr);}
// Refreshed at every Duke draw, including replay workers. The weapon draw
// replaces this fallback with the native per-joint environment colour.
static uint32_t biomask_light;
static void weapon_draw_bounds(bool duke_entry,bool list_step_after_duke) {
    if(duke_entry) {
        const uint32_t b=psx_mod_read_byte(player+0x1b);
        biomask_light=b|b<<8|b<<16;
    }
    const bool mask=list_step_after_duke && weapon_duke_draw && first_person_view_live() &&
        fp_blend>=0.999 && gameplay_context() && player_identity_ready() &&
        (psx_mod_read_half(player+0x35c)&2) && (int16_t)psx_mod_read_half(player+0x32)>0;
    if(list_step_after_duke && (psx_mod_read_half(player+0x35c)&2) && !psx_mod_replay_active() && std::getenv("DNTTK_MASK_TRACE"))
        std::fprintf(stderr,"mask-draw gate=%d duke=%d fp=%f orbit=%d game=%d ident=%d face=%u\n",mask,weapon_duke_draw,fp_blend,orbit_valid,gameplay_context(),player_identity_ready(),psx_mod_read_half(player+0x854));
    if(duke_entry) weapon_duke_draw=true;
    if(list_step_after_duke) weapon_duke_draw=false;
    if(duke_entry || list_step_after_duke) {display_hand_done=false;weapon_release();near_clip_viewmodel_flush();}
    if(mask && psx_mod_read_half(player+0x35e) && psx_mod_read_half(player+0x854)==2)
        near_clip_biomask(bob_dip,biomask_light);
}
// Ready pose (default). At rest Duke carries a drawn weapon raised (pistol)
// or across his body (shotgun), which from the eye shows the weapon's side or
// points it off screen. Each weapon's firing pose (the first frame that draws
// its muzzle flash, 0x800341e4) is recorded per slot: the hand relative to
// Duke's torso (the spine part, the arms' parent) and the torso's pose relative to a
// level eye. The weapon is held in that pose in front of the eye, with a
// short kick on each flash frame; the original flash stays attached to it.
// Seeds: firing poses measured in this build (d12 weapon survey, standing,
// level view). A weapon without a seed records its pose at its first shot in
// first person and uses the pistol's torso pose until then.
// DNTTK_FP_WEAPON_POSE=animated shows the live pose throughout.
struct FirePose { bool known; double q[4]; double rel[3]; double torso_q[4]; double torso[3]; bool seeded; };
// Survey 2026-09-29 (street, standing, level view): first muzzle-flash frame.
// Slots 9 and 10 fire from the carry pose, so the ready pose changes nothing.
// Survey 2026-09-29 (street, standing, level view): first muzzle-flash frame.
// Slots 9 and 10 fire from their carry pose.
static FirePose fire_poses[32]={
    {},
    {},
    {},
    {},
    {true,{0.7082,0.6628,-0.2209,-0.1015},{84.7,-149.7,263.4},{0.9992,-0.0398,0.0000,-0.0000},{-0.1,283.0,-28.3},true}, // 4 pistol
    {true,{0.7138,0.6955,0.0065,0.0820},{123.2,-77.3,24.5},{0.9992,-0.0398,0.0000,-0.0000},{-0.1,283.0,-29.7},true}, // 5 shotgun
    {true,{0.7138,0.6955,0.0063,0.0821},{124.0,-77.1,24.7},{0.9994,-0.0344,0.0000,-0.0001},{0.6,283.0,-26.2},true}, // 6
    {true,{0.7033,0.7107,0.0117,-0.0118},{149.5,-65.8,31.7},{0.9992,-0.0398,0.0000,-0.0000},{-1.5,283.0,-28.3},true}, // 7
    {true,{0.5829,0.8123,0.0116,0.0154},{77.9,-209.6,157.0},{0.9992,-0.0398,0.0000,-0.0000},{-0.1,283.0,-28.3},true}, // 8
    {true,{0.6855,0.7262,-0.0094,0.0524},{104.3,-50.7,65.0},{0.9994,-0.0344,0.0000,-0.0001},{0.6,282.0,-26.2},true}, // 9
    {true,{0.6855,0.7262,-0.0094,0.0524},{104.3,-50.7,65.0},{0.9994,-0.0344,0.0000,-0.0001},{0.6,284.0,-26.2},true}, // 10
    {true,{0.7068,0.7073,0.0060,-0.0097},{145.3,-65.2,42.5},{0.9994,-0.0344,0.0000,-0.0001},{-0.1,283.0,-25.5},true}, // 11
};
static bool weapon_ready_pose() {
    static const bool value=[]{const char* t=std::getenv("DNTTK_FP_WEAPON_POSE");return !(t && std::strcmp(t,"animated")==0);}();
    return value;
}
static double live_q[4], live_rel[3], live_torso_q[4], live_torso[3];
static bool live_valid;
static uint64_t weapon_readies, poses_recorded;
using Mat3=double[3][3];
static void mat_mul(const Mat3 a,const Mat3 b,Mat3 out) {
    double t[3][3];
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) t[r][k]=a[r][0]*b[0][k]+a[r][1]*b[1][k]+a[r][2]*b[2][k];
    std::memcpy(out,t,sizeof t);
}
// Unit quaternion (w, x, y, z) of a rotation matrix, and back.
static void mat_quat(const Mat3 m,double* q) {
    const double tr=m[0][0]+m[1][1]+m[2][2];
    if(tr>0) {const double s=std::sqrt(tr+1)*2;q[0]=s/4;q[1]=(m[2][1]-m[1][2])/s;q[2]=(m[0][2]-m[2][0])/s;q[3]=(m[1][0]-m[0][1])/s;}
    else if(m[0][0]>m[1][1] && m[0][0]>m[2][2]) {const double s=std::sqrt(1+m[0][0]-m[1][1]-m[2][2])*2;q[0]=(m[2][1]-m[1][2])/s;q[1]=s/4;q[2]=(m[0][1]+m[1][0])/s;q[3]=(m[0][2]+m[2][0])/s;}
    else if(m[1][1]>m[2][2]) {const double s=std::sqrt(1+m[1][1]-m[0][0]-m[2][2])*2;q[0]=(m[0][2]-m[2][0])/s;q[1]=(m[0][1]+m[1][0])/s;q[2]=s/4;q[3]=(m[1][2]+m[2][1])/s;}
    else {const double s=std::sqrt(1+m[2][2]-m[0][0]-m[1][1])*2;q[0]=(m[1][0]-m[0][1])/s;q[1]=(m[0][2]+m[2][0])/s;q[2]=(m[1][2]+m[2][1])/s;q[3]=s/4;}
    const double n=std::sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
    for(int i=0;i<4;++i) q[i]/=n;
}
static void quat_mat(const double* q,Mat3 m) {
    const double w=q[0],x=q[1],y=q[2],z=q[3];
    const double r[3][3]={{1-2*(y*y+z*z),2*(x*y-w*z),2*(x*z+w*y)},
                          {2*(x*y+w*z),1-2*(x*x+z*z),2*(y*z-w*x)},
                          {2*(x*z-w*y),2*(y*z+w*x),1-2*(x*x+y*y)}};
    std::memcpy(m,r,sizeof r);
}
// Recoil kick per muzzle-flash frame: up to 7 degrees muzzle up and 30 units
// back, decaying by 0.7 a frame.
static constexpr double kick_angle=7*3.14159265358979/180, kick_back=30, kick_decay=0.7;
static double kick;
static unsigned weapon_view_slot() {
    const unsigned slot=psx_mod_read_byte(player+0x3b8)==2 ? psx_mod_read_byte(player+0x3b9) : 32u;
    // D12D: 8003df40 resolves the upgraded flamethrower, gatling and RPG
    // to separate records, but they retain their base mesh and animation.
    // Share the complete first-person framing, including the seeded pose
    // before the first shot. This is display-only; never change the held
    // record used by the original game's ammo, damage or projectile paths.
    switch(slot) {
    case 27:return 9;
    case 28:return 7;
    case 29:return 8;
    default:return slot;
    }
}
// D08K10: the animated weapons must not inherit the crouch body's shoulder
// compensation. Rebuild the hand relative to the chest from the right arm's
// own animation channel, bypassing crouch_walk_upper's joint-record edits.
// This keeps draw/throw/reload motion, without per-weapon crouch offsets or
// new pose seeds. Standing normally uses the original D12 display path;
// D36H also uses this reconstruction for the Buffalo Rifle reload below.
// Replay workers do not run the stance/animation updates. Their fork-time
// crouch statics are stale, so use the eligibility published with this frame.
static bool override_crouch_weapon, override_swim_weapon;
static bool crouch_weapon_active() {
    if(render_override)return override_crouch_weapon;
    return input_modernized() && crouch_walk_modern() &&
        (crouch_upper_weight>0 || crouch_fire_tilting ||
         (crouch_owned && crouch_pose(psx_mod_read_half(player+0x60))));
}
// D11E: swimming also compensates the shoulder against a pitched torso.
// Reconstruct only the display copy from the same animation channel; never
// carry that world-space compensation into the first-person hand a second time.
static bool swim_weapon_view_active() {
    if(render_override)return override_swim_weapon;
    return input_modernized() && first_person_swimming();
}
// Buffalo presentation follows the original reload clock, but uses a grip
// designed for the eye camera. The retail wrist roll is a third-person pose.
struct RifleReload { double lift=0,pull=0; };
static RifleReload override_rifle_reload;
static bool rifle_clip(unsigned channel,unsigned& first,unsigned& count) {
    const unsigned clip=psx_mod_read_half(0x800c2d64+8*27+2*channel);
    if(clip>=2048)return false;
    const uint32_t r=psx_mod_read_word(0x800d8328+4*clip);
    first=r&0xffff;count=r>>16;
    return first!=0xffff && count>0 && count<=64 && first+count<=0xffff;
}
static RifleReload rifle_reload_pose() {
    if(render_override)return override_rifle_reload;
    if(weapon_view_slot()!=6 || psx_mod_read_half(player+0x74)!=27)return {};
    unsigned first,count;if(!rifle_clip(1,first,count))return {};
    const unsigned frame=psx_mod_read_half(player+0x7a);
    if(frame<first || frame>=first+count)return {};
    const uint32_t frm=psx_mod_read_word(0x800d8324);
    double elapsed=0,total=0;
    for(unsigned i=0;i<count;++i) {
        const unsigned ticks=psx_mod_read_word(frm+8*(first+i))>>16;
        if(i<frame-first)elapsed+=ticks;
        if(i==frame-first)elapsed+=std::clamp(int(ticks)-int(psx_mod_read_half(player+0x76)),0,int(ticks));
        total+=ticks;
    }
    if(total<=0)return {};
    const double phase=elapsed/total;
    // Raise, cock back/forward, then lower. Zero at both ends and zero slope
    // at each authored key; no host timer or native event/clip modification.
    constexpr double times[]={0,.22,.42,.59,.78,1};
    constexpr double lifts[]={0,1,1,1,.65,0};
    constexpr double pulls[]={0,0,1,0,0,0};
    for(unsigned i=1;i<6;++i)if(phase<=times[i]) {
        double t=(phase-times[i-1])/(times[i]-times[i-1]);t=t*t*(3-2*t);
        return {lifts[i-1]+t*(lifts[i]-lifts[i-1]),pulls[i-1]+t*(pulls[i]-pulls[i-1])};
    }
    return {};
}
static bool display_arm_channel(const std::array<CrouchQuat,32>& arm,Mat3 hand,double* rel,unsigned shoulder=13) {
    const uint32_t desc=duke_model();if(!desc)return false;
    for(int r=0;r<3;++r){rel[r]=0;for(int k=0;k<3;++k)hand[r][k]=r==k?1:0;}
    for(unsigned part=shoulder;part<shoulder+3;++part) {
        const int joint=duke_part_joint(desc,part);
        if(joint<0)return false;
        const uint32_t record=desc+0x44+0x28*joint;
        if(psx_mod_read_byte(record+2)!=part-shoulder+3)return false;
        double offset[3];
        for(int i=0;i<3;++i)offset[i]=int32_t(psx_mod_read_word(record+0x14+4*i));
        for(int i=0;i<3;++i)rel[i]+=hand[i][0]*offset[0]+hand[i][1]*offset[1]+hand[i][2]*offset[2];
        const auto& a=arm[part];
        const double conventional[4]={a[3],-a[0],-a[1],-a[2]};
        Mat3 local;quat_mat(conventional,local);mat_mul(hand,local,hand);
    }
    return true;
}
static bool rifle_ready_arm(unsigned channel,Mat3 hand,double* rel) {
    unsigned first,count;if(!rifle_clip(channel,first,count))return false;
    std::array<std::array<int32_t,4>,32> raw{};uint32_t mask=0;
    const unsigned shoulder=channel==1?13:10,needed=7u<<shoulder;
    if(!crouch_frame_quats(first+count-1,raw,mask) || (mask&needed)!=needed)return false;
    std::array<CrouchQuat,32> arm{};
    for(unsigned j=shoulder;j<shoulder+3;++j)
        arm[j]=crouch_qnorm({double(raw[j][0]),double(raw[j][1]),double(raw[j][2]),double(raw[j][3])});
    return display_arm_channel(arm,hand,rel,shoulder);
}
// D36H3: the ready weapon is already camera-anchored, but the native run
// loops roll the arm segments around that fixed grip. Retain a small part of
// that motion, blending toward the weapon's own ready chain. Nothing here
// writes the native skeleton, weapon anchor, animation clock or fire events.
struct RunArms {
    double weight=0, q[6][4]{}, left[3]{};
};
static RunArms override_run_arms;
static bool arm_clip(unsigned anim,unsigned channel,unsigned& first,unsigned& count) {
    const unsigned clip=psx_mod_read_half(0x800c2d64+8*anim+2*channel);
    if(clip>=2048)return false;
    const uint32_t r=psx_mod_read_word(0x800d8328+4*clip);
    first=r&0xffff;count=r>>16;
    return first!=0xffff && count>0 && count<=64 && first+count<=0xffff;
}
static bool arm_reference(unsigned anim,unsigned key,RunArms& pose) {
    const uint32_t desc=duke_model();if(!desc)return false;
    Mat3 joints[6];double points[6][3];
    for(unsigned side=0;side<2;++side) {
        unsigned first,count;
        if(!arm_clip(anim,side?1:2,first,count) || key>=count)return false;
        std::array<std::array<int32_t,4>,32> raw{};uint32_t mask=0;
        const unsigned shoulder=side?13:10,needed=7u<<shoulder;
        if(!crouch_frame_quats(first+key,raw,mask))return false;
        // Retail ready clip 29's right chain encodes shoulder/elbow only.
        // Its neutral wrist is implicit; do not reject both Gatling families
        // and silently leave their complete native run swing in place.
        if(anim==29 && side==1 && !(mask&(1u<<15))) {
            raw[15]={0,0,0,4096};mask|=1u<<15;
        }
        if((mask&needed)!=needed)return false;
        Mat3 m={{1,0,0},{0,1,0},{0,0,1}};double pos[3]{};
        for(unsigned part=shoulder;part<shoulder+3;++part) {
            const int j=duke_part_joint(desc,part);if(j<0)return false;
            const uint32_t rec=desc+0x44+0x28*j;
            if(psx_mod_read_byte(rec+2)!=part-shoulder+3)return false;
            double offset[3];for(int k=0;k<3;++k)offset[k]=int32_t(psx_mod_read_word(rec+0x14+4*k));
            for(int r=0;r<3;++r)for(int k=0;k<3;++k)pos[r]+=m[r][k]*offset[k];
            const auto a=crouch_qnorm({double(raw[part][0]),double(raw[part][1]),double(raw[part][2]),double(raw[part][3])});
            const double q[4]={a[3],-a[0],-a[1],-a[2]};Mat3 local;quat_mat(q,local);mat_mul(m,local,m);
            std::memcpy(joints[part-10],m,sizeof m);std::memcpy(points[part-10],pos,sizeof pos);
        }
    }
    Mat3 inverse;for(int r=0;r<3;++r)for(int k=0;k<3;++k)inverse[r][k]=joints[5][k][r];
    for(unsigned n=0;n<6;++n) {Mat3 relative;mat_mul(inverse,joints[n],relative);mat_quat(relative,pose.q[n]);}
    for(int r=0;r<3;++r)for(int k=0;k<3;++k)pose.left[r]+=inverse[r][k]*(points[2][k]-points[5][k]);
    return true;
}
static RunArms running_arm_pose() {
    if(render_override)return override_run_arms;
    RunArms pose;
    const unsigned slot=weapon_view_slot(),upper=psx_mod_read_half(player+0x74);
    if(!input_modernized() || !weapon_ready_pose() ||
       !(slot==5 || slot==7 || slot==9 || slot==10 || slot==11) ||
       !(upper==18 || upper==19 || upper==20 || upper==29) ||
       crouch_weapon_active() || swim_weapon_view_active() ||
       psx_mod_read_byte(player+0x22c)>1)return pose;
    auto run_frame=[](unsigned frame) {
        for(unsigned anim:{18u,19u}) {
            unsigned first,count;
            if(arm_clip(anim,1,first,count) && frame>=first && frame<first+count)return 1.0;
        }
        return 0.0;
    };
    const unsigned prev=psx_mod_read_half(player+0x78),cur=psx_mod_read_half(player+0x7a);
    if(prev==0xffff || cur==0xffff)return pose;
    const unsigned ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*cur)>>16;
    const double t=ticks?std::clamp((double(ticks)-psx_mod_read_half(player+0x76))/ticks,0.0,1.0):1;
    const double weight=.9*((1-t)*run_frame(prev)+t*run_frame(cur));
    if(weight<=0)return pose;
    if(!arm_reference(slot==7 || slot==11?29:20,0,pose))return {};
    pose.weight=weight;
    return pose;
}
static bool animated_weapon_channel(double* q,double* pos) {
    const unsigned slot=weapon_view_slot();
    if((!crouch_weapon_active() && !swim_weapon_view_active()) ||
       !(slot==1 || slot==2 || slot==3 || slot==12 || slot==13 || slot==14 || slot==32))return false;
    std::array<CrouchQuat,32> arm{};uint32_t mask=0;
    if(!crouch_channel_pose(1,arm,mask) || (mask&0xe000u)!=0xe000u)return false;
    Mat3 hand;double rel[3];
    if(!display_arm_channel(arm,hand,rel))return false;
    mat_quat(hand,q);std::memcpy(pos,rel,sizeof rel);
    return true;
}
// Level-frame hand (orientation lh, position rel from the eye) for display:
// the torso-relative live pose, blended toward the slot's firing pose, placed
// by that pose's torso (or the pistol's for a weapon not yet seen firing).
static bool display_pose(Mat3 lh,double* rel,bool& ready) {
    ready=false;
    const unsigned slot=weapon_view_slot();
    const FirePose* p=slot<32 && fire_poses[slot].known ? &fire_poses[slot] : nullptr;
    const FirePose& torso=p?*p:fire_poses[4];
    if(!torso.known) return false;
    double q[4],pos[3],w=0;
    std::memcpy(q,live_q,sizeof q);std::memcpy(pos,live_rel,sizeof pos);
    const bool channel_pose=animated_weapon_channel(q,pos);
    // Modernized view aiming (D07A) turns Duke's arms toward the view even at
    // rest, so a live pose blend would add the view pitch twice. The ready
    // pose normally stays at the firing pose; the flash adds its kick below.
    if(p && weapon_ready_pose() && !channel_pose) {w=1;std::memcpy(q,p->q,sizeof q);std::memcpy(pos,p->rel,sizeof pos);}
    double tm[3][3],r[3][3];
    quat_mat(torso.torso_q,tm);quat_mat(q,r);
    mat_mul(tm,r,lh);
    for(int i=0;i<3;++i) rel[i]=torso.torso[i]+tm[i][0]*pos[0]+tm[i][1]*pos[1]+tm[i][2]*pos[2];
    if(slot==6) {
        const auto pose=rifle_reload_pose();
        const double a=28.0*3.141592653589793/180*pose.lift;
        const Mat3 raise={{1,0,0},{0,std::cos(a),-std::sin(a)},{0,std::sin(a),std::cos(a)}};
        mat_mul(raise,lh,lh);rel[0]-=30*pose.lift;rel[1]-=150*pose.lift;rel[2]+=40*pose.lift;
    }
    ready=w>0;
    return true;
}
// Shared draw scratch for display arm segments and hidden unused arms. Each
// transform consumes it immediately; no allocation or saved skeleton is added.
static uint32_t hidden_matrix;
static uint64_t arm_hides;
static bool weapon_held() {const unsigned s=psx_mod_read_byte(player+0x3b8);return s==1 || s==2;}
// Close-camera fade. For the root joint the draw loop makes the rest of the
// actor semi-transparent and dark when the root is nearer than 1024
// (0x80034e18: render context +0x48 = 0x2000000, +0x4c = 0x600000, flat
// colors and GTE back color = depth/8). In the eye view the root clears the
// near cull only when looking down steeply, and the weapon then drew faint.
// For the hand and its attached draws the context gets Duke's plain values
// (+0x48/+0x4c 0, colors and back color from his brightness, actor +0x1b),
// restored at the next joint. Actors marked translucent on purpose (word 0
// bits 0x40, 0x2000, 0x4000) keep theirs.
static constexpr uint32_t render_context=0x800d67a8;
struct FadeSave { bool active; uint32_t command,tpage,flat; int32_t bk[3]; };
static FadeSave fade_save;
static uint64_t fade_clears;
static void fade_clear(CPUState* cpu) {
    const uint32_t command=psx_mod_read_word(render_context+0x48),tpage=psx_mod_read_word(render_context+0x4c);
    if(command!=0x2000000 || tpage!=0x600000 || (psx_mod_read_word(player)&0x6040)) return;
    fade_save={true,command,tpage,psx_mod_read_word(render_context+0x54),{(int32_t)cpu->gte_ctrl[13],(int32_t)cpu->gte_ctrl[14],(int32_t)cpu->gte_ctrl[15]}};
    const uint32_t b=psx_mod_read_byte(player+0x1b);
    psx_mod_write_word(render_context+0x48,0);psx_mod_write_word(render_context+0x4c,0);
    psx_mod_write_word(render_context+0x54,(fade_save.flat&0xff000000u)|b|b<<8|b<<16);
    for(int i=13;i<=15;++i) cpu->gte_ctrl[i]=(b<<4)&0xff0;
    ++fade_clears;
}
static void fade_restore(CPUState* cpu) {
    if(!fade_save.active) return;
    psx_mod_write_word(render_context+0x48,fade_save.command);psx_mod_write_word(render_context+0x4c,fade_save.tpage);
    psx_mod_write_word(render_context+0x54,fade_save.flat);
    if(cpu) for(int i=0;i<3;++i) cpu->gte_ctrl[13+i]=(uint32_t)fade_save.bk[i];
    fade_save.active=false;
}
// Arms preview the same grip as the hand. Only the native right-hand draw
// advances recoil/pose counters, preserving the original once-per-draw timing.
static bool prepare_display_matrix(uint32_t real,uint32_t desc,bool advance=false) {
    if(!advance && display_hand_done)return true;
    live_valid=false;
    if(!weapon_matrix) weapon_matrix=psx_mod_alloc_guest_memory(0x20,16);
    if(!weapon_matrix) return false;
    double c[3][3],h[3][3];
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) {
        c[r][k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k))/4096.0;
        h[r][k]=(int16_t)psx_mod_read_half(real+2*(3*r+k))/4096.0;
    }
    // Level frame: the camera's right and forward axes without pitch (game Y down).
    const double rl=std::hypot(c[0][0],c[0][2]),fl=std::hypot(c[2][0],c[2][2]);
    if(rl<0.5 || fl<0.1) return false;
    const double l[3][3]={{c[0][0]/rl,0,c[0][2]/rl},{0,1,0},{c[2][0]/fl,0,c[2][2]/fl}};
    // Joint matrices are stored world-to-joint: the renderer transposes them
    // (0x80010d60) before camera+0x20 times the result, and the flash offset
    // is transpose(hand) times the muzzle vector. So a joint's world rotation
    // is the transpose of its matrix (W = H^T below).
    // Torso joint and the hand relative to it: Jw^T W = Jr H^T, Jw^T (t - Jt) = Jr (t - Jt).
    const int torso_joint=duke_part_joint(desc,part_spine);
    if(torso_joint<0) return false;
    const uint32_t mats=psx_mod_read_word(player+0x3c),torso_m=mats+0x20*torso_joint;
    double jr[3][3],jt[3],t[3],hw[3][3],jw[3][3];
    for(int r=0;r<3;++r) for(int k2=0;k2<3;++k2) jr[r][k2]=(int16_t)psx_mod_read_half(torso_m+2*(3*r+k2))/4096.0;
    for(int i=0;i<3;++i) {jt[i]=(int32_t)psx_mod_read_word(torso_m+0x14+4*i);t[i]=(int32_t)psx_mod_read_word(real+0x14+4*i);}
    for(int r=0;r<3;++r) for(int k2=0;k2<3;++k2) {hw[r][k2]=h[k2][r];jw[r][k2]=jr[k2][r];}
    double hr[3][3],lj[3][3];
    mat_mul(jr,hw,hr);mat_quat(hr,live_q);
    for(int r=0;r<3;++r) live_rel[r]=jr[r][0]*(t[0]-jt[0])+jr[r][1]*(t[1]-jt[1])+jr[r][2]*(t[2]-jt[2]);
    // Torso pose in the level frame, for recording a firing pose.
    mat_mul(l,jw,lj);mat_quat(lj,live_torso_q);
    for(int r=0;r<3;++r) {
        double d=0;for(int i=0;i<3;++i) d+=l[r][i]*(jt[i]-(int32_t)psx_mod_read_word(camera+0x14+4*i));
        live_torso[r]=d;
    }
    live_valid=true;
    double rel[3],v[3],lh[3][3];
    bool ready=false;
    if(!display_pose(lh,rel,ready)) {
        // No torso reference yet: the live pose in the level frame.
        mat_mul(l,hw,lh);
        for(int r=0;r<3;++r) {double d=0;for(int i=0;i<3;++i) d+=l[r][i]*(t[i]-(int32_t)psx_mod_read_word(camera+0x14+4*i));rel[r]=d;}
    }
    if(advance)weapon_readies+=ready;
    if(kick>0.01 && ready) {
        // Muzzle up about the grip's level-frame x axis (game Y is down), and back.
        const double a=-kick_angle*kick,cs=std::cos(a),sn=std::sin(a);
        const double rx[3][3]={{1,0,0},{0,cs,-sn},{0,sn,cs}};
        mat_mul(rx,lh,lh);rel[2]-=kick_back*kick;
    }
    if(advance)kick*=kick_decay;
    {
        // Per-weapon tilt: pitch about x (muzzle up is negative about x, game Y
        // down), yaw about y, roll about z, all in the level frame.
        const unsigned tslot=weapon_view_slot();
        const double* tilt=tslot<32 ? slot_tilt(tslot) : weapon_slot_tilt[0];
        if(tilt[0] || tilt[1] || tilt[2]) {
            const double d=3.14159265358979/180,p=-tilt[0]*d,y=tilt[1]*d,r=tilt[2]*d;
            const double rx[3][3]={{1,0,0},{0,std::cos(p),-std::sin(p)},{0,std::sin(p),std::cos(p)}};
            const double ry[3][3]={{std::cos(y),0,std::sin(y)},{0,1,0},{-std::sin(y),0,std::cos(y)}};
            const double rz[3][3]={{std::cos(r),-std::sin(r),0},{std::sin(r),std::cos(r),0},{0,0,1}};
            double m[3][3];mat_mul(ry,rx,m);mat_mul(m,rz,m);mat_mul(m,lh,lh);
        }
    }
    const double* anchor=weapon_anchor();const double k=weapon_motion();
    const unsigned slot=weapon_view_slot();
    const double* extra=slot<32 ? slot_offset(slot) : weapon_slot_offset[0];
    for(int r=0;r<3;++r) v[r]=anchor[r]+k*(rel[r]-weapon_reference[r])+extra[r];
    // The level-frame world rotation lh is shown pitched with the view: world
    // rotation C^T lh, stored transposed as lh^T C. The render's C (x scaled)
    // times its transpose gives lh; the flash offset follows the same rotation.
    double out[3][3];
    for(int r=0;r<3;++r) for(int k2=0;k2<3;++k2) out[r][k2]=lh[0][r]*c[0][k2]+lh[1][r]*c[1][k2]+lh[2][r]*c[2][k2];
    for(int r=0;r<3;++r) for(int k2=0;k2<3;++k2)
        psx_mod_write_half(weapon_matrix+2*(3*r+k2),(uint16_t)(int16_t)std::clamp(std::lround(out[r][k2]*4096),-32768L,32767L));
    psx_mod_write_half(weapon_matrix+0x12,0);
    for(int i=0;i<3;++i) {
        const double w=(int32_t)psx_mod_read_word(camera+0x14+4*i)+c[0][i]*v[0]+c[1][i]*v[1]+c[2][i]*v[2];
        psx_mod_write_word(weapon_matrix+0x14+4*i,(uint32_t)(int32_t)std::lround(w));
    }
    if(advance)display_hand_done=true;
    return true;
}
// Retail arm meshes, posed from the accepted wrist toward an offscreen
// shoulder. Each bone retains its native length; the display-only swing puts
// its child endpoint exactly at the next joint. No skeleton or event edits.
static bool display_arm_matrix(uint32_t real,uint32_t desc,unsigned part) {
    const unsigned wrist_part=part<13?12:15;
    const int wrist_joint=duke_part_joint(desc,wrist_part);
    const int elbow_joint=duke_part_joint(desc,wrist_part-1);
    const int shoulder_joint=duke_part_joint(desc,wrist_part-2);
    if(wrist_joint<0 || elbow_joint<0 || shoulder_joint<0)return false;
    const uint32_t mats=psx_mod_read_word(player+0x3c);
    const uint32_t source[3]={mats+0x20*shoulder_joint,mats+0x20*elbow_joint,mats+0x20*wrist_joint};
    Mat3 h,d,delta,c;
    double native[3][3],target[3][3];
    for(int r=0;r<3;++r)for(int k=0;k<3;++k) {
        h[r][k]=(int16_t)psx_mod_read_half(real+2*(3*r+k))/4096.0;
        d[r][k]=(int16_t)psx_mod_read_half(weapon_matrix+2*(3*r+k))/4096.0;
        c[r][k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k))/4096.0;
    }
    for(int r=0;r<3;++r)for(int k=0;k<3;++k)
        delta[r][k]=d[0][r]*h[0][k]+d[1][r]*h[1][k]+d[2][r]*h[2][k];
    for(int n=0;n<3;++n)for(int r=0;r<3;++r) {
        native[n][r]=(int32_t)psx_mod_read_word(weapon_matrix+0x14+4*r);
        for(int k=0;k<3;++k)native[n][r]+=delta[r][k]*((int32_t)psx_mod_read_word(source[n]+0x14+4*k)-(int32_t)psx_mod_read_word(real+0x14+4*k));
    }
    Mat3 posed[3];
    for(int n=0;n<3;++n) {
        Mat3 j;for(int r=0;r<3;++r)for(int k=0;k<3;++k)j[r][k]=(int16_t)psx_mod_read_half(source[n]+2*(3*r+k))/4096.0;
        for(int r=0;r<3;++r)for(int k=0;k<3;++k)posed[n][r][k]=j[r][0]*delta[k][0]+j[r][1]*delta[k][1]+j[r][2]*delta[k][2];
    }
    auto run=running_arm_pose();
    double arm_weight=run.weight;
    const unsigned slot=weapon_view_slot();
    // Keep the accepted right-arm motion. These support hands use the full
    // walking grip during running, with the same native transition ramp.
    if(slot==7 || slot==11)arm_weight=part<13?std::min(1.0,arm_weight/.9):0;
    // User-selected Flamethrower / Energy contact: the middle firing key is
    // also the ready support grip. Only the left chain changes; events and
    // recoil remain live. Energy retains its additional housing clearance.
    if(part<13 && (slot==9 || slot==10)) {
        RunArms firing;
        if(arm_reference(24,2,firing)) {run=firing;arm_weight=1;}
    }
    if(arm_weight>0) {
        const unsigned base=part<13?0:3;
        for(int n=0;n<3;++n) {
            Mat3 relative,reference,current;quat_mat(run.q[base+n],relative);
            for(int r=0;r<3;++r)for(int k=0;k<3;++k) {
                reference[r][k]=relative[0][r]*d[0][k]+relative[1][r]*d[1][k]+relative[2][r]*d[2][k];
                current[r][k]=posed[n][r][k];
            }
            double qa[4],qb[4];mat_quat(current,qa);mat_quat(reference,qb);
            const auto q=crouch_slerp({qa[0],qa[1],qa[2],qa[3]},{qb[0],qb[1],qb[2],qb[3]},arm_weight);
            quat_mat(q.data(),posed[n]);
        }
        if(part<13)for(int r=0;r<3;++r) {
            double wrist=int32_t(psx_mod_read_word(weapon_matrix+0x14+4*r));
            for(int k=0;k<3;++k)wrist+=d[k][r]*run.left[k];
            native[2][r]+=(wrist-native[2][r])*arm_weight;
        }
        // Reconstruct parent origins from the rotated native child offsets:
        // blending positions independently would shorten bones and open seams.
        for(int n=1;n>=0;--n) {
            const unsigned child=duke_part_joint(desc,wrist_part-1+n);
            for(int r=0;r<3;++r) {
                native[n][r]=native[n+1][r];
                for(int k=0;k<3;++k)native[n][r]-=posed[n][k][r]*int32_t(psx_mod_read_word(desc+0x44+0x28*child+0x14+4*k));
            }
        }
    }
    if(part<13) {
        const double scale=slot_scale(weapon_view_slot());
        for(int r=0;r<3;++r) {
            const double grip=(int32_t)psx_mod_read_word(weapon_matrix+0x14+4*r);
            const double shift=(native[2][r]-grip)*(scale-1);
            for(int n=0;n<3;++n)native[n][r]+=shift;
        }
    }
    Mat3 rifle_wrist{};bool rifle_support=false;
    // D36H1: bring the Gatling support palm onto the underside. Translate
    // the whole left chain before solving its connected endpoints; the right
    // hand, weapon anchor, wrist rotation and native bone lengths stay fixed.
    // Camera-space units (Y down), shared by the base and Laser Gatling.
    if(part<13 && weapon_view_slot()==7) {
        for(int r=0;r<3;++r) {
            const double shift=12*c[0][r]-12*c[1][r];
            for(int n=0;n<3;++n)native[n][r]+=shift;
        }
    }
    // Energy Weapon: move the connected support chain clear of the housing.
    if(part<13 && slot==10)for(int r=0;r<3;++r)for(int n=0;n<3;++n)
        native[n][r]-=20*c[0][r];
    if(part<13 && weapon_view_slot()==6) {
        Mat3 right,left,inverse,relative;double rp[3],lp[3],offset[3];
        if(rifle_ready_arm(1,right,rp) && rifle_ready_arm(2,left,lp)) {
            for(int r=0;r<3;++r)for(int k=0;k<3;++k)inverse[r][k]=right[k][r];
            mat_mul(inverse,left,relative);
            for(int r=0;r<3;++r) {
                offset[r]=0;for(int k=0;k<3;++k)offset[r]+=inverse[r][k]*(lp[k]-rp[k]);
            }
            offset[1]-=70*rifle_reload_pose().pull;
            for(int r=0;r<3;++r) {
                double wrist=int32_t(psx_mod_read_word(weapon_matrix+0x14+4*r));
                for(int k=0;k<3;++k)wrist+=d[k][r]*offset[k]*slot_scale(6);
                const double shift=wrist-native[2][r];
                for(int n=0;n<3;++n)native[n][r]+=shift;
                for(int k=0;k<3;++k) {
                    rifle_wrist[r][k]=0;
                    for(int j=0;j<3;++j)rifle_wrist[r][k]+=relative[j][r]*d[j][k];
                }
            }
            rifle_support=true;
        }
    }
    std::memcpy(target[2],native[2],sizeof target[2]);
    const double side=part<13?-1:1;
    for(int n=1;n>=0;--n) {
        double length=0;for(int r=0;r<3;++r)length+=std::pow(native[n+1][r]-native[n][r],2);
        length=std::sqrt(length);
        if(length<1 || length>1024)return false;
        // Camera-space direction from child to parent, with Y down. Elbows
        // continue below the wrist; shoulders continue below the screen.
        const double dir[3]={side*(n?0.35:0.2),n?0.85:0.9,n?-0.4:-0.38};
        const double norm=std::sqrt(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);
        for(int r=0;r<3;++r)target[n][r]=target[n+1][r]+length*(c[0][r]*dir[0]+c[1][r]*dir[1]+c[2][r]*dir[2])/norm;
    }
    const unsigned n=part-(wrist_part-2);
    Mat3 swing={{1,0,0},{0,1,0},{0,0,1}};
    if(n<2) {
        double a[3],b[3],an=0,bn=0;
        for(int r=0;r<3;++r) {a[r]=native[n+1][r]-native[n][r];b[r]=target[n+1][r]-target[n][r];an+=a[r]*a[r];bn+=b[r]*b[r];}
        for(int r=0;r<3;++r){a[r]/=std::sqrt(an);b[r]/=std::sqrt(bn);}
        const double dot=std::clamp(a[0]*b[0]+a[1]*b[1]+a[2]*b[2],-1.0,1.0);
        if(dot<-.9999) {
            double axis[3]={0,-a[2],a[1]};
            if(std::abs(a[0])>.9){axis[0]=-a[1];axis[1]=a[0];axis[2]=0;}
            const double len=std::sqrt(axis[0]*axis[0]+axis[1]*axis[1]+axis[2]*axis[2]);
            for(double& v:axis)v/=len;
            for(int r=0;r<3;++r)for(int k=0;k<3;++k)swing[r][k]=2*axis[r]*axis[k]-(r==k?1:0);
        } else {
            const double v[3]={a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
            const Mat3 cross={{0,-v[2],v[1]},{v[2],0,-v[0]},{-v[1],v[0],0}};
            Mat3 square;mat_mul(cross,cross,square);
            for(int r=0;r<3;++r)for(int k=0;k<3;++k)swing[r][k]+=cross[r][k]+square[r][k]/(1+dot);
        }
    }
    Mat3 out;
    for(int r=0;r<3;++r)for(int k=0;k<3;++k)out[r][k]=posed[n][r][0]*swing[k][0]+posed[n][r][1]*swing[k][1]+posed[n][r][2]*swing[k][2];
    if(rifle_support && n==2)std::memcpy(out,rifle_wrist,sizeof out);
    if(!hidden_matrix)hidden_matrix=psx_mod_alloc_guest_memory(0x20,16);
    if(!hidden_matrix)return false;
    for(int r=0;r<3;++r)for(int k=0;k<3;++k)
        psx_mod_write_half(hidden_matrix+2*(3*r+k),(uint16_t)(int16_t)std::clamp(std::lround(out[r][k]*4096),-32768L,32767L));
    psx_mod_write_half(hidden_matrix+0x12,0);
    for(int r=0;r<3;++r)psx_mod_write_word(hidden_matrix+0x14+4*r,(uint32_t)(int32_t)std::lround(target[n][r]));
    return true;
}
// 0x800292a0 entry from the joint loop (ra 0x80034c6c): the hand joint's transform.
static bool interaction_depth_bridge(PerspectiveAction action) {
    // Push/Use keep their native contact poses. Admit the complete arm chain
    // to polygon clipping instead of losing whole segments at the joint origin.
    // This changes loop admission only, never their world depth or lighting.
    return action==PerspectiveAction::Push || action==PerspectiveAction::Use ||
        action==PerspectiveAction::Wall || action==PerspectiveAction::Ladder ||
        action==PerspectiveAction::Pole || action==PerspectiveAction::Mantle ||
        action==PerspectiveAction::Overhead || action==PerspectiveAction::WaterExit;
}
static void first_person_weapon_joint(CPUState* cpu,uint32_t ra) {
    // Attached weapon transforms retain their display domain; native arms
    // have always relinquished admission on any intervening transform call.
    if(ra==0x80034c6c) limb_scope_end(cpu);
    else {
        if(limb_draw.world_clip()) limb_scope_end(cpu);
        limb_draw.bridge_joint_depth=false;
    }
    if(ra==0x80034c6c && weapon_duke_draw && fp_blend>=0.5 && orbit_valid &&
       fp_policy.effective && (fp_policy.action==PerspectiveAction::Mirror || fp_policy.action==PerspectiveAction::Use || fp_policy.action==PerspectiveAction::Push || fp_policy.action==PerspectiveAction::Wall || fp_policy.action==PerspectiveAction::Ladder || fp_policy.action==PerspectiveAction::Pole || fp_policy.action==PerspectiveAction::Mantle || fp_policy.action==PerspectiveAction::Overhead || fp_policy.action==PerspectiveAction::WaterExit) &&
       cpu->gpr[4]==camera && cpu->gpr[20]==player) {
        const int part=duke_joint_part(duke_model(),cpu->gpr[23]);
        if(part>=int(part_arm_first) && part<=0x0f)
            limb_claim(cpu,LimbOwner::Interaction,part,interaction_depth_bridge(fp_policy.action));
    }
    // D12A: the right leg while the first-person quick kick plays (kick.inc).
    if(ra==0x80034c6c && weapon_duke_draw && fp_blend>=0.5 && orbit_valid && kick_leg_joint(cpu)) return;
    // D36J: ordinary locomotion legs retain the native pose and world depth.
    // Admit polygons even when their joint origin crosses H. Traversal and
    // interaction bodies keep their accepted policy (D36K owns that review).
    if(ra==0x80034c6c && weapon_duke_draw && fp_blend>=0.5 && orbit_valid &&
       fp_policy.effective && (fp_policy.action==PerspectiveAction::Ground ||
       fp_policy.action==PerspectiveAction::Committed || fp_policy.action==PerspectiveAction::Fall) &&
       cpu->gpr[4]==camera && cpu->gpr[20]==player) {
        const uint32_t desc=duke_model(),mats=psx_mod_read_word(player+0x3c);
        const int part=desc?duke_joint_part(desc,cpu->gpr[23]):-1;
        if(((part>=1 && part<=8) || part==21) && mats>=0x80010000 && mats<=0x801ffd80 &&
           cpu->gpr[5]==mats+0x20*cpu->gpr[23])
            limb_claim(cpu,LimbOwner::Body,part,true);
    }
    if(ra!=0x80034c6c || !weapon_duke_draw || fp_blend<0.5 || !orbit_valid || !weapon_view_enabled() || !weapon_held()) return;
    // Draw loop registers: s4 the actor, s7 the joint index.
    const uint32_t real=hand_matrix(),desc=psx_mod_read_word(player+0x40);
    if(!real || cpu->gpr[4]!=camera || cpu->gpr[20]!=player) return;
    const unsigned joint=cpu->gpr[23];
    const int part=duke_joint_part(desc,joint);
    if(part>=(int)part_arm_first && part<=(int)part_arm_last && cpu->gpr[5]==psx_mod_read_word(player+0x3c)+0x20*joint) {
        const unsigned slot=weapon_view_slot();
        // Support grips come from this weapon's native two-handed pose, with
        // its display scale applied about the unchanged right-hand anchor.
        const bool support=slot==3 || (slot>=5 && slot<=11);
        // Developer A/B diagnostics only, never a player preference.
        static const bool arms=[] {const char* t=std::getenv("DNTTK_FP_ARMS");return !(t && !std::strcmp(t,"0"));}();
        if(arms && (part>=13 || support) && prepare_display_matrix(real,desc) &&
           display_arm_matrix(real,desc,part)) {
            limb_claim(cpu,LimbOwner::DisplayArm,part,true);
            cpu->gpr[5]=hidden_matrix;return;
        }
        if(!hidden_matrix) hidden_matrix=psx_mod_alloc_guest_memory(0x20,16);
        if(!hidden_matrix) return;
        for(int i=0;i<9;++i) psx_mod_write_half(hidden_matrix+2*i,i%4==0?4096:0);
        psx_mod_write_half(hidden_matrix+0x12,0);
        for(int i=0;i<3;++i) psx_mod_write_word(hidden_matrix+0x14+4*i,psx_mod_read_word(camera+0x14+4*i));
        // D35G1: armed edge step-ups can enter Mantle without stowing the
        // weapon. These limbs are hidden at the eye for the viewmodel, not
        // native interaction hands. Do not let the traversal near-cull bridge
        // force them back into the draw loop at this artificial transform.
        limb_claim(cpu,LimbOwner::HiddenArm,part);
        cpu->gpr[5]=hidden_matrix;++arm_hides;return;
    }
    if(cpu->gpr[5]!=real || joint!=psx_mod_read_byte(desc+hand_record_byte)) return;
    if(!prepare_display_matrix(real,desc,true))return;
    const bool bridge=limb_draw.bridge_joint_depth;
    limb_claim(cpu,LimbOwner::DisplayHand,0x0f);
    limb_draw.bridge_joint_depth=bridge; // preserve armed traversal hand admission
    cpu->gpr[5]=weapon_matrix;weapon_live=true;++weapon_hands;
}
// Attached draws for the hand joint: weapon mesh, muzzle flash, held item.
static void first_person_weapon_attached(CPUState* cpu,uint32_t address,uint32_t ra) {
    if(!weapon_live || !weapon_duke_draw || cpu->gpr[20]!=player || cpu->gpr[4]!=hand_matrix()) return;
    if(address==0x80033e40 && ra==0x8003531c) {
        // Native actor lighting has run and close-camera weapon fade is already
        // corrected. Sample before muzzle flashes/other joints change context.
        biomask_light=psx_mod_read_word(render_context+0x54)&0xffffff;
        const double scale=slot_scale(weapon_view_slot());
        if(scale!=1) cpu->gpr[7]=(uint32_t)(int32_t)std::lround((int16_t)cpu->gpr[7]*scale);
        ++weapon_draws;
    }
    else if(address==0x800341e4 && ra==0x80035348) {
        // Record this weapon's firing pose (live animation, not the displayed one).
        const unsigned slot=weapon_view_slot();
        if(slot<32 && live_valid && (!fire_poses[slot].seeded || std::getenv("DNTTK_FP_WEAPON_TRACE"))) {
            FirePose& p=fire_poses[slot];
            if(!p.seeded) {
                std::memcpy(p.q,live_q,sizeof p.q);std::memcpy(p.rel,live_rel,sizeof p.rel);
                std::memcpy(p.torso_q,live_torso_q,sizeof p.torso_q);std::memcpy(p.torso,live_torso,sizeof p.torso);p.known=true;
            }
            ++poses_recorded;
            // DNTTK_FP_WEAPON_TRACE=1 (developer): log it for the seed survey.
            static const bool trace=std::getenv("DNTTK_FP_WEAPON_TRACE")!=nullptr;
            if(trace) std::fprintf(stderr,"ttk-fp-fire-pose slot=%u {true,{%.4f,%.4f,%.4f,%.4f},{%.1f,%.1f,%.1f},{%.4f,%.4f,%.4f,%.4f},{%.1f,%.1f,%.1f},true} anim=%u\n",
                slot,live_q[0],live_q[1],live_q[2],live_q[3],live_rel[0],live_rel[1],live_rel[2],
                live_torso_q[0],live_torso_q[1],live_torso_q[2],live_torso_q[3],live_torso[0],live_torso[1],live_torso[2],psx_mod_read_half(player+0x60));
        }
        ++weapon_flashes;kick=1;
    }
    else if(address==0x80033f5c && ra==0x80035250) ++weapon_items;
    else return;
    cpu->gpr[4]=weapon_matrix;
}

// D35E: the native joint loop skips an entire arm when its origin is
// nearer than GTE H, before the polygon near clipper can see it. The ownership
// contract admits native traversal/interaction arms, D36H display arms and
// D36J body/kick geometry. Eligibility stays with each semantic owner.
// s0 is only the loop's saved joint depth; real transforms and projection remain
// unchanged. Native limbs keep world occlusion; display limbs use viewmodel depth.
static void first_person_ladder_near(CPUState* cpu,uint32_t ra) {
    if(ra!=0x80034c8c || !weapon_duke_draw ||
       !limb_draw.bridge_joint_depth || !fp_policy.effective ||
       !orbit_valid || fp_blend<0.5 || cpu->gpr[20]!=player)return;
    // The actor loop saves a0 (camera) at sp+0x98. s3 is its render context,
    // not the camera pointer (0x8003490c / 0x80034abc).
    const uint32_t sp=cpu->gpr[29];
    if(sp<0x80010000 || sp>0x801fff60 || (sp&3) || psx_mod_read_word(sp+0x98)!=camera)return;
    const int h=cpu->gte_ctrl[26]&0xffff;
    if((int32_t)cpu->gpr[16]<h)cpu->gpr[16]=h;
}
