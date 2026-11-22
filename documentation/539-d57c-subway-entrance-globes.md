# D57C - LA subway entrance globe lights

2026-11-22. **Done, user-accepted** ("lock it in and close it out"). User, 2026-11-14 playthrough: the round
globe lights outside the subway entrances should give light; audit every LA
level that has them.

Private runs only: lab `recomp/analysis/d57c/` (local; `probe.py` from the
D41AB probe, private profile and a copy of the lab cards, offscreen on the
real GPU at 180 Hz, debug port). Every run checks the player's saves and
settings before and after (unchanged).

## What the game has (MEASURED)

- Level 0 prop survey (136 static prop models): the globes are prop type
  315, model `0x8012f4ac` on level 0: 25 vertices, a sphere 222 wide and 200
  tall about its own centre, its texture an evenly bright yellow glass
  (every texel lit; colour from VRAM 1.00 0.83 0.32). No animated face.
- Two of them, on the posts at the top of the "Subway" stairs: room 5,
  (17349, -10812, 14381) and (19519, -10812, 14381). The railings are type
  313 (64 vertices, 4088 long). Neither had a light: no level lamp is bound
  to them and the D41D1 sign scan passes over them (the yellow is not a sign
  colour).
- All 21 levels surveyed (scan trace per level): globes only on levels 0, 5
  and 9 (two each, models `0x8012f4ac`, `0x8012a0e4`, `0x8012b144`). Levels
  5 and 9 reuse the same street block: the same entrance, railings and globes
  at the same coordinates (room 5). No other level has them.
- Ruled out: type 170 (a red glowing dome, levels 0 and 5) already has D41D1
  sign lamps since 2026-11-09; type 195 (red caps on brass posts, level 0
  room 75) is a velvet-rope stanchion.

## Change

`recomp/src/ttk/prop_lights.cpp` (D41L prop lights), the D41Z caged-lamp way:
one rule found by model data, no per-level data.

- `globe_mesh()`: 25 vertices, first (0, -100, 0), last (29, 100, 29); kind
  `prop_light_globe`. `lamp_colour()` (the D41Z texture reader, now for any
  vertex count, also giving the lit share) gives the colour, and the model
  counts as a lamp only when at least 40% of its texture is lit, so a plain
  ball with the same mesh would not light up.
- A steady light at the globe's centre in the glass's colour (strength 1.0,
  radius 3600), and a soft halo over the globe (size 460, level 80, its
  colour). A lamp the level file binds to the globe would keep that lamp
  alone (none does). It goes out if the prop breaks (D54; the original does
  not break them).
- `lamp signs` skips the model (`prop_light_lit_mesh`); `lamp props off|on`
  includes the globes.
- Developer: `DNTTK_PROP_LIGHT_GLOBE="r g b strength radius"`,
  `DNTTK_PROP_GLOBE_HALO="size level"`; `DNTTK_PROP_LIGHT_TRACE=1` prints the
  globe count and each model's colour and lit share.
- Developer (lab): the console's `warp x y z [degrees]` now takes an
  optional facing (compass angle, Duke and the view), as `warp object` does.
- The light needs LIGHTING ENHANCED or RELIT; the halo is a Modernized
  Polymorph prop extra, as the caged lamps' (it shows in LIGHTING ORIGINAL
  too). Vanilla and Classic: unchanged (the prop lights do nothing there).

## Evidence (offscreen, private copies; player files unchanged after every run)

- Scan: level 0 `2 globes, 38 lights`; levels 5 and 9 two each; colour 1.00
  0.83 0.32, lit 1.00, on every model.
- The entrance, `lamp props off` / `on`, mean luminance
  (`research/before-after/D57C/entrance-*`, local):

  | Region | Relit | Enhanced |
  | --- | --- | --- |
  | Left globe | 37.5 -> 44.6 | 41.0 -> 45.7 |
  | Right globe | 35.8 -> 39.7 | 39.1 -> 41.5 |
  | Left post and railing | 31.6 -> 37.8 | 35.5 -> 39.2 |
  | Building to the right | 26.8 -> 33.8 | 28.2 -> 32.8 |
  | Brick wall behind | 42.0 -> 44.1 | 47.7 -> 48.7 |

- 180 Hz pacing at the entrance, Relit, off / on: p50 and p95 6 ms, max 7 ms,
  none over 10 ms.
- Local tests: 270 run, OK (2 skipped). Executable `073b22fe...`.

## Limits

- The lab view is from the foot of the stairwell looking up at the
  entrance: a warp onto the street there lands Duke under it (room 5's box
  reaches below the street and the game finds no floor), and no lab save
  stands near it. How far the light reaches the pavement round the entrance
  is not measured; the user's look decides, and a save at the entrance would
  allow tuning.
- A point light: no shade from the globe's cap or post.
