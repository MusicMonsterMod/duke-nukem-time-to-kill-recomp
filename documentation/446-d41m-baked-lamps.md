# D41M - Baked lighting for fixed lamps: design note and level 3 prototype

Status: full build (Needs playtest, 2026-11-11, section 6); design note and level 3 prototype below. Job board: D41M.
Evidence labels: CONFIRMED (code or our own measurement), INFERRED, UNKNOWN.

## 1. Why the corridor stutters and pops (what a bake has to remove)

The live path, as it stands after D41G and D41F2 (code map, 2026-11-11):

- Selection (CONFIRMED, `level_lights.inc`): per game tick the level's lamps
  are gated by room (shown by the host walk, or within 3 portals with
  shadows on), sorted by distance minus radius from Duke, capped at 12
  (`level_lamp_max`), the last ones faded over 4000 units toward the first
  one left out. Shadow slots (`light_shadows.inc`): the strongest 16 by
  colour x r^2 / (d^2 + r^2) from Duke; lights past the 16th are darkened;
  at most 8 new maps a tick, and a light with no map yet is dark for that
  tick. A light keeps its slot only while it stays within 1.15x the reach
  its map was drawn for. As Duke walks the set changes, and lights fade in
  and out: the "lights turn on and off as you go down the corridor".
- Relit (CONFIRMED, `relit.inc`): every lamp's gain (to 1.70x) and reach (to
  1.35x radius) follow the camera room's ambient, eased over about half a
  second. Walking from an outdoor room into a lamp room makes every lamp in
  view stronger and wider at once.
