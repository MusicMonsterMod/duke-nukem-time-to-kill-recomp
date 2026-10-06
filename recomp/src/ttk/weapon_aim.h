#pragma once
#include <stdint.h>
struct CPUState;
namespace ttk {
void aim_render_prepare();
const char* aim_debug_json();
// D12A: the original query-only segment trace (0x8006d980) under the aim code
// identity. kind 0 miss (hit = to), 1 world, 2 actor.
bool view_segment_query(CPUState* cpu,const double* from,const double* to,double* hit,int& kind);
}
extern "C" int ttk_aim_reticle();
extern "C" int ttk_aim_crosshair_enabled(void);
extern "C" void ttk_aim_toggle_crosshair(void);
// The project crosshair (recomp/assets/ui/crosshair.png) ARGB bitmap when the modern reticle is visible.
extern "C" int ttk_aim_crosshair_image(const uint32_t** pixels,int* width,int* height);
extern "C" int ttk_aim_crosshair_pixels(const uint32_t* pixels);
