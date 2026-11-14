# 470 - D41R, D41P, D23Y8, D23Y5, D23Y7, D23N2B on the field laptop

Closed 2026-11-14: D41R and D41P accepted and Done (user: "accepted as the
next incremental performance hike"); D23Y5, D23Y7, D23Y8 Blocked as measured
here, D23N2B In progress.

User, 2026-11-14: "work on D41R, D41P, D23Y8, then re-measure D23Y5 and
D23Y7, then D23N2B, on the field laptop, in that order, autonomously. Don't
stop to ask; make the sensible call, write down why". Later the same day: "if
we can get to 60hz / 120fps with at least 2x on this hardware i'll be so
happy, even if that means using original lighting and none of our dynamic
shadows. but if we can squeeze in some basic lighting and shadow effects, im
here for it", and to learn from the Quake and Hexen II ports on this laptop
(section 8).

Machine: the field laptop of notes 461, 468 and 469 (i7-5600U, 2 cores and
4 threads, HD Graphics 5500, one 15 W package). Host checked first: load
0.2, on AC power, no backup, update or game running. Lab:
`recomp/analysis/d23y6/` (local), the player's profile copied read only, the
lab's private card copy (never the player's cards or saved profile). Lab slot
N is UI slot N+1; DH5 is the copy of UI slot 5 (DUKE HILL's open town), DH
private slot 10 (DUKE HILL's start). Evidence labels: MEASURED, CODE,
INFERRED.

| Job | Result |
| --- | --- |
| D41R | LIVE SHADOWS OFF built (menu, launcher, console, manual); Needs playtest |
| D41P | Models lit from the bake of the floor they stand on, built; Needs playtest |
| D23Y8 | Measured: at most 9 points of the thread can move; Blocked |
| D23Y5 | Re-measured 57-64% of a core; Blocked |
| D23Y7 | Start meets the target; the town still steps to 30; Blocked |
| D23N2B | Piece 4b: joint matrices from the job start, exact; In progress |

## 1. D41R: LIVE SHADOWS OFF (MEASURED, built)

Measured first with `DNTTK_SHADOWS=off` (the switch the console `light
shadows off` sets), interleaved with SIMPLE on the real display at the
player's settings at 120, 3 runs each, the player's frame-rate governor:

| View | game median / lowest tenth | presents median / lowest tenth | screen fresh | emulation |
| --- | --- | --- | --- | --- |
| DH5, SIMPLE | 59.9 / 55.3 | 43 / 30 | 74.1% | 69% |
| DH5, OFF | 59.9 / 57.0 | 59 / 30 | 80.7% | 70% |
| UI 2, SIMPLE / OFF | 60.0 / 59.0, 60.0 / 58.9 | 60 / 59 both | 97.3%, 92.6% | 63%, 62% |
| UI 8, SIMPLE / OFF | 60.0 / 59.2, 59.9 / 58.3 | 60 / 59 both | 96.9%, 94.3% | 87%, 88% |
| UI 9, SIMPLE / OFF | 60.0 / 59.3, 60.0 / 59.1 | 60 / 42, 60 / 31 | 90.5%, 90.0% | 51%, 54% |

At a steady 60 presents in DH5 (governor held, 3 runs each): game median and
lowest tenth 59.4 / 51.7 with SIMPLE, 59.9 / 56.5 with OFF; CPU clock 2.05
against 2.20 GHz; emulation thread 91% against 87%. It helps the heavy view;
the other views were already near 60 and stayed within the noise.

Built (Modernized, Polymorph, LIGHTING ENHANCED or RELIT):

- OPTIONS > DISPLAY > LIVE SHADOWS: SHARP, SIMPLE, **OFF** (saved value
  `off`); `run.py --shadow-edges off`; the terminal settings line names it.
- Runtime (`light_shadows.inc`): `shadow_edges_off()`; `shadows_on()` is false
  under OFF (no live shadow map is drawn); SHARP and SIMPLE unchanged.
- Console: `light shadows off` now sets LIVE SHADOWS OFF for the session (not
  saved, as `sharp` / `simple`); `light shadows on` forces them on as before
  (from OFF with SIMPLE edges).
- The GRAPHICS grades are unchanged (SIMPLE already has LIGHTING ORIGINAL).
- Manual: OPTIONS > DISPLAY > LIVE SHADOWS.

Pictures (offscreen, matched still views, a test live light ahead): DUKE
HILL with the light differs from SIMPLE only around the light's shadows
(1.0% of pixels; without the light 0.04%); UI 2 without the light shows the
water layer's animation (the two shots 0.2 s apart in level time); the baked
lamp shadows on walls are unchanged. The launcher passed `off` (terminal line
"Live shadows: off"). Not checked by eye: the menu row itself (scripted
keys do not reach the pause menu offscreen, and the real display was in
use). Vanilla and Classic: the setting is Modernized and Polymorph only
(CODE).

## 2. D41P: models lit from the bake (MEASURED, built)

Cost first, GPU per redraw (`PSX_GPU_PHASES=2`, scene ms x GPU clock, which
varies by run), offscreen at 180, 2 interleaved runs each, developer
`DNTTK_D41P` (kept as a measurement switch):

| Change against now | UI 2 | UI 8 | UI 9 |
| --- | --- | --- | --- |
| Baked lamps without shadow maps (models lit unshadowed) | -5.4% | -7.4% | -7.8% |
| Baked lamps left out of model shading | +2.7% (noise; water) | -12.1% | -11.6% |
| All live shadows off | -2.2% | -11.7% | -8.8% |

It matters (about a tenth of the GPU's scene work in two of three views), so
it was built as agreed on 2026-11-12 (the cheapest shading), after Quake's
`R_LightPoint` (a model takes the light of the floor under it; section 8):

- When a bake goes live, its job (off the game thread) rasterises the level's
  floor faces (normal within 45 degrees of vertical) into a 128-unit grid
  (DUKE HILL 473 x 369 cells, 264,888 floor entries; level 6 545 x 585,
  359,928; level 3 753 x 665, 388,584). The grid is published with the face
  map in the shared table the redraw workers read (`lamp_bake_map.cpp`, the
  same pinned half for a whole redraw job).
- Each model mesh draw (`near_clip.cpp`; the GTE translation is its origin)
  looks up the nearest floor at or below that point and gives its corners
  that floor's baked surface (new framework entry
  `psx_mod_gpu_host_vertex_world_normal_surface`, a 9-word kind 6 record in
  redraw streams).
- The renderer already leaves the baked lamps out of the light loop for a
  triangle with a surface and adds the bake; for a model fragment the shader
  now reads the floor chart from the fragment's own side of the floor plane
  (`s_model`). No floor found: the live lamps, as before.
- Live lights (blasts, shots, test lights, TV screens, prop lamps) still
  shade models as before. The baked lamps keep their shadow maps for the
  unbaked surfaces (water, fills).
- Developer `DNTTK_MODEL_BAKE=0` restores the live lamps on models.

Result, the same method, 3 interleaved runs each: UI 2 -2.7%, UI 8 -9.9%
(raw scene ms 9.94 -> 9.58 at a lower GPU clock), UI 9 within the noise
(raw ms equal; one live run dropped to 40 presents). In UI 8 the lit
triangles with a bake surface rose from 198,544 of 198,544 (world faces) to
238,036 of 245,092 (`PSX_LIGHT_STATS=1`): the models now take the bake.
Matched still pictures (models live against bake): 0.35% of pixels differ in
DH5, 0.00% UI 2, 0.87% UI 8, 0.08% UI 9; the weapon and arms barely change
by eye. Less than "left out" because the baked lamps' shadow maps stay for
water and fills (next step: give those the bake too, then the baked lamps
need no maps: about -5 to -8%).

## 3. D23Y8: the emulated GPU on its own thread (MEASURED, not built)

A read-only map of the code (CODE): the emulation thread owns the GL
context; GP0 processing feeds guest-visible state (VRAM reads come from GL
through `ensure_cpu`, the widescreen classifier reads guest RAM during GP0),
and many title calls set renderer state in GP0 order (lights, shadows, lamp
levels, host vertices). Only the GL backend calls and presentation could
move to another thread; the GP0 decode must stay.

What could move, from 21,495 call stacks of UI 2 at the original 60 (note
469's profile, `d23y8_split.py`): GL backend work outside presentation 7.2%,
the display half of presentation 1.9%, GP0 decode that must stay 4.8%. At
most about 9 points of the 57-59% could leave the thread (before the queue's
own cost), so UI 2 would end near 48-50% at best and UI 8 (63-64%, section
4) near 55%: D23Y5's target is not reachable by D23Y8 alone, and an exact
queue touches the renderer's three copies, the render threads, redraw
sessions, savestates and every readback. Not built; Blocked with this
measurement. Resumable plan (smallest exact first step): presentation on its
own shared-context thread (the replay path already presents a finished
texture), then a `gr_*` / `gl_set_*` proxy queue with drain barriers at every
readback.

## 4. D23Y5 re-measured (MEASURED)

The original 60, offscreen, 3 runs each, CPU at 3.09-3.12 GHz: emulation
thread median UI 2 57.0-58.8%, UI 8 63.3-64.5%, UI 9 57.0-57.5% (target:
under 50%). Unchanged from note 469. Blocked.

## 5. D23Y7 re-measured (MEASURED)

The real display, the player's settings at 120, the player's governor, 3
interleaved runs each (D41P on in both):

| | game median / lowest tenth | presents median / lowest tenth | screen fresh | rate changes a run | CPU clock |
| --- | --- | --- | --- | --- | --- |
| DUKE HILL start, SIMPLE | 59.9 / 58.6 | 60 / 59 | 98.1% | 0 | 2.40 GHz |
| DUKE HILL start, OFF | 59.9 / 58.7 | 60 / 59 | 97.8% | 0 | 2.45 GHz |
| Town (DH5), SIMPLE | 59.9 / 57.7 | 42 / 30 | 73.8% | 4.0 | 2.74 GHz |
| Town (DH5), OFF | 59.9 / 57.8 | 40 / 30 | 72.9% | 4.7 | 2.59 GHz |

The start meets the target (60 game frames, lowest tenth 57 or more, 60 even
pictures). The town meets the game-frame part but still steps between 60
and 30 pictures; under the governor OFF shows no difference there (held at a
steady 60 it gains about 5 points of game lowest tenth, section 1). D23Y6's
cap, placement and one worker stay (note 469's A/B); the late warp stays
cancelled. Blocked: the town at 2x needs less work a redraw (D23N2B-N4) or
a lighter picture.

## 6. D23N2B piece 4b: joint matrices from the job start (MEASURED)

New developer check, bit 16 of `DNTTK_N2_CHECK` (`redraw_n2.inc`, namespace
`jr`): at every 4th gameplay job's start a copy of the worker is forked (its
memory as at the start, before any of the game's stores). At the flip it
runs every top-level hook of the job with the CPU state the job recorded,
without applying the game's stores, after giving the camera block the eye
piece 2 predicts (as the visibility check does). Each matrix our joint
callbacks pass for Duke's joints is compared word for word with the job's
(the address and its 8 words).

| Run (offscreen, 180) | jobs | joint calls | exact | differ |
| --- | --- | --- | --- | --- |
| UI 2, still | 240 | 4,320 | 4,320 | 0 |
| UI 8, still | 304 | 5,472 | 5,472 | 0 |
| UI 9, still | 208 | 3,744 | 3,744 | 0 |
| UI 2, walking, turning, firing | 416 | 7,072 | 7,072 | 0 |
| UI 8, walking, turning, firing | 320 | 5,760 | 5,760 | 0 |
| UI 9, walking, turning, firing | 512 | 9,216 | 9,216 | 0 |

Without the predicted eye, the walking runs differed by 1-2 units of
translation in about a quarter of the calls (the composition derives the eye
before Duke's draw). So our D36 joint code reproduces a later job's joint
matrices exactly from the job-start memory and piece 2's eye. The registers
are still the job's own (structural: the camera block, the table entry,
actor and joint index).

Next (resumable): (1) run the same from the earlier job's record (same
frame), as D23N4 will; (2) the other transform callers (`0x80033584`,
`0x8003133c`, `0x80038c68` / `0x80038d7c` / `0x80038e18`); (3) the rest of
note 456 section 6 (object culling, the camera-dependent hooks, sprites and
particles, a whole later job word for word).

## 7. Framework and Vanilla

The framework change (D41P's corner entry, its record and the shader's side
test) is exported to `recomp/patches/time-to-kill-accepted-source.patch`,
which applies to the pinned clean checkout and reproduces the working tree
exactly. Vanilla: dynamic lights, LIVE SHADOWS and the bake are Modernized
only (CODE); no Vanilla run was needed for these changes.

## 8. Research: how the Quake and Hexen II ports light this laptop

The user's own settings on this laptop (read from their configs, not
changed): Alkaline (QuakeSpasm-Spiked-M build, with an `ironwail.cfg` beside
it) at 1600x900, `r_dynamic 1`, `r_shadows 0.6` with
`r_shadows_groundcheck 1`, no FSAA; Hexen II (Hammer of Thyrion 1.5.9,
`glhexen2`) at 1600x900, FSAA 8, `r_shadows 1`, coloured dynamic lights,
`gl_glows` and missile glows.

How they light (from their published sources; INFERRED for these exact
builds):

- Static light is baked: lightmaps per surface, shadows included (our D41M
  bake).
- Dynamic lights have no shadows: they are added into the lightmaps of the
  surfaces they reach (`r_dynamic`), or drawn as a flat additive glow
  (`gl_flashblend`).
- Models take the light of the floor under them (`R_LightPoint` traces down
  from the model's origin and reads that lightmap), shaded with a fixed
  table, with no per-pixel lighting (D41P now does the bake lookup).
- "Shadows" on characters are planar: the model's mesh flattened onto the
  ground plane under it and drawn dark and translucent (`r_shadows`), no
  shadow maps.
- Glows: additive halos at flames, torches and missiles (Hexen II
  `gl_glows`), a cheap impression of light.

So LIVE SHADOWS OFF plus D41P is the Quake recipe; the "basic shadows" the
user sees in those games are planar model shadows, which we do not have.
Proposed as D41S (a cheap shadow and glow tier for LIVE SHADOWS OFF).

## 9. Reproduce

`recomp/analysis/d23y6/` (local): `abc.sh 3 "4 1 7 8" rS "" rO
"LAB_DNTTK_SHADOWS=off"` and `comb2.py rS,rO "4 1 7 8"` (D41R); `d41p.sh`
and `d41p2.sh` (D41P cost); `CASES=d41r` and `CASES=still` with
`LAB_DNTTK_MODEL_BAKE=0|1` (pictures); `d23y8_split.py prof60/stk`;
`OFF=1 RATE=60 abc.sh 3 "1 7 8" y5 ""` (D23Y5); `abc.sh 3 "10 4" y7S ""
y7O 'CTRL={"shadow_edges":"off"}'` (D23Y7); `RATE=180 OFFSCREEN=1
LAB_DNTTK_N2_CHECK=24 CASES=cad` (D23N2B).
