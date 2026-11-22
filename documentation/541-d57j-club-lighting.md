# D57J - Strip club lighting: a dark, hazy nightclub in Relit

2026-11-22. **Done, user-accepted** ("lock it in and close it out. im insanely impressed. this is just unfathomably good work.").
User, 2026-11-22: "in relit, it's like, too bright all in one go, wheras
what we want is some kinda colored lights that resemble the dark seductive
atmosphere of a hazy nightclub. this looks more like a room with the lights
all on bright."

Private runs only: lab `recomp/analysis/d57j/` (local; `probe.py` from the
D57I probe, private profile and a copy of the lab cards, offscreen on the
real GPU at 180 Hz, debug port), starting from the user's UI slot 12 of
2026-11-22 (copy in `research/savestates/d57j/`). Every run checks the
player's saves and settings before and after (unchanged).

## Why it read as "every light on" (MEASURED)

Level 0, the user's view, Relit, live lamps (bake off so each edit shows at
once), mean luminance of the play area (0-255) and the share of dark pixels
(luminance under 20):

| Case | Mean | Dark |
| --- | --- | --- |
| All lamps (as shipped) | 37.1 | 22% |
| Level lamps off: Relit's dimmed painted light alone | 14.6 | 85% |
| LIGHTING ORIGINAL | 20.6 | 71% |
| Any one club lamp off (13 tried, each alone) | 33.8-39.7 | 19-28% |

- Relit's dimmed base is already darker than Original there (the club rooms
  have lamps, so Relit takes their painted light to 60%). The base was not
  the problem.
- The wash is the lamps' sum: 13 lamps in the club rooms, none of them
  dominant (the nearest table lamp, the largest, 3.3 of the 22.5 they add).
  Four are scan lamps (D41D1: the painted glow of the whole room, radius
  4886-6000, strength 0.6) and the rest sign lamps of radius 3500-4224 at
  every table, neon tube, the back bar and the Bootylicious sign. Relit makes
  every lamp 1.7x as strong and 1.35x as far (6600-8100 for the scan lamps in
  a room about 10000 across): overlapping round fills, one even pink-orange
  floor.

## The club, found by data

Its fixtures, identified by model (vertex count, first and last vertex) in
`prop_lights.cpp`:

| Fixture | Type | Model | What it is |
| --- | --- | --- | --- |
| Neon bar | 193 | 48 vertices, (-1600, 0, 0) .. (1550, 102, 0) | magenta tube x -2000..2000 on the ceiling, four spot cans under it (x +-550, +-1600), lenses (x, 232, -160) aimed along model -z, 25 degrees down |
| Pole canopy | 117 | 13 vertices, (-31, -448, -41) .. (416, 0, 307) | red hexagonal lampshade over each pole stage, rim at y 0, a bulb ring round its band |
| Back bar | 4 | 26 vertices, (-512, 291, 71) .. (1365, -291, 71) | the bottle shelves over the bar counter |

Also identified: tables (79, a box with a glowing orange top), poles (76),
the mirror ball (201); the dancers are actors (kind 2, type 123), one under
each canopy.

