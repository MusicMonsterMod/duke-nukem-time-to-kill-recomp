# 476 - D23V GPU floor, D23N2B piece 6, D23Y7 and D23AA (field laptop)

Autonomous session of 2026-11-15 (night) on the field laptop (i7-5600U,
2 cores / 4 threads, Intel HD Graphics 5500, 60.01 Hz panel). User: "work on
these performance jobs on the field laptop, in this order, autonomously:
1. D23V GPU floor ... 2. D23N2B, then D23N3, then D23N4 ... 3. D23Y7 ...
4. D23AA ... Don't stop to ask; make the sensible call, write down why".
The user was away for the whole session.

Method throughout: the D23Y6 lab `recomp/analysis/d23y6/` (local, untracked),
the player's profile copied read-only, the mode set in the lab's profile copy
(`CTRL` / `RATE`, never `LAB_` switches for profile settings; `LAB_` only
for developer switches such as `PSX_GL_DEV_SKIP`), each run's settings line
checked in its log, private card copies only; interleaved and rotated A/B,
medians (and lowest tenth for game frames and presents), GPU busy (RC6) and
clock, CPU clock per run. Offscreen for profiles and GPU breakdowns; the real
display only while the user was idle (`idle.py`, `abc.sh` waits otherwise).
Evidence labels: MEASURED, CODE, INFERRED.

Host checked first (AGENTS.md): mains power, power profile performance,
schedutil; no backup, update or indexing activity (timers next at 07:27 and
later), load average under 0.5 idle, 6.3 GB memory available, 0.7 GB swap
used. The idle desktop alone keeps the GPU out of RC6 7.5-8.2% of the time
at 300 MHz (four 5 s samples, nothing of ours running): that is part of
every "busy" reading below. The build on the laptop predated the last two
commits (D50G, D17BH, made on the other machine), so the session started
with a full rebuild (55 min).

## 1. D23V: what the floor is on the HD 5500 (MEASURED)

Reading Intel's busy. On this GPU "busy" (time out of RC6) hardly moves
between scenes or bounds (75-82%); the clock does (380-707 MHz). RC6 is
entered within tens of microseconds of idle, so busy is real time the
render engine has work, at whatever clock the governor picked. The work
measure used here is busy x clock ("work", MHz-weighted busy). The GPU
timeline of a redraw (`PSX_GPU_PHASES=2`, `[gpu kinds]`) includes bubbles
where the GPU waits for submissions: with every textured run skipped the
"tex" kind still shows 2.5-3.1 ms a redraw, which is the render thread's
submission, not drawing.

Breakdown with developer bounds (`PSX_GL_DEV_SKIP`, wrong pictures, cost
attribution only; new bits 128-1024 added this session), offscreen, one
render thread, turning, 180 Hz, 2 interleaved runs each (`d23vb.sh`):

| Skipped | UI 2 work | UI 8 work | CPU clock (UI 2 / UI 8) |
| --- | --- | --- | --- |
| nothing (normal) | 491 | 576 | 2225 / 2012 MHz |
| textured runs (bits 1-16) | 343 (-30%) | 332 (-42%) | 2594 / 2594 |
| snapshot capture copies (128) | 423 (-14%) | 430 (-25%) | 2494 / 2544 |
| band copies of redraws (32) | 495 (0) | 542 (-6%) | 2336 / 2095 |
| fills and wide clears in redraws (256) | 486 (0) | 558 (-3%) | 2219 / 2036 |
| flat batches (512) | 504 (0) | 554 (-4%) | 2237 / 1995 |
| the presents' picture quad (1024) | 489 (0) | 562 (-2%) | 2295 / 2096 |

Run-to-run spread of "work" is about +-15-25. The first d23v.sh rep (base
against no textured runs against no textured runs, band copies or depth
clears, 180 and 120) agreed: UI 2 502 / 324 / 286, UI 8 569 / 312 / 324.

So, past the textured scene, the floor here is the snapshot capture: the
emulation thread copies the whole 2x canonical surface (2048 x 1024), the
raw VRAM mirror and every wide surface into a snapshot for the render
threads once a game frame, 2.2-2.5 ms of GPU timeline a frame (60 a
second). Removing it raises the CPU clock 270-530 MHz (the 15 W package,
note 463). Band copies, clears and fills, flat batches and the presents
are each within the spread. (On the 1080 Ti in note 459 the floor looked
different; that machine is not power-bound.)

## 2. D23V built: two exact cuts (CODE, MEASURED)

