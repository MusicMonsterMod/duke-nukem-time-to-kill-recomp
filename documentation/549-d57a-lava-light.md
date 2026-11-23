# D57A - Lava gives light

2026-11-23 (overnight session). **Needs playtest.** Job:
[D57A](../jobs/lighting.md#d57a---lava-gives-light-gold-and-guns).

User, 2026-11-14 playthrough: the lava in GOLD AND GUNS looks bright but
lights nothing around it. Work: lava faces as warm emitters, cheap, maybe a
slow throb; acceptance: nearby walls and Duke take the lava's light within
the performance budget.

Private runs only: lab `recomp/analysis/d57a/` (local; the D57H probe with a
private profile and its own card copy holding the user's UI slot 10 of
2026-11-14 13:42, the gate at the end of the long lava hall, from
`recomp/analysis/d22p/cards`; offscreen on the real GPU at 180 Hz, debug
port). Every run checks the player's saves and settings before and after
(unchanged).

## The lava (MEASURED)

- The long hall of GOLD AND GUNS (rooms 49-53, x 21000..61000) runs over a
  deep pit; the lava is a channel at its bottom, two tiles (2048) wide, a
  flat plane at y 6784, about 6900 under the hall's walkways.
- It is world geometry (not a prop): flat 1024 x 1024 faces with textures
  1696-1699, 1822, 1915, 1916 and 1926 (page `0xa`, CLUT `0x11b8`), a dark
  orange flowing texture (its mean as read from VRAM about 0.29 0.14 0.00)
  under vertex colours painted to a full red (about 255 120 120). The paint
  is what makes it glow; no texel of it passes a "bright texture" test, and
  the level's other textures are never this red and saturated.
- Nothing lit the pit: the level file's three scan lamps of the hall (red,
  0.6) sit high in the hall; the pit's striped walls stayed dark.

## Build

`prop_lights.cpp`/`.h`, `level_lights.inc` (console). Modernized Polymorph,
LIGHTING ENHANCED and RELIT (with the prop lights); ORIGINAL, Vanilla and
Classic unchanged.

- **Found by rule, game-wide**, once a level and after each load (and again,
  up to 20 tries, while a candidate's texture is not yet in VRAM): a world
  face is lava when it is flat (every corner at one height), every corner's
  painted red is at least 200 and well over its blue, and its texture is
  fiery (red the strongest channel, blue under 15% of it, green 25-75% of
  it). Only faces that pass the paint and flatness tests have their texture
  read, once per texture, from the level's VRAM snapshot that the neon scan
  (or, in LIGHTING ORIGINAL, D57F's share) already takes a moment after each
  load without a GPU wait (`near_clip_vram`). Reading VRAM directly
  (`gpu_vram_peek`, or one `gpu_vram_copy`) stalled the game thread 24 ms on
  the GL readback; from the snapshot the whole scan takes 0.33 ms (level 3:
  0.08 ms of it texture reads).
- **Pools**: the faces are gathered into pools 4000 across at one height;
  each pool gives one warm light (1.00 0.42 0.10, strength 1.0, radius 9000)
  900 above its surface. They take part with the prop lights, in rooms
  shown, and only the nearest two pools light at once: the second fades as
  the third comes as near (over 2000), so walking the hall hands the light
  on without a pop. Six pools at once (the prop lights' own cap) doubled the
  GPU in the hall (see Evidence); two cost about a quarter of that.
- **A slow throb**: each pool breathes on its own phase, about 0.85..1.0 over
  3.6 s, with a faint shimmer; no flicker.
- Tuning in the pit: radius 3600 (and 700 above) lit only the bottom of the
  walls, unseen from the hall's walkways; 9000 warms the walls up to the rim
  and, faintly, the cave ceiling (a strength-5 test showed the reach).
- `lamp lava off|on` (session) compares; `lamp props off` also turns it off.
  Developer: `DNTTK_LAVA_LIGHT="r g b strength radius"`, `DNTTK_LAVA_AT="lift
  pool"`, `DNTTK_LAVA_THROB="depth seconds"`, `DNTTK_LAVA_MAX="pools
  handoff"`, `DNTTK_LAVA_TRACE=1` (each texture's verdict, the pools, the
  scan's time).

## Evidence

Mean luminance (0-255), `lamp lava off|on`, first person, warped (the final
build: two pools at once):

| View | Relit off / on | Enhanced off / on |
| --- | --- | --- |
| South walkway beside the pit (45000, -300, -20600; the test save) | 45.8 / 48.1 | 46.3 / 46.7 |
| North walkway (45000, -300, -16200) | 29.7 / 30.1 | 27.8 / 28.0 |
| Over the channel at the hall's level (35586, -200, -18197) | 41.3 / 44.3 | 40.3 / 40.5 |
| In the pit over the lava (45000, 5600, -18432) | 27.0 / 37.0 | |

- With six pools at once (before the cap) the same views read Relit 45.7 ->
  52.3, 29.5 -> 32.5 and, in the pit, 26.6 -> 42.7: the far end of the
  channel lit too, at four times the cost.
- In the pit the striped walls glow orange near Duke; from the walkways the
  pit walls below and the near rim warm; Enhanced adds the light to the full
  painted light, so it shows less there.
- ORIGINAL: identical at the still walkway views (25.0 / 24.8, 40.8 / 40.8
  with six pools; the views over the channel are not a still camera: Duke
  falls).
- Shots: `research/before-after/D57A/` (local).

- 180 Hz, Relit, the south walkway (the test save), `lamp lava off|on`,
  three pairs: pacing p50 and p95 6 ms every pass, max 7-9 ms, no image over
  10 ms, before and after. GPU (clock-weighted): six pools at once off 16.7 /
  16.5 / 17.2 %, on 33.3 / 32.6 / 32.4 % (+16 points); two pools (kept) off
  17.3 / 14.4 / 15.2 %, on 18.9 / 20.8 / 21.0 % (about +4.6 points).
- The scan: 0.33 ms once a level and after a load (level 3: 54 faces, 0.08
  ms of texture reads from the snapshot).

- Survey, all 21 levels loaded in turn (the lava trace, `DNTTK_LAVA_TRACE=1`;
  run with the build that still read VRAM directly; the final build gives
  the same verdicts at level 3 and level 0 fresh starts):

| Level | Flat glowing faces (paint test) | Lava faces (texture test) | Pools |
| --- | --- | --- | --- |
| 3 GOLD AND GUNS | 54 | 54 | 15 (rooms 49-53, the long hall's pit) |
| 0 | 46 | 0 | 0 |
| 9 | 37 | 0 | 0 |
| 7 | 19 | 0 | 0 |
| 23 | 4 | 0 | 0 |
| 5 | 1 | 0 | 0 |
| 1, 2, 6, 8, 10-12, 21, 22, 24-29 | 0 | 0 | 0 |

  The red-painted flat faces of levels 0, 5, 7, 9 and 23 fail the texture
  test (not fiery) and are not taken; what they are was not looked at.
- Vanilla (`--vanilla` on the walkway save, traces on): no lava, prop-light
  or sky-scan activity.

## Limits

- Lab-judged: the user's look on the real display decides (strength, reach
  and the two-pool cap are the knobs: `DNTTK_LAVA_LIGHT`, `DNTTK_LAVA_MAX`).
- Only the nearest stretch of the channel lights; the far end stays as the
  original painted it.
- Point lights over the surface; the lava faces themselves keep the game's
  look.
- The rule needs the painted glow: a lava texture under plain paint would not
  be found (none in the 21 levels).
