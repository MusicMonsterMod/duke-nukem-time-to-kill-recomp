# D57N - GOLD AND GUNS: the lava-glow walls laid upside down, turned upright

2026-11-23. **Done, user-accepted** ("solid work!!! lock it in and close it out"). Job:
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

## Follow-up (2026-11-23): the ledge's shadow, the atomic health's glow

User: "it looks great! now just 2 things - the shadow of the ledge my NEW
slot 12 is pointing to seems to shift when you walk around it. furthermore,
the glow of the atomic health seems to be swallowed up by the glow of the
lava itself. if that's just a simple limitation of how lighting works, so be
it. but can you review". The new UI slot 12 (12:59) is copied to
`research/savestates/d57n-shadow/`: the crosshair is on a flat ledge in the
pit (room 49, y 4096, about 2700 over the lava) with an atomic health on it.

- **The shadow (cause).** D57A lit the nearest two of 15 lava pools (about
  2000-4000 apart) by distance to Duke; walking the hall changed which pools
  were lit, so the ledge's shadow on the wall above it was cast from a
  different point every few thousand units and slid. **Fix:** one pool per
  room (and height), split only past 12000: GOLD AND GUNS now has 5 pool
  lights (rooms 49-53), and the hand-off fade is 4000. Along the ledge's
  stretch of walkway the two nearest are always room 49's and room 50's, so
  the shadow stands still; a hand-off happens once a room, and fades.
- **The atomic health (cause).** D41AC dims pickups with their room in
  Relit, so its cyan and red texels sank under the lava's strong orange
  light; its D56 light is a small cyan light (0.85, 1700). **Fix:** the atomic
  health is found by its model (both model records: 53 vertices, first
  vertex (0, -60, 0), the last at y 81) as a glowing item: its own colours
  keep their glow, and a soft cyan halo (380 across, level 90) breathes round
  it with its light. Game-wide (every atomic health, Enhanced and Relit).
  Developer: `DNTTK_ATOMIC_HALO="size level"`.
- Evidence: the slot 12 view before and after
  (`research/before-after/D57N/slot12new-*`): the atom reads as a cyan point
  on the ledge. 180 Hz Relit at that view, `lamp lava off|on`, three pairs:
  pacing p50/p95 6 ms, max 7-8, none over 10; GPU off 13.5 / 18.0 / 15.9 %,
  on 20.1 / 19.5 / 19.6 % (about +4 points).

## Limits

- The user's look decides; the texture itself ("i dont actually think this
  texture looks that good") is a possible next step.
- A glow texture used half one way and half the other (no clear majority) is
  left alone.