Framework (`gpu_gl_renderer.c`, exported):

- **Direct band** (`rp_band_direct`): a redraw's wide band went into its
  cache image in two copies: the canonical centre into the wide surface
  (`wide_blit_center`, after saving those rows for the restore), then the
  whole wide band into the image. Now the two margins come from the wide
  surface and the centre straight from the canonical surface: the same
  texels (nearest, 1:1), one large copy fewer, nothing saved or put back.
  Redraw sessions only; the live surfaces keep their centre. Developer
  `PSX_REPLAY_BAND_DIRECT=0` restores the two copies.
- **Margins-only capture**: with the fast wide path every redraw takes its
  band's centre from the canonical surface (either path above), and nothing
  else a redraw does reads a wide surface's centre, so the capture copies
  only the two margin columns of each wide surface. Developer
  `PSX_REPLAY_CAPTURE_MARGINS=0` copies them whole.
- Developer bounds 128, 256, 512 and 1024 of `PSX_GL_DEV_SKIP` (above).

Pictures: the render-thread validator (`PSX_RT_VALIDATE=1`, every 8th
render-thread image against the same redraw drawn on the emulation thread,
RGBA exact) at 180 Hz walking, turning and firing: UI 2 174 of 174, UI 8 107
of 107, UI 9 295 of 295 exact (the old path in matched runs 153/153,
151/151, 295/295). Because the direct band runs on both sides of the
validator, matched shots too (UI 8, 180 Hz, same savestate, shots at frames
60, 200, 400): new against old 0.00%, 0.00%, 0.11% of pixels; two runs of
the same build 0.67-0.95% / 0.00-0.11% (the late camera's real-time
spread). Vanilla has no redraws or render threads: neither path runs there.

Before and after, offscreen, interleaved (2 clean runs each; a third was
spoiled by an overlapping run on the same debug port and discarded):

| | capture a frame | band a redraw | work | CPU clock | emulation thread | presents |
| --- | --- | --- | --- | --- | --- | --- |
| UI 2, 180 | 2.33 -> 1.70 ms | 0.66 -> 0.36 | 479 -> 505 | 2258 -> 2295 | 75 -> 70% | 180 -> 180 |
| UI 8, 180 | 2.28 -> 1.68 | 0.65 -> 0.37 | 551 -> 538 | 1995 -> 2095 | 90 -> 88% | 180 -> 180 |
| UI 2, 120 | 2.26 -> 1.66 | 0.65 -> 0.37 | 465 -> 490 | 2245 -> 2245 | 69 -> 68% | 113 -> 120 |
| UI 8, 120 | 2.26 -> 1.65 | 0.64 -> 0.36 | 539 -> 491 | 2095 -> 2080 | 86 -> 82% | 119 -> 120 |

The two pieces fell 27% and 45%; whole-GPU work and the CPU clock moved
within the spread (UI 2 up, UI 8 down). The capture's remaining 1.65-1.70
ms is the canonical surface (8 MB of colour a frame at 2x) and the raw
mirror. Note 469 tried an exact incremental capture (only the tiles written
since a snapshot slot was filled; about half the canonical tiles, because
the game's two frame bands are rewritten every frame) and an overlay image
cache, with no measured gain; they were not repeated. What is left of the
floor is a follow-up, D23V2 (section 7).

## 3. D23N2B piece 6: the other actor joint loop (MEASURED, CODE)

Note 471's piece 4d left `0x80033584` 82% exact from the job start. It is
the transform call of a second joint loop (in `0x80033160`), like Duke's:
it passes `*(s3 +0x3c) + s2` (a joint's entry in the actor's matrix table)
or, on the scaled path, a stack copy of it through ScaleMatrix. New
developer counts in `DNTTK_N2_CHECK` bit 64 (`redraw_n2.inc`, namespace
`jr`): the entry the job passes, the job's own stores into it before the
call (from the D23N1 record) and the PC of the last one. Offscreen at 180,
walking, turning and firing:

| View | calls | exact from the start | job wrote the entry first | last store PC of differing calls |
| --- | --- | --- | --- | --- |
| UI 8 | 12,598 | 9,978 (79%) | 12,598 | `0x8009876c` 2,509, `0x8009837c` 167 |
| UI 2 | 1,479 | 1,445 | 1,479 | `0x8009876c` 32, `0x8009837c` 2 |
| UI 9 | (none in the walk) | | | |

Disassembly (owned EXE, local scratch): `0x80098204 (actor, out)` builds the
actor's joint table every job: per joint the rotation through `0x80010d60`
from the animation table, the translation as the joint's offset plus the
actor's position (`actor +4`, `+8`, `+0xc`; `0x8009876c` and `0x8009837c`
store each joint's last translation word, `+0x1c`, so they are simply the
last store into the entry, in the joint loop and for the first joint). In
the first four differing calls printed, only the entry's `+0x18`
translation word differs, by 1 unit (`0x15ea` against `0x15e9`). INFERRED: the actor's position
differs between the jobs of a frame (the per-fraction pose patches), so the
job-start memory holds the earlier job's table. A rebuilt later job gets
these exact by running `0x80098204` (or a host port of its translation
part) after its own pose patches; not yet checked. Next (resumable): that
check, the remaining 2-7% of `0x80031dec` / `0x80032f48`, the hook-list
differences of piece 4c, then note 456 section 6 (object culling,
camera-dependent hooks, sprites and particles, a whole later job). D23N3
and D23N4 depend on it and were not reached.