- Cost per change (CONFIRMED): a new slot redraws six faces of a cube map in
  each of the three renderer instances (the emulation thread's and two
  render threads'); every lit world triangle is classified against every
  shadowed light in its mask on the render thread (D41F2, budget 400 us a
  light update); per redraw each lit triangle gets a light mask (D41G). Per
  pixel the shader loops over the triangle's lights, each with a shadow
  lookup (SMOOTH: an edge fetch and exact triangle tests).
- Observed (session log 2026-11-11 16:20): in the corridor, with 12-13
  lights, the worker redraws missed their presents for 45-50 presents at a
  time while walking; with `light mode original` the gaps stopped.
  INFERRED: the churn (new maps, new classifications) lands on the render
  thread at the moments the lamp set changes, which delays finished redraws
  ("render thread drawing" misses, note 445 section "Root causes" 3).

## 2. Options

What is fixed: each lamp's position, colour and radius (the level's
`.lights` file), and the walls that stop it (the static room mesh of D41F).
What varies: the brightness of animated lamps (candle 0.78-1.0 random each
tick, flicker, pulse, strobe, wave from the game's own table), traffic
lights' colour (D55), lamps that go out with their props (D54), and the
Relit gain. So a lamp's light at a point is its colour times a scalar that
changes, times a geometric term that never does: falloff, facing and
visibility. Precomputing that term is the bake.

- **A. Lightmaps on the world mesh** (Quake light styles; every engine's
  static lighting). Each world triangle gets a chart in an atlas; per texel
  the steady lamps' summed light and, for the strongest animated lamps of
  that triangle, one weight per lamp, scaled live by the lamp's current
  colour. Per pixel: one chart record, two or three filtered fetches, no
  loop, no shadow maps. Exact walls (rays against the same mesh). Needs a
  surface id per drawn world triangle (the face record is known at
  emission, note 413 / near_clip `world()`), a chart transform from world
  position (so clipped corners need no UVs), and two sides for single-panel
  walls (the shader lights the side the eye sees).
- **B. A light volume** (irradiance grid sampled by world position). No
  surface ids, but walls thinner than a cell leak light, and TTK's walls
  are single panels: rejected for the world. Kept for models (below).
- **C. Per-lamp stored shadow maps** (the live path without churn: every
  fixed lamp keeps a permanent map, no cap). Removes popping, keeps the look
  exactly, but the per-pixel loop and lookups stay and the memory grows
  with lamps (165 x 6 faces); the GPU cost remains.
- **D. Per-vertex bake**: TTK's polygons are 1024-unit grid cells; too
  coarse for lamp pools and wall shadows.

Choice: **A for the world, a light probe term for models** (Duke, enemies,
props; at most a few hundred pixels' worth of triangles): per model, per
tick, the fixed lamps that reach its centre and see it (rays against the
same mesh), as a handful of unshadowed lights. Live lights stay live: shots,
blasts, muzzle flashes, Necro blasts, fire, security TVs, test lights.

Design choices that change what the player sees (for review):

1. **Lamp reach no longer follows the camera's room.** A bake fixes each
   lamp's radius; it takes the Relit reach of the lamp's own room (the size
   it has when you stand near it). Today a lamp's pool widens or narrows as
   you walk between rooms of different ambient. The gain (strength) still
   follows the camera's room exactly as now.
2. **No cap, no fade, no room gate for fixed lamps.** Every lamp lights its
   surroundings all the time (walls still stop it). Corners now dark because
   their lamp was the 13th nearest, or two rooms away, become lit.
3. **Shadow edges from the bake's texel size.** D41F1's SMOOTH edges are
   exact per pixel; a lightmap edge is as sharp as its texels, filtered
   (the prototype: 32 world units, 2 x 2 visibility samples a texel; Duke
   is about 1000 units tall, so an edge softens over about 6 cm of his
   world). Finer texels cost memory (below).
4. Candle flicker: a face carries the strongest four animated lamps as live
   channels; a further animated lamp on it is baked at its average
   brightness (it stops flickering there). Level 3: 30 of 12,457 faces
   have more than four.

## 3. Prototype on level 3

Built (off by default; developer `DNTTK_LAMP_BAKE=1` or console `light
bake on|off|default` for this session):

- Bake (`recomp/src/ttk/lamp_bake.cpp`, pure computation): a BVH over the
  shadow mesh; per face (a triangle or a quad's two halves) a chart in the
  face's plane, clipped to the part a lamp reaches, one texel of border; a
  front and a back chart where lamps light both sides (single-panel walls:
  the shader lights the side the eye sees); per texel the live shader's own
  lamp term (falloff (1 - d/r)^2, facing 0.85|n.l| + 0.15, the side test)
  and visibility by rays against the same mesh, 2 x 2 samples. Steady lamps
  that no prop can break are summed (RGB9_E5); the strongest four lamps of
  each face that vary (styles, signals, breakable props) keep a weight each
  (RGBA8), scaled live by the lamp's colour. A few threads at the lowest
  priority; cached beside the executable (`cache/lamp-bake/levelNN-<hash>`),
  keyed by the mesh, the lamps and the lighting mode.
- Title (`lamp_bake.inc`, `lamp_bake_map.cpp`): faces from the shadow mesh
  build carry their record address; a shared map (made before the redraw
  workers fork, two halves flipped atomically) gives each drawn world face
  its surface; `near_clip` sends it with the face's corners. Per game tick
  every lamp's current colour (style, wave, signal, prop dying, Relit gain)
  goes with the lights. The live lamps stay for models, marked so baked
  surfaces skip them.
- Framework: a shared lightmap store (one reference-counted copy for the
  three renderer instances), per-vertex surface ids through the host vertex
  records and redraw streams (kind 6 with a surface, kind 9 lamp levels),
  the atlas uploaded about 4 MB an image, and the shader term added before
  the live lights' soft limit, so baked and live light combine as before.
  Developer `PSX_SHADOW_TUNE` 4th value 5: the baked light alone.

Level 3 (12,457 faces, 165 lamps): 8,113 faces lit, 1,191 with two sides;
atlas 4096 x 2746 at 32 units (90 MB a renderer instance, three instances);
baked in 3.3 s on 14 threads (now 4 low-priority threads), loaded from the
cache in about 55 ms. Offline at other densities (same mesh and lamps): 64
units 45 MB, 0.4 s; 24 units 268 MB; 16 units does not fit 16384 rows.

Bugs found and fixed on the way (developer builds only): the redraws' draw
time state outgrew its 8 KB slot (16 KB now, and a log line if it ever
does not fit: the redraws had silently lost all their lights); the bake's
uniform locations were set only after a lean/full program switch.

### Evidence (lab `recomp/analysis/perf/lab.py`, offscreen, private copy of UI slot 9)

Pictures (shots need `DNTTK_REDRAW_ONLY=0`: with redraw-only engaged the
debug port's `present_shot` captures the live frame, whose world is not
redrawn): `research/before-after/D41M/` (local), live and baked, one launch
each view:

| View | Pixels changed by more than 6 levels (live against live: noise) | Mean brightness live / baked |
| --- | --- | --- |
| down the corridor | 293,665 (5,183) | 31.4 / 35.5 |
| behind Duke | 10,350 (4,586) | 24.4 / 24.6 |
| the side wall | 75,567 (6,923) | 31.3 / 32.3 |

Near Duke the baked lamps match the live ones closely; down the corridor
the bake is brighter, because the lamps the live path leaves out (past the
12 nearest, or in rooms not shown) now light their stretch: design choice 2.

Cost, per redraw at 180 Hz, still view down the corridor, the game's own
GPU timers (`PSX_GPU_PHASES=2`; whole-GPU use was contaminated by a busy
browser on the desktop at the time, 13% at full clock with no game), two
launches each:

| Lighting | Textured draws a redraw |
| --- | --- |
| ORIGINAL | 3.29 / 3.33 ms |
| RELIT, live lamps | 5.15 / 4.67 ms |
| RELIT, baked lamps | 4.70 / 4.86 ms |

Walking down the corridor (5 s, 120/180/240 Hz, whole GPU, noisy): the
three redraw workers ran at 94-100% of a core in every configuration,
ORIGINAL included, and "workers behind" switch-offs happened in all of
them, also with the unmodified build (9-16 per launch).

Tests: `ttk-near-test` PASS (a stub for the new vertex call), `ttk-flash-test`,
`ttk-live-env-test`, `ttk-audio-test` PASS; `ttk-controls-test` stops at
its early fixture check, as before. With the bake off nothing reaches the
renderer (no surface ids, no flag-128 lights): inert by construction
(INFERRED; level 3's candle flicker makes shots differ run to run, so a
pixel comparison of off against the D50A build is not possible there).

### Findings

- The bake is not a GPU win in this view. D41G's light lists already leave
  fewer than half a lamp per lit triangle (`PSX_LIGHT_STATS`: 0.46), so the
  live loop is cheap; what lighting adds over ORIGINAL (about 1.4 ms a
  redraw here) is the lighting layer of the textured shader itself (the
  full program against the lean one), which both live and baked use.
- The corridor's stutter is the redraw workers, not the lamps: re-running
  the game's drawing for each redraw saturates three cores while walking
  there whatever the lighting (D23N).
- What the bake does give: no lamp popping on the world (no cap, no fade,
  no room gate, no shadow-slot churn for fixed lamps) and fewer failed
  redraws (0-1 a launch against 6-9 with the live lamps).
- Cost of the bake: 90 MB a renderer instance on level 3 at 32 units (three
  instances), a 3-10 s first bake per level (then cached).

### Recommendation

Do not build the full D41M as a performance job. Two smaller jobs serve the
user's two complaints better:

1. Stutter: D23N (captured redraws) removes the worker bound the corridor
   hits; nothing in the lighting can.
2. Popping: either finish the bake for its look (no popping, every lamp
   always lit) and accept its memory, or keep the live path and stop the
   popping where it comes from (a selection that does not cap or gate the
   lamps that light what is on screen, shadow maps kept for fixed lamps).
3. A baked-only shader variant (bake lookup and Relit ambient, no live loop
   or shadow code) for batches no live light reaches could recover part of
   the 1.4 ms lighting layer; only worth it with the bake.

The prototype stays in the build, off by default, for the user's review.

## 4. Plan for the full build

After the user's review of the design choices above:

1. Every lit level: bake on load (cached), checked level by level against
   the live path in matched shots (`research/before-after/D41M/`).
2. Models (Duke, enemies, props): a probe term (the fixed lamps that reach
   and see the model's centre, few rays a model a tick) instead of the live
   lamp list, so no fixed lamp needs a shadow slot; the 16 slots then serve
   live lights only and no map is drawn for a lamp.
3. Memory: share the lightmap textures across the three GL contexts (one
   upload), and choose the texel size per level from the measured area.
4. Make it the default in Relit and Enhanced; ORIGINAL, Classic and
   Vanilla unchanged (no world positions there, so no baked surface).
5. Shadows off: the bake keeps the walls (a look change against today's
   unshadowed lamps) or bakes unshadowed: a choice for the user.

## 5. User decisions (2026-11-11)

The user preferred the baked look in play and chose to finish the bake
(no lamp popping, every lamp always lit; no performance gain expected). On
the design choices of section 2: lamp reach fixed per lamp, shadow edges
at 32 units, walls always stop baked lamps (also with SHADOWS off). The
full build follows section 4.

## 6. Full build (2026-11-11)

Built with the user's choices of section 5:

- **On by default** in ENHANCED and RELIT on every level (developer
  `DNTTK_LAMP_BAKE=0` or console `light bake off` keeps the live lamps for
  the session). ORIGINAL, Classic and Vanilla never reach it.
- **Walls always stop baked lamps**: with SHADOWS off the room mesh is still
  built for the bake (no shadow slot is assigned, so no cube map is drawn).
- **Lamp reach fixed per lamp**: the bake uses each lamp's Relit reach in its
  own room, and the live lamp that lights models now carries the same reach
  (it no longer follows the camera's room), so a model and the wall beside it
  see the same pool.
- **One upload, shared**: the emulation thread's renderer uploads the
  lightmap a few rows an image into new textures and publishes them with a
  fence (`psx_lightmap_gl_publish`); the render threads adopt the same
  textures (`psx_lightmap_gl_adopt`, a GPU-side wait) instead of uploading
  their own; replaced textures are deleted 3 s later. Level 3 peak GPU memory
  1980 -> 1840 MiB.
- **Loads keep the bake**: a savestate load of the same level held the bake
  off for half a second (inputs momentarily missing) and then took it from
  the cache again (its key counted mesh rebuilds); the bake now waits up to
  5 s for the same inputs and is keyed by the mesh's content. Three loads of
  UI slot 9 in a row: one bake, no switch-off.
- **Models**: still lit by the live lamps (the nearest 12, shadowed), not by
  the light probe of section 4: a probe of unshadowed lamps would light a
  model through a wall that the live lamps' shadow maps stop. Recorded as
  the remaining step (below).

### Evidence (offscreen, private card copies, the player's profile copy)

Every level the saves reach, first visit then cached (`cache/lamp-bake/`
beside the executable; matched baked and live shots per slot in
`research/before-after/D41M/` (local), `lv<level>-s<slot>-baked|live`):

| Level | Faces | Lamps | Atlas | First bake | From the cache |
| --- | --- | --- | --- | --- | --- |
| 0 | 12,484 | 90 | 4096 x 2588 (84.8 MB) | 8.2 s (314 channel overflows) | 51-67 ms |
| 1 | 8,257 | 50 | 4096 x 1038 (34.0 MB) | 1.7 s | 8 ms |
| 2 | 12,027 | 97 | 4096 x 2546 (83.4 MB) | 5.4 s | 49 ms |
| 3 | 12,457 | 165 | 4096 x 2746 (90.0 MB) | 3.3 s (prototype) | 53-59 ms |
| 6 | 13,440 | 46 | 4096 x 1913 (62.7 MB) | 3.9 s | 57 ms |

Level 0's 314 channel overflows are faces with more than four animated
lamps (its traffic lights and neon): those beyond four are baked at their
average brightness there (design choice 4).

Corridor (UI slot 9), the D41M pictures `full-corridor-q0..q3`: baked,
live, SHADOWS off baked (the walls still stop the lamps), SHADOWS off live.
`PSX_RT_VALIDATE` (classes and reuse off): 0-3 differing redraws of about
105 against 0-1 with the prototype (per-instance uploads): within the run
noise. Corridor walk at 180 Hz with the bake: GPU 47.6% (51.0% live),
0 switch-offs.

### Remaining

- Models from a light probe (fixed lamps that see the model, frees the
  shadow slots for live lights), and the "baked-only" shader variant for
  batches no live light reaches (section 3, recommendation 3): not built.
- The first visit to a level shows the live lamps for its bake time (2-8 s),
  then switches; the user should judge whether that switch is visible.
- Needs the user's playtest: the look level by level, the corridor, loads.
