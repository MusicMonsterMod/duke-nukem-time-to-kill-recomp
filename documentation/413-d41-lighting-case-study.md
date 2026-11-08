# D41 - Foundational lighting: case study, options and D41A

Job D41 (pre-job, selected 2026-11-08). User: "make real lighting in this game
rather than of course the painted/simulated lighting the ps1 does"; "a really
deep research done so we can truly understand this lighting system"; work
autonomously, child jobs may be started.

Published case study (private artifact, owner's account):
https://claude.ai/artifact/M1MNyrZqtgzEFc7s72jSXh

Evidence labels: CONFIRMED (seen in code, notes or our own captures), INFERRED,
UNKNOWN.

## 1. How Time to Kill lights its world today

- CONFIRMED: room-mesh vertices are 8 bytes (x/y/z grid bytes, a colour-index
  byte, an RGB word) drawn as PS1 Gouraud vertex colour on GT3/GT4 records; a
  room flag (`ctx+0x50` bit 2) substitutes one flat colour (`ctx+0x54`)
  (`recomp/src/ttk/near_clip.cpp` header; notes 62, 336).
- CONFIRMED: a non-zero colour-index nibble replaces the vertex colour from
  the table at `0x800c37e4`. The muzzle flash sets that nibble on the vertices
  it reaches; the vertex pass `0x80011170` clears it after drawing (note 340
  revision 2, stuck muzzle-flash light). It is the only dynamic world light
  found; no falloff calculation was seen.
- CONFIRMED: an animated light table (`ctx+0x68`) applies when every corner of
  a polygon has index bit 4; the canal's lights animate (note 325) and level 10
  flickers (note 331). INFERRED: these are TTK's light styles. UNKNOWN: the
  writers and formulas of the colour-index and animated tables.
- CONFIRMED: fog is the GTE depth cue (DPCS) from the level's fog start
  (`ctx+0x6c`) toward a black far colour; past the far limit there is only the
  cleared background (note 97).
- CONFIRMED: model vertices carry no normals (note 184); props and actors use a
  per-mesh colour table or the flat colours at `ctx+0x54/+0x5c`, Duke starting
  from the brightness byte `player+0x1b` (note 247).
- CONFIRMED (local generated code scan): the GTE normal-lighting library
  routines (NCS/NCT/NCDS/NCDT/NCCS/NCCT/CC/CDP, `0x800B4DEC`-`0x800B4F3C`) have
  no direct call sites; the DPCS wrapper `0x800B4DD0` is called by the
  actor/prop renderers. INFERRED: actors are depth-cued, not lit by normals
  (calls through pointers not ruled out).
- CONFIRMED: explosions light nothing (sound, radius damage, camera shake;
  note 370 D39F "Blast research").

## 2. What Polymorph knows (before D41A)

- Polymorph is three title-side switches on the shared OpenGL backend
  (note 343). With unified geometry, `near_clip.cpp` rebuilds every world,
  prop and actor polygon on the host from the GTE state, clips it in view
  space and emits PS1 packets plus host vertices (16.16 screen position, view
  depth, exact UV, depth-test value, coverage).
- At emission it has view-space corners, the draw kind (world, prop, actor,
  sky, first-person layer), the texture page and CLUT, the original colour,
  colour index and fog. The host walk (`host_visibility.inc`) reads the
  frame's camera (camera block `+0x14` eye, `+0x20` matrix with row 0's 1.6
  aspect).
- The GPU received screen-space triangles only: no normals, no world
  positions, no lighting shader. High refresh draws real redraws: workers
  re-run the view composition with an interpolated camera and record a GPU
  stream (notes 353, 371).

## 3. How other engines light (research and captures)

Captures were made from the player's own installs on private copies:
EDuke32 r10669 (`~/Games/[duke3d]-eduke32-linux`, run from a scratch copy,
Xvfb then the real display for Polymer speed), UZDoom 5.0.3 Flatpak with a
separate config under `~/.var/app/org.zdoom.UZDoom/d41-casestudy`, Ironwail
0.8.1 with LibreQuake from a scratch copy. Duke-RT could not be run: it needs
ray-tracing hardware and the player's GTX 1080 Ti has none.

