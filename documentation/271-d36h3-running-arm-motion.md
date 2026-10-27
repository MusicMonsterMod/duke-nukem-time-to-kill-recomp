# D36H3 - Restrained two-handed running arms

2026-10-26. Done, user accepted and closeout authorized. User: "lock it in,
close it out!!!". Final revision 3 is accepted after the weapon-by-weapon review.
Evidence labels and ownership boundaries follow
[note 257](257-d36-first-person-presentation-audit.md). Earlier candidate and
pending-acceptance wording below is historical; the stated coverage limits remain.

## Revision 3 - Energy Weapon firing-style support grip

User reports only Energy Weapon 7 remains: its left hand is still inside the
housing and should sit roughly where it does during firing. The other revised
weapons require no further changes in this pass.

The existing native firing-reference path now also applies to normalized record
10. It uses animation 24, key index 2, and retains the previous 20-unit leftward
clearance. Thus idle/run/fire use the same support grip, instead of bringing the
hand back inside with the ready pose. Only the Energy left chain changes;
weapon/right-hand anchor, recoil/events and other weapon paths are unchanged.

Build and the full owned-data controls suite pass. The support invariance test
now includes Energy record 10. Private base-60 captures in
`recomp/analysis/d36h3/rev3/base60` verify record 10 and show the hand farther left
at idle/run, close to the prior firing reference. Captures also cover start/stop,
fire/return, crouch, pitch, switching, private save/load and flight.
The upgraded-120 route verifies record 10 with upgrade flag 9 and flight pose
164, plus the same transition checks. Sampled replay sequence frames retain the
outward support grip. Three workers, zero worker/plugin failures. No fresh
swimming or full-campaign sweep in this narrow revision. Both private runs are
stopped. The prior scope limits remain; user visual acceptance is pending.
No commit/push.

Revision 3 candidate SHA256: `87994034abe221147568f74de42614cbb7221e07323bc88e8260fc3125ff0143`.

## Revision 2 - user review and targeted support grips

The user accepts shotgun 3's first-pass look. Flamethrower 8's running motion
is also good, with a request to retain its left hand's firing position at rest.
Gatling 4 and Freezethrower 0 should use their walking left-hand grip while
running. Energy Weapon 7 needs its left hand moved slightly left out of the gun.
This is further implementation, not acceptance or closeout of the whole job.

**S/L:** native ready animation 29 has right-arm frame 1796 with shoulder and
elbow keys only; its wrist is neutral and omitted. The first-pass full-mask check
rejected that reference, so records 7/11 retained their full native running arm
motion. This supersedes the first-pass implication that all five families were
successfully corrected. The native test fixture formerly supplied all three keys
and did not represent this sparse retail frame. Revision 2 handles this specific
neutral wrist and now tests it; genuinely incomplete shoulder/elbow data still
falls back. Reconstructed ready wrist relationships agree with live walking
samples to native fixed-point rounding. The normal walking reference is not
replaced by a synthetic running grip.

Changes:

- Shotgun 3's accepted path is unchanged.
- Gatling 4 / Freezethrower 0 use the complete walking support reference during
  the run contribution, with the native entry/exit ramp. Their right arms retain
  the first-pass behavior; firing still uses the original animated support pose.
  The Gatling underside contact offset remains applied.
- Energy Weapon 7 translates the entire connected left chain 20 camera-space
  units left, including firing. The weapon and right hand do not move.
- Flamethrower 8 uses the left-chain relationship from native firing animation
  24, key index 2, at rest, while moving and while firing. This is a middle firing
  grip, not a new firing clock. It follows the existing display weapon and recoil,
  applies to normalized upgrade 27, and keeps native firing events intact.

Local evidence: `recomp/analysis/d36h3/rev2/`. `intake60` and `references`
record actual native walking/run/firing channels and matrices. Animation override
pointers reside in expanded guest memory; `references/extended.bin` supplies
those tables. A 2 MB base-RAM read alone does not contain the active frame data.
`candidate60` contains matched route captures for the four revised weapons.
The new tests compare walking/running support chains for 7/11/28 with the sparse
29 frame, and stable ready/run/fire support for 9/27. They preserve native bones,
weapon anchors, shot timing and existing ownership/replay tests. Player build
and controls/aim/near suites pass. Visual acceptance remains pending.

`upgraded120` confirms actual records 5/7/10/9/11, the four available campaign
upgrade flags, firing/return, crouch, pitch, weapon switching, private save/load
and flight pose 164 for all five. The Gatling and Freezethrower 24-present replay
sequences were reviewed in order and retain steady support grips. Three workers,
zero worker/plugin failures. These captures are bounded visual evidence, not a
performance or full-campaign certification.

`swim120` confirms actual first/third/first transitions and Gatling swimming,
firing and private save/load (127/128, upper 34), again with zero worker/plugin
failures. The attempted Flamethrower water selection retained record 4, so it is
not Flamethrower swimming evidence. Native water weapon restrictions remain;
no forced equipment write was used. All diagnostic runs are stopped.

Revision 2 candidate SHA256:
`fe66b625acd263957cd2e24cf27a535ec0c019cc7a85879118bf9f6b5ee64faf`.

