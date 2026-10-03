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
//     (face_view), so the original body-relative thrust is camera-relative.
//   • WASD → original Up/Down + the layout's strafe pads (input_pad).
//   • Space → Square (original lift; the original clears the hover lock).
//   • No vertical input → host engages the original hover lock at the current
//     height (Duke3D feel: hover, do not sink). The original drains fuel while
//     hovering too (s2 = 1 in the hover branch).
//   • Ctrl → descend: release the hover lock and let the original gravity
//     sink Duke (its idle behaviour); the original root apply lands him.
//   • J while flying → original item toggle (select_weapon), the handler then
//     cuts out to the 108 fall; the camera lease survives that fall.
// D08R Classic scheme (DNTTK_JETPACK=classic, Modernized only): the same
// lease, mouse facing (face_view), WASD bridge, J shortcut and cut-out fall
// grace as Modern, but none of the host vertical control below: no hover on
// release, no Ctrl descent, no level trim. The original lift (Space), gravity,
// fuel and hover toggle (physical L1 = walk, Shift by default) apply.
// Never: world-position writes, fuel writes, anim/mode writes, or any change
// in Vanilla. Host writes are limited to the hover lock bit, the hover base
// and phase, the vertical velocities +0x1e8/+0x1f8 and the body heading.

// Ballistic +0x1f8 while Ctrl is held: ~1/64 of it moves Y per update (base1
// probe: 7000 ↔ ~105 units). D08Q1: 5500 matches the underwater Ctrl dive
// (~29 units/frame, D08O deep lab); D08Q's 2800 gave ~16 units/frame.
static constexpr int32_t k_jet_descend_velocity = 5500;
// The handler adds a small lift (+0x1e8 -= 29 with one direction held, mod3
// probe) to any directional thrust, so original flight drifts upward ~10 units
// per update. Pre-load the opposite so WASD flight stays level.
static constexpr int32_t k_jet_level_trim = 18;
static unsigned jet_hover_engages, jet_descents, jet_updates, jet_classic_updates;
static bool jet_was_flying;
static uint64_t jet_epoch;

static bool jet_trace() {
    static const bool enabled=std::getenv("DNTTK_JET_TRACE")!=nullptr;
    return enabled;
}
static bool jet_hover_locked() { return (psx_mod_read_word(player+0x224)&0x08000000u)!=0; }
static void jet_engage_hover() {
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|0x08000000u);
    psx_mod_write_word(player+0x860,psx_mod_read_word(player+8));
    psx_mod_write_word(player+0x864,0);
    ++jet_hover_engages;
}
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
// Player-update entry (8005a210 hook), before the pre-state step and the
// mode-10 handler read heading, pads and the hover flag for this update.
static void jetpack_update() {
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
    if(!flying)return;
    if(!jetpack_input_ready())return;
    ++jet_updates;
    if(!f.held[original_aim])face_view();
    if(jetpack_classic()) {
        // D08R Classic: modern controls (mouse facing, WASD bridge), original
        // burst physics: Space lift, gravity, no host hover/descent/trim.
        ++jet_classic_updates;
        return;
    }
    const bool ascend=f.held[jump] && !f.held[crouch];
    const bool descend=f.held[crouch] && !f.held[jump];
    const bool moving=f.move_x!=0.f || f.move_y!=0.f;
    if(ascend) {
        // Original Square branch clears the hover lock and lifts; nothing to do.
    } else if(descend) {
        // Descend: release the hover lock (its pin would hold Y) and set a
        // steady downward ballistic velocity. With a direction held the
        // handler sets bit 31 (no gravity) so this is the whole descent; idle,
        // gravity adds ~350/update and the next write resets it, so the rate
        // stays near constant. The original root apply lands Duke (probe 3).
        if(jet_hover_locked())psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x08000000u);
        psx_mod_write_word(player+0x1f8,k_jet_descend_velocity);
        psx_mod_write_word(player+0x1e8,0);
        ++jet_descents;
    } else if(moving) {
        // Level flight: the hover pin would also throttle the thrust (under the
        // lock the handler moves Duke by the raw thrust vector, ~10 units per
        // update, instead of integrating it into the ballistic velocity). So
        // release the lock and zero the vertical velocities; the handler's bit
        // 31 (input held) keeps gravity off, so altitude holds.
        if(jet_hover_locked())psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x08000000u);
        psx_mod_write_word(player+0x1f8,0);
        psx_mod_write_word(player+0x1e8,k_jet_level_trim);
    } else if(!jet_hover_locked()) {
        jet_engage_hover();
    }
    if(jet_trace())std::fprintf(stderr,"ttk-jet anim=%u st=%u/%u flags=%08x y=%d foot=%d floor=%d base=%d v=%d/%d/%d bv=%d/%d/%d fuel=%d move=%.2f/%.2f asc=%d desc=%d hover=%d\n",
        animation,mode,psx_mod_read_byte(player+0x22d),psx_mod_read_word(player+0x224),
        (int32_t)psx_mod_read_word(player+8),(int32_t)psx_mod_read_word(player+0x10),(int32_t)psx_mod_read_word(player+0x1c8),
        (int32_t)psx_mod_read_word(player+0x860),(int32_t)psx_mod_read_word(player+0x1e4),(int32_t)psx_mod_read_word(player+0x1e8),
        (int32_t)psx_mod_read_word(player+0x1ec),(int32_t)psx_mod_read_word(player+0x1f4),(int32_t)psx_mod_read_word(player+0x1f8),
        (int32_t)psx_mod_read_word(player+0x1fc),(int16_t)psx_mod_read_half(player+0x35a),f.move_x,f.move_y,ascend,descend,jet_hover_locked());
}
