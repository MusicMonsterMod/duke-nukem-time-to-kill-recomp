# D08L1 - Walking and crouched walk-offs

Status: Done, user-accepted 2026-10-23. Explicit closeout authorized.

Accepted executable:
`8de27e4cae38b5053c5b456c3fc60ef96a2d2a46df4630dbb7309a8c01a63097`.

## Behavior and implementation

Modern standing walks and Modern crouch walks can leave supported ground without
jumping. The original fall simulation owns gravity, swept collision and landing.
Pushable-object travel is unchanged. Vanilla and CROUCH ORIGINAL retain their
existing paths.

Two independent restrictions were present:

- `terrain.inc` only borrowed the original running fall dispatcher for walking
  drops of 257-768 units. Larger standing walk-offs retained the original stop.
- `crouch_probe` explicitly rejected every drop over 256 units, including its
  radius-based look-ahead. This was deliberate in D08K2/K7, not Free Push logic.

The standing adapter now allows any positive drop over 256 when the original
probe returns its clear-drop result 6. It retains the existing walking-scale
root displacement, original dispatcher and collision tests. The fall lease
accepts a preceding walking gait as well as a running gait; walking horizontal
velocity remains capped at 2048. Post-jump falls retain their separate behavior.

The gameplay crouch mover opts into drops while its diagnostic mover keeps the
old conservative probe. Destination headroom is checked at departure height,
not at the lower floor; the head ray remains horizontal during a drop. Object,
wall, upward step and slope checks remain. The actual destination must lose
support before falling starts; a look-ahead drop alone does not start a fall.
The mover releases its custom animation and calls guarded resident routine
`0x8003f060(player, 1)`, the original running fall start (108, mode 9), retaining
walking-scale X/Z velocity and no upward impulse. It never snaps feet down to
the floor below. Existing resident guards cover the fall entry, initializer and
flight dispatcher; no generated-code or framework edits were needed.

`stance_request` can also adopt a grounded native kneel (178) when Modern crouch
clips are available. A native low-clearance transition or loaded crouch need not
have started through a host crouch request. Normal stand-up still requires the
existing clearance query; Original crouch, disabled input, unsupported modes
and missing clips do not acquire this ownership.

## Verification

Local evidence and private save copies:
`recomp/analysis/d08l1-20261023/` (ignored). Real key/mouse input, Xvfb, OpenGL,
private profiles and dummy audio. No timed run overlapped a build or native test.

- Player build and native controls/input/aim suites pass. Controls includes all
  21 level guards and the existing jump, mantle, crouch, push and Vanilla cases.
- New native cases cover a clear crouched fall, unchanged departure foot height,
  no upward impulse, departure-height headroom, wall-ray rejection, tall steps,
  missing floor, native forced-kneel acquisition and walking large-fall velocity.
- `baseline-edges.json` uses the accepted D08T3 executable; `edges.json` uses the
  candidate in third person at 60 FPS. `first120-edges.json` repeats the candidate
  routes in first person at 120 FPS.

| Route | Accepted baseline | Candidate |
| --- | --- | --- |
| Apartment bed, standing walk | Falls 383 units | Still falls and lands |
| Apartment bed, held crouch walk | Stops on top, no fall | 108/mode 9, lands 383 units below |
| Exterior platform, standing walk west | Stops near X 7481 | Falls and lands 2048 units below |
| Exterior platform, held crouch walk west | Stops near X 7486 | Falls and lands 2048 units below |

The candidate's sampled platform crouch fall has horizontal speed 1955; the
standing fall settles to its 2048 cap after the native initializer. No sampled
fall has negative Y velocity (an upward impulse). Both camera preferences work
on these routes. These are sampled traces, not an exhaustive collision proof.

Free Push regressions: slot 9's block stops at X 8700, remains there while W
stays held, and S reverses it to X 9120. Slot 12's dumpster still stops at X 8418
before the ladder. Push code is untouched; native push regressions also pass.
All 15 source save/sidecar/thumbnail hashes from the D08T3 manifest remain intact.
A private Vanilla route with 180 frames of original Up input ends at identical
X/Z, animation, mode and contact in the accepted and candidate binaries
(`vanilla-baseline.json`, `vanilla-candidate.json`). This is a bounded Vanilla
regression check, not full-campaign certification.

## Pre-acceptance diagnostic limits

**Automation did not verify the exact slot-12 softlock route.** Repeated real-input
approaches from the pushing end and side stopped below the dumpster; jumping
with E reached hanging animation 149 rather than a completed mantle. No change
was made to mantle or push behavior to force this test through. The user confirmed they mantled onto it; the scripted approach still has not
reproduced that completed mantle. Work continued on independent routes.

Separate diagnostic placements on the moved dumpster's top tested departures
with and without held Ctrl (`top-probe`, `top-low`, `top-held`). They fall and
land, but the placements do not reconstruct the original support/collision
history and did not reproduce forced low clearance. They are **not evidence that
the primary push/mantle/forced-crouch escape is fixed**. Native tests establish
the forced-kneel ownership and clearance contracts; actual low-ceiling escape,
ceiling-safe appearance, the complete slot-12 route and user feel were left for
user playtest. The user subsequently accepted the candidate (below). Broader campaign edges and live jump/mantle routes were not swept.

Existing framework
changes and submodule pins are untouched. No media or player settings were
changed. Test saves were written only in the private analysis directory.

## Final user acceptance

User: "it's definitely working. i'm happy to accept this lock it in and close it out".

D08L1 is Done. This gameplay confirmation closes the remaining slot-12 acceptance
gap; it does not turn diagnostic placement into automated route evidence. No
additional gameplay changes or rebuild followed acceptance. The authored source,
native regressions, manual and evidence are included in the authorized
MusicMonsterMod commit/push closeout. The runtime patch was re-exported unchanged
and verified against the pinned clean framework: all 51 patched files match the
live source. No next job was started.

Launch the accepted build with saved player preferences:

```sh
python3 recomp/tools/local/run.py
```

Capture is automatic. No F10 activation step is required.
