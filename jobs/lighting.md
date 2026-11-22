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

**Status: Done (user, 2026-11-12: "if ive closed any out, then they are accepted"; the fixed-lamp cache became the D41M bake). Dependencies: D41F1.** After the
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

**Status: Done (user, 2026-11-12: "if ive closed any out, then they are accepted"). Dependencies: D41A.** Clustered light lists (Ironwail) when
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

**Status: Done (user-accepted 2026-11-12, closeout authorized; full build 2026-11-11; acceptance pass 2026-11-12, [note 446](../documentation/446-d41m-baked-lamps.md) sections 6-8). Dependencies: D41F2, D41G.**
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

Work log, 2026-11-12 (acceptance pass, Needs playtest; note 446 section 8).
The leaks are fixed and confirmed, so this pass checked the remaining
acceptance points offscreen and removed what still showed when a level's
bake appears. Lighting cost: ORIGINAL against RELIT (baked) at 180 Hz on
levels 0, 1, 2, 3 (corridor included) and 6, alternated twice: within run
noise both ways, 180 presents/s throughout. Corridor walk (UI slot 9):
119-120, 179-180 and 238-240 presents every second at 120, 180 and 240 Hz,
no redraw-only switch-offs. First visit: the bake now uses half the logical
CPUs (at most 8, lowest priority) with the largest charts first, so the
live-to-baked switch comes sooner (level 0 8.3 -> 5.1 s, level 3 6.3 -> 3.6
s; byte-identical bake files; game pace unchanged during the bake); and the
switch no longer hitches: the first draws sampling the new atlas blocked the
emulation thread about 65 ms, gone with immutable texture storage
(`glTexStorage2D`). Also fixed: the lamp count had dropped out of the bake
key into a comment. New developer switches: `PSX_GL_DEBUG=1` (driver
messages), `PSX_BAKE_TEXSTORAGE=0`, `DNTTK_LAMP_BAKE_CACHE`,
`DNTTK_LAMP_BAKE_THREADS`. Remaining: the user's look and feel check per
level, including whether the first-visit switch is visible; real-display
figures. Not built (no measured need): model light probe, baked-only shader
variant.

Accepted 2026-11-12 on the real display. Look and smoothness in UI slots 2,
7, 9 and 10: "the highest btop reported on slot 2 was 40% so far!
impressive". First-visit switch: the level 3 bake file deleted, UI slot 9
loaded; the file was rebuilt (same size) and the switch showed no hitch: "it
worked perfectly. in fact it all worked perfectly ... and the gameplay was
perfect". Done; closeout authorized.

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

### D41P - Models lit from the bake

