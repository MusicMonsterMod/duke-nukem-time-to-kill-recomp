# D17AM3/D17AM4 - Unified geometry by default, sweep and renderer re-triage

2026-10-29. Done after the user's playtest (commit authorized); follow-ups
D17AN (slot 12 water through the bridge deck), D17AO (slot 11 ceiling hole),
D17AP (slot 9 grave soil flashing) and D35N2 (slot 8 mirror line). D17AF and
D17AH user-confirmed; D20H, D17AI and D17AK still need a check. Parents: D17AM (job board renderer plan),
D17AM1 design ([note 324](324-d17am1-unified-geometry-design.md)), D17AM2
switch ([note 325](325-d17am2-unified-geometry.md)). User instruction for this
pass: sweep the private fixtures and level-select levels in unified mode at
FOV 64/90/120, 60 and 120 FPS, 16:9 and 4:3, first and third person, with
and without the Bio Mask; fix regressions; make unified geometry the
Modernized default; remove the superseded special cases; then re-triage
D17AF (new slot 10 save), D17AH, D17AI, D17AK and D20H; keep Vanilla
unchanged; private save copies only; stop for a playtest before any commit.

## What changed

`recomp/src/ttk/near_clip.cpp`, `recomp/src/ttk/water_clip.inc`,
`recomp/tests/local/near_clip_native.cpp`. No framework, generated C,
header, asset or settings change; no runtime patch export needed.

1. **Unified geometry is the Modernized default.** `DNTTK_UNIFIED_GEOMETRY`
   now defaults to 1; `=0` (developer) restores the earlier near-boundary
   renderer everywhere. Vanilla never reaches this code.
2. **Superseded special cases removed** (each existed only because native
   packets beyond the 1536/3072 boundary had no depth; since D17AE, scene
   depth covers whatever this path leaves native, and unified geometry
   leaves almost nothing native):
   - thin-platform depth bridge (D17M, `DNTTK_PLATFORM_DEPTH`);
   - compact prop and walk-over pickup depth with the shared compact key
     (D17L/D17M, `DNTTK_PROP_DEPTH`);
   - static box-top depth (D17L, `DNTTK_STATIC_TOP_DEPTH`);
   - ladder depth (D17M1, `DNTTK_LADDER_DEPTH`);
   - native-order rule for safe scenery faces (D17AA; now the general rule);
   - the developer conservative / non-precise modes (`DNTTK_NEAR_MODE`,
     `DNTTK_NEAR_PRECISE=0`, texel-line `refine`, `DNTTK_NEAR_ROUND`,
     `DNTTK_NEAR_DEPTH_SPAN`, `DNTTK_NEAR_SORT`);
   - the D17AM1 measurement takeover (`DNTTK_NEAR_TAKEOVER=all`) and the
     D17AM2 per-renderer switch (`DNTTK_UNIFIED_OWNERS`).
   Kept, because the sky, the first-person layer and a mesh a frame cannot
   afford still use them: the 1536/3072 boundaries, depth-locality
   subdivision, the object bias, scene depth, the payphone contact rule,
   the D17W coplanar rank and the D17N whole-polygon bypass.
   Scene depth is no longer prepared for meshes unified geometry draws
   whole (they leave no native packets); a downgraded mesh starts it.
3. **Overflow by distance (new).** A scene mesh whose nearest vertex is at
   least 4096 units away may not use the last 48 KB of the scene budget
   (`DNTTK_UNIFIED_FAR_RESERVE_KB`, developer). Found on level 28 (WING'ED
   DEATH) at FOV 120: world 114 KB + props 30 KB + actors 5.5 KB exceeded the
   134.75 KB scene limit, and meshes drawn first took the budget, so 29 near
   meshes later in the frame fell back whole to the original renderer
   (no near clipping). With the reserve, distant meshes downgrade first
   (they have no polygon near enough to be drawn here, so they stay native
   with scene depth: the accepted D17AE renderer) and near fallbacks are 0.
   New debug counter `measure.near_fallbacks`.
4. **D20H: savestate loads in the middle of a frame (fixed, both
   renderers).** See below.
5. **D17AF: water tiles near Duke (fixed).** See below.
6. Debug JSON `near.water` (seen/taken/guard/grid/near_cleared/near_dropped).

## D20H - extreme colour corruption after a load (cause found)

