# 480 - D23N2B and D23N3 on the field laptop: whole later jobs exact, game packets included; D23N4 not switched on

Autonomous session of 2026-11-15 (night) on the field laptop (i7-5600U,
2 cores / 4 threads, Intel HD Graphics 5500). User: "Work autonomously on
the field laptop through this performance chain, in order, as far as exact
matches allow ... 1. D23N2B: finish the rebuild of a later job ... 2. D23N3:
identify the guest routines behind the game's own camera-dependent packets
... 3. D23N4: switch it on where exact ... 4. If D23N4 is on: re-measure
D23Y7 ... and D23AA". Evidence labels: MEASURED, CODE (CONFIRMED by the
disassembly), INFERRED.

## 1. Setup and host

- Host checked first (MEASURED): mains power, battery full, load 0.3, 6.3 GB
  memory available, 0.7 GB swap in use, no backup or update process, last
  package update 3 days earlier, CPU 41 C. CI green on `main` (the three
  latest runs). HEAD `28ef7df`, framework patch applied; `cmake --build
  --preset local-dev` reported everything current.
- Lab `recomp/analysis/d23y6/` (local, untracked), as in notes 476 and 479:
  the player's profile copied (this laptop's: first person, Lighting on,
  live shadows simple, draw distance extended, 2x), the mode in the lab's
  profile copy (`RATE=180`), `LAB_` only for developer switches, private
  card copies (the player's files hash-checked unchanged after every run),
  offscreen (SDL offscreen on the real GPU), case `cad` (walk, turn and
  fire, 8 patterns), UI 2, 8, 9 and 5 (runtime slots 1, 7, 8 and 4). Every
  run's settings line read `Frame rate: 180`. The desktop lab's tools
  (`recomp/analysis/d23n2b/`) are not on this laptop; new here:
  `n2base.sh` (the check in the four views), `sum_n2.py` (sums the last
  report of each worker), `mdis.py` (disassembler, llvm-mc, one marker per
  word so GTE commands keep their addresses: the first version shifted
  every address after a `cop2` word), `callers2.py`, `n3align.py` (aligns a
  dumped job's game packets with the rebuild's and names the store PCs),
  `n3pk.py`, `n3links.py`.
- Code: `recomp/src/ttk/redraw_n2.inc` (developer only: nothing runs
  unless `DNTTK_N2_CHECK` is set), and a counter export in
  `near_clip.cpp`/`near_clip.h` (`near_clip_viewmodel_counts`, read only
  by the check). No framework change.

## 2. Baseline on this laptop (MEASURED)

Note 478's setting (`DNTTK_N2_CHECK=65288`, `DNTTK_N2_WHOLE="1 1"`), 40 s
a view: later jobs host-exact UI 2 231 of 247 (94%), UI 8 107 of 108, UI 9
288 of 292, UI 5 60 of 60, as on the desktop; but whole jobs (game packets
included) only 6, 6, 5 and 2, and 3-9 rebuilds a view ran away (a chain
walked into code memory until the stream overflowed). The desktop's UI 9
had 166 of 169 whole: this laptop's settings draw more of the game's own
packets through the paths below.

## 3. What differed, and the fixes (CODE, MEASURED)

New developer diagnostics (all in the `[n2 check]` report):
`DNTTK_N2_DUMP=dir` writes the first six host-exact jobs whose game packets
differ (both tails, the job's and the earlier job's stores with PCs, the
hook list); the bucket table and bitmap compared at the flip; the weapon
packet accounting; every hook's GTE, the copy's against the job's; the
model records the copy projected against the job's stores.

**A correction to note 478.** The record of the earlier job (`prev`) is
replaced by the current job's at its flip, before the comparison runs, so
every "earlier job" value printed at comparison time (note 478's
`DNTTK_N2_SHOWRA` GTE "earlier", which supported "in the job its GTE
equals the earlier job's" for the transform at ra `0x8003437c`) was the
job itself. The check now keeps the earlier job's GTE list, hook list and
(for dumps) stores at the job's start.

**Bit 65536: the ordering-table link-up in host code (D23N3).** CONFIRMED:
`0x8001fba0` (hooked) and `0x8001fc44` (pass switch, hooked) call
`0x8001f9c4`, which calls `0x8002bb80`: it walks the 64 bitmap words at
`0x800d26a0` (each cleared), 32 buckets of `0x800d27a0` a word, top bit
first, linking each used bucket's tail to the chain so far and clearing
its head, and the chain goes to context +0xc. The copy replayed the
earlier job's link-up stores, which link the earlier job's buckets: any
bucket used by one fraction and not the other broke the chain (the
runaways). Now the walk runs in host code on the copy's buckets after
either hook, and its stores (PCs `0x8002bb80-0x8002bc14`, `0x8001f9e0`,
`0x8001fbc4`) are not replayed. Game-packet differences in UI 9 fell from
about 75,000 items to 530, in UI 2 from 177,000 to 1,600; no overflow.

**Bit 131072: Duke's shadow run alone (D23N3).** The semi-transparent quad
`0x2e` (store PCs `0x80031410-0x80031548`): CONFIRMED, `0x80030fe8`
(camera, object, height; its only caller Duke's draw at `0x8003540c`)
takes the bounds of Duke's listed joints relative to the root joint
through a rotation from the root's facing (VectorNormalS `0x800b3d3c`,
ApplyMatrixSV via `0x80010e38`), makes four corners from them, loads its
transform (ra `0x8003133c`) and projects them with RotTransPers4
`0x800b528c` (RTPT, RTPS, the key SZ3 >> 2 >> 3), then links the quad
(`0x8002bc18`, ra `0x8003154c`). In first person Duke's joints follow the
camera, so the bounds and corners change with the fraction (1-6 units). A
projection-only port was tried first: the corners still differed, because
the bounds do. The copy now runs `0x80030fe8` itself at the first hook
called from inside it (its entry state from that hook's frame: sp +208, a0
s5, a1 s1, a2 at sp +44), our hooks firing inside as in the job, and skips
the source's hooks and stores through the link hook.

**Bit 262144: the sprite queue run alone (D23N3).** The FT4 packets from
the emitter `0x8002cff0` (store PCs `0x8002d09c-0x8002d104`) outside small
models: CONFIRMED, the camera render `0x8002e48c` (camera in s2) calls
`0x8009b750` (camera, `0x800d2210`) at `0x8002e808`, which calls the queue
`0x8009b1c0`; per sprite it builds corner vectors on its frame, projects
them under the sprite's own GTE (libgte `0x800b3a9c`, `0x800b3b6c`,
`0x800b444c`, `0x800b4c9c`, `0x800b4ccc`; `0x8002883c`) and passes the
records to the face dispatcher `0x8002d508`. The queue has no entry hook
(hooks are fixed at recompile time, `mod_function_entry_funcs`; an empty
plugin registered at `0x8009b1c0` never fired, and was removed). The
wrapper's store of ra (PC `0x8009b768`, always executed) marks the call in
the source's stores: the camera render's sp is its address + 8. There the
copy runs the wrapper with the camera from s2 of the last hook the camera
render called, then skips the source's stores of the queue (until the
first store of other code) and its hooks (below that sp).

