#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// D19: a DNTTK_* setting as last confirmed in the OPTIONS screens, else getenv().
const char* ttk_live_env(const char* name);
void ttk_live_env_set(const char* name, const char* value);
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
