# D41X - The fire barrel's light

2026-11-22. **Done, user-accepted** ("it's absolutely perfect! accepted"). User, while playtesting D53G in UI slot 12:
"barrel in slot 12, is that really emitting light? hmm".

Private runs only: lab `recomp/analysis/d41x/` (local; `probe.py` from the
D17BN probe, private profile, a copy of the lab cards, a private copy of the
lamp files through `DNTTK_LIGHTS_DIR` for console lamp edits). Every run
checks the player's saves and settings before and after (unchanged).

## What lit it before (MEASURED)

The barrel in UI slot 12 is the fire barrel, prop type 220 (static prop,
kind 1), LEVEL00 room 22 at (27112, -6518, 37353), model `0x8012cb90`: 20
vertices, the drum from y +374 (floor) to its open rim at -122 (320 x 276),
a double-sided fire cross from -122 to -374. Level 0 places seven (rooms 22,
33, 37, 40, 45, 57, 63), level 5 one (room 2, another model address, the same
data).

Matched shots in one session (`light mode` console, same camera; wall and
floor RGB means behind and beside the barrel):

| LIGHTING | wall | floor |
| --- | --- | --- |
| Original | 92 70 36 | 91 57 43 |
| Enhanced | 92 70 36 | 91 57 43 |
| Relit (the player's) | 61 46 23 | 58 37 28 |
| Relit, `scan room 22` lamp removed | 76 58 30 | 70 46 35 |

- The warm patch round the barrel is the original level's painted vertex
  light: Vanilla, Original and Enhanced show it alike.
- None of our sources gave the barrel a light on level 0. The nearest lamp
  is `scan room 22` (a D41D painted-light scan lamp, steady, 830 units from
  the barrel): it adds about 0.3 levels at this spot in Enhanced, and in
  Relit its only effect is that room 22 counts as a room with a lamp, so its
  painted light is dimmed to 60% instead of 80%. The glows option and the
  model bake change nothing there.
- Level 5's barrel alone had a flickering lamp, from that level's file (a
  D41D1 sign lamp, `sign type 220 room 2`, candle style), so the same object
  lit in one level and not in the other.

Lab note: the player's profile has GRAPHICS AUTO, whose preset sets
`dynamic_lights` at launch (`player_profiles.py`), so a lab override of
`dynamic_lights` alone is replaced; in-session `light mode` comparisons are
the reliable A/B.

## Decision

User (2026-11-22, asked with the table above): a real fire light, "every type
220 fire barrel in every level", out when broken, one rule for all levels.

## Change

- `recomp/src/ttk/prop_lights.cpp` (D41L prop lights): `barrel_mesh()`
  recognises the fire barrel by its model data (20 vertices, first
  (0, -122, 0), last (80, 374, -138)), so every copy in every level and any
  custom level using the model takes part. The prop scan gives each standing
  one a fire emitter in its flames at model (0, -300, 0), above the rim:
  colour 1.0 0.52 0.18, strength 1.3, radius 3000 (Relit multiplies by the
  level's relit lamp gain and reach as for every prop light), flickering with
  the woodburner's fire curve (`prop_fire_level`, seeded per barrel). It is
  found by the scan only, not by `kind_of()` on every draw, and has no drawn
  extras: the fire is the model's own. D54: when it breaks (it explodes) the
  light sputters out (`[TTK prop light] fire light goes out`).
- `prop_lights.h`: `prop_light_lit_mesh`, used by the D41D1 sign scan
  (`level_lights.inc`, `lamp signs`) to skip models whose light comes from
  the prop lights, so an editor's `lamp signs` makes no second lamp for a
  barrel.
- `recomp/assets/lights/level05.lights`: the barrel's sign lamp line removed
  (68 to 67 lamps); the prop rule lights it now.
- Developer: `DNTTK_PROP_LIGHT_BARREL="r g b strength radius"`,
  `DNTTK_PROP_BARREL_Y` (model height of the light). Console `lamp props
  off|on` includes the barrels.
- LIGHTING Original, Vanilla and Classic: unchanged. The painted light and
  the scan lamps stay as they were.

## Evidence

- Level 0: `[TTK prop light] level 0: ... 7 fire barrels, 8 lights`. Level 5
  (debug `level 5`): 1 fire barrel, 67 lamps.
- Slot 12 view, same session, `lamp props on` / `off`: Relit wall 149 90 37
  against 61 46 24, floor 158 86 43 against 58 37 28; Enhanced wall 143 95 44
  against 92 70 36; Original unchanged. Twelve shots 9 frames apart: wall
  grey 86 to 94 (the flicker).
- Break: barrel health set to 1 on the private run, one pistol shot; bit 2
  of `+0x00` set, `fire light goes out: its prop 801d9864 broke`. The blast
  killed Duke (65 health, 600 units away), as the game does.
- `lamp signs` on level 0: 42 sign lamps instead of 49 (no `sign type 220`).
- Vanilla: pixel-identical to the shot before the change (0 of 786432
  pixels), no prop-light activity.
- 180 Hz, the barrel in view: 489 presents, 0 missed, 0 failed redraws,
  cadence p95 6.25 ms (6.09 before).

## Limits

- Accepted on the user's display as tuned (strength 1.3, as near the
  stove's fire); a measured offset is one developer variable away.
- The barrel's drum does not shadow its own light onto the floor (props cast
  no lamp shadows), so the floor right round it is lit too.
- Level 5's barrel was not viewed (it is far from the level start); the scan
  and lamp counts were checked.
- Follow-up backlog: D41Y, the construction barricades' (type 128) warning
  lamps blink and cast light.
