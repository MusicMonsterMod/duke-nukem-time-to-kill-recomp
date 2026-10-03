# D17D - Near-coplanar furniture and depth reconstruction

2026-10-03. Background investigation of the lower wooden shelf near the club
exit. User authorized offscreen testing and investigation of a general cause;
they also explicitly accepted that genuinely coplanar level geometry may have
no unambiguous global fix. D17A/B remain accepted.

## Reproduction and cause

Private `analysis/d17-high-refresh/cards-user8`, UI 9 / file 08, SHA-256
`46b8f4b0c3d9f9f79bf1720f07df6e116fcf3e9b28d7522a2f496bccc6312201`.
Tests use NVIDIA EGL through SDL offscreen, private preferences/cards and port
9231, with dummy audio. No user desktop input, player settings or saves are used.

The brown triangles are the furniture's lower shelf; the green triangles are
room floor rendered over it. This reproduces at 60 as well as 120/180 FPS.
Disabling host depth changes the fragments but retains painter-order artifacts.
It is not a texture streaming failure or an interpolation-only defect.

A diagnostic trace identifies the prop's lower horizontal surface as triangles
with local vertex indices 11/2/10 and 1/10/2, textured page 0x68, CLUT 0x579.
The owned RAM dump resolves the mesh to `0x801296d0`, vertices to
`0x801296f8`, and these two FT3 records to `0x8012983c` / `0x8012984c`.
These are evidence identities, not runtime selection conditions. The
corresponding local positions are:

| Vertex | X | Y | Z |
| --- | --- | --- | --- |
| 1 | 300 | 300 | -170 |
| 2 | 300 | 300 | 170 |
| 10 | -300 | 300 | -170 |
| 11 | -300 | 300 | 170 |

The overlapping world floor uses page 0x19, CLUT 0x14b9. In one traced camera,
the transformed planes are only 0.572 game units apart. The game transforms
world and prop meshes separately using integer translation and fixed-point
rotation. A tiny change of camera can change their apparent relative depth.

There are two contributors:

1. The host path preserves PS1 integer screen corners but previously attached
   each corner's unsnapped view-space Z. Snapping X/Y while retaining that Z
   tilts the rasterized depth surface. Independently triangulated surfaces can
   then intersect even when their source planes are parallel. Subdivision makes
   the intersection appear as triangular fragments.
2. The original surfaces are effectively coincident at the precision of the
   separate transforms. Correct plane depth alone removes the breakup but can
   still switch between an entire visible shelf and an entire covering floor
   during a slow turn. This is the placement ambiguity the user described.

## Bounded implementation

Near-clipped planar polygons now carry their source view-space plane through
clipping and subdivision. For each final raster corner, depth is evaluated by
intersecting that screen ray with the plane. Reciprocal depth is affine in
screen space, so all subdivisions of the plane agree. Visible snapped corners,
texture UVs and texture-perspective weights remain as before.

A static prop's depth plane receives a two-game-unit tolerance toward the eye,
along its normal, to resolve the independently quantized contact. This follows
the existing foreground priority of props in the ordering table, but is much
smaller than the existing four-slot (128-unit) ordering-table bias. Actors and
first-person weapons receive no contact tolerance. The weapon still draws over
the world as before. No mesh coordinates, room-specific tests, culling rules,
interpolation timing, mouse input, simulation or audio are changed.

The runtime carries depth-test Z separately from texture-perspective Z. The
replay stream transports both, and the old host-vertex API retains its previous
contract. This is temporary host rendering metadata, not a savestate format
change. Ordinary PS1 primitives remain on their original path.

Non-planar quads keep their previous depth geometry. Degenerate planes,
near-horizon rays, invalid depth and extreme extrapolation fall back to original
depth rather than creating an unbounded occluder. The correction is limited to
the existing guarded Modernized near-polygon path; Vanilla remains available.

Developer comparisons (not saved preferences): `DNTTK_PLANE_DEPTH=0` restores
the accepted prior depth behavior; `DNTTK_PROP_CONTACT=0` isolates plane
correction without the contact tolerance. The latter is deliberately insufficient
to stabilize this almost-coplanar shelf through all sampled turns.

