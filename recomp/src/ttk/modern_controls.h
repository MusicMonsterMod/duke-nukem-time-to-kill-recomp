#pragma once
#include <cstddef>
#include <cstdint>
namespace ttk {
bool movement_ready();
bool edge_jump_queued();
// DNTTK_FRAME_TRACE: other plugins add their hook time to the frame trace.
bool frame_trace_on();
void frame_trace_account(uint32_t address,long us);
bool view_aim_input_ready();
bool locomotion_input_ready();
bool airborne_input_ready();
bool short_fall_input_ready();
bool directional_takeoff_ready();
bool interaction_ready();
bool fire_draw_ready();
bool hold_button_sampled();
bool interaction_restore_ready();
bool interaction_alive();
bool interaction_holster_ready();
bool weapon_drawn();
bool weapon_holstered();
bool traversal_input_ready();
/* D08V: camera-only lease through an original mantle/hang/pull-up or unowned fall. */
bool traversal_camera_ready();
bool committed_camera_ready();
/* Modern free-swim: WASD bridges to D-pad; vertical is host-owned. */
bool swim_input_ready();
/* Waist-deep / mid water: land run, mouse look and Shift, not the swim path. */
bool wade_full_speed_ready();
/* Mid-depth wade (0x200..0x281, original mode 1 / anims 80/81): the host
   rewrites the wade handler's root toward camera-relative WASD at land pace. */
bool mid_wade_input_ready();
/* Suppress Square (modern Space) while wading or free-swimming so host owns
   land jumps / soft ascend. Square is original swim thrust (anim 176), not rise. */
bool swim_host_owns_jump();
/* Active-layout strafe pad masks (8005201c strafe words, default L2/R2). */
void swim_strafe_pads(uint16_t& left,uint16_t& right);
/* Original underwater state (player+0x22c == 5): WASD / Space / Ctrl inject
   Square thrust along the host-steered body yaw/pitch; no D-pad (that is pitch). */
bool swim_thrust_input_ready();
/* D08O1: the original swim states (surface 4, underwater 5) under the swim
   lease; weapon view aiming, shots and the crosshair apply there. */
bool swim_weapon_ready();
/* D08Q jetpack flight (original mode 10, anims 163-170): camera lease live,
   body faces the view, WASD -> original Up/Down + layout strafe pads. */
bool jetpack_input_ready();
/* D08R Classic jetpack scheme (DNTTK_JETPACK=classic): jetpack_input_ready()
   controls with the original burst physics (no host hover/descent). */
bool jetpack_classic_input_ready();
bool player_identity_ready();
/* D08T/D08T1 pushable objects (push.inc). Grab 119..121 on an object flagged
   0x08000000 is live; Duke touches one on normal ground; settled holstered idle 63. */
bool push_grab_ready();
/* D08X: let go of a stalled original object hang (mode 7): S, or W with no climb. */
bool object_hang_release_ready();
bool push_contact_ready();
bool push_idle_ready();
/* Original Up (16) / Down (64) for camera-relative movement along Duke's facing. */
uint16_t push_pad(float move_x,float move_y);
/* D08U top-of-ladder mount (ladder_top.inc): Duke stands at the top of a plain
   ladder within reach; E there asks for the host-blended original transfer. */
bool ladder_top_available();
void ladder_top_request();
/* D08J1: E held, heading at a plain ladder whose bottom is out of reach from
   the floor, inside the catching window for the gait: press the original jump. */
bool ladder_leap_ready();
void ladder_leap_note();
/* The leap is under way (until the catch, landing or timeout): E's reach stays held. */
bool ladder_leap_active();
/* Duke is on a plain ladder: S also holds Cross so he climbs down and steps off. */
bool ladder_descent_ready();
int ladder_bottom_hang();
bool ladder_end_below();
bool ladder_mount_finishing();
/* Original ladder exit (190 top, 185 bottom step-off): directions stay neutral. */
bool ladder_exit_ready();
const char* controls_debug_json();
// D17: draw-time host state for render replay workers (frame_replay.cpp).
// D17B late camera (live process only): the view the next game update would
// take from the mouse input pumped so far (yaw/pitch, radians, as the orbit
// camera uses them), the orbit pivot and the first-person blend. False when the
// modern camera does not own the view.
// at_ms >= 0: the mouse look as it was at that time (performance clock, ms).
bool replay_gameplay_context();
// D23E/D23H: one second of gameplay judged by overclock_lease: 0 kept up,
// 1 behind real time, 2 not a gameplay second (menus, loads). Sheds presents
// on a sustained deficit and steps back up when emulation keeps up again.
// Returns 1 after a shed, -1 when behind and nothing is left to shed (pause
// the overclock), else 0.
int replay_load_window(int window);
void render_state_prepare();
bool late_view(double& yaw, double& pitch, double at_ms=-1);
bool late_pivot(double* xyz);
double late_fp_blend();
size_t render_state_size();
void render_state_save(void* out);
void render_state_load(const void* in);
// D11B: the eye view is live (Modernized, orbit lease, first-person blend > 0).
bool first_person_view_live();
// D11B: Duke's own actor draw is in progress (object meshes belong to him).
bool first_person_duke_drawing();
// D11B: any actor draw (0x800348d8) is in progress, until the next list step.
bool first_person_actor_drawing();
// D12: true while Duke's hand joint, its weapon, muzzle flash or held item is
// drawn with the first-person weapon transform.
bool first_person_weapon_drawing();
// D14: Modernized widescreen is presenting a wider view (any camera). Near
// polygons the original drops or tears then sit inside the revealed margins.
bool widescreen_near_clip_live();
// Last modern camera-lease refusal: "context", "identity", "state" or "" (none yet).
const char* lease_refusal_reason();
}
