# Time to Kill jobs - Lighting

Job descriptions and work logs for D41 and its children, D54-D57. Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D41 - Foundational lighting system (pre-job)

**Status: Done (user-accepted; closeout authorized 2026-11-08; case study published: https://claude.ai/artifact/M1MNyrZqtgzEFc7s72jSXh, [note 413](documentation/413-d41-lighting-case-study.md); child jobs D41A-D41H added; the user authorized autonomous work on the child jobs, and D41A is implemented). Dependencies: none. Lighting itself is implemented under the child jobs, not under this ID.**
User, 2026-11-03: "i think we make a foundational lighting system for this
game. to bring it in line with other source ports like gzdoom, uzdoom,
eduke32, ironwail and so on. Real, genuine lighting"; "that job will actually
spawn a bunch of other jobs when i want to start this. It should also stand
up an artifact which can have screenshots and case studies of how lighting
has been achieved elsewhere, and to see what our options are in terms of the
implementation of real lighting, in the way that these retro engines have
acomplished it".

When selected:
1. **Case-study artifact** (published page, updated as the study grows):
   screenshots and how each port lights its world. GZDoom / UZDoom and
   EDuke32 still light from the maps' baked sector light, but with dynamic
   lights, light falloff and modern renderers on top (EDuke32: Polymost
   shading and visibility, Polymer per-pixel lights, the explosion world
   light-up measured in D39F); Ironwail / Quake: lightmaps, dynamic lights and
   lightstyles, a polygon engine closer to TTK's "n-space" engine (the user's
   name for it). Other relevant ports as found. Captures from the user's own
   installs on private copies where possible.
2. **TTK today:** survey the engine's own lighting tricks (vertex colours,
   light-style effects, explosion and muzzle effects, fog/depth cue) and what
   Polymorph already knows of each draw (geometry, normals, world positions).
3. **Options:** baked-from-original versus dynamic, per-vertex versus
   per-pixel, lightmaps, shadowing, cost at 180 Hz with render replay, and
   how Vanilla and Classic stay untouched.
4. **Child jobs:** a proposed breakdown (foundation, dynamic lights,
   explosion and weapon lights replacing D39F's removed flashes, level
   lighting) added to the board as Todo for the user to select.

Inputs: [note 370](documentation/370-d39-screen-flash-framework.md) D39F
"Blast research" (EDuke32 explosion light-up measurements, TTK explosion code
map); the D39 present tint. **Acceptance:** the artifact is published and
reviewed with the user, and the child jobs are on the board; no lighting code
under D41 itself.

### D41A - Light foundation

**Status: Done (user-accepted after playtest; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 5). Dependencies: D41.**
Polymorph gives every world, prop and actor corner its world position and the
OpenGL textured shader adds the frame's lights per pixel to the original vertex
colour (falloff, facing, soft shoulder). Sources: blasts (the D39F blast sound
calls, the TNT barrel's shake request) and Duke's muzzle flash; console
`light`. Per-image light lists travel to redraw workers in RenderState and to
the GPU in the replay stream. OPTIONS > DISPLAY > DYNAMIC LIGHTS (schema 71,
`--dynamic-lights`). Vanilla and Classic unchanged. Acceptance: the user
playtests blasts and gunfire in several levels at the player's frame rate,
judges look and strength, and confirms no slowdown and the option's off state.

### D41B - Weapon and projectile lights

**Status: Done (user-accepted after playtest: "the lighting looks absolutely incredible!! the brightness of the lighting is so beautiful but not intrusive. i love how the light follows the projectiles"; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) sections 7 and 10). Dependencies: D41A.**
Rounds carry light in flight (RPG 8, freezer 13, energy 15); every round's
launch (`0x800718ac`, Duke and enemies) lights a muzzle flash at its launch
point in its type's colour; wall hits (`0x8006f1b8`) light a short spark; the
energy weapon and flamethrower (no rounds) light from Duke's hand flash;
throws and bolts make no flash; blasts sized per source (barrel, dynamite,
pipe bomb or grenade, rocket, gib). Burning enemies, fire and enemy weapon
types other than the pig cop's moved to D41I.

### D41C - Duke and first person in the light

**Status: Done (user-accepted after playtest; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 8). Dependencies: D41A.** Duke's own model is drawn by the original
renderer (D12) and the first-person layer is painter-only, so neither takes
dynamic light or ambient dimming (in the D41A dimmed preview Duke stays
bright). Give Duke's model, legs, arms, weapon and held items world positions
(or a lighting path of their own) in both views. Read the D36 audit (note 257)
and D36H5 notes first; preserve their ownership boundaries. Acceptance:
matched shots of Duke beside a blast and under a test light, first and third
person, no presentation regressions.

### D41D - Level light sources

**Status: Done (user-accepted 2026-11-09: "excellent lock it in and close it out"; per-level lamp files, `lamp` editor, painted-light scan, level 0 first set, signs keep their look under light; [note 428](documentation/428-d41d-level-lights.md)). The remaining parts are D41D1. Dependencies: D41A.** Lights the levels themselves own: survey
emitters per level (neon, lamps, braziers, fires, monitors) by texture page and
CLUT and by peaks in the painted vertex light; decode the colour-index and
animated light tables (`0x800c37e4`, `ctx+0x68`) into light styles that drive
real lights; a data file of lights per level (like Duke-RT `LIGHTOVR` or
`.rtlights`) written by us, and a console light editor to place, colour and
save lights. Acceptance: a first set of levels with their lamps and signs
casting real light, user review per level.

### D41D1 - Level light sources, second part

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; `lamp signs`, `wave` style, room gate, neon signs take no light on their letters, first sets for all 21 selectable levels; [note 429](documentation/429-d41d1-signs-waves-levels.md)). Dependencies: D41D.** User (2026-11-09): signs emitting real
light, "i like that idea". Find emitters by texture page and CLUT (EXIT signs,
neon, monitors, lit panels) and give each a lamp in its own colour, the sign
itself kept crisp by the D41D shader rule; decode the animated light tables
(`ctx+0x68`) into lamp styles that follow the original flicker; first lamp
sets for more levels with the `lamp` editor. Acceptance: signs visibly light
their surroundings in level 0, then each new level reviewed by the user.

### D41E - Relit levels

**Status: Done (2026-11-10; [note 441](documentation/441-d41e-relit-levels.md)). Dependencies: D41C, D41D, D41F (all Done).** A LIGHTING option: Original
(no dynamic lights), Enhanced (D41A-D41D on the painted light) and Relit (the
painted light dimmed per level and room so placed lights, shadows and blasts
carry the scene). Readability checks level by level (remasters that relit
vertex-lit games were criticised for areas too dark to read). Acceptance: the
user plays relit levels and approves each one.

### D41F - Shadows

**Status: Done (2026-11-10; [note 439](documentation/439-d41f-shadows.md)). Selected as the D41E prerequisite. Dependencies: D41A.** Research spike, then shadow maps for
chosen lights (blasts, placed lamps) rendered from the host's world-space room
meshes, so light stops at walls; PCF filtering; cost limits at 180 Hz. GZDoom's
1D shadow maps do not fit TTK's 3D rooms. Acceptance: matched shots of a light
behind a wall and a lamp throwing shadows, measured frame cost.

### D41F1 - Hard shadow edges

**Status: Done (user-accepted 2026-11-11; [note 439 section 5](documentation/439-d41f-shadows.md)). Dependencies: D41F (Done).**
User, 2026-11-10: "there is a clear blurring on the shadows, but i think we
should use lines without the blur effect at shadow edges ... just hard shadow
lines and that's it", then "could we anti-alias those shadow edges so they
aren't jaggy?". Acceptance: shadow edges are hard lines with no soft
gradient and no stair steps; nothing else about the lights or shadows
changes.

Work log, 2026-11-10: the blur came from the D41F filtering, linear
(hardware 2 x 2) depth comparison on the shadow atlas plus 4 averaged taps
0.3 texel apart. The atlas is now nearest-filtered and the shader takes one
tap, so a receiver is either lit or shadowed. Normal offset, bias, tile size
and shadow rules are unchanged. Matched shots (UI slot 9 subway, a test light
beside the pillar, old and new builds): the pillar's floor shadow went from a
soft gradient to a crisp line,
`research/before-after/d41f1-hard-shadows/`. Placed-lamp views show no new
artefacts. Build OK; `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`
PASS; framework patch exported and checked against a clean `08ec704a`.
Limits: far from the light a 512 texel tile can show stair steps along an
edge (shadow-map texels); offscreen shots use llvmpipe, so the player's GPU
is the real check. Needs the user's playtest.

