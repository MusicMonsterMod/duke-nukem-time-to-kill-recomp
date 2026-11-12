# 450 - D41N moving lights without shadow-map churn; D23S console follow-up

Date: 2026-11-12. Status: Done (both jobs; user-accepted 2026-11-12: "colossally huge improvement in resource usage ... a significant drop in GPU usage in regular gameplay"). Lab:
`recomp/analysis/d41n/lab.py` (local; private card copies, the player's own
profile copied read-only, player files hashed before and after every run).
Before/after shots: `research/before-after/D41N/` (local).

## 1. The reports

- D41N (user, 2026-11-11, UI slot 6, Necros underwater on the level of UI
  slot 2): GPU "up to 70%" while the Necros fire, "as soon as the necros have
  been killed the GPU drops down into the 20s".
- D23S follow-up (user, 2026-11-12): after the D23S fix, "opening the console
  still adds about 60" (unit unknown: GPU memory MB or GPU %; measure both).

## 2. D23S follow-up: the console costs nothing now; the "60" was D41N

Measured with GPU utilization (weighted by clock) and GPU memory, console
closed, open, typed into, closed again, 6-8 s each:

| Where | Closed | Open | Typed | Closed again | Memory |
| --- | --- | --- | --- | --- | --- |
| Offscreen, 180 Hz, UI slot 2 | 33.8% | 33.6% / 32.1% | 34.2% / 32.2% | 33.9% / 30.3% | 1819 -> 1823 MiB |
| Offscreen, pause menu | 5.2% | 5.7% | - | 4.1% | +4 MiB |
| Real display (windowed, 180 Hz), UI slot 2 | 35.6% | 36.5% / 35.9% | 35.1% / 36.1% | 35.7% / 34.6% | game process 1441 -> 1445 MiB |
| Real display, pause menu | 11.6% | 11.6% | - | 11.6% | +4 MiB |
| Real display, UI slot 6 (Necros) | 63.1 / 60.7 / 64.6% | 57.2 / 61.5 / 63.5% | - | 62.0% | +4 MiB |

The real-display runs were one windowed launch each of the normal build
(player presentation, mouse captured), quit through the debug port; the
kernel log showed no display-engine timeouts before or after.

