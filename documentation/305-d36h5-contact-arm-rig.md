# D36H5 - First-person traversal contact-arm rig

2026-10-28. Traversal rig and interaction expansion accepted by the user;
commit/push closeout explicitly authorized.
This records the initial six-owner implementation; the subsequently selected
full interaction expansion is tracked in [note 306](306-d36h5-interaction-arm-coverage.md).
The accepted [D36 audit](257-d36-first-person-presentation-audit.md) and
[art guide](265-native-mesh-art-style.md) remain the reference. Evidence labels:
S source/native tests, L isolated runtime observations, U user report.

## Problem and selected approach

U: mantle, ladder and chain/pole views are particularly harsh because the
inside of Duke's shoulders becomes visible. The user explicitly prioritizes
professional first-person presentation and permits a substantial rework.

L: an archived ladder state, pose 186/mode 3, reproduces the exposed upper-arm
end while looking sideways/down. Matched `ladder-native-90-left.png` and
`final-ladder-90-left.png` are under `recomp/analysis/d36h5/`. S: semantic upper
arms 10/13, forearms 11/14 and hands 12/15 belong to the Interaction world-depth
path. The native shoulder remains in front of the eye in some look directions
while its torso attachment is hidden. This differs from the camera-relative
combat DisplayArm path; changing a weapon grip would not fix this case.

The candidate introduces a dedicated two-bone first-person contact rig, rather
than capping a visible shoulder or hiding useful arm segments. Native hand
positions and rotations remain the contact targets. Virtual shoulder anchors
follow the eye; an authored outward/downward/backward elbow pole controls the
bend. The analytic solve retains bone lengths inside comfortable reach, then
extends their axial lengths proportionally to maintain contact and a small
elbow bend. It does not widen the segments. Source bone rotations supply twist;
the existing retail surfaces, textures and costume mappings remain in use.
This is a new display pose over the existing geometry, not newly authored art.
Arm proportions during close mantle motion still need human review.

## Ownership and implementation

- `contact_arm_ik.h` contains the stateless geometric solve and degeneracy
  fallbacks. `first_person_contact_arm` in `first_person.inc` applies its result
  only at the authenticated primary-player joint draw.
- Unarmed Ladder, Pole, Wall, Overhead, Mantle and WaterExit use the contact rig.
  Only upper-arm/forearm draw matrices change. Both hand matrices remain native.
  Armed step-ups retain the accepted weapon presentation.
- Native model matrices, animation phase, movement, collisions, attachment
  objects, callbacks, weapon aim and attack behavior are not written by the rig.
  Interaction world depth, polygon clipping and lighting ownership remain.
  The rig does not change FOV/free look or restore body geometry.
- Third person, reflections, other actors and Vanilla retain their native rig.
  Combat weapons, held items, grounded unarmed swing, flight and Push/Use/Mirror
  are outside this new pose policy. Q and door-kick presentation are unchanged.
- Existing 32-byte arm scratch is reused. Replay reserves it before worker
  creation. A direct 60-Hz load into climbing can precede any weapon draw, so
  the non-replay path also acquires that same scratch when first needed.
  No new allocation category, replay field, save schema or player setting.
- `DNTTK_FP_CONTACT_ARMS=native` is a one-launch developer comparison. Normal
  Modernized first person uses the candidate. The override is not saved.

## Experiments retained locally

1. Initial weapon survey: the reported traversal case was not yet identified.
   Damage/death spoiled later samples, and a repeated-action steering mistake
   prevented some requested weapon changes. These are not full weapon coverage.
   The user's clarification redirected investigation to traversal ownership.
2. `ladder-ik1`: no effective correction on a direct climbing load at 60 because
   scratch had not yet been allocated. This is excluded as a working IK result.
3. `ladder-ik2`: corrected scratch lifecycle; a high/narrow shoulder anchor placed
   the elbow too close to the eye. Rejected framing.
4. `ladder-ik3`, `pole-ik3`, `mantle-ik3` and the first motion runs: wider/lower
   anchors, but native-elbow-guided bending could protrude into view during
   turning. Rejected in favor of the authored elbow pole.
