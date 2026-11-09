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
- D41B Weapon and projectile lights (sections 7 and 10; Done 2026-11-08).
- D41I Fire and enemy weapon lights (split from D41B; section 11; Done 2026-11-08).
- D41C Duke and first person in the light (section 8; Done 2026-11-08).
- D41D Level light sources: lamp files, editor, scan, level 0 (note 428; Done
  2026-11-09). D41D1: texture emitters, animated light tables, more levels.
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

## 10. D41B shot lights (accepted and closed 2026-11-08)

User: "the brightness of the lighting is so beautiful but not intrusive. i
love how the light follows the projectiles especially of the freezethrower".

Sources (`recomp/src/ttk/dynamic_lights.inc`, Modernized Polymorph with
DYNAMIC LIGHTS on; entry hooks registered as `ttk.lights` in
`modern_controls.cpp`; no framework change):

- Launch: the projectile launcher `0x800718ac` (a0 shooter, a1 origin
  vector, a3 projectile type), for Duke and every enemy. A muzzle light at
  the launch origin (the real muzzle point the round starts from) in the
  type's colour, one per shooter and tic (shotgun pellets share it); enemy
  flashes at 0.8 x radius. This replaces the D41B first part's
  `player+0xbc` point for every weapon that launches rounds.
- Impact: `0x8006f1b8` (a0 the round, a1 the point) is called when a round
  meets a wall; rounds that hit a character do not call it (checked with the
  pistol on a pig cop: the round ends without it; with monsters hidden,
  every pistol, shotgun, Gatling, rifle and freezer round on the wall
  called it). A short spark there; sparks within 600 units of one lit the
  same tic merge (shotgun).
- Weapons without rounds: the energy weapon (beam) and flamethrower fire no
  launch; Duke's hand flash (player word 0 bit 3) lights them when no launch
  of his came within a tic: green for the energy weapon, a flickering fire
  light 700 units ahead along the view for the flamethrower, warm otherwise.
- Rounds in flight (first part): RPG 8, freezer 13, energy round 15.
- Blasts sized per source: barrel 9000 / 20 tics / 1.10, dynamite 8500 / 18
  / 1.05, pipe bomb or Holy Hand Grenade (`0x800707cc`, one route) 7600 /
  16, rocket 7000 / 16, gib 3800 / 8 / 0.65.

Projectile types (lab `recomp/analysis/d41b/lab.py probe`, level 0, every
weapon fired with monsters hidden; local only):

| Type | Shot | Muzzle (rgb, radius, tics) | Impact | Flight |
| ---: | --- | --- | --- | --- |
| 0 | Desert Eagle | 0.85 0.58 0.28, 3400, 3 | warm, 1500, 2 | - |
| 1 | Combat Shotgun pellets | 0.95 0.62 0.30, 4400, 3 | warm, 1100, 2 | - |
| 2 | pig cop rounds | 0.85 0.58 0.28, 2800 (x0.8), 3 | warm, 1200, 2 | - |
| 6 | Buffalo Rifle | 1.00 0.64 0.30, 4600, 3 | warm, 1600, 2 | - |
| 7 | Gatling | 0.85 0.60 0.30, 3000, 2 | warm, 1300, 2 | - |
| 8 | RPG | 1.00 0.62 0.30, 4800, 4 | (blast) | warm, 2800 |
| 13 | Freezer | 0.55 0.50 1.25, 3000, 3 | violet-blue, 1900, 4 | violet-blue, 1800 |
| 15 | energy round | 0.45 1.15 0.40, 3200, 3 | green, 2300, 4 | green, 2200 |
| 4, 11, 12, 16, 17, 18 | pipe bomb, Holy Hand Grenade, dynamite, throwing knife, axe, crossbow bolt | none | none | none |
| other | enemy guns not probed (turrets, flyers, bosses) | 0.80 0.56 0.28, 3000, 3 | warm, 1200, 2 | - |

