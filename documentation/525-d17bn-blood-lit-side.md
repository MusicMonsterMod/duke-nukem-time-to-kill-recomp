# D17BN - Floor blood lit from the wrong side of its floor

Status: **Done, user-accepted and closed out 2026-11-21** ("it's now fixed! to an exceptionally high standard", "lock it in and close it out"). Job board: D17BN. Owner code:
`recomp/src/ttk/near_clip.cpp` (`emit`, the lit corners of a host
polygon) and `recomp/src/ttk/model_depth.inc` (`model_link`, which marks
D17AT's decals).

## Report

User, 2026-11-20 (UI slot 12, TIME TO KILL, level 0, looking down at a
large blood pool on the tiled floor): "it's flickering again. i'd like the
blood splatters to not flicker like i thought we fixed this, this must be
aggressively fixed." Save copy in `research/savestates/d17bn/` (local).

## What flickered

The pool is 13 blood objects, one flat textured quad each, all taken by
D17AT's small-model renderer as decals (784 of 784 faces over a slow turn:
none native, dropped, over budget or back-facing). While Duke turns, large
straight-edged parts of the pool switch between bright red and near black
from one image to the next, with the camera moving about one pixel.

## Ruled out (measured)

- D57F's solid blood (revision 4) and its stain test (revision 6): this
  pool's packets are already opaque (command `0x2c`, page `0x001c`, palette
  `0x57ac`), so neither applies; no palette verdict was made in the runs.
- Blob order: the 13 objects are drawn in the same order in every frame
  (order changes only when an object enters or leaves the view).
- The blob-over-blob depth rule (D17AT10: colour 6 units up tested against
  depth written 4 units up). The blobs lie at heights 5.1 units apart and
  several pairs sit about 2 units apart, a tie in principle; drawing every
  depth pass after every colour pass made no measurable difference, so the
  rule stays as accepted.
- Depth precision and draw order in the runtime: host depth is a reversed
  1/z, perspective-correct, 24-bit, `GL_LEQUAL`, drawn in ordering-table
  order (redraw workers only record).
- The floor chosen for the bake: every blob finds the same floor (9055 or
  9062) in every frame.

## Cause

`DNTTK_MODEL_BAKE=0` (models on the live lamps) removes it (changed blood
pixels per image 4500 -> 325), so the floor bake. A flat model face is lit
from the bake of the floor under it, and the shader (`baked_light`, D41P)
takes the floor chart's lit side or its unlit side by which side of the
floor plane the fragment lies on. Blood lies on the floor (centred at
y -6144 here; D17AT measured splats within about 2 units, some slightly
inside), so for each fragment that test was decided by the rounding of world
positions recomputed from the view every frame. The tie lines are straight
and move with the view: lit (bright red) on one side, the unlit side (near
black) on the other.

## Fix

A decal's lighting position (only that: the polygon, its depth and its
texture stay where they are, so nothing slides) is lifted 16 units off its
face toward the eye, the side that is seen. `model_link` marks D17AT's
decal faces (`Poly::decal`); `emit` adds the lift to the world position of
each lit corner. Game-wide: every flat small-model face drawn as a decal, on
floors and walls. Modernized only (D17AT's path never runs in Vanilla or
Classic). Developer `DNTTK_DECAL_LIGHT_LIFT=0` restores the old lighting
(0..64 units).

## Evidence

Lab `recomp/analysis/d17bn/` (local): `probe.py` (cases `view`, `still`,
`turn`, `counters`), `swap.py` (blood pixels changing between consecutive
images after alignment, inside the pool), `shift.py`, `sheet.py`,
`order.py`. Private card copies; the player's files hash-checked unchanged by
every run.

| Run (UI slot 12 save, slow turn, 180 Hz) | Changed blood px per image (mean) | Spike images |
| --- | --- | --- |
| before (`DNTTK_DECAL_LIGHT_LIFT=0`) | 4215 | 1 of 59 |
| fix | 2731 | 0 of 59 |
| fix at 120 Hz | 2835 | 0 of 39 |
| fix, real display (windowed, 180 Hz) | 2128 | 0 of 39 |

The remainder is texture crawl on lit texels; the matched frame sheets show
no straight-edged switching. Blood colour with the fix (68.6, 3.6, 1.3),
against Vanilla's (72, 6, 4) and D57F's accepted (62-69, 5-6, 2). Still
view: 61 changed pixels a shot (Duke's body bob). Other blood: the D17AT
fixtures (UI slot 7, wall and floor blood in a corner; UI slot 11, a
corridor pool) look the same with and without the fix and measure the same
or slightly lower (4367 -> 4176, 684 -> 684). Vanilla loads the save.
`ttk-near-test disc/SLUS_005.83` PASS; local Python suite 268 OK (2
skipped). Framework unchanged.

## Limits

- Accepted on the user's display (UI slot 12). Before/after sheets (local):
  `research/before-after/D17BN/`.
- A decal on a surface the bake does not cover is unaffected (no bake: the
  live lamps, as before).
