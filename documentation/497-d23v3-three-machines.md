# 497 - D23V3 on three machines: exact, lighter on the GPU, worse on citadel

Autonomous three-machine session of 2026-11-16. User: "C2. D23V3 on both:
PSX_REPLAY_CAPTURE_NEED=1 vs off, interleaved, 2+ runs; misses; whole-GPU
effect. If exact and not worse anywhere, propose default-on (leave the
decision to me)."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Setup

D23V3 (note 488): with `PSX_REPLAY_CAPTURE_NEED=1` the snapshot capture
copies only the canonical tiles redraws read; a redraw that would read a
skipped tile is drawn on the emulation thread instead (`[rt missed]`). The
lab `recomp/analysis/d23ab/` on each machine (note 496), offscreen, case
`cad` (30 s walks), the incremental capture of today (`inc`) against the
needed-tiles capture (`need`), interleaved and rotated, the host checked
before every run (all passed). "Work" is GPU busy times the shader clock,
the whole GPU.

## 2. Exactness and misses (MEASURED)

0 stale tiles loaded in every `need` run on every machine (about 100 runs).
Redraws drawn on the emulation thread because their capture skipped a
tile: 0-8 a 30 s run (UI 9 and the FAMILY JEWELS start most often), against
3,600-5,400 redraws a run.

## 3. Field laptop (MEASURED)

At the player's 4x (two runs a cell): the laptop is power-bound and the
redraws mostly off (note 494), so the capture hardly runs; the cells
follow whether redraws happened to run (UI 9 at 120 presented 40 redraws a
second with `need` and so did more GPU work); no conclusion there.

SIMPLE (2x), two runs a cell, pictures 120 or 180 in every second either
way:

| Rate | Scene | work, inc / need | change |
| --- | --- | --- | --- |
| 120 | UI 2 | 441 / 427 | -3% |
| | town | 436 / 429 | -2% |
| | UI 8 | 366 / 334 | -9% |
| | UI 9 | 351 / 339 | -3% |
| | FJ start | 418 / 425 | +2% |
| 180 | UI 2 | 455 / 450 | -1% |
| | town | 461 / 443 | -4% |
| | UI 8 | 337 / 328 | -3% |
| | UI 9 | 386 / 356 | -8% |
| | FJ start | 504 / 491 | -3% |

BALANCED (2x with lighting, the laptop's AUTO grade), 120: pictures 120 in
UI 2, 8 and the town either way, UI 9 115 / 120; the FAMILY JEWELS start
unsteady either way (inc 95, 94, 100, 120, 94; need 90, 100, 95, 88, 100
pictures a run); work lower in UI 8 and UI 9 (-5%, -10%), equal in UI 2
and the town.

## 4. Desktop (MEASURED)

AUTO (ULTRA, 4x), two runs a cell: pictures 120 and 180 in every scene
either way; work at 120 -12% (UI 2), -10% (the town), equal elsewhere
(+2% to -2%); at 180 within -8% to +3%.

## 5. Citadel (MEASURED)

AUTO (2x, 3 workers), 120 and 180, two runs a cell, then the doubtful
cells again: work lower in 9 of 10 cells (3-22%), but pictures:

- UI 2 at 120: `need` 71, 120, 120, 62, 120, 62, 62, 119, 119, 120
  pictures a run; `inc` 120 in every one of its runs that day but one (62
  in a D23N5 run without the restore). Each collapse starts with the
  presenter's "redraw-only off: workers behind" early in the walk, then
  pacing at every second refresh. So with `need` UI 2 fell back in 4 of 10
  runs, without it in 1 of about 20.
- UI 8 at 180: `need` 137, 104, 137, 141, 146; `inc` 147, 137, 145, 137,
  138: within citadel's spread.

INFERRED: on citadel three workers already crowd four cores (note 496,
D23AC); the redraws `need` draws on the emulation thread add work there
at the start of a walk, which tips the presenter into "workers behind"
more often.

## 6. Decision

Exact everywhere, lighter on the GPU on the field laptop at 2x and on
the desktop in places, but worse on citadel (UI 2 at 120 falls back to the
original presentation in 4 of 10 runs). It is not "not worse anywhere", so
default on is not proposed. D23V3 stays off; Blocked until citadel's
worker count (D23AC) is settled, then measured again there.

## 7. Reproduce

`xm.sh` with `CFGS="inc need" CFG_need=LAB_PSX_REPLAY_CAPTURE_NEED=1`
(`PRE` c2, c2s, c2b, c2bm on the field laptop; c2, c2m, c2m2 on citadel;
c2d on the desktop), `xmsum.py PRE`.