Values follow Quake's short muzzle flash (about 0.1 s) and EDuke32 Polymer's
orange muzzle light, scaled to the D41A first values; all are first values
for the playtest.

Evidence (offscreen, private apartment card copies, `DNTTK_LIGHT_AMBIENT=55`
for stills, local):

- Build OK. `ttk-near-test` PASS, `ttk-flash-test`, `ttk-live-env-test`,
  `test_player_profiles` OK; `ttk-controls-test` stops at the known D14
  `hud_intact` check (line 1755); no test source changed.
- Pistol, flamethrower, energy weapon, freezer, shotgun: the flash lights
  Duke and the ground around the gun in the weapon's colour (energy weapon
  changed from cyan to green and freezer to violet-blue after comparing
  with the game's own effects). Pig cop by the club door: its shot lights
  the wall and door frame around it for that tic.
- 180 Hz, Gatling and pistol with pig cops firing: 4720 presents, 0 missed
  redraws, 0 failed captures.
- Before/after stills: `research/before-after/d41b-shot-lights/` (local;
  the freezer still predates the violet tweak).

Limits (moved to D41I): burning enemies and fire (no source found yet), the TTK flamethrower's
flames themselves (only the jet light near Duke), enemy weapons other than
the pig cop's type 2 take the default warm values, impacts on characters
make no light (blood), and light still passes through walls (D41F).


## 11. D41I - Fire and enemy weapon lights (accepted and closed 2026-11-08)

Sources (`recomp/src/ttk/dynamic_lights.inc`; Modernized Polymorph with
DYNAMIC LIGHTS on; no framework change, no new game hook):

- **Burning objects.** The actor draw adds flame sprites on an object's
  joints when its flag word (+0) has bit `0x800` (the actor draw's call at
  `0x80033868` to `0x8003012c`; a second draw path, `0x80032304` to
  `0x8003038c`, checks the same bit; Duke's own burning was not
  reproduced in the lab). Each game tick the placement rows (`*0x800de720`, 48
  bytes, +4 the live object, up to the count `0x800de724`) and Duke are
  checked; the nearest five burning objects each carry a fire light 450
  above their feet: (1.15, 0.58, 0.22) x flicker 0.8-1.2, radius about 3600,
  changing every tick. An object counts as burning while bit `0x800` is set
  and bit `0x4` is clear: a burning pig cop reads `0x803` for about 6 s,
  a burning tree `0x802`, and both turn `0x804` when the burn is over (the
  body charred, the tree burned away; some props read `0x804` from the
  level's start). The first version tested `0x800` alone, and the user found
  a burned-away tree in UI slot 8 still lighting the wall (fixed before
  acceptance). Duke's
  bit stayed clear through walking, running, jumping, crouching and firing
  all ten weapon keys.
- **Flames in flight.** The flamethrower (type 14) and hi-temp flamethrower
  (type 26) flames are projectile pool records with flag bit 0 (not the
  value 3 that flying rounds use), up to 8 at once; the pool's draw loop
  `0x8006d08c` sends them to the flame sprite draw `0x8006cfd8`. Three of
  them, spread along the stream, carry a flickering fire light (0.75 x
  flicker, radius about 2400). The D41B nozzle light stays.
- **Enemy weapons.** Census (lab `recomp/analysis/d41i/lab.py enemy`, one
  spawned enemy per level, god mode, 7-14 s): type 26 enemies fire
  projectile type 3; turrets 204 and 540 type 5; robotic turret 622 type
  29; robotic turret 623 type 33 (rockets, the RPG's model `0x34b`); the
  level 3 gunship 605 type 31; type 95 type 34; boss 61 type 32; Moloch
  (62) type 35 from its twin gatlings. Flyers (44), the vest pig (1044) and
  types 847, 848 and 901 did not fire in the window. Values:

| Type | Shooter | Muzzle (rgb, radius, tics) | Impact | Flight |
| ---: | --- | --- | --- | --- |
| 3 | type 26 | 0.85 0.58 0.28, 3400, 3 | warm, 1300, 2 | - |
| 5 | turrets 204, 540 | 0.85 0.60 0.30, 3200, 2 | warm, 1300, 2 | - |
| 29 | robotic turret 622 | 0.85 0.60 0.30, 3200, 2 | warm, 1300, 2 | - |
| 31 | gunship 605 | 0.95 0.55 1.20, 3800, 3 | violet, 1600, 3 | - |
| 32 | boss 61 | 1.00 0.62 0.30, 4400, 3 | warm, 1600, 2 | - |
| 34 | type 95 | 0.85 0.58 0.28, 3400, 3 | warm, 1300, 2 | - |
| 35 | Moloch | 1.00 0.62 0.30, 4400, 2 | warm, 1600, 2 | - |
| 24, 33 | rockets (623 fires 33) | as the RPG | (blast) | warm, 2800 |
| 28 | incendiary RPG | 1.05 0.55 0.22, 4800, 4 | (blast) | 1.05 0.50 0.18, 3200 |
| 14, 26 | flamethrowers | none (hand flash) | none | fire lights above |

  Enemy flashes keep D41B's 0.8 x radius. The gunship's bolts were set
  violet after measuring the game's own glow when they reach Duke (about
  R76 G46 B77 added over the tunnel walls); the others show no coloured
  effect of their own and stay warm.

The projectile type table `0x800c4bc0` (37 entries of 64 bytes, indexed by
type) explains the families: word 4's high half is a category (2 rounds, 3
shotgun, 4 thrown and rockets, 5 flames, 6 energy, 7 and 0x14 freezer-like,
0x11 rifle) and word 1's high half the model (`0x34b` rocket, `0x799`
freezer, `0x76a` shared by types 27 and 35). Types 19, 22, 23, 25 and 30 are
also category 5 but were not seen; they take the default warm flash.

