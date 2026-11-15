# 481 - D23N2B weapon attachments and D23N3 sprites exact; D23N4 groundwork (desktop)

Autonomous session of 2026-11-15 (late evening) on the desktop (i7-5960X,
8 cores / 16 threads, GTX 1080 Ti). User: "Work autonomously on the desktop
through this chain, in order, as far as exact matches allow ... 0. Pull main
(the field-laptop session of note 480) ... First re-run the full check on
this machine's settings ... 1. D23N3: confirm the line effect run alone ...
chase the one-off UI 5 worker incident ... 2. D23N2B: make the weapon
attachments exact ... Then, if worthwhile, a host port of the portal walk
... 3. D23N4 groundwork: ... Find how a rebuild can know a later job's
culling before it runs ... Prove it with the check ... Then build D23N4
behind a developer switch, off by default ... If a step can't be made
exact, leave it off, record exactly what differs and why, and carry on."

Evidence labels: MEASURED, CODE (CONFIRMED by the disassembly), INFERRED.

## 1. Setup

- Host checked first (MEASURED): load 0.7, 23 GB memory available, 55 MB
  swap in use, no backup, update or package process; CI green on `main`
  (the five latest runs).
- Pulled `main`: first to `28ef7df` (note 479); note 480's commit `7464bb8`
  reached `origin` a few minutes later and was pulled then. The framework
  tree held the previous accepted patch exactly; it was reversed and the
  current patch applied (`apply_runtime_patches.py`, the stack verified).
  `cmake --build --preset local-dev` in the main checkout (generated code
  recompiled), then current.
- This session's code was written in a git worktree (the session's own
  isolation rule), with the local-only inputs (disc, generated code,
  config, fonts, game media) linked in, its own `build-local` and its own
  copy of the lab `recomp/analysis/d23n2b/` (cards included). Lab runs use
  the worktree's binary; the player's files are hash-checked at their real
  location after every run.
- The field laptop's helper tools of note 480 section 1 were not on this
  machine. Written here (local, in the lab): `n2base.sh` (the check in UI 2,
  8, 9, 5 = runtime slots 1, 7, 8, 4 at 180 through `lab.py`), `callers2.py`
  (jal call sites), `refs.py` (loads and stores by offset), `n3align.py`
  (pairs a dump's game items and names each differing word's store PCs in
  the job and the earlier job), `n3pk.py` (a dump's stores in a PC range,
  job against earlier), `n3links.py` (static call graph and prologues);
  `sum_n2.py` extended for note 480's report lines; `lab.py` gained case
  `lines`. `mdis.py` (capstone) was already here.
- Every run: `Frame rate: 180`, first person, Relit, Ultra, offscreen (SDL
  offscreen on the real GPU), case `cad` (walk, turn, fire), private card
  copies, the player's files unchanged afterwards.

## 2. Baseline on this machine (MEASURED)

