# D57N - GOLD AND GUNS: the lava-glow walls laid upside down, turned upright

2026-11-23. **Needs playtest.** Job:
[D57N](../jobs/lighting.md#d57n---gold-and-guns-lava-glow-texture-upside-down-ui-slots-10-and-12).

User, 2026-11-23: "can you take a look at save slots 12 and slot 10. this
texture appears to be upside down. it is a baked in light, to give an extreme
lava glow effect. admitedly, i dont actually think this texture looks that
good! the first experiment i want to do is can we vertically invert the two
that my crosshair is pointed at in those save states, because they dont line
up with the other use of this texture."

Private runs only: lab `recomp/analysis/d57n/` (local; its own card copy
holding the user's UI slots 10 and 12 of 2026-11-23 12:14 / 12:13, copies in
`research/savestates/d57n/`; offscreen, debug port). The player's saves and
settings were hashed before and after every run: unchanged.

## What the crosshair is on (MEASURED)

A ray from the camera (`0x800d6eb0`: eye +0x14, view row +0x2c) through the
crosshair, against every world face of the level:

- UI slot 10: room 49, face record `0x801af438`; UI slot 12: room 53, record
  `0x801b5ed4`. Both are 1024-wide sloped wall faces of the lava pit's lower
  band (y 5120 at the top, 6144 at the bottom; the lava at 6784), texture
  entry 1725: page `0x18`, palette `0x278`, UV 64..126 x 128..190.
- The texture is a painted glow, bright at one end (its bright quarter of
  rows 1.9 times its dark quarter, warm: 0.31 0.18 0.03).
- Its 98 wall faces in the level: 80 run v down the wall (entries 1690 and
  1691 and others: the glow bright at the lava), 18 the other way (entries
  1725, 1726, 1784, 1918, 1919, 1921, 1928; rooms 49, 52, 53): the glow at
  the top of the band, which is what the user saw.

## Change

`prop_lights.cpp`/`.h`, `near_clip.cpp`. Modernized (the Polymorph world
path), every LIGHTING mode (it is the texture's placement); Vanilla and
Classic unchanged. By rule, game-wide:

- Once a level and after each load, with the lava scan (D57A) and its VRAM
  snapshot: every world wall face (at least 256 tall, not animated) is
  grouped by its texture's page and palette. A group whose texture is a
  baked glow (its brighter quarter of rows at least 1.6 times its darker
  quarter, the bright end fiery: red the strongest, blue at most 35% of it)
  and whose faces mostly run one way in world height (70% or more, 12 faces
  or more) has its other faces flipped.
- The flipped faces' records are in a mod-memory block (allocated on the
  first live frame, filled by the live scan; the level at +0, the count, the
  sorted records), so every redraw worker reads its frame's own list. The
  near-clip world path mirrors such a face's v (v' = vmin + vmax - v).
- Not taken on GOLD AND GUNS: three brown stone textures with strong
  gradients (page 9, palettes `0x2f8`, `0x338`, `0x3f8`: not fiery; a first
  version without the hue test flipped 12 of `0x338`'s faces) and two fiery
  ones used one way only.
- Developer: `DNTTK_GLOW_FLIP=0` leaves the faces as they are;
  `DNTTK_LAVA_TRACE=1` lists each glow texture and what it flipped.

## Evidence

- Both saves, before (`DNTTK_GLOW_FLIP=0`) and after: the band under the
  crosshair glowed at its top, against the band beside it; it now glows
  along the lava edge like the rest of the channel
  (`research/before-after/D57N/`, local).
- 40 consecutive presents at 180 Hz (redraw workers live), UI slot 10: the
  flipped band identical in every one.
- The cost: one more check per world face drawn (a level compare, then a
  binary search of 18 records only on levels with flips); the scan reads
  the snapshot the neon scan already takes.
- Survey, all 21 levels loaded in turn (the trace): level 3, the 18 faces
  above; level 5 (OBEY OR DIE) had one more candidate with the first
  threshold of 6 faces (palette `0x3378`, 6 faces one way, 1 the other; not
  looked at), so the rule now needs 12 faces or more and leaves it alone;
  every other level: none.

## Limits

- The user's look decides; the texture itself ("i dont actually think this
  texture looks that good") is a possible next step.
- A glow texture used half one way and half the other (no clear majority) is
  left alone.
