# 478 - D23T re-checked, D23N2B small models, water, rooms and sky, D23V2 incremental capture (desktop)

Autonomous session of 2026-11-15 (afternoon) on the desktop (i7-5960X,
8 cores / 16 threads, GTX 1080 Ti). User: "work on these performance jobs,
in this order, autonomously, on the desktop: 1. D23T: give the object
renderer a minimal mesh when our host renderer empties one, proven exact,
and measure the job cost before and after. 2. D23N2B: host ports of the
small-model projection (0x8002d848 and its face emitters) and the water grid
(0x8002ef90), each checked exact with the piece 8 whole-job check, then the
room walk and sky backdrop differences, as far as exact matches allow (note
477 section 5). 3. D23V2: build it and prove pictures exact here; leave the
GPU measurement for the field laptop and say so."

Lab: `recomp/analysis/d23n2b/` (local, untracked), as in note 477: the
player's cards and savestates copied (hash-checked; the player's files
unchanged after every run), the mode in the lab's profile copy (`RATE=180`),
each run's settings line checked (`Frame rate: 180`, first person, RELIT,
Ultra), offscreen (SDL offscreen on the real GPU), `LAB_` only for
developer switches. Case `cad`: load a runtime slot, then walk, turn and fire
(8 patterns, 2.5 s each). Views: UI 8 (runtime slot 7), UI 2 (slot 1), UI 9
(slot 8), UI 5 (slot 4). Evidence labels: MEASURED, CODE (CONFIRMED by the
disassembly), INFERRED.

## 1. D23T: already built; re-checked on the current build (MEASURED)

D23T was Done before this session (note 458: built, exact over 9,824 jobs,
cost measured; user-accepted 2026-11-12). Nothing was rebuilt; the current
build was checked again.