Console: `light fire off|on` (session) for matched comparisons. Developer
diagnostics: `DNTTK_PARTICLE_TRACE=1` logs the live particle list (head
`0x800dd7f0`, 0x80-byte records, +8 kind, +0x10 position) and burning
objects once a second; `DNTTK_LIGHT_PROBE=<hex>[,<hex>]` counts entries of
game functions that already carry the mod hook (other functions need a
code generation change).

User: "When I burn an enemy the lighting effects are just beautiful";
"the lighting looks spectacular".

Evidence (offscreen, private apartment card copies, level 0 street at
`DNTTK_LIGHT_AMBIENT=55`, local `recomp/analysis/d41i/`):

- Build OK. `ttk-near-test` PASS, `ttk-flash-test` and `ttk-live-env-test`
  PASS, `test_player_profiles` 133 OK; `ttk-controls-test` stops at the
  known D14 `hud_intact` check (line 1759).
- Matched stills of the same moment with `light fire off` and `on`: a
  burning pig cop adds a warm pool on the street around it (lower half
  +3.4 to +4.5 levels, 8-14% of pixels brighter by more than 10); the
  stream adds less (lower half up to +2), the nozzle light already
  covering most of it. `research/before-after/d41i-fire-lights/` (local).
- 180 Hz with the stream and a burn: 7868 presents, 0 missed redraws, 0
  failed captures.
- Gunship 605 in the level 3 tunnel: its launches log type 31 (the violet
  entry, set after this capture measured its glow).

Limits: the burn light follows the flag, so it lasts the whole burn even
when the visible flames have shrunk; light still passes through walls
(D41F); the laser gatling (27) and the unseen category 5 types take the
default warm flash; level fires (braziers, burning barrels) light only if
the game sets the same flag on them, and lamps and other emitters are D41D.

## 12. D41J - Upgraded weapon light colours (accepted and closed 2026-11-09)

