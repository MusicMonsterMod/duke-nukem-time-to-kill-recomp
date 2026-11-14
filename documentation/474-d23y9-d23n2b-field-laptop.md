# 474 - D23Y9 resolution headroom (cancelled), D23N2B piece 5 (field laptop)

Outcome: D23Y9 measured, DYNAMIC RESOLUTION built and measured, then
withdrawn and removed at the user's request (section 5); D23N2B piece 5
exact (section 6); D23V not started (the session was paused for the user's
playtest and closed out). A menu fix and a menu grey-out came from that
playtest (section 7).

Autonomous session of 2026-11-14 (evening) on the field laptop (i7-5600U,
2 cores / 4 threads, Intel HD Graphics 5500, 60.01 Hz panel). User: "work on
these performance jobs on the field laptop, in this order, autonomously ...
1. Resolution headroom at 120 (add it to the board as its own job first) ...
2. D23V, the GPU floor ... 3. D23N2B: continue from note 471 section 3 ...
then D23N3 and D23N4 as far as exact matches allow." The user was away for
the whole session.

Method throughout: the D23Y6 lab `recomp/analysis/d23y6/` (local), the
player's profile copied read-only with the mode set in the lab's profile copy
(`CTRL` / `PRES`, never `LAB_` switches for profile settings), each run's
settings line checked in its log (`comb3.py` prints it), private card copies
only; interleaved and rotated A/B (`abc.sh`), medians and lowest tenth of
per-second game frames and presents, present cadence (screen fresh: the
share of the panel's refreshes that show a new picture), GPU busy (RC6) and
clock, CPU clock per run. Real display while the user was idle (every run
waits for X idle), offscreen for profiles. Host checked first: AC power,
power profile performance, no backup, update or indexing activity during
the runs (timers next at 00:00-00:22), load average under 1.5 idle, 6 GB
memory available. Evidence labels: MEASURED, CODE, INFERRED.

## 1. D23Y9: resolution headroom at 120 (MEASURED)

The player's settings (2x, FRAME RATE 120, LIGHTING ENHANCED, LIVE SHADOWS
OFF, MODEL SHADOWS ROUND, extended draw distance, window 1316 wide), the
real display, 3 interleaved runs each, 30 s walks (`d23y9.sh`). On the 60 Hz
panel FRAME RATE 120 presents at the panel's rate (D23Y6), so holding means
60 even pictures a second.

| Scene | scale | game median / lowest tenth | presents median / lowest tenth | screen fresh | rate changes a run | GPU busy @ clock | CPU clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| DUKE HILL town (DH5) | 2x | 60.0 / 58.8 | 31 / 30 (run medians 30 55 55) | 68.3% | 2.7 | 70% @ 581 MHz | 2.98 GHz |
| | 1x | 60.0 / 59.3 | 60 / 59 | 98.3% | 0 | 62% @ 432 MHz | 3.10 GHz |
| DUKE HILL start (DH) | 2x | 60.0 / 58.9 | 60 / 59 | 98.0% | 0 | 75% @ 644 MHz | 3.09 GHz |
| | 1x | 60.0 / 59.3 | 60 / 59 | 98.3% | 0 | 61% @ 389 MHz | 3.09 GHz |
| UI slot 2 | 2x | 60.0 / 59.1 | 60 / 59 | 96.9% | 0 | 76% @ 659 MHz | 2.30 GHz |
| | 1x | 60.0 / 59.5 | 60 / 60 | 98.3% | 0 | 64% @ 450 MHz | 3.12 GHz |
| UI slot 8 | 2x | 59.8 / 57.7 | 60 / 31 | 89.2% | 1.7 | 74% @ 604 MHz | 2.20 GHz |
| | 1x | 60.0 / 59.2 | 60 / 59 | 97.7% | 0 | 61% @ 437 MHz | 2.59 GHz |
| UI slot 9 | 2x | 60.0 / 59.3 | 59 / 30 (run medians 60 30 60) | 76.6% | 0.7 | 69% @ 565 MHz | 3.11 GHz |
| | 1x | 60.0 / 59.2 | 60 / 59 | 95.7% | 0.3 | 59% @ 401 MHz | 3.12 GHz |

Reading. At 1x every scene holds 60 even pictures. At 2x the town, UI 8 and
UI 9 step to 30; every step's reason in the logs is "emulation behind" (the
game short of 57 frames in a second), never late pictures (repeats 0%). The
GPU is not saturated (busy 69-76%) but runs 130-250 MHz faster at 2x, and the
CPU clock drops (UI 2 3.12 -> 2.30 GHz, UI 8 2.59 -> 2.20): the town is
power-bound, the GPU's share of the 15 W package taking the CPU's clock
(note 463), so it shows as a CPU shortfall and is cured by GPU work. 1.5x is
not available: the GL renderer's surfaces are VRAM at a whole multiple
(`s_scale`, integer in every viewport, scissor, copy and readback), so the
steps below 2x are 1x only.

## 2. D23Y9 built: DYNAMIC RESOLUTION (CODE; removed, section 5)

