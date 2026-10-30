# D17AR3 part 2 - Redraw-only live frames at 120 Hz

2026-10-30. Second D17AR3 item, approved by the user from the last section of
[note 338](338-d17ar3-host-visibility-performance.md): at 120 Hz every
gameplay present is a worker redraw, so stop the emulation thread drawing the
live frame nobody sees. Status: accepted after the user's playtest ("it
feels unbelievably smooth").

Evidence is local under `recomp/analysis/d17ar3/` (`canary.py`, `loadshot.py`,
`loadseq.py`, `roprobe.py`, plus part 1's `headroom.py` and `timeline.py`;
JSON and logs). Every run used private copies of the D17AQ fixture card and
savestates, offscreen video, dummy audio and private debug ports, and
hash-checked the player's saves and profile before and after.

## What it does

While **engaged**, a live gameplay frame:

- selects its host world as before (rooms for the props and object passes are
  unchanged) but emits no host pieces and hands none back to the game's walk
  (`host_visibility.inc`);
- sends its GPU DMA lists with the drawing primitives (GP0 0x20-0x7F)
  dropped. State packets, fills, VRAM uploads, copies and reads still run, the
  walker still reads every word at its own guest time (DMA timing and
  everything the game reads are unchanged), and the widescreen prepass is
  skipped for such lists;
- is never presented: when the display shows its image, the presenter keeps
  the screen as it is, and the real image is not copied out.

Engaged only for gameplay images with the late camera and live workers, after
8 presents in a row showed a redraw. 3 presents in a row without one (workers
stalled or restarting), a failed redraw, or a savestate load draw the next
live frames again. Frontend, loading, menus, pause, inventory, split screen,
60 Hz and Vanilla are never engaged (60 Hz and Vanilla never start replay
presentation; the plugin passes `enable(0)` every frame).

## Design notes

- **Frame boundary is the flip.** TTK sends each frame as about 9 linked-list
  DMAs (two large ones of 400-550 primitives, the rest single upload/copy
  nodes), chained from the GPU completion interrupt after the flip
  `0x8001fcbc` and before the next composition. Arming at the chain close
  `0x8001fba0` caught only a one-node copy list, and `0x8001fa78` is not
  called live. Now every flip calls `psx_mod_live_draw_skip_arm(x)`, where x
  is whether the composition before it was engaged; every list started until
  the next flip latches x. A frame drawn without a composition (pause screen,
  menus, loading) therefore never inherits a skip.
- **Blind images.** The lists between two flips draw the image displayed at
  the display change after the second flip. If any of them skipped, that real
  image is blind: not captured, and a present that would show it keeps the
  screen as it is (no swap). Re-presenting the last cache entry was tried and
  dropped: after a load that entry had already been redrawn from a pre-load job
  over the new VRAM (garbled textures on the old view).
- **Savestate loads** mark the image blind until a fully drawn image is
  displayed and disengage (`psx_mod_savestate_loads()` changed), so the first
  frames after a load are drawn.
- **Thumbnails.** `savestate_grab_thumb` (save, pause entry, level end) takes
  the centre 4:3 of the last presented redraw while the displayed image is
  blind; otherwise the displayed VRAM picture as before.
- Not covered: the developer debug-port `screenshot`/`screenshot_file`
  commands still read VRAM (blind while engaged); `present_shot` and
  `replay_dump` show what is presented. There is no player screenshot key.

## Switches and diagnostics

- `DNTTK_REDRAW_ONLY=0` (developer) or console `redrawonly off|on|default`
  (session only) disables or re-enables it.
- `PSX_LIVE_SKIP_CANARY=1` (developer check): a skipped list leaves its draw
  area magenta, so any stale pixel that reaches the screen shows.
- `DNTTK_REDRAW_ONLY_TRACE=1`: compose and flip events to the session log.
- Debug `render_replay` -> `plugin.redraw_only`: engaged, frames, engages,
  drops, lists and primitives skipped, presents held, blind, nodes sent
  unchanged by reason (`kept`); `ttk_input` host visibility
  `redraw_only_pieces`.
- Framework API in `mod_plugins.h`: `psx_mod_live_draw_skip_enable`, `_arm`,
  `_blind`, `_stats`; implementation in `gpu.c` (node filter), `dma.c`
  (live walker), `gpu_gl_renderer.c` (blind state, hold, thumbnail grab),
  `savestate.c`.

## Checks

- Canary over all 10 fixture slots at 120 Hz with host visibility (96
  presents each while turning): no magenta in any presented image, post-load
  image, pause screen (pause menu drawn correctly over the scene), savestate
  thumbnail or presents after reloading a state saved while engaged. Engaged
  in every slot, about 1,300 primitives skipped per frame.
- Loads in sequence (slot 5 then 6 and back, twice): the earlier garbled-
  texture frame is gone; images match redraw-only off.
- 60 Hz: never engaged, no list skipped.
- `ttk-near-test` passes. Runtime patch re-exported; it applies to the pinned
  framework and reproduces the working tree exactly. No hashed codegen header
  touched (fixture savestates load).
- Not automated: resuming from the pause menu (the offscreen test input does
  not reach the pause menu), inventory, level transitions, split screen. These
  need the playtest.

## Results

Same build, redraw-only off against on, 120 Hz, internal scale 4, 30 s,
seconds at full rate (presents > 110/s) of 24:

| Route | Host visibility | Off | On | no_job off / on | Present p95 off / on |
| --- | --- | --- | --- | --- | --- |
| Slot 10, turning | on | 20 | 24 | 102 / 8 | 9.5 / 8.9 ms |
| Slot 10, standing at the loaded view | on | 1 | 24 | 71 / 3 | 26.7 / 8.8 ms |
| Slot 2, turning | on | 11 | 24 | 47 / 3 | 9.4 / 8.8 ms |
| Slot 2, standing | on | 20 | 24 | 19 / 3 | 10.0 / 8.8 ms |
| Slot 10, turning | off | 24 | 24 | 4 / 4 | 8.8 / 8.9 ms |
| Slot 2, turning | off | 20 | 24 | 15 / 3 | 8.9 / 8.8 ms |

Forced full rate (`headroom.py`, no shedding, host visibility on): slot 10
loaded view 47.7 to 60.0 game fields/s (slack 0.0 to 0.65 ms), slot 2 loaded
view 60.2 to 60.0 with slack 0.0 to 2.1 ms. This matches part 1's throwaway
experiment.

## Remaining

- Playtest at the player's settings, especially pause/resume, inventory,
  level changes, savestate save/load and thumbnails.
- The debug `screenshot` commands read the blind VRAM while engaged.
- D17AR3 continues: default switch for host visibility, level 28, the full
  regression sweep, then "how far we can push this renderer" and its name.
