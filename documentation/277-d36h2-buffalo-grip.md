# D36H2 - Buffalo Rifle left-hand grip stability

2026-10-26. Done, user accepted both grip corrections and explicitly authorized
lock-in and closeout: "lock it in, close it out!!". The earlier left-hand
acceptance was "this is now fixed, and looks awesome". Evidence labels and
ownership follow [note 257](257-d36-first-person-presentation-audit.md).
Earlier candidate/Needs playtest wording below is historical; coverage limits
remain. Implementation, tests, manual and documentation commit/push authorized.

## Diagnosis

U: left support hand clips through the rifle during left/right turning;
this is not a request to change the forearm or right trigger grip.

S/L: private copied audit slot 9, actual equipment 2 / record 6, original
costume model (semantic left hand part 12, joint 4). Primitive traces identify
left-hand mesh 0x8011d8d0, face 0x8011d990, and rifle mesh 0x8011f900,
face 0x8011fd10. These are this loaded scene's addresses, not new hook constants.
Their projected faces overlap at the upper glove/rifle contact. Viewmodels
use average-depth painter sorting in `near_clip_viewmodel_flush`; negative
host depth deliberately excludes room depth testing.

Across 59 traced baseline turning frames, the relevant triangle average-depth
difference ranges from -0.153 to +2.030 units and changes sign in three frames.
Native fixed-point camera/transform rounding therefore reverses the relative
submission order despite a fixed relative grip. This establishes the small
contact-patch ordering instability, not a new whole-arm animation defect.

## Candidate

`display_arm_matrix` seats the Buffalo support wrist six units farther along
the rifle's local Y axis before applying the existing 70-unit native-clock
reload pull. The connected chain follows its wrist through the existing endpoint
solve. Wrist orientation, rifle/right-hand matrix, lift/cock/return timing,
weapon attachments, events, aim, damage and ammo code are unchanged.
No new mesh, animation, preference, allocation, save field, hook, renderer policy
or framework edit. Other weapon records and non-display owners retain their paths.

The matched 60-setting trace has 59 frames, depth separation +5.784 to +8.003
units and no sign reversal. Reviewed turning crops retain a supporting contact
without the identified flashing patch. The 120-setting trace also retains positive
separation in all 237 sampled process/frame groups, including replay workers
(+5.784 to +8.003 units). The small seating correction stays in
weapon-local space through the existing reload pose and replay interpolation.

## Verification and limits

Player build and owned-data controls, aim and near suites pass. Controls include
the original LEVEL00 fixture, LEVEL01 and the selectable-overlay directory.
Existing suites cover connected endpoints, semantic costume mapping, Buffalo
reload timing/return, draw cleanup, replay restoration and Vanilla exclusions.
No synthetic test merely repeating the new offset constant was added.

Private offscreen OpenGL runs at 60 and 120 settings cover left/right turning,
standing/crouched firing and reload, pitch changes, settling, and save/load during
upper 27 followed by return to ready upper 20. The 60 follow-up verifies actual
pistol record 4, return to rifle record 6 and first/third/first views. The initial
120 route's premature pistol sample and unsupported view action tokens did not
establish those transitions; the corrected 60 route does. The 120 run reports
three replay workers with zero worker/plugin failures. Captures are bounded
visual evidence, not exhaustive motion or performance certification.

Local evidence: `recomp/analysis/d36h2/`, including baseline executable, before60,
before120, tracewide60, candidate-trace60, candidate-trace120, candidate120
and final60. All private game processes are stopped. Private
settings and writable copies of prior audit test cards are used throughout;
player settings, saves and cards are not writable test inputs. Dummy audio means
these runs are not audio verification. No fresh full-costume, mirror, swimming,
flight, gas/power-up, software-renderer or live Vanilla sweep. User acceptance
of the hand contact and motion is still required.

Candidate SHA256:
`5b050af830987fe2015f93eedbab26c73261a60b0165c6caf8a2ed4301978a93`.

## Playtest

Select the Buffalo Rifle with 9 in first person. Turn left and right while
watching the left glove at the rifle contact, then fire/reload and crouch.
The existing cocking motion should retain its accepted feel.

