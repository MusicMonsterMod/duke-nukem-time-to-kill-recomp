# D57B - The secret DeLorean lights its cave

2026-11-23 (overnight session). **Needs playtest.** Job:
[D57B](../jobs/lighting.md#d57b---light-for-the-secret-delorean-gold-and-guns).

User, 2026-11-14 playthrough: the secret DeLorean in GOLD AND GUNS sits in
an almost entirely dark area, so the discovery is underwhelming. The job's
first plan was a lamp in the level's light file; the 2026-11-23 note on the
job prefers a rule by model (a prop light, as D57D/K/M), per the
fix-the-system rule. Done that way: no per-level data.

Private runs only: lab `recomp/analysis/d57b/` (local; the D57H probe with a
private profile and its own card copy holding the user's UI slot 9 of
2026-11-23 02:18, Duke in the car's cave looking at it, and UI slot 12,
outside its doors; copies in `research/savestates/d57b/`; offscreen on the
real GPU at 180 Hz, debug port). Every run checks the player's saves and
settings before and after (unchanged).

## The car (MEASURED)

Level 3, room 37 (a rock cave behind double doors, prop type 712). The car
is prop type 238, model `0x80128108`, at (-6053, 619, -50050), yaw 1507: 67
vertices, 756 x 463 x 2112. Its four wheels are separate props (type 237, 8
vertices, at the car's corners).

- The nose is at model -z (z -1032): the front panel's texture is a pair of
  square headlamps at each end of a black DMC grille (x +-200..+-290, y
  26..141).
- The tail is at +z (z 984..1080): the time machine's vent boxes up to the
  roof line and a louvered orange panel (y -57..53).
- The floor is at model y 182, the roof y -233.
- First vertex (-293, 141, -1032), the last (230, -57, 1020).
- Before D57B nothing lit it: no lamp in `level03.lights` near it and no
  prop-light kind; its cave reads by the room's dim painted light only.

## Build

`prop_lights.cpp`/`.h`: `delorean_mesh`, kind `prop_light_delorean`, by
model, game-wide. Modernized Polymorph, LIGHTING ENHANCED and RELIT (the
prop-light path); ORIGINAL, Vanilla and Classic unchanged.

- **Headlights on**: a warm-white halogen light (1.00 0.92 0.78, strength
  0.6, radius 2200) just before the nose at hood height (model (0, -40,
  -1350)). The renderer's lights are points, so the beam is a light out in
  front: it lights the bonnet and front wings and throws a pool on the rock
  the car faces.
- **Vents aglow**: a low orange light (1.00 0.48 0.20, 0.55, 2000) just
  behind the tail (model (0, -120, 1300)): the back of the car and the cave
  wall behind it take a warm glow.
- Both steady (no flicker) and not electric fixtures of their room (a room
  switch, D57L, does not put them out).
- Glows: a small warm-white halo on each pair of headlamps and an orange one
  on the vents (the neutral halo texture, levels 90 and 55).
- A light fixture for D41AC (its lamps and vents keep the emitter rule in a
  dim Relit room) and for `lit_mesh`.
- `lamp props off|on` compares. Developer:
  `DNTTK_PROP_LIGHT_DELOREAN_HEAD` / `_VENT` ("r g b strength radius"),
  `DNTTK_PROP_DELOREAN_HEAD_AT` / `_VENT_AT` ("x y z", model space),
  `DNTTK_PROP_DELOREAN_HALO` ("head_size head_level vent_size vent_level");
  the trace counts `DeLoreans`.

Tuning, at the user's slot (all Relit, mean luminance of the view):

| Try | Headlight | Vents | Relit after (before 31.0) | Look |
| --- | --- | --- | --- | --- |
| 1 | 1800 ahead, 1.0 / 2600 | behind, 0.6 / 1600 | 31.4 | only the rock ahead and the wall behind took light; the body stayed dim |
| 2 | at the nose, 1.0 / 2600 | 0.8 / 2200 | 56.6 | the car reads, but the cave lost its gloom (p90 57 -> 120) |
| 3 (kept) | at the nose, 0.6 / 2200 | 0.55 / 2000 | 44.7 | the body reads, a warm glow at the tail, the cave stays dark above |

## Evidence

Mean luminance (0-255) of the slot's view. Before = `lamp props off` (no
other prop light and no lava pool near the cave):

| Mode | Before | After |
| --- | --- | --- |
| Relit | 29.8 | 45.7 |
| Enhanced | 32.0 | 38.5 |
| Original | 31.6 | 30.5 |

- ORIGINAL: the only differing pixels are the weapon's bob and the HUD
  timer (diff mask in the lab, `s0-orig-diff.png`); the headlamps' glow seen
  there is the model's own texture.
- UI slot 12 (outside the double doors): every mode identical before and
  after (the lights are gated to the car's room).
- Shots: `research/before-after/D57B/` (local): `slot-<mode>-before|after.png`
  and the two rejected tunings.

- 180 Hz, Relit, the slot's view, `lamp props off|on`, three pairs: pacing
  p50 and p95 6 ms every pass, max 6-7 ms, no image over 10 ms; GPU
  (clock-weighted) before 7.2 / 5.9 / 7.3 %, after 10.8 / 8.3 / 9.0 %: about
  +2.5 points for the car's two lights (shadowed prop lights).
- Vanilla: the prop-light path does not run (D57H's check on the same build:
  no prop-light activity; `--vanilla` keeps the original game).

- Survey, all 21 levels loaded in turn (lab `lampsurvey`: the model matched
  in every room by type 238 and by shape, the scan trace counting
  `DeLoreans`): level 3 room 37 only, the one car. No other level has it.

## Revision 2 (2026-11-23): a moodier cave, the stands on the floor

User, reviewing the overnight build: "lighting on the car itself is accepted
- but can we make the room lighting overall moodier so the lighting on the
car becomes more of a focal point ... also, place the boxes that the car is
propped up on, on the ground as it's floating above ground awkwardly. i
think the elevation is actually important so may be worth just making the
boxes a little taller so they touch the ground but keeping the car the same
height (you make the call here dont ask me)".

**The room (MEASURED).** Relit took room 37 as an unlit room (no lamp of its
own: the 80% default ambient), and its two wall torches (the level file's
candle lamps on prop type 376, strength 1.0, radius 2545) are baked lamps
(D41M): a change at publish time does not reach them.

**Showcase room** (`club_lights.inc`, `relit.inc`; by model, game-wide): a
room holding the DeLorean is a showcase room. In RELIT its painted light is
turned down to 35% (it follows `lamp props off|on`), and its other lamps (the
level file's: the two torches) shine at 45% of their colour, set before the
bake reads them (so the level re-bakes once; `lamp save` writes the file's
values back). Developer: `DNTTK_SHOWCASE_AMBIENT`, `DNTTK_SHOWCASE_LAMPS`
(percent). The trace line `[TTK showcase]` counts the rooms and lamps.

**The stands** (`prop_lights.cpp`, `near_clip.cpp`; by geometry, game-wide):
the car rests on four crates (prop type 237, one model `0x80128048`, 8
vertices, 262 x 125 x 268, bottom at model y 0). Their bottoms are at y 927
(three) and 894 (one), the cave floor at 1024: 97 and 130 in the air. The
rule: a prop under the car's body (within 1300 of its centre) whose top meets
the car's underside (within 64) is a stand; the floor under it is the nearest
flat world face of the room. Its model gets a copy whose bottom corners go
down to the floor under its highest copy (+8 into the floor; the lower
copies sink a little, unseen): bottom 0 -> 138. The crates are taller, the
car keeps its height. The copy lives in a block of mod memory allocated on
the first live frame (before any redraw worker is forked) with its table
(level, model, copy), filled by the live prop scan; the near-clip object
path draws the copy in place of the model (and gives the model back if the
host path declines it). Modernized Polymorph in every LIGHTING mode (it is
geometry); Vanilla and Classic unchanged. Developer:
`DNTTK_DELOREAN_STANDS=0`; `DNTTK_PROP_LIGHT_TRACE=1` reports the stands.

**Evidence** (the user's UI slot 9; before = `lamp props off`):

| Mode | Before | After (rev 2) | After (rev 1) |
| --- | --- | --- | --- |
| Relit | 25.5 | 35.1 | 45.7 |
| Enhanced | 31.1 | 38.3 | 38.5 |
| Original | 30.4 | 30.4 | 30.5 |

- Relit: the ceiling and walls fall into gloom and the car, its tail glow and
  the headlit wall carry the cave (`research/before-after/D57B/rev2-*`).
  The torches' dimming is in "before" too (it is set for the level).
- The crates stand on the floor in every mode (`rev2-stands-zoom.png`); 40
  consecutive presents at 180 Hz (redraw workers live) all show them (region
  mean 24.3 in every one).
- 180 Hz Relit, three pairs: pacing p50/p95 6 ms, max 6-7, none over 10;
  GPU before 10.5 / 9.2 / 10.8 %, after 13.9 / 10.3 / 12.6 % (as rev 1).
- Vanilla on the slot: no showcase, stand or prop-light activity.

## Limits

- Lab-judged: the user's look on the real display decides. The two
  strengths are the knobs (`DNTTK_PROP_LIGHT_DELOREAN_HEAD` / `_VENT`), and
  for the room `DNTTK_SHOWCASE_AMBIENT` / `DNTTK_SHOWCASE_LAMPS`.
- Enhanced keeps the room's own painted light (only Relit dims a room);
  Enhanced shows the car's lights over it as before.
- The crate texture is stretched over the taller sides (it reads as taller
  crates).
- Point lights: there is no real beam cone; the "headlights" are a pool
  ahead of the nose.
