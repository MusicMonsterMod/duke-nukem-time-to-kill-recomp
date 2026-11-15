# 477 - D23N2B pieces 6-8: a whole later job rebuilt from its start (desktop)

Autonomous session of 2026-11-15 (afternoon) on the desktop (i7-5960X,
8 cores / 16 threads, GTX 1080 Ti). User: "work on these performance jobs,
in this order, autonomously: 1. D23N2B piece 6: run 0x80098204 after a later
job's pose patches and get 0x80033584 exact, then the rest of D23N2B.
2. D23N3, then D23N4 as far as exact matches allow; if N4 switches on,
measure ...".

Method: a new desktop lab `recomp/analysis/d23n2b/` (local, untracked; the
field laptop's `d23y6` lab is not on this machine) built from the D17BH
lab: the player's cards and savestates copied (hash-checked identical, the
player's files hash-checked unchanged after every run), the player's profile
copied with the mode set in the lab's profile copy (`RATE=180`, never a
`LAB_` switch for a profile setting), each run's settings line checked
(`Frame rate: 180`, first person, RELIT, Ultra), offscreen (SDL offscreen on
the real GPU), `LAB_` only for developer switches (`DNTTK_N2_CHECK`,
`DNTTK_N2_WHOLE`, `PSX_RT_VALIDATE`). Case `cad`: load a runtime slot, then
walk, turn and fire (8 patterns, 2.5 s each) for 30-50 s. The check lines
are cumulative per worker; `sum_n2.py` sums each worker's last line per
slot. Views: UI 8 (runtime slot 7), UI 2 (slot 1), UI 9 (slot 8), UI 5
(slot 4). Evidence labels: MEASURED, CODE, INFERRED.

Everything here is developer code in `recomp/src/ttk/redraw_n2.inc` and
`redraw_check.inc`; nothing runs unless `DNTTK_N2_CHECK` is set. No
framework change.

## 1. Piece 6: the actor joint table (MEASURED, CODE)

Note 476 found that every `0x80033584` entry is rewritten in the job by
`0x80098204`. Where it is called (CONFIRMED by the disassembly): the actor
draw `0x80032e78 (camera a0, actor a1 = s3)` gives an actor without a table
the shared one (`0x800d6a30`, at `0x80032f9c`), then builds it at
`0x80032ff8`: `0x80098204 (actor, table)`, or the plain build `0x800987cc`
when the actor type's word (`*0x800d2660 + 28 * type`) has `0x20000`, the
actor's `+0x60` is `0xdc` or its flags have 2; a nonzero return leaves the
draw. Then `0x80032364`, `0x80032bd8` (a top-level hook, ra `0x80033020`)
and `0x80032da4`, which write only the actor's own fields, and the joint
loop at `0x80033160` (the `0x80033584` transform). `0x80098204` takes the
joint rotations from the animation through `0x80010d60` and the
translations as joint offset plus the actor's position, and also stores the
root's height into `actor +0x10`.

New `DNTTK_N2_CHECK` bit 256 (`jr::rebuild_table`): in the copy forked at
the job's start, just before the actor draw's `0x80032bd8` hook, the same
build is run on a private stack after the job's own pose patches.

| View | `0x80033584` calls exact, before (bits 16+64) | with bit 256 |
| --- | --- | --- |
| UI 8 | 6,337 of 9,830 (64%) | 13,058 of 13,058 |
| UI 2 | 7,259 of 7,293 | 7,956 of 7,956 |
| UI 9 | (no calls) | (no calls) |

Scaled copies (ScaleMatrix on the stack) included. 802 / 468 tables built
by `0x80098204`, 31 by the plain build, none failed.

## 2. Piece 7: the matrices built on the caller's stack (MEASURED, CODE)

The remaining transform callers build their matrix on the stack before the
call (CONFIRMED by the disassembly):

- the actor draw (ra `0x80032f48`, actor s3) and the prop draw (ra
  `0x80031dec`, object s2): `0x8002afe4` of the angles at `obj +0x1c`,
  translation `obj +4/+8/+0xc`; the prop draw then scales by ScaleMatrix
  `(0x1000, half obj +0x22, 0x1000)` unless that half is `0x1000`;