- Exact: `DNTTK_T_CHECK=3`, UI 2, 8 and 9, 35 s each: 2,160 checked jobs, all
  exact (stream, every store, every hook entry's registers, the flip state);
  stores of the vertex pass saved about 1.5 million.
- Cost: the same binary, `DNTTK_OBJECT_NULL=0` (the whole vertex pass)
  against the default, interleaved A B A B, `DNTTK_REPLAY_PROFILE` (per-job
  worker time), 30 s per view, about 1,900 jobs each:

| View | whole pass: median / slowest tenth (ms) | short pass: median / slowest tenth | change of the median |
| --- | --- | --- | --- |
| UI 2 | 12.01 / 15.63, 12.04 / 15.76 | 11.72 / 15.29, 11.81 / 15.76 | -0.23 to -0.29 ms (2%) |
| UI 8 | 6.38 / 8.54, 6.69 / 8.83 | 6.04 / 8.29, 6.30 / 8.87 | -0.34 to -0.39 ms (5-6%) |
| UI 9 | 2.96 / 3.09, 2.89 / 3.02 | 2.94 / 3.13, 2.93 / 3.05 | within the spread |

(The slowest tenth is the 90th percentile of job times.) This agrees with
note 458 (0.10-0.19 ms a job then; this build's jobs carry more host work).

## 2. D23N2B pieces 9-13: the camera-dependent hooks of a rebuilt job (CODE, MEASURED)

All in `recomp/src/ttk/redraw_n2.inc` (namespaces `sm` and `wj`),
developer only: nothing runs unless `DNTTK_N2_CHECK` is set. New bits, all
used with piece 8 (1024) and the earlier job's stores and registers
(`DNTTK_N2_WHOLE="1 1"`); `DNTTK_N2_CHECK=65288` runs them all.

**Piece 9 (bit 4096): small models.** CONFIRMED: `0x800a2690` (context,
object) builds a model header on its stack and calls `0x8002e02c` (context,
header, matrix = object +0xc): `0x8002df20` sets the GTE (rotation
MulMatrix0(view, matrix), no transpose; TR ApplyMatrixLV(view, matrix.t -
view.t), no draw when any |d| >> 16 is set); `0x8002dce0` projects the
vertices three at a time into 8-byte records (SXY, SZ >> 2, then the flags
in the high half: 2 left, 1 right, 4 top, 8 bottom, 0x10 near, 0x20 far,
against context +0..+0xa, the padding records too); `0x8002d848` walks the
faces (flag words +4, indices +8, colours +0x10, uvs +0x18; 1-4 corners
from the top flag bits; culls 0x400 any corner 0x30, 0x800 shared flags,
0x200 NCLIP < 2 or > 120000) and calls 17 emitters (`0x8002c028`-
`0x8002d2cc`) whose packets take the records' SXY in standard GP0 layout
(corners in index order) and are linked by `0x8002bd48` at a key (a dot's
record SZ, else `0x8002bdc8`: first, last, mean, least or greatest corner SZ
by flags & 0x7000, less 0x40 under 0x8000; untextured faces with 0x2000000
add a texpage packet at the same key). `0x8002bd48` links into bucket key >>
3 of `0x800d27a0` (head, tail) and the bitmap `0x800d26a0`. In the copy the
transform goes into the 0x8002d848 hook's GTE, the records into memory
(before our model_depth hook, which then validates and edits them as in the
job), the face walk runs after the hook, and each emitter's 0x8002bd48 hook
gets its face's corners written into the packet and its key; every
0x8002bd48 call (all callers) is linked in host code after its hook, and
the game's own link stores are not replayed.

**Piece 10 (bit 8192): the water grid.** CONFIRMED: `0x8002ef90` (box,
clip rectangle) projects grid vertices (x = box +8 + 0x400 i, y = box +10 +
`*0x800c3cd8`, z = box +12, + 0x400 a row, all 16-bit) with RotTransPers3
into 16-byte records (SXY, SZ, flags: 1/2/4/8 against the rectangle, 0x10
SZ < H/2 read through the hooked `0x800b4d2c`, 0x20 SZ > context +0x74), one
row ahead and then row by row; a tile is skipped on shared flags, any 0x10
or s3 = (SZ sum) >> 4 over 0x4000; else an FT4 with the type's colour or,
past context +0x6c, DPCS toward the far colour (`0x800b4dd0`) unless
`*0x800c3d28`; linked by the ordering-table add `0x8002bc18` at s3 >> 3
(key clamped to 0-0x7ff, the same buckets). Both water passes set the GTE
per room with `0x80029660` (camera, room): the camera's view rows, TR
ApplyMatrixLV(view, room - eye), as `0x800292a0` without a matrix. In the
copy: the room transform at the box hook, each triple's positions and
depths at its first vertex's hook, each flag after its vertex's hook (the
game's flag stores not replayed), each tile's corners, colour and key before
its hook, every `0x8002bc18` link in host code.

**Piece 11 (bit 16384): room transforms.** Every `0x80029660` the source
ran (12 call sites; seen by its d = room - eye stores, PCs
`0x80029698`/`0x800296c8`, `0x800296a8`/`0x800296dc`, `0x800296f0`) is
redone for the job's eye: room = d + the source's eye, or the eye itself
when d is (0, 0, 0) (the sky backdrop, `0x80038cbc`: room = camera +0x14,
TR 0); the GTE the next hooks see. This fixed the sky backdrop object (840
words against 600: its TR was the previous transform's).

**The sky's clouds.** Piece 7 advanced the cloud timer `0x800c0c64` once a
job in the copy; with the earlier job's stores replayed that store is
already there, so the clouds moved twice (`0x80038da8`, `0x80038e44`). The
advance now runs only when no stores are replayed.

**Piece 13 (bit 32768): the room walk.** Its difference (one stretch late in
its stream; extra words in the rebuild) is the portal walk's output:
`0x8006276c`, run by the camera render at ra `0x8002e650`, builds the
portal list (`0x800d68a8`/`0x800d68b0`), the visited rooms and the room
rectangles that our host walk reads, and the copy had the earlier job's. The
copy now runs the game's own portal walk there for the job's camera (our
entry hooks fire inside it as in the job, the recursive entries included),
then what the caller does with its result (context +0x74 the far limit
`*0x800da34c` if the walk set its flag, else +0x70; +0x78 a quarter of it),
skips the source's stores up to the room walk hook and gives the next hooks
the GTE the walk leaves. This is game code run in the rebuild, not a host
port: it establishes the cause and an exact equivalent; a host port of the
portal walk is still open.

**Tried and removed:** redoing every replayed ordering-table head store as a
link against the copy's buckets. Once clearing stores (value 0) are
replayed as they are, it found no link to redo: with unified geometry the
object renderer links none of its own faces, and every link goes through
the hooked `0x8002bd48`/`0x8002bc18`.

Results, 70 s per view, the same binary with the new bits off (`3848`, note
477's setting) and on (`65288`):

| View | later jobs host-exact, bits off | bits on | whole job exact (game packets too), on | model face hooks before a job's first difference | water tiles likewise |
| --- | --- | --- | --- | --- | --- |
| UI 2 | 0 of 75 | 84 of 90 (93%) | 2 | 1,979 of 1,979 | 5,446 of 5,446 |
| UI 8 | 0 of 131 | 147 of 148 (99%) | 16 | 1,446 of 1,446 | 1,370 of 1,370 |
| UI 9 | 175 of 177 | 169 of 169 (100%) | 166 (98%) | - | 2,474 of 2,474 |
| UI 5 | 0 of 157 | 145 of 147 (99%) | 16 | 4,757 of 4,757 | - |

Earlier in the session, at each step: small models alone made every model
hook exact (UI 8 665/665, UI 5 2,871/2,871) and moved the first root to the
sky; water with the room transform made every tile exact (3,552/3,552 UI 2).

What is left:

- Host packets: one root, Duke's draw (ra `0x80034448`, 1-6 jobs a view):
  its matrix comes from the transform call at ra `0x8003437c`, one of note
  477's four rare callers: a random yaw (`rand` `0x800bb64c` unless
  `0x800be568` or `0x800d2540`) times a matrix of Duke's, for an attachment
  on Duke's own draw. In the job its GTE equals the earlier job's (Duke's
  own draws use the body camera our D36 code switches in); the copy applies
  the predicted eye. Not ported.
- The game's own packets after the flip (D23N3): in host-exact jobs the
  semi-transparent quad `0x2e` whose corners move 2-6 units and gouraud
  triangles `0x30`/`0x34` from the game's effect code; after the first
  differing packet the chain's addresses cascade, so the item counts are
  large. UI 9 has few: 166 of 169 whole jobs exact.
- A host port of the portal walk (piece 13 runs the game's).

Times: the rebuild is 6-40 ms a job with the check's recording (an upper
bound; with all bits on UI 2's rebuild averages 40 ms because the portal walk
and the host links run in the copy with every hook). D23N4 (switching later
jobs to the rebuild) was not part of this request and was not built.

## 3. D23V2: the incremental capture (CODE, MEASURED)

Framework, `gpu_gl_renderer.c` and `render_thread.h`, exported into
`recomp/patches/time-to-kill-accepted-source.patch` (applies to the pinned
clean framework, all 90 patched files match).

- Every write to the canonical surface already goes through
  `coh_invalidate` (draws, fills, copies, uploads, packs: D23M2's coherence
  list relies on it); it now stamps its 32 x 32 VRAM tiles with the current
  capture epoch. The stencil rebuild and the full reset stamp every tile.
- `gl_rt_capture` copies only the tiles stamped since its snapshot slot was
  last filled (runs along each tile row, `glCopyImageSubData`; the
  depth-stencil too when the capture copies it). A slot never filled, at
  another scale, last filled without depth-stencil when it is now wanted, or
  without `glCopyImageSubData`: the whole surface, as before. The raw mirror
  (1 MB) and D23V's margins-only wide copy are unchanged.
- Developer: `PSX_REPLAY_CAPTURE_INC=0` copies the whole surface always;
  `PSX_REPLAY_CAPTURE_CHECK=N` reads back the slot and a whole copy after
  every Nth incremental capture and counts differing tiles (in the
  `[gpu copies]` line of `PSX_GPU_PHASES=2`).

Pictures (MEASURED, 180 Hz, offscreen, the player's 4x):

- Readback check (`PSX_REPLAY_CAPTURE_CHECK=10`), UI 2, 8 and 9, 30 s each:
  285 captures compared, 0 differing tiles; 245 of 512 tiles copied a
  capture (both of the game's frame bands are rewritten every frame), the
  same as note 469's lab patch on the field laptop.
- Render-thread validator (`PSX_RT_VALIDATE=1`, every 8th render-thread image
  against the same redraw on the emulation thread, RGBA exact), UI 2, 8 and
  9, 40 s each: new 912 of 914, then UI 2 alone 309 of 309 twice; the
  pre-D23V2 binary 919 of 920, 304 of 305, 306 of 306. The differing images
  are the same two in both builds: one full frame at a level load, and a
  small box at the top of UI 2 (221-1081 x 0-150, at most 5 levels), which
  also appears with `PSX_REPLAY_CAPTURE_INC=0` and with the old binary, so
  it is run-to-run variation, not this change.
- Vanilla (no redraws, so no capture): matched shots of UI 8's savestate at
  frames 60, 200 and 400, pre-D23V2 binary against this one: identical.

Cost (MEASURED here, not the acceptance measurement): on the 1080 Ti the
whole capture is only 0.12-0.17 ms of GPU a game frame (`PSX_GPU_PHASES=2`,
interleaved, two rounds): incremental 0.08-0.09 ms in the second round, the
first round within its noise (0.10-0.22). Game rate and presents unchanged
(60 / 59.9-60.0). **The acceptance measurement (capture GPU time at least
halved again, GPU work as busy x clock, the CPU clock and present cadence
at 120 and 180) belongs on the field laptop, where the capture was 1.65-1.70
ms; it was not made here.** Note 469 found no game-frame gain from the same
idea there, so measure GPU work and clock, not only game frames.

## 4. Reproduce

`recomp/analysis/d23n2b/` (local):

- D23T: `RATE=180 OFFSCREEN=1 CASES=cad SLOTS=1,7,8 LAB_DNTTK_T_CHECK=3
  python3 lab.py`, `python3 sum_t.py TAG.log`; cost `LAB_DNTTK_REPLAY_PROFILE=1
  LAB_DNTTK_OBJECT_NULL=0|1`, `python3 sum_q.py TAG-s1.log ...`.
- D23N2B: `LAB_DNTTK_N2_CHECK=65288 LAB_DNTTK_N2_WHOLE="1 1"` (all), `3848`
  (note 477's setting), then `python3 sum_n2.py TAG-s1.log TAG-s7.log
  TAG-s8.log TAG-s4.log`; `LAB_DNTTK_N2_SHOWRA=<hex ra>` dumps a root hook's
  first differing words and the job's, copy's and earlier job's GTE.
  `mdis.py LO HI` disassembles the executable.
- D23V2: `LAB_PSX_GPU_PHASES=2 LAB_PSX_REPLAY_CAPTURE_CHECK=10` (exactness),
  `LAB_PSX_REPLAY_CAPTURE_INC=0|1` (A/B), `LAB_PSX_RT_VALIDATE=1` (the
  validator's totals in the `[cad]` lines), `VANILLA=1 CASES=vshot
  EXE=d23v2-before|d23v2-now` (matched shots).
