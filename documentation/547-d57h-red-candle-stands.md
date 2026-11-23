# D57H - Red candles on iron stands give light

2026-11-23 (overnight session). **Needs playtest.** Job:
[D57H](../jobs/lighting.md#d57h---red-candles-on-iron-stands-give-light-ui-slot-9).

User, 2026-11-18, from their UI slot 9 save (level 8): "this kind of candle
doesnt emit light (i dont think) so make it emit".

Private runs only: lab `recomp/analysis/d57h/` (local; the D57M probe with a
private profile and its own card copy holding the user's UI slot 9 of
2026-11-18 as slot 8, copy in `research/savestates/d57h/`; offscreen on the
real GPU at 180 Hz, debug port). Every run checks the player's saves and
settings before and after (unchanged).

## The candle (MEASURED)

The user's "i dont think" was right: the candle gave no light of its own. In
the slot's room (level 8, room 36) the nearest lamps are the level file's two
candle lamps on the larger type-498 props, 2900 and 4500 away; no lamp, prop
light or scan lamp sits at this candle, and no level's lamp file binds its
prop type.

It is prop type 361, model `0x80131bd8` (one model record for every copy):
59 vertices, 190 x 730 x 190.

- A four-legged iron foot (model y 279..365, 190 across), a square iron pole
  22 across up to y -240, a drip tray (y -240..-260), the red candle (a
  hexagonal block 60 across, y -260..-339) and its flame: four crossed quads
  with the game's animated flame texture (y -336..-365).
- The prop stands at y -365 over its floor, so the flame burns about 700
  above the floor, near Duke's chest.
- First vertex (-11, 297, -11), the last (0, -365, 11).

## Build

`prop_lights.cpp`/`.h`: `candle_stand_mesh`, kind `prop_light_candle_stand`,
found by model, game-wide. Modernized Polymorph, LIGHTING ENHANCED and RELIT
(the prop-light path); ORIGINAL, Vanilla and Classic unchanged.

- One small warm candle light just over the flame's tip (model y -390), the
  level files' candle colour (1.00 0.70 0.30), strength 0.7, radius 1400.
  Strength 0.9 / radius 1500 was tried first: the wall right beside the
  candle went hot in Relit (slot view p90 81 -> 136).
- The chandeliers' gentle candle flicker (`prop_candle_level`, about 0.93 to
  1.0 per copy): a flame, not a fixture, so it does not follow a room's
  light switch (D57L).
- A soft warm halo (the warm halo texture, 200 across, level 70) round the
  flame, breathing with the same flicker. The flame itself stays the game's
  own animated texture.
- The model is a light fixture for D41AC (its flame keeps the emitter rule
  in a dim Relit room) and for `lit_mesh` (the D41D1 sign scan leaves it to
  the prop light: no double light if a lamp file is ever rescanned).
- It breaks with its prop (D54) and is found again after a load (D41W), as
  every prop light.
- `lamp props off|on` compares (it also switches the other prop lights).
  Developer: `DNTTK_PROP_LIGHT_CANDLE_STAND="r g b strength radius"`,
  `DNTTK_PROP_CANDLE_STAND_AT="y"`, `DNTTK_PROP_CANDLE_STAND_HALO="size level"`;
  the trace (`DNTTK_PROP_LIGHT_TRACE=1`) counts `candle stands`.

## Evidence

Mean luminance (0-255) of the slot's view (first person, at the candle by the
log wall). Before = `lamp props off` (the look before D57H here: the level's
other prop lights, three iron rings in rooms 43-44, are more than 30000 away):

| Mode | Before | After |
| --- | --- | --- |
| Relit | 45.9 | 70.3 |
| Enhanced | 41.5 | 54.9 |
| Original | 37.9 | 37.8 |

- ORIGINAL: the only differing pixels between before and after are the
  weapon's bob, the HUD timer and the flame's animation frame (diff mask
  `f1-orig-diff.png`); no light and no halo.
- Shots: `research/before-after/D57H/` (local), `slot-<mode>-before|after.png`
  and the other copies' rooms.
- 180 Hz, Relit, the slot's view, before/after, three pairs: pacing p50 and
  p95 6 ms every pass, max 7-8 ms, no image over 10 ms. GPU (clock-weighted)
  before 15.9 / 16.6 / 16.2 %, after 20.3 / 21.1 / 21.7 %: about +5 points
  for the two candles in view (each a published, shadow-casting prop light,
  as every prop lamp). Capping the candles to the nearest two was measured
  and changed nothing (both in view are the nearest), so no cap was kept.

- Other copies (warped to the middle of each room, `lamp props off|on`):
  room 39 (four candles round a central floor) Relit 26.1 -> 32.9, Enhanced
  30.4 -> 33.8; room 23 (four candles) Relit 59.0 -> 72.7, Enhanced 70.7 ->
  77.0.
- Vanilla (`--vanilla` on the slot, trace on): no prop-light activity.
- Survey, all 21 levels loaded in turn (lab `lampsurvey`: the model matched
  in every room by type 361 and by shape, the scan trace as a second
  witness):

| Level | Candle stands |
| --- | --- |
| 8 | 22: rooms 22 (1), 23 (4), 36 (2, the slot's), 39 (4), 40 (4), 41 (3, up on y -5485), 60 (4, below at y 3731) |
| 0-3, 5-7, 9-12, 21-29 | none |

## Limits

- Lab-judged: the user's look on the real display decides (the strength is
  the one knob: `DNTTK_PROP_LIGHT_CANDLE_STAND`).
- The light is a point light with the prop lamps' cached shadow maps; up to
  six prop lights publish at once (the nearest), as before.
- The copies of a model breathe together in their halos (the draw does not
  know which prop it draws); their lights flicker each on its own.
