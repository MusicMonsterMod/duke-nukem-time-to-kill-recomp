# D41F - Shadows

Job D41F (selected 2026-11-10 as the prerequisite of D41E Relit levels;
accepted and closed 2026-11-10).
Before this job a dynamic light reached every lit surface within its radius,
so a lamp, a sign or a blast in one room lit the next room through the wall
between them; the D41D1 room gate only hid the worst cases (note 429,
Limits). Shadow maps now stop the light at the walls.

Evidence labels: CONFIRMED (measured or seen in our own captures), INFERRED,
UNKNOWN.

## 1. Research spike: what fits Time to Kill

- GZDoom's 1D shadow maps assume 2.5D sectors; TTK's rooms are full 3D
  meshes (note 413 section 3), so they do not fit.
- The host can read every room's mesh in world space (CONFIRMED): the room
  table `0x800da454` (count `0x800da47c`, 88-byte rooms, origin +0, piece
  count +0x32, piece pointers +0x40), each piece a world mesh (+0x14 vertex
  count, +0x18 polygon list offset, +0x1c 8-byte vertices; groups 0x60
  triangles and 0x61 quads, 0xff ends), decoded as level_lights.inc and
  near_clip.cpp's `world()` do. Level 0: 24443 triangles in 283 pieces,
  built in 1.4 to 2.9 ms.
- Rendered corners and the mesh agree (CONFIRMED, developer probe
  `DNTTK_SHADOW_PROBE=1`): emitted unclipped world corners plus the eye lie
  2.5 units from the nearest mesh corner (a constant 2.5 in y).
- Three renderer instances draw (the emulation thread's and two render
  threads', note 371 / D17AW), each on its own GL context shared with the
  first; redraw workers are forked processes that only record streams. So
  each instance keeps its own shadow atlas and draws its own maps from the
  recorded list, and the mesh lives once in a shared store.
- Chosen: omnidirectional (cube) shadow maps for the strongest lights near
  Duke, in a depth atlas, sampled per pixel in the existing textured shader
  with hardware PCF. Stencil shadows, lightmaps and ray tracing were set
  aside (note 413 section 4).

## 2. Implementation

Title (`recomp/src/ttk/light_shadows.inc`, included after
dynamic_lights.inc):

- Shadow mesh: every room piece's triangles in world space, one chunk per
  piece with its bounds, rebuilt when the room table, its pieces or their
  sizes change and after a savestate load; sent with
  `psx_mod_gpu_set_shadow_mesh`.
- Slots: `lights_publish` marks which lights may cast shadows (rounds in
  flight, placed lamps, TV screens, prop lamps, fire, blasts, test lights;
  not muzzle flashes, which sit at the gun beside a wall, or wall sparks).
  The strongest near Duke (colour x r^2 / (d^2 + r^2)) get up to 16 slots. A
  light keeps its slot, and its map is not redrawn, while it stays put and
  within the reach its map was drawn for (1.15 x its radius at the time, so
  a fading blast or a flickering fire keeps one map). At most 8 new maps a
  tick; the rest stay unshadowed for that tick.
- The light's flags carry the slot (`2 * (slot + 1)`, the wrap bit 1 kept),
  and the list rides in the `LightView` to redraw workers and is sent before
  the lights (`psx_mod_gpu_set_shadows`).
- Console: `light shadows`, `light shadows on|off|default`, and the
  developer `light shadows ray x y z x2 y2 z2` (nearest mesh triangle on a
  segment). Developer `DNTTK_SHADOWS=off`.

Framework (`time-to-kill-accepted-source.patch`):

- `mod_plugins.h`: `PSX_MOD_SHADOW_SLOTS` 16, `PSXModShadowChunk`,
  `psx_mod_gpu_set_shadow_mesh`, `PSXModShadow` (world position, reach,
  slot, stamp), `psx_mod_gpu_set_shadows`, the shared store's
  `psx_shadow_mesh_take` / `_generation`.
- `shadow_mesh.c` (new, not a copied source): the mutex-guarded store with a
  generation counter.
- `render_replay.c`: stream kind 8 (the shadow list); `gpu.c` records or
  applies it.
- `gpu_gl_renderer.c`: per instance a 16-bit depth atlas (12 x 8 tiles of
  512, 6144 x 4096, 50 MB), its FBO, a static VBO of the mesh and a small
  depth program. A slot's six faces are drawn from the light over the chunks
  within its reach and in front of each face when its stamp or the mesh
  generation changes, storing distance / reach; all GL state is saved and
  restored around it and queued draws are flushed first. The textured
  shader's `shadow_vis` picks the face, applies a normal offset of 3 texels,
  no depth bias, 4 PCF taps 0.3 texel apart, and multiplies the light's
  term. Developer `PSX_SHADOW_TILE`, `PSX_SHADOW_TUNE="offset bias spread
  [view]"` (view 1: red faces away, green shadowed, blue lit).
- Render-thread rename headers regenerated (`gen_names.py`).

TTK's walls are single panels, so a wall is both the occluder and the
receiver. For world faces (two-sided normals from screen derivatives) the
side the eye sees takes a shadowed light only when the light is on that
side; without this rule, the normal offset pushed back-lit wall pixels
through the wall to the light's side and lit them (a cube-face-shaped patch,
first capture). Models (one-sided smooth normals, D41H) keep their own
facing term.

