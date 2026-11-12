# D41M - Baked lighting for fixed lamps: design note and level 3 prototype

Status: full build (Needs playtest, 2026-11-11, section 6), light slivers fixed (section 7); design note and level 3 prototype below. Job board: D41M.
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

## 7. Light slivers (2026-11-11)

User: "various slivers of light in strange places ... fix every single
one", saved in UI slot 10 (level 6: a lit strip up a wall corner) and UI
slot 7 (level 3: one lit floor triangle).

Isolation (CONFIRMED, offscreen, private card copies): both appear with the
bake on, with SHADOWS on and off, and vanish with `light bake off`. With
SHADOWS off the D23O shadow classes do not run, so they are not the cause.

Causes and fixes (CONFIRMED by before/after shots):

1. **Charts read past their edges.** A chart covers only the part of its
   face that some lamp reaches (section 3), but every drawn pixel of the
   face looked up `chart origin + st` with no bound, so the rest of the face
   read whichever chart sat beside it in the atlas, filtered: whole
   triangles and strips lit by lamps that never reach them. The bake now
   stores the chart's size in the record's spare float (`w * 4096 + h`;
   charts with a side past 4095 texels are left to the live path) and
   `baked_light()` clamps `st` to the chart's texel centres, so the rest of
   the face keeps the chart's unlit border. Fixed both reported spots.
2. **Seams where faces meet.** Texel samples outside the face were clamped
   exactly onto its edge; where the face meets a wall or floor, the ray
   origin (2 units off the face) then lay on the other surface's plane and
   its hit (t about 0) was ignored, so light from behind leaked along the
   seam (a thin lit line along a wall foot, UI slot 8). Samples are now
   pulled up to 2 units into the face before the rays.

Cache key version 5: every level rebakes once (level 3: 7.2 s at low
priority), then loads from the cache. Sweep: all 12 slots, four directions
each, baked against live, before and after: no remaining sharp-edged lit
patches; remaining differences are the design choices of section 2 (lamps
the live path left out), sky animation and game events. Cost: one clamp per
lit pixel. Shots: `research/before-after/D41M/slivers/` (local).

## 8. Acceptance pass, first-bake switch (2026-11-12)

The leaks of section 7 are fixed and confirmed by the user. This pass checks
the job's remaining acceptance points offscreen and removes what still
showed when a level's bake appears.

### Lighting cost against ORIGINAL (180 Hz, offscreen, slow turn)

GPU work (clock-weighted), ORIGINAL / RELIT alternated twice per save, on
every level the saves reach (the player's bake cache):

| Level (lab slot) | ORIGINAL | RELIT (baked) |
| --- | --- | --- |
| 0 (3) | 17.4, 16.8% | 18.6, 16.3% |
| 1 (4) | 21.6, 18.9% | 20.3, 18.4% |
| 2 (2) | 15.0, 14.5% | 15.9, 11.9% |
| 3 (0) | 15.1, 16.9% | 14.2, 15.7% |
| 6 (1) | 18.8, 27.5% | 25.0, 19.3% |
| 6, another view (9) | 17.7, 19.2% | 17.4, 15.7% |
| 3, corridor (8, UI slot 9) | 19.0, 27.1% | 27.1, 20.6% |

The differences go both ways and are within run noise (the 27% readings came with the whole process busier, ORIGINAL and RELIT alike): dynamic lighting with
the bake costs nothing measurable over ORIGINAL headless (acceptance: at most
a few points). 180 presents/s in every run.

### Corridor walk (UI slot 9, RELIT, walk/turn/fire pattern, 30 s)

Presents a second, every second: 119-120 at 120 Hz, 179-180 at 180 Hz,
238-240 at 240 Hz; fields 56-63. No second shows a redraw-only switch-off.

### First visit to a level: the switch from live to baked

On a level's first visit (no cache) the live lamps show while the bake runs,
then the bake takes over. Two changes:

1. **Faster first bake.** Half the logical CPUs, at most 8 threads, at the
   lowest priority (was a quarter, at most 4), and the charts are taken
   largest first. With 16 CPUs: level 0 8.3 -> 5.1 s, level 3 6.3 -> 3.6 s,
   level 2 5.6 -> 3.7 s, level 6 4.1 -> 2.8 s, level 1 1.8 -> 1.2 s. The game
   kept 59-62 fields/s and 176-184 presents/s through the bake with 4 and
   with 8 threads; 12 gained little (8 cores). The bake files are
   byte-identical (all five levels, 4, 8 and 12 threads).
2. **No hitch at the switch.** The atlas textures were made with
   `glTexImage2D(NULL)` and filled a slice an image; the first draws that
   sampled the finished atlas blocked the emulation thread about 65 ms
   (fields 36-40 a second over a tenth of a second, also with the render
   threads off). Found with the D23Q gap log (`PSX_HOST_GAP_LOG_MS`) on one
   clock: bake active, upload 0.2 s (2 ms a slice), publish, then the block.
   Not a shader recompile (`PSX_GL_DEBUG=1`, new developer switch, logs the
   driver's messages: none at the switch). Smaller slices did not help.
   Immutable storage (`glTexStorage2D`) removes it: fields 54-67 a second
   through the switch, no block of 40 ms or more (levels 0 and 3).
   Developer `PSX_BAKE_TEXSTORAGE=0` keeps the old allocation for A/B. The
   same path serves cached loads.

Also: the bake key had lost the lamp count to a comment (harmless, the lamps
themselves were mixed); developer `DNTTK_LAMP_BAKE_CACHE` and
`DNTTK_LAMP_BAKE_THREADS` let the lab time first bakes without touching the
player's cache.

### Accepted (2026-11-12)

On the real display the user checked UI slots 2, 7, 9 and 10 (slot 2 at most
40% GPU in btop) and the first-visit switch (level 3 bake deleted, UI slot 9
loaded: no hitch): "it worked perfectly. in fact it all worked perfectly".

### Remaining

- Real-display figures (headless excludes the window present and the
  compositor).
- Not built (no measured need now): models from a light probe, the
  baked-only shader variant.