```sh
python3 recomp/tools/local/run.py
```

Saved preferences are used; no additional activation step is required.

## Right-wrist running follow-up - 2026-10-26

User accepts the left-hand correction and reports a similar small flicker at
the base of the right hand while running. This selects further D36H2 work,
not acceptance closeout or commit/push. The six-unit left-hand seating remains.

S/L: the visible patch belongs to the adjacent right forearm cuff, semantic
part 14 (mesh 0x8011e4a4 in this loaded scene), immediately at the fixed right
hand/stock. Native running upper loops 18/19 roll that forearm while the weapon
and trigger hand retain their seeded display grip. In the before trace, cuff
faces 0x8011e558 and 0x8011e548 overlap stock face 0x8011fca0 and reverse painter
order. Their measured depth differences span -4.760 to +6.551 and -1.630 to
+0.789 respectively. This is a different source from the accepted left glove.
Early backward-running samples reach a wall and idle; the later side/forward
samples verify actual lower 76/78 and upper 18/19, not just held Shift input.

The candidate extends the existing native-clock running-arm reference to record
6, applying its complete ready right-chain relationship only during the run
contribution. The previous/current native frame blend controls entry and exit.
The existing endpoint solve preserves bone length/contact. It does not move the
right hand, weapon, muzzle or left chain. Attack/reload, crouch, swimming and
flight retain their preceding paths. The existing RunArms snapshot already
carries this pose/weight for replay; no new state or timer is introduced.

The matched candidate trace has no order reversal for either identified pair.
The first pair remains -4.506 to -3.626 over 107 overlapping samples; the second
pair no longer overlaps in this route. Other tested cuff/hand/weapon pairs also
have no sampled sign reversal. Local evidence is `right-before120`,
`right-trace60`, `right-candidate-trace60` and `right-final120` under the same
ignored analysis directory. Source face addresses remain diagnostic evidence,
not hardcoded runtime selectors.

Build and controls/aim/near suites pass. New native tests require run-cuff
independence from changing native elbow rotation, unchanged left chain/right
grip, connected endpoints, native entry/exit, attack/reload exclusion, missing
reference fallback, replay capture/blending/restoration and untouched native
matrices. The existing Buffalo reload tests also pass.

The private 120 route verifies record 6 during forward/back/side running and
turning, running fire/reload, stop/start, crouch and pitch changes. A save made
in lower 76 / upper 18 loads successfully; the post-load sample is idle, so this
does not claim resumed running in that sample. A separate upper-27 save/load
continues reload and reaches upper 20. Pistol record 4, rifle return and actual
first/third/first views are verified. Three replay workers report zero failures,
and plugin failures are zero. Reviewed running wrist crops retain steady contact.
These are bounded captures, not exhaustive continuous-motion acceptance. The
prior full-costume, other-action, lighting and live Vanilla limitations remain.
All private processes are stopped. Player saves/settings/cards remain untouched.

Final follow-up candidate SHA256:
`360debe3975119f1f57a121f8173e5fd7ccbf58bf4c11df35d753e80703b987b`.

Needs playtest: run around with the Buffalo Rifle, turn/strafe/start/stop, then
fire and reload while moving. Check the small patch immediately below the right
glove and preserve the accepted left-hand result. Use the launch command above.
No framework edits, acceptance closeout, commit or push.

## Acceptance and closeout - 2026-10-26

D36H2 is Done by user acceptance of the left-hand turning correction and
right-wrist running follow-up. User explicitly authorized lock-in and closeout.
Accepted executable SHA256:
`360debe3975119f1f57a121f8173e5fd7ccbf58bf4c11df35d753e80703b987b`.

Implementation, focused regression tests, manual and documentation are included
in the authorized MusicMonsterMod commit/push. No source change, rebuild or game
launch during closeout. Existing framework edits are unchanged; no framework
patch export is needed for this title-only change. Original media, player saves,
settings and cards remain untouched. No next job started. The verification
limits above remain explicit rather than becoming claims of broader testing.
