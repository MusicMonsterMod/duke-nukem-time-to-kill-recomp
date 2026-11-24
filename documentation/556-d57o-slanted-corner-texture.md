# D57O - GOLD AND GUNS: a corner wall texture laid slanted, turned upright

2026-11-24. **Done, user-accepted** ("perfect work. lock it in and close it out"). Job:
[D57O](../jobs/lighting.md#d57o---gold-and-guns-corner-wall-texture-slanted-ui-slot-10).

User, 2026-11-24: "take a look at my slot 10. there is one surface here,
where the lines just dont line up properly. we did some texture work in this
area previously. can you rotate this texture so it lines up with the
adjacent textures."

Private runs only: lab `recomp/analysis/d57o/` (local; its own card copy of
the user's saves, offscreen, debug port; `probe.py` cases `aim`, `nbr`,
`ab`, `seq`, `slantall`). The user's UI slot 10 (2026-11-24 16:57) is copied
to `research/savestates/d57o/`. The player's saves and settings were hashed
before and after every run: unchanged.

## What the crosshair is on (MEASURED)

The D57N crosshair ray (camera `0x800d6eb0`) against every world face of the
level hits room 49, record `0x801af4ec`, 7704 units away: a triangle in the
lava pit's upper band (y 4096 to 5120), texture entry 1782 (page `0x18`,
palette `0x238`, 4-bit), the band's vertical rock stripes. It is not one of
D57N's glow faces (palette `0x278`, the band below).

The corner where the band turns from the diagonal wall (record `0x801af490`,
above, y 3072 to 4096) to the straight walls is filled by two triangles:

| record | corners (x, y, z) | uv |
| --- | --- | --- |
| `0x801af504` | (60416, 5120, -15360), (61440, 4096, -15360), (61440, 5120, -15360) | (63,191), (0,128), (0,191): upright |
| `0x801af4ec` | (61440, 4096, -15360), (60416, 5120, -15360), (60416, 4096, -14336) | (63,128), (63,191), (0,128): slanted |

`0x801af4ec`'s top edge runs along the diagonal wall above it with u 63 to 0
at v 128, as that wall does, but its bottom corner, 1024 lower, copies the
top-left corner's u (63). The point of the top edge straight above it (the
foot of the slope) is the edge's middle, u 31.5. So the texture's v ran
along the diagonal edge and the stripes leaned 45 degrees against every
face round it. The mirror corner in room 53 (`0x801b615c`) is laid the same
way.

## Change

`near_clip.cpp` (`upright_slanted_triangle`, after D57N's flip), the
Polymorph world path; Modernized only, so Vanilla and Classic are unchanged.
Only this one face: level 3, texture entry 1782 (used by no other face on
GOLD AND GUNS). Its bottom corner takes the u of the top corner directly
above it in x (u 0, was 63), as the lava band's corner triangle to its right
does (record `0x801af4fc`, entry 1784). The rows (v) are unchanged. The
stripes now run down the slope like the sloped band to its left
(`0x801af4d0`), seamless along their shared edge (u 0 at both corners), and
meet the diagonal wall above along the top edge. User, 2026-11-24: "It's
just the one texture in that exact spot". Developer: `DNTTK_SLANT_FIX=0`
leaves it as drawn.

First try (not kept): the bottom corner took the u at the middle of the top
edge (32), so the stripes ran straight down. User: "it needs to be turned
more counter clockwise, so it lines up to the textures above and below it
correctly. in a similar way to the triangle that is sloped, to the right of
it".

A first version applied this as a rule to every wall triangle with a copied
u. A survey of levels 0-8 found it would also change the mirror corner in
room 53, one face on level 1 and dozens on levels 6-8. The user wanted only
the one face, so the rule was dropped. This one-face change breaks the
project's usual no-level-data rule (by user request).

## Evidence

- UI slot 10: `research/before-after/D57O/slot10-crop-before-after.png`
  (local; original, first try, final). In the final the triangle no longer
  shows as a patch: its stripes carry on from the wall above and the sloped
  band beside it.
- 20 consecutive presents at 180 Hz (redraw workers live), one-face build:
  the triangle's pixels identical in every one.

## Limits

- Its edge with the flat corner triangle (`0x801af504`) still meets that
  face's stripes at an angle; three planes meet here.
