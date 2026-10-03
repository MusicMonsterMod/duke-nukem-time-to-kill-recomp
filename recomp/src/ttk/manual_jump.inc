// D08Z manual jump style (DNTTK_JUMP=manual, Modernized only; the launcher
// passes assisted for Vanilla). documentation/78-d08z-manual-jump.md.
// Assisted (default) keeps the original feel: a run jump pressed within the
// 1024-unit look-ahead of a gap is queued and leaves from the lip (D08Y), the
// arc is fixed after takeoff, and a standing/walking jump crouches for about a
// quarter of a second first. Manual hands the timing to the player:
//  * Takeoff on the press everywhere (the 0x800780b4 look-ahead is overridden
//    for gaps too, as D08I does for small drops).
//  * Edge grace: a jump pressed shortly after running off any ledge still
//    launches the original running jump (terrain.inc edge grace, large drops).
//  * Air steering: during an owned flight, WASD (camera-relative) bends the
//    horizontal velocity toward the requested direction, never above the
//    takeoff speed. No input keeps the momentum. Gravity, the vertical arc,
//    swept collision, impacts and landings stay original.
//  * Quicker standing takeoff: the original preparation 96 plays faster.
static bool manual_jump() {
    const char* value=std::getenv("DNTTK_JUMP");
    return value && !std::strcmp(value,"manual");
}
// Grace after the last grounded camera update, in input sequences (60 per
// second): about 0.15 s after the feet leave the edge.
static constexpr uint64_t manual_edge_grace=14;
// Steering per nominal 30 Hz update, as a fraction of the flight's speed cap:
// a full reversal takes about half a second. DNTTK_AIR_CONTROL scales it
// (0..3) for playtest tuning.
static constexpr double air_accel_fraction=0.14;
// The original advances the flight by v*dt/1024 per update; dt is 11..12 at
// 30 Hz (+0xba).
static constexpr double air_nominal_dt=11.0;
// Floor of the cap, so a short walking fall (2048) still steers usefully.
static constexpr double air_cap_floor=2048.0;
// Preparation 96 runs this many times faster (14..18 fields -> 5..6).
static constexpr int quick_takeoff_scale=3;
static double air_cap;
static unsigned air_steers, coyote_jumps, quick_takeoffs;
static bool quick_takeoff_boost;
static double air_control_scale() {
    const char* text=std::getenv("DNTTK_AIR_CONTROL");
    char* end=nullptr;const double value=text?std::strtod(text,&end):1.0;
    if(!text || !end || *end || !std::isfinite(value) || value<0 || value>3)return 1.0;
    return value;
}
// Seed the cap from the flight's own horizontal speed (call once the takeoff
// velocity is final).
static void manual_air_seed() {
    const double vx=(int32_t)psx_mod_read_word(player+0x1f4),vz=(int32_t)psx_mod_read_word(player+0x1fc);
    air_cap=std::max(std::hypot(vx,vz),air_cap_floor);
}
// From the owned ballistic update (0x8003ebf4 from 0x80055934), before the
// original gravity, swept collision and integration of this update.
static void manual_air_control(const InputFrame& f) {
    if(!manual_jump() || !flight_valid || flight_epoch!=f.epoch || f.held[original_aim] || air_cap<=0)return;
    if(!f.move_x && !f.move_y)return;
    const Vec2 wish=movement(view_yaw(),f.move_x,f.move_y);
    const unsigned dt=psx_mod_read_half(player+0xba);
    const double scale=dt>0 && dt<=64?dt/air_nominal_dt:1.0;
    Vec2 v{(double)(int32_t)psx_mod_read_word(player+0x1f4),(double)(int32_t)psx_mod_read_word(player+0x1fc)};
    v=air_steer(v,wish,air_cap,air_cap*air_accel_fraction*air_control_scale()*scale);
    psx_mod_write_word(player+0x1f4,(int32_t)std::lround(v.x));
    psx_mod_write_word(player+0x1fc,(int32_t)std::lround(v.z));
    ++air_steers;
}
// Lower-body runner 0x80059db0 from the player update (ra 0x8005a3d4, track
// player+0x60, delta at the update frame's sp+0x10, a3 = 0): scale the
// preparation's delta once per update. The original events still run in order.
static void manual_quick_takeoff(CPUState* cpu,uint32_t ra) {
    if(quick_takeoff_boost || !manual_jump() || ra!=0x8005a3d4 || cpu->gpr[4]!=player+0x60 ||
       cpu->gpr[5]!=player || cpu->gpr[7]!=0 || cpu->gpr[6]!=weapon_update_sp-0x30 ||
       psx_mod_read_half(player+0x60)!=96 || psx_mod_read_byte(player+0x22c)!=0 ||
       !input_snapshot(Context::Gameplay).active || !independent_camera() || !player_identity_ready())return;
    const int delta=(int16_t)psx_mod_read_half(cpu->gpr[6]);
    if(delta<=0 || delta>4096)return;
    psx_mod_write_half(cpu->gpr[6],delta*quick_takeoff_scale);
    quick_takeoff_boost=true;++quick_takeoffs;
}