Work log, 2026-11-10 (anti-aliasing, the user's follow-up): the one-tap hard
edge showed the shadow map's texel stairs. The atlas is linear-filtered again
and the single filtered tap is sharpened by (shadow texel size / screen pixel
size), the pixel size taken from screen derivatives at the top of the shader,
so the edge is about one screen pixel wide: hard, anti-aliased, and following
the filtered contour instead of the stairs. The third `PSX_SHADOW_TUNE` value
is now the edge width in pixels (default 1). Evidence: the same pillar shot
(`pillar-floor-antialiased.png`, and old / hard / anti-aliased crops in
`pillar-floor-edge-compare.png`); with 128-texel tiles (4 times coarser than
the default) the edge stays a straight line without stairs
(`coarse-128-tile-antialiased.png`). Lamp views unchanged; tests PASS; patch
checked against a clean `08ec704a`. Limit: where a shadow texel is smaller
than a screen pixel (far from the camera) the edge is the plain filtered
value, at most one pixel soft.

Work log, 2026-11-10 (second pass): the user still saw jaggies ("i just
wanted them to match the same effect as the smooth lines of the geometry";
if not possible, take them off). One filtered tap leaves small kinks at
shadow-texel spacing wherever a texel covers several screen pixels (close
up, high resolution, light grazing a surface). Now 3 x 3 filtered taps one
texel apart (developer `PSX_SHADOW_TAPS`, 1-4) smooth the kinks out of the
contour before the sharpening, and the sharpening allows for the light's
angle to the surface (a texel stretches by 1 / cos), so grazing edges stay
one pixel wide instead of going soft. Developer view (shadowed / lit
colours) with 128-texel tiles to stand in for a close-up: 1 tap shows kinks,
3 x 3 a straight line (`edge-1tap-vs-3x3-coarse.png`). Ruled out: larger
1024 tiles and perspective-correct world positions changed nothing measured
(arcs seen in a low-light test were the light's own round pool, not the
shadow). Tests PASS; patch checked against a clean `08ec704a`. Cost: 9 taps
per shadowed light per pixel (was 4 in D41F), not yet measured on the
player's GPU. Needs the user's look; if edges still read jaggy, the user
asked for the anti-aliasing to come off.

Work log, 2026-11-10 (exact edges): user, "look at slot 8 and give it another
pass. there is still some stepping. and the corners of things are rounded
... i want it to actually match the shape of the game geometry. go deep".
Any filtered texel grid gives stairs or rounded corners, so edges now come
from the geometry itself: the shadow pass also writes the occluding
triangle's id per texel (R32UI beside the depth atlas, 100 MB); near an
edge (the offset lookup and a plain one at the ray's own texel are mixed or
disagree) the shader tests the segments from 4 rotated-grid points of the
pixel to the light against the triangles of the 4 x 4 texels around the ray
(the shadow mesh VBO read as a buffer texture). The edge has the shape of
the occluder, with true corners and 4x anti-aliasing; away from edges the
depth map still decides, and where a shadow texel is smaller than a pixel the
filtered edge is used. Developer: `PSX_SHADOW_EXACT=0` turns it off,
`PSX_SHADOW_TUNE` view 3 shows exact-test pixels yellow. Evidence at the
player's settings (Relit, internal scale 3, 1467 window, linear output; UI
slot 8, private copies): with 128-texel maps the exact edge matches the 512
one except 64 pixels, all on the spinning pickup and the HUD timer (filtered
at 128: 1087 pixels off); a test light in the alcove: the filtered jamb
shadows wobble and round off at the corner, the exact ones are straight lines
from the jamb corners (`slot8-*-jamb-filtered-vs-exact.png`). Vanilla boots
and draws. Tests PASS; patch checked against a clean `08ec704a`. Limits: a
triangle smaller than a shadow texel can be missed by the test (as by the
map); cost on the player's GPU not measured (exact test runs only in the
edge band, about 16 id fetches and a few triangle tests per edge pixel).

Work log, 2026-11-10 (fast exact edges, SHADOW EDGES): user, "the performance
is without a doubt choppy now. i have tested and compared with
psx_shadow_exact=0"; after a talk about options, "We go for A's smooth edges.
C's menu settings. and simple jaggy shadows as the fallback (also available in
menu settings)". Measured first on the player's GPU (GTX 1080 Ti, real
display, Relit, internal scale 3, 1401 window, UI slot 8 fence view with a test
light, 30 fps so the main renderer presents; new developer timer
`PSX_SHADOW_PROF=1`, GPU time of each frame's scene draws): the previous exact
edges took 16.5 ms a frame against 4.0 ms with `PSX_SHADOW_EXACT=0` or with
shadows off, far over a 180 Hz frame. Rebuilt for cost, same look:
- `shadow_vis` takes one filtered tap instead of 3 x 3 (developer
  `PSX_SHADOW_TAPS` removed). The edge band is a 2 x 2 plain lookup at the ray's
  own texel: fully lit returns lit at once, fully shadowed with the offset tap
  shadowed returns shadowed, anything else runs the exact test.
- `sh_exact` reads the ids of the 3 x 3 texels around the ray (was 4 x 4) and
  tests each triangle in the light's frame: the three planes through the light
  and an edge, and the triangle's own plane for the distance. The edge planes
  are linear in the point, so the 4 rotated-grid points of a pixel cost a few
  multiply-adds each instead of 4 full segment tests; it stops once all 4 are
  shadowed. The mesh is read as 3 RGBA32F texels a triangle (a buffer of its
  own, was 9 R32F reads).
- At most 2 exact tests per pixel (the first lights that need one; developer
  `PSX_SHADOW_XMAX`, 0-16), and a faint light (falloff x brightness 0.03 or
  less) keeps the filtered edge.
- OPTIONS > DISPLAY > SHADOW EDGES: SMOOTH (default, the exact edges) or SIMPLE
  (one shadow-map texel, stepped; no id pass, and the 100 MB id texture is only
  made the first time SMOOTH draws). Live; changing it redraws every map.
  Profile schema 73 `shadow_edges`, `run.py --shadow-edges smooth|simple`,
  `DNTTK_SHADOW_EDGES` (Vanilla: simple), console `light shadows smooth|simple`
  (session). The setting lives in the framework's shadow store
  (`psx_mod_gpu_set_shadow_edges`, not copied per render thread).
- Developer view 3 now shows yellow where any light's exact test decided and
  magenta where tests ran without anything to test.
Results, same machine and view: SMOOTH 4.1 ms, SIMPLE and shadows off 3.8 to
4.1 ms (within run noise); in the UI slot 9 pillar view (Relit) SMOOTH, SIMPLE
and off all 2.6 to 3.6 ms, and allowing 16 exact tests a pixel instead of 2
adds about 0.3 ms. Shape (offscreen): in the pillar view with 128-texel maps the
exact band runs along a straight edge (`fast-devview-exact-band-128.png`);
matched smooth and simple shots in one run (`fast-smooth-vs-simple-*.png`).
Tests: profile suite (136, new ShadowEdgesOptionTest), `ttk-near-test`,
`ttk-flash-test`, `ttk-live-env-test` PASS; patch checked against a clean
`08ec704a`; Vanilla boots and draws. Limits: GPU cost was measured at 30 fps
in two views, not in a long play session at 180 Hz; a pixel past its 2 exact
tests (several overlapping shadow edges) keeps the filtered edge for the
other lights; sub-texel triangles can still be missed.

Work log, 2026-11-11 (the 180 Hz stutter): the user's playtest stayed at 90
presents (once 60) on a 180 Hz display, the same with SHADOW EDGES SIMPLE; "go
deep ... find out what's going wrong". Found, on the player's GPU (lab at the
player's settings, `nvidia-smi` utilization, per-thread CPU, `eu-stack`
samples of the emulation thread):
- Mechanism: at 180 Hz the emulation thread presents each redraw itself
  (`gl_renderer_rp_tick` -> `SDL_GL_SwapWindow`, called from guest memory
  accesses and device service). When the GPU is saturated the NVIDIA driver
  sleeps inside the swap, the emulation stops for 0.2-1.2 s (frame-trace
  walls 439-1214 ms with under 3 ms of hooks), the audio ring runs dry and the
  D23H governor sheds to 90 or 60; every probe back to 180 stalls again and
  fails. Reproduced by walking, turning and firing in UI slot 8 (level 6).