A lower resolution holds the pictures, so (as asked) a dynamic internal
resolution, a setting, OFF by default until the user's playtest:

- Framework (`gpu_gl_renderer.c`, exported): `live_rescale` changes the
  internal scale between two presents. The hr and wide surfaces (and the
  scratch copy) are re-created at the new size under the same FBO names, the
  colour (filtered going down, nearest going up), depth and stencil blitted
  across on the emulation thread, the geometry shift uniforms re-set, the
  hold image dropped. Requested with `gl_renderer_request_scale` (applied
  after the next swap outside a redraw session); never above the configured
  scale. Render threads start at the configured scale and follow each
  snapshot's scale in `gl_rt_load` (their surfaces re-created, the snapshot
  loaded whole). `psx_mod_request_internal_scale` / `psx_mod_internal_scale`
  for the game side; `render_copy/rc*_names.h` gained the four new globals.
- Game (`frame_replay.cpp`): with DNTTK_DYNAMIC_RES on, in
  `replay_load_window`, the first slow second (game frames under 57 but 40
  or more; a hitch under 40, such as a load or save, needs a second slow
  second) drops one scale step before any present-rate step; after 20 s x
  backoff clean seconds at the full present rate it tries one step up; a try
  that falls behind within 10 s doubles the backoff (at most 16); the backoff
  halves after each clean minute at the configured scale. Off, with the
  redraws off or at the original presentation, the configured scale comes
  back. Developer: `DNTTK_DYNRES_WAIT=seconds`, console `scale` / `scale N`
  (held until `scale 0`). The render-replay JSON reports `scale`,
  `res_drops`, `res_rises`, `res_backoff`.
- Setting: OPTIONS > DISPLAY > DYNAMIC RESOLUTION OFF / ON (live),
  `--dynamic-resolution on|off`, profile `dynamic_resolution` (schema 76,
  older profiles get OFF), the launcher's settings line, the manual. Vanilla:
  always off (the launcher sets `DNTTK_DYNAMIC_RES=off`; Vanilla has no
  redraws).

A switch costs 0.6-16.7 ms on the emulation thread (median about 5 ms).

## 3. D23Y9 pictures (MEASURED)

Matched shots from the same savestate at the same game frames
(`CASES=dyn`, offscreen), switching with the console mid-run:

- Original presentation (FRAME RATE 60), UI slot 2: 2x throughout against a
  run switched to 1x at frame 160 and back at frame 300: 0.00% of pixels
  differ before, 0.00% after the return (frames 340 and 900); during 1x the
  picture equals a run started at 1x (0.00%).
- FRAME RATE 120 with redraws on the render thread, UI slot 8: the 1x stretch
  equals a 1x launch (0.00% and 0.02%), the return equals 2x (0.01%, 1.17% at
  frame 900, within the 0-1.4% of two unswitched runs at 120, whose late
  camera depends on real time); no failed redraw sessions.

So the picture at the configured scale is unchanged, and while lower it is
exactly the 1x picture (13-17% of pixels differ from 2x: softer edges).

## 4. D23Y9 on against off (MEASURED)

The real display, the player's settings at 120, interleaved. Tuned twice:
the first build waited for two slow seconds (in the town the CPU fell to
1.3-1.9 GHz meanwhile) and retried 2x after 10 s; the second dropped from the
first slow second and retried after 20 s; the third ignored a hitch second
under 40 frames.

| Scene | OFF: presents median / lowest tenth, screen fresh, rate changes | ON (second build, 3 runs) | ON (third build, 1-2 runs) |
| --- | --- | --- | --- |
| DUKE HILL town | 50 / 30, 76.1%, 3.3 | 60 / 59, 97.7%, 0; below 2x 58% of the walk | 60 / 59, 97.6%, 0; below 2x 86% |
| DUKE HILL start | 60 / 59, 95.1%, 1.0 | 60 / 59, 98.1%; below 2x 57% | 60 / 59, 97.9%; below 2x 57% |
| UI 2 | 60 / 59, 92.9%, 0.7 | 60 / 59, 97.8%; below 2x 57% | 60 / 59, 98.0%; below 2x 57% |
| UI 8 | 60 / 59, 97.0% | 60 / 59, 97.5%; below 2x 70% | 60 / 59, 97.8%; below 2x 96% |
| UI 9 | 60 / 59, 92.9%, 0.7 | 60 / 59, 92.9%; below 2x 71% | 60 / 59, 97.7%; below 2x 61% |

ON held the town at 60 even pictures, but it also lowered the resolution in
scenes that hold 60 at 2x for most of each walk (the savestate load's slow
seconds started it; the 20 s retry rarely came within a 30 s walk).

## 5. Withdrawn (user decision)

