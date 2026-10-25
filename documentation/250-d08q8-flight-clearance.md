# D08Q8 - Stable jetpack floor, ceiling and height-limit clearance

2026-10-25. Accepted by the user; lock-in/commit/push closeout authorized.
Candidate and revision evidence below is retained as the implementation history.

## Requested behavior

The current UI slot 12 corridor should permit flight along rising floors and
descending ceilings, following the Duke3D stair-flight reference. The user
also requested removal of head-contact judder at maximum outdoor height.
The reference `research/screencaps/jpacking-stairs.mp4` was inspected locally.

## Implementation

`jetpack.inc` resolves flight at the authenticated `8004ac08` root-apply entry,
only for Duke, the native mode-10 caller `8004b570`, active Modernized input,
valid camera ownership, living actor and verified executable/level identity.
All three jetpack schemes use the clearance adapter. Classic retains native
thrust, gravity, hover behavior and landing. Vanilla bypasses it.

The original routine applies horizontal motion only with a nonzero vertical
root, then applies vertical collision separately. Earlier Modern flight used
alternating vertical nudges to avoid stationary-height horizontal stalls.
Ceiling contact could also reflect/reintegrate the vertical root, while the
host kept requesting ascent or an unreachable held altitude.

The new adapter uses the original `8007a98c` sweep for both the route and the
full destination envelope. A fixed flight head/foot envelope avoids animated
pose changes moving the boundary each update. Isolated probes restore actor,
CPU stack/scratch and the cached world-space head point read by `8003964c`.
The latter must move with trial positions; moving only the actor root leaves
stale head coordinates in the collision query.

Collision point and normal give the floor/ceiling target. The normal's
horizontal component includes the native collision radius when following a
slope. Clearance adjustments are limited to 64 units per update, with a
bounded 320-unit trial for vertical stair faces. Solid walls and gaps that
cannot accommodate the verified route remain blocked. Only a proven route is
committed; query failure leaves the native path in control.

Modern/EDuke32 hold the resolved height at contact and no longer need the
alternating vertical nudge or stale cached-floor correction. Fresh takeoff
retains the native rising-foot departure until the steady-flight envelope
fits; a zero-root launch at floor contact retains the tiny upward departure.
This prevents the larger flight envelope from immediately landing a new pack.
Vertical and stationary probes also retain the native result-1 contract: an
adjacent vertical wall permits travel parallel to it. Horizontal motion still
requires a clear wall sweep. Without this distinction, a wall-adjacent platform
launch was incorrectly pinned by the new adapter. Native item, fuel, firing,
water and interruption paths remain active.

One opt-in generated hook was added through `game.local.toml` and normal
regeneration. Generated C was not hand edited. The owned executable supplies
new identity guards for the head-point lookup and collision subqueries.
No framework source or dependency pin changed. Existing flight saves load.

## Verification and evidence

Private evidence is under `recomp/analysis/d08q8/`; offscreen OpenGL, dummy
audio, separate settings/cards and copied states. Player data is not a writable
test destination. The host check found little current CPU/I/O pressure, ample
available memory, and brief hourly Timeshift checks in recent history. That
alone does not prove conditions in every earlier user session. The reported
contact problem was reproduced as actual vertical position changes.

- Baseline current slot 12: the last 30 completed updates of held ceiling
  ascent ranged from Y -11861 to -11803, a 58-unit oscillation.
- `radius-modern-60` and `final-eduke32-120`: the corresponding settled
  interval is exactly Y -11819. Both directions traverse the sloped corridor,
  with continued flight through ascent, descent and released-height holds.
  These runs preceded the subsequent takeoff and wall-parallel corrections.
- A probe-heavy intermediate search was rejected: its extra guest work
  increased animation dt as high as 40. The plane-based candidate keeps dt 10
  in 687/690 Modern samples and 686/689 EDuke32 samples; the remainder are 15.
  This is update-interval evidence, not full-system or monitor FPS certification.
