# D57L - LA apartment mood lighting: research, plan and build

2026-11-22. **Done, user-accepted** ("its perfect!!"; the user's choice of the mock-ups: L4 lit, D3 switched off; follow-up D41AC). Job:
[D57L](../jobs/lighting.md#d57l---la-apartment-mood-lighting-hazy-and-mysterious-with-a-lights-out-glow).

User, 2026-11-22: "when the lights go off (using the switch right in front of
you), the room dims, but the lights stay bright. i really like this
aesthetic ... having the big bright "downward lighting" look ... is a bit too
much! i feel like it should be more hazy and mysterious in there. i dont mind
for a bit of recolor of the lighting if it suits the occasion. when you switch
the lights off, we do something where there is still somewhat of a light
source somewhere ... maybe dim the desk lamp?"

Private runs only: lab `recomp/analysis/d57l/` (local; the D57K probe with a
private profile and its own card copy; offscreen on the real GPU at 180 Hz,
debug port), from the user's UI slot 12 of 2026-11-22 21:05 (copy in
`research/savestates/d57l/`). The player's saves and settings were hashed
before and after every run: unchanged.

## What the switch does (MEASURED)

Guest RAM read before and after E at the switch, with a second read without
input as the noise baseline:

- The level flag word `0x800dd878` goes `0x8 -> 0x408` (bit `0x400`, the
  same bit the D08/D35 notes record for the moving bed).
- Room 8's record (88 bytes at `0x800da454` + 88 x 8) changes one word that
  matters: **+0x54, the room's light colour, `60 60 60` -> `10 10 10`**
  (+0x44 is a counter that changes in every room).
- Every other room of level 0 has `80 80 80` there. The apartment is the one
  room drawn at 3/4 of the painted light when lit, and at 1/8 when switched
  off. The game applies it to the room's painted light in every mode
  (Original included).

Why "the lights stay bright": the apartment has no lamp in `level00.lights`;
its only lights are the two prop lights, the D57D table lamp (type 228,
(7781, -12060, 7442)) and the D57K hanging lamp (type 229, (11969, -12842,
9424)). They are emitted by `prop_lights.cpp` at a fixed strength and never
read the room's light colour, so the room goes dark round them while they
keep burning.

Also measured: Relit counts the room as "unlit" (ambient 80%), because the
two prop lamps are not counted as room lamps.

Mean luminance of the play area (0-255), the slot's view (facing the switch)
and three views from the room's centre (sofa and bookcase; the window wall
with the woman and the bed; the bed and the table lamp):

| Mode | Lit, switch view | Off, switch view | Lit, room views | Off, room views |
| --- | --- | --- | --- | --- |
| Original | 97.4 | 32.1 | 71-78 | 13 |
| Enhanced | 99.0 | 34.1 | 75-81 | 16-19 |
| Relit | 89.8 | 34.7 | 65-71 | 17-22 |

(The room views are from the doorway side; the centre views in the mock-ups
below are 78-89 lit and 28-39 off in Relit.)

Lit, the room reads as an even wash: the painted light is bright on every
wall and the floor, and the table lamp bleaches the wall behind it nearly
white. Switched off, the room falls dark and the two lamps keep their full
pools: the table lamp's wall and the hanging lamp's pool by the sofa and
bookcase. That is the look the user likes.

## Survey: rooms whose light colour is not neutral

All 21 levels loaded in turn (lab `roomcol`: each room's +0x54 read after
the load and again 10 s later):

