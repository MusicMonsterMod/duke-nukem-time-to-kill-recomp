#pragma once
#include <stdint.h>
struct CPUState;
namespace ttk {
void aim_render_prepare();
const char* aim_debug_json();
// D49: Duke's view-aimed beam in projectile object (pool slot): its record and
// the view it was aimed from (eye, unit forward). False for other projectiles.
bool aim_beam_view(uint32_t object,uint32_t& record,double eye[3],double fwd[3]);
bool aim_beam_follow_enabled();
// D49C: redraws advance each energy beam's animated look (wave phase, stripes)
// to the redraw's time between game frames. DNTTK_BEAM_ANIM=0 / "beamanim off".
bool aim_beam_anim_enabled();
// D49F: first-person Laser Gatling bolts launch from the drawn barrel (and its
// pose turns in to match). DNTTK_BOLT_LAUNCH=0 / "boltlaunch off".
bool aim_bolt_launch_enabled();
// D12A: the original query-only segment trace (0x8006d980) under the aim code
// identity. kind 0 miss (hit = to), 1 world, 2 actor.
bool view_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind);
// D10F1: the same query for the camera lens, passing over one actor (0: Duke)
// and naming the actor it stops at (kind 2).
bool camera_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind,uint32_t ignore,uint32_t* actor);
}
extern "C" int ttk_aim_reticle();
extern "C" int ttk_aim_crosshair_enabled(void);
extern "C" void ttk_aim_toggle_crosshair(void);
extern "C" void ttk_aim_reset_crosshair_override(void);
// The project crosshair (recomp/assets/ui/crosshair.png) ARGB bitmap when the modern reticle is visible.
extern "C" int ttk_aim_crosshair_image(const uint32_t** pixels,int* width,int* height);
extern "C" int ttk_aim_crosshair_pixels(const uint32_t* pixels);
