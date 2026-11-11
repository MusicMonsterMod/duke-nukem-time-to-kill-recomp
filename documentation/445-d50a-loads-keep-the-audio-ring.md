# D50A - Loads, saves and level starts keep the audio ring full

Status: Needs playtest (2026-11-11). Job board: D50A. Follow-up found on the
way: D50B (a save can end the game, pre-existing).

## Report

2026-11-11 playtest, session logs 16:05 and 16:20 (real display, 180 Hz):
after savestate loads (UI slots 1 and 2), a new game's level 0 start and a
save, the governor logged `audio running low` with the ring at 0-40 of 180
ms and up to about 1,700 underruns in one interval; presents fell to about
90 a second for 20-30 s. Separately `[TTK lamp] level N: ... neon sign faces
take no dynamic light (20-70 ms)` stalled the game thread at every level
start and load.

## Root causes

Evidence labels: CONFIRMED (code or our own measurement), INFERRED.

1. **The audio bridge never filled up again after running low** (CONFIRMED,
   `recomp_audio_drc.h`). The output is pulled at the device's rate from a
   ring the game fills at its own rate; a rate trim of at most 0.5% holds
   the ring near its 180 ms target. The bridge waited for the target only
   once, at startup (`primed`). After that, anything that took audio out of
   the ring without putting it back left it short for good, refilled by the
   trim at about 5 ms a second:
   - a savestate load: the emulator is paced to the wall clock and never
     repays a stall, and the load stalls it (apply, renderer restart, the
     first frames drawn live again): each load cost the ring 30-60 ms in the
     lab, and repeated loads stacked (offscreen lab: 140, 115, 90, 65, 61,
     54 ms over six loads). The legacy push path cleared its queue on a load
     (`psx_sdl_audio_clear`); the bridge kept the old audio and played on;
   - a turbo-load mute (CD loading at level starts and after some loads):
     the pump stops while the device keeps pulling, so the ring empties by
     design; play then resumed into an empty ring, where every gap between
     the game's 60 Hz pushes underruns: the player's 0-40 ms, the underrun
     counts and the faint crackle;
   - a save: written on the emulation thread, 85 ms in level 3 (serialize,
     zlib, file), more than five 180 Hz frames; the ring fell 90-150 ms.
   The governor then shed presents (correctly by its D50 rule: the ring
   was low and falling, with underruns).
2. **The neon sign pass ran on the game thread** (CONFIRMED). At 45 ticks
   after each level start or load, `level_lamps_signs` scanned every static
   prop's mesh, scored its textures from VRAM (one emulator call per texel),
   found lamp candidates, cast rays for them and kept only the neon faces:
   20-70 ms (level 3, 165 lamps: 69 ms).
3. Also seen (CONFIRMED, not changed here): for the first 10 or so logic
   frames after a load the emulation thread draws the frames live again
   (redraw-only switches off at a load, D17AR3); those frames take 40-120
   ms instead of 33. The worker redraws then miss their presents for 250
   ms at a time ("workers behind") because in the level 3 corridor each
   worker job takes about 17 ms, close to the three workers' limit at 180
   Hz. That cost belongs to D23N (captured redraws) and D41M.

## Change

Framework (`time-to-kill-accepted-source.patch`):

- `recomp_audio_drc.h`: the bridge re-buffers. When the ring runs dry while
  playing it fades out (3 ms) and waits until the ring holds the target
  again before playing (`rebuffers` counted); `rab_rebuffer(b, clear)` does
  the same on request, `clear` dropping the queued audio first. The
  controller holds its correction while not playing.
- `main.cpp`: a savestate load drops the old audio and re-buffers (as the
  legacy path clears its queue); the end of a turbo-load mute re-buffers
  when the ring is under 90% of its target. `psx_audio_out_rebuffers()`;
  the debug `audio_stats` reports `rebuffers`.
- `boot_state.c`, `savestate.c`: saves are written off the emulation
  thread. `boot_state_capture` copies the state in its disk layout with raw
  sections; a writer thread compresses each section exactly as before and
  writes the file beside the target, renamed over it when complete
  (`boot_state_write_packed`). The next poll after the write reports it as
  before (status, `SAVED` line, diagnostics, notify). A load, another save
  and exit wait for a write in flight. `save_timing` logs the capture and
  write times. Developer `PSX_SAVE_VERIFY=1` also writes the old direct
  save beside it (`.direct`) for a byte comparison.
- `gpu.c`: `gpu_vram_copy` (all of VRAM in one step after one GL sync).
  Render-copy name headers updated.

Title:

