// D53: Duke 3D-style security monitors (security_monitor.cpp). See
// documentation/432-d53-security-monitors.md.
//
// The near-clip object path, the second texture bank and the light list call
// the module through these pointers, set by its constructor, so test binaries
// that link near_clip.cpp or modern_controls.cpp alone need nothing more.
#pragma once
#include <cstdint>
#include "mod_plugins.h"

namespace ttk {
// The guest copy of the TV mesh (vertices, palette and polygon list).
inline uint32_t security_tv_copy=0,security_tv_copy_end=0;
inline bool security_tv_owns(uint32_t address) {return security_tv_copy && address>=security_tv_copy && address<security_tv_copy_end;}
// Self-lit copies of retail meshes (the console keypad): retail textures,
// fixed bright colour, drawn without room or dynamic light.
inline uint32_t security_glow_copy=0,security_glow_copy_end=0;
inline bool security_glow_owns(uint32_t address) {return security_glow_copy && address>=security_glow_copy && address<security_glow_copy_end;}
// The TV mesh (or a self-lit copy) for a mesh the object renderer is about to draw, or 0
// (no change). Former description: the TV mesh for a monitor mesh the object renderer is about to draw, or 0
// (TTK look, Classic, Vanilla, no pack, not a monitor).
inline uint32_t (*security_tv_swap)(uint32_t mesh)=nullptr;
// D53C: true while the object (a prop) is drawn as the Duke 3D TV: the look
// is on and its model is the TTK monitor panel the TV replaces.
inline bool (*security_tv_object)(uint32_t object)=nullptr;
// Paints the TV atlas into the 1024x512 second texture bank (x 0..255).
inline void (*security_tv_paint_bank)(uint16_t* bank)=nullptr;
// Screen lights for the TVs near Duke; returns how many were written.
inline unsigned (*security_tv_lights)(PSXModLight* out,unsigned room)=nullptr;
// Debug JSON object (looks, swaps, lights, D53A feed state).
const char* security_monitor_debug_json();
// D53A: the security camera feed (frame_replay.cpp draws it). The alternate
// camera the view composition draws through while Duke +0x224 has bit
// 0x20000000 (D52), one per player.
constexpr uint32_t security_feed_camera=0x800d7010u,security_feed_camera_bytes=0xb0u;
struct SecurityFeedRequest {uint32_t player;uint8_t camera[security_feed_camera_bytes];};
// Live frame: true when a feed picture should be drawn now (fills the camera).
inline bool (*security_feed_request)(SecurityFeedRequest& out)=nullptr;
// A finished feed picture is in the renderer's cache entry: put it on the TV.
inline bool (*security_feed_store)(int cache)=nullptr;
}
