# 508 - D23Y5 the title's host world per frame: what can be reused, the object coplanar prune (field laptop)

Job: [D23Y5](../jobs/campaign-performance.md#d23y5---a-lighter-emulation-thread).
Builds on [note 506](506-d23y-chain-baseline.md) (where the emulation
thread's time goes) and [note 507](507-d23y8-gl-server-d23y5-d23af3.md)
section 3 (why the 50% target was out of reach). Labels: MEASURED (run
here), CODE (read from the source), INFERRED.

## 1. Summary

- The title's host world at the original 60 on the field laptop is 11-24%
  of the emulation thread (section 2). Almost all of it is per-face work
  that depends on the view: projecting corners, culling, clipping at the
  near and screen planes, subdividing, writing packets. Nothing of that
  can be reused from one frame to the next exactly.
- One block was large and needed no view: **the coplanar search of
  objects** (props and actors), 8.5% of the emulation thread in UI slot 9
  (about 4.8 points of a core) and 0.3-0.5% elsewhere. World meshes already
  cache their pairs (D17AR3); objects compared every pair of planar faces in
  view space, every frame.
- A model-space cache of those pairs is **not exact** (section 3.1), so the
  search was made cheaper instead, by a pruning that skips only pairs the
  rule provably rejects: the same pairs in the same order, exact by
  construction (section 3.2). On by default; developer
  `DNTTK_COPLANAR_PRUNE=0` restores the full search, `=2` runs both.
- Exactness (MEASURED): 173,329 object meshes of the final version
  compared with the full search (desktop, four scenes, and the field
  laptop, two), 0 differing; `PSX_VRAM_DUMP_DIR` dumps byte-identical off
  against on on the desktop (section 4).
- Cost (MEASURED, field laptop, the original 60, real screen): in UI 9 the
  search 8.6% -> 3.6% of the emulation thread and the emulation thread
  57% -> 55%; the other scenes unchanged (the search was 0.2-0.7% there).
  **D23Y5's target (under 50%) is still not met** (54-63%); what is left in
  the host world is view-dependent emission (section 2).

## 2. Per-frame profile at the original 60 (MEASURED)

Field laptop (i7-5600U, 3.1 GHz, real 1080p screen), build of note 507
(HEAD `4344886` source), lab `recomp/analysis/d23y8` case `cad` (savestate,
then 30 s of walking, turning and firing), FRAME RATE 60 with the AUTO
grade rows, one profiled run per scene (the runtime's stack sampler, 1 ms
of emulation-thread CPU). Inclusive share of the emulation thread:

| Block | UI 2 | UI 9 | DUKE HILL town | FAMILY JEWELS start |
| --- | --- | --- | --- | --- |
| emulation thread (unprofiled runs) | 56% | 57% | 61-62% | 56% |
| object pass (`object`) | 5.1 | 17.2 | 7.1 | 7.6 |
| of it: coplanar search (`rank_coplanar`) | 0.5 | 8.6 | 0.5 | 0.6 |
| of it: smooth model normals (D41H) | 0.5 | 0.1 | 0.8 | 0.6 |
| visibility walk (`host_vis_walk`) | 6.7 | 4.5 | 6.6 | 6.7 |
| of it: room and piece selection | 0.1 | 0.1 | 0.1 | 0.1 |
| of it: world pieces (`world`, near clip) | 6.4 | 4.3 | 6.5 | 6.8 |
| `draw` (clip, subdivide, emit) of both | 10.8 | 11.0 | 9.5 | 9.7 |
| `emit` (packets) | 6.7 | 3.9 | 5.6 | 5.5 |

Per frame at 56-62% of a 16.7 ms field the whole host world is about
1.1-2.3 ms, of which the selection of rooms and pieces is about 10 us.

What each block's time is (leaf view, CODE):

- `draw`/`emit`: corner positions in view space, the five clip planes, the
  depth plane, subdivision, screen positions and colours rounded, packet
  words written to guest memory through the bus (`psx_write_word`, about
  1.7% in UI 2 alone). All of it follows the camera.
- `world`: the mesh's vertices projected (GTE-exact), faces sorted into
  host and native, culling against the eye and the room rectangle. The view
  enters every step. Its view-independent parts (the face list, texture
  entries, the baked lamp surface lookup, 0.1-0.2%) are read from guest
  memory each time; checking them unchanged would cost the same reads.
- `smooth_model_normals` welds corners by exact equality of their view
  positions; two model corners can round to one view position, so a weld
  kept in model space would not be the same weld. Not cached.
- The walk's own selection (portal walk, box tests) is about 0.1%.
- Outside the host world but listed by the same profile: the step probe
  of modern controls (`terrain_step_probe`, 1-2% in the town and FAMILY
  JEWELS) calls the game's own collision routine; it is gameplay, not a
  rendering result, and is left alone.

## 3. The object coplanar search

### 3.1 Why not a cache (CODE, INFERRED)

`rank_coplanar` (D17W) finds, for each mesh drawn, the pairs of planar
faces that lie on one plane and overlap, using the faces' corners in view
space: normals within `|dot| >= 0.9999`, every corner of the second face
within 0.5 units of the first face's plane, overlap of more than 0.5 units.
For world meshes D17AR3 found the pairs once per mesh in the level's own
integer coordinates, because a world piece is drawn with the camera's
rotation and a translation. An object is drawn through its own GTE matrix
(rotation, any scale the model carries) and its corners are rounded where
the GTE rounds them; the 0.5-unit tolerances and the angle test are not
invariant under that, so pairs found in model space could differ from the
pairs found in view space for faces near either threshold. Such a cache
would change, rarely, which face wins a coplanar fight, so it was not built.

### 3.2 The prune (CODE)

The search's rule is unchanged; it now runs only on pairs it could accept
(`coplanar_pairs`, objects only; world meshes keep their buckets). A first
version (normals and offsets from the eye only) was exact but saved
nothing: UI 9's heaviest model has 196 planar faces, most of them parallel
and on nearly one plane, so 13.4 million pairs in 20 s still reached the
rule and none was found (MEASURED). The final version has three bounds:

- Normals are unit length (`near_depth_plane`). If `|dot| >= 0.9999`, with
  s the sign of dot, `|a - s b|^2 = 2 - 2|dot| <= 0.0002`, so
  `|a - s b| <= 0.01415`, and for every component
  `||a_k| - |b_k|| <= |a_k - s b_k| <= 0.01415`. Faces are sorted by
  `|n.x|` and compared only within 0.015 of it, then by `|n.y|` and
  `|n.z|` within 0.015.
  In code the faces are bucketed by `|n.x|` in steps of 0.015, so a pair
  lies in one bucket or two neighbouring ones.
- Plane offset: the rule does not change when the origin moves, so offsets
  are taken from O, the first planar face's first corner:
  `e = n . (c0 - O)`. The rule needs b's first corner within 0.5 of a's
  plane, and `n_a . (c0_b - O) = s e_b + (n_a - s n_b) . (c0_b - O)`, so
  `|e_a - s e_b| < 0.5 + 0.01415 |c0_b - O|`. Within a bucket faces are
  sorted by `|e|` and only those within `0.5 + 0.015 R + 0.001` (R the
  largest `|c0 - O|`, the model's size, not its distance) are visited; each
  pair is then held to its own bound.
- In-plane overlap: the rule's first separating axes are the normals of
  a's edges in a's plane, fixed 3D vectors V for face a (precomputed with
  a's interval on each). b's corners lie in their bounding box, so b's
  interval on V lies inside the box's interval on V; if even that overlaps
  a's interval by less than `0.5 - 0.001`, the rule rejects the pair. No
  convexity is assumed.
- The 0.001 margins cover rounding (coordinates are GTE integers, far below
  1e6).
- Pairs that pass go through the original rule and are returned sorted, as
  the full search returned them (record order), so ranking is unchanged.
- A face with a plane or first corner that is not finite sends the mesh
  back to the full search (counted as `prune_fallbacks`; none seen).

Developer `DNTTK_COPLANAR_PRUNE=0` compares every pair as before; `=2` runs
both and counts meshes whose pairs differ (`coplanar.prune_checked`,
`prune_mismatch`, `prune_fallbacks` in the near clip's debug JSON). The lab's
`copl` case prints them.

## 4. Exactness (MEASURED)

In-process comparison (`DNTTK_COPLANAR_PRUNE=2`, desktop offscreen, the
original 60, case `cad` then `copl`):

| Scene | meshes compared | differing | fallbacks |
| --- | --- | --- | --- |
| UI 2 | 16,899 | 0 | 0 |
| UI 9 | 20,720 | 0 | 0 |
| DUKE HILL town (264 pairs found) | 35,838 | 0 | 0 |
| FAMILY JEWELS start | 38,591 | 0 | 0 |
| field laptop, UI 9 (real screen) | 24,916 | 0 | 0 |
| field laptop, DUKE HILL town (276 pairs found) | 36,365 | 0 | 0 |

(The first version was compared too: 117,153 meshes on the desktop and
60,872 on the laptop, 0 differing.) In UI 9 the rule now runs on 0.73-1.06
million pairs in 20-30 s instead of 13.4 million in 20 s.

`PSX_VRAM_DUMP_DIR` (case `vwait`: savestate, no input, VRAM and the
presented picture at +60, +200 and +400 vblanks), prune off against on,
desktop offscreen: UI 2, UI 9 and DUKE HILL town byte-identical (VRAM and
pictures); Vanilla UI 9 identical. FAMILY JEWELS start is not repeatable
on its own: three runs with the prune off gave two different results and
three with it on gave two, the common result shared by both (the scene's
asynchronous host lighting, note 507 section 2.4); the in-process
comparison above covers it.

Field laptop (real screen): Vanilla UI 9 identical off against on. The
Modernized dumps are not repeatable there between identical runs (the
town gave two different +60 dumps with the prune off; UI 2's +60 differed
once with it on, its +200 and +400 identical), as note 507 found for the
laptop's pictures; there the in-process comparison is the proof.

## 5. Cost (MEASURED)

Field laptop, real screen, the original 60 with the AUTO grade rows, case
`cad`, prune off and on interleaved, two runs each (host gate passed every
run; no kernel GPU errors). Emulation thread (median of each run's seconds)
and the search's inclusive share from one profiled run of each:

| Scene | emulation thread off | on | search off | on |
| --- | --- | --- | --- | --- |
| UI 2 | 55, 54 | 54, 54 | 0.5 | 0.4 |
| UI 9 | 57, 57 | 55, 55 | 8.6 | 3.6 |
| DUKE HILL town | 63, 62 | 62, 63 | 0.5 | 0.7 |
| FAMILY JEWELS start | 56, 56 | 56, 58 | 0.6 | 0.7 |

Game frame rate 60.0 (lowest tenth 58.9-59.8) in every run either way.
The gain is UI 9's (about 2 points); elsewhere the differences are within
the runs' spread. In UI 9 the remaining 3.6% is mostly visiting the
196-face model's pairs in its one bucket; a cheaper visit (for example a
grid in the plane) would take most of what is left, about 2 more points
there.

## 6. State

The prune is on by default in Modernized (the object coplanar ranking runs
only with the unified renderer); Vanilla never reaches it. The change is
title code (`recomp/src/ttk/near_clip.cpp`); no framework change.

## 7. Reproduce

Lab `recomp/analysis/d23y8/` (local): `y5ab.sh` (the field laptop batch:
timing off/on interleaved, a profile of the default, the in-process check,
VRAM dumps), `vx.sh` (dumps), `lab.py` case `copl`, `sub.py TAG ROOT...`
(the stack samples under a function, by child and leaf).
