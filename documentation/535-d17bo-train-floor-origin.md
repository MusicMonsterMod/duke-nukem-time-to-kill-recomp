# D17BO - The subway train flickers as Duke walks

2026-11-22. **Done, user-accepted** ("lock it in and close it out!"). User, closing D41Y: "load up the new slot 12
i have just saved, walk forwards and backwards and notice the train to the
left of the shot flickers."

Private runs only: lab `recomp/analysis/d17bo/` (local; `probe.py` from the
D41Y probe, private profile and a copy of the lab cards with the
`research/savestates/d17bo/` copy of UI slot 12, offscreen on the real GPU,
debug port). Every run checks the player's saves and settings before and
after (unchanged).

## What flickers (MEASURED)

- The train car is a static prop: model `0x8011c41c` (69 vertices, 65
  faces, 9,471 units long, sides at z +-898), drawn by the prop loop
  (`0x80032288`), object record `0x801d5d64` in this save (position at +4:
  7399, -5784, 39932; mesh at +0x44).
- Shots taken one at a time (one per request, about 6 images apart at
  180 Hz) never show it: they land on the same image phase. A burst of
  every presented image (new developer capture, see Tooling) does: walking
  forwards then back, **60 of 360 images** have the train's whole side
  differing from both neighbours by about 15 luminance levels while the
  neighbours match each other. Only the train changes (floor, walls and
  ceiling steady). The side alternates between warm lit (pink) and grey,
  image by image: a blink at the presentation rate.
- By lighting mode: RELIT 67 such images, ENHANCED 79 (about 4.6 levels),
  ORIGINAL none; Vanilla none. `lamp props off` (the D41X/D41Y prop lamps)
  changes nothing. `DNTTK_MODEL_BAKE=0` or `DNTTK_LAMP_BAKE=0` removes it:
  the owner is the D41P model bake (a model takes the baked lamps of the
  floor under its origin).

## Cause

D41P finds a model's floor from its mesh origin, rebuilt from the view: the
GTE translation (integer view space) times the inverse view rotation, plus
the eye. That carries the view's rounding (under a unit), and a live image
and its in-between redraws use different eyes. D57F snapped the point to 8
units so an origin on a 128-unit cell line (the LA time portal's pad) always
finds the same cell. The train's origin is z 39932: exactly half way between
39928 and 39936, the snap's own rounding boundary, and 39936 is a cell line
(312 x 128). Traced (`DNTTK_FLOOR_TRACE_MESH=8011c41c`): the rebuilt origin
wanders 39931.46 .. 39932.09, and the lookup alternates between floor 12413
(1467 lookups) and floor 12409 (51) in one burst. The two floors carry
different baked light, so the whole train took one or the other light in
turn. Any fixed grid has such boundaries; only an exact origin avoids them.

Ruled out on the way (each A/B unchanged or no flicker change): Polymorph vs
the Classic world renderer, texture and geometry precision, world
subdivision (D17N), model shadows, smooth light, smooth/warped/folded quads,
unified geometry, open models (D17BA), model and scene depth, and
perspective-correct vertex colour (tried in the GL shader, measured no
change, reverted). The large dark wedges on the train's side in single shots
are its own door and window frames sweeping past at a grazing angle, not an
artefact. The palette colours of the train's vertices never change.

## Fix (game-wide)

`recomp/src/ttk/near_clip.cpp`: in the object draw, when the caller is the
static prop loop (`0x80032288`) and the record in s2 names the mesh being
drawn (+0x44), the frame keeps the record's integer position (+4). The D41P
floor lookup then uses that exact position, without the snap, whenever it
agrees with the rebuilt origin to 16 units; otherwise (and for every other
model: actors, moving objects, Duke's layers) it keeps the snapped rebuilt
origin as before. The redraw workers replay the same draw with the same
registers, so live images and redraws now find the same floor. A prop on a
cell line still finds the same cell (the exact integer, as D57F's snap
intended). No level data; every static prop of every level.

Developer: `DNTTK_EXACT_ORIGIN=0` keeps the snapped view origin (A/B);
`DNTTK_EXACT_ORIGIN_TRACE=1` logs fallbacks and a per-process tally
(stderr); the `ttk_input` near report has `exact_origin` [used, fell back]
(main process only; most model draws run in the redraw workers).

## Evidence (offscreen, private copies; player files unchanged after every run)

- UI slot 12, walk forwards 1.0 s then back, 360 consecutive images, first
  person, player profile: RELIT 0 one-image flashes (was 67), ENHANCED 0
  (was 79), 120 Hz 0; `DNTTK_EXACT_ORIGIN=0` 52 (the flicker returns).
  Floor trace with the fix: 12413 every lookup once the level's bake is
  published (0 before it, as before).
- Level survey (levels 0-3, 5-12, 21-29 by console `level`): no fallback in
  any level; the record and rebuilt origins agree for every static prop
  (tallies of 80,000+ exact lookups per worker).
- Vanilla: loads and plays the save; no flashes (the model bake is
  Modernized only).
- `ttk-near-test` PASS; Python suite 269 OK (2 skipped); CI's runtime
  Python tests pass; the runtime patch applies cleanly to the pinned
  psxrecomp commit.

Remaining: the user's look on the real display at 180 Hz. Actors and moving
objects still use the snapped view origin; a standing actor or moved object
whose origin happens to sit 4 units off a multiple of 8 near a cell line
could still blink. None measured; their records are reachable at other
draw callers if one is reported.

## Tooling (developer, framework)

`present_shot` takes `count`: with a path holding one `%d` or `%0Nd`, each
of the next `count` presented images is written to its own file, so a
one-image flicker cannot fall between two requests. A `.raw` path skips PNG
encoding (8-byte width/height header, then RGB rows top-down), which keeps
the game at pace (PNG bursts stalled it to about one new image in 30).
`recomp/analysis/d17bo/flick.py` scores one-image flashes in a burst.
