# D57I - LA street lamp posts give sodium-vapour light

2026-11-22. **Done, user-accepted** ("lock it in and close it out"). User,
2026-11-22: "add lighting to these lamp post models. im really thinking these
have got to be sodium vapor lamps. really go for that 90s street at night
feel."

Private runs only: lab `recomp/analysis/d57i/` (local; `probe.py` from the
D57C probe, private profile and a copy of the lab cards, offscreen on the
real GPU at 180 Hz, debug port). Every run checks the player's saves and
settings before and after (unchanged).

## What the game has (MEASURED)

- The lamp post is prop type 160: 26 vertices, a pole (model z -611..-467,
  foot y 1462 on the street, top -1461) with an arm reaching 1060 out over
  the road to a cobra-style lamp head (z 421..611, y -1462..-1360). The
  head's underside is two faces painted with one white-hot bulb that meet at
  (0, -1360, 523): the lens, 2820 above the street. The rest of the texture
  is plain metal; the original gives the post no light.
- Level 0 (model `0x801265e4`): six posts, rooms 0, 1, 1, 4, 4 and 5, each
  arm turned to the road (yaw 0, +-1024, 2048).
- All 21 levels (scan trace per level; levels 6 and 21 from their own runs):
  posts only on levels 0 (6), 5 (5) and 9 (6), the shared LA street block.
  Levels 1-3, 6-8, 10-12 and 21-29 have none.
- No level lamp is bound to a post.

## Research (sources)

- Philips SON high-pressure sodium datasheets: nominal CCT 1900-2000 K
  (colour code 220), chromaticity x 0.540 y 0.420
  ([SON 70W E CO](https://www.signify.com/api/assets/v1/file/Signify/content/928486700091_EU.en_IE.PROF.FP/SON_70W_E_E27_CO_1CT_24.pdf),
  [SON-T 70W](https://www.signify.com/api/assets/v1/file/Signify/content/928152800035_EU.en_SG.PROF.FP/SON_T_70W_220_E27_1CT_12.pdf)).
  Colour rendering about Ra 20; the spectrum is the broadened sodium D line
  near 589 nm ([Wikipedia, sodium-vapor lamp](https://wikipedia.com/wiki/Lucalox)).
- That chromaticity lies outside sRGB: it converts (D65, clipped) to linear
  1.00 0.25 0.00, a pure orange. The eye at night adapts and sees a warm
  orange-amber with a little pink; the usual rendering value is 255 183 76
  ([McNeel forum, RGB values of various lights](https://discourse.mcneel.com/t/rgb-values-of-various-lights/96513)).
  The light uses 1.00 0.60 0.20, between the two.
- Cobra heads are IES type II or III: they throw the light down and out over
  the road, not back onto their own pole
  ([TxDOT light distribution patterns](https://www.txdot.gov/manuals/trf/hwi/lighting_equipment/light_sources-i1002196/light_distribution_patterns-i1023727.html),
  [NYC street design manual, lighting](https://www.nycstreetdesign.info/sites/default/files/2020-03/Lighting_SDM_v3_2020_0.pdf)).
  A 90s street at night is pools under each post with darker road between.

## Change

`recomp/src/ttk/prop_lights.cpp` (D41L prop lights), the D41Z/D57C way: one
rule found by model data, no per-level data.

- `street_lamp_mesh()`: 26 vertices, first (-71, 1462, -611), last
  (75, -1446, 611); kind `prop_light_street` (`prop_lights.h`).
- One steady light per post in HPS orange-amber (1.00 0.60 0.20, strength
  1.7, radius 2400; Relit makes it 1.7x stronger and 1.35x further, as every
  scene light). It stands at model (0, 650, 1000): 810 above the street and
  480 beyond the lens, out over the road, where a cobra head aims. The
  renderer's light is a point with (1 - d/R)^2 falloff; at the lens it barely
  reached the street, and under the lens it lit its own pole as a bright
  column (moving it out halved that).
- A soft halo on the lens (size 560, level 100, colour 1.00 0.72 0.38).
- A lamp the level file binds to the post would keep that lamp alone (none
  does). It goes out if the prop breaks (D54).
- `lamp signs` skips the model (`prop_light_lit_mesh`); `lamp props off|on`
  includes the posts. Steady: no flicker or warm-up (the D41AA lesson; a
  failing lamp only if the user wants one).
- Developer: `DNTTK_PROP_LIGHT_STREET="r g b strength radius"`,
  `DNTTK_PROP_STREET_AT="y z"` (model height and reach of the light),
  `DNTTK_PROP_STREET_HALO="size level r g b"`; `DNTTK_PROP_LIGHT_TRACE=1`
  prints the street lamp count per level.
- The light needs LIGHTING ENHANCED or RELIT; the halo is a Modernized
  Polymorph prop extra, as the globes' and caged lamps' (it shows in LIGHTING
  ORIGINAL too). Vanilla and Classic: unchanged (a `--vanilla` run makes no
  prop light scan at all).

## Tuning (lab, level 0 street, slot 3 copy)

| Try | Light (model y, z) | Strength, radius | Look |
| --- | --- | --- | --- |
| First | 160, 523 | 1.3, 3800 | an orange wash over the whole block |
| v2 | 600, 523 | 2.0, 2400 | separate pools; its own pole a bright column |
| v3 (kept) | 650, 1000 | 1.7, 2400 | pools out over the road; the pole about half as bright |

## Evidence (offscreen, private copies; player files unchanged after every run)

- Scan: level 0 `6 street lamps, 44 lights`; level 5 five; level 9 six.
- Mean luminance, `lamp props off` / `on` (`research/before-after/D57I/`,
  local):

  | View, region | Relit | Enhanced |
  | --- | --- | --- |
  | Side view: road under post A's arm | 42.1 -> 56.4 | 47.0 -> 56.8 |
  | Side view: pavement by post B | 38.9 -> 64.8 | 43.6 -> 54.7 |
  | Side view: far car under post C | 51.4 -> 62.5 | 59.0 -> 67.1 |
  | Crossing view: road under post A's arm | 41.4 -> 62.6 | 46.0 -> 56.4 |
  | Crossing view: post A's pole | 32.2 -> 51.0 | 35.8 -> 45.0 |
  | Crossing view: Babylon facade (out of reach) | 80.7 -> 80.1 | 90.7 -> 91.6 |

- 180 Hz at the side view, Relit, prop lights off / on: p50 and p95 6 ms,
  max 8 ms, none over 10 ms, both. GPU (clock-weighted) 29.6 / 30.5 and
  27.5 / 29.8 %: about 1.5 points for every prop light in view.
- Local tests: 270 run, OK (2 skipped). Executable `e47a3899...`.

## Limits

- A point light: the post's own pole still takes some of it, and the pool is
  round, not the long oval of a real type II/III head. A downward cone would
  need a renderer change (a cost on every lit pixel).
- Prop lights share six slots, nearest to Duke first (D41L), with the caged
  lamps, globes and fire barrels; a far post can fade out when nearer prop
  lights fill them. Not seen to matter on the street in the lab views.
- The lab judged the look offscreen (on the real GPU, not the player's
  display): the user's look on the real display decides.