User, 2026-11-14, after a playtest: "i just cant accept this dynamic
resolution thing as a solution when quake alkaline plays at the full 120fps
at native resolution just stupidly slick, smooth, absolute twitch perfect
controls ... i dont want to degrade the game that way, and for that reason,
i request we revert it first, and then scratch it off the board." Removed
completely: the framework live rescale (the exported patch is identical to
before), the controller, the OPTIONS row, `--dynamic-resolution`, the
console `scale` command and the manual entry. The profile schema stays at 76
(the user's profile had migrated) and drops the retired
`dynamic_resolution` key. D23Y9 Cancelled; its measurement (section 1)
stands: the town at 2x is power-bound, and its cure is less CPU and package
power per redraw (D23N2B-N4, D23V), not a lower resolution.

The explanation given to the user (from notes 462, 463 and section 1): each
picture beyond the game's own is a redraw that re-runs the game's drawing
code (23-30 ms of CPU here), and the 15 W package shared by CPU and GPU
lowers the CPU's clock when both work; the input-to-picture time (113-135
ms against Alkaline's about 52) is mostly the original game's 30 Hz logic
and buffering. Captured redraws (D23N) remove most of the redraw CPU.


## 6. D23N2B piece 5: matrices built in a job, on the host (MEASURED, CODE)

Note 471 section 3 left the sky's three transform calls and `0x8003133c`
(one each a job, never exact from the job's start memory). Disassembled
(the owned EXE, local scratch only):

- The sky routine `0x800388e4` builds three matrices on its stack from the
  camera block and one timer: the backdrop (ra `0x80038c68`) from the look
  direction (`cam +0xc`, `+0x10`) through VectorNormal, ratan2 and an
  acos-like table into a yaw angle; the two cloud layers (ra `0x80038d7c`,
  `0x80038e18`) from `*0x800c0c64 >> 5` and `>> 3`, the first scaled by
  (0x1000, 0xa00, 0x1000); each through `0x8002afe4` (rsin/rcos products
  truncated toward zero, then MatrixNormal: GTE OP twice and the VectorNormal
  core per row) with the eye as translation.
- `0x8003133c` is not from the camera (note 471 inferred that): its function
  `0x80030fe8` builds a yaw matrix from an object's record (`*(s1 +0x3c) + 32
  * byte[*(s1 +0x40) +0x24]`, the larger of two normal pairs normalised) and
  a height argument.

Host ports of all of it (`redraw_n2.inc` namespace `sk`, developer
`DNTTK_N2_CHECK=128`), checked word for word at each transform entry before
our callbacks (the 9 rotation halves and the translation; the pad half is
stack). One decoding error on the way (the core's GPF is sf 0, word
0x4b90003d) was found by the check itself. Offscreen at 180, walking,
turning and firing:

| View | `0x80038c68` | `0x80038d7c` | `0x80038e18` | `0x8003133c` |
| --- | --- | --- | --- | --- |
| UI 2 | 2,240 exact, 0 differ | 2,240 / 0 | 2,240 / 0 | 2,240 / 0 |
| UI 8 | 1,664 / 0 | 1,664 / 0 | 1,664 / 0 | 1,664 / 0 |
| UI 9 | (no sky) | | | 2,624 / 0 |
| DUKE HILL town | 64 / 0 | 64 / 0 | 64 / 0 | 64 / 0 |

With the camera of a later job predicted exactly (note 456, piece 2), these
four matrices can be host-built for a later job from its predicted camera,
the timer and the object records (INFERRED: those two are the same for every
job of a frame, as they are written only by the game's own frame, not by
drawing; not yet checked across jobs).

## 7. Playtest findings (CODE, MEASURED)

- LIGHTING changed in OPTIONS took effect only after a relaunch at FRAME
  RATE 120 (user report): every picture there is a redraw worker's, and
  leaving OPTIONS refreshed the workers for MODEL SHADOWS, GLOWS and LIVE
  SHADOWS only. LIGHTING now refreshes them too (`game_menu.cpp`
  `leave_screen`). Not checked in a run (the lab cannot drive OPTIONS
  offscreen; the path is D41S2's).
- With DRAW DISTANCE ORIGINAL no lighting effect shows (user report):
  measured in UI slot 2 at 120, Relit and Enhanced at ORIGINAL are
  pixel-identical to LIGHTING OFF; the lights are attached in the host world
  pass, which `draw_distance.inc` runs only with EXTENDED. At the user's
  request ("grey them out today, but add it to the backlog as a fix for
  another time"): OPTIONS greys out LIGHTING, LIVE SHADOWS, MODEL SHADOWS and
  GLOWS while DRAW DISTANCE is ORIGINAL ("NEEDS DRAW DISTANCE EXTENDED, IN
  ADVANCED"; the D19N `needs` mechanism), the manual says so, and the fix is
  backlog job D41V. Not seen in a run (as above).

## 8. Reproduce

`recomp/analysis/d23y6/` (local): `d23y9.sh` and `comb3.py y9x2,y9x1` (1x
against 2x), `d23y9b.sh` / `comb3.py y9off,y9on` (the removed setting),
`CASES=dyn` (matched shots around a console switch; needs the removed
build), `RATE=180 OFFSCREEN=1 LAB_DNTTK_N2_CHECK=128 CASES=cad` (piece 5),
`CASES=atload AT=60,240` with `CTRL='{"dynamic_lights":"relit","draw_distance":"original"}'`
(the ORIGINAL lighting finding). Disassembly: the owned EXE through
`llvm-objdump` in a local scratch directory.
