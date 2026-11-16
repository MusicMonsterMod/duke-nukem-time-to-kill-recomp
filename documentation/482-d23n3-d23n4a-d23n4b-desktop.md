# 482 - D23N3 residue, D23N4a ports and detectors, D23N4b built off (desktop)

Autonomous session of 2026-11-15/16 on the desktop (i7-5960X, 8 cores / 16
threads, GTX 1080 Ti). Part 1 of the three-part D23N plan. User: "Work
autonomously on the desktop through this chain, in order, as far as exact
matches allow ... 1. D23N3: find the UI 5 residue ... then widen the check
... 2. D23N4a: exact host ports of the frustum test 0x80031584 and the
transform's GTE data registers, checked on their own; then detectors at
every divergence class of note 481 section 5. Prove with the check: every
job the rebuild gets wrong is flagged, none accepted. Anything unprovable
falls back. 3. D23N4b: build it behind a developer switch, off by default
... Pictures exact (PSX_RT_VALIDATE=1, matched shots), Vanilla unchanged;
measure worker CPU on the desktop ... don't commit".

Evidence labels: MEASURED, CODE (CONFIRMED by the disassembly), INFERRED.

## 1. Setup

- CI green on `main` (the five latest runs). Host idle apart from this
  session's builds (load under 3 when the lab ran, 24 GB memory free).
- Code in a git worktree (session isolation) at `8b77275`, with the local
  inputs (disc, generated code, config, fonts, game media, the patched
  framework and UI checkouts, `saves/`) linked in; its own `build-local`
  (`-DPython3_EXECUTABLE=/usr/bin/python3`, the venv's ninja). The lab
  `recomp/analysis/d23n2b/` was copied into the worktree (cards included);
  the pre-session binary is `build-local/ab/pre` (main's build).
- Every lab run: `Frame rate: 180`, first person, Relit, Ultra, offscreen
  (SDL offscreen on the real GPU), private card copies, developer switches
  only through `LAB_`; every run's settings line checked; the player's files
  hash-checked unchanged after each run.
- The FAMILY JEWELS start was not in the lab's savestates (slots 1, 6, 7 are
  level 6 but later in the level): made as note 476 did (a new game, console
  `level 6`, saved into private slot 11 of the lab's card copy only; lab
  case `newgame`). UI 5 is DUKE HILL's open town (notes 470, 471), so the
  town's run is UI 5's.

## 2. D23N3: the UI 5 residue (CODE, MEASURED)

**Cause.** The object renderer `0x80010000` is hooked; our callbacks run,
then the game's own body runs (on the mesh our host path left: none, a
minimal D23T mesh, or polygons it leaves to the game when it cannot take
them). Its stores are top-level game stores, so the copy replayed the
earlier fraction's: quads `0x38` (and `0x2e`, `0x28`) written at PCs
`0x80010680-0x8001068c` with the other camera's corners.

**Fix (new bit 33554432).** At a `0x80010000` hook the copy dispatches the
game's routine itself from the hook's call (our callbacks fire inside, as
in the job) and drops the source's stores from the body (PCs
`0x80010000-0x80010d18`, near_clip.cpp's hashed extent). CONFIRMED: the body
calls only its own helpers (`0x80010284`, `0x80010374`, `0x80010480`,
`0x800104a8`), so no hook fires inside it. (A first version skipped calls
followed by a deeper hook, which wrongly excluded every call followed by a
sibling call: the stack pointer cannot tell the two apart.)

UI 5, 120 s, same run with and without the bit: game-packet differences 6
of 272 jobs -> 0 of 256.

**The wider check** (`MASK=67108616`, 120 s a view):

| View | later jobs compared | whole-exact | left |
| --- | --- | --- | --- |
| UI 2 | 176 | 176 | |
| UI 8 | 212 | 212 | |
| UI 9 | 322 | 322 | |
| UI 5 (DUKE HILL town) | 293 | 291 | 2 (arena class) |
| FAMILY JEWELS start | 224 | 166 | 57 (arena class), 1 host |