| Engine | Light model | Source of lights | Notes |
| --- | --- | --- | --- |
| EDuke32 Polymost | Sector shade tables, visibility fog, palette emulation in a shader | Map sector shade | Keeps the 8-bit look in true colour |
| EDuke32 Polymer | Multipass forward per-pixel point and spot lights, shadowed spots | SE49/SE50 placed by mappers; code lights for muzzle flashes and blasts | Each surface redrawn per light (up to 10); slow. Captured: an RPG blast lights floor and skylight in Polymer, nothing in Polymost |
| Duke-RT | Path tracing (Raze fork, NVIDIA NRI, DLSS RR) | Hand-authored `LIGHTOVR` overlay: `actorrule`, `muzzleflashrule`, per-map lights, emissive overrides; sector light as a signal; `sectorcolor` | Episode 1 relit by hand |
| GZDoom / UZDoom | Sector light modes plus GLDEFS dynamic lights per pixel from a uniform buffer; 1D shadow maps; SSAO, bloom, tonemap; opt-in ZDRay lightmaps | Lights bound to actor frames | 1D shadow maps only work for 2.5D maps. Captured: a TechLamp lights pillars and floor; a rocket washes the corridor |
| Ironwail / Quake | Baked lightmaps (bounce, AO), 4 light styles per surface, dynamic lights | Map light entities (ericw-tools), effects (muzzle 200, rocket 200, blast 350 units) | Ironwail: GPU lightmap updates and clustered forward dlights. Captured: lightmap vs fullbright, a rocket lighting a dark corridor |
| DarkPlaces / FTEQW | Real-time world lights, stencil then shadow maps | `.rtlights` files, generated or hand-authored | Community light packs |
| RTX Remix | Path tracing over captured D3D9 draws | Converted fixed-function lights; hand-authored lights and emissive masks | Closest analogue to Polymorph's draw interception |
| RT64 (N64 recomps) | Planned path tracing | Game memory scripts, light editor | N64 data has no lights either; still planned |
| Tomb Raider I-III Remastered, Nightdive KEX | Added real-time light over vertex-lit originals | Authored | Old graphics one toggle away; reviews cite areas too dark to read |

Sources (checked 2026-11-07/08): wiki.eduke32.com (Polymost, Polymer, SE49,
SE50); NBlood `polymer.cpp`; github.com/postmemetic/Duke-RT (README,
LIGHTOVR-AUTHORING.md, SECTOR-COLOR-AUTHORING.md); zdoom.org GLDEFS;
forum.zdoom.org 1D shadow maps (p=983376); GZDoom 4.11 release notes;
doomwiki.org UZDoom, ZDRay; ericw-tools light docs; QuakeSpasm `cl_main.c`,
`r_brush.c`, `r_alias.c`; github.com/andrei-drexler/ironwail (`gl_rlight.c`,
`gl_shaders.h`); quakewiki.org RTlights in FTEQW; NVIDIA RTX Remix light
conversion docs; github.com/rt64/rt64; Zelda64Recomp README; Nightdive
PowerSlave Exhumed page; xboxera Tomb Raider I-III Remastered review. Full
list in the artifact.

Patterns for finding lights when the data has none: lights bound to things
(actors, projectiles, events), to surfaces (texture emitters), driven by the
old light (area brightness, palette), and authored files edited in game.

## 4. Options for TTK

Constraints: 180 Hz player display with redraw workers; Vanilla and Classic
untouched; no ray-tracing hardware; Polymorph owns every world, prop and
actor polygon.

- Per-vertex dynamic light: too coarse on TTK's large polygons, host cost per
  corner per redraw.
- Per-pixel lights in the existing textured shader: chosen for the
  foundation (D41A). One loop per lit fragment, one uniform upload per image,
  no new draw calls.
- Clustered forward (Ironwail): when lights multiply (D41G).
- Lightmaps: need UV unwrapping of every room and a bake tool; research only.
- Shadow maps from the host's world-space rooms: for selected lights (D41F).
- Path tracing: not on the player's GPU.
- Relighting (dim the painted light, light with real lights): an option once
  Duke takes light and levels have light sources (D41E).