- `0x8003133c` (piece 5's yaw matrix): its height argument is
  `*(s5 +0x204)` of its caller, Duke's draw `0x800348d8` (`0x80035408`); in
  the job it is on the stack, so the copy takes Duke's actor from the
  `0x80032bd8` hook at ra `0x8003492c` (a0);
- the sky (piece 5): `0x800388e4` first advances its timer `0x800c0c64` by
  `*0x800d21fc` unless `0x800be568` or `0x800d2540` is set, so a redraw job
  moves the clouds before drawing them; the copy applies it once a job.

Bit 512 writes these host builds (piece 5's `rot`, `jt::scale`) into the
matrix the call passes, before the hook. Every transform call from the job
start, by caller (bits 8+16+64+256+512, `DNTTK_N2_CHECK=856`; the eye
predicted, bit 8):

| Caller | UI 8 | UI 2 | UI 9 |
| --- | --- | --- | --- |
| props `0x80031dec` | 14,233 / 14,233 | 2,643 / 2,643 | 4,864 / 4,864 |
| actor joints `0x80033584` | 8,405 / 8,405 | 7,276 / 7,276 | - |
| actors `0x80032f48` | 2,376 / 2,376 | 560 / 560 | - |
| `0x8003133c` | 576 / 576 | 608 / 608 | 608 / 608 |
| sky `0x80038c68`, `0x80038d7c`, `0x80038e18` | 576 / 576 each | 608 / 608 each | - |
| Duke's joints (`0x80034c6c`, piece 4b) | 10,368 exact, 0 differ | 10,386 / 0 | 10,944 / 0 |
| rare: `0x8003437c`, `0x80031900`, `0x80033f14`, `0x80090068` | 0 / 21 | 0 / 34 | - |

Baseline without bits 256/512 (same run conditions): props 99.5%, actors
97%, `0x8003133c` and the sky 0%. The matrix's pad half (`+0x12`, caller
stack the transform never reads) differs in about 2,300 calls and is left
out. The four rare callers (about 0.1% of calls) are not ported; a rebuild
falls back to the full job when one appears.

## 3. Piece 8: a whole later job rebuilt (MEASURED, CODE)

New bit 1024 (namespace `wj`): at every 4th later job of a frame (the
worker's previous job was of the same game frame at another fraction) a copy
is forked at the job's start. At the flip it rebuilds the job's stream:
stores of a chosen source applied in order, each top-level hook called with
a chosen CPU state after pieces 6-7, the game's own items between hooks
spliced; then the chain close and the submit from the job's flip state,
like D23N1. The comparison runs after the job's own submit: each hook's
packets against the same hook's in the real stream, word for word (the
game's PGXP records left out, as in D23N1), the game items between hooks,
then the game's packets after the flip. The earlier job's record now also
keeps its stream (bit 1024). Bit 2048 writes the transform's results
(piece 1's exact port) into the scratchpad after each transform hook and,
with the earlier job's registers, its GTE rotation and translation into the
next hooks. `DNTTK_N2_WHOLE="S C"`: stores none (0), the earlier job's (1,
its eye words replaced by the predicted eye) or the job's own (2); CPU
state the job's own (0) or the earlier job's (1, the same hook list call for
call).

Sanity (`"2 0"`): UI 8, 66 of 66 later jobs exact, hooks 2,697 of 2,697,
game items after the flip 195,986 of 195,986.

`DNTTK_N2_CHECK=3848` (bits 8, 256, 512, 1024, 2048), the setting D23N4
would run (`"1 1"`: the earlier job's stores and registers):

| View | later jobs | host packets exact | whole job exact (with game packets) | rebuild / job ms |
| --- | --- | --- | --- | --- |
| UI 9 | 125-143 | 121-122 (85-97%) | 113 (79-90%) | 2.1 / 7.2 |
| UI 8 | 82-95 | 0 | 0 | 3.9 / 12.7 |
| UI 2 | 51-54 | 0 | 0 | 5.1 / 16.1 |
| UI 5 | 96 | 0 | 0 | 4.9 / 14.9 |

(Times are per job, averaged per worker; the job's time includes the
recording, so it is an upper bound. 2-7 of every 100 later jobs had a hook
list that differs from the earlier job's and were skipped.)

Without game stores (`"0 0"`, the job's own registers) only 0-15% of later
jobs are host-exact: the hooks read much that the game's code writes during
the job. With the earlier job's stores most of that is right, and what is
left is camera-dependent game code. Each job's first differing hook (the
roots; after one the packet addresses of every later hook shift):

| Root (hook / ra) | UI 8 | UI 2 | UI 9 | UI 5 | What produces its input |
| --- | --- | --- | --- | --- | --- |
| small models `0x8002bd48` / `0x8002d114` | 67 | 14 | 4 | 71 | the game projects the model in `0x8002d848` and builds faces in `0x8002c000-0x8002d508` (CODE, model_depth.inc); the later job draws faces the earlier did not (128 words against none) |
| water `0x8002bc18` / `0x8002fbb4` | 9 | 25 | 4-12 | - | the game's water grid projection `0x8002ef90` builds the tile packet our `water_clip` reads; corners differ by 1-4 units |
| room walk `0x80062b48` / `0x8002e688` | 12 | 9 | 3 | 9 | our host walk; one stretch about 90% into its 190,000 words differs (cause not established) |
| sky backdrop object `0x80010000` / `0x80038cf0` | 4 | 4 | 1 | 15 | 840 words against 600: the object renderer culls differently for the backdrop (cause not established) |
| Duke's draw `0x80010000` / `0x80034448`, the object pass `0x800374cc` | 2-3 | 2 | 1 | 1 | not established |

## 4. D23N3: the game's own packets in a rebuilt job (MEASURED)

In host-exact later jobs of UI 9, 8 of 121 still differ after the flip: the
game's camera-dependent packets. Two kinds, from the first dumps: a
semi-transparent raw textured quad (GP0 `0x2e`) whose screen corners move
by 2-6 units between fractions (projected by the game's code), and gouraud
textured triangles and quads (`0x34`, `0x3c`) drawn by the later job and not
by the earlier one. This matches note 449 section 7 (one semi-transparent
quad and about two textured quads a job). Their guest routines are not yet
identified; the record's store PCs for those packet addresses would name
them. D23N3 itself was not built.

## 5. D23N4: not switched on (INFERRED from sections 3-4)

A later job can skip the game's code only when its rebuild is exact, and it
is exact only in views without actors, small models and water (UI 9: 79-90%
of later jobs). Switching on now would change pictures elsewhere, so D23N4
was not built, and the 120 Hz CPU clock, emulation thread and cadence, the
DUKE HILL town and the FAMILY JEWELS start were not re-measured (the user's
condition: "if N4 switches on"). What it needs first, in order of how many
jobs each blocks: host ports of the small-model projection and face
emitters, of the water grid projection, the room walk and backdrop
differences explained, then D23N3's two packet kinds.

## 6. With the check off (MEASURED)

Interleaved A/B, the pre-session binary against this one, offscreen at 180,
UI 8, 2, 9, 30 s each, `PSX_RT_VALIDATE=1`, two rounds: game 59.9-60.0 a
second in all; presents per second before 59.6 / 57.1 / 59.9 and 59.7 /
58.0 / 59.9, now 59.3 / 58.0 / 59.9 and 59.1 / 57.7 / 59.9 (these offscreen
runs present near the game rate in both builds); the validator's
differences 0 and 1 in both builds (one full-frame difference at a level
load in the second round of each). No `[n2 check]` lines without the
switch. Vanilla (`--vanilla`) boots and loads a savestate.

## 7. Reproduce

`recomp/analysis/d23n2b/` (local): `RATE=180 OFFSCREEN=1
LAB_DNTTK_N2_CHECK=856 CASES=cad SLOTS=7,1,8 python3 lab.py` (pieces 6-7),
`LAB_DNTTK_N2_CHECK=3848 LAB_DNTTK_N2_WHOLE="1 1"` (piece 8; `"2 0"` the
sanity run), then `python3 sum_n2.py TAG-s7.log TAG-s1.log TAG-s8.log`;
A/B `EXE=d23n2b-before|d23n2b-now LAB_PSX_RT_VALIDATE=1`.