**Bit 524288: the muzzle flash run alone (D23N2B).** `0x800341e4` (hooked,
ra `0x80035348`) runs from its hook's state; its inner hooks are not called
and only Duke's draw's own stores are replayed until the next outer hook.
Not yet exact: the flash's mesh lands at another depth in the copy (host
vertex z about 511 against 666 in one job): its matrix comes from our D36
weapon matrix, which follows the camera, and the copy's differs. Not
explained; listed below.

**Bit 1048576: the line effect run alone (D23N3).** Mono lines (GP0
`0x40`, colour `045681`, ends 1-3 units apart; UI 8, 2 of 191 jobs):
CONFIRMED, store PCs `0x8008fd80-0x8008ff24` in `0x8008f71c`, called by
`0x8008ffc4` (camera, `0x800d2210`; 48-byte frame; the camera render's
call at `0x8002e7fc`) after its transform (ra `0x80090068`); linked by
`0x8002bc18` (ra `0x8008fdb4`, `0x8008fe80`, `0x8008ff4c`). The copy runs
`0x8008ffc4` from that transform hook (sp +48, the camera in a0), its
hooks not called and of the source's stores up to the next outer hook only
the camera render's replayed. Built after the last run that showed the
lines; the final runs drew none, so it is not verified.

**Also tried and removed:** keeping a GTE the copy computed only while the
source's own GTE at later hooks was unchanged (bit 131072's first use):
built on the wrong "earlier job" values above; no gain (13 -> 15 whole jobs
in UI 2).

**Checks.** The copy's small-model transforms equal the job's at every
`0x8002d848` hook (3,394 of 3,394) and its records' screen positions equal
the job's (18,840 words); weapon packet accounting equal in every
host-exact job.

## 4. Result (MEASURED)

`DNTTK_N2_CHECK=2096904` (note 478's 65288 and 65536, 131072, 262144,
524288, 1048576), `DNTTK_N2_WHOLE="1 1"`, 60 s a view at 180 Hz:

