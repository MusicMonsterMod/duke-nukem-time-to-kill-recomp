# D40 - Switch renderer live in game (Polymorph / Classic)

2026-11-03. Status: Done (accepted 2026-11-03: "this is awesome. confirmed working! lock it in close it out."). User: "i want the ability to switch
renderer on the fly like in eduke". Before D40, OPTIONS > DISPLAY > RENDERER
(D17AR6, [note 343](343-d17ar6-renderer-choice.md)) applied on the next launch.

Evidence is local under `recomp/analysis/d40-live-renderer/` (`probe.py`,
logs, shots, `*-results.json`). Every run used private copies of the D17AQ
fixture cards and a private profile, offscreen (NVIDIA EGL), debug port; the
player's saves, cards and profile were hash-checked unchanged after each run.

## Survey: what the renderer choice touches

| Consumer | Before D40 | Class | D40 |
| --- | --- | --- | --- |
| Unified geometry `unified()` (near_clip.cpp; also water_clip, model_depth) | `DNTTK_UNIFIED_GEOMETRY`, latched | frame boundary (read many times a frame) | live state; developer value wins |
| Host world visibility (host_visibility.inc) | `DNTTK_HOST_VISIBILITY`, latched; console override | frame boundary; reaches workers through RenderState (D17AR2) | live state; console and developer values win |
| Redraw-only live frames (frame_replay.cpp) | `DNTTK_REDRAW_ONLY`, latched; console override | decided per composition, armed at the flip | live state; console and developer values win |
| Render thread (D17AW, render_thread.c) | chosen once at activation | frame boundary; needs stale slot snapshots forgotten | toggled at the switch (framework) |
| Present tint / Classic flashes (`screen_flash_classic()`) | `DNTTK_WORLD_RENDERER`, latched | per present / per player update | live state; a flash in flight is cleared on a switch to Classic |
| Classic frozen quads (first_person.inc, D35J5) | per frame from the latch | per frame | follows the live state |
| Near-clip packet arenas | allocated once, independent of the renderer; `cursor_limit()` per frame from `unified()` | per frame | unchanged (switch only at a flip) |
| Redraw workers (forked) | inherit the launch value | per job | RenderState carries `world_classic` |
| Near path (`DNTTK_NEAR_CLIP`) | on in both renderers (D17AR6) | - | unchanged |

Nothing needs a level reload.

## Implementation

- **One runtime state** (`live_env.cpp`): `ttk_world_classic()` /
  `ttk_world_classic_set()`, seeded from `DNTTK_WORLD_RENDERER`.
  `ttk_env_switch(name)` reads a developer switch as unset (-1), `0` or on, so
  `DNTTK_UNIFIED_GEOMETRY`, `DNTTK_HOST_VISIBILITY` and `DNTTK_REDRAW_ONLY`
  still win when set; unset, the live renderer decides (Classic: off).
- **run.py** no longer sets those three switches for Classic; it exports only
  `DNTTK_WORLD_RENDERER` (Vanilla: `classic`). The game derives them.
- **Frame boundary**: a switch is a request (`world_renderer_request()`),
  applied on the emulation thread at the next flip (`0x8001fcbc`,
  `redraw_only_flip`, main process only, before `near_clip_flip()` and the
  redraw-only arm). The frame just composed keeps the renderer it was built
  with (its redraws carry it in RenderState: if it was a redraw-only frame,
  its lists stay skipped and its Polymorph redraw is shown); the next frame is
  drawn wholly by the new renderer. Redraw-only re-engages on its own after 8
  good presents on Polymorph and drops at the next composition on Classic.
- **Redraw workers**: `RenderState.world_classic`, set from the live frame
  and loaded per job (as host visibility already was).
