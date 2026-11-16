# 484 - D23N4b on the field laptop: not run (its condition not met)

Autonomous session of 2026-11-16, part 2 of the chain. User: "Only if N4b
is exact and cheaper: field laptop, update its checkout and build,
CPU/power/clock/cadence at 120/180/240 on vs off in the same views
(interleaved runs, at least 2 each), then D23Y7 and D23AA re-measured with
N4b on, offscreen and once on the real display. Leave the default-on
decision for me."

## Decision (MEASURED on the desktop, note 483)

The condition is not met on either count:

- **Not exact.** After this session's three fixes, verify-first at 120 s:
  UI 2 1,070 of 1,072 rebuilds identical, UI 5 1,411 of 1,424 (its two
  host draws off; with them on even a full job is not repeatable, note 483
  section 5); the fork proof leaves 2 wrong jobs unflagged in UI 2 and 1 in
  UI 5 (UI 8, UI 9 and the FAMILY JEWELS start 0).
- **Not cheaper.** One worker, interleaved, two runs each: the game's CPU
  is 6-16% higher with N4b (UI 2 113.5 -> 122.0%, UI 5 111.3 -> 119.3%,
  FAMILY JEWELS start 120.4 -> 138.7%, means of two). Note 482's laptop
  figures were taken while the worker was being killed at its deadline
  (note 483 section 7) and do not stand.

So the laptop's checkout was not updated for N4b, nothing was measured with
N4b on, and D23Y7 and D23AA were not re-measured: with N4b off by default
their notes (476) stand. The laptop was used for parts 3 and 4 only (notes
485, 486), on its existing `6e887c5` build.

## What would make part 2 worth running

N4b needs both: no unflagged wrong job and no inexact accepted rebuild in
the five views, and a first-job capture cheap enough that the rebuilt later
jobs save more than it costs (note 483 section 13). At one worker the
laptop's placement already matches N4b's (one redraw worker), so once
both hold on the desktop the same lab (`recomp/analysis/d23n4/`, case
`wcpu` with RAPL power and the mean clock) answers part 2 directly.
