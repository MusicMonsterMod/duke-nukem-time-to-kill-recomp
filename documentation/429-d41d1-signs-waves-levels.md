# D41D1 - Level light sources, second part (signs, waves, more levels)

Status: **Done** (user-accepted 2026-11-09: "lock it in and close it out"). Modernized Polymorph with DYNAMIC
LIGHTS only; Vanilla and Classic unchanged. Builds on D41D
([note 428](428-d41d-level-lights.md)) and the D41A per-pixel path
([note 413](413-d41-lighting-case-study.md)).

## What is in

- **Signs give light (`lamp signs`).** Glowing props are found by their
  textures and each gets a lamp in its own colour, just in front of the
  glowing faces: EXIT and SECURITY signs, the club neon, red tube lights,
  blue, green and pink lit panels, mine lanterns. Game-wide rule, no list of
  types (`level_lights.inc`, `level_lamps_signs`).
- **The original animated light as a lamp style (`wave:P:D`).** A lamp can
  follow TTK's own light table live, phase P (0-31), depth D (0-1). `lamp
  scan` gives it to painted-light clusters whose vertices take the table and
  agree on a phase.
- **Room gate.** A lamp lights only while Polymorph shows its room (the D17AR
  host visibility walk), fading over 8 ticks, so a sign behind a wall no
  longer tints the room in front of it. `lamp gate off` compares.
- **Scans keep each other's lamps.** `lamp scan` replaces only earlier scan
  lamps; `lamp signs` replaces only earlier sign lamps; placed lamps stay. A
  sign already lit by a placed lamp of a similar colour within 2500 gets no
  second lamp (level 0's accepted Bootylicious lamp).
- **First lamp sets for every selectable level** (`recomp/assets/lights/`),
  each `lamp scan` + `lamp signs`, for the user's review per level.

## User report: club inner sign smudged (fixed, neon signs only)

User, UI slot 7 (level 0, inside the club): "the bootylicious sign suffers
the same issue as our exit sign from earlier". The inner sign (prop type 51,
one 88 x 64 face) stores its neon dim (max 0.4-1.0, saturation 0.5-0.7) and
is brightened by its corner colour, so the D41D per-texel keep rule does not
hold it and lamps fill in its darker strokes and halo.

A first fix left every glowing prop out of dynamic light, with a `lamp glow`
compare switch; the user saw no difference in play and it was shelved
(files local under `recomp/analysis/d41d1/shelved-glow/`). After the
difference was explained (the lamp's glow on the room stays; only light on
the sign's own letters goes), the user chose: "limit to the neon signs.
leave out the lamp glow toggle".

Now: a static prop with a face (64 texels or more, not animated) of neon on
a dark ground, 40% of texels darker than 0.2 and 10% at max 0.7 or more and
saturation 0.5 or more outside flame hues (12-70 degrees), takes no dynamic
light (`near_clip.cpp` `lit_frame`, `near_clip_set_glow_meshes`). The set
is found 45 ticks after each level or savestate load and once more when the
object animation table appears (18-39 ms once, offscreen). Its lamp still
lights the room around it. Lanterns, torches (animated or flame-coloured)
and lit panels still take light. No console switch.

| Level (private slot) | Neon meshes | What |
| --- | --- | --- |
| 0 (UI 4, 7) | 18 | club inner sign, SECURITY, red light panels, red tubes |
| 5 (UI 6) | 12 | |
| 2 (UI 3) | 14 | a blue-white lamp lens (type 760) |
| 3 (UI 1) | 1 | |
| 1 (UI 5), 6 (UI 2) | 0 | (first try caught level 1's lantern glows: hue rule) |

UI slot 7 crops (`research/before-after/D41D1/club-inner-sign-neon-before-after-off.png`,
local: before, after, lamps off): sign pixels differ from lamps off by 24.5
on average before, 0.1 after.

## Findings

### The animated light table (CONFIRMED, level 0 and the sweep)

- The pointer at `ctx+0x68` (render context `0x800d67a8`) steps one 4-byte
  entry per game tick (every second field) through a static table,
  `0x800da350 + 4 * phase`, phase 0-31, then wraps. The table holds 64
  entries, `3072 * |sin(pi k / 32)|` (0 at k = 0, 3072 at k = 16, two
  periods), so `pointer + 4 * 31` is always inside it.
- A world vertex with colour-index bit 4 (`w0 >> 24 & 0x10`) carries its
  phase in the colour word's top byte (`w1 >> 24`, 0-31; never written at
  run time, 3727 vertices on level 0 unchanged over 30 samples). A polygon
  whose corners all have bit 4 is drawn with DPCS toward the far colour by
  `table[pointer + 4 * phase] / 4096` (`0x80011908`; `near_clip.cpp` does the
  same). So the original light is a travelling ripple, darkening each vertex
  from 1.0 to 0.25 of its painted colour on a 32-tick period.
- On level 0 the phases inside a painted-light cluster mostly disagree
  (coherence 0.03-0.25: a shimmer that averages out); four clusters agree
  enough (0.35-0.55, rooms 63, 70, 72) and their scan lamps now follow the
  wave. A wave lamp's brightness is `1 - D * table / 4096`, divided by its
  mean over one period, so its average stays its strength (depth 1: 0.48 to
  1.91 of strength, the original's ratio).

### Signs are props, not world polygons (CONFIRMED)

- Level 0's world textures (2357 entries, `ctx+0x58`, 12 bytes: CLUT and UV,
  tpage and UV, UV pair) contain no signs. The EXIT signs (type 171), the
  club's Bootylicious sign (type 475), neon tubes, lanterns and lit panels are
  static props (object kind 1) drawn by the object renderer `0x80010000`
  from mesh `+0x44`: `+6` vertex count, `+0x10` 8-byte vertices, `+0x14`
  polygon groups, textured records carrying their texture words inline at
  `+4/+8/+12`, flags in the top 7 bits of `+8`.
- Animated faces (flag `0x20`) take their current frame from the object
  animation table: 16-byte entries `(t1 >> 16) * 16`, current frame pointer
  at `+8`, frame words `s1, s2` added as in `0x800105d0`. `ctx+0x60` holds
  that table only while objects draw (0 at game-tick time), so
  `near_clip.cpp` records it (`near_clip_object_animation`).
- One-sided prop faces wind so that the corner cross product points into the
  model (3104 of 3462 closed-model faces on level 0); the lamp goes 300 units
  along the opposite direction, rotated by the prop's yaw (`+0x1c`).
- VRAM is read through `gpu_vram_peek` (the backend syncs its copy first) on
  the game thread, the same thread the debug server reads it from.

### The emitter rule

Texels inside each face's UV box, through its CLUT, scored
`smoothstep(0.75, 0.95, max) * smoothstep(0.6, 0.9, saturation)` (stricter
than the D41D shader rule that keeps signs crisp). Line art on a dark ground
(40% or more of the texels below 0.2: neon outlines) counts three times its
fraction. A prop with glowing area (face area x fraction) of 3000 or more
gets a lamp, radius `1200 + 6 * sqrt(glow)` within 1500-5000, strength 1.0.
Its colour is the dominant hue: lit texels (max 0.6-0.9, saturation 0.3-0.6)
in 12 hue bins weighted by face area, the top bin and its neighbours averaged
(the club sign is pink by its neon, not yellow by its bulb ring). Props whose
glow is mostly animated get `candle` when warm (lanterns) or `flicker`
(neon). Pickups (type flag `0x20000000`) are skipped. Props of one place and
colour within 600 give one lamp.

Rule check on level 0 (offline prototype `tex_scan.py`, `obj_scan.py`,
`rule_eval.py`, then in game): the first, looser rule also took sand, wood,
a gold trim bar and the first-aid box; the strict rule keeps EXIT (171),
SECURITY (858), the club sign (475), the "Club" neon (16), magenta neon (193),
red tubes (2, 4, 79, 478), blue/green/pink panels (77, 176-179), the yellow
lamp (213) and red lights (170).

## Level sweep (2026-11-09)

Every selectable level, reached with the D26A `level N` travel from a
private copy of UI slot 4, 150 frames after arrival: `lamp scan`, `lamp
signs`, `lamp save` (`sweep2` in `probe.py`). Files in
`recomp/assets/lights/levelNN.lights`. Signs found per level, mostly:

| Level | Scan lamps (wave) | Sign lamps | Main sign kinds |
| --- | --- | --- | --- |
| 0 | 51 (4) | 30 (+1 placed) | EXIT, SECURITY, club neon, tubes, lit panels |
| 1 | 9 | 40 | lanterns (warm) |
| 2 | 24 (2) | 73 | mine lanterns, cyan lights |
| 3 | 18 (2) | 147 | mine lanterns |
| 5 | 24 (2) | 44 | street signs, red and yellow lights, neon |
| 6 | 21 | 25 | |
| 7 | 16 (1) | 70 | wall torches (candle) |
| 8 | 1 | 51 | torches, blue lights |
| 9 | 14 | 39 | |
| 10 | 6 | 147 | torches, red lights |
| 11 | 8 | 127 | torches |
| 12 | 11 (3) | 95 | braziers, blue lights |
| 21-26 | 1-16 | 1 each | |
| 27 | 4 | 7 | |
| 28 | 0 | 0 | |
| 29 | 3 | 20 | cyan pillars (flicker) |

Sweep shots `sw-L*.png` (one per level, local). Lanterns, torches and
braziers are warm animated props: candle style. Level 0 keeps its placed
Bootylicious lamp; the club sign is not doubled.

## Evidence

Private copies of the player's savestates and cards, Xvfb, debug port,
`recomp/analysis/d41d1/probe.py` (local); player saves, cards and profiles
hash-checked unchanged after every run. Shots local under
`recomp/analysis/d41d1/`.

- Subway EXIT (UI slot 9), lamps off / D41D lamps / with sign lamps
  (`ex2-trip.png`): the sign's red light falls on the corridor walls and
  ceiling around it. Before the room gate (`ex-trip.png`) the blue "Club"
  sign behind the wall (room 82, 3400 away) tinted the corridor's right
  wall; with the gate it is gone.
- Club street (UI slot 4, walked to the door, god mode;
  `cl5-trip.png`): the sign scan finds the Bootylicious sign (animated
  neon) and colours it pink; on level 0 the accepted placed lamp already
  lights it, so no second lamp is added.
- Mine lantern, level 3 (UI slot 1, `l3-trip.png`): warm candle glow on
  the timbers around the lantern.
- Wave: a test lamp `wave 0 1` traced every 10 ticks: the pointer moves 10
  entries per 10 ticks and the lamp follows the table entry (0.48-1.91).
- Sweep: see the table above.
- Vanilla: unchanged (`lamp` refuses outside Modernized; no lamps load).
- Build OK; framework untouched (all changes are title code under
  `recomp/src/ttk/`). `ttk-input-test` and `ttk-near-test` pass.
- Installed files load in the player build (`level 0: 82 lamps`).

## Limits

- Light still passes through walls between rooms that are both shown (no
  shadows: D41F). The room gate removes the worst cases only.
- A lamp in a room that is not shown does not light the shown room next to
  it (light around a corner can fade out as the room leaves view).
- Signs with animated faces are scored on the frame current at scan time.
- The neon pass costs 18-39 ms once, 1.5 s after a load (offscreen; not
  measured on the player's machine).
- Not measured at 180 Hz on the player's GPU; per image still at most 12
  lamps (unchanged from D41D).
- The level sets are automatic drafts; each level needs the user's review
  (`lamp` editor: pick, del, power, style, save).
