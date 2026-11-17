# D55A - Traffic lights light the street again (baked lamps)

Status: Done (accepted 2026-11-17: "lock it in and close it out"). Job board: [D55A](../jobs/lighting.md#d55a---traffic-lights-no-longer-light-the-street).
Evidence labels: CONFIRMED (code or our own measurement), INFERRED, UNKNOWN.

## Report

User, 2026-11-14, OBEY OR DIE (level 5): the traffic lights used to flash and
light the geometry around them (D55, [note 438](438-d55-traffic-lights.md));
that seemed gone, while bullet holes near them still followed the flashing.

## Causes (CONFIRMED)

Since D41M ([note 446](446-d41m-baked-lamps.md)) the level's lamps light the
world through the bake, and since D41P2 only through it. A lamp that varies
keeps a channel: a stored light shape scaled live by the lamp's colour each
tick (`lamp_bake_levels`). Two faults stopped the traffic lights there:

1. **No brightness in the bake's lamp levels.** D55 reads a signal lamp's
   frame (colour and brightness) inside the live lamp loop
   (`level_lamps_publish`). The bake's levels took only the colour, from
   `signal_last` (the colour last seen lit), at full strength: on walls and
   floors every traffic light was on all the time, amber and red together.
   It was also refreshed only for the nearest 12 lamps (the live list), while
   the bake lights every lamp.
2. **The four-channel limit.** A face holds at most four channel lamps; more
   are baked at their mean (a signal at 0.5, steady). The street under a set
   of traffic lights is reached by up to six: level 0 had 102 overflowing
   lamps on faces (101 of them traffic lights), level 9 103, level 5 12,
   level 3 8.

Bullet holes and other decals are drawn by the game over the wall and are
lit live, not from the bake (INFERRED from the report; not traced).

## Fix

- `level_lights.inc`: every signal lamp's frame is read once a tick for all
  signal lamps (`signal_state`: colour and brightness; held while the props
  cannot be read; a broken one keeps the colour last shown and sputters out
  as D54). The live loop and the bake's levels both apply it
  (`signal_lamp_now`).
- `lamp_bake.cpp`: a chart that more than four varying lamps may reach takes
  twice its width in the atlas; the copy to its right holds channels 4-5 in
  the R and G of the channel texels. Lamp ids are packed three to a float in
  the record (`[13]` lamps 0-2, `[14]` 3-5; exact below 2^24). Channel lamps
  are chosen by their light on the face times their swing (how far the style
  strays from its mean: signal and breakable 0.5, strobe 0.87, flicker and
  pulse 0.3, candle 0.11, wave depth/2), so if a face ever has more than six,
  the gentle ones are baked at their mean first. Cache key version 6: every
  level bakes again once on its first visit.
- Framework (exported patch): the bake shader decodes six ids and takes the
  second fetch only on wide charts (`i4 < 255`).
- The bake log adds `(N on/off lamps)` to the overflows and a histogram,
  `faces by varying lamps: 1-4, 5-6, 7-8, 9+`.

## Evidence (offscreen, private card copies, private bake cache, lab `recomp/analysis/d55a/`, local)

Level sweep, first bake (faces with 5-6 varying lamps; overflows after the fix):

| Level | Faces 5-6 | Overflows before | After | Atlas rows before / after |
| --- | --- | --- | --- | --- |
| 0 | 68 | 102 (101 on/off) | 0 | 1879 / 1907 |
| 5 | 11 | 12 (12) | 0 | 1395 / 1401 |
| 9 | 71 | 103 (102) | 0 | 1021 / 1048 |
| 3 | 8 | 8 (5) | 0 | 1729 / 1733 |
| 7, 29 | 0 | 0 | 0 | unchanged |

No face on these levels has more than six.

Level 5 street (Duke placed at 15600, -9600, 4000 facing the kerb lights;
the pre-fix build kept beside the fixed one), warmth (R + G - 2B) of the lit
building corner in 16 shots about 0.25 s apart:

| Lighting | Before | After |
| --- | --- | --- |
| ENHANCED (BALANCED) | 19.2 every shot | 18.2 / 5.5, in step with the amber light |
| RELIT (PRETTY, ULTRA) | 30.7 every shot | 28.3 / 5.5 |

Over-road red lights (aimed at 17700, -9600, 7557): redness 2.19-2.20 in
every shot before, 1.80 (red on) / 1.52 (red off) after. Signal trace: the
amber pair and the red pair alternate every 0.6 s, all eight lamps read each
tick. Pictures: `research/before-after/D55A/` (local; `grid-relit.png`,
`grid-street.png`, `grid-red.png`, `diff-red-after.png`).

GPU work, same view, 180 Hz, 15 s, alternated (nvidia-smi, clock-weighted):
before 20.5% and 21.8%, after 19.5% and 21.8%: no measurable cost. 180
presents a second throughout.

Tests: unit tests (260, 2 skipped), `ttk-near-test`, `ttk-input-test`,
`ttk-flash-test`, `ttk-live-env-test`, `ttk-audio-test`, the three CI runtime
tests, `check_repo`; the exported patch on a clean pinned framework matches
all 74 modified files. The codegen hash is unchanged (savestates load).

## Regression found in playtest: bright patches (fixed 2026-11-17)

User, the same day: "in save slot 7, there are now random bright patches?
... also ... obey or die". CONFIRMED cause: the shader decoded the packed
ids with `int(c3.z + 0.5)`. The empty marker (three 255s, 16777215 = 2^24 - 1)
plus 0.5 rounds to 2^24 in 32-bit float, so a face with fewer than four
channel lamps (most faces) read lamp 4 as id 0: a "wide" chart. Its second
fetch read the neighbouring chart in the atlas and added those weights in
lamp 0's colour: pink and purple slabs on pavements, roads and signs (UI
slot 7, level 0), blue-ish patches on the level 5 street. The ids are exact
integers in the float; they are now converted without the 0.5. UI slot 7,
four views, RELIT: 82,000-199,000 pixels brighter by more than 12 levels in
the faulty build against the fixed one; the patches are gone, and the
traffic lights still flash (corner warmth 5.2 / 23.0). Shots
`research/before-after/D55A/patches/` (local). The level 5 measurements in
the evidence above were taken with the faulty build; the flash itself is
unchanged by the fix.

## Profiles

- PRETTY, ULTRA (RELIT) and BALANCED (ENHANCED): traffic lights flash on
  walls, floors and the models standing there through the bake's channels:
  one multiply a channel, a second texture fetch only on the wide charts.
- SIMPLE (LIGHTING ORIGINAL): no lamp light at all, as before; the game's own
  flashing lenses show. A cheap glow at the lens (D41S style) would be the
  next step there if wanted; not built.

## Remaining

- Accepted by the user after the patch fix (2026-11-17).
- SIMPLE grade (LIGHTING ORIGINAL): a lens glow is not built.
- Field laptop: not measured (the change adds no work per pixel except on
  the wide charts; desktop showed no difference).
