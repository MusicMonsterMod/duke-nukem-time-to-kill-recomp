# D41AB - Duke 3D random lights (train beacon)

2026-11-22. **Done, user-accepted** ("lock it in and close it out. ten out of ten"). User, accepting D41AA: "my crosshair is
pointing at that overhead light. i wonder if we can make that blink in the
same way that duke3d's lights can blink. i essentially want to reimplement
that exact aesthetic ... its essentially a random flickering effect, it
doesnt look like a glitch, its got a real aesthetic to it."

Private runs only: lab `recomp/analysis/d41ab/` (local; `probe.py` from the
D41AA probe, private profile and a copy of the lab cards with the user's UI
slot 12 from `research/savestates/d41ab/` as slot 11, offscreen on the real
GPU at 180 Hz, debug port). Every run checks the player's saves and settings
before and after (unchanged).

## The reference (MEASURED from source and the tutorial map)

EDuke32 source (`research/eduke32/source/duke3d/src`, local):

- `actors.cpp`, `case SE_4_RANDOM_LIGHTS` (sector effector lotag 4, "Flashing
  lights"), run once a game tick:
  - lit when `((g_globalRandom / (hitag + 1)) & 31) < 4`: one tick in eight;
  - lit: floor and ceiling shade `SE shade + (g_globalRandom & 15)`, the
    walls the same (the comment: "Got really bright"), the sector's
    ceiling/floor palette swapped to the original; wall sprites in the
    sector (`cstat & 16`) take the sector's shade;
  - not lit: the floor shade and the darkest wall shade that `A_Spawn` kept
    (`game.cpp`, `case SE_4_RANDOM_LIGHTS`: `T3` = floor shade, `T4` = the
    walls' highest shade), the SE's own palette.
- `game.cpp` `G_DoMoveThings`: `g_globalRandom = krand()` once a tick while
  not paused; the tick is `REALGAMETICSPERSEC` = 30 a second
  (`duke3d.h`). Every SE 4 reads the same value, so lamps with the same
  hitag flash together and other hitags take other bits.
- SE 3 (`SE_3_RANDOM_LIGHTS_AFTER_SHOT_OUT`) is the same test, active once
  its light is shot out.
- Build turns shade into brightness linearly over the palette's 32 shades
  (Polymost `getshadefactor`: `(numshades - clamp(shade)) / numshades`).
- `research/Tutorial.map` (local, parsed: map version 7, 282 sectors, 333
  sprites): its two SE 4 rooms use SE shade 0 and sector shade 20 (walls
  10-15), hitags 9 and 11. So a Duke random light sits dim (shade 20, 0.375)
  and on one tick in eight flashes to a random brightness between full
  (shade 0) and shade 15 (0.53), each flash one tick (33 ms) long, sometimes
  two or three in a row.

## Change

- `recomp/src/ttk/duke_flicker.h` (new, header only): SE 4 as written above.
  `duke_tick(ms)` (30 a second), `duke_global_random(tick)` (stands in for
  `g_globalRandom`: a hash of the tick, so every thread and redraw of one
  moment agree; the game's value is `krand()` after all the tick's other
  draws, so its successive values are unrelated too), `duke_random_light()`
  (the lit test and shade), `duke_shade_factor()`, the level, and the mean
  and swing for the bake.
- Level lamps (`level_lights.inc`, D41D): a new style `duke[:H[:ON[:OFF]]]`
  (hitag, lit shade, unlit shade; `duke` alone is `duke:0:0:20`, the
  tutorial's). It works in the live lamp list and the D41M baked lamps (a
  bake channel, `lamp_bake.inc` mean and swing). Console `lamp style duke
  [hitag] [lit] [unlit]`; the `.lights` file writes `duke:H:ON:OFF`.
- Prop lights (`prop_lights.cpp`, D41L): a style option (`PropStyle`: duke
  or steady). The tick is taken once per game frame (`composition_hook`,
  `duke_tick_frame`), so a frame's light list and every image's halo read
  the same tick and flash together.
- The subway train's cyan roof beacon (D41AA) uses it: the beacon's light and
  its halo take the SE 4 level (hitag 0, lit 0, unlit 20) in every state
  (parked, running, the crash dip, the wreck). Its strength at shade 0 is
  1.2 (was a steady 0.8): dim it gives 0.45, a flash 0.64-1.2.
- Developer: `DNTTK_PROP_TRAIN_BEACON_STYLE="duke H ON OFF"` or `"steady"`
  (the D41AA beacon).
- LIGHTING ENHANCED and RELIT only (the light needs dynamic lights; the halo
  is a Modernized Polymorph extra). Original, Vanilla and Classic:
  unchanged.

## Evidence (offscreen, private copies; player files unchanged after every run)

- Unit test `recomp/tests/local/test_duke_flicker.py`: the EDuke32 rule
  tick by tick for hitags 0, 1, 9 and 11 (300000 ticks each: lit share
  0.125 +- 0.006, flashes mostly one tick, none longer than 8), hitags 0 and
  11 flash apart, the shade factor (shade 20 = 12/32), the level held for
  each whole 1/30 s tick at any image rate.
- Trace (`DNTTK_PROP_TRAIN_TRACE=1`, UI slot 12, the wreck): beacon 0.38
  dim, flashes 0.91, 0.94, 0.97.
- 540-image burst at 180 Hz, Relit, Duke still (`burst/b0`, mean luminance):
  the end door 98.8 dim, up to 130.5 at a flash; the beacon's halo 97.9 ->
  115.1; the tunnel ceiling 38.0 -> 51.4. Lit 52 of 540 images, 7 flashes of
  5, 6, 6, 6, 6, 11 and 12 images: one tick (6 images at 180 Hz) or two;
  door, halo and ceiling change on exactly the same images.
- LIGHTING ORIGINAL, same burst (360 images): door and beacon areas
  constant.
- Level lamp: `lamp here`, `lamp style duke:5:0:20`, `lamp power 2`, train
  lights off, after the bake settles: ceiling 48.7 dim, flashes to 70.6 (the
  same lamp steady: 70.6 throughout) in 13% of images, runs of 4-7 and one
  of 12. Console round trip: `duke:7:-4:22`, `duke` (= `duke:0:0:20`),
  `duke 2` (= `duke:2:0:20`).
- 180 Hz pacing, UI slot 12 Relit, train lights off / on: p50 and p95 6 ms,
  max 8 / 7 ms, none over 10 ms.
- Showcase: `research/before-after/D41AB/slot12-relit-beacon-unlit.png`
  and `-flash.png` (local); the D41AA steady beacon is
  `research/before-after/D41AA/slot12-relit-after.png`.
- Local tests: 270 run, OK (2 skipped). Executable `d0182ebc...`.

## Limits

- The painted beacon face on the train model keeps its texture brightness;
  the halo and the light flicker (Duke's SE 4 also shades the sector's
  surfaces, which here is the light's reach).
- No palette swap: Duke's SE 4 can swap the sector palette while lit; a lamp
  here keeps its colour.
- The tick comes from the draw clock, sampled when a game frame is composed
  (the prop lights) or the light list is built (level lamps); the pause
  menu was not measured (Duke's lights freeze while paused).
- The beacon's dim level (unlit shade 20) and strength 1.2 were judged
  offscreen; the user's display decides.