`DNTTK_N2_CHECK=2096904 DNTTK_N2_WHOLE="1 1"`, 60 s a view (note 480
section 4's setting):

| View | later jobs compared | host-exact | whole-exact | hook list differs | first differing hooks |
| --- | --- | --- | --- | --- | --- |
| UI 2 | 74 | 71 | 71 | 2 | the flash x3 |
| UI 8 | 118 | 115 | 115 | 3 | the flash, the line effect, the weapon mesh |
| UI 9 | 158 | 158 | 155 | 1 | none (3 jobs' game packets: sprite quads `0x2e`) |
| UI 5 | 131 | 128 | 128 | 1 | the flash x3 |

Same pattern as note 480 (the desktop compares fewer jobs a minute: every
4th later job of each worker). The line effect drew here, in UI 8, and was a
root: bit 1048576 was not exact.

## 3. The weapon attachments (D23N2B; CODE, MEASURED)

**The muzzle flash.** New developer capture of the flash's inputs, the job's
against the copy's (the hand matrix it reads after our hook, the muzzle
vector, the rand seed `0x800e81b8`, the matrix its transform reads, the GTE
at its draw). In the copy our flash callback had not run at all: the copy
dispatched `0x800341e4` with its return address set to the check's
sentinel, and `first_person_weapon_attached` acts only for Duke's draw's
call (ra `0x80035348`). So the copy's flash composed its yaw with Duke's
real hand joint instead of our D36 weapon matrix (the composed matrices
differed by hundreds of units; note 480's "mesh depth 511 against 666").
Fix: the copy dispatches the routine with the call's own return address as
`ra` and as the stop address, as piece 13's portal walk already did. Exact
from then on (every flash in every view).

**The scaled weapon mesh (ra `0x80033f44`).** CONFIRMED: `0x80033e40`
(hand matrix, mesh, yaw, scale; camera at the caller's sp +0x10) draws
under the hand's transform when yaw is 0 and scale 0x1000; otherwise it
builds its own matrix on its stack from ours (a copy, PCs
`0x80033eb4-0x80033ee0`, or a yaw rotation `0x8002acc4` and MulMatrix
`0x800b488c`), ScaleMatrix `0x800b4b5c`, the transform (ra `0x80033f10`)
and the draw (ra `0x80033f44`). The copy replayed the source's stores of
that stack copy, the earlier fraction's copy of our matrix, which follows
the camera: off by a few units, and only for slots with a scale (hence
rare). Fix (bit 2097152): `0x80033e40` runs alone from its hook like the
flash, with its real return address `0x8003531c`. Exact.

**The attach point (found under D23N3, below).** `0x80032364` is redone in
the copy (bit 16777216).

**A host port of the portal walk** (`0x8006276c`): not built. Here the
game's walk run alone costs 0.20-0.35 ms of a 6-51 ms rebuild (under 5%,
note 480: 0.43-0.76 ms on the laptop); it only matters once D23N4 runs
rebuilds instead of jobs, and is not worthwhile before that.

## 4. The line effect and the game's sprites (D23N3; CODE, MEASURED)

**The line effect** (bit 1048576). CONFIRMED: `0x8009022c(start, end,
colour)` appends a 3D line to the list `0x800c4580` (count `0x800c4584`,
capacity `0x800c458c`, allocated by `0x8008f694`, cleared after the camera
render by `0x800900a0`); `0x8008ffc4` draws it. The orange lines (`81 56
04`, GP0 colour `045681`) come from `0x8006f7a4` (`0x80070230`), reached
through a callback; the other callers are 12-line debug boxes. Two faults in
the copy's run: (1) its skip range ended at the first hook back at the
caller's stack pointer, but after `0x8008ffc4` returns the camera render
calls the sprite wrapper and small models, also deeper; the copy skipped
those hooks and their stores (MEASURED: 1,152 and 2,784 words of the job's
against none). Now only the routine's own hooks (its links, ra in
`0x8008f71c-0x800900a0`). (2) The copy also filtered out the sprite
wrapper's marker store (`0x8009b768`) as one of the line routine's, so the
sprite queue did not run in line jobs; the marker is now let through.
Result: UI 8, 120 s, 5 line jobs: 199 of 199 later jobs whole-exact.

**Duke's joint sprites** (bit 4194304; UI 9's sprite quads). Found with the
dumps: the differing `0x2e` quads are queued sprites (emitter `0x8002cff0`,
store PCs `0x8002d0a8-0x8002d0cc`); their queued positions (`0x8009b7fc`,
PCs `0x8009b84c-0x8009b858`) differ between fractions by 1-25 units and are
written during the render job itself. CONFIRMED: near its end Duke's draw
(object in s4), when the object's flags have 0x800, calls `0x800307c0`
(position, 0xff, j) for each joint j below the byte at *(object +0x40),
position = the joint table *(object +0x3c) + 32 j + 0x14, return
`0x80035484` (found from the routine's own ra store; there is no static
reference); it queues one sprite per joint. In first person the joints
follow the camera. In the copy, at that loop's first store (PC
`0x800307d0`; the caller's sp is its address + 8) the loop runs over the
copy's joint table, when the source made as many calls as the copy has
joints; the source's stores of it are not replayed. Exact (UI 9: 141-154
jobs a run).