| View | later jobs | host packets exact | whole job exact (game packets too) | hook list differs (skipped) | rebuild / job ms |
| --- | --- | --- | --- | --- | --- |
| UI 2 | 320 | 299 (93%) | 299 | 12 | 7.02 / 17.90 |
| UI 8 | 227 | 223 (98%) | 223 | 21 | 8.33 / 19.70 |
| UI 9 | 437 | 431 (99%) | 431 | 7 | 4.91 / 15.29 |
| UI 5 | 258 | 255 (99%) | 249 | 2 | 8.78 / 21.59 |
| UI 5 again | 288 | 284 (99%) | 284 | 4 | 8.39 / 19.53 |

The run before it (without bit 1048576): UI 2 323 of 339, UI 8 189 of 193
(the two line jobs), UI 9 429 of 435, UI 5 280 of 283, all whole-exact
but those two. Before this session (section 2): whole jobs 6, 6, 5 and 2.
In every host-exact job the game's packets are now identical word for
word, but six jobs in UI 5's first final run, all in one worker: the job's
chain held host arena packets (`0x34`) where the rebuild's held the game's
(the buckets at the flip differ in 3); not seen in the 60 s repeat or in
any earlier run of the session, cause not established.

The roots left (each job's first differing hook): the muzzle flash (ra
`0x80035348`, 2-21 jobs a view, while firing) and the weapon mesh's object
draw (ra `0x80033f44`, 0-3 a view: the copy's GTE for the weapon's scaled
stack matrix is off by a few units; the job's is close to the earlier
job's). Both come from our D36 weapon matrix, which follows the camera,
being copied by the game's code onto its stack.

Times (MEASURED; per job, averaged per worker; the job's includes the
check's recording, so both are upper bounds): the rebuild is 31-43% of the
job. Of a rebuilt job the routines run alone cost: the portal walk
0.43-0.74 ms, Duke's shadow 0.20-0.26 ms, the sprite queue 0.11-0.21 ms,
the flash under 0.02 ms. A host port of the portal walk (still open) would
save at most the first.

Where later jobs' hook lists leave the earlier job's (the skipped 1-8%):
water tiles against a grid vertex (`0x8002bc18`/`0x8002fbb4` against
`0x800b4d2c`/`0x8002f508`), the portal walk's recursion
(`0x8006276c`/`0x80062ae4` against `0x800b4d2c`/`0x80062364`), actor and
prop draws (`0x80010000`/`0x80033778` against `0x800292a0`/`0x80033584`;
`0x80010000`/`0x80032288` against `0x8001ca4c`/`0x80037614`), a small model
against a sprite face, the shadow's link against the next object. All are
camera-dependent culling: the later job draws a face, tile, room or object
the earlier one did not, or the reverse.

## 5. D23N4: not switched on (INFERRED from section 4)

A switched-on rebuild replays the earlier job's hook list. Section 4's 1-8%
of later jobs (1-8% a view: UI 8 the most) have another list, and a rebuild cannot tell without running
the job (in the check the job's own list is the reference). Those jobs
would show the earlier fraction's culling: an object, room or water tile a
frame late or early. With the two weapon roots, 2-10% of later jobs would
change. So D23N4 stays off, as the user's rule says, and nothing that
depends on it was measured: no A/B at 120 and 180, and D23Y7 and D23AA not
re-measured (step 4's condition). What it needs: a prediction of a later
job's culling before its rebuild (the portal walk's rooms already run for
the job's camera in the copy; objects, faces and tiles would need the same
tests run early, or a rebuild that follows the game's culling decisions
rather than the earlier job's list), the weapon attachments built from the
copy's weapon matrix, and a fallback to the full job for whatever stays
uncovered.

## 6. With the check off (MEASURED)

Nothing of this runs without `DNTTK_N2_CHECK` (no `[n2 check]` line in
the runs below); outside the check only a counter was added to
`near_clip.cpp`.

- Render-thread validator (`PSX_RT_VALIDATE=1`, case `perf2` turning, 40 s,
  180 Hz): UI 2 309 of 309 images exact, UI 8 232 of 232, UI 9 316 of 316.
- Vanilla (`--vanilla`): matched shots of UI 8's and UI 2's savestates at
  frames 60, 200 and 400 byte-identical to note 479's shots of the same
  base build.

## 7. Reproduce

`recomp/analysis/d23y6/` (local): `MASK=1048328 SECS=60 ./n2base.sh`
(`LAB_DNTTK_N2_CHECK=$MASK LAB_DNTTK_N2_WHOLE="1 1"`, UI 2, 8, 9, 5 at
180), then `python3 sum_n2.py n3f1-s1.log n3f1-s7.log n3f1-s8.log
n3f1-s4.log`; `EXTRA="LAB_DNTTK_N2_DUMP=$PWD/dir"` and `python3
n3align.py dir/*.bin` (or `n3pk.py FILE N`) for the packets that differ;
`EXTRA="LAB_DNTTK_N2_SHOWRA=<hex ra>"` for a root hook's words.
