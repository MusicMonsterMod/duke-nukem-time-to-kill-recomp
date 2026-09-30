# D08L - Inertial platform edge run-off

Status: **Done** (2026-09-30). User: "im happy with that!" The measured brake
is removed and the feel was accepted.

## Failing comparison (before)

Isolated instance, private copies of the apartment state (`apt-cards`, slot 1),
Shift-run with real keys, per-frame samples of position, animation, mode and
velocity (`player+0x1f4/0x1f8/0x1fc`). Speeds are horizontal world units per
guest frame, averaged over 6 frames.

| Case | On the ground | In the air | Notes |
| --- | --- | --- | --- |
| Apartment bed, run off (drop 383) | ~47 | ~47.5 (8876) | Already continuous (D08F short-fall lease) |
| Apartment bed, walk off | ~10 | ~9.5 (1827) | Already continuous |
| Fire-escape platform outside the apartment window, run off west (drop >= 2048, 108 reports 2048) | ~47 | **16.5 (3058)** | Braked to a third of run speed for the whole ~60-frame fall; ~900 units of air travel; back to ~47 after landing |
| Same platform, running jump | ~47 | 54.5 (10085) + upward impulse | Original running jump |

The original fall initializer gives every edge departure a horizontal velocity
of 3058. The D08F short-fall lease replaced that with the measured stride speed
only for drops of 256-768 units, so furniture drops felt continuous while
exterior platforms braked. The same limit also dropped the mouse-camera lease
during large falls: the camera counter stopped and yaw froze until landing.

A second defect was found on the existing small-drop path: the stride estimate
can spike to its 14000 clamp at a run start. On the bed, 1 of 8 timed run-offs
left at 14000 (75 units per frame, above the running jump), on the pre-change
binary and the new one alike (identical sequence
`[14000, 8875, 8875, 8875, 9751, 8875, 8875, 8875]`).

## Change (Modernized only)

`recomp/src/ttk/terrain.inc`, `recomp/src/ttk/modern_controls.cpp`:

- The short-fall lease is also granted for a drop over 768 when Duke is running
  and the fall directly follows a ground stride (previous player-update
  animation 72..79). Only the original run dispatcher falls from there; the
  original walk still stops at the edge. A fall after a jump (103 etc.) or an
  existing fall is not affected.
- The lease is unchanged otherwise: once-only horizontal velocity along the
  latched movement direction at the measured stride speed before original
  gravity and swept collision; later impacts keep their own velocity; no air
  steering; camera-only mouse lease through the fall; the original landing
  selector receives continued movement.
- Every edge departure is capped at 10000 when running (below the running
  jump's 10085, and with no upward impulse) and 2048 when walking (the documented
  limit that previously applied only when no stride was measured).
- The six-input-frame edge-jump grace stays limited to drops of 768 or less.
- New debug counter `run_offs` in `ttk_input.controls`; the
  `DNTTK_TRAVERSAL_TRACE` trace prints `ttk-runoff` for unleased 108 frames.

Vanilla returns before these hooks. No generated C, disc or media edits.

## After

Binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

| Check | Result |
| --- | --- |
| Platform run-off (3 runs) | 9172 through the whole fall (~45-49/frame), 2700 units of air travel, lands into the running stride (78) at ~49 |
| Mouse look mid-fall, third and first person | Live: yaw -90 to -54 (third), -90.5 to -60.9 (first); travel direction unchanged |
| Running jump off the same edge | Unchanged (104, 10085) |
| Space pressed mid-fall | No jump, no upward velocity, edge jumps +0 |
| Walking toward the platform edge | Original stop at x 7475 (no fall) |
| Bed run-off, same 8 timings | `[10000, 8875, 8875, 8875, 9751, 8875, 8875, 8875]` (spike capped) |
| Bed walk-off | 1804 |
| Native `ttk-controls-test` | PASS; new cases for the running large-ledge lease (walking and post-jump falls rejected, spike capped) and the small-drop cap |
| Native aim / input / scene / near, Python (74, 2 skipped) | PASS |
| Vanilla route `d08l-vanilla-1` | exit 0 |

Evidence: `recomp/analysis/d08l-edge/` (`runs/*.json` traces,
`runs/final-platform.json`, logs, `before.bin`, `after-src/`). Private cards:
`apt-cards` slot 1 (apartment spawn), slot 2 (on the bed), slot 3 (fire-escape
platform). Probes: `probes/start.sh`, `stop.sh`, `tr.py` (port 9191, display :91).

## Limits

- Accepted by the user: the run-off now carries Duke about three times
  further horizontally off large ledges than before (still less than a running
  jump from the same edge).
- Only the first alley's fire-escape platform and the apartment bed were
  measured; other exterior ledges, slopes and very high falls (hard landing
  105, fall damage) were not exercised.
- Wall contact during the fall uses the original swept collision; the checked
  route landed and ran into the alley wall without clipping, but no ledge that
  ends next to a wall was specifically swept.
