# D57D - The strip club's back-room lamp gives light

2026-11-22. **Done, user-accepted** ("lock it in and close it out!! thats excellent."). Follow-up: D57K, the hanging ceiling lamp (prop type 229) in room 8.
User, 2026-11-14 (playthrough commentary): in the strip club, "the lamp
inside the secret room should be a real light". Acceptance: the room and
the objects in it take the lamp's light.

Private runs only: lab `recomp/analysis/d57d/` (local; the D57J probe with a
private profile and its own copy of the lab cards, offscreen on the real GPU
at 180 Hz, debug port), starting from the D57J test save (the user's UI slot
12 of 2026-11-22, Duke in the club on level 0). Every run checks the
player's saves and settings before and after (unchanged).

## Which room, which lamp (MEASURED)

The level names hold no "SOMEONE'S GONNA PAY" (the club levels are 0 TIME TO
KILL, 5 OBEY OR DIE and 9 PIG FACTORY); the room was found by data instead.
Room records (`0x800da454`, 88 bytes each) carry an origin (+0, +4, +8),
flags (+12) and portals (count +0x30, list +0x38, 36 bytes each, the target
room at +14). Following the portals out of the club rooms of level 0 (D57J:
75, the bar, and 76, the stage):

| Room | Joins | What is in it |
| --- | --- | --- |
| 12 | 3 (street), 75 | the club's entrance: red EXIT sign (171), a pink booth (196, 200) |
| 10 | 76, 13 | an empty passage behind the stage |
| 13 | 10 | a dressing room: vanity (194) with mirror (584), two benches (199), two chairs (14), posters (637), a box table (196) and on it the lamp (228); a pig cop |

Room 13 is the room behind the stage. Its drawn floor is 512 above the
club's (the passage's portal into it spans y -11776..-10240, the stage's
-11008..-9728), and a warp into it drops Duke to a floor near the club's
height with the camera under the room's drawn floor; the room is probably
reached by a lift or a climb (not traced). The lab therefore takes its
pictures within two frames of each warp, from just under the floor line.

The lamp: prop type 228, 20 vertices, a shaded table lamp: a square stem
(model y 38..196, 108 across) under a four-sided shade, its open rim at y 21
(278 across) narrowing to the top at y -196 (98 across), painted a pale warm
cream (248 248 176 at its brightest; the D41D1 sign scan, which looks for
saturated glowing texels, never made a lamp for it). Level 0 has two:

| Room | Where | Position |
| --- | --- | --- |
| 13 | the club's back room, on the box table | (23615, -11036, -7426) |
| 8 | a furnished upper room off the street (vanity 194, box 196, the same set) | (7781, -12060, 7442) |

Room 8 may be the apartment of [D57E](../jobs/lighting.md#d57e---la-apartment-lamp-gives-light)
(not confirmed: warp cannot stand Duke in it either).

Survey of all 21 levels (lab `lampsurvey`: every level loaded in turn, 20 s
to settle, the model matched in every room and the prop-light trace as a
second witness):

| Level | Table lamps |
| --- | --- |
| 0 | 2: room 13 (the club's back room), room 8 |
| 5 | 1: room 13 (the back room of the burning club) |
| 9 | 3: room 8, two in room 13 |
| 1-3, 6-8, 10-12, 21-29 | none |

## Change

`prop_lights.cpp`/`.h`: `table_lamp_mesh`, kind `prop_light_table_lamp`, by
model (first vertex (-54, 196, -54), the last (-49, -196, 49)), game-wide,
no per-level data:

- **Light**: one steady light at the bulb, inside the shade just above its
  rim (model y -40): a tungsten bulb behind a cream shade, about 2700 K,
  1.00 0.78 0.50, strength 1.2, radius 2600 (Relit takes it 1.7x as strong
  and 1.35x as far, as every lamp). The table top, the wall behind it, the
  ceiling over it and the furniture round it take a warm pool.
- **Shade glow**: a soft warm halo on the shade (model y -90), 520 across,
  level 70, 1.00 0.90 0.62.
- A lamp a level file binds to the prop (D41D1 `bind`) keeps that lamp
  alone, as for the globes and street lamps; `lit_mesh` keeps the sign scan
  from making a second lamp for it.
- LIGHTING ENHANCED and RELIT, Modernized Polymorph only (the prop light
  gates); LIGHTING ORIGINAL, Vanilla and Classic unchanged. Console `lamp
  props off|on` compares (with the other prop lights).
- Developer: `DNTTK_PROP_LIGHT_TABLE_LAMP="r g b strength radius"`,
  `DNTTK_PROP_TABLE_LAMP_AT="y"`, `DNTTK_PROP_TABLE_LAMP_HALO="size level r
  g b"`; `DNTTK_PROP_LIGHT_TRACE=1` counts table lamps per level.

## Evidence

Offscreen on the real GPU at 180 Hz, private copies; player files unchanged
after every run. Level 0, room 13, `lamp props off` (before) and `on`
(after), bake shown, each picture within two frames of a warp (the camera
just under the room's floor line, so the lower half of each picture is
black and in both). Mean luminance / 90th percentile of the play area:

| View | Relit before | Relit after | Enhanced before | Enhanced after |
| --- | --- | --- | --- | --- |
| Facing the lamp, 1700 away | 17.9 / 60.7 | 32.8 / 122.1 | 20.2 / 71.0 | 30.7 / 107.2 |
| Down the room, 3600 away | 17.0 / 59.3 | 25.0 / 79.8 | 20.4 / 70.9 | 23.9 / 75.5 |

- The wall behind the lamp, the box table, the ceiling over it and the
  back wall round it take a warm pool; the shade glows. Shots in
  `research/before-after/D57D/` (local).
- Prop-light trace: level 0 `2 table lamps` (46 prop lights in all).
- LIGHTING ORIGINAL: no change with prop lights off or on (the pictures
  differ only by the pig cop's pose and Duke settling after the warp).
  Vanilla (`--vanilla` run): no prop-light activity.
- 180 Hz, facing the lamp, Relit, three off/on pairs: pacing p50 and p95 6
  ms both ways, max 7-8 ms (one 14 ms frame in one on pass); GPU
  (clock-weighted) off/on 13.0/19.1, 17.4/19.5, 16.7/19.4 %: about +2 to +3
  points with a lamp filling the view (the first pair includes the GPU's
  clock-state drift). One small baked light per lamp and one halo quad.
- Tests: `python3 -m unittest discover -s recomp/tests/local` 270 OK (2
  skipped).

## Limits

- The lab cannot stand Duke in room 13 (or in room 8): its pictures look
  across the room from just under the floor line. The light on the floor,
  the benches and the vanity as the player sees them waits on the user's
  look on the real display, ideally from a save state in the room.
- How the player reaches room 13 (a lift or a climb from the passage) was
  not traced.
- Whether room 8 is D57E's apartment is not confirmed; it is lit by the
  same rule.