Tuning (CONFIRMED, matched shots `recomp/analysis/d41f/`): a one-texel depth
bias lit receivers just behind an occluder; offset 3 with spread 0.3 left the
fewest bright lines along wall corners; a 1024 tile removed them almost
entirely and 512 nearly so (chosen, 16-bit depth to keep the memory down).
Growing the shadow triangles in their own plane did not help and was
removed.

## 3. Evidence

Private copies of the player's savestates and cards
(`recomp/analysis/d41f/lab.py`, local), player files hash-checked unchanged
after every run.

- Light behind a wall (UI slot 8, sandstone room; test light 3000 ahead,
  beyond the far wall): without shadows the whole room is lit; with shadows
  it matches the room without the light.
  `research/before-after/d41f-shadows/light-behind-wall-*.png`.
- A light throwing a shadow (UI slot 9 subway, test light behind the
  pillar): the pillar's front goes dark and its shadow cuts a sharp edge
  across the lit ceiling and floor. `pillar-shadow-*.png`.
- Placed lamps with the D41D1 room gate off (subway): the blue spill of a
  lamp behind the wall leaves the pillar and floor; the Cola machine's own
  light stays on its wall. `subway-signs-gate-off-*.png`.
- In-room test lights: lit walls and floors unchanged, no acne.
- Cost, player's machine (GTX 1080 Ti, real display, 180 Hz, Chrome using
  about one core meanwhile): shadows on and off alternating over 10 s each
  while firing showed 0 missed redraws and matching frame-time histograms;
  under Laser Gatling fire (48 launches, 9 blasts in 10 s) the emulation
  thread's renderer drew 59 maps, about 0.12 ms of host time each, at 180
  presents/s. Static lamps are drawn once per instance.
