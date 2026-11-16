# 483 - D23N4b: three defects fixed, nearly exact, not cheaper (desktop)

Autonomous session of 2026-11-16, part 1 of the chain. User: "D23N4 on the
desktop: make N4b's accepted rebuilds exact (note 482 sections 8 and 10:
start from the one-packet allocation difference, the dumps and the
ordering-table diff; log the packet-arena cursor at every hook in the
reference and the rebuild to find the first difference), port or bound the
actor hit test 0x80076330 to cut fallbacks, then a lean first-job capture
(record only what the rebuild needs; drop check-only mask bits). Prove with
DNTTK_N4_VERIFY_FIRST in UI 2, 8, 9, 5 and the FAMILY JEWELS start at 120 s,
and measure the game's CPU on vs off (case wcpu, one worker)."

Evidence labels: MEASURED, CODE (CONFIRMED by the disassembly or read in the
source), INFERRED.

## 1. Setup

Desktop (i7-5960X, GTX 1080 Ti), the session worktree of note 482 at
`6e887c5` with its local inputs and build, lab `recomp/analysis/d23n2b/`.
Every run: `Frame rate: 180`, first person, Relit, Ultra, offscreen (SDL
offscreen on the real GPU), private card copies, developer switches only
through `LAB_`, the player's files hash-checked unchanged. N4b runs use one
redraw worker (`DNTTK_REPLAY_WORKERS=1`, note 482 section 5). UI 2, 8, 9 and
5 are runtime slots 1, 7, 8 and 4; the FAMILY JEWELS start is private slot
11 (note 482).

## 2. The arena cursor at every hook (tool, MEASURED)

`DNTTK_N4_CURSOR=1` (developer) records the packet arena cursor
`0x800d67a8` at every top-level hook entry and exit of the recorded source,
of the verify-first reference and of the in-place rebuild, and on a
difference prints the first entry where they part; it also keeps a water
tile's four corners (flags and SZ as the job reads them), a word to follow
(the first post-flip difference's packet, found in main RAM or the DMA
aperture), the renderer state and full CPU at the object pass entry, and
main RAM, scratchpad and mod memory there. `DNTTK_N4_SELF=1` (developer)
runs a verified job in full a second time instead of rebuilding it: is a
full job repeatable?

## 3. Defect 1: water tiles judged on stored flags (CODE, MEASURED)

**First difference.** In every inexact UI 2 rebuild of note 482 the
reference fires one more water-tile link `0x8002bc18` (ra `0x8002fbb4`,
one 40-byte packet) than its source, just before the next row's first
vertex hook; the rebuild follows the source, so its cursor equals the
source's (why note 482's cursor check passed) and every later packet
address shifts.

**Cause.** The game's tile loop (`0x8002f6b0-0x8002fbd0`, CONFIRMED) runs
after a row's last vertex triple and skips a tile when its corners' flag
words share a bit, any has 0x10, or (the four SZ summed, +3 when negative)
>> 4 is 0x4001 or more; every other tile links. While the water near path
is live our callbacks zero a near vertex's whole flag word before the loop
reads it (`water_near_flags` at `0x800b4d2c`, then `water_clip` at a
tile's link for both rows: water_clip.inc). The D23N4a detector compared the
flags as stored: a copy corner flagged 0x10 beside the source's plain
corner (flags 6) made both "cull" a tile the job built (its corner word
was 0 at the loop).

**Fix.** The copy walks the game's loop on its own records (the job's at
that point), one tile at a time: the next tile is found from memory before
the source's next hook (after the last tile's callbacks ran, as the loop
reads it), and the source's tile hooks must be exactly those tiles (flag
"water tiles"). The stored-flag tests (`water_differs`, the tile cull
count) are superseded and kept behind `DNTTK_N4_OLD_WATER=1`.

UI 2, 60 s: verify-first 272 of 272 rebuilds exact (note 482: 634 of 672).

## 4. The actor hit test 0x80076330 bounded (CODE, MEASURED)

In the composition it runs from one site, the segment query `0x80077560`
(ra `0x800775f0`; reached through a pointer, no direct caller; CONFIRMED):
a0 the actor to ignore, the from/to pair on the caller's stack, outputs at
the object's +0x184/+0x194, stack arguments 5 and 6 zero; a hit calls
`0x80077468`. UI 5, 40 s: 3,100 such hooks in one worker; the other
unprovable causes were near zero (the occluder test 55).

**Bound, not ported.** The game's routine is run on a scratch CPU from the
hook's entry state (the caller's stack pointer kept for the stack
arguments; our callback for that one entry muted with `g_ttk_hook_mute`,
because the routine's own call tree reaches hooked routines such as the
look-at `0x80097a44` whose callbacks must run as in the job; every write
undone). Its result is v0 and every address it wrote with the final value,
hashed. The first job records the hash at each such hook; the copy
computes its own at the same hook and accepts only an equal one.

