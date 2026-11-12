# D23N2 - A later redraw job's transforms, camera and visibility, rebuilt and checked

Status: Done (pieces 1-3 built and verified 2026-11-12, accepted with D23M2), developer checks only, no
player-facing change. Job board: D23N2 (rebuild a later job of the same game
frame for its own camera and fraction). Builds on the D23N1 record
([note 454](454-d23n1-capture-and-rebuild.md)) and the D23N2 inventory
([note 455](455-d23n2-what-a-later-job-changes.md)). Evidence labels:
CONFIRMED (our measurement, code or the game's disassembly), INFERRED.

## 1. What a later job's time is made of

Before building pieces, the record was extended so it can say where a job's
time goes and which game instructions write what differs between two jobs of
one frame (developer, `recomp/src/ttk/redraw_check.inc`):

- every logged top-level store now carries the PC of the guest instruction
  that made it (`g_debug_last_store_pc`, set by the generated code);
- every hook entry carries its entry and exit time (TSC), and the job its
  start and flip time;
- `DNTTK_N2_TRACE=path` (with `DNTTK_N2_DIFF=1`) writes pairs of records (the
  earlier and the later job of a frame) for offline analysis
  (`DNTTK_N2_TRACE_N`, `DNTTK_N2_TRACE_SKIP`).

Measured offscreen, UI slot 2, turning at 180 Hz, 30 jobs of 6.4 ms
(CONFIRMED):

| Where | Share of a job |
| --- | --- |
| hook callbacks (our host code and the guest code it dispatches) | 55.5% |
| - host room walk (hook `0x80062b48`) | 25.1% |
| - host object renderer (hook `0x80010000`, 28 calls) | 16.7% |
| - water and sprite hooks (`0x8002bc18`, `0x800b4d2c`) | 8.6% |
| the game's own code between hooks | 44.5% |
| - after the fill hook `0x800b4d2c` (sprites, `0x8002f...`) | 10.3% |
| - after the object hook `0x80010000` (mostly `0x80010010`, below) | 7.7% |
| - after `0x8002bc18` | 6.1% |
| - after the room lift `0x800a2690` | 5.4% |

A host CPU profile of the three workers (`PSX_PROF_WORKERS`, call stacks)
agrees: our hook work about 50%, the game's code and its emulation about 43%,
the image load 4%.

Then the question that bounds the whole of D23N: how much of the game's code
in a later job does the same as in the earlier job? Over 22 pairs with the
same hook sequence (CONFIRMED):

- game code in intervals whose stores and the next hook's CPU state are
  identical in both jobs: **5.9%** of a job;
- game code in intervals where something differs: **40.9%** of a job;
- hooks entered with identical CPU state: 11.7% of a job.

So a later job cannot reuse much of the earlier job as it is: nearly all of
the game's code that runs in a job is camera- or fraction-dependent and must
be recomputed. The saving D23N can make is the difference between running
that code emulated and computing the same results in host code. The store
PCs name the routines (about 20, CONFIRMED): the transform `0x800292a0` and
its maths leaves (`0x80010d60`, `0x800b444c`, `0x800b41dc`), the game's
object renderer vertex pass `0x80010010`, room transforms (`0x80029660`,
`0x80029814`, `0x800297ec`, `0x800298e8`), the room lift family
(`0x8002df20`, `0x8002df64`, `0x8002dce0`), sprites and particles
(`0x8002fa48`, `0x8002f450`, `0x8002f900`, `0x800b51cc`, `0x80062258`),
`0x80030fe8` and the small models (`0x8002d848`).

One of them is dead work (CONFIRMED in `near_clip.cpp`): since unified
geometry the host takes every polygon of an object, but the object renderer
still receives the mesh copy with no polygons left and projects all its
vertices (`0x80010010`, 2,900 stores a job). The world renderer was given a
null mesh for the same case in D17AQ; the object renderer was not. That is
most of the 7.7% above, in every worker job and in the live frame. It is on
the board as its own job (D23T), not fixed here.

## 2. Piece 1: the object transform `0x800292a0`

The routine, from its disassembly (CONFIRMED):

1. `d = matrix.t - camera.eye` (camera +0x14), stored at `0x1f800320`; when
   any component is 32768 or more from the eye (toward zero), all three are
   halved toward zero and stored again;
2. `ApplyMatrixLV` (`0x800b444c`) with the view rotation (camera +0x20): each
   component split into `v / 32768` and `v % 32768` toward zero, MVMVA with
   sf 0 on the high parts and sf 1 on the low parts, result
   `low + (high << 3)`, to `0x1f800108`; halved: doubled in place;
3. the result to the translation of `0x1f800148`;
4. `0x80010d60`: the 3x3 transposed into `0x1f8001e8` (pad half kept);
5. `MulMatrix0` (`0x800b41dc`): `view x matrix^T`, MVMVA sf 1, IR saturated
   to 16 bits, column by column, into `0x1f800148` (the fifth word is IR3
   sign-extended);
6. `SetRotMatrix` / `SetTransMatrix` from `0x1f800148`; returns the
   translation's z.

`xf::compute` in `recomp/src/ttk/redraw_n2.inc` is a pure host port of all
six steps. The check (`DNTTK_N2_CHECK=1`) runs it at every top-level call in
every job, from the call's inputs as the routine reads them (after our own
callbacks, which may substitute the matrix), and compares every word it
predicts against the routine's own stores (taken from the record by PC: the
routine and its three leaves) and, when the object renderer is the next
hook, against the GTE rotation and translation that hook receives.