**What is left: the arena class (not rebuildable by replay).** In the
FAMILY JEWELS start the object renderer's own face culls change with the
camera (the job's body drew 128 quads, the earlier job's 126). The copy's
body draws the job's 128, so its packet arena cursor moves two packets
further than the source's; the source's later absolute cursor and packet
stores (other game code) no longer fit, and every later packet address
shifts. Replaying stores cannot follow that. Counted per job ("D23N3,
arena" line): FAMILY JEWELS 61 of 192 jobs (10 exact anyway), every
inexact job but one in that class; UI 5 2. With the detectors it is a flag
(section 4), so such a job falls back.

## 3. D23N4a: exact ports, checked on their own (CODE, MEASURED)

**The frustum test `0x80031584` (r a0, rectangle a1).** CONFIRMED: it reads
GTE *control* registers only (`cfc2` TRX, TRY, TRZ and H; note 481 assumed
data registers), the halves `*0x800d2214`/`*0x800d2216` and the rectangle:
TRZ < -r: 0; TRZ < r: 1; x = lo(TRX H)/TRZ, y = lo(TRY H)/TRZ;
mx = lo((lo(r w)/320) H)/TRZ, my = lo((lo(r h)/240) H)/TRZ (the compiler's
multiply-high sequences, 0x66666667 >> 7 and 0x88888889 + n >> 7); 0 when x
< rx - mx, rx + rw + mx < x, y < ry - my or ry + rh + my < y; else 1. The
game's divide traps when TRZ is 0 (then the port says unprovable). Ported in
`n4::frustum`.

Its six call sites (CONFIRMED): each is the transform `0x800292a0` (z
returned), a far test against context +0x74 (the prop's z - r, else z), the
hooked `0x800b4d2c` (just `cfc2 H`), a near test (the prop's z + r, else z,
against H), then the frustum test, with the arguments in the caller's
registers or stack at the `0x800b4d2c` hook: A `0x80031784`, B the prop
`0x80031d10`, C/D the actor draw `0x80032e78` (two), E Duke's draw
`0x800348d8`, F `0x80038e5c`.

Check (bit 67108864): at every `0x800b4d2c` hook of a site, the port against
the game's own `0x80031584` run on a scratch CPU on the same state. Five
views at 120 s: **about 930,000 calls, all identical** (sites A, B, C, D, F
exercised; E is not reached in first person).

**The transform's GTE data registers.** CONFIRMED: the last GTE work of
`0x800292a0` is MulMatrix0 `0x800b41dc`'s third MVMVA (sf 1, lm 0, RT x V0),
so it leaves V0 = the matrix's third row, MAC1-3 = the view's rows times it
>> 12, IR1-3 those saturated, FLAG bits 24-22 (and 31 for IR1/IR2); nothing
else in the routine writes a data register. Ported in `n4::data_of`.
Checked at the next hook through the runtime's own `mfc2`/`cfc2` views:
**exact at every next hook that follows the transform directly** (the
object renderer and `0x800b4d2c`: about 1.6 million); where another hook
comes next (Duke's shadow, transform ra `0x8003133c`, then its link
`0x8002bc18`) game GTE code ran in between, as expected.

## 4. D23N4a: the detectors and the proof (CODE, MEASURED)

Bit 134217728 (with 1024): every job records its culling decisions (hook
index, decision) with the ports; a later job's copy takes its own for its
fraction at the same hooks and is flagged where they differ or cannot be
taken. Jobs whose hook list differs are rebuilt too, from the source's
list (as a rebuild that does not know the job's list would be), and counted
wrong. Classes:

- far and near/frustum per site (ports); the prop's box test `0x800365e8`
  when context +0x50 has 0x400 (not pure: run as the game's code on a
  scratch CPU with its writes undone); the prop's second stage
  (`0x80031f80`: a second box test, then the hooked occluder test
  `0x8002ee50`, whose game result is not ported: unprovable);
- water boxes (the box test at the grid's entry), and water tiles: every
  tile's decision (corner flags and SZ words) with the copy's records
  against the source's (flags from its stores, SZ read before the copy
  writes the triple; rows as wide as the grid's pass-0 row, two record
  buffers alternate), plus note 478's "linked tile the copy would cull";
- small models: drawn only when the room's bit is set in `0x800e1f10` and
  `0x8002df20` returns 0 or more (out of range -1, else the translation's
  z: behind the eye is a no-draw too), against the source's 0x8002d848;
  queue mismatches;
- the hooks every routine run alone fires (portal walk, shadow, flash,
  weapon mesh, lines, object renderer, sprite runs) against the source
  hooks it replaces; the shadow when the source never linked it;
- the packet arena after the object renderer's body (section 2);
- GTE changed by game code between hooks at an object renderer hook;
- the actor hit test `0x80076330` (game code run in the composition on the
  job's own actor poses): not ported, unprovable.

**Proof** (`MASK=268435208`, 120 s a view, the final run):

| View | rebuilt later jobs | wrong | wrong and flagged | **wrong, not flagged** |
| --- | --- | --- | --- | --- |
| UI 2 | 270 | 108 | 108 | **0** |
| UI 8 | 299 | 94 | 94 | **0** |
| UI 9 | 318 | 2 | 2 | **0** |
| UI 5 | 319 | 42 | 42 | **0** |
| FAMILY JEWELS start | 263 | 122 | 122 | **0** |

The earlier runs on the way also had 0 unflagged in the final form; three
classes had been missed by intermediate versions (the prop's second stage,
water tiles judged by flags alone, game GTE work before the object renderer)
and were found because the over-broad first detectors caught them by
accident and were then narrowed. **False alarms** (exact jobs flagged)
remain high: in the 40 s runs after the last fixes, 0-13 of 35-58 exact jobs
in UI 2 and FAMILY JEWELS; in UI 5 the actor hit test runs in most
first-person jobs (171 of 319 flagged unprovable).

## 5. D23N4b: built behind a developer switch, off by default (CODE, MEASURED)

`DNTTK_N4_REBUILD=1` (with the mask above and `DNTTK_N2_WHOLE="1 1"`): a
worker records (only) a frame's first job; its later jobs of the same frame
do not run the game's composition: `worker_draw` saves the job's image,
`wj::body(true)` rebuilds the stream in place from that record with the
detectors, and the normal chain close and submit run from the source's
flip state; any flag, failure, a prefix of another length or an arena
cursor that is not the source's at its flip reloads the image and runs the
full job (the rebuild disabled for that run). `DNTTK_N4_VERIFY=N` /
`DNTTK_N4_VERIFY_FIRST=N` compare every Nth job's stream with the full job
(after, or first and unrecorded), `DNTTK_N4_DUMP=dir` writes the first
differing pairs. The forked comparison is the check without the switch.

**Where it applies.** At 180 Hz with three workers a frame's jobs go to
different workers, so a worker almost never has a later job of its own
frame: N4b needs one worker (`DNTTK_REPLAY_WORKERS=1`, the field laptop's
placement). All N4b runs below use one worker.

**Not exact.** Accepted rebuilds against the full job (verify-first, clean
unrecorded reference, 120 s): UI 2 634 of 672, UI 5 610 of 624 identical;
the five-view verify run: UI 2 627/656, UI 8 16/16, UI 9 63/64, UI 5
999/1024, FAMILY JEWELS 976/976. The differences: mostly the game's packets
after the flip (one 40-byte packet allocated differently before the first
bucket's fill quad, so every later packet address shifts, while the
rebuild's arena cursor equals its source's); some water tile hooks
(`0x8002bc18/0x8002fbb4`) and object renderer hooks (`0x80010000/0x800351f4`);
host precise-vertex records in one dump. Fixed on the way: the host ports'
statics persisted between in-place rebuilds (now reset), the recorder's
channel after spliced items (now the source's at its flip). Cause not
established. The fork check found no unflagged wrong job; the in-place
mode differs from the fork in state that is not the guest image (host
statics of our hooks, recorder state, the source being the latest recorded
job of the frame) - the next suspect list.

**Not cheaper.** The game's CPU, UI 5, one worker, 40 s (case `wcpu`; its
"workers" figure counts every game process, the emulation thread, render
threads and worker together: the process it takes as the main one is the
launcher):

| Run | the game's processes, % of a core | presents/s |
| --- | --- | --- |
| plain (no developer switch) | 91.2 | 60.0 |
| the check's recording only (mask) | 100.2 | 60.0 |
| N4b on | 94.5 | 60.0 |

A rebuild costs 2-4 ms against a full job; the capture (every store and
each hook's CPU state of the first job) and the live decision records
(scratch box tests) cost more than the rebuilds save. A lean production
capture was not built.

By the user's rule ("pictures exact") **D23N4b stays off**; no default-on
decision is possible. CPU and power on the field laptop would only measure
an inexact path.

## 6. With every developer switch off (MEASURED)

- Render-thread validator (`LAB_PSX_RT_VALIDATE=1`, case `cad`, UI 2, 8, 9,
  40 s each): this build 908 of 908 images exact; the pre-session binary
  915 of 915.
- Vanilla (`--vanilla`, case `vshots`): UI 8's and UI 2's savestates, shots
  at +60, +200, +400 frames, pre-session binary against this one:
  byte-identical (6 of 6).
- Local CI: 259 tests OK (4 skipped), job board and repository checks OK.

## 7. Code and lab

All in `recomp/src/ttk/redraw_n2.inc` (namespace `n4`, `wj::body`,
`n4b_run`), `redraw_check.inc` (the capture's flip state, prefix, cursor
and channel; later jobs not recorded under N4b) and `frame_replay.cpp`
(`worker_draw`: image save, rebuild or fallback, verify). Nothing runs
without `DNTTK_N2_CHECK` / `DNTTK_N4_REBUILD`. No framework change.

Lab (local) `recomp/analysis/d23n2b/`: `n2base.sh` (`EXTRA` for more
`LAB_` words), `sum_n2.py` (new lines), `lab.py` cases `newgame` (`LEVEL`,
`SAVE`), `wcpu`, `vshots`; FAMILY JEWELS start in private slot 11.
Reproduce: `MASK=268435208 SECS=120 SLOTS=1,7,8,4,11 ./n2base.sh` (the
proof); `EXTRA="LAB_DNTTK_N4_REBUILD=1 LAB_DNTTK_N4_VERIFY_FIRST=2
LAB_DNTTK_REPLAY_WORKERS=1"` for N4b.

## 8. Part 2 on the field laptop (MEASURED)

User, during the session: "once this set of jobs is finished ... move over
to the field laptop ... compiling and then testing on the field laptop ...
then autonomously work on the third task". The laptop (i7-5600U, 2 cores /
4 threads, 15 W) was fast-forwarded to `8b77275` and this session's changes
applied there uncommitted (the same patch as the desktop worktree); built;
lab `recomp/analysis/d23n4/` (this session's scripts with a copy of the
laptop lab's cards: UI 5 town = runtime 4, FAMILY JEWELS start = private
11). One redraw worker (the laptop's placement). Case `wcpu` now also gives
package power (RAPL through `sudo -n`) and the mean clock. 40 s a view,
offscreen, one run each in this order: plain, N4b, at 120, then 180, then
the check's recording alone at 120.

| Run | town: game's CPU / presents / package / clock | FAMILY JEWELS start |
| --- | --- | --- |
| plain 120 | 182.1% / 119.8 / 18.6 W / 2479 MHz | 219.1% / 110.7 / 15.0 W / 2253 MHz |
| N4b 120 | 169.6% / 119.8 / 15.0 W / 2278 MHz | 99.1% / 118.4 / 14.9 W / 2588 MHz |
| recording only 120 | 168.7% / 120.0 / 15.0 W / 2251 MHz | 204.7% / 110.8 / 15.0 W / 2382 MHz |
| plain 180 | 188.8% / 167.8 / 15.0 W / 2109 MHz | 218.8% / 163.6 / 15.0 W / 2277 MHz |
| N4b 180 | 171.2% / 158.0 / 15.0 W / 2288 MHz | 94.5% / 102.0 / 14.9 W / 2569 MHz |

Reading (INFERRED where stated): the town with the check's recording alone
(no rebuild) is as low as with N4b, so the difference to plain is run order
and the 15 W package, not rebuilds. In the FAMILY JEWELS start N4b rebuilt
one later job in 60 s (the verify run below), so its lower figure is not a
saving; why the game used less CPU there with the switch is not
established (no worker failures or deadline kills in the log). Verify-first
at 120 (60 s): town 191 of 192 accepted rebuilds identical to the full job
(387 rebuilt, 77 fell back of 464). **Default-on decision: no** (not exact,
no measured saving). D23Y7 and D23AA: nothing here is on by default, so
note 476's measurements stand; not re-measured.

Ordering-table comparison of failing in-place rebuilds (desktop,
verify-first): the rebuilt and reference tables differ in a few bucket
words, each pointing one 40-byte packet apart: before the flip something
allocated one packet otherwise in the job than in its source without
changing any hook or detected decision, and the rebuild followed the
source. The missing stream item in the dumps is one empty list node
(kind 2) of that chain.

## 9. Part 3 not started

D23Y8 (note 470 section 3) is a framework threading change of the GL
backend and presentation (a shared-context presentation thread first, then
a proxy queue with drain barriers at every readback), measured there as not
reaching D23Y5's target alone. Built unattended it could not be verified on
a real display (the D23K driver hangs make unattended real-display runs off
limits), so it was left for a session that starts with it.

## 10. Next

1. N4b exactness: the in-place differences above, starting from the dumps
   (`DNTTK_N4_DUMP`) of the post-flip packet case; compare host statics of
   the hooks between a forked copy and an in-place rebuild of the same job.
2. A lean capture (without the check's full record) and cheaper decisions,
   or N4b cannot pay for itself.
3. Fewer false alarms: the actor hit test `0x80076330` (port it or bound
   its effect), water.
