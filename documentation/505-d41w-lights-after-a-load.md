# D41W - Lights present from the first frame after a load

2026-11-17. **Done, user-accepted** (looks awesome to me. i'd say lets lock it in and close it out). User report (2026-11-16, field laptop, LIGHTING
RELIT): "when the game loads, not only do lights take a while to load in".
Private runs only: the lab `recomp/analysis/d41w/` (local; `probe.py` from the
D17BK probe, with a private profile and a copy of the lab cards; `match.py`,
`diff.py`). Every run checks the player's saves and settings before and after
(unchanged).

## Cause (MEASURED, CODE)

After a load the baked lamp light (D41M) goes through these stages: the room
mesh and lamps are rebuilt, every prop lamp is checked against its prop's
wall, the bake is read from the cache on a thread, the renderers' copy and
the floor grid (D41P) are made, the game thread publishes it, then the
emulation thread's renderer uploads the atlas.

Two things made the lamps arrive late, and one made them vanish meanwhile:

1. **A hole with no lamps.** The game thread published the bake as soon as it
   was read. From then on (D41P2) the fixed lamps left the live light list,
   but the renderer only switched the bake on after the last atlas row was
   uploaded, 4 MB an image: 15 images for RELIT's level 6 atlas (62.7 MB),
   9-10 for ON. For those images world faces got neither the live lamps nor
   the bake. Matched shots (below) show the hole: 2.2 mean difference on
   DUKE HILL's town against 0.3 for the live lamps.
2. **A slow upload.** 4 MB an image is right while a bake is already shown
   (no hitch), but after a load nothing is shown and the whole atlas took
   only 18-35 ms of upload work, spread over 9-15 images (130-360 ms).
3. **A slow start.** The bake waits for every prop lamp's wall check, done 8
   a tick: 110-400 ms after the load on the desktop and the field laptop.

RELIT's atlas is about 60% larger than ON's, so it showed more: that is the
"RELIT arrives later" of the first measurement.

## Change

- Framework (`gpu_gl_renderer.c`, `lightmap_store.c`, `mod_plugins.h`): the
  emulation thread's renderer uploads a new store generation into separate
  textures while the ones shown stay in use, and swaps them in when complete
  (a cleared store still drops them at once). Upload steps stay 4 MB; while a
  bake is shown one step an image as before, while none is (after a load)
  steps for up to 6 ms an image (developer `PSX_BAKE_UPLOAD_MS`, 0 = one step).
  New `psx_mod_lightmap_shown()`: 1 when the renderer draws with the newest
  stored lightmap. The `[gl bake]` line gives the images, the upload time and
  a CLOCK_MONOTONIC stamp.
- Title (`lamp_bake.inc`): a finished bake waits until the renderer shows it
  (2 s fallback for a renderer that never uploads), then the face map, the
  floor grid and the switch from live lamps to the bake are published
  together. So the live lamps light the level until the bake is on screen,
  never neither. The renderers' copy is made before the floor grid, so the
  upload overlaps it. A bake shown from another level is stopped when a new
  one starts. One line per load gives the stage times
  (`[TTK bake] level N after the load ...`).
- Title (`level_lights.inc`): while no bake is shown, the prop lamps' wall
  checks run as many as fit in 8 ms a tick (25-40 lamps took 7-10 ms on the
  desktop and the field laptop), not 8 a tick.
- Developer `DNTTK_BAKE_WAIT=0` with `PSX_BAKE_UPLOAD_MS=0` gives the old
  timing after a load for A/B runs (the double-buffered upload stays).

Modernized only (LIGHTING ENHANCED or RELIT). Vanilla has no dynamic lights
and runs none of this (smoke run: no bake lines, normal load). The bake's
content is unchanged.

## Evidence

Times from the game thread seeing the load to the renderer drawing with the
bake (`[gl bake]` stamp), RELIT unless noted, two loads per slot (the first
from another level; the second reloads the same level, which keeps the bake):

| machine, display | level 6 (UI slot 2) old / new | DUKE HILL (UI slot 5) old / new |
|---|---|---|
| desktop offscreen, warm | 496 / 312-317 ms | 433 / 192-200 ms |
| desktop offscreen, cold cache | - / 459 ms | - / 248 ms |
| field laptop offscreen, warm | 703 / 464 ms | 592 / 263 ms |
| field laptop offscreen, cold | 671 / 391 ms | 733 / 177 ms |
| field laptop real 60 Hz screen | 770 / 411 ms | 752 / 394 ms |
| field laptop offscreen, ON | 589 / 375 ms | 456 / 165 ms |

Cold: the bake files evicted from the page cache (`posix_fadvise`) before the
load; reads took 81-153 ms on the desktop, 36-76 ms on the laptop.

Matched frames: a first load against the reload of the same level (which
shows the bake from its first frame), at equal game frames, mean absolute
difference per pixel. DUKE HILL, desktop: old 0.39 (live lamps), 2.22 at
504 ms (the hole), then 0.07-0.20; new 0.31 while the live lamps show, 0.00
from 360 ms. Field laptop offscreen: old 2.17 at 718 ms, new at most 0.54;
real screen: old 2.14 at 951 ms, new at most 0.81 (late frames; level 6 shows
0.4-2.2 at late frames in both builds, the lamps' flicker phase). Before and
after shots: `research/before-after/D41W/` (local).

LIGHTING switched in play (console `light mode`, DUKE HILL): ON to RELIT keeps
the ON bake (porch region 31.1 -> 30.8 as the ambient eases), then steps to
the RELIT bake at 296 ms (32.1 easing to 31.6); RELIT to ON steps at 162 ms;
no dip in either. While a bake is shown the upload takes one step an image
(9 images).

## Limits

- The bake is not on screen from the first frame: the first picture after a
  savestate load comes about 75 ms after the load, the bake 180-460 ms after
  it. Until then the live lamps light the level; they differ from the bake by
  about 0.3 mean (DUKE HILL). Holding the picture until the bake is ready
  would make loads longer; not done.
- While the live lamps show, their shadow maps cost GPU for those frames.
- Not measured on citadel.
