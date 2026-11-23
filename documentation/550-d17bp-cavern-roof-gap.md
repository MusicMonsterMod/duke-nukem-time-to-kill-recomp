# D17BP - GOLD AND GUNS: no sky through the cavern roof's gap

2026-11-23 (overnight session). **Done, user-accepted** ("solid work!!! lock it in and close it out"). Job:
[D17BP](../jobs/graphics.md#d17bp---gold-and-guns-sky-patch-in-the-cavern-ceiling-ui-slot-10).
(The user's overnight list named "D17BJ", which is Done; the new D17BP, whose
lead is D17BJ, was taken as meant.)

User, 2026-11-23: "there's a patch of sky which has opened up in the cavern
on gold and guns". Save: the user's UI slot 10 (runtime slot 9, 02:13), copy
in `research/savestates/d17bp/`.

Private runs only: lab `recomp/analysis/d17bp/` (local; its own card copy,
the D17BJ cabin save and the D17AO1 start-shaft save from their labs;
offscreen on the real GPU, debug port). The player's saves and settings were
hashed before and after every run: unchanged.

## What it is (MEASURED)

- With the borrowed sky off (`DNTTK_SKY_BORROW=0`) the patch is black: there
  is no face there. Room 16's ceiling mesh has a hole: six edges used by one
  face only close a loop about 2000 across (x 58368..60416, z
  -47104..-45056, y -3712..-4096, sloped like the roof round it). No face of
  any room and no prop covers it (the props there are stalactites). It is a
  gap in the original geometry, which the game draws black and which a dark
  cave hides.
- D17BJ ([note 504](504-d17bj-cabin-sky.md)) draws the borrowed sky behind
  every composition of the level so the cabin's open shaft shows it; every
  other opening shows it too, this gap among them. Its survey (24 views) had
  not visited this cavern.
- Room flags do not tell them apart (both rooms 16 and 45 are plain `0x1`;
  the level has no sky-flagged room), nor does painted light (room 16 is
  brighter on average than the cabin's room 45).
- The shape does. The cabin's opening (room 45) is the level top rim of a
  shaft: a rectangle of unshared edges all at y -7168, each on a vertical
  wall going down from it, at the room's highest point. The cavern gap's
  edges are sloped and its neighbouring faces are sloped roof.

## Change

`sky_borrow.h`, `sky_borrow.cpp` (Modernized, the levels with a painted sky
and no sky of their own: GOLD AND GUNS).

- Once a level (and after each load) the level's mesh is read for open shaft
  tops: in each room, an edge used by one face only, level (both ends at one
  height), whose face is a wall in the edge's vertical plane going down from
  it; a room with three or more such edges at one height, at the room's
  highest point (a rim at the floor is a pit), has an open shaft top. GOLD
  AND GUNS: room 45 only (the cabin shaft, 10 rim edges). A first version
  without the highest-point test also took room 43 (a rim at floor level).
- The borrowed sky is drawn while a painted sky face was left out in this or
  the last composition (D17AO1's own rule: the start shaft) or while a room
  with an open shaft top is shown (the host's room walk, or the game's own
  portal-walk bitset `0x800e1f10` when the host did not walk: Classic).
  Otherwise the background stays black, as the original drew it.
- Developer trace `DNTTK_SKY_SHAFT_TRACE=1`: the shaft rooms found, and which
  shaft room is shown (or none) when it changes.

## Evidence

Same saves, first person, offscreen 180 Hz (lab shots `n1..n3`, `c1..c3`):

| View | Before (D17BJ) | After |
| --- | --- | --- |
| Cavern (the user's UI slot 10) | pink-purple patch of sky in the roof | black gap, as the original; trace: no shaft room shown |
| Cabin shaft (D17BJ's save) | sky | sky (room 45 shown) |
| Level start shaft (D17AO1's save) | sky through the painted face | the same (painted face left out) |
| Classic renderer, the same three | sky / sky / painted picture | black / sky (game walk) / painted picture |

- Shots: `research/before-after/D17BP/` (local).
- Cost: the mesh read runs once a level and after a load: 4.6 ms on the
  game thread for GOLD AND GUNS's 12,457 faces (trace), at the load; per
  composition, a check of the shaft rooms' shown bits. Levels without a
  painted sky never run it.
- Vanilla (`--vanilla`, the trace on): no sky scan (the borrowed sky needs
  the Modernized profile).

## Revision 2 (2026-11-23): the gap closed with rock

User, reviewing: "now it's just a black patch on the ceiling ... Either we
add some depth to the rock around the side, and give it a great halo ... or
we just patch it with the rock texture." (Their new UI slot 12, copy in
`research/savestates/d17bp-idea/`, looks up the level start's deep rock
shaft with the sky at its top.) The call: patch it with rock, the roof as
the level clearly meant it.

**The gap (MEASURED):** one missing cell of the roof's grid: its six corners
lie on the room's 1024 grid round one missing grid point (59392, -46080),
and the four pieces round it each hold some of them (no one piece could
carry the patch).

**Cap** (`sky_borrow.cpp` `find_shafts`, `sky_borrow.h`,
`host_visibility.inc`, `near_clip.cpp`; game-wide, levels with a painted
sky): the unshared edges that are not shaft rims are joined into closed
loops; a loop is a roof gap when its corners are on the room's grid, it is
near level (at most 1024 in height) and at least a tile across, it lies in
the upper quarter of its room, and its corners ring one grid point. It is
closed by a cap piece in the game's own world-piece format: the corners and
that point (at their mean height), a fan of triangles each three corners of
one 1024 tile, each textured with an entry of the level's texture table
that has the page and palette of the roof face across its edge and corners
that fit the tile (any mirror; within 2 texels; a triangle face's entry has
no fourth corner), painted with the roof's own vertex colours. Doorways
(vertical loops) and pit rims are not roof gaps (the trace lists every loop
and the verdict). The caps and their table (level, room, piece) live in a
block of mod memory allocated on the first live frame and filled by the
live game thread only, so each redraw worker reads its frame's own table.
`host_visibility.inc` draws a room's caps with its pieces; a cap is never
given to the game's own walk. Developer: `DNTTK_ROOF_CAPS=0` leaves the gaps
open; `DNTTK_SKY_SHAFT_TRACE=1`.

**Evidence:** GOLD AND GUNS: one roof gap capped (room 16: 7 corners, 6
triangles); 1 shaft top (room 45) as before; the scan 5.6 ms at the load.
The cavern from the user's slot: sky (D17BJ), black (rev 1), rock (rev 2)
from the same camera, no seam (`research/before-after/D17BP/rev2-*`); the
same in Relit, Enhanced and Original; 40 consecutive presents at 180 Hz all
show the cap (region mean 12.1 in every one). Cabin shaft and start shaft:
sky as before. Vanilla: no cap, no scan.

## Limits

- The cap is drawn by the Polymorph host path: under the Classic renderer the
  gap stays black (the original look).
- A roof gap that does not ring a single grid point is left open (black),
  reported by the trace; GOLD AND GUNS has none.
- An opening the rule does not call a shaft (a hole in a roof, a crack)
  draws black; one it does call a shaft shows the sky whenever its room is
  shown, as D17BJ.
- Verified offscreen; the user's look on the real display decides.
