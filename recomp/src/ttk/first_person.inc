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
// Unsupported states (swimming, jetpack flight, grabbing a pushable object,
// anything without the orbit lease) blend back to the third-person orbit.
static constexpr uint32_t min_boom_word=0x800c3c48, min_boom_original=768;
static constexpr unsigned head_joint=9, duke_joints=19;
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
static bool eye_offset_valid, fp_supported;
static bool min_boom_written;
static uint32_t head_flag;
static uint64_t fp_updates, fp_fallbacks, head_hides, projections;
static int last_projection;
static const char* fp_reason="off";

static uint32_t head_record() {
    const uint32_t desc=psx_mod_read_word(player+0x40);
    if(desc<0x80010000 || desc>0x801fff00 || psx_mod_read_byte(desc)!=duke_joints) return 0;
    const uint32_t record=desc+0x44+0x28*head_joint;
    // Record byte 2 is the hierarchy level; the neck is a level-3 child of the spine.
    return psx_mod_read_byte(record+2)==3 ? record : 0;
}
static bool neck_origin(double* xyz) {
    const uint32_t mats=psx_mod_read_word(player+0x3c);
    if(!head_record() || mats<0x80010000 || mats>0x801fff00) return false;
    for(int i=0;i<3;++i) xyz[i]=(int32_t)psx_mod_read_word(mats+0x20*head_joint+0x14+4*i);
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
    if(!head_flag) return;
    psx_mod_write_byte(head_flag,psx_mod_read_byte(head_flag)&~1u);
    head_flag=0;
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
static void first_person_release(const char* reason) {
    if(fp_blend>0) ++fp_fallbacks;
    fp_blend=0;eye_offset_valid=false;fp_reason=reason;
    min_boom_restore();head_restore();
}
// Once per verified normal-camera update, after orbit_begin.
static void first_person_update(const InputFrame& f,unsigned ticks) {
    // Waist-deep water keeps the land gait and is supported (the swim lease is
    // also live there); swim modes 4/5 and deeper water are not.
    const bool swimming=in_water_early() && !shallow_land_water_early();
    // D08T: grabbing/pushing a large object puts the eye inside it; show the
    // push from the orbit and return to the eye after letting go.
    // D08V: mantles, hangs and unowned falls also show from the orbit.
    const bool supported=!swimming && !jetpack_camera_only() && !push_state_early() && !traversal_camera_early();
    fp_supported=supported;
    const bool wanted=f.first_person && supported;
    fp_reason=!f.first_person?"off":!supported?"unsupported":"active";
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
// moving the head, follow quickly). View bob is then added on purpose: a dip
// and a slight sway driven by the distance Duke walks, scaled by speed.
// DNTTK_VIEW_BOB: off, subtle, on (default), strong, or a scale (0..3).
static double view_bob_scale() {
    static const double v=[]{
        const char* t=std::getenv("DNTTK_VIEW_BOB");
        if(!t || !*t || !std::strcmp(t,"on")) return 1.0;
        if(!std::strcmp(t,"off") || !std::strcmp(t,"0")) return 0.0;
        if(!std::strcmp(t,"subtle")) return 0.5;
        if(!std::strcmp(t,"strong")) return 2.0;
        char* end=nullptr; const double d=std::strtod(t,&end);
        return end && !*end && d>=0 && d<=3 ? d : 1.0;}();
    return v;
}
static constexpr double bob_height=8, bob_sway=3, bob_cycle=640, bob_walk_speed=24;
static double eye_rel[3], bob_phase, bob_amount, bob_last[3];
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
    // Eye relative to its references: x/z to the root, y to the pivot.
    // Game Y grows downward.
    const double target[3]={neck[0]-root[0],neck[1]-eye_height-pivot_y,neck[2]-root[2]};
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
        eye_rel[i]=eye_rel_valid && eye_offset_valid ? eye_rel[i]+step : target[i];
        eye_offset[i]=eye_rel[i];
    }
    const bool was_valid=eye_offset_valid;
    eye_rel_valid=eye_offset_valid=true;
    // Deliberate view bob from horizontal travel.
    double bob_y=0,bob_x=0,bob_z=0;
    const double scale=view_bob_scale();
    if(bob_last_valid && was_valid) {
        const double dist=std::hypot(root[0]-bob_last[0],root[2]-bob_last[2]);
        const double speed=dist<400 ? dist : 0;   // a teleport is not a step
        bob_amount+=(std::min(speed/bob_walk_speed,1.6)-bob_amount)*0.25;
        bob_phase=std::fmod(bob_phase+speed*2*3.14159265358979/bob_cycle,2*3.14159265358979);
        if(bob_amount<0.02) bob_phase*=0.8;   // settle at rest
    }
    for(int i=0;i<3;++i) bob_last[i]=root[i];
    bob_last_valid=true;
    if(scale>0) {
        const double a=bob_amount*scale;
        bob_y=-bob_height*a*(1-std::cos(2*bob_phase))/2;   // up at mid-step
        const double sway=bob_sway*a*std::sin(bob_phase);
        const double rx=forward[2],rz=-forward[0],rn=std::hypot(rx,rz);
        if(rn>1e-6) {bob_x=sway*rx/rn;bob_z=sway*rz/rn;}
    }
    ++bob_updates;
    const double half=std::max(0,(int)(int16_t)psx_mod_read_half(camera+0x42))/2.0;
    const double eye[3]={root[0]+eye_rel[0]+bob_x,pivot_y+eye_rel[1]+bob_y,root[2]+eye_rel[2]+bob_z};
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
    const uint32_t record=head_record();
    if(!record) return;
    if(psx_mod_read_byte(record)&1) {
        psx_mod_write_byte(record,psx_mod_read_byte(record)&~1u);
        ++head_reclaims;
    }
    if(!eye || fp_blend<0.5 || !orbit_valid) return;
    psx_mod_write_byte(record,psx_mod_read_byte(record)|1);
    head_flag=record;++head_hides;
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
// From the hand joint's transform to the next joint's: the hand mesh and its
// attached draws, which the near-clip object path draws on top (D12).
static bool weapon_viewmodel;
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
static bool kick_leg_joint(CPUState* cpu);
static void weapon_release() {weapon_live=false;weapon_viewmodel=false;fade_restore(nullptr);}
static void weapon_draw_bounds(bool duke_entry,bool list_step_after_duke) {
    if(duke_entry) weapon_duke_draw=true;
    if(list_step_after_duke) weapon_duke_draw=false;
    if(duke_entry || list_step_after_duke) {weapon_release();near_clip_viewmodel_flush();}
}
// Ready pose (default). At rest Duke carries a drawn weapon raised (pistol)
// or across his body (shotgun), which from the eye shows the weapon's side or
// points it off screen. Each weapon's firing pose (the first frame that draws
// its muzzle flash, 0x800341e4) is recorded per slot: the hand relative to
// Duke's torso (joint 1, the arms' parent) and the torso's pose relative to a
// level eye. The weapon is held in that pose in front of the eye, with a
// short kick on each flash frame; the original flash stays attached to it.
// Seeds: firing poses measured in this build (d12 weapon survey, standing,
// level view). A weapon without a seed records its pose at its first shot in
// first person and uses the pistol's torso pose until then.
// DNTTK_FP_WEAPON_POSE=animated shows the live pose throughout.
struct FirePose { bool known; double q[4]; double rel[3]; double torso_q[4]; double torso[3]; bool seeded; };
static constexpr unsigned torso_joint=1;
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
static unsigned held_weapon_slot() {
    return psx_mod_read_byte(player+0x3b8)==2 ? psx_mod_read_byte(player+0x3b9) : 32u;
}
// Level-frame hand (orientation lh, position rel from the eye) for display:
// the torso-relative live pose, blended toward the slot's firing pose, placed
// by that pose's torso (or the pistol's for a weapon not yet seen firing).
static bool display_pose(Mat3 lh,double* rel,bool& ready) {
    ready=false;
    const unsigned slot=held_weapon_slot();
    const FirePose* p=slot<32 && fire_poses[slot].known ? &fire_poses[slot] : nullptr;
    const FirePose& torso=p?*p:fire_poses[4];
    if(!torso.known) return false;
    double q[4],pos[3],w=0;
    std::memcpy(q,live_q,sizeof q);std::memcpy(pos,live_rel,sizeof pos);
    // Modernized view aiming (D07A) turns Duke's arms toward the view even at
    // rest, so a live pose blend would add the view pitch twice. The ready
    // pose is the firing pose throughout; the flash adds its kick below.
    if(p && weapon_ready_pose()) {w=1;std::memcpy(q,p->q,sizeof q);std::memcpy(pos,p->rel,sizeof pos);}
    double tm[3][3],r[3][3];
    quat_mat(torso.torso_q,tm);quat_mat(q,r);
    mat_mul(tm,r,lh);
    for(int i=0;i<3;++i) rel[i]=torso.torso[i]+tm[i][0]*pos[0]+tm[i][1]*pos[1]+tm[i][2]*pos[2];
    ready=w>0;
    return true;
}
// Arm joints (upper arms, forearms, left hand: records at hierarchy levels 3
// to 5 under the spine, joints 2-6 of Duke's 19) are hidden while the weapon
// is drawn in front of the eye: the right hand has moved away from them, and
// two-handed carries hold them across the view. Their transform call gets a
// matrix at the eye, so the loop's own near cull (depth < H) skips the joint.
static constexpr unsigned arm_first=2, arm_last=6;
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
// 0x800292a0 entry from the joint loop (ra 0x80034c6c): the hand joint's transform.
static void first_person_weapon_joint(CPUState* cpu,uint32_t ra) {
    if(ra==0x80034c6c) {weapon_viewmodel=false;fade_restore(cpu);}
    // D12A: the right leg while the first-person quick kick plays (kick.inc).
    if(ra==0x80034c6c && weapon_duke_draw && fp_blend>=0.5 && orbit_valid && kick_leg_joint(cpu)) return;
    if(ra!=0x80034c6c || !weapon_duke_draw || fp_blend<0.5 || !orbit_valid || !weapon_view_enabled() || !weapon_held()) return;
    // Draw loop registers: s4 the actor, s7 the joint index.
    const uint32_t real=hand_matrix(),desc=psx_mod_read_word(player+0x40);
    if(!real || cpu->gpr[4]!=camera || cpu->gpr[20]!=player) return;
    const unsigned joint=cpu->gpr[23];
    if(joint>=arm_first && joint<=arm_last && cpu->gpr[5]==psx_mod_read_word(player+0x3c)+0x20*joint) {
        if(!hidden_matrix) hidden_matrix=psx_mod_alloc_guest_memory(0x20,16);
        if(!hidden_matrix) return;
        for(int i=0;i<9;++i) psx_mod_write_half(hidden_matrix+2*i,i%4==0?4096:0);
        psx_mod_write_half(hidden_matrix+0x12,0);
        for(int i=0;i<3;++i) psx_mod_write_word(hidden_matrix+0x14+4*i,psx_mod_read_word(camera+0x14+4*i));
        cpu->gpr[5]=hidden_matrix;++arm_hides;return;
    }
    live_valid=false;
    if(cpu->gpr[5]!=real || joint!=psx_mod_read_byte(desc+hand_record_byte)) return;
    if(!weapon_matrix) weapon_matrix=psx_mod_alloc_guest_memory(0x20,16);
    if(!weapon_matrix) return;
    double c[3][3],h[3][3];
    for(int r=0;r<3;++r) for(int k=0;k<3;++k) {
        c[r][k]=(int16_t)psx_mod_read_half(camera+2*(3*r+k))/4096.0;
        h[r][k]=(int16_t)psx_mod_read_half(real+2*(3*r+k))/4096.0;
    }
    // Level frame: the camera's right and forward axes without pitch (game Y down).
    const double rl=std::hypot(c[0][0],c[0][2]),fl=std::hypot(c[2][0],c[2][2]);
    if(rl<0.5 || fl<0.1) return;
    const double l[3][3]={{c[0][0]/rl,0,c[0][2]/rl},{0,1,0},{c[2][0]/fl,0,c[2][2]/fl}};
    // Joint matrices are stored world-to-joint: the renderer transposes them
    // (0x80010d60) before camera+0x20 times the result, and the flash offset
    // is transpose(hand) times the muzzle vector. So a joint's world rotation
    // is the transpose of its matrix (W = H^T below).
    // Torso joint and the hand relative to it: Jw^T W = Jr H^T, Jw^T (t - Jt) = Jr (t - Jt).
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
    weapon_readies+=ready;
    if(kick>0.01 && ready) {
        // Muzzle up about the grip's level-frame x axis (game Y is down), and back.
        const double a=-kick_angle*kick,cs=std::cos(a),sn=std::sin(a);
        const double rx[3][3]={{1,0,0},{0,cs,-sn},{0,sn,cs}};
        mat_mul(rx,lh,lh);rel[2]-=kick_back*kick;
    }
    kick*=kick_decay;
    {
        // Per-weapon tilt: pitch about x (muzzle up is negative about x, game Y
        // down), yaw about y, roll about z, all in the level frame.
        const unsigned tslot=held_weapon_slot();
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
    const unsigned slot=held_weapon_slot();
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
    fade_clear(cpu);
    cpu->gpr[5]=weapon_matrix;weapon_live=weapon_viewmodel=true;++weapon_hands;
}
// Attached draws for the hand joint: weapon mesh, muzzle flash, held item.
static void first_person_weapon_attached(CPUState* cpu,uint32_t address,uint32_t ra) {
    if(!weapon_live || !weapon_duke_draw || cpu->gpr[20]!=player || cpu->gpr[4]!=hand_matrix()) return;
    if(address==0x80033e40 && ra==0x8003531c) {
        const double scale=slot_scale(held_weapon_slot());
        if(scale!=1) cpu->gpr[7]=(uint32_t)(int32_t)std::lround((int16_t)cpu->gpr[7]*scale);
        ++weapon_draws;
    }
    else if(address==0x800341e4 && ra==0x80035348) {
        // Record this weapon's firing pose (live animation, not the displayed one).
        const unsigned slot=held_weapon_slot();
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
