# 463 - D23Y: 120 fps on 2-core hardware - research and plan

User, 2026-11-13: "we need to put a backlog job in, or series of backlog jobs
in now to get hardware of this level to work with our 120 fps. ancient aliens
can do it. eduke can do it. half life can do it. there's no reason why we
cant do it with enough deep research ... i want it feeling essentially perfect
on this computer".

This note is the research and the plan; its first child (D23Y1) is built.
Evidence labels: MEASURED (the field laptop, this session), CODE, INFERRED.

## 1. The machine's budget (MEASURED)

i7-5600U (2 cores, 4 threads, 4 MB L3), HD Graphics 5500, one package:
RAPL PL1 15 W sustained (28 s window), PL2 25 W. Temperatures stayed about
50 C idle; the limit is power, not heat. CPU and GPU share it: more work on
either lowers the clock of both.

| Our game, the user's settings, UI slot 2, walking | Game frames/s | Lowest tenth | All threads | CPU clock |
| --- | --- | --- | --- | --- |
| original 60 (no redraws) | 59-60 | 58-59 | 61-67% | about 3.1 GHz |
| redraws at 60 (late camera) | 54-58 | 36-48 | 121-138% | 1.2-2.3 GHz |
| redraws at 120 | 43-51 | 26-33 | 162-214% | 1.1-2.2 GHz |

With the governor off (`DNTTK_REPLAY_SHED=0`), turning in UI slot 2:

| Redraws | Pictures/s | Workers (3) | Emulation thread | Render thread | All | GPU busy |
| --- | --- | --- | --- | --- | --- | --- |
| at 60 | 54 | 162% | 79% | 24% | 270% | 80% |
| at 120 | 90 | 208% | 59% | 19% | 291% | 79% |

