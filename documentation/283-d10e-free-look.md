# D10E - Held third-person free look

2026-10-27. Done, user-accepted. Closeout, commit and push explicitly authorized.
Inspired by @vakho10's [issue #1 camera suggestion](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/issues/1),
with the bounded D10E inspection scope retained.

## Delivered behavior

Hold middle mouse to orbit the third-person camera while keeping movement,
Duke's facing and weapon aim aligned with the pre-inspection gameplay view.
Release eases the shortest angular distance back to that view. V requests the
same return and suppresses a new inspection until middle mouse is released.
Repeated presses during return continue the current inspection offset. Mouse
steering and keyboard turn/look keys resume immediately on release while the
residual visual offset eases away independently. The crosshair is hidden
while inspection/return is active: firing follows Duke's retained aim, not the
inspection screen center. Fire is neither suppressed nor delayed.

The action is optional and rebindable in KEY BINDINGS > CAMERA > FREE LOOK.
Action ID is appended; schema 54 migrates schema 53 profiles with a recovery
backup. A customized Mouse3 binding wins and leaves Free Look unbound. Modernized
is still the default profile; Vanilla and first-person input remain unchanged.

## Source findings and implementation

- S: `orbit_begin`, `view_yaw`, `face_view`, upper-body aim and the weapon adapter
  previously consumed the same camera direction. Merely skipping body facing
  would still turn movement and shots with the inspection camera.
- S: gameplay yaw/pitch now remain separate from transient inspection offsets.
  Only camera matrices, the native obstruction solver and late presentation
  receive the offset. Preferred boom and shoulder handling retain their owners.
- S: `view_yaw` supplies the retained heading during inspection; gun rays use
  the retained camera-eye offset translated with Duke and the retained forward
  vector. Existing muzzle obstruction, spread, damage and projectile code stay
  authoritative. Upper-body direction follows the same gameplay ray.
- S: inspection is bounded by authenticated ground locomotion, global third
  person, active captured input and ordinary player/camera ownership. Swimming,
  flight, attached traversal, scripted cameras and first person do not acquire it.
  Unsupported states, input epochs, world/load changes and camera refusal clear it.
- S: draw-time inspection state joins the existing replay snapshot. Late mouse
  samples and return decay use the same presentation path; no framework changes,
  guest allocation, generated-code edits or savestate-format changes.
- S: `ttk_input.controls.free_look` exposes active/held and angular offsets.
  The local input-file diagnostic accepts `mouse3` (SDL button 2).

## Verification

- Player build passes. Existing diagnostic snprintf warnings remain unrelated.
- Native controls suite with owned EXE and clean LEVEL00 fixture passes, including
  separate inspection/gameplay yaw, fixed facing and shot vector, translated eye,
  gradual release, repress and first-person/input boundary checks. Existing
  camera/shoulder/collision, movement, traversal, first-person and Vanilla guards
  remain covered by that suite.
- Native SDL input suite passes, including middle-button press/release, host-only
  pad behavior and rebinding Free Look to G. Native weapon-aim suite passes.
- 120 Python input/profile tests pass, including schema 53 migration with a
  customized middle-button Fire binding and existing settings recovery cases.
- L: private offscreen OpenGL runs at 60 and 120 presentation settings use copies
  of a prior test savestate (file slot 0, cowboy costume) and private settings/cards.
  Both exercise held orbit, walk/back/strafe/run, pistol fire, release, repress,
  reload of a pre-inspection save, and V return while middle mouse stays held.
  Duke's sampled heading stays 2950 while the camera orbits. Live shot counters
  advance with no rejected shots and their direction stays near the retained
  heading. Crosshair disappears during orbit and returns after settling.
- L: at 120 the sampled orbit reaches about 172 degrees horizontally and 23
  degrees vertically. Captures show the side/front inspection with native body
  geometry and HUD. Screenshot inspection is not certification of motion feel.

Local scripts, JSON and captures: `recomp/analysis/d10e/`. An initial slot-0
attempt used a private folder containing only slots 3 and 5 and correctly failed
preflight. A later slot-5 route entered native animation 213; that route does not
count as firing/release coverage. The final route above uses the actual copied
slot-0 file, checks drawn equipment and positive shot counts, and asserts the
inspection remains active during ordinary locomotion.

## Limits and player review

This is a ground-inspection feature, not a replacement control scheme. It does
not add head tracking or right-click aiming. Unsupported action transitions clear
the orbit rather than carrying it into a differently owned camera. Extensive
campaign obstruction paths, every weapon, saving during an active inspection,
real desktop focus changes and human smoothness judgment are not certified by
these bounded runs. Existing solver and input tests complement, not replace,
those observations. No performance-regression claim is made from FPS settings.

Review in third person (P if necessary): hold middle mouse, orbit above/below and
around Duke, walk/run/strafe/fire, then release. Confirm the retained movement and
firing policy feels appropriate and the return is comfortable, including near
walls and with shoulder/distance preferences. D10E remains Needs playtest.

```sh
python3 recomp/tools/local/run.py
```

D08K12 crouch running was added separately to the backlog after D18's remaining
follow-ups. No crouch-run behavior was implemented.

## Release steering correction - 2026-10-27

User report: after using free look, mouse steering did not always return.
Confirmed in a private baseline run: after releasing middle mouse and continuously
moving the mouse, gameplay yaw stayed -1.757600 and Duke's heading stayed 2950.
The residual inspection offset settled around 0.051101 radians instead of zero,
so inspection remained active across all three repetitions.

Cause: the return phase still consumed mouse deltas into the inspection offset.
Continued mouse movement replenished the offset faster than it could reach the
completion threshold, withholding gameplay steering indefinitely. The earlier
release tests stopped the mouse and missed this case.

Correction: consume mouse deltas only while the inspection button is held. On
release or V cancellation, restore normal mouse and keyboard steering immediately;
only the visual offset decays. This preserves the smooth camera return without
allowing new input to extend it. First-person and Vanilla paths are unchanged.

Private fixed runs at 60 and 120 each repeat orbit/release/continuous steering
three times. Duke's heading changes by the first 0.15-second sample; by the later
sample inspection is inactive even though mouse motion never stopped. For the
first 60 FPS repetition, heading advances 2950 -> 3024 -> 3590 rather than staying
at 2950. The native regression checks first-update yaw/pitch response, body facing
and eventual completion under uninterrupted input. Local evidence:
`recomp/analysis/d10e/baseline-60.json`, `fixed-60.json`, `fixed-120.json` and
`release-steering.py`. The player build is rebuilt. D10E remains Needs playtest;
no acceptance, commit or push.

## Acceptance and closeout - 2026-10-27

User: "lock it in and close us out!!". D10E is Done, including the release
steering correction. The final native controls suite passes with the continuous
input regression; both fixed private 60/120 routes pass. User acceptance supplies
the final playtest decision while retaining the named coverage limits above.

Commit and push of implementation, tests, manual and documentation are authorized
as MusicMonsterMod. Framework changes predate this job and remain unchanged; the
accepted patch is exported and checked against the clean pinned dependency during
closeout. D08K12 stays Todo after D18 follow-ups; no next implementation starts.
