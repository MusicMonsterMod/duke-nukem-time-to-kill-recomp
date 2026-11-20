# D17BI - Flat props lie on top of what they rest on

Status: **Done, user-accepted and closed out 2026-11-20** ("accepted!! lock it in and close it out"). Executable
`48ae3028d3fdc618dff593ebfc84d5efa4b2ad9afbae332b3ab5fcfb9bfec580`.

## Report

User, 2026-11-14 (playthrough commentary): two magazines on a desk at the
far end of an apartment room in OBEY OR DIE flicker excessively.

## Where

OBEY OR DIE (level 5), room 8: the upstairs apartment off the alley (papers
on the floor, posters, two gun racks). In the room's object list
(room `+0x48`):

| Object | Type | Position | What |
| --- | --- | --- | --- |
| `0x801d82e8` | 196 | (7609, -11564, 7440) | the desk in the corner, mesh `0x80126e28` |
| `0x801dccd8` | 932 | (7764, -11864, 7465) | magazine, mesh `0x80141ff0` |
| `0x801dcc78` | 932 | (7472, -11864, 7434) | magazine, same mesh |

The magazine mesh is one textured gouraud quad (record type `0x2c`) on four
vertices with y = 0: (+-74, 0, +-115). The desk top is the desk mesh's
y = -300 face. Both are static props drawn by the room loop (`0x80032288`,
object renderer `0x80010000`), so both are host geometry.

## Cause

Primitive trace (`DNTTK_PRIMITIVE_TRACE`, private save, 180 Hz): in every
redraw each magazine's four corners lie exactly on the desk top plane (view
y 198.0 against 198.0, 201.0 against 201.0, ...). Two separate meshes:

- the D17W/D17AV coplanar ranking compares faces inside one mesh only;
- the D17D prop contact allowance lifts every static prop by the same 2 units.

So the depth test met two equal depths and chose by rounding, which changes
as the camera moves: the magazines broke up and came and went. With the
game's own packets (`DNTTK_NEAR_CLIP=0`) they are whole in every frame: the
original ordering table paints them after the desk. Not a game bug.

## Fix (Modernized, game-wide)

`recomp/src/ttk/near_clip.cpp`: `object()` marks a scene mesh that is not an
actor or Duke's body as a **flat prop** when every vertex (model space, exact
integers) lies within half a unit of one plane (`flat_mesh`). A flat prop
lies on something as a decal: `prop_contact_shift` gives it 4 units instead
of 2, so it wins over the prop or world surface it rests on by 2 units, and
anything nearer than that still hides it. Host pieces and scene-depth native
packets share the rule (D17AL), so a prop's depth is the same on both paths.
Only depth moves; screen corners, UVs and ordering are unchanged, so nothing
floats. No per-level data. Classic shares the near path; Vanilla never
reaches it.

Developer: `DNTTK_FLAT_PROP_CONTACT=2` restores the plain prop rule
(0..16; `DNTTK_PROP_CONTACT=0` still turns every allowance off).

## Evidence

Lab `recomp/analysis/d17bi/` (local): `probe.py` (cases `tour`, `land`,
`repro`, `hover`, `info`), `roomobjs.py` (a room's object list from a RAM
dump), `cmp.py` (desk plane against magazine vertices in a trace),
`sheet.py`, `zoom.py`. Private card copies; the player's files hash-checked
unchanged by every run. Offscreen, the player's profile (180 Hz, first
person, FOV 90).

- Same route from the test save (walk toward the desk, 20 steps), desk-top
  crop, magazine-blue pixels in steps 8-19: old rule 10, 16, 3, 22, 13, 11,
  26, 10, 16, 6, 3, 52 (six drops, largest 16); fixed 13, 10, 10, 19, 19, 35,
  31, 28, 44, 47, 49, 58 (three drops of at most 4: walk bob). Visually both
  magazines whole in every fixed frame, patchy in the old ones.
- Off/on matched loads of all 12 lab card saves: 0 to 73 differing pixels on
  seven; on the other five (UI 3, 4, 8, 10, 11) only animated things differ
  (fire, the fading level title, a glow, a moving creature, weapon bob, neon,
  HUD blinks); no occlusion change.
- Vanilla: UI slot 7 loads and plays. Local unit tests 268, OK (2 skipped).

## Test save

UI slot 12 (runtime slot 11): fresh `level 5`, `dnkroz` on, Duke standing in
the apartment's far corner facing the desk, about 5,300 units away; walk
toward it. Copy in `research/savestates/d17bi/`; the D17BL ladder save it
replaced is in `research/savestates/d17bl-2/` (byte-identical).

Lab note: the `warp` command drops Duke through upper-storey floors when it
moves him far (player `+0x220`, the ground reference, is cleared); a short
warp in place keeps it. The save was set up by flying the jetpack, warping
while hovering and switching it off in the room.

Before/after (local, not committed): `research/before-after/D17BI/`, the
same walk from the test save with `DNTTK_FLAT_PROP_CONTACT=2` and with the
fix (a full frame and the desk-top strip of steps 8-19).

## Remaining

A flat
prop lying on an exactly coplanar flat prop (two papers overlapping) still
ties between themselves; none seen.
