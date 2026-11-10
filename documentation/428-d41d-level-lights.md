# D41D - Level light sources (first part: lamps, editor, level 0)

Status: **Done** (user-accepted 2026-11-09: "excellent lock it in and close it
out"). The remaining parts are D41D1. Modernized Polymorph with DYNAMIC LIGHTS
only; Vanilla and Classic unchanged. Builds on the D41A per-pixel path
([note 413](413-d41-lighting-case-study.md)).

## What is in

- **Lamps per level.** A text file per level,
  `recomp/assets/lights/levelNN.lights` (tracked, authored by us, like
  Duke-RT's `LIGHTOVR` or DarkPlaces' `.rtlights`), one lamp per line:
  `x y z  r g b  radius  strength  style  # note`. Loaded on entering a level
  (`level_lights.inc`, beside `spawn_update`). A dev build reads and saves
  the source tree's folder (compile definition `TTK_LIGHTS_DIR`); otherwise
  `lights/` beside the executable; `DNTTK_LIGHTS_DIR` overrides.
- **Per image.** `lights_publish` takes, after rounds in flight, the nearest
  lamps to Duke (at most 12; 8 of the 32 slots stay for blasts and
  flashes) among those whose rooms are shown. Since D41E there is no 6000
  range: a lamp fades only when nearer lamps push it out of the 12, over
  the last 4000 before the first lamp left out, or past 32000
  ([note 441](441-d41e-relit-levels.md)).
- **Styles** at the game tick: steady, flicker (neon buzz with dips), pulse,
  candle, strobe, broken (dark with bursts). Each lamp has its own phase.
- **Painted-light scan** (`lamp scan [threshold]`). The room table
  (`0x800da454`, 88-byte rooms: origin +0, piece count +0x32, piece list
  +0x40; a piece is a world mesh: +0x14 vertex count, +0x1c 8-byte vertices,
  x and z grid byte << 10, y byte << 7, RGB word) gives every world vertex
  and its painted colour; the decode matches each piece's stored bounds
  exactly (level 0: 84 rooms, 16629 vertices). Vertices at or above the
  threshold (170) are clustered (2560 across, 4096 in height), clusters of
  one room with a similar colour within 6000 merge, each room keeps its three
  largest, and each becomes a lamp at the cluster's weighted centre, 65% up
  its room column, in the cluster's mean colour, radius 2500-6000, strength
  0.6.
- **Editor** (backtick console `lamp`): list, add (at the point the
  crosshair meets, through the original segment query), here, pick, move,
  nudge, color, radius, power, style, note, del, save, reload, clear, scan,
  off/on. The selected lamp blinks for 2.5 s after an edit. Commands are
  queued to the game thread and run under the same guards as `spawn`.
- **Level 0 first set** (52 lamps): the 51 scan lamps plus a hand-placed
  flickering pink lamp at the club's "Bootylicious" sign.

## Signs keep their look (user report, 2026-11-09)

User, UI slot 9 (level 0 subway): "the exit sign is smudgy". Cause: the scan
puts a red lamp (room 17) beside the EXIT sign, from the red light the sign
paints on the corridor. The sign's vertices are not bright (its brightness is
its texture), so the lamp light added to the vertex colour pushed the red
letters past full before the texture multiply: they clipped flat and their
edges filled in. Fix, game-wide in the Polymorph textured shader
(`gpu_gl_renderer.c`, framework patch): on lit triangles, a texel that is
bright and strongly coloured (an emitter: sign letters, neon) keeps its
original colour, weight `smoothstep(0.3, 0.55, max) * smoothstep(0.6, 0.85,
saturation)`; other texels take light as before, and unlit triangles are
unchanged. It applies to every dynamic light (blasts and flashes too): a sign
gives light, it is never lit flat by one. Matched shots
(`research/before-after/D41D/exit-sign-*.png`, local): smudged, fixed, lamps
off; the fixed sign matches the lamps-off sign while the walls and ceiling
around it keep the lamp's red light; the club sign lamp still lights the
façade. A first threshold (0.5-0.8 brightness, 0.45-0.75 saturation) left the
letters' dimmer edges lit and a little heavy.

## Findings

- The colour-index nibble is 0 on every level 0 vertex. Index bit 4 (the
  animated light table, note 413 section 1) covers 3727 vertices in whole
  areas (canal, corridors), not single lamps: it marks surfaces that take the
  light table, not emitters.
- The painted light's coloured pools mark the level's light sources clearly
  (map: `research/before-after/D41D/level0-painted-light-scan.png`, local):
  red, pink and green neon in the club (rooms 75/76), blue room 68, cyan
  64/65, lilac 70/71, green 35, warm street pools.
- Signs and lamps drawn as textures (the club sign) leave no pool; the scan
  cannot see them. Those are hand-placed for now; texture emitters are the
  next part.

## Evidence

Private copies of UI slot 4 (file slot 3, level 0), Xvfb, debug port,
`recomp/analysis/d41d/probe.py` (local); player saves, cards and profiles
unchanged after every run.

- `lamp scan` in game: 51 lamps, the same as the Python prototype
  (`scan.py`); `lamp save` wrote `level00.lights`; `lamp list` lists them;
  `lamp add`, `nudge`, `style`, `note`, `save` placed the sign lamp.
- Club street, lamps off / on (`research/before-after/D41D/club-sign-*.png`):
  the façade glows pink around the sign and door. With the first 9000-unit
  scan radii the club's inner red lamps lit the façade through the wall;
  capping scan radii at 6000 stopped that.
- Vanilla (`--vanilla`): `lamp` answers "Modernized profile only"; no lamps
  are loaded.
- Framework patch re-exported (`export_runtime_patch.py`) for the shader rule.
- Build OK; `ttk-input-test`, `ttk-near-test` pass; `ttk-controls-test`
  builds (it stops at its known earlier fixture checks).

## Limits and next parts

- Light passes through walls (no shadows: D41F). Interior lamps should stay
  small; a lamp beside a thin wall lights both sides.
- Not measured at 180 Hz on the player's GPU: up to 12 more lights per lit
  pixel. Check for slowdown in the playtest.
- Next parts (D41D1): texture emitters (signs, monitors, lamps by texture
  page and CLUT, found automatically), TTK's animated light tables
  (`ctx+0x68`) decoded into styles that follow the original flicker, and
  first sets for more levels, each reviewed by the user.
- Lamps are chosen around Duke, not the camera; a far third-person camera
  can see a lamp switch in.