- Cause: the shader. GPU at 180 Hz, standing in the same view: the pre-D41F1
  renderer 30% with lighting off, 38% with Relit and shadows; the D41F1
  renderer 66% with lighting off and 72% with shadows, in every mode
  including SIMPLE. Compiling the exact test out (developer
  `PSX_SHADOW_NOX=1`) gave 30% back: the cost was the code's presence, not
  its use. The textured shader's register allocation (and so occupancy for
  every pixel) is set by its heaviest path; `sh_exact` kept a local id list
  indexed in a loop.
- Fix: `sh_exact` without the list (repeated ids skipped against the last
  one, the four sample points as a bit mask): 30% lighting off, 41% Relit
  with smooth shadows. Walking slot 8 for 60 s at 180 Hz: 180 presents every
  second, no stall, no shed (before: 90, then 60, within seconds).
- Also found: the fast pass's shortcut "plain lookup fully lit means lit" put
  a bright line along the foot of walls facing away from the light (the
  plain lookup's slope bias reaches past the wall); the exact test again
  runs unless both lookups agree, as in the first exact pass. Smooth and
  simple shots now differ only along edges (1217 pixels).
- Cost now, player's level 6 view at 180 Hz: lighting off 38%, Relit 42%,
  SIMPLE 57%, SMOOTH 76% (the exact test decides about 1% of the screen
  there, mostly a floor where the two lookups disagree); the walk held 180.
- Developer: `PSX_SHADOW_NOX=1` (exact test compiled out),
  `PSX_SHADOW_XMARGIN="units fraction"` (self-hit margin, default `2 0.002`;
  tighter margins did not change the wall-foot line), `PSX_ALLOW_PTRACE=1`
  (lets `eu-stack`/`gdb -p` attach under Yama ptrace_scope 1).
Tests PASS (`ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`); patch
checked against a clean `08ec704a`; Vanilla boots and draws. Limits: SMOOTH
leaves about a quarter of the GPU free at 180 Hz in that view; busier scenes
are not measured.

Work log, 2026-11-11 (GPU breakdown, measurement only): user, "76% of the gpu
is so high". Developer `PSX_SHADER_STRIP=exact|shadows|lights`. Level 6 view at
180 Hz: lighting code out 35%; lighting off 39%, Relit 43%, SIMPLE 57%, SMOOTH
76%; exact test out 56%; shadow lookups out 44% (maps on). Scale 2: 33% / 52%
(off / SMOOTH); widescreen off: 32% / 48%. Unused code now costs 2-4 points;
the base is redraw volume, not pixels; shadow lookups (14) and the exact test
(19) follow the pixels. Note 439 section 5.

Work log, 2026-11-11 (optimization pass): user, "work through the entire stack,
autonomously ... techniques that other games use ... bring this down
considerably". A research report (decoupled shading, baked shadowmasks,
silhouette maps, irregular z-buffer and frustum-traced shadows, clustered
shading, shadow caching, register pressure on Pascal) ranked a precomputed
edge bit first. Measured on the player's GPU (level 6 view, 180 Hz, player's
settings; clock-weighted `nvidia-smi` utilization, developer GPU timestamps
`PSX_GPU_PHASES=1`, NVIDIA's assembly dump `__GL_WriteProgramObjectAssembly=1`):
- The cube-face tables `SH_F/SH_R/SH_U` (const arrays indexed at run time) were
  compiled into local memory: 18 stores at the start of every fragment, in
  every mode. Now swizzles (`sh_fru`): SMOOTH 72% -> 57%, SIMPLE 53% -> 48%.
- The exact test inline again (deferring it after the light loop kept more
  state live and cost every mode 3 points): SMOOTH 54%, SIMPLE 45%.
- Widescreen: the native-wide mirror pass scissored the full wide width, so
  every lit pixel of the centre was shaded a second time, then overwritten by
  the canonical centre copy (`wide_blit_center`). With the fast path the
  mirror now draws once per side margin, scissored to it (developer
  `PSX_WIDE_MARGINS=0` restores the full width): SMOOTH 53.5% -> 41%, SIMPLE
  45% -> 36%. Images match the full-width mirror (50 pixels in the unturned
  view offscreen; on the GPU only the animated fire differs).
- Edge texture (R8 beside the atlas, drawn with each map): pass 1 marks a
  texel whose nearest surface jumps against a neighbour (neither plane,
  followed along the other's ray, meets the other's surface; the tolerance
  grows at grazing rays), pass 2 grows the marks by 4 texels. Creases and the
  inside of many-faced occluders are no edge. SMOOTH runs the exact test only
  on marked texels, for a light bright enough there and meeting the surface
  above ~11 degrees; elsewhere one filtered tap. The shadow mesh buffer holds
  4 RGBA32F texels a triangle (the corners, the unit plane).
- Exact test: one edge plane at a time into a 4-bit mask; a hit counts only
  off the point's own plane (`sh_eps`, developer `PSX_SHADOW_XMARGIN`, default
  1.5 units + 0.05 percent of the distance), which removed a bright line at
  the foot of walls. Checked with original geometry precision too.
- Developer reference: console `light shadows reference` with
  `PSX_SHADER_REFERENCE=1` runs the exact test at every pixel. Pillar view:
  SMOOTH differs from it in 101 pixels of 921,600 (SIMPLE 883).
Result, same view, full clock: lighting off 39% -> 28%, Relit 43% -> 31%,
SIMPLE 57% -> 37%, SMOOTH 76% -> 42%. Shadows over Relit: SMOOTH 33 -> 10
points, SIMPLE 14 -> 6. A 60 s walk in slot 8 at 180 Hz held 180 presents
every second. Per redraw (GPU timestamps): scene about 0.75-1 ms with
lighting off, of it 0.45-0.7 ms in the textured draws; at 1x the scene still
takes about 0.55 ms: the remaining base is the textured PS1 draw itself.
Tests PASS (`ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`, profile
suite); patch checked against a clean `08ec704a`; Vanilla boots and draws.
Not done (larger designs the research ranked): a world-space shadow cache for
static lamps baked per level; reusing shadow results across the 180 Hz
redraws of one 30 Hz world frame; a cheaper textured PS1 shader.

Closeout, 2026-11-11. User playtest after the optimization pass: "the game is
genuinely playing better now. i didnt get a single stutter ... looking out over
the entire castle high up using the jetpack and didnt experience a single
stutter"; GPU use stayed at about 45% or less everywhere, slot 8 included,
with SMOOTH edges. Accepted. Follow-ups on the board: D23J (base cost) and
D41F2 (shadow reuse and cache).

### D41F2 - Lighter shadows: cached and reused

**Status: Needs playtest (2026-11-11). Dependencies: D41F1.** After the
D41F1 optimization pass, SMOOTH shadows add about 10 points of GPU use over
Relit in the level 6 view at 180 Hz (42% against 31%), SIMPLE about 6. The
two larger designs from the D41F1 research (ranks #2 and #4) were left out
of that pass:

- **Reuse across redraws.** The game runs at 30 Hz and Polymorph redraws each
  game frame about six times with a late camera. Shadowed lights only change
  with the game, so the shadow result per surface could be worked out once
  per game frame and reused (or reprojected) by the other redraws.
- **World-space cache for fixed lamps.** For each lamp that does not move,
  work out once which parts of the world it shadows and keep the answer, so
  the shader looks it up instead of testing the shadow map on every pixel.
  Built while the game runs from the loaded level's own geometry (at level
  load, or a lamp at a time as lamps first come into view, with no visible
  pause), so custom maps get it with no authoring step; no per-level files
  ship. Moving things (Duke, enemies, doors, lifts, breakables) keep the live
  shadows; a door or breakable that moves invalidates the lamps it touches.

Measure first (per-program GPU use and the D41F1 timers at 180 Hz), then
build whichever buys more. Acceptance: SMOOTH shadows add clearly less than
today's ~10 points in slot 8 and the level 6 view at 180 Hz, shadows match the
current output (exact reference comparison as in D41F1), no pause at level
load, and Vanilla unchanged. Related: D41G (light lists), D23J (base cost).

