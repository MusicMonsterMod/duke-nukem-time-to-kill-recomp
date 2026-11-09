# D41K - Props respond to dynamic light like walls do

Status: accepted and closed 2026-11-09 (user: "lock it in. close it out").

Job D41K (stood up from D53, note 432 revisions 2 and 3; selected
2026-11-09). Report: in Polymorph a dynamic light barely brightens props; a
green light at strength 25, radius 600 over the LEVEL00 control console only
"greyed" the desk top, while the same kind of light lights walls clearly.

Evidence labels: CONFIRMED (seen in code or in our own measurements),
INFERRED, UNKNOWN.

## 1. Measurements

Lab: `recomp/analysis/d41k/lab.py` (local only), offscreen, private copies
of the player's cards (`analysis/d41k/cards`), Modernized, Polymorph,
DYNAMIC LIGHTS on. The saves no longer start near the D53 console (all
level 0 slots are 8000 to 42000 units from it), so the subway platform (file
slot 8, UI 9) and the club (file slot 6, UI 7) were used. New developer
tools:

- `DNTTK_LIGHT_PROBE=1`: the shader's light terms evaluated per lit corner
  (vertex colour, normal kind, N.L, falloff, gain) summed for world, prop
  and actor, per prop mesh, with lit/unlit polygon counts per prop mesh and
  the reason a prop polygon went unlit (sky, glow face, ...), every 120
  images.
- Console `light at <x> <y> <z> <r> <g> <b> [radius]`: a test light at a
  world position (colour up to 32), so a prop can be lit from a chosen
  distance. A `rel F R D` helper in the lab places it from the camera.

Findings:

- CONFIRMED: props are lit. Every polygon of the subway train (mesh
  `8013f7d8`) carries the lit flag, its world positions and smooth normals;
  white light near its side raises a panel from 25 to 34/43/58/76/96 (8-bit
  mean) at strength 0.5 to 25. Prop corners there are neutral (rgb 128 = 1x).
- CONFIRMED: strong coloured light washed to grey. The D41A shoulder
  compressed `base + light` per channel above 0.5 toward 1.1, so at strength
  25 every channel saturated equally: green (7.5, 25, 7.5) on the train panel
  gave (96, 122, 98), against (35, 57, 36) at strength 4. This is the D53
  "greyed desk top".
- CONFIRMED: the shoulder started at the neutral 0.5, where prop corners
  sit, so their light was compressed from its first unit, while dark painted
  walls (world corner mean 0.28 on the subway) took it linearly first; and
  painted colours above 0.5 were dimmed whenever any light was in the image.
  Club, level lamps only, against DYNAMIC LIGHTS off: table red 86 -> 95
  (+9), lamp shade 91 -> 101 (+10), floor 20 -> 56 (+36).
- CONFIRMED: the D41D1 neon rule took whole props out of dynamic light. A
  prop with any neon-like face (neon on a dark ground) was listed by mesh and
  none of its faces was lit. On level 0 that covered, besides the club sign
  (type 51) and the TVs (858), a metal control panel with a small green
  screen (type 478, 1 neon face of 6), the bar's shelves (types 2 and 4, 5
  of 15), light boxes (type 79, 1 of 5) and red lamps (type 170, 8 of 24).
- Not a cause here: the D41D per-texel emitter rule (a debug view showed the
  whole train as ordinary surface; only yellow hazard stripes count as
  emitters); the D53 console textures are mostly grey metal. UNKNOWN: the
  console itself was not re-measured in place (no save near it).

## 2. Fix

- Shader (`gpu_gl_renderer.c` `lit_col`, framework patch): the painted
  colour is kept as it is and only the added light is soft-limited: linear
  to 0.35, then toward +1.0 (2x texture), measured on its brightest channel
  and applied to all three, so a coloured light keeps its hue. A neutral
  prop and a dark wall take the same light; bright painted texels no longer
  dim when a light appears. The emitter rule, falloff, facing and normals are
  unchanged. Unlit and raw-texture triangles are untouched.
- Neon faces (`level_lights.inc`, `near_clip.cpp`): the scan lists neon face
  records instead of meshes (`near_clip_set_glow_faces`); `object()` marks a
  polygon whose face record is listed (`Poly::glow`) and `lit_frame` leaves
  only those faces unlit. Level 0: 234 listed faces (whole meshes) -> 68.
  `DNTTK_LAMP_TRACE=1` also logs each prop's neon face count.
- Renderer-wide, Polymorph with DYNAMIC LIGHTS only; Classic and Vanilla
  have no dynamic lights and are unchanged.

Old and new added light for one light of strength L on its brightest
channel (base unchanged in the new curve):

| L | dark wall 0.28, old | neutral prop 0.5, old | any surface, new |
| --- | --- | --- | --- |
| 0.3 | +0.30 | +0.24 | +0.30 |
| 1 | +0.66 | +0.49 | +0.76 |
| 3 | +0.81 | +0.60 | +0.99 |

## 3. Evidence (after)

Matched stills, `research/before-after/D41K/` (local; before = the same tree
with the two fixes reverted, so both builds take identical `light at`
placements):

- `green-strong-train`: train panel (74, 97, 77) -> (47, 103, 48), green
  instead of grey; window (89, 153, 88) -> (80, 156, 80).
- `white-train`: white strength 1: identical (below the knee, as intended).
- `lamps-club` (+ `lamps-club-lights-off`): level lamps only. Added over
  lights off: table +9 -> +24, second table +6 -> +21, lamp shade +10 -> +26,
  floor +36 -> +42.
- `blast-peak`, `blast-fade`: a real RPG blast at the same point in both
  builds (deterministic), frames 0.04 s apart: whole-frame mean 165.4 ->
  180.6 at the peak, then within 0.4 of each other from the next frame on;
  walls and floor look the same.
- `club-counter`: the counter keeps its painted red (57 -> 62) under the
  lamps.
- Probe, club view: prop polygons skipped as glow 27 -> 8 per image, lit
  prop polygons up by about 25 per image.
- 180 Hz (club, RPG blast): 1833 redraws, 0 misses, 0 failed captures.
- `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS. Framework
  patch exported and verified: the pinned `08ec704a` checkout plus the patch
  matches every modified and new framework file.

## 4. Limits

- Not judged by the player yet: scenes with lamps and blasts are brighter on
  props and on bright painted surfaces (that is the fix, but how much is a
  taste call); the curve constants (knee 0.35, ceiling +1.0) are first values.
- A neon face stays unlit as a whole: the 478 panel's screen face includes
  the metal around its screen.
- The D53 console was not measured in place; the cause found (grey under
  strong coloured light) matches its report.
- Shadows, light through walls: D41F, unchanged.
