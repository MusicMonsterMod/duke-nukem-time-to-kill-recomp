# D08T13 - Pushing stops at bars (every collision box)

Status: **Done** (user-accepted 2026-10-30: "the fix is in! and it's absolutely
perfect"; closed). Modernized manual push/pull only;
Vanilla unchanged.

## Report

After D08T11, the slot 8 Duke-symbol block pushed away from its recess went
straight through a set of metal bars, and Duke clipped through them while
pushing. User saves: UI slot 8 (block just before the bars) and UI slot 9
(block already in them), both 2026-10-30 13:58.

## Cause

The bars belong to a LEVEL06 frame object, type 354 at (105473, 20352, 26687),
whose box list (`+0x44`) holds three boxes: box 0 the lintel (above the block),
box 1 the right pillar, box 2 the bars across the left of the opening (X
104449-104961, floor to the block's top). `push_step_clear()` checked every
placed object in the room lists, but only box 0, through `object_box()`
(`0x80073ae8` rules for index 0). The lintel never overlaps the block, so the
block and Duke (the same object test with his 220 radius) passed through the
bars. Duke's walk is stopped there by the original collision; only the
manual mover missed it.

## Fix (game-wide)

`recomp/src/ttk/push.inc`: `object_box_count()` reads the list's box count
(halfword `+4`; 3 for the frame, 1 for the block and window panels) and
`object_box_at()` reads box N (16 bytes each from the list's `+8`).
`push_bounds()` takes the box index and the room-object test in
`push_step_clear()` checks every box of every object, for both the block and
Duke. A box that already overlaps at the start (an older save such as slot 9)
does not trap the block: a move may reduce the overlap, or keep it while
backing the block out toward Duke (Duke himself: away from the obstacle);
the stationary grab check accepts the existing overlap. Nothing else in
travel, support, rays, limits or acquisition changes.

## Evidence

Private copies in `recomp/analysis/d08t13/` and `recomp/analysis/d08t11/`
(local), real keyboard/mouse on a private X display.

- Slot 8 (`push.py`): grab, W for 4 s: the block stops at Z 27261, its front
  face 2 units short of the bars' face (26751); S pulls it back out.
- Slot 9: grab works; W does not move it further in; S backs it out to 28281.
- Duke leading (long pull from the D08T11 recess save): before, Duke walked
  through the bars to Z 19465; now he stops at 26981, his radius short of them.
- `ttk-controls-test`: the D08T3 group adds a three-box gate with the lintel in
  box 0 and the bars in box 2 (block stops; a block already in the bars cannot
  go on and backs out; box 0 alone would not stop it). All 115 groups,
  `ttk-input-test` and `ttk-aim-test` pass. Player saves only read.

## Limits

- Live check at this one gate; other multi-box objects in the campaign now
  stop pushed objects where their extra boxes stand, which is the intent but
  not individually verified.
- The original level's floor support and wall rays are unchanged.