Reproduced during the sweep: the A/B harness loads pose 0 of a scene, takes
captures, then loads pose 1. On level 2 (MINER 69ER) at 120 FPS that
sequence corrupted the picture (fine-grained wrong colours across walls and
floor, the reference look of `research/screencaps/crazy-colors.png`) and,
one cycle later, halted with `FATAL: GPU GP0 unimplemented command 0x1F
(word 0x1F801C00)`: the GPU had walked into memory that is not a packet.
The slot 8 third-person black frame with radial spikes seen in the same
sweep had the same sequence and is consistent with the same cause.

Cause: a savestate is usually taken part way through drawing a frame (the
pose states resume at 0x80031FA4 and 0x80033000, inside the prop and object
loops), and the loaded game finishes that frame. Its ordering table links
into the packets already written to the host packet region recorded in the
D20C mirror. This process's packet cursor (region and offset) came from
whatever it drew before the load, and it was only re-synchronised at the
next frame boundary. Until then new packets could be written over the
restored ones in the same region, so the ordering table pointed into the
middle of different packets: garbage commands that overwrite VRAM, and in
the worst case an unknown command. Held first-person packets from before
the load could be linked the same way. Unified geometry writes far more
host packets per frame, which made the overlap more likely, but the
mechanism is the same for the earlier renderer (the user's D20H report
predates unified geometry).

Fix: `region_after_load()` runs at every near hook, packet allocation and
first-person flush, not only at frame boundaries. On a new load it drops
held first-person packets and pending scene depth and continues in the
region after the mirrored one, from its start; the mirror is updated.

Evidence (private copies of the level-2 pose states, `d20h.py`, unified,
120 FPS, load pose 0, captures, load pose 1, captures at FOV 90 and 120):

| Build | Cycles | Corrupted captures | Halts |
| --- | --- | --- | --- |
| HEAD (unified switch) | 7 (then halted) | 2 | 1 |
| HEAD (unified switch), repeat | 20 | 1 | 0 |
| HEAD, earlier renderer | 20 | 0 | 0 |
| Fixed (unified) | 20 | 0 | 0 |
| Final build (default) | 30 | 0 | 0 |

The pose-1 state loaded on its own 12 times never corrupted: the overlap
needs the process cursor to be in the loaded region below the loaded
frame's packets. New contract: a mid-frame load sends the rest of the
frame to the next region and leaves the restored packets intact. Not
established: that this is the only cause of the user's original report, or
the earlier renderer's real-world rate (0 of 20 cycles here).

## D17AF - slot 10 water vanishing around Duke (cause found, fixed)

Fixture: the user's new UI slot 10 (`state_800AB6FC_slot09`, 2026-10-29
19:38), private copy. Walking the flooded channel, the water overlay
stopped about a third of the way up from the bottom of the view: the tiles
around Duke were missing, in both renderers.

Cause: the native water builder (0x8002ef90..0x8002fc50) flags each grid
vertex 0x10 when its SZ is under H/2 (0x8002f508/0x8002f5b0/0x8002f65c, after
reading H through 0x800b4d2c) and skips any tile with such a corner before
building it (0x8002f70c..0x8002f71c). In the eye view that is the tile Duke
stands in and those beside him, part of which is in view. Those tiles never
reached the D17AF clip hook (0x8002bc18), which only handled tiles with
saturated screen corners.

Fix: while the eye view is live, the near flag (and the meaningless
screen-rectangle bits of a vertex behind the eye) of each stored vertex is
cleared at the 0x800b4d2c calls and again when a row's tiles are built, so
the original builds the tile and the clip draws its visible part. Such a
tile is depth-tested at its surface pushed 32 units back instead of painter
order: its native slot comes from corners at SZ 0 and is far too near, and
painter order laid it over the channel walkway. A primitive trace shows the
walkway grating lies 10-15 units below the water surface; the original
always painted the grating over the water, and the 32-unit allowance keeps
that while the channel floor (150-300 units down) stays under the water. If
such a tile cannot be clipped (grid mismatch, budget) its saturated native
packet is dropped, never drawn. Far and screen culling of other tiles is
unchanged. Third person keeps the original (the camera is far from the
tiles near Duke).

Evidence: same route, HEAD against fixed, FOV 90, 60 FPS: water reaches the
bottom of the view at every step; walkway grating unchanged; third-person
view unchanged. Contracts: near-flag clearing (current/previous row,
caller and Vanilla guards), skipped tile clipped with pushed depth, dropped
on grid mismatch, untouched in Vanilla.

