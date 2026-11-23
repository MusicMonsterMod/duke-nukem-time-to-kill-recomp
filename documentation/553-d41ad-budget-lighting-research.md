# D41AD - LIGHTING ENHANCED as a budget mode: where its cost goes, and three designs

[D41AD](../jobs/lighting.md#d41ad---lighting-enhanced-becomes-the-budget-lighting-mode),
2026-11-23. Sections 1-6: the research (the cost of today's ENHANCED and
three designs). The user chose design A ("your recommendation!"); section 7
is its implementation, now LIGHTING ENHANCED, and its measurements;
sections 8-9 the user's further trims and the distance fade. Done,
user-accepted 2026-11-23 ("thats the one!!!").

## 1. Method

MEASURED on the field laptop (i7-5600U / HD 5500, 15 W, CachyOS, Mesa iris),
real 1080p 60 Hz screen, GRAPHICS BALANCED (2x, LIGHTING ENHANCED, simple
shadow edges, draw distance extended), FRAME RATE 60, third person. The D23AF
census route (stand, a slow 360 turn, a walk with turns, look up and down, a
weapon burst) on levels 6, 8, 22 and 27 (THE REAPER), one configuration a
launch, two alternating repetitions of every configuration, four rounds. Same
binary as the desktop build (built here, copied; the laptop's own binary
restored afterwards). Private profile and card copies; the player's files
hashed unchanged on every run. Values are the per-level medians of the
census's one-second samples, then the mean of the two repetitions and the
four levels.

- GPU %: clock-weighted busy (i915 RC6 residency and the actual clock).
- worker %: the single redraw worker's CPU.
- p10: the lowest tenth of new pictures a second.

New developer switches (`PSX_SHADER_STRIP`, compiled into the textured program
at start, wrong pictures except `loop`; cost bounds, not for play):

- `loop`: the painted light and the bake kept, the live-light loop and the
  shadow layers compiled out.
- `bake1`: as `loop`, with the bake replaced by one filtered fetch of the
  steady lightmap at a made-up planar UV.
- `bake0`: as `loop`, with the bake replaced by a constant (no fetch).

## 2. Results

| configuration | GPU % (6 / 8 / 22 / 27) | GPU mean | over ORIGINAL | worker mean | p10 (22 / 27) |
|---|---|---|---|---|---|
| LIGHTING ORIGINAL (4 rounds) | 42 / 39 / 34 / 32 | 36.7 | 0 | 53.9 | 58 / 56 |
| `PSX_SHADER_STRIP=lights` (ENHANCED's CPU work, no lit shader path) | 46 / 40 / 36 / 34 | 39.1 | +2.4 | 54.4 | 57 / 59 |
| `bake0` (lit path entered, no fetch) | 48 / 42 / 37 / 34 | 40.2 | +3.5 | 54.4 | 47 / 58 |
| `bake1` (one fetch, made-up UV; 2 rounds) | 51 / 47 / 41 / 41 | 44.6 | +7.9 | 59.4 | 48 / 58 |
| `loop` (bake only, no live lights, shadows, model fx, smoothing) | 48 / 48 / 45 / 43 | 46.2 | +9.5 | 58.8 | 40 / 57 |
| `loop` with every other ENHANCED part on | 52 / 50 / 46 / 44 | 48.0 | +11.3 | 60.6 | 52 / 52 |
| LIVE SHADOWS OFF | 53 / 51 / 47 / 46 | 49.3 | +12.6 | 67.6 | 28 / 46 |
| model shadows and glows off | 55 / 49 / 46 / 47 | 49.1 | +12.4 | 63.0 | 37 / 56 |
| shadows, model fx, smoothing off, shadow code compiled out | 50 / 52 / 47 / 48 | 49.3 | +12.6 | 63.1 | 37 / 56 |
| LIGHTING ENHANCED today (3 rounds) | 52 / 50 / 48 / 44 | 48.6 | +11.9 | 60.5 | 48 / 54 |

Every configuration held 60 game fields a second with no fallback on these
four levels; the differences are GPU load, worker CPU and the low tenth.
Run-to-run spread is about 2 GPU points (the ORIGINAL rounds: 35.6-37.3).

## 3. What the numbers say

- **The cost is the textured fragment shader's lit path, not the light
  sources.** With every one of ENHANCED's CPU jobs still running (lamp lists,
  triangle light masks, shadow-map passes, the bake upload) but the lit path
  compiled out, the laptop is 2.4 points over ORIGINAL. LIVE SHADOWS OFF,
  model shadows and glows off, smooth normals off and the shadow code compiled
  out save nothing measurable (within the spread).
- **The extra worker CPU in ENHANCED is the 15 W package, not light work.**
  Worker CPU follows GPU load (53.9% at ORIGINAL, 54.4% with all the CPU
  light work but no lit shader, 60.5% at ENHANCED): the GPU's power is taken
  from the cores (note 509 section 3).
- **The ledger** (GPU points over ORIGINAL): everything outside the lit
  shader path (CPU light work, shadow-map passes, uploads) +2.4 at most
  (`lights`); entering the lit path with no light and no fetch +3.5 (`bake0`,
  shadows and model fx off: the world position and normal inputs, the light
  masks, the emitter test); the bake lookup as written about +6 more (`loop`
  minus `bake0`: four RGBA32F chart fetches, the steady lightmap, the channel
  map and up to six lamp fetches a pixel, on every textured batch); the
  live-light loop, shadows and the rest about +2.4 more. Total +11.9.
- `bake1`'s made-up UV spans the 4096-texel-wide atlas in 4096 world units,
  about 32 times denser than the bake's real 32 units a texel, so it
  thrashes the texture cache: +7.9 is an upper bound for one fetch, not an
  estimate.
- **Mesa compiles every textured program SIMD8 only on this GPU**
  (`INTEL_DEBUG=fs`), ORIGINAL's lean program too, so SIMD width is not the
  difference. Static estimates: lean 901 instructions / 43.7k cycles, today's
  full program 2115 / 699k (3 loops), `loop` 1366 / 62k, `bake1` 1016 / 45.9k.
  `bake1` adds 5% instructions and costs 4 points more than `bake0`: on this
  iGPU the cost is memory latency (sends), not ALU.
- **The lean program almost never runs in ENHANCED.** `tex_prog_pick`
  chooses it per frame only when no light exists and the bake is off, so with
  baked lamps on, every textured batch runs the full program, including
  batches no lamp reaches.
- **Look**: at two lamp-lit desktop saves (subway platform and train,
  offscreen, the lab's card copy), the bake alone (`loop`) reproduces today's
  ENHANCED on the platform (0.3% of pixels differ by more than 8 levels,
  against 13.7% ENHANCED vs ORIGINAL); at the train, whose lamps are live
  prop lights, it keeps about 60% of the change (5.3% vs 8.7%).

The acceptance target (within about a third of today's +11.9: about +4)
cannot be met by trimming today's pipeline (`loop`: +9.5). It needs a
cheaper bake lookup and the lean program wherever nothing lights a batch.

## 4. Designs (PROPOSED; costs are estimates from section 3)

### A. Quake-style one-fetch lightmap (recommended)

The id Software model, checked against the released GLQuake source
(`WinQuake/gl_rsurf.c`, `gl_rlight.c`): every lit surface reads one lightmap
texel (16 units a luxel there, 32 here) through a UV computed per vertex;
changing lights (lightstyles, dynamic lights) are rebuilt on the CPU for the
touched surfaces only and uploaded as dirty rows (`R_BuildLightMap`,
`R_AddDynamicLights`: linear falloff `rad - dist` after subtracting the
light's distance from the plane).

- The bake's chart transform and the front/back choice move off the pixel:
  per vertex (the chart is planar, so the UV interpolates exactly) and per
  triangle (the eye's side of a plane is constant over it).
- One composited RGBA8 lightmap: the steady light plus every lamp channel at
  its current colour, so the pixel reads one texel instead of chart, steady,
  channel and lamp textures. Flickering and breakable lamps are re-composited
  on the CPU for their texels only (Quake's lightstyles), dirty sub-rectangles
  uploaded.
- Per-batch program choice: a batch with no baked surface and no live light
  runs ORIGINAL's lean program.
- Live lights (muzzle flashes, blasts, burning things, rounds in flight): a
  small unshadowed loop (up to 4) in a third program used only by batches a
  live light reaches (A1, simplest, consistent with the redraw workers), or
  Quake dlights added into the lightmap texels on the CPU (A2: cheaper on the
  GPU, but every redraw worker's picture needs the texels of its own snapshot;
  more work and more risk).
- Models: one colour from the floor's lightmap a model (D41P), on the CPU.
- Estimated +3 to +5 over ORIGINAL (the `bake0` plumbing +3.5 less the lean
  batches, plus one cache-friendly RGBA8 fetch); lamps and fixtures keep
  their baked pools and wall shadows, blasts and flashes stay lit. No live
  shadows, no smooth model shading, no per-pixel model light.
- Work: renderer (new program variant, vertex-stage chart lookup, lightmap
  compositor), game side (per-lamp texel lists, flicker updates). The largest
  of the three; validate on the laptop and the desktop at 180 Hz.

### B. Glows only (Quake's `gl_flashblend`)

GLQuake's cheap path: each dynamic light is an untextured, additive,
camera-facing disc (radius 0.35 x the light's, orange centre fading to black,
depth writes off), and no surface is lit. Here: ORIGINAL's shader everywhere,
the existing halos (D41M, D57) and new glows for blasts and flashes.

- Estimated +0.5 to +1 over ORIGINAL (a few blended quads).
- Lamps and blasts glow, but light no wall, floor or model: lamp pools,
  baked wall shadows and the dark-room mood are gone. Fails "lamps visibly
  lit" unless the user accepts glow as the look.
- Work: small (halos exist; flash and blast glows to add).

### C. Today's ENHANCED trimmed

Today's pipeline with the live loop, live shadows, model shadows, glows and
smoothing off (`loop`, section 2): measured +9.5 over ORIGINAL against
+11.9. Looks like today's ENHANCED for fixed lamps, loses moving and prop
lights. Smallest work, but misses the target by more than a factor of two:
listed only to show why trimming is not enough.

### Not again

Per-vertex world lighting (TTK's 1024-unit polygons, notes 413 section 4 and
446 section 2 D) and a world light volume (leaks through single-panel walls,
note 446 section 2 B) stay rejected. Every lighting effect still needs DRAW
DISTANCE EXTENDED (D41V).

## 5. Other engines

- GZDoom: the cost of its shader dynamic lights grows with the sum of
  surfaces each light touches (lead developer, ZDoom forum); its budget switch
  `gl_lights_additive` makes every light additive, "improves performances, but
  degrades image quality" (ZDoom wiki, Dynamic light options).
- EDuke32 Polymost has no dynamic lights (shade tables only), the parity
  target for ORIGINAL.

## 6. Evidence and limits

- Lab: `recomp/analysis/d41ad` (local): desktop `probe.py` and look shots;
  `laptop/` holds the laptop scripts (`ab.sh`, `r2.sh`-`r4.sh`), every run's
  census output, the Mesa stats runs and `summary.json`.
- Four levels, one route, third person, BALANCED 60 only. Not measured: first
  person, the D57 lights' own levels on the laptop, citadel (Vega 8), the
  desktop at 180 Hz. The design A and B costs are estimates; a prototype must
  be measured on the laptop before acceptance.

## 7. Implementation: LIGHTING ENHANCED as the budget mode (design A)

User, 2026-11-23: "your recommendation!" (design A). The name stays
ENHANCED for now (a menu label change if the user wants LITE or FAST).

### What ENHANCED does now

- Game (`dynamic_lights.inc` `light_budget_on`): ENHANCED (Modernized,
  Polymorph, lights on, not RELIT) publishes the budget flag every tick
  (`psx_mod_gpu_set_light_budget`, a process-wide flag beside the shadow
  edges, `shadow_mesh.c`; also part of the redraw reuse key). Live shadows
  are off in ENHANCED (`shadows_on`), so MODEL SHADOWS and GLOWS take their
  place there as with LIVE SHADOWS OFF (`near_clip.cpp` `d41s_base`). The
  shadow mesh is still built for the bake. Console `light budget off|on`
  (session, live env `DNTTK_LIGHT_BUDGET`) draws the old ENHANCED for
  comparisons.
- Renderer (`gpu_gl_renderer.c`): four budget programs (the bake only; the
  bake and the live lights; each with the night-vision remap), built with
  `SH_BUDGET` in both stages and no shadow layers. The vertex stage reads the
  surface's chart (four RGBA32F texels) once a vertex and passes the chart's
  texel coordinates and the side test (smooth: both linear in the world
  position, which interpolates linearly with w 1, so they are exact) and the
  two atlas origins, the chart size and the lamp ids (flat). The fragment
  stage picks the side, clamps, and reads the steady lightmap once; the
  channel map and lamp levels only on surfaces with flickering, pulsing or
  breakable lamps (a flat test on the lamp ids).
- Per batch (`tex_prog_pick`, a scan of one vertex a triangle): the lean
  program where no lit triangle has a baked surface or a live light, the bake
  only where none has a live light, else the bake with the live lights.
- Dark surfaces: when an upload completes the emulation thread's renderer
  marks every surface whose chart has no lamp light on either side; the store
  publishes the table with the textures (`lightmap_store.c`
  `psx_lightmap_gl_dark`, copied by each render thread for its own
  generation, else nothing is culled). In the budget mode a lit triangle on
  such a surface drops its surface (the same pixels: the budget shader reads
  nothing there), so more batches go lean.
- RELIT, ORIGINAL, Vanilla and Classic never set the flag: their programs'
  preprocessed source is unchanged (`#ifdef SH_BUDGET` blocks), and
  `shadows_on`, the reuse key and the triangle queue behave as before.
- Developer switches: `PSX_LIGHT_BUDGET=0` (no budget programs built),
  `PSX_BUDGET_TEST=nofetch|noloop` (cost bounds, wrong pictures), the D41AD
  research strips `PSX_SHADER_STRIP=loop|bake1|bake0`, and per-class batch,
  vertex and dark-triangle counts in `PSX_GL_FLUSH_DIAG=1`.
- Codegen hash unchanged (0xebce51ea): savestates load.

### Exactness

MEASURED, desktop, offscreen, the lab's card copy:

- `PSX_REPLAY_VALIDATE_LEAN=1` (with `PSX_RENDER_THREAD=0`; lean not allowed
  draws every budget batch with the lights program, so it checks the per
  batch choice and the dark culling) while firing and turning: 6336 and 7808
  redraws at two saves, 2 failures each: one pixel 7 levels off at the top
  edge of the widescreen margin (once a run), and the session's switch from
  the lab profile's RELIT to ENHANCED rebuilding the bake between the two
  draws (the validator's own artifact).
- The vertex-stage bake against the old per-pixel bake (`light budget on`,
  `off`, `on` in one session; pixels that also change between the two `on`
  shots - Duke's idle animation, the track gravel - left out, 85% of the
  picture kept): 0.06-0.07% of pixels more than 8 levels apart, largest 24-30
  levels, mean 0.01. Those scenes have no live shadows to lose.

### Cost

MEASURED, field laptop, the same method as section 1 (real screen,
BALANCED 60, levels 6, 8, 22, 27, two repetitions, the laptop's own binary
kept as `.pre-d41ad`). Each round has its own ORIGINAL baseline (it drifts
36.4-40.3 between rounds), so the comparison is within a round:

| round | ORIGINAL | new ENHANCED | over ORIGINAL | old ENHANCED (`DNTTK_LIGHT_BUDGET=off`) |
|---|---|---|---|---|
| 5 (no dark culling, model fx off) | 38.3 | 45.1 | +6.8 | 50.2 (+11.9) |
| 6 (final build) | 36.4 | 42.9 | +6.5 | - |
| 7 (final build) | 40.3 | 43.9 | +3.6 | - |

- The new ENHANCED costs about +5.6 GPU points over ORIGINAL (mean of the
  three rounds; spread about 2), against +11.9 for the old: about half.
  Worker CPU 59.6% against 63.1% (ORIGINAL 55.6%).
- Round 7 `PSX_SHADER_STRIP=lights` (every CPU part of the budget mode, the
  plain shader): 38.5, below its ORIGINAL: the CPU light work costs nothing
  measurable; what remains is all in the shader's lit path.
- Round 6 bounds: `nofetch` +5.1, `noloop` +7.4 (within the spread of
  +6.5): neither the lightmap read nor the live-light loop is the remainder;
  it is the lit path's own overhead (its inputs, the emitter test, the
  soft limit). About 10 live prop lights are always in view on these levels
  (lanterns, train lamps; probe), so 93% of textured vertices draw with the
  lights program; the bake-only program rarely applies.
- Desktop, GTX 1080 Ti, SDL offscreen, BALANCED at 180, levels 6, 22, 27,
  two repetitions: every mode 179.9 pictures a second; GPU 11.5% ORIGINAL,
  11.8% new ENHANCED, 12.6% old ENHANCED, 12.9% RELIT. No regression at
  180.
- The low tenth of pictures on levels 6, 22 and 27 stays about as with the
  old ENHANCED (35-50 against ORIGINAL's 55-58): those dips follow the redraw
  worker.

### Against the acceptance

- Lamps, fixtures, flickering lamps, blasts, shots, fires and prop lights
  stay lit (the same bake and live lights; no live shadows).
- Cost: about +5.6 over ORIGINAL; the target was within about a third of the
  old +11.9 (about +4). Close, not clearly met.
- BALANCED 60 holds (medians 57-60) where ORIGINAL does; the low-tenth dips
  remain.
- The user's laptop preference: needs a playtest.

### Next levers (PROPOSED, not built)

- The lit path's fixed overhead: fewer fragment inputs (pack the flat chart
  data; drop the smooth model normal when no live light reaches a batch) and
  a cheaper emitter test and soft limit in the budget programs.
- Split batches where the program class changes, so triangles no live light
  reaches use the bake-only program (only worth it once the fixed overhead is
  smaller: the loop itself measured within the noise).

## 8. Trimmed further (user: "trim what it does by 25%")

User, 2026-11-23, after the laptop playtest: the new ENHANCED looked like the
old one and "even too high fidelity"; "trim what it does by 25%... turn off
superfluous lighting and make some serious budget cuts. dont even worry
about the shadows". Taken as: at least a quarter off its cost over ORIGINAL.

### Cuts (ENHANCED only)

- **Batches split by class** (`gpu_gl_renderer.c`, the triangle queue): the
  triangle's class (painted light, bake, live lights) is a batch key, so a
  triangle no light reaches draws with the lean program even beside lit ones.
  A class change ends a batch only once it has 32 triangles (a smaller batch
  takes the higher class); developer `PSX_BUDGET_SPLIT_MIN` (0: no split).
  Exact: `PSX_REPLAY_VALIDATE_LEAN=1` 7360 redraws, 0 failures. At the
  subway save 95% of textured vertices now draw lean (7% before).
- **8 live lights** (`dynamic_lights.inc`): the existing importance ranking
  (D41T) keeps the 8 most important (bright, near Duke) instead of 32;
  developer `DNTTK_LIGHT_BUDGET_MAX`.
- **Light distance 8000**: a triangle whose nearest corner is farther than
  8000 units from the eye takes the painted light only (no bake, no live
  light); developer `PSX_BUDGET_FAR` (0: no limit). Distant lamp pools are
  not drawn: they appear as Duke comes within that distance.

### Cost

MEASURED, field laptop, same method (round 8, two repetitions each, one
binary; the previous budget mode by `DNTTK_LIGHT_BUDGET_MAX=32,
PSX_BUDGET_FAR=0, PSX_BUDGET_SPLIT_MIN=0`):

| configuration | GPU % (6 / 8 / 22 / 27) | mean | over ORIGINAL | worker % |
|---|---|---|---|---|
| ORIGINAL | 44.8 / 38.1 / 36.3 / 31.1 | 37.6 | 0 | 54.5 |
| previous budget ENHANCED | 43.0 / 42.7 / 37.2 / 37.4 | 40.1 | +2.5 | 61.7 |
| trimmed ENHANCED | 46.0 / 40.5 / 34.0 / 30.6 | 37.8 | +0.2 | 58.2 |

The trimmed ENHANCED costs the laptop's GPU what ORIGINAL does (within the
spread), against +11.9 for the original ENHANCED. Its redraw worker is 3.7
points over ORIGINAL (the old ENHANCED: 7.5). Level 22's low tenth of
pictures dips in both budget builds (worker about 76% there, ORIGINAL 60%):
CPU-side, not these cuts.

### Remaining

- The user's look check of the cuts on the laptop (lights dropped by the
  8-light cap in busy scenes; lamp pools appearing at 8000 units).
- Level 22's worker-bound dips in ENHANCED (CPU light work there).

## 9. The light distance cut, measured, then faded

MEASURED, field laptop, round 9 (three repetitions each): ORIGINAL 36.2%
GPU, trimmed with the 8000 cut 40.2 (+4.0), trimmed without a distance limit
(`PSX_BUDGET_FAR=0`) 43.1 (+6.9); THE REAPER (level 27) 29.7 / 33.3 / 42.3,
its low tenth 59 / 55 / 39. The limit earns its keep on large levels. Rounds 8
and 9 disagree on the trimmed mode (+0.2 and +4.0; ORIGINAL itself moves 3-4
points between rounds): about +2.5 taken together.

User, on the laptop: the cut's edge "looks like a huge hard shadow that just
creeps around the surface... makes original actually look better" (the cut
went by each large triangle's nearest corner, so its edge was jagged and slid
as the camera moved); reference: EDuke32, where a big room's far end darkens
smoothly (Build's per-sector visibility and shade tables; fullbright texels
keep their glow). "make it really lovely."

Now (TEX_VS, budget programs): each vertex carries a fade, 1 to 5000 units
from the eye and 0 at 8000 (smoothstep; 5/8 of `PSX_BUDGET_FAR` to all of
it), interpolated over the triangle; the added light (bake and live lights,
before the soft limit) is multiplied by it, so the lamp light dims into the
painted light with no edge, and emitters keep their look (as Build's
fullbrights). The CPU still drops only triangles whose every corner is past
8000, where the fade is already 0: the same pixels, the same saving.

MEASURED, round 10 (three repetitions each): ORIGINAL 37.3%, ENHANCED with the
fade 39.8 (+2.5); THE REAPER 30.0 / 32.5; worker 55.5 / 58.4. The fade costs
what the cut did. The laptop runs this build (its own kept as `.pre-d41ad`).

Overall, LIGHTING ENHANCED on the field laptop: +11.9 GPU points over
ORIGINAL at the start of the job, +2.5 now (about a fifth).

Remaining for that laptop: the worker-bound dips (level 22: worker 72%
against 62% at ORIGINAL, low tenth 36 against 58): CPU light work there, and
the redraw worker's ceiling on the heavy levels at any lighting (note 509).

