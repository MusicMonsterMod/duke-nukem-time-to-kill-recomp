# 487 - D23N4b's cost by part, the early stop, and the decision (desktop)

Autonomous overnight session of 2026-11-16 (desktop lane). User: "D23N4
cost: measure each part of N4b's first-job capture (store log, hook CPU
states, decision records and box tests, hit-test scratch runs, fallback
image save) with case wcpu at one worker, interleaved, 2+ runs each, UI 2,
5 and the FAMILY JEWELS start. Try the leanest exact cuts ... and
re-measure. Write the decision: can N4b pay, and at how many workers."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Setup

Desktop (i7-5960X, GTX 1080 Ti), the session worktree at `5226b66`, lab
`recomp/analysis/d23n2b/` (local), `Frame rate: 180`, offscreen, one redraw
worker (`DNTTK_REPLAY_WORKERS=1`), N4b lean mask `DNTTK_N2_CHECK=201326344`
with `DNTTK_N2_WHOLE="1 1"` (note 483). Views: UI 2, UI 5 and the FAMILY
JEWELS start (runtime slots 1, 4, 11), case `wcpu` (load, settle, 40 s of
walking, turning and firing). Host idle before every run (load 0.2-0.5, no
backup, update or rsync process; the next update timer three hours away).

## 2. The tool (developer)

`DNTTK_N4_COST=1` times each worker job at its outermost `worker_draw`
with the TSC (calibrated against the SDL clock) and prints a `[n4 cost]`
line every 256 jobs: jobs and mean ms by class (recorded first job,
rebuilt later job, fallen-back later job, not recorded), and inside them
the hook observer (all of it, its decisions part, the decisions' scratch
runs), the record's copies at the flip, stores and hooks a recorded job,
the fallback image save, the rebuild attempt (accepted, failed), a
fallback's image reload and its full job. Its own cost is within the
run-to-run spread (whole-game CPU with and without it, section 4).

`n4cost.sh` runs the configurations interleaved (`plainc`, `n4bc` with the
timers; `plain`, `n4b` without), `n4cost_sum.py PRE` summarizes.

## 3. Where a job's time goes (MEASURED, before the cut)

Mean ms a worker job, two runs each (`k-*`):

| View | plain job | recorded first job | rebuilt later job | fallen-back later job |
| --- | --- | --- | --- | --- |
| UI 2 | 10.29, 10.06 | 11.02, 11.12 | 8.26, 7.97 | 19.11, 19.87 |
| UI 5 | 7.72, 7.59 | 8.36, 8.25 | 5.98, 5.97 | 14.53, 14.65 |
| FAMILY JEWELS | 10.31, 9.47 | 10.67, 11.07 | 9.36, 9.86 | 15.58, 16.44 |

The capture (a recorded job less a plain one: 0.4-0.9 ms) by part, UI 2
(UI 5, FAMILY JEWELS):

| Part | ms a recorded job |
| --- | --- |
| hook observer, all | 0.32 (0.17, 0.29) |
| of it: the culling decisions and hit tests | 0.23 (0.13, 0.23) |
| of those: the scratch runs (box tests, the hit test) | 0.16 (0.05, 0.14) |
| the record's copies at the flip (stores, hooks, stream) | 0.19 (0.11, 0.14) |
| the store log through the write wrappers (residual) | about 0.2-0.4 (INFERRED) |
| stores / top-level hooks a job | 25,300 / 1,140 (15,600 / 575; 18,900 / 866) |

And per later job: the fallback image save 0.45 ms; a rebuild attempt
6.9 ms accepted, 7.2 ms failed (UI 5: 4.8, 5.2; FAMILY JEWELS 8.0, 5.9); a
fallback then reloads the image (0.5 ms) and runs the full job (10.75 ms,
itself recorded).

So the capture is not what makes N4b dear. Two things are:

1. **A rebuild saves only 2-2.5 ms of a 10 ms job** (UI 2 8.1 against
   10.2; FAMILY JEWELS less). It skips the game's own code, but every
   host hook still runs for real: our world and model draws, the D36 joint
   callbacks, the near clip. They are most of a job, and they depend on the
   later job's camera and poses, so they cannot be skipped (CODE: wj::body).