## 5. D41A - Light foundation (accepted and closed 2026-11-08)

### Pipeline

1. Sources, game tick (`recomp/src/ttk/dynamic_lights.inc`, main process):
   - blasts: entry of the sound routine `0x8006b73c` with the D39F return
     addresses (`0x80070418` projectile, `0x800708e4` pipe bomb, `0x80071044`
     dynamite, `0x8006413c` gib) and under water `0x8006b270` (`0x80070858`,
     `0x80070ff8`); the TNT barrel's shake request `0x80028a2c` from
     `0x800973e0`. Position from a1. Warm light (1.30, 0.78, 0.36), radius
     7000, 16 tics: 0.8 on the first tic, then (1-t)^2.
   - Duke's muzzle flash: player word 0 bit 3 (the hand's flash, note 63) at
     the Duke update `0x800412a4`: (0.80, 0.56, 0.28), radius 3400, 3 tics,
     placed 260 units ahead along the camera's forward row at 620 above Duke.
   - console `light [r g b [radius]]` / `light off` (test lights at Duke + 900
     up).
   - a savestate load drops lights in flight; 24 sources, 32 per image.
2. Per image: `LightView` rides in `RenderState`, so each redraw worker has
   the list of its image.
3. Draw time: the host walk calls `lights_upload` with its camera; positions
   become eye-relative, `psx_mod_gpu_set_lights` sends them, and
   `near_clip_light_frame` gives `near_clip.cpp` the inverse camera matrix
   (`d = 4096 M^-1 v`). Every unified-scene corner (world, prop, actor; not
   sky, first-person layer, Bio Mask or depth-only passes) gets
   `psx_mod_gpu_host_vertex_world`. Cleared at the walk's start and at
   `near_clip_flip`.
4. Framework (`time-to-kill-accepted-source.patch`):
   - `mod_plugins.h`: `psx_mod_gpu_host_vertex_world`, `PSXModLight`,
     `psx_mod_gpu_set_lights` (`PSX_MOD_MAX_LIGHTS` 32).
   - `gpu.c`: host vertex entries carry world position and a lit flag; a
     triangle whose three corners are lit passes them to the GL renderer.
   - `render_replay.c`: stream kinds 6 (world corner) and 7 (light list).
   - `gpu_gl_renderer.c`: `TexVert` +12 bytes (world position), lit flag in
     the spare `drs` byte, attribute 12, uniforms `u_nlights`, `u_lights[64]`,
     `u_amb`; changing lights flushes the open batch first. The textured
     fragment shader adds, per light, colour x (1 - d/r)^2 x (0.15 + 0.85
     |N.L|) to the vertex colour (N from `cross(dFdx, dFdy)` of the world
     position, two-sided), with a soft shoulder above the neutral 0.5 (max
     about 2.2x texture) instead of clipping to white, before the PS1 x2
     modulation. Unlit and raw-texture triangles are bit-identical to before.
   - Render-thread copies: `render_copy/rc_names.h`, `rc2_names.h`
     regenerated with `gen_names.py`.
5. Settings: OPTIONS > DISPLAY > DYNAMIC LIGHTS (on/off, live), profile
   schema 71 `dynamic_lights` (migration adds `on`), `run.py
   --dynamic-lights`, terminal `--settings` **I**, `DNTTK_LIGHTS` (Vanilla
   always `off`). Developer: `DNTTK_LIGHT_AMBIENT=<percent>` scales the
   original colour of lit surfaces; `DNTTK_LIGHT_TRACE=1` logs sources.

### Evidence

- Build: `cmake --build --preset local-dev` OK.
- Tests: `test_player_profiles` 133 OK (new `DynamicLightsOptionTest`);
  `ttk-near-test` PASS (it also failed to link before this job for a missing
  `cowboy_hat_mesh` stub, now stubbed); `ttk-flash-test`, `ttk-live-env-test`
  PASS. `ttk-controls-test` stops at the D14 widescreen HUD layout check
  (line 1754), the known failure recorded before this job; it is the same
  with `DNTTK_LIGHTS=off`.