Work log, 2026-11-11 (Needs playtest). Measured first (offscreen, UI slot 8,
RELIT, test light, 180 Hz, clock-weighted whole-GPU work): SMOOTH 32%, SIMPLE
23%, shadows off 18%; with the exact edge test compiled out SMOOTH 24%; with
every shadow lookup compiled out 19%. SMOOTH's ~14 points are the lookups
that run (exact test about 8-9, filtered tap and edge fetch about 5), not the
code's presence. Built: **shadow classes per triangle** (framework,
`gpu_gl_renderer.c`, `sh_classify_tri`). Only the static room mesh casts
shadows, so whether a world triangle is in a light's shadow does not depend
on the camera. For each lit world triangle (models excluded) and each
shadowed light in its D41G list, a conservative test on the CPU against the
mesh (the renderer instance's own copy, chunks culled by box and by the
pyramid's faces): **wholly lit** when no mesh triangle enters the pyramid from
the light to the triangle (lifted 3 units off the receiver's plane, edge
contact within 0.25 units outside, triangles inside the map's 4-unit near
plane ignored): the shader skips that light's shadow lookup (a second 32-bit
mask, `a_ln`/`v_lnosh`); **wholly shadowed** when the three rays to the corners
all cross one mesh triangle at least 4 shadow texels inside its edges, within
the map's reach: the light leaves the triangle's list; otherwise the
per-pixel path as before. Results cached per instance (64k entries) by the
quantized world corners, slot, map stamp, mesh generation and reach; new
pairs cost at most 400 us of CPU per light update (developer
`PSX_SHADOW_CLASSIFY_US`). Developer `PSX_SHADOW_CLASSIFY=0` off, `=2`/`=3`
verify (nothing skipped; magenta where the lookup disagrees with the lit /
shadowed class).
- Classes in the slot 8 view: 27% of pairs wholly lit, 20% wholly shadowed,
  53% per pixel; a still view converges in about 10 s, then every pair is a
  cache hit and classification costs no CPU. Walking: about 75% hits, about
  7% of a core per renderer instance for new pairs.
- Correctness (verify modes, 3 frames each): UI slot 2 0 pixels disagree;
  UI slot 8 2 pixels (lit class) and 20 pixels (shadowed class) of 589,824,
  the shadowed ones one pixel wide along the contact edge of the block on the
  boards, where the per-pixel anti-aliased edge let a sliver of light in.
  Matched stills on and off look the same (only fire and water animate).
- Cost, two alternating rounds: whole-GPU SMOOTH UI slot 8 30.9 / 31.0% ->
  27.6 / 28.5%, UI slot 2 27.9 / 29.8% -> 23.5 / 23.8%; GPU timestamps of the
  textured draws per redraw (`PSX_GPU_PHASES=2`, ms x MHz, slot 8) 3038 / 3022
  -> 2685 / 2575, against about 1050 with shadows off: the shadows' own cost
  down 20-25%. SIMPLE mixed (lower in 2 of 4 pairs). 180 presents a second
  in every run, CPU unchanged once converged.
Not met yet: "clearly less than ~10 points" is only partly reached (SMOOTH
over off about 14 -> 11 points in slot 8 offscreen); the per-pixel half holds
large triangles that really straddle a shadow edge. Not built: reuse across
the redraws of one game frame and a world-space shadow cache per lamp (the
next steps if more is needed). Real-display figures wait for the user. Needs
the user's playtest.

### D41G - Light scale and cost

**Status: Needs playtest (2026-11-11). Dependencies: D41A.** Clustered light lists (Ironwail) when
lights exceed a few dozen; lighting for untextured polygons (G3/F3), a policy
for sprites and translucent faces (emissive or lit); measured cost per frame
at 120/180/240 Hz on busy levels. Acceptance: measured budgets and no
regressions on the D17 performance routes.

Work log, 2026-11-11 (first part, stopped by the driver hang). Light lists per triangle
(framework, `gpu_gl_renderer.c`): each lit triangle carries a 32-bit mask of
the lights whose sphere of reach meets its bounding box (worked out when the
triangle is queued; margin 0.2% + 2 units), and the textured shader's light
loop visits only those, in the same order (GLSL 3.30 shift loop; `findLSB`
needs 4.00 and broke the first build). Developer `PSX_LIGHT_MASK=0` lists
every light (the D41A loop); `PSX_LIGHT_STATS=1` logs lit triangles and
lights listed/kept. Evidence so far: image A/B at UI slot 2 with a test
light, mask on against off, identical (0 pixels; only the FPS overlay
differs between any two runs); in UI slot 8 a lit triangle keeps 1.6 of 10
listed lights, in UI slot 2 almost none of 5. First 180 Hz A/B (headless,
one round): RELIT + SMOOTH 20.8% -> 18.3% and ENHANCED + SMOOTH 25.9% ->
23.3% in UI slot 2; the second round was contaminated by an orphaned test
game and stopped when the NVIDIA modeset driver hung (D23K log). Not done:
clean repeated A/B, 60 s walk at 180 Hz, the other D41G items (untextured
polygons, sprites, translucent faces, budgets).

Work log, 2026-11-11 (continued; Needs playtest). All runs offscreen on the
player's GPU (SDL offscreen, `RATE=180`, the player's profile copied
read-only, a test light ahead of Duke; monitor off, and no real-display runs
after the morning's driver hang), one launch at a time with the driver log
checked before and after each. Lab `recomp/analysis/d41f2/lab.py` (local).
- Clean A/B, light lists on against `PSX_LIGHT_MASK=0`, two alternating
  rounds, whole-GPU work weighted by clock: UI slot 2 RELIT + SMOOTH 30.6 /
  32.9% -> 27.0 / 27.0%, ENHANCED + SMOOTH 29.1 / 30.6% -> 23.4 / 27.5%, RELIT
  with shadows off 25.7 / 22.6% -> 21.9 / 21.3% (a lit triangle keeps 0.07 of
  3-6 lights); UI slot 8 RELIT + SMOOTH 33.5 / 33.9% -> 32.1 / 31.8%, ENHANCED
  + SMOOTH 30.0 / 30.2% -> 28.7 / 28.4% (2.4 of 6 lights kept). Lower in 10
  of 12 pairs; the two exceptions (slot 8, shadows off) ran at 1350-1800 MHz,
  where the clock weighting is least reliable. 180 presents a second in every
  run.
- Census (`PSX_LIGHT_STATS=1`, new counters), slot 8 at 180 Hz: 154,000
  untextured (G3/F3) triangles in 5 s, none with world positions: they are
  the HUD, fades and effects, not world geometry, so there is nothing to
  light. Translucent world faces already take the light (15,000 lit in 5 s:
  the lit flag never excluded them). Sprites (rectangles) carry no world
  position and stay as drawn (emissive), which is the policy.
- Budget, UI slot 8, RELIT + SMOOTH, light lists and D41F2 classes on: a 60 s
  walk, turn and fire at 180 Hz held 177-181 presents and 58-60 game fields
  every second; redraw validation clean (0 bad). Whole-GPU 28-34% (clock
  weighted, offscreen: no window present or compositor).
- Clustered lists (Ironwail) are not needed at this scale: at most 32 lights,
  culled per triangle on the CPU, as note 444 section 4 found.
- Vanilla boots, loads a savestate and draws (no light code runs).
Not done: the D17 performance routes and real-display figures (need the
user present; see the driver hang); 120 and 240 Hz budgets. Needs the user's
playtest.

### D41M - Baked lighting for fixed lamps

**Status: Needs playtest (full build 2026-11-11, [note 446](../documentation/446-d41m-baked-lamps.md) section 6). Dependencies: D41F2, D41G.**
User, during the 2026-11-11 playtest of D41G and D41F2 in the level 3
corridor of UI slot 9: "that area now feels unplayable", "the lights also
turn on and off in this area as you go down the corridor. i feel like we need
to basically reimplement these in a much more lightweight way", and "i want us
to really focus on making this optimized to the highest standard possible".
Build new rather than tune the live path further.

Evidence (session log 2026-11-11 16:20, real display, 180 Hz): the game held
60 frames/s, about 180 presents/s and full audio, but with 12-13 lights in
view the redraw workers fell behind again and again and redraw-only turned
off for 45-50 presents (about a quarter of a second) at a time while
walking, so motion kept switching between 180 Hz and repeated frames. With
`light mode original` the long gaps stopped; with the lights back they
returned at once. `light shadows off` helped only a little (user). Each
redraw worker used 75-83% of a core there (about 40-50% on level 2) and the
GPU about 52% at full clock. The popping is the 16-lamp shadow cut and the
nearest-lights selection changing as Duke walks.
Offscreen A/B at that spot (private copy of the save, 180 Hz, slow turn,
shadows on): GPU work 26.2% with D41G light lists and D41F2 shadow classes
both off, 22.7% with light lists only, 21.8% with both; ORIGINAL 17.8%.
Worker CPU about 45% each in every case, so D41G/D41F2 did not cause the
corridor problem. The offscreen turn did not reproduce the fallbacks; they
come with walking, where the lamp set keeps changing.

