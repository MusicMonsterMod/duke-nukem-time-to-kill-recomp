# D41AA - Subway train lights

2026-11-22. **Done, user-accepted** ("excellent!!! ... close out the job at hand"). User, accepting D41Z: "thoroughly inspect
the train model ... there are clearly sections here which look like
lights"; starting the job: "let's see what we can do with it to make it look
more exciting" (UI slot 10, the train before it moves; UI slot 9, saved at
the train's controls: press E and it runs).

Private runs only: lab `recomp/analysis/d41aa/` (local; `probe.py` from the
D41Z probe, private profile and a copy of the lab cards with the user's UI
slots 9, 10 and 12, offscreen on the real GPU, debug port). Copies of the
three saves in `research/savestates/d41aa/` (local). Every run checks the
player's saves and settings before and after (unchanged).

## What the game does (MEASURED)

- One train: a static prop (kind 1) of type 77, model `0x8011c41c` (69
  vertices: a car 9470 long, model x -9361..110, y -1794 (roof)..46, z
  +-898), record `0x801d5d64`. It moves: the type 77 overlay drives it
  through its state byte `+0x35` (D44B's jump table 0x800caa2c):

  | State | Meaning | Measured (UI slot 9, E pressed at 0 s) |
  | --- | --- | --- |
  | 0 | parked at (32741, -5784, 41976), yaw 2048, room 23 | until 0.78 s |
  | 1 | pulling away, the speed doubling | 0.78 - 1.09 s |
  | 2 | at 256 units a game tick (about 7700 a second), west | to x 7910, 4.25 s |
  | 3 | the crash: slews (yaw 2048 -> 1958) and skids 2044 sideways (world z 41976 -> 39932), throwing debris; the game's own white flash and fireball | 4.25 - 4.64 s |
  | 4 | stopped at (7399, -5784, 39932), room 82 (UI slot 12) | from 4.64 s |

  It relinks through rooms 23, 81, 22, 19 and 82 as it runs. Its nose (model
  x +110, a sloped cab) leads.
- The lamps the texture shows (D41Z first inspection, MEASURED from VRAM):
  a bumper cluster low on both ends (faces 0 and 2: amber, red, amber at
  texels u 72, 80, 88, v 186: model y -27), red markers on the end corners,
  and a cyan beacon on the roof of the flat tail end (face 3, x -9361..-8846,
  y -1523). No face is animated.
- Level 0's lamp file bound one D41D1 sign lamp to the train at its start
  (`sign type 77 room 23 prop 77 32741 -5784 41976`, cyan, radius 3992).
  D54 needs a bound prop to stand at exactly its place, so the moment the
  train pulled away the lamp sputtered out as if broken, and the moving and
  wrecked train had no light at all.

## Change

`recomp/src/ttk/prop_lights.cpp` (D41L prop lights), the D41X/D41Y/D41Z way:
one rule found by model data (`train_mesh()`: 69 vertices, first (-2, 46,
-899), last (-341, -357, 683); kind `prop_light_train`), no per-level data.

- The prop scan finds the train; each game frame `train_update()` reads its
  state (`+0x35` for type 77; another type with this model keeps the parked
  look), notes when it pulls away (0 -> 1) and when it hits (2 -> 3), the side
  it skids toward (its motion since the impact, in model space) and, while it
  runs, the room it is linked in (the room gate).
- Its lights are placed from the record (`+4` position, `+0x1c` yaw) every
  image, so they ride with it. Levels come from the clock
  (`train_look()`), shared by the light list and the halos:
  - bumper clusters (amber 1.00 0.45 0.06, 0.7, radius 1300) at both ends;
    the cyan roof beacon (0.14 0.76 1.00, the old sign lamp's colour, 0.8,
    radius 3000);
  - pulling away, the headlight strikes up (90 ms on, off, a flicker, then a
    ramp to full by 0.7 s) and holds with a slight hum: a warm light (1.00
    0.80 0.50, 1.6, radius 8000) 3200 units ahead of the nose, so it lights
    the tunnel walls and rails ahead rather than its own cab (1100 and 2400
    ahead were tried: the cab front washed white);
  - the impact: an orange flash (1.00 0.58 0.22, 3.2, radius 9000) beside
    the nose on the skid side, white-hot for 70 ms then a fire's flutter
    dying over about 3 s; skid sparks (a hard strobe, 2.0, radius 2800) low
    by the leading wheels on that side for 0.95 s; the beacon dips for 0.4 s;
    the headlight blows and sputters out in 1.1 s;
  - wrecked (state 4, also a later save): the smashed nose cluster holds
    steady at 0.85 after its sputter; the tail cluster and the beacon stay
    lit. (A first version buzzed and dropped out; the user read it as a
    flicker fault, see below. `DNTTK_PROP_TRAIN_BUZZ=1` brings it back.)
- Halos (the neutral halo, drawn extras in model space): one on each of the
  six bumper lamps in its colour, the beacon, the headlight while it shines,
  the impact flash and four hot sparks jumping about the leading wheels.
- `recomp/assets/lights/level00.lights`: the train's sign lamp is removed
  (the train's own lights replace it); `lamp signs` skips the model
  (`prop_light_lit_mesh`), so it is not scanned back; `lamp props off|on`
  includes the train.
- Developer: `DNTTK_PROP_LIGHT_TRAIN_LAMPS` / `_BEAM` / `_BEACON` / `_FLASH`
  / `_SPARKS` ("r g b strength radius"), `DNTTK_PROP_TRAIN_BEAM_AT="x y z"`,
  `DNTTK_PROP_TRAIN_HALO="lamp beacon beam flash sparks"`,
  `DNTTK_PROP_TRAIN_BUZZ=1` (the wreck's buzzing nose lamps),
  `DNTTK_PROP_TRAIN_TRACE=1` (state changes, and the levels, room gate,
  visibility and light budget every 100 ms).
- The lights need LIGHTING ENHANCED or RELIT (DYNAMIC LIGHTS); the halos are
  a Modernized Polymorph prop extra. LIGHTING ORIGINAL, Vanilla and Classic:
  unchanged.

## Evidence (offscreen, private copies; player files unchanged after every run)

- Trace, UI slot 9, E: `state 0 -> 1`, `1 -> 2`, the D44B quake, `2 -> 3`,
  `3 -> 4`; the beam 0.10 -> 0.63 -> 0.96 over the strike-up; at the impact
  flash 1.00, beacon 0.40, then the nose cluster's sputter and buzz; the room
  follows the train (23, 81, 22, 19, 82).
- Approach, Relit (`research/before-after/D41AA/approach-*`, local): the
  tunnel wall and rails ahead of the train lit warm, the bumper lamps
  glowing.
- Crash, Relit, mean of the top-left quarter of the view after the impact
  (`lamp props off` / on):

  | After impact | 0.35 s | 0.53 s | 0.7 s | 1.0 s | 1.35 s |
  | --- | --- | --- | --- | --- | --- |
  | off | 32 | 41 | 34 | 32 | 32 |
  | on | 82 | 82 | 79 | 63 | 57 |

  (The first flash frame, 94, is the game's own and the same either way.)
  A first try with the flash and sparks inside the car body (model z 100
  and 700) added only a few levels: the flank Duke sees faces away from
  them.
- UI slot 12 (the wreck's tail end): the beacon lights the end door, the
  tunnel ceiling (cyan) and rails, the cluster the track
  (`slot12-relit-*`, `slot12-enhanced-*`); Original identical on and off.
- UI slot 10 (parked nose, state 0): the headlight stays off until the train
  pulls away; the nose cluster is behind the weapon from that spot.
- Vanilla, UI slot 12: pixel-identical to the build before the change (0 of
  786432 pixels).
- 180 Hz, Relit: UI slot 12 `lamp props` off / on, p50 and p95 6 ms, max 9
  / 10 ms; during the ride off / on, p50 and p95 6 ms, max 8 / 9 ms; none
  over 10 ms.
- Local tests: 269 run, OK (2 skipped).

## Flicker report (user, new UI slot 12)

User: "the lights in this shot flicker if you walk a step forward and back"
(a new UI slot 12 at the wreck's nose: its bumper cluster in the gap between
the nose and the wall; copy in `research/savestates/d41aa/slot12-flicker/`).

- Ruled out (trace while walking): the room gate (room 82 shown, every train
  light at visibility 1.00 throughout), the light budget (8 free, 6 used,
  constant); the halos zoomed image by image are steady. Walk bursts showed
  no flashing beyond Duke's own motion (the per-pixel totals vary more
  between identical runs than between variants, so they decide nothing).
- Found: the wreck's buzz. Standing still, 1080 images (6 s at 180 Hz), the
  lamp area: buzz on 38.4..43.6 with 9 image-to-image jumps over 1 level;
  buzz off 41.7..43.6 and none. The dropouts switch the nose cluster at
  about 34 Hz every few seconds, a strobe on the wall right beside Duke.
- Fix: the wreck's nose cluster holds steady (0.85) after its sputter at
  the impact. Rechecked: lamp area 41.9..43.6, 0 jumps, over 1080 images.
- On the way, one burst had black captured images (one, then 30 in a row)
  during a hitch to 1.3 game frames a second: my own raw bursts (11 GB
  written, 24 GB in the write cache) stalled the capture. With the bursts
  removed, the same run had no black images and no hitch; the game was not
  at fault.

## Limits

- The lights are point lights: no beam cone or shadows; the car's own
  surfaces near a light take some of it.
- Light positions are read once a game frame; the train moves 256 units a
  tick, so a light may step against an interpolated image at 180 Hz. Judge on
  the real display.
- The side windows stay dark glass (a lit interior would be an addition the
  texture does not show).
- The crash strength (3.2) and beacon (0.8) were judged offscreen; the
  user's display decides.
