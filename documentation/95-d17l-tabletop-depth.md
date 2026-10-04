# D17L - Native tabletop overwrote the depth-tested cup

2026-10-04. **Done - user-accepted.** The user authorized background development launches. All runs in
this pass use offscreen rendering, private profiles and dated save/card copies.
No foreground window was needed. This continues the independently written
[R01-inspired primitive diagnostics](94-d17l-primitive-diagnostics.md).

## Reproduction and identified primitives

Private UI 11 retains the reported hash
`267c6a268992f8468179211291031a668908dfa081c70bcf9439280004e933ef`.
Approaching and retreating from the central table shows the cup's lower portion
cut off at distance and restored closer to the table. This is partial overdraw,
not an entire missing mesh or a subpixel crack.

The trace identifies cup instance `0x801de8d0`, mesh `0x80129a3c`, and its
supporting table instance `0x801de6f0`, mesh `0x8011baa0`. Other placements of
the same cup mesh are separate trace identities. These addresses are evidence;
no runtime repair condition uses a mesh, instance, texture or save-slot ID.

One stationary capture joins title producer records to actual GP0 submission:

- The cup has six emitted host triangles at OT slot 159. Its visible projected
  silhouette spans roughly X=14..20, Y=-12..-8 before GPU drawing offsets.
- The distant table retains native ownership. Its top face is source record
  `0x8011bb0c`, indices `[1,0,2,3]`; its projected corners are
  `(-4,-4), (11,-9), (70,-4), (66,-9)`.
- In `d17l-packets-gpu-57.json`, cup submissions 30130-30135 match host packet
  addresses from the title trace (GP0 source points four bytes past the tag).
  The top is the later native FT4 submission 30148 at GP0 source `0x001100e8`.
- The table top's original AVSZ4 key is slot 156. Thus it paints after the
  cup despite the cup being physically above its surface. It overlaps the
  lower cup pixels and has no host-depth test. The cup's own depth metadata
  cannot protect it from a later painter-only primitive.

This explains why the previous compact-prop depth extension did not finish
D17L: it changed the cup, while leaving this distant occluding surface native.
As the table approaches, it enters the existing near-depth path and the cup
becomes complete. Stable bar props remain useful controls, not evidence that
all supporting surfaces use the same renderer path.

## Bounded correction

`near_clip.cpp` gives distant opaque top faces of simple static boxes host depth
without changing their ordinary distant appearance:

- The existing authenticated static-prop caller must be active, with precise
  Modernized rendering and compact-prop depth enabled. Actors, articulated
  dancers, the viewmodel and Vanilla do not qualify.
- All eight mesh vertices must be distinct corners of a nondegenerate local
  bounding box. The eligible face lies entirely on its upper boundary (minimum local Y).
  Compound scenery, sloped/lower/side faces and translucent/faded surfaces do
  not acquire this new path. Existing near/unsafe-face handling stays in place.
- Keep integer GTE corner positions, original PS1 triangle split, native
  AVSZ3/AVSZ4 scale and polygon OT bias, original far rejection and integer
  NCLIP for single-sided faces. Preserve affine UVs with equal texture weights;
  raster-plane depth is separate. No subdivision or contact offset is added.
- Keep whole-mesh packet-budget fallback and existing near-path guards.

The top now participates in depth testing against the cup instead of simply
painting over it. There is no crack rim, blanket culling change, enlarged prop
radius, scene-specific offset or all-object depth takeover. The original
movement and pixel snapping remain visible. D15's optional precision settings
have not been implemented or enabled by this repair.

Developer comparison: `DNTTK_STATIC_TOP_DEPTH=0` restores the preceding path.
It is not a saved preference or a player setting. Default is enabled within
the guarded eligibility above.

An initial experiment covering all distant static upper faces restored the cup
but added unrelated compound scenery and costly depth runs. It was rejected:
120-target samples fell to about 112 distinct images/s with output underruns.
The final eight-corner-box restriction replaces that experiment.

## Validation and limits

The owned-EXE native harness checks actual emitted packet contracts: affine
weights/UVs, integer corners, original quad diagonal, AVSZ4 scaling, both OT
bias directions, far and zero-area rejection, mirrored NCLIP, and budget
fallback. It excludes incomplete/duplicate-corner boxes, nonplanar/lower faces,
translucency, foreign callers and Vanilla. These checks and the trace-on/off
observation oracle pass. Python: 102 tests, 100 pass and two conditional skips.

Final offscreen validation also includes:

- 41 approach/retreat captures per rate at 60/120/180 and 4x scale, reaching the table and
  returning to the distant failure range. The lower cup remains complete.
- Real/redraw present sequences at 120/180 (2x diagnostic readback scale)
  and separate bar-prop controls. Sequence capture waits for all 24 files
  before issuing another request; the older helper's single-dump completion
  status does not establish that a sequence is finished.
- The older accepted shelf and two closets plus the accepted blood and subway
  states, using their correct dated copies, with movement/look at 120/180.
  No recurrence of the accepted visual defects was observed in these routes.
- Third-person smoke and Vanilla exclusion: Vanilla reports zero host-depth
  triangles and zero replay workers.
- Replay GPU-surface restore checks across three private state loads: 312
  comparisons, zero mismatches. The first attempt hit the harness's 30-field
  wait timeout under costly readbacks; the final oracle checks successful
  state application directly. This diagnostic run is not a speed test.
- Player build, controls fixtures and all 28 original card/state hash/mtime
  checks pass. All diagnostic games were closed after verification.

Matched off/on timing samples use CPU 100%, 4x, tracing disabled, dummy audio
and eight-second mouse sweeps after warmup. They measure submitted distinct
images, not monitor delivery or listening acceptance:

| Target | Scene | Comparison off | Final on | Output underrun samples, off/on |
| --- | --- | --- | --- | --- |
| 120 | UI 11 table | 118.23/s | 118.99/s | 0 / 0 |
| 120 | Club opening | 119.72/s | 118.57/s | 0 / 0 |
| 180 | UI 11 table | 172.26/s | 171.50/s | 35692 / 39116 |
| 180 | Club opening | 171.49/s | 168.13/s | 19844 / 22850 |

No worker failures occurred. The 120-target result avoids the rejected broad
experiment's regression. These samples do not promise a locked 180 FPS or
zero added cost: both builds underrun at 180 in these runs, with a small
additional candidate deficit, particularly in the opening scene. The user
accepted D17L as complete; this does not establish locked 180 FPS performance.

Private evidence lives under `recomp/analysis/d17l-20261004` and existing
`analysis/d17-high-refresh/shots/d17l-*`. No game assets, states, captures,
reference source or generated code are distributed. Framework source and its
accepted patch are unchanged. The user subsequently accepted this repair:
"first off, i accept this as complete!" D17L is closed as Done, preserving
implementation `1e6fb0a`. The acceptance closeout adds no new gameplay evidence.

Final player executable SHA-256:
`98676b43d3e86d96c1787e13284a14aaacb9ff24dbb934b2792b003d6eade950`.

Player launch from this workspace:

```sh
python3 recomp/tools/local/run.py
```

Load F7 UI 11, press F10 if mouse capture is needed, and approach/retreat from
the cup on the table ahead. Check at 120 first, then the usual refresh setting.
No new activation setting is required.
