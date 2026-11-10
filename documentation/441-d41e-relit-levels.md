# D41E - Relit levels

Job D41E (selected 2026-11-10; dependencies D41C, D41D, D41F all Done).
Status: Done (2026-11-10, user: "lock it in and close it out").
Modernized Polymorph only; Vanilla and the Classic renderer unchanged.
Builds on the D41A per-pixel path ([note 413](413-d41-lighting-case-study.md)),
the D41D/D41D1 lamp files ([428](428-d41d-level-lights.md),
[429](429-d41d1-signs-waves-levels.md)) and D41F shadows
([439](439-d41f-shadows.md)).

Evidence labels: CONFIRMED (measured or seen in our own captures), INFERRED,
UNKNOWN.

## 1. What is in

- **LIGHTING option.** OPTIONS > DISPLAY > LIGHTING: ENHANCED (default),
  RELIT, ORIGINAL. It replaces the DYNAMIC LIGHTS row and keeps its profile
  key `dynamic_lights` (`on`, `relit`, `off`), so no schema migration; live,
  like before. `run.py --lighting enhanced|relit|original` (and the older
  `--dynamic-lights on|relit|off`), terminal `--settings` **I Lighting**,
  console `light mode enhanced|relit|original` (session only, for matched
  comparisons). Vanilla is always `off`.
- **Relit ambient** (`recomp/src/ttk/relit.inc`). The painted vertex light of
  lit surfaces (world, props, actors, Duke's model and first-person layers:
  everything the D41A path lights) is scaled by an ambient chosen for the
  room the camera is in (the host visibility walk's source room), eased at
  1/48 per game tick (0.4 in about half a second) when it changes, like eyes
  adjusting; a level or savestate load starts at the room's value. Per level,
  in the lamp file:
  - `ambient P` rooms with lamps of their own (default 60);
  - `ambient P unlit` rooms with no lamp of their own (default 80; never
    below the first);
  - `ambient P outdoor` sky rooms (room flag `+12 & 8`; default 85);
  - `ambient P room N` one room;
  - `relit lamps G` / `relit reach R`: the level's scene lights (placed and
    sign lamps, D53 screens, D41L prop lamps; not shots, fire or blasts) are
    G percent as strong (default 170) and reach R percent as far (default
    135) in Relit, eased with the ambient.
  Older builds skip these lines (they parse no lamp). Console `lamp ambient
  ...` reads and edits them; `lamp save` writes them.
- **Readability curve** (framework, `gpu_gl_renderer.c` `lit_col`). The
  ambient does not scale flat: the factor is `amb + (1 - amb) * 0.08 / (0.08
  + max(r, g, b))` of the vertex colour (PS1 units, 0.5 neutral). Monotone
  for every ambient, exactly 1 at ambient 1 (Enhanced unchanged bit for bit),
  and dark colours keep more of their light than bright pools: at ambient 0.6
  a neutral 0.5 becomes 0.33 (x0.66), a dark 0.1 becomes 0.078 (x0.78).
- **Lamps behind their wall** (bug fix, both Enhanced and Relit). D41D1's
  sign scan puts a prop's lamp 300 units along the glowing faces' outward
  normal (from face winding). For some wall-mounted props it points into the
  wall: level 3's mine lanterns had their lamps about 150 units behind the
  planks. Before D41F the light came through the wall anyway; with D41F
  shadows (and the single-panel side rule) such a lamp lit nothing on the
  player's side. Now, once the room mesh is built, every prop-bound lamp
  whose segment from its prop crosses the mesh is mirrored through the prop
  (or, if both sides are closed, pulled back to just before the wall), 8 a
  tick, logged `[TTK lamp] level N lamp I was behind its prop's wall`. New
  `lamp signs` scans apply the same check. Helper `shadow_ray`
  (light_shadows.inc) is the D41F console ray, factored out.
