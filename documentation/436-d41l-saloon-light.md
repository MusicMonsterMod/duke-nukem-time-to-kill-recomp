# D41L - Saloon light: woodburners and chandeliers

Status: **Done** (user-accepted 2026-11-10: "i cant believe how good it looks. i love
the haze on the wood burner in front. the position is perfect, and so is the
style. lock it in and close it out"). Modernized Polymorph only; Vanilla
and Classic unchanged. Builds on the D41D lamps
([note 428](428-d41d-level-lights.md)) and the D41K light curve
([note 434](434-d41k-props-in-light.md)).

## Report

User, UI slot 7 (runtime slot 6, level 1, the saloon): "the saloon bar is
just dingy and miserable ... we have the woodburner, that doesnt emit ... we
also have a chandalier up the top, that should be the light source, basically
lighting up the stage a bit especially ... the light should be elegant you
know not this uniform block color. in fact it shouldnt be that much
different, just pointing out the 2 obvious light sources". Then: "there's
more than one woodburner btw ... will you make that particular model emit?"
Yes: both props are recognised by model, not by placement.

## The models (CONFIRMED, level 1)

- **Woodburner** (prop type 261): object mesh, 28 vertices, 32 faces. Body
  540 x 460 at model y 835..1435 (y down), a pipe to y -1535. The door face
  is the +z quad (vertices 6 4 7 5, tpage 1066), its grate texels at
  x -150..150, y 1165..1300. Signature: first corner (-270, 1435, -230),
  vertex 24 (109, -1535, -18).
- **Chandelier** (prop type 263): 28 vertices, twelve FT4 arm planes from the
  centre column (0, -212..235) out to radius 896, all one 32 x 24 4-bit
  texture (tpage 569, u 96..127, v 64..87, double-sided) with a candle at the
  outer top of each arm (u about 99.5: radius 795, y -212), and a chain
  triangle. Signature: list header `0x000c002c`, first corner (-896, 235, 0),
  vertex 26 (0, -235, -25).
- Level 1 has 6 woodburners (rooms 5, 6, 19, 36, 38, 45) and 1 chandelier
  (room 6, over the saloon floor, the stage's dancers at z about 536).

## What is in

`recomp/src/ttk/prop_lights.cpp` / `prop_lights.h` (title code):

- **Scan** at the composition entry (0x80026164, live frames, Modernized
  only), on level change and every 300 frames: static props whose mesh
  matches a model become light emitters, placed by the prop's yaw.
- **Lights** (DYNAMIC LIGHTS on): after the level lamps and D53 screens, up
  to 6 nearest Duke, each faded by the room gate (the light's room shown by
  Polymorph, 0.25 s) and by reach (as lamps).
  - Woodburner fire: model (0, 1180, 560), (1.0, 0.46, 0.15), strength 1.5,
    radius 2800, `prop_fire_level`: a slow breath, smooth flutter and the odd
    lick of flame, 0.55..1.0, from smooth value noise (no per-tick jitter).
  - Chandelier candles: model (0, -120, 0), (1.0, 0.80, 0.52), 0.9, 1600 (the
    ceiling and arms).
  - Chandelier fill: model (0, 700, 0), (1.0, 0.74, 0.46), 0.5, 5000.
  - Candle shimmer `prop_candle_level`: about 0.92..1.0.
- **Drawn extras** (`near_clip.cpp` `prop_glows`, after the prop's own
  faces): additive (blend mode 1), self-lit, painter-ordered quads textured
  from the second bank (tpage 0x124 = x 256, 15-bit, semi-transparent
  texels), per-poly `foreign` mark. Billboards are built in view space from
  the projected anchor (`project_view`), so they face the camera.
  - 12 candle flames (40 x 84, each stretching 0.88..1.12 and leaning on its
    own clock) and 12 halos (240 across, faint).
  - The woodburner's ember quad over the grate, 3 units in front of the door,
    following the fire level, and a faint 560 x 420 bloom before it.
  - Animated from SDL ticks, the same clock as the lights, so redraw
    workers draw them in step.
- **Textures** (procedural, painted into the second bank once, at x 256..335,
  rows 0..31): flame (16 x 32 teardrop, gold core, orange edge, faint blue
  foot), halo (32 x 32 soft warm disc), embers (32 x 16, hottest low, in
  clumps). Original art; no retail texels.
- **Level 1 stage pool**: `level01.lights` gains one steady warm lamp
  (22050, -1500, -1200), radius 4000, strength 1.8, in front of the stage,
  as the chandelier's light falling on it.
- Console: `lamp props off|on` (session); `lamp off` hides these too.
  Developer env: `DNTTK_PROP_LIGHT_FIRE`, `DNTTK_PROP_LIGHT_CANDLES`,
  `DNTTK_PROP_LIGHT_FILL` ("r g b strength radius"), `DNTTK_PROP_FLAMES`
  ("w h level halo_size halo_level"), `DNTTK_PROP_LIGHT_TRACE=1`.

## Tuning in the lab

Matched third-person shots from the slot (`analysis/d41l/variants.py`,
local), red-channel means:

| Region | Props off | Fill 1.0/6500 | Fill 0.8/6000 + stage 0.95 | Chosen: fill 0.5/5000 + stage 1.8/4000 |
| --- | --- | --- | --- | --- |
| Dancers | 36 | 57 | 56 | 61 |
| Stage front | 30 | 48 | 47 | 51 |
| Back wall above | 34 | 57 | 53 | 55 |

Stronger fills (1.5, 2.0) washed the whole room toward one colour again and
burned the painting near the chandelier; the chosen balance lifts the stage
most and leaves the upper walls and corners darker. A stage lamp placed high
and close to the stage front plane (z -150) added nothing there: the light
must come from in front and lower. Added light is soft-limited (D41K), so the
stage pool shows best with a modest fill.

## Evidence

Private copies of the player's cards and savestates
(`recomp/analysis/d41l/cards`), offscreen, debug port; player saves, cards
and profile hash-checked unchanged after every run.

- Scan: `level 1: 6 stoves, 1 chandeliers, 8 lights`.
- Matched shots, `research/before-after/D41L/` (local): `saloon-before.png`
  / `saloon-after.png`, `stove-before-after.png` (fire light on the floor
  and wall, grate glowing), `chandelier-flames.png` (a flame on each candle),
  `saloon-first-person-180hz.png` (180 Hz, first person).
- Vanilla (`--vanilla`): no scan, no flames, the original stove.
- Build OK; `ttk-input-test` and `ttk-near-test` pass. No framework change.

## Limits

- The room-5 woodburner (round the corner) and the others are the same model
  and are found by the scan; only the saloon's was seen in a shot.
- Light still passes through walls between shown rooms (D41F shadows).
- Flames and glows are drawn whatever DYNAMIC LIGHTS is set to; their cast
  light needs it on. Not shown in Classic.
- Other chandelier or stove models in later levels are not recognised until
  added.
- Not measured on the player's GPU: 26 extra quads per chandelier in view,
  up to 3 more lights per image in the saloon.