User, D10H9 playtest: "the upgraded flamethrower glows blue, but our
lighting doesnt change from the regular flamethrower"; scope added: "also
check out the upgraded laser gatling gun, make sure the colors on the laser
lights are right too".

- **Owners.** The held byte `player+0x3b9` stays 7 or 9; `0x8003df40`
  resolves it with the inventory upgrade bit 8 (`+0x2c4+4*slot`) to record
  28 (Laser Gatling) or 27 (HiTemp). The hand flame draw (`0x8003cd30`)
  picks blue sprites for record 27; the flame record draw `0x8006cfd8`
  picks blue sprites for pool type 26. The Laser Gatling launches
  projectile type 27, which had no entry and took the default warm flash
  and spark.
- **Burns.** A pig cop set alight by the HiTemp reads flag `0x803`, the same
  as one lit by the regular flamethrower, and burns with the same orange
  joint sprites (matched lab stills), so burning objects keep the warm fire
  light.
- **Values.** Colours sampled from offscreen stills (blue or violet pixels of
  the HiTemp flame: 0.67 0.24 1.00; the laser bolts: 1.00 0.18 0.41), scaled
  like the warm fire values. HiTemp hand flash 0.75 0.38 1.20 x flicker,
  radius 5200; HiTemp flames in flight 0.72 0.34 1.18 x flicker, radius
  about 2400. Laser Gatling (type 27): muzzle 1.20 0.26 0.52, 3000, 2 tics;
  spark 1.25 0.32 0.56, 1300, 2 tics; in flight 1.05 0.22 0.45, 1500.
- **Evidence.** Lab `recomp/analysis/d41j/lab.py shots` (`UP=1` upgrade by
  `dnupgrade`, `WANT=9` or `7`), 60 Hz third person and 180 Hz first person,
  before and after: the HiTemp stream lights the street blue-violet, the
  Laser Gatling's muzzle and wall hits light pink; the regular flamethrower,
  regular Gatling (type 7) and burning pig cops are unchanged.
  `ttk-input-test` and `ttk-aim-test` pass. Executable
  `d4a2a11f5f052be8a105acad30122ce16b5d4d812a9e51ccb5691e3933f5ad36`.
- **Accepted.** User: "this current job at hand is fully accepted". The first-person nozzle pilot flame of the
  HiTemp is drawn orange by the game itself (not a light); unchanged.

## 13. D41J1 - Laser Gatling light reads red (accepted and closed 2026-11-09)

User: "the laser gatling gun i think reads more white in hts projectile
lighting ... can we give more of a red tone".

- **Cause.** One type 27 light was the sampled red-pink; the shader adds it
  correctly (a pure red console test light adds about 44 red, 1.5 green).
  The Laser Gatling's fire rate overlaps several muzzle flashes and bolt
  glows near the gun; the soft shoulder caps red near 1.1 while green and
  blue grow linearly, so the stack turned pale pink-white, and the blue-grey
  street added lavender.
- **Values.** Type 27 muzzle 1.25 0.05 0.02 (3000, 2 tics), spark 1.30 0.07
  0.03 (1300, 2 tics), in flight 1.05 0.04 0.02 (1500). Keeping some green
  and blue (0.12-0.14, 0.05-0.08) still stacked to salmon in first person.
- **Rule for coloured lights that fire fast.** Keep the off channels near
  zero; the shoulder means a stack saturates the main channel and then only
  the others can grow.
- **Evidence.** `recomp/analysis/d41j/lab.py shots` and `probe`, 180 Hz first
  person and 60 Hz third person, before/after in
  `research/before-after/d41j1/` (local).
- **Accepted.** User: "lock it in and close it out".

## 14. D41K - Props in dynamic light (accepted and closed 2026-11-09)

The soft shoulder of section 5 is replaced: the painted colour stays as it
is and only the added light is soft-limited, on its brightest channel (hue
kept). The D41D1 neon rule leaves only neon faces unlit, not their whole
prop. [Note 434](434-d41k-props-in-light.md).
