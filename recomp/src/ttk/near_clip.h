#pragma once
#include <cstdint>
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
// D17A: the frame accounting the near clip carries from one renderer call to
// the next (packet budget, held weapon packets). A render replay worker loads
// the live frame's value so its redraws take the same decisions.
struct NearClipFrameState { uint32_t first_cursor,host_bytes,held_frame; };
// Reserve draw-time scratch before replay workers fork.
void near_clip_prepare();
NearClipFrameState near_clip_frame_state();
void near_clip_load_frame_state(const NearClipFrameState& s);
// Diagnostic counters (world taken, world triangles, object taken, object
// triangles, mesh fallbacks, budget hits, copy overflows, refused).
void near_clip_counters(uint64_t out[8]);
}