The sections below retain first-pass implementation and evidence. Their scope
statements about unchanged firing support are superseded for the requested
Flamethrower grip and Energy Weapon offset above.

## Diagnosis and implementation

**U:** excessive right-hand motion while running with shotgun (3), Gatling (4),
Energy Weapon (7), Flamethrower (8) and Freezethrower (0).
**S:** the seeded right-hand/weapon anchor is steady, but `display_arm_matrix`
previously transported native arm rotations into that fixed grip. The connected
endpoint solve changes a bone's direction without eliminating its axial twist.
**L:** private LEVEL05 slot-8 samples reproduce rotating forearms during native
run-ready upper loops 18/19, with actual held records 5/7/10/9/11 verified.
Backward input reaches a wall and idle; those samples are not running evidence.

`running_arm_pose` reads the weapon's own ready clip (20 or 29), resolves both
arm chains by semantic part IDs, and expresses them relative to the right wrist.
During the run contribution, display arm rotations and the support wrist blend
90 percent toward that ready relationship, retaining 10 percent native motion.
The previous/current native frame blend controls entry and exit. There is no
host animation timer, new setting or saved pose cache.

Parent origins are reconstructed from the native child offsets before the
existing connected endpoint solve, preserving bone length and joint contact.
D36H1's Gatling support offset is applied afterwards, as before. The right-hand
weapon matrix, camera bob, muzzle/aim, native matrices, clips, recoil and fire
or reload events are not changed. Only the five selected families, including
normalized challenge records 27/28, qualify. Crouch, swimming, flight, attack
channels and other weapons keep their existing paths. Draw ownership continues
to exclude reflected/third-person actors and Vanilla.

RenderState captures the reference and weight for replay workers and interpolates
the weight between frames of the same weapon. It does not rely on a worker's
old simulation state. Missing/incomplete reference data yields the existing pose.
No new guest allocations, mesh/material assets, framework edits or generated
source changes.

## Verification

**S:** incremental player build passes. Controls, aim and near native suites
pass with the owned executable; controls also uses the original LEVEL00 fixture,
LEVEL01 and the complete selectable-overlay directory. New draw-boundary tests
check reduced twist, connected base/upgrade endpoints, unchanged right grip and
native skeleton, native-tick start/stop, firing exclusion and captured/interpolated
replay state. Existing tests retain costume mapping, ownership, load retirement,
Buffalo reload, muzzle attachment and Vanilla coverage.

**L:** local evidence is `recomp/analysis/d36h3/`, including the prior executable,
`before60`, `candidate60`, `upgraded120`, `swim120` and `final120`. Runs use private
settings and copied test states/cards, offscreen OpenGL, dummy audio and port 9493.
No player's writable state is used. Base 60-setting captures cover all five
records, idle/walk/run/strafe and stopping. The 120-setting pass confirms campaign
upgrade flag 9 for shotgun/Gatling/Energy/Flamethrower; Freezethrower remains
record 11, flag 1. It covers start/stop, diagonal input, firing/return, crouch,
pitch, pistol/back switching, private save/load and flight. The first Freezethrower
flight attempt stayed grounded and is excluded; the final route confirms record 11
in flight pose 164. Its attempted scripted Caps Lock token did not enable RUN MODE
(lower pose 72, walking), so those samples only establish walking and view-toggle
recovery; fresh RUN MODE input acceptance remains for user playtest.

The 24-present shotgun replay sequence was reviewed in order, with calmer arm
motion and no visible wrist separation in that sequence. All five ready/run
comparisons were reviewed. Swimming checks use the Gatling: surface/deep poses
127/128, firing upper 33, private save/load and actual first/third/first toggles.
The 120-setting upgrade and swimming runs each report three workers and zero
worker/plugin failures; the final flight/view route does too. All diagnostic runs
are stopped. These are bounded visual checks, not a performance benchmark.

Limits: live captures are not exhaustive or phase-aligned A/B movies. Direct
challenge records are covered by native tests, not a fresh challenge-level route.
No fresh full-costume, mirror, gas/power-up, software-renderer or Vanilla live sweep.
The user must judge motion and grip quality in play before acceptance. D36H2's
Buffalo flicker remains a separate unstarted job.

First-pass candidate SHA256:
`4772a51eb00eab1adf1d40246c4dc4d37f1f2c5114b18cf5dcf64a4e05a2394a`.

## Playtest

Use first person and keys 3, 4, 7, 8 and 0. Run, strafe and stop; check the wrists,
then fire and switch weapons while moving. Include upgrades and RUN MODE.

```sh
python3 recomp/tools/local/run.py
```

The launcher uses saved preferences. No additional activation or option is needed.


## Acceptance - 2026-10-26

User accepted the final result and explicitly authorized implementation/tests,
manual and documentation commit/push. D36H3 is Done. Accepted executable:
`87994034abe221147568f74de42614cbb7221e07323bc88e8260fc3125ff0143`.
No additional source change, build or game launch during closeout. Existing
framework modifications predate this job and are not changed by it. No new job
started. Earlier candidate entries preserve the review and validation history.
