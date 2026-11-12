#pragma once
#include <cstdint>
#include <vector>
#include <array>
struct CPUState;
namespace ttk {
// D11B: world-mesh near clipping for the eye view (near_clip.cpp).
// Screen-space helpers exposed for the native test.
struct NearGte {
    int16_t r[3][3];int32_t tr[3],fc[3],ofx,ofy;uint16_t h;
};
// GTE-exact RTPS for one vertex: packed SXY, SZ and the unclamped view position.
struct NearProjected { uint32_t sxy; int32_t x16,y16; uint16_t sz; double view[3]; bool safe; };
NearProjected near_project(const NearGte& g,int16_t vx,int16_t vy,int16_t vz);
// View-space plane for depth evaluation at the actual raster position.
struct NearDepthPlane { double n[3], d; };
NearDepthPlane near_depth_plane(const double a[3],const double b[3],const double c[3]);
double near_raster_depth(const NearDepthPlane& plane,const NearGte& g,double x,double y,double fallback);
// DPCS (sf=1, lm=0) of an RGBC word toward the far color with IR0.
uint32_t near_dpcs(const NearGte& g,uint32_t rgbc,int16_t ir0);
const char* near_clip_debug_json();
// D12: links the first-person weapon packets held back during Duke's draw
// into the nearest ordering-table slot, farthest first (see near_clip.cpp).
void near_clip_viewmodel_flush();
void near_clip_biomask(double dip,uint32_t light);
void near_clip_frozen_tint();
// D39B: the two-word flash record after it (screen_flash.h), 0 before the arena.
uint32_t near_clip_flash_record();
// D20I: the reinforcement count word (the retired D08G7 word, mirror + 4), 0 before the arena exists.
uint32_t near_clip_enemy_extras_word();
// D17A: the frame accounting the near clip carries from one renderer call to
// the next (packet budget, held weapon packets). A render replay worker loads
// the live frame's value so its redraws take the same decisions.
// host_region: the D17R1 host packet arena region the frame writes.
struct NearClipFrameState { uint32_t first_cursor,host_bytes,held_frame,host_region; bool region_chosen; };
// Every flip (0x8001fcbc) ends a displayed frame's packet region.
void near_clip_flip();
// D41A: the frame's camera (camera +0x20 matrix, 4.12 with row 0's aspect)
// for world positions of lit corners (relative to the eye); valid false
// stops them (dynamic_lights.inc).
void near_clip_light_frame(bool valid,const double m[9],bool smooth=false);
// D41F (DNTTK_SHADOW_PROBE=1, developer): emitted world corners (eye-relative) since the last call.
std::vector<std::array<float,3>> near_clip_shadow_probe_take();
// D41K: DNTTK_LIGHT_PROBE=1 (developer): the image's lights for the probe.
void near_clip_light_probe_lights(const void* lights,int n,float amb); // PSXModLight[n]
// Reserve draw-time scratch before replay workers fork.
void near_clip_prepare();
NearClipFrameState near_clip_frame_state();
void near_clip_load_frame_state(const NearClipFrameState& s);
// Diagnostic counters (world taken, world triangles, object taken, object
// triangles, mesh fallbacks, budget hits, copy overflows, refused).
void near_clip_counters(uint64_t out[8]);
// D17AR2 host visibility. True when a room-walk piece would be drawn here
// whole (Modernized unified geometry with the near path live).
bool near_clip_host_world_live();
// D52: the camera the view composition (0x80026164) draws from, read from
// guest memory (live frame or a redraw worker's image): 0x800da488 while
// 0x800c3c4c is set, the player's alternate camera 0x800d7010 (+0xb0 per
// player) while the player's +0x224 has the script bit 0x20000000 (security
// monitors, scripted views), else the player's camera 0x800d6eb0 (+0xb0).
uint32_t composition_camera();
// D52: true while that camera is not the player's own (a game-owned view).
bool game_camera_view();
// D41D1: the object animation table last seen while objects drew (0 if none).
uint32_t near_clip_object_animation();
// D23T: the object renderer's null mesh for an object whose every face is
// host-drawn: 1 on (default), 0 the original's vertex pass (developer
// DNTTK_OBJECT_NULL=0), 2 on with the skipped pass's leftovers poisoned (the
// D23T check). A negative mode only reads it. Returns the previous mode.
int near_clip_object_null(int mode);
uint64_t near_clip_object_null_count();
// D41D1, D41K: neon sign faces of static props (face record addresses,
// level_lights.inc); dynamic lights skip those faces, so a sign gives light
// but none falls on its letters, while the rest of the prop takes light.
// Emulation thread.
void near_clip_set_glow_faces(std::vector<uint32_t> faces);
// The one RAM effect drawing a world piece in a non-flat room has (the
// colour-index nibble clear), for a redraw-only live frame that draws none.
void near_clip_host_world_unlit(uint32_t mesh);
// Draws one world piece (cpu: a0 mesh, a1-a3 and ra as the room walk's call,
// GTE and render context set for its room) as the room walk would. False when
// it is left to the game's own walk (the frame cannot afford it).
bool near_clip_host_world_piece(CPUState* cpu);
// While filter is set, the room walk's own mesh calls draw only kept pieces.
void near_clip_host_walk(bool filter,const std::vector<uint32_t>& kept);
// Host pieces drawn, left to the game, walk calls nulled, left by the
// budget (far meshes, near meshes).
void near_clip_host_counters(uint64_t out[5]);
// Water clip diagnostic counters (seen, taken, guard_reject, grid_reject).
void near_clip_water_counters(uint64_t out[4]);
}