**Status: Done (accepted and closed 2026-11-14, [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 2; rescoped 2026-11-12). Dependencies: D41M, D41O.**
User, 2026-11-12: "im noticing that shadows on game geometry is blurred at
the edges, but on the character, it's still sharp!", and in the playtest:
"I am still noticing the smooth shadow on duke's gun." Since D41M, fixed
lamps are baked for walls and floors (soft 32-unit edges, one lookup a
pixel), while models (Duke, his first-person weapon, actors, props) are
still shaded by those lamps live, with LIVE SHADOWS edges.

Rescoped with the user the same day: the mismatch itself is fine; the
cheapest shading is preferred ("i would say we actually go for the cheapest
shading in that case possible"; "I'm completely down for trying that").
Reference: Duke Nukem 3D's Build engine, where mappers authored a shade per
floor, ceiling and wall and sprites took the shade of the sector they stood
in, with no lighting work at draw time.

Work:
1. Measure (lab, 180 Hz, UI slots 2, 8 and 9, the player's profile) what
   live fixed-lamp shading of models costs: GPU per frame with it as now
   and with fixed lamps removed from model shading.
2. If the cost matters: models take fixed-lamp light from the bake where
   they stand (sampled at the model or per vertex), like Build sprites;
   live lights (shots, blasts, flickering and moving lamps) keep shading
   models as now. If it does not: record the measurement and close.

Acceptance: the measurement recorded; if built, GPU at 180 Hz lower by the
measured amount, models brighten and darken smoothly through baked pools of
light (matched before/after shots), live lights unchanged, Vanilla and
Classic unchanged.

Work log, 2026-11-14 (Needs playtest), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 2. Measured first (GPU per
redraw, `PSX_GPU_PHASES=2`, scene ms x GPU clock, offscreen at 180,
developer `DNTTK_D41P`): baked lamps without shadow maps -5.4% / -7.4% /
-7.8% (UI slots 2, 8, 9), baked lamps left off models +2.7% (noise) / -12.1%
/ -11.6%, all live shadows off -2.2% / -11.7% / -8.8%. It matters, so built
after Quake's `R_LightPoint` (user 2026-11-14 pointed at the Quake and
Hexen II ports on the laptop): when a bake goes live its job rasterises the
level's floor faces into a 128-unit grid shared with the redraw workers
(`lamp_bake_map.cpp`); each model mesh draw (`near_clip.cpp`, the GTE
translation as origin) takes the baked surface of the nearest floor at or
below it for its corners (framework `psx_mod_gpu_host_vertex_world_normal_surface`,
a 9-word kind 6 stream record); the renderer then leaves the baked lamps out
of that triangle's light loop and adds the floor's bake (the shader reads the
floor chart from the fragment's side of the plane). Live lights still light
models directly. Developer `DNTTK_MODEL_BAKE=0` restores the live lamps.
Result, 3 interleaved runs each: UI 8 -9.9% (raw 9.94 -> 9.58 ms), UI 2
-2.7%, UI 9 within the noise; in UI 8 the lit triangles with a bake surface
198,544 -> 238,036 a 5 s window. Matched still pictures: 0.00-0.87% of
pixels changed, the weapon and arms barely by eye. The baked lamps keep
their shadow maps for the unbaked surfaces (water, fills): giving those the
bake too would drop the maps (measured -5% to -8%), a follow-up. Playtest:
models walking through lamp pools (the town, UI slot 8).

Work log, 2026-11-14 (Done). User: "we can commit and push the current work, thus closing it out. accepted as the next incremental performance hike". Closed with the build as measured
above. The follow-up (the unbaked surfaces, water and fills, from the bake
too, so baked lamps need no shadow maps at all, measured -5% to -8% of the
GPU scene work) is not part of this job; propose it as its own job when
selected.


### D41P2 - Fixed lamps only through the bake

**Status: Done (user-accepted 2026-11-14: "baked lamps look great"; closeout authorized; [note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md) section 1). Dependencies: D41P.**
User, 2026-11-14: "D41P follow-up (add it to the board as its own job
first): unbaked surfaces (water, fills) take the fixed lamps from the bake
too; then baked lamps draw no shadow maps at all. Measure first (note 470
measured -5% to -8% GPU scene work), then matched before/after pictures in UI
slots 2, 8, 9 and DUKE HILL from the slot 5 copy. Baked lamp shadows on walls
must look the same; SHARP, SIMPLE, OFF, Classic, ORIGINAL and Vanilla
otherwise unchanged."

Acceptance: what still shades the fixed lamps live gets them from the bake;
the fixed lamps then leave the live light list (no shadow map, no shader
loop); the GPU cost measured before and after on the field laptop; matched
pictures in UI slots 2, 8, 9 and DUKE HILL's town; the bake on walls
unchanged; the other modes unchanged.

Work log 2026-11-14 ([note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md)
section 1): measured first. A census of lit polygons (developer
`DNTTK_BAKE_CENSUS=1`) showed water is not lit at all and every world face
has its bake; what still took the fixed lamps live was flat prop faces (1-3%
of lit polygons) and meshes whose origin found no floor (UI 2: about 8% of
model polygons; room-attached meshes at their room's corner). Built:
`near_clip.cpp` gives those the floor under the polygon's own centre (a
flat face sent with its face normal so the shader reads the floor's lit
side), `lamp_bake_floor` searches the 8 cells around a cell with no floor
(UI 2 no-floor share 8% -> 3%, UI 8, 9, DH5 0); `lamp_bake.inc` then leaves
the fixed lamps out of the live list whenever the bake shows (no shadow
candidacy, no shader loop; note 470's `nolamps` mode is now the default;
developer `DNTTK_D41P=live|noshadow`, `DNTTK_POLY_FLOOR=0`). Counts
(`DNTTK_SHADOW_COUNT=1`, SIMPLE): fixed lamps in the list with a map before
10 / 5 / 12 / 10 (UI 8, 2, 9, DH5; DH5's 18 lights overflowed the 16 slots),
after 0. GPU per redraw, offscreen at 180, 3 interleaved runs: at the
player's OFF within the noise (scene ms 12.96/12.97, 9.57/9.47,
13.37/13.46, 9.24/9.35); at SIMPLE UI 2 -7.7%, UI 8 -6.5% (ms x clock), UI 9
and DH5 within the noise; CPU unchanged. Pictures (matched stills): UI 2 and
DH5 identical in OFF and SIMPLE, UI 9 identical in OFF; UI 8 the blood pools
on the floor now take the floor's bake (0.8% of pixels darker), UI 9 SIMPLE
1.3% of pixels slightly brighter; the baked shadows on walls unchanged.
Vanilla, Classic and ORIGINAL: no bake, unchanged by construction. Needs
playtest.

- 2026-11-14 closeout: user playtest, "baked lamps look great"; then "lock it in and close it out". Done.

### D41Q - Chandelier flames sit on their candles

**Status: Done (user-accepted 2026-11-17: "lock it in and close it out"; [note 436](../documentation/436-d41l-saloon-light.md) section D41Q). Dependencies: D41L.**
User, 2026-11-12 playtest: "the flames on the chandalier are not actually
central to the candles themselves". The chandelier's flame glows (D41L,
`prop_lights.cpp` prop glows) are drawn offset from the candle tips. Work:
measure each candle tip on the chandelier model in game space, place each
flame quad on its tip (including the chandelier's turn and sway), and check
from several angles and distances, first and third person. Acceptance: in a
matched before/after pair from the same spot, each flame sits on its candle;
no cost at 180 Hz; Vanilla and Classic unchanged.

Work log, 2026-11-17 (Needs playtest), [note 436](../documentation/436-d41l-saloon-light.md)
section D41Q. Measured: each arm plane maps texture u 96 (outer corner) to
u 127 (column); the candle is texel column u 100 (centre 100.5) with its tip
on the top row (y -212), so it stands 26.5/31 of the arm out, radius 766.
D41L placed the flames at radius 795 (u 99.5), one texel (29 units) outward.
In game (offscreen, level 1 saloon, private card copy) the flames stood on
the outer edge of each candle top or past it, worst from below. A second
cause showed from below: the flames were screen-aligned, so looking up they
stood along the screen's up instead of the candle's and their tips drifted
off the candle line. Built: the candle point is the measured fraction of
each arm (`candle_at_u`, so it follows the model), and flames are upright
billboards (`PropGlow::upright`, `near_clip.cpp` `prop_glows`): they stand
along the prop's own up axis as transformed (turn and sway included), face
the camera about it, and shorten when seen along it (not below 40%). Plain
billboards (halos, stove bloom, pickups) compute the same corners as
before. Matched before/after from one savestate spot (third person, below,
steep and side views), `research/before-after/D41Q/` (local): each flame
now sits on its candle top. Cost: one extra vertex transform per flame (12
per chandelier in view), not measured on the player's GPU. Vanilla and
Classic: the glows are gated by `visuals()` as before. Limit: a candle is a
texel strip on a flat arm plane; seen edge-on it disappears in the original
model and its flame then burns alone (both builds). Unit tests 260 OK,
`ttk-near-test` and `ttk-input-test` pass. Playtest: the saloon in level 1,
first and third person, look up at the chandelier from below and from the
side.

Work log, 2026-11-17 (Done). User: "lock it in and close it out". Closed
with the build as described above; the edge-on lone flame stays a known limit.

### D41R - Live shadows off

**Status: Done (accepted and closed 2026-11-14, [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 1). Dependencies: D41F1, D41O.**
User, 2026-11-14: "a really, really quick win for improving frame rate on
these low end systems too which id like you to add in, is the ability to
turn shadows off in polymorph". Today OPTIONS > DISPLAY > LIVE SHADOWS has
SHARP and SIMPLE only; turning shadows off exists only as the session
console command `light shadows off`. Steady lamps are baked (D41M) and keep
their wall shadows either way, so OFF only removes the shadow maps of live
lights (blasts, flickering, pulsing and breakable lamps) and the live
shadow work for models (see D41P).

Work:
1. Measure before building (no code): on the field laptop, D23Y6 lab, the
   player's settings at FRAME RATE 120, DUKE HILL from the UI slot 5 card
   copy and UI slots 2, 8 and 9, interleaved A/B of `light shadows off`
   against SIMPLE (game median and lowest tenth, presents, screen fresh,
   CPU clock, GPU busy). Note 469 section 2 has the baseline: lowest tenth
   51-54 at a steady 60 there, 57.1 with LIGHTING ORIGINAL.
2. If it helps: add OFF to LIVE SHADOWS (saved, `--shadow-edges off`,
   terminal settings), the console and menu kept in step; decide with the
   user whether the SIMPLE graphics grade uses it. Manual entry.
3. Acceptance: OFF draws no live shadow map; matched pictures show only
   the live-light shadows gone (baked lamp shadows unchanged); SHARP and
   SIMPLE unchanged; Classic, ORIGINAL and Vanilla unchanged.

Work log, 2026-11-14 (Needs playtest), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 1. Autonomous session on
the field laptop (first of D41R, D41P, D23Y8, D23Y5, D23Y7, D23N2B). Host
quiet. Measured first, `DNTTK_SHADOWS=off` (as the console `light shadows
off`) against SIMPLE, interleaved on the real display at the player's
settings at 120, 3 runs each: DUKE HILL's town (UI slot 5 copy) game lowest
tenth 55.3 -> 57.0, presents median 43 -> 59, screen fresh 74.1% -> 80.7%;
UI slots 2, 8 and 9 already near 60, within the noise. Held at a steady 60
in the town: game lowest tenth 51.7 -> 56.5, CPU clock 2.05 -> 2.20 GHz,
emulation thread 91% -> 87%. Built: LIVE SHADOWS SHARP / SIMPLE / OFF (saved
`off`, `run.py --shadow-edges off`, the terminal settings line), OFF draws no
live shadow map (`shadow_edges_off()` in `light_shadows.inc`); the console
`light shadows off` now sets OFF for the session (not saved), `light shadows
on` forces them on as before. SHARP, SIMPLE and the GRAPHICS grades
unchanged; the player's saved settings untouched. Pictures: with a test live
light its shadows go (1.0% of pixels), without it the town matches SIMPLE
(0.04%); baked lamp shadows on walls unchanged. Unit test extended
(`--shadow-edges off`, Vanilla still gets `simple`). Manual: OPTIONS >
DISPLAY > LIVE SHADOWS. Not checked by eye: the menu row (scripted keys do
not reach the pause menu offscreen). Playtest: OPTIONS > DISPLAY > LIVE
SHADOWS OFF in DUKE HILL's town.

Work log, 2026-11-14 (Done). User: "we can commit and push the current work, thus closing it out. accepted as the next incremental performance hike". Closed with the build as measured
above; the user accepted it without a further report. Remaining ideas move
to D41S (planar model shadows and glows for LIVE SHADOWS OFF).


### D41S - Quake-style model shadows and glows

**Status: Done (user-accepted 2026-11-14: "that flamethrower glow is magical", "lock it in and close it out"; [note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md) sections 2 and 2a). Dependencies: D41R, D41P.**
User, 2026-11-14: "quake alkaline can clearly do lighting and shadows on this
computer ... if we can squeeze in some basic lighting and shadow effects, im
here for it". The Quake and Hexen II ports the user plays on the field
laptop light with baked lightmaps, unshadowed dynamic lights and, for
characters, planar shadows: the model flattened onto the floor under it and
drawn dark and translucent (`r_shadows`, Alkaline at 0.6 with a ground
check); Hexen II adds glows (additive halos at flames and missiles,
`gl_glows`). With LIVE SHADOWS OFF and D41P we have the first two.

Work: (1) planar shadows for Duke (third person), enemies and props, from
the floor D41P finds, dark and translucent, no shadow maps; (2) glows at
flames and missiles if the first-person view benefits; (3) measure on the
field laptop (DUKE HILL's town, UI slots 2, 8, 9) at FRAME RATE 120 and 2x.
Acceptance: the user can choose it with LIVE SHADOWS OFF, measured within
the D23Y7 budget, Vanilla and Classic unchanged.

Work log 2026-11-14 ([note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md)
section 2; user: "Learn from Ironwail/QSS-M and Hammer of Thyrion on this
laptop. Measure the cost at my settings at 120; each effect gets its own
setting if it costs anything; matched pictures; manual entry"). Built for
LIVE SHADOWS OFF (Polymorph, ENHANCED or RELIT): MODEL SHADOWS
(`near_clip.cpp` `model_shade`): each enemy mesh flattened along a fixed
direction onto the plane of the floor under its lowest point (the D41P grid
now keeps each floor's slope), drawn half dark from a black semi-transparent
texel, depth-tested above the floor; a ground check per corner (the floor
under it on that plane within 24 units, own cell only) stops the shadow at
ledges and pits; a mesh's faces ranked a tenth of a unit apart in depth so
overlaps darken once (the PS1 mask bracket drew nothing: the GL renderer's
stencil keeps stale mask bits). GLOWS (`scene_glows`): an additive halo at
each flame and glowing shot in flight, tinted by its light. Settings:
OPTIONS > DISPLAY > MODEL SHADOWS, GLOWS (ON default), `--model-shadows`,
`--glows`, schema 75, manual. Cost at the player's settings at 120 (real
display, 3 interleaved runs): game frames unchanged (lowest tenth e.g.
three enemies 56.9 -> 57.4, DH5 56.7 -> 57.8); presents vary both ways with
the governor; GPU per redraw offscreen within the noise (three enemies
11.09 -> 11.14 ms). Pictures: the enemy's shadow on DUKE HILL's street
(0.57% of pixels darker, nothing else). Not covered: Duke (third person),
pickups and props cast none; RPG rockets have no glow; the menu rows not
driven by eye. Needs playtest.

- 2026-11-14 playtest (user): model shadows show under enemies; the flamethrower glow works; GLOWS OFF and MODEL SHADOWS OFF are kept after a relaunch; baked lamps (D41P2) confirmed. Bug: MODEL SHADOWS OFF did not remove the shadows until a relaunch, because the effects are drawn by the forked replay workers, which keep their own copy of the settings. Fixed: leaving a screen with MODEL SHADOWS, GLOWS or LIVE SHADOWS changed restarts the workers (`game_menu.cpp` leave_screen, as the control scheme already did). Built; not yet retested in the menu. Also reported: no model shadows in the pause menu's frozen frame (not investigated).

- 2026-11-14 D41S2 ([note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md) section 2a). User: "should we add a cheaper model shadow then? we clearly already have it, the cheap sprite circle under duke", then "yes proceed". MODEL SHADOWS is now SHAPE / ROUND / OFF (`--model-shadows on|round|off`, no schema change, manual updated). Duke's shadow is drawn by the game, not by a host mesh; measured in a picture as a hard-edged oval at about 55% (PS1 mode 0 over black). ROUND draws the same under each enemy: a 12-sided fan of the planar shadow's black patch on the floor's plane. The enemy is the object in s3 at the actor draw (callers 0x80033778 and 0x80034448). Its size comes from the limbs' reach in the image before, and the ground check works per wedge. A subtractive soft version was tried and dropped (black centre, red rim on sand). Pictures: a neutral oval matching Duke's. **Correction:** the first D41S cost runs (offscreen and real display) were void: the launcher overrode the lab's `LAB_DNTTK_MODEL_SHADOWS`/`LAB_DNTTK_GLOWS`, and every "off" run logged both on. Redone offscreen at 180 with the modes in the lab's profile copy (each run's settings line checked, 3 interleaved runs per mode). Raw scene ms, none / SHAPE / ROUND / glows only: DH5 + 3 enemies 11.55 / 11.58 / 11.64 / 11.43, DH5 9.50 / 9.49 / 9.62 / 9.53, UI 8 9.22 / 9.25 / 9.25 / 9.27. That is within the noise (spread 5-7%); ROUND is not measurably cheaper than SHAPE. Still to do: the real-display check at 120 (needs the player away), the menu rows by eye, the pause menu frame. Needs playtest.

- 2026-11-14 closeout: user, "lock it in and close it out. amazing". Done. The pause menu frame and the real-display cost at 120 move to D41S3.


### D41S3 - D41S follow-ups

**Status: Todo. Dependencies: D41S.**

Two items left open when D41S closed ([note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md) section 2a):

- The pause menu's frozen picture shows no model shadows (user, 2026-11-14). Find which path draws that frame and give it the shadows SHAPE or ROUND would draw in play.
- The D41S cost at the player's settings at 120 on the real display: none against SHAPE, ROUND and glows, interleaved, while the player is away. Set the modes in the lab's profile copy (`CTRL`), not with `LAB_` switches, which the launcher overrides, and check each run's settings line. Offscreen at 180, all were within the noise.

#### Work log

- 2026-11-14: added at D41S's closeout.
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

### D55A - Traffic lights no longer light the street

**Status: Done (accepted 2026-11-17; [note 502](../documentation/502-d55a-traffic-lights-in-the-bake.md)). Dependencies: D55.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14, OBEY OR DIE: traffic lights used to flash and light the
level geometry around them (D55); that seems gone. Bullet holes nearby
still respond to the flashing light, so the light itself still exists.
Suspects to check: changes since D55 (D41M baked lamps, D41O, D41P models
lit from the bake, D41R LIVE SHADOWS OFF), the user's LIGHTING / LIVE
SHADOWS / ULTRA settings, and whether D55's lights are now treated as fixed
(baked) lamps. Work: compare on ULTRA and the user's settings; restore
geometry light if it was lost; if it was turned off for performance,
measure its cost and write down the trade-off. Acceptance: walls and props
near a traffic light take its colour and flash again, matching the bullet
holes.

Lead (read 2026-11-14 in `lamp_bake.inc` and [note 446](../documentation/446-d41m-baked-lamps.md)
section 2 point 4; not tested): since D41M, flashing and flickering lamps are
baked too, as live channels. Their light shape and wall shadows are baked once
and only their brightness changes each tick (`b.channel` for any lamp that is
not steady; D55's traffic lights are `lamp_signal`, mean 0.5). A face holds at
most four such channels. Any more are baked at their average brightness and
stop flashing on that face: note 446 counts 314 such faces on level 0, "its
traffic lights and neon". Check this first. The bullet holes still flash
because they are lit live, not from the bake.

Direction by profile (user, 2026-11-14: "something like the traffic lights
should be like, respected by the live lighting etc in the more expensive
profiles, but in the budget ones, just the plain view, anything to really
tone it down you know to save resource in those budget views ... if there is
anything we can do to even fake it in the cheaper views that they flash we
should certainly do that"):

- Expensive profiles (PRETTY, ULTRA): traffic lights and other animated lamps
  are fully respected: flashing light on walls, props, models and sprites,
  as D55 built it.
- Budget profiles (SIMPLE, BALANCED, LIVE SHADOWS OFF, the field laptop): no
  live light work for them. Fake the flash as cheaply as possible, in order
  of preference: (1) the bake's channel (already a fake: a stored shape with
  a live brightness, one multiply a texel) with the four-channel limit
  handled so signals keep flashing; (2) if even that is too much, the
  game's own flashing texture plus a cheap additive glow at the lamp (Hexen
  II `gl_glows` style, see D41S), with no light on the walls.
- Since D41P2 (merged 2026-11-14) the fixed lamps, traffic lights among
  them, light only through the bake when it is shown, so the bake's
  four-channel limit now decides whether they flash at all. The D41S glows
  (additive halos at flames and rounds) are the framework for fallback (2).
- The same rule applies to every animated lamp class (neon, candles,
  strobes), not only traffic lights. Measure each step on the field laptop.

**Work log (2026-11-17, Needs playtest).** Two faults in the bake, which
lights the level's lamps since D41M (and alone since D41P2):

1. The bake's per-tick lamp levels gave a traffic light its lit colour at
   full strength and never its brightness (D55 applies the frame's
   brightness only in the live lamp loop): on walls and floors every signal
   was on all the time, amber and red together. Now every signal lamp's
   frame (colour and brightness) is read once a tick for all of them
   (`signal_state`, `level_lights.inc`) and both paths apply it.
2. The four-channel limit: the street under a set of traffic lights is
   reached by up to six varying lamps (level 0: 68 faces, 101 traffic lights
   baked steady at half; level 9: 71 faces; level 5: 11; level 3: 8). A
   chart that may need more than four now takes twice its width in the
   atlas, channels 4-5 in the copy beside it; three lamp ids a float in the
   record; the shader makes the second fetch only there. Channel lamps are
   ranked by light x swing, so beyond six the gentle ones go first. Overflows
   0 on levels 0, 3, 5, 9 (7 and 29 had none); atlases 6-28 rows taller.
   Cache key version 6 (each level bakes again once).

Evidence (offscreen, private copies; pre-fix build kept for A/B): level 5
street, building corner warmth in 16 shots, ENHANCED 19.2 every shot before,
18.2 / 5.5 in step with the amber light after; RELIT 30.7 before, 28.3 / 5.5
after; red over-road lights 2.19-2.20 before, 1.80 / 1.52 after. GPU work at
180 Hz, same view, alternated: 20.5, 21.8% before, 19.5, 21.8% after. Matched
shots `research/before-after/D55A/` (local). Tests: unit tests, ttk near,
input, flash, live-env, audio, CI runtime tests, check_repo; patch exported
and checked on a clean pin; savestates load.

Profiles: PRETTY/ULTRA (RELIT) and BALANCED (ENHANCED) flash through the
bake's channels (the cheap fake the job prefers, option 1). SIMPLE uses
LIGHTING ORIGINAL: no lamp light, the game's own flashing lenses; the D41S
style glow (option 2) is not built. Remaining: user playtest in OBEY OR DIE;
not measured on the field laptop.

**Work log (2026-11-17, later, Needs playtest).** User: "in save slot 7,
there are now random bright patches? ... also ... obey or die". My shader
decode rounded the empty lamp-id marker (2^24 - 1, + 0.5) up to 2^24, so most
faces looked "wide" and added a neighbouring chart's weights in lamp 0's
colour (pink slabs in UI slot 7, blue-ish patches on the level 5 street).
Fixed: the ids convert without + 0.5. Reproduced and checked in UI slot 7
(four views, faulty against fixed build) and on the level 5 street; the
traffic lights still flash. Patch re-exported, tests pass. Note 502.

**Accepted (2026-11-17).** User: "lock it in and close it out". Done.

### D56A - GOLD AND GUNS flying enemies' shots give light

**Status: Todo (backlog; user playthrough commentary 2026-11-14). Dependencies: D56.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: the flying enemies in GOLD AND GUNS fire projectiles
that should light their surroundings the way D56 did for the Necro blasts.
Work: identify the projectile type, give it a bolt light on the D56 framework
(`dynamic_lights.inc`) in a colour that matches it; the light follows the
shot and goes when it does. Acceptance: shots light the walls in flight, no
light left behind; measured cost at 180 Hz.

### D57A - Lava gives light (GOLD AND GUNS)

**Status: Todo (backlog; user playthrough commentary 2026-11-14). Dependencies: D57.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: lava in GOLD AND GUNS looks bright but contributes no
light. Work: lava faces as warm emitters through the existing level-light
path (D41D texture emitters, baked with D41M where fixed), cheap; maybe a slow
throb. Acceptance: nearby walls and Duke take the lava's light, within the
performance budget.

### D57B - Light for the secret DeLorean (GOLD AND GUNS)

**Status: Todo (backlog; user playthrough commentary 2026-11-14). Dependencies: D57.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: the secret DeLorean in GOLD AND GUNS is in an almost
entirely dark area, which makes the discovery underwhelming. Work: a subtle
added lamp (per-level light file) that shows the car and keeps the mood.
Acceptance: the car reads clearly in the user's playtest, still atmospheric.

### D57C - LA subway entrance globe lights give light

**Status: Done ([note 539](../documentation/539-d57c-subway-entrance-globes.md)), user-accepted 2026-11-22 ("lock it in and close it out"). Dependencies: D57.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: the round globe lights outside the subway entrances
should give light. Work: find every such fixture in the LA levels and make
them consistent lamps (per-level light files or by texture), baked where
fixed. Acceptance: each globe lights its surroundings; cost measured.

Work log, 2026-11-22 (Needs playtest):

- Found by a level 0 prop survey (136 models, textures read from VRAM): the
  globes are prop type 315, a 25-vertex sphere with a bright yellow glass
  texture, two on the posts at the top of the "Subway" stairs (room 5). All
  21 levels surveyed: they exist only on levels 0, 5 and 9, the same street
  block each time. Ruled out type 170 (red domes, already lit by D41D1 sign
  lamps) and type 195 (velvet-rope stanchions).
- `prop_lights.cpp`: `prop_light_globe`, found by model and a lit texture; a
  steady warm light at each globe (colour from its glass, strength 1.0,
  radius 3600) and a soft halo, ENHANCED and RELIT; no per-level data.
  Developer `DNTTK_PROP_LIGHT_GLOBE`, `DNTTK_PROP_GLOBE_HALO`; lab `warp x y
  z [degrees]`.
- Evidence: posts, railings and the building beside the entrance brighten
  (Relit 26.8 -> 33.8 on the building); 180 Hz pacing unchanged (p95 6 ms);
  270 local tests OK.
- Limit: the lab sees the entrance only from the stair foot (warp cannot
  stand Duke on that street); the light on the pavement waits on the user's
  look on the real display.

Closed out, 2026-11-22: user "lock it in and close it out". Done; lab trimmed to its scripts and
results. Follow-up backlog: [D57I](#d57i---street-lamp-posts-give-sodium-vapour-light-la-levels)
(the street lamp posts give sodium-vapour light).

### D57D - SOMEONE'S GONNA PAY strip club secret-room lamp gives light

**Status: Done ([note 542](../documentation/542-d57d-club-back-room-lamp.md)), user-accepted 2026-11-22 ("lock it in and close it out!! thats excellent."). Dependencies: D57.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: in the strip club of SOMEONE'S GONNA PAY, the lamp inside
the secret room should be a real light. Acceptance: the room and objects in
it take the lamp's light.

Work log, 2026-11-22 (Needs playtest):

- No level is named SOMEONE'S GONNA PAY (the club levels are 0 TIME TO KILL,
  5 OBEY OR DIE, 9 PIG FACTORY); the room was found by data, following the
  room portals out of D57J's club rooms on level 0: stage room 76 -> an
  empty passage (room 10) -> room 13, a dressing room behind the stage
  (vanity and mirror, benches, chairs, posters, a box table with the lamp on
  it). Its drawn floor is 512 above the club's.
- The lamp: prop type 228, a shaded table lamp (20 vertices, cream shade,
  never made a lamp by the D41D1 sign scan). All 21 levels surveyed: level 0
  (room 13 and room 8, a furnished upper room off the street), level 5 (room
  13) and level 9 (room 8, two in room 13).
- `prop_lights.cpp`/`.h`: `prop_light_table_lamp`, by model, game-wide: a
  steady warm tungsten light at the bulb (1.00 0.78 0.50, strength 1.2,
  radius 2600) and a soft glow on the shade; ENHANCED and RELIT; Original,
  Vanilla and Classic unchanged. Developer `DNTTK_PROP_LIGHT_TABLE_LAMP`,
  `DNTTK_PROP_TABLE_LAMP_AT`, `DNTTK_PROP_TABLE_LAMP_HALO`.
- Evidence (lab `recomp/analysis/d57d/`): facing the lamp, Relit 17.9 ->
  32.8 mean (the wall behind, the table and the ceiling take a warm pool),
  Enhanced 20.2 -> 30.7; Original unchanged; Vanilla makes no prop scan;
  180 Hz pacing unchanged (p95 6 ms), GPU about +2 to +3 points with the
  lamp filling the view; tests 270 OK. Shots `research/before-after/D57D/`
  (local).
- Limit: warp cannot stand Duke in room 13 (he drops to a floor near the
  club's height, the camera under the room's floor line), so the lab sees
  the room from just under its floor. The user's look in the room decides;
  a save state there would help. Room 8 may be D57E's apartment lamp (lit by
  the same rule; not confirmed).

Closed out, 2026-11-22: user "lock it in and close it out!! thats excellent." Done; lab
`recomp/analysis/d57d/` trimmed to its scripts and results (shots kept in
`research/before-after/D57D/`). Follow-up backlog:
[D57K](#d57k---hanging-ceiling-lamp-gives-light-la-apartment) (the hanging
ceiling lamp of the user's UI slot 12, in the same room 8 as the second table
lamp).

### D57E - LA apartment lamp gives light

**Status: Done (closed with D57K, 2026-11-22). Dependencies: D57, D57K.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: a lamp in a Los Angeles apartment-building area should
light its surroundings, following the same conventions as the other lamps.
First step: confirm which level and room with the user (or their savestate).
Acceptance: the lamp lights the room around it.

Note, 2026-11-22: level 0 room 8 is a furnished upper apartment off the LA
street. Its shaded table lamp (type 228) has given light since D57D; its
hanging ceiling lamp (type 229) is [D57K](#d57k---hanging-ceiling-lamp-gives-light-la-apartment).
Ask the user whether either is this lamp; if so, D57E closes with them.

Work log, 2026-11-22 (Needs playtest; user: "proceed to do them in order",
D57K first): D57K lights the room 8 hanging lamp. With D57D, every lamp of
the LA apartment set now gives light: the table lamp (type 228) and the
hanging lamp (type 229), in room 8 of levels 0 and 9 (the 21-level surveys
of D57D and D57K found no other copies of either outside the club rooms).
The level and room of the 2026-11-14 commentary are not on record (no video
from that day in the screencaps folder), so the user confirms on the D57K
playtest whether this is the lamp they meant; if so D57E closes with D57K,
otherwise a save state at the lamp starts it.

Closed out, 2026-11-22, with D57K: user "lock it in, and close it out." The
user went straight on to the same room's mood lighting (D57L), so the
apartment lamps of room 8 are taken as this lamp. Done.

### D57F - Time-portal machine crystal slots glow when filled (OBEY OR DIE)

**Status: Done (user-accepted 2026-11-20: "lock it in and close it out"). Dependencies: D57, D08A43.** [Note 523](../documentation/523-d57f-crystal-slot-glow.md).
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: when the crystals are put into the final time-portal
machine, the slots should glow, in the visual language of the original
crystal holders. Work: a glow and light per filled slot, in the crystal's
colour; correct after loads. Acceptance: an elegant glow per inserted
crystal, nothing on empty slots.

Work log, 2026-11-20 (Needs playtest; [note 523](../documentation/523-d57f-crystal-slot-glow.md)):
- Found: the start holders' coloured light is ours, the D41D1 sign lamp at
  each crystal (`level00/05/09.lights`), bound to the crystal by type and
  place (D54). A crystal set into the time machine (level 5 holders 7, 10,
  11, the same generic holder as its first) sits at a new place, so it had
  no lamp: dark grey holders.
- Fix (`prop_lights.cpp`, Modernized Polymorph, game-wide): a crystal seated
  in a crystal holder where no level lamp is bound to it gets a steady live
  light with the colour, reach, strength and offset of its type's level lamp
  (`level_lamp_bound`, new in `level_lights.inc`); it goes when taken.
  Start holders unchanged. Devices left alone: the first build also lit the
  D08A43 conveyor slot's crystal, which turned the red brick wall behind it
  muddy orange; rejected. Console `lamp crystals off|on` for A/B.
- Lab (fresh `level 5`, private cards, offscreen): blue and green set into
  holders 10 and 11, then red into 7 (another run): each lit in its colour,
  empty holders dark; kept through a savestate save and load; green taken
  back out with E: its light goes (trace 1 -> 0). Conveyor slot: crystal
  in, no light. Shots `research/before-after/D57F/` (local). Unit tests OK.
- Not done: no extra halo (the light on the holder is the start holders'
  look; a D41S-style halo is the next step if wanted); levels 0 and 9 not
  exercised; the user's play through OBEY OR DIE is the playtest.

Work log, 2026-11-20, revision 2 (user: the top part of the machine should
not look like an emitter; only the floor panel and filled slots; if
anything, its dead centre): the hanging top (type 192, levels 0 and 5) has
no lamp; the game lights its Gouraud faces nearly white. In Modernized
Polymorph its vertex colours are now scaled to 0.45 (dark metal under the
room's and the pad's light) and a soft cool halo breathes at its centre disc
over the pad; it gives no light. Checked at the user's UI slot 9 save (level
0's machine, green and blue in): region mean (72, 90, 126) -> (51, 70, 106);
the green and blue crystals lit there too. Note 523 revision 2.

Work log, 2026-11-20, revision 3 (user: "dont have that as an emitter.
instead, just emit light from the centre part between it's claws"; the
blood looked self-lit): the top still glowed because since D41P a model
takes the bake of the floor under it, here the pad under its blue lamp. The
top now takes no floor bake (live lamps at their real distance), keeps 0.6
of the game's light, and a live blue light with the halo sits at its centre
between the claws. UI slot 9: top bright pixels (101, 126, 184) -> (70, 84,
113); level 5 the same. The blood is that bright with all our lamps off (its
own look). A general "hanging props skip the floor bake" rule touched 33
props in a level tour, so it is not in this job. Note 523 revision 3.

Work log, 2026-11-20, revision 4 (user accepted the top: "without a doubt,
thats the one!"; the blood in UI slot 9 still glowed): revision 3's blood
reading was wrong. The game draws a blood pool (flat, additive semi mode 1)
onto black before the floor (solid crimson, Vanilla (72, 6, 4)); our D17AT
host draw came after the floor and added it: (125, 28, 27), with or without
any light. Such faces are now drawn opaque: (62, 5, 2), the game's (74, 7,
3). Game-wide, `model_depth.inc`; `DNTTK_BLOOD_OPAQUE=0` restores. A shader
emitter-rule change was tried, made no difference and was reverted
(framework byte-identical). Note 523 revision 4.

Work log, 2026-11-20, revision 5 (user, UI slot 6: with `dnstuff` crystals
the machine did not open and red did not show): D08A43's slot rule took red
from machine holder 7 to an LA switch (type 477, +0x35 1 = on, red's
number) and emptied the holder. Fixed in `crystal_slots.inc` (devices need
the slot's and holders' type flags 0x01008000; a holder-seated crystal never
moves; crystals already moved return to Duke). UI slot 6: red returned,
inserted, the portal opens; level 5 conveyor cases pass. Note 523 revision
5, note 519.

Work log, 2026-11-20, revision 6 (user: the open portal looks black):
revision 4's opaque rule also caught the portal's sparkle, an additive
glow whose palettes are mostly near black. Only palettes with no near-black
blended entries (stains such as blood) are drawn opaque now, read from the
neon scan's async VRAM snapshot (or one of our own with LIGHTING ORIGINAL)
into a table shared with the redraw workers. UI slots 6 and 9, RELIT and
ORIGINAL: portal as Vanilla, blood solid. Note 523 revision 6.

Work log, 2026-11-20, revision 7 (user: the pad under the portal sprite
flickers; "use the real display"): reproduced on the real display. The pad
stands exactly on a corner of the bake's 128-unit floor grid, and the floor
lookup (D41P, from the view with sub-unit rounding) jumped between four
floors and none as the eye moved. Lookup points snapped to 8 units: one
floor every frame, the pad steady on the real display. Note 523 revision 7.

Accepted 2026-11-20. User: "lock it in and close it out". Done.

### D57G - Iron-ring chandeliers give light (RESISTANCE IS FEUDAL)

**Status: Done (user-accepted 2026-11-17: "lock it in and close it out"; [note 436](../documentation/436-d41l-saloon-light.md) section D57G). Dependencies: D41L, D54.**
User, 2026-11-15, from their UI slot 1 save (runtime slot 0, level 7,
RESISTANCE IS FEUDAL): "make the 2 chandeliers emit light". Backlog only;
no code yet.

Investigation (CONFIRMED from the savestate's RAM, not yet seen in game):

- A private copy of UI slot 1 was taken at the request (saved 2026-11-15
  01:33 UTC), with the player's two cards, in `recomp/analysis/d57g/cards`
  (local, ignored; hash-checked identical to the player's file). Duke stands
  in room 3 at (336, 520, 13737).
- The two chandeliers are static props of **type 372** (mesh `0x8013695c`
  in this save; 51 vertices, 20 FT4 + 20 FT3; tpages 41 and 0), both in
  room 3, hung high above the hall at (3081, -2619, 15821) and
  (3081, -2619, 10767). Model: a pentagonal iron ring about 2070 across
  (model y 570..950, y down) on five chains to an apex at y about -950, with
  a candle flame at each of the five corners (the tpage-0 FT3 diamonds, model
  y about 340..560, radius about 1000). The flames are part of the model, so
  unlike D41L's chandelier no flame quads need drawing (and D41Q's offset
  problem does not arise). Render: `recomp/analysis/d57g/m372.png`; prop
  census of the level: `props.txt` (`props.py`, `mesh.py`).
- Level 7 has **5** of them: rooms 1, 3 (two), 20 and 31. The fix is by
  model, so all five light, and any other level using the mesh.
- They are not lit today: D41L's `prop_lights.cpp` scan only knows the
  saloon chandelier (type 263, 28 vertices, signature `0x000c002c`), and
  `recomp/assets/lights/level07.lights` has no lamp at them. Room 3's only
  lights are the wall torches (type 484, `candle` style, about 2500 units
  away at x about 5500).

Work: add the type-372 mesh to the prop-light scan by a vertex signature
(as `chandelier_mesh`, not per-level data), with a warm candle light at the
ring (five candle points or one light at the ring centre, whichever reads
best for the cost) and a broad fill below, tuned with the user against the
D41L chandelier; check whether these props turn or sway (place lights by
yaw, follow movement if any) and that D54 puts the light out if one is shot
down. Measure the GPU at 180 Hz in UI slot 1's hall (live prop lights, not
baked). Acceptance: from UI slot 1, both chandeliers visibly light the hall
below them and their own ring, in a matched before/after pair
(`research/before-after/D57G/`); the other three level-7 copies light too;
cost within the noise at 180 Hz; Vanilla and Classic unchanged.

Work log, 2026-11-17 (Needs playtest):

- **In** (title only, `prop_lights.cpp` / `prop_lights.h`): a new prop
  kind `prop_light_ring`, found by `ring_mesh` (51 vertices, list header
  `0x0014002c`, first vertex (-4, 948, 828), the apex (0, -955, -67)), so
  every copy on every level. Two lights each, placed by the prop's yaw: the
  candles, model (0, 450, 0) among the flames, (1.0, 0.80, 0.52), 0.9,
  radius 2400; the fill, model (0, 1800, 0), (1.0, 0.74, 0.46), 0.6, radius
  6000. Both shimmer as candles and join the D41L list (6 nearest Duke, room
  gate, D54 sputter-out). The model's own flames are a small tongue on each
  candle block, unseen from the hall floor, so a soft warm halo (D41L halo
  texture, 420 across, level 70) is drawn at each of the five candles, placed
  from the candle block's own vertices (14+8c..17+8c). Developer:
  `DNTTK_PROP_LIGHT_RING`, `DNTTK_PROP_LIGHT_RING_FILL`, `DNTTK_PROP_RING_AT`
  ("candle_y fill_y"), `DNTTK_PROP_RING_HALO` ("size level y").
- **Scan** (offscreen, private copy of UI slot 1): `level 7: 0 stoves, 0
  chandeliers, 5 rings, 10 lights`. The props do not turn or move (yaw 0,
  positions unchanged across reads).
- **Matched before/after** (same savestate, same camera steps, the HEAD
  build kept as `build-local/...d57g-before`), `research/before-after/D57G/`
  (local): `ring-near`, `ring-far`, `hall-below` (`-before`, `-after`,
  `-cmp`). RGB means: vault by the ring 45,46,46 to 84,75,62; the ring
  40,36,29 to 65,53,38; the floor below 26,18,16 to 50,37,27; the hall's back
  wall 44,37,27 to 51,43,32. Room 1's ring (a round tower; Duke moved there
  with `write_ram`, lab only) lights too: `room1-ring-off-on.png`. Rooms 20
  and 31: found by the scan (the same mesh) but not seen; moving Duke there
  put him out of the level.
- **GPU at 180 Hz** (headless, SDL offscreen on the real GPU, the player's
  controls and presentation copied, graphics custom so 180 holds; 15 s each,
  alternating; clock-weighted work): props off 16.1 / 11.6 / 15.4 % (mean
  14.4), on 15.6 / 15.9 / 13.6 % (mean 15.0). 180 presents/s held. The
  difference is inside the run-to-run spread. Excludes the compositor; not
  measured on the real display.
- **Vanilla**: no prop-light scan (no trace lines). Classic: unchanged code
  path (`visuals()` and the publish need Modernized Polymorph); not run.
- Build OK; `ttk-near-test` and `ttk-input-test` pass. No framework change.
- **Not checked**: D54 on these props (whether a ring chandelier can be shot
  down was not tried; the shared path puts out a broken prop's light).
  Tuning is the lab's first pass, to set with the user against the D41L
  saloon chandelier.

Closed out 2026-11-17: the user accepted the first-pass tuning ("lock it in
and close it out").

### D57H - Red candles on iron stands give light (UI slot 9)

**Status: Todo (backlog, reported 2026-11-18, not investigated). Dependencies: D57G.**

User, 2026-11-18 (desktop), from their UI slot 9 save: "this kind of
candle doesnt emit light (i dont think) so make it emit". Backlog only; no
code yet. Evidence: the user's UI slot 9 savestate (runtime file slot08,
level 8, saved 2026-11-18 00:44), copied (read only, the original
untouched; SHA-256 `a4c05503de06f520cd562f865df232765b10dd607d4b7368628e9fca0fa5f96c`,
identical to the player's file) to `research/savestates/d57h/` with its
`.ttk` and `.thumb` (local, never committed). Test on a copy of that file,
never the player's slot. The thumbnail shows a squat red candle with a small
flame on a tall iron stand, beside a log wall.

Work: first confirm the user's "i dont think": check whether the candle
already gives light (any `.lights` lamp or prop light near it). If not,
identify the prop (type and mesh signature, as D57G did for the ring
chandelier), and light it by model in `prop_lights.cpp` so every copy on
every level gives a small warm candle light at the flame, breaking with the
prop (D54) and correct after loads (D41W). Acceptance: the candle in the
slot 9 view lights the wall and floor around it in a matched before/after
pair, other copies on the level light too, Vanilla unchanged.

### D57I - Street lamp posts give sodium-vapour light (LA levels)

**Status: Done (2026-11-22). Dependencies: D57C.**

User, 2026-11-22, accepting D57C: "the next job is a job that needs going
into the backlog which is now to add lighting to these lamp post models. im
really thinking these have got to be sodium vapor lamps. really go for that
90s street at night feel."

- The lamps: the street lamp posts of the LA streets (one stands beside the
  "Subway" entrance in D57C's shots, `research/before-after/D57C/`, local; the
  tall pole with a lamp head at the left). First step: find the lamp post
  model(s) by a prop survey (the D57C lab `recomp/analysis/d57c/`, `props`
  and `faces` cases), every copy and every level that has them (D57C found
  levels 0, 5 and 9 share the LA street block); confirm with the user if a
  candidate is unclear.
- Look: high-pressure sodium street lighting, as on a 90s city street at
  night: a warm orange-amber light (low-pressure sodium is near
  monochromatic orange-yellow, about 589 nm; high-pressure a broader
  orange-pink-amber) thrown down from the lamp head in a pool on the
  pavement and road, a soft glow on the lamp head; research the colour and
  falloff with sources before tuning. Optionally the slow warm-up or an
  occasional failing-lamp cycle on one or two posts, only if it reads as
  atmosphere, not a fault (D41AA's lesson).
- Scope: by model, game-wide, no per-level data (D41Z/D57C pattern in
  `prop_lights.cpp`); LIGHTING ENHANCED and RELIT; Original, Vanilla and
  Classic unchanged; cost measured at 180 Hz. Test on the real street: warp
  cannot stand Duke on some LA streets (D57C), so use a save state if needed
  (ask the user).

Work log, 2026-11-22 ([note 540](../documentation/540-d57i-sodium-street-lamps.md)):

- Found by prop survey: the post is prop type 160, a 26-vertex pole with an
  arm reaching out over the road to a cobra head whose lens (two faces
  painted with one white-hot bulb) is 2820 above the street. All 21 levels:
  posts only on levels 0 (6), 5 (5) and 9 (6), the shared LA street block.
- Researched HPS with sources (Philips SON datasheets: 1900-2000 K, x 0.540
  y 0.420, Ra about 20; out of sRGB, it clips to pure orange; the usual
  rendering 255 183 76; cobra heads aim down and out, IES type II/III). The
  light is 1.00 0.60 0.20, between the two.
- `prop_lights.cpp`: `street_lamp_mesh`, kind `prop_light_street`: one
  steady light per post out over the road (810 above it, 480 beyond the
  lens; strength 1.7, radius 2400) and a soft sodium halo on the lens.
  LIGHTING ENHANCED and RELIT; Vanilla and Classic unchanged (a `--vanilla`
  run makes no prop scan). Tuned on the level 0 street from a lab save (warp
  stood Duke on the street there): an orange wash at first, then separate
  pools, then the light moved out over the road so the post's own pole takes
  about half as much.
- Relit, road under a post's arm 41.4 -> 62.6, a facade out of reach
  unchanged; 180 Hz pacing unchanged (p95 6 ms both); GPU about +1.5 points
  for all prop lights in view. Tests 270 OK (2 skipped). Waits on the
  user's look on the real display (`research/before-after/D57I/`, local).
- Closed out 2026-11-22: user "lock it in and close it out". Done. Lab
  `recomp/analysis/d57i/` trimmed to its scripts and results. Next up: D57J.

### D57J - Strip club lighting: a dark, hazy nightclub in Relit

**Status: Done (accepted 2026-11-22). Dependencies: D57I.**

User, 2026-11-22, closing D57I: "review the lighting inside the strip club.
the problem is that in relit, it's like, too bright all in one go, wheras
what we want is some kinda colored lights that resemble the dark seductive
atmosphere of a hazy nightclub. this looks more like a room with the lights
all on bright. so we need to play with lighting redone here in a way to
represent that. if we could even research some other club situations too
that would be great, because i want the aesthetic to be really good here
you know. and of course the club comes up more than once in the game so we
need to pay attention to that too."

- Test spot: the user's UI slot 12 of 2026-11-22 17:15, copied to
  `research/savestates/d57j/` (local, never committed; the lab loads it as
  slot 11 from its own card copy, `recomp/analysis/d57j/`). Level 0, Duke at
  (6829, -9720, -3461) in the club: patterned green walls, the pink neon
  ceiling tubes, the "Bootylicious" sign, the pole stages with their red
  canopy lamps, tables and the bar. Lab shots `c0-*.png` (Original,
  Enhanced, Relit).
- What lights it now (MEASURED, lab `lamp list`, level 0): four D41D1 scan
  lamps in the club rooms 75 and 76, each at strength 0.6 and radius 4886 to
  6000 (red 1.00 0.26 0.31, pink 1.00 0.48 0.51, mint 0.60 1.00 0.72,
  magenta 1.00 0.27 0.67, all at y -10547), and the Bootylicious sign lamp
  (1.00 0.35 0.85, r 3500, flicker). Broad overlapping radii plus Relit's
  lamp gain (1.7x, reach 1.35x) give an even fill over the whole room: the
  likely cause of the "lights all on" look (not yet proven; first step a
  per-lamp off/on study, and whether Relit's dimmed base light is dimmed
  enough here).
- Look to aim for (research 2026-11-22, see below): a low ambient base;
  light in layers, brighter at the bar and entrance and darkest in the
  seating; saturated coloured pools (magenta, red, violet, a cool blue
  accent) from the real fixtures, the pink neon tubes, the stage canopy
  lamps and the signs, so pools and dark gaps, not a fill; the dancers on the
  pole stages as the brightest, warm-keyed point of the room; a haze feel
  (light visible in the air round the lamps: halos or a light fog tint
  within the club only, if cheap). Optionally a slow colour drift or chase on
  a stage, only if it reads as atmosphere (the D41AA lesson).
- Scope: the club in every level it appears in (find them: levels 5 and 9
  reuse level 0's street block; survey all 21 levels for the club's rooms,
  lamps or props, by data, not by level number). Game-wide rules, no
  per-level data where it can be avoided (fix the system, keep custom
  levels possible); where the lamps come from the level lamp files, a change
  to how scan lamps are made or tuned for such rooms is preferred over
  hand-placed lamps. RELIT first, ENHANCED reviewed too; Original, Vanilla
  and Classic unchanged; cost measured at 180 Hz (GPU clock-weighted).
  Before/after shots to `research/before-after/D57J/`; the user's look on
  the real display decides.
- Research so far (a start; widen it in the job): nightclub lighting keeps
  the ambient low because ambient light flattens the effect lighting
  ([Night meet light, lighting techniques for nightlife](https://www.slideshare.net/JackKelly25/night-meet-light-discussion-of-lighting-techniques-for-nightlife));
  layered zones, brighter near the entrance, accent-led lounges
  ([nightclub lighting guide 2026](https://betopperdj.com/blogs/news/stage-lighting-for-nightclubs-2026));
  fewer ambient fixtures, more highlights on architecture, narrow pin spots
  with gels making saturated pools over the bar (OHM Lounge,
  [Live Design](https://www.livedesignonline.com/mag/meditative-space-focus-lighting-adds-energy-and-color-manhattans-ohm-lounge));
  low moody light from under banquette seating
  ([Live Design, The Heat](https://www.livedesignonline.com/architainment/heat));
  haze makes beams and colour visible in the air
  ([Showtec, nightclub lighting](https://showtec.co.uk/?p=167));
  table-dance clubs as "mood ... an aura of tension and drama"
  ([Robe case study](https://www.robe.cz/api/robe/pdfs/en/news/352));
  in game scenes the neon signs act as key lights, with fog for depth
  ([80.lv, neon alley in UE4](https://80.lv/articles/lighting-exercise-in-ue4-night-time-alleyway-with-neon-signs),
  [80.lv, cyberpunk lights and colour](https://80.lv/articles/lights-and-color-in-cyberpunk-environments)).
  Still to study: reference clubs in games and film (Duke Nukem 3D's own
  strip club in Red Light District under EDuke32/Polymer, Hitman's Berlin
  club, Cyberpunk 2077's Lizzie's Bar, film neon interiors), with sources.

Work log, 2026-11-22 ([note 541](../documentation/541-d57j-club-lighting.md)):

- Cause (MEASURED, level 0, the user's view, Relit): all lamps mean 37.1,
  level lamps off (Relit's dimmed painted light alone) 14.6, Original 20.6;
  any one of the 13 club lamps off moves it by 3.3 at most. Relit's base is
  dark enough; the wash is four broad scan lamps plus sign lamps of
  3500-4224 at every table and tube, at Relit's 1.7x strength and 1.35x
  reach.
- The club by data: its fixtures by model in `prop_lights.cpp` (neon bar
  type 193, pole canopy 117, back bar 4). Survey of all 21 levels: levels 0
  (rooms 75, 76), 5 (17, 41; the burning club) and 9 (12) only.
- Research widened (real venues, games, film; note 541): low ambient, light
  only from visible fixtures, pools with dark gaps, magenta/red with a cool
  blue accent against one warm near-skin key on each dancer, thin haze.
- Change (`recomp/src/ttk/club_lights.inc`, new; `prop_lights.cpp`/`.h`,
  `level_lights.inc`, `relit.inc`): a room holding a club fixture is a
  nightclub room: Relit ambient 30%; scan lamps out; the other lamps
  saturated at 55% reach; added (not saved) a warm amber-pink key under each
  canopy (strength 4.0, radius 2000) and a magenta or blue gel pool down each
  spot can's aim; haze glows on the tubes, can lenses (with soft shafts) and
  canopies in Enhanced and Relit. `lamp club off|on` compares; `lamp save`
  keeps the file's values.
- Evidence: Relit mean / dark share before -> after: level 0 slot 40.6 / 17%
  -> 22.7 / 64%, level 0 stages 47.0 -> 28.8, level 5 50.1 -> 31.1, level 9
  48.5 -> 29.9; Enhanced reviewed (34.2 -> 26.7 on the slot view); Original
  and Vanilla unchanged. 180 Hz pacing unchanged, GPU no measurable change;
  tests 270 OK. Shots in `research/before-after/D57J/` (local).
- Needs playtest: the user's look on the real display.

Closed out, 2026-11-22: user "lock it in and close it out. im insanely impressed. this is just unfathomably good work." Done; lab trimmed to its scripts and
results (shots kept in `research/before-after/D57J/`).

### D57K - Hanging ceiling lamp gives light (LA apartment)

**Status: Done (user-accepted 2026-11-22: "lock it in, and close it out"). Dependencies: D57D.** [Note 543](../documentation/543-d57k-hanging-ceiling-lamp.md).

User, 2026-11-22, closing D57D: "the next one is the light i have just saved
in slot 12. can you check that light, and then add it to the backlog."

Checked 2026-11-22 (lab `recomp/analysis/d57k/`, local; the user's UI slot
12 of 2026-11-22 19:49 copied to `research/savestates/d57k/`, local, never
committed; the lab loads it as slot 11 from its own card copy):

- The spot: level 0, room 8, the furnished upper apartment off the LA street
  (sofas, vanity with mirror, a TV, the D57D table lamp at (7781, -12060,
  7442)). Duke at (11136, -11769, 9863), looking up at the lamp.
- The lamp: prop type 229, a pendant hanging from the ceiling at (11969,
  -12842, 9424), 1430 from Duke: 16 vertices, 272 x 940 x 274; a cord from
  the ceiling (model y -470) down to a shallow four-sided cream shade (apex
  y 379, rim y 451, 272 across) with a white bulb hanging under it (y
  448..470, painted 248 248 248). Model first vertex (13, 470, -13) per the
  face dump; read the exact words for the matcher.
- Today it gives no light: room 8 has no lamps in `level00.lights`; ENHANCED
  is identical to ORIGINAL there (mean 118.1), RELIT only dims the painted
  light (98.8). The room's painted light is already bright, so Relit's base
  dimming and the lamp's pool decide how it reads.

Work: by model, game-wide, no per-level data (the D57C/D57D pattern in
`prop_lights.cpp`): a steady warm tungsten light at the bulb under the shade
(a pendant throws its light down and out: the floor and furniture under it
brighter, the ceiling above the shade darker), a soft glow on the bulb and
shade; LIGHTING ENHANCED and RELIT; Original, Vanilla and Classic unchanged.
First step: survey all 21 levels for the model (D57D's `lampsurvey` case);
D57D found room 8 on levels 0 and 9. Check whether Relit should dim this
room's painted light further so the lamp reads as the source. Cost measured
at 180 Hz; before/after shots to `research/before-after/D57K/`. The lab can
load the slot (warp could not stand Duke in this upper room in D57D).
Acceptance: the room under the lamp takes its light, judged by the user on
the real display.

Work log, 2026-11-22 (Needs playtest):

- Model words read from the slot (lab `verts`): first vertex (13, 470, 13)
  (the face dump's -13 above was a face corner), the last (136, 451, 137).
- Survey of all 21 levels: the model is in room 8 of level 0 and room 8 of
  level 9 (the same apartment set, at another load address); none elsewhere.
- `prop_lights.cpp`/`.h`: `pendant_mesh`, kind `prop_light_pendant`, by
  model, game-wide: one steady warm tungsten light just under the bulb
  (1.00 0.78 0.50, strength 0.5, radius 2200) and a soft glow on the bulb
  (300 across, level 40); ENHANCED and RELIT; Original, Vanilla and Classic
  unchanged. Developer `DNTTK_PROP_LIGHT_PENDANT`, `DNTTK_PROP_PENDANT_AT`,
  `DNTTK_PROP_PENDANT_HALO`. The table lamp's 1.2 bleached this brightly
  painted room (Relit 90th percentile 251); 0.7 still washed the wall; 0.5
  reads as a warm pool with the shade visible.
- Evidence from the user's slot: Relit 100.0 -> 131.4 mean (90th percentile
  119 -> 196), Enhanced 118.3 -> 139.8, Original identical; 180 Hz pacing
  p50/p95 6 ms off and on, GPU 17.4-17.6 % off vs 17.0-17.8 % on (no
  measurable cost); Vanilla no prop-light activity; tests 270 OK. Shots
  `research/before-after/D57K/` (local).
- Limits: one lab view (the slot looks up at the lamp); Relit does not dim
  the room's painted light further (a follow-up if the lamp should read as
  the only source); level 9's copy not looked at.

Closed out, 2026-11-22: user "lock it in, and close it out." Done; lab
`recomp/analysis/d57k/` trimmed to its scripts and results (shots kept in
`research/before-after/D57K/`). Follow-up backlog:
[D57L](#d57l---la-apartment-mood-lighting-hazy-and-mysterious-with-a-lights-out-glow)
(the same room's mood lighting and its light switch; user's resaved UI slot
12, next up).

### D57L - LA apartment mood lighting: hazy and mysterious, with a lights-out glow

**Status: Done (2026-11-22, user-accepted: "its perfect!!"; [note 544](../documentation/544-d57l-apartment-mood-lighting.md)). Dependencies: D57K.**

User, 2026-11-22, closing D57K: "now the next job after this one will be
what to do when you turn the lights out in that room. i have resaved slot
12 so you'll want to copy it over. First off, when the lights go off (using
the switch right in front of you), the room dims, but the lights stay
bright. i really like this aesthetic, so it made me think what if we did a
little work in this room overall where we actually make the room a bit more
seductive - not as in the nightclub, but having the big bright "downward
lighting" look for the apartment of a "lady of the evening" if you know
what i mean, is a bit too much! i feel like it should be more hazy and
mysterious in there. i dont mind for a bit of recolor of the lighting if it
suits the occasion. when you switch the lights off, we do something where
there is still somewhat of a light source somewhere because i really like
that aesthetic somehow of the small amount of light in the room when the
switch is flicked. maybe dim the desk lamp? so maybe we actually plan this
one out a bit, and also do some research on lighting that room too just
like we did with the strip club."

The spot: level 0 room 8, the furnished upper apartment off the LA street
(D57K; the same set is room 8 of level 9). The user's resaved UI slot 12 of
2026-11-22 21:05 is copied to `research/savestates/d57l/` (local, never
committed); Duke stands at the room's light switch.

Work, planned before it is built (as D57J):

1. Research (sourced, as D57J's nightclub study): how film, games and
   lighting design light a dim, intimate 90s apartment of this kind - low
   warm practicals (shaded lamps, a red or amber gel or bulb), haze, light
   kept low and off the ceiling, pools rather than a wash; and how a room
   reads after the main light is switched off (one practical left on, light
   from the window or the street, the TV).
2. Measure the room as the game draws it: its painted light in each mode,
   the lamps (D57D table lamp, D57K hanging lamp), the switch (what the game
   changes when it is flicked: the room's shade, a sector light, a flag),
   and why the prop lights stay bright after it (they do not follow the
   switch today).
3. A written plan with mock-ups or lab shots for the user to choose from
   before the build: the lit look (hazier, warmer or recoloured, less of a
   bright downward pool from the hanging lamp) and the lights-out look
   (the hanging lamp goes out with the switch; a small light stays, e.g.
   the table lamp dimmed low, the window or the TV).
4. Build by rule, not by level data (fix the system, not the level): a
   room rule as D57J's club rooms (found by its fixtures, so level 9's copy
   follows), the switch read from the game's own state; ENHANCED and RELIT;
   Original, Vanilla and Classic unchanged; cost measured at 180 Hz;
   before/after shots to `research/before-after/D57L/`.

Acceptance: the room reads hazy and mysterious when lit, and when the switch
is flicked a small light remains, judged by the user on the real display.

Work log:

- 2026-11-22 (research, measurement and plan; nothing built).
  [Note 544](../documentation/544-d57l-apartment-mood-lighting.md). Lab
  `recomp/analysis/d57l/` (local), from the user's slot 12 of 21:05 (copied
  to `research/savestates/d57l/`); player saves and settings unchanged after
  every run. Measured: the switch sets level flag `0x400` and changes room
  8's light colour (room record +0x54) `60 60 60` -> `10 10 10`; the two prop
  lamps (D57D table lamp, D57K hanging lamp) never read it, hence the dark
  room with both lamps at full. Survey of all 21 levels: only level 0 room 8
  and level 9 room 25 (dark from the load) are not `80 80 80`. Room 8's
  street window is a portal into an outdoor room (room 3). Relit counts the
  room as unlit (80%). Sourced research (Body Heat, Eyes Wide Shut, Lost
  Highway, Drive, Blade Runner in American Cinematographer; the DOE tungsten
  dimming curve; Level Design Book, Robert Yang, The Dark Mod). Mock-ups
  with developer tunes and session lamps (`research/before-after/D57L/plan/`,
  local): lit L3 (warm) and L4 (rose), lights-out D3 (table lamp low and
  amber plus a faint street glow) and D4 (the table lamp alone). Plan: lamps
  follow their room's light colour game-wide (ceiling fixtures go out,
  plug-in lamps turn low and warmer), an apartment rule found by the hanging
  lamp (Relit ambient 45%, the chosen looks, window glow at portals to
  outdoor rooms, a thin glow round the shades). Waits for the user's choice.
- 2026-11-22 (build). User: "its gotta be the l4 rose, and i think D3".
  Game-wide: prop lamps read their room's light colour each image; under
  0x40 (switched off) electric fixtures go out and a table lamp turns low
  and amber (D3). Apartment rooms (holding the hanging lamp): the rose
  hanging lamp and warmer table lamp (L4), Relit ambient 45%, a sodium glow
  inside each portal into an outdoor room (level 0's window; strength 0.35,
  radius 1400 to match D3 once centred in the opening). Lamp halos follow
  their own prop (the draw's registers give the record; none missed).
  `lamp apartment off|on`. Relit room views lit 82/78/89 -> 56/59/64,
  switched off 28/39/37 -> 15/31/28; Original identical; the club's pass
  unchanged; 180 Hz pacing p95 6 ms, GPU within noise; Vanilla no activity;
  tests 270 OK. Shots `research/before-after/D57L/` (local). Level 9's copy
  takes the rule without a window glow and was not seen in the lab. Needs
  the user's look on the real display.
- 2026-11-22 (playtest 1). User: "the only bit that doesnt look quite right
  is the little bit of haze around the hanging lamp when its switched off ...
  this is wonderful work." Lab, the resaved slot: no halo of ours round the
  switched-off lamp (pixel-identical with prop lights on/off); the save
  thumbnail's glow also appears with the fix, so it is not the shown image.
  Fixed a real gap: the halo's look was reset by each prop rescan and only
  restored by the light pass, so the old warm halo could show (pause/save
  screens, the image after a rescan); the halo now reads its room's light
  colour itself. Waits for the user's look with this build.
- 2026-11-22 (playtest 2). User: "can we bring the halo back when the light
  is on, and make the halo disappear when the light is off? it looked
  awesome". The lit rose halo (level 28) sank into the rose wall; now a
  warm-white bulb glow, 300 across at level 60 (lab A/B/C: the D57K halo
  faint, pale rose washed out). Off: none (checked 3x bright, live flick).

Closed out, 2026-11-22: user "its perfect!!", then "lock in and close out the work at
hand". Done. The user's follow-up report (saturated reds glowy in dim light,
Duke's shirt and the medkit, the strip club too) is the Relit emitter test,
not this job: new job D41AC. Lab trimmed to its scripts and results.

### D41T - Explosions light cheaply: one light per chain, no blast shadows

**Status: Done (2026-11-14). Dependencies: D50E.**
User, 2026-11-14, after the D50E barrel crash: "we can merge every barrel
chain in the game ... there's a barrel section in miner 69er too so maybe we
can do like a full optimization ... i think the cheapest option overall is
gonna be the best one here ... i really hope we can make this effect simple
and cheap to do. and maybe open our eyes to similar issues within the game".

How it works today (read 2026-11-14 from `src/ttk/dynamic_lights.inc` and
`light_shadows.inc`; nothing changed). Every explosion goes through
`light_blast_at`: TNT barrels through the shake hook (`lights_shake`, ra
`0x800973e0`); dynamite, pipe bombs and grenades, rounds and gibs through the
blast sound hooks (`lights_sound`, `lights_sound_water`). The vest pig is
probably lit as a gib blast (same routine as `0x8006413c`; not verified).
Each blast:

- adds its own light (`light_srcs[24]`; a barrel 9000 reach, 20 tics);
- is a shadow caster, so it claims a new cube map (up to
  `shadow_new_per_tick` = 8 a tick, 16 slots). Gib bursts claim one too;
- calls `lights_publish()` at once, so the whole light view is rebuilt per
  event: lamp search and sort, props, TVs, `shadows_assign`, the bake marks.
  A 20-barrel chain rebuilds it 20 times in one tick.

What goes wrong in a chain:

- casters past the maps go black (`l.r=l.g=l.b=0`, lines 211 and 235) but
  keep their slot and full reach. A faded caster in the top 16 can reach
  colour 0 and still gets a map drawn. The D50E core showed about 20 barrel
  lights in the room, most at colour 0;
- the table fills first come: lamps up to 32 - 8, then up to 8 fire lights
  (burning objects, flames) take the 8 slots kept for blasts and flashes, so
  a chain that sets things burning can leave explosions with no light at all;
- `light_add` replaces the oldest entry when full, so in a firefight the
  long-lived blasts are pushed out first by muzzle flashes and sparks;
- `lights_flight` takes the first 8 rounds in flight by pool index, not the
  nearest.

How other engines do it (looked at 2026-11-14):

- EDuke32 Polymer (source on this machine): each EXPLOSION2 gets a point light
  for about 2 tics (`lightcount = 2`). Point lights get no shadow maps (only
  spotlights do, `polymer.cpp` 6213). Low-priority lights are skipped, and a
  surface draws at most `pr_maxlightpasses` lights (5 by default).
- Quake (the engine under Alkaline, id's released source): one dlight per
  explosion (radius 350, gone in 0.5 s, decaying). No shadows. 32 slots; when
  full, a new light overwrites an old one.
- Hexen II: `gl_glows` / `gl_missile_glows`, cheap additive halos, no lights.

Work, cheapest first (Modernized Polymorph only; Vanilla and Classic
unchanged):

1. One light per chain: a blast near a live, still-bright blast (a reach and
   tic window, to be measured) feeds that light instead of adding one. It
   moves toward the newer blast, brightens a little (capped) and lives a
   little longer. Every explosive type shares it.
2. Blasts cast no shadows: a short flash, as in Polymer and Quake. No cube
   maps for blasts or gibs.
3. A light that is dark this tick (colour 0, envelope near 0) is not published
   and sets no mask bits.
4. When over 32, keep lights by importance (the existing `shadows_assign`
   score: brightness, reach, distance), not table order. Blasts and fire
   share the reserved slots by score.
5. Rebuild the light view once a tick, not on every event.

Since D41P2 (field laptop, merged 2026-11-14): with a bake shown, the fixed
lamps leave the live light list (light only through the bake), so the table
has more room for blasts. A chain can still fill it, because blasts
(`light_srcs[24]`), fire and rounds in flight add up to more than 32.
Re-measure the table during a chain before step 4.

Acceptance: before/after matched shots of one barrel and of a chain (the
user's RESISTANCE IS FEUDAL room; MINER 69ER's barrel section); measured at
180 Hz and on the field laptop during a chain (no frame spike, fewer
lights); the user's playtest.

Work log:

- 2026-11-14 ([note 472](../documentation/472-d50e-d41t-barrel-chains.md) sections 2-4). Barrels spawn in the lab: types with flag bit `0x10000` go to the barrel blast (level 7 type 944, level 2 type 299). Measured before: 11 bunched barrels gave 11 lights, 11 casters, 10 maps and 11 rebuilds in one tick; the blast reserve was counted before D41P2 took the baked lamps out; casters past the maps stayed listed dark. Built: one light per chain for every explosive (within 4500 units and 12 tics; moves 65% toward the newer blast, +12% brightness per blast up to 1.6x, +5% reach up to 1.35x, life +2 tics up to 32); no blast shadows; dark lights leave the list; candidates staged in 64 entries and cut to the 32 most important (the D41F score); nearest 8 rounds in flight; the full source list replaces its dimmest; one rebuild a tick (dirty flag, rebuilt at the image's capture when needed). After: 1 chain light, 0 blast maps, 1-2 rebuilds a tick. 180 Hz offscreen, 12 bunched barrels, 3 interleaved runs (user watching YouTube meanwhile): GPU rise over the chain +1.7 to +3.4 points against +8.6 to +10.6, peak 62-77% against 88-93%, 0 misses either way. Look tuned on ULTRA (one fireball travelling with the chain instead of the corridor flattened white), checked on BALANCED; SIMPLE has no dynamic lights. Pictures: `research/before-after/D41T/` (local). Remaining: the user's playtest (RESISTANCE IS FEUDAL room, MINER 69ER) and the field laptop measurement. Needs playtest.
- 2026-11-14: accepted and closed. User: "lock it in and close it out! it 100% works". The field laptop measurement was not made; the user accepted on play. Follow-up: D41U (explosion chain audio).

### D41U - Explosion chains sound as one blast

**Status: Done (2026-11-14). Dependencies: D41T.**
User, 2026-11-14, closing D41T: "make sure that we can consolidate the audio
too. the same way we have consolidated the explosions there".

How D41T works (so the audio can follow it): nothing is worked out ahead of
time. The game decides which barrels chain while it runs (a blast damages what
is near it, and a barrel that dies goes off a few tics later). D41T only
watches: when a blast happens within 4500 units of a blast light still in its
first 12 tics, it feeds that light instead of adding one. So a row of barrels
that all go off becomes one light, and a barrel too far away to be set off
simply never joins (its own light starts if it is shot later).

Known (notes 370 and 396): the barrel blast routine `0x80097090` plays no
blast sound of its own; dynamite, pipe bombs, grenades, rockets and gib bursts
call the sound routine `0x8006b73c` (`0x8006b270` under water) with the
blast's position. Where the barrel's bang comes from is not known yet.

Work (Modernized only; Vanilla unchanged):

1. Find every explosion's sound call, the barrel's included (a sound trace
   while spawned barrels go off, `spawn 944` in level 7, `spawn 299` in
   level 2), and count the voices a chain starts today and what they cost
   (SPU voices taken, other sounds cut off).
2. The same rule as the light: a blast sound within the chain's reach and
   window while one is still loud is not started again; the playing voice
   carries the chain (perhaps louder, capped), so the chain is one big bang
   and a rumble, not a stack.
3. Keep what makes a chain read: the first bang at once (no delay), and a
   barrel far enough away to be its own blast keeps its own sound.

Acceptance: a measured voice count before and after on a barrel chain (level
7 and MINER 69ER), no other sound cut off during a chain, and the user's
playtest.

Work log:

- 2026-11-14 ([note 473](../documentation/473-d41u-blast-chain-audio.md)). Found the barrel's bang: the object death-sound dispatcher `0x80098a88` (a switch on type) plays the type's sound, `0x8006b270(0x6002, barrel+4, 0x800)` for type 944 and `0x6001` for TNT 299; a barrel is a type with flag bit `0x10000`, so no per-level data. Measured before: one voice per barrel (8 barrels 8 voices, 5 started in one tick; 20 bunched took all 24 SPU voices, which the game never steals, so other sounds could not start; the mix clipped, -0.5 dBFS). Built (`blast_audio.inc`, Modernized, not Classic): the first blast plays at once; barrels within 4500 units of a chain whose bang began under 15 tics ago are refused and swell and move its voice; the chain's next stage adds one deeper rumble (pitch -2, about 3 semitones down); a chain still going after 15 tics bangs again with the old voices ducked to a tail; rockets, grenades, pipe bombs and dynamite join only when they go off together (RPG blasts 13-14 tics apart each bang). After: 1 barrel unchanged; 8 barrels 3 voices over the chain, 20 bunched 2, MINER 69ER 2; peak 6-8 SPU voices in use; no clipping; held RPG fire identical to before; `chainsound off` reproduces the original. Native group D41U passes (the suite now links; it stops later at older stale groups). Recordings and waveforms in `research/before-after/D41U/` (local). Trade-off for the playtest: the chain's peak is about one close barrel's (one voice is at the SPU ceiling), longer and with a second boom; knobs in note 473 section 5. Not exercised in game: dynamite, pipe bombs, under water. Needs playtest.
- 2026-11-14: accepted and closed. User: "working beautifully. i'd give it a 10/10 in fact. lock it in and commit".

### D41V - Lighting with DRAW DISTANCE ORIGINAL

**Status: Todo (user 2026-11-14: "grey them out today, but add it to the backlog as a fix for another time"). Dependencies: D41A, D17AR4.**
With DRAW DISTANCE ORIGINAL none of the lighting effects show: LIGHTING
ENHANCED and RELIT, the baked lamp light, LIVE SHADOWS, MODEL SHADOWS and
GLOWS. Measured 2026-11-14 (UI slot 2, FRAME RATE 120, matched shots): Relit
and Enhanced at ORIGINAL are pixel-identical to LIGHTING OFF. Cause (CODE):
the lights are attached to each image in the host world pass
(`host_visibility.inc` `host_vis_walk` -> `lights_upload`), which
`draw_distance.inc` `draw_distance_apply` reaches only with EXTENDED; at
ORIGINAL the pass never runs, so the lights are never uploaded and Relit's
camera room stays unknown. Interim (built 2026-11-14): OPTIONS greys out
LIGHTING, LIVE SHADOWS, MODEL SHADOWS and GLOWS while DRAW DISTANCE is
ORIGINAL ("NEEDS DRAW DISTANCE EXTENDED, IN ADVANCED"). Fix: run the host
world pass (or at least the light upload and Relit's room) with the original
view limits, so ORIGINAL keeps the game's own range and gets every lighting
effect; then remove the grey-out. Acceptance: at ORIGINAL, Enhanced and
Relit draw as at EXTENDED within the original range (matched pictures),
the original draw distance unchanged, Vanilla unchanged, the user's check.


### D41W - Lights arrive late after a load

**Status: Done (2026-11-17, user-accepted). Dependencies: none.**

User, 2026-11-16 (field laptop): "when the game loads, not only do lights
take a while to load in" and loading felt slower. At the time the player's
LIGHTING was RELIT, whose bake atlas is about 60% larger (DUKE HILL level 3:
4096x2746, 90 MB against 4096x1729, 56.7 MB with LIGHTING ON), loaded from
the cache in 40-137 ms (slower when it was not already in memory), then
uploaded and the lamp scan run off the game thread. Work: time each stage
from a load to the first lit frame (cache read, upload, lamp snapshot and
scan) at RELIT and ON on the field laptop, and cut or hide the gap (keep
the previous level's lighting off until the new bake is ready, prefetch
the cache, upload in parts). Acceptance: lights present from the first
gameplay frame after a load, pictures unchanged afterwards.

Work log, 2026-11-16 (measured, field laptop's real screen): after a
savestate load with Duke idle, shots at frames 5-360 compared with the
settled picture (frame 360), two runs each at 2x: DUKE HILL's town settles
around frame 30 with LIGHTING RELIT (difference 10.4, 7.4, 5.5-7.9, then
3.0 at frames 5, 10, 20, 30) and around frames 10-20 with ON (5.5, 3.2-5.0,
3.0-4.1); UI slot 2 shows the same at both (its frame 5 is D17BK's 4:3
picture). So RELIT's lights arrive about a quarter of a second later.

Work log, 2026-11-17 ([note 505](../documentation/505-d41w-lights-after-a-load.md); lab `recomp/analysis/d41w/`, local).
Stage times after a load (new trace line `[TTK bake] level N after the load`,
`[gl bake]` with a monotonic stamp): the game thread published the bake as
soon as it was read, so (D41P2) the fixed lamps left the live list, while the
renderer uploaded the atlas 4 MB an image and only then drew with it: 9-15
images with neither the live lamps nor the bake (CODE, MEASURED: DUKE HILL's
porch unlit in matched shots, 2.2 mean difference against 0.3 for the live
lamps). Before that the bake waited for the prop lamps' wall checks, 8 a
tick (110-400 ms). Built: the renderer uploads a new bake into separate
textures while the shown ones stay (framework, so a LIGHTING change never
drops the bake either) and, while none is shown, uploads for up to 6 ms an
image; the game publishes the bake (face map, floor grid, live lamps off)
only when `psx_mod_lightmap_shown()` says the renderer draws with it (2 s
fallback); the renderers' copy is made before the floor grid; the wall
checks run as many as fit in 8 ms a tick while no bake is shown. Developer
`DNTTK_BAKE_WAIT=0` and `PSX_BAKE_UPLOAD_MS=0` give the old timing. Measured
(RELIT, load to bake on screen): desktop 496/433 ms -> 312-317/192-200 ms
(level 6 / DUKE HILL); field laptop offscreen 703/592 -> 464/263 ms, cold
cache 671/733 -> 391/177 ms, its real 60 Hz screen 770/752 -> 411/394 ms;
ON on the laptop 589/456 -> 375/165 ms. Matched frames against a same-level
reload (the bake from its first frame): no lampless frame in the new build
(DUKE HILL desktop 0.31 then 0.00 from 360 ms; laptop real screen at most
0.81). LIGHTING switched in play: no dip. Vanilla smoke run: no bake code
runs. Local tests: 262 pass (2 skipped), the five CI runtime tests pass, the
exported patch applies to the pinned source and matches the tree. Limits:
the bake still arrives 180-460 ms after the load, the live lamps (about 0.3
mean difference from it) light the level until then; the first picture comes
about 75 ms after the load. Not measured on citadel. Needs the user's check
on the field laptop (rebuilt with this change).

- 2026-11-17: accepted and closed. User: "looks awesome to me. i'd say lets lock it in and close it out".

### D41X - Does the barrel in UI slot 12 really emit light?

**Status: Done ([note 533](../documentation/533-d41x-fire-barrel-light.md)), accepted 2026-11-22. Dependencies: D41.**

User, 2026-11-22 (while playtesting D53G): "barrel in slot 12, is that really
emitting light? hmm". Work: on a private copy of UI slot 12, identify the barrel
(object type and level) and what lights it: the original's own vertex
colours or light, a D41 dynamic or prop light (`prop_lights`), a glow (D41
glows) or the lamp bake. Compare with the original (Vanilla, `--dynamic-lights
off`, glows off) and decide with the user whether the light fits the object.
Fix the rule for the whole object type, not this one level (no per-level data).
Acceptance: the user agrees the barrel's lighting matches what it is.

Work log, 2026-11-22: the warm patch round the slot 12 barrel (fire barrel,
prop type 220, LEVEL00 room 22) is the original's painted vertex light
(Vanilla, Original and Enhanced alike); none of our lights lit it on level 0
(the steady `scan room 22` lamp adds about 0.3 levels there and in Relit only
dims the room to 60%), while level 5's barrel alone had a sign lamp from its
level file. User chose a real fire light for every fire barrel. Now
`prop_lights.cpp` finds the barrel by its model data in the prop scan (7 on
level 0, 1 on level 5) and gives it a warm flickering fire light in its
flames (1.0 0.52 0.18, strength 1.3, radius 3000, model y -300) that sputters
out when it blows up (D54); the sign scan skips it (`prop_light_lit_mesh`)
and level 5's barrel lamp line is removed. Original, Vanilla (pixel-identical)
and Classic unchanged; 180 Hz redraws clean. Waits on the user's look on the
real display ([note 533](../documentation/533-d41x-fire-barrel-light.md)).

Closed 2026-11-22. User: "it's absolutely perfect! accepted". Follow-up
backlog: D41Y (construction barricade warning lamps).

### D41Y - Construction barricade warning lamps flash

**Status: Done (user-accepted 2026-11-22, [note 534](../documentation/534-d41y-barricade-warning-lamps.md)). Dependencies: D41, D41X.**

User, 2026-11-22 (accepting D41X, in a new UI slot 12 save): "add flashing
lights to this object too, this kind of construction barrier thing". Save
copy: `research/savestates/d41y/` (local).

What is there (MEASURED, offscreen on a private copy): the save is LEVEL00
room 20, Duke at (-12530, -6015, 41418) facing two orange-and-white
construction barricades. Each is a static prop (kind 1) of type 128, model
9 vertices (first (-205, 255, -119), last (-205, -255, 2); at `0x80125fc4`
on level 0), with a small amber warning lamp drawn on its top rail. The lamp
is steady in the original (the D41D1 sign scan rates it a non-animated
glow, colour 1.00 0.77 0.00, about 2237) and casts no light: no lamp file
binds type 128, and our lights give it none. Level 0 places four (rooms 20
and 23, two in each); other levels not surveyed.

Work, the D41X way (one rule, no per-level data):
1. Survey every level for type 128 (and any other barricade or warning-lamp
   models) with the level select.
2. Find the lamp's faces in the model (the bright amber texels) and their
   model position.
3. Modernized Polymorph: the lamp blinks like a real barricade light (about
   1 Hz, on/off with a soft edge, a per-barricade phase so a row does not
   blink in lock step), drawn as a glow over the lamp face (D41L drawn
   extras) and casting a small amber blinking light (`prop_lights.cpp`, found
   by model data like the fire barrel; lit by LIGHTING ENHANCED and RELIT; out
   when the barricade breaks, D54). Exclude the model from `lamp signs`
   (`prop_light_lit_mesh`).
4. LIGHTING ORIGINAL keeps the lamp steady with no light? Decide with the
   user whether the blink itself (a drawn change) belongs to every LIGHTING
   setting or only ENHANCED and RELIT. Vanilla and Classic unchanged.

Acceptance: in UI slot 12 the barricade lamps blink and light their rails and
the ground around them at 180 Hz on the real display; every barricade in
every level does the same; breaking one puts its light out; Vanilla
pixel-identical; the user agrees it looks like a real barricade light.

#### Work log

2026-11-22 (Needs playtest). Found: **the lamp already blinks in the
original.** Its face (face 0, a 16 x 16 quad on top of the A-frame) is an
animated texture, 2 frames of 600 ms, lit amber and dark, all barricades in
step (one shared animation record); the frame words alternate every 0.6 s in
RAM and on screen. The card's "steady" was a misreading of the sign scan's
static rating. What was missing was light. `prop_lights.cpp` now recognises
the barricade by its model data (9 vertices; kind `prop_light_barricade`)
and gives each standing one an amber light at its lamp (1.0 0.66 0.10,
strength 0.7, radius 1100) that follows the game's own frame through the
D55 signal reading (`signal_now`, exported as `prop_signal_now`), read once a
game frame: on with the lit frame, a 90 ms bulb fade after it; plus a soft
amber halo over the lamp while it is lit. Out when it breaks (D54); `lamp
signs` skips the model; `lamp props off|on` includes it. The blink, ORIGINAL,
Vanilla and Classic are untouched (step 4 needs no decision: the blink is the
original's). Evidence: level 0 scan 4 barricades; RELIT floor beside one 53
lit against 32 dark, rails 103 against 86 (props off: no change); Vanilla 0
of 786432 pixels changed against the old build; 180 Hz p95 6 ms with props on
and off, 600 presents each; break by hand logs `barricade lamp light goes
out` for both. Survey of 16 levels (0-2, 5, 7, 9, 11, 12, 22-29): barricades
on level 0 only; levels 3, 6, 8, 10 and 21 not reached by the lab's level
jump. First tune (1.0, 1500) lit the whole floor and was cut. Limits: lights
in step (the original's), halo per model, no lamp shadows; not yet seen on
the real display. Executable `4f4f20a5...`. Before/after:
`research/before-after/D41Y/` (local).

Closed 2026-11-22. User: "lock it in and close it out!" Follow-up backlog: D17BO (the
subway train flickers as Duke walks, new UI slot 12).

### D41Z - Caged wall lamps give light (UI slot 12)

**Status: Done (user-accepted 2026-11-22: "its perfect!! lock it in and close it out"; [note 536](../documentation/536-d41z-caged-wall-lamps.md)). Dependencies: D41, D41X, D41Y.**

User, 2026-11-22, closing D17BO: "these lamps are not emitters, so we need
to do the same kind of thing as we have been doing with other objects".

Test save: UI slot 12 (saved 2026-11-22 12:08, LEVEL00, the subway tunnel:
Duke at (26354, -6016, 41758) looking at a small caged lamp, pale yellow
behind bars, on the tunnel wall at the crosshair), copied to
`research/savestates/d41z/` (local, never committed). Load private copies,
never the player's card.

Work, as D41X (fire barrels) and D41Y (barricade lamps): find what the lamp
is (a prop with its own type and model, or part of the wall's texture), how
many there are and on which levels, and whether the original animates it.
Then make every such lamp a light source game-wide, found by its model or
texture, never by placed positions (no per-level data): a small warm light
at the lamp in LIGHTING ENHANCED and RELIT, in its colour, steady or in step
with the original's own animation, with a soft glow on the lamp if it reads
better; out if the lamp can be broken. Check it against the level lamps
already placed there (`lamp list`, D41D sign scan, D41M bake) so the light
is not doubled. LIGHTING ORIGINAL and Vanilla unchanged.

Acceptance: the caged lamps in UI slot 12 light the wall and floor around
them in ENHANCED and RELIT, and the same lamps elsewhere do too; no
doubling with existing lamps; no cost at 180 Hz worth noting; ORIGINAL and
Vanilla unchanged; the user agrees.

Work log, 2026-11-22 ([note 536](../documentation/536-d41z-caged-wall-lamps.md)):

- Measured: the lamp is a static prop, not texture: types 665 and 667 (pale)
  and 851 (red, level 5) share one 26-vertex cage model, one record per type
  and level, steady (no animated face), unbreakable by gunfire. Level 0 has
  24, level 5 32 (8 red), level 9 7; none on the 13 other levels reached.
- Implemented in `prop_lights.cpp`: every caged lamp, found by its model,
  gets a steady light 90 units before its bulb in the colour read from its
  own texture (pale 1.00 0.99 0.60, red 1.00 0.06 0.06), strength 0.8,
  radius 1600, and a soft halo in that colour, in LIGHTING ENHANCED and
  RELIT. A lamp the level file already binds to that prop (level 5's red sign
  lamps) keeps that lamp alone; `lamp signs` skips the model; out when marked
  broken (D54).
- Evidence: slot 12 wall 34 -> 71 (Relit), 50 -> 71 (Enhanced), Original
  unchanged; light points in their own rooms for all 24 level 0 lamps; Vanilla
  pixel-identical; 180 Hz pacing unchanged; local tests OK.
- Remaining: the user's look in UI slot 12 on the real display; levels 5 and
  9 checked by trace only.

Closeout, 2026-11-22: user "its perfect!! lock it in and close it out". Done. Lab `recomp/analysis/d41z/` trimmed
to its scripts, results and the D41AA train inspection.

### D41AA - Subway train lights (UI slot 12)

**Status: Done ([note 537](../documentation/537-d41aa-subway-train-lights.md)), user-accepted 2026-11-22 ("excellent!!! ... close out the job at hand"). Dependencies: D41, D41X, D41Y, D41Z.**

User, 2026-11-22, accepting D41Z: "thoroughly inspect the train model as
shown here. i feel like there are spots on the train that could be lit and it
would add drama to it ... there are clearly sections here which look like
lights".

Test save: UI slot 12 (saved 2026-11-22 12:47, LEVEL00: Duke at (18763,
-6015, 40290) facing the front of the parked subway train on the tracks),
copied to `research/savestates/d41aa/` (local, never committed). Load
private copies, never the player's card.

First inspection (D41Z lab, `tm-results.json`, `tm-mosaic.png`, local;
MEASURED):

- The train is a static prop (kind 1) of type 77, model `0x8011c41c` (69
  vertices, 65 faces, 9470 long: x -9361..110, y -1794 (roof)..46, z +-898),
  record `0x801d5d64` in room 82 at (7399, -5784, 39932), yaw 1958 (D17BO's
  train). No face is animated (no animation table), so any light is steady
  unless we choose otherwise.
- Faces that look like lights, from their textures:
  - Faces 0 and 2: the bumper strip low on both ends (y -357..46): a lamp
    cluster of amber, red, amber squares (hot texels 210 106 0).
  - Face 3: a bright cyan panel on top of the front end (x -9361..-8846,
    y -1523), 231 of 238 texels bright (mean 75 196 242): the blue window
    seen over the cab in the save.
  - Faces 1, 17, 18, 19 and 61-63: the end panels' lower corners carry two
    red marker lamps each (200 0 0); faces 17 and 19 also hold the amber
    hazard stripes (not lamps).
  - The side windows (faces 4-6, 21, 23, 26, 33, 36, 40-42, 52) are dark
    grey glass: lighting them (a lit car interior) would be an addition, not
    something the texture shows; ask the user.
- Existing lamps: level 0's file has one D41D1 sign lamp bound to a type 77
  prop (`sign type 77 room 23 prop 77 32741 -5784 41976`, cyan 0.14 0.76
  1.00, radius 3992) - the cyan panel of a second train at a place where the
  room-list scan now finds none (only the room 82 train). Find what that
  second train is (a moving object, another state, another list) before
  adding lights, so no train is lit twice and none is missed.

Work, as D41X-D41Z: make the train's lamps light sources game-wide, found by
its model (and by its face textures for where on the model), never by placed
positions: the bumper clusters cast low amber/red light on the rails and
tunnel floor, the red markers small red glows, the cyan panel a cool light,
each with a soft halo on the lamp where it reads better; LIGHTING ENHANCED
and RELIT, ORIGINAL and Vanilla unchanged. Keep the D17BO fix (no flicker
while walking past). Check every level the train model appears on.

Acceptance: in UI slot 12 the train's lamps visibly light the rails, floor
and tunnel around its ends in ENHANCED and RELIT and add drama; no doubling
with the existing type 77 sign lamp; no flicker; no cost at 180 Hz worth
noting; ORIGINAL and Vanilla unchanged; the user agrees.

Work log 2026-11-22 (Needs playtest; [note 537](../documentation/537-d41aa-subway-train-lights.md)):
user "let's see what we can do with it to make it look more exciting"; UI
slot 10 (parked) and slot 9 (at the controls, E starts it) copied to
`research/savestates/d41aa/`.

- MEASURED: one train (record `0x801d5d64`, type 77). E: parked (state 0)
  at (32741, -5784, 41976), pulling away at 0.78 s, 256 a tick west, the
  crash (state 3) at x 7910 after 4.25 s (it slews and skids 2044 sideways),
  stopped (4) at the slot 12 pose. The level file's cyan sign lamp was bound
  to the start pose, so it sputtered out (D54) as the train left: the moving
  and wrecked train had no light.
- Change (`prop_lights.cpp`, by model, no level data): lights ride with the
  train every image and follow its state: both bumper clusters and the cyan
  roof beacon; the headlight strikes up as it pulls away and lights the
  tunnel ahead; at the impact an orange flash and skid sparks on the side it
  skids toward, the beacon dips, the headlight blows; the wreck's nose
  cluster buzzes and drops out. Halos on the six bumper lamps, beacon,
  headlight, flash and sparks. Level 0's train sign lamp removed;
  `lamp signs` skips the model; `lamp props` covers it.
- Evidence: state trace and light levels as designed; crash region 32-41 ->
  79-82 for 0.7 s after impact; slot 12 tail end lit cyan and amber;
  Original unchanged; Vanilla pixel-identical; 180 Hz pacing unchanged
  (slot 12 and during the ride); local tests OK. Before/after pairs in
  `research/before-after/D41AA/` (local).
- Remaining: the user's look on the real display (slot 9: press E; slot
  12), especially the crash strength and the beacon.
- Same day, user: "the lights in this shot flicker if you walk a step
  forward and back" (new UI slot 12 at the wreck's nose). Found: the wreck's
  intended buzz (dropouts strobing at about 34 Hz; still burst 9 jumps,
  none without it). Room gate, budget and halos ruled out. Fix: the wreck's
  nose cluster holds steady after its impact sputter (0 jumps over 1080
  images); `DNTTK_PROP_TRAIN_BUZZ=1` keeps the old look for developers.

### D41AB - Train beacon flickers like Duke 3D lights (UI slot 12)

**Status: Done ([note 538](../documentation/538-d41ab-duke3d-random-lights.md)), user-accepted 2026-11-22 ("lock it in and close it out. ten out of ten"). Dependencies: D41AA.**

User, 2026-11-22, accepting D41AA: "my crosshair is pointing at that
overhead light. i wonder if we can make that blink in the same way that
duke3d's lights can blink. i essentially want to reimplement that exact
aesthetic ... its essentially a random flickering effect, it doesnt look like
a glitch, its got a real aesthetic to it."

- The light: the subway train's cyan roof beacon over its tail end (model
  face 3; D41AA `train_beacon`, a light that rides with the train, and its
  halo). Test save: UI slot 12 (saved 2026-11-22 14:03, LEVEL00, the wreck
  at rest, Duke at (18190, -6017, 41162) looking up at the beacon), copied to
  `research/savestates/d41ab/` (local, never committed). D41AA's UI slot 9
  (at the controls, press E) shows it on the moving train.
- Reference, to study before building: the EDuke32 wiki's lighting effects
  category (https://wiki.eduke32.com/wiki/Category:Editing_Lighting_Effects,
  with its tutorial) and the tutorial map the user placed at
  `research/Tutorial.map` (local). Reimplement the Duke 3D random-light
  flicker closely from how the engine does it (its sector effector and the
  game code's timing and shade steps), not an approximation: random, but
  never reading as a glitch.
- Scope: the flicker drives the beacon's light and its halo together (and,
  if the original affects the lit surfaces' shade, the equivalent here), at
  any frame rate (verify at 180 Hz). Build it as a reusable lamp style (a
  level-lamp `style` and a prop light option) so other lamps can use it
  later; game-wide, no per-level data. Avoid D41AA's lesson: a fast hard
  strobe read as a flicker fault, so match the reference's rhythm.
- Modernized only (LIGHTING ENHANCED and RELIT); Original and Vanilla
  unchanged.

Work log, 2026-11-22 (Needs playtest):

- Reference read: EDuke32 `actors.cpp` SE 4 (`SE_4_RANDOM_LIGHTS`) and its
  spawn in `game.cpp`, `g_globalRandom` drawn once a 30 Hz tick;
  `research/Tutorial.map` parsed (its SE 4 rooms: SE shade 0, sector shade
  20). A Duke random light sits dim and on one tick in eight, at random,
  flashes for one 33 ms tick to a random shade 0-15.
- `recomp/src/ttk/duke_flicker.h` implements that rule; level lamps get the
  style `duke[:H[:ON[:OFF]]]` (live and baked; console `lamp style duke`),
  prop lights a style option, and the train's cyan roof beacon (light and
  halo, read from one tick per game frame) uses it: hitag 0, lit 0, unlit
  20, strength 1.2 at shade 0. Developer
  `DNTTK_PROP_TRAIN_BEACON_STYLE="duke H ON OFF"|"steady"`.
- Evidence (UI slot 12, 180 Hz, Relit): flashes of 6 images (one tick) or
  11-12 (two), door, halo and ceiling on the same images; a `duke` level
  lamp flashes the same way; Original constant; pacing unchanged (p95 6
  ms); unit test of the rule; 270 local tests OK.
- Waits on the user's look on the real display (UI slot 12; UI slot 9,
  press E, for the moving train). Limits in the note: the painted beacon
  face stays bright, no palette swap, the pause menu not measured.

Closed out, 2026-11-22: user "lock it in and close it out. ten out of ten". Done; lab trimmed to its scripts and
results (raw bursts removed).

### D41AC - Relit emitter test catches saturated model colours

**Status: Todo (user 2026-11-22). Dependencies: D41D, D41E.**

User, 2026-11-22 (D57L, UI slot 12 resaved 22:55, copy in
`research/savestates/d57l/red/`): "the red of duke's shirt and the med kit
red, seem to be quite glowy in this dim light. i noticed that in the strip
club too?" Then: "lets stand it up as a new job".

Found (D57L lab, the apartment's vanity mirror view, room lit): the world
shader (`recomp/psxrecomp/runtime/src/gpu_gl_renderer.c`, the D41D emitter
term in the lit path) treats any texel that is bright and strongly coloured
as a light source: `em = smoothstep(0.3, 0.55, max) * smoothstep(0.6, 0.85,
saturation)`, and an emitter texel keeps its unlit look (`rgb * v_col * 2`),
skipping Relit's dimmed ambient and the lamps. Meant for sign letters and
neon; it also catches cloth and paint. Mean pixel values, Original -> Relit:
walls 126 -> 69 (55%), Duke's red shirt and the medkit's red cross 189 -> 177
(94%), Duke's skin 144 -> 136 (94%; not an emitter by the test: also check
whether models are dimmed less than the world). Prop lamps and the D57L rule
made no difference. The strip club (saturated reds and pinks) is very
likely the same; not measured there.

Work:

1. Survey what the test catches: a developer view that tints emitter texels,
   across the levels (signs, neon, screens, cloth, pickups, blood, enemies).
2. Choose with the user, by rule (no per-level data): a tighter threshold;
   models (characters, pickups) never emitters, with lit props such as the
   club's neon bar kept; or explicit emitters (the known sign, neon and
   screen faces). Also settle the model ambient question above.
3. Relit and Enhanced; Original, Vanilla and Classic unchanged; export the
   runtime patch; cost at 180 Hz; before/after shots (the apartment mirror,
   the strip club).

Acceptance: saturated model colours dim with the room in Relit while real
signs and neon keep their glow, judged by the user on the real display.
