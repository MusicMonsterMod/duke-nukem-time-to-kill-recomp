#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Shared Modernized text surface:
// 0 message (2x), 1 Atomic heading (2x), 2 centered message (2x),
// 3 small message at native 1x (console / compact quotes). ARGB8888 pixels.
// Returns zero for Vanilla, missing/corrupt packs, or insufficient output space;
// callers retain their existing generic renderer in that case.
int ttk_font_rasterize(const char* text,int style,uint32_t* pixels,int capacity_w,
                      int capacity_h,int available_w,int* width,int* height);
#ifdef __cplusplus
}
#endif