## Re-triage results (D17AM4)

- **D17AF**: fixed as above. Needs the user's walk through the slot 10
  channel.
- **D20H**: cause found and fixed as above. Needs the user's normal play
  with loads.
- **D17AH** (slot 8 sky): the D17AH painter-order sky (note 319) holds under
  unified geometry: continuous clouds while turning at FOV 90/120, 120 FPS,
  HEAD current, HEAD unified and the final build. No change; needs the
  user's look.
- **D17AI** (old slot 12 fixture, wall near the pushable cube): not
  reproduced in either renderer on forward, backward and turning routes
  past the cube at FOV 90/120. The D17AM2 slot-12 panel fix (coplanar
  rank) and scene depth are the likely reasons; needs the user's check.
- **D17AK** (shared FOV sweep): this sweep is its evidence. Shared causes
  found: D20H (packet region after mid-frame loads) and the D17AF water
  rejection. No other shared fault was seen.

## Sweep

Harness: `recomp/analysis/d17am3/` (`lib.py`, `poses.py`, `ab.py`,
`compare.py`, `corrupt.py`, `drive.py`, `run_exe.py`). Private copy of all
player slots (`fixture/`, hashes in `originals.sha256`), offscreen, Corrected
precision, internal scale 2; every run hash- and mtime-checks all player
saves, cards and settings. Scenes: UI slots 1-12 and the 21 level-select
levels (0-3, 5-12, 21-29), 33 in all. For each scene, four pose states
(load view, turned, walked, turned back and walked) were saved once, then
loaded in each renderer:

- `fp60`: first person, 60 FPS, 16:9, FOV 64/90/120, Bio Mask at two poses
  (FOV 90/120);
- `fp120`: as fp60 at 120 FPS, FOV 90/120;
- `tp60`: third person, 60 FPS, 16:9;
- `fp43`: first person, 60 FPS, 4:3, FOV 64/90/120.

A baseline was frozen from HEAD (`.am3-head`, earlier renderer and unified
switch) so later builds could not change it; the final build was frozen as
`.am3-cand2` and run with the switch unset (the new default).

Share of pixels differing by more than 24 levels, per configuration
(median / 90th percentile / pairs at or over 0.5%):

| Comparison | fp60 | fp120 | tp60 | fp43 |
| --- | --- | --- | --- | --- |
| HEAD earlier renderer vs HEAD unified | 0.015 / 2.1 / 98 of 522 | 0.024 / 3.6 / 82 of 388 | 0.018 / 0.68 / 16 of 132 | 0.009 / 1.9 / 59 of 396 |
| HEAD earlier renderer vs final | 0.014 / 2.8 / 97 of 524 | 0.032 / 4.6 / 89 of 392 | 0.016 / 1.5 / 18 of 132 | 0.013 / 2.6 / 71 of 396 |
| HEAD unified vs final | 0.000 / 0.63 / 60 of 522 | 0.008 / 2.1 / 59 of 388 | 0.000 / 0.55 / 15 of 132 | 0.000 / 0.10 / 21 of 396 |

Final-build counters over all 132 A/B runs (185356 frames): 0 packet skips,
0 near-mesh fallbacks, 0 copy overflows, 0 replay failures, 0 misses;
51929 downgrades and 13110 whole-mesh fallbacks, all of distant meshes
(native with scene depth). Every run reported unified geometry with the
switch unset. No capture shows the corruption signature (`corrupt.py`;
the corrupted HEAD capture scored 1.66-1.98, the final maximum is 0.48, a
death screen). The largest final-vs-HEAD-unified differences are the two
HEAD failures (slot 8 black frame, level 2 corruption), drawn correctly by
the final build, and game-state divergence.

Final route counters (`routes-final/`, 120 FPS, FOV 120, all 33 scenes,
30937 frames plus 14828 with the Bio Mask worn in 29 scenes): 0 packet
skips, 0 near-mesh fallbacks, 0 copy overflows, 0 replay failures, 0
misses; downgrades in levels 6, 9, 21, 27, 28, 29 and slots 2, 5, 6.

Far-reserve tuning (FOV 120, turn routes; near-mesh fallbacks are the
harmful case): level 28 with 0 / 16 / 24 / 48 KB: 29 / 0 / 0 / 0; canal
(UI 2) with 0 / 16 / 48 KB: 0 / 12 / 0 (route timing varies between runs).
48 KB is kept.

