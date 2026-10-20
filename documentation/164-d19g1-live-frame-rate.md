# D19G1 - FRAME RATE live after a 60 launch

Status: **Done** (user-accepted 2026-10-20: "it works! i can confirm it works"). Modernized only; Vanilla unchanged.

## Problem

D19G made FRAME RATE in the in-game OPTIONS apply on CONFIRM only while
replay presentation (D17) was running. A game launched at 60 never sets it up
(`psx_mod_set_present_replay` runs at activation, before the window), so a
change waited for the next launch.

## What changed

- Framework (tracked patch): `psx_mod_request_present_replay(fn, user, hz)`
  starts replay presentation while the game runs, and `fn = NULL` stops it.
  Like the D19G rate request it is queued and applied on the present path
  (`apply_mod_replay_request`, from `apply_mod_display_requests`). Start does
  what window creation does for a replay chosen at launch: provider, pacer
  tick (`gl_renderer_rp_tick`), service tick, swap interval 0. Stop clears them,
  restores the vsync choice from before replay
  (`s_mod_replay_saved_vsync`, also kept at activation), and the original
  present path runs again. `psx_mod_present_replay_available()` (OpenGL
  context and raster live, no netplay) tells the menu whether it can apply.
  `gl_renderer_set_replay_provider` now clears every captured replay image and
  the display deadline when the provider changes. New
  `gl_renderer_replay_available()`. `mod_plugins.h` and `gpu_gl_renderer.h`
  are not codegen-hashed; savestates stay valid.
- `frame_replay.cpp`: `activate` reads the developer settings at every
  Modernized launch; `apply_rate` holds the per-rate settings (interpolation,
  slack floor, pacing divisor). `frame_rate_set`: 60 stops replay
  presentation (from any launch, so 60 is always the original presentation);
  another rate starts it (render workers fork at the next gameplay
  composition, as at a launch) or retargets it. The timeline is reset on start
  and stop.
- `game_menu.cpp`: the description says NEXT LAUNCH from `row_live` (it used
  the row's static flag, so FRAME RATE never showed it). With OpenGL the row is
  live from any launch rate; with the software renderer it says NEXT LAUNCH.
- Change from D19G: a launch above 60 switched to 60 used to keep replay
  presentation at a 60 target; it now stops it.

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19g1-rate`, level 1 corridor savestate 11, port 9371).
Executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`.

| Check | Result |
| --- | --- |
| 60 launch, before any change | replay off, 0 replay presents, 59.9 fields/s (as before) |
| 60 -> 120 live | `replay presentation started`, 3 workers fork; in play 119.5 presents and 119.5 distinct images/s |
| 120 -> 144 | 144 presents, 143 distinct images/s |
| 144 -> 60 | `replay presentation stopped`; in play 0 replay presents, 59.7 fields/s |
| 60 -> Unlimited | started again; about 500 distinct images/s in play |
| -> Display, -> 60 | started (offscreen the host refresh is unknown: 60 presents), stopped |
| 120 launch -> 60 -> 120 | stopped, then started again |
| Resume hitch | worst gap between field polls 28-29 ms around the resume that forks the workers; plain play 22-24 ms, 120 -> 144 resume (no fork) 27.9 ms |
| Software renderer, 60 launch | FRAME RATE description ends NEXT LAUNCH; a change does not start replay |
| `--vanilla` | `Profile: Vanilla (this launch only)`, no replay |
| Native | input, controls (LEVEL00 fixture), aim, near, live-env, font, inventory pass |
| Python | 157 OK (12 skipped) |
| Patch | exported; applies to the pinned framework commit and reproduces the edited files; `check_repo.py` OK |

Harness notes: the pause list and our screens are found through
`render_replay.plugin.frontend.game_menu.owned`; savestates 2 and 5 of the
D19G copy start Duke at 10 health beside an enemy, so they die mid-test.

## Not verified / limits

- The real display: Display (monitor refresh), vsync restored after a stop,
  the feel of the switch. The user judges.
- The worker fork was not timed on its own; it happens once per session, at
  the first gameplay image after the first live start. Workers stay alive
  (idle) after a stop and are reused.
- Software renderer launched above 60 (unchanged from before): the plugin
  reports replay on but nothing is presented through it, and FRAME RATE
  changes do not apply in that session.