UI 5, 40 s: rebuilt later jobs 117 -> 777 of 1,113; hit tests in the copy
2,866 equal, 254 different (those fall back).

## 5. UI 5: a full job is not repeatable (MEASURED)

UI 5's remaining verify-first differences were one small Gouraud quad
(GP0 0x3e) one colour step apart (0x34 against 0x33), written in mod memory
by host code at the object pass entry `0x800374cc`. With main RAM,
scratchpad, CPU and GTE identical there, `DNTTK_N4_SELF=1` showed the full
job itself is not repeatable in the same worker:

| UI 5, 30-40 s | full job run twice: identical |
| --- | --- |
| as played | 941 of 1,184 |
| sleeping enemies off (`DNTTK_SLEEPING_ENEMIES=0`) | 655 of 864 |
| and object reach off (`DNTTK_OBJECT_REACH=0`) | **880 of 880** |

Both host-dispatched game renderers of the object pass (D17AQ's sleeping
stand-ins through `0x80032e78`, D17AZ's out-of-room objects through
`0x80031d10`) leave host state that changes the next run's colours; the
image (main RAM, scratchpad, both mod-memory regions) is fully restored, so
it is C++ state (not identified; INFERRED: a light or shade cache). This is
a property of the redraw path without N4b: a picture there depends on the
worker's history. In UI 5 verify-first therefore cannot judge a rebuild
unless both are off.

## 6. Defect 2: Duke's joint loop had no detector (CODE, MEASURED)

The fork proof (the check without N4b) found one unflagged wrong job in
UI 8 once the hit test no longer flagged every job: the job ran one more
joint transform than its source. Duke's joint loop in his draw `0x800348d8`
is a seventh site (CONFIRMED): the transform (ra `0x80034c6c`, z in s0), far
(culled when context +0x74 < z), the hooked `0x800b4d2c` (ra `0x80034c8c`),
near (culled when z < H), no frustum test, then the box test `0x800365e8`
when context +0x50 has 0x400. Added as its own decisions. First version:
every UI 2 job flagged (102 of 102): our D35E callback at that near hook
raises s0, the loop's saved depth, to H for Duke's own limbs
(`first_person_ladder_near`), so the game tests that, not the transform's
z. Now the copy sets s0 to its own transform's z before the hook's
callbacks and decides on s0 as they leave it: UI 2, 40 s, 7 wrong jobs all
flagged, no exact job flagged.

## 7. Defect 3: the joint table build halted the worker (CODE, MEASURED)

With N4b the single worker was killed at the 250 ms job deadline 84-389
times a 120 s run in UI 5 and the FAMILY JEWELS start (0 without N4b), so
note 482's and this session's first CPU figures counted lost work, and the
FAMILY JEWELS verify-first compared nothing. `sudo eu-stack` on the worker:
`psx_fatal_halt` from `psx_unknown_dispatch` under `jr::rebuild_table` in
`wj::body`. The piece 6 table build ran on a private stack allocated once
in mod memory; in place, every image load resets the allocator to the live
process's watermark, so the stack lay past it, the build read its return
address as 0 and halted. Fix: the build runs on the job's own stack
pointer at the hook (`0x80032bd8` is called from the same frame of the actor
draw as the build). After it: 0 kills in every run. Also: every early
return of `wj::body` left its hook observer installed; `n4b_run` now clears
it.

## 8. The lean capture (MEASURED)

