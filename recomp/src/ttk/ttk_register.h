/* D23AH: the title's plugin registrations run in one fixed order.
 *
 * Each source registers its hooks from a constructor. Two things depend on the
 * order those constructors run in:
 *   - the GPU DMA arenas allocated there (near_clip's host and mask arenas,
 *     loading_widescreen's regions): savestates carry that aperture with the
 *     saving process's allocation records, and a load restores it only up to
 *     the first record that differs (memory.c, psx_mod_memory_snapshot_read);
 *   - function-entry hooks registered at the same guest address (0x8002bc18,
 *     0x8001fba0, 0x80026164, 0x8002b9f4, 0x8001ca4c, 0x80010000) run in
 *     registration order (mod_runtime.cpp, psx_mod_function_entry).
 * Plain constructors run in link order, which a link-time optimized build does
 * not keep (GCC merges them into one function): an LTO build laid the arenas
 * out differently, so a savestate's aperture came back only in part and the
 * game ran differently from the same state. The ranks below are the normal
 * build's link order (CMakeLists.txt sources), so every build, compiler and
 * optimization level registers exactly as the normal build always has.
 *
 * Prioritized constructors run before unprioritized ones. Registration only
 * touches function-local registries, plain C statics and constant-initialized
 * pointers, so it does not depend on any other file's dynamic initialization.
 * A new registering source takes the next free rank at its link position. */
#ifndef TTK_REGISTER_H
#define TTK_REGISTER_H

#include "mod_plugins.h"

#define TTK_RANK_MOVIE_POLL          1   /* fmv_poll.c */
#define TTK_RANK_SCENE_LIFETIME      2   /* scene_lifetime.cpp */
#define TTK_RANK_CONTROLS            3   /* modern_controls.cpp */
#define TTK_RANK_AIM                 4   /* weapon_aim.cpp */
#define TTK_RANK_NEAR_CLIP           5   /* near_clip.cpp: arenas first */
#define TTK_RANK_SKY_WIDESCREEN      6   /* sky_widescreen.cpp */
#define TTK_RANK_SKY_BORROW          7   /* sky_borrow.cpp */
#define TTK_RANK_GAS_WIDESCREEN      8   /* gas_widescreen.cpp */
#define TTK_RANK_FRAME_REPLAY        9   /* frame_replay.cpp */
#define TTK_RANK_LEVEL_SELECT       10   /* level_select.cpp */
#define TTK_RANK_GAME_MENU          11   /* game_menu.cpp */
#define TTK_RANK_ANIM_OVERRIDE      12   /* anim_override.cpp */
#define TTK_RANK_LOADING_WIDESCREEN 13   /* loading_widescreen.cpp: arena */
#define TTK_RANK_WATER_POCKET       14   /* water_pocket.cpp */
#define TTK_RANK_SECURITY_MONITOR   15   /* security_monitor.cpp */
#define TTK_RANK_PROP_LIGHTS        16   /* prop_lights.cpp */
#define TTK_RANK_SAVESTATE_PANEL    17   /* savestate_panel.cpp */
#define TTK_RANK_HIDDEN_LEVEL_SOUND 18   /* hidden_level_sound.cpp */
#define TTK_RANK_HIDDEN_LEVEL_LOOK  19   /* hidden_level_look.cpp */

#if defined(__GNUC__) || defined(__clang__)
/* Priorities 0-100 are the implementation's. */
#define TTK_REGISTER(name, rank)                                            \
    static void name(void) __attribute__((constructor(1000 + (rank))));    \
    static void name(void)
#else
/* No constructor priorities: link order, as PSX_MOD_CONSTRUCTOR always was. */
#define TTK_REGISTER(name, rank) PSX_MOD_CONSTRUCTOR(name)
#endif

#endif
