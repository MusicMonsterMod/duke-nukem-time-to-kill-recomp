# 479 - D23V2 incremental capture measured on the field laptop

Autonomous session of 2026-11-15 (evening) on the field laptop (i7-5600U,
2 cores / 4 threads, Intel HD Graphics 5500). User: "work on D23V2
autonomously: measure the incremental snapshot capture against the whole
copy, PSX_REPLAY_CAPTURE_INC=0 against the default, in the field laptop lab
recomp/analysis/d23y6. Run at 120 and 180, in UI slots 2, 8 and 9,
offscreen, interleaved A/B, at least 2 rounds ... Prove it exact here too
... Acceptance (note 478 section 3): capture GPU time at least halved
again."

Evidence labels: MEASURED, CODE, INFERRED.

## 1. Setup

- Pulled to `bd8e8ee`, submodules at their pins; the framework tree held the
  previous accepted patch exactly (it reversed cleanly), so it was reversed
  and the new patch applied (`apply_runtime_patches.py`), then
  `cmake --build --preset local-dev` (55 min, generated code recompiled).
  The binary carries `PSX_REPLAY_CAPTURE_INC` and `PSX_REPLAY_CAPTURE_CHECK`.
  The pre-D23V2 binary (built 03:44 the same day, without those switches)
  was kept in the lab for the Vanilla shots.
- Host checked first: mains power, schedutil, no backup tool configured or
  running, last package update 3 days earlier, no timers due during the
  session; load came only from our build, then under 1 idle; 6.5 GB memory
  available, 0.8 GB swap in use.
- Lab `recomp/analysis/d23y6/` (local, untracked): the player's profile copied,
  the frame rate in the lab's profile copy (`RATE`), `LAB_` only for
  developer switches, private card copies (player files hash-checked
  unchanged after every run), offscreen (SDL offscreen on the real GPU), case
  `perf2` turning (`SPIN='dx=6'`) for 20 s, `PSX_GPU_PHASES=2`. Every run's
  settings line read `Internal resolution: 2x` and `Frame rate: 120` or
  `180`. New: `d23v2.sh` (A/B, interleaved and rotated), `d23v2x.sh`
  (readback check and validator), `d23v2v.sh` (Vanilla matched shots, case
  `vshot`), `v2sum.py`; `perf2` now also samples presents and game frames
  each second and keeps the swap cadence.

## 2. Cost (MEASURED)

Five rounds (three on the first binary, two on the fixed one from section
3), both configurations in each, 60 runs. Medians over runs; presents and
game frames are the median / lowest tenth of every second of every run.
Capture is GPU time a game frame from the `[gpu copies]` line; work is GPU
busy (RC6) x clock in MHz.

| Rate, view | capture full -> inc (ms) | change | work full -> inc | CPU clock (MHz) | emulation thread | presents / lowest tenth | game / lowest tenth |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 180, UI 2 | 1.65 -> 1.08 | -34% | 498 -> 500 | 2379 -> 2395 | 68 -> 66% | 180/178 -> 180/178 | 60/59 both |
| 180, UI 8 | 1.62 -> 1.05 | -35% | 557 -> 562 | 2179 -> 2191 | 81 -> 79% | 180/178 both | 60/59 both |
| 180, UI 9 | 1.60 -> 1.07 | -33% | 505 -> 520 | 2474 -> 2468 | 63 -> 62% | 180/178 both | 60/59 both |
| 120, UI 2 | 1.60 -> 1.06 | -34% | 531 -> 510 | 2394 -> 2395 | 62 -> 61% | 120/119 both | 60/59 both |
| 120, UI 8 | 1.59 -> 1.04 | -34% | 566 -> 547 | 2195 -> 2195 | 76 -> 75% | 120/119 both | 60/59 both |
| 120, UI 9 | 1.59 -> 1.07 | -33% | 472 -> 442 | 2494 -> 2494 | 60 -> 59% | 120/119 -> 120/87 | 60/59 both |

- The capture fell by a third in every cell, steady across rounds (single
  runs 1.03-1.09 ms incremental but the run of section 3, 1.57-1.67 whole), with 245 of 512 canonical
  tiles copied a capture (both of the game's frame bands are rewritten every
  frame), as on the desktop. **Acceptance (at least halved) is not met.**