2. **Half the later jobs fall back** (UI 2 48-52% rebuilt, UI 5 67-75%,
   FAMILY JEWELS 41-53%): water tiles the later camera culls otherwise,
   small models, the sprite-like runs, shadows without a source link and
   the hit test (`[n2 check] D23N4b rebuilds ... fallbacks by class`).
   These are real differences between the two cameras, not detector false
   alarms (note 483's proofs), and a fallback costs the attempt, the reload
   and a full job.

## 4. The cut: stop a flagged rebuild at its first flag (CODE, MEASURED)

In place a rebuild returns `!bail && !F`, and flags are only ever added
(`flag` ORs them into `F`), so a rebuild with any flag is never accepted.
It now returns at the first hook after a flag instead of running the rest
of the job (`redraw_n2.inc`, wj::body). Exact by construction: the same
jobs are accepted and fall back. Developer `DNTTK_N4_EARLY=0` runs a
flagged rebuild to the end, as before, so every class of flag is counted.

After it (`e-*`, two runs each):

| View | plain job | N4b mean job | failed attempt | whole-game CPU, plain / N4b (no timers) |
| --- | --- | --- | --- | --- |
| UI 2 | 10.12, 10.06 | +15.2%, +16.3% | 7.2 -> 3.9 ms | 112.4 / 119.2% |
| UI 5 | 7.61, 7.49 | +5.4%, +3.8% | 5.2 -> 2.8 ms | 110.9 / 114.6% |
| FAMILY JEWELS | 9.47, 9.42 | +7.2%, +17.6% | 6.1 -> 3.3 ms | 124.9 / 131.2% |

(whole-game CPU: the game's processes but the launcher: the emulation
process, about 77% of a core here, plus the worker, as note 483 measured
it; the worker alone is 37-50%.) Before the cut the same means were
+19.7-21.8% (UI 2), +9.2-12.1% (UI 5), +19.3-23.8% (FAMILY JEWELS).

## 5. The other lean cuts, sized (MEASURED, INFERRED)

What a perfect N4b would cost, from the same counts: every fallback costing
only a recorded job (no attempt, no reload) and no image save:

| View | N4b ideal against plain |
| --- | --- |
| UI 2 | +0.5%, +0.9% (before the cut's runs), +0.7%, +2.7% (after) |
| UI 5 | -3.0% to -5.8% |
| FAMILY JEWELS | -4.4% to +9.3% |

The remaining cuts are smaller than that ceiling:

- Only the CPU fields the rebuild reads in each hook record, a compact store
  log and no flip copies (swap the record instead): together at most the
  observer's copy part, the flip copies and part of the store log, about
  0.3-0.5 ms a recorded job, 1.5-2.5% of all jobs (INFERRED; not built).
- Re-forking or reloading the slot image instead of the image save: 0.45 ms
  a later job, about 2% of all jobs; it needs a framework hook (the slot may
  be republished once its jobs have loaded it, `render_worker.c`); not
  built in this lane.
- Cached decisions: the decisions' scratch runs are 0.05-0.16 ms a recorded
  job (under 1%).

## 6. Decision

**N4b cannot pay, at any number of workers.** At one worker, its best case,
the measured mean job stays 4-18% dearer after the exact cut, and even a
perfect N4b (no fallback cost at all) is within -6% to +9% of plain,
because a rebuild still runs every host hook and about half the later jobs
cannot be rebuilt. With more workers the jobs of a frame spread over them,
so each worker records more first jobs and rebuilds fewer (INFERRED from
the rule `prev.serial == p.serial` per worker); sharing one capture between
workers would remove the extra recording but not either limit above.

The gap is not under 5% in UI 2 or the FAMILY JEWELS start, so step D2 of
this session (the open detector classes, a shared capture) was not
started. D23N4b stays a developer switch, off by default. The part of
D23N4 that does pay is outside N4b: less host work per redraw (the host
draws are most of a job, section 3), which is where D23R's 1.5 cores must
come from.

## 7. With every switch off (MEASURED)

The cut and the timers sit behind `DNTTK_N4_REBUILD=1` and
`DNTTK_N4_COST=1`; with them unset no code path changes (the early stop
runs only in an in-place rebuild, the timers read one static switch). The
session's desktop build (this note's code and note 489's), every developer
switch off:

- Vanilla (`--vanilla`, case `vshots`, `offcheck5.sh`): UI 8's and UI 2's
  savestates, shots at +60, +200 and +400 frames, the committed build
  (`5226b66`) against this one: **byte-identical, 6 of 6**.
- Render-thread validator (`PSX_RT_VALIDATE=1`, case `cad`, UI 2, 8, 9,
  40 s each): **914 of 915 images exact**; the one is the level-load frame
  right after the bake loads (UI 9), the same single level-load image notes
  481 and 483 recorded.

## 8. Reproduce

`bash n4cost.sh` (env `PRE`, `REPS`, `CFGS`, `SLOTS`), then
`python3 n4cost_sum.py PRE`; `grep wcpu PRE.sum` for the whole-game CPU.