**The particles and the attach point** (UI 5's rare sprite quads). The
camera render (camera in s2) calls `0x8006d08c`, `0x8009ce44`, `0x8008efd0`,
`0x8008ffc4` and `0x8009b750` back to back with no stores of its own
between (`0x8002e7d8-0x8002e808`). `0x8009ce44` walks the particle list
(`0x8009ca2c`) and queues each particle at particle +0x1c. Bit 8388608 runs
it alone (marker: its first store, PC `0x8009ce48`, caller sp = address +
16; it ends at `0x8008efd0`'s first store, PC `0x8008efd4`). That alone did
not fix them: the differing particle's position equals Duke's object's
attach point (+0xbc..+0xc4), which is written during the job by
`0x80032364` (object, joint table): a joint's translation plus its rotation
applied to an offset (ApplyMatrix `0x800b4aac`). Its callers, Duke's draw
(`0x80034918`) and the actor draw (`0x8003300c`), then call the hooked
`0x80032bd8` with the same arguments (return `0x8003492c`, `0x80033020`).
Bit 16777216 runs the game's `0x80032364` again in the copy just before
that hook (after piece 6's joint table), and the hook gets the GTE it
leaves. UI 5, 120 s: game-packet differences 9 -> 4 jobs of 265-281.

**What still differs (not fixed; MEASURED, UI 5 only, a late stretch of a
120 s run, 4 of 265 jobs; none in the 60 s runs of section 6):** gouraud
quads `0x38` written by the game's own object renderer path (store PCs
`0x80010680-0x8001068c`, corners 1-4 units apart), and one flat quad `0x28`
whose words are written inside a hook (no store PC). Cause not established;
the next step is a dump with `n3align.py` naming the object whose
`0x80010000` call the game drew itself.

**The note 480 UI 5 worker incident** (host arena `0x34` packets where the
rebuild had the game's, six jobs of one worker): not seen in this session's
runs (UI 5: five runs, 60-120 s). The `0x34` items in the 120 s particle
run were cascades after a differing sprite (the chain's addresses shift).

## 5. D23N4 groundwork: how a rebuild could know a later job's culling (CODE, MEASURED)

Where the hook lists leave each other (MEASURED, all views, the final
runs): water tiles (`0x8002bc18/0x8002fbb4` against the next grid vertex
`0x800b4d2c/0x8002f508`), prop draws (`0x80010000/0x80032288` against the
list step `0x8001ca4c/0x80037614`), the portal walk's recursion
(`0x8006276c/0x80062ae4`, `0x80062b48/0x8002e688` against
`0x800b4d2c/0x80062364`), small models (`0x800a2690` against
`0x8002d848`/`0x8002bd48`), the shadow's link against the next hook. In the
final runs 6, 5, 1 and 2 of 84, 103, 151 and 130 later jobs (UI 2, 8, 9, 5).

Each is a culling test taken by the game's code between two hooks, on
values the copy already computes for its own fraction:

- **Props** (CONFIRMED, `0x80031d10`): after the prop's transform (ra
  `0x80031de8`, depth z returned), cull when z - r > context +0x74 (far),
  when z + r is below the hooked `0x800b4d2c`'s value (near), or when the
  frustum test `0x80031584(r, ...)` fails (it reads the transform's GTE
  results with `mfc2`); r = the object's half at +0x5a. The copy's transform
  is piece 1's exact port (bit 2048) for the same object.
- **Small models** (note 478 section 2): the model's transform `0x8002df20`
  (no draw when any |d| >> 16 is set) and the face culls of `0x8002d848`
  are both already run on the host in the copy (bit 4096).
- **Water tiles** (note 478 section 2): the tile test (shared flags, any
  0x10, s3 over 0x4000) is already run on the host for every linked tile
  (bit 8192; its counter "tiles the port would skip"); the unlinked tiles
  of a box follow from the same records.
- **The portal walk**: already the game's own walk for the job's camera
  (bit 32768); its recursive hooks fire inside the copy's run and can be
  compared with the source's.
- **Duke's shadow**: run alone (bit 131072); its link either happens or not
  inside the run.

So a rebuild can follow the game's culling decisions instead of the
earlier job's list: at each such point evaluate the test for the copy's
fraction (or watch the hooks of the routine it runs alone) and, when the
decision differs from the source's, abandon the rebuild and run the full
job. Not built in this session: the prop and actor tests need exact ports
of the frustum test `0x80031584` and the GTE data registers the transform
leaves (the host transform port gives only the control registers), and the
water and small-model ports need the unlinked side of their decisions. The
proof the user asked for ("no job the rebuild gets wrong is accepted") is
therefore not made, and by the user's rule **D23N4 was not built and is not
switched on**; no A/B, no CPU or power measurement (left for the field
laptop in any case).

## 6. Result (MEASURED)

`DNTTK_N2_CHECK=33554184` (note 480's 2096904 plus 2097152, 4194304,
8388608 and 16777216), `DNTTK_N2_WHOLE="1 1"`, 60 s a view at 180:

| View | later jobs compared | host-exact | whole-exact (game packets too) | hook list differs (skipped) | rebuild / job ms |
| --- | --- | --- | --- | --- | --- |
| UI 2 | 78 | 78 | 78 | 6 | 51.4 / 102.3 |
| UI 8 | 98 | 98 | 98 | 5 | 12.2 / 42.1 |
| UI 9 | 150 | 150 | 150 | 1 | 6.2 / 31.2 |
| UI 5 | 128 | 128 | 128 | 2 | 15.5 / 47.2 |

Every compared later job identical word for word, host and game packets.
Times include the check's recording on both sides (upper bounds; UI 2's
desktop jobs carry six workers' recording). Routines run alone, per
rebuilt job: the portal walk 0.20-0.35 ms, the shadow 0.01-0.20, the sprite
queue, joint sprites and particles 0.03-0.29, the flash, lines and weapon
mesh 0.00-0.11.

## 7. With the check off (MEASURED)

All of this is in `recomp/src/ttk/redraw_n2.inc` and runs only with
`DNTTK_N2_CHECK` (no `[n2 check]` line in the runs below).

- Render-thread validator (`PSX_RT_VALIDATE=1`, case `cad`, UI 2, 8, 9,
  40 s each): 906 of 907 images exact; the pre-session binary (the same
  commit without this code) 914 of 915. Both differing images are whole
  frames (2,579,972 pixels: a level load), the run-to-run case of note 478.
- Vanilla (`--vanilla`): matched shots of UI 8's and UI 2's savestates at
  frames 60, 200 and 400, the pre-session binary against this one:
  byte-identical (6 of 6).

## 8. Reproduce

Lab `recomp/analysis/d23n2b/` (local): `MASK=33554184 SECS=60 ./n2base.sh`
(`TAG`, `SLOTS`, `EXTRA="LAB_DNTTK_N2_DUMP=$PWD/dir"`), then `python3
n3align.py dir/*.bin` and `python3 n3pk.py LO HI dir/*.bin` for the
packets and stores that differ; `refs.py`, `callers2.py`, `n3links.py`,
`mdis.py` for the code.
