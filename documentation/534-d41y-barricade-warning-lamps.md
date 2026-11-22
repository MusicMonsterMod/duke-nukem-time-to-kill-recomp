# D41Y - Construction barricade warning lamps

2026-11-22. **Done, user-accepted** ("lock it in and close it out!"). User, accepting D41X in a new UI slot 12
save: "add flashing lights to this object too, this kind of construction
barrier thing".

Private runs only: lab `recomp/analysis/d41y/` (local; `probe.py` from the
D41X probe, private profile and a copy of the lab cards, offscreen on the
real GPU, debug port). Every run checks the player's saves and settings
before and after (unchanged).

## What the game does (MEASURED)

- The barricade is a static prop (kind 1) of type 128, model `0x80125fc4` on
  level 0: 9 vertices, 5 faces. An orange-and-white A-frame (feet at y 255,
  z +-119, top rail to y -138) and, on top at one end, the warning lamp: a
  16 x 16 textured quad, face 0, x -205..-92, y -255..-138, z 2.
- **The lamp already blinks in the original.** Face 0's texture is animated
  (texture word 2, bit `0x20 << 25`) through the mesh's animation table
  (`+0x0c`, entry `0x80125fec`): a script of 2 frames of 600 ms, frame 0
  lit amber (CLUT `0x1e38`, texels up to 232, mean 143 100 0), frame 1 dark
  (CLUT `0x2079`, up to 112, mean 50 18 0). Read live, the record's frame
  words alternate `0x1e388020` / `0x20798030` every 0.6 s; screenshots show
  the lamp at 151 110 5 lit and 53 26 4 dark. All barricades share the
  record, so they blink in step. The job card's "steady in the original"
  (from the D41D1 sign scan's static rating) was wrong.
- What was missing: light. No lamp file binds type 128, the sign scan rates
  the lamp below its threshold, and our lights gave it none.
- Level 0 places four (rooms 20 and 23, two in each).

## Change

`recomp/src/ttk/prop_lights.cpp` (D41L prop lights), the D41X way: one rule
found by model data, no per-level data.

- `barricade_mesh()` recognises the model (9 vertices, first
  (-205, 255, -119), last (-205, -255, 2)); kind `prop_light_barricade`.
- The prop scan gives each standing barricade an amber light at its lamp
  (model (-148, -196, 0)): colour 1.0 0.66 0.10, strength 0.7, radius 1100
  (Relit multiplies by the level's lamp gain and reach as for every prop
  light).
- The light follows the game's own blink: each game frame
  (`composition_hook`) every barricade model's lamp level is read with the
  D55 signal reading (`signal_now`, exported as `prop_signal_now`): the
  frame shown against the animation's brightest frame. On with the lit
  frame, a 90 ms bulb fade after it. A dark lamp is published at 0, so the
  light list does not change with the blink.
- A soft amber halo (the neutral halo, 240 across, level 90, tinted with
  the lit frame's colour) over the lamp while it is lit (drawn extras, near-
  clip object path), so the lamp reads as a light source.
- D54: when a barricade breaks its light sputters out from full
  (`[TTK prop light] barricade lamp light goes out`).
- `lamp signs` skips the model (`prop_light_lit_mesh`), and `lamp props
  off|on` includes it.
- Developer: `DNTTK_PROP_LIGHT_BARRICADE="r g b strength radius"`,
  `DNTTK_PROP_BARRICADE_HALO="size level decay_ms"`, `DNTTK_BLINK_TRACE=1`
  (lamp level per frame).
- The blink itself is the original's and is untouched in every setting.
  The light needs LIGHTING ENHANCED or RELIT (DYNAMIC LIGHTS); the halo is a
  Modernized Polymorph prop extra like the woodburner's embers. Vanilla and
  Classic: unchanged.

## Evidence

- Scan, level 0: `0 stoves, 0 chandeliers, 0 rings, 7 fire barrels, 4
  barricades, 12 lights`. Blink trace: lamp level 1.00 on 329 frames, 0.00
  on 344.
- Slot 12 view, matched by the game's frame word (lit / dark), `lamp props
  on` / `off`, RGB means (first tune, strength 1.0 radius 1500):

  | LIGHTING, props | floor beside, lit | dark | rails, lit | dark |
  | --- | --- | --- | --- | --- |
  | Relit, on | 77 59 22 | 31 31 20 | 117 85 49 | 87 68 48 |
  | Relit, off | 27 31 25 | 36 35 21 | 85 66 46 | 86 67 47 |
  | Enhanced, on | 87 76 42 | 26 25 16 | 125 92 58 | 105 82 58 |
  | Original, on | 27 27 18 | 39 41 29 | 104 81 57 | 102 79 56 |

  (floor readings move about 10 with Duke's idle sway). That first tune lit
  the whole floor in front of Duke; strength 0.7 and radius 1100 keep a
  warm pool round each barricade's feet and rails
  (`research/before-after/D41Y/`, local).
- Break: bit 2 of `+0x00` set by hand on both barricades: `barricade lamp
  light goes out` for each, the pool gone.
- Vanilla: pixel-identical to the build before the change (0 of 786432
  pixels; two runs of the old build also identical).
- 180 Hz, the barricades in view, Relit: 600 presents each with `lamp props`
  off and on, interval p50 and p95 6 ms, max 8 / 7 ms, none over 10 ms.
- Level survey (16 levels: 0-2, 5, 7, 9, 11, 12, 22-29): barricades on level
  0 only; the other prop counts match D41L, D41X and D57G. Levels 3, 6, 8, 10
  and 21 were not reached by the lab's level jump.

## Limits

- The original blinks every barricade in step (one shared animation record);
  a per-barricade phase would need the drawn texture changed per object, so
  the lights stay in step with what is drawn.
- The halo is drawn per model, not per object: a barricade marked broken by
  hand but still drawn keeps its halo. A barricade broken in play was not
  tried.
- Props cast no lamp shadows: the light reaches through the barricade's own
  rails.
- Judged offscreen; the user's display at 180 Hz decides the strength.