Finding: with D23S's own console texture (uploaded only when the console
changes), opening the console no longer changes GPU work anywhere, on the
real display either, and adds 4 MiB (its 1280 x 440 image). In UI slot 6 the
GPU is about 60% with the console open and closed alike: that is the Necro
bolt cost (section 3), which the user saw while the console was open. The
only remaining console cost was the texture kept after closing; it is now
deleted once the console has fully retracted (`gpu_gl_renderer.c`, the
present's console block), so a closed console holds no GPU memory.

Not done (measured gain zero): drawing the console as glyph quads from a font
atlas, as Quake (`Draw_Character`, a 16 x 16 `char_texture`) and Quake III
(`SCR_DrawSmallChar`) do. It would save the 4 MiB while the console is open
and the 2.25 MB upload per keystroke; no GPU time is spent on it today.

## 3. D41N: what cost the GPU

UI slot 6, offscreen, 180 Hz, `PSX_GPU_PHASES=2` (GPU time per redraw):

| Config | GPU work | "other" (shadow maps) | tex (shading) | lights |
| --- | --- | --- | --- | --- |
| Before (bolts shadowed) | 60.7% (max 69.1%) | 4.48 ms | 2.07 ms | 14 |
| `light bolts off` | 22.4% | 0.02 ms | 0.88 ms | 6 |
| Shadows off | 24.0% | 0.02 ms | 1.04 ms | 8 |

Cause: a light that moves gets a new shadow slot every game tick (its slot
is keyed by position), so each of the eight bolts in flight needed a new cube
map (6 faces of the room mesh, plus the SMOOTH edge passes) at 60 maps per
bolt a second. Each renderer (the emulation thread and every redraw worker,
separate processes) draws every map whose stamp it has not yet seen, so the
same map was drawn in each process. Out of new maps for the tick
(`shadow_new_per_tick`) or slots (16), the remaining bolts went dark for that
tick.

The same churn in the sweep (UI slot 2's level, fixed view, old against new,
alternating; "new" is new shadow slots a second):

| Effect | Before | New |
| --- | --- | --- |
| Freezer | 43.2% / 43.4% (57 new/s) | 35.7% / 34.8% (0) |
| Energy weapon | 45.6% / 46.4% (23 new/s) | 41.9% / 42.1% (0) |
| Flamethrower on a burning Necro | 37.6% / 47.4% (48-75 new/s) | 24.0% / 27.3% (0) |
| RPG (one rocket a window) | 36.0% (9 new/s) | 35.0% (0) |
| Gatling, pipe bombs | no moving maps either way | unchanged (blasts 0.3 new/s, one map each) |

Muzzle flashes and impact sparks were never shadowed; blasts hold still and
shrink, so each keeps one map for its life (unchanged).

## 4. Research: how shipped games treat effect lights

- id Tech 4 (Doom 3, GPL source `neo/game/Projectile.cpp`, `Weapon.cpp`,
  `renderer/tr_light.cpp`): projectile, explosion and muzzle lights take a
  light material; the `muzzleflash` material (used by the rocket launcher's
  flash and explosion light, `def/weapon_rocketlauncher.def`) and the imp
  fireball's flying light (`lights/impflyflash`) begin with `noshadows`, and
  the renderer skips shadow work when `!LightCastsShadows()`.
- EDuke32 Polymer (`source/build/src/polymer.cpp`, `source/duke3d/src/actors.cpp`):
  every game effect light (`G_AddGameLight` for projectile flashes, weapon
  flashes, explosions, fire, freeze and shrinker sparks) is a point light
  (`lightRadius` 0), and `polymer_prepareshadows` only gives shadow maps to
  spotlights in view (`r_pr_shadowcount`, default 5), so all of them are
  unshadowed. Leaks are held back by sector culling: `polymer_culllight`
  floods from the light's sector through open portals only.
- Unreal Engine ("Movable Light Mobility"): three shadowed movable point
  lights can take "upwards of 14 milliseconds" of shadow depths, about 1 ms
  cached, and "lights that move around will not have their shadows cached".
- Unity (LightPerformance; HDRP shadow update modes): a point light's shadow
  is six cube faces, "quite costly"; cached atlases redraw only on change.
- DOOM (2016) (Courreges's graphics study of the SIGGRAPH 2016 talk): one
  shadow atlas, static light maps kept between frames, regenerated only when
  something moves under the light.

So the industry rule is the one adopted here: shadows for lights that hold
still (cached), no shadows for short-lived moving effect lights.

## 5. Change

`recomp/src/ttk/dynamic_lights.inc` `lights_publish`: only lights that hold
still are shadow candidates - placed lamps, TV screens, prop lamps, blasts and
test lights. Rounds and bolts in flight (`lights_flight`), flames in flight
and burning objects (`light_fire`) glow unshadowed, as muzzle flashes and
sparks always did. `light_shadows.inc`: session toggle `light shadows moving
on|off|default` (developer `DNTTK_SHADOWS_MOVING=on` at launch) for matched
comparisons, and the `light shadows` reply now gives new slots, deferred
slots and the moving-light state. `gpu_gl_renderer.c`: the closed console's
texture is deleted (section 2). No shader change; the unshadowed path is the
existing one (a few multiplies a light after the D41G per-triangle light
mask). Framework patch exported.

## 6. Results

UI slot 6 with the Necros firing (8-10 s windows, old and new alternating):

| Rate | Before | New | Bolt lights off | Bolt lights on, later (fewer bolts) |
| --- | --- | --- | --- | --- |
| 120 Hz offscreen | 48.6% / 40.6% | 17.1% / 17.1% | 13.5% | 19.0% |
| 180 Hz offscreen | 61.4% / 55.1% | 23.9% / 22.0% (steady 21-26%) | 19.0% / 22.5% | 21.1% |
| 240 Hz offscreen | 72.6% / 68.7% | 30.4% / 30.1% | 25.4% | 27.7% |
| 180 Hz real display | 64.3% / 56.1% | 28.1% / 28.3%; 26.8% / 25.6% (steady 24-30%) | 21.7% | - |

Presents held at the rate in every new window (234 of 240 before). The
remaining 2-5 points against no bolt lights are the shading of up to eight
large violet lights and the bolt sprites themselves. Single 67% samples in
real-display new windows were carry-over from the preceding old window
(nvidia-smi averages over its sample period); two new-only runs had none.

Look (matched savestate replays in UI slot 6 at 80 and 120 game frames, and
alternating toggles with the flamethrower firing at a wall): the bolts light
the corridor the same; with the flamethrower the walls get a slightly fuller
warm glow, because no flame light now goes dark for want of a map. The
unshadowed fire is the D41I look the user accepted before D41F shadows
existed (2026-11-08).

Unchanged: ORIGINAL publishes no lights (0 shadowed, no new slots with the
old behaviour forced on); Classic and Vanilla refuse the light commands
(code unreachable there); the Vanilla console opens and closes at 8.7-8.8%.
`ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test` PASS.

## 7. Limits

- An unshadowed moving light can light the far side of a thin wall when it
  flies or bursts next to it, as muzzle flashes and sparks always could. Not
  seen in the shots; Polymer's answer (a sector flood from the light) would
  need the room of each world triangle in the renderer and is not built.
- The RPG fires one rocket per held click in the lab, so rocket churn was
  measured over single rockets; pipe-bomb blasts showed one map per blast.
- Offscreen figures exclude the window present and compositor; the real
  display adds about 4 points (section 6).
- The player's own look check and play in heavier fights are the open items.
