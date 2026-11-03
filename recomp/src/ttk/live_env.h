#pragma once
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
#ifdef __cplusplus
}
#endif
