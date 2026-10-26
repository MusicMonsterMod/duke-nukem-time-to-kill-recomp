# D17AA - Slot-12 wall tear exposes the pushable block

Status: Done, 2026-10-26. User accepted the result with "perfect!!!! Lock it in
and close us out", authorizing implementation/documentation commit and push
for D17AA and the previously accepted D17Z.

## Reproduction and cause

Private copy of current slot 12: LEVEL09 room 35, player starts near
(-11598, 5631, 42092). State SHA-256:
`4dbf85f513c302be0cc8ff1bb1c77d9a848b1460c371cc1f1fbbdf2677e51d23`.
This is a newer slot-12 checkpoint than the D17Z corridor fixture.

Moving forward exposes the block side through the left stone wall around
player Z 41700-41400. Settled screenshots can conceal the fault as camera bob
changes the host/native takeover boundary. Block instance `0x801df024`, mesh
`0x8014316c`, type 924; wall mesh `0x801b700c`, face `0x801b72f4`.
The block is already in host rendering while the intervening wall remains
native. Per-piece ordering plus the host foreground prop bias puts its side
after the wall. Native AVSZ face ordering keeps it behind the wall.
Disabling object takeover or zeroing the global prop bias also removes the
symptom in diagnostic runs; neither broad workaround is shipped.

## Implementation

`near_clip.cpp` retains the original AVSZ3/4 ordering slot and native face bias
for safely projected, opaque, noncompact static scenery when corrected textures
bypass retail subdivision. All emitted pieces retain that source slot, with
native far rejection and existing host clipping/depth. No extra depth geometry
or broad world takeover is introduced. Compact props, actors, viewmodels,
translucent/unsafe/oversize faces and Original textures retain their paths.
Vanilla remains outside the Modernized hook. D17Z's native world-quad split
is preserved. No framework changes.

## Validation

- Player build succeeds; SHA-256
  `9342a18490c3b9304371f3f2d2d3cabce077f000a13465f326b1392ebfa9eb32`.
- New native triangle/quad ordering regression covers source biases -1/0/+1.
  It fails with the new rule disabled and passes in the candidate. Existing
  compact, actor, viewmodel and clipping tests remain passing.
- Native suite passes all four corrected/original geometry/texture combinations.
  Both Original-texture combinations produce identical before/after output.
- Private 60/120 first-person advance, retreat and turn routes show an opaque
  wall while the legitimately exposed block front remains visible. The 120 Hz
  route reports 1232 presents, 1227 replays, zero replay misses/failures/drops.
  These diagnostic captures are not a full performance benchmark.
- Sampled table/cup, platform, wardrobe, apartment and club scenes retain their
  expected occlusion. Combined regression run reports zero replay misses,
  failures or dropped writes.
- Third-person 120 Hz advance/retreat/turn smoke also retains the wall and
  visible block, with zero replay misses/failures/dropped writes.
- Sky native test, repository policy check and whitespace check pass.

Local evidence: `recomp/analysis/d17aa/`, including `walk/` (baseline),
`fixed/`, `comparison.jpg`, `route60/`, `route120/`, `regress/`, native test
outputs, trace captures and `player-manifest.json`. All gameplay diagnostics
use private cards, saves and profiles; player files are unchanged.

## User acceptance and coverage limits

User accepted the slot-12 fix and authorized closeout. A scripted interaction
attempt did not reach a valid grab (contact and push object stayed zero), so it
is not evidence of successful pushing or pulling. The change only affects
render ordering. No full campaign claim is made.

Launch from the workspace: `python3 recomp/tools/local/run.py`.
