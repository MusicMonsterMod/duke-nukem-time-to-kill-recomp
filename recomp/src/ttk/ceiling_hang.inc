// D08J4: camera-relative travel along a climbable ceiling (monkey bars).
//
// The original object hang is mode 7 (0x80055208 enters it for any object
// whose type flags hold 0x80; 149 hangs, 150/151 are the hand-over-hand
// cycle). Its controls are tank controls: Up/Down carry Duke along his own
// facing and Left/Right only turn him on the spot. The original keeps him
// under the ceiling objects (+0x17c moves on to the next grate) and stops him
// at an end without letting go; only Square lets go.
//
// In Modernized, on such a hang under any climbable object that is not a crate
// (crates, 0xc0, still let go at once: D08X), WASD picks a direction relative
// to the camera. Duke turns toward it at a fixed rate and the original Up
// carries him once he faces it closely enough; the original decides where he
// can go. Ctrl lets go (original Square, as Space already does). Vanilla and
// every other hang are unchanged.
static constexpr int ceiling_turn_rate=320;      // game yaw units per player update (4096 = 360 degrees)
static constexpr int ceiling_advance_window=640; // advance once within ~56 degrees of the wanted heading
static unsigned ceiling_updates,ceiling_turns,ceiling_advances,ceiling_drops;
static bool ceiling_advance,ceiling_drop;

static bool ceiling_hang_state() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(psx_mod_read_byte(player+0x22c)!=7 || animation<149 || animation>151)return false;
    const uint32_t flags=object_type_flags(psx_mod_read_word(player+0x17c));
    return (flags&object_climbable)==0x80u;
}
bool ceiling_hang_ready() {return traversal_input_ready() && ceiling_hang_state();}

// Player update (0x8005a210), before the original mode-7 handler reads its
// buttons and moves Duke along his yaw.
static void ceiling_hang_update() {
    ceiling_advance=ceiling_drop=false;
    if(!ceiling_hang_ready())return;
    ++ceiling_updates;
    const auto& f=input_snapshot(Context::Gameplay);
    if(f.held[crouch]) {ceiling_drop=true;++ceiling_drops;return;}
    if(!f.move_x && !f.move_y)return;
    const auto dir=movement(view_yaw(),f.move_x,f.move_y);
    const int wanted=static_cast<int>(std::lround(std::atan2(dir.x,dir.z)*4096/tau)) & 4095;
    const int heading=psx_mod_read_half(player+0x1c)&4095;
    const int error=((wanted-heading+2048)&4095)-2048;
    if(error) {
        const int step=std::clamp(error,-ceiling_turn_rate,ceiling_turn_rate);
        const int turned=(heading+step)&4095;
        psx_mod_write_half(player+0x1c,turned);
        psx_mod_write_half(player+0x24,turned);
        ++ceiling_turns;
    }
    if(std::abs(error)<=ceiling_advance_window) {ceiling_advance=true;++ceiling_advances;}
}
// The host's picks for the original pad: Up once Duke faces the wanted way,
// Square for Ctrl. pc_input sends no Left/Right (they would turn him) or Down.
bool ceiling_hang_advance() {return ceiling_advance && ceiling_hang_ready();}
bool ceiling_hang_drop() {return ceiling_drop && ceiling_hang_ready();}