Findings in the HEAD A/B (current against unified):

- 1438 image pairs; median difference 0.01-0.02% of pixels per
  configuration. Pairs over 0.5% were reviewed as 132 contact-sheet rows
  (worst image per scene, configuration and pose). They are game-state
  divergence (the renderers change emulated timing, so enemies, projectiles,
  explosions, Duke's falls and deaths, animated lights and view bob differ
  after a load), except: the D20H corruption and the slot 8 black frame
  (both explained above), and a shading difference on walls very near the
  eye (one Gouraud quad interpolated across the screen, as the PS1 does,
  where the earlier renderer used many small pieces). No missing or
  misplaced geometry was found.
- Unified route counters on all 33 scenes: no packet skips, copy overflows,
  replay failures or misses; Bio Mask worn in 31 scenes (level 0's start
  offers no mask; slot 4's mask leg ended in death). Level 28 exceeded the
  budget (fix 3 above).
- Harness defects found and corrected (not renderer faults): a pose job
  still holding a debug port made two baseline A/B runs drive another
  process (rerun); 4:3 is `--widescreen off`, not `4:3` (rerun).
- Pre-existing, unchanged: on the level-select loading screen the frame
  boundary is never detected, so host packets accumulate in one region
  until the loading disc falls back to the original renderer (same with
  the earlier renderer; the disc stays visible).

## Contracts and native suites

`ttk-near-test recomp/disc/SLUS_005.83` passes with the switch unset and
`=0`, each with Corrected and Original precision. Updated: the default run
covers unified; the earlier-renderer contracts run with `=0`; contracts for
the removed special cases were removed or moved inside the near boundary.
Two stale contracts were repaired: D17Z expected the pre-D17AE 480-pixel
guard band (it failed with Corrected textures at HEAD, as recorded in the
D17AL handoff), and the D17N whole-polygon contract re-altered a caller
instruction the water contract had already restored. `ttk-input-test`,
`ttk-aim-test`, `ttk-sky-test`, `ttk-gas-test`, `ttk-live-env-test`,
`ttk-loading-test` and `ttk-controls-test` (LEVEL00 guard fixture,
LEVEL01) pass.

## Performance

One game at a time, idle machine (load average under 2), offscreen 16:9,
120 FPS, FOV 120, 30 s turn/walk route after the load (`perf.py`). Emulated
frames per second, live hook time (average/peak ms per frame), 120-FPS
worker time per in-between image (average/max ms):

| Scene | Build | Emulated fps | Hook ms | Worker ms | Misses / failures |
| --- | --- | --- | --- | --- | --- |
| Canal (UI 2) | HEAD earlier renderer | 54.96 | 0.75 / 5.0 | 16.1 / 44.0 | 0 / 0 |
| | HEAD unified | 58.06 | 1.62 / 7.8 | 12.4 / 24.4 | 0 / 0 |
| | Final | 55.69 | 1.07 / 7.7 | 11.8 / 38.6 | 0 / 0 |
| Western street (UI 5) | HEAD earlier renderer | 58.22 | 0.95 / 5.5 | 13.5 / 37.7 | 0 / 0 |
| | HEAD unified | 59.96 | 0.96 / 6.6 | 8.4 / 21.4 | 0 / 0 |
| | Final | 59.98 | 0.81 / 6.4 | 6.4 / 20.8 | 0 / 0 |
| Level 28 | HEAD earlier renderer | 54.56 | 1.06 / 10.8 | 11.3 / 51.9 | 0 / 0 |
| | HEAD unified | 58.19 | 1.41 / 11.8 | 8.5 / 29.8 | 0 / 0 |
| | Final | 57.92 | 1.24 / 11.3 | 8.9 / 30.4 | 0 / 0 |

The final build is at least as fast as the earlier renderer in every
scene. "Emulation behind real time" lines appear in the canal and level 28
with every build, including the earlier renderer; offscreen capture is not
the D23 present-cadence measurement, so a live 120 Hz check remains part of
the playtest.

## Limits

- Offscreen captures with several games running in parallel; image A/B
  after a load cannot be pixel-exact once the game state diverges.
- Third person is covered at 60 FPS, 16:9 only; 4:3 at 60 FPS only;
  Original precision only by contracts.
- D20H: one reproducing sequence; the fix is general but other causes of
  the user's report are not excluded.
- D17AI not reproduced, so not proven fixed.
- The water depth allowance (32 units) is tuned on slot 10's channel.
