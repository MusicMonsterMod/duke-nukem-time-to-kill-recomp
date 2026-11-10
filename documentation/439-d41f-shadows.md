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
  `recomp/analysis/d41f/ser-grid.png`); a separate finding.
- No player option yet: shadows follow DYNAMIC LIGHTS; the console toggles
  them for the session.