- Framework patch exported and verified: a pristine pinned checkout
  (`08ec704a`) plus the patch matches every modified and new framework file.
- In game (offscreen, private apartment fixture copies, lab
  `recomp/analysis/d41/lab.py`, local only):
  - apartment, test light: walls, door and frame lit per pixel
    (`shots/a-none.png`, `b-test.png`).
  - apartment and level 0 street at 60 and 180 Hz: RPG blasts light walls
    and street and fade over about half a second; at 180 Hz the
    `render_replay` counters showed 0 misses and 0 failed captures.
  - Gatling burst at 180 Hz: the street around Duke brightens on alternate
    tics (crop mean +8 levels between rounds).
  - `DNTTK_LIGHT_AMBIENT=55`: lit surfaces dim; a blue test light and an RPG
    blast carry the scene. Duke (original renderer) does not dim.
  - `--dynamic-lights off`: no light sources logged.
- Before/after: `research/before-after/d41a-lights/` (local).

### Limits (for D41B-D41H)

- Duke's own model and the first-person weapon are drawn by the original
  path (D12) and take no light; in the dimmed preview Duke stays bright.
- No shadows: light passes through walls.
- Untextured (G3/F3) polygons, sprites and translucent faces are not lit.
- Muzzle light is ahead of Duke at shoulder height, not at the real muzzle;
  all colours, radii and lives are first values for the playtest.
- Lights update at the game tick (30 Hz); redraws in between show the tick's
  lights.
- Light reaches across room boundaries the host did not draw only where
  their polygons are drawn (no world-space light culling yet).

## 6. Child jobs (on the board)

- D41A Light foundation (this note; Done 2026-11-08).
- D41B Weapon and projectile lights (first part accepted, section 7; rest Todo).
- D41C Duke and first person in the light (section 8; Done 2026-11-08).
- D41D Level light sources (emitters, animated light tables, per-level light
  files, light editor).
- D41E Relit levels (LIGHTING Original / Enhanced / Relit).
- D41F Shadows (research spike, then shadow maps for chosen lights).
- D41G Light scale and cost (clustered lights, untextured polygons, sprites,
  translucency, 180 Hz budgets).
- D41H Smooth light on models (section 9; Done 2026-11-08).

## 7. D41B first part (accepted 2026-11-08)

- Projectile pool probe (lab `types`, level 0, every weapon fired): the pool
  `0x800da978` (100 x 0x60) marks a flying round with flag half `+0` = 3 and
  owner `+0x10` = the shooter; types seen: shotgun 1, Gatling 7, RPG 8,
  energy beam 15 (pistol 0, note 410); type 2 and 4 records with flag 1
  appear around shots (effects, not decoded).
- Rounds in flight carry light, rebuilt each tick: RPG rocket (0.95, 0.55,
  0.22) radius 2800, energy round (0.25, 0.75, 1.10) radius 2200. Captured:
  the rocket warms the street ahead of Duke before its blast
  (`shots/flight/`).
- Muzzle light at `player+0xbc` (the launch point rounds start from, about
  400 units from Duke's feet) when within 1500 units of Duke; otherwise the
  D41A point. Gatling at 180 Hz: firing frames +9 levels over the crop mean.
- Remaining: per-weapon muzzle colours and sizes, enemy muzzle flashes,
  impacts, per-blast sizes, fire.

## 8. D41C - Duke and first person in the light (accepted and closed 2026-11-08)

User: the light over the gun is so smooth "it actually now looks like it's
made of metal"; "fully accepted".

Before: `near_clip.cpp` `object()` returned early for every primary-Duke mesh
that no first-person owner claimed (`duke_skips`), so the original renderer
drew him with his painted colours; the first-person layer (`f.viewmodel`)
and fixed-projection limbs (`f.scene` false) were excluded by `lit_frame`.

- Third person: `lit_body` - this image has lights (`light_frame`), unified
  geometry, a Native-owned part and `third_person_body_lightable()` (Duke's
  primary draw, `fp_blend` 0, not the death eye, no power-up effect
  `third_person_body_effect`). Such a mesh takes the existing `body` path:
  every face host-drawn as an actor of the unified scene with depth, so its
  corners get world positions. The native proximity fade (translucency near
  the camera) still comes through the context's command bits.
- First person: `Frame::duke_lit` marks primary-Duke presentation draws
  (viewmodel or any non-Native limb owner). `lit_frame` accepts them under
  unified geometry; the first-person layer's translucent faces (the muzzle
  flash) are not lit. Their view-space corners are where they appear before
  the eye, so `light_inv` gives eye-relative world positions as for the room.
