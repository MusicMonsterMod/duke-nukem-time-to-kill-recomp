# D55 - Traffic lights light the street in their own colour and flash

Status: **Done** (accepted 2026-11-10). Modernized Polymorph with DYNAMIC
LIGHTS only; Vanilla and Classic unchanged. Builds on the D41D1 sign lamps
([note 429](429-d41d1-signs-waves-levels.md)) and D54 broken lights
([note 437](437-d54-broken-lights.md)).

## Report

User, UI slot 6 (level 5 street), after D54: "I'd like traffic lights to all
be emitters, but they need to be synced to the color and the flash.
currently we have a flicker effect that is similar to the fire flicker.
essentially in the future we could put down a custom traffic light somewhere
and our colors would be correct".

Before: the four traffic light props on the street and the four over it had
`sign` lamps in a fixed amber (`1.00 0.86 0.00`) with the `candle` style, a
random fire flicker, unrelated to what the lights show.

## How TTK flashes a traffic light (CONFIRMED, level 5)

Read live on a private copy of UI slot 6 (file slot 5):

- Two traffic light models: type 217 (on poles at the kerb, mesh
  `0x80127a5c`) and type 216 (hanging over the road, mesh `0x8012783c`).
  Each has one animated face (texture word 2, bit `0x20 << 25`).
- An animated face takes its frame from its mesh's animation table: the
  pointer at mesh `+0x0c` (the one the object renderer puts at
  `ctx+0x60`, `0x800d67a8`), 16-byte entries at `(t1 >> 16) * 16`. Entry
  `+8` points at the current frame words `s1, s2`, added to the face's
  texture words as in `0x800105d0`.
- That pointer never moves: it points 12 bytes into a running-animation
  record (a 44-byte doubly linked node), and the game copies each new frame
  into it. The record keeps its script at `+28`: word 0 is
  `(frames - 1) << 24 | ms per frame`, word 1 points at the frames' 16-bit
  indices into the frame table (pointer at `0x800c5780`, 12 bytes a frame).
- The traffic lights run 2 frames of 600 ms (`0x01010258`): type 217
  frames `0x43` (lit amber) and `0x44` (dark), type 216 frames `0x41`
  (dark) and `0x42` (lit red). Both run in step, so the amber lights are lit
  while the red ones are dark and the other way round (sampled 20 s: a
  change every 0.58-0.63 s).
- Colours, read from VRAM through each frame's CLUT: 217 lit frame amber
  texels up to `(240, 208, 0)`, dark frame at most `(64, 48, 0)`; 216 lit
  frame red up to `(248, 0, 0)`, dark frame at most `(40, 0, 0)`.

## What is in

- New lamp style **`signal`** (`level_lights.inc`). Each game tick a signal
  lamp finds its standing prop (the D54 `prop TYPE x y z` entries, through
  the shared prop snapshot), reads the prop's animated faces' current frames
  and takes:
  - the colour of the frame's lit texels (the sign scan's emitter rule and
    dominant hue), and
  - its brightness against the animation's brightest frame (all frames from
    the script), so a dark frame is a dark lamp.
  The switch is on the game tick the texture changes, as the original's.
  Frame glows are cached per face and frame (cleared on level and savestate
  loads). When the prop breaks the lamp keeps the colour last shown while it
  sputters out (D54).
- **The sign scan picks it by itself**, whatever the prop's type or level:
  an animated sign whose animation runs 100 ms frames or slower and has a
  frame at most a tenth as bright as its brightest gets `signal`. Flames run
  faster (40-66 ms frames) and keep `candle`, though a sputtering fire
  (level 7, type 372) has dark frames. Such flashing signs count at half the
  usual glow (the red over-road lens scores 2339 against 3000), and their
  lamps reach at least 3000 (the over-road lights hang about 2000 above the
  street; at the old 1629 they lit nothing). A traffic light placed or
  spawned anywhere and scanned (`lamp signs`), or a lamp given `lamp style
  signal` and tied to it with `lamp bind`, shows its colours.
- The scan scores animated faces on their animation's brightest frame,
  through the mesh's own animation table (`+0x0c`). Before, it used the frame
  showing and the table of the last object drawn: the red lights were scored
  through the amber lights' table (their old amber lamps), and on levels 0
  and 9 the traffic lights, dark at scan time, got no lamp at all.
- Level files (only these lamps changed; every other lamp as it was):
  - level 5: the eight traffic light lamps take `signal`, radius 3000; 216's
    fallback colour is red;
  - levels 0 and 9: the same eight traffic lights (the streets are shared),
    added from the scan;
  - level 29: the 16 pulsing cyan neon lamps (types 1014, 1015: 7 frames of
    142 ms, bright to dark and back) go from `flicker` (a random buzz) to
    `signal`, at their old places.
- `DNTTK_SIGNAL_TRACE=1` logs each signal lamp's level and colour per tick.

## Evidence

Private copies of the player's cards and savestates (local lab
`recomp/analysis/d55`), offscreen, debug port 9562; player saves, cards and
profile hash-checked unchanged after every run.

- Signal trace, UI slot 6 copy: lamps 26 and 27 (type 217) amber `1.00
  0.86 0.00` at level 1 for 18 ticks, then 0; lamps 28 and 29 (type 216)
  red `1.00 0.00 0.00` at level 1 while the amber ones are 0 (596 lamp
  ticks: amber 119 on and 127 off each, red 127 on and 119 off each).
- Sign scan on every level with a file (private copy of the files): signal
  lamps on levels 0, 5, 9 (eight traffic lights each) and 29 (16 neon); no
  other level.
- Matched shots, same camera and the same animation phase, old files against
  new (`research/before-after/D55/grid-street.png`, `grid-over.png`,
  local): before, the amber pool stays through both phases; after, it is lit
  in the amber phase and dark in the red one. `diff-after3.png` (red phase
  minus amber phase): red on the brick wall under the over-road lights,
  amber gone from the road around the kerb lights.
- D54: bit 2 set by hand on a traffic light: `lamp 26 goes out`, and its
  signal reading stops.
- Build OK; `ttk-input-test`, `ttk-near-test` pass. No framework change.

## Limits

- Levels 0, 9 and 29 checked by scan and trace, not looked at in play.
- The original painted light under the lights is unchanged.
- Strength is the file's (1.0); the red over-road light on the street is
  faint at that height. `lamp radius` and `lamp power` tune it.
