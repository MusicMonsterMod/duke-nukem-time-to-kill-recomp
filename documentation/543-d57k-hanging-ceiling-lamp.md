# D57K - The LA apartment's hanging ceiling lamp gives light

2026-11-22. **Done, user-accepted** ("lock it in, and close it out"). D57E closes with it. Follow-up: D57L, the room's mood lighting and its light switch.
User, 2026-11-22, closing D57D: "the next one is the light i have just saved
in slot 12. can you check that light, and then add it to the backlog."
Acceptance: the room under the lamp takes its light.

Private runs only: lab `recomp/analysis/d57k/` (local; the D57D probe with a
private profile and its own card copy holding the user's UI slot 12 of
2026-11-22 as slot 11; offscreen on the real GPU at 180 Hz, debug port).
Every run checks the player's saves and settings before and after
(unchanged).

## The lamp (MEASURED)

Level 0, room 8: the furnished upper apartment off the LA street (sofas, a
vanity with mirror, a TV, a bookcase, the D57D table lamp). The lamp is prop
type 229 at (11969, -12842, 9424), yaw 0: 16 vertices, 272 x 940 x 274. A
cord from the ceiling (model y -470) down to a shallow four-sided cream shade
(apex y 379, rim y 451, 272 across) with a white bulb hanging under it (y
448..470, painted 248 248 248). First vertex (13, 470, 13), the last (136,
451, 137). The bulb hangs about 930 under the ceiling, near the height of
Duke's head.

Before this job the room had no lamp in `level00.lights`: ENHANCED drew it as
ORIGINAL, and RELIT only dimmed its painted light.

Survey of all 21 levels (lab `lampsurvey`: each level loaded in turn, the
model matched in every room by type and by shape, the prop-light trace as a
second witness):

| Level | Hanging lamps |
| --- | --- |
| 0 | 1: room 8, the LA apartment (the trace counts it: "1 hanging lamps") |
| 9 | 1: room 8 at (11815, -12842, 9459), the same apartment set |
| 1-3, 5-8, 10-12, 21-29 | none |

## Change

`prop_lights.cpp`/`.h`: `pendant_mesh`, kind `prop_light_pendant`, by model
(the vertex words, not the load address: level 9's copy sits elsewhere in
RAM), game-wide, no per-level data, in the D57C/D57D pattern:

- **Light**: one steady light just under the bulb (model y 520): tungsten,
  1.00 0.78 0.50, strength 0.5, radius 2200 (Relit takes it 1.7x as strong
  and 1.35x as far, as every lamp). The wall round the lamp, the sofa, the
  bookcase and the floor under it take a warm pool; the ceiling, far above
  the bulb, takes little.
- **Bulb glow**: a soft warm halo at the bulb under the shade (model y 455),
  300 across, level 40, 1.00 0.90 0.66.
- Weaker than the table lamp (1.2 / 2600): this room's painted light is
  already bright, and 1.2 bleached the wall behind the lamp nearly white
  (Relit mean 94 -> 160, 90th percentile 251). 0.7 still washed the wall;
  0.5 reads as a warm pool with the shade visible.
- A lamp a level file binds to the prop (D41D1 `bind`) keeps that lamp
  alone; `lit_mesh` keeps the sign scan from making a second lamp for it.
- LIGHTING ENHANCED and RELIT, Modernized Polymorph only (the prop light
  gates); LIGHTING ORIGINAL, Vanilla and Classic unchanged. Console `lamp
  props off|on` compares (with the other prop lights).
- Developer: `DNTTK_PROP_LIGHT_PENDANT="r g b strength radius"`,
  `DNTTK_PROP_PENDANT_AT="y"`, `DNTTK_PROP_PENDANT_HALO="size level r g b"`;
  `DNTTK_PROP_LIGHT_TRACE=1` counts hanging lamps per level.

## Evidence

From the user's slot (Duke in room 8, looking up at the lamp), bake shown,
`lamp props off` (before) and `on` (after). Mean luminance / 90th percentile
of the play area:

| Mode | Before | After |
| --- | --- | --- |
| Relit | 100.0 / 119.3 | 131.4 / 196.4 |
| Enhanced | 118.3 / 142.6 | 139.8 / 191.2 |
| Original | 118.3 / 142.6 | 118.3 / 142.6 (identical) |

- Shots in `research/before-after/D57K/` (local).
- 180 Hz, Relit, the same view, three off/on pairs: pacing p50 and p95 6 ms
  every pass, max 6-9 ms, no image over 10 ms; GPU (clock-weighted) off
  17.42 / 17.63 / 17.56 %, on 17.45 / 17.03 / 17.76 %: no measurable cost.
  One small light and one halo quad.
- Vanilla (`--vanilla` run with the trace): no prop-light activity.
- Tests: `python3 -m unittest discover -s recomp/tests/local` 270 OK (2
  skipped).

## Limits

- The lab sees one view, the user's slot (looking up at the lamp). The pool
  on the floor and the furniture under it, as the player walks the room,
  waits on the user's look on the real display.
- Relit does not dim this room's painted light further: the room reads as a
  lit apartment with the lamp as its warm source. If the user wants the lamp
  to read as the only source, that is a measured follow-up (a darker base
  for this room by a rule, not by level data).
- Level 9's copy is lit by the same rule; it was not looked at in the lab.
