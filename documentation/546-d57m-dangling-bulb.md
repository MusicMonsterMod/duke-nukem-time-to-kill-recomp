# D57M - Obey or Die apartment: the dangling bulb

2026-11-23. **Done, user-accepted** ("10/10 lock it in and close it out"). Job:
[D57M](../jobs/lighting.md#d57m---obey-or-die-apartment-dangling-bulb).

User, 2026-11-23: "this is the same apartment but in level 5 obey or die.
there's this lightbulb dangling here. can we make that emit light properly
and give it similar moody treatment that we gave the apartment room. it's
only one light so dont spend too long on it, should be a real quick one."

Private runs only: lab `recomp/analysis/d57m/` (local; the D57L probe with a
private profile and its own card copy; offscreen on the real GPU at 180 Hz,
debug port), from the user's UI slot 12 of 2026-11-23 01:07 (copy in
`research/savestates/d57m/`). The player's saves and settings were hashed
before and after every run: unchanged.

## The bulb (MEASURED)

Level 5, room 8: the apartment set again (the D57L room number), here with
no hanging lamp and no table lamp. Its one fixture is a bare bulb on a cord:
prop type 929, model `0x80141e40` at (8455, -13012, 8393), yaw 0.

- 16 vertices, 84 x 604 x 84: a cord of two crossed quads 22 wide from the
  ceiling (model y -302, world -13314) down to y 181, and the bulb, two
  crossed quads 84 wide at y 181..302 (centre y 241, world about -12771).
  First vertex (-42, 181, 0), the last (0, 181, -11).
- Before D57M the bulb gave no light (no prop-light kind, no level lamp in
  the room: `lamp list` has none in room 8), and the D57L apartment rule
  found no apartment room in level 5 (it keys on the hanging lamp).
- The room's light colour (+0x54) is `80 80 80` (a plain room) and stays so;
  no room of level 5 is dimmed by the game (D57L survey). Whether this
  apartment's switch dims anything in level 5 was not looked at.
- Survey, all 21 levels loaded in turn (the bulb by model, and prop type
  929): the one in level 5 room 8 only. Levels 0 and 9 keep their hanging
  lamp (0 bare bulbs).

## Build

`prop_lights.cpp`/`.h`, `club_lights.inc`, `near_clip.cpp`. Modernized
Polymorph, LIGHTING ENHANCED and RELIT (the prop and apartment gates);
ORIGINAL, Vanilla and Classic unchanged.

- **The bare bulb** (`prop_light_bulb`, found by model, game-wide): a
  hanging lamp in every rule. It is an electric fixture (it goes out when
  the game darkens its room, as D57L), it marks its room as an apartment
  room, and its halo follows its own prop record (the D57L draw lookup).
  The light stands just under the glass (model y 420; at the bulb's centre,
  y 330 tried first, it threw a hard hot spot on the ceiling 640 above).
- **In an apartment room** (any room holding a hanging lamp or a bare bulb;
  today level 0 and 9 room 8, and level 5 room 8): the D57L mood. Relit
  ambient 45% (a level file's `ambient P room N` still wins), the street's
  sodium glow at each portal into an outdoor room (level 5's apartment has
  one: `level 5: 1 apartment rooms, 1 window glows`), and the bulb a low,
  warm dimmed-tungsten pool (1.00 0.58 0.34, strength 0.45, radius 2000)
  with a hot glow on the glass (300 across, level 140, 1.00 0.86 0.58).
  The glow is strong on purpose: in Relit the lower room ambient greys the
  bulb's own white texture, and at level 60 the glow sank into the warm
  ceiling round it (lab zooms, `za`/`zb`/`zc`).
- Outside an apartment room (none today) the bulb takes the shaded lamp's
  warm tungsten (1.00 0.78 0.50, 0.5, 2200) and a glow of 300 / 100.
- `lamp apartment off|on` (session) compares as for D57L; off leaves the
  bulb lit with its plain look. `lamp props off` removes it. Developer:
  `DNTTK_PROP_LIGHT_BULB`, `DNTTK_APARTMENT_BULB` ("r g b strength
  radius"), `DNTTK_PROP_BULB_AT` ("y"), `DNTTK_PROP_BULB_HALO`,
  `DNTTK_APARTMENT_BULB_HALO` ("size level r g b"). Trace
  (`DNTTK_PROP_LIGHT_TRACE=1`): the scan line counts `bare bulbs`.

## Evidence

Mean luminance (0-255) of the slot's view (first person, looking up at the
bulb). Before = `lamp props off` and `lamp apartment off` (exactly the look
before D57M in this room: it had no prop light and no apartment rule):

| Mode | Before | After |
| --- | --- | --- |
| Relit | 55.6 | 71.7 |
| Enhanced | 65.2 | 76.3 |
| Original | 65.2 | 65.2 (identical) |

- Shots: `research/before-after/D57M/` (local), `slot-<mode>-before|after.png`.
- Room views (third person from the room's centre, five spots): Relit
  before 33-41, after 34-58, the pool round the bulb and dark corners;
  Enhanced 53-80 before, 51-78 after (the room's own light kept; the Relit
  ambient applies to Relit only); Original within 0.6 of before at every
  spot. The lab cannot level the camera (the slot looks up, and `warp` and
  the key look do not move the Modernized pitch on an uncaptured run), so
  these views show mostly ceiling and upper walls.
- 180 Hz, Relit, the slot's view, before/after, three pairs: pacing p50 and
  p95 6 ms every pass, max 6-8 ms, no image over 10 ms; GPU (clock-weighted)
  before 24.2 (warm-up) / 9.4 / 10.5 %, after 18.7 / 11.4 / 11.7 %: one
  light and one glow quad, no measurable cost.
- Local tests: 270 run, OK (2 skipped).

## Limits

- Lab-judged, then accepted by the user on the real display.
- Vanilla (`--vanilla` on the slot): no prop-light or apartment activity.
- Not checked: level 5's apartment switch, if it has one.
