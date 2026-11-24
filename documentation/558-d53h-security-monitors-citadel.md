# D53H - Security monitors slow the game on citadel

Job: [D53H](../jobs/graphics.md#d53h---security-monitors-slow-the-game-on-citadel-third-tv-shows-no-feed).
Machine: citadel, the TV PC (Ryzen 3 PRO 3200GE 4c/4t, Radeon Vega 8 iGPU,
KDE Wayland 1920x1080 at 60 Hz). Player settings there: GRAPHICS custom,
internal scale 4, LIGHTING relit, LIVE SHADOWS sharp (smooth), FRAME RATE 120,
DRAW DISTANCE extended, first person.

## 1. Report

User, 2026-11-24: "weird problems/dropouts when interacting with the security
monitor ... it started running very poorly. the third monitor doesnt even show
the video feed. it only happened this bad when i interacted with the third
monitor, but i think all 3 werent completely happy." Place: the LEVEL00 cell
block's three Duke 3D TVs. The user left the game paused there; a copy of
their slot-1 save from the spot (never the original) is the test fixture
(`research/savestates/d53h/`, local).

## 2. What happened (MEASURED, citadel's session log and live debug queries)

- The level held 60 game frames a second up to the TVs. At the TVs it fell
  to 52-53, then 42, 40 and 36, with audio underruns.
- The pacer then switched to the original presentation ("redraws again in
  120 s"). That stops the redraw workers, which draw the D53A feed, so every
  TV went back to SECURITY: the "third monitor shows no feed".
- Host idle (load 0.3, no CPU, memory or IO pressure, no updates or backups).
  The GPU read 100 % busy even while paused; the swap blocked 16 ms at the
  median and 36 ms at p95.

## 3. Two causes

### 3a. Citadel's install was half updated (deployment)

Citadel ran the desktop's D19Z4 build (sha256 914ddbd8..., the desktop's
normal build), but only the executable had been copied: the checkout
was still at fe18de2 (2026-11-17, 64 commits behind) with 19 lab files
modified. Its `run.py` had none of the Modernized redraw defaults, among
them D23AO's `DNTTK_REPLAY_FLOOR=0` / `DNTTK_REPLAY_SHED=0` (the frame rate
never changes) and the D23AK lighter worker. The fallback in section 2 is
therefore the old launcher's, against the D23AO rule. Fixed by bringing the
checkout to main (lab files kept as a git stash and a diff in `~/ttk-ab/d53h/`
on citadel) and applying the current runtime patch (identical to the tracked
one). Build packs were already identical to the desktop's.

### 3b. The TV screens' light cast live shadows (the slowdown)

D53's TV screen light (and the power sign's) was a shadow candidate like a
placed lamp (`lights_publish`, `dynamic_lights.inc`). Standing at the TV wall
they are the strongest lights, so they took live shadow slots, and every pixel
of the wall in front of Duke sampled their smooth shadow maps: 10 shadowed
lights there instead of 6 (DNTTK_SHADOW_COUNT trace, desktop). The maps were
cached (no rebuilds); the cost is the per-pixel sampling, large at scale 4 on
Vega 8. Citadel A/B with the synced launcher, real screen, 6 s per phase, TV3
(the third, right-hand TV) and TV2:

| phase (after leaving the camera, at the TV) | live shadows | feed off | shadows off | scale 2 |
| --- | --- | --- | --- | --- |
| TV3 presents / GPU busy | 34 / 95 % | 41 / 93 % | 54 / 93 % | 56 / 77 % |
| TV2 presents / GPU busy | 33 / 87 % | 43 / 98 % | 60 / 83 % | 57 / 64 % |

Turning the feed off barely helped; live shadows off did. The D53A feed (one
more full-resolution redraw of the camera's room, 15 a second, box filtered to
96x96) is a smaller second cost.

## 4. Change (Modernized, Polymorph, lights only)

The TV screens' and power sign's lights no longer cast live shadows (as in Duke
3D, whose screens cast none); they still light the wall and flicker as before.
Placed lamps, prop lamps and test lights are unchanged. Developer switch:
`DNTTK_SECURITY_LIGHT_SHADOWS=1` keeps the old behaviour. Vanilla, the Classic
renderer and LIGHTING ORIGINAL have no dynamic lights and are unchanged.

## 5. Evidence

MEASURED, citadel, real screen, the same build, interleaved (new, old, new,
old; old = `DNTTK_SECURITY_LIGHT_SHADOWS=1`):

| at the TV after the camera | new run 1 | new run 2 | old run 1 | old run 2 |
| --- | --- | --- | --- | --- |
| TV3 game / presents / GPU | 60.1 / 57.8 / 69 % | 60.1 / 58.1 / 70 % | 50.0 / 34.9 / 96 % | 52.8 / 36.5 / 86 % |
| TV2 game / presents / GPU | 59.9 / 59.3 / 78 % | 59.8 / 59.5 / 66 % | 54.8 / 34.8 / 87 % | 54.1 / 36.2 / 94 % |
| swap median (TV3, TV2) | 7.7, 2.8 ms | 7.9, 1.0 ms | 40.7, 40.7 ms | 42.4, 41.6 ms |

The feed kept running in every run (107-108 pictures stored on TV3). Watching
the cameras themselves held 60 / 59-60 in all runs.

Picture (desktop, offscreen): at the TV after the camera, new against old,
0 of 786432 pixels differ by more than 24 levels. From the start position,
the differences are the weapon's idle sway and the TV light's flicker phase
(seen only at 6x gain); no shadow shape changed.

Lab: `recomp/analysis/d53h` (local): `lab.py` (desktop, shared helpers),
`clab.py` (standalone; REAL=1 for citadel's screen through the saved session
environment; phases idle, watch and feed per TV; CPU per thread and worker,
GPU busy from amdgpu, presents, swap times).

Field laptop (MEASURED, the user's own session from the citadel-scene icon
at 20:34, the build before this change, its own settings): 60 presents a
second throughout; game speed dipped to 55.6-58 for a few seconds.

## 6. Observed, not changed

- The first E press after walking up to a TV is ignored; the second enters
  the camera view. In the camera view, E does not step back (jump does).
  The manual says "Use it again to step back". Not investigated here.
- Citadel at scale 4 is already 63-81 % GPU busy in that room before any TV.
- The feed redraw is full resolution for a 96x96 picture; drawing it smaller
  needs a per-job scale in the render thread copies (the renderer's VRAM
  target is sized by the scale). Possible follow-up.

## 7. State

User-accepted 2026-11-24 ("lock it in and close it out"). Citadel runs
the fixed build with the synced launcher (its previous executable kept as
`.pre-d53h`); the laptop and the desktop the LTO+PGO build with the fix.
