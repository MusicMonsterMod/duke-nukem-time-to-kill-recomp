# D15 accepted geometry and texture precision

User-accepted 2026-10-04. The user explicitly authorized documentation,
implementation commit and push after the successful final playtest.

## Accepted presentation and reproduction cases

120 Hz is the primary quality target. The user accepted the noticeably steadier
geometry, restored seamless street floor (slot 4), and reduced post-movement idle
wobble (slot 1). The final playtest confirms:

- Slot 5: the left-side black/missing-area failure no longer reproduces.
- Slot 6: the closet and surrounding furniture render clearly and consistently;
  the wardrobe back no longer disappears.
- Previous slot 8: the door/solid geometry remains opaque in normal play. This
  case is accepted and closed. A recurrence elsewhere is a new regression.

D15 is **Accepted**. The newly replaced subway slot 8 is independent **D17P**,
and does not qualify or block this acceptance.

Accepted executable SHA256:
`35756df57b9a6ddd31ee0dabdffb51876faf7a3b0c05effb78be285dfd9dc9a7`.

## Implementation

Modernized/OpenGL has independent Original/Corrected geometry and texture
precision options, saved in profile schema 23. Defaults remain Original.
Vanilla and software rendering retain Original. Launcher settings use G; CLI
flags are `--geometry-precision original|corrected` and
`--texture-precision original|corrected`. Changes apply on next launch.

Resident mesh and subdivision code carries projection provenance into packet
RAM. The runtime accepts exact-address, current-generation, matching-word
metadata only when every corner of the original polygon is valid. Geometry
uses fractional positions; texture correction uses perspective depth. Replay
workers record the metadata with host vertices for the parent renderer.
Original OT order, game simulation and collision remain intact. Unsupported
or unproven primitives fall back to the original path. Build definitions enable
instrumentation without hand-editing generated C.

The floor regression came from clearing the precision timeline at an empty OT:
older packet buffers could still be queued for drawing. Per-write invalidation
and actual state/RAM timeline changes remain; mesh entry resets registers only.
This retains valid precision for queued geometry without accepting stale words.

The accepted idle polish stops the very slow within-deadband camera recentering
when corrected geometry is enabled and the player root has stopped moving in
XZ. Existing movement bob, larger corrections and posture response remain.

The final close-up regression was frame-budget bookkeeping, not a new surface
culling rule. An empty OT was an unreliable indication of a new mesh frame;
non-mesh primitives could already occupy it. A stale/zero cursor exhausted the
host clipping budget and forced native PS1 near rejection until a view changed.
Accounting now starts at authenticated scene composition `0x80026164`, caller
`0x800268f4`, including replay workers. Out-of-arena cursors fail closed. Budget
caps and the existing clipping/depth rules remain. Slot 5/6 before/after captures
confirmed restored surfaces. The exact old slot 8 transient was not independently
isolated by the agent; the user's final playtest establishes its acceptance.

R01 informed the separation of original presentation, precision and clipping
and the importance of provenance. No Disruptor implementation was transplanted.
See [R01](93-disruptor-reference-research.md).

## Verification and limits

- Player build, native controls, PGXP and near-clip tests passed. Final near tests
  exercise populated OT initialization, wrong caller, stale cursor, exhausted
  budget and frame-state restoration; Original/Corrected packet digest matches.
- Python: 104 tests, 102 passed and two conditional skips.
- Final replay restoration: 45 checks, zero mismatches. Slot 6 alpha-zero redraw
  matches the real image pixel-for-pixel.
- Slot 5/6 movement and turns at 60/120, repeated loads through slots 1/4/5/6/8,
  six street views, idle position stability, third-person and Vanilla checked.
- All 28 player save/card file hashes and mtimes stayed unchanged during tests.
- At 120 Hz/4x, eight-second turning samples produced 119.77, 111.39, 119.89 and
  119.26 distinct submitted images/s in slots 4/5/6/old 8 respectively, with
  zero audio underruns. This does not establish locked 120 unique images/s or
  campaign-wide fidelity. Existing higher-refresh busy-scene limits remain.

Local evidence (not distributed): `recomp/analysis/d15-20261004/`,
`recomp/analysis/d15-regression/`, and `recomp/analysis/d15-culling/`.
The latter contains `review.json`, before/after captures, tests and replay data.
The framework source is preserved in the exported runtime patch and verified
against the clean pinned dependency; generated code and retail data stay local.

## Immediate follow-up: D17P

The user replaced UI slot 8 with a subway corridor reproduction. Load the new
slot 8, walk forwards and watch the distant scene. Multiple horizontal black
lines/bands appear to follow distant geometry or textures along a horizontal
boundary. "Possible culling" is the user's visual description, not a diagnosis.
Investigate visibility/culling, clipping, polygon gaps, depth, geometry correction,
texture rendering, precision or other renderer behavior using controlled
comparisons and primitive evidence. Preserve this accepted baseline.

New slot 8 SHA256: `7bf643dcfff92379ce5b9f06789eedf0a1c8c9cd4a46c1cfcde8670d9dba5743`.
Previous accepted door slot 8 SHA256:
`c648d3391e73c8a6b28166b630a1ea96d60b62f39658ec559a918fc7e25839db`.
UI slot 8 corresponds to zero-based debug slot 7 / `slot07.pst`.
Use private copies, never the player's writable cards. No cause is established
by this intake. See the canonical [D17P job](../MODERNIZATION_JOBS.md#d17p---distant-horizontal-black-bands-in-the-new-subway-slot-8).