So a redraw costs the workers about 23-30 ms of CPU at these clocks (about
10 ms on the 1080 Ti desktop's i7-5960X, note 444). The emulation thread's
own profile is almost unchanged with redraws on (generated code 12-14%,
memory 9%, our title code 11%, replay bookkeeping 0.1%), yet it needs 38%
more CPU time for the same game work at the same clock: its physical core is
shared with a worker (the CPU's two threads per core) and the 4 MB L3 with
all of them (INFERRED from the unchanged shares; no hardware counters
without root).

The reference programs on the same laptop (MEASURED, 20 s turning):

| Program | Pictures/s | CPU | GPU busy |
| --- | --- | --- | --- |
| EDuke32 Polymost 1600x900, `r_maxfps 120` | 120 | one thread 97% (mostly its frame limiter waiting, note 444), 105% all | 73% |
| UZDoom + Ancient Aliens, the user's settings, 60 cap | 60 | main thread 29%, 35% all | 82% |
| UZDoom, the same at a 120 cap | about 70 (GPU-bound) | main thread 29%, 41% all | 92% |

They spend one thread on a whole frame. UZDoom with the user's 1080p settings
does not reach 120 here either (the GPU is full); EDuke32 does. Half-Life was
not measured (not installed).

## 2. Why the redraw path is expensive here (CODE, MEASURED)

D17's redraws re-run the game's own frame tail in forked worker processes for
every in-between picture (`frame_replay.cpp`, `render_worker.c`): emulated
MIPS code, our host renderer (`near_clip.cpp`), the GTE, guest memory
tracking. On 2 cores that is the dominant cost three ways at once: CPU time,
contention with the emulation thread on its core and cache, and package
power that lowers every clock. D23N's captured redraws (notes 449, 454-456)
remove the game code from later jobs; that is necessary but not enough
alone: the host re-projection still costs about a third of a job (note 449),
about 8-10 ms here, so 120 pictures a second would still need about a core.

The GPU is the second wall: at the user's 3x Relit the original 60 already
uses 64-69% of the HD 5500 (note 464), so 120 full redraws at 3x cannot fit
whatever the CPU does. EDuke32 reaches 120 by drawing far less per picture
(note 444: 0.2 ms a frame on the 1080 Ti against our 1.0-1.4).

## 3. What gives 120 on this class of machine (INFERRED, ranked)

1. **Late warp: reproject the last picture on the GPU for in-between
   presents.** Each in-between present re-draws the newest finished scene
   image with the newest camera look, using its depth (we draw the 3D scene
   with host depth: D17AM), as VR's reprojection does; the HUD, first-person
   weapon and screen overlays are drawn on top unwarped. Cost: one
   full-screen pass (well under 1 ms of GPU at 1080p here, INFERRED from the
   blit timings of note 461: 0.55 ms for 2-3 full-screen copies) and no CPU
   per picture. The camera then turns at the display rate with the newest
   input (the latency fix of note 462 as well); the world's own motion stays
   at the game's rate. Risks: edges uncovered by the turn (the widescreen
   draw already renders margins; a depth-aware fill covers the rest), moving
   objects judder at 30 updates a second, transparent layers without depth.
2. **Captured redraws (D23N2B, D23N3, D23N4)** for full in-between pictures
   with moving objects interpolated, at a rate the CPU allows (60 redraws a
   second plus warps to 120 here; full rate on 4+ cores).
3. **Game speed first in the governor (D23Y1, built):** never keep redraws
   that slow the game itself.
4. **Placement on 2 cores:** keep the emulation thread's physical core free
   of workers when redraws run (affinity), one worker on the other core;
   measure against the default.
5. **A lighter emulation thread:** 55-70% of a core at 3.1 GHz for the
   original 60 here; every point saved is headroom on 2 cores (and is D23Z's
   one-off spikes too).

## 4. Built: D23Y1, game speed first (CODE, MEASURED)

The D23H governor stepped presents down to every 3rd refresh, then paused
the CPU overclock, but kept the redraw machinery running, so at a fixed 120
the game itself ran at 43-56 frames a second. Now, still behind at every 3rd
refresh for 3 judged seconds, the redraws stop and the original presentation
runs (`frame_replay.cpp`, `replay_load_window`); the FRAME RATE setting is
unchanged. It retries the redraws after 120 s of gameplay, doubling the wait
(2, 4, 8 minutes) when a retry falls back within a minute; a retry that falls
behind in its first 20 s goes back at its first slow second. A FRAME RATE
choice in OPTIONS cancels a pending retry. Log lines: `[TTK pace] the original
presentation (...)` and `[TTK pace] redraws again`. Developer:
`DNTTK_REPLAY_FLOOR=0` (never), `DNTTK_REPLAY_FLOOR_WAIT=seconds`.

Measured, the user's settings at a fixed 120, 45 s walks: the fallback
engaged after 10-11 s and the game returned to 59-60 frames a second (UI
slot 2: 28-47 before, 58-60 after; UI slot 9: 22-52 before, 58-60 after); a
retry cost one second at 24 frames and returned.

## 5. The plan (child jobs)

| Job | What | Measured target on the field laptop |
| --- | --- | --- |
| D23Y1 | Game speed first: the governor's last step is the original presentation (built) | no second under 57 game frames after the fallback |
| D23Y2 | Late warp research spike: reproject the newest scene image with the newest look on the GPU for in-between presents; HUD and weapon unwarped | 120 pictures a second at 2x with CPU as the original 60 (all threads under 75%), GPU under 80%, input to picture under 50 ms |
| D23Y3 | Late warp, the player build: edges, transparent layers, moving objects, OPTIONS (FRAME RATE above 60 on 2 cores uses it), Vanilla unchanged | the user's judgement at 120; game 60 throughout |
| D23Y4 | Placement on 2 cores: the emulation thread's core kept free of workers, one worker, A/B over five runs each | redraws at 60 with game frames 60, lowest tenth 57 or more |
| D23Y5 | A lighter emulation thread (with D23Z): profile the original 60 and cut its largest shares | emulation thread under 50% at 3.1 GHz in UI slots 2, 8 and 9 |
| D23N2B-N4 | Captured redraws (existing jobs), with the laptop as a measured target | a later job's CPU under 5 ms here |

Order: D23Y2 first (it decides whether the warp can carry 120 on this GPU),
D23Y4 in parallel (small), then D23Y3; D23N continues for 4+ core machines
and for moving objects. Acceptance for the series stays D23Y's: a steady 120
on the laptop's 60 Hz panel (and a 120 Hz display if available), UI slots 2,
8 and 9 with the user's settings, game speed never lowered, and the user's
judgement.

## 6. Reproduce

Labs (local): `recomp/analysis/d23w/lab.py` (`CASES=rates` walks,
`CASES=perf2` per-thread CPU), `recomp/analysis/d23x/walk.sh` and
`walk2.sh` (modes and the fallback), `recomp/analysis/d23x/sym.py`
(symbolizes `PSX_PROF` profiles). References: EDuke32 and UZDoom run from
copied configs in a scratch folder; restore the display mode afterwards.
