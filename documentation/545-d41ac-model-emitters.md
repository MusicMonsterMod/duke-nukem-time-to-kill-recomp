# D41AC - Only light fixtures glow as emitters

2026-11-23. **Done, user-accepted** ("lock it in and close it out. the signs look amazing!"; the world side left as it is: "i like how the gold gets picked up by the light"). Job: [D41AC](../jobs/lighting.md#d41ac---relit-emitter-test-catches-saturated-model-colours).

User, 2026-11-22 (D57L, UI slot 12, copy in `research/savestates/d57l/red/`):
"the red of duke's shirt and the med kit red, seem to be quite glowy in this
dim light. i noticed that in the strip club too?"

Private runs only: lab `recomp/analysis/d41ac/` (local; the D57L probe with
private card copies of the apartment save and the strip club save
`research/savestates/d57j/`; offscreen on the real GPU at 180 Hz, debug
port). The player's saves and settings were hashed before and after every
run: unchanged.

## The rule before (MEASURED)

The world shader's lit path (`recomp/psxrecomp/runtime/src/gpu_gl_renderer.c`,
D41D) keeps any texel that is bright and strongly coloured at its unlit look:
`em = smoothstep(0.3, 0.55, max) * smoothstep(0.6, 0.85, saturation)`, mixed
toward `rgb * v_col * 2`, skipping Relit's dimmed ambient and the lamps. Meant
for sign letters and neon.

A developer view shows what it catches: `PSX_EMITTER_VIEW=1` tints emitter
texels magenta on the world and green on models (shading normals), over
grey; `PSX_EMITTER_VIEW=2` shows the raw texel, unlit. Both are compiled in
only when set (no cost otherwise).

What it caught, by surface (raw texel max and saturation, medians):

| Surface | Kind | max | saturation |
| --- | --- | --- | --- |
| Duke's red shirt | model | 0.82 | 0.83 |
| Medkit cross | model | 0.81 | 1.00 |
| Apartment door (in the mirror) | model | 0.48 | 0.73 |
| Club neon tubes | model (fixture) | 0.91 | 1.00 |
| Club pole canopies | model (fixture) | 0.48 | 0.73 |
| Club tabletops | model | 0.77 | 0.83 |
| Club dancers | model | 0.59 | 0.90 |
| Club gold wall panels | world | 0.60 | 0.95 |

The texel values do not separate paint from light: the shirt and the cross
sit inside the neon's range. A tighter threshold would cut the canopies
before the shirt. The rule has to be by surface.

