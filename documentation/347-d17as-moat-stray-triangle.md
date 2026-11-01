# D17AS - Slot-2 moat: the dark triangle is a stray authored face

2026-11-01. Report-first job. No game code changed. Status: Done - closed as
original level design. User decision 2026-11-01: option 1, leave as authored;
no fix is planned.

User clip `research/screencaps/triangle.mp4`: around 1.5-2.5 s a large
sharp-edged dark triangle covers the moat floor while Duke wades, and around
7-9 s a dark wedge crosses the centre of the view underwater. User theory: an
authored quirk in the level, like the slot-11 ceiling gap (D17AO, note 327).

## Method

Private copy of the D17AQ fixture (`recomp/analysis/d17aq/fixture`, file slot
1 = UI slot 2), offscreen, player files hash-checked by every run. Harness in
`recomp/analysis/d17as/`:

- `route.py` - the clip's route from the moat start: look down, wade forward
  into deeper water, dive, level out and turn; a shot every step; optional
  primitive trace of one step (`TR`) or from a step to the end (`TRALL=1`).
- `cover.py` - which traced emits cover given screen points, nearest first.
- `world.py`, `unproj.py` - traced vertices and emits back to world offsets
  from the eye (the room matrix row 0 carries the widescreen aspect, so the
  full matrix is inverted, not transposed).
- `overlay.py` - outlines one traced face over the matching shot.

## Reproduced

Wading forward from the start (steps 2-11 of `shots/r1`) shows the dark
triangle at the lower left, growing as Duke moves in, as in the clip.
Diving and turning underwater (`shots/u2`, steps 45-54) shows the dark wedge
with a straight edge across the centre and the castle to the right, as in the
clip at 7-9 s.

## Not a renderer regression

Same route, step 4-10 shots (`cmp.png`): the triangle is identical with the
current default (Polymorph), `DNTTK_UNIFIED_GEOMETRY=0`, `DNTTK_NEAR_CLIP=0`
(the game's own GPU packets), `DNTTK_HOST_VISIBILITY=0`,
`DNTTK_WATER_DEPTH=0` and `DNTTK_WATER_CLIP=0`. Vanilla's camera stays back
from this spot on the same inputs and does not frame it.

## The face

Both symptoms are one face: `0x8018dd5c`, a triangle (vertices 47, 34, 32)
of room mesh `0x8018dad0`, drawn by the game's room walk (caller
`0x80062db8`). Traced pixel cover at the surface frame puts it nearest after
the water tile; underwater its traced outline matches the dark wedge exactly
(`under.png`, step 52), and at the surface its apex matches the triangle's tip
(`surface.png`).

World offsets from the eye at the surface frame (x, y down, z; 1024 grid):

| Vertex | Offset | Meaning |
| --- | --- | --- |
| 32 | (-311, 249, 1134) | water surface, on the channel's west wall |
| 34 | (-311, 2297, 1134) | 2048 below the surface, same wall |
| 47 | (713, 2297, 1134) | 2048 below the surface, across the channel |

The water tiles sit at y 248-249, so the face hangs from the surface into the
deep channel, in the plane z = 1134 where this room meets the next one. In
that plane the room has no other face, and vertex 47 is used by no other
face: the triangle closes nothing. The next room (`0x8018de28`) continues the
channel through the same plane (west wall at x = -311 from the surface down,
floor at y 3321), so the channel is open there and this half-wall stands
across it. Its missing other half would make it a full partition; on its own
it is a free-standing triangle across open water.

It shows the same rock texture as the walls, only darker than the floor
around it (presumably its vertex lighting; not decoded). Both traced views
look along the channel from the start side; whether it is visible from the
far side was not tested.

## Conclusion

The user's theory holds. The triangle is a stray face in the level's own room
data, drawn by the original room walk in every renderer path, including the
game's native packets. The original camera stays behind Duke at this spot
and evidently never frames it; the independent first-person camera wades and
dives right up to it. Same class as D17AO.

Options:

1. Leave as authored (as D17AO). No code change.
2. Level-data fix: drop this one face when level 6's room data loads,
   identified by mesh shape and vertex positions, not by address, Modernized
   only. Small and contained, but it is a per-level patch.
3. A generic stray-face rule (a face sharing no edge with any other face,
   standing across a portal plane). Not recommended: legitimate single-sided
   decals and trims would match too, and every level would need a sweep.

Recommendation: option 1, recorded alongside D17AO.

## Decision

User, 2026-11-01: option 1. The triangle is part of the original level
design, made that way in the game's own data; it stays as authored and the
investigation is closed. Do not reopen it as a renderer problem.

## Limits

- The face bytes were not matched against the disc's level file; the
  evidence for "authored" is that the game's own room walk draws it from the
  loaded room data with no host involvement (`DNTTK_NEAR_CLIP=0`).
- Whether the face also blocks movement was not tested.
- The water tile does not visibly tint the triangle at the surface; not
  investigated further since the same holds in every path.
