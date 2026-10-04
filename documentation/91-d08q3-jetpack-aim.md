# D08Q3 - jetpack view aiming and crosshair

2026-10-04. **Done, user-accepted:** "i accept this! great work."
Accepted implementation: `f284dc1`. Recorded test coverage limits remain;
no additional gameplay test was performed during acceptance closeout.

## Cause and change

The original armed jetpack state is mode 10, animations 163-170. It retains
an authenticated camera/input lease through `jetpack_input_ready()`, but
`movement_ready()` intentionally rejects it. The shot adapter and energy-beam
completion required that ground lease. The reticle allowed ground movement or
locomotion, which also explicitly excludes flight. Weapon presentation and
view-aim input policy had the same omission.

A private replay of the preserved D08Q2 slot-10 intake reproduces the issue:
flight animation 164, mode 10, armed equipment 2; the original pistol projectile
launches, but adapted shots remain zero and the reticle reports false.
This establishes a reproduction from that private copy without assuming that
it captures every location or weapon in the user's report.

`weapon_aim.cpp` now accepts either the existing ground lease or the existing
jetpack lease for shot adaptation and energy-beam completion. Reticle eligibility
uses the same addition. `view_aim_input_ready()` also admits jetpack flight, so
weapon presentation follows view pitch and the existing view-aim input policy.
No new hook, address, guard exemption, projectile constructor, damage rule,
flight physics, inventory repair, renderer or framework change.

The flight predicate still checks Modernized mode, capture, independent camera,
live camera lease, player/camera ownership, alive/state flags, equipped pack,
flight animation and authenticated owned EXE/LEVEL00 code. Ground locomotion
and airborne steering predicates remain false in flight. Vanilla and original
aiming keep their existing paths; disabling the crosshair does not disable aiming.

## Verification

Private evidence: `recomp/analysis/d08q3-jetpack-aim/`, debug port 9343,
Xvfb display 95, private profiles and a copy of the earlier private intake cards.
No tests launch on the player's cards or preferences. Functional Xvfb results
are not performance or physical audio measurements.

- Native aiming: both gun caller contracts, vertical aim, crosshair preference,
  original-aim refusal, Vanilla, changed-code and absent-flight rejection;
  energy-beam completion with no ground lease. Existing parallax, physical
  muzzle cover, actor overlap and original fallback checks pass.
- Native controls: all eight flight animations admit view aiming without ground
  movement or air steering; capture, Vanilla, death, camera-owner, changed-code
  and pack-off rejection; ground handoff. The complete suite, including the
  accepted D08Q2 recovery and prior traversal/camera cases, passes.
- Input and inventory HUD suites pass. Incremental local-dev player build,
  initialized geometry math shard and native movie shard checks pass.

Live checks in both Modern and Classic flight:

| Check | Result |
| --- | --- |
| Pistol / shotgun / RPG / energy / flame / freezer | Adapted shot counts increased by 3 / 1 / 2 / 7 / 4 / 3 in Modern, and 3 / 1 / 2 / 8 / 4 / 3 in Classic; all sampled firing endpoints remain mode 10 with a visible reticle |
| Yaw and pitch | Mouse changes the view; original launch traces carry the adapted direction toward the traced view target, including vertical components |
| W/A movement, Space ascent, Ctrl input while firing | Adapted shots continue in mode 10 in both schemes; Classic retains its original burst/gravity behavior rather than host descent |
| Idle flight and I toggle | Reticle remains eligible; toggling I hides/restores it without affecting the aiming policy |
| Presented crosshair | Full Xvfb window capture inspected in armed Classic flight; enabled crosshair visible at screen center. Canonical guest screenshots omit this host overlay |
| J off and landing | Pack closes and ground eligibility returns |
| First-person transition | Ground eye blend approximately 1, flight fallback 0, landing returns to 1; ground shots continue adapting |
| Fuel exhaustion fixture | Private fuel set to 80 with fuel cheat disabled; original handler ends flight at 5 remaining in Modern and 0 in Classic, and lands with eye view restored |
| Player data | All 28 player file SHA-256 and mtime values match intake |

The pistol and shotgun exercise the normal firearm path; TTK still constructs
original bullet projectiles. No new instant-hit damage path was introduced.
Live traces confirm adapter and original launch behavior, not every weapon's
full damage/impact chain. All six weapons were selected before takeoff.

Build SHA-256:
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.

## Scope and player check

Original weapon selection restrictions remain: select a weapon on the ground;
the flight shortcut path still permits J/item-use for the pack only. Holstered
or unsupported equipment does not acquire a reticle. This change adapts shots
that reach the authenticated original weapon dispatch; it does not manufacture
shots or enable attacks the original animation/state machine rejects. Thrown
weapons and upgraded variants retain the existing dispatcher coverage, but have
not all received a separate live flight trajectory/impact replay in this pass.

Check your reported location with both Modern and Classic flight. Aim up/down,
turn, hover, move, climb/descend and fire at enemies or surfaces; test J off,
landing, fuel depletion and I crosshair toggling. Confirm that the crosshair
and shots feel correct. Existing first-person play intentionally falls back to
third person in flight and returns on landing. F10 captures Modernized input
if needed. Full campaign/overlay coverage remains separate work.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