- `DNTTK_DUKE_LIGHT=0` (developer) restores the D41A behaviour. Lights off,
  Classic and Vanilla: `light_frame` is false and nothing changes.
- Evidence (local, `recomp/analysis/d41c/`): apartment, ambient 55%: crop of
  Duke's back, mean RGB none/warm/blue before (45,31,24)/(135,71,35)/(65,60,62),
  after (41,28,20)/(141,73,34)/(63,61,69), visibly dimmed and tinted; first
  person: the pistol dims and takes warm and blue light. 180 Hz rate lab:
  2165 replays, 0 misses, 0 failed captures.
- Limits: faceted light on Duke (flat normals, D41H); the muzzle light at
  the hand lights Duke's own body strongly for its 3 tics (judge in
  playtest); a power-up effect returns Duke to the original renderer for its
  duration, so he is unlit then; mirror Duke stays as before.

## 9. D41H - Smooth light on models (accepted and closed 2026-11-08)

User: "it looks good still! ... it looks great".

Before: the shader took each lit triangle's normal from screen derivatives of
its world position (one flat normal per face, two-sided), so bodies and
round props showed their facets under a light (D41C limits).

- `near_clip.cpp` `smooth_model_normals` (per `object()` draw, while the
  image has lights or a dimmed ambient and the toggle is on): world positions
  of the mesh's corners (`light_inv`), corners at the same model position
  welded, every face of the list (drawn or culled) with an area-weighted
  normal, and for each corner of each drawn face the sum of the faces at
  that corner within the crease (`DNTTK_SMOOTH_CREASE`, default 65 degrees)
  of its own. Outward: `det(v0,v1,v2) = n . v0` and front faces have
  `sign*det > 0` (`front_facing`), so the outward normal is `-sign * n`,
  times the sign of `det(light_inv)` in world space. Trace check: 97.9% of
  drawn one-sided faces face the eye (the rest are folded and odd faces).
- Corners carry the normal (`Poly::n`, `Vertex::n`, interpolated through
  clipping); `emit` sends `psx_mod_gpu_host_vertex_world_normal`. Faces seen
  from both sides (double-sided bit, open-model backs) send a 0.4-long
  normal.
- Framework: `HostPV` normal and `lit` 2, replay kind 6 with 8 words (5 still
  accepted), `gl_set_host_world(enabled, w, n)`, `TexVert.wn` (attribute 13).
  The fragment shader uses the interpolated normal when it is longer than
  0.05; longer than 0.6 means one-sided: `nl = clamp((N.L + 0.25) / 1.25)`
  (a little wrap past the terminator), otherwise `|N.L|` as D41A.
- Toggle: `LightView.smooth` travels to redraw workers in RenderState; console
  `light smooth on|off|default` (session), `DNTTK_SMOOTH_LIGHT=0` (developer
  default off), `DNTTK_SMOOTH_TRACE=1` (cost and orientation every 4000
  meshes).
- Evidence (local, `recomp/analysis/d41h/`): level 0, ambient 55%, test light
  at Duke, a spawned figure (type 123) and pig cop (57), saved to a private
  slot of the card copy and reloaded with `light smooth off` and `on` for
  matched poses. Thighs, torso, arms, Duke's back and legs and the street
  pole lose their facet breaks; the pistol keeps its hard edges. 180 Hz:
  5218 redraws, 0 misses; 4.6 us per lit mesh (17.6 faces mean).
- Changed look to judge: model light is one-sided, so a light behind a
  figure or prop no longer lights the side facing the camera (the apartment
  armchair with the test light beyond it now stays dim).
- Limits: the world stays per-face (flat walls); separate meshes (limbs drawn
  as separate parts) do not share normals across their seam; normal and
  specular detail remain optional later work.