## 4. D23Y7 re-measured on top of D23V (MEASURED)

The real display, the player's settings (2x, FRAME RATE 120, LIGHTING on,
SIMPLE edges, extended draw distance; each run's settings line checked),
3 interleaved runs each, 30 s walks (`abc.sh`, case `cad`), new build
against the same build with `PSX_REPLAY_CAPTURE_MARGINS=0
PSX_REPLAY_BAND_DIRECT=0` (developer switches only):

| Scene | | game median / lowest tenth | presents median / lowest tenth (run medians) | screen fresh | rate changes a run | GPU busy @ clock | CPU clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| DUKE HILL town (private 4) | old | 59.9 / 56.4 | 31 / 30 (60 30 30) | 69.0% | 4.7 | 73% @ 604 | 2699 |
| | new | 59.9 / 57.4 | 31 / 30 (39 31 30) | 66.4% | 4.7 | 69% @ 604 | 2694 |
| UI slot 8 | old | 59.5 / 55.9 | 60 / 30 (59 59 60) | 85.4% | 2.0 | 74% @ 621 | 2195 |
| | new | 59.4 / 56.5 | 60 / 33 (60 60 60) | 91.5% | 1.3 | 77% @ 661 | 2095 |
| UI slot 9 | old | 60.0 / 59.1 | 59 / 30 (59 60 30) | 75.2% | 0.3 | 71% @ 628 | 3095 |
| | new | 60.0 / 59.1 | 60 / 59 (60 60 60) | 93.0% | 0.7 | 75% @ 610 | 3093 |

UI 8 and UI 9 hold 60 pictures more of the time (screen fresh +6 and +18
points), within what three runs can separate (note 474 had UI 9 at run
medians 60 30 60 before). The town still steps to 30. Offscreen at 120 in
the town (one run each): the redraw worker 98% of a CPU in both, the
emulation thread 78% new against 88% old, render thread 39%. The reason is
unchanged from notes 469-470: at 2x the town needs more CPU than the 15 W
package leaves (the worker re-running the game's drawing code for every
redraw is a whole logical CPU), so the cure remains captured redraws
(D23N2B-N4) or a lighter emulation thread (D23Y8). Blocked, re-measured.

## 5. D23AA: the FAMILY JEWELS start (MEASURED)

Host state first (AGENTS.md). The user's report came from the long
playthrough of 2026-11-14, which was played on the other machine: this
laptop's cards and savestates are unchanged since 11-13 and its session
logs from that day rotated out (the newest 20 start at 16:21). So that
session's host conditions cannot be checked from here. On this laptop
during the runs: idle, nothing scheduled (section intro).

