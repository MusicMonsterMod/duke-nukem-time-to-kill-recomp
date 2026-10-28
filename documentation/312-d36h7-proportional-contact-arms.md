# D36H7 - Proportional contact arms with shoulder reach

2026-10-28. Selected by the user after discussing occasional stretched arms.
Done, user-accepted; closeout authorized.

User accepted the result and authorized closeout on 2026-10-28: "great work. lock it in and close us out". D36J5 and D36H7 are Done. Recorded verification and campaign limits remain unchanged; D19X/D19Y remains pending separately.
References: accepted D36 audit 257, contact rig 305 and repair/coverage guide 306.
S = source/native tests, L = private runtime evidence, U = user report.

## Cause and bounded change

U: arms look good overall but occasionally stretched. S: the existing contact
rig keeps its virtual shoulder fixed relative to the eye, while the hand stays
at its native world contact. Any reach beyond 98% of the native two-bone length
extends both segments axially. This preserves contact but can exaggerate length.
The reported appearance has not been localized to a specific user play session.

The candidate accommodates excess reach by moving the virtual shoulder toward
the hand before solving the elbow. It moves only as far as needed for natural
lengths, at most 96 units, and remains below the camera-space plane
`y - 0.8*z >= 96` (Y down). The plane uses the widest supported vertical view
and reserves space around the shoulder origin. This avoids bringing an exposed
shoulder end into the picture merely to shorten an arm. Visual checks remain
necessary; a bound on the origin is not an exhaustive mesh visibility proof.

Reachable poses retain the accepted shoulder. No pose/FOV thresholds select a
different rig, no timing state is introduced and both limbs keep the same
relative proportions. The existing solver handles residual unreachable reach:
this is a reduction in stretching, not a hard maximum on final bone scale.
Hands remain exactly at native contact points, with connected elbow endpoints.

Implementation: `contact_arm::reachable_shoulder` in `contact_arm_ik.h`, called
from `first_person_contact_arm` before the accepted solver. The same nine
contact owners use it: Push, Use, Mirror, Wall, Ladder, Pole, Mantle, Overhead
and WaterExit. Eligibility/equipment and action-camera gates are unchanged.
Weapons/held-item grips, unarmed locomotion, swimming/flight arm owners, legs,
Q/door kicks, reflections, third person and Vanilla retain their prior owners.
No mesh/material changes, gameplay matrices, callbacks, animation, camera/FOV,
settings schema, framework changes or new guest/replay allocations.

For one-launch diagnostic comparison, `DNTTK_FP_CONTACT_ARMS=fixed` retains the
accepted fixed-shoulder contact rig; `native` still selects original arm poses.
Normal launch uses the new shoulder accommodation. Neither override is saved.

## Quantitative evidence (S, recorded-pose calculation)

The following values are calculations from preserved native matrices and
camera poses in the preceding private action coverage. They compare the old
and new solve for the same pose, not average gameplay or user acceptance.
Local data: `recomp/analysis/d36h7/recorded-pose-reach.json`.

| Sample | Original segment scale | Candidate segment scale |
| --- | --- | --- |
| Toilet, left/right | 1.567 / 1.574 | 1.191 / 1.198 |
| Ladder, left/right | 1.398 / 1.417 | 1.025 / 1.074 |
| Mantle, left/right | 1.302 / 1.321 | 1.000 / 1.000 |
| Wheel, left/right | 1.663 / 2.121 | 1.330 / 1.747 |
| Card, left/right | 1.251 / 1.928 | 1.000 / 1.553 |

The distant wheel/card cases show the explicit limit: the bounded, concealed
shoulder cannot always reach a target with unscaled bones. Forcing a hard cap
would require further pose work or sacrificing contact, neither claimed here.

## Verification

S: player build and controls/input/aim/near suites pass. Animation-save metadata,
mod-memory recovery and old/warm Bio Mask reservation tests pass. Pure reach
checks cover natural, extended and high targets, bounded travel, the lower
screen constraint, exact hands and non-increasing bone scale. Actual-hook tests
across both costume mappings and all nine contact owners retain joined elbow/
wrist endpoints, native hand matrices, native skeleton bytes and reflection/
view/Vanilla isolation. They now require reduced shoulder-to-hand distance and
bounded concealed shoulder movement instead of a fixed Z=-64 origin. Existing
D36J5 legs/toggle and projection regressions also pass.

L: separate fixed-shoulder and candidate ladder runs at 60 capture default/90
FOV and forward/down/up/sideways looks. `matched-ladder-inputs.json` records
which samples have exact camera/native-matrix equality. The default-down and
90-level/up/left/right pairs match both; other pairs are not pixel-exact A/B
claims. The inspected downward pair retains the hand location with a less
elongated visible arm and no exposed shoulder root in that view.

L: candidate 120 ladder motion covers look sweeps, rung movement and private
save/load; current toilet/vending routes at 120 observe 275/277 and completion,
with maximum-downward and sideways samples. The inspected toilet capture keeps
both hands together while reducing the extended-arm appearance. All tests use
private settings and writable save/card copies with Xvfb and dummy audio.
Player save/card/profile hashes and mtimes are checked; no player data is used
as test output. Local scripts, captures, recordings and logs are under
`recomp/analysis/d36h7/`.

L: the subsequent archived 60-setting wheel, crystal insertion and card routes
observe 270, 267 and 266 respectively, including equipment/contact handoffs and
completion. Inspected wheel/card frames retain object contact and show no
exposed shoulder end in those views; they are not an exhaustive endpoint audit.
Ladder motion records 186-189. The 120 ladder and current-interaction runs report
911 and 1121 completed replay-worker jobs respectively, zero failed worker jobs
and zero failed replay sessions. All completed runs preserve all 41 player files.
Repository/privacy checks pass. No failing private saves were overwritten.

## Limits and review

This is a shared contact-rig change, not a new combat-arm or projection design.
No exhaustive campaign/costume/lighting/contact review is claimed. The 120
setting does not establish sustained 120-fps performance or sound quality.
Extreme contacts still extend proportionally. User visual acceptance was received at closeout on 2026-10-28.
The older isolated overhead restoration observation in note 306 remains
unexplained; this change does not claim to fix it.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.