- **Render thread** (framework, `render_thread.c`):
  `psx_mod_set_render_thread()` may change while the game runs. Turned off,
  queued redraws still finish and are adopted; new ones draw on the
  emulation thread. Turned on again, `psx_rt_drop()` forgets the slots'
  snapshots (older than their workers' frames); each slot draws on the
  emulation thread until its next publish. First start is lazy, as before.
- **Flashes**: a switch to Classic clears the flash record
  (`screen_flash(0)`), so a Polymorph tint stops at once and does not return
  on a switch back. Classic's frozen quads and Polymorph's frozen tint follow
  the renderer each frame.
- **OPTIONS > DISPLAY > RENDERER** is a live row (no NEXT LAUNCH): CONFIRM
  requests the switch and the side file saves the choice as before.
- **Console** `renderer [polymorph|classic]` (EDuke32's `setrendermode`),
  this session only; `renderer` alone names the current one; Vanilla answers
  that the switch needs the Modernized profile. Listed in `help`.
- **Diagnostics**: `[TTK renderer] classic (switch N, field F)` in the log;
  the replay status JSON (`render_replay` plugin) has
  `renderer: {renderer, pending, switches, render_thread}`.

## Evidence (offscreen, 1280 wide window, private fixture copies)

| Case | Result |
| --- | --- |
| Flags, 4 switches (third, 180) | each switch: renderer, unified, host visibility, redraw-only and render thread follow within 30 fields; host-visibility frames stop on Classic and resume on Polymorph; redraw-only re-engages on Polymorph; 0 failures |
| Matched still views, slots 1, 3, 6, 10 (third, 180) | Polymorph -> Classic differs (mean abs 5.2-7.2); back to Polymorph matches the first shot (0.01-0.61; slot 1 3.5 from animation) |
| Stress, 40 switches every 0.4 s, walking, turning, firing | 180: 60.0 fields/s, worst 0.4 s window 58.3, 180.1 presents/s, 0 failures; 120 first person (Classic launch): 59.9, 58.8, 119.4, 0 failures; 60 first person: 59.9 fields/s |
| Present dump across switches while turning (24 presents each) | no black or torn present (per-present means within 1.7); same image cadence as a Classic launch; distinct images/s with and without a switch match (the later fall to 30/s happens with no switch: the scripted route) |
| Flash in flight (`flash=100,63,0,0`) | Polymorph tints; on Classic the level is 0 and no tint is drawn; back on Polymorph none returns |
| Frozen (hurt type 7), both ways | third person: Classic no tint, Polymorph frozen blue; first person: Classic's frozen quads, Polymorph's tint |
| Savestates | save on Classic, load on Polymorph and on Classic: both load and draw with the session's renderer; 0 failures |
| Classic launch -> Polymorph | the render thread starts mid-session ("1 render thread(s) ready"), 0 failures |
| OPTIONS keys (DISPLAY, RENDERER Right, CONFIRM) | row shows no NEXT LAUNCH; switches in game; side file `world_renderer: classic` |
| Load-shedding governor | pace divisor 1 and no sheds for 20 s after a switch, either way |
| Vanilla | console refuses the switch; Classic, no host frames |

Suites: `ttk-live-env-test` (new world-renderer and switch cases),
`ttk-flash-test`, `ttk-near-test` (now links `live_env.cpp`),
`ttk-input-test`, `ttk-aim-test`, `ttk-controls-test` (LEVEL00, LEVEL01, all
levels; 125 PASS); Python 225 with the one failure that predates this job
(`test_primitive_trace`), 2 skipped; `test_player_profiles` updated (run.py
sets none of the three switches). Framework patch exported; it applies to the
pinned commit `08ec704` and reproduces the working tree. No generated code or
codegen hash change.

## Limits

- The console switch is session-only by design; the OPTIONS row shows the
  saved choice, not a console switch made since.
- A first playtest report of Classic looking wrong after a switch did not
  reproduce (player settings and slot 3 copies: a live switch matched a
  Classic launch through the same menu path and walk); the user then
  confirmed the switch working on the real display.
- Classic is frozen (2026-11-01): it was not changed, only reached live.