- Build OK; `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS.
  Framework patch exported and verified against a clean checkout of the
  pinned framework (`08ec704a`).
- Vanilla: launches and plays; lights (and so shadows) stay off.

## 4. Limits

- Props, enemies and Duke cast no shadows (they receive them); see-through
  room faces (fences, grates, windows) cast solid shadows (no alpha test in
  the shadow pass). Moving room geometry keeps the shadow of its start
  position until the next rebuild.
- Faint light lines can remain along some wall corners behind an occluder
  (shadow-map resolution).
- Lights past the 16 slots (or past 8 new maps in one tick) are unshadowed.
- Shadow maps are drawn at the game tick like the lights (30 Hz).
- Not a shadow fault: a ceiling face in the sandstone room (UI slot 8) drops
  out to black on some frames with shadows off as well (time series
  `recomp/analysis/d41f/ser-grid.png`); a separate finding. D17BG (note 440):
  offscreen llvmpipe only, not seen on the player's GPU.
- No player option yet: shadows follow DYNAMIC LIGHTS; the console toggles
  them for the session.

## 5. D41F1: hard shadow edges (2026-11-10)

At the user's request shadow edges are hard lines. `gpu_gl_renderer.c`: the
depth atlas uses `GL_NEAREST` (the compare is no longer filtered over 2 x 2
texels) and `shadow_vis` takes one tap instead of 4 PCF taps; the spread
value of `PSX_SHADOW_TUNE` is unused. Offset 3 and bias 0 are unchanged.
Matched shots: `research/before-after/d41f1-hard-shadows/` (UI slot 9 subway,
`lab.py` case `wall` with `SLOT=8 TURN=3.3 AHEAD=1200 LAT=1400 H=700
RADIUS=7000`, old and new builds; camera yaw differs by a few pixels between
runs). Far from the light the 512 tile can show stair steps along an edge.

Anti-aliasing (the same day, the user's follow-up): the atlas is linear again
and `shadow_vis` sharpens its one filtered tap, `clamp((v - 0.5) * sc + 0.5)`
with `sc = texel / (pixel * width)`; `pixel` is the world size of a screen
pixel (`s_pix`, from `dFdx`/`dFdy` of the world position in `main`, where
control flow is uniform), `width` the third `PSX_SHADOW_TUNE` value (default
1 pixel). The edge is hard, anti-aliased and follows the filtered contour
rather than the texel stairs; with 128 tiles it stays straight. Where `sc`
is 1 or less the filtered value is used as it is.

Second pass: `shadow_vis` takes n x n filtered taps one texel apart (n from
developer `PSX_SHADOW_TAPS`, default 3) and sharpens with
`sc = texel * n / (cos * pixel * width)`, `cos` the light's angle to the
surface (minimum 0.1). With 128 tiles the 1-tap contour shows kinks and the
3 x 3 one is straight. 1024 tiles and a perspective-correct world position
were tried and made no measured difference (reverted).

Exact edges (same day): `SH_FS` also writes `u_base + gl_PrimitiveID + 1`
(the draw's first triangle plus the primitive) to an R32UI attachment
(unit 6, cleared per face); the mesh VBO is also a buffer texture (R32F,
unit 7); `u_shlw` holds each slot's world light position from its last map.
`shadow_vis` keeps the filtered lookup to find the edge band (it, or a
2 x 2 plain lookup at the ray's own texel with a slope bias, is mixed, or
they disagree) and there calls `sh_exact`: the unique ids of the 4 x 4
texels around the ray, Moller-Trumbore segment tests (barycentric slack
1e-4, hits within 2 units + 0.2 percent of the receiver or within 2 units
of the light ignored) from 4 rotated-grid sample points (`s_dx`, `s_dy`
screen derivatives of the world position) to the light. All three shadow
samplers get their units at program setup (samplers of different types
must not share a unit). Resolution check at the player's settings: exact
at 128 and at 512 texels agree except the animated pickup and HUD.

Fast exact edges and SHADOW EDGES (same day; the user found play choppy).
Measured on the player's GPU (developer `PSX_SHADOW_PROF=1`: a GL_TIME_ELAPSED
query over each frame's scene draws, logged every 240 frames; Relit, internal
scale 3, 30 fps, UI slot 8 fence view with a test light): the exact edges
above cost 16.5 ms a frame against 4.0 ms without them. Now:

- One filtered tap. The edge band is a 2 x 2 plain lookup at the ray's own
  texel (slope bias 1.5 texels): fully lit is lit, fully shadowed with the
  offset tap also shadowed is shadowed, anything else runs `sh_exact`.
- `sh_exact`: the ids of the 3 x 3 texels around the ray; per triangle (three
  RGBA32F texels from a buffer of its own) the planes through the light and
  each edge, `n_i = A x B` etc. with A, B, C relative to the light, signed by
  the triangle's orientation `C . (A x B)`; a point d (relative to the light)
  is behind the triangle when `d . n_i >= -1e-4 |d| |n_i|` for all three and
  the triangle's plane cuts the segment at t = `(A . N) / (d . N)` between
  2 units from the light and 2 units + 0.2 percent from the receiver. The edge
  terms are linear in d, so the 4 rotated-grid points are `e + e_x s_x +
  e_y s_y` from the screen derivatives; the loop stops when all 4 are
  shadowed.
- At most `u_shxmax` exact tests a pixel (developer `PSX_SHADOW_XMAX`, default
  2), and none for a light whose falloff times brightness is 0.03 or less.
- `u_shmode` (framework `psx_mod_gpu_set_shadow_edges`, the title's
  DNTTK_SHADOW_EDGES): 1 SMOOTH as above, 0 SIMPLE, one texel at its centre
  (hard and stepped). SIMPLE draws no ids; the R32UI id texture is made and
  attached the first time SMOOTH draws a map. A change redraws every map.
- Developer view 3: yellow where any light's exact test decided, magenta where
  tests ran with nothing to test.

Results on the player's GPU, same views: fence view SMOOTH 4.1 ms, SIMPLE and
shadows off 3.8 to 4.1 ms; UI slot 9 pillar view (Relit) all three 2.6 to
3.6 ms, 16 exact tests a pixel about 0.3 ms more than 2. Offscreen, 128-texel
maps: the exact band follows a straight edge
(`research/before-after/d41f1-hard-shadows/fast-devview-exact-band-128.png`).

The 180 Hz stutter (2026-11-11). Play held 90 presents, not 180, in SMOOTH and
SIMPLE alike. At 180 Hz the emulation thread presents every redraw
(`gl_renderer_rp_tick` -> `SDL_GL_SwapWindow`); with the GPU saturated the
driver sleeps in the swap, the game stops for 0.2-1.2 s, the audio ring
empties and the D23H governor sheds (stacks sampled with `eu-stack`,
developer `PSX_ALLOW_PTRACE=1`). GPU utilization at 180 Hz, same view: before
D41F1 30% lighting off / 38% Relit with shadows; with the fast exact pass 66% /
72%, in every mode; with the exact test compiled out (`PSX_SHADOW_NOX=1`)
30% / 39%. The textured shader's register allocation follows its heaviest
path: `sh_exact` kept `uint ids[9]` indexed in a loop. Now it tests each of the
nine texel ids as it reads it (skipping a repeat of the last), with the four
sample points in a bit mask: 30% / 41%, and walking UI slot 8 (level 6) for
60 s at 180 Hz held 180 presents every second. The exact test runs unless the
plain and the offset lookup both say lit or both say shadowed: trusting the
plain lookup alone ("lit") left a bright line at the foot of walls facing
away from the light. Self-hit margin: developer `PSX_SHADOW_XMARGIN` (default
2 units + 0.2 percent). Level 6 view at 180 Hz now: lighting off 38%, Relit
42%, SIMPLE 57%, SMOOTH 76%.

GPU breakdown (2026-11-11, player's GPU, level 6 view at 180 Hz; developer
`PSX_SHADER_STRIP=exact|shadows|lights` compiles that layer and those inside it
out of the textured shader): lighting code compiled out 35% in every mode;
with it, lighting off 39%, Relit 43%, SIMPLE 57%, SMOOTH 76%. Exact test out:
SMOOTH 56% (the test costs 19 points). Shadow lookups out: 44% with shadows on
(maps 3, lookups 13-14). So unused lighting code costs only 2-4 points now;
the base (lighting off) hardly follows resolution or widescreen (scale 2: 33%,
widescreen off: 32%), it is the volume of 180 redraws a second, while the
shadow terms follow the pixels (SMOOTH at scale 2: 52%, widescreen off: 48%).

Optimization pass (2026-11-11), level 6 view at 180 Hz, full clock: lighting
off 39% -> 28%, Relit 43% -> 31%, SIMPLE 57% -> 37%, SMOOTH 76% -> 42%.
- Face tables: `SH_F/SH_R/SH_U` indexed at run time compiled to local memory
  (NVIDIA assembly `TEMP lmem0[18]`, 18 stores per fragment, every mode);
  replaced by swizzles (`sh_fru`, `sh_uv`).
- Widescreen mirror: with the fast path (`wide_blit_center` copies the
  canonical centre over the wide surface) each mirror draw runs once per side
  margin, scissored to it (`WIDE_MARGINS`, `wide_margin_scissor`); the full
  width shaded the centre twice. Developer `PSX_WIDE_MARGINS=0`.
- Edge texture (R8 atlas on unit 8, a tile-sized R8 on unit 9 between the
  passes, `SH_EFS`/`SH_E2FS`, drawn after each map's ids): a texel is an edge
  when its nearest surface jumps against a 4-neighbour (each plane followed
  along the other's ray misses the other's surface by more than
  (4 + 1% of the distance) / cos), grown by four texels. SMOOTH: marked
  texel, light bright enough (`u_shq.y`) and meeting the surface above ~11
  degrees -> exact test; else one filtered tap. Shadow mesh buffer: corners
  plus the unit plane (n, d) per triangle.
- Exact test: one edge plane at a time into a 4-bit mask; a hit counts when
  the point is off the hit triangle's plane by more than `sh_eps` (1.5 units
  + 0.05 percent of the distance, developer `PSX_SHADOW_XMARGIN`), not by
  distance along the ray (that let light through the foot of walls).
- Accuracy: developer reference (`PSX_SHADER_REFERENCE=1`, console `light
  shadows reference`: the exact test at every pixel): SMOOTH differs in 101
  pixels of 921,600, SIMPLE in 883 (pillar view, offscreen).
- Per redraw (developer `PSX_GPU_PHASES=1`, GPU timestamps): setup 0.01 ms,
  scene 0.75-1.0 ms with lighting off (textured draws 0.45-0.7), end 0.16-0.3
  ms; at 1x the scene still takes about 0.55 ms. SMOOTH: scene 1.4-1.6 ms.
- Measured and not kept: skipping faint lights and a higher exact threshold
  (about 1 point); deferring the exact test after the light loop (+3 points
  in every mode, more live state); a plane test instead of the filtered tap
  away from edges (three dependent fetches cost more than one tap).

Lesson for shader work here: measure GPU utilization at the player's 180 Hz
with the shader as built, in every mode; code that never runs still costs
every pixel when it raises the register count.