- Final `final-sky-eduke32-120`: fresh level-0 takeoff, outdoor ceiling at Y -15997,
  held ascent, travel at the ceiling, holster/fire-to-draw, firing, private
  flight save/load, steering/firing after load and J-off/landing. The last
  30 ascent updates have zero vertical variation; all 645 recorded dt values
  are 10. First-person blend stays 1. Native sound handles remain active in
  flight and clear on J-off; dummy audio is not a listening test.
- Final `release-classic-60`: third-person native takeoff reaches mode 10 and the same
  ceiling, both directions of held-thrust corridor travel remain in flight,
  and release/landing retains Classic behavior. Earlier Classic scripts that
  toggled an already-armed pack off are not flight evidence.
- Final `release-corridor-120`: 112 stationary ascent/contact updates resolve
  to exactly Y -11819; both ceiling and floor travel directions remain mode
  10. Animation dt is 10 in 684/687 samples and 15 in the remaining three.
- `platform-final12`: the earlier private alley/platform fixture passes all
  138 checked flight samples, including wall-adjacent takeoff, held descent
  onto the platform, departure to lower ground despite a stale -13312 floor
  cache, released-height hold, and J-off/landing. Flight mode, pack-on bits and
  native sound handle stay active until J-off.
- Native controls include an independent room/step collision oracle: stable
  repeated ceiling/floor contact, horizontal ceiling travel, rising floors,
  descending ceilings, blocked narrow gaps, scratch/head restoration,
  caller/Vanilla/modified-code rejection, wall-parallel ascent versus horizontal
  wall crossing, and the fresh departure contract.
  Final rebuilt controls suite passes against the executable and all supplied
  level fixtures. EDuke32 fuel/toggle, input and aim suites also pass
  (`controls-final.log`, `fuel-final.log`, `input-final.log`, `aim-final.log`).
  `git diff --check` passes. All private test game processes were stopped.

Debug `read_ram` can observe a temporary isolated query. Completed `ttk-jclear`
and `ttk-jpost` traces, not arbitrary sampled trial positions, are the position
oracle. Presented screenshots were inspected for the corridor and outdoor
ceiling. The outdoor capture retains the previously documented high-altitude
sky/background gap; this task does not change sky rendering.

Candidate executable SHA-256:
`33a8c56fffd9b228ae9fa6aa6833355a00bdd73868279469a2e12626b9070b39`.

## Remaining acceptance

User playtest is required for feel at slot 12, maximum outdoor height, edges,
wall-adjacent flight and tight passage transitions. Bounded native and retail
routes do not prove every campaign slope, object, moving platform or costume.
No additional player setting is required. Modernized capture is automatic.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Use the saved jetpack scheme: J to activate, Space to ascend, Ctrl to descend
in Modern/EDuke32; Classic keeps its original J-arm/Space-lift behavior.

## Follow-up smoothing pass - 2026-10-25

User reports the first candidate is still intermittently juddery. A focused
revision retains the proven separated native route and tries a fully swept
diagonal route when that fails. The full destination must still fit. The
first combined-only experiment caused extra pauses and was rejected.
Slope corrections are now capped at 48 and stair corrections at 32 rather
than 64 units per update, with proportional horizontal travel on steep contact corrections.
This trades some traversal speed for smaller height changes at tight edges.
The native oracle now includes continuous rising-floor progress assertions.

The latest player session loaded slot 12 at 19:45:52. Its log records 88.4
presents/s but no contact trace, so it cannot identify individual judders.
Host inspection found low load, 24 GB available memory and no current I/O
pressure; no backup/update events appeared in the 19:44-19:48 journal query.
These checks do not prove the cause of the reported judder.

A 32-unit slope cap was also rejected because it introduced periodic pauses.
Failed alternate sweeps now preserve the original collision plane, so a side
wall from the alternate query cannot hide the floor/ceiling correction.

Revised executable: `c56e7e59b368334c9ba6f74fca86ef6332f95e346ddb9c52bbccfab0bd18a084`.
Full native controls suite with level fixtures passes (`revision4-controls.log`).
The earlier Classic/outdoor/platform evidence above belongs to the first
candidate; this quick revision adds the private slot-12 rerun and native slope
coverage, not a fresh exhaustive three-scheme route sweep.