## 3. Piece 1b-1d: a later job's transform inputs

`DNTTK_N2_CHECK=2` predicts each call's camera and matrix in a later job of a
frame from the earlier job's call of the same index and the later job's own
start state (memory after its camera and pose patches, its substitution
table):

- **camera** (piece 1c): the composition writes the camera block once
  (`0x8002e4dc`..`0x8002e52c`): the eye (+0x14) and the view translation
  (+0x34) from `0x80039c7c(camera, out)`, a pure leaf of the camera block's
  anchor, distance and rotation. The rotation changes within a job only
  through our own host code (the late first-person body camera switched in
  for Duke's own draws, `replay_body_camera`). Prediction: the game's
  `0x80039c7c` run at the job start on a scratch CPU with a private stack,
  and the job's two rotations (world, body);
- **matrix**: an object transform the redraw substitutes (D17AY blended
  `Frame::Xf`) is the later job's table entry for the same object and call
  index; everything else is classed by who builds it.

For actor joints (the actor draw `0x800348d8`, ra `0x80034c6c`) a separate
check reads the actor's matrix table entry (actor +0x3c plus the joint
offset): in about 46,000 calls in first and third person, every entry was
unchanged from the job start (CONFIRMED). Third person passes the entry
directly (predicted); first person passes a stack copy that the game scales
(`ScaleMatrix` `0x800b4b5c`) and our D36 presentation code adjusts in its
hooks, so it is predicted by re-running our own code on the entry.

## 4. Pieces 2-3: rooms and visibility

Our host room walk (`host_vis_walk`, `host_visibility.inc`) draws every room
piece (25% of a job). Its inputs (CONFIRMED in code): the camera block
(rotation, eye, source room +0x90), the level's room and piece tables, the
view rectangle `0x800d2210`, the GTE projection (H, offsets), and one
camera-dependent game output, the portal walk `0x8006276c`'s list of rooms
seen through portals (`0x800d68a8`, `0x800d68b0`).

`DNTTK_N2_CHECK=4` predicts that list at a later job's start: the game's own
portal walk on a scratch CPU, after doing what the composition does before
the call (the derived eye, the visited bitmap `0x800e1f10` and the list
cleared, the context fields +0x6c..+0x78 and the view rectangle as the
earlier job had them at its call; the widescreen reveal widens the rectangle
during the composition), with every write of the scratch run logged and
undone afterwards. At the job's real room walk entry the check compares the
list and every input of our host walk.