Why it is expensive now: every fixed lamp is a live light with live shadow
maps, evaluated per pixel in every redraw, three redraws per game frame at
180 Hz. The level's lamps come from fixed per-level light files and only
the static room meshes cast shadows, which is the case baked lighting exists
for (Quake lightmaps and light styles; modern engines bake static lights and
keep few dynamic ones).

Plan:
1. Research and design note: lightmaps over the room meshes against a light
   volume against per-lamp stored shadow maps; how each drawn PS1 triangle
   is matched to its room-mesh surface (the host world corners and the
   static shadow mesh of D41F); texel density; memory; bake at level load
   or offline next to `recomp/assets/lights/levelNN.lights`, cached and
   invalidated when a light file changes; how Quake-style light styles keep
   TTK's animated lamp tables, signs (D41D1), broken lights (D54) and
   traffic lights (D55) working (one baked channel per animated lamp,
   scaled live).
2. Prototype on level 3 (UI slot 9 corridor), compared against the current
   live path in images and in cost.
3. Full implementation: fixed lamps baked with their wall shadows; the live
   path only for shots, blasts, muzzle flashes, Necro blasts, fire and moving
   lights, few at once; Duke, enemies and props still lit by the baked lamps
   (sampled from the bake or a light volume).

Acceptance: the UI slot 9 corridor and the D17 routes hold 180 Hz while
walking with no redraw-only fallbacks caused by lighting, at 120 and 240 Hz
as well; dynamic lighting costs at most a few GPU points over ORIGINAL in
every lit level; no lamp popping anywhere; the look matches the current
lighting in matched before/after shots (user review per level); level loads
not noticeably slower; ORIGINAL, Classic and Vanilla unchanged; measured
headless, then confirmed by the user on the real display.

Work log, 2026-11-11 (design note and level 3 prototype; needs review).
Note 446. Prototype, off by default (`DNTTK_LAMP_BAKE=1`, console `light
bake on|off`): lightmaps over the static room mesh with the live shader's
own lamp term and ray-cast walls, two-sided charts, steady lamps summed
and the four strongest varying lamps of each face as live channels, cached
per level; surfaces reach the redraw workers through a map shared before
they fork; a shared, reference-counted store and a spread-out upload for
the three renderer instances. Level 3: 12,457 faces, atlas 90 MB a
renderer at 32 units, 3.3 s first bake, 55 ms from the cache. Pictures: near
Duke baked and live match closely; down the corridor the bake is brighter
(lamps the live path cuts now light their stretch). Cost: textured draws a
redraw at 180 Hz, still corridor view, live 4.7-5.2 ms, baked 4.7-4.9 ms,
ORIGINAL 3.3 ms: no GPU win, the lighting cost is the shader's lighting
layer, not the lamps (D41G leaves 0.46 lamps a lit triangle). Walking the
corridor the redraw workers sit at 94-100% of a core in every lighting
mode, ORIGINAL included: the stutter is D23N's. Fixed on the way: the
redraws' draw-time state outgrew its 8 KB slot (16 KB, and a log line);
the bake's uniform locations. Recommendation and the design choices for
the user in note 446 section 2 and "Recommendation".

User decisions, 2026-11-11: after the playtest the user preferred the
baked look ("they just looked brighter") and, once told the bake keeps
every lamp on and stops the popping (no performance gain), chose "Finish
the bake". Design choices: lamp reach fixed per lamp (its own room's
Relit reach); shadow edges at 32 units; walls always stop baked lamps,
also with SHADOWS off. Full build: every lit level, models lit from the
bake (no shadow slots for fixed lamps), default in RELIT and ENHANCED,
matched before/after shots per level for the user, ORIGINAL/Classic/
Vanilla unchanged.

Work log, 2026-11-11 (full build, Needs playtest). With the user's choices
(lamp reach fixed per lamp, 32-unit shadow edges, walls always stop baked
lamps): the bake is on by default in Enhanced and Relit on every level
(`light bake off` / `DNTTK_LAMP_BAKE=0` keep the live lamps); with SHADOWS
off the room mesh is still built for it; the live lamps that light models
carry the reach the bake used; the emulation thread's renderer uploads the
lightmap once and the render threads adopt its textures (level 3 peak GPU
memory 1980 -> 1840 MiB); a same-level savestate load keeps the bake (it
was switched off for half a second and re-taken from the cache at every
load). Levels 0, 1, 2, 3, 6 (every level the saves reach): first bake
1.7-8.2 s in the background, then 8-67 ms from the cache; matched baked and
live shots per slot in `research/before-after/D41M/` (local) look alike.
Not built: models from a light probe (they keep the live, shadowed lamps;
an unshadowed probe would light through walls), the baked-only shader
variant. GAME_MANUAL: Baked lamps paragraph. Note 446 section 6.

User decision, 2026-11-11 (after the playtest): "we want to go for absolute
leanness ... keep the bake and fix the leaks". Fixed lamps stay baked (soft
32-unit edges, no live shadow maps for them); next step is to find and fix
the light slivers reported in the user's UI slot 10 and 7 saves (first
confirm the bake is the cause, `light bake off` against on, against the
D23O shadow classes).

Work log, 2026-11-11 (light slivers fixed, Needs playtest). Confirmed on
private copies of UI slot 10 (level 6: a lit strip up a wall corner) and UI
slot 7 (level 3: one lit floor triangle): both show with the bake on, also
with SHADOWS off (so not the D23O shadow classes, which only run with
shadows on), and vanish with `light bake off`. Two bake leaks, both fixed in
the bake itself (no new pass, no runtime cost beyond one clamp a pixel):
1. A chart covers only the part of its face a lamp reaches, but the shader
   sampled the whole face, so the rest of the face read (and filtered into)
   whichever chart lay next to it in the atlas: whole triangles and strips
   lit by unrelated lamps. Each chart's size now goes in its record's spare
   slot and the shader clamps the lookup inside the chart (its unlit
   border). Fixed both reported spots.
2. Texel samples were clamped exactly onto the face's edges, so where a
   face meets a wall or floor the visibility ray started on that surface's
   plane, whose hit was ignored: a thin lit seam along wall feet (seen in
   the sweep, UI slot 8). Samples are now pulled 2 units into the face.