Survey of all 21 levels (lab `clubsurvey`, every level loaded in turn, 20 s
to settle; the club pass's own log line as a second witness):

| Level | Club rooms | Fixtures |
| --- | --- | --- |
| 0 | 75 (bar, tables), 76 (stage) | back bar, 2 neon bars, 2 canopies (4 tables, mirror ball) |
| 5 | 17, 41 | back bar, 2 neon bars, 2 canopies (2 tables; the club on fire) |
| 9 | 12 | back bar, 2 neon bars, 2 canopies (4 tables, mirror ball) |
| 1-3, 6-8, 10-12, 21-29 | none | none |

A first survey with a 6 s settle missed level 5: its props link in a few
seconds after the level number changes. The club pass therefore looks again
every 30 ticks for 30 s after a load while it has found nothing.

## Research (sources)

Real venues:
- Gentlemen's club rigs: colour LED washes on the stages, focal fixtures above
  them, LED tape on the bar and wall panels, dimmable ambient zones
  ([ADJ, Grandview Live](https://www.adj.com/blogs/news/a-grandview-over-120-adj-fixtures-installed-at-new-daytona-beach-venue));
  haze systems in the room ([LSi, Scarlett's Cabaret](https://lsionline.co.uk/news/martin-lc-series-at-scarlett-s-cabaret-3v6tyv)).
- "Capturing mood and creating an aura of tension and drama ... very similar
  to theatre" ([LSi, Robe at Spearmint Rhino](https://lsionline.co.uk/news/robe-turns-the-tables--ny9cfi)).
- Bastard Amber (Rosco 02) was made for skin: "very flattering on skin tones"
  ([Rosco Spectrum](https://spectrum.rosco.com/index.php/2012/01/spectrum-wavelengths-designer-series-louis-hartmann));
  paired with a cool blue as a contrast
  ([Rosco Spectrum](https://spectrum.rosco.com/index.php/2015/11/subtle-warmth-with-bastard-ambers)).
- Theatre colour: pale warm on the performer's front, deep colour from the
  side and back, warm against cool
  ([Theatrecrafts](https://theatrecrafts.com/pages/home/topics/lighting/choosing-and-using-colour/)).
- Haze makes the light visible in the air; keep it thin
  ([Chauvet, atmospheric impact](https://chauvetprofessional.com/news/atmospheric-impact/);
  [Showtec](https://showtec.co.uk/?p=167)); keep the ambient low, layers and
  accents rather than fill
  ([Night meet light](https://www.slideshare.net/JackKelly25/night-meet-light-discussion-of-lighting-techniques-for-nightlife),
  [Live Design, OHM Lounge](https://www.livedesignonline.com/mag/meditative-space-focus-lighting-adds-energy-and-color-manhattans-ohm-lounge)).

Games:
- "Contrast always comes first"; the reference lights stayed prominent while
  the surroundings "fall away into darkness"; light only from objects in the
  scene ([80.lv, moody lighting](https://80.lv/articles/building-moody-lighting-for-environments)).
- "We only want it to feel dark, not actually be dark"
  ([Robert Yang, lighting theory part 4](https://www.blog.radiator.debacle.us/2015/07/lighting-theory-for-3d-games-part-4.html)).
- Neon as an emissive tube plus a point light, faked volumetric fog from
  translucent cones ([80.lv, cyberpunk lights](https://80.lv/articles/lights-and-color-in-cyberpunk-environments)):
  the same means as the halos and shafts here.

Film:
- Drive's strip club: practicals only, mauve globes, "we would turn off
  globes we didn't see to increase the contrast"; "contrasting colors, and I
  like a lot of red" ([American Cinematographer, Drive](https://theasc.com/articles/drive-road-warriors)).
- Only God Forgives: real Bangkok clubs, "a really dark nightclub", red-gelled
  practicals ([American Cinematographer](https://theasc.com/articles/-Only-God-Forgives)).
- Hustlers: "pinks, purples, blues, and reds that are put onto their
  bodies", saturated neon as power, not lit "like a music video"
  ([Awards Daily, Todd Banhazl](https://www.awardsdaily.com/2019/10/29/interview-todd-banhazl-on-the-look-of-hustlers-the-color-story-is-this-great-tragedy-of-the-american-dream),
  [PremiumBeat](https://www.premiumbeat.com/blog/interview-cinematographer-todd-banhazl/)).
- Euphoria: warm against cool, practicals as sources
  ([American Cinematographer, Euphoria](https://theasc.com/article/euphoria-rev/)).

No published lux figure or pole-specific gel list was found; the rules below
are derived from the sources, not quoted. Not verified: Duke Nukem 3D's Red
Light District under Polymer, GTA V's club, John Wick's Red Circle.

Rules taken: low ambient; every light belongs to a visible fixture; pools
with dark gaps (take lights away for contrast); a two-colour scheme (magenta
and red, a cool blue accent) against one warm, near-skin key on each
performer, the brightest point in the room; halos only on the sources and
weaker than their light.

## Change: one rule for nightclub rooms

`recomp/src/ttk/club_lights.inc` (new, included after `relit.inc`), with
the fixtures in `prop_lights.cpp`/`.h`. A room holding a neon bar, a pole
canopy or the back bar is a nightclub room. Once the level's rooms are built
(before the D41E wall checks, so the D41M bake takes the result):

- **Relit ambient 30%** there (other indoor rooms with lamps: 60%). A level
  file's `ambient P room N` still wins.
- **Scan lamps out** of those rooms (the even glow).
- **Every other lamp there to a pool**: same colour, more saturated
  (each channel / max raised to 1.6), 55% of its reach.
- **Added lamps** (not written by `lamp save`; steady, baked):
  - a warm key under each pole canopy: 1.00 0.68 0.55 (the Bastard Amber
    direction), strength 4.0, radius 2000, 600 under the rim (760 over the
    stage): the dancer and the stage top are the brightest, warmest point;
  - a gel pool down each spot can's aim, 1300 from the lens: magenta 1.00
    0.10 0.55 for the outer cans, blue-violet 0.30 0.22 1.00 for the inner,
    strength 1.3, radius 1500. The stage bar's cans fall on the dancers, the
    other bar's on the tables.
- **Haze** (`prop_lights.cpp`, Modernized Polymorph with LIGHTING ENHANCED or
  RELIT): five faint magenta glows along each neon tube, a glow on each can
  lens in its gel and a soft shaft down its aim (two crossed quads, a new
  32x32 beam texture in the second bank at u 128), the canopy's bulb ring and
  the inside of its shade.
- `lamp save` writes the file's own values (reshaped lamps' colour and
  radius, the scan lamps taken out); the rule is applied at every load.
- LIGHTING ORIGINAL: no lamps and no club haze (the club as the game drew
  it). Vanilla and Classic never run it.
- Console `lamp club` (state, the camera room), `lamp club off|on` (session:
  reloads the lamps without or with the rule, for comparisons).
- Developer: `DNTTK_CLUB=0`, `DNTTK_CLUB_AMBIENT`, `DNTTK_CLUB_REACH`,
  `DNTTK_CLUB_SATURATE`, `DNTTK_CLUB_KEY="r g b strength radius"`,
  `DNTTK_CLUB_KEY_AT="x y z"`, `DNTTK_CLUB_SPOT="strength radius distance"`,
  `DNTTK_CLUB_GEL="outer r g b inner r g b"`, `DNTTK_CLUB_HAZE="neon can beam
  canopy"`.

## Tuning (lab, level 0)

Pole key, Relit, mean of the user's view / a view facing the stages:

| Try | Key (strength, radius, under the rim) | Means | Look |
| --- | --- | --- | --- |
| a | 2.4, 2300, 220 | 22.1 / 28.3 | dancers not the focus |
| b | 4.0, 2600, 420 | 26.7 / 35.5 | brighter, the floor round the stage lit |
| c | 4.0, 3200, 600 | 30.4 / 42.8 | dancers bright, a wide green-yellow wash on the carpet |
| d (kept) | 4.0, 2000, 600 | 22.8 / 29.1 | dancers the brightest warm point, the pool on the stage |
| e | 4.0, 1700, 750 | 21.2 / 27.3 | as d, the stage top dimmer |

## Evidence

Offscreen on the real GPU at 180 Hz, private copies; player files unchanged
after every run. Mean luminance / dark share, `lamp club off` (before) and
`on` (after), bake shown (as the player sees it); shots in
`research/before-after/D57J/` (local):

| View | Relit before | Relit after | Enhanced before | Enhanced after | Original (club off / on) |
| --- | --- | --- | --- | --- | --- |
| Level 0, the user's slot view | 40.6 / 17% | 22.7 / 64% | 34.2 / 29% | 26.7 / 59% | 20.6 / 20.7 |
| Level 0, facing the stages | 47.0 / 19% | 28.8 / 53% | 46.0 / 25% | 35.9 / 43% | 31.0 / 30.8 |
| Level 5, facing the stages | 50.1 / 17% | 31.1 / 41% | 47.8 / 20% | 40.3 / 29% | 34.8 / 34.7 |
| Level 9, facing the stages | 48.5 / 19% | 29.9 / 49% | 50.5 / 18% | 42.4 / 24% | 36.5 / 36.5 |

- Club pass (log): level 0 `5 fixtures in 2 rooms; 4 scan lamps out, 7
  lamps to pools, 10 lamps added`; level 5 `5 fixtures in 2 rooms; 3 ... 6
  ... 10`; level 9 `5 fixtures in 1 rooms; 3 ... 9 ... 10`. Relit trace in
  the club: `room 75 target 0.30 (club)`.
- Original: unchanged with the rule on or off (within the dancers'
  animation), no club haze. Vanilla (`--vanilla` run on the slot): no lamp
  or club activity; the lamp console answers "Modernized profile only".
- 180 Hz, the user's view, Relit, three alternating off/on pairs after a
  warm-up: pacing p50 and p95 6 ms both ways, max 8-10 ms (one 14 ms frame
  in an off pass); GPU (clock-weighted) off/on 38.3/36.6, 37.9/36.4,
  25.6/26.5 %: no measurable cost (the drift between pairs is the GPU's clock
  state). Four broad scan lamps out, ten small baked lamps and a few halo
  quads in.
- Local tests: 270 run, OK (2 skipped). Executable `1881e8d2...`.

## Limits

- Point lights: the canopy key also lights its shade and the pole, and the
  gel pools are round, not the cones of real cans (the shafts suggest the
  beam). A cone light would need a renderer change.
- No moving or chasing light (the D41AA lesson); the mirror ball has no
  sparkle. Both are possible follow-ups if the user wants movement.
- Judged offscreen on the real GPU; the user's look on the real display
  decides.