## 5. Results (CONFIRMED)

Offscreen, 180 Hz, the player's profile copy, private card copies; 30 s of
walking, turning and firing per view, all three checks on
(`DNTTK_N2_CHECK=7`).

| View | Transform calls (all jobs) | Host port exact | Next-hook GTE checked / exact | Halved calls | Later-job calls: camera predicted | Later jobs: portal list and host walk inputs exact |
| --- | --- | --- | --- | --- | --- | --- |
| UI slot 2 | 173,243 | all | 17,804 / all | 12,260 | 173,243 of 173,243 | 3,066 of 3,066 |
| UI slot 8 | 495,961 | all | 17,840 / all | 0 | 495,961 of 495,961 | 3,001 of 3,001 |
| UI slot 9 | 306,624 | all | 18,004 / all | 0 | 306,624 of 306,624 | 3,046 of 3,046 |
| UI slot 2, third person | 159,147 | all | 17,619 / all | 12,949 | 159,147 of 159,147 | 3,077 of 3,077 |

Matrices of later-job calls (pairs with the same number of calls):

| View | Calls | Substituted, predicted | Unchanged since the job start | Built in the job | Camera and matrix both predicted |
| --- | --- | --- | --- | --- | --- |
| UI slot 2 | 87,163 | 17,914 of 17,914 | 447 | 68,802 | 12% |
| UI slot 8 | 230,707 | 148,417 of 148,417 | 957 | 81,333 | 29% |
| UI slot 9 | 153,353 | 85,682 of 85,742 | 3 | 67,608 | 19% |
| UI slot 2, third person | 81,670 | 9,821 of 9,821 | 57,722 | 14,127 | 83% |

- The transform's host port is exact: about 1.13 million calls, 71,267 GTE
  states, none different, including about 25,000 halved translations.
- The camera of every transform call of a later job is known at the job's
  start.
- The visible room list of every later job (12,190 jobs) is known at its
  start; the 22-30% of turning pairs that entered other hooks (note 455)
  differ below the room level (objects and the game's own culling), not in
  the rooms.
- Not yet predicted: matrices the game or our presentation code builds during
  the job (first-person Duke and his weapon `0x800388e4`, `0x80098204`,
  `0x8002b068` / `0x8002b188` / `0x800b3e5c`), and in UI slot 9, 60 of 85,742
  substituted transforms (0.07%) that the (object, call index) rule did not
  match (cause not established; a repeated key in the substitution table is
  the likely one, INFERRED).

With the checks off nothing of this runs; the D23N1 same-job check is still
exact with the extended record (992 of 992 jobs, UI slot 2).

## 6. What it means for the programme (INFERRED from section 1)

- A rebuilt later job still has to compute the camera-dependent results of
  game code that takes about 41% of a job. Done as host code (the transform
  port is a few dozen multiplies a call, against up to 0.24 ms a job for the
  emulated routine and the caller code after it) that is where D23N saves;
  done by running the game's routines again it saves almost nothing.
- Pieces 1-3 are exact and cheap, and they settle the largest structural
  question: rooms are a function of the predicted camera and static data.
- The remainder (D23N2B on the board): the matrices built in the job, object
  culling, the camera-dependent hooks (water and fills, small models and
  decals, the room lift, the sky, the HUD), the game's sprite and particle
  code, and the rebuild of a whole later job checked word for word.
- D23T (the dead vertex pass) is independent and cheap, and would also make
  every job and the live frame lighter.

## 7. Using it

```
DNTTK_N2_CHECK=7 python3 recomp/tools/local/run.py
```

The lab passes it as `LAB_DNTTK_N2_CHECK=7` (`recomp/analysis/d23k/lab.py`,
local). Results are `[n2 check <pid>]` lines every 64 jobs per worker.
`DNTTK_N2_DIFF=1 DNTTK_N2_TRACE=path` writes record pairs (the analysis
scripts used here are local, not tracked).
