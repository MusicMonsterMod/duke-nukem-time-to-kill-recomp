# D17V - Modern view bob: steady Duke 3D-style tempo

**Done (user-accepted 2026-10-19):** "total victory. i even played it on
strong and i love it."

Modernized first person only. Vanilla and third person are untouched. Code:
`recomp/src/ttk/first_person.inc` (`first_person_anchor`).

## User report (2026-10-19)

The running bob looked like a sped-up walking bob and read as erratic. The
request was a smoother, more realistic bob emulating Duke Nukem 3D's, using the
user's eDuke32 recording `research/screencaps/running.mp4` (walking, then
running). Duke 3D's weapon motion is out of scope.

## Duke 3D reference

Source (`research/eduke32/source/duke3d/src/player.cpp`, `P_ProcessInput`):

```
if (pSprite->xvel > 16 && on ground, not water)
    pycount += 52; pycount &= 2047;
    pyoff = klabs(pSprite->xvel * sintable[pycount]) / 1536;
else pyoff = 0;
```

`game.cpp` adds the interpolated `pyoff` to the camera z (`cl_viewbob`). In
other words:

- The phase advances a fixed 52/2048 of a turn per 30 Hz tic. That is one dip
  every 0.66 s (about 1.52 dips a second) at every speed.
- The eye sinks by |sin| times a depth proportional to speed (`xvel`, distance
  per tic). Running only dips deeper (twice as deep, at twice the speed).
- There is no sideways sway or roll. The camera moves at an even speed
  (velocity with friction).

Video check: vertical image motion in the recording (row-profile correlation,
`running.mp4` at 640x360) peaks at 1.50 Hz in both the walking and the running
half. The running peak is about 2.3 times stronger.

## Measured causes in TTK (driven routes, baseline a0246cad)

1. **Tempo from distance.** The D17C bob advanced its phase by distance (one dip
   per 320 units). Walking is about 10 units per vblank (1.9 dips/s). Running
   is about 50 (5 times faster), which gave about 9 dips/s. That is close to
   TTK's 30 Hz game frames, so the dip was sampled at a few points a cycle and
   jittered (eye height jumping about 6 to 12 units between frames).
2. **Depth from per-frame distance.** Strength used the distance per camera
   update with a per-update filter, so uneven game frames and root steps made
   it pulse.
3. **Stride surge.** The eye rode Duke's root, which moves in stride pulses:
   13 to 33 units per game frame walking and 57 to 155 running (a pulse about
   every 3 frames, about 10 Hz). The view's forward speed surged by about ±25%
   while running. Duke 3D's camera moves evenly.

## Implementation

All timing uses the host vblank sequence (`input_host_frame`), not camera
updates.

- **Glide.** The eye's x/z follow Duke's root through a glide. The glide keeps
  a low-passed velocity (0.9 per vblank), advances by it, and is pulled back
  toward the root (0.85 per vblank). It may be at most 16 units ahead of the
  root along the motion and 64 behind or beside it. Two still updates are a
  stop: the velocity zeroes and the eye settles onto the root (no overshoot).
  A step of 400 units or more is a teleport, and the glide resets.
- **Bob.** Speed is the glide velocity. Depth is 16 units at walking speed (10
  per vblank), growing as speed^0.43 to 32 at running speed (about 50), capped
  at twice the walking depth and tapering below 3 per vblank. The depth eases
  in and out (0.87 per vblank), and it eases out whenever Duke is not in ground
  mode (`player+0x22c` != 0: jumps, falls). The phase advances 26/2048 of a
  turn per vblank (Duke 3D's rate) while bobbing, and holds while standing.
  The eye sinks by depth x setting x |sin(phase)|. There is no sway.
- **Setting.** `view_bob` off/subtle/on/strong (0, 0.5, 1, 2) scales the depth
  as before. **Off** also disables the glide: the eye is exactly the D17C
  stable eye (fallback).
- Diagnostics: `DNTTK_BOB_LOG=1` prints one `[TTK bob]` line per eye update
  (host frame, root, speed, amount, phase, dip, glide).

## Evidence (offscreen, real GPU, harness `recomp/analysis/d17-high-refresh`)

Route: `cards` slot 4, first person, walk 2 s, run 2.5 s, stop, turn, run,
jump, run, stop (`bobD.log`, plot `bobplot.py`; glide model `glidesim2.py`;
probe `bobgait.py`). Camera updates every 2 vblanks throughout.

| | Before | After |
| --- | --- | --- |
| Dip tempo walking / running | 1.9 / about 9 per s | 1.52 / 1.52 per s |
| Dip depth walking / running (on) | 6.6 / 12.8 units, jittering | 16 / 32 units, steady |
| Sideways sway | 3 units | none |
| Eye speed sd, walking (mean 10.4) | 2.2 | 1.3 |
| Eye speed sd, running (mean 49-52) | 10.5-13.1 | 4.4-6.2 |
| Mean frame-to-frame eye speed change, running | 13.5-17.8 | 4.1-5.1 |
| Glide while steady running | - | up to 18 (54 at run start) |

- Jumps ease the dip out and back in on landing. After a stop the depth fades
  in about 0.25 s, and the eye settles onto the root with no bob while
  standing.
- Running into a wall: the root stops, and the glide and dip return to 0.
  Screenshots normal (no see-through).
- `ttk-input-test`, `ttk-controls-test` (with the D22B levels) and
  `ttk-near-test` PASS. Python 146 OK (12 skipped). Vanilla route
  `d17v-vanilla` completed.
- `ttk-aim-test` does not link, with or without this change: it lacks the
  `psx_mod_host_call_begin/end` stubs that `weapon_aim.cpp` uses since D19J.
  Pre-existing.

Executable `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193`.

## Limits

- The depth (16 units walking at "on") is a judgement call. Duke 3D's own dip
  is deeper relative to eye height (about 9% walking, 19% running). **strong**
  doubles ours toward it. `DNTTK_VIEW_BOB` also takes a numeric scale (0..3)
  for tuning.
- The glide smooths the view, not Duke. His body, collision and the
  third-person camera are unchanged.
- Accepted by the user's playtest (including strong).