Bake cache version 5 (every level rebakes once; stale caches removed).
Evidence: sweep of all 12 slots, four directions each, baked against live,
before and after; `research/before-after/D41M/slivers/` (local).
`ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS. Note 446
section 7. Remaining: the user's check of the two saves and play in general.

User playtest, 2026-11-11 (after the fix): "that patch of light has
disappeared which is excellent" (UI slot 7), "that light sliver has gone"
(UI slot 10), "the light slivers have gone" (UI slot 2's level). Other
findings of that playtest went to D41N, D41O, D50D and D23S.

### D41N - Moving live lights without shadow-map churn

**Status: Done (user-accepted 2026-11-12, closeout authorized; fixed 2026-11-12; [note 450](../documentation/450-d41n-moving-lights-d23s-console.md)). Dependencies: D41M.**
User (playtest 2026-11-11, UI slot 6, Necros underwater on the level of
UI slot 2): GPU "up to 70%" while the Necros fire many projectiles, "as soon
as the necros have been killed the GPU drops down into the 20s"; gameplay
stayed smooth. The biomask Necros of UI slot 12 also fire but the GPU stays
at 25-30%.

Measured (offscreen, 180 Hz, private card copies, 10 s each): UI slot 6
62% mean (74% peak) with 12-15 live lights; the "other" GPU phase (shadow
work) 3.7-4.0 ms a redraw; SHADOWS off 27% ("other" 0.02 ms); light mode
ORIGINAL 23%. UI slot 12: 24.5% with no live lights at all (its Necros'
projectiles carry no light), "other" 0.27 ms. So the cost is the shadow maps
of moving projectile lights (each a new cube map as it flies), not the bake.

Options for the user (look choice): projectile lights unshadowed (leanest;
Duke3D/Polymer-style glow, may light through a thin wall near the bolt);
or shadowed only for the few strongest/nearest, at a lower map size;
or a cap on shadowed moving lights per frame. Acceptance: UI slot 6 holds
within a few points of the same view without projectiles; ORIGINAL,
Classic and Vanilla unchanged.

User decision, 2026-11-12: pick the leanest option, starting with
unshadowed projectile lights, shadows only if the look clearly needs them;
then sweep other heavy fights and effects for the same churn and fix it at
the source; measure at 120, 180 and 240 Hz.

Work log, 2026-11-12 (Needs playtest). Cause confirmed: a moving light got a
new shadow slot every tick (slots are keyed by position), so each bolt in
flight needed a fresh cube map 60 times a second, drawn again in every
renderer process; out of maps, the rest went dark for the tick. Research
(note 450 section 4): Doom 3's effect-light materials are `noshadows`,
EDuke32 Polymer never shadows its point (effect) lights, Unreal does not
cache moving lights' shadows. Change: only lights that hold still cast
shadows (lamps, screens, prop lamps, blasts, test lights); rounds, bolts and
flames in flight and burning objects glow unshadowed, as muzzle flashes and
sparks always did. Session toggle `light shadows moving on|off` for
comparisons; `light shadows` reports new slots. Results, UI slot 6 with the
Necros firing: 120 Hz 48.6% -> 17.1%, 180 Hz 61.4% -> 22-24% (no bolt
lights 19-22.5%), 240 Hz 72.6% -> 30.1%, real display (180 Hz, windowed)
64.3% -> 25.6-28.3%, steady with no peaks; presents held at the rate. Sweep:
freezer 43% -> 35%, energy weapon 46% -> 42%, flamethrower on a burning
Necro 38-47% -> 24-27%, RPG 36% -> 35%; no moving maps are made any more;
blasts keep one map each (unchanged). Look: matched UI slot 6 replays and
flamethrower shots look the same (fire slightly fuller, as no flame light
goes dark), `research/before-after/D41N/` (local). ORIGINAL, Classic and
Vanilla unchanged (no lights published there). GAME_MANUAL Shadows
paragraph updated. Remaining: the user's look check; an unshadowed moving
light can light the far side of a thin wall it passes (not seen in shots).

Accepted, 2026-11-12. User after playing: "colossally huge improvement in resource usage ... a significant drop in GPU usage in regular gameplay". Closeout authorized.

### D41O - Shadow setting names after the bake

**Status: Done (user-accepted 2026-11-12, closeout authorized; [note 451](../documentation/451-d50b-d50c-saves-and-the-pit.md)). Dependencies: D41M.**
User (UI slot 8): the shadows "still exhibit the same kind of view ...
blurred at the edges, not like the sharp ones that we made beforehand",
GPU 20-24% there; "if we're gonna have to keep these then we probably think
about changing the name of the Shadows in the settings". Fixed lamps are
baked (soft 32-unit edges) whatever the setting; SHADOWS SMOOTH / SIMPLE now
only change live lights (shots, blasts, projectiles). Rename or regroup the
options so they say what they do (and GAME_MANUAL), user to choose wording.

Work log, 2026-11-12 (Needs playtest). OPTIONS > DISPLAY > SHADOW EDGES is
now **LIVE SHADOWS**, choices **SHARP** (was SMOOTH, default) and SIMPLE,
help line "BLASTS AND FLICKERING LAMPS. STEADY LAMPS ARE BAKED". The stored
profile values stay `smooth`/`simple` (no schema change, saved settings and
`--shadow-edges` keep working); the launcher help and the settings summary
say LIVE SHADOWS / sharp. Console: `light shadows sharp` (alias of smooth),
and `light shadows` reports `sharp edges`. GAME_MANUAL section rewritten:
what the setting covers (blasts and lamps that change: flickering, pulsing,
traffic lights, breakable), that baked lamps keep their soft edges.
Verified: build, `test_player_profiles` (136 tests) pass; the in-game
OPTIONS > DISPLAY page shows LIVE SHADOWS: SHARP with the new help line
(offscreen shot, local). Wording is the agent's proposal; the user may pick
other words. [Note 451](../documentation/451-d50b-d50c-saves-and-the-pit.md).

Accepted with D50B and D50C, 2026-11-12 ("lock all this in and close it
out"); the wording stands.

### D41H - Smooth light on models

**Status: Done (user-accepted after playtest: "it looks good still! ... it looks great"; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 9). Dependencies: D41C.** Per-vertex normals for actor and prop
meshes (averaged per mesh, cached by mesh) so light rounds over bodies instead
of showing flat facets; optional later: normal or specular detail. Acceptance:
matched shots of an enemy under a test light, faceted and smooth.

Work log 2026-11-08: `near_clip.cpp` `smooth_model_normals` gives every lit
actor and prop corner (Duke too) the area-weighted mean of the mesh's face
normals at that corner, within a 65 degree crease, welded across texture
seams, outward (front faces face the eye), recomputed per draw rather than
cached so animated meshes stay correct (about 4.6 us per mesh). The GL
shader interpolates them; model light is one-sided with a slight wrap,
double-sided faces and the world keep D41A's two-sided facing. Console
`light smooth on|off|default`. Matched offscreen shots (private savestate
reloaded faceted and smooth): a figure, a pig cop, Duke, the street pole and
the pistol, `research/before-after/d41h-smooth-light/`. 180 Hz 0 missed
redraws. Remaining: user playtest of the look in both views; normal or
specular detail stays optional later work.

### D41I - Fire and enemy weapon lights

**Status: Done (user-accepted after playtest: "the lighting looks spectacular"; burned-away tree fix verified in the lab; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 11). Dependencies: D41B.**
The D41B scope not done: burning enemies and fire (find the burn and flame
effect sources and give them flickering fire light), and per-type values for
enemy weapons other than the pig cop's rounds (type 2): turrets, flyers and
bosses currently take the default warm flash and spark (note 413 section 10
table). Probe their projectile types on the levels that place them (D22I
census, note 412) and set colours and sizes. Acceptance: user playtest.

Work log 2026-11-08: burning objects light while their flag word has bit
`0x800` (the bit the actor draw checks before adding flame sprites to the
joints; scanned each tick over the placement rows and Duke, nearest five);
the flamethrowers' flame records (pool types 14 and 26, flag bit 0) light
along the stream; enemy weapon types from a lab census get their own
values (warm guns for types 3, 5, 29, 32, 34 and 35, violet for the level 3
gunship's bolts 31, RPG-like rockets 24, 28 and 33). Console `light fire
on|off`. Matched stills: a burning pig cop lights the street around it;
180 Hz 0 missed redraws. Flyers and types 847, 848 and 901 did not fire in
the census and take the default warm flash. Needs the user's playtest.

Playtest round 1 (user, 2026-11-08): "When I burn an enemy the lighting
effects are just beautiful"; the gunship's projectile lights "are very well
done indeed". Bug: in UI slot 8, burning the tree in front of Duke left its
light on after the tree had burned away. Cause (private slot 8 copy): the
tree's flag word goes 0x802 while it burns and 0x804 when it is gone,
keeping 0x800 (two other props read 0x804 from the start); a burning pig
cop reads 0x803, then 0x804 when its burn ends. Fix: an object burns while
bit 0x800 is set and bit 0x4 is clear. Retest on the copy: the tree lights
only while it burns and the wall stays dark after; a burning pig cop still
lights for its whole burn (about 6 s). The user's separate report of the
mission item text overlap is D08A39.

### D41J - Upgraded weapon light colours (HiTemp Flamethrower, Laser Gatling)

**Status: Done (user-accepted 2026-11-09: "this current job at hand is fully accepted"; closeout authorized; [note 413](documentation/413-d41-lighting-case-study.md) section 12; user report 2026-11-09, D10H9 acceptance: "the upgraded flamethrower glows blue, but our lighting doesnt change from the regular flamethrower"). Dependencies: D41I.**
The upgraded (hi-temp) flamethrower's flames are drawn blue, but the D41
dynamic lights from its stream, hand flash and flames in flight use the
regular flamethrower's warm, flickering fire colour. Lead:
`recomp/src/ttk/dynamic_lights.inc` already names the hi-temp flame records
(pool type 26, besides 14) and the weapon's hand flash; find where the
upgraded weapon (slot 27) and its type 26 records take the warm values, and
give them a blue that matches the drawn flame (sample it from captures), with
the same flicker, size and cost. Burning enemies set alight by the upgraded
flamethrower are included if their drawn flames are blue too. Regular
flamethrower, other lights and Vanilla unchanged. Test: matched stills of
both flamethrowers firing at a wall and burning an enemy, first and third
person, 180 Hz (spawn the upgrade with the console). Acceptance: user
playtest.

Scope added (user, 2026-11-09): "also check out the upgraded laser gatling
gun, make sure the colors on the laser lights are right too". The Laser
Gatling (slot 28, the Gatling's upgrade) gets the same check: its muzzle
flash, rounds in flight and impact lights match the colour of its drawn
laser shots; the regular Gatling is unchanged.

**Work log (2026-11-09, Needs playtest).** `dynamic_lights.inc`. Owners:
the held byte stays 7/9 and `0x8003df40` resolves the upgrade bit to records
28/27; the hand and flame record draws pick blue sprites for record 27 and
pool type 26; the Laser Gatling launches type 27, which took the default warm
flash. Now the HiTemp hand flash and flames in flight light blue-violet
(sampled from the drawn flame), and type 27 has a red-pink muzzle, spark and
glow in flight (sampled from the bolts), same sizes, flicker and cost. Burns
lit by the HiTemp stay warm: same flag word (`0x803`) and orange flame
sprites as the regular flamethrower. Lab stills (60 Hz third person, 180 Hz
first person, before/after): the HiTemp lights the street blue-violet and the
Laser Gatling lights pink; regular weapons and burns unchanged.
`ttk-input-test`, `ttk-aim-test` pass. [Note 413](documentation/413-d41-lighting-case-study.md)
section 12. Needs the user's playtest (`dnupgrade` upgrades both).

### D41J1 - Laser Gatling light reads red

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; user, 2026-11-09: "the laser gatling gun i think reads more white in hts projectile lighting. the same way we gave the energy weapon green and the freeze thrower blue light, can we give more of a red tone to the laser gatling gun's lighting"). Dependencies: D41J.**
D41J gave the Laser Gatling's projectile type 27 a red-pink muzzle flash,
wall spark and glow in flight (`recomp/src/ttk/dynamic_lights.inc`
`shot_lights`), sampled from the drawn bolts. In play the light reads
whitish: the pink keeps a strong blue part, the street's blue-grey base
adds to it, and the fast fire stacks lights into the shader's soft shoulder,
where red tops out first and green and blue catch up. Give type 27 a clear
red light that reads as red the way the freezer reads blue and the energy
weapon green, same sizes, lives and cost. Regular Gatling, other weapons and
Vanilla unchanged. Test: matched before/after stills of the Laser Gatling
firing at a wall, third person 60 Hz and first person 180 Hz (`dnupgrade`).
Acceptance: user playtest.

**Work log (2026-11-09, Needs playtest).** Cause, measured: a single
D41J light was red-pink as intended (console test light on the street: a
pure red light adds only red), but the Laser Gatling fires fast, so two or
three muzzle flashes and several bolt glows overlap near the gun. The
Polymorph light sum's soft shoulder tops red out near 1.1 while green and
blue keep climbing, so the D41J colour (blue 0.52, green 0.26) stacked toward
pale pink-white on the floor in front of Duke; the blue-grey street pushed
it toward lavender. Fix: `recomp/src/ttk/dynamic_lights.inc` `shot_lights`
type 27 is near-pure red: muzzle 1.25 0.05 0.02, spark 1.30 0.07 0.03, in
flight 1.05 0.04 0.02; sizes, lives and cost unchanged. A first try that kept
some green and blue (1.30 0.12-0.14 0.05-0.08) still stacked to salmon. Lab
`recomp/analysis/d41j/lab.py shots` (`UP=1 WANT=7`, `NSHOT` burst frames,
`LIGHTSOFF=1` baseline) and `probe` (test-light colours), first person
180 Hz and third person 60 Hz: the floor, pole and wall in front of the gun
now light deep red; the game's own drawn muzzle sprite is unchanged.
Before/after in `research/before-after/d41j1/` (local). Regular Gatling
(type 7), other weapons and Vanilla untouched. `ttk-input-test` passes;
`ttk-aim-test` fails its line 100 assertion on the committed source as well
(not from this job). Executable
`1c6addcdf5ba8f47b84704d7d6ce59f9bd6cd39e88c3968eb0d1e582c031b153`.
[Note 413](documentation/413-d41-lighting-case-study.md) section 13. Needs
the user's playtest (`dnupgrade`).

### D41K - Props respond to dynamic light like walls do

**Status: Done (user-accepted 2026-11-09: "lock it in. close it out"; [note 434](documentation/434-d41k-props-in-light.md)). Dependencies: D41A, D41H (Done).**
Seen in D53 (note 432, revisions 2 and 3): in Polymorph a dynamic light
barely brightens props. A light over the LEVEL00 control console at
strength 25, radius 600 only greyed the desk top, while the same light
setting lights walls clearly (the TV screen light, the power sign halo).
Hypothesis to confirm, not a finding: props are already bright from their
own vertex colour, and the shader's soft shoulder (D41A) leaves little room
to add light; or the prop path weighs the light differently.

Plan: measure one light at fixed distances on a wall and on several props
(console, crates, signs) and characters, logging the shader inputs; find
where the response is lost; fix it renderer-wide (Polymorph only), so blasts,
muzzle flashes, lamps, signs and the D53 lights all light props as they light
walls. Check characters and Duke (D41C), which share the model path, and
Classic/Vanilla unchanged. Acceptance: matched before/after shots of props
under a blast and a lamp, user playtest.

**Work log (2026-11-09, Needs playtest).** Props were lit (the subway
train's polygons all carry the lit flag, positions and normals; white light
brightens it steadily), but three things held them back. (1) The D41A soft
shoulder worked per channel on painted colour plus light, so a strong
coloured light washed to grey (green strength 25 on the train: (96, 122, 98);
the D53 "greyed desk top"). (2) It started at the neutral 0.5 where prop
corners sit (128), so prop light was compressed from its first unit while
dark walls (about 0.28) took it linearly, and painted colours above 0.5
dimmed whenever any light was in the image. Club, lamps only, over lights
off: table +9, floor +36. (3) The D41D1 neon rule listed whole meshes, so a
control panel with a small screen (type 478), the bar's shelves (2, 4),
light boxes (79) and red lamps (170) took no light at all. Fix: the shader
(`lit_col`, framework patch) keeps the painted colour and soft-limits only
the added light, linear to 0.35 then toward +1.0, on its brightest channel
(hue kept); the neon scan lists face records (`near_clip_set_glow_faces`,
`Poly::glow`) so only neon faces stay unlit (level 0: 234 -> 68 faces).
After: green strength 25 on the train (47, 103, 48); club lamps on props
table +24, lamp shade +26, floor +42; a real RPG blast looks the same on
walls (frame means equal from the frame after the peak). 180 Hz: 1833
redraws, 0 misses. `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`
PASS; framework patch exported and verified against the pin. Developer:
`DNTTK_LIGHT_PROBE=1`, console `light at x y z r g b [radius]`. Stills
`research/before-after/D41K/`. Not measured in place: the D53 console (no
save near it). Needs the user's playtest (lamps, blasts, coloured lights on
props).

### D54 - Broken lights go out

**Status: Done (accepted 2026-11-10; [note 437](documentation/437-d54-broken-lights.md)).**

User, UI slot 6 (level 5 street): "In front of you is a barrel and two
traffic lights ... destroy all of them and you'll see that the lighting
effects remain ... when Breaking an object that Emits light then the light
should also get destroyed ... that's actually game wide". Scope:

- Find how TTK breaks a prop (object freed, mesh or kind swapped, a broken
  model) and every source of our light tied to a prop: D41D1 sign lamps
  (level files, `sign type N` notes), hand-placed lamps on a prop, D41L
  prop lights by model, D53 screen lights, and any other glow drawn for a
  prop.
- Bind each such light to its prop at run time; when the prop breaks, the
  light goes out (a short dying flicker, not a pop) and stays out, also
  after a savestate load in the broken state.
- Game-wide rule, not a list per level. Painted vertex light (the original's
  baked light) is reported, not changed, unless the investigation shows a
  safe way.
- Modernized Polymorph only; Vanilla and Classic unchanged.

Acceptance: in UI slot 6 the barrel and both traffic lights lose their light
when destroyed; other light-emitting props checked on other levels; user
playtest.

**Work log (2026-11-10, Needs playtest).** Investigation in a private copy of
UI slot 6 (file slot 5): the traffic lights (type 217) and the fire barrel
(type 220) broken by gunfire. At the breaking hit the prop's health (`+0x32`)
reaches 0 and bit 2 of `+0x00` is set; shortly after, the object is unlinked
from its room lists; type, place and mesh stay in memory. Our lights tied to
props: D41D1 sign lamps (fixed places in the level files, lit for good), D41L
woodburner and chandelier lights and D53 screen and power-sign lights (rescan
every 300 frames). Built: `recomp/src/ttk/prop_state.h`, a shared snapshot of
standing props (linked, kind 1, bit 2 clear, same type and place) with a room
table fingerprint so a level change does not read as every prop breaking.
Lamps take `prop TYPE x y z` entries in their notes; `lamp signs` writes them
and `lamp bind all` added them to every existing file (922 sign lamps, 21
levels, none unbound, lamp values unchanged). A light whose props all break
sputters out over 0.4 s and stays dark; loads take the state as found (no
sputter). D41L and D53 emitters check their prop each image and skip broken
props when scanning, and rescan on a savestate load; the D53 bounds writer
skips broken or unlinked monitors. Console: `lamp bind`, `unbind`, `bind
all`, `breaks off|on`. Evidence: the three lamps go out at their breaks
(log and matched shots `research/before-after/D54/`, local); a lab save made
after the breaks loads dark and the slot loads lit; flag set by hand on a
level 1 woodburner and a level 0 monitor puts out the fire light, the screen
light and the monitor's sign lamp; no false outs at level changes; build OK,
`ttk-input-test` and `ttk-near-test` pass. Remaining: user playtest in UI slot
6; the original baked (painted) light around a broken emitter stays (the
orange on the wall over the barrel), a possible follow-up.

### D55 - Traffic lights light the street in their own colour and flash

**Status: Done (accepted 2026-11-10; [note 438](documentation/438-d55-traffic-lights.md)).** Dependencies: D41D1, D54.

User, UI slot 6 (level 5 street), after D54: "I'd like traffic lights to all
be emitters, but they need to be synced to the color and the flash.
currently we have a flicker effect that is similar to the fire flicker.
essentially in the future we could put down a custom traffic light somewhere
and our colors would be correct". Scope:

- Find how TTK shows a traffic light's state (animated texture frames or
  another mechanism) and read it live.
- Every traffic light (each type, every level) gives a light whose colour and
  on/off follow the light the game is showing, frame for frame, instead of a
  fixed amber with a candle flicker.
- Driven by the prop, not by a list of places: a traffic light placed or
  spawned anywhere gets the right colours. Lights still go out when the prop
  breaks (D54).
- Modernized Polymorph with DYNAMIC LIGHTS only; Vanilla and Classic
  unchanged.

Acceptance: in UI slot 6 the street lights change colour and flash with the
traffic lights; matched shots of each state; user playtest.

**Work log (2026-11-10, Needs playtest).** TTK flashes its traffic lights by
texture animation: each model (217 on the kerb, 216 over the road) has one
animated face; its mesh's animation table (`+0x0c`) points at a running
animation record that the game copies each frame into. The script (record
`+28`) gives 2 frames of 600 ms: 217 lit amber / dark, 216 dark / lit red,
in step, so amber and red blink in turn. New lamp style `signal`
(`level_lights.inc`): each game tick the lamp reads its standing prop's
current frame and takes that frame's lit colour and its brightness against
the animation's brightest frame. The sign scan gives `signal` by itself to
any animated sign whose animation runs 100 ms frames or slower and has a
frame at most a tenth as bright as its brightest (flames run 40-66 ms frames
and keep `candle`), scores animated faces on their brightest frame (it used
the frame showing and, wrongly, the animation table of the last object
drawn, which made the red lights amber), takes flashing signs at half the
glow, and gives signal lamps a radius of at least 3000 so the over-road
lights reach the street. Level files: level 5's eight traffic light lamps
restyled (216 now red); levels 0 and 9 gain their eight (none before);
level 29's 16 pulsing neon lamps (1014, 1015, 142 ms frames) go from
`flicker` to `signal` at their old places. Evidence: signal trace (amber on
for 18 ticks, red on for the next 18), matched before/after shots
`research/before-after/D55/` (local), the traffic light's flag set by hand
puts its signal lamp out (D54), build OK, `ttk-input-test` and
`ttk-near-test` pass. Remaining: user playtest in UI slot 6; level 0, 9 and
29 lamps checked by scan and trace, not looked at in play.

### D56 - Necro blast and atomic health give light

**Status: Done (user-accepted 2026-11-10; [note 442](documentation/442-d56-necro-atomic-lights.md)).** Dependencies: D41B, D41L, D54.

User, 2026-11-10: "I am starting to feel that a couple of the in-game
objects could be light emitters too so if you load up my new slot 8, can we
make the necros blast emit. can we also make atomic health emit light too.
investigate these 2, stand up a ticket and work on them." Scope:

- The Necro's blast lights the world in its own colour while it flies, at
  the Necro when fired and where it hits, every Necro, every level.
- The atomic health pickup gives light, every copy in every level, and stops
  when it is picked up.
- Modernized Polymorph with LIGHTING on; Vanilla and Classic unchanged.

Acceptance: in UI slot 8 the blasts light the mine corridor as they come;
an atomic health glows; user playtest.

**Work log (2026-11-10, Needs playtest).** Lab `recomp/analysis/d56/`
(local), private copies of UI slots 8 and 6 (file slots 7 and 5). Necro
blast: Necros (type 50) launch projectile type 20 through the launcher
`0x800718ac` (traced: `launch enemy ... type 20`); in flight the pool record
reads flag `1` (bit 0), not the `3` of rounds, so the D41B flight lights
skipped it, and type 20 had no entry (default warm flash and spark). New
`ShotLight` field `bolt`: lit in flight on flag bit 0, with a small throb,
and a burst where the record vanishes unless a wall hit sparked there (hits
on characters skip `0x8006f1b8`; per tick, reset on level change and
savestate load). Type 20: violet launch flash (0.80 0.36 1.25, 2800, scaled
0.8 for enemies), violet flight light (0.75 0.34 1.20, 2600) and burst (0.95
0.45 1.35, 3000, 5 ticks); D39E's purple screen flash is the colour
reference. Atomic health: a pickup object (kind `+0x14` = 4) of type 581 in
the room lists. `prop_lights.cpp` scans them every 10 frames into the D41L
emitter list (room gate, budget, shadows) with a cool cyan-blue light (0.42
0.82 1.30, strength 0.85, radius 1700, 260 above its foot), a slow 1.4 s
throb, following the object's live position; a pickup no longer linked
(taken) gives no light at once. Console: `light bolts off|on`, `lamp pickups
off|on`; tuning `DNTTK_PROP_LIGHT_ATOMIC`. Evidence: slot 8 timed shots, the
left wall's mean colour from 24/21/18 (no blast near) to 53/33/53 as a blast
reaches Duke, bursts logged at the hits on Duke; matched off/on shots of
the corridor and of three spawned atoms on the level 5 street (cyan-blue pool
below them); the spawned atom picked up went dark; build OK, `ttk-input-test`
and `ttk-near-test` pass. Not checked: the player's GPU at 180 Hz, a single
atom's strength in play (the shot had three stacked), Vanilla launched (the
code is gated on Modernized and LIGHTING).

### D57 - Light emitter review: objects and items that should give light

**Status: Todo (backlog; user 2026-11-10: "do not work on this now").** Dependencies: D56.

User, 2026-11-10, after asking for D56: "there are many more, so we can
stand up a backlog job for a review of objects and game items that should be
light emitters. The first thing we should do in that backlog job is to make
an artifact of a list of items that should be emitters, ideally with
images." Scope:

- First step: survey every level's pickups (items, weapons, ammo, keys,
  power-ups), props, enemy projectiles and effects for things that look like
  they glow but give no light (glowing sprites, lit faces, energy effects),
  skipping what already gives light (D41B/D41I/D41J shots and fire, D41D/
  D41D1 lamps and signs, D41L woodburners and chandeliers, D53 screens, D55
  traffic lights, D56 Necro blast and atomic health).
- Publish the list as an artifact: per candidate an in-game image (offscreen
  lab shot, spawned where possible), object type or projectile type, levels
  seen, a proposed light colour and behaviour (steady, throb, flicker), and a
  priority. The user picks from it.
- Then: implementation in child jobs on the D56 framework (pickup lights by
  type in `prop_lights.cpp`, bolt lights in `dynamic_lights.inc`), Modernized
  Polymorph only; Vanilla and Classic unchanged.

Acceptance (first step): the published artifact, reviewed by the user.