- Whole-GPU work moved -6% to +3%, inside the run-to-run spread (single runs
  of one cell range by up to 25%). The CPU clock did not move. The emulation
  thread was 1-2 points lower with the incremental capture in all six cells:
  small, but in the same direction every time.
- Swap cadence (`render_replay`, medians of runs): p50 5.55-5.57 ms at 180
  and 8.31-8.34 at 120 in both; p95 and p99 within 0.1-0.6 ms of each other,
  neither side consistently ahead.
- The 87 in the lowest tenth for 120 UI 9 incremental is one run whose first
  4 s ran at 60 presents: the emulation fell behind (56.7 frames/s, one audio
  underrun) and the pacer presented every second refresh, then recovered.
  That backoff appears in 8 of the 60 runs, 3 whole and 5 incremental, so it
  is not this change.

Where the remaining 1.05 ms goes (INFERRED from the table, not separately
measured): with full = C + X and incremental = 0.48 C + X (245/512 tiles),
the canonical surface C is about 1.05-1.08 ms and the fixed rest X (the raw
VRAM mirror and D23V's wide margins) about 0.53-0.57 ms, which no tile
selection removes. Halving the capture (to about 0.80 ms) would need the
canonical copy at 0.25 ms or less, about 115 tiles. So the frame bands
themselves must not be copied: copy only what a redraw can read (the job's
other option), or reduce the mirror and margins.

## 3. One run stayed on the whole copy; fixed (MEASURED, CODE)

In 1 of the 18 incremental runs of the first binary (180, UI 9, round 3)
every capture was whole: 0 incremental, 150 whole each window, capture 1.63
ms. CODE: the incremental path needs `s_rr_ci_checked`, the flag that the
first `glCopyImageSubData` was checked for a GL error, and only
`rr_rect_blit` (the emulation thread's copy-on-write blits) sets it.
INFERRED: in a session whose redraws never reach that blit, the capture
never goes incremental.

Fix (framework, `gl_rt_capture`, exported): when `glCopyImageSubData` is on
and not yet checked, the capture copies one tile of the live surface into
its slot and runs the same check; the copy that follows rewrites or keeps
that tile from the same live surface, so the slot is the same either way, and
a driver that refuses the copy still falls back to blits. After it, all 12
incremental runs went incremental (148-150 incremental, 0 whole a window).
At 1 in 18 before, 12 clean runs are weak evidence of the fix on their own;
the code path is the evidence.

## 4. Pictures (MEASURED)

- Readback check (`PSX_REPLAY_CAPTURE_CHECK=10`, 30 s, UI 2, 8 and 9 at 180
  and 120): first binary 624 captures compared, 0 differing tiles; fixed
  binary 626, 0.
- Render-thread validator (`PSX_RT_VALIDATE=1`, 180, 40 s): incremental 963
  of 963 exact, whole copy 952 of 952 (first binary); fixed binary 962 of
  962 and 938 of 938. Unlike the desktop's two differing images, none
  differed here.
- Vanilla (no redraws, so no capture): UI 8 and UI 2 savestates, no input,
  shots at frames 60, 200 and 400, pre-D23V2 binary against this one, two
  runs each: all four shots byte-identical at every frame (the frames differ
  from each other, so the scene was live).

## 5. Status and next

The incremental capture is exact, keeps whole-GPU work, the CPU clock and
cadence where they were, and takes a third off the capture. The user closed
D23V2 on that ("it runs fine" ... "yep close it out"), accepting a third
instead of half. The rest is D23V3 (Todo, low priority): leave the frame bands out of the
capture where no redraw reads their old contents, or cut the mirror and
margins (about 0.55 ms that tile selection cannot reach).

## 6. Reproduce

From `recomp/analysis/d23y6/` (local): `REPS=3 PRE=v2 ./d23v2.sh`, then
`python3 v2sum.py v2,v2f`; exactness `XP=x2f ./d23v2x.sh` (the `[rt]` lines
and the `checked` counts in `[gpu copies]`); Vanilla `./d23v2v.sh`, then
`imgdiff.py` on the `x2v-*` shots.