Final `revision4-corridor` passes both floor/ceiling travel directions in
flight. All 329 substantial-motion samples retain horizontal progress; 113
stationary ascent/contact samples resolve to Y -11819. Animation dt is 10 in
684/687 samples and 15 in three. Smaller correction sizes are implemented,
but these bounded measurements do not prove the user-reported intermittent
judder is eliminated. User feel acceptance remains pending.

## Ordinary-flight surface following - 2026-10-25

User clarified that holding Space while moving smooths contact, but ordinary
horizontal flight must work without it. A private no-thrust slot-12 route
reproduced alternating zero/48-unit vertical corrections in the prior build
(`no-thrust-before`: 88 zero, 99 at 48 among fast forward zero-root samples).
The previous correction cap change did not address this repeated-contact gap.

Modern/EDuke32 now remember the contacted plane and use its tangent for WASD
movement without vertical buttons. Each update confirms the same plane at the
proposed destination with a native probe, then validates the actual route and
full envelope. Missing/different geometry, explicit vertical input, changed
capture epoch or changed position prevents reuse. The held altitude follows
the resolved position. Classic's original thrust behavior remains unchanged.
Fractional vertical travel carries between updates to prevent rounding drift.
This is continuous contact following, not an automatic climb/descent input.

Native tests require continuous floor/ceiling progress with neither vertical
button, exact accumulated fractional height, and release at the end of the
surface. The full controls suite also covers the existing narrow-gap, caller,
Vanilla, head restore and takeoff contracts.

Initial retail evidence before the final fractional carry addition:
`no-thrust-after` (EDuke32/120 target) and `no-thrust-modern` (Modern/60 target,
third person) traverse both directions in flight without Space or Ctrl.
Sustained slope travel changes height around 39 units each update, replacing
the previous zero/48 pattern. `surface-sky` covers outdoor contact, firing,
private flight save/load and J-off. Final build evidence follows below.

The latest user log loaded slot 12 at 19:53:14 and recorded 82.5-89.8
presents/s; it lacks individual contact traces. Current host load/I/O pressure
was low with about 24 GB available memory. A firmware metadata refresh at
19:55:52 was after that launch; no evidence attributes contact judder to it.

`no-thrust-final` verifies the fractional carry build in the private slot-12
route. A final wall guard subsequently stops the tangent when horizontal
travel is blocked and no valid vertical clearance correction exists; the
native oracle specifically presents a wall only to horizontal sweeps while
retaining the slope under vertical probes. This prevents unrequested climbing
at walls. It does not alter successful open-slope movement.

Final candidate SHA-256:
`b259c3ed80956758203ddd2e7ef7d6c334e6cd4718d6aab971d152c2fb7c71cb`.
Full controls with supplied level fixtures, EDuke32 fuel/toggle, input and aim
pass (`follow-wall-*.log`). Player slot-12 state still matches the original
private test copy byte-for-byte. `git diff --check` passes; pins unchanged.

Final `surface-release` private slot-12 route completes both directions along
ceiling/floor with no vertical buttons during travel; all sampled states stay
in flight. Fast forward following height increments: {36: 1, 39: 93, 42: 6, 41: 2, 40: 29, 43: 2}.
Animation dt samples: {'10': 744, '15': 5}. No zero-height intervals occur
in those confirmed fast forward following samples. This is bounded route
evidence; remaining campaign surfaces and user feel still need playtest.

## Acceptance and closeout - 2026-10-25

User: "lock it in and close us out!!" D08Q8 is Done. This accepts the final
ordinary-flight surface following revision and authorizes the documentation,
implementation commit and push. The final candidate hash above is the accepted
executable. No post-acceptance gameplay edits, builds or launches were made.

Re-exporting the runtime patch produced no change. It applies cleanly to pin
`08ec704a974b1f3a16335b4afeb340b9eff19926` and reconstructs all 52 authored
framework files byte-for-byte. The UI pin is unchanged. Player data, generated
code, media, builds and private evidence remain local. Native suite and private
route results above remain bounded evidence, not a full campaign certification.
No next job started.