| Level | Rooms | Not `80 80 80` |
| --- | --- | --- |
| 0 | 84 | room 8 (the apartment) `60 60 60`; `10 10 10` after the switch |
| 9 | 76 | room 25 `10 10 10` from the load (not the apartment set: level 9's apartment, room 8, is `80 80 80`) |
| 1-3, 5-8, 10-12, 21-29 | - | none |

So the game uses this word for exactly two rooms. A rule keyed on it touches
nothing else today, and follows any room a custom level dims.

## The window (MEASURED)

Room 8 (origin (7168, -13312, 7168), flags 0x3) has two portals: one into
room 83 (the door), and one into **room 3, an outdoor room (flags 0x8)**: a
2048 x 1024 opening in the window wall (z 7168, x 10240-12288, y -12288 to
-11264), the street window. The window glow can therefore be found by rule:
a portal from the room into an outdoor room.

## Research (sources)

Film:
- Body Heat (Richard H. Kline): night interiors, "I would kill the overhead
  lights and light a lamp, if it were to be lit"; the glow from exposure and
  a fog filter rather than heavy filtration
  ([American Cinematographer](https://theasc.com/articles/shotdeck-drop-body-heat-richard-kline)).
- Eyes Wide Shut (Larry Smith): interiors "lit mainly by practical household
  fixtures"; "I'd simply dim down the household bulbs to get a warmer
  feeling", against a saturated blue from outside, "shafts of light that would
  bring out the sharpness of the blinds"
  ([American Cinematographer](https://theasc.com/articles/a-sword-in-the-bed-eyes-wide-shut)).
- Lost Highway (Peter Deming): Lynch: "a murky black darkness is scarier than
  a completely black darkness"; hidden practicals, paper lanterns
  ([American Cinematographer](https://theasc.com/articles/lost-highway-highway-to-hell)).
- Drive (Newton Thomas Sigel): practicals on dimmers, "We would turn off
  globes we didn't see to increase the contrast"; real cobra-head sodium
  lamps outside ([American Cinematographer](https://theasc.com/articles/drive-road-warriors)).
- Blade Runner (Jordan Cronenweth): "to get shafts of light one must have
  some medium, which necessitated the use of smoke"; a room lit by one
  practical (the refrigerator)
  ([American Cinematographer](https://theasc.com/article/blade-runner-cronenweths-photography/)).

Colour:
- A dimmed incandescent lamp warms smoothly, "from 2744 K at 100% power to
  1600 K at 5% dimming" (US DOE,
  [SSL R&D poster](https://www.energy.gov/sites/default/files/2020/02/f71/ssl-rd2020-hansen-poster.pdf)):
  a lamp turned low should also turn deeper amber.
- High-pressure sodium street lamps, about 1900-2100 K
  ([Philips SON datasheet](https://www.assets.lighting.philips.com/is/content/PhilipsLighting/fp928150108828-pss-en_au);
  D57I already lights the street this colour).
- A cool TV glow is mostly slow shifts in colour and level, about 1/4-1/2 CTB
  ([Cinematography Mailing List](https://cinematography.net/edited-pages/SimulatedTVFx.htm)).

Games:
- "A motivated light is a light source with a plausible fixture";
  secondary spaces dimmer and less focused
  ([The Level Design Book](https://book.leveldesignbook.com/process/lighting)).
- Contrast first ([Robert Yang, part 2](https://www.blog.radiator.debacle.us/2015/03/lighting-theory-for-3d-games-part-2.html));
  "We only want it to feel dark, not actually be dark"
  ([part 4](https://www.blog.radiator.debacle.us/2015/07/lighting-theory-for-3d-games-part-4.html), as D57J).
- Darkness as very low ambient, never absolute black
  ([The Dark Mod wiki](https://wiki.thedarkmod.com/index.php/Virtual_Darkness)).

Not verified with a source: Taxi Driver, Basic Instinct, Klute, In the Mood
for Love, Chungking Express and Mulholland Drive apartments (searched, no
usable lighting text); the "small light left on" as a named game trope.

Rules taken:
- Lit: the light comes from the practicals, not a broad fill: the painted
  light lower, the hanging lamp a warm pool under its shade instead of a
  bright downward light, dark between the pools, the ceiling dim.
- Warmth from the bulbs (dimmed tungsten), optionally a rose cast on the
  hanging lamp (the user's "recolor ... if it suits the occasion").
- A thin haze round the sources only (a glow on the shades, a faint glow at
  the window), never the club's smoke.
- Lights out: the ceiling lamp goes out with the switch; one practical stays,
  turned low and so deeper amber (the DOE curve); never pure black; the
  street's sodium at the window as a faint second source.

## Mock-ups (lab shots, Relit)

Made without code changes: the prop lamps' developer tunes
(`DNTTK_PROP_LIGHT_PENDANT`, `DNTTK_PROP_PENDANT_HALO`,
`DNTTK_PROP_LIGHT_TABLE_LAMP`), the room's Relit ambient (`lamp ambient 45 room
8`, session) and one session lamp for the window (`lamp here`, sodium 1.0 0.50
0.18, radius 1100, strength 0.22, just inside the window wall). Sheets in
`research/before-after/D57L/plan/` (local): `round2-L3-L4-D3-D4.png` (the
candidates), `round1-*.png` (first tries, today's look L0 and D0),
`today-*.png`.

| Option | State | What | Room views (mean) |
| --- | --- | --- | --- |
| L0 | lit | today | 82 / 78 / 89 |
| **L3** | lit | painted light 45%; hanging lamp warmer and dimmer (1.0 0.62 0.34, 0.35, radius 1700, softer bulb glow); table lamp warmer (1.0 0.66 0.36, 0.9, 2300); a faint sodium glow at the window | 53 / 58 / 67 |
| **L4** | lit | as L3 with a rose hanging lamp (1.0 0.45 0.42, 0.40): a pink cast on the wall behind the sofa | 53 / 56 / 66 |
| D0 | off | today: room dark, both lamps at full | 28 / 39 / 37 |
| **D3** | off | hanging lamp out; table lamp low and deep amber (1.0 0.55 0.22, 0.8, 2200); the faint window glow | 13 / 30 / 31 |
| **D4** | off | as D3 without the window glow: the table lamp alone | 15 / 27 / 24 |
| L1, L2, D1, D2 | - | first round: the window lamp at 0.9 / 1900 washed the wall, floor and the woman orange (rejected); D2's table lamp at 0.55 left the sofa side nearly black | - |

## Plan (to build after the user's choice)

1. **Lamps follow the room's light (system, every level).** Each frame the
   prop lights read their room's light colour (+0x54). When the game dims a
   room below the value it had when the level loaded (level 0 room 8:
   `60` -> `10`; level 9 room 25 starts at `10` and so never changes), a ceiling fixture (the
   hanging lamp, and any future ceiling kind) goes out with it, and a lamp
   that plugs in (the table lamp) stays on, turned low and warmer. A level
   file's lamps in that room follow the same way. No per-level data: any
   room the game dims, in any level or a custom one, follows its switch. The
   switch is instant in the game, so the change is instant (no fade).
2. **The apartment rule (as D57J's club rooms).** A room holding the hanging
   lamp (prop type 229, by model) is an apartment room: levels 0 and 9, room
   8. There, in LIGHTING ENHANCED and RELIT:
   - Relit's ambient 45% (today 80%, "unlit"); a level file's `ambient P room
     N` still wins;
   - the hanging lamp and the table lamp take the chosen lit look (L3 or L4);
   - the window glow, found by rule: a faint sodium lamp just inside each of
     the room's portals that open onto an outdoor room (room 8: the street
     window, measured above);
   - a thin haze: a soft glow round each shade (the D57J halo means), no
     shafts or smoke unless asked.
3. **Lights out** takes the chosen look (D3 or D4) from step 1.
4. ORIGINAL, Vanilla and Classic unchanged; `lamp apartment off|on`
   (session) for comparisons; cost measured at 180 Hz (two small lamps and a
   few halo quads at most); before/after shots to
   `research/before-after/D57L/`; level 9's copy looked at.

## The user's choice

User, 2026-11-22: "its gotta be the l4 rose, and i think D3". Step 1
(game-wide) went unanswered; built as recommended (game-wide), say if not.

## Build

`prop_lights.cpp`/`.h`, `club_lights.inc`, `relit.inc`, `level_lights.inc`,
`near_clip.cpp`. Modernized Polymorph, LIGHTING ENHANCED and RELIT (the prop
and level lamp gates); ORIGINAL, Vanilla and Classic unchanged.

- **Lamps follow their room's switch (game-wide).** Each image the prop
  lights read their room's light colour (+0x54, the red byte). Under `0x40`
  (half a plain room's `0x80`) the room is switched off: the electric
  fixtures (hanging, caged, globe and street lamps) give no light, and a
  table lamp turns low and amber (1.00 0.55 0.22, strength 0.8, radius
  2200; its shade's glow level 45, 1.00 0.62 0.30). Flames, fire, pickups
  and the train are not switched. Read from the game's own state, so it is
  instant like the game's dimming and survives savestates. Today it touches
  only level 0 room 8 (level 9 room 25, dark from the load, holds none of
  these lamps). A level file's lamps do not follow (none in either room).
- **Apartment rooms**: a room holding the hanging lamp (prop type 229, by
  model). Its lamps take the L4 look: the hanging lamp rose (1.00 0.45
  0.42, strength 0.40, radius 1700; bulb glow 300 across, level 60, warm
  white 1.00 0.90 0.66: see playtest 2), the table lamp warmer (1.00 0.60
  0.32, 0.9, 2300). Relit
  ambient 45% (a level file's `ambient P room N` still wins). A sodium glow
  (1.00 0.50 0.18, strength 0.35, radius 1400) 250 inside each portal into
  an outdoor room, 300 above its centre: level 0's street window. The
  window's first value (0.22 / 1100, as the mock-up's session lamp) read
  weaker than D3 once centred in the opening; 0.35 / 1400 matched D3 (0.5
  turned the woman orange).
- **Halos per lamp**: the halo of a table or hanging lamp is drawn with its
  model, so the draw now finds the prop record in the draw's registers
  (`prop_glow_object`, the record whose +0x44 is the model) and takes that
  lamp's look (16 slots published by the game thread); every lamp halo in
  the lab found its record (thousands found, none missed). Unknown, the
  model's look is used when all its copies agree, else the old look.
- Console `lamp apartment` (state, the camera room), `lamp apartment
  off|on` (session; off is the look before D57L, the switch following
  included). Developer: `DNTTK_APARTMENT=0`, `DNTTK_APARTMENT_AMBIENT`,
  `DNTTK_APARTMENT_WINDOW="r g b strength radius inside above"`,
  `DNTTK_APARTMENT_PENDANT`, `DNTTK_APARTMENT_TABLE`, `DNTTK_DARK_TABLE`
  (`"r g b strength radius"`), `DNTTK_APARTMENT_PENDANT_HALO="size level r g
  b"`, `DNTTK_DARK_TABLE_HALO="level r g b"`. The window lamp is not written
  by `lamp save`. Trace (`DNTTK_PROP_LIGHT_TRACE=1`): lamps in switched-off
  rooms, the halo record lookups; the lamp pass logs `[TTK apartment] level
  N: R apartment rooms, W window glows`.

## Evidence

Offscreen on the real GPU at 180 Hz from the user's slot; player saves and
settings unchanged after every run. Mean luminance of the three room views
(sofa and bookcase / window wall and bed / bed and table lamp), `lamp
apartment off` (before) and `on` (after):

| Mode | Lit before | Lit after | Off before | Off after |
| --- | --- | --- | --- | --- |
| Relit | 82 / 78 / 89 | 56 / 59 / 64 | 28 / 39 / 37 | 15 / 31 / 28 |
| Enhanced | 90 / 83 / 93 | 86 / 80 / 88 | 23 / 33 / 29 | 17 / 27 / 22 |
| Original | 83 / 75 / 82 | identical | 16 / 22 / 16 | identical |

(The mock-ups: L4 53 / 56 / 66, D3 13 / 30 / 31.)

- Shots: `research/before-after/D57L/` (local), `<lit|dark>-<mode>-view<N>-
  before|after.png` and the Original views.
- The lamp pass: level 0 `1 apartment rooms, 1 window glows`; level 9 `1
  apartment rooms, 0 window glows` (its apartment's portals lead to rooms
  flagged 0x1, not outdoor). After E: `2 lamps in switched-off rooms`.
- The club is unchanged (its pass shares the code): `5 fixtures in 2 rooms;
  4 scan lamps out, 7 lamps to pools, 10 lamps added`, as D57J.
- 180 Hz, Relit, the bed view, `lamp apartment` off/on: lit, four pairs,
  pacing p50 and p95 6 ms every pass, max 7-10 ms, no image over 10 ms; GPU
  (clock-weighted) off 34.8 / 35.2 / 35.1 / 35.3 %, on 35.4 / 35.8 / 35.4 /
  35.1 % (a first three-pair run drifted 29 -> 39 % with the clock state;
  the repeat is flat). Switched off, three pairs: pacing the same, GPU off
  36.0 / 34.8 / 34.1 %, on 33.8 / 33.6 / 33.8 %. No measurable cost.
- Vanilla (`--vanilla` on the slot): no prop-light or apartment activity.
- Local tests: 270 run, OK (2 skipped).

## Playtest 1: haze round the switched-off hanging lamp

User, 2026-11-22 (slot 12 resaved 22:39, copy in
`research/savestates/d57l/haze/`): "the only bit that doesnt look quite right
is the little bit of haze around the hanging lamp when its switched off ...
this is wonderful work."

- Loading the slot (room switched off, looking up at the lamp), the area
  round the bulb is pixel-identical with the prop lights on and off, and with
  `lamp apartment off|on`: no halo of ours is drawn there, and a live flick
  off-on-off draws none either.
- The slot's save thumbnail does show a soft glow on the bulb, but a lab
  quick save with the fix shows the same while the live image at that moment
  shows only the pale bulb: the thumbnail is not the image the player sees.
- One real gap fixed: the halo's look (rose / out / as before) was published
  by the light pass and reset by every prop rescan (every 300 frames and on
  each load); whenever the light pass had not yet run since (the pause and
  save screens, the image after a rescan) the old warm halo came back round
  the switched-off lamp. The draw now works the look out itself: the scan
  records each lamp's prop, room and apartment flag, and the halo reads the
  room's light colour from RAM as it draws.
- Not established: which of these the user saw on the real display; the
  user's look with the fixed build decides.

## Playtest 2: the bulb's halo back while the light is on

User, 2026-11-22: "the halo is now gone completely. can we bring the halo
back when the light is on, and make the halo disappear when the light is
off? it looked awesome". With the light on the rose halo (level 28, 1.00
0.62 0.58) was drawn but sank into the rose wall light round it. Lab, lit,
the lamp up close: the D57K halo (300, 40, warm white) faint against the
darker rose room; 300 / 60 warm white a clear little glow at the bulb (kept);
360 / 60 pale rose washed out. Switched off: no halo (checked brightened 3x,
before and after a live flick).

## Limits

- Level 9's apartment takes the rule (the lamps' looks, the ambient) but
  has no window glow, and the lab did not get a usable view of it (the warp
  put the camera under the set); not judged.
- The lab sees three views from the room's centre; the user's look on the
  real display decides (Needs playtest).
- Haze is only the shades' glows (no shafts or smoke), as the plan.
- A switched-off room's level-file lamps do not follow the switch (no level
  has one there today).

## The questions asked (answered above)


- Lit: **L3** (warm) or **L4** (rose hanging lamp)?
- Switched off: **D3** (table lamp low and amber, plus the faint street glow
  at the window) or **D4** (the table lamp alone)? Or today's D0 kept, with
  only the hanging lamp following the switch?
- Should step 1 (lamps follow the room's switch) apply game-wide? Recommended:
  yes, it is the game's own state and the fix the system rule.