## Verification

- The lower shelf remains intact in the 15 stationary/slow-yaw/slow-pitch
  screenshots at 60/120/180, and the inspected 24-present sequences at 120/180.
  The same route with `DNTTK_PLANE_DEPTH=0` reproduces the fragments. Plane-only
  comparisons establish why the contact tolerance is needed.
- Paired offscreen 4x regressions cover UI 4 (opening), 5 (club), 3 (subway),
  6/11 (closet edges) and 9 (shelf), at 120 and 180. Inspected captures show
  no new see-through walls; these are bounded routes, not campaign acceptance.
- All six 120 FPS candidate samples have zero output underruns; distinct
  submission rates range from 118.7 to 120.2/s. No workers fail.
- A longer settled 180 FPS comparison, with three seconds after screenshot
  readback and eight seconds of turning per scene, records candidate rates
  of 179.85 opening, 177.84 club and 179.95 subway, with zero underruns.
  The corresponding old-depth samples are 180.15, 163.13 and 180.16; the club
  has 19,068 missing audio sample frames. Do not interpret this variable
  workload as a performance improvement caused by depth correction. Earlier
  short samples had occasional underruns in both variants. These are dummy
  sink timing results, not actual-device listening or input-to-photon tests.
- The expensive GPU-surface restore oracle at 2x passes 190 checks with zero
  mismatches across shelf/closet loads. Initial runs using the normal harness's
  30-field deadline timed out under this full-readback diagnostic (also on the
  old-depth path). The completed correctness run waits bounded wall time after
  each private load instead; its speed is not performance evidence.
- Third-person private-state smoke passes. Vanilla's host-depth and replay
  worker counters remain zero. Native near-depth math, aiming, input and
  controls suites pass. SDL offscreen cannot support the input suite's relative
  mouse assertion; the successful input run uses private Xvfb.
- Python suite: 100 tests, 98 pass and two skip. Runtime independent-depth
  replay-codec, pipeline deadline, worker deadline and dirty-text restore tests
  pass. New math assertions check source-plane depth order, invariance under
  subdivision, shifted projection centres and degenerate/horizon fallbacks.
- Fresh-card headless Vanilla boot/intro/title/gameplay route completes and
  exits cleanly. Intro, spawn, fire and turn captures were reviewed; no new
  full-campaign or audible-FMV acceptance is inferred from this bounded run.
- Incremental player build and native movie/math shard checks pass. The complete
  exported framework patch applies to a clean pinned dependency, with all 39
  changed/new files byte-identical. All twelve original player savestate hashes
  and mtimes match the pre-existing manifest.

Final candidate SHA-256:
`7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
Codegen remains `8bab543c`. No media, generated C or player data edited.

Local captures and JSON records use the `d17d-` prefix in
`analysis/d17-high-refresh`; screenshots are not distributed retail assets.
`d17d_visual.py`, `d17d_contact.py`, `d17d_regress.py`, `d17d_settled.py` and
`d17d_extra.py` are local reproduction drivers using the documented private D17
harness. `shots/d17d-final-comparison.png` and `d17d-final-turns.png` collect
before/after images. The original accepted executable remains preserved in
`accepted-2026-10-03/`.

## Limits and player acceptance

This is a general correction for the demonstrated near-polygon depth problem,
plus a bounded tie-breaking policy for static-prop contacts. It is not proof
that every D17E/F/J artifact shares this cause, nor a universal solution to
coplanar props or actually intersecting geometry. Two overlapping props can
still be ambiguous. The tolerance can choose the prop in a contact band where
a mathematically exact depth test would choose the wall/floor; it must not be
expanded into a blanket draw-through policy.

The player should inspect UI 9's lower shelf while still and turning slowly,
then check ordinary opening/club/apartment play at 120 and 180 FPS. Final D17D
acceptance requires their confirmation. Other follow-up jobs are not started.
