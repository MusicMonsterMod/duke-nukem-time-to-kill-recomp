# 491 - D23M3B: the output-sized redraw target is not exact (desktop check)

Autonomous overnight session of 2026-11-16 (laptop lane; the exactness check
ran on the desktop with the lane's build). User: "D23M3B: build note 460
section 4's output-sized target behind a developer switch, off by default;
exact with PSX_RT_VALIDATE in UI 2, 8, 9; measure GPU a redraw and whole GPU
on the laptop at 120/180 (note 486: ~0.5 ms of 5 ms to win)."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. What was built (CODE)

Note 460 section 4's design, behind developer `PSX_RT_OUTPUT_TARGET=1`
(render threads only):

- a session qualifies when it loads lazily, presents wide with the fast
  path and margins-only mirror, and its frame's wide surface is the current
  mirror target;
- the band is the draw area's top at the first primitive; fills and wide
  clears before it are queued and applied then;
- canonical draws keep the canonical projection (`u_xoff` 0, `u_xhalf` 512,
  a 1024 x 512 viewport at the scale) with the viewport origin moved by
  whole pixels, `(wide_off - base_x) * S` and `-band * S`; the mirror's
  margin draws keep the wide projection with the origin moved by
  `-band * S`; both scissored to their part of the band, into an FBO of the
  cache image with an output-sized depth-stencil;
- the screen-door hash (`gl_FragCoord`) reads `floor(gl_FragCoord.xy +
  u_fcoff)`, the surface's own pixel;
- output tiles not written whole are loaded from the snapshot (canonical
  centre, wide margins, depth-stencil), with D23V3's stale-tile check;
- packs, VRAM copies, CPU readbacks, uploads into the band, stencil
  rebuilds and a band other than the busiest draw area redo the session on
  the VRAM path (from the same import);
- the band copy, the centre copy, the margin mirror's copy-on-write and the
  VRAM-wide depth clear go away for a qualifying session.

## 2. Result (MEASURED)

Desktop (GTX 1080 Ti), the lane's build, 180, case `cad`, 40 s each,
`PSX_RT_VALIDATE=1`:

| View | sessions in the image | redone on the VRAM path | validator exact / bad (cumulative) |
| --- | --- | --- | --- |
| UI 2 | all (13 in the last period) | 0 | 10 / 299 |
| UI 8 | all (261) | 0 | 51 / 562 |
| UI 9 | all (150) | 0 | 173 / 734 |

So every session ran in the output target and about four in five images
differ from the VRAM path's. The differences are small: 3-140 pixels an
image, up to 59 levels, spread over the centre and the margins. At every
differing pixel inspected in the dumps (`PSX_RT_VALIDATE_DUMP`) the render
thread's colour equals one horizontal neighbour and the emulation thread's
the other: an edge (a triangle's or a texel's) one pixel over. They occur in
the margins too, whose draws are shifted only vertically.

## 3. Cause (INFERRED)

Note 460 assumed that moving the viewport origin by whole pixels lands the
same floats on the same samples. The vertex positions are the same, but the
viewport transform to window coordinates is evaluated by the GPU in
floating point at the new origin, and window coordinates of another
magnitude round differently before the rasterizer snaps them to its
sub-pixel grid; a vertex that moves by a fraction of a sub-pixel moves an
edge (and the interpolated texture coordinate that picks a texel) by one
pixel where it passes close to a pixel centre. Any output-sized target
needs the centre drawn at another origin than the canonical surface, so
this design cannot be exact on this GPU; the only exact ways keep the
centre drawn at the canonical origin (a canonical-sized target, which keeps
the copy) or a GPU feature for integer viewport offsets (not in GL 3.3).

## 4. Decision

Not exact: reverted (not committed); the code is kept in the local lab
(`recomp/analysis/d23n2b/d23m3b-output-target.diff`). D23M3B is Blocked
with this measurement; the laptop's GPU measurement was not run (a
non-exact path is not a candidate). What D23M3 measured as removable (0.5
ms of 5 ms a redraw on the field laptop, note 486) stays.