The skin question (from D57L: "Duke's skin 94%, not an emitter; check
whether models are dimmed less than the world"): with matched regions, Duke's
arm keeps 58-60% of its Original brightness in Relit, the walls 55%. Models
dim like the world. The 94% was a region that caught lamp-lit pixels.

## The rule now

A model is an emitter only if it is a light fixture: a mesh the prop-light
system already knows by model, game-wide (`prop_light_kind`, D41L/D57: the
woodburner, chandeliers, barricade lamp, caged lamp, subway globe and train,
street lamp, the club's neon bar, pole canopy and back bar, the table lamp,
the hanging lamp). Every other model (Duke and his weapons, pickups,
enemies, doors, plants, furniture) dims with the room. World faces (signs,
neon letters) keep the texel test as before. No per-level data.

- Title (`recomp/src/ttk/near_clip.cpp`): an object draw whose mesh has no
  prop-light kind sets `no_emitter`; its corners carry
  `PSX_MOD_SURF_NO_EMITTER` (bit 31) in the surface word of the host vertex
  calls. The surface word already rides HostPV and the redraw records, so
  redraws keep the mark with no new record kind.
- Framework (`mod_plugins.h`, `gpu_gl_renderer.c`): `gl_set_host_surface`
  takes the bit off before the surface index is used (the bake bounds check
  is unchanged); the triangle's `v_lit` is 2 instead of 1, and the shader's
  `em` is 0 there.
- Console `lamp emitters all` (session) brings back the old rule for
  comparison; `lamp emitters fixtures` restores it. Developer
  `DNTTK_MODEL_EMITTERS=all` starts with the old rule.
- The switch reaches the redraw workers through `RenderState`
  (`emitters_fixtures`): they are forked at startup and kept the startup
  value otherwise (found in the lab: the console flipped only the main
  process, and every presented image at 180 Hz is a worker redraw).

## Results (MEASURED)

Relit, percent of the Original pixel value, same savestate:

| Region | Before | After |
| --- | --- | --- |
| Apartment walls | 55% | 55% |
| Duke's red shirt (red pixels) | 90% | 53% |
| Duke's arm | 58% | 60% |
| Medkit cross (red pixels) | 101% | 101% |

The medkit is not lit by the emitter rule any more: it sits in a baked lamp
pool. `light bake off` drops it to 55%. That is real lamp light (the apartment's
table lamp), and it is the same under either rule.

Strip club: the neon tubes and pole canopies keep their glow; the tabletops
and the dancers now dim with the room. The gold wall panels (world) are
unchanged.

Survey, all 21 selectable levels (each level's start, Relit, emitter view,
old rule then new): model emitters remain only on fixtures. What stopped
glowing: level 0's STOP sign, level 9's street bushes, level 12's urn plants,
the club's tables and dancers, Duke, pickups. Level 3's hanging lanterns
(not in the fixture list) look the same either way at the start: their red
caps sit in strong warm lamp light.

Original: pixel-identical between the two rules outside Duke's animating
mirror figure (shirt 128.6 vs 128.4), as the shader rule runs only when
dynamic light is active. Vanilla: the save loads, no lighting activity.

Cost, 180 Hz, the apartment, three pairs old/new: presentation pacing p50
and p95 6 ms in every pass, max 7-8 ms, no image over 10 ms. GPU
(clock-weighted) 22.1/22.4, 21.1/27.4, 40.0/38.9: no consistent difference;
the drift follows host load (a browser using most of a core during the
runs). The shader adds one compare in the lit path.

Before/after shots: `research/before-after/D41AC/` (local): the apartment
mirror and the strip club, Original, Relit before and after, and the
emitter view before and after.

## Playtest 1 (2026-11-23): the EXIT sign and the Bootylicious sign

User: "the stuff in the apartment now looks great! but there's a couple of
issues it has caused - check slot 10 - the exit sign is now illegeble. and
then check slot 9, the bootylicious sign doesnt actually look illuminated at
all." Private copies of the player's slots (UI 10 and 9 = slots 9 and 8,
both level 0).

EXIT sign (UI slot 10): a model, not a known fixture, so the new rule lit it
like paint; its own sign lamp (D41D1) then washed it into a red blob (red
130% of Original, letters gone). Caused by this job.

Bootylicious sign (UI slot 9): equally dim under both rules (red 55% of
Original). Not caused by this job: an older gap. D41K's neon faces ("neon
signs take no light") are found by the neon scan about 45 ticks after a
level load or savestate load, but the redraw workers are forked copies taken
at the load and never received the list. Every 180 Hz redraw drew the
sign's letters lit, and the club's 30% ambient (D57J) dimmed them. Only the
live image (one in three at 180 Hz) had them right.

Fixes:

- The neon scan (`neon_scan`, `recomp/src/ttk/level_lights.inc`) also
  returns the glowing static props: meshes whose glowing area reaches
  `sign_min_glow` by the sign-lamp weights (D41D1's rule for giving a prop
  a lamp; non-animated faces). Those keep the emitter rule like the prop-light
  fixtures (`near_clip_set_glow_meshes`). Level 0: 34 props (the EXIT boxes,
  the club's bar, street and traffic signals, the neon bar).
- When the scan's result arrives, the redraw workers are refreshed
  (`psx_mod_replay_workers_refresh`, once per level or savestate load) so
  redraws hold the neon faces and the glowing props. Developer
  `DNTTK_NEON_REFORK=0` skips it; `DNTTK_NEON_TRACE=1` lists each scanned prop.

Results (MEASURED, Relit, percent of Original):

| Sign | Before the fix | Refresh off | After |
| --- | --- | --- | --- |
| EXIT (red) | 102% (old rule) / 130% blob (new rule) | 130% blob | 102%, letters legible |
| Bootylicious (red) | 55% | 55% | 88%, drawn as its own neon |

Duke's shirt stays at 53% (walls 55%). In the club the table tops now draw
unlit as well: D41K always counted them as neon faces, but before this fix
that held only in the live image, so the club's tables and sign are now the
same in every image (a visible change to the D57J look, for the user's eye).

## Open (not changed here)

The world side of the same test catches more than signs: whole saturated
floors and walls keep their unlit look in Relit (the level 3 subway floor,
level 12's red floor tiles, level 27's rock walls, the club's gold wall
panels, a level 0 overhang). They skip Relit's dimming entirely. That is a
separate choice for the user (world emitters by a list of sign and neon
faces, a tighter world threshold, or leave as is); not part of this job's
acceptance.
