# 471 - D41P2, D41S and D23N2B on the field laptop

Session of 2026-11-14 (afternoon). User: "work on the D41P follow-up, then
D41S, then D23N2B, on the field laptop, in that order, autonomously. Don't
stop to ask; make the sensible call, write down why". D41P2 and D41S built
(Needs playtest), D23N2B pieces 4c and 4d measured (In progress). Nothing
committed.

Machine: the field laptop of notes 461 and 468-470 (i7-5600U, 2 cores and 4
threads, HD Graphics 5500, 15 W). Host checked first: load 0.3-0.7, on AC,
no backup, update or other game running; the user idle for the real-display
runs (`idle.py` before each, as `abc.sh` does). Lab: `recomp/analysis/d23y6/`
(local), the player's profile copied read only (Enhanced, LIVE SHADOWS OFF,
FRAME RATE 120, 2x, extended), the lab's private card copy; private slot 11
was used for the scenes built here (never the player's saves, cards or saved
profile). Lab slot N is UI slot N+1; DH5 is the copy of UI slot 5 (DUKE
HILL's town). Evidence labels: MEASURED, CODE, INFERRED.

| Job | Result |
| --- | --- |
| D41P2 | Fixed lamps only through the bake; built, Needs playtest |
| D41S | Planar model shadows and glows for LIVE SHADOWS OFF, with settings; built, Needs playtest |
| D23N2B | Piece 4c (from the earlier job's record) exact; piece 4d (other transform callers) measured; In progress |

## 1. D41P2: the fixed lamps only through the bake (MEASURED, built)

**Measured first.** The job's wording ("water, fills") came from note 470's
guess. A census of lit polygons (developer `DNTTK_BAKE_CENSUS=1`, per
process every 5 s) showed instead: water is not lit at all (not part of the
lit scene), every world face has its bake, and what still took the fixed
lamps live was flat prop faces (1-3% of lit polygons, UI 8, 9, DH5) and
meshes whose origin found no floor (UI 2: about 8% of model polygons;
room-attached meshes whose origin is their room's corner, outside or above
the floors).

**Built** (Modernized, Polymorph, LIGHTING ENHANCED or RELIT, a bake shown):

- `near_clip.cpp` (`emit`): a mesh with no floor under its origin, and any
  flat face without a bake of its own, takes the floor under the polygon's
  own centre (a flat prop face takes its mesh's floor first). A flat face
  is sent with its face normal at length 0.3 (two-sided, as the shader
  derives it), so the shader reads the floor chart's lit side as for a
  model (`s_model`). No framework change.
- `lamp_bake_map.cpp` (`lamp_bake_floor`): a cell with no floor under the
  point looks in the 8 cells around (a floor narrower than a cell: a beam, a
  step). No-floor share: UI 2 8% -> 3%, UI 8, 9, DH5 0.
- `lamp_bake.inc` (`lamp_bake_d41p`): with a bake shown, the fixed lamps
  leave the live light list (no shadow candidacy, no shader loop): note
  470's `nolamps` measurement mode is now the default. A model over a pit or
  beyond the level's floors takes its painted light (Quake: a model with no
  light point under it).
- Developer: `DNTTK_D41P=live` (D41P as accepted: the lamps listed with
  maps), `DNTTK_D41P=noshadow`, `DNTTK_POLY_FLOOR=0`, `DNTTK_SHADOW_COUNT=1`
  (every 300 ticks: lights listed, fixed lamps and those with a map, maps
  drawn).

**Counts** (`DNTTK_SHADOW_COUNT=1`, LIVE SHADOWS SIMPLE, after the load):

| View | fixed lamps listed with a map, before | after | other lights shadowed |
| --- | --- | --- | --- |
| UI 8 | 10 | 0 | 0 |
| UI 2 | 5 | 0 | 1 |
| UI 9 | 12 | 0 | 0 |
| DH5 | 10 of 12 (18 lights for 16 slots: two dimmed) | 0 | 6 |

The maps are drawn once and cached (later intervals draw 0-3), so the
saving each frame is the shader's light loop and shadow lookups on models.

**Cost**, GPU per redraw (`PSX_GPU_PHASES=2`), offscreen at 180, 3
interleaved runs each; work = scene ms x GPU clock (as note 470), raw scene
ms beside it because the clock varies by run:

| View | player's OFF: work / raw ms | SIMPLE: work / raw ms |
| --- | --- | --- |
| UI 2 | -1.0% / 12.96 -> 12.97 | -7.7% / 13.00 -> 12.98 |
| UI 8 | -2.9% / 9.57 -> 9.47 | -6.5% / 9.59 -> 9.27 |
| UI 9 | -5.5% / 13.37 -> 13.46 | +3.7% / 13.30 -> 13.32 |
| DH5 | +8.1% / 9.24 -> 9.35 | +3.3% / 9.20 -> 9.21 |

CPU (all threads, emulation thread) unchanged in every case (saturated at
180). So: at the player's LIVE SHADOWS OFF within the noise (expected: the
lamps already drew no maps there); at SIMPLE a few percent in two of four
views, within the noise in the other two. Note 470's "-5% to -8%" for
`noshadow` was partly the clock weighting (its raw ms: UI 2 +2.4%, UI 8 -5%,
UI 9 -2.6%).

A lab pitfall found on the way: the agent's shell is zsh, which does not
split an unquoted `$e`; inline loops passing two `LAB_` variables in one
`$e` set only the first. The A/B scripts are bash (`#!/bin/bash`) and were
not affected; inline loops now run under `bash -c`.

**Pictures** (matched stills 240 frames after the load, offscreen, D41S
off, old = `DNTTK_POLY_FLOOR=0 DNTTK_D41P=live`):

| View | OFF | SIMPLE |
| --- | --- | --- |
| UI 2 | 0.00% | 0.00% |
| UI 8 | 2.63% (two old runs differ 8.6% here: noise) | 0.78% darker: the blood pools on the floor take the floor's bake |
| UI 9 | 0.00% | 1.34% faintly brighter (mean difference 0.24 of 255) |
| DH5 | 0.00% | 0.00% |

The baked lamp shadows on walls are unchanged in every view. SHARP and
SIMPLE keep their live shadows for blasts, flickering and breakable lamps,
TV screens and prop lamps; Classic, LIGHTING ORIGINAL and Vanilla have no
bake (CODE: unchanged).

## 2. D41S: Quake-style model shadows and glows (MEASURED, built)

Learned from the ports on this laptop (their configs read, not changed):
Hammer of Thyrion (Hexen II) has `r_shadows 1`, `gl_glows`,
`gl_missile_glows`, `gl_other_glows`; QSS-M (Alkaline) has
`r_shadows_groundcheck 1`, `r_shadows_bmodels 0` and a no-shadow list for
flames and bolts. No sources are installed; the method follows their
published code (INFERRED for these builds): models flattened along a light
direction onto the plane of the floor under them, drawn dark and
translucent, overlaps stopped by the stencil (QuakeSpasm); glows are
additive halos facing the camera.

**Built** (`near_clip.cpp`, `prop_lights.*`, `lamp_bake_map.*`,
`dynamic_lights.inc`; Modernized, Polymorph, LIGHTING ENHANCED or RELIT,
LIVE SHADOWS OFF only):

- **MODEL SHADOWS** (`model_shade`): each mesh of a moving object (the
  object renderer outside the static-prop loop `0x80032288`: enemies; not
  the sky, a TV, the first-person layer) is flattened along a fixed world
  direction (down, a quarter toward +x and a fifth toward +z) onto the plane
  of the floor under its lowest point. The D41P floor grid now keeps each
  entry's slope, so the exact height of a planar floor is known anywhere
  (`lamp_bake_floor_height`), and the shadow follows sloped floors. Ground
  check: each projected corner must stand over a floor lying on that plane
  within 24 units, in its own grid cell (no neighbours), else the face is
  left out: the shadow stops at a ledge or a pit. Drawn as textured faces
  with a black semi-transparent texel (mode 0: half the background) from
  the second texture bank, depth-tested and written 4 units above the floor.
- Overlaps: the PS1 mask bit (GP0 E6 set and check, the PS1's stencil) was
  tried first; nothing drew, because the GL renderer's stencil copy of the
  mask bit is written only while "set mask" is on and keeps stale ones
  (MEASURED: with the bracket no pixel changed, without it 0.57%). Instead a
  mesh's shadow faces share one ordering slot and each is lifted a further
  0.1 unit toward the eye than the one emitted before it (at most 64 ranks):
  the first drawn wins the depth test, so a mesh's own faces darken the
  floor once. Two meshes (limbs) can still overlap twice, as GLQuake
  without a stencil. Developer `DNTTK_SHADE_MASK=1` keeps the bracket for a
  later renderer fix; `DNTTK_SHADE_DEBUG=1` draws the faces opaque red
  without the ground check.
- **GLOWS** (`scene_glows`, once per image at its first room piece): an
  additive halo facing the camera at each light source in flight (the shot
  pool's rounds and bolts with an in-flight light: energy, freezer, laser,
  the Necro's blast) and each flame (flamethrowers, burning objects),
  tinted by its light, sized from its reach (rounds 0.10, flames 0.16 of it,
  120-900 units), drawn half its size toward the eye so a wall beside it
  does not cut it, faded in over two sizes from the eye. A neutral halo was
  added to the second bank beside the D41L flame and halo.
- Settings: OPTIONS > DISPLAY > MODEL SHADOWS and GLOWS (ON, OFF; default
  ON), `run.py --model-shadows on|off`, `--glows on|off`, the terminal
  settings line, profile schema 75 (migration adds both ON); Vanilla
  passes off. Developer `DNTTK_D41S_TRACE=1` (counters every 300 images).

Not covered (MEASURED): Duke himself casts none in third person (his body is
host-drawn only while the image carries live lights, D41C; with the fixed
lamps out of the list it is usually the game's own, which `object()` skips);
pickups and props cast none (the static-prop loop draws the pickups);
the RPG's rockets give no glow (they are not in the shot pool and have no
in-flight light). The menu rows were not driven by eye (scripted keys do not
reach the pause menu, as in note 470); the game-menu table is covered by the
build and the profile tests.

**Pictures**: a scene in DUKE HILL's town with a spawned enemy (type 57)
saved to private slot 11 and loaded under each setting: the shadow lies
behind the enemy's legs on the street (0.57% of pixels darker, nothing
else changes); on UI 8's sloped roof the floor is found (slope 0.5) but an
enemy on the narrow parapet partly finds none and is left without a shadow
(the ground check). Glows: the trace shows one source per flame or bolt and
a halo drawn; at the flamethrower's muzzle its own sprites swamp the halo,
and the Necro's bolts are already large glowing sprites, so the halos are a
modest addition there. Matched stills at load + 240 frames with the effects
off and on: DH5 and the three-enemy scene 0.00% (the shadow falls behind
the enemy from the camera), UI 8 0.01%, UI 9 0.72%, UI 2 13% (the water
layer's animation: the shots are 0.03 s apart in level time, as in note 470).

**Cost at the player's settings at 120**, real display, the player's
governor, 3 interleaved runs each (`abc.sh`, effects off against on, a
scene with three enemies ahead built with god mode on):

| View | game median / lowest tenth, off -> on | presents median / lowest tenth, off -> on | CPU clock |
| --- | --- | --- | --- |
| DH5 + 3 enemies | 60.0 / 56.9 -> 60.0 / 57.4 | 30 / 30 -> 30 / 30 | 2.59 / 2.65 GHz |
| DH5 | 60.0 / 56.7 -> 60.0 / 57.8 | 59 / 30 -> 44 / 30 | 2.30 / 2.66 GHz |
| UI 2 | 60.0 / 59.0 -> 60.0 / 58.9 | 60 / 59 -> 60 / 59 | 2.30 / 2.30 GHz |
| UI 8 | 59.4 / 55.9 -> 59.2 / 57.2 | 60 / 30 -> 60 / 59 | 2.29 / 2.20 GHz |
| UI 9 | 60.0 / 59.1 -> 60.0 / 59.4 | 60 / 30 -> 59 / 30 | 3.09 / 3.09 GHz |

Game frames are unchanged; presents swing both ways run to run (the
governor). GPU per redraw offscreen at 180 (3 interleaved runs, raw scene
ms): three enemies 11.09 -> 11.14 (+0.5%; +8% clock-weighted, the "on" runs
at a higher GPU clock), DH5 9.33 -> 9.12, UI 8 9.22 -> 9.24; CPU unchanged.
So the cost is within the noise, at most a few percent with several enemies
in view; each effect still has its own setting so it can be dropped.

**Correction (2026-11-14, later the same day): the off/on figures above are
void.** The lab passed the switches as `LAB_DNTTK_MODEL_SHADOWS=off` /
`LAB_DNTTK_GLOWS=off`, but the launcher sets both from the profile and
overrode them: every "off" run's log says "Model shadows: on; glows: on". The
real-display table and the offscreen 180 figures compared on with on. Redone
offscreen with the modes written into the lab's profile copy (`CTRL`, each
run's settings line checked: 9 runs per mode), section 2a below. The
real-display measurement at 120 waits for the player to be away from the
machine.

### 2a. D41S2: playtest fixes, MODEL SHADOWS ROUND, the cost redone

Playtest (user, 2026-11-14): the shadows show under enemies, the flamethrower
glow works, GLOWS OFF and MODEL SHADOWS OFF are kept after a relaunch.
Reported: MODEL SHADOWS OFF did not remove the shadows until a relaunch, and
no shadows in the pause menu's frozen frame.

- **Menu change applies at once.** The effects are drawn by the forked
  replay workers, which keep their own copy of the live settings; the menu
  restarted them only for the control scheme and the red dot. Leaving a
  screen with MODEL SHADOWS, GLOWS or LIVE SHADOWS changed now restarts them
  (`game_menu.cpp` leave_screen). Not driven by eye (scripted keys do not
  reach the pause menu).
- **ROUND** (user: "should we add a cheaper model shadow then? we clearly
  already have it, the cheap sprite circle under duke"): MODEL SHADOWS is now
  SHAPE / ROUND / OFF (`--model-shadows on|round|off`, no schema change).
  Duke's own shadow is not a model the host draws (no mesh, no host packet
  in the traces); sampled in a picture it is a hard-edged oval taking the
  sand from about (100, 62, 36) to (55, 35, 21): the PS1's mode 0 over
  black. ROUND draws the same: a 12-sided fan of the planar shadow's black
  patch (`prop_shade_tpage`, mode 0) on the floor's plane under the enemy's
  centre, one per enemy and image. The enemy is the object in s3 at the
  actor draw callers 0x80033778 and 0x80034448 (position at +4, the hips;
  traced against the limb positions). Its radius is 0.6 of how far the
  enemy's limbs reached from that centre in the image before (80 to 300
  units). The ground check drops each wedge whose rim corner is off the
  plane. A first try with the white halo and subtractive blending (mode 2)
  gave a near-black centre with a red rim on the sand, so it was dropped.
  Pictures (slot 11, walked back): a neutral oval under the enemy matching
  Duke's in tone and size.
- **Cost, redone** (`d41s2cost.sh`: none, SHAPE, ROUND with glows off, and
  glows only; offscreen at 180, `PSX_GPU_PHASES=2`, the player's profile with
  LIVE SHADOWS OFF, interleaved and rotated, 3 runs each; raw scene ms per
  redraw, medians):

| View | none | SHAPE | ROUND | glows only | run-to-run spread |
| --- | --- | --- | --- | --- | --- |
| DH5 + 3 enemies (slot 11) | 11.55 | 11.58 | 11.64 | 11.43 | about 5% |
| DH5 | 9.50 | 9.49 | 9.62 | 9.53 | about 7% |
| UI 8 | 9.22 | 9.25 | 9.25 | 9.27 | about 5% |

  CPU clock 2.1-3.1 GHz per run, GPU busy 70-81%. Every difference is at
  most 1.3%, inside the noise; ROUND is not measurably cheaper than SHAPE on
  this laptop (both are a few dozen small translucent faces per enemy). In
  slot 11 the enemies' feet are near the bottom of the view, so part of
  their shadows is off screen.

## 3. D23N2B pieces 4c and 4d (MEASURED)

Developer bits of `DNTTK_N2_CHECK` (`redraw_n2.inc`; `redraw_check.inc` now
keeps the previous job's record also for bit 32):

- **32, piece 4c**: with 16, the hooks' CPU state comes from the earlier job
  of the same frame (another fraction, its own registers), as a rebuild
  (D23N4) would have it, when its hook list matches the later job's call for
  call (same hook and return address); other jobs are counted and skipped.
- **64, piece 4d**: with 16, every call of the transform `0x800292a0`, not
  only the joint loop's, compared by caller (the matrix passed and its 8
  words, from the job start).

Piece 4c, offscreen at 180, walking, turning and firing (`CASES=cad`,
`DNTTK_N2_CHECK=56`):

| View | jobs run from the earlier record | joint calls exact | differ | jobs skipped: hook lists differ / first job of a frame |
| --- | --- | --- | --- | --- |
| UI 2 | 195 | 3,315 | 0 | 27 / 210 |
| UI 8 | 54 | 972 | 0 | 62 / 188 |
| UI 9 | 238 | 4,284 | 0 | 62 / 228 |

So the joint matrices are exact also from the earlier job's registers. The
jobs whose hook list differs from the earlier job's (a hook more or less:
objects culled or drawn differently at the other camera) are the next thing
a rebuild must predict.

Piece 4d (`DNTTK_N2_CHECK=88`, the job's own registers; UI 8, UI 2 and 9
alike; with the earlier record, `120`, the same pattern):

| Caller | exact | differ | reading (INFERRED) |
| --- | --- | --- | --- |
| `0x80031dec` | 7,760 | 179 | mostly a matrix as at the job start |
| `0x80033584` | 7,851 | 1,742 | partly built in the job (per object) |
| `0x80032f48` | 1,407 | 111 | as `0x80031dec` |
| `0x8003133c` | 0 | 320 (one a job) | built in the job from the camera |
| `0x80038c68`, `0x80038d7c`, `0x80038e18` | 0 | about 320 each (one a job) | the sky's, built from the camera |
| `0x8003437c`, `0x80033f14`, `0x80031900`, `0x80090068` | 0-3 | 3-9 | rare |

Handoff (resumable): host builds of the matrices built during the job from
the camera (the sky's three calls and `0x8003133c`: one each a job, never
exact from the start memory), then what makes `0x80033584`'s 18% and the
others' 2-7% differ (which objects, which matrix words), then the hook-list
differences of piece 4c, then the rest of note 456 section 6 (object
culling, the camera-dependent hooks, sprites and particles, a whole later
job word for word).

## 4. Framework and Vanilla

No framework file changed this session (`export_runtime_patch.py` leaves
the tracked patch identical). Vanilla: every change is Modernized, Polymorph
and lit-scene only (CODE); a Vanilla launch drew normally. Local tests: 259
OK (4 skipped).

## 5. Reproduce

`recomp/analysis/d23y6/` (local): `d41p2ab.sh`, `d41p2ab2.sh` and `p2sum.py
p2 old,new` / `p2s old,new` (D41P2 cost), `d41p2pics.sh` and `imgdiff.py`
(pictures), `LAB_DNTTK_BAKE_CENSUS=1` and `LAB_DNTTK_SHADOW_COUNT=1` with
`CASES=still`; D41S: `CASES=d41smake` (`ENEMY`, `WEAPON`, `AMMO`, `KEY`,
`FIRE`, `CHEAT=dnkroz`, `DEST=11`) then `CASES=atload` on slot 11,
`abc.sh 3 "11 4 1 7 8" fxon "" fxoff "LAB_DNTTK_MODEL_SHADOWS=off
LAB_DNTTK_GLOWS=off"` and `comb2.py fxoff,fxon "11 4 1 7 8"`, `d41scost.sh`
and `p2sum.py sc off,on "11 4 7"` (void: see the correction in section 2);
D41S2: `d41s2cost.sh` and `p2sum.py s2 none,shape,round,glow "11 4 7"`,
`CASES=back BACK=110` and `CASES=tp` with `CTRL='{"model_shadows":"round"}'`
(the lab's profile copy; `LAB_` switches for profile settings are overridden
by the launcher); D23N2B: `RATE=180 OFFSCREEN=1
LAB_DNTTK_N2_CHECK=56|88|120 CASES=cad`.
