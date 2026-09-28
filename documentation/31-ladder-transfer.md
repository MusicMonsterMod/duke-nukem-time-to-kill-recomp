# D08: running takeoff and ladder transfer

**User accepted — D08 Done (2026-09-26).** After playtesting this delivery, the
user confirmed every ladder jump worked as expected, E reliably caught the next
ladder, and the requested fluidity was achieved. EDuke32-style weapon/item shortcuts
are queued as D08A; broader unverified traversal coverage is retained as D08B.

The user accepted Escape/Start but still found the first alley's platform-to-ladder
jump unreliable and authorized changing the mechanic itself. This pass reproduces
the actual transfer, rather than treating ordinary street jumps as acceptance.

## Reproduction

The previous delivery (`090695be...`) climbed the first ladder and reached the
platform at approximately (9173, -11764, 13706). A holstered run/jump toward the
second ladder, **with E continuously held**, entered running animation 104 and
then reach animation **109**. At that transition the adapter changed its pad from
49135 (including unarmed Cross) to 64511 (no Cross or forward). The flight lease
only recognized 103/104/105, so both input and camera ownership dropped out.
Duke passed the target and fell to the alley floor, ending in recovery 105 at
(4900, -9703, 13706). This is a reproduced adapter defect, not a judgment about
the user's timing or fatigue.

## Changes

The existing owned running-flight lease now includes 109. The original dispatch
table routes it through the same `80055904` airborne handler. That handler calls
`800557F8`, which switches to the reach pose, then `80055208` for original surface
acquisition. Its original collision/contact routines decide whether to attach.
109 gets input/camera continuity only: the once-only takeoff velocity correction
still accepts only 103/104, so reaching cannot inject extra velocity. Flight epoch,
player identity, state, death and resident/overlay byte guards remain required.

An E request at takeoff or while airborne retains one reach intent until the owned
flight ends. Releasing E before contact no longer drops it. Grounded interaction
pulses remain short. Capture/focus/menu loss, deliberate C, original ownership
loss and the end of flight clear it. Armed Cross remains forbidden; E uses the
existing automatic holster and redraw sequence. No C press is needed for the
verified transfer. The actual climbing animation and W ascent remain original.

The original running-jump helper `80078C0C` forecasts support **1024 units ahead**
and can defer a requested jump until an edge. For the authenticated call from
`80053500` only, the existing `800780B4` pre-query hook changes that forecast to
current support: zero forecast displacement and endpoint equal to current player
position. Original selection can therefore start the jump on request instead of
waiting for the ledge. This is a stack-local forecast change, not a player position
write or a bypass of the movement/airborne collision queries. It requires running
movement, the normal camera lease, the expected caller and saved return address,
plus a new full-function SHA guard for `80078C0C..80078D30`. Vanilla and original
precision aiming bypass it. It does not provide coyote-time after walking off an
edge, change gravity, lengthen jumps, teleport to ladders, or widen collision.

## Evidence

- Native input: airborne E persists after release, carries across takeoff, clears
  on landing/manual holster, and never emits armed Cross. Existing buffering,
  menu/capture, binding, repeat, temporary holster and focus tests still pass.
- Owned EXE/overlay native controls: 109 retains only an existing flight lease;
  capture epoch invalidates it; reaching never reapplies takeoff velocity. The
  takeoff forecast changes only its stack arguments, with the entire player
  object unchanged. Caller/code/running guards are tested. Existing aiming and
  captured-crash scene-lifetime suites also pass.
- Baseline private route: continuous E fails after transition to 109, as above.
- First candidate, comparable short run-up: catches the second ladder (148 ->
  210, state 3), climbs through 189, exits to normal movement on the upper platform.
  Its manually holstered weapon correctly remains holstered.
- Delivery route: starts the transfer with the pistol drawn, prepares E while
  stationary, uses a later run-up, jumps, then releases E **31 guest frames before
  attachment**. It catches at (4590, -11770, 13769), climbs onto the upper platform,
  and redraws automatically at (2368, -13821, 13706), normal animation 63/state 0/0.
  Ammo remains 200 and the view-aim shot counter remains zero. No manual C input.
  The first sampled flight is six guest frames after the recorded jump event;
  debug/SDL scheduling means this is not an exact input-latency measurement.

The final delivery also passes all six directional running-jump regressions:
five clear landings continue running, one original obstacle impact retains
recovery. Escape pause/resume still passes. The 14-checkpoint Vanilla replay
exits 0; fire/jump/inventory/turn screenshots reviewed. Python: 56 run, 54 passed,
two skips.

All integration runs use private Xvfb, isolated cards/profiles and dummy audio.
A logged player-health longevity fixture permits enemy-exposed research; no
position, velocity, animation, enemy-health or collision fixtures are used.
The input timeline records actual guest frames: nominal event intervals accumulate
SDL/debug overhead and must not be treated as exact game-frame run-up durations.
Screenshots of the attached second ladder and armed upper-platform exit were
reviewed. See [evidence](reports/d08-ladder-transfer.json) for binary identities,
regressions and private artifact locations.

At delivery, D08 was Needs playtest. The subsequent user acceptance above closes
this controls iteration; D08B retains oblique/diagonal approaches, other ladder/ledge
types, swimming, scripts and campaign progression. This pass verifies
the reported transfer and a concrete handoff defect; it does not claim a complete
replacement movement engine, full-campaign fidelity or new audio acceptance.
