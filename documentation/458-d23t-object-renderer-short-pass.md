# D23T - The object renderer no longer projects meshes the host emptied

Status: built and verified (2026-11-12), Needs review. Job board: D23T.
Found in D23N2 ([note 456](456-d23n2-transform-camera-visibility.md)
section 1). Evidence labels: CONFIRMED (our measurement, code or the game's
disassembly), INFERRED.

## 1. The dead work

Since unified geometry (D17AM3) our host code (`near_clip.cpp`, `object()`)
draws every face of a prop or actor joint, and hands the game's object
renderer `0x80010000` a mesh copy holding the faces it left. With none left,
that copy has an empty polygon list, but the routine still runs its vertex
pass over every vertex (CONFIRMED, disassembly):

- `0x80010070`-`0x800101f4`, three vertices an iteration: LWC2/MTC2 of the
  vertices into VXY0-2/VZ0-2, RTPT (`0x8001009c`), the SXY and SZ results read
  back, bit 20 set when SZ < H (H read by CFC2 at `0x800100cc`), bits 16-19
  for the clip rectangle under context flag `0x80`, and six words written to
  the vertex buffer at render context +0x10 (the scratchpad, `0x1f800000`);
- then the polygon pass finds the empty list (`0x80010218`) and the epilogue
  stores the unchanged packet cursor (`0x80010cd0`) and restores the 14 saved
  registers.

The world renderer got a null mesh for the same case in D17AQ; the object
renderer did not.

## 2. What the pass leaves, and what reads it (CONFIRMED)

What the whole pass leaves behind: the GTE data registers VXY/VZ, IR0-3,
SXY0-2/SXYP, SZ0-3, MAC0-3 and FLAG; the vertex buffer; its register save
area below the caller's stack pointer; the cursor store; and v0 (the cursor),
v1 (arena end), t0-t2 (the last triple's VZ words), t3/t4 (the clip
rectangle, under flag `0x80`), t5 (0), t6 (the buffer pointer) and t7
(context +0x54). The code right after the 16 call sites in the executable
does not use v0/v1/t0-t7, but several of them return straight to their own
callers, so the question was answered dynamically with a new developer check
(section 4), in two steps.

**A null mesh is not exact.** With every one of those leftovers overwritten
with pseudo-random words after each skipped pass (check mode 4), the GP0
streams of 1,632 jobs in UI slot 2 were still identical (the picture never
depends on them), but 1,542 jobs stored a different word at PC `0x80010dbc`,
in every job the hook entries after the call saw different registers, and
RAM differed at the flip. `0x80010d60` copies a 3 x 3 matrix with its pad
halfword; callers build that matrix with halfword stores and leave the pad as
whatever a register or the scratchpad held, so the vertex pass's leftovers
reach RAM through the pad.

**The scratchpad slots are read.** A shortened pass (below) without the host
fill left 5% of jobs different in one word: the game builds a matrix at
scratchpad `0x1f800040` (its m22 halfword stored by `0x80097b78`) without
writing its pad at `0x1f800052`, which is vertex 10's SY in the object vertex
buffer; `0x80010d60` copies that pad into the matrix at `0x800d6a50` (field
+0x20 of the structure at `0x800d6a30`, used by `0x800987cc`, `0x80098204`
and `0x8009d514`). In the original game that pad is whatever the last object
pass left there; nothing in the job read it back (stream and every later
store identical), but RAM differed.

Both findings decided the design: the leftovers must be the same, not just
unread.

## 3. Change

`near_clip.cpp` (`object()`, `object_null_mode`), no framework change. For
an object whose every face is host-drawn, the routine gets a copy of the mesh
header with:

- the vertex count and pointer of the **last two vertex triples** only (one
  triple when the mesh has three vertices or fewer), and
- an empty polygon list.

Its own vertex pass then runs one or two iterations instead of all of them.
Everything the whole pass leaves is decided by its last two iterations (SZ0 is
the previous RTPT's SZ3; FLAG, IR, MAC, SXY and VXY are the last RTPT's; the
registers come from the last triple and the context), so the GTE registers,
their PGXP state, v0/v1/t0-t7, the save area and the cursor store are those
of the whole pass, by construction: the same guest instructions compute them.

The two iterations write their triples into the first two slots of the
buffer. Our host code writes every later slot as the whole pass would: SXY
and SZ from the exact host projection `near_project` (already computed for the
host draw; recomputed with the GTE's own H for limbs drawn with a fixed
projection, and for the one or two padding vertices the pass reads past the
count), bit 20 when SZ < H, and the rectangle bits under flag `0x80`. Only
buffer slots 0-5 then differ from the whole pass's (they hold the last
triples' values instead of the first's), and nothing reads them (section 4).

