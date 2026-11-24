# D23AR - GOLD AND GUNS, the top level in shot

Job: [D23AR](../jobs/campaign-performance.md#d23ar---gold-and-guns-top-level-in-shot).
The user, playing GOLD AND GUNS (level 3) on the field laptop: the slowdowns
come from the top level being in shot, not from the lava pit. The player's
slot 8 is that view (a read-only copy in the local labs as `top08`).

Labels: MEASURED (a run on this build), INFERRED (from measurements, not
tested directly).

## 1. What the view costs

MEASURED, field laptop (i7-5600U, 15 W), real 60 Hz screen, the player's
settings, the D23AH LTO+PGO build, the view held still 25 s: 23-28 pictures a
second, redraw worker 96%; with DRAW DISTANCE ORIGINAL 60 pictures; the lava
pit looked down on 60.

MEASURED, desktop, SDL offscreen, the same view loaded from the lab card,
DRAW DISTANCE EXTENDED (developer traces `DNTTK_HOST_VIS_TRACE=1`, now with
time per room, and the new `DNTTK_SLEEPING_TRACE=1`, process id on each line
so the redraw workers' lines separate from the live frame's):

- 11 rooms, 49 room pieces a picture. Five far rooms are 3-14 native pixels
  across with one piece each (0.24 ms together); the far end of the tunnel
  (rooms 49 and 50, 20 pieces through openings about 65 x 60 pixels) costs
  1.6 ms of the world's 3.3 ms. Those rooms are visible (drawn with ORIGINAL
  too): culling rooms by screen size would cut away scenery and save little.
- Sleeping enemies: 26 eligible, 5 drawn, three on screen about 7 native
  pixels tall (88 pixels in a 1024 x 576 shot), 0.7-0.8 ms each, 2.4 ms a
  picture on the workers.
- Worker profile (2734 stacks): host shape building `draw_shape` 35%, the
  final per-corner stage `emit` 25%, sleeping enemies 18.5%, the dispatcher's
  code-validity checks for guest calls (`dirty_ram_dispatch_inner`,
  `overlay_loader_dispatch`, `cand_crc`, `text_native_ok_ranges_slow`) 10%.

The cost of a face does not depend on its size on screen: a far face takes the
same precise path as a near one. Two parts of that path only matter for large
faces, and they are now skipped for small ones.

## 2. What changed (Modernized, unified geometry only)

1. Smooth quads by screen size (`near_clip.cpp`, `smooth_quad_large`). D17AV
   draws a flat world quad whose corner colours do not blend evenly as four
   triangles meeting at its centre. That split was 43% of the world's emission
   at this view (MEASURED: 3.26 -> 1.87 ms with `DNTTK_SMOOTH_QUADS=0`). It is
   now made only when the quad's screen extent reaches 8 native pixels.
   Sweep (desktop, same view): 4 px saves 4%, 8 px 16%, 16 px 28%, 32 px 35%
   of world emission; 8 px changes 319-354 pixels by more than 4 colour steps,
   all at the far end of the tunnel; 16 px visibly changes torch-lit floor and
   ceiling patches at mid distance. A rule weighting the threshold by the
   corner-colour twist was tried and did worse. Developer
   `DNTTK_SMOOTH_MIN_PX=<px>` (0: every quad, as before), console
   `smoothquads min <px>|default` (live frames only).
2. Small models skip refinement (`near_clip.cpp`, `object`). A model whose
   projected vertices span under 6 native pixels skips coplanar ranking and
   smoothed normals. MEASURED: sleeping enemies on the workers 2.38 -> 2.11 ms
   a picture, pictures identical to the colour step (12 px: 41 pixels change
   by up to 11 steps). Developer `DNTTK_SMALL_MODEL_PX=<px>` (0 off).

Also tried for small models and dropped: drawing them with their own native
packets (no gain), and skipping limbs under 3 pixels entirely (8%). Most of a
sleeping enemy's cost is the game's own actor code and the dispatcher's checks
on its calls, not the mesh stage, so a mesh level of detail cannot pay for it.

MEASURED, desktop workers, both rules off against on (two runs each): world
3.31 -> 2.85 ms, sleeping enemies 2.41 -> 2.15 ms, about 0.73 ms of a 10.5 ms
picture (7%).

## 3. Field laptop

MEASURED, the same D23AR build (normal, not PGO) on the laptop, real screen,
the player's settings, `top08` held 25 s, rules off and on interleaved, six
rounds each. The runs fall into two states, whichever setting:

- A: the game holds 60 fields a second, the redraw worker is saturated (95%),
  21-23 pictures a second.
- B: the emulation thread is saturated (87-90%), the game slows to 33-36
  fields a second (55-60% speed), 26-29 pictures.

INFERRED: B is the redraw-only fallback (the log's "redraw-only off: workers
behind"), with live frames drawn on the emulation thread. Which state a run
settles in decides far more than this job's change. Within state B the rules
give 26.3-27.3 -> 28.2-29.0 pictures and 32.8-32.9 -> 33.7-35.7 fields a
second (+5%). The state A pairs are confounded by the 15 W clock (the "on"
runs at 2.2 GHz, the "off" ones at 2.8-2.9 GHz).

## 4. The larger levers, not built

- The redraw-only fallback in this view: half the game's speed in state B.
  Belongs with D23AQ (the laptop's struggle points); measure how often the
  player's laptop sits in it.
- The dispatcher's validity checks on guest calls inside redraws (10% of the
  worker, exact; framework).
- Smooth quads in the shader: the blend done by the GPU on one quad instead of
  four emitted triangles (up to 43% of world emission; renderer work, close
  but not identical to the current four-triangle fan).

## 5. Installed

The LTO+PGO build with this change is installed on the desktop and the field
laptop (the laptop's previous executable kept as `.pre-d23ar`). MEASURED,
desktop workers at the view on it: world 2.47 ms, sleeping enemies 1.67 ms a
picture.

## 6. Reproduce

Local lab `recomp/analysis/d23ar` (ignored): `count.py`, `workers.py`,
`sweep.py` (switch sets per run, worker and live-frame medians, a present shot
each), `diff.py`, `stacks.py`; on the laptop `~/d23ar/lap-ar.sh` with the
D23AH census lab.
