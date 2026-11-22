# D41Z - Caged wall lamps give light

2026-11-22. **Done, user-accepted** ("its perfect!! lock it in and close it out"). User, closing D17BO: "these lamps are not
emitters, so we need to do the same kind of thing as we have been doing with
other objects" (UI slot 12, LEVEL00 subway tunnel).

Private runs only: lab `recomp/analysis/d41z/` (local; `probe.py` from the
D41Y probe, private profile and a copy of the lab cards with the slot 12
save, offscreen on the real GPU, debug port). Every run checks the player's
saves and settings before and after (unchanged).

## What the game does (MEASURED)

- The lamp is a static prop (kind 1), not wall texture. In slot 12 it is
  type 667, model `0x8013f7d8`: 26 vertices, a bulb 86 wide and 200 tall
  behind four bars, standing out from its wall plate (model z 31) to z -31;
  model -z faces the room.
- Its texture is steady: no face is animated (texture word 2 bit
  `0x20 << 25` clear on all 22 faces). The bulb's texels are pale warm white,
  mean about 160 157 117.
- The same geometry comes as several model records (one per type and
  level), found by geometry, not by pointer:

  | Level | Type 667 | Type 665 | Type 851 (red) |
  | --- | --- | --- | --- |
  | 0 | 6 | 18 | - |
  | 5 | 22 | 2 | 8 |
  | 9 | 6 | 1 | - |

  Types 665 and 667 share texture page, CLUT and UVs (the same pale lamp).
  Type 851 on level 5 is the same cage with a red bulb (texels about
  150 29 27).
- Twelve pistol shots on the slot 12 lamp leave it standing (no break flag,
  model unchanged): the original does not break it.
- No level lamp sits on a pale lamp: the nearest are D41D painted-light scan
  lamps (`scan room N`, 950 units and more away), which stand for the room's
  painted light and belong to no prop (as D41X found beside the barrels).
  Level 5's eight red lamps already have D41D1 sign lamps bound to them
  (`sign type 851 room N prop 851 ...`, red 1.00 0.12 0.12, radius 1714).
- Other levels: none of the 13 others the lab's level jump reaches (1, 2, 7,
  11, 12, 22-29). Levels 3, 6, 8, 10 and 21 were not reached.

## Change

`recomp/src/ttk/prop_lights.cpp` (D41L prop lights), the D41X/D41Y way: one
rule found by model data, no per-level data.

- `cage_mesh()` recognises the model (26 vertices, first (0, 100, -3), last
  (-43, -99, 31)); kind `prop_light_cage`.
- The prop scan gives each caged lamp a steady light 90 units before its
  bulb (model (0, 0, -90)): strength 0.8, radius 1600 (Relit multiplies by
  the level's lamp gain and reach as for every prop light).
- Colour from the lamp's own texture (`cage_colour`, once per model per
  scan): the mean of the model's texels with a channel at 0.4 or more, read
  from VRAM through each face's CLUT, scaled to the brightest channel and
  raised to the power 1.5. Measured: pale lamps 1.00 0.99 0.60, the red lamp
  1.00 0.06 0.06.
- A lamp the level's lamp file already binds to that prop
  (`level_lamp_bound` 2: level 5's red lamps) keeps that lamp alone and gets
  no second light. While a new level's lamps are not loaded yet the lamps
  wait and the scan repeats ten frames later.
- A soft halo over the bulb (the neutral halo, 300 across, level 60, in the
  bulb's colour; drawn extras, near-clip object path), red lamps included.
- D54: a lamp marked broken loses its light (`caged lamp light goes out`).
- `lamp signs` skips the model (`prop_light_lit_mesh`), so a custom level's
  sign scan does not add a second lamp; `lamp props off|on` includes it.
- Developer: `DNTTK_PROP_LIGHT_CAGE="r g b strength radius"` (the colour
  only when the texture cannot be read), `DNTTK_PROP_CAGE_AT="z"`,
  `DNTTK_PROP_CAGE_HALO="size level"`; `DNTTK_PROP_LIGHT_TRACE=1` prints the
  counts and each model's colour.
- The light needs LIGHTING ENHANCED or RELIT (DYNAMIC LIGHTS); the halo is a
  Modernized Polymorph prop extra. LIGHTING ORIGINAL, Vanilla and Classic:
  unchanged.

## Evidence

- Scan trace: level 0 `24 caged lamps (0 lit by the level file), 36 lights`;
  level 5 `24 caged lamps (8 lit by the level file)`; level 9 `7 caged
  lamps`.
- Slot 12 view, `lamp props off` / `on`, RGB mean of the wall round the lamp
  (`research/before-after/D41Z/`, local):

  | LIGHTING | props off | props on |
  | --- | --- | --- |
  | Relit | 34 22 12 | 71 49 22 |
  | Enhanced | 50 32 17 | 71 47 24 |
  | Original | 50 32 17 | 50 32 17 |

- Orientation, all 24 level 0 lamps (yaw 0, 1024, -1024, 2048): the light
  point lies in the lamp's own room for every one; the point 300 units
  behind the wall plate lies outside every room for 12 (the other 12 rooms'
  boxes cover both sides).
- Break: bit 2 of `+0x00` set by hand on the slot 12 lamp: `caged lamp light
  goes out`, the wall back to 34 22 12.
- Vanilla: pixel-identical to the build before the change (0 of 786432
  pixels).
- 180 Hz, Relit, slot 12: 600 presents each with `lamp props` off and on,
  interval p50 and p95 6 ms, max 7 / 8 ms, none over 10 ms.
- Local tests: 269 run, OK (2 skipped).

## Limits

- The halo is drawn per model, not per object: a lamp marked broken by hand
  keeps its halo. The original does not break these lamps.
- Props cast no lamp shadows: the light reaches through the lamp's own bars.
- Level 5 and level 9 lamps were checked by scan and trace, not by eye.
- Judged offscreen; the user's display at 180 Hz decides the strength.