5. `final-*`: final authored-pole candidate. `ladder-native-*` is the matched
   native-pose diagnostic comparison. No geometry visibility shortcut is used.

## Verification and limits

S: player build and controls/input/aim/near suites pass. Animation-save recovery
checks pass. Controls use the owned executable, LEVEL00 guard fixture, LEVEL01
and all level overlays. New actual-hook tests exercise both costume orders,
multiple contact distances, connected elbow/wrist endpoints, unchanged hands
and native matrices, off-eye shoulder roots, the native comparison, reflected
actor isolation and first/third selection across the six traversal owners.
Pure solve checks cover reachable, extended, folded and straight-source targets.
Existing weapon, gameplay, clipping, interaction, Q/door and Vanilla tests pass.
These synthetic tests do not certify retail contact geometry or artistic quality.

L: ignored evidence is `recomp/analysis/d36h5/`, with reproduction scripts
`climb.py` and `motion.py`, identity hashes, raw state samples, before/after
images and continuous recordings. All runs use private profiles and writable
copies of saved states/cards, Xvfb OpenGL and dummy audio. Player save/card and
profile hashes/mtimes are checked before/after each run. No sound-quality claim.
The historical ladder/pole/mantle fixtures are identified by source directories
`analysis/d36-audit/cards`, `analysis/d35f/cards` and `analysis/d35g/cards`.
They are not claims about the player's current slot contents: current slot 12
now loads a ground state, not this archived ladder reproduction.

Final runtime evidence:

- `final-ladder`, `final-ladder43-fixed`, `final-ladder219`: default/60/80/90,
  forward/down/up and both sideways looks, 16:9/4:3/21:9. Actual ladder pose 186,
  mode 3, no equipment. The matched sideways/down comparison removes the bare
  upper-arm end and retains a continuous visible arm toward its hand.
- `final-ladder60` and `final-ladder120`: continuous look sweep and rung motion;
  poses 186/187/188/189. The 60 run also reaches 190, ground and fall. The 120
  script's timed movement remains on the ladder, so it does not certify exit.
- `final-pole60` and `final-pole120`: attached pose 192, native exit 196, ground
  return; the 60 run also samples fall. These are one chain/pole-family route,
  not every chain and pole object/costume.
- `final-mantle60` and `final-mantle120`: hang 149, pull-up 140, ground return;
  look sweeps and close geometry are retained in the recordings. The restored
  hang is largely world-occluded; it is not evidence of visible-hand quality
  for that entire hang interval. Pull-up exposes the reposed arms for review.
- Motion runs also save and reload only a private slot, then attempt a drop.
  A request to drop is not proof of the same action in every final state.
- The final 120 ladder, pole and mantle samples each report three replay workers,
  zero failed captures/jobs and no worker deaths. Recording/debug reads add
  overhead; these are restoration checks, not performance benchmarks.
- All 41 player save/card/profile files retain their hashes and modification
  times. No framework, original media, generated C or player settings edits.
- Two failed aspect harness starts used invalid `--widescreen 4:3`; the launcher
  rejected them before gameplay. Corrected `off` means 4:3 and produced the
  retained final 4:3 evidence. Failed starts are not counted as coverage.

Sampling and capture are sequential, not atomic snapshots.
Recordings retain continuous motion; inspected frame sequences support the
bounded comparison but do not replace human motion acceptance. A requested
120 setting and zero failed replay jobs are not a 120-FPS performance result.

The full D36H5 board matrix remains larger than this traversal correction:
all weapon/upgrade/item transitions, every costume in live play, dark/gas/effect
variants, every mantle surface and ladder/pole/wall/overhead/water-exit route,
damage/cancellation and campaign-wide coverage are not certified here. The
existing weapon-owner tests passing does not turn the failed initial survey
into live weapon coverage. Human visual acceptance remains required.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Review mantle entry/pull-up/exit and ladder/chain/pole motion while looking
sideways and down. Try the normal view and console `fov 80`/`fov 90`; these
console changes are session-only. Pay attention to shoulder ends, elbow bending,
arm proportions, hand contact and return to the weapon. No commit or push.