A fresh FAMILY JEWELS start (console `level 6` from a private copy, saved
into private slot 11 of the lab's card copy only; `mkhill` with `LEVEL=6`).
The drawbridge is the raised wooden gate straight ahead across the moat.
Shooting it does nothing (a flying enemy attacks); how the bridge is
lowered was not found in this session.

Real display, the player's settings at 120, 3 interleaved runs against
DUKE HILL's start (private slot 10), 30 s walks:

| | game median / lowest tenth | presents (run medians) | screen fresh | late repeats a run | redraw job (worker) |
| --- | --- | --- | --- | --- | --- |
| FAMILY JEWELS start | 60.0 / 58.9 | 60 30 59 | 79.6% | 225, 117, 60 | 13.6-15.1 ms |
| DUKE HILL start | 59.9 / 58.7 | 60 60 60 | 88.6% | 5, 3, 9 | 9.0-9.5 ms |

The repeats (a redraw not ready, the previous picture shown again) come in
the first ~14 s of each walk, while Duke is at the start, then stop. That
matches the report. Standing at the start facing the castle (offscreen,
120): a redraw job costs 30.5-36.8 ms on the worker (three slots of 8.3 ms
at 120), presents fall to 40-46 a second. The same spot with DRAW DISTANCE
ORIGINAL (the lab's profile copy): 21.9 ms.

What the view holds (the main process's near-clip counts a frame, 5 s each,
offscreen):

| View | object-path polygons | packet demand | world / props and water / actors / sky (bytes, run average) | water meshes |
| --- | --- | --- | --- | --- |
| FJ start, facing the castle | 1,279 | 129 KB | 25,312 / 79,179 / 10,540 / 12,248 | 65.8 |
| FJ start, turned 90 / 180 / 270 degrees | 513 / 272 / 575 | 80 / 54 / 49 KB | | 0 |
| DUKE HILL start | 377-583 | 36-45 KB | 861 / 21,989 / 0 / 19,941 | 0 |
| DUKE HILL town | 377-638 | 42-56 KB | | 0 |

So the start looks over the moat (66 water meshes) at the castle's walls
and through its gate into rooms 18,000-28,000 units away, with enemies in
view: about three times DUKE HILL's geometry, all of which each redraw
rebuilds in the worker (the profile's extra time is the Polymorph host
geometry: `draw`, `emit`, `world` and recording host vertices, 4.2 ms of
title code a job against 2.1 ms at DUKE HILL). The drawbridge object itself
is not the cost: moved out of view (a diagnostic write in the private copy)
the polygon count is 1,269, because the castle interior shows through the
gate instead. INFERRED: the stutter "goes away once the drawbridge is
lowered" because by then Duke has left the start's long view, not because
of the bridge.

Tried: the recorder's host-vertex table laid out as one slot a cache line
(`render_replay.c`), two binaries swapped between interleaved runs: 30.5 /
30.6 / 36.8 ms against 30.5 / 35.1 / 36.8 ms, no gain; reverted. No exact
cut found; the cure is the same as D23Y7's (captured redraws, D23N4: later
jobs would reuse the first job's geometry instead of rebuilding it).
Blocked, measured.

## 6. Pictures, Vanilla, cleanup

Pictures as section 2. Vanilla launched (`--vanilla`, offscreen, private
copy): "Profile: Vanilla (this launch only)", no render threads, so neither
D23V path runs. Framework patch exported and checked: applied to the pinned
framework commit it reproduces the working tree's runtime and recompiler
exactly. All lab runs used private card copies; the lab's player-file check
passed every run. The lab extensions (`d23vb.sh`, `vsum.py`, cases `seq`,
`polys`, `objs`, `mkhill LEVEL`) are local and untracked.

## 7. Follow-ups

- D23V2 (new, Todo): the rest of the capture, the 2x canonical surface
  (1.65-1.70 ms of GPU a game frame here): copy only what a redraw can read
  (its band, packs, copy sources, sampled pages) or the tiles written since
  that snapshot slot was filled together with the margins-only wide copy;
  note 469's incremental capture alone showed no game-frame gain, so measure
  GPU work and the CPU clock, not only game frames.
- D23N2B: section 3's next step.
- D23Y7 and D23AA: wait on D23N4.

## 8. Reproduce

`recomp/analysis/d23y6/` (local): `d23v.sh` (base / no textured runs / no
band or depth), `CFGS="base c31 c32 c128 c256 c512 c1024" ./d23vb.sh`
(bounds), `OLDENV="LAB_PSX_REPLAY_CAPTURE_MARGINS=0
LAB_PSX_REPLAY_BAND_DIRECT=0" CFGS="new old" RATES="180 120" ./d23vb.sh`
(A/B), then `vsum.py PRE cfgs "rates" "slots"`; validator
`LAB_PSX_RT_VALIDATE=1 CASES=perf2`; D23Y7 `abc.sh 3 "4 7 8" y7new ""
y7old "<OLDENV>"` and `comb3.py`; D23N2B `RATE=180 LAB_DNTTK_N2_CHECK=80
CASES=perf2`; D23AA `LEVEL=6 DEST=11 SLOT=10 MINWAIT=6 SETTLE=300
CASES=mkhill`, `abc.sh 3 "11 10" aa ""`, `CASES=polys`, `CASES=objs`.
