# 489 - A redraw job run inside a job kept its own clock (desktop)

Autonomous overnight session of 2026-11-16 (desktop lane). User: "the UI 5
non-repeatability (note 483 section 5): find the host state the
sleeping-enemy and object-reach draws keep (DNTTK_N4_SELF=1 bisects it) and
make a full job repeatable without changing its picture (matched shots,
PSX_RT_VALIDATE, Vanilla unchanged)."

Evidence labels: MEASURED, CODE (read in the source), INFERRED. Setup as
note 487 section 1 (desktop, 180, offscreen, one worker, N4b lean mask).

## 1. The finding

Note 483 section 5 found a full UI 5 job not repeatable in the same worker
(941 of 1,184 second runs identical with the sleeping enemies and object
reach on) and took it for host state those two draws keep.

**Tool (developer).** `DNTTK_N4_STATEDIFF=file` (the output of `nm -S -C
--defined-only` for the executable) with `DNTTK_N4_SELF=1` and
`DNTTK_N4_VERIFY_FIRST=N`: every data and bss object of up to 1 MB (10,221
objects, 46.9 MB; larger ones by 4 KB page hashes with
`DNTTK_N4_STATEDIFF_BIG=1`, trace rings left out) is taken at the job's
start and again before its second full run, and each object that changed
is counted against whether the two runs' streams were identical.

**Result.** UI 5, 60 s: 109 of 736 second runs differed, and the one object
changed in all 109 was `prop_glow_quads` (in 20 of the 627 identical ones):
the jobs that differ are those that draw D41L's prop glows (the stove's
embers, the chandelier's flames and halos), whose colours and sway follow
the draw clock (`prop_lights.cpp`, `now_ms`).

**Cause.** A worker job holds one draw clock and pins the lamp bake table
for its whole run (`worker_draw`'s `ClockHold`, D23N1). The verify-first
reference run, the self run and N4b's fallback are `worker_draw` called
inside a job; each made its own hold (another tick) and its release on
return cleared the outer job's hold, so the rest of the outer job drew from
the live clock. The object pass's host draws only made it visible: object
reach draws the out-of-room stoves and chandeliers, so with both off fewer
glows were drawn.

**Fix.** The clock is held and the table pinned at the outermost call only
(a depth count); a job run inside a job keeps the outer job's. The ordinary
redraw path never nests, so it is unchanged.

UI 5, 120 s, both host draws on: second runs identical **1,648 of 1,648**
(before 941 of 1,184).

## 2. What it changes

- Note 483's UI 5 figures: its verify-first 3,122 of 3,248 and the
  "both host draws off" runs were measured with the outer job on the live
  clock after the reference run; they are superseded by section 3.
- No picture of the ordinary redraw path changes: `worker_draw` is only
  re-entered by the developer checks and by N4b's fallback (CODE).

## 3. Proof (MEASURED)

- The self check (`DNTTK_N4_SELF=1`), UI 5, 120 s, both host draws on:
  **1,648 of 1,648** second runs identical (before 941 of 1,184).
- Verify-first (`DNTTK_N4_VERIFY_FIRST=2`), UI 5, 120 s, both host draws
  on: accepted rebuilds **1,341 of 1,344** identical to the full job
  (note 483: 3,122 of 3,248; with both draws off 1,411 of 1,424). The three
  left are D23N4's open run-alone class (note 483 section 9).
- With every switch off (note 487 section 7, the same build): Vanilla
  shots 6 of 6 byte-identical against the committed build; validator 914
  of 915 (the level-load frame).

## 4. Reproduce

Lab `recomp/analysis/d23n2b/` (local): `nm -S -C --defined-only
build-local/Duke_Nukem__Time_to_Kill_Recompiled > syms.txt`, then
`MASK=201326344 SECS=120 SLOTS=4 TAG=sd3 EXTRA="LAB_DNTTK_N4_REBUILD=1
LAB_DNTTK_REPLAY_WORKERS=1 LAB_DNTTK_N4_VERIFY_FIRST=2 LAB_DNTTK_N4_SELF=1
LAB_DNTTK_N4_STATEDIFF=$PWD/syms.txt" sh n2base.sh` and `grep "n4 state"
sd3-s4.log`.
