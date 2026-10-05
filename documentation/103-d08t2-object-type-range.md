# D08T2 - Pushable objects in every era (object type range)

Status: **Accepted** (2026-10-05): D08T2, and D08U1 (slot-12 ladder) after the mount-blend fix below. Baseline `4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`. Candidate executable SHA-256
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`
(after the retest fixes below; first candidate `06c1f8b1...`).

## Report

User, D22B game-wide playtest: blocks marked with the Duke symbol cannot be
pushed with Modern controls (Roman area, medieval area), but push with Legacy
controls. Reproduction: UI slot 1 (savestate file 00, SHA-256
`ab8d2e156b6bdaf802f7a454449a07eaa8a47f530f99108a96fb0b8abefcbc72`), first
medieval level, block directly ahead.

## Cause

The block is an ordinary original pushable object: object `0x801df0e4`,
**type 924**, type flags `0x081890e2` (pushable `0x08000000` and climbable,
like the alley dumpster's `0x081090e2`). The original grab rules (D08T,
[note 64](64-d08t-pushable-objects.md)) apply unchanged.

`object_type_flags()` in `recomp/src/ttk/push.inc` refused any type `>= 512`,
an arbitrary bound chosen when only the first map (dumpster type 165) was
known. Every Modernized object feature reads it, so for those types:

- Grab / push / pull (D08T, D08T1) never saw a pushable object;
- the top-of-ladder mount (D08U, `ladder_top.inc`) never saw the ladder;
- the hold-E ledge/crate mantle (D08X, `ledge_reach.inc`) never saw the object.

The real size comes from the original: `0x8001b634` allocates the type table
with `0x7428` bytes and stores it at `0x800d2660`; `0x7428 / 28 = 1062` types.
The bound is now `object_type_count = 0x7428 / 28`.

The type table is one global table (identical after `level N` in every level
surveyed: LEVEL00, 03, 07, 10, 21, 24, 27). Types at or above 512 that were
invisible:

| Flag | Types in the table | At or above 512 |
| --- | --- | --- |
| pushable `0x08000000` | 133, 137, 165, 271, 568, 651, 924, 979 | 568, 651, 924, 979 |
| ladder `0x200` | 19 | 521, 575, 639, 752, 828, 850 |
| climbable/mantle `0xc0` | 197 | 114 |

## Evidence

Private copies only (`recomp/analysis/d08t2-20261005/`), real xdotool keys on
a private Xvfb display, Modernized, port 9251. A/B: the same build with the old
bound restored, then the fix.

| Check | Old bound | Fixed |
| --- | --- | --- |
| UI slot 1 block, W to contact, hold RMB, W, release (third person) | no grab, block moved 0 | stowed weapon, grab 121, block moved 1129, weapon redrawn on release |
| Same, first person | - | grab 121, block moved 1126 |
| E alone at the block | nothing | nothing (E still never grabs) |
| D08U1 UI slot 12 ladder (file 11, type 639), E then S | nothing | mounts (156/186), climbs to the bottom |
| D08T1 alley dumpster suite (third person, view aim) | - | all cases as accepted: W+E mantles, E alone nothing, grab push/pull, release, walk-in grab, E while grabbing, pause, grab against nothing |
| Vanilla original push of the dumpster | - | grab, push +1185, pull -936 (accepted: +1185 / -923) |

Native: `ttk-controls-test` gained type 924 and 1061 (accepted) and 1062
(outside the table, refused) cases; `ttk-input-test`, `ttk-aim-test` pass;
`level_overlay_guards.py --check` matches. Codegen hash unchanged (no
generated or framework change).

## Limits

- Only the medieval block was pushed live; the Roman-area block and the two
  near the start of the medieval area were not reached offscreen. The fix is
  in the shared type lookup, so every pushable type now qualifies.
- 114 more climbable types and 6 more ladder types now reach the D08X mantle
  and D08U ladder-top features. That is the intended game-wide coverage, but
  only the slot-12 ladder was exercised; watch for unexpected mantles.
- `cheats.inc` (monster toggle) still refuses types `>= 1024`; not changed.

## Retest fixes (2026-10-05)

**Hints.** D08T1 showed `HOLD RMB TO GRAB` only on the first two touches and
`W/S PUSH/PULL - RELEASE RMB TO LET GO` on the first three grabs of a session
(D08U `E TO CLIMB DOWN`: first two ladder tops). With the medieval blocks now
pushable, the user saw them once and never again. Each now shows on every
fresh touch, grab or ladder top, at most once per 300 input frames (about
5 s), and logs `[TTK input] Hint: ...`. Live: three touches and two grabs
logged five hints.

**Slot-12 ladder bottom (D08U1).** The ladder ends about 1000 units above the
floor. At the lowest rung `0x8007ded0` (is there ladder 500 below Duke?) fails
and the original goes 156 -> 211 -> 207, hanging from the bottom rung; the
original pad alone does the same. From 207, original Square lets go (108 fall,
landing) and Up climbs back; Down only flips 211/207 with Duke's body in the
floor, the reported glitch. `ladder_bottom_hang()` (207/211, mode 3/3, plain
ladder attached) makes S send Square in 207 and neither Down nor Square in
211. Real keys, third and first person: S held from the top reaches the floor
and Modernized control returns, weapon redrawn. The D08U sewer ladder still
ends with the original 185 step-off.

**Next:** D08T3 (free manual push and pull) records the request for
continuous manipulation instead of one fixed shove.

## D08U1 second fix: the ladder bottom decided before the swing

The first fix above only made S drop out of the 207 hang; the 156 -> 211
sideways swing still played, which the user rejected. The 188/189 case at
`0x80044128` picks the hang from the descent flag `+0x6a` and `0x8007ded0`
alone, in the same update, so a pad change is too late. The player update now
asks `0x8007ded0` itself (isolated original call, touch fields restored) on
plain ladders (186..189). True while stepping down: `+0x6a` is cleared and the
original stops at this rung; on the last rung S sends the original Square
let-go (fall 108, landing). A 300-unit lookahead was tried first and rejected:
on the sewer ladder its probe point fell below the floor and replaced the 185
step-off with a drop. Real keys: slot 12 hold and tap S both reach the floor
without the swing (third and first person); W from the last rung climbs out at
the top; the sewer ladder still steps off with 185; the alley climb is
unchanged. Candidate `cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.

## D08U1 final cause: the mount blend cut short (accepted)

The user's `[TTK ladder]` session lines showed the mount ending at y -9114
(offscreen runs: -8901, the climbing line) and S then looping the step poses
in place. With dropped frames (scale 4 in the sewer area) the original 156
transfer advances several frames per player update and ends after about 8 of
the host's 12 blend updates. `ladder_top_update()` now finishes the blend while
Duke rests in 186/187 (Up/Down withheld, `ladder_mount_finishing()`), or
applies the remainder at once if a step has started. Reproduced with
`DNTTK_LADDER_MOUNT_UPDATES=18` (diagnostics only): the old code froze Duke at
-9115; the fix reaches -8901 and descends. Other hardening from the same
investigation: a 96-unit probe margin for the last-rung stop, a last-rung
latch, and E's Cross withheld while letting go. User accepted 2026-10-05.