- **Lamps seen from afar** (user's first Relit review, UI slot 8, the
  level 2 mine corridor: "the lighting isnt visible longer distance ... you
  can see the lighting fade in the closer you get to the light source
  directly ahead"). Cause: D41D published a lamp only while its reach came
  within 6000 of Duke, faded in over the last 2000; D41L prop lamps used
  the same window. The torch lamps on the corridor's end wall (lamps 36 and
  37, 7000-10000 ahead) were left out from the far end and ramped up as
  Duke walked (published strength 0.36, 0.72, 1.25, 1.49, 1.68). In
  Enhanced the painted light hid it; in Relit, where the lamps carry the
  scene, it showed. Now every lamp whose room is shown (room gate) is a
  candidate, nearest first; the budget stays 12 (6 for prop lamps). A lamp
  fades only when nearer ones push it out: over the last 4000 before the
  first lamp left out, so a swap is continuous, and past 32000 (a safety
  limit). Lamps out of sight or broken (D54) take no place in the budget.
  Both modes.
- **Lamps that stay on when turning** (user's second review, the new UI
  slot 8: "look to the right 90 deg quickly, and you'll see the light fades
  in"). Cause: the D41D1 room gate let a lamp light only while Polymorph
  showed its room, and the rooms shown face the camera. A quick turn swapped
  the lamps (a trace: 6 lamps behind, then 11 for 8 ticks, then 6 others),
  each fading in over 8 ticks (prop lamps 0.25 s). With shadows on (walls
  stop the light, D41F) the gate now also takes every room within 3 portals
  of the camera's room (room +0x30 portal count, +0x38 36-byte portals,
  target +14), whichever way it faces, and a lamp whose room comes in turns
  on at once and only fades out. With `light shadows off` it stays the
  rooms shown, as before. Shadow maps: more casters than the 16 maps now
  happen (18 at most on level 0), so the weakest by the D41F score go dark
  rather than light through walls, the last ones in fading toward the
  first left out (within 1.5x of its score); a light whose reach grew while
  the 8 new maps a tick are used up keeps its shorter map; one with no map
  yet (the tick after a load) stays dark for that tick. Both modes.
- **Level 29** (Moloch's arena, already the darkest level: 48% of the
  sweep view below luminance 10 in Enhanced) gets `ambient 80` and
  `ambient 90 unlit` in its lamp file. Every other level uses the defaults
  for now.

## 2. Evidence

Private copies of the player's savestates and cards, Xvfb (llvmpipe),
debug port, `recomp/analysis/d41e/lab.py` (local); player saves, cards and
profiles hash-checked unchanged after every run. Matched shots in
`research/before-after/d41e-relit/` (local).

- Untextured world polygons (which the lit shader never sees) are
  negligible: 4096 untextured triangles over a whole run, none with world
  positions, against more than 62000 lit textured ones (temporary counter,
  removed). Relit therefore dims everything that the dynamic lights reach,
  and nothing else is left bright. CONFIRMED.
- Mine, level 3 (UI slot 1): with shadows on, the lantern lamp lit nothing,
  in Enhanced as well (`mine-lantern-enhanced-before-wallfix.png`). The ray
  from the lamp toward Duke hit the plank wall at x -7168 after 177 units,
  with the lamp at -7316 and the lantern prop at -7102. After the wall check
  (30 lamps moved on level 3) the lantern lights its timbers again
  (`...-after-wallfix.png`). In Relit the lanterns throw warm pools and the
  corridor between them is darker (`mine-lantern-relit.png`). CONFIRMED.
- The first Relit attempt (ambient only, lamps unchanged) just darkened the
  scene. Lamps on the 1x painted light barely show, so Relit also needs
  stronger lamps (the `relit lamps` / `relit reach` defaults). CONFIRMED in
  shots.
