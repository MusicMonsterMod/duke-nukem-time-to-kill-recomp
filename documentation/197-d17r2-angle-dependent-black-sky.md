# D17R2 - Sky turns black at certain viewing angles

2026-10-22. Status: Todo. Backlog and initial source investigation only.
Do not implement until selected. No rendering code changed, build or game
launch performed for this investigation. The reported view has not yet been
independently reproduced; its actual cause remains unestablished.

## Report and reproduction

Load current UI save slot 8 (file slot 07). Immediately look left; if needed,
look slightly upwards while facing left. The sky unexpectedly becomes black,
apparently depending on the view direction. Expected: sky remains correctly
rendered through camera turns, with no black gaps or exposed empty background.

Slot numbers are reused. On later implementation, copy the current save and
settings to private storage and record its hash, level, camera, yaw/pitch,
aspect ratio, precision, draw distance and frame rate. Do not assume it is
the LEVEL01 slot 8 used by D17R1 without checking the current save.

## Previous work and initial technical findings

- D17O ([note 92](92-d17o-sky-intake.md)): `frame_replay.cpp::transform_load`
  leaves the three authenticated sky matrix calls camera-relative, using each
  redraw worker's current eye and rotation. `sky_render.h` identifies return
  addresses `0x80038c68`, `0x80038d7c`, `0x80038e18` and guards resident sky
  routine `0x800388e4` (0x578 bytes). A missed caller/identity gate could allow
  the world-object interpolation path; that is a hypothesis to trace, not a
  diagnosis of this report. Compare real frames and redraws during movement.
- D17R ([note 172](172-d17r-sky-edges.md)): `sky_widescreen.cpp` separately
  widens the sky colour fill, scales row 0 of the camera-facing backdrop and
  expands the object-renderer clip rectangle. The rectangle helper acts only
  on an exact -256/+256 pair; the margin also requires a valid sky identity
  and a single-player view. Capture the actual rectangle/matrix/caller at the
  failing angle. Partial backdrop coverage, a missed gate or view-dependent
  near/outcode rejection are plausible candidates, not established causes.
- D17R1 ([note 173](173-d17r1-sky-blocks.md)): near cloud quads went missing
  when the near-clip takeover exhausted its former shared render-ring budget.
  Current `near_clip.cpp` uses four separate 128 KB GPU DMA regions, but
  `mesh_budget_fallback` and packet-budget fallback paths still exist. Inspect
  `host_bytes_peak`, `mesh_fallbacks`, `packet_skips`, culled polygons and the
  failing frame's GP0 packets before ruling out dropped geometry. Preserve
  the aperture-aware redraw walker and vertex-address keys in the exported
  framework patch. Their existence is not proof the current angle is covered.
- The prior sky survey found city levels 0/5/9 have zero fill colour and
  original black overhead. That is context to compare with Vanilla, not an
  explanation for the current slot-8 symptom. Establish the level and missing
  layer first. No texture corruption or missing art has been demonstrated.
- D08AE changes only shallow-water flight eligibility. It does not alter sky,
  clipping or replay code. A regression from a particular sky improvement must
  be demonstrated with matched views/builds rather than inferred from timing.

## Investigation when selected

1. Reproduce the exact left/up angle from a private current slot-8 copy;
   capture both sides of the failure boundary, then sweep yaw and pitch.
2. Compare 4:3/16:9, first/third person, 60/saved high refresh, original/extended
   draw distance and Vanilla. Use `DNTTK_SKY_WIDE=0` and `DNTTK_SKY_CAMERA=0`
   only as controlled diagnostic comparisons, not proposed player workarounds.
3. Trace `DNTTK_SKY_TRACE=1`, sky selectors/fill colour, backdrop matrices,
   clip bounds, per-polygon projection/culling, near-clip budget and OT/GP0
   coverage. Determine whether pixels were never submitted, clipped, culled,
   overdrawn or lost only in a redraw. Inspect motion sequences as well as
   still images; D17R1's first playtest exposed a redraw-only failure.
4. Compare the previous accepted sky scenarios and matched older builds where
   available. Prove the failing path before changing geometry, culling or art.

## Acceptance

- Current slot 8 no longer turns black when looking left or slightly up.
- Sky remains stable throughout camera-angle sweeps and moving turns.
- D17O camera-relative sky, D17R widescreen coverage and D17R1 whole cloud
  polygons remain intact, including their previous private regression views.
- No new rendering, clipping/culling, packet-budget or performance regression;
  Vanilla and existing display/camera modes remain correct.
- Focused sky/near tests, actual presented motion at 60/high refresh and user
  playtest substantiate the fix. Broad claims require corresponding coverage.

Dependencies: D17O, D17R, D17R1 (Done). Priority: minor visual follow-up.
No implementation started and no separate camera or texture job created.
