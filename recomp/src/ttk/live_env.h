#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// D19: a DNTTK_* setting as last confirmed in the OPTIONS screens, else getenv().
const char* ttk_live_env(const char* name);
void ttk_live_env_set(const char* name, const char* value);
// D53E: the confirmed values travel to the redraw workers with each published
// frame (render_state_save/load): a forked worker's own table is the one it
// was forked with, so without this a live change reached paused frames (drawn
// live) but not the in-game redraws. pack writes name\0value\0 pairs ending
// with an empty name and returns the bytes used (whole pairs that fit, at
// least 1); unpack sets each value that differs in this process.
size_t ttk_live_env_pack(char* out, size_t cap);
void ttk_live_env_unpack(const char* in, size_t len);
// D40: the world renderer, switchable in game: 1 Classic, 0 Polymorph.
// Seeded from DNTTK_WORLD_RENDERER; a switch takes effect at a frame
// boundary (frame_replay.cpp) and reaches redraw workers with each job.
int ttk_world_classic(void);
void ttk_world_classic_set(int classic);
// D40: a developer switch's explicit value: -1 unset (the renderer decides),
// 0 for "0", else 1.
int ttk_env_switch(const char* name);
// D23N1: the clock for anything drawn from the time (glow sway, flicker,
// blinking). Live it is SDL's millisecond clock; a redraw job holds it at the
// job's start, so a job's picture depends only on its inputs (a rebuilt job
// matches the one it rebuilds, and the glows of one redraw do not move while
// it is drawn).
uint32_t ttk_draw_clock_ms(void);
void ttk_draw_clock_hold(uint32_t ms);   // 0 releases it
#ifdef __cplusplus
}
#endif