- Corridor fade (private copy of UI slot 8, 6 steps of 45 frames toward
  the end wall, each shot lamps on and `lamp off`): the lamp-on minus
  lamp-off peak rose 39, 80, 87, 97, 109, 142 as Duke closed in. Not a
  shader distance term (none exists), not the shadows (with `light shadows
  off` it still grew), not the world positions (the D41F probe at rest:
  corners within about 1 unit of the room mesh at every eye distance). A
  trace of the published lights showed the torch lamps entering from the
  margin. After the fix they publish at full strength from the far end
  and the end wall glows from the start
  (`mine-corridor-walk-before-after-range.png`, `...-far-before-range.png`,
  `...-far-after-range.png`). Level 0 (90 lamps) and 7: at most 16 lights
  per image, each with a shadow slot, so none lights through walls.
  `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS. CONFIRMED.
- Quick turn (new UI slot 8, about 90 degrees in 15 frames, a shot every
  12 frames, published lights traced every tick): before, the set changed
  from 6 lamps to 11 and to 6 others as Duke turned, ramping in; after, the
  same 12 lights through the whole turn, each only following its own
  flicker style. Shots `mine-turn-before-after-gate.png`,
  `mine-turn-before-gate.png`, `mine-turn-after-gate.png` (the first frame
  after the turn is darker in every run: the camera settling and the
  llvmpipe face dropout of D17BG, not the lights). Levels 0 and 7: lights
  without a shadow map only for one tick after a load, and dark then.
  Native tests PASS. CONFIRMED.
- Level sweep: every selectable level (0-3, 5-12, 21-29), reached with
  `level N` from a private copy of UI slot 4, 700 frames after arrival,
  Enhanced then Relit from the same view. The camera room's value and mean
  luminance (Relit / Enhanced):

| Level | Room (value, why) | Relit / Enhanced mean |
| --- | --- | --- |
| 0 | 2 (85, outdoor) | 0.94 |
| 1 | 34 (85, outdoor) | 0.96 |
| 2 | 10 (80, unlit) | 0.85 |
| 3 | 2 (60, lamps) | 1.22 (lantern pools) |
| 5 | 5 (85, outdoor) | 1.00 |
| 6 | 1 (85, outdoor) | 0.90 |
| 7 | 0 (60, lamps) | 0.84 |
| 8 | 0 (80, unlit) | 0.86 |
| 9 | 1 (60, lamps) | 0.84 |
| 10 | 19 (80, unlit) | 0.83 |
| 11 | 0 (85, outdoor) | 0.89 |
| 12 | 6 (60, lamps) | 0.87 |
| 21 | 2 (80, unlit) | 0.70 * |
| 22 | 8 (80, unlit) | 0.82 |
| 23 | 11 (60, lamps) | 0.74 |
| 24 | 9 (80, unlit) | 0.91 |
| 25 | 5 (80, unlit) | 0.85 |
| 26 | 17 (80, unlit) | 0.93 |
| 27 | 1 (60, lamps) | 0.80 |
| 28 | 0 (85, outdoor) | 0.91 |
| 29 | 6 (80 after the level file, lamps) | 0.90 (0.76 before) |

  \* Level 21's and level 0's Enhanced shots caught a red damage flash (Duke
  was hit while idle), so their ratios are not like for like. One view per
  level is only a first readability check; the user reviews each level in
  play.
- Build OK; `test_player_profiles` 135 OK (new `LightingOptionTest`);
  `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS. Framework
  patch exported and verified: a clean checkout of the pinned framework
  (`08ec704a`) plus the patch matches every patched file.
- Not measured: the cost on the player's GPU at 180 Hz. Two runs on the
  real display stopped before the first frame (the window never came up,
  probably because the screen was asleep or locked; nothing was left
  running). Relit draws no more lights than Enhanced. The lamps reach 1.35x
  further, so more pixels take each lamp, and a lamp's shadow map is redrawn
  once while its reach eases.

## 3. Limits

- The ambient follows the camera's room, so rooms seen through a doorway
  take the camera room's value, not their own.
- A lamp more than 3 portals from the camera's room still lights only while
  its room is shown; turning toward one fades nothing in, but turning away
  fades it out over 8 ticks (its light rarely reaches that far).
- The level values are the defaults everywhere except level 29. Each level
  needs the user's review in play (`lamp ambient ... here`, `lamp save`).
- Prop lamps (D41L) and TV screens (D53) do not count as "lamps of a room"
  for the unlit rule. Only lamp-file lamps do.
- The wall-check fix moves lamps in memory at load. The level files keep
  the old positions until a `lamp save` (the check runs again on every
  load). It needs shadows on (the room mesh), so with `light shadows off`
  the old positions stay; they then light through the wall as before D41F.
- Sky, sprites and painter-only layers keep their original brightness, as
  they never took dynamic light.