Of the N4b mask `268435208`, only bit 67108864 (the ports checked against
the game's own routines at every call) is check-only: without it, UI 2
verify-first 48 of 48 exact. Bits 8, 256 and 512 feed the rebuild (without
them 19 of 240 exact). Lean mask: **201326344**. What else the first job
records is what the rebuild uses (every top-level store, each hook's CPU
state, the stream, the culling decisions with their scratch box tests and
the hit-test hashes).

## 9. The proof (MEASURED, final build)

Verify-first (`DNTTK_N4_VERIFY_FIRST=2`, every second candidate job run
first in full, unrecorded, then rebuilt and compared word for word), lean
mask, 120 s a view:

| View | rebuilds compared | identical | later jobs rebuilt / fell back |
| --- | --- | --- | --- |
| UI 2 | 1,072 | 1,070 | 2,087 / 1,273 |
| UI 8 | 1,088 | 1,088 | 4,245 / 2,475 |
| UI 9 | 1,792 | 1,792 | 3,597 / 51 |
| UI 5 | 3,248 | 3,122 (section 5) | 2,884 / 748 |
| UI 5, both host draws off | 1,424 | 1,411 | 2,866 / 766 |
| FAMILY JEWELS start | 688 | 688 | 1,377 / 1,535 |

Fork proof (the check, mask `268435208`, no N4b, 120 s a view): wrong jobs
not flagged UI 2 **2**, UI 8 0, UI 9 0, UI 5 **1**, FAMILY JEWELS 0 (the
run before the joint fix: 0, 0, 0, 0, 0 with the joint detector's false
alarms; before it UI 8 1, UI 5 1, FAMILY JEWELS 1).

**Open residue** (not established; next steps):

- UI 2's two: the job visited one more (or one fewer) water box
  `0x8002ef90` (ra `0x800300c4`) than its source, so a decision before the
  box test (the pass's own room or box loop) is not detected.
- UI 5's (and earlier FAMILY JEWELS') one: same hook list, the game's own
  sprite-like packets (GP0 0x2e, 0x26) after the flip one pixel apart:
  game code run alone in the copy with slightly different GTE input (the
  check counts 8,698 hook GTE differences; D23N3's class).
- Verify-first: UI 2 2 of 1,072, UI 5 13 of 1,424 with the host draws off,
  FAMILY JEWELS 2 of 224 in a 40 s run (object renderer hooks from
  `0x80031b44`).

## 10. CPU on against off (MEASURED)

Case `wcpu`, one worker, 40 s a view, interleaved (plain, N4b, plain, N4b),
the game's processes but the launcher, % of a core:

| View | plain | N4b lean |
| --- | --- | --- |
| UI 2 | 113.7, 113.3 | 122.1, 121.8 |
| UI 5 | 110.9, 111.7 | 117.6, 121.0 |
| FAMILY JEWELS start | 117.1, 123.7 | 142.9, 134.5 |

Presents 60.0 and game 59.9-60.0 in every run, 0 worker kills. **N4b costs
6-16% more** (before the halt fix the N4b runs read 62-88%: killed jobs,
not a saving). A rebuild costs 2.5-4 ms against a 10-15 ms full job, but
the first job's capture (every store and each hook's CPU state), the
detectors' scratch runs (box tests, the hit test twice) and the image save
for a fallback cost more than the 30-60% of later jobs that are rebuilt
save. **Not exact and not cheaper: D23N4b stays off**; part 2 (the field
laptop with N4b on) was not run (note 484).

## 11. With every developer switch off (MEASURED)

- Vanilla (`--vanilla`, case `vshots`, `offcheck.sh`): UI 8's and UI 2's
  savestates, shots at +60, +200 and +400 frames, the committed build
  (`6e887c5`, `ab/now`) against this one (`ab/now3`): **byte-identical, 6
  of 6**.
- Render-thread validator (`LAB_PSX_RT_VALIDATE=1`, case `cad`, UI 2, 8, 9,
  40 s each): **909 of 910 images exact**; the one is the level-load frame
  right after the bake loads (39,865 pixels, at most 4 levels, the top band),
  the same single level-load image note 481 recorded with its pre-session
  binary (906/907; note 482: 908/908).
- Local CI: 259 tests OK (4 skipped), job board and repository checks OK.

## 12. Code and lab

`recomp/src/ttk/redraw_n2.inc` (the row walk `n4::wrow_*`, `n4::hit_hash`,
the joint site, `rebuild_table`'s stack, the slow-rebuild and diagnostic
lines), `redraw_check.inc` (the cursor trace), `frame_replay.cpp`
(`DNTTK_N4_SELF`, verify-first's diagnostics), `modern_controls.cpp`
(`g_ttk_hook_mute`: one branch at the top of `ttk::hook`, 0 unless a scratch
run sets it). No framework change. Lab: `sum_n4b.py`, `n4bproof.sh`,
`n4bproof2.sh`, `n4bproof3.sh`, `wstacks.sh`, `jals.py`, `funcs.py`,
`calltree.py`, `offcheck.sh`.

Reproduce: `MASK=201326344 SECS=120 SLOTS=1,7,8,4,11 TAG=fa
EXTRA="LAB_DNTTK_N4_REBUILD=1 LAB_DNTTK_REPLAY_WORKERS=1
LAB_DNTTK_N4_VERIFY_FIRST=2" sh n2base.sh` then `python3 sum_n4b.py
fa-s*.log`; the fork proof `MASK=268435208 SECS=120 SLOTS=... TAG=fb sh
n2base.sh`; CPU `sh n4bproof3.sh` (all three).

## 13. Next

1. Cost before exactness: N4b can only pay if the first job's capture is a
   fraction of a job. Measure the capture's parts (store log, hook CPU
   states, decisions, hit-test scratch runs, image save) with the same
   `wcpu` case, then decide whether D23N4 continues.
2. The two open detector classes above (water box visits; run-alone sprite
   packets).
3. The non-repeatable host draws of section 5 (a redraw-path defect without
   N4b: find the state and keep it per job).