- `level_lights.inc`: a new neon scanner (`neon_scan`) that keeps only what
  the neon set needs (the same rule: static props, textured non-animated
  faces, 64+ texels, 40% dark, 10% neon) and reads a copy of main RAM and
  VRAM taken in one step; it runs on its own thread and the game thread
  applies its result when ready. The lamp editor's `lamp signs` keeps the
  old full scan. Developer `DNTTK_NEON_CHECK=1` runs both and logs whether
  the face sets match.
- `savestate_panel.cpp`, `main.cpp`: the slot sidecar (level, statistics
  screen) is taken when the state is copied
  (`ttk_savestate_captured`) and written once the file is.
- `modern_controls.cpp`: the hitch line counts rebuffers.

Vanilla: the bridge and the save writer are shared by both profiles (the
same audio and the same save files, written off the emulation thread); the
neon scanner and the hitch line are Modernized only.

## Evidence

Lab `recomp/analysis/perf/lab.py` (local): a fresh private copy of the
player's cards and settings, offscreen on the player's GPU (SDL
`offscreen`), real audio into a PulseAudio null sink `ttklab`, the ring
sampled every 50 ms through the debug port. Before = the build without
these changes; after = with them.

Savestate loads (slots 8, 1, 2: the level 3 corridor of UI slot 9 and two
level 6 / level 2 saves; each loaded twice in one launch), the ring's lowest
point from 2 s after the load to 8 s, and where it ends:

| Rate | Build | Lowest after 2 s (worst / mean of 6) | End (mean) | Rebuffers |
| --- | --- | --- | --- | --- |
| 120 | before | 94 / 119 ms | 150 ms | - |
| 120 | after | 146 / 170 ms | 185 ms | 5 |
| 180 | before | 105 / 136 ms | 166 ms | - |
| 180 | after | 141 / 168 ms | 177 ms | 5 |
| 240 | before | 79 / 123 ms | 157 ms | - |
| 240 | after | 119 / 160 ms | 194 ms | 5 |

(The first load of a launch starts from an empty ring in both builds, so 5
of 6 loads re-buffer.) Before, repeated loads stacked: six loads in one
launch left the ring at 140, 115, 90, 65, 61 and 54 ms. No underruns in
either build offscreen: the player's 0-40 ms and underrun bursts need the
real output device, but the mechanism (play resuming into a near-empty
ring) is the one measured here.

Saves (slot 11 on the lab cards, twice per launch): the ring stayed above
150 ms in both builds offscreen at 120/180/240 Hz; the game frame holding
the save took 111-119 ms before and about 18 ms after (frame trace).

Level start (console `level 3` from the corridor): no turbo-mute drain
happened in this lab in either build (no rebuffer), lowest point 111-161 ms
in both; the player's level 0 start drain was not reproduced offscreen.

- Saves: `PSX_SAVE_VERIFY=1` on two saves, the asynchronous file and the
  direct one are byte-identical; the emulation thread pays 4-7 ms (copy and
  thumbnail) instead of 85 ms; the file is complete about 85 ms later and
  reported on the next poll (175 ms after the request).
- Neon: on the game thread 1.4-3.0 ms (the copy) instead of 20-69 ms; the
  scan takes 33-108 ms on its own thread. `DNTTK_NEON_CHECK=1` on levels 0,
  2, 3 and 6 (UI slots 4, 3, 9 and 2): the new and old face sets are
  identical (22, 1, 4 and 0 distinct faces).
- Re-buffer and the stall after a save and a load (D50B): a loop of loads,
  saves and loads (8 rounds a launch, 3 launches each) stalled once in 19
  rounds with the unmodified build, once in 23 with this build and 0 in 24
  with this build and `PSX_AUDIO_LOAD_REBUFFER=0` (developer A/B switch):
  the stall predates this change.
- Vanilla (`--vanilla`, same lab): loads keep the ring at 158-174 ms after
  2 s; a save costs the emulation thread 4.6 ms and writes in 90 ms; no
  neon or hitch lines (Modernized only).
- Tests: `ttk-near-test` PASS, `ttk-flash-test` ok, `ttk-live-env-test`
  PASS, `ttk-audio-test` PASS; `ttk-controls-test` links again (stubs added
  for the D41F shadow calls and the two new functions) and stops at its
  early fixture check (line 493), as before this change.

## Limits

- Offscreen lab with a null sink: the player's real output device, window
  and 180 Hz display remain to be confirmed in play.
- A load still costs a short silence (the re-buffer: about 180 ms of audio
  produced before playing, during the load's own pause).
- The first frames after a load still draw live (point 3 above); D23N and
  D41M address the worker cost behind it.
- D50B: a save admitted at the IRQ return to the flat scheduler can end the
  game, and a load after a save can stall it; seen with and without this
  change.
- D50C: walking forward from UI slot 9 for about 25 s ends at the pit
  (Duke at 24065, 6665, -17157) with `GPU GP0 unknown command` (emulation
  halted) or a stalled emulation thread, with and without this change.
