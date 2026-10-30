# D08T11 - Pushed objects stay movable (game-wide)

Status: **Done** (user-accepted and closed 2026-10-30). Modernized only; Vanilla unchanged.
No framework, generated-code, media or floor-data change.

## Report

UI slot 8 (saved 2026-10-30): the medieval Duke-symbol block could no longer be
pushed or pulled. The user asked whether every push/pull block could be fixed
at once instead of one by one as play finds them.

## Cause

The block (type 924, object `0x801de56c`) had been pushed 2490 units into a
wall recess. At the tile in front of it, bit 0 of byte 4 of Duke's floor record
is clear (`39004500007f01`), so the original grab gate `0x80051970` refuses,
exactly as at fixed scenery (D08T4). Modernized free push/pull (D08T3) lets
objects travel to places the original grid shoves never reached; the existing
extension (D08T7, D08T9, D08T10) only covered a list of verified lanes, and this
block's lane was not on it. Same failure class as every earlier report.

## Rule

The level's placement rows (`*0x800de720`, 48 bytes, count `0x800de724`, the
table D20E and D26E already read) hold each object's type, live object pointer
and authored x/y/z. They are not rewritten when an object moves (slot 8: row 287
still says Z 32255 while the block is at 34745). So:

> A pushable object whose current x or z differs from its authored placement has
> been pushed, and may be grabbed along that axis wherever it now stands.

Fixed scenery can never be pushed, so it never qualifies: the LEVEL05 street
dumpster stays fixed with no list entry. Height-only differences grant nothing.
The row must point at the object and carry the same type. The verified lanes
remain for objects still at their authored spot on a no-grab floor.
Everything after eligibility is unchanged: the original line-up, the target
facing on the allowed axis, the stationary support/clearance check, manual
travel, collision, the D08T5 ladder limit, sound and Vanilla gating.

Implementation: `push_displaced_axes()` and `push_mobile_axes()` in
`recomp/src/ttk/push.inc`; `push_lane_allows()` and `push_lane_acquire()` use
the combined axes.

## Evidence

Private copy of UI slot 8, `recomp/analysis/d08t11/` (local), real keyboard
and mouse on a private X display, private profile/cards/port.

- Before: contact, Grab held: idle 63, no grab, block unmoved.
- After (`try2.py`): grab 121 from the recess; pulled from Z 34745 to 32585,
  pushed back to 33665, released, reapproached, grabbed again (121), pulled
  to 32015.
- `ttk-controls-test` (all-level fixtures): new D08T11 group - acquisition
  along the displaced axis from both facings, refusal across it, at the
  authored spot, for height-only changes, a row of another type, a row for
  another object, an empty table, line-up/wall refusal and Vanilla; floor
  byte unchanged. Existing D08T1-T10 groups pass; 115 groups total.
- Player save/card files were only read (hashes recorded before testing).

## Limits

- Assumes the game's own memory-card save rebuilds placement rows from the
  level data (a card save could otherwise record a moved object as authored);
  only savestates were tested. The lane list remains the fallback.
- One live level tested; the rule is level-independent but other objects'
  travel is not individually verified.
- D08T12 (crouch soft lock reported at the same block) did not reproduce on
  three scripted routes; the user's save made while stuck did, and it is
  fixed separately ([note 333](333-d08t12-squeezed-crawl.md)).