`DNTTK_OBJECT_NULL` (developer): 1 (default) as above, 0 the whole pass
(matched comparisons), 3 a bare null mesh (the cost bound; not exact).
`near_clip` debug JSON gains `object.null` (shortened objects).

## 4. The check (CONFIRMED)

`recomp/src/ttk/redraw_t_check.inc`, included by `frame_replay.cpp`, built on
the D23N1 tools ([note 454](454-d23n1-capture-and-rebuild.md)). For every
Nth gameplay redraw job (`DNTTK_T_CHECK=N`) the worker forks at the job's
start. The copy runs the job with the whole pass (mode 0) and records its
stream into shared memory; the job runs it in the new mode. Both log every
top-level store of the game's code (address, value, size, PC) and the CPU
state at every top-level hook entry. Compared after the submit:

- the GP0 streams, item by item, every word, PGXP records included;
- the store logs, every store, without the pass's own stores (PCs
  `0x80010014`-`0x800101ec` and `0x80010cd0`) on both sides;
- the hook sequence and every register (GPR, GTE data and control, HI/LO) at
  every hook entry;
- at the flip: CPU state, main RAM, scratchpad (buffer slots 0-5 allowed,
  every later slot must match) and mod memory.

`DNTTK_T_CHECK_START=1` also poisons, at the job's start, every vertex buffer
range seen in earlier checked jobs, to show the composition never reads what
the previous frame left there. `DNTTK_T_CHECK_MODE=4` runs the poisoned null
mesh of section 2.

Offscreen, 180 Hz, the player's profile, private card copies, every third
job, walking, turning and firing:

| View | Jobs | Exact | Objects shortened | Pass stores saved a job |
| --- | --- | --- | --- | --- |
| UI slot 2 | 1,632 | 1,632 | 29,751 | 948 |
| UI slot 8 | 2,112 | 2,112 | 140,978 | 1,858 |
| UI slot 9 (level 3 corridor) | 2,240 | 2,240 | 75,488 | 1,248 |
| UI slot 2, third person | 2,208 | 2,208 | 55,111 | 1,221 |
| UI slot 9, buffers poisoned at job start | 1,632 | 1,632 | 46,928 | 1,100 |
| UI slot 2, buffers poisoned at job start | 1,616 | streams, stores, hooks and flip state all identical | 29,288 | 947 |

Every job exact: 9,824 jobs, about 132 million compared stores, 2.1 million
hook entries with identical registers, flip states identical; at the flip the
only differences are buffer slots 0-5 and 3 bytes of the mesh copy's header in
mod memory (its count and vertex pointer). In the last row the leftover start
poison stays in buffer ranges the job never draws over, as expected; nothing
read it.

On the final build of the session (with D23U's code, off by default):
`PSX_RT_VALIDATE` 691 of 691 render-thread images exact, Vanilla screenshots
byte-identical to the session's starting binary, `ttk-input-test` and
`ttk-near-test` pass.

## 5. Cost (CONFIRMED)

Same binary, `DNTTK_OBJECT_NULL=0` against the default, alternating, turning
at 180 Hz (`SPIN=dx=3`), offscreen, 20 s each, three runs (UI slot 2: one
baseline run that fell to 159 presents a second is set aside):

| View | Workers (3 processes) | Emulation thread |
| --- | --- | --- |
| UI slot 2 | 163-165% -> 161-164% | 62% -> 63% |
| UI slot 8 | 142-145% -> 141-144% | 66-67% -> 66-67% |
| UI slot 9 | 118-119% -> 116-118% | 60-61% -> 60-61% |

Per job (`DNTTK_REPLAY_PROFILE`, about 2,550 jobs a run, two runs each):
UI slot 2 7.71 / 7.65 -> 7.60 / 7.51 ms (0.10-0.15 ms, about 1.6%), UI slot 9
6.16 / 6.16 -> 5.97 / 6.01 ms (0.15-0.19 ms, about 2.7%).

So the saving is real but small: 1-2 points of a core in all, about 0.1-0.2
ms a job. Note 456 put the game code after the object hook at 7.7% of a job;
that figure was measured with every top-level store logged by the D23N
recorder, which inflates a loop that is almost all stores (INFERRED: its
~2,900 stores a job each paid a log write). The emulation thread is unchanged:
with redraw-only live frames (D17AR3) its own frame draws little.

## 6. Limits

- Vanilla is unchanged: `object()` runs only while the eye view, the third-
  person camera or the widescreen near clip is live, all of which require the
  Modernized profile.
- What remains of the object renderer's cost in a job is our host draw
  itself (16.7% of a job, note 456) and the game code around it; D23N
  (captured redraws) is the large CPU saving.
