# 511 - D23AI: BALANCED's GPU passes on the field laptop's HD 5500

Job: [D23AI](../jobs/campaign-performance.md#d23ai---balanced-gpu-passes-on-the-field-laptops-hd-5500).
Builds on [note 509](509-d23ag-overnight-chain.md) section 3 (BALANCED's GPU
work takes the 15 W package's power from the CPU) and [note 510](510-d23ak-what-a-redraw-shares.md)
(D23AK: the redraw worker 30% lighter, now the Modernized default). Labels:
MEASURED (a run on the named machine), CODE (read from the source), INFERRED.

## 1. Summary

- Each pass of a BALANCED redraw is now timed on the GPU timeline
  (developer `PSX_GPU_PHASES=2`, section 2).
- Field laptop, real 60 Hz screen, BALANCED, FRAME RATE 60, D23AK defaults:
  a redraw is 6.0-7.6 ms of GPU, and 75-80% of it (4.6-6.0 ms) is the lit
  textured batches. The rest is small: the band copy 0.34, wide clears 0.33,
  fills and copies 0.24, the snapshot load 0.11 ms. The present is 0.7 ms.
  No lean textured batch is drawn at BALANCED: every textured batch takes the
  full program.
- Inside the lit textured time (compile-out A/B, section 4): the lighting
  layer is 1.2-2.0 ms (25-35%), the shadow lookups and the exact-edge layer
  are within the runs' spread (about 0.3 ms). The base textured fragment work
  (3.3-4.6 ms) is the larger part.
- Tried, not kept (both exact): the full program without the exact-edge layer
  while SHADOW EDGES is SIMPLE (no gain on this GPU), and D23U's depth prepass
  (it saves 1.0-1.3 ms of lit shading and costs 0.5-1.0 ms itself).
- With the D23AK bundle the medians are already 59-60 new pictures on every
  measured level; the lowest tenth dips on levels 6, 22, 27 and 29 (32-57).
  No exact GPU cut found here moves them. Removing the lighting layer
  (a picture change) did lift level 22's lowest tenth (55 against 34-44):
  GPU time saved does turn into pictures on this package.

## 2. Instrumentation (CODE, developer only)

`gpu_gl_renderer.c`, all behind the existing `PSX_GPU_PHASES=2`; with the
switch off nothing runs (one flag test per call):

- The redraw kinds (`[gpu kinds]` lines) gain `tex-lit` (a textured batch
  drawn with the full program; `tex` is now the lean one only),
  `shadow-map` (shadow-map renders inside a redraw) and `present`.
  `tex_batch_cat` repeats `tex_prog_pick`'s test.
- `gp_span(kind, cat, end)`: inside a redraw it pushes the kind onto the
  redraw's timeline; outside one it is a timeline span of its own (kind 2
  shadow maps, 3 presents) with mean and maximum, logged every 5 s as
  `[gpu passes] ... outside redraws: shadow maps ... present ...`.
- The present span brackets `rp_present_slot`'s own GPU work: bezel, the
  scaled quad with its LUT and tint, the overlay, through the swap.
- Summarizer: `recomp/analysis/d23ai/kinds.py game.log` (local): per
  renderer instance, ms a redraw per kind, weighted by each block's redraws.

Caution (note 459): timestamps serialize the GPU, and the GPU's clock moves
with the package's power, so the milliseconds are for shares and A/Bs within
one machine, not absolute capacity.

## 3. The passes (MEASURED)

Field laptop (i7-5600U, HD 5500, 15 W), real 60 Hz screen, census route
(d23ag `census.py`), BALANCED, FRAME RATE 60, third person, one level a
launch, host gate OK. GPU ms a redraw, the render thread's instance:

| level | redraw | tex-lit | band | wide-clear | fill-copy | other kinds | new pictures median / lowest tenth | GPU % |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 6 | 7.56 | 6.05 | 0.35 | 0.33 | 0.24 | flat 0.29, load 0.12, mirror 0.09 | 59.6 / 36.0 | 53.7 |
| 21 | 6.81 | 5.71 | 0.35 | 0.32 | 0.24 | load 0.11 | 60.0 / 59.2 | 44.8 |
| 22 | 6.81 | 5.36 | 0.35 | 0.33 | 0.24 | mirror 0.31, load 0.12 | 59.2 / 44.1 | 45.9 |
| 24 | 5.97 | 4.64 | 0.34 | 0.35 | 0.25 | mirror 0.18 | 60.0 / 59.0 | 49.1 |
| 26 | 6.00 | 4.74 | 0.34 | 0.33 | 0.25 | mirror 0.12 | 60.0 / 59.5 | 42.9 |
| 27 | 6.19 | 5.09 | 0.33 | 0.32 | 0.23 | load 0.11 | 59.7 / 52.5 | 45.4 |
| 29 | 6.66 | 5.60 | 0.33 | 0.30 | 0.22 | load 0.11 | 59.8 / 50.2 (11 s fallback) | 45.9 |

Outside redraws: the present 0.67-0.82 ms at 59 a second. Shadow maps drawn
outside a redraw are rare (a level's start) but take 20-44 ms each; inside
redraws they average under 0.01 ms.

Overdraw (`[gpu overdraw]`, fragment shader invocations a redraw against its
654,720 output pixels): level 6 2.9-3.8 a pixel, levels 22 and 27 1.2-1.9.

Desktop (GTX 1080 Ti), offscreen, THE REAPER, the same settings: a redraw
0.90 ms, of it tex-lit 0.80; present 0.03 ms.

## 4. Inside the lit textured time (MEASURED)

`PSX_SHADER_STRIP` (existing developer switch, D41F1) compiles a layer out of
the full program. `exact` draws the same pictures while SHADOW EDGES is
SIMPLE (BALANCED's setting: CODE, `shadow_vis` returns at its one-texel
lookup when `u_shmode` is 0, before any exact-edge code); `shadows` and
`lights` are cost bounds with wrong pictures. One run each, interleaved by
configuration:

| level | off | exact | shadows | lights |
| --- | --- | --- | --- | --- |
| tex-lit ms, 6 | 5.88 | 5.67 | 6.06 | 4.65 |
| tex-lit ms, 22 | 5.26 | 5.54 | 5.45 | 3.50 |
| tex-lit ms, 27 | 5.47 | 5.35 | 5.26 | 3.31 |
| lowest tenth, 6 / 22 / 27 | 59.1 / 39.4 / 57.4 | 38.1 / 43.1 / 54.5 | 40.6 / 48.0 / 57.9 | 56.0 / 55.3 / 58.1 |
| GPU %, 6 / 22 / 27 | 53.9 / 43.1 / 41.5 | 55.4 / 46.8 / 40.8 | 56.4 / 49.0 / 41.0 | 45.3 / 36.9 / 38.6 |

The exact-edge layer and the shadow lookups are within the spread here; the
lighting layer is 1.2-2.0 ms. Texture filtering is off at BALANCED (no
profile option sets it), so a CLUT texel costs two dependent fetches: the
base cost is the program's arithmetic and its register pressure.

## 5. Tried, not kept (MEASURED)

- The full program without the exact-edge layer, chosen while SHADOW EDGES
  is SIMPLE (two more program variants, plain and with the night-vision
  remap). Exact: desktop VRAM dumps and present shots at +60/+200/+400
  (d23y8 `vx.sh`, BALANCED, FRAME RATE ORIGINAL, lab slots 1 and 8) matched
  off, with 13-17 shadow-map renders and the variant drawing. No gain on the
  HD 5500 (section 4's `exact` column), so it was removed (leanest choice).
- D23U's depth prepass (`PSX_GL_ZPREPASS=1`), two interleaved runs each:
  tex-lit 5.3 -> 3.9-4.7 ms, but `zpre` itself 0.5-1.0 ms; a whole redraw
  5.9-7.3 against 6.5-7.3 ms off; GPU 43-48% against 42-56%; lowest tenth
  6: 48.9 / 32.2 against 59.2 / 34.9, 22: 48.3 / 45.1 against 43.9 / 34.4,
  27: 57.8 / 57.8 against 56.0 / 52.5 (one 13 s fallback in the prepass
  arm). Within the spread; note 469 found the same on this laptop. It stays
  off by default.

## 6. Where that leaves BALANCED 60 on the laptop (PROPOSED)

- The dips are seconds where the CPU and GPU share the 15 W package (note
  510 section 6c). GPU time saved helps them (section 4's `lights` column),
  but the passes that could be cut exactly are already lean; the remaining
  GPU work is the full textured program on every textured pixel.
- Look trade-off for the user to choose (none applied): LIGHTING ORIGINAL on
  the laptop saves about 1.2-2 ms a redraw (a quarter to a third of the lit
  textured time).
- A larger exact project: draw the triangles that no light, bake or ambient
  change reaches with the lean program, splitting runs in their order. The
  shader's plain path is not provably bit-identical to the lit path with
  zero lights (`mix(a, a, t)` and the soft limit in floating point), so it
  needs its own validation, and the extra draws cost the render thread.
- Rare 20-44 ms shadow-map renders outside redraws (a level's start) are a
  possible hitch source, not seen in these runs as a slow second.

## 8. Follow-up: the user's test 1 on the laptop (MEASURED)

User, 2026-11-18, first person: "it literally went down to 22fps". Session
log (`recomp/analysis/play/logs/` on the laptop); host idle throughout (AC,
no backups, updates or timers, nothing in the journal).

- The 22: on level 3, after standing still about a minute, one second at
  21.4 game frames and 21 presents with the audio buffer drained, while the
  console was open just before `level 6`; the Modernized console does not
  pause the game. Not reproduced (lab `d23y8/lab.py` case `cstall`: 70 s
  idle, console open and close, `levels`, `fps`, `help`, `level 6` by the
  debug port and typed with real X key events; 11 attempts, slowest second
  55.8, longest frame gap 86 ms). Cause unknown; D23AM, Monitoring.
- Level 6 (FAMILY JEWELS): the game thread fell to 53-55 frames a second
  twice and the governor showed the original presentation for 15 s, then
  30 s. Census, first person, two runs: median 59.1 / 59.3, lowest tenth
  30.2 / 27.1, no fallback; in the slow seconds the worker 93-99% busy, the
  game 60 fields, the package between 23 W (3.1 GHz) and 15 W (1.9-2.5 GHz).
  Worker profile (670 and 945 samples): first person and third person alike,
  about 80% the game's own code, host drawing 22-26%, objects 20-26%. Not a
  GPU pass; D23AL.
- GOLD AND GUNS at UI slot 9 (user: "like 20 something unique frames"),
  the lab's identical copy, first person, standing: new pictures median
  24.4-26.1, lowest tenth 21; worker 96% (about 19 ms a job against 7.5 facing
  away), 21-35 of 50-55 finished jobs a second shown in time; GPU about 85%
  busy, 10.7 ms a redraw (lit textured 9.3). Worker profile (25,095
  samples): world walk 29%, sleeping enemies 18%, objects 23%. Worker-bound
  first, with the GPU close behind; D23AL.

## 9. The slot 9 view itself (MEASURED, desktop, the lab's identical copy)

User: "why dont we try perhaps actually analyzing that area in my statesave
... if the end user doesnt see any physical difference im 100% here for it".

- Not a vast area: a narrow mine tunnel (GOLD AND GUNS). Host visibility:
  10 rooms, 48 world pieces, about 160 world polygons a frame. Objects: about
  93 meshes a frame (about 640 polygons, 800 triangles). Of the level's 375
  objects 6 lie within 8,000 units, 112 within ORIGINAL's 26,416.
- Sleeping stand-ins: 9 listed, 5 drawn a frame, about 0.5 ms each on the
  desktop game thread (the game's own actor renderer per stand-in). All lie
  inside the camera's view cone (down the tunnel); 2 a frame lie outside
  their room's screen rectangle (behind the tunnel's walls): developer
  `DNTTK_SLEEP_VIEW_TEST=1r`, game thread 2.7 -> 2.2 ms a frame.
- Props: about 30% of the draws of the prop renderer 0x80031d10 lie wholly
  outside their room's screen rectangle (developer `DNTTK_OBJ_RECT_STATS=1`,
  a box of 4 x the actor's radius, at least 1,536 units), in the game thread
  and the redraw worker alike.
- So the view's cost is objects and stand-ins drawn behind walls and down
  the tunnel, not the map's size. A per-object room-rectangle test is a
  game-wide rule (every level, custom ones included); exactness needs a
  bound that covers each object's mesh, then the VRAM-dump and in-worker
  checks. Estimated (INFERRED) 10-15% off a worker job in this view.

## 7. Reproduce

`recomp/analysis/d23ai/` (local): `kinds.py` (the summary), `vxab.sh`
(exactness A/B with d23y8 `vx.sh`), `laptop/` (the laptop's run scripts
`aiph.sh`, `aist.sh`, `aizp.sh` and their logs; the game logs stay in the
laptop's `recomp/analysis/d23ag/runs/laptop-{ph,st,zp}-*`).
