# Next-session handoff

## 2026-10-29 - D38 playable piano accepted and closed

User authorized lock-in and closeout of revision 2: MusicMonsterMod's supplied
piano WAV plays on native E use, with smooth distance falloff anchored at the
piano. Native animation, effects/master volume and Vanilla are preserved.
Build, controls/audio suites and private 60/120-setting routes pass within
[note 315](315-d38-playable-piano.md)'s limits. All 40 player files were preserved.

README credits MusicMonsterMod for the piano performance and Bio Mask mesh;
CreatorsHome for the Bio Mask audio source; and MichelleGrobler, distributed
through freesound_community, for the night-vision source. Source/license notes
remain linked. No post-acceptance gameplay edits, rebuild or launch.
Implementation and documentation commit/push authorized as MusicMonsterMod.
No next job selected. Launch: `python3 recomp/tools/local/run.py`.



## 2026-10-29 - D08T10 slot-5 box repeat grab accepted and closed

User: "lock it in and close us out! great work". D08T10 is Done, with
implementation, documentation, commit and push closeout authorized. Pulling
the LEVEL01 type-271 box crossed Duke onto a floor that forbids fresh grabs.
Its verified Z lane now uses the existing guarded acquisition extension;
original alignment, support/collision, manual travel and Vanilla are preserved.

Build and controls/input/aim suites passed. Private third-person 60-FPS routes
verified repeated grabs, signed travel and affected-floor reload; their final
limit assertion was invalid because the box had not reached the end. The
first-person 120-FPS route passed repeated grabs, reload, held stop near
Z -20613, release/regrab/retreat and pause/resume. Full evidence and remaining
campaign/opposite-face live/sound limits are in the D08T10 job-board work log.
All 40 intake player files retain hashes/mtimes. No rebuild or launch after
acceptance. Framework export is unchanged and all 55 files match the clean
pinned dependency after patch application. No next job selected.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - HUD levels and camera sliders accepted and closed

User: "lock it in, close it out!" D19U/D19U1 are Done; D19X/D19Y were already
accepted. The shared implementation, regressions, manual and notes 309/314
are included in the authorized MusicMonsterMod commit/push closeout.
Existing build/native/Python and private runtime evidence stands. No gameplay
changes, rebuild or launch after acceptance. Intermediate reload stalls remain
recorded in note 314 with their exact cause unproven; final routes passed.
Repository/privacy and staged whitespace checks pass. Framework export is
unchanged and matches all 55 changed/new files after applying it to the clean
pinned dependency. No next job selected.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D19U/D19U1 HUD levels (Needs playtest)

D19X and D19Y are complete and user-accepted. Acceptance was requested;
commit/push closeout was not. Existing slider implementation is preserved.

D19U and then D19U1 were selected and implemented. Minus steps Full ->
Borderless -> Hidden; Equals/Plus reverses, clamped at the endpoints. Gameplay
settings, rebindable actions and schema-60 persistence agree. Borderless
retains icons/values and removes frame art, including in high-refresh replay.
Native suites, migration and private menu/load/restart checks pass. Player
files unchanged. See [note 314](314-d19u-hud-levels.md) for scope, rejected
attempts, intermediate reload stalls and final passing coverage. The stalls'
exact cause is not established; preserve their private states and logs.

User visual/gameplay acceptance remains pending. No framework edit, commit
or push. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D35I1 wall-top descent (Done, user-accepted)

User accepted the slot-7 wall-top entry and authorized closeout: "lock it in,
close it out". Reachable thin climbing-wall panels now offer E TO CLIMB DOWN.
E stows the weapon and enters the native wall catch; S descends, retaining
lateral contact and existing wall movement, camera and exit rules. Vanilla
and ladder/pole behavior are preserved.

Player build and controls/input/aim/near suites pass. Private 60/120-setting
routes cover descent/panel transfer, reload, release, out-of-reach refusal
and native top exit. Player files remain unchanged. Evidence and bounded
coverage: [note 313](313-d35i1-wall-top-descent.md).
No framework edits or next job. Unrelated D19X/D19Y work remains separate.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D36H7 proportional contact arms (Done, user-accepted)

Contact shoulders now accommodate up to 96 units of excess reach before bone
extension, constrained below the picture. Hands stay at native contacts;
reachable poses, gameplay, combat arms and legs retain their existing policies.
Build, native suites and animation-save checks pass. Private ladder comparisons,
interaction samples and explicit residual-stretch limits are in
[note 312](312-d36h7-proportional-contact-arms.md). User visual acceptance and closeout authorized 2026-10-28.
No framework edit. Launch: `python3 recomp/tools/local/run.py`.



## 2026-10-28 - D36J5 first-person legs (Done, user-accepted)

Revision 5 fixes the current slot-7 ladder-end hang's exposed hip ends. Ladder
poses add bounded backward leg translation only when their projected hip roots
need clearance. Matched downward captures, build/native checks and private
hang/exit/load evidence are in note 311. Other owners retain prior framing.


Revision 4 corrects crouch-running leg loss: low/forward crouch eyes now use
an animated pelvis-to-neck safety footprint. Standing limits, torso hiding,
leg framing and native gait remain unchanged. Build/native checks and private
crouch movement captures are recorded in note 311; user accepted.


Revision 3 extends legs to every supported action eye: climbing/traversal,
interactions, swimming and flight. The torso remains hidden, the Advanced
On/Off choice applies throughout, and the accepted camera guards remain.
Note 311 now contains the complete documented policy inventory and separates
native coverage, current runtime routes and remaining campaign limits.


Revision 2 moves ordinary legs 48 units backward in the draw pose to conceal
open hip ends when running at maximum downward look. Native skeleton, fixed
projection, kicks and saved toggle are preserved; see note 311.

Ordinary locomotion legs return without torso/pelvis, using fixed H=256 display
projection with native poses and world depth. Advanced > First-person Legs
is On by default; Confirm/Cancel/Defaults and schema-59 persistence are wired.
Build, native controls/input/aim/near suites and 109 profile tests pass. Private
menu and FOV/movement samples, failed harness attempts and limits are in
[note 311](311-d36j5-first-person-legs.md). Player files remain unchanged.
User accepted all revisions and authorized documentation/commit/push closeout
on 2026-10-28. No framework edit; D19X/D19Y pending changes preserved. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J4 accepted and closed

User: "lock it in, close it out". D36J4 is Done: the native door-kick right leg
and thigh attachment now share the fixed weapons/Q display projection.
Implementation, regressions, manual and [note 310](310-d36j4-door-kick-fov.md)
are included in the authorized MusicMonsterMod commit/push. Existing native
suite and private slot-10 FOV/replay evidence stands with its explicit limits.
No post-acceptance gameplay change, rebuild or launch. Pending D19X/D19Y
camera-slider work is preserved separately, outside this commit. Existing
framework patch checked against the clean pin; repository/privacy checks pass.
No next job selected.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D36J4 door-kick fixed FOV (Needs playtest)

The current slot-10 hinged-door right leg and thigh attachment now share the
weapons/Q fixed display projection. Native door timing/events and gameplay
matrices stay unchanged. Build and controls/input/aim/near tests pass, including
a baseline-failing emitted-packet FOV regression. Private door entry/opening/
recovery passes at FOV 60/default/80/90, at 60/16:9 and 120/21:9; 2193 replay
worker jobs complete without failures. All 41 player files are preserved.

[Note 310](310-d36j4-door-kick-fov.md) records implementation, failed attempts,
visual evidence and coverage limits. User visual acceptance remains pending.
D19X/D19Y changes are preserved independently. No framework edit, commit or push.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D19X/D19Y camera sliders (Needs playtest)

User selected both jobs together. Gameplay now offers First-person FOV directly
after View Bob (60-90 degrees, default 64) and a Camera Distance slider (Original
or 768-6144). Keyboard/controller and selected-row wheel adjustment, confirmed
live application, Cancel, Defaults, saved custom distances and restart
persistence are implemented. Schema 58 adds the FOV preference safely.

Build, native controls/input suites and profile/launcher tests pass. Private
60/120-setting checks cover real menu input, virtual gamepad input, endpoints,
view changes, Alt-wheel synchronization, Defaults and restart. All 41 player
files are unchanged. [Note 309](309-d19xy-camera-sliders.md) records evidence,
excluded attempts and limits. User review remains pending; neither job is Done.
No framework edits, commit or push. No next job selected.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J3 accepted and closed

User accepted revision 3: "thats it! lock it in and close it out". D36J3 is Done.
Q now shares the weapons' fixed display projection across world FOV, with the
entry/recovery easing retained. The rejected adaptive translation is removed.
Native door projection and gameplay remain unchanged.

[Note 308](308-d36j3-quick-kick-transitions.md) retains build/native/runtime
results, rejected experiments and coverage limits. Authored implementation,
tests, manual and documentation are included in the authorized MusicMonsterMod
commit/push. No post-acceptance gameplay change, rebuild or launch. The existing
framework patch is exported and checked against the clean pin; repository
privacy checks pass. No next job selected.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J3 revision 3: fixed Q presentation (Needs playtest)

User rejected adaptive FOV retraction and requested the weapons' stable view.
Q now uses their existing fixed H=256 projection; the revision-2 translation is
removed. Its pose/size/framing match across world FOV 60/75/80/90. Native door
parts retain world projection. Entry/recovery easing and gameplay stay intact.
Build and controls/input/aim/near suites pass, including exact emitted-packet
invariance and both costume maps. Matched poses, bounded 120 replay/movement,
view/load recovery and ultrawide checks are recorded in
[note 308](308-d36j3-quick-kick-transitions.md). User visual acceptance remains
pending. Player files preserved; no framework edit, commit or push.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J3 revision 2: wide-FOV kick framing (Needs playtest)

User identified the exposed hip at full Q extension at FOV 90. Matched captures
confirm it. The leg now retracts toward its default hip screen anchor as FOV
widens, using one shared translation with unchanged depth, proportions and hit
behavior. Default/narrow FOV and native door presentation remain unchanged.
Build and controls/input/aim/near suites pass, including the expanded FOV test.
[Note 308](308-d36j3-quick-kick-transitions.md) records current FOV/aspect/cowboy/
replay evidence, earlier lifecycle evidence and excluded synthetic-menu failures.
All player files preserved. User visual acceptance pending; no commit or push.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J3 quick-kick transition candidate (Needs playtest)

User selected D36J3. The no-body Q rest pose exposed a thigh patch at the screen
edge at wide FOV. A shared draw-only lowering now eases the complete left leg
into/out of view while retaining the accepted strike pose and weapon framing.
Native right-leg door presentation, hit timing/damage/sound and gameplay matrices
are unchanged. The new regression fails on the baseline and passes on the fix;
player build and controls/input/aim/near suites pass.

[Note 308](308-d36j3-quick-kick-transitions.md) records matched frames, motion,
FOV/aspect, 60/120 replay, cowboy, door, real-key pause/view and Vanilla exclusion
checks, bounded timing and remaining visual limits. Synthetic-input menu/door
attempts that failed are excluded; ordinary keyboard reruns pass. All 41 player
save/card/profile files retain hashes and mtimes; diagnostic processes exited.
User visual acceptance is pending. No framework edit, commit or push.

Launch from workspace root: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36H6 accepted and closed

User accepted the significant quality improvement, superb weapons, and brief
costume testing with handheld items and interactions, then explicitly requested
lock-in and closeout. D36H6 is Done for the delivered cold unarmed-load repair
and coverage pass. Authored implementation, native regression and documentation
are included in the authorized MusicMonsterMod commit/push.

Existing build/native/runtime results and the limits in [note 307](307-d36h6-arm-coverage.md)
stand. Brief costume testing is user evidence, not exhaustive all-costume,
effect or scripted-event coverage. The isolated overhead restoration observation
remains unexplained. Future reports use [note 306](306-d36h5-interaction-arm-coverage.md)
and preserve failing private saves. No post-acceptance gameplay edit, rebuild
or launch. D36J3 remains Todo; no next job selected.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36H6 cold unarmed load repair and coverage pass

D36H6 was selected and is Needs playtest for a bounded repair. Fresh 60-Hz
unarmed loads could expose upper-arm ends because the accepted lowering path
waited for scratch allocated by a weapon draw or replay. It now allocates the
same scratch on first use. The cold-hook regression fails against
the unfixed source; both costume maps pass with the repair; player build and controls/input/
aim/near/animation-save suites pass.

Private coverage includes all fourteen base weapon records, six upgrade flags,
direct challenge records 27/28/29, card/crystal equipment handoffs, holstered
ground/flight, and two swimming routes at 60/120 with view/save restoration.
Fresh-load before/after images and evidence limits are in [note 307](307-d36h6-arm-coverage.md).
All 41 player save/card/profile files retain hashes and mtimes. Rare callbacks,
all-costume/effect combinations and the isolated overhead halt remain open;
this repair is not claimed to resolve that halt. User visual acceptance is
pending. No framework edit, commit or push.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36H5 accepted and closed

User accepted traversal and expanded interaction arms, then explicitly requested
lock-in/closeout and a reusable guide for future arm reports. D36H5 is Done for
the accepted contact-arm scope. Implementation/tests and documentation are
included in the authorized MusicMonsterMod commit/push. No post-acceptance
gameplay edits, rebuild or launch. Repository closeout checks pass.

Start future arm reports at [note 306's repair guide](306-d36h5-interaction-arm-coverage.md#how-to-handle-a-future-arm-report),
then the design/experiments in [note 305](305-d36h5-contact-arm-rig.md).
Current slot identities must be rediscovered; preserve failing private saves.
Unrun broad coverage transfers to D36H6 (Todo), including the isolated overhead
restoration observation, which remains unexplained despite successful repeats.
D36J3 remains Todo. No next implementation job selected.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36H5 interaction coverage expansion (Needs playtest)

User accepted the traversal arm rig and explicitly selected full interaction
coverage, highlighting current slots 6/7. All nine authenticated contact owners
now share the accepted rig, adding Push, Use and Mirror. Native hands remain
world targets, with eye-relative shoulders and solved elbows. Equipment and
reflection ownership remain separate. See [note 306](306-d36h5-interaction-arm-coverage.md)
for the route ledger, evidence, isolated overhead save/load halt and unresolved
campaign variants. Native suites, twelve restoration cycles and a full repeat
pass; the isolated halt is not explained. The broader
D36H5 weapon/item/effect review remains open; no commit/push closeout authorized.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36H5 traversal contact-arm candidate (Needs playtest)

User prioritized exposed shoulder interiors during mantle/ladder/chain/pole work
and allowed a substantial first-person rework. The candidate uses a dedicated
two-bone contact rig: native hands remain fixed, with virtual shoulders and an
authored elbow bend. World depth, gameplay matrices and weapon owners remain.
Build and native suites pass; private 60/120 ladder/pole/mantle motion and ladder
FOV/aspect comparisons are recorded in [note 305](305-d36h5-contact-arm-rig.md).
Human review of proportions, contact and transitions remains, alongside the
broader board coverage gaps. All 41 player files preserve hashes/mtimes.
No framework edit, acceptance closeout, commit or push. D36J3 remains unstarted.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D36J2 accepted and closed

User accepted the default no-body integration and explicitly authorized lock-in
and closeout. D36J2 is Done. Ordinary Modernized first person hides the body;
arms/items and temporary Q/door legs remain. The exact `DNTTK_FP_BODY=visible`
override is diagnostic and one-launch only. Prior build/native/runtime results
stand with the limits in [note 304](304-d36j2-no-body-default.md).
D36H5 shoulder polish and D36J3 kick review remain Todo; no next job selected.
Authored implementation/tests/manual/docs are authorized for MusicMonsterMod
commit/push. No rebuild, game launch or player-file write during closeout.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D36J2 default integration (Needs playtest)

Ordinary Modernized first-person launches now use the accepted no-body policy.
Exact `DNTTK_FP_BODY=visible` retains the one-launch diagnostic comparison;
no saved preference. Arms/items/Q/door owners and D14E1 safety remain. Build and
controls/input/aim/near suites pass; private FOV/aspect, action and restoration
runs are recorded with actual states and visual limits in
[note 304](304-d36j2-no-body-default.md). Human acceptance remains. D36H5 shoulder
polish and D36J3 kick review are unstarted. No framework edit, commit/push or
acceptance closeout. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D36J1 accepted; three follow-ups queued

User playtested and accepted the no-body prototype/direction, then explicitly
authorized backlog splitting and commit/push. D36J1 is Done for the bounded
prototype and evidence in [note 303](303-d36j1-quick-kick-research.md).
D36J2 (default integration), D36H5 (arms/weapon polish, exposed shoulders first)
and D36J3 (kick motion/overlap review) are Todo in the canonical job board.
The user's main concern is shots exposing disembodied shoulders/upper-arm ends;
D36H5 must investigate the actual geometry/owner before choosing a remedy.
No follow-up job is started. Broader original research is transferred, not
represented as fully executed. Existing build/native/runtime evidence and
limits stand; no new build or launch needed for closeout. Commit/push authored
implementation, tests and docs as MusicMonsterMod; preserve framework edits.
Current no-body launch: `DNTTK_FP_BODY=hidden python3 recomp/tools/local/run.py`.
Normal baseline: `python3 recomp/tools/local/run.py`. D36J2 removes the flag need.


## 2026-10-28 - D36J1 no-body prototype (Needs playtest)

User selected an opt-in no-body experiment. It hides ordinary first-person body
while retaining arms/equipment and unchanged Q/door presentation. Matched
60/80/90 images support the reported visibility contribution. Build/native
suites and bounded private jump/Q, aspect, equipment, view/load and replay
checks pass; [note 303](303-d36j1-quick-kick-research.md) and the
[artifact](artifacts/quick-kick-realism.html) retain evidence and limits.
No permanent body-policy decision, research closeout, commit or push. All 41
player save/card/settings files preserved. Broader research remains open.
Prototype: `DNTTK_FP_BODY=hidden python3 recomp/tools/local/run.py`.
Accepted body baseline: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D36J1 investigation stood up

User requested quick-kick realism research at FOV 80/90, especially downward and
airborne views. D36J1 is In progress for research setup; experiments are pending.
[Note 303](303-d36j1-quick-kick-research.md) and the
[standalone artifact](artifacts/quick-kick-realism.html) define the evidence ledger,
initial source references and controlled experiment/acceptance matrix.
Camera-relative Q versus world-body attachment is a hypothesis, not a reproduced
cause. Preserve accepted left-leg Q, separate right-leg door kick and D14E/E1.
No gameplay change, build, launch, player-file write, commit or push. Next research
step: establish a private matched baseline before testing offsets/pose blending.


## 2026-10-28 - D14E1 accepted and closed

User accepted the corner body guard and explicitly requested lock-in and
closeout. D14E1 is Done. Implementation, regression tests, manual and documentation
commit/push are authorized as MusicMonsterMod. Prior build/native-suite and
bounded runtime evidence stand with the limits in
[note 302](302-d14e1-corner-body-guard.md). No further gameplay change, rebuild,
game launch or next job during closeout. Player saves/settings remain untouched.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D14E1 corner body guard (Needs playtest)

Slot 7 reproduces an obstruction camera outside Duke exposing his incomplete
first-person body. A draw-time eye/pelvis check now suppresses unsafe world-body
geometry, while preserving weapons, camera-relative kick and normal downward
body recovery. Build/native suites and bounded private FOV/aspect/replay checks
pass. [Note 302](302-d14e1-corner-body-guard.md) records evidence and limits.
Player files unchanged; no framework edits, commit/push or closeout.
Launch: `python3 recomp/tools/local/run.py`; load slot 7 and back out/re-enter.


## 2026-10-28 - D14E accepted and closed

User accepts D14E and explicitly requests lock-in and closeout. D14E is Done:
connected first-person locomotion body, clearance-based torso pose and scoped
pelvis lighting correction. Implementation, tests, manual and documentation
commit/push are authorized as MusicMonsterMod. Prior build/native-suite and
bounded runtime evidence stand with their limits in
[note 301](301-d14e-first-person-body.md). Default FOV and supported range remain
unchanged. No new gameplay change, rebuild, game launch or next job during
closeout. Player saves/settings remain untouched.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D14E connected first-person body candidate

D14E is Needs playtest. Researched studio first-person practices and iterated
native torso admission, waist pose and crouch clearance in game. The candidate
connects vest/belt/legs during ordinary movement without changing weapon framing,
world depth, the gameplay skeleton or Vanilla. Newly admitted pelvis geometry
also receives the guarded late-fade correction.

Build/native suites, FOV/aspect screenshots, movement recording and bounded
120-setting replay checks pass. [Note 301](301-d14e-first-person-body.md) records
iterations, verified routes, failed selection exclusions and remaining limits.
Human visual acceptance, full action/costume coverage and FOV above 90 are not
claimed. Player files unchanged; no framework edits, commit/push or closeout.
Launch: `python3 recomp/tools/local/run.py`; compare `fov 80` and `fov 90` looking
down during movement, crouch, jump and Q.


## 2026-10-28 - D14D accepted and closed

User confirms the adaptive sky FOV fix is all working and explicitly requests
lock-in and closeout. D14D is Done. The backdrop adapts to actual projection
and aspect, preserving default geometry, sky art/motion and Vanilla behavior.
Prior build, sky/near tests and private aspect/replay evidence stand with their
limits in [note 300](300-d14d-adaptive-sky-fov.md). Implementation, tests and
documentation commit/push are authorized as MusicMonsterMod. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D14D adaptive sky FOV candidate (Needs playtest)

The reported side seams are reproduced as insufficient partial-backdrop coverage
at wider world FOV. The backdrop now scales with actual render H and aspect,
including replay and 4:3. Build, sky/near suites and private slots 2/5/6 samples
pass within [note 300](300-d14d-adaptive-sky-fov.md)'s limits. Default geometry,
cloud motion/art and Vanilla retain prior paths. Player files are unchanged.
User visual acceptance remains; no acceptance closeout, commit or push authorized.
Launch: `python3 recomp/tools/local/run.py`; compare `fov 80` and `fov 90`.


## 2026-10-28 - D36I1 accepted and closed

User accepted the slot-5 mirror grooming arm correction and explicitly requested
lock-in and closeout. D36I1 is Done. Mirror arms now reach safe polygon clipping
without whole-joint cutoff popping, retaining native animation, world depth and
reflection ownership. Prior build, controls/input/aim/near suites and private
60/120, FOV, repeat-use and mid-action save/load checks stand with their limits in
[note 299](299-d36i1-mirror-arms.md). Implementation, tests and documentation
commit/push are authorized as MusicMonsterMod. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D14C accepted and closed

User accepted D14C and explicitly requested lock-in and closeout. D14C is Done:
expected world-FOV leg framing is retained, and native per-joint shading no
longer reapplies faded brightness after the limb correction. Prior build,
controls/input/aim/near suites and private visual/replay evidence stand with
the limits in [note 298](298-d14c-leg-fov.md). Implementation, tests, manual and
documentation commit/push are authorized as MusicMonsterMod. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D14C leg FOV lighting candidate (Needs playtest)

Matched geometry traces confirm expected world-FOV leg framing without changing
camera/pose. Wide FOV also exposed a native lighting defect: per-joint shading
reused faded brightness after the existing limb fade correction. The candidate
scopes the native brightness register through that joint, restoring it before
unowned geometry. World projection/depth, Q/door mechanics and D14B framing stay.
Build and controls/input/aim/near suites pass, with private visual, movement,
load/view and 120-setting replay evidence in [note 298](298-d14c-leg-fov.md).
User visual acceptance remains; no commit/push or next job is authorized.
Launch `python3 recomp/tools/local/run.py`; compare `fov 60`, `fov default`
and `fov 90` while looking down and moving. Player files are unchanged.


## 2026-10-28 - D14B accepted and closed; D14C backlog only

User confirms the arms are solid and accepts D14B, authorizing implementation,
tests, manual and documentation commit/push closeout as MusicMonsterMod.
D14B is Done; prior build/native-suite and bounded visual evidence stands with
its limits in [note 297](297-d14b-weapon-projection.md).

D14C is Todo: investigate first-person leg presentation across world FOV, starting
with a controlled 60/default/90 comparison when selected. Brief source and
existing-image review confirms body/kick retain world H, but does not establish
the cause of the user's uncertain leg-view report. No new game run, build or
leg implementation. No next job is selected. Launch:
`python3 recomp/tools/local/run.py`.

## 2026-10-28 - D14B weapon projection candidate (Needs playtest)

World FOV was shrinking and recentering the accepted weapon/arm display. The
object renderer now uses local H=256 for semantic DisplayHand/DisplayArm meshes,
including attached weapons, items and muzzle flashes. Native body, kick and
contact limbs retain world projection. No per-weapon retuning, gameplay-matrix,
asset, framework or save-format change. Build and native controls/input/aim/near
tests pass. Private base/alternate/upgraded weapon, aspect, replay and interaction
samples are documented with their limits in [note 297](297-d14b-weapon-projection.md).
Player files remain unchanged. D14B needs visual acceptance; no commit/push or
closeout is authorized. D19X remains separate. Launch:
`python3 recomp/tools/local/run.py`; compare `fov default` and `fov 90`.

## 2026-10-28 - D14A accepted; commit/push closeout

User confirms the first-person FOV playtest passes and accepts D14A. The board
now marks it Done. Session-only `fov [60..90|default]` is accepted; D14B weapon
presentation and D19X saved slider remain separate follow-ups. No new job is
started here; the user reports a separate background continuation.

Native controls with all-level fixtures, input, aim and near-clip tests pass.
Framework export is unchanged; clean pinned-source application and byte checks
pass for all 55 modified/new files. Repository/privacy checks pass. Historical
candidate evidence and broader coverage limits remain in [note 296](296-d14a-first-person-fov.md).
Commit and push are explicitly authorized. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-28 - D14B backlog intake closed; implementation unselected

Added D14B to the canonical job board: decouple first-person weapon/arms size
and placement from world FOV, using accepted original-FOV presentation as the
baseline. User reproduction: UI slot 4, `dnmonsters`, `dnstuff`, `fov 90`, cycle
weapons. The user praises world FOV and reports undersized/misplaced weapons;
this intake adds no independent gameplay evidence or implementation.

D14B is Todo. Scope includes projection versus placement diagnosis, reliable
geometry ownership, all weapon slots and effects, native body/legs, lighting,
aim/projectile behavior, walls and replay regressions. A future approval artifact
may be useful but is not requested now. The local continuation skill now records
plain locally opened HTML, like ttk-fonts, without a web server when HTML is
requested. Backlog documentation commit/push closeout is authorized; existing
D14A candidate changes remain separate and its evidence/status are unchanged.


## 2026-10-28 - D14A first-person FOV candidate (Needs playtest)

Session-only `fov [60..90|default]` and `DNTTK_FP_FOV` change real first-person
projection in horizontal degrees at 4:3. Default 64 preserves H=256 exactly;
third person retains its original projection. Replay snapshots carry the value.
A scoped correction prevents the native close-camera fade from hiding contact
arms at wider FOVs; native pose/world depth and intentional effects remain.

Build/native controls/input/aim/near suites pass. Private aspect/projection,
view-transition, replay and D36 interaction evidence, plus remaining playtest
limits, are in [note 296](296-d14a-first-person-fov.md). Player saves/settings
remain unchanged. D19X is unselected; no persistence/menu slider, commit/push or
closeout. Launch `python3 recomp/tools/local/run.py`, then use the backtick
console: `fov 75` / `fov 90`; `fov default` restores the exact baseline.


## 2026-10-28 - D19V accepted and closed (Done)

User accepted: "perfectly done, lock it in and close it out". The redundant
Use Interaction/Mirror view settings are removed; supported actions follow
global VIEW. Schema 57 migrates old overrides without resetting other valid
preferences. Implementation, tests, manual and documentation commit/push are
authorized as MusicMonsterMod. Prior build and regression results stand;
[note 295](295-d19v-action-view-cleanup.md) preserves automated verification
limits separately from user acceptance. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D19V action-view cleanup (Needs playtest)

USE INTERACTION VIEW and MIRROR VIEW are removed from Advanced. Supported use
and mirror actions follow global VIEW, retaining native action/presentation
ownership. Schema 57 removes old overrides, including stale menu sidecar values,
without resetting other valid settings. Hidden old launcher flags are no-ops.
Build, 105 profile tests and native controls/input/aim/near suites pass.
[note 295](295-d19v-action-view-cleanup.md) records private runtime evidence and
remaining user visual acceptance. No commit/push closeout or next job.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D09B accepted and closed (Done)

The user confirms "all working" and explicitly authorizes closeout. D09B is
Done: scheme-based aim/marker presets, exclusive native/modern markers and the
live-switch render-worker fix are accepted. Manual overrides remain available;
Vanilla is unchanged. Build, native aiming tests and private slot-4 before/after
captures passed; user playtest acceptance is distinct from injected diagnostics.
See [note 294](294-d09b-classic-aim.md). No further D09B work remains.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D09B live-switch marker fix (Needs playtest)

Reproduced the slot-4 red dot remaining visible after Legacy-to-Modern at
120 Hz. Forked render workers retained the previous live setting. Confirmed
scheme/red-dot changes now refresh workers after applying the complete preset.
Build/native aiming tests pass; composed before/after firing captures confirm
the Modern crosshair remains and the unwanted dot disappears. Private save
copies only. See [note 294](294-d09b-classic-aim.md). No commit/push.


## 2026-10-28 - D09B scheme presets (Needs playtest)

Prior marker behavior is user-confirmed working. Changing and confirming the
control scheme now applies aim/marker defaults: Legacy = auto-aim/red dot on,
crosshair off; Modern = the reverse. Manual adjustments remain available and
survive unchanged confirmation, save loads and restart. Cancel changes nothing.
Build/native aiming tests, private real-menu transitions, persisted overrides
and menu visual inspection pass. [Note 294](294-d09b-classic-aim.md) records
scope and evidence. No commit/push. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D09B slot-4 follow-up (Needs playtest)

User reports auto-aim works; missing red dot is single-player UI slot 4.
The private copy shows the native red dot during firing in composed output.
RED DOT ON now suppresses the modern crosshair and its hotkey; OFF restores
its previous preference. Build and native aiming tests pass, with a live-menu
restoration check. The reported disappearance is not diagnosed; physical-pad
acceptance remains. [Note 294](294-d09b-classic-aim.md). No commit/push.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D09B investigation and menu correction (Needs playtest)

Private single-player Legacy firing confirms native auto-aim and red-dot
operation, independently across all four preference combinations. The menu's
false RED DOT dependency on AUTO AIM was removed and its description clarified.
Player build, 106 profile tests and revised-menu visual inspection pass.
No gameplay or framework changes; physical-controller acceptance and the user's
specific failure remain unresolved. [Note 294](294-d09b-classic-aim.md) retains
fixture injections, evidence and limits. No commit/push or closeout authorized.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D17AD loading background accepted and closed

The native tiled loading backdrop now fills widescreen margins with translated
copies of its original artwork. The centre image, texture scale and spinner
remain unchanged. Adaptive 4:3 retains clipped side tiles for window growth
during a held loading frame. Build/native packet and save-recovery checks pass;
private fixed-aspect and resize captures are recorded in
[note 293](293-d17ad-loading-background.md), with verification limits.
User accepted: "great work! lock it in and close us out". D17AD is Done.
Implementation, tests, manual and documentation commit/push are authorized as
MusicMonsterMod. Existing verification limits remain; no new gameplay changes,
rebuild, launch or next job during closeout.
Launch from the workspace root: `python3 recomp/tools/local/run.py`.


## 2026-10-28 - D17AC phone visibility accepted and closed

The current slot-11 phone rear panel sits roughly 10-11 units inside its wall.
It remains submitted but loses visibility when the wall enters host depth
testing. A shape-verified 16-unit allowance applies only to the opaque phone
rear panel from its open side; raster geometry and gameplay stay unchanged.

Build, near packet contracts in all precision combinations and trace tests pass.
Private 60/120 approaches, third person and original precision retain the phone;
542 separate replay restoration checks have zero mismatches. Vanilla smoke and
41 player-file hash/mtime checks pass. Diagnostic games are closed.
[Note 292](292-d17ac-phone-wall-depth.md) records evidence and limits.
User accepted: "wonderful. lock it in and close it out". D17AC is Done;
implementation, tests and documentation commit/push are authorized as
MusicMonsterMod. No new gameplay changes or next job started during closeout.
Launch from the workspace root:
`python3 recomp/tools/local/run.py`, then F7 UI slot 11 and approach/retreat.


## 2026-10-28 - Five requests queued only

D19V (remove redundant action-view settings), D09B (classic gamepad auto-aim
and red dot), D19W (keyboard/gamepad bindings pages), D17AC (slot-11 phone)
and D17AD (widescreen loading background) are Todo in MODERNIZATION_JOBS.md.
User explicitly said not to work on them now and limited minor research to
five minutes overall. Source-only intake; no diagnosis, implementation, build,
launch or gameplay verification. Later research-launch permission was unused.
D09B requires a findings report before restoration and actual-gamepad Legacy
verification. No job selected. User subsequently authorized committing and
pushing this backlog intake; implementation remains unstarted.


## 2026-10-28 - Two-player baseline accepted; D34C closed for now

User: "lock it in, close us out. two player now works and is fundamentally
how it plays off the disk". The current original-style two-player baseline
and D34C ownership foundation are accepted for retention. This explicitly
authorizes implementation, tests and documentation commit/push closeout as
MusicMonsterMod.

D34C is Closed for now: foundation accepted, modern gameplay and presentation
deferred. Independent modern per-player movement/camera/aim, action contexts,
viewport replay and 60/120 Hz acceptance remain unfinished. Its existing scope
and acceptance matrix are retained for a future explicit selection; they are
not marked Done by this baseline acceptance. The user's report does not add
agent-observed physical-device, all-arena or performance tests.

Existing build and bounded verification remain in [note 291](291-d34c-local-multiplayer-foundation.md).
No gameplay changes, rebuild or launch for this closeout. Player settings,
saves and cards remain untouched. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D34C selected; initial ownership foundation (In progress)

D34C is selected. Independent two-assignment SDL samples and device generations
are implemented and tested. A separate owned-disc manifest verifies all six
arena BASEOVR overlays; campaign guards stay unchanged. The player build,
native input suite and eight overlay tests pass. [Note 291](291-d34c-local-multiplayer-foundation.md)
records static ownership evidence, the bounded private results-state inspection,
and remaining work. No modern split-screen gameplay candidate is ready yet:
per-player actions, movement, cameras, aiming and presentation remain. No
physical-device or performance acceptance claimed. No commit/push or player
settings/card changes. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D09A accepted and closed; D34C opened

User reports the controller/keyboard arrangement "100% works" and two-player
works as a foundational first step, then explicitly requests the next
modernization job and lock-in/closeout. D09A is Done by user acceptance.
Implementation, tests, framework patch, manual and documentation commit/push are
authorized as MusicMonsterMod. This does not claim new automated or exhaustive
two-physical-pad tests, or diagnose the historical character-selection report.

D34C is now Todo: independent per-player movement/camera/aim, applicable modern
actions and per-viewport presentation for local split-screen, preserving the
classic option and original match rules. It is the next multiplayer job, not
started. Online/LAN and first-person presentation overhaul remain separate.
Existing bounded evidence is retained in [note 290](290-d09a-keyboard-gamepad-seats.md).
No gameplay edits, rebuild or launch during closeout; player settings/cards
remain untouched. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D09A keyboard/gamepad seats (Needs playtest)

Explicit KEYS AND PAD preference routes keyboard to P1 and the first gamepad
to P2 throughout character selection and original split-screen play. Two-pad
routing remains selectable. Mixed-mode hotplug preserves the unused pad's
assignment instead of promoting it; bound keyboard/mouse actions work without
single-player mouse capture. Schema 56 retains existing preferences.

Build, native input/controls/aim and 106 profile tests pass. Private SDL/X11
selection, separate movement/actions, pause/resume, match save/load, unplug/
reconnect and results/exit routes pass within [note 290](290-d09a-keyboard-gamepad-seats.md)'s
limits. Two-pad and campaign regressions pass. Physical-controller and display
acceptance remain. Framework patch exported and clean-pin verified; player
settings/cards untouched. No acceptance, commit or push performed.


## 2026-10-27 - D09 accepted and closed; D09A queued

User: "controller is working solidly" and "lock it in, close it out".
D09 is Done by user acceptance. Implementation, regression tests, framework
patch, manual and documentation commit/push are explicitly authorized as
MusicMonsterMod. Existing build and bounded verification evidence remain in
[note 289](289-d09-modern-controller.md); no gameplay change or
new build is needed for closeout. User acceptance is not a newly observed
physical two-pad match or a diagnosis of the earlier character-selection report.

D09A is Todo in the background backlog: keyboard and gamepad as separate local
player controllers, with independent character confirmation and gameplay input.
It is scoped on the board and not started. Player settings/cards remain untouched.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D09 modern controller candidate (Needs playtest)

D09 was selected and implemented as a bounded Modernized campaign candidate.
Left stick moves relative to the camera; right stick looks/aims independently.
Fixed pad actions use the existing gameplay layer alongside keyboard/mouse.
OPTIONS > CONTROLS adds saved look speed, radial deadzone and inversion.
A release gate prevents held menu confirmation from becoming a gameplay jump.
Classic split screen and Vanilla retain their original pad controls.

Build, native SDL input/controls/aim and profile tests pass. Private 60/120
campaign routes and two-seat selection/movement/pause/results routes pass
within [note 289](289-d09-modern-controller.md)'s limits. One PS3
controller is recognized with neutral readings inside the default deadzone;
physical movement, reconnect, two-pad match and display acceptance remain.
The historical Red Duke / Blue Duke blocker is not diagnosed. Character
selection now explains the two-controller confirmation requirement.

No player settings/cards changed. No commit/push or next job started.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D34B closed for now; controller acceptance deferred

User reports being stuck at Red Duke / Blue Duke character selection and
explicitly requests no code changes: "lock it in and close it out for now".
D34B is Closed for now, with physical-controller and real-display acceptance
deferred to D09. This is authorization to retain and commit/push the existing
implementation, not a successful physical-controller playtest. The cause of
the reported selection blocker has not been diagnosed.

Prior virtual SDL and native test evidence remains valid within
[Note 288](288-d34b-classic-split-screen.md)'s limits. No code changes, build or
game launch during this closeout. D09 now records the selection blocker and
required two-pad match regression; it remains Todo and has not been started.
Implementation, tests, framework patch and documentation are included in the
authorized MusicMonsterMod closeout. No player settings or cards changed.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D34B classic split screen ready for playtest

D34B Needs playtest. Two independent controller seats and character selection,
native 4:3 split presentation, original match controls, and results Quit to MAIN
MENU are implemented. Old one-seat snapshots remain readable without migration.
Modernized stays the default; Vanilla remains available.

Build and SIO compatibility/input/controls/aim tests pass. Private SDL virtual
pads exercised both orientations, separate movement, kill/respawn, pause from
either pad, match save/load and a natural five-minute match through results.
Private campaign Modernized and Vanilla save-load/movement checks pass, including
Modernized pause/resume mouse capture. Physical-controller and real-display
acceptance remain; native alternating viewport cadence is intentional.
[Note 288](288-d34b-classic-split-screen.md) records evidence and bounded claims.

Framework patch exported and verified against its clean pin. Player settings
and cards untouched. No acceptance, commit or push performed. Connect two
controllers, choose TWO PLAYER, confirm each character on its own pad, then
confirm again to start. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D18B accepted and closed

User: "perfect!!!! lock it in and close it out. absolutely beautiful".
D18B is Done. Continuous Modernized music under speech and its listening
presentation are accepted. Implementation, tests, manual and documentation
commit/push are explicitly authorized as MusicMonsterMod.

Accepted executable remains unchanged: music/XA/gadget native tests and
private 60 Hz lifecycle and hardware-backed 120 Hz overlap evidence pass within
[Note 287](287-d18b-concurrent-music.md)'s stated limits. Preserve native-sector
rather than sample-exact music restoration on load, and the distinction between
software-renderer starvation and the clean hardware-backed run. D18C remains
open. Closeout verifies repository/privacy hygiene and the exported framework
patch against its clean pin. No next job started.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D18B concurrent music ready for playtest

Modernized level music now continues underneath native speech. The independent
stream reuses the runtime XA decoder and owned mounted disc, respects existing
SOUND gains/mono mode, pauses with gameplay and resets ownership on loads or
track changes. Native speech timing and Vanilla playback remain in place.
A linked stereo peak guard adds no output buffering.

Build and music/XA/gadget native suites pass. Private 60 Hz lifecycle captures
and the final NVIDIA offscreen 120 Hz voice route have zero output underruns.
Software 120 Hz also underruns with the original audio path; D18C remains open.
Listening, balance and broad campaign/death/Continue acceptance remain pending.
Loads reconstruct from the native saved music sector rather than preserving a
sample-exact independent cursor. [Note 287](287-d18b-concurrent-music.md) records
implementation, comparisons, source/fixture evidence and remaining limits.

D18B Needs playtest. Player settings/cards untouched. No commit/push authorized
or performed; no next job started. Launch from the workspace root:
`python3 recomp/tools/local/run.py`.

## 2026-10-27 - D08K13/D08K14 accepted and closed

User explicitly accepts revision 2 ("this work is perfect") and requests lock-in,
closeout, the updated HTML artifact and extensive documentation. Both jobs are
Done. Implementation, tests, manual and documentation commit/push are authorized
as MusicMonsterMod. Preserve this accepted gait and current travel speeds.

D08K13 refreshes the native inactivity timeout while Modern crouch owns movement,
preventing sustained backwards input from escaping through fidget 179 into roll
181. D08K14 supplies distinct 48-pose walk/run clips: lower hands, relaxed wrists,
opposing arms, contact compression, toe-off and full-boot floor clearance. Run
uses longer strides and brief flight; walk/run changes retain phase and blend
from the displayed pose. Stationary kneel, collision/clearance/ledge ownership,
Original and Vanilla behavior remain unchanged.

Build, 16 animation tests and native controls/input/aim suites passed. Real SDL
third-person/default and first-person/rebound routes passed both crouch controls,
at least nine seconds sustained backwards run per control and repeated
forward/backward/run changes. User acceptance closes visual review; natural
vent traversal and broad terrain/armed clipping coverage remain unverified.

Accepted local artifact: `recomp/analysis/d08k14/revision2/preview/index.html`.
Tracked regeneration tool: `recomp/tools/local/preview_crouch_run.py`.
[Note 286](286-d08k13-d08k14-crouch-movement.md) records the root cause, MGS frame
study, rejected first candidate, user speed decision, exact accepted parameters,
implementation map, evidence, limitations and reproduction commands. No next job
started. Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D08K12 and D21A accepted and closed

User confirmed crouch running works, accepted Ctrl takeover and explicitly
requested: "perfect! lock it in, close it out". Both jobs are Done.
Implementation, tests, manual and documentation commit/push are authorized as
MusicMonsterMod. Run while crouched uses configured pace controls; pressing
Ctrl during C-toggle crouch keeps Duke low until Ctrl release, subject to
standing clearance. Space cancellation remains unchanged.

Player build, native input/controls/aim suites and private 60/120 real-SDL
routes passed. User acceptance supplies final gameplay approval while notes
284/285 retain the natural-vent, live-ledge and bounded-test evidence limits.
No next job started.

Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D21A Ctrl takeover ready for playtest

User confirms D08K12 works. D21A adjusts the previous Ctrl cancellation:
pressing Ctrl during C-toggle crouch transfers to momentary crouch, keeping Duke
low until Ctrl release. Standing still requires clearance. Space retains its
stand-without-jumping cancellation. Custom keyboard and mouse bindings work.

Build and native input/controls suites pass. Private real-SDL 60/default and
120/rebound tests pass for takeover/hold/release, crouch running, pause/load and
Space/fresh-jump behavior. [Note 285](285-d21a-crouch-takeover.md) retains limits.
D21A Needs playtest; D08K12 user-verified. No closeout, commit or push authorized.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D08K12 crouch running ready for playtest

D08K12 is Needs playtest. Hold the configured speed modifier while using Ctrl
crouch or C toggle to move at shared running pace; release restores walking pace
with RUN MODE off. RUN MODE inversion follows standing controls. The existing
crouch gait advances with distance; no new animation or geometry is introduced.
Original swimming's forced shallow-water run is excluded from crouch selection.

Build and native controls/input/aim suites pass. Private real-SDL 60 third-person
and 120 first-person tests pass for held/toggled crouch, release, custom bindings,
save/load and cancellation. Low clearance, blocked standing and ledge handoff
are native-fixture verified; natural vents and live ledge departures still need
player review, along with faster-gait feel. [Note 284](284-d08k12-crouch-running.md).
Player settings/cards untouched. No closeout, commit or push authorized.

Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D10E accepted and closed

User accepted the final free-look and release-steering behavior and explicitly
authorized lock-in/closeout. D10E is Done. Hold middle mouse in third person to
inspect without changing movement/aim heading; release restores mouse steering
immediately while the visual offset eases back. FREE LOOK is rebindable and
existing custom middle-button bindings are preserved.

Player build, native controls/input/aim suites, 120 Python input/profile tests
and private 60/120 routes passed, including repeated release while continuously
moving the mouse. [Note 283](283-d10e-free-look.md) retains evidence and limits.
Implementation, tests and documentation commit/push authorized as MusicMonsterMod.
D08K12 crouch running remains Todo after D18 follow-ups. No next job started.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D10E release steering correction

User reported intermittent loss of mouse steering after free look. Reproduced:
continuous mouse input replenished the return offset indefinitely while gameplay
yaw stayed fixed. Release now restores steering immediately and decays only the
remaining visual offset. Private 60/120 runs pass three repeated releases with
continuous mouse motion; a native regression checks first-update steering and
return completion. [Note 283](283-d10e-free-look.md) records the cause and evidence.
Rebuilt candidate remains Needs playtest; no commit/push.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D10E free-look candidate; D08K12 queued

D10E is Needs playtest. Hold middle mouse in third person to inspect while
movement, facing and fire retain their pre-orbit direction; release/V eases back.
Crosshair hides during inspection. FREE LOOK is rebindable; schema 54 preserves
custom middle-button bindings. Ground locomotion only; other camera owners clear
inspection. Build, native controls/input/aim suites and 120 Python tests pass;
private 60/120 gameplay evidence and limits are in [note 283](283-d10e-free-look.md).
Human motion/feel acceptance remains open. No framework edits, commit or push.

D08K12 crouch running is Todo after D18's remaining follow-ups. Support both
Ctrl and C via configured run/crouch actions, preserve crouch clearance/animations
and ledge walk-offs, and test vent traversal and blocked standing when selected.
No crouch-run implementation started.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D21 accepted and closed

User accepted the completed SOUND menu and HELD CROUCH behavior and explicitly
authorized closeout. D21 is Done. SOUND provides Master, Sound Effects, Music,
Speech and stereo/mono mode with confirm/cancel/defaults and persistence.
C toggles crouch; Ctrl or Space cancels it and stands Duke up where there is
room. The cancelling hold is consumed; fresh Ctrl crouches and fresh Space
jumps. Both actions remain configurable. No accessibility panel was added;
VIEW BOB remains in GAMEPLAY and Vanilla keeps its existing play behavior.

Build, native SDL regression, 118 Python input/profile tests and private live
menu/input checks passed. User acceptance supplies the final playtest decision.
Note 282 retains the test evidence and historical iterations. D18 remains Done
as accepted research; its separate implementation jobs are unchanged.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D21 Space exits HELD CROUCH

Space now cancels C's crouch toggle and stands Duke up where there is room,
without jumping. The cancelling jump hold is consumed until release; a fresh
press jumps normally. This follows the configurable JUMP binding. Ctrl's
previous cancellation remains intact. SOUND is unchanged.

Player build, native SDL regression and fourteen Python input/profile tests
pass. Private real-SDL gameplay verifies standing idle throughout the consumed
Space hold and jumping on the next press, alongside C/Ctrl/pause/load regression.
Ready for user playtest; no closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D21 Ctrl cancels HELD CROUCH

At the user's request, pressing Ctrl while C has crouch latched now clears
that latch and stands Duke up when there is room. The cancelling hold stays
consumed until release; the next Ctrl press works as momentary crouch again.
This follows the configurable CROUCH binding, including mouse bindings.
Player build, native SDL input suite, fourteen Python tests and private real-SDL
gameplay checks pass, including remaining upright while Ctrl stays down and
normal crouch on the next press. SOUND remains user-accepted and unchanged.
The new cancellation is ready for user playtest; no closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D21 HELD CROUCH corrected to toggle

User accepted SOUND, then clarified that C should keep crouch after release.
C now toggles the stance; Ctrl remains momentary. HELD CROUCH remains the label,
with explicit toggle help. Pause preserves the toggle; load/death/rebinding and
swim/jetpack/unsupported traversal clear it. Sound code unchanged.

Build, native SDL input regression, fourteen Python input/profile tests and
private real-SDL gameplay checks pass. Note 282 records the corrected candidate
and boundaries. C correction Needs playtest; no closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.


## 2026-10-27 - D21 candidate ready for playtest

SOUND is recomp-owned in the accepted menu style: Master, Sound Effects, Music,
Speech and Mode. Confirm applies/saves, Cancel discards, Defaults resets draft.
C is the configurable CROUCH 2 held input alongside Ctrl. No accessibility
panel or other proposed accessibility features were added.

Build, 118 Python tests, native SDL input suite, pause 60/120 and title 120
captures, native gain/persistence/load checks and private real SDL crouch checks
pass. Actual listening and broader audio-transition coverage remain open.
D21 is Needs playtest; note 282 has details and the candidate fingerprint.
No player saves/cards touched, framework changes or commit/push.
Launch from workspace root: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D21 scope correction

D21 now covers only the reworked SOUND menu and configurable C held crouch.
The accessibility panel proposal is withdrawn; VIEW BOB stays in GAMEPLAY.
No crouch toggle, text sizing or subtitle work. Sound-only preview revised
at the same standalone link. Note 282 records source findings, including
C binding restrictions and four-slider replay storage considerations.
Design/source investigation only; no gameplay changes, build, commit or push.


## 2026-10-27 - D18 complete; D21 design kickoff

D18 is Done by explicit user acceptance of its completed research scope.
Note 281 retains unverified listening/sync, desktop/window-close and natural
ending routes as evidence limits; these no longer block D18 completion.
D21 is In progress: initial accessibility/sound research and interactive
preview requested. Note 282 records existing controls, proposed scope and
the D18B dependency loop. No gameplay changes, build or launch. No commit
or push authorized in this session. Existing framework changes preserved.


## 2026-10-27 - D18 reliability session closeout authorized

User requested "lock us in and close us out" after the previous session clear.
The delivered D18 investigation, diagnostic runner and D10E/D18F backlog intake
are locked in for commit/push as MusicMonsterMod. D18 remains Needs playtest:
listening/sync, real desktop display/window-close and natural ending routes
remain unverified. D10E and D18F remain Todo; no next implementation started.
Note 281 preserves the measured results and unresolved offscreen starvation.
Closeout verification: four FMV poll/DMA math tests, runner syntax and CLI help
pass. Framework export is unchanged and applies cleanly to the pinned dependency.
No new build or game launch; player saves/cards remain untouched.
Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## 2026-10-27 - D18F recomp-owned FMV playback queued

D18F is Todo, backlog only: preserve original movies while giving the recomp
ownership of timing, presentation and audio, with an optional replacement/mod
asset interface. Scope and acceptance are on the job board, grounded in note
281. No implementation started; D18's outstanding playtest checks remain open.


## 2026-10-27 - D18 reliability findings ready for playtest; D10E queued

D10E is Todo only: optional held middle-mouse third-person free look, inspired
by @vakho10's GitHub issue #1. Input/camera/movement/aiming ownership inspected;
no camera implementation started. D18 is Needs playtest.
[Note 281](281-d18-fmv-reliability.md) records all eight native-table clips,
private playback/skip/quit routes, cadence, seek/read and audio counters.
Listening/sync, real desktop fullscreen/window-close and natural ending routes
remain open. Both profiles reached fresh gameplay and resumed music; Modernized
offscreen gameplay showed output starvation, documented without a desktop
regression/cause claim. No gameplay or decoder changes; diagnostic runner added.
No player saves/cards touched; no commit/push authorized.


## 2026-10-27 - D17AB accepted and closed

D17AB is Done. User: "lock it in and close us out!". Mirror cloud/HUD/palette
fixes accepted; implementation, tests and documentation commit/push authorized.
[Note 280](280-d17ab-mirror-rendering.md) retains packet-lifetime diagnosis,
old/fixed comparisons, 60/120 evidence and verification limits. Earlier candidate
entries are historical. No new build, game launch, framework edits or next job
during closeout. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D17AB clouds traced; Needs playtest

Pre-fix/sky-off/fixed comparisons identify overhead sky triangles as another
manifestation of premature deferred-packet arena reuse. Fixed trace has zero
rapid weapon-to-sky address reuses (old: 14), with normal sky emission retained.
Mirror 4x 60/120, sky suite and bounded outdoor 120 checks pass. Existing candidate
unchanged; no new sky-source or framework edits. Earlier open-cloud entries are
historical. [Note 280](280-d17ab-mirror-rendering.md) records evidence and limits.
User playtest pending; no acceptance closeout, commit or push.
Launch from workspace root: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D17AB mirror investigation and corruption candidate

In progress. New selected job covers slot-5 indoor sky/clouds, close-range HUD
loss and freezer jump/fire palette corruption. A pending-viewmodel packet arena
reset defect is reproduced and guarded; build, old-fail/new-pass regression and
Original/Corrected near suites pass. Private 60/120 candidate captures and verified
120 freezer stress/reload avoid the reproduced corruption. Cloud cause and full
HUD-transition acceptance remain open. [Note 280](280-d17ab-mirror-rendering.md).
No acceptance closeout, commit or push. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D19Q4 accepted and closed

D19Q4 is Done. User: "lock it in and close it out". Implementation, tests,
framework patch, manual and documentation commit/push authorized. Saved overwrite
text and restored-inventory flashes are fixed in the accepted candidate.
[Note 279](279-d19q4-load-ui-flashes.md) retains the 60/120 load-route evidence,
regression results and verification limits. Earlier candidate entries below are
historical. No new build, game launch or next job during closeout.
Launch from the workspace root: `python3 recomp/tools/local/run.py`.


## 2026-10-27 - D19Q4 load UI candidate

D19Q4 is Needs playtest. Reproduced two separate causes: saved overwrite-dialog
VRAM presented during auto-resume, and inventory restoration announced as a new
selection. Candidate waits for a fresh gameplay frame and clears old inventory
feedback across successful loads. Private 60/120 LOAD GAME/F9/F7, paused/repeated
loads, movement and later pause/resume pass; deliberate confirmations and bracket
browsing remain. Native inventory/controls and D37A checks pass; Vanilla pause and
statistics preserved. Framework patch exported and clean-pin verified.
[Note 279](279-d19q4-load-ui-flashes.md) records evidence and limits.
Awaiting user acceptance. No closeout, commit or push. Launch from workspace root:
`python3 recomp/tools/local/run.py`.


## 2026-10-27 - PRIV01 privacy cleanup accepted

PRIV01 is Done. User accepted the cleanup and explicitly authorized closeout,
commit and push. Removed 311 username occurrences from 85 tracked files,
including Markdown, JSON reports and two Blender assets. Updated AGENTS.md,
Claude and Cursor guidance plus the locally installed Codex continue skill.
CI now rejects literal personal home paths in tracked text and binary content.
Tracked byte scan, repository hygiene, JSON validation and Blender loading
passed; no gameplay changes or build required. Existing Git history is retained
by user decision. Existing submodule working changes remain untouched.


## 2026-10-27 - D16 bounded texture foundation accepted

The user accepted the visible club replacement and all D16 work, then explicitly
authorized commit and push. D16 is Done at the demonstrated foundation scope:
capture/export -> editable Photon art -> compile -> original/HD replacement.
Existing fallback, identity and save/load evidence stands; no broader campaign
coverage is claimed. Scripting/modding, new material assignment, animated/cutout
coverage and optional palette-constrained exports are future work, not remaining
acceptance requirements. Replacement RGB is not quantized to the original CLUT;
native cutout/blend classification and lighting remain in charge.
[Note 278](278-d16-texture-foundation.md) records the workflow and limitations.
Retail-derived artwork, captures and private saves remain local.


## 2026-10-27 - D16 real texture specimen ready for review

D16 is Needs playtest for its bounded first candidate. Exact DB00A.RAW source
and palette were captured, edited in Photon and replaced in game at 40x40 and
160x160. Interactive evidence: `recomp/analysis/d16-texture-foundation/texture-proof.html`.
Editable source and all retail-derived evidence stay local. [Note 278](278-d16-texture-foundation.md)
records commands, identity/invalidation rules, tests and coverage limits.
Build, 201 tests (2 skipped), runtime checks and clean-pin patch verification
pass. Private 60/120 checks cover the sample, palette/missing-file fallback and
save/load restoration. One-region RGB-only OpenGL foundation; broader coverage
and user visual acceptance remain. No acceptance closeout, commit or push.
Normal launch: `python3 recomp/tools/local/run.py`; original textures unless
the one-launch `DNTTK_TEXTURE_SAMPLE` variable is supplied (see note 278).


## 2026-10-27 - D16 scope revised; not started

D16 remains Todo, now "Texture manipulation and replacement foundation". User
wants the technical door opened for texture edits/replacements, demonstrated by
a controlled small Photon-authored example. Research texture identity/lifecycle
and preserve native palette, animation, transparency and reuse behavior. A
modding pipeline, full HD pack and user-facing mod tools are outside this job.
See the canonical D16 acceptance criteria in `MODERNIZATION_JOBS.md`. This was
backlog scoping only; no implementation, build, launch, commit or push.


## 2026-10-26 - D36H2 accepted and closed

D36H2 is Done. User: "lock it in, close it out!!". Both Buffalo grip corrections
are accepted; implementation, tests, manual and documentation commit/push
explicitly authorized. [Note 277](277-d36h2-buffalo-grip.md) retains measured
ordering evidence, 60/120 checks and coverage limits. Accepted executable:
`360debe3975119f1f57a121f8173e5fd7ccbf58bf4c11df35d753e80703b987b`.
No new build, launch, framework edits or next job during closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H2 right-wrist running candidate

Left-hand fix accepted. The selected running follow-up is Needs playtest.
The forearm cuff immediately below the right glove rolled against the stock;
record 6 now uses its ready right-chain relationship during native running.
Right-hand/weapon anchor, left grip and reload stay unchanged. Build and
controls/aim/near suites pass, including new cuff/transition/replay tests.
Private 60 trace and 120 running/fire/load/view checks are in
[note 277](277-d36h2-buffalo-grip.md), with coverage limits. Test running,
turning, strafe and fire/reload with weapon 9. All private processes stopped.
No acceptance closeout, commit or push. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H2 left-hand grip candidate

Needs playtest. Left/right turning reproduced a glove/rifle painter-order
reversal. A six-unit support-grip seating adjustment removes the measured
reversal; accepted rifle lift/cock/return and right grip remain unchanged.
Build and controls/aim/near suites pass; private 60/120 turning, reload and
mid-reload save/load checks completed. [Note 277](277-d36h2-buffalo-grip.md)
records bounded visual evidence and remaining coverage limits. Test weapon 9,
turn left/right while watching the left glove, then fire/reload and crouch.
No acceptance closeout, commit or push. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D07G accepted and closed

D07G is Done. User: "perfect, lock it in and close it out". Windowed crosshair
readability is accepted; implementation, manual and documentation commit/push
explicitly authorized. [Note 276](276-d07g-crosshair-scaling.md) retains build,
aim/UI tests, 36 measured capture checks and remaining platform coverage limits.
No new build, game launch or next job during closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D07G crosshair scaling candidate

Needs playtest. Crosshair scaling now follows fitted game-picture height with
a crisp 18-pixel minimum and fractional growth at larger sizes. OpenGL and
SDL3/software window/aspect checks pass; aiming and visibility behavior retained.
Build, native aim suite and 10 UI art tests pass. Framework patch exported and
checked against the pinned dependency. [Note 276](276-d07g-crosshair-scaling.md).
User should compare small windows, resizing and fullscreen, and I off/on.
No acceptance closeout, commit or push. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D35J5 accepted and closed

D35J5 is Done. User: "awesome!!! lock it in and close us out". Frozen first-person
blue vision is accepted; implementation, tests, manual and documentation
commit/push explicitly authorized. [Note 275](275-d35j5-frozen-first-person.md)
retains the tested 60/120 slot-11 route, renderer workaround and coverage limits.
No new build, game launch, framework edits or next job during closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D35J5 frozen first-person candidate

Needs playtest. Slot 11 reflected shots now retain first person with blue vision;
native frozen restrictions and thaw remain authoritative. Final build, native
controls/aim/near suites, 36 profile tests and private 60/120 lifecycle checks pass.
Modernized now uses the existing mixed ordered GL path at native rates too, to
preserve frozen HUD icons. Its isolated-path root cause remains unresolved.
Death/continue, water/gas and broad renderer regression coverage remain limited.
[Note 275](275-d35j5-frozen-first-person.md) records evidence and limitations.
Load UI slot 11, keep aim and shoot. No commit/push or acceptance closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D08A31 closed with no action required

D08A31 is Done by explicit user direction: "close this one with no action
required and close us out". Physical Left Shift+W+1/2 input loses number
presses and delivers unexpected Shift releases; the user reports the same
failure in EDuke. Right Shift and RUN MODE work. Specific keyboard/host cause
remains unconfirmed. [Note 274](274-d08a31-running-number-keys.md).
Temporary tracing/test additions removed; normal player rebuilt. No gameplay,
binding, Vanilla or framework changes. Documentation closeout and commit/push
authorized. Diagnostic session stopped; no next job started.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36K1 accepted and closed

D36K1 is Done. User: "lock it in, close it out". Unarmed jetpack arm continuity
is accepted; implementation, tests, manual and documentation commit/push
explicitly authorized. [Note 273](273-d36k1-jetpack-arms.md) retains verification
limits; earlier candidate entries are historical. Accepted executable:
`1af5089a7d377ae6480dadfd496a97724797f743e538259ca29b3cd18c430427`.
No new build/game launch, framework edits or next job during closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36K1 unarmed jetpack arms candidate

Needs playtest. Native flight arms now share safe world clipping, preventing
whole-arm rejection while hands remain drawable. Native flight poses and ground
arm lowering are preserved. Build and controls/aim/near suites pass; private
60/120 movement, switching, view and save/load checks pass. Deep-pitch backward
flight captures show connected forearms. [Note 273](273-d36k1-jetpack-arms.md)
records evidence and limits. Candidate:
`1af5089a7d377ae6480dadfd496a97724797f743e538259ca29b3cd18c430427`.
No acceptance closeout, commit/push or framework change. Launch
`python3 recomp/tools/local/run.py`; select Boot/empty hands and first person,
fly backwards and sweep pitch. User visual acceptance remains.


## 2026-10-26 - D36H4 accepted and closed

D36H4 is Done. User: "lock it in and close it out!". Connected unarmed arms
and the substantially lowered framing are accepted. Implementation, tests,
manual and documentation commit/push authorized. [Note 272](272-d36h4-unarmed-running-arms.md)
retains bounded verification and coverage limits; earlier candidate entries
are historical. Accepted executable:
`e975163dad5028dc629fae8bf43031c0901086c7d64143f623a8345261ff60e3`.
No new build/game launch, framework edits or next job during closeout.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H4 lowered-arms revision

User requested substantially lower unarmed arms. Both connected chains now
render 120 camera-down units lower, preserving native gameplay matrices and
swing. Build and controls suite pass; private 60/120 framing, Q, switching,
crouch, load and actual view restoration checked. [Note 272](272-d36h4-unarmed-running-arms.md).
Needs playtest for height. Candidate
`e975163dad5028dc629fae8bf43031c0901086c7d64143f623a8345261ff60e3`.
No acceptance closeout, commit or push. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H4 unarmed locomotion arms candidate

Research completed and the selected fix implemented. Empty-handed arms now use
native-body safe clipping, retaining their original swing and world depth.
Build and controls/aim/near suites pass; private 60/120 captures reproduce the
floating glove before and show connected arms after, with switching, Q, crouch,
load and verified first/third restoration. [Note 272](272-d36h4-unarmed-running-arms.md)
records evidence and limits. Needs playtest: key 1/Boot, run and look slightly
down, then strafe/stop/kick/switch. No new setting required.
Candidate `f400701023c3173a1581c213566406ff629b43f800d5ce381b036ed2a96ca937`.
No framework change, acceptance closeout, commit or push.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H3 accepted and closed

D36H3 is Done. User: "lock it in, close it out!!!". Final running-arm motion and
support grips are accepted; implementation, tests, manual and documentation
commit/push explicitly authorized. Accepted executable:
`87994034abe221147568f74de42614cbb7221e07323bc88e8260fc3125ff0143`.
[Note 271](271-d36h3-running-arm-motion.md) retains automated/live evidence,
the corrected sparse ready-clip handling and coverage limits. Earlier candidate
entries are history. No new build or game launch for closeout, no framework
changes and no next job started. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H3 revision 3 Energy support grip

Needs playtest. User reports only Energy Weapon 7's left-hand placement remains.
Its support now uses the firing grip during idle/run as well, retaining the
leftward clearance. Other weapons and the right-hand/weapon anchors are unchanged.
Build and the owned-data controls suite pass; [note 271](271-d36h3-running-arm-motion.md)
records private visual evidence and limits.
Candidate `87994034abe221147568f74de42614cbb7221e07323bc88e8260fc3125ff0143`.
Launch `python3 recomp/tools/local/run.py`, select 7, and compare idle/run/fire.
No acceptance closeout, commit/push or next job.


## 2026-10-26 - D36H3 revision 2 support grips

Needs playtest. User accepts shotgun 3's first-pass look. Gatling 4 and
Freezethrower 0 now use walking left-hand grips while running, after fixing the
sparse ready-29 reference that the first pass rejected. Energy Weapon 7's left
chain moves slightly left; Flamethrower 8 keeps its firing support grip at rest
and in motion. Weapon/right-hand anchors and native events remain unchanged.
Player build and controls/aim/near suites pass; private 60/120 visual checks and
4/0 replay sequences pass within the limits in [note 271](271-d36h3-running-arm-motion.md).
Candidate `fe66b625acd263957cd2e24cf27a535ec0c019cc7a85879118bf9f6b5ee64faf`.
Launch `python3 recomp/tools/local/run.py`; review 4/0 running grips, 7 clearance,
and 8 idle/run/fire grip. No commit/push, acceptance closeout or next job.


## 2026-10-26 - D36H3 running-arm candidate

D36H3 is Needs playtest. The five reported two-handed families retain a small
amount of native running motion while their display arms stay close to the ready
grip. Weapon framing, camera bob, native gameplay/events and the accepted Gatling
contact remain intact. Build and controls/aim/near native suites pass; private
60/120 captures cover base/upgrades, replay and bounded lifecycle regressions.
[Note 271](271-d36h3-running-arm-motion.md) preserves evidence and limits.
Candidate `4772a51eb00eab1adf1d40246c4dc4d37f1f2c5114b18cf5dcf64a4e05a2394a`.
User visual acceptance pending; no commit/push or next job started.
Launch `python3 recomp/tools/local/run.py`, use first person, keys 3/4/7/8/0,
and judge running/strafe/start-stop motion, then firing and switching.

## 2026-10-26 - D08A30 accepted and closed

D08A30 is Done. User: "perfect. lock it in, close it out". Reversed weapon-wheel
direction is accepted: up = next, down = previous. Implementation, input tests,
manual and documentation commit/push explicitly authorized.
Accepted executable: `baab7c72704d40e84557544a2f34abd43153577b0f64670006de7b63c7865b84`.
The board's D08A30 work log preserves automated/live evidence and limits.
Earlier candidate/backlog entries are history. No new build or game launch
for closeout, no framework changes, and no next job started.
Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D08A30 wheel direction candidate

D08A30 is Needs playtest. Captured gameplay wheel up now selects next and down
selects previous. Keyboard actions, Alt-wheel and weapon safety are unchanged.
Player build and native input/controls/aim suites pass; private offscreen
120-setting checks cover both directions idle, running in both views, flying
and swimming (including the underwater availability filter). See the D08A30
work-log entry in MODERNIZATION_JOBS.md for evidence and limits.
Candidate: `baab7c72704d40e84557544a2f34abd43153577b0f64670006de7b63c7865b84`.
Player files preserved; runs stopped. No commit/push or next job started.
Launch `python3 recomp/tools/local/run.py` and check both directions while
standing and running, including RUN MODE. Earlier backlog entries are history.


## 2026-10-26 - D08A29 accepted and closed; D08A30 backlog

D08A29 is Done. User accepted running weapon selection and explicitly authorized
lock-in/closeout, including implementation and documentation commit/push.
Accepted executable:
`01dfa5416a79dc5858c5665786216010b6abc4b2824f15085c88d592636a2233`.
[Note 270](270-d08a29-running-weapon-selection.md) retains the test evidence and
scripted wall-stop/remote coverage limits. Earlier candidate entries are history.
D08A30 is Todo: wheel up selects next, wheel down selects previous. Backlog only;
current wheel mapping remains unchanged, and no next implementation started.
No new build or game launch during closeout. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D08A29 candidate ready for playtest

Two-handed run-ready upper loops 18/19 were missing from shortcut eligibility
and equipped-weapon history. Added them using the original native switch path,
with a guard for the verified handler. Build, controls/input/aim suites and
bounded 60/120 live checks pass; existing unsafe-state gates remain. See
[note 270](270-d08a29-running-weapon-selection.md) for evidence,
wall-stop limitations and inconclusive live remote switch-out coverage.
D08A29 is Needs playtest. No commit/push or next job started.

Launch `python3 recomp/tools/local/run.py`. Candidate binary
`01dfa5416a79dc5858c5665786216010b6abc4b2824f15085c88d592636a2233`.

## 2026-10-26 - D36H1 accepted and closed; new backlog

D36H1 is Done. User accepted the grip and authorized lock-in/closeout, including
implementation and documentation commit/push. Accepted binary:
`8783c0b34f51f73255a0dcf5192dbff00ccd392ad0906009b1b86dd47ffe51a3`.
[Note 268](268-d36h1-gatling-grip.md) preserves test evidence and coverage limits.
New Todo jobs: D08A29 wheel weapon selection while running, D36H3 restrained
running weapon/hand motion (keys 3/4/7/8/0), D35J5 frozen first-person blue vision.
[Note 269](269-running-weapons-and-frozen-view-intake.md) records the authorized
small source investigation; no live reproduction or implementation of these jobs.
Earlier D36H1 candidate entries are historical. No new build or game launch.
Launch `python3 recomp/tools/local/run.py`. No next implementation started.


## 2026-10-26 - D36H1 Gatling underside grip candidate

D36H1 is Needs playtest. A small display-only support-hand displacement closes
the idle gap without moving the weapon/right hand. Build and native controls,
aim and near suites pass; private 60/120 captures cover base/upgrade, crouch,
look, firing/return, switch/load, flight and swimming. Actual view toggle checked
in the swimming harness. See [note 268](268-d36h1-gatling-grip.md) for limits.
Candidate: `8783c0b34f51f73255a0dcf5192dbff00ccd392ad0906009b1b86dd47ffe51a3`.
Player files preserved; diagnostic runs stopped. No commit/push or next job.
Launch `python3 recomp/tools/local/run.py`; first person, key 4, judge idle
underside contact and fire/return. D36H2 remains Todo.


## 2026-10-26 - D17Z and D17AA accepted and closed

D17Z and D17AA are Done. The user accepted both fixes and explicitly requested
lock-in and closeout. D19Q3 is Done per the earlier completion correction.
Accepted executable: `9342a18490c3b9304371f3f2d2d3cabce077f000a13465f326b1392ebfa9eb32`.
Native UV mapping and scenery ordering fixes, regression tests and documentation
are included in the authorized commit/push. See [note 266](266-d17z-medieval-wall-textures.md)
and [note 267](267-d17aa-wall-block-ordering.md) for diagnostics and limitations.
The user accepted the gameplay result; the scripted push/pull attempt itself
remains inconclusive. Earlier candidate/pending-closeout entries are historical.
No next job started. Launch `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D17Z accepted; D17AA wall tear candidate

User accepted D17Z as working perfectly; its closeout remains pending explicit
instruction. D17AA was then created and implemented at the user's request.
The updated slot-12 block side was sorting over a native wall during approach.
Safe, opaque, noncompact scenery now retains native face ordering while keeping
host clipping/depth. Private 60/120 approach, retreat and turn checks pass, as do
native packet regressions, sky and repository checks. Original-texture packet
output is unchanged. D17AA is Needs playtest; actual push/pull remains unverified.
See [note 267](267-d17aa-wall-block-ordering.md). Candidate executable:
`9342a18490c3b9304371f3f2d2d3cabce077f000a13465f326b1392ebfa9eb32`.
Launch `python3 recomp/tools/local/run.py`, load slot 12 and step forward, then
approach/retreat past the block. Player files preserved; no commit/push.
The older candidate entries below are historical. D19Q3 remains Done.


## 2026-10-26 - D17Z candidate ready for corridor review

D17Z is Needs playtest. Corrected-texture world quads retain the native 1-2
triangle split before near clipping, avoiding wall UV remapping at takeover.
Build/native regression and private 60/120 corridor checks pass; Original-texture
packet output and matching Original/Vanilla captures are unchanged. See
[note 266](266-d17z-medieval-wall-textures.md) for evidence and coverage limits.
Candidate: `0d8950aca24488efa223b7238f169762409180564e5c714fbb739d7b691093cd`.
Launch `python3 recomp/tools/local/run.py`, load current slot 12 and walk/back/turn
through the medieval corridor. Compare `research/screencaps/medieval-glitch-wall.mp4`.
Player files preserved. No framework edits, commit/push or next job. D19Q3 is Done
on the user's explicit completion correction below.

## 2026-10-26 - D19Q3 completion correction

D19Q3 is Done. User confirmed it was already completed and requested the board
correction. Earlier Needs playtest entries are historical. No new validation
or build; D17Z remains active. No commit/push requested.

## 2026-10-26 - D08A28 accepted; native mesh style foundation

D08A28 is Done. User accepted revision 5 ("This is the one") and explicitly
requested documentation and closeout. [Style guide 265](265-native-mesh-art-style.md)
is the required starting point for future custom mesh art; [note 264](264-d08a28-biomask-ps1-authenticity.md)
records the revision history and bounded validation. Accepted source:
`recomp/assets/models/biomask/ps1/`; 184 triangles / 90 emitted, effective 32x16
art regions, 48 source RGB colours, dark five-sided tubing and Buffalo-barrel
steel gradients. Both exporters need explicit `--asset-dir` for this revision.
Original high-detail source remains restorable. Current executable SHA256:
`93cf0b9bbe3c88cd26ab3ec131ac5dd3923698feacef39a5c477bcf51a526c89`.
Build/native and private Buffalo comparison checks pass; earlier lifecycle,
lighting, save-load, software and Vanilla evidence retains its recorded limits.
No other job started. D36H1/D36H2/D17Z remain Todo. For the eventual D17Z fix,
remind the user of `research/screencaps/medieval-glitch-wall.mp4` for comparison.

## 2026-10-26 - D08A28 dark pump tubing and painted steel

User requested darker Buffalo-pump-style tubing and stronger Flamethrower-style
metallic textures. Current texture pass uses the pump's four near-black greys,
narrow painted groove highlights, stepped steel bands, dark recesses and scuffs.
Exported geometry/UVs/face colours are byte-identical to the preceding five-sided
plain-tube revision. Slot-3 allocation fix retained. Build/near suite, private
120 load/warm/cross-level restoration, 60 weapon comparisons, native lighting
and software smoke pass. [Note 264](264-d08a28-biomask-ps1-authenticity.md) has evidence.
Candidate: `bf1f526dfe23c6feb5847dbab6313ee16b89a5a23eb24d1492195c8c746067cd`.
Needs playtest. Original asset and player files preserved; diagnostic games stopped.
No commit/push. Launch `python3 recomp/tools/local/run.py`; first person, B mask.

## 2026-10-26 - D08A28 plain five-sided tube and texture revision

User rejected the ribbed/flat-grey demake. Current candidate has plain pentagonal
constant-radius tubes with no geometric ridges, Buffalo-grip-inspired painted
bands, and Flamethrower-inspired metal edge/recess detail. 184 source / 90 emitted
triangles; existing 256x128 runtime atlas, original coarse texture art. Slot-3
startup-allocation fix retained. [Note 264](264-d08a28-biomask-ps1-authenticity.md)
records the revised art and tests. Build/native/save recovery, private slot-3
warm/cross-level reloads at 120, reference captures at 60, lighting and software/
Vanilla smokes pass. Original source and player files preserved.
Candidate: `f65e677e517829f5af253c71bc383eff4112e56a4734e1894f10e22cd68f3ad7`.
Needs playtest; no commit/push. Launch `python3 recomp/tools/local/run.py`, P first
person, B to equip a charged mask. D36H1/H2/D17Z remain Todo.

## 2026-10-26 - D08A28 missing mask on slot-3 load fixed

The user saw no mask. Reproduced with a private copy of current UI slot 3:
active mask and valid draw gate, but late DMA allocation failed after the saved
allocation tail had filled the aperture. Mask packets now reserve their existing
buffer at startup before loads. Old animation layouts still use D37A recovery;
GPU clearing and player saves remain unchanged. Native suites and new startup/
snapshot regressions pass; private slot-3 and 120-setting warm/cross-level reloads
show the revised mask with zero replay failures. [Note 264](264-d08a28-biomask-ps1-authenticity.md)
records the missed coverage and correction. Candidate:
`2c86ce64a4ddfa42a6d7fa7d45aedef342c105b9ccb0480ac07e84dc6cbe549e`.
Needs playtest; no commit/push. Launch `python3 recomp/tools/local/run.py`, load
slot 3, first person and B to equip. No save recreation needed.

## 2026-10-26 - D08A28 PS1 biomask revision ready

D08A28 is Needs playtest. The user selected implementation with "lets do it,
lets go!". Separate Blender revision has 488 triangles (was 2400), five broad
six-sided hose ribs, coarse original texture art and balanced UV density.
Accepted model/atlas/generator remain intact, with rollback commands in the
revision README. [Note 264](264-d08a28-biomask-ps1-authenticity.md) records original
weapon measurements, comparisons and validation limits. Build, native suites,
60/120 private lifecycle, gas lighting, software/Vanilla and save recovery pass.
Candidate: `666587f7aef9cc24ffa9c404352e27dc2e0096dd1d602a4c0595b4cb08db31ce`.
Player files preserved; diagnostic games stopped; existing framework untouched.
Human artistic acceptance remains. No closeout, commit or push. D36H1/H2/D17Z
remain Todo. Launch: `python3 recomp/tools/local/run.py`; P first person, B mask.

## 2026-10-26 - Bio Mask PS1 authenticity job scoped

D08A28 is Todo. [Note 264](264-d08a28-biomask-ps1-authenticity.md) records the
user's full demake direction and initial asset/pipeline inspection: preserve the
accepted model, study Energy/Flamethrower/Buffalo references, then simplify geometry
and match apparent texel density through the existing Blender workflow. In-game
comparisons and lighting/runtime regression evidence are required for acceptance.
Implementation selection pending clarification. No model/texture/runtime edits,
build, Blender run, game launch, commit or push. D36H1/H2/D17Z remain Todo.

## 2026-10-26 - Three visual follow-ups researched, implementation unstarted

D36H1 (Gatling underside support grip), D36H2 (Buffalo grip-area flicker) and
D17Z (current slot-12 medieval corridor wall glitch) are Todo. User requested
research and backlog creation only, explicitly no implementation. [Note 263](263-weapon-grips-and-medieval-wall-intake.md)
records inspected source, accepted baselines, reviewed user-video samples,
uncertainties and separate acceptance/regression plans. No new live reproduction
or root-cause proof. No code changes, build, launch, player-file writes, commit
or push; existing framework modifications preserved. D36L remains unselected.
When D17Z is fixed, remind the user in the playtest handoff and closeout that
`research/screencaps/medieval-glitch-wall.mp4` remains available for comparison.
Reproduction route: private copy of current UI slot 12, walk down the corridor.

## 2026-10-26 - D36K accepted and closed

User confirmed "fixed, lock it in, and close it out." D36K is Done. The
traversal/special-action coverage and the slot 12 swimming backpack correction
are accepted. Only the mapped backpack joint is hidden during the effective
first-person swimming draw; native motion and third-person presentation remain.
[Note 262](262-d36k-traversal-presentation-coverage.md) preserves the coverage,
regression evidence and bounded limits. Accepted executable:
`ba5f474f1af044de08ca9e71691d6e32eb766d4b86ae5dfc974814377f7de803`.
Implementation, tests and documentation closeout, commit and push are authorized
as MusicMonsterMod. Existing framework edits remain untouched. No rebuild,
game launch or player-save/settings writes during closeout. D36L remains
unselected; D19Q3 remains Needs playtest. No next job started.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-26 - D36K swimming backpack correction ready

D36K is Needs playtest for the user's slot 12 retest. The sole reported critique
in `backpack.mp4` reproduced during submerged up/down movement with downward
look. The candidate hides only the mapped backpack joint during the effective
first-person swimming draw; third person, reflection, flight, movement and
weapon presentation retain their existing behavior. [Note 262](262-d36k-traversal-presentation-coverage.md)
records source ownership, the baseline/candidate comparison, native regressions,
60/120-setting private runs, view switching and save/load. Build and tests pass;
120 replay recorded zero failures. All 42 intake player-save/card/settings/patch
files retain hashes/mtimes. Diagnostic games stopped; framework edits untouched.
Candidate: `ba5f474f1af044de08ca9e71691d6e32eb766d4b86ae5dfc974814377f7de803`.
No acceptance closeout, commit or push; D36L remains unselected.
Launch: `python3 recomp/tools/local/run.py`; load slot 12, P first person,
dive and move up/down while looking down.

## 2026-10-26 - D36K coverage ready for visual review

D36K is Needs playtest for human motion/appearance acceptance. The selected
traversal/special-action coverage pass found no new demonstrated presentation
defect requiring a gameplay change. Accepted D36J executable remains
`1f4ee0da21ddc0e5659a17dfaad8d80b8ccf0a9b1cf86dd681e1fd061e11b6d2`.
[Note 262](262-d36k-traversal-presentation-coverage.md) records natural 60/120
routes, both action choices/global Third Person, 240-setting stress, Vanilla
smokes, native use, swimming/water exit, flight, gas/gadgets and death/Continue.
Failed short-tap, wrong-save and premature menu attempts are explicitly excluded;
rare selector/script/costume cases remain named. Native controls/input/aim/near
and D37A save-restoration tests pass. All 43 intake files retain hashes/mtimes;
all diagnostic games stopped. Existing framework edits are untouched.
No gameplay/asset/save-format/player-setting changes, player-binary rebuild,
acceptance closeout, commit or push. D36L remains unselected; D19Q3 remains
Needs playtest. No next job started.
Launch: `python3 recomp/tools/local/run.py`; P changes view. Review traversal
and special-action motion using the existing saved preferences.


## 2026-10-26 - D36J accepted and closed

User: "lock it in and close us out". D36J is Done. The accepted result uses
Duke's actual plain left leg for Q, without a holster; door 116 retains its
separate right-leg presentation. Native body clipping/fade corrections and
unchanged kick gameplay are included. [Note 261](261-d36j-body-and-kick.md)
preserves the revision history, verification and bounded coverage limits.
Accepted executable: `1f4ee0da21ddc0e5659a17dfaad8d80b8ccf0a9b1cf86dd681e1fd061e11b6d2`.
Implementation, tests and documentation commit/push are authorized as
MusicMonsterMod. D37A is Done by separate full user playtest acceptance;
D19Q3 remains Needs playtest. D36K/L remain unstarted. No next job started.
No build, launch or player-file writes during closeout; existing framework
changes remain untouched. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-26 - D36J revision 2: plain left-leg Q (Needs playtest)

User clarified that Q should use the plain left leg, without the right leg's
holster. The prior candidate's right-leg choice is rejected and superseded.
Q now uses actual costume-specific left-leg parts 1-4, retargeted to the
existing display motion. Right leg/attachment stay native during Q; door 116
keeps its separate right-leg presentation. Body clipping/fade and kick
damage/timing are unchanged. Build and full controls suite pass; reviewed
Boot/pistol/Gatling/RPG captures show the plain leg. Note 261 records revision
evidence. User visual acceptance remains; no commit/push requested.
Candidate: `1f4ee0da21ddc0e5659a17dfaad8d80b8ccf0a9b1cf86dd681e1fd061e11b6d2`.
Launch: `python3 recomp/tools/local/run.py`; P first person, Q kick.

## 2026-10-26 - D36J body/kick candidate (Needs playtest)

Native locomotion legs now reach safe polygon clipping and retain world depth;
the close-camera fade is corrected without removing intentional effects.
The kick thigh retains natural orientation, with part 21 attached to its
moving hip. Q damage/timing, door events, accepted guns, save layout and Vanilla
remain unchanged. Build, controls/input/aim/near suites and D37A recovery pass.
Private 60/120-setting captures, physical P switching, save/load and bounded
cost/replay evidence are in [note 261](261-d36j-body-and-kick.md). User visual
review and the named broader coverage remain. D36K/L are unstarted.
D37A is Done on the user's full playtest acceptance; D19Q3 remains Needs playtest.
Candidate: `9186ed9531217dcedaeb9da3c410290649609565d33650e0a250403602563a61`.
No commit/push or D36J acceptance closeout requested. Player files preserved.
Launch: `python3 recomp/tools/local/run.py`; P first person, Q kick.

## 2026-10-26 - D37A fully accepted; D36J selected

User confirms D37A was playtested and fully accepts it. D37A is Done; note 255
preserves the existing verification and limits. D19Q3 remains Needs playtest.
D36J body/quick-kick presentation is selected and In progress under note 257.
D36K/L remain unstarted. No commit or push requested.

## 2026-10-26 - D36I accepted and closed

User confirms the arms are solid, native animation is unaffected and the result
is complete and foundational, and explicitly requests lock-in/closeout. D36I
is Done. Authored implementation, tests and documentation are authorized for
commit/push as MusicMonsterMod. [Note 260](260-d36i-interaction-arm-continuity.md)
records acceptance and preserves the bounded verification and coverage limits.
Accepted executable: `e6e6922f51217d2383d56458b31dfd2784952a4c192b6d62cf54e6b9641565ea`.
No gameplay changes, rebuild, game launch or player-file writes during closeout.
Existing framework changes remain untouched. D36J-L remain unstarted; D37A/D19Q3
retain their separate Needs playtest statuses. No next job started.
Launch: `python3 recomp/tools/local/run.py` (Modernized default; `--vanilla`
selects original play for one launch).


## 2026-10-26 - D36I interaction arm candidate (Needs playtest)

Push/Use native arms now bypass whole-joint origin rejection through the
existing guarded clipping bridge. Real matrices, native contact motion, world
occlusion and lighting are retained; D36H weapon/held-item ownership remains.
Build, controls/input/aim/near suites and D37A save-recovery checks pass. Private
slot-8 motion/depth evidence plus wheel/vending/item/ladder and save/load routes
are recorded in [note 260](260-d36i-interaction-arm-continuity.md), including
failed diagnostic attempts and bounded 60/120-setting coverage. No framework,
allocation or replay/save format changes. User motion/contact review remains.
D36J-L are unstarted; D37A/D19Q3 retain their separate Needs playtest statuses.
No acceptance closeout, commit or push authorized.
Candidate: `e6e6922f51217d2383d56458b31dfd2784952a4c192b6d62cf54e6b9641565ea`.
Launch: `python3 recomp/tools/local/run.py`. Capture is automatic; P selects
first person. Check slot 8 grab/push/reverse/release, then wheel and vending.


## 2026-10-26 - D36H accepted and closed

User: "fully accept! lock it in and close it out!!!!!!". D36H is Done, including
connected combat/held-item arms and the final Buffalo Rifle cocking motion.
Implementation, tests and documentation closeout are authorized for commit/push
as MusicMonsterMod. [Note 259](259-d36h-connected-arms.md) records acceptance,
passing controls/aim/clipping checks, private motion/save-load evidence and
bounded coverage. Earlier candidate entries below are historical; the first
Buffalo correction remains rejected and superseded.
Accepted executable: `db2e95b17b2faff913f4ba2893bb9d1272c6dfa3fc4d2e8c7d007de286541517`.
No gameplay changes, rebuild, game launch or player-file writes during closeout.
D36I-L remain unstarted; D37A/D19Q3 retain their separate Needs playtest statuses.
Launch: `python3 recomp/tools/local/run.py` (Modernized default; `--vanilla`
selects the original profile for one launch).


## 2026-10-26 - D36H Buffalo motion revision (Needs playtest)

The user's `research/screencaps/buffalo.mp4` rejects the previous rifle follow-up:
the gun rolls down while the support hand rises, with a reported vibrating
settle. That earlier candidate is superseded, not visually accepted.
The new display-only pose lifts the rifle and keeps the support hand beside it
for a short cocking stroke, driven by native upper-27 clip timing. Rifle pose
and recoil are interpolated in replay; the steady support grip no longer follows
independent native wrist motion. Other weapon records and Vanilla retain their
existing paths. Build and controls/aim/near tests pass. Private 60/120-setting
motion recordings show the raised cycle and stable inspected settling frames;
these are visual diagnostics, not a 120 FPS performance certification.
[Note 259](259-d36h-connected-arms.md) records evidence and remaining limits.
User visual review remains; no closeout, commit or push authorized.
Candidate: `db2e95b17b2faff913f4ba2893bb9d1272c6dfa3fc4d2e8c7d007de286541517`.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H Buffalo Rifle reload correction (superseded after video review)

User: arms are "99.9% perfect" except key 9's Buffalo Rifle stays fixed while
its hand cocks/reloads. The rifle now follows native upper-27 right-arm motion
around the accepted firing grip; idle and other weapons retain their policy.
Build and controls/aim/near checks pass, plus private standing/crouched cycles
and 120 Hz mid-reload save/load. [Note 259](259-d36h-connected-arms.md) records
the source finding, implementation and limits. Retest key 9 firing/cocking.
D36H remains Needs playtest; no closeout, commit or push authorized.
Candidate: `4cd7269e8251f99bcfb2d1ceb8c5f4b2c0d46449d61add7edbc2708b3df5afac`.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D36H connected arms (Needs playtest)

D36H candidate adds connected retail combat/held-item arms around the accepted
weapon grip, with native per-weapon support grips for two-handed weapons. Native
interaction arms remain under their existing owners. Build and controls/input/
aim/near/save-recovery checks pass; private captures verify all base records,
six campaign upgrades, held items, swimming/flight, view toggles and save/load
with the limits in [note 259](259-d36h-connected-arms.md). No replacement asset,
new guest allocation or framework edit. User visual grip/pose and continuous
motion approval remains. D36I-L unstarted; D37A/D19Q3 remain Needs playtest.
No acceptance closeout, commit or push authorized.
Candidate: `9bffb4f704fbf22c02d0b203c6995c067f18f6e237b0fdd66462b9251f1da806`.
Launch: `python3 recomp/tools/local/run.py` (automatic capture; P changes view).


## 2026-10-26 - D36G accepted and closed

User: "lock it in and close it out". D36G is Done by user acceptance.
Implementation, regression tests and documentation closeout are authorized for
commit/push as MusicMonsterMod. [Note 258](258-d36g-limb-ownership.md) records the
shared limb ownership contract, passing checks and bounded capture evidence.
Accepted executable: `e2062e66f70eb0bbe20be9c8a7bb8bfbf9e5cf234107779d4e98dcd3c979f1d3`.
No new build, game launch or player-file writes during closeout. Existing framework
modifications remain untouched. D36H-L remain unstarted; D37A/D19Q3 retain their
separate Needs playtest statuses. User chooses the next job.
Launch: `python3 recomp/tools/local/run.py` (automatic capture; P changes view).


## 2026-10-26 - D36G shared limb ownership (Needs playtest)

User selected D36G. A shared draw-local descriptor now names limb owner, semantic
part, transform space, depth, lighting and cleanup around the existing hooks.
Existing weapon/kick/native interaction visuals and traversal-only depth bridge
are preserved. Interrupted actor/reflection boundaries and replay/load token
retirement have native tests. No new guest allocations, save/replay fields,
assets, preferences or framework edits. D36H-L remain unstarted.

Build and controls/input/aim/near suites pass; D37A save recovery checks pass.
Private 60/120 runs preserve sampled push/pull, vending, ladder, weapon and kick
presentation. Native tests cover third-person/action choices, Vanilla and lifecycle;
these are bounded checks, not whole-campaign visual certification. Player files
are unchanged and private processes stopped. See [note 258](258-d36g-limb-ownership.md)
for captures, evidence limits and the playtest checklist. D37A/D19Q3 remain Needs
playtest. No acceptance closeout, commit or push authorized.
Candidate `e2062e66f70eb0bbe20be9c8a7bb8bfbf9e5cf234107779d4e98dcd3c979f1d3`.
Launch: `python3 recomp/tools/local/run.py` (automatic capture; P changes view).


## 2026-10-26 - D36 research accepted and closed

User explicitly requested lock-in and closeout of the research, identifying it
as integral to future jobs. D36 and D36A-F are Done. The accepted
[note 257](257-d36-first-person-presentation-audit.md) is the reference for future
first-person body/arms, interactions, clipping, kick and presentation-restoration
work. Read its source catalogue, evidence labels, traceability matrix and
regression contracts before implementing related changes. Existing uncertainty
and coverage limits remain; acceptance does not turn hypotheses into findings.

D36G-L remain report-only implementation proposals, not selected or started.
Suggested order remains shared ownership, slot-8 interaction continuity,
combat/item arms, body/kick and remaining visual coverage. User chooses the next
job. D37A/D19Q3 remain Needs playtest; player build is unchanged at
`736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
Documentation commit/push closeout is authorized as MusicMonsterMod. No gameplay,
asset or framework edits, build, launch or player-file writes during closeout.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-26 - D36 presentation audit ready for review

D36 and D36A-F are Needs review. [Note 257](257-d36-first-person-presentation-audit.md)
contains the state/source catalogue, geometry and weapon matrices, current private
interaction evidence, body/kick diagnosis, architecture options, traceability and
proposed D36G-L implementation scopes. Those IDs are report-only proposals, not
selected or started jobs. Suggested order: shared ownership contract, slot-8
interaction continuity, combat/item arms, body/kick, remaining visual coverage.

Source confirms hidden combat/item arm parts, traversal-only joint-depth bridging
and an end-for-end kick-thigh workaround. Current captures and per-part depth
samples support the slot-8 and walking-leg culling diagnosis. A literal third leg,
every costume/interaction and all interruption/high-refresh combinations are not
certified. All base weapon identities and six campaign upgrade flags were checked;
Crossbow's new check is idle only. Prior functional acceptance is preserved.

No gameplay/assets/framework changes or build. All private diagnostic processes
stopped; player saves/cards/settings and executable hashes/mtimes match intake.
Current build remains `736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
D37A/D19Q3 remain Needs playtest. No research acceptance, closeout, commit or push
is implied. Review the audit before selecting implementation work.
Launch: `python3 recomp/tools/local/run.py` (Modernized default, automatic capture).

## 2026-10-26 - D19U/D19U1 HUD shortcut backlog only

Added D19U Todo: live `-` hides status bars (existing OFF), `+` shows them (ON).
Future child D19U1 Todo adds the intermediate borderless presentation: minus steps
Full -> Borderless -> Hidden; plus reverses it. Borderless retains status contents
while removing all borders. Both remain unstarted; no source edits/build/launch.
User authorized commit/push of backlog updates, including pending D07G crosshair
scaling documentation. D19Q4 and other backlog jobs remain unstarted.


## 2026-10-26 - D07G crosshair scaling backlog only

Added D07G Todo for the user's very small crosshair in windowed mode. Completed
D08A22 (switcher/card scale from fitted game picture) is the primary prior-work
reference; its art-specific scale factor is not assumed to fit the crosshair.
See MODERNIZATION_JOBS.md for scope and acceptance. No source changes, build,
launch or investigation of the crosshair. D19Q4 also remains unstarted.
No commit/push requested for this new intake.


## 2026-10-26 - Session closeout; D19Q4 backlog only

User authorized commit/push and session closeout. Added D19Q4 Todo for intermittent
overwrite-confirmation text (wording paraphrased) and [ / ] inventory-selector
flashes on loading. No investigation, implementation, build or launch for D19Q4.
See MODERNIZATION_JOBS.md for the scoped future work. No shared cause established.
D37A and D19Q3 changes/tests/reports are included in closeout; their Needs playtest
statuses remain without asserting additional user acceptance. D37 research is Done.
D36 research remains Todo. No next job started.
Current build: `736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.


## 2026-10-26 - D19Q3 automatic load resume (Needs playtest)

User reports D37A looks good, then requests that loaded games never start paused.
D19Q3 supersedes D19Q2's intentional Modernized pause preservation. Every applied
paused gameplay load now runs the guarded resume path, without relying on sidecars
or which loader was used. [Note 256](256-d19q3-always-resume.md).
Real F9, F3 LOAD GAME, F7, movement and later Escape pause/resume pass in private
Xvfb. Direct/repeated slot 9, metadata-free paused saves and running saves pass;
statistics and Vanilla preservation pass. Manual updated; player files preserved.
Current candidate: `736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
Includes D37A repair. Neither repair has acceptance closeout/commit/push authorization.
Launch: `python3 recomp/tools/local/run.py`. No extra activation step.


## 2026-10-26 - D37 closed; D37A repair ready for playtest

User authorized research closeout and implementation of the linked repair. D37 is
Done. D37A is Needs playtest: [note 255](255-d37a-animation-save-restore.md).
Partial save restore now recovers verified animation metadata, restores native
tables and rebuilds current overrides without undoing GPU packet clearing.
Private warm/repeat/reverse-order/cross-level loads and first/third-person movement
passed; crouch and first-person kick counters advance. Vanilla new-game movement,
native recovery/GPU clearing regressions and 15 animation pipeline tests passed.
Candidate: `b94b58e4be7cbeda84b60494d3a9e814c08b4160bfcea6feba0c9ea96040a738`.
Framework patch exported/applied to clean pinned source and byte-compared.
Player files preserved. No repair acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. Gameplay capture is automatic.


## 2026-10-26 - D37 movement failure research (Needs review)

[Note 254](254-d37-movement-failure.md) records a reproduced save-load animation
memory failure: new-game -> UI slot 9 gives partial restore and clears the animation
header while guest globals keep pointing at it. After resume, WASD cannot move
horizontally but mouse yaw and frame counts advance. Original pre-restart logs
show partial restores; restart logs show full restores and animation application.
Historical atop/journal/backup checks do not support sustained host contention.
Exact original live state was not captured. Research only, no fix implemented.
Accepted binary unchanged; private test settings/cards and copied savestates only.
No acceptance closeout, commit, push or next job.


## 2026-10-26 - D36/D37 backlog intake only

Registered D36 comprehensive first-person body/arms/interaction quality research,
with D36A-F research tasks for the existing-system catalogue, weapon arms,
body/quick kick, interactions, architecture alternatives and final roadmap.
Preserved all user reproduction references and research/regression boundaries in
MODERNIZATION_JOBS.md. D37 separately records the approximately 07:26 UK movement
failure and requires historical log/host correlation without assuming a visual bug.

All new jobs are Todo, not started. User permits in-game investigation while
scoping; no launch or investigation was needed for this intake. No source/model/
asset changes, build, player-file access or completed research claims. Existing
accepted first-person jobs remain closed. Future report numbers will be allocated
when research is selected, avoiding collisions. No implementation job is started
or authorized by this intake; future fixes require the researched roadmap and
explicit selection. No commit or push requested for this intake.


## 2026-10-26 - D23D accepted and closed

User explicitly requested: "close out D23D", reporting that attempts to reproduce
it found "absolutely no problems with the playthrough". D23D is Done on user
playtest evidence. The original Level 9 strip-club-area slowdown report is retired;
no residual slowdown was reported. This does not establish its historical cause
or prove attribution to an earlier optimization. D23C's brief moat water-entry
observation remains separate and unchanged.

Documentation-only closeout; no new automated profiling, gameplay/framework edits,
build or game launch. Accepted executable remains
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
Existing framework changes and dependency pins preserved; player files untouched.
Reopen on a reproducible report and correlate host activity with gameplay telemetry
before optimizing. No next job started.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D23C accepted and closed

User approved closeout. D23C is Done: the original sustained moat/Necro slowdown
no longer reproduces in current user playtest and the bounded private recheck.
The brief water-entry game-image cadence dip remains recorded, cause unresolved.
[Note 253](253-d23c-moat-recheck.md) and its measurement summary preserve the
acceptance limits. No optimization, rebuild or post-acceptance launch.
Accepted executable remains
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
Documentation/evidence closeout authorized. Existing framework edits and pins
preserved; player files untouched. No next job started.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D23C moat recheck ready for review

The user no longer meaningfully reproduces the old slowdown. Three private
slot-2 water routes and a fresh level-6 check hold approximately 120 presents/s
and normal emulation speed. A brief underlying game-image cadence dip remains
near water entry (about 20 to 15-16/s, then recovers), with no identified cause.
No sustained slowdown or host contention observed. Recommend retiring the original
report on acceptance; D23C is Needs review, not closed. [Note 253](253-d23c-moat-recheck.md).
No gameplay/framework edits or rebuild; accepted executable unchanged. Player
files preserved, diagnostics stopped. No commit/push or next job started.


## 2026-10-26 - D23 accepted and closed

User accepted the findings and explicitly authorized marking D23 complete
and closing it out. D23 is Done. [Note 252](252-d23-performance-baseline.md),
its chart/JSON and the authored audit/summarize tools preserve the bounded
performance baseline and measurement limits. No new gameplay changes, builds
or launches were needed for closeout.

Accepted executable remains
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
No framework edits for D23; existing local framework changes and dependency
pins are preserved. Player files and private evidence remain local. D23C's
D23 prerequisite is satisfied; D23C remains Todo and has not started.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-26 - D23 performance baseline ready for review

D23 is Needs review. [Note 252](252-d23-performance-baseline.md) and its linked
chart/JSON document two roughly 15-minute private offscreen audits, each with
nine level loads and ten minutes of sustained gameplay. Modernized: 120.000
presents/s, p99 8.960 ms; both profiles: 59.941 emulated fields/s, zero warmed
output underruns, no hang, clean shutdown under 0.08 s. Loads: 5.98-8.08 s.

The raw memory-growth threshold triggered follow-up: newly resident pages
were predominantly fixed-size SIO/audio/other diagnostic histories, not an
established unbounded leak. Dummy-audio overflow was absent in separate
Pulse/PipeWire checks; post-routing steady windows were clean. SDL initially
ignored PULSE_SINK, so the test stream was explicitly moved to the private
sink. Sink removed and default output verified unchanged. See the note for
exact attribution, desktop/YouTube conditions and measurement limitations.

Accepted executable remains
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
All 91 recorded player files unchanged. No game/framework edits, rebuild,
commit or push. Repeat helpers: `tools/local/performance_audit.py` and
`tools/local/summarize_performance_audit.py` under recomp. No next job started;
D23C remains the separate targeted moat/Necro investigation after review.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q9 accepted and closed

User accepted jetpack walk/run and steroid pace and explicitly authorized
lock-in/closeout. D08Q9 is Done. Modern/EDuke32 horizontal flight follows the
shared ground pace and existing Shift/Caps Lock choice, with active steroids.
Classic and Vanilla retain their behavior. [Note 251](251-d08q9-flight-pace.md)
records native tests, private gameplay routes and measurement limits.

Accepted executable: `9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
No post-acceptance gameplay edits, builds or launches. Runtime patch re-export
unchanged; all 52 authored framework files reconstruct against the clean pin.
Player data and dependency pins untouched. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q9 flight walk/run pace (Needs playtest)

User requested a dedicated job and immediate implementation. Modern/EDuke32
horizontal flight now uses the shared land walk/run target, the existing
Shift/Caps Lock choice, and the native steroid speed proportion while active.
Horizontal root resolution removes native thrust/animation timing error before
collision. Classic keeps native burst physics; Vanilla bypasses the adapters.

Build, native controls with supplied level fixtures, EDuke32 fuel/toggle,
input and aim checks pass. Final private Modern/60/third-person and
EDuke32/120/first-person routes pass flight reload, directional movement,
steroids, live Shift release, hover, descent and J-off. Short-window travel
measurements vary; user feel acceptance remains pending. See
[note 251](251-d08q9-flight-pace.md). No framework edits or pin changes.

Candidate: `9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
Player settings/cards/saves untouched; diagnostic processes stopped. No commit
or push authorized. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q8 accepted and closed

User accepted the ordinary-flight surface following revision and explicitly
authorized lock-in/closeout. D08Q8 is Done. Modern/EDuke32 follow confirmed
floor/ceiling slopes without held Space/Ctrl, with edge release and blocked-wall
handling. Classic retains native thrust/landing; Vanilla is unchanged.
[Note 250](250-d08q8-flight-clearance.md) records tests, private routes and
coverage limits.

Accepted executable: `b259c3ed80956758203ddd2e7ef7d6c334e6cd4718d6aab971d152c2fb7c71cb`.
No post-acceptance gameplay edits, builds or launches. Runtime patch re-export
unchanged; all 52 authored framework files reconstruct against the pinned clean
dependency. Player data and dependency pins untouched. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q8 ordinary-flight surface following (Needs playtest)

User found that holding Space smoothed contact, while ordinary horizontal
flight still juddered. Modern/EDuke32 now follow a contacted slope continuously
without Space/Ctrl, revalidating the plane and route each update. Following
releases at edges; blocked horizontal motion cannot carry its vertical tangent.
Fractional height carry prevents rounding drift. Classic retains its native
thrust and landing; Vanilla is unchanged.

Native controls, input, aim and fuel tests pass. Private no-thrust slot-12
routes cover Modern/third-person and EDuke32/first-person; outdoor, firing and
flight save/load checks are recorded with build boundaries in
[note 250](250-d08q8-flight-clearance.md). User feel acceptance remains pending.
No commit/push closeout. Player data untouched.

Candidate: `b259c3ed80956758203ddd2e7ef7d6c334e6cd4718d6aab971d152c2fb7c71cb`.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q7 accepted and closed

User accepted flight fire-to-draw and explicitly authorized lock-in/closeout.
D08Q7 is Done. Held left click draws and fires; a settled short click draws
without a delayed shot. [Note 249](249-d08q7-flight-fire-draw.md) records
implementation, native tests, private three-scheme routes and coverage limits.

Accepted executable: `f3d2f69f4d0e8272d96bd94a52849dcc4dde605905143fd15b605760d6a73de8`.
No post-acceptance gameplay edits, builds or launches. Runtime patch re-export
unchanged; all 52 modified/new files reconstruct against the pinned framework.
Player data and dependency pins untouched. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08Q7 flight fire-draw candidate (Needs playtest)

Left click now draws a settled holstered weapon during authenticated jetpack
flight, using the existing single native Circle tap. Held fire then shoots;
a released short click does not queue a shot. Native restrictions remain.
Build and controls/input/aim suites pass; private Modern, Classic and EDuke32
routes pass. [Note 249](249-d08q7-flight-fire-draw.md) records evidence and the
player checklist. User acceptance pending; no commit/push closeout.

Candidate: `f3d2f69f4d0e8272d96bd94a52849dcc4dde605905143fd15b605760d6a73de8`.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A27 accepted and closed

User accepted the full-width toxic gas fix and explicitly authorized closeout.
D08A27 is Done. Modernized gas now follows the active widescreen viewport;
native gas simulation, Bio Mask behavior, 4:3 and Vanilla remain unchanged.
[Note 248](248-d08a27-widescreen-gas.md) records implementation, private slot-11
captures, native tests, 60/120-target boundary routes and coverage limits.

Accepted executable: `019d10de6a4977d6319f53811227121e8befc2a7b5057294b9ad8b13d65e741c`.
No post-acceptance gameplay changes, builds or launches. The unchanged framework
patch reconstructs all 52 modified/new files against the clean pin. Player data
and dependency pins are untouched. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A26 accepted and closed

User explicitly authorized commit, push and closeout after accepting the mesh
and native-lighting pass. D08A26 is Done: the first original custom mesh is
integrated, with editable Blender/Photon assets and CC0 texture provenance.

Accepted executable: `c775837d26203c05f46d50503f6a657b7ae1b85e01bbb0ec062c1e388fe15e62`.
No post-acceptance gameplay changes, builds or launches.
[Note 247](247-d08a26-bio-mask-mesh.md) records tests and coverage limits.
D08A27 is the user-selected next job: fix the gas's 4:3 cutoff in widescreen,
using a private copy of UI slot 11. This closeout does not implement that job.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A26 lighting revision; D08A27 next

User accepted the mesh visually and requested native PS1 lighting before
closeout. The mask now shares the weapon's environment RGB modulation, with
Duke's current brightness as the no-weapon fallback. Packet/control tests and
private slot-11 120-target lighting/fire/holster/gas/load checks pass. See
[note 247](247-d08a26-bio-mask-mesh.md). Final lighting play acceptance pending.

D08A27 is queued for the toxic gas's 4:3 cutoff in widescreen, with slot 11 as
the test case. The user selected it to begin after D08A26 closeout. Do not lose
this next-job instruction. No commit/push or closeout yet.


## 2026-10-25 - D08A26 Bio Mask mesh candidate (Needs playtest)

The user approved the Blender shape and requested game integration. Modernized
first person now renders the real low-poly respirator with the active native
mask, existing bob and replay timing. Vanilla emits no added mesh.
Editable sources, CC0 texture provenance and exporters are included.

Private lifecycle checks pass at 60 FPS/4:3 and 120-target/16:9; the latter
verified replay correctness but did not sustain 120 presents/s offscreen.
Native packet/control/input/aim tests pass. An animation/DMA allocation conflict
was fixed before final checks. [Note 247](247-d08a26-bio-mask-mesh.md) records
implementation and remaining visual/performance acceptance. No commit or push.

Launch: `python3 recomp/tools/local/run.py`; use P for first person, B for the
owned/charged mask, or enter gas with automatic Bio Mask enabled.


## 2026-10-25 - D08A24 accepted and closed

User: "lock it in, close it out". D08A24 is Done. Automatic gas entry and exit,
instant manual use while firing, and the edited on/off gas cue are accepted.
Implementation, tests, licensed asset/provenance, manual and documentation
closeout/commit/push are authorized as MusicMonsterMod.

Accepted executable: `bc3828d8fc5b2a2cfaf3d82edd92ebda1d382124f4f8a9fafcdbdaddaa1e48b4`.
No post-acceptance gameplay changes, builds or launches. Runtime patch export
is unchanged; all 51 files match the clean patched pinned dependency. Dependency
pins and player data are untouched. [Note 246](246-d08a24-auto-bio-mask.md) retains
verification evidence and coverage limits. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A24 revision 2: automatic mask removal (Needs playtest)

User requested automatic removal after leaving gas. AUTO BIO MASK now removes
an active mask after three consecutive native updates without head exposure,
playing the removal cue once and stopping charge drain. Entry remains instant;
brief boundary dips reset the exit grace. Firing, weapon and camera stay live.
Manual use in clean air remains available. Active gas saves stay protected;
active clean-air saves resolve off after the grace with one removal cue.

Build and native controls suite pass. Private slot-11 routes at 60 FPS third
person and 120 FPS first person pass three repeated crossings during fire,
exact cue counts, stopped charge drain, manual clean-air use and both load cases.
All 40 player source save/card files retain this revision's initial hashes/mtimes.
Diagnostics stopped. No framework, audio asset/mixer or settings-schema changes
in this revision. No acceptance closeout, commit/push or next job.

Candidate: `bc3828d8fc5b2a2cfaf3d82edd92ebda1d382124f4f8a9fafcdbdaddaa1e48b4`.
[Note 246](246-d08a24-auto-bio-mask.md) contains evidence and remaining limits.
Launch: `python3 recomp/tools/local/run.py`; slot 11, walk into and out of gas.


## 2026-10-25 - D08A24 automatic/instant Bio Mask (Needs playtest)

User selected UI slot 11, requested instant use while shooting, and supplied a
licensed gas-decompression cue for activation/removal with editing authorized.
AUTO BIO MASK ON (default) activates at native toxic-area head exposure before
gas damage. B/Enter also toggle instantly while firing; OFF restores native
manual behavior. The mask stays on outside gas, draining normally; manual-off
in gas lasts until exit/re-entry. Loads reset that suppression, preserve native
saved mask/charge, and do not play an activation cue for an active saved mask.

The 0.55-second edited sound follows effects volume, with no queued repeats.
Build, controls/input/aim/audio suites, 104 profile tests, private 60/120 FPS
routes, live menu OFF/ON and host PCM checks pass. Final 120 FPS smoke confirms
the strict gas-area caller guard and cue on automatic activation/manual removal.
All 40 source save/card files retain hashes/mtimes. Private diagnostics stopped;
no player-data, dependency pin or framework changes. Existing framework edits
remain preserved. User control-feel and sound-mix acceptance remain.

Candidate: `e68d5f45dd835ce4281284fb9e5aa06186cf699880a77e92fd2a4b3567bd93e7`.
[Note 246](246-d08a24-auto-bio-mask.md) records implementation, licensed audio,
evidence and limits. No acceptance closeout, commit/push or next job.
Launch: `python3 recomp/tools/local/run.py`; load slot 11, walk into gas while
firing, and try B/Enter. AUTO BIO MASK is under OPTIONS > ADVANCED > ITEMS.


## 2026-10-25 - D08A25 accepted and closed

User: "its perfect! lock it in and close us out". D08A25 is Done. The chosen
0.24-second A cue on both Modern night vision activation and manual removal
is accepted. Implementation, licensed asset/provenance, tests, manual and
documentation are authorized for commit and push as MusicMonsterMod.

Accepted executable:
`a92f2378eb3ed1f9502aee095c22632d463b2eb6be37caca5212a94cc2020313`.
No post-acceptance gameplay changes, builds or launches. Runtime patch export
is unchanged; all 51 files match the clean patched pinned dependency.
Dependency pins and player data are untouched. Existing framework edits are
preserved in the exported patch. [Note 245](245-d08a25-night-vision-sound.md)
retains test evidence and coverage limits. D08A24 remains Todo; no next job
started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A25 removal cue revision (Needs playtest)

User liked A in game and requested the same sound when taking goggles off,
as in Duke 3D. Successful Modern N/Enter toggles now play A in both directions.
Failed/empty activation, held-key repeats, depletion and load/pause remain silent.
Original/Vanilla and the approved asset, gain and audio mixer are unchanged.

Build and controls/audio native tests pass. A private 120 FPS first-person
route verifies on/off through N and Enter, repeated toggles while firing,
held-key suppression and no extra cue on pause or active save/load. Host PCM
captures match A on activation (correlation 0.583) and removal (0.631); both
are absent with effects muted (below 0.002). This is pipeline verification;
the revised in-game listening check remains with the user.

All 40 player source files retain hashes/mtimes; private session stopped.
No new framework edits, asset edits, commit/push or acceptance closeout.
Candidate: `a92f2378eb3ed1f9502aee095c22632d463b2eb6be37caca5212a94cc2020313`.
Launch: `python3 recomp/tools/local/run.py`; press N on and off.


## 2026-10-25 - D08A25 selected A integrated (Needs playtest)

User selected A-tight, the 0.24-second night vision activation cue. The licensed
asset is embedded and plays once after successful Modern N/Enter activation,
including during fire, with native effects volume and host master volume.
No cue on removal, failed/empty activation, depletion, pause or active save/load.
Original/Vanilla remain unchanged. Framework seam mixes host PCM only, with
load/mute cancellation and bounded rapid-toggle restart.

Build, controls/input/aim/audio tests, private 60 FPS third-person and 120 FPS
first-person routes pass. Host PCM captures verify A in the enabled mix and
its absence when effects are muted. Original native equip/removal passes with
zero new cue events; Vanilla exclusion is native-test coverage. All 40 player
source files retain hashes/mtimes. Framework patch applies cleanly and matches
all 51 changed files against the pinned dependency. Diagnostics stopped.

[Note 245](245-d08a25-night-vision-sound.md) records the contracts,
asset provenance, evidence and limits. User in-game listening/mix acceptance
remains. No acceptance closeout, commit or push; no next job started.
Candidate: `0db3051ee6370c402916f059cbc4dee4e7c8388894a964a5c4cb3fece596b48b`.
Launch: `python3 recomp/tools/local/run.py`; N with NIGHT VISION MODERN.


## 2026-10-25 - D08A25 night vision sound options

User requested a new job and audio options first. D08A25 is In progress
(audio options), dependent on completed D08A23. Gameplay integration awaits
sound selection. Local auditions are in `research/d08a25-nightvision-audio/`:
A-tight (0.24 s), B-balanced (0.32 s), C-fuller (0.42 s), and
`ABC-comparison.wav` (each twice, A/B/C order). WAV masters are mono 48 kHz
16-bit PCM; MP3 previews also exist. `make_options.py` reproduces the edits;
`manifest.json` records source hash, provenance, processing and measured levels.

The supplied Pixabay camera-flash recording is the only candidate audio source.
Its rising tonal body is reversed, time-compressed and lowered in pitch, with
a short source transient at the front and shaped decay/fades. The Duke 3D capture
is reference only: the first isolated event is approximately 1.17-1.38 s with
a falling tonal contour. Capture ambience limits exact pitch/envelope matching.
All three files have -21 dBFS RMS, peaks below -6 dBFS, correct durations and
zero-valued endpoint samples. Waveform/spectral verification completed; no
claim of listening acceptance or in-game mix verification. User should audition
and select/refine before implementation. Planned cue is successful Modern
activation only, respecting effects volume and preserving Original/Vanilla.

No gameplay source edits, build, launch, player-data changes, commit or push.
Existing framework modifications preserved. Source/license URL and acceptance
criteria are in the D08A25 backlog entry. This is preparation, not closeout.


## 2026-10-25 - D08A23 accepted and closed

User: "lock it in, close it out!!". D08A23 is Done. Instant night vision with
uninterrupted fire and the persistent MODERN / ORIGINAL setting is accepted.
Implementation, tests, manual and documentation are authorized for commit and
push as MusicMonsterMod. D08A24 remains Todo; no next job is started.

Accepted executable:
`ec32ea578286344faea16de7d073a80dc120ff4c0ca72f62f8d4b7c4afaf9ec6`.
No post-acceptance gameplay changes, builds or launches. Player data and
framework pins are untouched. Runtime patch export is unchanged; all 51 files
match the patched clean dependency. [Note 244](244-d08a23-instant-night-vision.md)
retains the verified coverage and limits. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08A23 instant night vision candidate (Needs playtest)

Modern night vision now toggles immediately with N or inventory Enter, including
while firing, without holstering or taking weapon/movement animation ownership.
TTK's effect and native battery drain/depletion remain. OPTIONS > ADVANCED >
ITEMS > NIGHT VISION offers MODERN (default) / ORIGINAL, applied on CONFIRM;
launcher schema 51, `--night-vision modern|original`, terminal settings N.
Vanilla retains original behavior. D08A24 remains Todo.

Build, controls/input/aim suites and all 102 profile tests pass. Private Xvfb
60 FPS third-person and 120 FPS first-person routes cover held fire, repeated
N and inventory Enter, held-key suppression, movement, depletion, active save/load
and pause/resume. The live menu applies both choices; Original retains native
holstering. Forty player save/card files retain their hashes and mtimes.
User combat-feel/presentation playtest remains; this is not campaign-wide proof.
[Note 244](244-d08a23-instant-night-vision.md) records native contracts and limits.

Candidate: `ec32ea578286344faea16de7d073a80dc120ff4c0ca72f62f8d4b7c4afaf9ec6`.
No framework/generated/pin changes, acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. Modern input captures automatically.


## 2026-10-25 - D08T9 accepted and closed

User: "perfect! lock it in and close it out". D08T9 is Done. Repeat grabs
on the verified LEVEL05 dumpster Z lane are accepted. Implementation, tests,
manual and documentation closeout is authorized as MusicMonsterMod. No next
job is started.

Accepted executable:
`5ea6c549eb2fc1d3cb4ed8c8f14af031927b4b67556dd16ccfc3d4a6212ba5cc`.
No post-acceptance gameplay changes, builds or launches. Player data and
framework pins are untouched. Runtime patch export is unchanged; all 51 files
match the patched clean dependency. [Note 243](243-d08t9-level5-dumpster.md)
retains coverage and limits. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D08T9 level-5 dumpster candidate (Needs playtest)

Private current slot 8 reproduces loss of regrab after longer movement:
Duke crosses onto a floor tile whose original grab permission is clear.
The verified LEVEL05 movable dumpster now has Z-lane acquisition, retaining
original line-up, support and collision checks. The separate fixed dumpster
is unchanged. Build and controls/input/aim suites pass; private 60/120 FPS
routes pass repeated push/pull, release/regrab, boundary save/load, both lane
limits and pause/resume. Alley and fixed-scenery regressions pass. All 40
player source files retain their hashes and mtimes.

Candidate `5ea6c549eb2fc1d3cb4ed8c8f14af031927b4b67556dd16ccfc3d4a6212ba5cc`.
User playtest remains: slot 8, push, release, reapproach, grab and try W/S.
[Note 243](243-d08t9-level5-dumpster.md) records the exact coverage and limits.
No framework/generated/pin changes, acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35G1 accepted and closed

User: "lock it in and close it out!!". D35G1 is Done. Hidden viewmodel arms
stay excluded from the mantle draw path during armed edge step-ups. The fix,
regression test, backlog research and documentation are authorized for commit
and push as MusicMonsterMod. D08A23 instant night vision and D08A24 automatic
Bio Mask remain Todo. No next job is started; D35 remains closed.

Accepted executable:
`58821f8d6191768dde25a04940386ace8d98b5caf0ca52a7ae860d6bad37094f`.
No post-acceptance gameplay changes, builds or launches. Existing framework
changes, dependency pins and player data are untouched. [Note 242](242-d35g1-jump-edge-artifact.md)
retains the verified routes and limits; acceptance does not expand that coverage.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35G1 jump artifact candidate; gadget backlog research

D08A23 (instant N night vision, immediate fire) and D08A24 (automatic Bio Mask
on gas-area entry) are Todo with independent settings toggles and initial source
research on the board. Neither gadget feature is implemented.

D35G1 is Needs playtest. Native armed step-up 134 retained the pistol while the
mantle near-cull bridge revived its hidden native arms, producing stretched
skin-colored polygons. Hiding an arm now clears its interaction classification.
Build and four native suites pass, including a regression that fails before the
fix. Repeated private 60/120 routes remove the reproduced artifact; E-stowed
native hands remain visible. [Note 242](242-d35g1-jump-edge-artifact.md) records
routes, displayed-window captures and limits. User edge/corner playtest remains.

Candidate `58821f8d6191768dde25a04940386ace8d98b5caf0ca52a7ae860d6bad37094f`.
No framework/generated/pin changes, player-data writes, acceptance closeout,
commit or push. D35 stays closed; only its focused child is under review.
Launch `python3 recomp/tools/local/run.py`, load slot 5 and repeat the video
route, ordinary landings and E mantles. F10 captures if needed.


## 2026-10-25 - D35K4 accepted and closed

User: "lock it in and close it out". D35K4 is Done. Wheel pose 270 retains
first person under USE INTERACTION VIEW with native action restrictions and
camera safety guards preserved. D35/D35K remain closed; no next job is started.

Accepted executable:
`fe660aaf2bd31c478c58d8bbf48b1989513adf9662c3644419d982a5c0375614`.
Implementation, tests, manual and documentation are authorized for commit/push
as MusicMonsterMod. No post-acceptance gameplay changes, builds or launches.
Existing framework changes and dependency pins are untouched; no player data
changed. [Note 241](241-d35k4-wheel-first-person.md) retains the limits: fresh Vanilla activation, successful wheel
world activation and other variants were not verified by the diagnostic runs.
Acceptance does not expand that coverage. Launch: `python3 recomp/tools/local/run.py`.



## 2026-10-25 - D35K4 wheel turn candidate (Needs playtest)

Current slot 5 is LEVEL00's type-206 wheel, native pose 270. It now follows
USE INTERACTION VIEW with committed-action and camera safety guards intact.
Build and four native suites pass; private 60/120 routes retain first person
and complete. Action/global Third Person, pause/load and Vanilla saved-action
completion pass. The checkpoint exercises a restricted wheel attempt; successful
world activation and fresh Vanilla activation remain unverified. All 40 player
save/card hashes and mtimes are unchanged. User framing playtest remains.
[Note 241](241-d35k4-wheel-first-person.md) records evidence.
Candidate `fe660aaf2bd31c478c58d8bbf48b1989513adf9662c3644419d982a5c0375614`.
No framework/generated/pin changes, acceptance closeout, commit or push.
Launch `python3 recomp/tools/local/run.py`; slot 5, global and USE INTERACTION
VIEW First Person, approach the wheel and press E. F10 captures if needed.


## 2026-10-25 - D35K3 accepted and closed

User: "lock it in and close it out". D35K3 is Done. The statue's native pose
279 retains first person under USE INTERACTION VIEW, with native effects,
action restrictions and camera safety guards preserved. D35/D35K remain Done;
no further job is started.

Accepted executable:
`0e18a38e16e0a0a3a87afe62b44e559a87e7cad569c6a82267c4c1819439339f`.
Implementation, tests, manual and documentation are included in the authorized
MusicMonsterMod commit/push. No post-acceptance gameplay changes, builds or
launches. Existing framework changes and dependency pins are untouched; no
player data changed. [Note 240](240-d35k3-statue-first-person.md) retains the validation limits, including fresh
Vanilla activation and other statue variants. Acceptance is not campaign-wide
coverage. Launch: `python3 recomp/tools/local/run.py`.



## 2026-10-25 - D35K3 statue candidate (Needs playtest)

Current UI slot 5 is LEVEL09's type-926 winged statue. Its naturally selected
pose 279 now follows USE INTERACTION VIEW under the existing native action,
mode/history, script and camera ownership guards. Build and four native suites
pass; private 60/120 routes retain first person, complete and set the native
one-time effect flag. Action/global Third Person and pause/load checks pass.
Vanilla saved-action completion passes; fresh Vanilla activation remains
unverified. [Note 240](240-d35k3-statue-first-person.md) records evidence/limits.
User framing/comfort acceptance remains. D35/D35K stay Done. No framework,
generated-code or dependency-pin changes; no acceptance closeout, commit or push.
Candidate `0e18a38e16e0a0a3a87afe62b44e559a87e7cad569c6a82267c4c1819439339f`.
Launch: `python3 recomp/tools/local/run.py`; load slot 5, global First Person,
USE INTERACTION VIEW First Person, approach the statue and press E.



## 2026-10-25 - D35K3 statue interaction queued (Todo)

User reports interacting with the statue in current UI slot 5 switches to third
person. D35K3 records this as a focused backlog job; level, object and pose are
not yet verified. Use a fresh private copy when selected, not the old slot-5
door/toilet identity. Follow note 222's method and notes 238/239's interaction
evidence. D35/D35K remain Done. No implementation, build, launch, commit or push.


## 2026-10-25 - D35K2 and overarching D35 accepted and closed

User: "its perfect! lock it in" and "that now concludes in fact the overarching
D35 job so close that out too". D35K2 is Done; D35K bounded research and D35
parent are Done by explicit acceptance. The user may report further first-person
drops as they encounter interactions. Keep D35 as their category; create focused
child jobs using the existing evidence and fixes rather than reopening the whole
audit or starting an unsolicited sweep. Preserve deliberate cinematics and the
accepted third-person death behavior. D35N1 missing mirror voice remains Todo.

Accepted executable `d819cd7ad757ff3f3adf04b1382393b2cbcc70d3e822f3e84754439e753d996a`.
No new gameplay changes, build or launch during closeout. Explicit authorization
covers committing/pushing the toilet implementation, tests, manual and research
notes as MusicMonsterMod. Existing framework changes and pins are untouched.
Evidence limits in [notes 238](238-d35k-interaction-inventory.md)/[239](239-d35k2-toilet-first-person.md) remain; acceptance is not all-campaign proof.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35K2 toilet first-person candidate (Needs playtest)

Pose 275 now uses the existing USE INTERACTION VIEW presentation rule with
native action, mode/history and camera ownership guards intact. Build and four
native suites pass; private 60/120 use routes retain first person and complete.
Action view choice, pause/load and Vanilla saved-action completion pass.
Fresh Vanilla activation from the supplied checkpoint remains unverified, as
does fresh healing: its one-time effect was already consumed and stays so.
[Note 239](239-d35k2-toilet-first-person.md) records evidence and limits.
Candidate: `d819cd7ad757ff3f3adf04b1382393b2cbcc70d3e822f3e84754439e753d996a`.
User framing/comfort playtest remains. No framework/generated/pin changes,
commit or push. Broader D35K findings are not automatically implemented.
Launch: `python3 recomp/tools/local/run.py`; load slot 5, global First Person,
USE INTERACTION VIEW First Person, press E at the toilet.


## 2026-10-25 - D35K interaction inventory ready for review

Autonomous research from current slot 5's toilet confirms native pose 275
loses first person at 60/120, with normal ownership and recovery. Shared
interaction selector classes are mapped; direct alternate-camera entry calls
resolve to nine trigger subtypes in LEVEL00/05/09. Their gameplay identity,
control semantics and routes remain open. [Note 238](238-d35k-interaction-inventory.md).
D35K is Needs review; proposed toilet fix D35K2 is Todo, not started.
Slot-7 mirror 278 retains first person. Vanilla activation attempt did not
trigger the action and does not establish Vanilla toilet coverage.
No gameplay changes or build; accepted executable remains `4f8a04989350f79c087aef6e106a4f3b81b03828245692c294c40df316cbec69`.
All 40 source save/card hashes and mtimes match; private diagnostics stopped.
Existing framework changes/pins preserved. No commit/push or acceptance closeout.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35K1 door perspective and kick accepted and closed

User: "lock it in, close it out!". The delivered slot-5 first-person door
opening and visible right-leg kick are accepted as D35K1 Done. The bounded
fix retains native action timing/events, view choices and Vanilla behavior.
D35K remains Todo for broader scripted-event research; no next job started.
[Note 237](237-d35k-slot5-door.md) preserves validation and coverage limits.
Accepted executable: `4f8a04989350f79c087aef6e106a4f3b81b03828245692c294c40df316cbec69`.
Implementation, tests, manual and documentation are included in the authorized
MusicMonsterMod commit/push. No post-acceptance gameplay changes, builds,
launches, player-data changes or framework/pin edits. Existing framework
working-tree modifications remain untouched.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35K right-leg door-kick polish (Needs playtest)

The user confirms the perspective fix works and requested a visible right-leg
kick. The revised candidate reuses existing kick poses/meshes and thigh
rendering on the right, driven by native pose-116 channel progress. It adds
no attack damage or sound and retains native door events. [Note 237](237-d35k-slot5-door.md).
Build and four native suites pass; private 60/120 captures, action/global
Third Person, pause/load and Vanilla saved-action completion pass. Framing
playtest remains. Diagnostics stopped; private saves/settings only. The older
source manifest predates the user's latest slot-5 save; note 237 records that
comparison limit. No framework/generated/pin changes, commit or push.
Candidate: `4f8a04989350f79c087aef6e106a4f3b81b03828245692c294c40df316cbec69`.
Launch: `python3 recomp/tools/local/run.py`; use the door with global First
Person and USE INTERACTION VIEW First Person. Broader D35K inventory open.


## 2026-10-25 - D35K slot-5 door candidate (Needs playtest)

Current UI slot 5's hinged-door action uses pose 116, previously excluded from
first-person use presentation despite normal camera ownership. The candidate
adds it to USE INTERACTION VIEW with native action lockouts and all existing
identity/death/script/ownership guards intact. [Note 237](237-d35k-slot5-door.md).

Build and four native suites pass; private 60/120 routes, action/global Third
Person, pause/load and Vanilla saved-action completion pass. User playtest is
pending; the broader D35K scripted-event inventory remains open. D35 remains
Needs review and D35L remains Done/no action needed.
Candidate: `e8c065ed61fb9cb44d0205a933ce4a52d34aac7050f4ed8cfe39aa7070deccd7`.
Diagnostics stopped, player saves/cards unchanged, private settings only.
No framework/generated-C/pin changes, acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. Use slot 5 with global First Person
and USE INTERACTION VIEW First Person; approach the door and press E.


## 2026-10-25 - D35L accepted as-is; no action needed

The user explicitly prefers the existing third-person death presentation,
reports no issue with the behavior and requested closing D35L. D35L is Done
(user-accepted; no action needed). Preserve existing death/respawn behavior;
no first-person death option or additional research is requested.
This is a documentation-only closeout, based on user acceptance rather than
new agent verification of death variants or recovery routes. No code changes,
builds, launches or player-data changes. Existing framework modifications
remain untouched. D35K remains Todo and D35 remains Needs review.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35J4 accepted and closed

User explicitly instructed "lock it in and close it out!". D35J4 is Done.
Slot-9 crystal-holder take/return poses 268/267 now retain first person under
USE INTERACTION VIEW. Native action/item handling, Third Person choices and
Vanilla retain their behavior. [Note 236](236-d35j4-time-crystal-perspective.md)
records the build, four native suites, private 60/120 routes, pause/load and
Vanilla action-load evidence, including the initial load invalidation limit.

Accepted executable: `7da56adc98ba7b4bde4b92591a06cd35953da1a7b117450607363684264b5663`.
Implementation, regression tests, manual and documentation are included in the
explicitly authorized MusicMonsterMod commit/push. No post-acceptance gameplay
edits, rebuilds or launches. No framework edits/pin changes; existing framework
working-tree modifications preserved. No player data changed. Acceptance does
not establish additional underwater/crystal/campaign coverage.
D08Q7, D08T9, D35N1 and D08Q8 remain Todo; no next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35J4 time-crystal candidate (Needs playtest)

The user selected D35J4. Slot-9 crystal-holder take/return poses 268/267 now
retain first person under USE INTERACTION VIEW. Native action and item handling
remain unchanged; global/action Third Person and Vanilla retain their behavior.
[Note 236](236-d35j4-time-crystal-perspective.md) records evidence and limits.

Player build and controls/input/aim/near suites pass. Private 60/120 sequences,
third-person comparisons, pause/load and Vanilla saved-action completion pass.
The immediate load invalidation precedes stable first-person restoration;
user framing/comfort and broader crystal variants remain for playtest.
Candidate: `7da56adc98ba7b4bde4b92591a06cd35953da1a7b117450607363684264b5663`.
Diagnostics stopped; player save/card hashes and mtimes unchanged. No framework
or generated-C edits, player settings writes, implementation commit/push or
acceptance closeout. The other four reports remain Todo, including the
user-confirmed slot-8 dumpster repeat-push failure. Launch:
`python3 recomp/tools/local/run.py`. Test slot 9 with global First Person
and USE INTERACTION VIEW set to First Person.


## 2026-10-25 - Five new reports queued; D35J4 first

Backlog-only intake authorized with commit and push. All five jobs are Todo:
bug 1 D08Q7 (holstered weapon draw during jetpack flight), bug 2 D08T9
(level-5 dumpster repeat push/pull), bug 3 D35N1 (slot-6 missing mirror quote),
bug 4 D08Q8 (slot-12 jetpack floor/ceiling clearance), and bug 5 D35J4
(slot-9 time-crystal perspective). See the [canonical backlog](../MODERNIZATION_JOBS.md#user-reported-follow-ups-queued-2026-10-25).

**D35J4 is first when the user later authorizes work. Do not start any job yet.**
The user confirmed the slot-8 level-5 dumpster cannot be pushed again after
the first push. Repeat pulling and the cause remain unverified. The jetpack
reference is `jpacking-stairs.mp4` in `screencaps`; no video or save inspected.
These are user reports, not agent-reproduced findings. The accepted D35J/M/N result
remains the baseline, with a newly reported crystal gap awaiting investigation.
No gameplay changes, build, launch or player-data changes in this intake.


## 2026-10-25 - D35J/J1-J3/M/N accepted and closed

User explicitly accepted the combined candidate and authorized closeout:
"lock it all in and close us out". D35J bounded research and D35J1-J3, D35M
and D35N are Done. Idle gestures and held inventory retain first person;
USE INTERACTION VIEW covers the verified NPC, lever, key/door and crystal
actions. Independent MIRROR VIEW covers mirror use; reflected Duke remains
opaque up close in Modernized. Vanilla retains its existing behavior.

Evidence: [note 233](233-d35j-transient-perspective-research.md),
[note 234](234-d35j-use-and-idle-first-person.md),
[note 235](235-d35n-mirror-interaction-and-opacity.md). Player build, four native
suites, 100 profile tests, private 60/120 routes, live settings, pause/load and
Vanilla action-load checks passed before acceptance. Closeout makes no further
gameplay changes. Framework export matches the tracked patch and applies to
the clean pinned dependency; its existing working-tree modifications remain
intentional. No player saves/settings changed or next job started.

Accepted executable SHA-256:
`78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.
Untested damage/hazard variants and broader campaign coverage are not implied
accepted. Launch: `python3 recomp/tools/local/run.py`; Modernized is default,
`--vanilla` remains a one-launch override. Future work requires a new selection.


## 2026-10-25 - D35N mirror extension (Needs playtest)

Slot 12 mirror use now stays in first person, with an independent MIRROR VIEW
setting. Slot 7 reflected Duke stays opaque at close range in Modernized.
Intentional effects and Vanilla retain their policy. Previous idle/use work
remains included; the user reported it solid so far.
[Note 235](235-d35n-mirror-interaction-and-opacity.md).

Build, controls/input/aim/near suites and 100 profile tests pass. Private
60/120 mirror routes, global third person, live menu, pause/save/load and
Vanilla saved-action completion pass. Reflected head and arms remain visible.
User framing/comfort and broader mirror variants await playtest.
Candidate: `78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.
Private sessions stopped; player source saves unchanged. No authored framework
changes, player settings/card writes, acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. Test slot 12 with E and slot 7
close to the mirror; global First Person is required for the eye-view action.


## 2026-10-25 - D35J1-J3 / D35M candidate (Needs playtest)

All reproduced idle/NPC/lever/key/crystal and held-inventory perspective gaps
have a candidate fix. Idle/inventory retain the selected eye; USE INTERACTION
VIEW independently chooses First Person (default) / Third Person for the
verified use-action family. Native action permissions and events are retained.
[Note 234](234-d35j-use-and-idle-first-person.md).

Player build, controls/input/aim/near suites and 99 profile tests pass. Private
60/120 routes, item consumption, weapon return, third-person choices, live
menu, pause/load and Vanilla action-load checks pass. User comfort/framing
and broader variant coverage remain; jobs are Needs playtest, not accepted.
Candidate: `7c490764e5fd86316d1ef833e59467243a54489e31d72f6e3e0530cddea7d148`.
No framework/generated-C/pin changes, player settings/card writes, acceptance
closeout, commit or push. Private sessions stopped; source saves unchanged.
Launch: `python3 recomp/tools/local/run.py`. P selects First Person; gameplay
captures automatically. Test idle, slots 8-11 and held/released Scroll Lock.


## 2026-10-25 - D35J research candidate (Needs review)

Autonomous tests on private copies reproduced all five supplied cases:
idle fidgets (64/65/66), slot 10 NPC (277), slot 11 lever (118), and slots
8/9 key/crystal use (266). Held native inventory also drops first person
through its 0x200 guard. Normal camera ownership and the selected preference
persist; observed cases recover. One survivable RPG blast retained first
person. Enemy-hit variants, knockdown, heavy-fall and water-hazard coverage
remain unverified. [Note 233](233-d35j-transient-perspective-research.md).

D35J1-J3 are Todo, not started; D35M owns the use-interaction implementation.
No gameplay changes or build. Accepted D35I executable remains current:
`6b2b0ec48fa56c3fbd3a0a5aefac23dcb111ea4c95d9e66010f989e5408a2d42`.
Private sessions stopped; source save hashes/mtimes unchanged. No player
settings/cards changed, acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35I accepted and closed

User: "fully accepted! lock it in and close it out". D35I is Done.
First-person wall climbing, WALL CLIMB VIEW and fluid Modern WASD/diagonal
movement are accepted. [Note 232](232-d35i-wall-climbing.md) records tests,
implementation and the remaining campaign/other-wall coverage limits.

Accepted executable: `6b2b0ec48fa56c3fbd3a0a5aefac23dcb111ea4c95d9e66010f989e5408a2d42`.
Authored implementation, tests, manual and documentation included in authorized
MusicMonsterMod closeout. Runtime patch re-export unchanged; all 51 changed/new
files match against the pinned clean framework index. No dependency pins
changed. No post-acceptance gameplay edits, rebuilds or launches. No player
data changed and no next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35I wall climbing candidate (Needs playtest)

WALL CLIMB VIEW independently offers First Person (default) / Third Person.
Modern WASD now climbs walls fluidly, including normalized diagonals and prompt
stop/reversal, retaining native boundary/transfer queries and exits. The
user's slot-10 wall route is verified on a private copy; its top exit is left
of the straight-up obstruction. [Note 232](232-d35i-wall-climbing.md).

Player build, controls/input/aim/near suites and 98 profile tests pass. Private
first/third-person routes cover catches, mounts, directions, panel transfers,
upper/bottom exits, drop, saved attachment and pause/resume. Live menu and
Vanilla checks pass. User framing and movement-feel playtest remains; this is
not whole-campaign coverage.

Candidate: `6b2b0ec48fa56c3fbd3a0a5aefac23dcb111ea4c95d9e66010f989e5408a2d42`.
All diagnostic sessions stopped. Player slot-10 hashes/mtimes unchanged; no
player settings/cards, framework source or pins changed. Generated hook added
through the normal generator. No acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. P selects First Person; gameplay
captures automatically. WALL CLIMB VIEW is under Advanced Action Perspective.


## 2026-10-25 - D35F1 accepted and closed

User: "totally accept. lock it in and close us out". D35F1 is Done.
The attached chain now stays visible when its owning lower room leaves the
visible-room list, with native exit continuity and render replay support.
[Note 231](231-d35f1-chain-visibility.md) records the cause, tests and limits.

Accepted executable: `c604343e8378f0f8dcb438e892fa4e1082f3d94ad58cabbfba6085cc1303b306`.
Authored implementation, tests and documentation included in authorized
MusicMonsterMod closeout. Runtime patch re-export unchanged; all 51 changed/new
files reproduce against the pinned clean framework index. No dependency pins
changed. No post-acceptance gameplay edits, rebuilds or launches. Broader
campaign and direct attachment-cleared exit-save limits remain explicit.
No player data changed and no next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35F1 chain visibility candidate (Needs playtest)

Current UI slot 10's chain spans rooms. It vanished when its owning room 43
left the visible-room list during ascent in room 44. The candidate draws only
the attached pole-family object through the native renderer when that room
is absent, including native exit continuity and render replay. Movement,
attachment, room membership and Vanilla are unchanged.
[Note 231](231-d35f1-chain-visibility.md).

Player build and native controls/input/aim/near suites pass. Private routes
verify first-person upward/downward look, descent, Ctrl drop and upper exit;
the Third Person action choice also completes the upper exit. Primitive trace
confirms chain draws in the live process and all three replay workers while
room 43 is absent. User slot-10 playtest and broader campaign coverage remain.

Candidate: `c604343e8378f0f8dcb438e892fa4e1082f3d94ad58cabbfba6085cc1303b306`.
Diagnostic sessions stopped. No player settings/saves/cards, framework source,
generated C or pins changed. No acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. Gameplay captures automatically.


## 2026-10-25 - D35H accepted and closed; D35F1 queued

User explicitly requested "lock in this work and close it out" after recording
a separate backlog job. D35H is Done: OVERHEAD VIEW independently chooses
First Person / Third Person for monkey bars and ceiling hangs, retaining the
existing eye/hands and movement. [Note 230](230-d35h-overhead-perspective.md).

New Todo D35F1 records the user's current UI slot-10 chain disappearing as
Duke climbs toward the top. Backlog only: do not start it until selected.
No chain investigation, fixture capture or gameplay change was performed.

Accepted executable: `3fd09af36c36682d3ca94da1bba1c4bad4229dbd5060c4de3f9410bd78b3a164`.
Authored implementation, tests, manual and job records included in authorized
MusicMonsterMod closeout. Runtime patch re-export unchanged; all 51 changed/new
files reproduce against the pinned clean framework index. No post-acceptance
gameplay edits, rebuilds or launches. Recorded slot-9/campaign coverage limits
remain; no player data changed. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-25 - D35H overhead perspective candidate (Needs playtest)

The user reports first-person monkey bars already working. Confirmed in code:
D35G's mode-7 mantle eye/hands cover ceiling hangs. OVERHEAD VIEW now independently
offers First Person (default) / Third Person in Advanced Action Perspective,
using the existing eye/hands and unchanged D08J4 movement. Schema 47;
`--overhead-view`. See [note 230](230-d35h-overhead-perspective.md).

Player build, controls/input/aim/near suites and 97 profile tests pass. Private
historical grate route covers both choices, all travel directions, object
transfers, edge holds, mouse turning, saved hangs and Ctrl/Space drops. Live
menu cancel/confirm and global P pass. The current slot-9 copy loads on the
moat walkway; its route and user acceptance of the option remain unverified.

Candidate: `3fd09af36c36682d3ca94da1bba1c4bad4229dbd5060c4de3f9410bd78b3a164`.
All diagnostic sessions stopped. No player settings/saves/cards, framework
source, generated C or pins changed. No acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. P selects First Person;
OVERHEAD VIEW is in OPTIONS > ADVANCED > ACTION PERSPECTIVE. CONFIRM saves it.


## 2026-10-24 - D35G accepted and closed out

User: "lock it in, close it out!". D35G is Done. MANTLE VIEW offers First
Person / Third Person through mantles, ledge hangs, shimmy and water climb-outs,
with a stable eye and native hands. Movement and collision remain unchanged.
See [note 229](229-d35g-first-person-mantles.md) for tests and coverage limits.

Accepted executable: `96cb138081ecd17feda384a95f41ff7d26a49d9721563da34c898e6838d9f41d`.
Authored implementation, tests, manual and job records included in authorized
closeout as MusicMonsterMod. Runtime patch re-exported unchanged and verified
against the pinned clean framework index: all 51 changed/new files matched.
No post-acceptance gameplay edits, rebuilds or launches. Acceptance does not
establish unrecorded whole-campaign or blocked-ceiling coverage. No player data
changed and no next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35G first-person mantles (Needs playtest)

MANTLE VIEW in Advanced Action Perspective offers First Person (default) /
Third Person for ledge hangs, shimmy, pull-ups and water climb-outs. Stable
body-height eye and native hands; movement/collision/attachment unchanged.
Schema 46, launcher `--mantle-view`. [Note 229](229-d35g-first-person-mantles.md).

Build, native controls/input/aim/near and all 96 profile tests pass. Private
alley, sewer, airborne catch and natural level-6 water exit retain first
person; both action choices, saved hangs, global P and live menu cancel/confirm
pass. Pull-up image sequence and menu inspected. User comfort and broader
campaign/blocked-ceiling coverage remain unverified.

Candidate: `96cb138081ecd17feda384a95f41ff7d26a49d9721563da34c898e6838d9f41d`.
All diagnostic sessions stopped. No player settings/saves/cards, framework
source, generated C or pins changed. No acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. P selects First Person;
MANTLE VIEW defaults to First Person. Gameplay captures automatically.


## 2026-10-24 - D35F accepted and closed out

User: "so so good! let's lock it in and close it out!". D35F is Done.
CHAIN/POLE VIEW offers First Person / Third Person, with a steady eye and
native hands through chain/pole traversal. Existing movement and attachment
remain unchanged. See [note 228](228-d35f-first-person-poles.md).

Accepted executable: `f8fcbc1703d1c2ff713c9df54c2eef10f794bb647284e002c0277814e6105506`.
Authored implementation, tests, manual and job records included in authorized
closeout as MusicMonsterMod. Runtime patch re-exported unchanged and verified
against the pinned clean framework index: all 51 changed/new files matched.
No post-acceptance gameplay edits, rebuilds or launches. Unlocated solid-pole/
lamppost and natural no-floor-bottom coverage limits remain; acceptance does
not establish unrecorded full-campaign coverage. No player data changed and
no next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35F first-person chains/poles (Needs playtest)

CHAIN/POLE VIEW offers First Person (default) / Third Person in Advanced
Action Perspective. Stable body eye, column clearance and native hands;
existing movement, attachment, A/D latch and exits unchanged. Schema 45;
launcher `--pole-view`. See [note 228](228-d35f-first-person-poles.md).

Build, native controls/input/aim/near suites and all 95 profile tests pass.
Private chain types 842/841 cover top and airborne catches, circle/look,
descent/exits, explicit drop, pause/save/load, menu cancel/confirm, global view
and Third Person comparison. Shared catch 148's brief camera gap was found
and fixed. Separate solid pole/lamppost and natural no-floor-bottom traversal
remain unverified; user eye/hand/transition comfort acceptance is pending.

Candidate: `f8fcbc1703d1c2ff713c9df54c2eef10f794bb647284e002c0277814e6105506`.
All diagnostic sessions stopped. No player settings/saves/cards, framework
source, generated C or pins changed. No acceptance closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py`. P selects global First Person;
CHAIN/POLE VIEW defaults to First Person; gameplay captures automatically.


## 2026-10-24 - D35E accepted and closed out

User: "lock it in, close it out!!!". D35E is Done. LADDER VIEW offers First
Person / Third Person, with a stable body eye and native climbing hands through
plain-ladder traversal. Existing movement and collision remain unchanged.
See [note 227](227-d35e-first-person-ladders.md) for tests and coverage limits.

Accepted executable: `f1a319d4581e1713f7cf2a78c81480a0aa1674916690856c06c4ca5cf9b55ebc`.
Authored implementation, tests, manual and job records included in authorized
closeout as MusicMonsterMod. Runtime patch re-exported unchanged and verified
against the pinned clean framework index: all 51 modified/new files matched.
No post-acceptance gameplay edits, rebuilds or launches. Acceptance does not
establish unrecorded full-campaign or every-costume coverage. No player data
changed and no next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35E first-person ladder candidate (Needs playtest)

LADDER VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers First Person
(default) / Third Person, live on CONFIRM and saved (schema 44). Plain ladders
retain a stable body eye and native hands through mount, climb, hang and exit;
shared pole/ledge transfers keep their earlier policy. Existing movement and
collision remain unchanged. See [note 227](227-d35e-first-person-ladders.md).

Build, native controls/input/aim/near and all 94 profile tests pass. Private
lifecycle, enemy-at-top exit, rung save/load, pause, global view, menu cancel/
confirm and third-person comparison pass within the note's coverage limits.
The final extra 120-setting route covers failed grab, standing catch and
hang-to-ladder transfers. Camera comfort, hands and near-panel feel await the
user's playtest. No full-campaign or every-intermediate-frame certification.

Candidate: `f1a319d4581e1713f7cf2a78c81480a0aa1674916690856c06c4ca5cf9b55ebc`.
All diagnostic sessions stopped. No player settings/saves/cards, framework
source or pins changed. New getter hook generated normally; no hand-edited
C. No acceptance closeout, commit or push. Launch:
`python3 recomp/tools/local/run.py`. P selects First Person; capture is automatic.


## 2026-10-24 - D35D accepted and closed out

User: "lock it in, and close it out!!". D35D is Done. PUSH/PULL VIEW offers
First Person / Third Person, with a stable body eye and native hands during
grab/push/pull. Movement and limits remain unchanged. See
[note 226](226-d35d-first-person-push-pull.md) for tests and coverage limits.

Accepted executable: `09c5d8237e1f6da7da0c2651cf794ec27e1db12c466f88cebe7f9d558c0bca45`.
Implementation, tests, manual and job records included in authorized closeout
as MusicMonsterMod. Runtime patch re-exported unchanged and verified against
the pinned clean framework index: all 51 modified/new files reproduced.
No post-acceptance gameplay edits, rebuilds or launches. Acceptance does not
establish unrecorded full-campaign or damage-interruption coverage. No player
data changed and no next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35D first-person push/pull candidate (Needs playtest)

PUSH/PULL VIEW offers First Person (default) / Third Person under Advanced
Action Perspective. Stable body eye, native arm poses and ordinary world-depth
near clipping; existing movement, lanes and limits unchanged. Global Third
Person wins. Schema 43; launcher --push-view. See [note 226](226-d35d-first-person-push-pull.md).

Build, native controls/input/aim/near and all 93 profile tests pass. Four prior
private dumpster/symbol-block saves pass acquisition with W held, stop/reverse,
boundaries, looking around, blocked fire and release/redraw at 120 FPS setting:
1,072 sampled states retained first person, including 991 push states. At 60,
menu cancel/confirm, global view, pause and private grab save/load pass. Opposite
dumpster-face regrab, fixed-object exclusion and held W+E mantle also pass.
Eye height, hand visibility and transition feel await user playtest.

Candidate: `09c5d8237e1f6da7da0c2651cf794ec27e1db12c466f88cebe7f9d558c0bca45`.
All diagnostic sessions stopped. No player data, framework source, generated C
or pins changed. No commit, push or acceptance closeout. Launch:
`python3 recomp/tools/local/run.py`. P selects global First Person; gameplay
captures automatically.


## 2026-10-24 - D35C accepted and closed out

User: "lock it in, close it out". D35C is Done. JETPACK VIEW offers
First Person / Third Person, applies live on CONFIRM and persists independently
of the flight scheme. Global third person wins; flight exits follow the current
global view. See [note 225](225-d35c-jetpack-perspective.md).

Accepted executable: `9265a3f7b044df83eb47ba3de49e310993eabae3358892f8b24da772389f4053`.
Implementation, tests, manual and job records included in authorized closeout
as MusicMonsterMod. Runtime patch re-exported unchanged and verified against
the pinned clean framework index: all 51 modified/new files reproduced.
No post-acceptance gameplay edits, rebuilds or launches. User acceptance does
not establish unrecorded water/contact or full-campaign coverage. No player
data changed and no next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35C jetpack perspective candidate (Needs playtest)

JETPACK VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers First Person
(default) / Third Person, live on CONFIRM and persistent independently of the
flight scheme. Global third person wins; leaving flight follows the current
global view. Shared resolver/replay policy and accepted D11F bridges reused.
Schema 42; launcher --jetpack-view. No flight, fuel, hover or flame code changes.
See [note 225](225-d35c-jetpack-perspective.md).

Player build, native controls/input/near-clipping/EDuke32 fuel suites and all 92
profile tests pass. Private real-menu cancel/confirm, airborne switching,
flight save/load and relaunch passed. Two-level third-person routes cover all
three schemes (1,066 samples, 775 in flight). Platform descent passed 138 active
samples; mirror capture shows reflected flames. Sound continuity checked by
native loop handle with dummy audio, not listening. Natural water/contact
interruptions and camera-transition feel remain for user playtest. Final EDuke32
fuel-out/fall/landing, global view toggles, pause/resume and flight save/load
also passed. All diagnostic sessions stopped.

Candidate: `9265a3f7b044df83eb47ba3de49e310993eabae3358892f8b24da772389f4053`.
No player data, generated C or framework sources changed. No acceptance
closeout, commit or push. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D10B1 accepted and closed out

User: "lock it in and close it out!" D10B1 is Done. Native third-person
obstruction handling and Duke/scenery fading are the default again; D10B is
available only with explicit developer opt-in `DNTTK_TIGHT_CAMERA=1`.
The earlier fade-curve experiment is superseded and removed. No other job
started. Accepted executable:
`eb44c596365fdc76922aff91fd294be84510ce497a8c133436ae82f065aa5cb5`.

Build, controls (unset/0/1) and near-clipping tests passed as recorded below.
User acceptance does not establish exhaustive campaign verification. Authored
implementation, tests, manual and job records included in authorized closeout
as MusicMonsterMod. Runtime patch re-exported unchanged and checked against
the clean pinned framework index. No post-acceptance gameplay edits or rebuild.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D10B1 rollback candidate (Needs playtest)

User's slot-6 comparison found native box translucency useful during combat;
D10B's opaque scenery reduced visibility. Logged D10B1 before implementation.
Ordinary launch now uses the former `DNTTK_TIGHT_CAMERA=0` behavior: native
third-person obstruction/fade handling, no D10B compression, centering,
follow overrides or patterned Duke dissolve. Only explicit
`DNTTK_TIGHT_CAMERA=1` enables the historical D10B comparison. No saved setting
changed. Earlier and subsequent jobs remain intact. The unaccepted later-fade
curve experiment was removed; its historical record is retained below.

Build PASS. Native controls suite PASS with environment unset, 0 and 1;
near-clipping suite PASS, including native third-person occluder input and
retained first-person suppression. Default tests cover native minimum/follow,
untouched Duke fade and disabled replay coverage. Updated two old assertions
that assumed D10B was always active. Logs: `recomp/analysis/d10b1/` (local).
Candidate SHA-256: `eb44c596365fdc76922aff91fd294be84510ce497a8c133436ae82f065aa5cb5`.
No new live gameplay observation; slot-6 acceptance of this build and cramped
routes remain for user playtest. Framework modifications were not changed;
shared renderer/replay support remains inert by default. No commit/push or
acceptance closeout. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D10B later proximity fade (Needs playtest)

User requested a later, gentler fade with the existing spatial pattern.
Coverage now transitions over 450-1350 world units (previously 650-1700),
keeping Duke solid closer to the camera. At centered distance 1200, coverage
is about 93% instead of 54%. Shoulder relief and camera behavior are unchanged.
Player build and native controls regression suite passed. No live visual
verification in this follow-up; user playtest remains. No framework edits,
commit, push or acceptance closeout. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-24 - D35B accepted and closed out

User: "great work. fully accept, close it out". D35B is Done: SWIMMING VIEW
selects First Person / Third Person, applies live on CONFIRM, and persists
independently of swimming pace. Global third person wins; exit follows current
global view. Mantles retain their existing policy. See [note 224](224-d35b-swimming-perspective.md).

Accepted executable: `37b2f270bb73c026ac094963adf283e82083f565cba536d31d71130d1d7558b5`.
Implementation, tests, manual and supporting documentation are included in
authorized closeout as MusicMonsterMod. Runtime patch re-export unchanged;
clean pinned-framework verification reproduced all 51 modified/new files.
No gameplay edits, rebuilds or launches at closeout. Existing coverage limits
remain recorded. No next job started; D35 research review status is unchanged.
Launch `python3 recomp/tools/local/run.py` with saved preferences.


## 2026-10-24 - D35B swimming perspective candidate (Needs playtest)

SWIMMING VIEW in ADVANCED > ACTION PERSPECTIVE offers FIRST PERSON (default)
or THIRD PERSON. CONFIRM applies live and persists independently of
swimming pace. Global third person wins; exits follow current global view.
Mantles retain their third-person policy. Schema 41; launcher --swim-view.
See [note 224](224-d35b-swimming-perspective.md).

Player build, native controls/input and all 91 profile tests pass. Private
menu cancel/confirm, live switch, underwater load/relaunch and 64 two-level
matrix samples pass (both paces/views, configured 60/120 FPS). Natural entry/
exit and transition feel await user playtest. Candidate:
`37b2f270bb73c026ac094963adf283e82083f565cba536d31d71130d1d7558b5`.
No player data changed; diagnostic processes stopped. No framework edits,
acceptance closeout, commit or push. D35 research is complete as a deliverable,
with its review status unchanged. No next job started.
Launch `python3 recomp/tools/local/run.py`; select first person with P if needed,
then try both SWIMMING VIEW choices. Gameplay captures automatically.



## 2026-10-24 - D35A accepted and closed out

User confirmed unchanged behavior and explicitly requested closeout. D35A is
Done: shared presentation policy, replay-safe resolved state and settings
helpers, with current action support and guards preserved. No new action
options. See [note 223](223-d35a-action-perspective.md).

Accepted build: `d0710eb9081889a13856dc12a203cccc27a4fd09b169ed1bb264a420e4db8fc3`.
Implementation, tests and supporting documentation are included in authorized
closeout as MusicMonsterMod. No gameplay edits, rebuilds or launches at
closeout. D35B-D35N remain unstarted; D35 research status remains Needs review.
Launch `python3 recomp/tools/local/run.py` with saved preferences.


## 2026-10-24 - D35A foundation candidate (Needs playtest)

The shared presentation resolver, replay-safe policy state and settings helpers
are implemented. Existing first-person support/fallbacks and action permissions
are preserved. No action toggle is exposed before its implementation. Launcher
wording now reflects accepted swimming/jetpack support. See
[note 223](223-d35a-action-perspective.md) for changes and evidence.

Player build, controls/input/profile tests and private offscreen view/save-load
and menu cancel/confirm/resume checks passed. User regression playtest remains.
Candidate: `d0710eb9081889a13856dc12a203cccc27a4fd09b169ed1bb264a420e4db8fc3`.
No acceptance closeout, commit or push; prior D35 research and framework changes
preserved. Launch `python3 recomp/tools/local/run.py` with saved settings.

## 2026-10-24 - D35 first-person audit ready for review

Research only: [note 222](222-d35-first-person-audit.md) inventories host camera
ownership, blend/release/reset and indirect anchor failures, native special-view
entry/exit, supported actions, missing action perspectives and configuration.
D35 is Needs review; D35A-D35N are Todo and none is started.

Swimming and jetpack FP are already accepted; only per-action legacy-view
choices are missing. Push/pull and attached traversal explicitly yield FP.
Doors and mirror interactions are user-confirmed transitions with exact native
causes still awaiting traces, tracked separately as D35M/N. Damage/inventory,
other scripts and death have focused research stages. Whole-campaign coverage
remains D22; no claim of exhaustive dynamic event coverage.

Recommended start after research acceptance: D35A shared policy, then D35B/C
small configuration jobs; reproduce doors/mirrors early, then bounded push/pull
and climb work. No gameplay/settings/animation changes, builds or launches;
existing framework modifications preserved. No commit/push or acceptance
closeout. Current saved-settings launch remains:
`python3 recomp/tools/local/run.py`.


## 2026-10-24 - D10B accepted and closed out

User: "fully accept!! lock it in and close it out". D10B is Done. Accepted
collision-aware third-person compression, shoulder centering and eased return,
Duke-only proximity fade, and solid near doors/walls. Saved framing, Vanilla
and Original camera behavior are preserved.
Accepted build: `4396ea72a48d95b6de13999501ff8b4f53f46515533b8aa7d7a6ca42d54429eb`.

Implementation, regression tests, exported framework patch and documentation
included in authorized closeout as MusicMonsterMod. Existing test evidence and
limits remain in [note 221](221-d10b-tight-camera.md). No post-acceptance gameplay
edits, rebuilds or launches. Runtime patch re-exported and verified against the
clean pinned framework. No next job started.
Launch `python3 recomp/tools/local/run.py` with saved preferences.


## 2026-10-24 - D10B tight-space camera candidate (Needs playtest)

Collision-aware third-person compression, shoulder centering, held/eased return
and Duke-only proximity fade on OpenGL are implemented. Near doors/walls stay
solid. Saved framing is preserved; Vanilla and Original camera retain their
behavior. Software retains the native body fade.

Build, controls/input/aim/near-clipping and replay codec tests pass. Final private
60/120 FPS apartment routes passed yaw/shoulder/view changes, firing and private
save/load; the sewer ladder-bottom heading sweep also completed. Runtime patch
exported and verified against the clean pinned framework. See
[note 221](221-d10b-tight-camera.md) for evidence and remaining acceptance.

Candidate: `4396ea72a48d95b6de13999501ff8b4f53f46515533b8aa7d7a6ca42d54429eb`.
User playtest remains for camera feel, fade appearance and wider campaign routes.
No commit, push or acceptance closeout. Launch `python3 recomp/tools/local/run.py`
with saved preferences; select third person (P by default) to try D10B.


## 2026-10-24 - D11F accepted and closed out

User: "lock it in and accept! close it out!" D11F is Done. Accepted first-person
jetpack flight in all three schemes, with Modern/EDuke32 floor/platform hover,
descent below departed platforms, release-height retention and mirror flames.
Accepted build: `85d6969593b96e0dc0b8358e310daca0b83e31e0f12ee95780bcf191042a3fd9`.

Implementation, regression tests and documentation included in authorized
closeout as MusicMonsterMod. Existing test evidence and limitations remain in
[note 220](220-d11f-first-person-jetpack.md). No post-acceptance gameplay edits,
rebuilds or launches. Runtime patch re-exported unchanged and verified against
the clean pinned framework. No next job started; user chooses next work.
Launch `python3 recomp/tools/local/run.py` with saved preferences.


## 2026-10-24 - D11F slot 12 platform correction (Needs playtest)

The earlier ground-level fix trusted a stale native floor cache. A fresh native
collision sweep now prevents descent onto platforms from ending flight and
allows Ctrl to descend below them after leaving. Space/Ctrl release keeps the
new height. Modern 60 FPS / EDuke32 120 FPS private slot-12 routes pass;
final EDuke32 route includes release-height and J-off checks (138 samples).
Native controls and EDuke32 fuel/toggle/safety suites pass. See
[note 220](220-d11f-first-person-jetpack.md) for setup limits and mirror evidence.

Candidate `85d6969593b96e0dc0b8358e310daca0b83e31e0f12ee95780bcf191042a3fd9`.
No framework changes, player-save writes, commit, push or acceptance closeout.
Launch `python3 recomp/tools/local/run.py`; user slot-12 playtest remains.


## 2026-10-24 - D11F ground-hover and mirror follow-up (Needs playtest)

Modern/EDuke32 Ctrl descent keeps the pack running at a low ground-level hover,
including its sound and flames; release holds, Space climbs and J shuts it off.
The mirror now receives the flame flags consumed by the primary Duke draw.
Classic and Vanilla physics remain unchanged. Build/native controls (including
EDuke32 fuel/toggles), input, aim and near-clipping suites pass. Private Modern
60 FPS / EDuke32 120 FPS club-floor routes passed 248 samples, including J-off;
mirror flames visually verified. Remaining user playtest and limitations are in
[note 220](220-d11f-first-person-jetpack.md).

Candidate `46a93ab9f8eef1a500275f4013a3bd59b98efba9484b25dc9bd8e0151a9fe769`.
No framework edits, commit, push or acceptance closeout.
Launch `python3 recomp/tools/local/run.py`; press P if needed.


## 2026-10-24 - D11F first-person jetpack candidate (Needs playtest)

Modern, Classic and EDuke32 retain selected first person through flight,
with a stable eye and presentation coverage for pack-off/fuel-out/landing.
Flight physics, controls and third-person policies remain unchanged.
[Note 220](220-d11f-first-person-jetpack.md) records implementation and limits.

Build/native suites pass. Two-level 120 FPS flight matrix: 1,070 samples.
Final executable: native controls plus 224 Modern/EDuke32 60 FPS transition
samples, including flight save/load, pause/resume, toggles and fuel-out.
Broader campaign/weapon/clipping coverage and user camera-feel acceptance remain.
Candidate `9912e67e4ba820de6b620bf6727c7990125c5c8578261e8b21e98050e76c2438`.

No framework edits, commit, push or acceptance closeout. Private diagnostic
sessions stopped. Launch `python3 recomp/tools/local/run.py`; press P if needed.


## 2026-10-24 - D08S accepted and closeout authorized

User: "this is it!!! lock it in, close it out". Revision 4 is accepted:
J starts immediately, ground starts lift slightly, and midair starts hold
the activation height without a boost. Modern and EDuke32 share these
controls; EDuke32 retains its 61-second fuel rule. D08S is Done.

Implementation, tests and player documentation are included in closeout.
Existing build/native tests and private 60/120 FPS evidence stand; no
post-acceptance gameplay changes or additional game launches. Runtime patch
re-exported unchanged and verified against the clean pinned framework.
Commit and push authorized as MusicMonsterMod. No next job started.

## 2026-10-24 - D08S revision 4: no midair boost

Latest user clarification: ground starts keep the accepted lift; airborne
starts arrest vertical movement and hold the activation height. Modern and
EDuke32 now skip both startup lift and extra host floor-clearance climb on
air activation. Native collision and explicit Space/Ctrl controls remain.
[Note 219](219-d08s-instant-jetpack.md).

Build/native controls suites pass. Private jump/falling-restart checks at
60/120 FPS held exactly; ground lift remains 642 units. Candidate:
`8e976893091271ddbaaa019878cba08f5acede6874433336fcdfca69a611b548`.
Needs playtest. No closeout, commit, push or next job.

## 2026-10-24 - D08S revision 3: Modern midair activation

User accepted the lift/controls and requested J activation in midair too.
Modern now starts flight during jumps/falls; rapid off/on can reverse the
pack's own closing transition. The accepted lift/flight feel and fuel are
unchanged. [Note 219](219-d08s-instant-jetpack.md).

Build/native suites pass; private live rising-jump and rapid falling off/on
checks pass at 60/120 FPS. Candidate:
`d57f64b3c33bdff4182b5bc66f81f5a933abc26e26c14428f0018389214e7ae6`.
D08S remains Needs playtest for this addition. No commit/push or next job.
Launch `python3 recomp/tools/local/run.py --jetpack modern` (saves Modern).


## 2026-10-24 - D08S revision 2: J-only lift in Modern

User supplied `research/screencaps/eduke32-jetpack.mp4` and corrected Modern:
J must start flight and lift slightly without Space. Modern now enters
flight from grounded J; Modern and EDuke32 share a short automatic lift
through native velocity/collision, then hover. Their fuel rules remain
unchanged; Classic still requires Space. [Note 219](219-d08s-instant-jetpack.md).

J-only private live checks pass at 60/120 FPS (640-642 unit rise and stable
hover); controls and D08S native suites pass, including early Ctrl override.
Candidate `6564b4e4101957de920a0773dedc4237a25ba761c593462cd635097dee7ed89c`.
D08S remains Needs playtest. No commit/push or next job.
Launch `python3 recomp/tools/local/run.py --jetpack modern` (saves Modern).
Press J alone; Space climbs further and Ctrl descends. Capture is automatic.


## 2026-10-24 - D08S candidate ready for playtest

D08S is Needs playtest. The new `eduke32` jetpack scheme gives instant J
activation on ground/in midair, instant J-off/fuel-out into a fall, and a
61-second full tank including hover. Existing Modern flight supplies movement,
hover and collision; native landing/water transitions close the pack.
[Note 219](219-d08s-instant-jetpack.md) records implementation and evidence.

Player build, native controls/input/aim and 89 profile tests pass. Private
60/120 FPS tank runs timed about 61.1 seconds; live toggles, movement/fire,
pause, landing, water and Modern/Classic regressions are recorded. Wider
campaign and user feel remain acceptance work. No framework edits, commit
or push. Candidate:
`de95de02b313f248375862ab39bcc46405807473f5bf96dd135c670dc1a1f887`.

Launch `python3 recomp/tools/local/run.py --jetpack eduke32`. This saves the
scheme; `--jetpack modern` restores the existing default. J on/off, Space up,
Ctrl down, WASD/mouse; capture is automatic. Flight remains third person
(D11F is separate). No next job started.


## 2026-10-24 - D08Z6 accepted and closed

User accepted revision 2 and explicitly requested lock-in/closeout. D08Z6 is
Done. [Note 218](218-d08z6-blocked-run-jumps.md) records forward carry after
clearance and first-press response during grounded run stops, with native
and live evidence and the separate diagnostic reload-halt limitation.
Earlier Needs playtest entries below are historical.

Accepted executable:
`e250b5b5ec43c2a2a8fd1c39e6ab414b89d90d3b0329979dbdb42ba05a7fa78f`.
No gameplay edits, rebuild or new live test after acceptance. Runtime patch
re-exported unchanged and verified against clean pinned framework
`08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match. Pins unchanged.
Implementation, tests, manual and documentation commit/push explicitly
authorized as MusicMonsterMod. No next job started.

Launch `python3 recomp/tools/local/run.py`. Current saved settings select
manual jumping and slide walls. Slot 10: Shift + W against the step, then
one Space tap. P changes view; capture is automatic.


## 2026-10-24 - D08Z6 revision 2: first Space press

User reports the first Space press does nothing. Reproduced a short tap during
grounded stop animation 94 expiring before idle; the earlier 2-second waits
missed this interval. Manual running now starts the same 96 preparation from
grounded 94/95 on a fresh press, with camera/epoch/identity and action guards.
Forward carry and original clearance remain intact. [Note 218](218-d08z6-blocked-run-jumps.md).

Build and controls suite pass. Single-tap tests cover both views at 60/120 Hz,
early/late stop and settled idle. Candidate is Needs playtest, not accepted.
Executable `e250b5b5ec43c2a2a8fd1c39e6ab414b89d90d3b0329979dbdb42ba05a7fa78f`.
A preliminary private reload dispatch halt and its evidence limits are in
note 218; valid jump tests used fresh launches. No framework edits or commits.
Launch `python3 recomp/tools/local/run.py`; UI slot 10, Shift + W into the
step then one Space tap. P switches views; capture is automatic.


## 2026-10-24 - D08Z6 candidate ready for playtest

D08Z6 is Needs playtest. [Note 218](218-d08z6-blocked-run-jumps.md) records
the blocked-run standing-speed cap and bounded manual air-steering correction.
The existing 97 -> 98 clearance handoff now permits held running input to
build toward normal running-jump speed, preserving the native vertical arc
and collision. Walking, assisted, ordinary running and Vanilla are unchanged.

Player build and native controls/input/aim suites pass. Private live routes
cover both views at 60/120 Hz, slot-10 middle/right approaches, early and late
release, lateral steering, and matched accepted-build obstacle/open-run checks.
Same tested apex; greater forward carry onto the target ledge. Player saves,
cards and settings unchanged; diagnostic games stopped. Wider geometry and
user feel remain acceptance work. No framework edits, commit or push.

Candidate executable: `eff4617cd7d7558db0f1308c3d742bbbc6d4e0655f53d5007bca8348eaa56045`.
Launch `python3 recomp/tools/local/run.py`. Current player settings already
select manual jumping and slide walls. Load UI slot 10, hold Shift + W into
the step near its right corner until stopped, then press Space without
releasing Shift + W. P changes view; capture is automatic. No next job started.



## 2026-10-24 - D08AF accepted and closed

User accepted the plunge-to-swimming handoff and explicitly requested lock-in
and closeout. D08AF is Done. [Note 217](217-d08af-plunge-swim-handoff.md)
records the accepted horizontal blend, native resistance/collision/release
behavior and existing test limits. No gameplay edits, rebuild or new live
test after acceptance; earlier candidate entries below are historical.

Accepted executable:
`710de20931fbebc816ef47e3b601b0b76afb381a6f1c4fafd4c84f7d22d1605c`.
Runtime patch re-exported unchanged and verified against clean framework pin
`08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match. Pins unchanged.
Implementation/tests/manual/documentation commit and push explicitly authorized
as MusicMonsterMod. Wider campaign geometry remains unexhausted. D08Z6 remains
Todo; no next job started.

Launch: `python3 recomp/tools/local/run.py`. Capture is automatic; P changes
camera view. SWIMMING MODERN includes the accepted handoff.


## 2026-10-24 - D08AF candidate ready for playtest

D08AF is Needs playtest. [Note 217](217-d08af-plunge-swim-handoff.md) records
the native residual-fall motion dip and bounded horizontal swim-input blend.
The accepted-binary comparison shows a reduced slot-9 late motion dip, equal
sampled entry depth and identical released-input final XYZ/zero velocity.
24 candidate entry routes cover both cameras, 60/120 Hz and two water
locations; separate swim/oxygen/fire/Original/Vanilla checks complete.
All 40 player save/card files retain their hashes. User feel and wider
geometry acceptance remain outstanding. No commit/push or next job started.

Launch `python3 recomp/tools/local/run.py`. Load UI slot 9, hold Shift + W,
press Space and keep moving into the water. Judge the end of the plunge;
release/reapply W as a second check. P switches cameras. Capture is automatic.
The previous D08AF Todo entries below are historical.

## 2026-10-24 - D11E accepted and closed

User accepted first-person swimming and explicitly requested lock-in/closeout.
D11E is Done. [Note 216](216-d11e-first-person-swimming.md) records the accepted
stable swim eye, animated weapon display, replay publication, holster-camera
continuity and deliberate third-person mantle behavior, with test limits.

Accepted executable:
`6725addce03114878375d08d81331a4bd4032aa5169d21fff74a586602b73ea6`.
No gameplay changes, rebuild or new live test after acceptance. Existing native
and live results were reviewed; acceptance does not add unperformed coverage.
Implementation, tests, manual and research/candidate notes are in closeout.
Runtime patch re-exported unchanged and verified against clean framework pin
`08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match. Pins unchanged.
Commit/push explicitly authorized as MusicMonsterMod.

D08Z6 (slot-10 blocked-run ledge jump) and D08AF (slot-9 plunge-to-swim
hesitation) remain Todo. No next job started. Earlier D11E Needs review /
Needs playtest entries below are historical.

Launch: `python3 recomp/tools/local/run.py`; P selects first person if needed.
Space ascends, Ctrl dives, E climbs out. Capture remains automatic.


## 2026-10-24 - D11E candidate ready for playtest

User authorized first-person swimming implementation after the research phase.
D11E is Needs playtest. [Note 216](216-d11e-first-person-swimming.md) records
the stable surface/underwater eye, animation-channel swim weapon display,
replay publication, swim holster presentation guard and early mantle override.
Mantles still use third person and return to the selected view on supported land.

Player build and final native controls suite pass, with new swimming/weapon/
worker/transition coverage. Input, aim and near-clip suites pass. Evidence
includes 415 swimming samples across levels 2/6 at configured 60/120/240,
real keyboard holster/P/pause/console/save-load checks, plus final first/third/
Vanilla and actual 4:3 regression captures. Note 216 distinguishes candidate
revisions and failed diagnostic attempts. Full natural entry/climb-out routes,
wider geometry/costume coverage and user feel remain acceptance work.

Candidate executable:
`6725addce03114878375d08d81331a4bd4032aa5169d21fff74a586602b73ea6`.
Accepted pre-D11E baseline remains `95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.
Private test profiles/cards only; diagnostic game/Xvfb processes stopped.
No framework/generated-C/media changes, commit or push. No new job started.
Launch `python3 recomp/tools/local/run.py`; press P if needed. Space ascends,
Ctrl dives, E climbs out; capture is automatic.


## 2026-10-24 - D11E swimming first-person research ready for review

User selected D11E for deep research before implementation. The board now says
Needs review (research; implementation outstanding).
[Note 215](215-d11e-first-person-swimming-research.md) is the implementation
handoff: current source contracts, six owned-EXE guard matches, original camera
flow, 360 private baseline samples in levels 2/6 at configured 60/120 FPS,
proposed eye/weapon/replay work and explicit remaining questions/test matrix.

The first-person preference survives swimming; current source deliberately
refuses the eye view. Raw neck position changes by hundreds of units with swim
posture, so merely removing that refusal is insufficient. Research supports a
bounded prototype, not finished waterline/weapon rendering. Preserve current
swim physics, oxygen, combat, costume lookup and D26B ownership. Decide and
verify the existing temporary third-person mantle exception explicitly.

No gameplay/framework edits or rebuild; accepted executable remains
`95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.
No player saves/cards/settings used by diagnostics; private test processes
stopped. No commit/push. Local evidence: `recomp/analysis/d11e-research/`.
Implementation, visual playtesting and acceptance remain outstanding.


## 2026-10-24 - D08T6 accepted and closed

User explicitly authorized completion notes, Done status, commit and push.
D08T6 now ties its acquisition-versus-held-travel investigation to accepted
D08T7 lane reacquisition and D08T8 apartment-platform pull-up/drop.
[Note 211](211-push-pull-lock-zones.md) preserves the original research and adds
completion evidence, corrected location and links to both implementation records.

Documentation-only closeout after source/native-assertion/retained-evidence
review; no build, new live test or gameplay/framework edits. Accepted executable
remains `95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.
Wider placement coverage remains limited. The intermittent reload GPU halt is
still unresolved; note 213's recurrence guidance applies. No next job started.
Earlier Needs review entries for D08T6 below are historical.


## 2026-10-24 - D08T8 accepted and closed

User confirms the apartment-platform fix works exactly as intended and
explicitly authorizes lock-in and closeout. D08T8 is Done. Accepted build:
`95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.
Implementation, native tests, manual and evidence are included in closeout.
[Note 214](214-d08t8-alley-platform-hang.md) preserves test coverage and limits.
The private high-refresh reload GPU halt remains unresolved; revisit note 213
if it recurs. This acceptance does not claim that GPU problem was fixed or
that every campaign placement was tested. No subsequent job is started.
Earlier Needs playtest entries below are historical.

## 2026-10-24 - D08T8 candidate ready for playtest

Implemented the apartment-platform pull-up retry using the original clearance
query, plus Space-to-Down drop with held-input consumption. Dumpster/dragging
and platform route pass; W, Space, S, pause, stowed climbing and fresh regrab
pass in private runs. Native controls/input/aim pass; Vanilla remains original.
Both cameras reach the platform. One first-person 120 FPS private hang reload
halted with GP0 0x1F323232; subsequent baseline/candidate restores passed.
Cause unresolved and live post-mortem capture incomplete; see D20G recurrence
checklist and [note 214](214-d08t8-alley-platform-hang.md). No GPU changes.

D08T8 is Needs playtest, not accepted. Wider placement/obstruction matrix and
fresh live ladder/ceiling/crouched-escape checks remain. Test processes stopped;
all 41 player source hashes unchanged. No framework edits or commit/push.
Candidate identity and test evidence are in note 214. Earlier entries below
are historical, including the pre-fix executable and reproduction-only status.

## 2026-10-24 - D08T8 apartment platform hang reproduced

User corrected the location: current UI slot 7, climb the dumpster successfully,
then mantle the apartment platform directly ahead. Dragging checks pass by user
report. Two private routes reach the platform hang; separate W/Space/S checks
confirm W cannot pull up, Space cannot drop, S releases. Mode 6, animation 149;
host pad routes all three inputs with capture active. Cause remains open.

[Note 214](214-d08t8-alley-platform-hang.md) records evidence and limits.
D08T8 is Needs review for this requested reproduction, with fix outstanding.
D08T6 remains Needs review; D08T7 remains Done. All 41 player source files
unchanged; test processes stopped. No gameplay/framework edits, build or
commit/push. Accepted executable remains `0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.


## 2026-10-24 - D20G investigation accepted and closed

D20G is Done by explicit user instruction. 82 private restores across accepted
D08T7 and pre-D08T7 D08T5 passed; the original slot-11 GPU halt remains
unreproduced, with cause and regression origin unresolved. No fix is claimed.

**If similar save/load GPU halts, wrong colours or scrambled geometry recur,
consult Done job D20G and [note 213's recurrence checklist](213-d20g-slot11-load-halt.md#recurrence-checklist-for-future-done-job-reviews).**
Capture live mod-memory aperture/Expansion 1 as well as RAM before quitting;
the earlier RAM-only DMA walk was invalid after its aperture link. Retain load
sequence, settings, binary hashes and host history. Mid-composition host render
state is a diagnostic lead, not a proven cause. D20C remains relevant background.

No gameplay/framework changes or rebuild. Current accepted executable remains
`0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.
Player files unchanged. User authorized documentation/commit/push closeout.
D08T8 remains Todo; await the user's next selection.

Launch: `python3 recomp/tools/local/run.py`; automatic gameplay capture.


## 2026-10-24 - D08T7 accepted and closed; D20G next after context clear

User: mechanic is "absolutely solid" after several tests, then explicitly
requested backlog creation and commit/push to lock in the completed task.
D08T7 is Done. Accepted executable:
`0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.
Implementation, tests, manual and evidence are part of authorized closeout.
No gameplay edits or rebuild after acceptance; player files unchanged.
Runtime patch re-exported unchanged and verified against clean pin
`08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match.

**Next work: D20G (Todo), only after fresh context.** Investigate the captured
UI slot 11 reload GPU fatal halt, assess regression versus baseline and whether
any fix is necessary. [Note 213](213-d20g-slot11-load-halt.md) is the intake.
This closeout does not start that investigation or implement a speculative fix.
After its review/closeout, return to push/pull work; D08T8 remains Todo.
D08T6 is the earlier research record, still Needs review on the board.

Launch: `python3 recomp/tools/local/run.py`; automatic capture.


## 2026-10-24 - UI slot 11 reload fatal halt captured (08:46:44 BST)

User reports D08T7's mechanic is solid across several tests, but asked to inspect
an app frozen while loading UI slot 11 before giving further feedback. No job
acceptance closeout or commit/push was requested.

`session-20261024-083523.log`: internal slot 10 preflight/apply succeed, 20.2 ms
restore and 22.9 ms total load; then fatal GPU GP0 command 0x1F, word 0x1F20161D.
The same slot loaded successfully earlier in the session. At capture, debug
frame is 40732 and savestate generation 4 reports last load successful. The
fatal dump shows GPU DMA channel 2 active. This establishes a post-restore
emulation halt, not a host process crash or proof of a corrupt save. Root cause
and any relationship to D08T7 remain unproven; do not suppress the opcode blindly.

Private evidence: `recomp/analysis/slot11-halt-20261024/`: copied session log,
28 MB automatic fatal dump, full 2 MiB live RAM, debug status/input, system journal,
process snapshot, private copies of current UI slots 11/12 and SHA-256 manifest.
All six source save/sidecar/thumbnail files unchanged. Debug quit initially
reported busy/frozen; the app exited before the subsequent TERM attempt (PIDs
already absent). No application process remains. No gameplay or framework edits.

Host snapshot at 08:47:44: CPU/I/O/memory pressure avg10/avg60 zero, about 25 GiB
available RAM, negligible swap, no Timeshift/rsync process found. Journal:
automatic upgrades 08:41:10-08:41:21; Bottles scope ended 08:46:18; PackageKit
ended 08:46:19; firmware metadata refresh 08:47:06. These do not establish host
conditions at every frame or a cause of the fatal GPU command. No OOM or GPU
reset appears in the captured interval. No new reproduction/build was attempted
for this immediate log inspection; preserved evidence supports later isolation.


## 2026-10-24 - D08T7 lane regrab candidate (Needs playtest)

The alley dumpster and three verified LEVEL06 symbol-block lanes can acquire a
new grab when Duke's original floor permission bit is clear. Original alignment,
standing/support/collision checks and the X 7292 ladder cap remain. Fixed LEVEL05
scenery stays fixed. Policy uses authenticated level/type/lane/rotation, not RAM
addresses, slot numbers or previous host grabs, so boundary reloads work.

Private real-input tests pass: boundary regrab/reverse, midpoint, exact return,
save/reload, both dumpster ends, recent slots 9-11, fixed-dumpster control and
first-person preference at 120 FPS with pause/resume. Build and native
controls/input/aim pass, including all 21 level guards. 40 intake-source files
unchanged. See [note 212](212-d08t7-lane-regrab.md) for evidence and limitations.
Candidate: `0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.

User feel and broader approach coverage remain. D08T8 side mantle is unchanged
and Todo. No framework edits, acceptance closeout, commit or push. Launch:
`python3 recomp/tools/local/run.py`; automatic capture, saved player preferences.


## 2026-10-24 - Push/pull lock-zone research and adjacent mantle backlog

D08T6 is Needs review; D08T7 (lane-wide regrab) and D08T8 (dumpster side mantle)
are Todo. See [note 211](211-push-pull-lock-zones.md). Current source and retained
D08T5 traces show held reverse works at X 7292, but new grabs can fail because
Duke's floor lacks the original grab bit. A mobility policy must also cover
original acquisition and reloads while preserving fixed scenery and guardrails.
The reported side mantle W/Space failure is not freshly reproduced; no common
cause is established. Research only, no build/live run or gameplay changes.
The accepted D08T5 executable remains current. No commit/push or acceptance
closeout. Implementation remains separate incremental work.


## 2026-10-24 - Slot 12 mantle slowdown: not reproduced on user retest

User reported severe slowdowns while pushing the dumpster to its limit, mantling
it and mantling the alley platforms from UI slot 12. The local session log
`recomp/build-local/logs/session-20261024-080422.log` confirms internal slot 11
loaded and emulation fell to 33.0 and 26.7 frames/s around the climbing sequence,
with high-refresh presentation reduced.

Read-only system checks found Timeshift started at 08:00:01 BST and its rsync
processes were still active at 08:07, overlapping the 08:04-08:05 gameplay.
Backup and game shared the same physical disk. At inspection, I/O pressure
averaged about 54% over the preceding minute; about 27 GiB RAM was available
with negligible swap use. Flatpak update activity also overlapped gameplay.
These observations make host contention a suspect, not a proven cause of the
mantle-specific stalls. The log did not contain per-frame profiling, and the
available historical atop sample ended before this gameplay session.

The user then reported they could not reproduce it on another attempt. No code,
settings or background processes were changed by the agent; no regression or
fix is established. Do not reopen mantle work on this evidence alone.

Standing user instruction: whenever performance is reported, check the state
of the computer at the time, including backups and updates, before assuming a
regression. Capture current evidence promptly and correlate system history with
the gameplay log. This rule is also recorded in `AGENTS.md`.

## 2026-10-24 - D08T5 accepted and closed

User: "lock it in!! close us out". D08T5 is Done: the dumpster stops at slot 7's
X 7292 reference position. Prior build/native/live evidence and existing grab-floor
restrictions are retained in [note 210](210-d08t5-dumpster-limit.md).
Accepted executable: `bf634052fc9cf7758e08474d8c1f87d73d7b259a00e8a9e9cf36f70a9d397459`.
No gameplay edits or rebuild after acceptance. Runtime patch re-exported unchanged
and verified against the clean pin (51 files match). Implementation, tests,
manual and evidence included in authorized MusicMonsterMod closeout.
No next job started. Launch: `python3 recomp/tools/local/run.py`; automatic capture.

Candidate history below predates acceptance.


## 2026-10-23 - D08T5 dumpster ladder limit (Needs playtest)

User selected a new job and authorized immediate work. Current UI slot 7's
LEVEL00 dumpster centre is (7292,-9728,13829); Modern push now stops at X 7292,
1126 units before the former boundary. Held reverse and exact return verified;
other pushable support/collision checks and Vanilla retained. Native suites pass.
Real-input slot-7 approach catches the ladder and reaches climbing animation 187.

Existing west-side regrab refusal reproduces on both baseline and candidate:
that floor's original grab bit is clear. No new floor-rule exemption added.
Final spacing/ladder feel awaits user playtest. See [note 210](210-d08t5-dumpster-limit.md).
Candidate: `bf634052fc9cf7758e08474d8c1f87d73d7b259a00e8a9e9cf36f70a9d397459`.
All 36 intake source save files unchanged. No framework changes, commit/push or
acceptance closeout. Launch: `python3 recomp/tools/local/run.py`; automatic capture.


## 2026-10-23 - D08L1 accepted and closed

User: "it's definitely working. i'm happy to accept this lock it in and close it out".
D08L1 is Done: user gameplay confirmation closes the remaining forced-crouch
walk-off acceptance. Native/live tests, Vanilla comparison and unchanged object
edge restrictions are recorded in [note 209](209-d08l1-walk-offs.md). Automated
tests did not reproduce the exact slot-12 climb; that limitation remains recorded.

Accepted executable: `8de27e4cae38b5053c5b456c3fc60ef96a2d2a46df4630dbb7309a8c01a63097`.
No gameplay changes or rebuild after acceptance. Authored implementation, tests,
manual and evidence are included in the authorized MusicMonsterMod closeout.
No next job started. Launch: `python3 recomp/tools/local/run.py`; automatic capture.

Candidate history below predates this acceptance.


## 2026-10-23 - D08L1 walk-offs candidate (Needs playtest)

Standing walks now use the original fall path for larger drops at walking speed.
Modern crouch walking permits collision-checked departures and hands support loss
to the original fall, without snapping down to the floor. Native/loaded forced
kneels can acquire Modern crouch ownership; stand-up still checks clearance.

Build and controls/input/aim suites pass, including all 21 level guards. Accepted
binary comparisons reproduce standing/crouched platform stops and the crouched
bed-edge stop; candidate third-person 60 FPS and first-person 120 FPS routes fall
and land. Slot 9's block still stops at X 8700 and reverses; slot 12's dumpster
still stops at X 8418. All 15 checked player save/sidecar/thumbnail hashes match.

**Primary acceptance remains unverified:** real-input front/side approaches did
not complete the slot-12 mantle. Separate diagnostic top placements fall, but
do not establish the actual forced-crouch escape or ceiling-safe appearance.
The user confirmed they mantled onto it; gameplay acceptance is still needed. Do not mark Done or claim the exact
softlock resolved. See [note 209](209-d08l1-walk-offs.md) for evidence and limits.

Candidate: `8de27e4cae38b5053c5b456c3fc60ef96a2d2a46df4630dbb7309a8c01a63097`.
No framework/pin, media or player-settings edits; no acceptance closeout, commit
or push. Launch: `python3 recomp/tools/local/run.py`; automatic capture.


## 2026-10-23 - D08T3 accepted and closed; D08L1 Todo

User accepted Free Push after final testing of slots 9-12 and explicitly
requested closeout and commit/push. D08T3 is Done: free dumpster movement to the
ladder, box push/pull, stable enemy collision and supported-edge refusal all
confirmed. Accepted executable: `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
No further gameplay changes or rebuild after acceptance. Authored source,
native regression tests, manual and [note 208](208-d08t3-manual-push-pull.md)
are included in closeout; player saves and media unchanged.

**Next selectable job: D08L1 (Todo)** in [the job board](../MODERNIZATION_JOBS.md#d08l1---modern-locomotion---allow-walking-and-crouch-walking-off-ledges).
After pushing the slot-12 dumpster to the ladder and mantling it, low overhead
clearance forces crouching and Duke cannot crouch-walk off. Record this as a
separate locomotion softlock, not a Free Push defect. Investigate original vs
modern ledge restrictions, grounded/crouched support and falling transitions.
Allow standing and forced/voluntary crouched walk-offs while preserving object
ledge safety, jumps, mantles, collision and Vanilla. Not implemented or started.

Candidate history below predates this acceptance.



## 2026-10-23 - D08T3 candidate (Needs playtest)

Review revision: reproduced and fixed the held-W grab timing failure that
escaped the first pass, plus the dumpster crossing its ladder. Flat panels
and complete room object lists now block the dumpster before contact. Final
private routes on slots 9-12 pass held-W grabbing, neutral hold, reverse,
sustained boundary refusal and release; native regressions cover both bugs.
Earlier pause/focus/mantle evidence below is from the first candidate.

Manual held push/pull and supported-edge checks are built. W/S stop immediately
on release, Grab stays held, reverse works, and a blocked object stays held.
Slot 9 stops at its ledge; slot 8's already-floating block is not relocated.
Build/native suites and isolated gameplay checks pass for the supplied objects,
release/pause/focus, saved moving pose, repeated loads and E mantle. User feel,
sound and puzzle-completion acceptance remain. See
[note 208](208-d08t3-manual-push-pull.md) for evidence and limits.

Candidate: `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
No player-save/settings or framework changes. No acceptance closeout, commit or
push. Launch: `python3 recomp/tools/local/run.py`; automatic capture.


## 2026-10-23 - D26B accepted and closed

User accepted the view/control ownership candidate and explicitly requested
"lock it in, close it out". D26B is Done. The authored camera/input changes,
native regression tests, manual and evidence notes are included in the
MusicMonsterMod closeout. No new implementation or build after acceptance.

Accepted executable:
`a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec`.
[Note 207](207-d26b-view-ownership.md) records the ownership contract, verification,
deliberate overrides and remaining coverage limits. Runtime patch re-exported
unchanged and verified against the clean pinned framework (51 files match).
No framework/pin or player-save changes. No next job started.
Launch: `python3 recomp/tools/local/run.py`. Automatic capture, no F10 step.


## 2026-10-23 - D26B view/control ownership candidate (Needs playtest)

D26B now covers the user's requested ownership of view/control choices across
console, menus, focus, saves/loads and transitions. Reproduced console/pause
camera teardown on input release. Player preferences now survive inactive
snapshots; authenticated camera presentation is separate from action/capture
permission. Loads and level teardown discard old world-specific camera state;
newly acquired supported cameras start in the selected view, including jumps.

Build and native input/aim/controls suites pass, with all 21 level guards. Six
live first/third-person runs at 60/120/240 FPS cover console, pause, focus, F7,
cross-perspective loads and rapid load/Escape. Final live level transitions,
same-level restart and Vanilla save/pause checks pass. See
[note 207](207-d26b-view-ownership.md) for evidence and explicit coverage limits.
Legacy/Original camera and unsupported traversal retain deliberate overrides;
they do not overwrite the saved view. User gameplay acceptance remains.

Candidate: `a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec`.
No commit/push or acceptance closeout. No framework or player-save changes.
Launch: `python3 recomp/tools/local/run.py`. Automatic capture, no F10 step.

## 2026-10-23 - D17Y accepted and closed

User confirmed both slot-8 graphical oddities are completely resolved, fully
accepted the fix and explicitly requested "lock it in. close it out". D17Y is
Done. The missing-animation face filter and direct open upper portal correction
are accepted, with their authored implementation and regression tests included
in the closeout. See [note 206](206-d17y-slot8-rendering.md) for diagnosis,
validated build, capture matrix, native tests and coverage limits.

Runtime patch re-exported unchanged and verified against the clean pinned
framework. No new framework edits or player settings/save changes. No next
job started; wait for the user's selection. Launch:
`python3 recomp/tools/local/run.py`. Gameplay capture remains automatic.


## 2026-10-23 - D17Y rendering candidate (Needs playtest)

Both current slot-8 reports reproduced and corrected in focused captures.
The fire platform was covered by missing-animation faces sampling the live
framebuffer; Modernized now filters those faces from a private mesh copy.
The wall strips were clipped at an open upper portal boundary; Extended draw
distance now extends only the direct taller sky-room neighbor's top edge.
Downstream apertures, valid flame textures and original meshes are preserved.

Build and native near/controls regression suites pass. First/third-person,
4:3/16:9, 60/120 FPS captures pass. Saved Auto was checked offscreen, where
it resolves to 60 without a physical monitor. User playtest and physical-display
Auto remain. See [note 206](206-d17y-slot8-rendering.md) for diagnosis, private
save identity, executable hash, evidence and limits. Player profile and all
39 save/card files unchanged. No framework edits, commit or push.

Launch: `python3 recomp/tools/local/run.py`. Resume captures automatically;
no F10 step. Await the user's slot-8 retest before acceptance/closeout.


## 2026-10-23 - D04B accepted and closed; D17Y queued

User accepted D04B ("way better") and authorized the bounded follow-up and
implementation/documentation commit/push. A brief post-load Escape interruption
still recovers automatically; no claim that every split-second interruption is
gone. Quick win: confirmed menu exit bypasses the generic 12-frame capture
cooldown, while requiring a new authenticated gameplay offer. Build and input
regression suite pass; focused live timing checks are in [note 205](205-d04b-control-recovery.md).
Final executable: `41bb0257acb5bd2e89677b5062fad2c74da0ed0e94d08cca546ae9847dd564c5`.
Broader prior evidence and coverage limits remain in that note.

D17Y is Todo: current UI slot 8, jumping while looking at the fire platform to
Duke's right causes severe flicker; turning 90 degrees right and jumping makes
the upper castle walls disappear until landing. User report only; not started.
Choose a job before any further implementation. Framework patch re-exported
unchanged and verified against the clean pin (51 files). No player saves/cards
changed. Launch: `python3 recomp/tools/local/run.py`; no F10 step required.


## 2026-10-23 - D04B control recovery candidate (Needs playtest)

Quickload plus rapid Escape reproduced a permanent host capture hold on the
accepted D17S baseline. The candidate ends that hold on explicit menu
acknowledgement or fresh gameplay after the Start pulse. F10 is inert; no
implicit tank fallback. OPTIONS > CONTROLS > CONTROL SCHEME is the persistent
MODERN / LEGACY choice (schema 40, default Modern). Identity guards retained.

Build, native input/aim/controls suites (all 21 overlay guards), 100 Python
tests, 36 timing sequences across 30/60/120/180/240/Unlimited and both views,
real menu/save/load/console routes, and Vanilla save/pause checks pass.
[Note 205](205-d04b-control-recovery.md) records evidence and remaining player
acceptance / live death and traversal limits. Candidate:
`31b9ed43bc8738ad4eeb968af629d3be423ff2172a97cfa0d22305766bed950c`.
D17S remains the accepted baseline. No commit/push or player-save changes.
Launch: `python3 recomp/tools/local/run.py`. No F10 step required.


## 2026-10-23 - D19Q2 accepted (Done); D04B queued (Todo)

User accepted the paused-save investigation and explicitly requested commit
and push. [Note 204](204-d19q2-paused-saves.md) retains evidence and limits.
No gameplay changes or rebuild; D17S executable remains the baseline.

New separate report: loading then pressing Escape shortly afterward can lose
Modern controls. D04B is Todo: Modern default, persistent settings-only
Modern/Legacy choice, no implicit Legacy fallback, no F10 scheme switching or
manual recovery requirement, reliable pause/focus/load transitions. Detailed
acceptance is on the board. D04B has not started; choose it explicitly before
implementation. D19Q2 testing does not disprove this new timing report.

Documentation-only closeout authorized as MusicMonsterMod. Player saves/cards
unchanged. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-23 - D19Q2 paused-save investigation (Needs review)

No confirmed regression on the unchanged D17S executable `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
The reported UI slot 10 preserves pause on debug load, but quickload resumes.
SAVE GAME/F2, gameplay quicksave, F3/title loads, F7/debug and Vanilla routes
checked on private copies. Real keyboard loads resume and recapture correctly.
Legacy/debug paused saves without `resume 1` deliberately remain paused.
[Note 204](204-d19q2-paused-saves.md) records the matrix and limits.
No gameplay change, rebuild, player-save modification, commit or push.
Awaiting user review; do not mark Done or perform acceptance closeout yet.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17S accepted (Done)

User fully accepted AUTO and explicitly authorized closeout. D17S is Done.
New Modernized profiles use AUTO: a whole display-refresh divisor capped at
120 FPS; existing choices and Vanilla remain intact.
[Note 203](203-d17s-auto-frame-rate.md) retains evidence and measurement limits.
Accepted executable: `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
No gameplay changes or rebuild. Framework patch re-exported and clean-pin
verification matches all 51 changed/new files. Implementation, tests and
documentation included in the authorized commit/push as MusicMonsterMod.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17S AUTO frame rate (Needs playtest)

AUTO added to FRAME RATE and the launcher, default for new Modernized profiles;
existing saved choices retained. Compatibility-first target is the highest
display-refresh divisor at or below 120 FPS (180 Hz -> 90, 240 Hz -> 120),
unknown display -> 60. Live display changes re-evaluate the target.
Build, compiled rate/setter tests, 88 profile tests, isolated offscreen live
menu transitions and clean-pin patch verification pass. Physical monitor
pacing and user acceptance remain. [Note 203](203-d17s-auto-frame-rate.md).
Candidate: `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
Launch: `python3 recomp/tools/local/run.py --frame-rate auto` (saves AUTO).
No commit or push.


## 2026-10-22 - D17X accepted (Done)

User fully accepted both fixes and explicitly authorized closeout.
D17X is Done: boat-support flat host depth and native warped-quad triangles
for soil floor contact. [Note 202](202-d17x-posts-soil.md) retains test evidence
and limits. No further gameplay changes or rebuild. Accepted executable:
`e5a6a69cf7bf8ff9d72581ee2171b1b8c7f64546f71809b846b0b3de3f3e7b4a`.
Framework patch exported and clean-pin verification matches all 50 files.
Implementation, tests and documentation included in the authorized commit/push
as MusicMonsterMod. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17X soil revision (Needs playtest)

User confirmed slot 6 fixed. Slot 7 now reproduces as triangular soil cutouts
exposing brighter ground during movement; the earlier nonreproduction note
is superseded. Warped host quads retain the native diagonal and per-triangle
plane depth/contact correction. World ordering and the boat fix are retained.
Build and owned geometry/ordering tests pass, as do the GL batch tests.
Inspected 60/120 FPS, 16:9/4:3 routes and continuous redraw sequences retain
the soil edges; boat, minecart and six older scenes checked.
[Note 202](202-d17x-posts-soil.md) records details and limits.
Candidate: `e5a6a69cf7bf8ff9d72581ee2171b1b8c7f64546f71809b846b0b3de3f3e7b4a`.
Recheck UI slot 7, first person, slowly forward/back and sideways near the
soil corners. F10 if needed. Launch: `python3 recomp/tools/local/run.py`.
No acceptance closeout, commit or push.


## 2026-10-22 - D17X candidate (Needs playtest)

Boat-support white slivers reproduce on private UI slot 6 and are corrected
by honoring opaque host flat/gouraud depth in GL. UI slot 7 soil white corners
did not reproduce on the current baseline; their cause remains unestablished.
Build, compiled GL batch checks and owned near tests pass. Offscreen 60/120,
16:9/4:3 routes cover both saves and the preserved D17W cart; six older scenes
were inspected. [Note 202](202-d17x-posts-soil.md) records evidence and limits.
Candidate: `8ef0ab59d8889f80a1f89e58282c8e30a4c62a602359ebc34d3bd60ae060c4a3`.
Framework patch exported and verified against the clean pin (50 files).
Player saves/profile unchanged. No acceptance closeout, commit or push.
Player check: first person, slowly forward/back at the slot 6 posts and slot 7
soil corners. F10 if capture is needed. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17M1 accepted (Done)

User confirmed the ladder/platform fix and explicitly authorized closeout.
D17M1 is Done on user gameplay acceptance; [note 201](201-d17m1-ladder-platform.md)
retains automated evidence and the scripted traversal limitation.
Accepted executable: `1381185d45a6b0a68b5f8529fe6f4e812e5130c645a9a313319cb0d37186559f`.
No new gameplay changes or rebuild. Framework patch export unchanged;
clean pinned-dependency verification matches all 50 patched files.
Implementation, tests and documentation included in the authorized commit/push.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17M1 candidate (Needs playtest)

Plain ladder upper panels now share depth with the intervening platform while
retaining their original draw ordering. Authenticated type flags select the
correction, not a slot or mesh address. Original textures, Vanilla and climb
controls retain their existing paths. Build, renderer/trace tests and offscreen
60/120/180 camera sweeps pass; older-scene and performance checks recorded in
[note 201](201-d17m1-ladder-platform.md). Player ascent/descent acceptance remains.
Candidate: `1381185d45a6b0a68b5f8529fe6f4e812e5130c645a9a313319cb0d37186559f`.
Use updated UI slot 10 with Corrected textures; F10 if capture is needed.
Launch: `python3 recomp/tools/local/run.py`.
No framework edits, acceptance closeout, commit or push.


## 2026-10-22 - D17M accepted; D17M1 ladder backlog

User fully accepted D17M and explicitly authorized closeout, commit and push.
D17M is Done; accepted executable remains
`8a034f815fc3abf27c27f843dfcc27e01444b4a5e383fa177fe7370ed98412da`.
No rebuild or further gameplay edits. Framework patch export unchanged;
verification against the clean pinned dependency matches all 50 patched files.
[Note 200](200-d17m-platform-ammo.md) retains test evidence and limits.

Brief inspection of a private copy of updated UI slot 10 supports a separate
D17M1 Todo for upper-ladder/platform visibility. The compact-object depth gates
are a plausible lead, not an established cause. Board entry records save hash,
reproduction and acceptance. No implementation started. D19Q2 paused-save
investigation also remains Todo. Earlier D17G/H/I/J triage and D17M code/tests
are included in the explicitly authorized commit/push.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-22 - D17M platform ammo correction (Needs playtest)

Current reproduction is UI slot 10. The candidate gives thin overhead
platforms depth and narrowly extends compact depth coverage to walk-over
pickups in Corrected textures. Original textures and Vanilla keep their
existing paths. Owned-EXE renderer tests pass in five settings combinations;
primitive-trace tests pass in both texture modes. Offscreen captures cover
first person at 60/120/180, third person at 120, elevated pickup visibility,
and six older regression scenes. The final 180-target stationary samples
measure 178.6/s at the old table and 180.3/s at the platform, with zero dummy-
audio underruns. User ladder ascent/descent and gameplay acceptance remain.
Player saves/cards/profile unchanged. No framework/generated-source edits,
commit or push. [Note 200](200-d17m-platform-ammo.md) records bounds and evidence.
Candidate: `8a034f815fc3abf27c27f843dfcc27e01444b4a5e383fa177fe7370ed98412da`.

Added D19Q2 Todo at the user's request for recurring paused-save behavior.
It remains deferred; diagnostic loading into a pause menu alone does not
prove a player SAVE/LOAD regression. No other job started.

Load slot 10 with Corrected textures for playtest:
`python3 recomp/tools/local/run.py` (F10 if mouse capture is needed).

## 2026-10-22 - D17J complete, no action necessary

The user confirmed the apartment, alley platform, pavement, club stairs and
bar-stool environments are all stable and explicitly requested completion
with no action necessary. D17J is Done on user gameplay evidence. Frame rate
and camera were not reported; no specific resolving change is established.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. D17M is the next
unfinished D17 report for manual review; implementation has not started.


## 2026-10-22 - D17I complete, no action necessary

After receiving the manual Unlimited camera test instructions, the user
explicitly requested completion with no action necessary. D17I is Done
on user acceptance. No detailed test observations or settings were supplied;
no measured timing improvement or specific resolving change is claimed.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. No next job started.


## 2026-10-22 - D17H complete, no action needed

The user could not reproduce the old train-control and platform/ledge stutters
after receiving the manual test route, reporting "play is totally smooth".
They explicitly requested completion with no action needed. D17H is Done
on user gameplay evidence. Frame rate was not reported; no specific resolving
change is established. No gameplay changes, rebuild or automated tests.
The accepted D12D executable remains the baseline. Documentation only;
no commit or push. No next job started.


## 2026-10-22 - D17G complete, no action needed

The user retested the old strip-club translation-jerk report on the current
player build and could not reproduce it, reporting perfect graphics and
stability in that area. They explicitly requested D17G be marked complete
with no action needed. D17G is Done on user gameplay evidence; frame rate
and camera were not reported, and no specific resolving change is established.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. No next job started.


## 2026-10-22 - D12D accepted (Done)

User: "fully accepted!! lock it in, and close us out". Upgraded first-person
weapon framing is accepted. Implementation, tests and documentation commit/push
explicitly authorized. Accepted regression baseline:
`bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0`.
[Note 199](199-d12d-upgraded-viewmodels.md) retains tests and coverage limits.
No gameplay rebuild; framework export unchanged and verified against the clean
pinned dependency (all 50 patched files match). No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D12D upgraded viewmodels (Needs playtest)

Challenge Stage 4's held record 29 had no first-person framing or firing
seed. The three separate upgraded records now share their base presentation
slots (27/28/29 -> 9/7/8), with original weapon gameplay records untouched.
Candidate `bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0`.
Build and native input/aim/controls suites pass; live offscreen captures cover
stage 4 at 60/120 Hz, stages 23/25, all six campaign upgrades standing/firing/
crouched, and third-person/Vanilla exclusion smoke checks.
[Note 199](199-d12d-upgraded-viewmodels.md) records evidence and limits.
User visual acceptance remains. No framework changes, commit or push.
Launch: `python3 recomp/tools/local/run.py`; P first person, F10 if needed.


## 2026-10-22 - D08K5 and D08K accepted (Done)

User: "total acceptance! lets lock it in and close this one out!!!"
Final coverage accepted; D08K5 and the D08K crouch-walking parent are Done.
Documentation, commit and push closeout explicitly authorized. No gameplay
changes or rebuild. Accepted executable remains
`c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
[Note 198](198-d08k5-crouch-coverage.md) retains verification and coverage limits.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D08K5 coverage ready for final acceptance

D08K5 Needs playtest (final acceptance), D08K still awaiting it. No gameplay
changes; accepted D17R2 executable remains `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
Fresh multi-era/camera, combat, saves, traversal and death/Continue checks,
all three native suites and reviewed Vanilla route pass. Prior terrain
coverage and fixture-only low-ceiling evidence are distinguished in
[note 198](198-d08k5-crouch-coverage.md). User reports extensive happy use.
No request for a full campaign replay. No acceptance closeout, commit/push
or next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17R2 accepted (Done)

User fully accepted D17R2 and explicitly requested closeout. Accepted executable
`c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
The angle-dependent black sky is fixed; [note 197](197-d17r2-angle-dependent-black-sky.md)
records implementation, tests and coverage limits. Implementation, tests and
documentation commit/push authorized. Framework export unchanged; verified
against the clean pinned dependency, all 50 patched files match. No rebuild
for documentation closeout. No next job started; wait for user selection.


## 2026-10-22 - D17R2 sky-pass omission (Needs playtest)

Current UI slot 8 is LEVEL06 room 28. Turning away from a sky-flagged
neighbour culls that neighbour and skips the entire original sky pass.
Modernized now draws the loaded sky background independently of that flag,
after its original far-limit use. No world visibility or Vanilla changes.
Candidate `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
Build, native sky/near contracts, current-slot 60/120 motion and angle sweep,
4:3/third-person/original precision-distance samples and previous sky views
pass. [Note 197](197-d17r2-angle-dependent-black-sky.md) records evidence and
limits. User slot-8 left/up playtest remains. No framework changes or commit/push.
Launch: `python3 recomp/tools/local/run.py` (F10 if capture is needed).


## 2026-10-22 - D08AE accepted; D17R2 queued

User fully accepted D08AE after testing the reported deep-water locations
and shallow water: "all solid". D08AE Done; accepted executable
`c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.
[Note 196](196-d08ae-deep-water-entry.md) retains tests and coverage limits.
Commit/push of authored implementation and documentation explicitly authorized.
Framework export unchanged; patch verified against the clean pinned dependency
and all 50 patched files match. No gameplay rebuild for closeout.

D17R2 Todo: current UI slot 8, look left then slightly upwards; sky turns
black. [Note 197](197-d17r2-angle-dependent-black-sky.md) records source-only
findings, candidate matrix/clipping/packet paths, prior D17O/D17R/D17R1 work
and acceptance criteria. No live reproduction or confirmed cause, no rendering
code changed, no build/launch. Wait for selection before implementation.


## 2026-10-22 - D08AE deep-water entry (Needs playtest)

Deep entry now releases shallow-flight protection when native queried local
volume depth exceeds 640. Original body/neck entry and effects take over;
no new drag constants, velocity writes, hooks or framework edits.
Candidate `c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.
Private slot 9/2 entry matrix and 24 verified-depth routes pass, together with
shelf/slope, gait/crouch, swim-fire/dive/ascent, ORIGINAL and Vanilla checks.
Build, native controls over 21 overlays, input/aim and hook contracts pass.
All 39 original player manifest files unchanged. [Note 196](196-d08ae-deep-water-entry.md)
records evidence, sampling limits and an existing crouch-branch warning.
User entry-feel review remains; D08AD stays the accepted baseline.
No acceptance, commit/push or next job. Launch from this workspace with
`python3 recomp/tools/local/run.py`, test pool UI slot 9 and moat UI slot 2.


## 2026-10-22 - D08AD accepted; D08AE queued

User accepted D08AD: "the animation is fixed. lock it in and close it out."
The accepted executable remains
`9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
Implementation, documentation, commit and push authorized after creating
D08AE: restore deep-water entry resistance and timely swim handoff.

D08AE stays Todo, investigation only. Private current UI slots 9 (pool) and
2 (moat) reproduce submerged airborne descent almost to the bottom under
MODERN. ORIGINAL enters swimming much earlier. Cached support equals water
surface, despite deep native volume: D08AC's shallow-flight bypass predicate
is strongly implicated. No deep-water fix applied; do not automatically start
it. [Note 195](195-d08ad-deeper-water-animation.md) and
[note 196](196-d08ae-deep-water-entry.md). Prior tests of already-swimming
states did not establish entry fidelity. Framework export unchanged and
verified against the pinned clean dependency; no gameplay rebuild needed.


## 2026-10-22 - D08AD deeper-water animation fix (Needs playtest)

Walking's terrain probe invalidated the D08AC patch scope before native
dispatch, resetting the walk clip every update. A separate native completion
selector restarted the same running stride half. Arm the scope after the
probe and retain native land stride choices at completion. No physics or
backpack-specific changes. [Note 195](195-d08ad-deeper-water-animation.md).

Candidate `9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`:
full 72/74 and 76/78 cycles at 60/120 Hz, private updated slot-10 route and
visual captures; native controls over 21 overlays, input, aim and hook-contract
tests pass. Paired jump heights, crouch, deep swimming, ORIGINAL and Vanilla
regressions checked. All original save/card hashes unchanged. User playtest
pending; no acceptance, commit or push. Existing framework edits preserved.


## 2026-10-22 - D08AD queued (backlog/source investigation only)

Added one Todo follow-up for frozen third-person walking and abbreviated
running in the user's updated deeper-water UI slot 10. D08AC movement/jump
physics and D08K11 crouching remain accepted. [Note 195](195-d08ad-deeper-water-animation.md)
records depth classification, legacy 80/81 conversion and possible repeated
track resets; these are source findings and hypotheses, not a reproduced
root cause. No gameplay edits, build, launch or save/card access. Implement
only when selected; no commit/push authorized by this backlog request.


## 2026-10-22 - D08AC accepted (Done)

User: "perfect!!! lock it in and close us out". Grounded shallow/mid-water
land movement, jumps, air control and landing accepted. SWIMMING MODERN
uses shared land mechanics; ORIGINAL retains its legacy wade jumps and
ledge assistance. Deep swimming and Vanilla remain unchanged.
[Note 194](194-d08ac-shallow-water-locomotion.md) records implementation,
paired traces, native suites and live regressions. Executable
`30ca9b3fa5bd0a6350dd8b7ef1a509914ecfb3e60dcf74ec9ebba0982b7eb838`
is the accepted regression baseline. Broad campaign/shoreline coverage
remains an ongoing regression concern, not a claim of exhaustive testing.
Implementation and documentation commit/push authorized. No next job started.


## 2026-10-22 - D08K11 accepted (Done)

User: "awesome! i accept the behaviour here, very good. lock it in and close it out."
Shallow/mid-water Modern crouch and crouch walking accepted; documentation,
commit and push authorized. [Note 193](193-d08k11-shallow-water-crouch.md).
Executable `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8`
is the accepted regression baseline. D08K5's broad collision/campaign sweep
remains separate. No next gameplay job started.

Closeout CI sweep: [run 39793587393](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/runs/39793587393)
failed importing `ttk_anim` because NumPy was not installed. Local requirements
now include NumPy/Pillow and CI installs them before testing. A clean source-only
checkout with a fresh Python environment passes 180 tests (23 owned-input/tool
skips), the media-validator build, and all three framework regression scripts.
The exported framework patch is unchanged and applies idempotently to the pinned
clean dependency. This CI fix changes no gameplay or accepted executable.


## 2026-10-22 - D08K11 shallow-water crouch (Needs playtest)

Modern crouch/crouch walk now works in grounded shallow and mid-depth water,
including native mode 1; the original mid-water stand-up veto is bypassed
only during an owned crouch dispatch, then restored. Deep-water handoff
uses the original clearance-checked stand-up and native swim rules.
Build and native input/aim/controls pass; private slot 10, direction/fire/
stand-up checks, both cameras and water options, controlled depth fixtures
and separate swimming/Original/Vanilla regressions completed. Natural
shoreline and player feel review remain. [Note 193](193-d08k11-shallow-water-crouch.md).
Candidate `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8`.
D08K10 remains the accepted baseline. No commit/push or next job.


## 2026-10-22 - D08K10 accepted (Done)

User: "perfect!! lock it in, close it out. great work". First-person crouched
weapon alignment, including revision 2's high-refresh crouch walking, accepted.
Documentation, commit and push authorized. [Note 192](192-d08k10-crouched-viewmodel.md).
Executable `72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`
is the accepted regression baseline. No next job started; no rebuild needed
for documentation closeout.


## 2026-10-22 - D08K10 revision 2: replay crouch poses (Needs playtest)

User confirmed idle but reported bad weapon poses while crouch walking.
Reproduced at 120 Hz: render workers lacked the crouch eligibility state
and used stale fork-time values. Each published frame now includes it.
Player build and native controls pass, including a forked-worker snapshot
regression. Actual presented captures verify all six weapons while moving
forward/sideways/backward. The earlier high-refresh verification claim
was too broad; this revision checks the worker's visible output.
[Note 192](192-d08k10-crouched-viewmodel.md). Candidate
`72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`.
User retest pending. No framework edit, commit/push or next job started.


## 2026-10-22 - D08K10 crouched first-person weapons (Needs playtest)

Animated weapons and held items now read the right arm's animation channel
for the eye-view copy during Modern crouching, avoiding the crouch body's
shoulder compensation. Standing and seeded guns retain their framing;
third-person animation, Original crouch and Vanilla paths are unchanged.
Build and native input/aim/controls pass; private 60/120 Hz weapon checks
completed. User review of appearance and transitions remains.
[Note 192](192-d08k10-crouched-viewmodel.md). Candidate executable
`fd7ed0ef639a96d79cfdb27b4cf18e2c72e3128a856575e125198f4418abced0`.
D08K9 remains the accepted baseline. No commit/push or next job started.


## 2026-10-22 - D08K9 accepted (Done)

User: "lock it in, close it out!" Held crouch on touchdown and the final
CROUCH-toggle/menu revision accepted. LANDING is hidden from Advanced;
crouch-on-landing follows CROUCH alone. Documentation, commit and push
closeout authorized. [Note 191](191-d08k9-crouch-on-landing.md). Executable
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`
is the accepted regression baseline. No next job started.


## 2026-10-22 - D08K9 crouch on landing (Needs playtest)

Crouch held through a jump takes over in the soft touchdown update, with
a flight-to-kneel/walk blend and no recovery wait or intermediate gait.
Private slot 4: 24/24 cases across both jump styles/cameras at 120 Hz;
60 Hz follow-ups, release/late press and damaging-fall fixture pass. Native
input/aim/controls and player build pass. User review of feel is pending.
[Note 191](191-d08k9-crouch-on-landing.md). Candidate executable
`5bff834e83785cb7d7023001198aebba1d66df881de7d05cbe13d051b081b091`.
D08K8 remains the accepted baseline. No commit/push or next job started.
Functional behavior accepted by the user; follow-up hides LANDING from
Advanced and makes crouch-on-landing follow CROUCH alone. Build, native
controls and live LANDING ORIGINAL check pass. Revised candidate
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`.


## 2026-10-22 - D08K11 opened (Todo, backlog only)

[D08K11: shallow-water crouching](../MODERNIZATION_JOBS.md#d08k11---enable-modern-crouching-and-crouch-walking-in-shallow-water)
records the user's slot 10 flooded-subway report and a source-only investigation.
Crouch entry and walk eligibility explicitly reject the broad water predicate;
176 also serves as a water-dip animation. Depth/state, collision, input ownership
and head-submersion handling need a later private-copy runtime investigation.
No gameplay changes, build or launch; the D08K8 accepted build remains current.
Include D08K11 before D08K5's final coverage; no implementation job started.


## 2026-10-22 - D08K8 accepted (Done)

User: "this is perfect!! lock it in, close it out". Stationary kneel and
transitions accepted; closeout authorized. [Note 190](190-d08k8-stationary-kneel.md).
Executable `d798757cb2de56f946074165423faea379a1711d51f81b60cf00c70d8780e1f3`
is the accepted regression baseline. Next suggested: D08K9, D08K10, then D08K5.
No next job started; no rebuild needed for documentation closeout.


## 2026-10-22 - D08K8 stationary kneel (Needs playtest)

CROUCH MODERN now holds the original one-knee pose, blending from the last
shown pose into and out of walking. Saved animation tables refresh on load.
Build and focused tests pass; private 60/120 Hz checks completed. User visual
acceptance and unarmed gameplay review remain. See [note 190](190-d08k8-stationary-kneel.md).
Executable `d798757cb2de56f946074165423faea379a1711d51f81b60cf00c70d8780e1f3`.
No commit or push; D08K7 remains the accepted baseline.


## 2026-10-22 - D08K7 accepted (Done)

- User: "confirmed working! lock it in and close it out"
- D08K7 Done (note 189). Executable
  `a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963` is the regression baseline. Next suggested: D08K8, D08K9, then D08K5.

## 2026-10-22 - D08K7 stuck crouch on objects (Needs playtest)

- Slot 10 dumpster: crouch walks over the top and stops at the edge where the
  standing walk stops; releasing Ctrl stands Duke; W never rolls in CROUCH
  MODERN. Objects are judged as the original walker (`0x8007926c`) judges
  them; a refused stand-up keeps the crouch walk on the directions and stands
  Duke once there is room. Note 189. Executable
  `a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963`.
- Next suggested: D08K8 (kneel as the still crouch), D08K9, then D08K5.

## 2026-10-22 - Crouch refinements D08K7-D08K9 opened (backlog only)

- User report and requests, research only, no code changed. D08K7: Ctrl
  released on the slot 10 dumpster leaves Duke kneeling and W rolls. Likely
  cause: `crouch.inc` refuses stand-up whenever `player+0x174` (object
  contact) is set, and with Ctrl released the original handler gets the
  directions and rolls. D08K8: the one-knee kneel (178) as the still crouch;
  D08K4 revision 2 replaced it because of a kneel/walk pop, which must be
  solved. D08K9: crouch held through a jump; `landing_recover()` skips
  crouch, so 105 or the run gait plays before 176. Suggested order K7, K8, K9,
  then D08K5.

## 2026-10-22 - D08K6 accepted (Done)

- User: "that's it!! i'm happy with this end to end. the anim is now fully complete. the technical implementation is now fully complete. this is totally playable in every way at this point. lock it in, close it out"
- D08K6 Done (note 188). D08K waits only on D08K5. Executable
  `789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03` is the
  regression baseline.

## 2026-10-22 - D08K6 revision 4: variant C, head up, hunched firing (Needs playtest)

- Default crouch is variant C. Head looks ahead (an arm-range bug left it on
  the kneel's head when armed). Firing keeps the hunch: the chest's aim
  look-at is tilted down 50 degrees and the shoulders compensate, so the gun
  stays on the crosshair. Note 188 revision 4. Executable
  `789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03`.

## 2026-10-22 - D08K6 revision 3: crouched shots aimed up fixed (Needs playtest)

- While firing, the original aim build points the chest at the view; our lean
  and arm compensation on top tipped the gun 49-58 degrees up. The upper body
  now uses its channels' own pose while the aim bit is set. Crouched fire is
  level, also while crouch walking. Note 188 revision 3. Executable
  `cd48d3e3d7cd5f0b35f3364b2179d0a4209b1ca3ac79b67ec6354312f92db900`.

## 2026-10-22 - D08K6 revision 2: arms straight down after crouch fixed (Needs playtest)

- Armed crouch left both arms at identity in every later animation: channel
  joint masks grew (left arm/chest channels gained the right arm and chest)
  and the weapon-arm compensation compounded on stale records. Masks trimmed
  to each channel's original joints; weapon arm from its channel frames.
  Note 188 revision 2. Executable `6ca0122ee58dce894263394fa0d1ba7623763e8bba5aa36f6967bf9c6d77dbcf`.

## 2026-10-22 - D08K6 tactical crouch walk (Needs playtest)

- New crouch-walk look: hips higher (0.62 leg lengths), feet under the hips,
  chest 48 degrees over the knees, head level, fists up front; upper body
  written after every blend, so the armed left-arm shake (animation 178's
  upper channels churning) is gone. Note 188 also records everything needed
  for future custom animations and models. Variant packs a/b/c/d under
  `recomp/analysis/d08k6/packs/` for `DNTTK_ANIM_PACK`.
- Executable `dda1d28f2626ffa5b0464d0adcc83bb99483dc3879ef9e1009069d342382a3be`.
  Limits: pistol idle hangs at the thigh; first person not judged.

## 2026-10-22 - D08K1-D08K4 accepted (crouch walking, technical)

- User: "now it's functionally working. the job is fulfilled on a technical
  level ... close out the done tasks". D08K1-D08K4 Done (notes 184-187).
  Executable `993c2c705d1f86fa27c1608ed92ba0574a1ade50097c909918b656b65e1c46c5` is the regression baseline.
- D08K6 opened (Todo): crouch-walk look (Solid Snake, not "on the toilet") and
  the slight left-arm shake while crouch walking.

## 2026-10-22 - D08K4 Modernized crouch walking (Needs playtest)

- CROUCH MODERN (default) walks low with held crouch plus a direction, Duke
  facing the view; CROUCH ORIGINAL rolls; Vanilla unchanged. Profile schema 39.
  Clips built from the disc at build time. Note 187. Executable
  `862d7b9b8a98c91c55427ada9a49e402ef2d3083be99450a5e7475e02fc81c32`.

## 2026-10-22 - D08K3 crouch-walk loops (Needs review)

- New IK gait for a crouch walk (the original has only a kneel): forward,
  backward and strafes, planted feet (slide under 2 units vs 23-40 in the
  original walks), footstep events, generated locally from the disc by
  `ttk_crouch_walk.py`. In-game preview by pack; Blender file for review.
  D08K2 hull raised to 0x2c0 for the walk's 654 height. Note 186.

## 2026-10-22 - D08K2 crouch-height collision (Needs review)

- A crouch probe asks the original move queries (object probe, floor and
  ceiling, head ray, slope) with a 0x2c0 crouch hull instead of the standing
  0x370, refuses steps up or drops over 0x100, and follows the floor. Debug
  console `crouchwalk` / `crouchgo` (test mover) and `crouchscan`; Modernized
  only, nothing changes without them. Stand-up keeps the 0x370 rule.
- Finding: no crouch-only ceiling or overhang at any selectable level start
  (levels 0-3, 5-12); the walkthrough agrees, so the hull is a safety rule and
  the feature's value is precise combat movement. Live: walls, props, curbs,
  stairs and two edges behave; clearance gate shown with a test hull; native
  fixture covers the crouch-only case. Note 185. Executable
  `3e58625a743856ff8e1832cfc5baac9c86e5d797b80cf2fa21e1f94e6d61b8e0`.

## 2026-10-22 - D08K1 animation pipeline (Needs review)

- Duke's skeleton, meshes, textures and animations are decoded from the disc
  (HIE/MOD/ANM/FRM/ROT, A/B.RAW) by `recomp/tools/local/ttk_anim.py`; every
  clip of DB00-DB09 (9,481) re-encodes byte for byte. An animation's four
  clips are body channels (legs/root, right arm, left arm, chest+head), so a
  crouch walk needs only a new legs clip.
- Renders and offline FK match live memory (joints within 4.4 units, blended
  quaternions within 8/4096). Blender bridge (`ttk_anim_blender.py`) exports
  rigged, textured Duke with clips as actions and imports edits back.
- Runtime override packs (`recomp/src/ttk/anim_override.cpp`, Modernized
  only, `DNTTK_ANIM_PACK` or `ttk-anim.pack`; none ships): a Blender
  re-encoded idle plays within 2.39/4096 of the original; a demo pack makes
  Duke idle kneeling; savestates adopt or restore it. Vanilla unaffected
  (route reviewed). Note 184. Executable
  `5998cf36936d588c5433f3cc5e9cc001308a8717e78a4b9388ec029f3f6d3cfe`.

## 2026-10-22 - D20E accepted (ENEMIES KILLED X OF Y)

- User: "lock it in, this is complete. close it out". D20E Done. Note 183. Executable
  `156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef` is the regression baseline.

## 2026-10-22 - D20E statistics research: ENEMIES KILLED X OF Y (Needs playtest)

- The statistics screen's kills line now reads `X OF Y`. Y uses the level
  load's own rule for its enemies-left count (placement table, same totals on
  7 levels and on every difficulty); the live count is not used because
  dancers are counted but never subtracted. Kills survive death and Continue.
- Accuracy dropped by the user (shots `+0x3b2` and hits `+0x3b4` documented
  only); damage taken parked (no original counter). Note 183.
  Executable `156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef`.

## 2026-10-21 - D18D accepted (music after Continue)

- User: "i can confirm the music started with no further issue. lock it in,
  close it out". D18D Done. Note 182. Executable
  `23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a` is the regression baseline.

## 2026-10-21 - D18D music after Continue (Needs playtest)

- Cause: the runtime finished the near CD seek at Continue (drive paused a
  few sectors from the music's resume point) in 0x800 cycles, inside
  libcd's `CdControl`, before Time to Kill installed its seek callback, so
  the music player stuck in state 1, CD muted, until a voice line.
- Fix in the framework (`cdrom.c`, exported patch): near SeekL/SeekP use
  Beetle's seek time (min 20000 cycles, plus the paused-drive restart).
  Modernized and Vanilla resume music two frames after CONTINUE; level-start
  voice-to-music and intro FMV unchanged; about 37 ms more per level load.
  Not yet heard on the real audio device. Note 182. Executable
  `23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a`.

## 2026-10-21 - D11G cancelled (cannot reproduce)

- User: "cancel the job. i cant seem to replicate it either now. if it comes
  back i'll let you know but close it as cant replicate". No code changed.
- Six routes from a private copy of UI slot 11 all started the next level in
  first person with no input (debug load, pause LOAD GAME, title LOAD GAME,
  F3, statistics SAVE GAME then CONTINUE; the player's profile at 120 fps in
  a window and on the real GPU). If it returns, keep the session log: the
  likely causes are capture-related (focus lost during the load, an F10
  opt-out) or a level-start state that holds the camera lease inactive.
  Regression baseline unchanged:
  `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.

## 2026-10-21 - D20D accepted (richer statistics); D11G opened

- User: "accepted! ... close this work out, fully accepted". D20D Done.
  Note 181. Executable
  `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d` is the regression baseline.
- D11G (Todo, not actioned; next up): a new level can start in third person
  although first person is selected (player UI slot 11, next level); a left
  click brings first person back.

## 2026-10-21 - D20D richer statistics screen (Needs playtest)

- Approved layout: level name, DIFFICULTY:, kills, time, BEST TIME (NEW
  BEST! when beaten), continues, secrets, MISSION ITEMS, challenge item,
  HEALTH / ARMOR. Best times per level and difficulty in
  `ttk-best-times.txt` beside the savestates. `!` and `%` now drawn in the
  game's own small font; the font picker shows ( ) % and the double quotes
  (section 16). Offscreen checks pass, Vanilla unchanged. Note 181.
  Executable `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.

## 2026-10-21 - D20F accepted (statistics screen list)

- User: "thats so much better! lock it in, close it out, 10/10". D20F Done.
  Note 180. Executable
  `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496` is the regression baseline.
  D20D (richer statistics, builds on this layout) and D20E stay Todo.

## 2026-10-21 - D20F statistics screen list (Needs playtest)

- CONTINUE and SAVE GAME are a selectable list under the statistics lines
  (lines moved up 25); Up / Down / wheel, Enter chooses, V still saves;
  Escape or a save returns on SAVE GAME; CONTINUE only before the final
  ending. The original's wait is about 1 s, not 5 s (note 179 corrected).
  Offscreen checks pass, Vanilla unchanged. Note 180. Executable
  `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496`.

## 2026-10-21 - D20B accepted (statistics screen); D20F opened

- User: "i can confirm its now working and positioned correctly with the overlap ... i think we can close out the current work, just to break this task down a bit, as it is indeed working fine now." D20B Done. Note 179. Executable
  `f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.
- D20F (Todo, not actioned): CONTINUE and SAVE GAME as a selectable list on
  the statistics screen (V is legacy and not discoverable on keyboard and
  mouse). D20D (richer statistics) and D20E (research) stay Todo; D20D now
  builds on D20F.

## 2026-10-21 - D20B first playtest: SAVE GAME date overlap fixed (Needs playtest)

- User: works functionally; on SAVE GAME from the statistics screen the date
  overlapped the line under the picture (their 1444x1020 window). The host
  image used the 16:9 fit while menu frames there are presented at 4:3; it now
  follows the presented rect (framework, patch exported). Also covers the
  title LOAD GAME in tall windows. Note 179. Executable
  `f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.

## 2026-10-21 - D20B statistics screen rebuilt (Needs playtest); D20D, D20E opened

- Option B, part 1 (user: remove the built-in save, "lets go for option B"):
  after a one-player level the statistics screen is ours in the original's
  look; V (SAVE) opens SAVE GAME (savestate slots), never the memory card;
  CONFIRM goes on. Saves there show the level's last picture and LEVEL
  COMPLETE on LOAD GAME and load back onto the screen. Vanilla and two-player
  unchanged. Hooks `0x8001FCBC`, `0x800270E8`; codegen hash unchanged.
  Note 179. Executable
  `b217e8898cbfa52a2baabe0efa01d13a3cbea83d9b016950fff636a6757bbbf4`.
- Testing aid: console `endlevel` ends the level as its exit does. Executable
  `c4bc40729ab7a1efaf7ac90b3ec8492bbdc7bd2a7ad2c9cf123c574a182bc82a`.
- Playtest: backtick, `endlevel`; wait for the prompts, V to save, Escape /
  save back, Enter to continue; then load that slot from LOAD GAME.
- D20D (richer lines, best times) and D20E (enemy total, accuracy, damage
  research) opened, not actioned.

## 2026-10-21 - D20C and D20A accepted

- User: "rock solid. it's working! the artifacting is no longer present upon
  load. i recreated the steps here and it's all fine. lock it in, close it
  out." D20C and D20A Done (one cause: savestates now carry mod memory).
  Note 178. Executable
  `03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`.

## 2026-10-21 - D20C wrong colours after a load fixed (Needs playtest); D20A same cause

- Cause: savestates did not carry mod memory (Expansion 1 and the GPU-DMA
  aperture). The first-person near clip's packet arena is in the aperture and
  linked from the game's ordering table, so a load could walk another frame's
  packets: scrambled textures, wrong colours kept in VRAM, or a GP0 halt.
  D20A's halting list `0x80F40BD0` is in that arena: same bug.
- Fix: framework section `BS_SEC_MODMEM` (restored on matching layout; the
  aperture is otherwise filled with end-of-list headers, also for older
  saves); near clip reserves its arena at startup and resumes past the
  loaded region. Codegen hash unchanged. Note 178. Executable
  `03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`.
- Playtest: load the player's slot 1 several times (F9 / LOAD GAME), in first
  person; also F6 right after F9, restart, load.

## 2026-10-21 - D20 accepted (quick save and quick load); D20C opened

- User: "its all working!" D20 Done. Note 177.
- D20C (Todo, not actioned): colours sometimes wrong after loading a
  savestate (their slot 1, intermittent; "try loading several times").

## 2026-10-21 - D20 quick save and quick load (Needs playtest); D20A, D20B opened

- EDuke32-style, Modernized one-player: F6 quick save over the last slot
  used (saved to or loaded from; none at launch: SAVE GAME opens), F9 quick
  load (no slot: LOAD GAME), F2 / F3 open SAVE / LOAD GAME from play (Escape
  back to play), console `quicksave` / `quickload`, KEY BINDINGS SAVES rows,
  launcher schema 38. F6 port swap off in Modernized. Note 177. Executable
  `20f8edc1d1d6260e59747df7d31ae4a438f4081a8eb19ee402ea9924f1b3e930`.
- D20A (framework, pre-existing): a save within ~1.5 s after a load can halt
  the emulator when loaded after a restart (GP0 0xFA). D20B: the original
  memory card saves, split from D20.

## 2026-10-21 - D19T1 accepted (no quotes on DELETE SAVED GAME:)

- User: "perfect. again lock it in close it out." D19T1 Done.

## 2026-10-21 - D19T1 no quotes on DELETE SAVED GAME: (Needs playtest)

- Ad hoc user request: the slot name line on DELETE SAVED GAME: (LOAD GAME
  and SAVE GAME) is drawn without quotes. D19T quote glyphs kept. Built, not
  launched.

## 2026-10-21 - D19T accepted (real double quotes)

- User: "that's awesome. lock it in close it out." D19T Done: real opening
  and closing double quotes in the menu font and the font pack (steel name
  line on DELETE SAVED GAME:). Note 176.

## 2026-10-21 - D19T real double quotes in the menu font (Needs playtest)

- `"` in Modernized menu text draws as real double quotes in both menu
  sizes, built from the font's own apostrophe (opening one turned half
  round), and in the font pack (Console steel: the DELETE SAVED GAME: name
  line, after the user's first playtest). Test with `DNTTK_TEST_MENU_TEXT`.
  Note 176. Executable
  `52f5f1a3ae67040f6e9d52d544ba914e7115c1e38985a23ce066937c22aea29c`.

## 2026-10-21 - D19S accepted (delete saves from LOAD GAME); D19T opened

- User: "perfect. lock it in." Delete on a used LOAD GAME / SAVE GAME slot
  asks DELETE SAVED GAME: with the name in Console steel (NO first); YES
  removes only that slot's files. Note 175.
- D19T opened (Todo): real double quote marks in the game's menu font.

## 2026-10-21 - D08Z5 accepted

- User: "perfect!!! lock it in close it out." D08Z5 Done; executable
  `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da` is the regression baseline.

## 2026-10-21 - D08Z5 second playtest: slope jump climbs (Needs playtest)

- User: "i dont like how this feels ... i prefer the idea of when jumping again, duke is actually trying to jump up a gradient slope a little higher ... What we've got right now is that dukes jump feels almost cancelled out". The Modern slope jump (98) never rose: Duke took off
  pressed into the face, so the air sweep `0x8007a98c` called it floor on the
  first update and landed him. Modern now keeps the jump's rise, drifts up the
  slope only as fast as the arc clears the face (k 0.25 of the rise against
  the face's tilt `+0x2be`), and while rising tests each update with the
  original sweep (drift, straight up, or lifted up to 160 units). A jump at
  the bottom of the face gets the same rise. Slot 10 car: jumps rise about
  430 units and land 90 to 390 higher on the face; tapped runs ended on the
  car in 3 of 6. Executable `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da`. [Note 174](174-d08z5-wall-backflip.md).

## 2026-10-21 - D08Z5 first playtest: backflip still there, fixed (Needs playtest)

- User: "this absolutely doesnt seem to have provided any kind of fix ...
  there is even a chance duke will slip out of first person". The player's
  logs showed animations 99/100, not the launch the first delivery removed:
  Jump pressed while Duke slides on the car face, facing up the slope, makes
  the slide handler's jump branch (`0x800547ac`) start the backflip 99/100
  (up to about 2400 units back). The first harness only tapped Space before
  the landing, so it never reached it. Modern now starts the original's own
  slope jump 98 in Duke's facing instead (armed by the branch's facing test
  `0x8005446c`, ra `0x800547fc`; rewritten at the initializer `0x800493a4`).
  The camera lease now also covers 99/100 from mode 0 or 9 (it dropped first
  person in ORIGINAL). Mashing Space into the slot 10 car: 18 Modern runs, no
  99/100, 17 slide jumps turned into 98; ORIGINAL keeps the backflip with the
  camera held. Executable `cd458c38c3ab3b22658ef1b0896fc3bed36df2cf02a8976ce391d2514bd261d2`. [Note 174](174-d08z5-wall-backflip.md).

## 2026-10-21 - D08Z5 wall backflip option (Needs playtest)

- The "backflip" off the slot 10 police car is the original steep-slope
  slide (mode 2, slip pose 145) followed by a backwards launch from the fall
  table `0x800c27c8` when the face ends (525 to 812 units back). New
  ADVANCED > MOVEMENT > WALL BACKFLIP, MODERN (default) / ORIGINAL: Modern
  keeps the slide but zeroes that launch (`wall_backflip.inc`), so Duke drops
  where the face ends (70 to 410 back). Profile schema 37, `run.py
  --wall-backflip`, Vanilla original. Hooks `0x8005446C`, `0x8003F060`
  regenerated (codegen hash unchanged). Executable
  `191646ef67d6890ec435ddc771a9f31c9da349eb6e83d8a8c5b341860a4d3647`.
  [Note 174](174-d08z5-wall-backflip.md).
- Separate finding: the player's slot 10 save halts the emulator on some
  reloads in one session (GPU GP0 unknown command). Not investigated.

## 2026-10-21 - D17R1 accepted

- User: "lock it in and close it out". D17R1 Done; executable
  `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f` is the
  regression baseline.

## 2026-10-21 - D17R1 first playtest: redraws broken, fixed (Needs playtest)

- User: "quite a lot of graphically breaking issues". The redraw workers'
  ordering-table walk ended at any bit-23 link, and every new host-packet
  arena address has bit 23 set, so high-refresh in-between images lost
  everything after the sky. Framework `render_replay.c` now ends the walk as
  the live GPU walker does. Moving replay sequences checked whole. Executable
  `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f`.
  [Note 173](173-d17r1-sky-blocks.md).

## 2026-10-21 - D17R1 sky cloud blocks fixed (Needs playtest)

- The stair-stepped holes in the clouds came from the near clip's packet
  budget: with the extended draw distance the game filled 61% of its render
  ring, the near clip's 60% cap was spent, and the sky (drawn last) fell back
  to the original path, which drops its near cloud quads. The near clip now
  has its own packet arena in the GPU DMA aperture; framework `gpu.c` keys
  those addresses apart from RAM. 0 fallbacks in the canal and four sweeps.
  Camera jerk not reproduced. Executable
  `d632379685d59dd1fa953f088459ac69af1bff6022ef5bd8886ac2a0bbf27f03`.
  Awaiting the user's playtest. [Note 173](173-d17r1-sky-blocks.md).

## 2026-10-21 - D17R accepted; D17R1 and D17X opened

- User: "Perfect!! Lock this in." D17R Done; executable
  `fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d` is the
  regression baseline.
- New Todo jobs: D17R1 (sky artifacting when looking up/down and turning,
  medieval UI slot 2 with possible camera jerk, UI slot 8) and D17X (small
  white triangles on boat posts, UI slot 6, and grave-dig soil, UI slot 7, in
  first person; backlog, not started).

## 2026-10-20 - D17R sky edges fixed (Needs playtest)

- The black top corners and dark side edges when looking up in 16:9 came
  from the shared sky routine still drawing for 4:3. New plugin
  `recomp/src/ttk/sky_widescreen.cpp` (no regeneration, no framework change)
  widens the fill, the backdrop and the sky clip rectangle. Checked offscreen
  on slot 9, medieval slot 2, D17O slot 10, third person, 21:9 and 120 FPS.
  HOG HEAVEN outdoors was not reached offscreen. Executable
  `fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d`. Awaiting the user's playtest.
  [Note 172](172-d17r-sky-edges.md).

## 2026-10-20 - D26G accepted

- User: "lock it in and close it out!!". D26G Done; executable
  `5ac80ba1394f8942b4998159d79fd515e09e959d73aea456e65b4eb95b7b03da` is the
  regression baseline.

## 2026-10-20 - D26G console key repeat (Needs playtest)

- Held Backspace and characters repeat in the backtick console (framework
  `debug_console_event` accepts SDL repeat events for them). Checked with
  real X keys on private Xvfb. Executable
  `5ac80ba1394f8942b4998159d79fd515e09e959d73aea456e65b4eb95b7b03da`.
  Awaiting the user's playtest.

## 2026-10-20 - D17W accepted

- User: "that cart is looking handsome!!". D17W Done; executable
  `8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c` is the
  regression baseline.

## 2026-10-20 - D17W minecart fixed (Needs playtest)

- Slot 10's cart is clean in first person from every distance and angle
  tried. Fix in `near_clip.cpp` `rank_coplanar` (coplanar overlapping faces
  of one object mesh keep the original's paint order; `DNTTK_NEAR_COPLANAR=0`
  compares). A/B on 12 other saves showed no change. Executable
  `8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c`. Awaiting
  the user's playtest; D19Q1's Done header, misfiled under D17W, moved back.

## 2026-10-20 - D19Q1 accepted

- User: "confirmed working!". D19Q1 Done; the executable below is the
  regression baseline.

## 2026-10-20 - D19Q1 fixed (Needs playtest)

- SAVE GAME's picture came from stale CPU VRAM (the image at the previous
  save, under OpenGL); not first person. `savestate_grab_thumb` syncs
  first. Two saves in one session, first person, after LOAD GAME and Vanilla
  F7 all store play. Executable
  `92178369fc5153178009edf801bd5c07299fee5d6d9bab927ea6ca2b6fc3e043`.

## 2026-10-20 - D19Q1 opened (SAVE GAME thumbnail shows the menu)

- Player UI slot 1's picture is the OVERWRITE THIS SAVE? page; first person
  pauses on a third-person view, untested by D19Q. Research in the job.

## 2026-10-20 - D19Q accepted

- User: "im happy with this". D19Q Done; executable
  `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b` is the regression baseline.

## 2026-10-20 - D17W opened (minecart white triangles)

- New Todo D17W: player UI slot 10, the overturned minecart shows white
  untextured triangles in first person only (also with GEOMETRY ORIGINAL;
  clean in third person). Leads in the job (D11B near clip first).

## 2026-10-20 - D19Q SAVE GAME (Needs playtest)

- Pause list SAVE GAME opens LOAD GAME's page with every slot open; empty
  saves at once, used asks OVERWRITE THIS SAVE? (NO first); SAVED TO SLOT NN
  or THE GAME COULD NOT BE SAVED. Thumbnail = play when the pause opened;
  LOAD GAME on such a save resumes play. Framework: atomic slot writes,
  thumbnail grab / write. Identity guard no longer covers the card-restored
  GAME SETTINGS values. Executable
  `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b`,
  [note 171](171-d19q-save-game.md).

## 2026-10-20 - D19R done: startup card wait at host speed

- The user delegated testing and closeout. States 1 and 2 (card probe, file
  read) run their VBlanks unpaced with the framework idle skip on for that
  window only (new `psx_mod_request_unpaced`, exported patch; codegen hash
  unchanged). States 1-2: about 1.9 s to 0.24 s; black after the movies about
  0.35 s. Settings still load; blank and fresh cards continue; Vanilla
  unchanged. Executable `2fb1bd3cc0e8ee78fd94c41da6c66c39a5fe7d5ff4eddd813e321bc2f2833408`,
  [note 170](170-d19r-card-wait.md).

## 2026-10-20 - D19K accepted; D19R opened

- User: "confirmed now working as intended. lock it in." D19K Done; executable
  `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9` is the regression baseline.
- New Todo D19R (startup card probe hang, research in the job): state 1's
  card probe blocks the main loop for about 1.5 s (90 fields) in the BIOS card
  routines, black screen; options listed (unpaced card wait, keep the title
  image, defer the probe, host settings read).

## 2026-10-20 - D19K revision 2: LOAD GAME opens on slot 01 (Needs playtest)

- User: "superb, everything is working as intended!"; LOAD GAME now opens on
  slot 01 instead of the newest save. Executable `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9`.

## 2026-10-20 - D19K revision 1: steel text, startup card check unseen; D19Q opened (Needs playtest)

- LOAD GAME's save time and the in-game warning (now UNSAVED PROGRESS WILL BE
  LOST) in Console steel TTK Medium Italic with its shadow, drawn by the host
  with the thumbnail. Modernized startup no longer shows CHECKING MEMORY CARD,
  CHECKING SAVE GAME FILE or the no-card message; the card reads and the
  settings unpack still run (hook `0x8008B454` regenerated, codegen hash
  unchanged). New Todo D19Q (SAVE GAME screen to match). Executable
  `2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`, [note 169](169-d19k-load-game.md).

## 2026-10-20 - D19K LOAD GAME from our savestates (Needs playtest)

- LOAD GAME on the title, pause and death lists now opens our page (layout A
  of the mockup): the 12 savestate slots by level name, newest selected, the
  chosen slot's thumbnail, era and save time beside them, then LOAD THIS
  GAME? and a load like F7's. Empty and other-build slots are greyed; a failed
  load keeps the game and says so. The memory card LOAD stays Vanilla's.
  Framework: thumbnail draw in both present paths; `boot_state_check_buffer`
  no longer passes every slot when given no reason buffer. Offscreen loads
  from pause, title and death; Vanilla original; tests pass; patch verified
  against the pin. Executable `2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`,
  [note 169](169-d19k-load-game.md).

## 2026-10-20 - D08Z4 accepted

- User: "thats the one! lock it in and close it out!!". D08Z4 Done; executable
  `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778` is the regression baseline.

## 2026-10-20 - D08Z4 falls keep the eye view (Needs playtest)

- Slot 6 cop car: the original slope slide on the car body alternates 145/108/105,
  and each 108 counted as an unowned fall, which first person showed from the
  orbit (the "brief third person"). Unowned falls (107/108) now stay in the
  eye, game-wide; mantles, hangs and climbs keep the orbit. 40 scripted jumps
  (gaits, headings, both jump styles, both wall-contact modes) stay in the eye;
  E-held mantles still use the orbit; third person and Vanilla unchanged;
  native suites pass. Executable `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778`,
  [note 168](168-d08z4-falls-keep-eye.md).

## 2026-10-20 - D34 accepted

- User: "amazing read". D34 research Done; follow-ups D34A-D34G stay
  proposed in note 167 for another day.

## 2026-10-20 - D34 multiplayer research (Needs review)

- Research only (no code, build or game run). [Note 167](167-d34-multiplayer-research.md):
  two-player mode is one shared simulation drawing one viewport per game
  frame; our build has no port-2 pad (`MAX_PLAYERS 1`) and every Modernized
  system is player-1-only, so two-player play falls back to the original
  controls and presentation. Recommended route: deterministic input sync of
  the game's own two-player mode (framework `recomp-net`), own view full
  screen through D17 replay, self-hostable relay. Proposed follow-ups
  D34A-D34G await the user's review. Executable unchanged
  (`6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33`).

## 2026-10-20 - D19N accepted

- User: "lock it in and close it out!!". D19N Done; executable
  `6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33` is the
  regression baseline. Backlog: D19K, D19F, D26G.

## 2026-10-20 - D19N review pass (Needs playtest)

- Fixed the title CONTROLS screen and ADVANCED going blank when a slider was in
  view at a high frame rate (slider objects now in guest RAM, the original
  SOUND screen's). GAME SPEED's slider has room; AUTO AIM moved to ADVANCED >
  ORIGINAL GAME beside RED DOT; USE ITEM is Enter only (profile schema 36).
  Executable `6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33`, [note 166](166-d19n-options-layout.md).

## 2026-10-20 - D19N OPTIONS, the rest under ADVANCED (Needs playtest)

- Layout A as approved: OPTIONS is SOUND (original), CONTROLS, DISPLAY,
  GAMEPLAY and ADVANCED (one list under headings). AUTO AIM (profile, default
  OFF) replaces AIM ASSIST and sets the game's auto aim; GORE, STATUS BARS and
  GAME SPEED stay the original's values on the card; CONFIGURE CONTROLLER is
  gone from Modernized, which forces the default pad layout (a card copy keeps
  Vanilla's layout and auto aim in saves). Dependent rows are dimmed.
  Hooks `0x80082F64`, `0x80089920` regenerated (codegen hash unchanged).
  Executable `ce2b297e1f854aad818b5318b3636183986742973b94c1bed6d35bad2bcff6a9`,
  [note 166](166-d19n-options-layout.md). Offscreen and tests as described.
  Awaiting the user's playtest.

## 2026-10-20 - D24D + D24E accepted

- User: "its perfect! lock it in close it out". D24D and D24E Done; font pack
  `2609f2e6077a080c25d327582471ca1a19ab816ca1e078a44ddadeaaafe7316d`,
  executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`
  unchanged. Backlog: D19K, D19F, D19N, D26G.

## 2026-10-20 - D24D + D24E TTK-font ( ) and % (Needs playtest)

- `<` `>` were already native. ORIGINAL MOVEMENT uses `( )`, which the disc
  fonts lack, so they are now built from each font's own `O`. `%` is `./.`
  from its own `/` and periods. Only the font pack changed (executable
  `bbfaf117...` unchanged). Font tests pass, and offscreen MEDKIT 100% and
  console `(PAREN) 50%` show them in the TTK fonts.
  [note 165](165-d24d-d24e-font-glyphs.md). Awaiting the user's check.

## 2026-10-20 - D19G1 accepted

- User: "it works! i can confirm it works. lock it in close it out". D19G1
  Done; executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29` is the regression baseline. Backlog: D19K, D19F,
  D19N, D24D, D26G.

## 2026-10-20 - D19G1 FRAME RATE live after a 60 launch (Needs playtest)

- In-game FRAME RATE now applies on CONFIRM from a 60 launch (replay
  presentation and its render workers start mid-session) and 60 stops it from
  any launch, back to the original presentation. Software renderer: NEXT
  LAUNCH. Framework request `psx_mod_request_present_replay` in the tracked
  patch. Executable
  `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`,
  [note 164](164-d19g1-live-frame-rate.md). Offscreen as described; tests
  pass. Awaiting the user's playtest.

## 2026-10-20 - D08T4 accepted

- User: "lock it in, close it out, another winner". D08T4 Done; executable
  `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4` is the
  regression baseline. Backlog: D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D08T4 slot 10 dumpster (Needs playtest)

- Fixed by design: the level marks the floor at that dumpster as no-grab
  (original idle grab `0x80051890` checks bit 0 of byte 4 of Duke's floor
  record `*(player+0x48)`); Vanilla cannot grab it either, only climb it. Our
  grab hint ignored that gate. Now the hint and Grab apply only where the
  original can grab, game-wide; alley dumpster and medieval block unchanged,
  Vanilla unchanged. Executable
  `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4`,
  [note 163](163-d08t4-fixed-pushables.md). Native tests pass. Awaiting the
  user's playtest.

## 2026-10-20 - D10D accepted

- User: "perfect, lock it in, close it out". D10D Done; executable
  `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892` is the
  regression baseline. Backlog: D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D10D keyboard turning (Needs playtest)

- Left / Right arrows turn the view (and Duke) everywhere in Modernized play:
  on foot, first and third person, swimming, jetpack; Duke 3D rates (slow
  first 1/8 s, then about 79 degrees/s walking, 158 running), smooth at high
  refresh. Captured, the arrows no longer reach the original D-pad, so the
  jetpack stops strafing on them; the tank fallback turns Duke with them.
  KEY BINDINGS CAMERA rows TURN LEFT / TURN RIGHT, `--bind`; launcher schema
  35. Lab offscreen (level 1 slot 9) as described. Executable
  `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892`, [note 162](162-d10d-keyboard-turning.md). Native and Python tests
  pass. Awaiting the user's playtest.

## 2026-10-20 - D10C accepted

- User: "awesome!! ... lock this current one in, and close it out". D10C Done;
  executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19` is the
  regression baseline. New Todo: D10D (Left / Right arrows turn left / right
  in Modernized). Backlog: D10D, D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D10C Duke 3D look keys (Needs playtest)

- Page Up / Page Down look up / down while held and ease back on release;
  Home / End tilt and keep the view (a Page Up / Down tap brings it back).
  First and third person, mouse look on top, smooth at high refresh (late
  camera). KEY BINDINGS CAMERA rows and `--bind`; launcher schema 34 adds the
  keys to older profiles. Lab offscreen (level 1 slot 9): holds, locks,
  taps, mouse and V as described; KEY BINDINGS swap saved. Executable
  `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19`,
  [note 161](161-d10c-look-keys.md). Native and Python tests pass. Awaiting
  the user's playtest; open question whether center should be level.

## 2026-10-20 - D07F accepted

- User: "confirmed working. lock it in and close it out". D07F Done;
  executable `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef` is the
  regression baseline. Backlog: D10C, D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D07F airborne shots follow the crosshair (Needs playtest)

- Jumps and falls hold only the camera-only lease; shots there now use the
  view aim (`airborne_shot_ready()`). Lab: 24 airborne shots (RPG, shotgun,
  pistol; manual and assisted; first person; up and down), 0 misses, rockets
  explode at the crosshair point. Executable
  `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef`,
  [note 160](160-d07f-airborne-shots.md). Native and Python tests pass.
  Awaiting the user's playtest.

## 2026-10-20 - D19G accepted

- User: "looking good!! lock it in and close this job out". D19G Done;
  executable `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da` is the
  regression baseline. New Todo: D07F (airborne shots ignore the crosshair),
  D10C (Duke 3D look keys), D19G1 (frame rate live after a 60 launch).
  Queued: D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D19G KEY BINDINGS and live settings (Needs playtest)

- PC CONTROLS > KEY BINDINGS: every Modernized action in five groups; Enter,
  then a key or mouse button binds it at once and saves it (a taken key
  swaps, Escape cancels, Backspace clears, V asks RESTORE DEFAULT KEYS?).
  Live on CONFIRM now: mouse sensitivity and invert, display mode,
  widescreen, camera, view, shoulder, camera distance, weapon aim, and frame
  rate unless launched at 60. Design approved from a Photon mockup built from
  the disc's own interface art. Offscreen and Xvfb checks, Python 153 OK,
  native suites, patch and `check_repo.py` pass. Executable
  `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da`,
  [note 159](159-d19g-options-stage-2.md). Not verified: the real display
  (borderless / exclusive, window reshape) and the user's feel.

## 2026-10-20 - D33 accepted

- User: "perfect!! lock it in, close it out". D33 Done; executable
  `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-20 - D33 Modernized by default, `--vanilla` one launch (Needs playtest)

- Plain `run.py` always launches Modernized; `run.py --vanilla` plays Vanilla
  for that launch only (never saved). `--mode` removed; the in-game PROFILE
  row removed; a saved Vanilla choice moves to Modernized once with a notice.
  Offscreen checks, Python 152 OK, native suites and `check_repo.py` pass.
  Executable `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2`,
  [note 158](158-d33-default-profile.md). Not verified: the user's launches.

## 2026-10-20 - D08AB accepted

- User: "solid lock it in and close it out". D08AB Done; executable
  `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AB SWIMMING MODERN / ORIGINAL option (Needs playtest)

- New SWIMMING row in OPTIONS > MODERNIZED (after LANDING): MODERN (D08AA
  pace, default) / ORIGINAL (pre-D08AA water speeds with modern controls,
  including the forced run in shallow water - user choice). Live, saved
  (schema 33), `run.py --swimming modern|original`; Vanilla always original.
  Lab: ORIGINAL reproduces the D08AA "before" table (surface 26 / 6 / 15,
  underwater 27.6, subway forced run), MODERN unchanged, a mid-swim menu
  change applies at once both ways; regressions and suites pass. Executable
  `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508`,
  [note 157](157-d08ab-swimming-option.md). Not verified: the user's
  playtest. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AA accepted; D08AB queued

- User: "this is perfect. swimming now feels great! it's responsive, and the gameplay allows the player to dodge enemy fire and not face these jarring speed changes when swimming." D08AA Done; executable
  `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366` is the
  regression baseline. New Todo D08AB: a SWIMMING MODERN / ORIGINAL row in
  PC CONTROLS (user request). Queued: D08AB, D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AA one walk/run pace on land and in water (Needs playtest)

- Shallow water forced the run; the mid-depth wade walked at twice the land
  walk; surface strokes (~26 forward, ~6 sideways) and underwater thrust
  (~28) ignored Shift. One shared pace (the land baselines, walk ~10.5 and
  run ~50 units per vblank) now drives all of them: shallow uses the land
  gait with Shift, the mid-depth root follows the pace step, and a new hook
  at the swim tail's rotation (`0x80066e34`, ra `0x80046ec8`) rescales the
  original stroke velocity; Space / Ctrl keep their vertical speed. Measured
  walk 9.9-11.1 and run 49-51 in every state; swim regressions and Vanilla
  unchanged; suites pass. Executable
  `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366`,
  [note 156](156-d08aa-unified-pace.md). Not verified: the user's playtest.
  Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D19O and D19L accepted

- User: "accepted! lets bring this one to a solid close and lock it in". D19O
  and D19L Done; executable
  `114a77530e92839c095aeb833078a84823b312417e7256c5c476460c1d7f38b2` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D19O round 2 and D19L: the menu stays up through demo scene loads (Needs playtest)

- User playtest: judder at scene changes and loading screens; held Down
  stopped when the demo scene changed. Our menu skipped the guest's second
  pass of an image at two load hand-overs (gone about 100 and 200 ms), the
  presenter showed the older real image in a long logo pause, and held keys
  did nothing for the 1.9 s blocking load (then a burst). Now the menu is
  drawn on every image, moves during the whole load, and the repeat never
  bursts; load timing unchanged. Covers D19L's acceptance too. Executable
  `114a77530e92839c095aeb833078a84823b312417e7256c5c476460c1d7f38b2`,
  [note 155](155-d19o-menu-flashback.md#round-2-the-users-playtest-2026-10-19).
  Not verified: the user's playtest.

## 2026-10-19 - D19O menu flash after a stall or demo/logo switch (Needs playtest)

- The flash came after a replay timeline reset (a host stall; every demo/logo
  switch during a demo scene load), not from each menu change: D19J measured
  it right after its own dump stall. For about 100 ms the real image, one to
  two images behind the redraws, was shown. The newest menu image is now
  redrawn on its own until a pair exists, and the last redraw stays through
  the reset. Executable
  `226719899432b4b43694269a108943d1823e25fbe87c13089489278d64af5161`,
  [note 155](155-d19o-menu-flashback.md). Not verified: the user's playtest.

## 2026-10-19 - D07E accepted

- User tested Modernized `view` and `original` weapon aiming: "it plays
  solidly ... both work as intended beautifully". D07E Done; executable
  `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5` is the regression baseline.
  Queued: D19O, D19K, D19L, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D07E stale lease and original-aim facing (Needs playtest)

- Modern shots and movement lost their lease when the game ran below 15 fps
  (upgraded energy stream: 34 of 107 shots used the original auto-aim). The
  lease window is now one game frame, 4..10 vblanks (4 at the normal 20 fps).
- `original` weapon aiming: Duke now faces the camera (user choice), so the
  original shot, auto-aim and energy lock follow where he looks instead of the
  save's heading. Executable
  `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5`,
  [note 154](154-d07e-frame-lease.md). Not verified: the user's playtest.

## 2026-10-19 - D19P accepted

- User: "lock it in. close it out! it's great" D19P Done; executable
  `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63` is the
  regression baseline. Queued: D07E, D19O, D19K, D19L, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19P death screen QUIT TO MENU (Needs playtest)

- Death list QUIT is now QUIT TO MENU, asking QUIT TO MENU?; YES still goes
  through the game over screen to the title. Fixed on the way: the D19
  identity guard covered the selected item `0x800c3f94`, so Modernized menus
  could stay original for a session. Executable
  `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63`,
  [note 153](153-d19p-quit-to-menu.md). Not yet verified: YES in level 1 and
  first person, the user's playtest.

## 2026-10-19 - D22D accepted

- User: "it's perfect!!" D22D Done; executable
  `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98` is the
  regression baseline. New Todo D19P (death screen: QUIT becomes QUIT TO
  MENU beside QUIT GAME). Queued: D19P, D07E, D19O, D19K, D19L, D19G, D19F,
  D19N, D08AA, D24D, D26G.

## 2026-10-19 - D22D energy weapon damages enemies (Needs playtest)

- Cause: a D07C Modernized regression. The beam updater `0x800733C8` damages
  only the actor in projectile `+0x14`, and D07C cleared it on every
  view-aimed beam. Vanilla (same save, same enemy) locked and killed it; the
  Modernized baseline never damaged it.
- Fix: the first beam locks onto the targetable actor under the crosshair,
  within the original reach (6000), or none (a miss); the upgrade's extra
  beams keep the original's other locks inside 45 degrees of the view. The
  completion no longer races the host input sequence. Executable
  `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98`,
  [note 152](152-d22d-energy-beam.md).
- Lab: levels 1 and 0, two enemy types, first and third person, kills; 8
  degrees off is a miss; upgraded extra beam; Vanilla unchanged.
  `ttk-aim-test` links again with a D22D case; controls, input, Python 146
  OK. New Todo D07E (stale-lease shots use the original auto-aim; `original`
  weapon aim keeps a stale aim vector). Not yet verified: the user's playtest.

## 2026-10-19 - D17V accepted

- User: "total victory. i even played it on strong and i love it." D17V Done;
  executable `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193`
  is the regression baseline. Queued: D19O, D19K, D19L, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D17V modern view bob (Needs playtest)

- Modernized first person: the bob now keeps Duke 3D's steady tempo (about
  1.5 dips a second) walking and running, dipping deeper rather than faster
  when running, with no sway; the eye glides evenly instead of riding Duke's
  stride surge. Off restores the plain D17C eye. Executable
  `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193`,
  [note 151](151-d17v-view-bob.md). Pre-existing: `ttk-aim-test` lacks the
  D19J `psx_mod_host_call_*` stubs and does not link.

## 2026-10-19 - D19J accepted

- User: "fully accept". D19J Done; executable
  `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d` is the regression baseline.
  Now unblocked: D19K (needs D24B, accepted) and D19L. Queued: D19O, D19K,
  D19L, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D19J title main menu re-implemented (Needs playtest)

- Modernized: after PRESS START our main menu in the original's look (ONE
  PLAYER, TWO PLAYER, LOAD GAME, OPTIONS, QUIT GAME), plus CHOOSE DIFFICULTY
  LEVEL, TWO PLAYER OPTIONS and CHOOSE CHARACTER, drawn with the originals'
  calls on state 12 / sub-screen 12 (new entry hook `0x80022324`). Starting a
  game hands the page to the original handler with one Cross press. One
  keyboard picks both two-player characters. LOAD GAME stays original
  (D19K). Framework: saves and loads wait for plugin guest calls
  (`psx_mod_host_call_begin/end`); a save inside a menu call had crashed the
  run. Executable
  `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d`,
  [note 150](150-d19j-title-menu.md). New Todo D19O (frontend redraw
  flash-back, also in the original menus). Still queued: D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19M accepted

- User: "perfect!! thats working". D19M Done; executable
  `de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43` is the regression baseline.
  Title main menu QUIT GAME comes with D19J. Queued: D19J, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19M QUIT GAME (Needs playtest)

- QUIT GAME is the last item of the pause list and the death list (the death
  list's QUIT still means game over); QUIT GAME? YES / NO, NO highlighted;
  YES sends the window close event, exit code 0, settings kept. Title main
  menu gets it in D19J (user choice). Executable
  `de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43`,
  [note 149](149-d19m-quit-game.md). Still queued: D19J, D19G, D19F, D08AA,
  D24D, D26G.

## 2026-10-19 - D19I accepted; D19M and D19N added

- User: "it's working as intended". D19I Done; executable
  `f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c` is the regression baseline.
  New Todo: D19M (QUIT GAME item that closes the program, YES / NO prompt),
  D19N (merge the original OPTIONS screens with ours: one OPTIONS screen,
  the rest under ADVANCED; first-pass overlaps: AUTO AIM vs our aim rows,
  CONFIGURE CONTROLLER vs PC CONTROLS / D19G, GAME SPEED vs CPU SPEED; design
  artifact first). Still queued: D19J, D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D19I in-game menu text at full brightness (implemented)

- Our pause list, death list, briefing, prompts and pause OPTIONS were drawn
  at the view composition, into the world's drawing chain, under the paused
  world's darkening quad (`0x80025f28`) and behind Duke. In gameplay they are
  now drawn in the pause-dispatch tick, after the quad, where the original
  draws its pause text; title menus unchanged. Text measures 255 red like the
  original (briefing exactly the original's 255, 176, 34; was 135, 95, 25).
  World darkening unchanged; 120 Hz presents never lose the text. Executable
  `f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c`,
  [note 148](148-d19i-menu-brightness.md). Still queued: D19J, D19G, D19F,
  D08AA, D24D, D26G.

## 2026-10-19 - D19H accepted; D19I-D19L added

- User: "accepted!! this works and it's functional". D19H Done; executable
  `681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2` is the regression baseline.
  New Todo: D19I (in-game menu text drawn at half brightness, likely under
  the paused-world quad), D19J (title main menu re-implemented like
  OPTIONS), D19K (LOAD GAME from our savestates inside the menus, replacing
  the memory-card LOAD in Modernized), D19L (menus stay up during demo scene
  loads). Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D19H pause menu and death screen as lists (Needs playtest)

- Modernized: Escape opens RESUME / MISSION BRIEFING / OPTIONS / LOAD GAME /
  RESTART LEVEL / MAIN MENU (layout B); the death screen lists CONTINUE / LOAD
  GAME / RESTART LEVEL / QUIT (challenge: REPLAY LEVEL / GIVE UP), with YES / NO
  prompts, in the original menu style (game's own draw calls, screen values 8
  and 9). Each choice runs the original handler's and main loop's own
  sequence; LOAD uses the original screens. Menu taps come from key events.
  D19's glyph guard no longer covers menu variables. Offscreen: every choice,
  challenge stage 1, Vanilla and title OPTIONS match the originals; suites
  pass. Executable
  `681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2`,
  [note 147](147-d19h-pause-menu.md). Continues kept as the original's (user
  expects to scrap them later). Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D08O4 accepted

- User: "this is awesome. fully accepted!". D08O4 Done; executable
  `3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d` is the regression baseline.
  Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D08O4 short Ctrl dive stops on release (Needs playtest)

- The surface dive (original anim 133) is a fixed ~30-update thrust along
  body pitch that ignores Square; after Ctrl was released the host pitched
  Duke back to the view, so the dive carried him forward. With no swim input
  held, the host now ends 133 the way the original handler does (anim 127).
  Slot 3 copy: drift after release 1474 -> 9 units (3-update tap), 796 -> 10
  (15-update hold); W, W + fire, Space surfacing, Ctrl+W and Vanilla
  unchanged; suites pass. Executable
  `3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d`.
  Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-18 - D08O2B accepted; D08O4 added

- User: "it certainly is working, duke shoots in the right direction". D08O2B
  Done; executable
  `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25` is the
  regression baseline. New Todo D08O4: in slot 3 a brief Ctrl dive keeps
  carrying Duke forward with the dive/forward stroke after release (lab: about
  850 units of horizontal drift after a 15-frame tap). Queued: D08O4, D19G,
  D19F, D08AA, D24D, D26G.

## 2026-10-18 - D08O2B swim-fire on every Duke model (Needs playtest)

- Slot 3 (level 2 sewer, file 02) uses a Duke model whose joints are ordered
  differently from level 6, where D08O2A was tuned by record number; the hook
  turned a leg instead of the shoulders. Shoulders/chest are now found by part
  (`duke_model.inc`, shared with first person). Slot 3 copy dive + W + fire:
  gun hand 85 -> 221; level 6 regression matches; Vanilla no arming; suites
  pass. Executable `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25`,
  [note 146](146-d08o2b-swim-fire-costumes.md). Still queued: D19G, D19F,
  D08AA, D24D, D26G.

## 2026-10-18 - D08G5 accepted

- User: "perfect!!". D08G5 Done; executable
  `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79` is the regression baseline.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08G5 partial cheat entry keeps mouse look (Needs playtest)

- While `dn...` was pending, the input layer dropped all mouse events (so the
  camera stalled) and the key that broke the sequence was eaten. Now typing
  owns the keyboard only: mouse look and wheel stay live, a click cancels the
  prefix and acts, and a breaking key (W, Space, ...) acts at once.
  `ttk-input-test` PASS (new check fails on the old source). Executable
  `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79`.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08A22 accepted

- User: "fully accepted!!". D08A22 Done; executable
  `67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546` is the regression baseline.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08A22 strips scale with the picture (Needs playtest)

- The inventory/mission switcher strips and the mission card take their scale
  from the fitted game picture (picture height / 480, fractional, at least 1x)
  instead of an integer floor of the window height, and sit inside the
  pillarbox/letterbox. Strip icons now match the native HUD icons at every
  window size (offscreen: 18 vs 18 px at 1440x1080, 10 vs 10 at 800x600;
  OpenGL 4:3 and 16:9, Software). Framework patch re-exported and verified.
  Executable `67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546`.

## 2026-10-18 - D19 accepted

- User: "fully accept!!". D19 Done; executable
  `fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b` is the regression baseline.
  New Todo D19G (key bindings screen, live display/camera options). Queued:
  D19G, D19F, D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D19 stage 1: in-game OPTIONS extended (Needs playtest)

- Modernized OPTIONS (title and pause) adds PC CONTROLS, DISPLAY and
  MODERNIZED, drawn by the game's own menu code so they look like GAME
  SETTINGS. Settings save to the profile; eight gameplay options apply at
  once, the rest at the next launch. New hooks 0x800220F4 and 0x80025FB0
  (regenerated; savestates still load). Executable
  `fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b`. [Note 145](145-d19-game-menu.md).
  New Todo D19F1 (prompt icons follow the input device). Next for D19: key
  bindings screen, live display/camera changes, D19F icons.

## 2026-10-18 - D19 menu mockup; D19F queued

- Design only (no code, build or launch). Interactive HTML mockup of the
  original menus in the disc's own font and button sprites, with proposed PC
  CONTROLS, DISPLAY and MODERNIZED screens: private artifact
  https://claude.ai/artifact/6FxFwzfzdCp2doDjSesfHA. User loved it. D19 now
  replicates the existing menu (Sonic 3 A.I.R. dropped). Menu text is the big
  italic font squeezed to 0.725 with a 3 px glyph overlap. Queued D19F: PC
  button icons in the exact PS1 prompt style (Photon Studio). D19B stays
  Todo. Still queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D19D accepted

- User: "amazing work". D19D Done; executable
  `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe` is the regression baseline.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D19D PC menu keys (Needs playtest)

- Modernized menus: **Escape/Backspace** back out one level (Triangle),
  **Enter** confirms (Cross; Start on PRESS START), **V** is Square, the
  **mouse wheel** moves menu cursors. Escape never quits: it resumes in the
  pause menu (Triangle there is QUIT) and does nothing on PRESS START and the
  death screen. CONFIGURE CONTROLLER (assigns any pressed button) takes Start
  for both keys until its prompt. Fixes Escape confirming the death screen's
  REALLY QUIT?. Gameplay Escape/Enter, movies, F7 and Vanilla unchanged.
  Executable `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe`. [Note 144](144-d19d-menu-keys.md).
  Accepted 2026-10-18.

## 2026-10-18 - D30 recomp.board listing (Done)

- The project's row on [recomp.board](https://recomp.fyi/game/duke-nukem-time-to-kill-ps1)
  was written by the board's scan and unclaimed, with an empty toolchain,
  status and targets and a wrong Europe SLES-01515 "works from" entry.
  `.recomp.json` at the repository root now claims and writes it: USA
  SLUS-00583 only, Psxrecomp, in-progress, Linux, maintainer, links, help and
  notes. Validated against `https://recomp.fyi/schema/v1.json`. README has a
  recomp.board badge and link. The row shows claimed after the board's next
  daily read. Update the file when status or targets change (D22, D24, D25).
  No game code changed; nothing built or launched.

## 2026-10-18 - D19E accepted

- User: "excellent work". D19E Done; executable
  `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf` is the regression baseline.
  The interim original-menu work is closed out. Still queued: D08O2B, D08AA,
  D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19E menu responsiveness close-out (Needs playtest)

- Menu taps are no longer lost while the loading spinner plays or on LOAD
  GAME with a saved game. A Modernized menu press is held until the game has
  read it (its own pad history), and taps made meanwhile are queued. Menus
  over the spinner are drawn from the newer image, so press to screen there
  is 100-113 ms mean (was 191-220). 90 of 90 taps over the spinner (was 24 of
  30); every screen in the sweep took every tap exactly once at 120, 180 and
  60. Demo menus, smoothness, load timing, gameplay and Vanilla unchanged.
  Executable `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf`. [Note 143](143-d19e-menu-closeout.md).
  Limits: the original hides the menu for the first 1.8 s of a demo scene
  load (presses land when it returns); menu text still at 10 Hz. Accepted 2026-10-18. Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19C accepted; D19E queued

- User: "its more responsive when the demo is playing, but when the loading
  spinner plays it is less responsive. lets commit at this point." D19C
  Accepted for demo-backed menus; executable
  `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828` is the regression baseline.
  New Todo D19E: menus over the loading spinner are less responsive; fix
  that and sweep every menu screen for best response (close-out). Still
  queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19C more responsive original menus (Needs playtest)

- Menu presses reach the screen about 100 ms sooner at the saved Modernized
  rate: main menu press-to-screen 220 -> 124 ms mean (24 of 24 taps one step
  each). Frontend in-betweens are drawn from the newer image of each pair,
  the world blended back toward the older poses; the guest, its 10 Hz tick
  and the demo behind the menu are untouched. Smoothness, gameplay start, 60
  and Vanilla unchanged. Executable
  `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828`.
  [Note 142](142-d19c-menu-response.md). Awaiting the user's playtest.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D17U accepted

- User: "i accept!! absolutely perfect." D17U Done (smooth spinning-logo
  loading screens at the saved Modernized rate); executable
  `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc` is the regression baseline. Still queued:
  D08O2B, D08AA, D08A22, D24D, D26G, D19C, D19D.

## 2026-10-18 - D17U smooth loading screens (Needs playtest)

- The spinning-logo loading screens now get D17 in-between images at the
  saved Modernized rate: level loads 119 distinct presents/s at 120 (30 guest
  images, 5 counted before), demo scene loads 112 (guest 10); 180 and 240
  scale. Load time unchanged (6.82-6.84 s); Vanilla and 60 original. Hook
  `0x80025554` added and regenerated (codegen hash unchanged). Executable
  `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc`.
  [Note 141](141-d17u-loading-screens.md). Awaiting the user's playtest.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19C, D19D.

## 2026-10-18 - D17T accepted; D17U, D19C, D19D queued

- User: "i can confirm this works ... it's a good first start." D17T Done;
  executable `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`
  is the regression baseline. New Todo: D17U (smooth loading screens, the
  spinning Duke logo), D19C (more responsive original menus), D19D (Esc backs
  out of sub-menus; PC-friendly menu navigation). Still queued: D08O2B,
  D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D17T smooth title menu and attract demo (Needs playtest)

- Frontend (mode 0) runs 10 images/s by the original's flip (6 fields); a
  30 Hz experiment desynced the attract demo, so the guest pace stays.
  Frontend images now get D17 in-betweens at the saved Modernized rate
  (118-119 distinct/s at 120), and short menu taps are held for 6 fields
  (45 ms taps 12/12, 7/12 before). Executable
  `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`.
  [Note 140](140-d17t-frontend-smoothness.md). Awaiting the user's playtest
  on their display. Still queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D17T queued (smooth title menu and attract demo)

- User: the title menu and attract demo should present at the saved
  Modernized frame rate. New Todo D17T. Found: D17 replay refuses outside
  gameplay mode (`0x800bcbb0 == 1`); the attract demo measured at about 10
  guest images per second (offscreen, 180, private profile). First step of the
  job: find whether 10 Hz is frontend pacing or CPU-bound. Still queued:
  D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-09 - D08G6 accepted (`dntlod`)

- User: "accepted!" D08G6 Done. Fun typed cheat shows "Play Zaxtor's
  Oblivion!" only. Executable
  `74cec66e398217530d0b8abc3fa0ccfe06308f6b73a17e5aafa628b3f0f2a540` is the
  regression baseline. Queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-09 - D08AA queued (unify walk/run speeds)

- User: surface swim too slow; underwater and shallow/ankle-deep water force
  run without Shift. New Todo D08AA - unify walk/run horizontal speeds across
  land, shallow water, surface and underwater to the current land baselines;
  shared path preferred; no baseline retune here; keep swim physics and
  swim-fire. Test: UI slots 2-3, slot 8. Still queued: D08O2B, D08A22, D24D,
  D26G.

## 2026-10-09 - D08O2B queued (swim-fire aims down again)

- User: UI slot 3, swimming and firing points the weapon down again; slot 2
  still aims forward. New Todo D08O2B (depends D08O2A; note 119). Still
  queued: D08A22, D24D, D26G.

## 2026-10-09 - D08A20 accepted (option A square)

- User: "fully accepted. ... this is it, excellent!!!" D08A20 Done; executable
  `a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95` is the
  regression baseline. [Note 139](139-d08a20-challenge-hud.md).
- Layout: 14x14 challenge icon in the native box's left cell (option A). Font
  picker section 15 marks A accepted; section 8 shows E as shipped.
- Still queued: D08A22, D24D, D26G.

## 2026-10-09 - D08A21 accepted; challenge icons tracked; D08A22 queued

- User: "completely approve this change." D08A21 Done; executable
  `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21` is the regression baseline.
- Challenge item icons tracked: `assets/ui/items/hud-challenge.png` (14x14 art)
  and `16px/hud-challenge.png` (from the user's `challenge-16.gif`), both with
  15-colour copies; items.json lists CHALLENGE ITEM (not drawn until D08A20).
- New: D08A22, host switcher strips scale with an integer floor of
  output height / 480, so they read small in small windows.

## 2026-10-08 - D08A21 14x14 HUD icons swapped in (Needs playtest)

- The user's 14x14-in-16x16 medkit, steroids, invincibility, invisibility and
  Double Duke icons (`research/inv/items-14/`) are now the active art in
  `recomp/assets/ui/items/` under the same names; 15-colour copies remade with
  `reduce_icon_15col.py`; the five cell/CLUT tables in
  `recomp/src/ttk/gadget_hud.inc` regenerated.
- The full 16x16 versions are kept, unused by the build, in
  `recomp/assets/ui/items/16px/` (user: for a future interface).
- Verified offscreen (private card copy): power-up, steroids and medkit boxes
  now leave a gap inside the frame; the switcher strip shows the new medkit and
  steroids. `test_ui_art` / `test_ttk_inv_icons` OK (new border test).
- Executable `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21`.
- Also 2026-10-08: backlog D08Z4, D08A20 (challenge icon delivered, top-right
  mockups in the font picker), D12D, D18E, D22D, D24E.

## 2026-10-08 - D08A12 accepted

- User: "perfectly done." D08A12 Done; executable
  `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d` is the regression baseline.
- Still queued: D24D, D26G.

## 2026-10-08 - D08A12 modern dynamite (implemented)

- **Original:** drawing dynamite lights it (fuse 4000 at the draw completion
  `0x8004e5ac`, burned by `0x8004dea4` case `0x8004e018`, about 3.7 s). The
  case ignores weapon requests and the holster button, so a switch waits for
  the throw or the blast.
- **Modern (`dynamite` = `modern`, default):**
  - A drawn stick stays unlit: fuse held full, sparks dark, sizzle refused.
  - 3, other weapon keys, the wheel, X or E's holster stow it, as for the Holy
    Hand Grenade.
  - Fire (Cross) lights it with a full fuse, and the original runs from then
    on.
- **Option:** profile schema 32, `run.py --dynamite original` (menu T).
  Vanilla stays original.
- **Hooks:** three new entry hooks, regenerated; codegen hash unchanged
  (savestates load).
- **Verified live (private copies):** held 6 s unlit, stow, wheel
  pass-through, a savestate round trip, then a lit throw and its explosion.
  Original: the stick exploded in hand and the switch waited.
- **Follow-up:** after a throw the next stick is drawn (unlit) once fire is
  let go; none when no sticks are left. Live: four throws in a row redrew.
- **Executable:** `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d`.
  [Note 138](138-d08a12-modern-dynamite.md). Nothing committed.

## 2026-10-08 - D08A19 accepted

- User: "well thats absolutely superb. I fully accept, this is the sort of design extension i wanted, introducing something the original game never had, and it looks completely at home. I'd call this feature finished." D08A19 Done; executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8` is the regression baseline.
  Still queued: D24D, D26G.

## 2026-10-08 - D08A19 used mission items (Needs playtest)

- User art delivered (`mark-used.png`, 8x8 green tick); font picker section 14
  shows it. Used mission items keep their full-colour icon with the tick in the
  strip and on the card, status USED (gold), counted as collected; Enter on one
  says ALREADY USED.
- "Used" = bit 0 cleared while Duke holds that item (animation 47); marker
  `0x4000` in the item's own flag, so saves keep it and a level reset clears it.
- Verified live in level 0 (red crystal at its holder) and by staged writes in
  level 6; savestate round trip; Vanilla never marked. Executable
  `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8`.
  [Note 137](137-d08a19-used-mission-items.md). Nothing committed.

## 2026-10-08 - D08Z3 accepted; D24D, D26G, D08A19 queued

- User: "Completely accept this work as complete, working, and done." D08Z3
  Done; executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e` is the regression baseline.
- Queued: D24D (`<`/`>` message fallback), D26G (console key repeat),
  D08A19 (USED status and green tick; placeholder in font picker section 14,
  8x8 `recomp/assets/ui/items/mark-used.png`).

## 2026-10-08 - D08Z3 modern landing (Needs playtest)

- Stand-still after landing = the original recovery 105 (straight-up jump,
  direction released before touch-down, bounce), about 0.6 s with no input,
  in Vanilla too. Modernized `landing` = `modern` (default): a held direction
  walks/runs off the update after the touch-down, a jump press hops again;
  `run.py --landing original` restores it. Schema 31 (the player's profile
  migrates on next launch).
- Measured: `original` 36..44 fields to move, `modern` 3..8; routes
  unchanged. Executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e`.
  [Note 136](136-d08z3-modern-landing.md). Nothing committed.

## 2026-10-08 - D08Z2 accepted

- User: "accepted!!". D08Z2 Done (armed scramble plays full body; the D08Z1
  safety net no longer freezes the legs). Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression
  baseline. Next: D08Z3.

## 2026-10-08 - D08Z2 revision 1: legs frozen after a corner bump (Needs playtest)

- User: after a bump at the statue (UI slot 8) Duke's lower half stays locked
  in one pose whatever he does (UI slot 6).
- Cause (D08Z1 safety net): its 107 undo ran at the next animation runner
  call, after the update had started 107 on the tracks; track 3's OR-only
  joint mask kept 107's leg joints (`0x1c00` -> `0x1cef`), freezing the legs,
  also in savestates. Reproduced at the monument corner; the original bounce
  never does it.
- Change: undo at the bounce's contact sound `0x8006bbd8` (ra `0x80055a10`),
  right after the 107 write; new opt-in hook, regenerated, codegen hash
  unchanged, savestates load.
- Evidence: three corner catches, mask stays `0x1c00`, legs stride after;
  native 45 groups, Python 136 OK. Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986`.
  [Note 135](135-d08z2-mantle-upper-body.md#revision-1-legs-frozen-after-a-corner-bump).
- The player's new UI slot 6 keeps the frozen legs (saved polluted state).
  Nothing committed.

## 2026-10-08 - D08Z2 mantle upper body (Needs playtest)

- Cause: the D08Y low-lip step-up (no E) could start with a weapon drawn, and
  since D08Z1 it starts from every wall slide. The original mantles only with
  a free upper body (`0x80051cf0` refuses on upper-table bit 8) and sets only
  the lower animation, so the legs played 134 while the upper track held the
  static weapon-ready pose 20. Holstered E mantles, crate mantles and the
  slot-12 grab and pull-up were full body and matched `original` frame for frame.
- Change: `jump_mantle_start` (one start for the crate, lowered-catch and
  step-up mantles) hands a weapon pose on the upper track to the mantle;
  `mantle_upper_restore` puts the kept upper block back when the mantle ends
  (without it the weapon stayed lowered and would not fire).
- Evidence: armed slot-5 step-ups play full body, weapon pose back one update
  after, Mouse1 fires; D08Z1 regression routes same outcomes; native 45
  groups (D08Y group extended), Python 136 OK. Executable `9de8da617862f53e545ac3c8be0f23623979fea2265944e2f3cdfaa3ec3f96b0`.
  [Note 135](135-d08z2-mantle-upper-body.md).
- Open: the user's playtest. Nothing committed.

## 2026-10-08 - D08Z1 accepted; D08Z2, D08Z3 queued

- User: "mechanically this feels significantly better, where we can call the actual job as accepted." D08Z1 Done; executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34` is the regression baseline.
- D08Z2 (next): the mantle now plays as a static leg pose, a regression from
  D08Z1. D08Z3: occasionally Duke is stuck for about a second after landing.
  Leads in the job board.

## 2026-10-08 - D08Z1 revision 1: monument corner (Needs playtest)

- The slot-6 monument corner bounced (convex corners defeated the wall clip).
  Jumps now turn around a corner in 15-degree steps, and any original bounce
  that still happens is turned into a slide, with its 107 undone in the same
  update. Slot 6 corner: no 107 in 21 jumps; earlier routes unchanged.
  Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. [Note 134](134-d08z1-jump-wall-slide.md).

## 2026-10-08 - D08Z1 jump wall slide (Needs playtest)

- Modernized jumps no longer bounce off walls: a wall only stops the motion
  into it, so Duke keeps his arc and slides along it (EDuke32 style); rising
  into a ceiling stops the rise and keeps the forward speed. No 107 stagger,
  rumble or contact sound. `run.py --jump-walls original` (menu K) restores
  the bounce; Vanilla unchanged. Profile schema 30.
- Verified offscreen: no 107 in 48 wall jumps over three levels, both jump
  styles; D08X/D08Y/D08U/D08W/D08Z routes keep their grabs, mantles and
  scrambles; native 45 groups, Python 136 OK, `check_repo.py`. Executable
  `c91a3ef128d9f98d923efe858fef75f432b8d729b2ffe5f16654f0034a56b627`. [Note 134](134-d08z1-jump-wall-slide.md).
- Needs the user's playtest. Nothing committed.

## 2026-10-08 - D24C accepted

- User: "all accepted". D24C Done: TTK-font `!` and `>` console prompt.

## 2026-10-08 - D24C TTK-font `!` and `>` console prompt (Needs playtest)

- Messages, headings and the console now draw `!` from the TTK font's own I
  and period (build-time, `build_ttk_fonts.py`) instead of the 8x8 system font.
- The console prompt and echoed commands start with `>` instead of `]`.
- Verified in a private offscreen run (`dnstuff`, console); suites pass. Needs
  the user's look in their own window.

## 2026-10-08 - D08A18 accepted

- User: "accepted and confirmed working!!!" D08A18 Done.

## 2026-10-08 - D08A18 built (Needs playtest)

- Enter / U on the browsed mission item: at the lock that takes it, the normal E
  interaction (the original uses it up); elsewhere CAN'T USE THIS HERE, or NOT
  FOUND YET, as cheat-style notices. Note 133.
- Verified offscreen in level 0 (crystal holder use, both refusals); suites
  pass. Card readers / key doors and other levels need the user's playtest.

## 2026-10-08 - D08A17 accepted

- User: "confirmed it's all working as intended!" D08A17 Done; executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline.

## 2026-10-08 - D08A17 power-up countdowns (Needs playtest)

- Invincibility (`+0x8a0`), invisibility (`+0x89c`) and Double Duke (`+0x89e`)
  are 6000-unit timers (20 s) set by their pickups and drained by Duke's
  update; death zeroes them and Continue gives 1500 invincibility (original).
  In Modernized each running one now has a lit box with the user's D08A16
  icon counting down in percent, stacked above the selected gadget, an
  active jetpack and running steroids (`gadget_hud.inc`). User choices:
  percent, the Continue protection shows, no expiry warning.
- Executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`;
  private Xvfb evidence and suites pass;
  [note 132](132-d08a17-powerup-countdowns.md). Waiting for the user's
  playtest. Not committed.

## 2026-10-08 - D08A16 accepted

- User: "fully accepted." D08A16 Done: the user's power-up icons are in
  `recomp/assets/ui/items/` (`hud-invincibility`, `hud-invisibility`,
  `hud-double-duke`, each with a `-15col` copy). There is no switcher-strip
  variant (power-ups are never held). D08A17 (countdown boxes) is now ready.
  Not committed.

## 2026-10-08 - D08A16 picker section ready (waiting for the user's art)

- D08A16 In progress: local picker `recomp/analysis/d24a-fonts/ttk-font-picker.html`
  section 11 has placeholders, in-game coin references (all one radiation
  coin; gold/orange invincibility, silver-violet/blue invisibility,
  silver/pink-red Double Duke), 15-colour previews and a stacked HUD mockup.
  Next: the user delivers `hud-invincibility.png`, `hud-invisibility.png`,
  `hud-double-duke.png` to `recomp/assets/ui/items/`; reduce them with
  `reduce_icon_15col.py`, rerun `make_picker.py`, user accepts. No gameplay
  change; nothing committed.

## 2026-10-08 - D08A14, D08A15, D08G4 accepted; D08A16, D08A17 queued

- User: "fully accepted and ready to close it out." D08A14, D08A15 (with the
  pickup correction) and D08G4 Done. Executable
  `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.
- Queued: D08A16 (power-up coin icons for invincibility, invisibility and
  Double Duke; human design ticket, placeholders in the local font picker
  when it begins) and D08A17 (HUD countdowns for the three power-ups, after
  the icons). Not started.

## 2026-10-08 - D08A14, D08A15, D08G4 (Needs playtest)

- **D08A14:** death ends steroids in Modernized `portable`: running and held
  doses are cleared while Duke is dead (user chose that a held dose is lost
  too), so after Continue there is no box, heartbeat or switcher entry.
  `original` and Vanilla keep them. [Note 127](127-d08a4-portable-steroids.md#d08a14---death-ends-steroids).
- **D08A15:** the heartbeat restarts with a beat at once when `dnhyper`
  refreshes steroids mid-run. Corrected by the user: a steroids pickup mid-run
  now stops the run and leaves a full held dose (Duke 3D feel), like the
  inventory cheats; executable `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472`. Cause: a refresh to 9000 never read
  above the run's first reading (9000 minus one drain step), so the old beat
  count held the next beat back. [Note 129](129-d08a10-steroids-heartbeat.md#d08a15---restart-mid-run-beats-at-once).
- **D08G4:** `dnstuff`, `dnitems`, `dninventory` set armor to 100 (10000, the
  original full armor value); running steroids stop and a full dose is held
  (user rule when selected). [Note 38](38-debug-cheats.md).
- Final executable `a4cd7a4f8efcfc44ef86e816adc3b245fcde9a733acd1e9f503fdf51e0235ef0`.
  Suites: `ttk-controls-test` 44 groups (new D08A14, D08G4), input,
  inventory, Python 131 OK (2 skipped), overlay guards, `check_repo.py`.
- **Not yet verified:** the user's playtests. Nothing committed.

## 2026-10-08 - D08A13 accepted; D08A14, D08A15 queued

- User: "i accept that this works." D08A13 Done; executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05` is the
  regression baseline.
- Queued: D08A14 (death and Continue end steroids and remove them from the
  inventory) and D08A15 (heartbeat silent for a few seconds when steroids
  restart mid-run). Not started.

## 2026-10-08 - D08A13 steroids independent of damage (Needs playtest)

- **Found:** the coupling is original. Duke's damage handler `0x800a40a8`
  checks running steroids first (`0x800a4154`): the hit is cancelled entirely
  (no armor or health loss) and costs the steroids 1500 of 9000. The armor
  element only displayed the countdown.
- **Changed (Modernized, `steroids` `portable`):** new entry hook
  `0x800A40A8` (one regenerated line, codegen hash unchanged, savestates
  load) turns the running bit off for Duke's damage call only. The hit takes
  the normal path (armor 75%, health, death and its return value), and the
  timer is untouched. The bit returns at the next TTK hook, before the drain,
  kick, status bar or heartbeat read it. Vanilla, `original` and
  `DNTTK_STEROID_SHIELD=original` keep the shield.
- **Evidence:** pig cop on savestate slot 3: 5-10 hits per run, timer drains
  5 per frame regardless (60 and 120 fps, armor, `dnhyper`, R dose,
  savestate); Vanilla 4 absorbed hits of 1500. Death and Continue resume the
  remaining time. Suites, Python 131, overlay guards, Vanilla route. Executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05`.
  [Note 127](127-d08a4-portable-steroids.md#d08a13---steroids-independent-of-damage).
- **Note for the user:** steroids no longer protect Duke in Modernized.
- **Not yet verified:** the user's playtest; explosions, falls and drowning
  in isolation; natural pickups.

## 2026-10-08 - D12C accepted

- User: "perfect!! fully accepted." Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f` is the regression baseline.
- D12C: a connecting kick (first or third person, Modernized) plays the wall
  bump thud `0x2007` one octave up at three times its volume, once at the
  first contact; misses stay silent. [Note 131](131-d12c-kick-impact.md).

## 2026-10-08 - D12C kick impact sound (Needs playtest)

- **What:** in Modernized, a kick that connects (an enemy, a breakable or
  other object, a wall) plays the game's own wall-bump thud `0x2007` once, at
  the first contact; empty-air, missed and out-of-range kicks stay silent.
  First-person quick kick and the third-person original kick.
- **How:** the damage sphere `0x800a979c` stores the attacker in its own
  frame only when it strikes something; host calls read that slot. Walls use
  the original segment query where the boot reaches. Third person: new entry
  hook `0x800A979C` (one regenerated line, codegen hash unchanged, savestates
  load) makes the kick's sphere call on the host and leaves the original call
  touching nothing, so damage is applied once. `DNTTK_KICK_IMPACT=off` turns it off.
- **Evidence:** private runs on the first street and the alley (pig cop, club
  door, garbage bag, head-on wall, empty air, floor, out of range); suites,
  Python 131, Vanilla route. Executable
  `3873288acd438d2e367ca2c0fffa20f0953134de61af7715522fec0ffcc87c98`.
  [Note 131](131-d12c-kick-impact.md).
- **Not yet verified:** the user's ear in play; a multi-kick crate; other levels.
- **Revision (same day):** the user found it very quiet; 92% of the thud's
  energy is below 150 Hz. It now plays one octave up (user's pick):
  `0x8006b73c` with pitch byte 48, SPU pitch `0x7E8`. Executable
  `b62af2b911cfdcc7c300ac025f00f1218b6f8c7439bbd8eda460ce9243283e3d`.
- **Revision 2:** user asked to double its volume: the impact's own sound
  object volume `+0x6a` is doubled (5192 -> 10384); live voice `0xC2A` ->
  `0x1855`. Executable `de112f13e75afef2e31543f48f09ec58c49ba4f6ada9a93b914d2e66cf0d9dbd`.
- **Revision 3:** multiplier 3 by default; the silent first kick at a wall
  was the routine refusing a `0x2007` already playing (Duke's quiet wall
  bump). A playing `0x2007` is now stopped first (`0x80068900`). Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f`.

## 2026-10-08 - D27A accepted; D12C queued

- User: "i fully accept this. great work. i have wanted this one for a long
  time so this minor change has a huge impact on me as a player." Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829` is the
  regression baseline.
- New Todo D12C: kick impact sound driven by the real kick hit result (wall,
  crate, actor), using the existing wall-collision thud (not Duke's grunt);
  empty-air and out-of-range kicks stay silent; no duplicates over targets'
  own damage sounds; kick gameplay unchanged.

## 2026-10-08 - D27A Modern Shift/run is silent (Needs playtest)

- **What:** in Modernized, pressing, holding and releasing Shift no longer
  plays the walk/run click (`0x0001`); running is unchanged and the D08A10
  steroids heartbeat still plays the same click on its beat.
- **How:** a new entry hook on the player update `0x800412A4` stores the run
  state the original toggle is about to set at `player+0x27b` first, so the
  original sees no change and never clicks (`run_click.inc`; one regenerated
  line, codegen hash unchanged, savestates load). Vanilla's L1 toggle still
  clicks. `DNTTK_RUN_CLICK=original` keeps the click.
- **Evidence:** SPU KEYON counts of the click sample per leg: Shift taps 10 -> 0,
  hold/release 2 -> 0, walking taps 8 -> 0; beat identical with or without
  Shift (19 beats, 15/15/18 fields). Suites and Python 131 pass. Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`.
  [Note 130](130-d27a-silent-shift.md).
- **Not yet verified:** the user's ear in play.

## 2026-10-08 - D08A11 accepted; D27A and D08A12 queued

- User: "this job is fully accepted." Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892` is the
  regression baseline.
- New Todo D27A: Modern Shift/run silent. The sound is the known walk/run
  toggle click `0x0001` via `0x8006bbd8(1)` (note 129); remove it from the
  Modern Shift path only and keep the D08A10 heartbeat, which uses it.
- New Todo D08A12: Modern dynamite - selecting it (6 twice, mouse wheel) must
  not commit Duke to a fuse; research the original equip/arm/fuse state
  machine first; Dynamite Behaviour Modern (Modernized default) / Original.

## 2026-10-08 - D08A11 `dnhyper` in the steroids box (Needs playtest)

- **What:** with portable steroids, `dnhyper` now counts down in the D08A8
  steroids box (lit pill icon, stacked or selected like a used dose) and the
  armour element shows only armour. It ends with nothing held.
- **Cause and fix:** `dnhyper` sets the running bit without the owned bit;
  the HUD predicates in `steroids.inc` now treat the running bit alone as
  running steroids, and the status-bar hide/restore no longer depends on the
  owned bit (savestate-safe). A pickup during a `dnhyper` run selects steroids
  (`pickup_select.inc`). R still takes only a held dose.
- **Evidence:** 60 and 120 fps private runs (no armour, armour 50, savestate
  reload, end, pickup refresh, R dose regression, `original` profile); suites
  and Python 131 pass. Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892`.
  [Note 127](127-d08a4-portable-steroids.md) (D08A11 section).
- **Not yet verified:** the user's look in play.

## 2026-10-08 - D08A10 accepted; D08A11 queued

- User: "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!" Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca` is the regression baseline.
  [Note 129](129-d08a10-steroids-heartbeat.md).
- New Todo D08A11: `dnhyper` shows its countdown in the original armor
  element instead of the D08A8 steroids box. Likely cause: `dnhyper` sets the
  running bit without the owned bit that the HUD predicates require.

## 2026-10-08 - D08A10 sound fix: the Shift click (Needs playtest)

- User on the first build: "the beat is right but the sound is wrong. i want
  the sound specifically when you press shift". Found it: sound `0x0001`, the
  walk/run toggle click, played through the non-positional call
  `0x8006bbd8(1)` (SPU sample `0x012F0`, pitch `0x228`-`0x22F`). The beat now
  makes that exact call; same sample and pitch as a real Shift press in levels
  0 and 6, rhythm unchanged. Suites pass. Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`.
  [Note 129](129-d08a10-steroids-heartbeat.md).

## 2026-10-08 - D08A10 steroids heartbeat (experimental, Needs playtest)

- **What:** in Modernized with portable steroids, a low heartbeat thump (the
  game's own sound `0x1012`, Duke's bank 1) plays at Duke while steroids run,
  225 bpm as Duke 3D's `DUKE_HARTBEAT` (reference video: 268 ms between beats).
  Stops with the effect, a damage cut skips ahead, death and level change end
  it. `DNTTK_STEROID_BEAT=off` silences it, `=<id>` tries another sound (the
  Shift-run footstep is `0x2000`). Console `sfx <id>` auditions any sound.
- **How:** `steroids_beat.inc`, one beat per 80 units of the steroids timer
  (300 units/s) from the existing authenticated player-update hook; the
  original sound call on a private stack. No new hooks or codegen change.
- **Evidence:** identical rhythm at 60 and 120 fps; pickup + R, savestate load,
  damage cuts, `level 6`, level 6 play, `original`/`off`; suites and Python 131
  pass. Executable
  `d0d1f09355d69f4ea3ff1a1a6ea7737bf261f512d83443b3d0f3edf20c861acf`.
  [Note 129](129-d08a10-steroids-heartbeat.md). Previews:
  `recomp/analysis/d08a10-beat/preview/` (local).
- **Not yet verified:** the user's ear (keep, change sound, or revert); the
  rhythm steps 250/250/300 ms in 20 Hz scenes; death while running.

## 2026-10-08 - D08A9 accepted; D08A10 queued (experimental)

- User: "fully accepted, working beautifully." Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6` is the
  regression baseline. [Note 128](128-d08a9-pickup-selection.md).
- New Todo D08A10 (experimental, may be reverted): while steroids run, a
  heartbeat-like sound at 226 bpm as in Duke 3D, using a game sound (an
  existing heartbeat if any, else Duke's Shift-run sound). Reference video
  `research/screencaps/Video_2026-10-07_23-59-03.mp4`.

## 2026-10-07 - D08A9 picked-up gadget becomes the selection (Needs playtest)

- **What:** in Modernized, picking up a jetpack, Bio Mask, goggles, medkit or
  steroids (or topping one up) makes it the `[ / ]` selection and the HUD box
  at once, as in Duke 3D; Enter / U then use it. Keys, mission items, full
  gadgets left on the ground and `dninventory` leave the selection alone.
- **How:** `pickup_select.inc` compares items 1-5 around the original pickup
  dispatcher (`0x80081a48` entry, `0x8001ca4c` ra `0x8007fe78`; existing
  hooks) and selects through the guest menu ID `0x800c3f94`. Waits while a
  gadget is mid-activation. No new hooks or codegen change.
- **Evidence:** private Xvfb runs in levels 0 (60 fps) and 6 (120 fps), new
  native D08A9 group, suites and Python 131 pass. Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`.
  [Note 128](128-d08a9-pickup-selection.md).
- **Not yet verified:** natural (non-spawned) pickups, death/Continue, the
  user's feel.

## 2026-10-07 - D08A8 accepted; D08A9 queued

- User: "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8` is the regression baseline.
  [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A9: when Duke picks up a gadget (jetpack, Bio Mask, goggles,
  medkit, steroids), it becomes the `[ / ]` selection and the HUD box, as in
  Duke 3D. Mission items and keys leave the selection alone.

## 2026-10-07 - D08A8 steroids countdown in the steroids box (Needs playtest)

- **What:** in Modernized (`steroids` `portable`), running steroids count down
  in their own HUD box with the pill icon, lit: in the item slot while selected
  (they now stay selected after R, so the box counts down in place), otherwise
  one box row above (above an unselected jetpack that is on). The armour element
  on the left shows only armour, and stays hidden with no armour.
- **How:** status bar element 2 shows steroids whenever `+0x364` bit 1 is on.
  For the status bar draw only, bit 1 is off; it comes back at the first hook
  after the status bar returns (`0x8001fc44`, or new lightweight hook
  `0x8002E850`; one regenerated line, savestates still load).
- **Evidence:** private Xvfb runs (GL/Software, 4:3/16:9, 60/120 fps), Vanilla
  unchanged; native suites and Python 131 pass. Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`. [Note 127](127-d08a4-portable-steroids.md).
- **Accepted** (see the entry above).

## 2026-10-07 - D08A4 accepted; D08A8 queued

- User: "mechanically, the steroids work perfectly. you pick them up, you can
  press r to run, and thats it ... i accept this job as complete now as it's
  functional". Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5` is the
  regression baseline. [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A8: the running countdown uses the original armor element and
  icon; show it in the steroids box with the pill icon instead (reference
  `research/screencaps/ttk-roids.png`).

## 2026-10-07 - D08A4 built: portable steroids, EDuke32 style (Needs playtest)

- **What:** in Modernized (`steroids` `portable`, the default), picking up
  steroids keeps them: STEROIDS shows as before, and they appear in the `[ / ]`
  switcher after the medkit and in the HUD item box with the user's own
  pill-bottle icon. **R**, or Enter/U with steroids selected, takes them
  (`USED STEROIDS`, the pickup sound, the original effect). One at a time: more
  steroids stay on the ground while one is held; a pickup while they run
  refreshes them, as in the original. `original` and Vanilla keep the original
  rule. `run.py --steroids portable|original`, `--settings` S (profile schema 29).
- **How:** the original still has a held steroids item (item 4, `+0x364` bit 0
  with its amount `+0x366`); only its pickup case `0x800827e8` runs it at once.
  The pickup is turned into a held item at the pickup tail's own sound call (new
  entry hook `0x8006B73C`, one regenerated line; savestates still load). A first
  design with a count in spare flag bits was replaced after a write trace showed
  the level-end snapshot `0x80083348`, which the card save writes, keeps only
  bit 0.
- **Evidence:** private copies of the user's cards and savestates, levels 0 and
  6: pickup held, second pickup left, R runs it, refresh while running;
  savestate reload, level completion with the stats-screen card save, loading
  that save, and death with Continue keep it; `original` and Vanilla unchanged;
  suites pass. Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5`.
  [Note 127](127-d08a4-portable-steroids.md). Next: the user's playtest.

## 2026-10-07 - D08V1 accepted

- User: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area". Their playtest log `session-20261007-202257.log` shows
  two chain top exits (196) with no identity failure.
  Executable `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`
  is the regression baseline. [Note 126](126-d08v1-control-loss.md).

## 2026-10-07 - D08V1 built: Modernized controls never lost to a chain exit (Needs playtest)

- **What:** climbing a chain or pole out at the top (the original exit 196,
  which the user reached after a mid-air catch) no longer switches Modernized
  controls off for the rest of the session. If the game's code check ever fails
  for real, the screen says `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`,
  the mouse turns Duke and WASD move him, and Modernized returns when the check
  passes. Vanilla unchanged.
- **Cause:** the original pole/hang side probe `0x800439e4` toggles bit 0x40 of
  the flag-table entry of Duke's current animation. A code guard covered that
  table, so the game's own write (`0x61 -> 0x21` on entry 196) read as tampered
  code. The fallback had no way to turn.
- **Fix:** the table is now a masked state guard (bit 0x40 on all 280 entries,
  and entry 265's low half for `BONUS.OVR`), with every other bit still
  authenticated. New `guard_writer_audit.py` (static, delay-slot aware, and live
  via `--port`) found no other reachable writer into any guard set.
- **Evidence:** private copy of UI slot 3. The baseline reproduced the lockout
  on the first top exit. Fixed: top exit, catch jumps, a missed jump and fall
  keep the lease; the live audit is clean; a forced loss turns with the mouse,
  moves and recovers; Vanilla host idle; suites pass. Executable
  `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`.
  [Note 126](126-d08v1-control-loss.md). Next: the user's playtest.

## 2026-10-07 - D08J5 accepted; D08V1 queued

- User: "that definitely works, and i accept it ... it works perfectly."
  Executable `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`
  is the regression baseline. [Note 125](125-d08j5-chain-descent.md).
- New job D08V1: after a missed jump that caught the chain in the air, the
  code guard on the upper-body animation flag table (`0x800c2a8c`) tripped on
  entry 196 (the original cleared bit 0x40 at run time) and Modernized
  controls stayed off for the rest of the session. Log copied to
  `recomp/analysis/control-loss-20261007/`. Modern controls must never be lost
  this way.

## 2026-10-07 - D08J5 built: climb down chains and poles (Needs playtest)

- **What:** in Modernized, at the top of a chain or pole press **E**
  (`E TO CLIMB DOWN` shows): Duke grabs it, swings round to the far side and
  hangs facing the platform. **S** climbs all the way down and steps off at the
  bottom; **W** climbs back up; **Ctrl** or **Space** lets go anywhere; S at an
  end with nothing below lets go. Vanilla unchanged.
- **Cause:** the original has no top mount for poles/chains and the jump from
  the slot-3 platform overshoots. Its down probe also accepts a floor above
  Duke, so on the platform side near the top Down climbed him back out
  (original, Vanilla too); a new codegen hook `0x8003964C` stops that in
  Modernized (regenerated one line; savestates still load).
- **Evidence:** private copy of UI slot 3: mount, full descent and step-off in
  third and first person, W exit, Ctrl/Space, A/D, platform-side descent;
  Vanilla and the slot-5 ladder regression unchanged; suites pass. Executable
  `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`.
  [Note 125](125-d08j5-chain-descent.md). Next: the user's playtest.

## 2026-10-07 - D08J3 accepted; D08J5 queued

- User: "we have an absolute winner once again!! it's working well, very fun,
  looks great." Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520` is the regression baseline.
  [Note 124](124-d08j3-climb-camera.md).
- Backlog: D08J5, climbing down chains (UI slot 3, re-saved with a chain in
  front of Duke).

## 2026-10-07 - D08J3 follow-up: climbs centre the view (Needs playtest)

- User: "it works perfectly!" and asked that climbs centre the view whatever
  the over-the-shoulder setting. Built: the shoulder offset eases to centre
  during ladder/pole/chain climbs and back afterwards; the saved preference is
  unchanged. Chain run and suites pass. Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`. [Note 124](124-d08j3-climb-camera.md).

## 2026-10-07 - D08J3 built: free camera on ladders, poles and chains (Needs playtest)

- **What:** in Modernized, the mouse camera stays free while Duke climbs
  ladders, poles, chains and climbing walls (orbit, look up/down, Alt+wheel,
  V recentre). On poles and chains A/D go left/right on screen from any
  camera side. Vanilla unchanged.
- **Finding:** the climb keeps the normal camera; in mode 3 the original only
  swaps in a high look-down boom, and the host orbit lease stopped there.
- **Evidence:** private copies of UI slots 5 (ladder) and 3 (chain): full
  orbit while climbing, climbing unchanged, A/D screen-relative behind, front
  and side; ladder top/enemy/descent regression and Vanilla unchanged; suites
  pass. Executable `2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`.
  [Note 124](124-d08j3-climb-camera.md). Next: the user's usability playtest.

## 2026-10-07 - D08J4 accepted

- User: "it's a complete winner for me. I totally accept." Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa` is the regression baseline. [Note 123](123-d08j4-ceiling-hang.md).

## 2026-10-07 - D08J4 built: climbable ceilings (Needs playtest)

- **What:** in Modernized, hanging from a climbable ceiling (monkey bars),
  WASD move Duke relative to the camera like walking and he no longer lets go
  in mid-travel; at an end he keeps hanging. Ctrl or Space lets go. Vanilla
  unchanged.
- **Cause:** the falls were ours: the D08X object-hang release let go after W
  had been held about 40 updates (and on S). The tank steering is original
  (mode 7: Up/Down along the facing, Left/Right turn).
- **Evidence:** private copy of UI slot 3: camera-relative travel in all
  directions, no drops, ends hold, Ctrl/Space let go; suites pass. Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`.
  [Note 123](123-d08j4-ceiling-hang.md). Next: the user's playtest, including
  whether the red-carpet ledge can be reached.

## 2026-10-07 - D08J2 accepted; D08J4 queued

- User: "fully and completely accept it all." Executable
  `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the regression
  baseline. [Note 122](122-d08j2-pole-chain-sidestep.md).
- Backlog: D08J4, ceiling monkey-bar climbing drops Duke at the wrong points
  (UI slot 3, re-saved at the top of the chain).

## 2026-10-07 - D08J2 built: pole and chain A/D direction (Needs playtest)

- **What:** in Modernized, while Duke hangs on a pole or chain, A takes him
  round it to the left and D to the right on screen. Vanilla unchanged.
- **Cause:** original. Left/Right on the hang-climb (192..195) reach the
  sidestep `0x800439e4`; D-pad Left carries Duke to his own right, then
  `0x80043c08` turns him to face the object. A was D-pad Left.
- **Evidence:** private copy of UI slot 3 (the chain): A/D in third person and
  the first-person profile, W/S climbing; Vanilla D-pad unchanged; suites pass.
  Executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`.
  [Note 122](122-d08j2-pole-chain-sidestep.md).

## 2026-10-07 - D08U2 accepted

- User: "fully accepted. this is exactly what i wanted. well done!" Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e` is the
  regression baseline. [Note 121](121-d08u2-ladder-top-enemy.md).

## 2026-10-07 - D08U2 built: climb off a ladder past an enemy (Needs playtest)

- **What:** in Modernized, an enemy (or other character) standing near a
  ladder top no longer keeps Duke on the top rung: holding W, he climbs off
  with the original exit and stops short of the enemy. Walls and other
  objects still block; the enemy is not pushed (user: optional). Vanilla
  unchanged.
- **Cause:** original. The exit test `0x8007d240` refuses when its forward
  probe `0x8007cde8` meets any cell object, characters included.
- **Evidence:** private copies of UI slots 5 and 12: slot 5 in both views,
  the second alley ladder with an enemy respawned on its landing, D08U/D08U1
  mounts and descents unchanged; suites pass. Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`.
  [Note 121](121-d08u2-ladder-top-enemy.md).

## 2026-10-07 - D08U2 queued

- Backlog: D08U2, an enemy standing at a ladder top stops Duke climbing off;
  Duke should push it back. Test location: the user's UI slot 5.

## 2026-10-07 - D08O3 accepted; D08J2 and D08J3 queued

- User: "fully passed my playtest. feels great to play. i accept this job
  now as complete!!" [Note 120](120-d08o3-e-in-water.md).
- Backlog: D08J2 (poles and chains: A turns right, D turns left; make A
  left and D right; chain test location: the user's UI slot 3) and D08J3 (free camera while climbing ladders, poles
  and chains; needs investigation and usability testing).

## 2026-10-07 - D08O3 built: E in water keeps the weapon (Needs playtest)

- **What:** E while swimming still stows the weapon (needed to climb out at
  a ledge), but without a climb-out Duke draws it again automatically,
  without stopping his swimming. Land E and Vanilla unchanged.
- **Evidence:** private level 6 lab: walls, ledge, underwater, 11 swimming
  runs; regressions and suites pass. [Note 120](120-d08o3-e-in-water.md).

## 2026-10-07 - D08O2A accepted

- User: "this is actually rock solid ... I'm happy with it and its 100%
  playable." Known minor issue: one final shot at the end of a burst can
  fire with the weapon pointing down (seen in the user's capture).

## 2026-10-07 - D08O2A built: natural swim-fire pose v2 (Needs playtest)

- **What:** in Modernized, swimming underwater while firing now raises only
  Duke's arms and weapon toward the crosshair; the chest, head and legs keep
  the swim stroke (v1 stood the whole upper body up). Vanilla unchanged.
- **How:** new hook on the matrix composition `0x800b42ec` re-orients the
  two shoulders to their floating-fire pose relative to the view, at either
  model build; no aim bit or other game memory is written.
- **Evidence:** private level 6 lab, six weapons x four directions, 120 fps:
  gun hand near the floating-fire value, chest unchanged from plain
  swimming; floating, surface, ground and Vanilla unchanged; suites pass.
  [Note 119](119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O2 accepted (v1); D08O2A queued

- User: "it works and i accept this as a v1". The more natural pose (torso
  stays in the stroke, arms raised to fire, head looking up) is Todo D08O2A.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D08O2 built: weapon forward while swimming and firing (Needs playtest)

- **What:** in Modernized, swimming underwater while firing turns Duke's
  upper body and weapon toward the crosshair, as when he fires floating still;
  the legs keep the stroke. Vanilla unchanged.
- **How:** new hook `0x800411b8` sets the original aim bit `0x100` for Duke's
  model build only (cleared at the neck's aim call), so the original arm-aim
  builder runs during the stroke and the D07A hook gives it the view.
- **Evidence:** private level 6 lab, six weapons x four directions, gun hand
  along the view equals the floating-fire pose; 60 and 120 fps; Vanilla,
  ground and surface unchanged; suites pass.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D23E and D24A accepted

- User: "we can mark D23E and D24A both as accepted. we've proven these are
  now working significantly better". D23E (busy-scene stutter,
  [note 104](104-d23e-busy-scene-stutter.md)) and D24A (open assets,
  [note 112](112-d24a-open-assets.md)) are Accepted. No code changed.

## 2026-10-07 - D08A7 accepted: custom medkit gadget icon

- User: "amaazing work. i accept" (Done). The medkit shows the user's sprite
  in the switcher and the HUD box; health keeps the cross.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A7 built: custom medkit gadget icon (Needs playtest)

- **What:** in Modernized the medkit's inventory picture is the user's own
  sprite, full colour in the `[` / `]` strip and 15 colours in the D08A6 HUD
  box. The health box keeps the cross. Vanilla unchanged.
- **How:** the strip pack takes item 5 from `assets/ui/items/gadget-medkit.png`;
  the HUD box draws a medkit record whose 4bpp cell and palette sit in rows the
  disc's HUD sheet leaves empty (cell 960,205, palette 1008,206), loaded by a
  GP0 `A0` packet in the HUD ordering table each frame the box is drawn.
- **Evidence:** live VRAM survey (band empty in intro, title, pause, Select,
  six levels; the game fills x 960-991 from row 223 at run time), GL 4:3 /
  16:9, Software 4:3, 120 fps with savestate reload, Vanilla no upload.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A6 accepted: selected gadget on the HUD

- User: "i fully accept!" (Done). The gadget picked with `[` / `]` always has
  its original-style box over ammo, lit when on, dim when off; it follows the
  selection at every frame rate. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 playtest fix (Needs playtest)

- User saw the medkit box stay after hovering the jetpack, with both shown.
  Cause: at 120 fps the D17 worker redraws replay the status bar on a copy of
  guest memory, where the host selection was stale. The box now reads the
  original menu ID `0x800c3f94`, kept equal to the selection.
- `[` / `]` now hover in every gameplay state (flight, swimming, firing); the
  selected jetpack lights in place without sliding out of the ammo box.
- Verified from a copy of the user's savestate with the user's settings,
  three runs. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 built: selected gadget on the HUD (Needs playtest)

- **What:** in Modernized the gadget chosen with `[` / `]` (the one Enter / U
  uses) always has a box at the original item slot over ammo: the HUD's own
  box, icon and red digits with its charge. Lit when on, dim digits when off,
  medkit (health cross) always lit. An unselected jetpack that is on moves
  one row up; Bio Mask / goggles keep their original place left of ammo.
- **How:** the status bar's own calls, from its entry hook after the D14 shift
  (`src/ttk/gadget_hud.inc`); dim digits are the original glyph packets at
  colour 0x31. The whole HUD layout is now saved and restored each draw.
- **Evidence:** private Xvfb runs: every selection, cycling, N / J / Enter,
  stacking, savestate load, level change, GL 4:3 / 16:9, Software 4:3, Vanilla
  unchanged; native tests and Python 120 OK.
- **Open:** user look confirmation; death / Continue and resize not run;
  Software 16:9 cuts the right HUD with or without this job (existing).
  [Note 115](115-d08a6-selected-gadget-hud.md).







## 2026-10-07 - D08A5 accepted: mission inventory on , / .

- User revision after the first try: mission items are their own **mission
  inventory** on `,` / `.` (previous / next, the < > keys), in the switcher's
  place with the item card at the top. `[` / `]` show gadgets only. Enter / U
  while it is open just close it, so an instinctive Enter no longer toggles
  the jetpack. The `\` binding is gone.
- Original strafe (Comma/Period, PS L1/R1) is unbound by default; profile
  schema 28 moves the keys and drops schema 27's `\` action.
- Verified in native tests, Python 120 OK and a private Xvfb run (browse,
  wrap, Enter swallow, `[` back to gadgets). User: "i fully accept!" (Done).
  Open for later: controller input for both inventories.
  [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - D08A5 built: mission item tracking (Needs playtest)

- **What:** design E is in the game (Modernized only). `[` / `]` show a mission
  row under the gadgets in every level with mission items; `\` browses it and
  shows the item card at the top (FOUND / NOT FOUND YET), clearing 2.5 s after
  the last press. The gadget frame is the tile0020 orange swap.
- **Playtest fixes:** `\` now works on UK keyboards (the key left of Z, SDL
  `NONUSBACKSLASH`); mission and gadget frame borders are even (pixel-centre
  stretch) and the panel outline has margin below it.
- **Profiles:** schema 27 adds the `\` binding to saved profiles (custom
  bindings kept); older launchers refuse the migrated file.
- **Data:** the original Select inventory's own list (name function
  `0x80087d4c`, flag `player+0x354+4*i` bit 0). Crystals are items 11-13 and
  level 6's skeleton keys items 7-8, not the ticket's guesses.
- **Evidence:** native and Python suites pass; private Xvfb runs show the row,
  card, timeout, a live jewel pickup (1/5), level travel and no row in level 8;
  Vanilla unchanged. `ttk-controls-test` fails on stale fixtures with or
  without the change.
- **Open:** user look confirmation in play; controller D-pad focus is not
  implemented (no controller path opens the switcher); the block sits about
  26 px higher than the mockup. [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - Mission item icons in; tracking design E locked; D08A5 queued

- **Icons:** the user's original 16x16 icons are tracked in
  `recomp/assets/ui/items/`, with `items.json` mapping all 15 mission items
  and steroids to a file, the game's object types and levels:
  - one keycard drawing serves all seven key-slot items;
  - Scrap of Paper also serves Torn Paper (user-confirmed);
  - every other item has its own drawing.
  `test_ui_art.py` checks coverage and format.
- **Keycards:** every key-slot key (types 153/154, named per level) is the same
  flat keycard object in the game, including medieval level 7. A pig carries
  and drops it there. Checked in RAM.
- **Tracking design:** the user compared four mockups (A panel, B HUD tracker,
  C pickup card, D switcher row) in the research page and iterated a fifth,
  **E**, which is locked: the mission row under the `]` switcher, `\`
  browsing, and a card at the top. Exact spec in **D08A5** on the board.
- **Next session: build D08A5.** Start from the board entry. The working
  mockup code (layout, palette swaps, card) is `drawE()` and `swapped()` in
  `recomp/analysis/d24a-fonts/picker.template.html` (local).
  - Switcher code: `src/ttk/inventory_hud.cpp`; packs via
    `tools/local/build_ttk_inv_icons.py`.
  - Item names and found state: the player's mission flags (`+876`..`+916`,
    see the job). Level crystals (items 11-13) are confirmed by D26F
    ([note 117](117-d26f-crystals.md)): taken from holders, not walk-over
    pickups; `spawn 761` (generic crystal) sets item 14 instead. D26F (accepted)
    adds `spawn 1761/2761/3761`: the real crystals as custom
    walk-over pickups (recipe in note 117).

## 2026-10-06 - D24A, D24B, D26E: open assets, TTK fonts, savestate menu, debug spawn

User, end of session: "im happy with all progress tonight. the document is
superb and an amazing piece of research."

- **D24A (Needs playtest: fresh-clone test).** No Duke Nukem 3D art or
  `research/` input remains:
  - TTK disc fonts (Big and Medium Italic, system 8x8) are built from the
    player's disc;
  - switcher digits are the CC0 3x5 Microfont;
  - the selection frame and crosshair are the user's original art;
  - HUD icons are built from the disc in every build;
  - messages and the console draw at half scale, in the "Console steel"
    palette with a navy shadow.
  The README credits PSXRecomp, Alexbeav's PS1 Recomps, recomp-ui and
  Microfont. Remaining: the fresh-clone test against the pushed commit.
  [Note 112](112-d24a-open-assets.md).
- **D24B (Accepted).** The F7 savestate menu uses the TTK fonts and disc art,
  with level names recorded per slot. [Note 113](113-d24b-savestate-menu.md).
- **D26E (Accepted).** The console has `spawn <item>` and `items`, now working
  from the real console and along the camera view; the half-sunk landing is
  the game's own drop. It also gained scrollback (512 lines, PgUp/PgDn/wheel),
  Up/Down command recall and `history` (D26C, accepted).
  [Note 111](111-d26e-debug-spawn.md).
- **Research page:** the local UI and font research page (fonts, palettes, UI
  sprites, BS stills, per-level mission items, design slots, savestate mockup)
  is `recomp/analysis/d24a-fonts/ttk-font-picker.html`. It is retail-derived
  and local only; `research/TTK-UI-and-Font-Research.html` links to it.
- **Next:** the user designs the open icons (15 mission items and keys, plus
  steroids) for the placeholders on that page.

## 2026-10-06 - D24A queued (public clone completeness)

A fresh-clone test of `main` built and played the same as the working copy,
but lacked the TTK inventory icons (disc-derived, not built by `build.py`),
the Duke message font and the green digits (Duke 3D art in `research/`). New
Todo D24A covers moving them into a proper build route. It notes that Duke 3D
art cannot be committed without a licence, and lists the options (derive from
the TTK disc, an original font, or user-supplied files).

## 2026-10-06 - D08Q6 accepted

User accepted the Modern jetpack fix ("i accept this as fixed! the test passes
and this feels great"): revision 5b, with height held, the original's speed,
flame and poses. Not committed.

## 2026-10-06 - D08Q6 revision 5b: original speed, flame and poses (Needs playtest)

Modern flight now moves at the original jetpack's top speed (about 10x the
5a cut) and shows the original flame (moving, climbing and hovering) and lean
poses. The original's flight inputs are fed again and their thrust is
cancelled so the host still owns the velocity. Height hold, landing, J, fire,
switching and fuel are verified live; Classic and Vanilla are unchanged; all
suites pass. Next: the user's playtest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 5: Modern flight model rewritten (Needs playtest)

Root cause found: the original root apply `8004ac08` throws away horizontal
motion whenever the vertical root is 0, so a precisely held height left WASD
dead unless Space or Ctrl was held. Modern flight was rebuilt from scratch.
The host owns the flight velocity: eased camera-relative WASD/arrows,
Space/Ctrl, height hold with the floor cap, and a +-1 vertical step while
moving. The original keeps collision, landing, cut-outs, firing and fuel
(drained at the original rate). Verified live with a copy of the user's
profile; all suites pass. Duke keeps the hover pose while moving. Next: the
user's playtest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 4: Right Shift dropped capture; idle creep (Needs playtest)

From the user's session log: pressing Right Shift sent the original Select,
which silently released Modernized capture mid-flight. The host stopped with
the hover lock on, so WASD barely moved until Space. Right Shift now counts as
Shift and never sends Select while captured (game-wide; on foot it runs like
Left Shift). The idle creep was a stalled horizontal coast, now stopped while
hovering. Both were reproduced and fixed live. Suites pass. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 3: Shift and arrow keys in flight (Needs playtest)

Shift no longer reaches the original hover toggle in Modern flight; that
toggle was dropping Duke 128 units against the new hover. The arrow keys now
fly like WASD; before, they sent raw D-pad input that the hover pin throttled.
`dnkroz` keeps the jetpack the whole time god mode is on. Live: arrows fly,
Shift has no effect on height, and the largest step is 7 units. Suites pass.
Next: the user's retest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - `dnkroz` now gives the jetpack (D08Q4 follow-up)

`dnkroz` turning god mode on now also grants the jetpack (full fuel) when Duke
lacks one, at the user's request during the D08Q6 retest. Unlimited fuel for
an owned pack already worked. Cheats are still refused in mid-air. Verified
live and in the native suite. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08Q6 revision 2: smooth hover (Needs playtest)

The user found revision 1 jerky. Its hover twitched +17/-25 about every 6
updates, because phase 0 left one timestep of the steep part of the bob sine and
the error-driven slew echoed it. Revision 2 parks the bob at its flat peak and
slews the base gently. Hover is flat and stops settle over 3 updates, while the
altitude still holds. Suites pass. Next: the user's retest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 Modern jetpack altitude hold (Needs playtest)

Modern flight no longer creeps upward. The altitude captured when Space or
Ctrl ends is held through flight, turning, looking up/down and hovering.
Level flight steers back to it, hover pins to it with the original bob off, and
it is capped at the original floor approach height over higher ground. The lab
circuit ends within 9 units (before: about 460 units of climb in six legs).
Ctrl landing, J fall, firing, weapon switching, Classic and Vanilla are
unchanged. Suites pass. Next: the user's LEVEL01 playtest. Not committed. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q5 accepted; D08Q6 queued

User accepted weapon switching in jetpack flight ("excellent!! i accept.").
New Todo D08Q6: in Modern flight Duke's altitude creeps upward over long
circuits (LEVEL01 apartment/alley, `dnkroz`, much looking up); height should
stay fixed unless Space or Ctrl is used. Not committed.

## 2026-10-06 - D08Q5 weapon switching in jetpack flight (Needs playtest)

Number keys, the wheel, semicolon/apostrophe and X now switch weapons while
flying (Modern and Classic). The original mode-10 state already completes the
ground weapon request with the normal holster/draw; our shortcut layer had
stopped at the J check. Verified live with every weapon, firing after each
switch; swim rules, ground and Vanilla unchanged; suites pass. Dynamite's own
blast still knocks Duke out of flight (original). Next: the user's playtest.
Not committed. See [note 108](108-d08q5-jetpack-weapon-switch.md).

## 2026-10-06 - D08G3 and D08Q4 accepted; D08Q5 queued

User accepted `dnupgrade` and the Duke3D-style `dnkroz`. New Todo D08Q5:
weapon switching with number keys and the wheel while flying the jetpack.
Committed and pushed.

## 2026-10-06 - D08G3 `dnupgrade` and D08Q4 `dnkroz` (Needs playtest)

`dnupgrade` upgrades weapons 4/5/7/8/9/10 through the original upgrade bit and
the persistent mask `+0x85f` (which saves and pickups reapply), carrying ammo
to the Laser Gatling, Incendiary RPG and HiTemp Flamethrower records.
`dnkroz` now raises health to 100 (never lowering an Atomic Health surplus;
the original pickup already caps at 200) and keeps an owned jetpack full in
every scheme. Verified live in levels 6 and 12, Modern and Classic flight, a
savestate round trip and Vanilla, on private copies; suites pass. Next: the
user's playtest. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08O1 accepted; D08O2 queued

User accepted fire while swimming ("mechanically, it does exactly what it's
meant to"). Remaining issue, now Todo D08O2: the weapon points downwards while
Duke swims and fires; make it point forward. D08Q4 scope extended to Duke3D's
`dnkroz` (health to 100, unlimited jetpack, Atomic Health to 200). Committed
and pushed.

## 2026-10-06 - D08O1 fire while swimming (Needs playtest)

Underwater the original tests fire before the swim thrust, so holding fire
stopped Duke. Modernized now keeps him swimming while he fires (fire hidden
from the swim handler only, restored right after; new hooks `0x800455bc`,
`0x80055e80`, codegen hash unchanged). Swim shots use view aiming and the
crosshair; weapon keys work in water, limited to the weapons the original
allows there. Verified live in level 6 on private copies; Vanilla, ground and
jetpack firing unchanged; suites pass. Next: the user's playtest at their
location and a second level. Not committed. See
[note 107](107-d08o1-swim-fire.md).

## 2026-10-06 - Backlog: D08A4, D08S reopened, D08Q4, D08Z1, D11F; D08O1 started

User requests: D08A4 EDuke32-style portable steroids (stored item, R to use);
D08S reopened as the EDuke32 jetpack scheme (instant J on/off, midair, 61 s
of fuel); D08Q4 `dnkroz` gives unlimited jetpack fuel; D11F first person while
flying the jetpack; D08Z1 keep jump momentum when bumping a wall (Modernized
option for the future menu). All Todo. D08O1 (fire while swimming) selected
and In progress.

## 2026-10-06 - D23H accepted

User accepted after a play session with several saves. Not committed; no next
job selected.

## 2026-10-06 - D23H implemented (Needs playtest)

Cause measured: a shed was permanent at fixed/display rates (`late_pace` runs
only at Unlimited on the present timeline), and one-off hitches (saves 83-98 ms,
F7 menu) shed. New `replay_load_window()` (`src/ttk/frame_replay.cpp`), driven
by `overclock_lease()`: shed on two behind seconds in a row, step up after clean
seconds with decaying backoff. `[TTK pace]` log lines show every change and a
per-minute summary. Offscreen A/B and a 21-minute run hold 120. Next: the user
plays a long session with saves (the default settings) and checks the session
log's `[TTK pace]` lines. Not committed.

## 2026-10-06 - D29 added (progression items and objectives legibility)

From the user's Level 2 playthrough: the bank-vault papers can be collected
without noticing, cannot be inspected later, and nothing connects them to the
vault (the user needed a walkthrough). New Todo D29, research and design
first: how the papers, the code data and the vault check work internally,
what the game tracks and shows, whether original item data and Objectives
text can be read or updated (not by extending the inventory switcher; the
user will bring presentation options), a game-wide audit of similar opaque
progression, a lightweight information model, presentation options, objective
guidance without spoilers, and a smallest first build. Principle: remove
unnecessary obscurity without removing discovery. Not started.

## 2026-10-06 - D23H added (presents fall to about 60 over long play)

The user reports that after about 10 minutes of play on the D23F default, presents
fall from 120 to what looks like 60 and stay there. The session logs tie all 13
slowdown events to savestate saves and loads (about a second's stall). Likely
mechanism: D23E sheds presents to every 2nd refresh, and the step-up backoff
(`pace_backoff`) doubles and never decays, so after a few saves 120 needs minutes of
perfectly clean play to come back. Not reproduced or measured yet: presents/s
are not logged. Job written up with the evidence and a measure-first plan; to
continue next session. Nothing built or committed.

## 2026-10-05 - D23F accepted; fast timing is the Modernized default

User accepted D23F ("absolutely beautiful ... responsiveness is literally
like PC accurate now") and made it the default: profile schema 26,
`cpu_timing` fast by default (older saved accurate switches once). New Todo
jobs D23G (finish the fast path: dispatch, overlays, interpreter, observers,
redraws) and D17S (auto frame-rate default capped at 120). Documentation,
commit and push not yet authorized. No next job selected.

## 2026-10-05 - D23F fast CPU timing: Needs playtest

New Modernized setting `--cpu-timing fast` (default accurate; Vanilla always
accurate): a purpose-built timing model for the recompiled game code,
force-included into the generated shards, leased from gameplay. Generated C
and hashed headers untouched (savestates still load). At 150%/120 Hz slots
1, 9, 10 keep 120 presents/s (accurate: about 40) with no shedding or pauses;
the emulation thread drops by about 40% at 100%. Candidate
`e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966`,
[note 105](105-d23f-fast-timing.md). The Vanilla route diverges from its old
captures with or without D23F (cause not established). Nothing committed.
Next: the user plays with `--cpu-timing fast --cpu-overclock 150` at 120 Hz
and decides whether fast becomes the default.

## 2026-10-05 - D23F added: faster timing model (big Todo)

The user prefers 100% CPU for now. At 150% and 120 Hz one emulation thread
cannot run the faster game and 120 redraws per second, so presents drop even
in light scenes (slot 1 75/s, slot 9 54/s); still images are identical. New
big Todo D23F: observers off in player sessions, then a design study of a
Disruptor-style fast timing model (cycle model is about 34% of the thread).
D23E stays Needs playtest at 100%. User authorized commit and push.

## 2026-10-05 - D23E western-town stutter: Needs playtest

Slots 9 and 10 are in the western town. At the player's `cpu_overclock 100`
the 16:9 view overloads the emulated CPU (game frames 4 fields with 5-6 steps;
Vanilla 4:3 slot 9 is a steady 3). At 120 Hz the default 150% could not hold:
redraws took 19% of the emulation thread, emulation fell behind and the
safety net paused the overclock 5 s at a time. Now the redraws shed load
first (presents every 2nd/3rd refresh) and the overclock stays. 150%/120 Hz:
slot 9 about 93% of frames at 2-3 fields (was about 30%), slot 10 98% at 3.
Candidate `18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`,
[note 104](104-d23e-busy-scene-stutter.md). Nothing committed. Next: the user
plays slots 9/10 at `--cpu-overclock 150` and judges turning and audio.

## 2026-10-05 - D23E added (enemy-area stutter)

New Todo D23E: stutter while walking left/right looking down at enemies in UI
slot 9, and walking the strip in UI slot 10 with `dnkroz`. Savestate hashes
are recorded in the job. Profile first; check overlap with D23C, D23D and
D17H. Not started. No next job selected.

## 2026-10-05 - D11D and D12B accepted; closeout

User: "i can confirm that i accept both jobs as complete! document commit
push". First-person joints are found by part through the model's table, so
the cowboy levels (1, 2, 3, 27) get the right eye height, head hide and kick
leg. Regression baseline
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
No next job selected.

## 2026-10-05 - D11D and D12B: first-person joints found by part, Needs playtest

The cowboy costume (levels 1, 2, 3, 27) orders Duke's joints differently: the
neck is joint 15 there, not 9, and the right leg is joints 5-8, not 14-17. The
eye, head hide, weapon torso, arm hiding and kick leg now find joints through
the model's own part table (`desc+0x24`). The neck sits 691-705 above the toes
in all 21 levels, and the cowboy kick shows the jeans and brown boot.
Medieval, Roman and the first map are unchanged. Candidate
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
Nothing committed. Next: the user checks slot 8 (dancer height) and the kick
in a cowboy level. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 accepted; D08T2/D22C accepted; closeout

User: "accepted!! done, commit. great work". Accepted today: D22C (F10
recapture opts back in to automatic capture), D08T2 (object type range 1062:
Duke-symbol blocks push with RMB; hints on every contact) and D08U1 (slot-12
ladder: last-rung stop and let-go, mount blend finished after dropped frames).
Regression baseline
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
New Todo: D08T3 (free manual push/pull, suggested next), D08O1 (fire while
swimming); D17R gained the UI slot 2 medieval sky report. No next job
selected.

## 2026-10-05 - D08U1 cause found: mount blend cut short by dropped frames

The user's `[TTK ladder]` log showed the mount ending at y -9114 (offscreen
always -8901), and Duke frozen there while S looped the step poses. With
dropped frames the original 156 transfer ends before the host's 12-update
blend, leaving Duke above the climbing line. Reproduced with a longer
diagnostic blend (stuck at -9115 with the old code); the host now finishes
the blend while Duke rests on the ladder. Candidate
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 not reproduced; robustness fixes and ladder log line

User: slot-12 ladder still glitches on S. Their log shows Duke never reached
the bottom (only 186-189 near the top, three retries). Not reproduced with a
private copy of their profile (first person, 120 fps) for held/tapped S, E
held, E taps or mouse look. Fixed: E-held swing at the bottom, rest-pose
latch, E's Cross blocking the let-go. New `[TTK ladder]` session-log line per
ladder pose change. Candidate
`071487b1f35bcbfd9bf4734c69fd06926fb1b1c11283c2c29dce80156428784a`.
Next: the user retries and describes the glitch; read their newest
`recomp/build-local/logs/session-*.log`.

## 2026-10-05 - D08T2 accepted; D08U1 slot-12 ladder bottom fixed, Needs playtest

User accepted D08T2 (type range, hints) except the slot-12 ladder ("he just
glitches out"). Cause: the ladder's last rung is ~990 above the floor, so the
original swings off sideways (156 -> 211) into a bottom-rung hang (207). Now
the player update asks the original's own probe `0x8007ded0`; at an open
bottom the descent flag is cleared so the original stops on the last rung,
and S lets go with the original Square (clean fall and landing). Hold or tap
S both work in third and first person; W climbs back; the sewer ladder keeps
its 185 step-off. Candidate
`cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.
Nothing committed. Next suggested: D08T3 (free push/pull).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 retest fixes; D08T3 added

User retest: hints never reappeared (D08T1 capped them per session; now every
fresh touch/grab/ladder top, at most every ~5 s); slot-12 ladder S glitch
(the ladder ends above the floor and the original hangs from its bottom rung;
S now lets go with the original Square and Duke drops to the floor). New Todo
D08T3: free manual push/pull while holding Grab, suggested as the next job.
D08T2 and D08U1 Needs playtest. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 Needs playtest (also fixes D08U1)

The medieval Duke-symbol block (UI slot 1) is object type 924; the shared
object type lookup in `push.inc` refused types `>= 512`, but the original has
1062. Fixed from the original allocation (`0x7428 / 28`). RMB now grabs and
pushes it in third and first person; the D08U1 slot-12 ladder (type 639) now
mounts with E and descends; dumpster and Vanilla unchanged; native tests pass.
114 more climbable types now reach the D08X mantle too. Candidate
`06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
Nothing committed. [Note 103](103-d08t2-object-type-range.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22C accepted; D08T2 in progress; D08O1 added

User: "I accept this work." D22C is Accepted; candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b` is the new
regression baseline. D08T2 selected (UI slot 1, file 00, medieval
Duke-symbol block). New Todo D08O1: fire weapons while swimming (UI slot 2,
file 01), modelled on the D08Q3 jetpack work. D17R gained the UI slot 2
medieval sky report ("strange behaviour with the sky directly ahead").

## 2026-10-05 - D22C control mode persists through level select, Needs playtest

Reproduced with real keys: after an F10 release and F10 recapture, every
later `level N` (11 and 12 tested) arrived with the mouse free, which looks
like Legacy controls until F10. F10 that captures now opts back in to
automatic capture (`recomp/src/ttk/pc_input.cpp`); an F10 release still opts
out. Not specific to Level 11. Native input/controls/aim/near tests pass.
Candidate `f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22B accepted; game-wide playtest backlog

User: "Accepted." D22B is Accepted; executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the new
regression baseline (the candidate entry below is historical). The user's
`level N` playtest passed: first-person height and costume-aware kick in the
medieval and Roman/HOG HEAVEN eras, Level 9 armed rolls, general playability.
New Todo jobs, none started: D23C (medieval moat/Necro slowdown, profile
later), D23D (Level 9 strip-club-area slowdown, low priority), D08U1 (player
slot 12 ladder, E cannot descend; file 11 SHA-256 `1608ee9c...`), D08T2
(Duke-symbol pushable blocks, modern RMB grab game-wide), D22C (Level 11
started in Legacy controls), D26B (console leaves first person), D26C (console
history), D26D (authoritative level-select numbering; level numbers in these
reports are console indices until D26D). D17R now also reproduces in HOG
HEAVEN. Documentation only. No next job selected.

## 2026-10-04 - D22B every level candidate, Needs playtest

Modernized controls and the selected first- or third-person view now work in
all 21 levels `levels` lists. The lease authenticates each level by a body
derived from the owned disc (`recomp/tools/local/level_overlay_guards.py`,
`--check` keeps `control_guards.inc` honest). Dodge rolls and steep-slope
slides keep the mouse camera and view; the original's own back-steps and
strafes are taken over by the lease; unowned jumps keep the camera in third
person too; landing poses 94/95/106 keep it. Statistics screen, savestates,
pause and level select return Modernized control by themselves. Offscreen:
zero identity refusals in every level; LEVEL00 slots and Vanilla unchanged.
Not covered: first person while swimming (new D11E), real death/Continue and
natural level exits outside LEVEL00; occasional jump camera gaps seen in two
runs are unexplained. Candidate
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017`; codegen
hash unchanged, savestates load. Nothing committed. See
[note 102](102-d22b-every-level.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D26A accepted; next D22B

User: "excellent! mark as accepted." D26A is Accepted; executable
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851` is the new
regression baseline. The candidate entry below is historical. **Next: D22B,
Todo - next** (not started): Modernized controls and first person in every
level. Today only LEVEL00 and LEVEL01 authenticate; the other 19 selectable
levels fall back to original controls and third person. Use `levels` /
`level N` to reach them. The user authorized documentation, commit and push.

## 2026-10-04 - D26A debug level select candidate

Backtick console: `levels` lists the 21 levels the original title-screen
level-select cheat offers, with the game's own names (0 TIME TO KILL, 1-3, 5-12,
21-26 challenge stages, 27-29 bosses). `level N` ends the current level with the
pause menu's restart code and loads level N through the original mode 1 init.
The level index changes only inside that init; changing it earlier crashed Old
West pairs. Offscreen: all 21 levels load in one session with their own
overlay; savestates, Vanilla and the pause/title refusals work. Modernized
controls stay limited to LEVEL00/LEVEL01 (D22B). Candidate
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`; codegen hash
unchanged. **Needs playtest:** in a game, backtick, `levels`, `level 2`,
savestate there. Nothing committed. See
[note 101](101-d26a-level-select.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D22A accepted; new Level 2 jobs

User playtest: "I can confirm and accept D22A as working." The portal into
LEVEL01 (the user's Level 2) works and the level is completely playable with
Modernized controls. D22A is Accepted; executable
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0` is the new
regression baseline. The candidate entry below is historical. See
[note 100](100-d22a-portal-level-identity.md).

New Todo jobs (reproductions: player UI slots 8/9 = files 07/08, LEVEL01;
hashes in the board entries): D11D first-person eye height (slot 8), D12B
costume-aware first-person kick (slots 8/9), D17R sky black at the edges when
looking up (slot 9), D26A debug level-select panel, D22B Modernized controls
owning every level, state and transition end to end. The user's rule: fix the
shared system, not the level. The user plans to survey the rest of the game
with D26A. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D22A portal level identity candidate, Needs playtest

Cause: the Modernized lease authenticated only the LEVEL00 overlay body; the
first map's portal loads LEVEL01.OVR over the same base `0x800ca968`, so the
lease failed closed and original controls/third person returned. LEVEL01 is
now an authenticated level overlay (tag-keyed `level_overlays[]` in
`control_guards.inc`, owned-disc digests, same scratch-tail rule as LEVEL00);
unknown levels still fail closed and the apartment conveniences require
LEVEL00. Offscreen: movement, mouse look, jump, fire, pause/resume and first-
and third-person all work after the portal, repeated in one session, with zero
identity refusals; the 12 LEVEL00 slots and Vanilla are unchanged. Tests pass;
codegen hash unchanged. Candidate
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0`.
**Needs playtest:** UI slot 7 -> portal -> continue, then play the Old West
level with Modernized controls in first and third person. Nothing committed.
See [note 100](100-d22a-portal-level-identity.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q accepted (Done)

User, after extended play: "the whole job is 100% accepted", and
praised how cleanly the game now plays. D17Q (residual wall
flicker from per-triangle UV seams) is Done, user-accepted. Accepted
executable `a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960` is the new regression baseline. The candidate entry below is
historical. See [note 99](99-d17q-uv-seams.md) for cause, fix and limits. The
user authorized documentation, commit and push. No next job selected or
started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q UV seam candidate, Needs playtest

The diagonal line still visible after D17N (UI 12 panel at the crosshair on
load, flickering on W/S) is a one-texel UV seam, not geometry. The GL/VK
backends applied the 2D mirrored-sprite UV bump (`gpu_uv.h`) per triangle.
A world-wall triangle with an exactly vertical integer edge qualified while
its partner did not, so the two halves of one polygon sampled one texel
apart, and movement toggled which halves qualified. Traced positions, integer
SZ and UVs form one consistent perspective map (about 0.03 texel); an offline
exact render is seamless. Fix: perspective-corrected 3D triangles use full UV
limits and no sprite bump (`PSX_UV_3D_LEGACY=1` compares). Original textures
and Vanilla are pixel-identical; UI 12 and the UI 11 stage canopy lose their
seams. The 12-slot 120 Hz sweep and 60/180 Hz runs had zero replay misses.
Tests pass and the codegen hash is unchanged, so savestates load. Candidate
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with
texture precision `corrected`. Nothing committed. See
[note 99](99-d17q-uv-seams.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N accepted; D17Q added

User playtest: "The result is excellent." D17N (world subdivision zigzag; the
user's message called it D17P) is Accepted. Accepted executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Cause, solution and fallback behaviour: [note 98](98-d17n-world-subdivision.md).
The candidate entry below is historical. New Todo job D17Q: residual subtle
wall/surface flicker as independent Corrected-renderer stability polish (see
the job board; references D17N, D17P, D15 and R01). D18D (music after Continue)
remains Todo. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N world subdivision candidate, Needs playtest

The vibrating diagonal lines on walls (UI slot 12, and generally) come from the
original world renderer: every polygon within `0x2000` units goes through a
screen-space subdivision (`0x80012960` triangles, `0x8001205c` quads) whose
midpoints are integer screen averages with rounded UVs and no projection data.
D15's Corrected textures therefore skipped those pieces and they were drawn
affine; the bends follow piece diagonals and move with the eye. With Corrected
texture precision, world polygons whose corners all have exact projections and
fit the GPU primitive limit are now drawn whole with exact perspective (hooks
`0x800114EC`/`0x8001160C`; `DNTTK_WORLD_SUBDIVISION=1` compares the old path).
Original textures, Vanilla and software are unchanged.

Private offscreen evidence: straight panel borders across 120 Hz W/S sequences;
12-slot sweep, exact-projection share 0.29-0.43 to 0.73-1.00, lower packet use,
game rate unchanged, zero budget hits or replay misses; 60/180 Hz checked.
Tests pass; codegen hash unchanged, so savestates load. Candidate executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with texture
precision `corrected`; recheck accepted D15/D17 cases. Nothing committed.
New backlog job D18D (music silent after death and Continue) was added, not
started. See [note 98](98-d17n-world-subdivision.md).

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P accepted (Done)

User: "i fully accept this fix." D17P is Done, user-accepted: the Modernized
Draw distance option (`extended` default) removes the UI slot 8 subway bands.
Accepted executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
The user authorized documentation, commit and push. The candidate entry below
is historical. See [note 97](97-d17p-distant-bands.md) for cause, evidence and
limits. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P distant bands candidate, Needs playtest

The UI slot 8 subway bands are three original rendering limits, present in
Vanilla too: the far cut-off (rooms past the render limit are never drawn and
the fog fade before it spans a pixel or two), integer portal rectangles a pixel
or two short of distant openings (whole ceiling strips dropped), and integer
NCLIP on one-pixel distant faces. New Modernized **Draw distance** option,
`extended` by default (`original` restores the limits; Vanilla always original):
render-only limits doubled, portal rectangles 2 native pixels wider, exact PGXP
NCLIP signs when geometry or texture precision is Corrected. Profile schema 24,
`run.py --draw-distance original|extended`, settings menu **D**.

Private offscreen evidence: matched 4x views show a continuous ceiling and the
real corridor end; all 12 private slots keep their game rate with ring use at
most 50% and no budget hits or replay misses; 120 Hz cost unchanged; Vanilla
untouched; existing savestates still load (codegen hash unchanged). Tests pass.
Candidate executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
**Needs playtest:** walk forwards in UI slot 8 and look around; recheck the
accepted D15 cases. Nothing committed. See
[note 97](97-d17p-distant-bands.md).

Launch: `python3 recomp/tools/local/run.py` (the first launch migrates saved
settings to schema 24 with Draw distance `extended` and keeps a recovery copy).


## 2026-10-04 - D15 Accepted; new subway slot 8 is D17P

The user accepts the final playtest: slot 5 left-side black/missing areas no
longer reproduce, slot 6 closet and surrounding furniture are stable, and the
previous slot 8 door remains opaque. Geometry stability, floor seams and idle
polish were already accepted. D15 is **Accepted** at the primary 120 Hz target.
See [implementation, tests, user evidence and limits](96-d15-accepted-precision.md).

The user explicitly authorized documentation, implementation commit and push
on the current branch. Preserve the title changes and exported framework patch;
Vanilla remains available and precision is optional in Modernized/OpenGL.
Accepted executable: `35756df57b9a6ddd31ee0dabdffb51876faf7a3b0c05effb78be285dfd9dc9a7`.

**Immediate next job: D17P**, selected by the user. They replaced UI slot 8
with a subway corridor state: walk forwards and watch multiple horizontal
black bands in distant geometry/textures. Treat "possible culling" as a
hypothesis, compare rendering paths, and preserve the accepted D15 baseline.
This is separate from the accepted previous slot 8 opacity case. The new
state identity, private-copy intake and acceptance criteria are on the
[canonical board](../MODERNIZATION_JOBS.md#d17p---distant-horizontal-black-bands-in-the-new-subway-slot-8).
No technical cause is established by intake alone.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L accepted (Done)

User: "first off, i accept this as complete!" D17L is Done, user-accepted.
Preserve implementation `1e6fb0a` and the evidence in
[note 95](95-d17l-tabletop-depth.md). Earlier Needs playtest and unresolved
candidate entries below are historical. Busy-scene 180 FPS limitations remain;
this acceptance does not close other jobs or establish full campaign coverage.

The user clarified the workflow: implement and test, obtain acceptance, then
wait for an explicit instruction to document, commit and push. They authorized
this closeout in the next message. AGENTS.md records that approval requirement;
background development testing remains authorized. This closeout changes only
documentation and instructions, with no new build, launch or gameplay test.
No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L tabletop depth fix, Needs playtest

Runtime primitive/provenance capture identifies the remaining cup cut-off:
its depth-tested triangles were followed by a native tabletop quad that
painted over them without depth testing. The fix gives eligible opaque upper
faces of authenticated static eight-corner boxes depth while retaining native
integer positions, affine textures, quad topology, culling and OT order.
The broad upper-surface experiment was rejected on measured cost.

Approach/retreat captures at 60/120/180 retain the full cup. Accepted shelf,
closets, blood and subway routes remain intact; replay restore oracle passes
312 checks, Vanilla reports no host depth/replay workers, and Python runs 102
tests: 100 pass, two conditional skips. Primary 120 samples have zero output
underruns; 180 remains below target in these busy scenes with a small added
candidate deficit. User visual/audio acceptance is still required.

The user replaced the no-launch restriction: background/offscreen development
tests are authorized; foreground windows are reserved for necessary visual
verification. AGENTS.md records this. Player files are unchanged and test
games are closed. No Disruptor code was copied; no framework/generated code
changed. See [cause, fix and evidence](95-d17l-tabletop-depth.md).
Launch: `python3 recomp/tools/local/run.py`; F7 UI 11, F10 if capture is needed.


## 2026-10-04 - D17L selected; offline primitive diagnostics ready

D17L is In progress for the bounded cup diagnosis. Added opt-in title-owned
per-instance/per-face tracing and offline observational-equivalence checks.
No Disruptor code copied. The private UI 11 hash matches note 89. Native packet
and controls fixtures pass; the player build succeeds. No new game run or
cause/fix is claimed. Isolated diagnostic launch authorization was requested
under AGENTS.md and is pending. See [scope, evidence and continuation](94-d17l-primitive-diagnostics.md).
R01, D08Q3 and D17O remain Done; accepted rendering baselines are unchanged.


## 2026-10-04 - R01 DisruptorRecomp reference research complete

[Comparative review and pinned source index](93-disruptor-reference-research.md).
R01 is Done as a research-only job. Retain clean, ignored
`research/DisruptorRecomp` at `408f214d3109dbc6cbdded7edd128cbf8de6466a`,
framework `193a60b805e1eaa853129d6ccf63022440d4b143`; project patches were read,
not applied. The review records acquisition, architecture, all requested systems,
source/functions, applicability, licensing/provenance and ranked experiments.

Recommended next choices: a primitive/provenance census for D17L/M/N, D15's
optional Original/Corrected geometry with independent texture correction, and
measured CPU-accounting overhead. No experiment selected or started. Disruptor's
image crossfade does not replace D17; its gap rims do not establish fixes for
TTK culling/depth; TTK already has gameplay-scoped CPU overclock. Preserve
accepted D17/D17B/D17O and the primary 120 FPS baseline.

User confirms research-only reuse: learn techniques and write purpose-built TTK
code, with no 1:1 copying or mechanical translation.

The included license text differs from standard PolyForm Noncommercial despite
its label; clarify before future code adaptation. No source imported. Ignore
and clean-reference checks passed. Static research only: no build, launch,
gameplay/renderer edit or player-file write; no new gameplay evidence claimed.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O accepted (Done)

User: "amazing work!!! accepted. well done. document. commit. push".
D17O is Done, user-accepted. Preserve implementation `25734d1` and player binary
`8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
The preceding implementation's Needs playtest status below is historical.

[Architecture, cause, verification and remaining limits](92-d17o-sky-intake.md).
Original sky geometry/coarse animation and busy-club 180 FPS limitations remain;
this acceptance does not establish full campaign coverage or close other jobs.
Documentation-only acceptance closeout: no new gameplay test, build, launch or
player-file write. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O sky camera-relative replay repair (Needs playtest)

Verified original sky architecture and a high-refresh defect: the three
camera-relative sky matrices were recorded under the last world object, then
substituted with older world-space matrices. A narrow, resident-code-authenticated
exemption retains the original sky transforms, art and timer. In the sampled
turn sweep, camera-origin displacement fell from up to 1075.397 units to zero.
No framework, generated C, flight-control, clipping or media changes.

Private captures cover 60/120/180, ground first person, third-person flight,
multiple headings/pitches and a second street location. Near/input/aim/controls/
inventory suites and final player/shard checks pass. Modern/Classic pistol fire,
reticle eligibility, actual pack-off, landing/first-person return and reflight
pass. 1106 worker surface checks had zero mismatch. All 28 player file hashes
and mtimes match intake. Existing runtime patch stack verified already applied.

Primary 120 FPS routes were clean. Busy club 180 FPS still repeats frames and
can underrun audio, reproduced with the previous sky path too; do not claim
universal 180 FPS stability. Original coarse cloud motion and high-pitch geometry
limits remain. User sky appearance confirmation is required; no next job started.

[Full architecture, provenance, diagnosis, A/B evidence and limits](92-d17o-sky-intake.md).
Build SHA-256: `8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10 and look/turn/move;
F10 captures Modernized input if needed. The fix applies to high-refresh redraws.


## 2026-10-04 - D08Q3 accepted; next D17O sky appearance

User accepts D08Q3: "i accept this! great work." Done; preserve `f284dc1`.
The older Needs playtest entry below is historical. No new gameplay test was
performed for this acceptance closeout.

**Next: D17O, Todo - next.** Load UI slot 10 (file 09), look up at the sky and
investigate its strange/glitchy movement. This is not jetpack-specific. First
establish how the original game renders/animates sky, then diagnose and test a
bounded stabilization. The user explicitly requires substantial documentation
of architecture, cause, evidence, implementation and remaining limitations.
[Canonical job](../MODERNIZATION_JOBS.md#d17o---unstable-sky-appearance-when-looking-up-slot-10).

[Detailed intake/plan and exact save identity](92-d17o-sky-intake.md).
Immutable private snapshot: `recomp/analysis/d17o-sky-intake-20261004/cards-intake`;
28 files with adjacent hash/mtime manifest. Current slot 10 differs from the
older jetpack fixture. Use a separate writable copy and private preferences.
Do not assume a skybox, parallax defect or high-refresh cause from the report.
Preserve Vanilla, accepted visual work and D08Q2/Q3.

This checkpoint only records acceptance/backlog and copies intake data. Sky
investigation has not started; no code edit, build, game launch or player-file
write. Commit/push this handoff before the user clears context.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q3 implemented, Needs playtest

Flight now admits Modernized view aiming, weapon presentation, energy-beam
completion and the enabled crosshair through the existing guarded jetpack lease.
Ground movement eligibility, flight physics, Vanilla and D08Q2 are preserved.

Private Modern/Classic routes verify six weapon types, mouse yaw/pitch, firing
while moving/climbing/descending, I toggle, J-off, fuel exhaustion, landing and
first-person fallback/return. Native aiming/controls/input/inventory suites and
the player build pass. All 28 player file hashes/mtimes match intake.
[Cause, verification and coverage limits](91-d08q3-jetpack-aim.md).

User confirmation remains. Select weapons before takeoff; flight switching is
unchanged. Thrown/upgraded variants are not all separately live-validated.
No next job started. Build SHA-256:
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.

Launch: `python3 recomp/tools/local/run.py`. F10 captures Modernized input if
needed. Check the reported jetpack aiming/crosshair in both flight schemes.


## 2026-10-04 - D08Q2 accepted; next D08Q3 jetpack aiming/crosshair

User confirms D08Q2: "confirmed fixed! accepted." Marked Done; preserve the
accepted recovery from `2f02967`. Earlier Needs playtest and immediate-next
D08Q2 entries below are historical. No additional gameplay tests in this closeout;
the natural map-pickup replay remains outside the recorded automated evidence.

**Next: D08Q3, Todo - next.** User cannot aim to shoot while using the jetpack,
and the crosshair is missing. Cause, affected weapons/scheme and exact location
are unverified. Read the [canonical job](../MODERNIZATION_JOBS.md#d08q3---jetpack-weapon-aiming-and-missing-crosshair)
for scope/acceptance. Investigate view aiming and reticle eligibility separately;
preserve Modern/Classic flight, ground aiming, existing flight camera transitions,
Vanilla and the accepted D08Q2 fix. The old slot-10 private snapshot is a regression
fixture, not yet a confirmed reproduction of this new issue.

Documentation only this session: D08Q3 has not started; no code edits, build or
launch. Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q2 repaired, Needs playtest

The reported UI slot 10 is repaired automatically in captured Modernized ground
play: original jetpack pending `0x8001` becomes owned/off `0x1`, with full fuel
unchanged. Death during deployment can strand pending across Continue; the
original `dnstuff` grant preserves it. A private extended-deployment timing
fixture plus real RPG death/Continue reproduced the baseline failure and verifies
the repair. The user's exact original timing is not established.

Only the inactive closed/off endpoint is repaired. Picker/J, Modern and Classic
flight, fuel rejection/depletion and private save/load pass. Native controls,
input, aiming and inventory suites and the player build pass. All 28 player
card/state hashes/mtimes still match intake. Framework/renderer unchanged.
Natural map-pickup replay and user acceptance remain; no next job selected.
See [D08Q2 evidence](90-d08q2-jetpack-continue.md).

Build: `c8f979d50bd502ded05294fde2e55b59c90f132f386d21a89865538ca8836a41`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10, try brackets,
J and Space; F10 captures Modernized input if needed. No repeat cheat required.


## 2026-10-04 - Immediate next investigation: D08Q2 jetpack unavailable

The user will clear context after this checkpoint. **Start next with D08Q2**,
not another D17 job. Status: Todo - immediate next; investigation has not
started. User sequence: died -> Continue -> returned to gameplay -> entered
`dnstuff` -> jetpack cannot be used, and the inventory picker skips it.
Failing state is **UI save slot 10 (file 09)**.

Private snapshot: `recomp/analysis/d08q2-jetpack-20261004/cards-reported`;
manifest alongside it. Slot hash:
`161253aa41f4f2c407f57ef7cff1371ba7889c4bdeba793183f9045b6184e18e`.
Use private cards/settings. The saved result may not reproduce the preceding
transition by itself; compare working inventory and the full death/Continue/
cheat sequence. Trace ownership, fuel, picker eligibility and activation
alongside Continue/reset and cheat grants. Cause is unknown; picker exclusion
is evidence to inspect inventory state, not proof of a particular defect.
See the canonical [D08Q2 job](../MODERNIZATION_JOBS.md#d08q2---jetpack-unavailable-after-death-continue-and-dnstuff).

This checkpoint only records the report and preserves a copy. No gameplay
code, build, launch or player-file changes. D17L stays open; D17M/N remain
queued; D17C/D/E/F/K accepted behavior must be preserved.


## 2026-10-04 - D17E/K accepted; D17L open; new D17M/N backlog

The user accepts D17E subway peripheral visibility and D17K stable ground
blood. Preserve their current behavior from `72085d7`. D17C remains Done;
D17D/F remain Accepted. D17L returns to Todo: the isolated cup still flickers,
while nearby bar cups/bowl remain stable. It does not merely await acceptance.

Subsequent user retest on 2026-10-04 reports no visible change. D17L remains
open (Todo); no new fix is claimed. This checkpoint documents that retest
only. Further implementation is deferred.

Offline inspection of current UI 11 confirms polygonal table/cup meshes and
shared cup/bowl prototypes across table and bar placements. The residual
instance-level failure is not yet explained; the next L pass must trace the
specific cup and occluding surface across good/bad frames. No gameplay changes
or game launch occurred in this follow-up.

New separate Todo jobs: D17M, UI 9 shotgun ammo visible through its platform
while climbing the ladder; D17N, UI 12 jagged diagonal/vibrating walls during
forward/back movement. Causes remain unclassified. Slots 9/12 have changed;
preserve older private shelf/blood states for accepted regression checks.
[User results, current save hashes and mesh inspection](89-d17-playtest-followup.md).

Launch from this workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17C accepted; D17E/K/L visual candidate

D17C is Done on the user's acceptance of the current camera stability and look.
D17E/K/L were selected together and are now Needs playtest. D17D/F remain
accepted; no other job started.

- D17E: no missing peripheral subway wall reproduced on full-width wall/edge
  and stair routes at 60/120/180. Awaiting player closure, not a claimed new
  visibility fix.
- D17K: enhanced floor pieces now retain the original source polygon's
  farthest-corner ordering key, preserving native blood in front of the floor.
- D17L: compact static props (up to 256 local units per axis, authenticated
  caller) keep depth across the near radius. Opaque faces share an ordering
  key to avoid excessive draw-state changes. Actors and larger distant
  scenery keep the previous takeover boundary; Vanilla remains available.

[Causes, private save identities, implementation and verification](88-d17e-k-l-visuals.md).
Current UI 11 is the table report, UI 12 the blood report; older closet states
were checked separately. The accepted shelf/closets and camera are preserved.
Final 120-target samples have zero audio underruns, worker failures or new
mesh-budget fallbacks. At 180, final club samples are about 178-179 distinct
images/s, with a small table-route output underrun still measured; do not claim
zero-cost 180 FPS or actual-device audio acceptance. User should also listen
for crackle at their usual refresh setting.

New owned-EXE packet/guard tests and existing near-plane math pass. Python:
98 passed, two skipped. Original player files/settings were not written.
Framework patch and generated game code are unchanged.
Candidate SHA-256: `151ea6aab6957eeb2c3da5cf990700d566a227eb723eb829d13600146c098520`.

Historical candidate next action (superseded by the 2026-10-04 report above):
user playtest of UI slots 3, 11 and 12, starting at 120
and then their usual refresh setting. F10 captures Modernized input if needed.
Launch from the workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17D/F accepted; next-round visual and portal bugs

The user explicitly accepts the UI 9 prop, UI 11 closet and closet inside
the strip club as fixed. D17D and the related closet job D17F are Accepted.
Preserve the source-plane depth correction and bounded static-prop contact
tolerance from `b88d0e5`; do not reopen these jobs for the separate new reports.
Accepted binary: `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
[Cause, implementation, tests and user acceptance](87-d17d-contact-depth.md).

Next-round backlog (all Todo, none started):

- D17K: UI save slot 12, blood on the ground directly in front of Duke.
  User describes coplanar blood; exact renderer/placement cause unverified.
- D17L: UI save slot 11, props sitting on the table directly ahead. Walk
  forwards/backwards: props get cut off and reappear as Duke moves closer.
  This is distinct from the accepted closet in that save.
- D22A: UI save slot 7 -> walk into the portal -> start the next level.
  Duke loses all Modernized controls and first-person view. Cause unverified;
  inspect transition/overlay coverage and control/view state without bypassing
  identity guards. Preserve the selected mode and camera when gameplay resumes.

Read the new board entries for scope and acceptance. Verify current save hashes
and make dated private copies before testing; slot numbers alone do not prove
that an older `cards-user8` snapshot is still the current reproduction. Use
background/offscreen tests and preserve the accepted UI 9/11/club fixes.
The user will clear context and choose the next job; no implementation starts
from this handoff alone. This closeout changed documentation only, with no
build, game launch or player-data changes. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17A/B accepted and implementation published

Read [86](86-d17-acceptance-and-regression-baseline.md) first. D17A and D17B
are **Accepted**, explicitly closed after natural play at 120/180 FPS. Do not
reopen the broad parents for residual defects. The board has D17D-J and D18C
with current/historical private save identities. No next job was started.
120 is the primary quality baseline, 180+ supported high refresh, 240+
robustness, Unlimited stress/debug. Do not impose a 120 ceiling or destabilize
accepted mouse response, opening/club/apartment, idle eye, PS1 character or FMVs.

Important repository correction: `recomp/` source/tools/tests/configuration are
now tracked at the root. Never restore its blanket exclusion. Framework/UI are
pinned root submodules; build applies the complete tracked
`recomp/patches/time-to-kill-accepted-source.patch`. Old incremental patches
are historical only. Export future framework edits with
`python3 recomp/tools/local/export_runtime_patch.py`; unchanged gitlinks do not
save local modifications. Media, generated C, builds, saves, analysis, research
and extracted retail assets stay ignored. Original disc dump is now under game/.

Accepted executable and local snapshot hashes are in note 86. The local binary
is not replaced by the separate clean-source verification build. Do not infer
new 120 numeric benchmarks from the earlier 180 synthetic tests; user acceptance
is a natural-play result. A field clock near 60 is not the Unique image rate:
120 can show 120 freshly rendered views without 120 simulation updates.


## 2026-10-03 - accepted baseline and bounded movement polish

Read [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md)
first. User accepts the previous build as the first polished/enjoyable baseline.
Preserved executable/source: local `polish-baseline`; new private states:
`cards-user8` (UI 12 club Shift+W/S; UI 10 now train-platform ledge; UI 9 furniture).
Never reuse old slot descriptions without checking the current manifest.

Retained change: `run.py` defaults PSX_GL_PERF off except when
PSX_REPLAY_PROFILE=1; explicit overrides survive. Only native addition is
DNTTK_POSE_TRACE=1 capture/completion timestamps. Accepted renderer algorithms,
90 ms world delay, mouse sampling and audio remain unchanged. Faster phase
correction and 130 ms buffering were tested and removed/rejected.

Slot 12 remains open: roughly 67 ms capture intervals plus about 38 ms
composition time and advance scheduling exhaust the coherent snapshot pair.
Regular present intervals and fresh late mouse rotation do not imply smooth
world translation. Next work should address snapshot availability without
blindly adding movement latency or speculative geometry. Slot 10 can drain
audio under movement stress; do not declare crackle solved from slot 1's
zero-underrun result. Full train sequence and actual audio-device listening
remain unverified. D17D Todo tracks isolated furniture/closet/subway artifacts;
slot 9's board defect appears at 60 Hz too. No visual workaround shipped.

100 Python tests pass; real movement/jump/fire and opening/dancer mouse routes
recorded. Opening/dancer camera steps stay under one degree, approximately
179-180 images/s and zero underruns. Original 12 states match hashes/mtimes.
Build `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`,
codegen `8bab543c`. Game closed; no commits or player-setting changes.


## 2026-10-03 - save-driven camera stability candidate (D17A/B Needs playtest)

Read [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md)
first. New private states are `cards-user7`, UI N = file N-1: 1/2 slow,
3 subway peripheral culling, 4 improved opening, 5 early club, 8 minor slowdown,
10 later club. Old reports' slot meanings are obsolete. Never test against the
player's originals. Original hashes are in `warmup-baseline/save-manifest.json`.

Preserve these fixes: camera ownership no longer expires after four fields;
late redraws get two presents' grace and unfinished expirations inform the
readiness estimator; mixed opaque/dual-source batches retain original order;
GPU profiling avoids blocking query readback; pinned fetched SDL3 X11 events
map milliseconds into SDL nanoseconds; known render/kick/aim scratch is reserved
before workers fork. SDL repair is idempotent and fails closed on source drift.

Fixed 180 Hz is substantially improved in measured real 500 Hz mouse sweeps.
Unlimited remains a failing adaptive-scheduler stress case (114 ms stale camera
sample); do not report that mode as solved. Subway culling was not reproduced
in the bounded full-width capture route, so no claimed fix. Further campaign
visibility, actual audio listening and subjective mouse acceptance need the
user's test. Codegen remains `8bab543c`. Final executable SHA-256:
`28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
Final checks: 100 Python tests and four native suites pass; 2,848 GP0 and
3,056 RGBA comparisons are exact. Third-person club, kick/shot, pause/resume,
repeated loads and the Vanilla route pass their bounded checks. Original save
hashes/mtimes match. No commits or player-setting changes. Game left closed.


## 2026-10-03 - latest opening/club candidate (D17B Needs playtest)

Read [83-opening-responsiveness.md](83-opening-responsiveness.md) first; it
supersedes the older performance conclusions below. The player's saved first
person / Match Display / CPU 100% settings are unchanged. The final cold route
has zero audio underruns or failed workers, camera p99 6.0 ms and max 8.3 ms
from the first valid redraw. Final Match Display club turning delivers 179.5
distinct images/s, p99 6.18 ms and realtime emulation. Unlimited is adaptive,
not guaranteed to outperform a fixed target. Minor UI 8/9/11 geometry pops
remain open; UI 10 is the good comparison. Full campaign and audio listening
still need the user's playtest. Do not mark D17B Done without that acceptance.

Important fixes to preserve: per-image code-guard invalidation and reconstruction
after restore; exact initialized math shard; deferred-cycle-aware pipeline
deadlines; timestamped interpolation; ordered translucent batching; worker
refresh on new DLLs rather than invalidated movie functions; atomic worker
start timestamps and guarded timeout subtraction. The old restore bitmap clear
was unsafe and must not be restored as a performance shortcut. The math shard
uses normal byte validation. CPU 150% is a different stress workload and did
not establish the stock-CPU result.

Codegen `8bab543c`; known old player and intermediate saves explicitly
compatible, wrong hash/ABI rejected. Final binary SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
Runtime and generator patch reverse checks pass. The 98-test Python suite,
new compiled guard/deadline tests, GP0/RGBA comparisons and Vanilla route pass;
see note 83 for the old structural-test limitation and RAM/MMIO interpretation.
Nothing committed. Tests use private cards/profiles under local analysis.
The player build is left closed; launch with `python3 recomp/tools/local/run.py`.

## 2026-10-03 - renderer quality pass (D17A/B Needs playtest)

Read [82-renderer-quality-pass.md](82-renderer-quality-pass.md) first. The new
candidate fixes verified replay writes outside the saved live framebuffer
band and related depth restoration defects. Camera/visibility provenance and
Unlimited scheduling also received systemic fixes. D17C idle behavior remains
unchanged. No claim of stable 180 distinct images/s in the club: the worker
CPU tail remains the main measured limit (p95 about 21 ms). Final real-display
180 target gives about 85 distinct images/s in the turning sample; Unlimited
about 100. Treat the earlier "refresh or two" response claims as superseded.

Surface checks: 227 copy-on-write + 232 fallback, zero mismatches. Normal
visual route: 24 sequences without detected backsteps. Endpoint route: 409/412
pixel-exact; three street images differ during turning and lack generation
labels to classify. Do not call this a clean endpoint pass. Use private card
copies and the quality scripts; no player saves were used. Next performance
work should measure worker dispatch/cycle/cache costs and actual display/input
latency, not optimize average FPS. Campaign playtest is still required.
No-input RAM/MMIO equivalence is 630/630; driven 60/180 has two transient
RAM mismatches (629/631), with MMIO exact and repeated 60/60 exact. Keep that
open; earlier synthetic-driver sensitivity is a clue, not a diagnosis.
Vanilla fresh-card capture route passes. The final real-mouse sweep still
has long low-motion/repeat runs; the retained baseline does too, with nearly
identical p99 presentation intervals. The yaw-threshold heuristic cannot
prove image freezes or input latency. See the A/B details in note 82.
Nothing committed. The player build
is left closed; launch with run.py.

## 2026-10-03 - fifth pass: loss of control, late camera rebuilt (Needs playtest)

Ask the user for a fresh-game playtest: turning smoothness (start area and
club), loads of several saves in a row, holstered weapon + fire. Not
reproduced: UI 1 table artifact, UI 6 closet popping; ask for a screenshot or
a description. Measure turning with `realsweep.sh` + `holds.py` (STEP_S=0.001
PX=1) on the real display only. New switches: `DNTTK_LATE_SAMPLE`,
`DNTTK_LATE_LOG`; removed: `DNTTK_LATE_OVERRUN`, `DNTTK_LATE_JIT`. Nothing
committed.

## 2026-10-03 - D17A see-through fixed, D17B dancer jerk (Needs playtest)

Ask the user to replay UI 1-4 (`cards-user4` copies): doorway, statue,
apartment (no see-through), and sweeping across the dancers. Idle stability is
accepted: do not change the D17C eye. If turning still hitches, measure with
`realsweep.sh` + `staleclass.py` (real window and mouse); the remaining limit
is worker latency at 4x, and the next step would be reprojection with the
world and the HUD/weapon captured separately. New switches:
`DNTTK_LATE_OVERRUN`, `DNTTK_LATE_DUE_FREE`; lead default 16 ms. Nothing
committed.

## 2026-10-02 - D17A/B/C third pass (Needs playtest)

Ask the user to replay UI 1-5 (`cards-user3` copies) at Match Display: club
turning with the dancers (late camera, paced), idle stability and view bob
settings, closet, collar, apartment wall. If turning still feels heavy at
every 2nd/3rd refresh, next is present-time reprojection on the GPU. Developer
switches added: `DNTTK_LATE_PACING`, `DNTTK_VIEW_BOB` (numeric scale),
`PSX_HOST_DEPTH`, `PSX_HOST_UV`. Open: Duke stuck after the earlier UI 3 load.
Nothing committed.

## 2026-10-02 - D17A precise near geometry, D17B late camera (Needs playtest)

Ask the user to replay the strip club (dancers in view, turning) and the UI 4-9
states at Match Display. If it feels heavy, compare `DNTTK_LATE_CAMERA=0` and
the 150% CPU option. Developer switches: `DNTTK_LATE_CAMERA`,
`DNTTK_LATE_LEAD_MS`, `DNTTK_LATE_PEEK`, `DNTTK_LATE_JIT`, `DNTTK_LATE_TEST_YAW`,
`DNTTK_NEAR_MODE=conservative|full`, `DNTTK_NEAR_PRECISE`,
`DNTTK_NEAR_SPLIT_PX`, `DNTTK_NEAR_SPLIT_RATIO`, `DNTTK_REPLAY_NEAR_STATE`,
`DNTTK_REPLAY_LOG`. Open: Duke stuck after loading UI 3 not reproduced; cost on
the real display unmeasured (offscreen 93-180 distinct/s). Nothing committed.

## 2026-10-02 - D17C parked; D17A in progress

D17C parked (user saw no difference with the bob off). D17A: in-between
fidelity proven (alpha 0/1), near-wall flicker reproduced in the club at
16:9 first person and localized to the conservative world near clip's piece
ordering (documentation/81-d17a-instability.md). Next: insert the taken
polygon's pieces at its own position in the mesh order; test with
`recomp/analysis/d17-high-refresh/cwtest.py TAG` (STEER=1, CARDS=cards-club,
SLOT=1, OC=100, SCALE=2; target 0 back-and-forth) and `deferab.py` stills
(apartment/street, no vanished walls). Ask the user for F7 savestates at the
other reported places. Nothing committed.

## 2026-10-02 - D17 accepted; D17A/B/C added; D17C experiment (Needs playtest)

**Next session starts with D17C** (user's choice): get the user's playtest
verdict on `DNTTK_VIEW_BOB=off`, then keep it off, make it an option, or
redesign it. After that D17A or D17B.

D17 is Done (user-accepted). Next jobs on the board: D17A (instability and
black areas at the places the user listed), D17B (mouse-to-present latency
measurement, then a late camera update if the 30 Hz camera is the cause), D17C
(view bob). D17C test: `DNTTK_VIEW_BOB=off python3 recomp/tools/local/run.py`
(first person; environment only, not saved). Measure with
`recomp/analysis/d17-high-refresh/bobframes.py KEYS SECS` (per-field root and
eye height; STEER=1). Nothing committed.

## 2026-10-02 - D17 second playtest fixes (Needs playtest)

Recheck at Match Display (180 Hz): the street, the apartment (sink, wardrobe,
light switch), the subway past the EXIT sign and the power button should now be
smooth and stable; backtick console `fps` should show about 180 FPS, a Unique
count near it and Game 30. Developer switches added: `DNTTK_TEST_INPUT=FILE`
(scripted input), `PSX_REPLAY_COW=0` / `PSX_REPLAY_COW_CHECK=1`. Harness:
`steer.py`, `monitor.py`, `popsweep.py`, `seqcheck.sh`, `shift2.py`,
`CARDS=cards-apartment|cards-subway` (private copies of the D11B states).
Offscreen runs keep the GPU at its P5 clocks (no display), so they are a worst
case. Open: inherent snapping and original-mesh cracks; `ttk-input-test` needs
a live desktop session. Nothing committed.

## 2026-10-02 - D17 playtest review fixes (Needs playtest)

Recheck the first playtest's problems at the saved rate (`run.py`; the profile
has 120): the strip-club street and other heavy outdoor areas should now be as
smooth as the sewer with no audio artifacts; walking through doors should not
show other rooms; the intro and level movies should look like 60 Hz (no
flicker, normal brightness); after a first-person quick kick (Q) the leg should
be gone. Player sessions now run with `PSX_FORENSICS=0` (launcher; use
`--diagnostics` or `PSX_FORENSICS=1` for the forensic rings). New developer
switches: `PSX_GL_STATE_CACHE=0`, `PSX_PROF=FILE` with `PSX_PROF_CALLERS=1` or
`PSX_PROF_REPLAY=1` (profiler, `symprof.py`), `DNTTK_ROOM_WALK=always|off|probe`,
`DNTTK_TEST_KICK=<frames>` (with `DNTTK_TEST_DRIVE`). Harness additions in
`recomp/analysis/d17-high-refresh`: `slotscan.sh` (per-slot coverage and load),
`mipsdis.py` (static code from the generated C), `genpatch.py` (regenerates the
runtime patch). Open: vertex snapping and affine warp with small camera steps
(geometry correction off), third-person camera lerp around wall corners, fewer
in-betweens than asked above 120 Hz in the heaviest scenes. Nothing committed.

## 2026-10-02 - D17 high refresh rate (Needs playtest)

Playtest on the 180 Hz monitor: `run.py --frame-rate display` (saved;
`--frame-rate 60` returns to the original). Check: camera turns, running and
enemies look smoother than at 60 with no change in game speed; no tearing in
windowed and fullscreen (if exclusive fullscreen tears, try
`PSX_REPLAY_VSYNC=1`); `--frame-rate 30` and `120/144/240` also behave;
first person, widescreen, menus, movies and level loads; any flicker between
real and in-between frames (a mismatch) or stutter in busy scenes. Developer
switches: `DNTTK_FRAME_INTERP=off` (pacing only), `DNTTK_INTERP_STEPS`,
`DNTTK_REPLAY_BUDGET_MS`, `DNTTK_REPLAY_SLACK_MS`, `DNTTK_REPLAY_WORKERS`,
`DNTTK_REPLAY_TEST=alpha0|alpha1` (fidelity), `PSX_FP_RAM_HASH=1`
(equivalence). Harness: `recomp/analysis/d17-high-refresh` (`d17.py`,
`trip.sh`, `equiv.py`, `cadence.sh`). Runtime patch
`time-to-kill-zzzzzzzzzz-render-replay.patch` (new files render_replay.c/h,
render_worker.c). Regenerated hooks: `0x80026164`, `0x800632B0`,
`0x80031D10`, `0x80032E78`, `0x80031C14`, `0x8001FBA0`. Open: driven-input
equivalence at 180/unlimited (one pad sample), vsync on the real display,
moving room geometry. Pre-existing: `ttk-near-test` link error
(`frame_trace_*`). Nothing committed.

## 2026-10-01 - D08J1 accepted (Done)

User playtest: "genuinely working solidly." Committed. Next job: user's choice.

## 2026-10-01 - D08J1 overhead ladder grab (Needs playtest)

Playtest at save slot 6: hold E and run (or walk) at the ladder; Duke should
leap and grab it with no Space, then W climbs. Also: stop under it and tap E;
start a little to one side; try with a weapon drawn (E stows first; pressed
late while running, Duke stops at the wall and then leaps straight up).
Without E he should stop at the wall as before. Tuning:
`ladder_leap_distance()` windows (750/800/450), `leap_side` (180),
`leap_step` (70), rise limits 300..1400 in `recomp/src/ttk/ladder_top.inc`;
`DNTTK_TRAVERSAL_TRACE=1` logs `ttk-ladder-leap`. Scripts:
`recomp/analysis/d08j1-ladder-run` (`sweep.py`, `leap2.py`, `regress.sh`).
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Nothing committed.

## 2026-10-01 - D08Z accepted (Done)

User: "i love it. lock it in." Player profile stays on `jump: manual`
(assisted remains the default for new profiles). Next job: user's choice.

## 2026-10-01 - D08Z manual jump style (Needs playtest)

Playtest with `run.py --jump manual` (saved; `--jump assisted` returns to the
default). Check: running jumps leave on the press, the slot-5 gap by timing,
late presses just after an edge, steering with the mouse + W and braking with S
mid-air, quicker standing jumps, E grabs/mantles still work. Tuning knobs:
`air_accel_fraction` (0.14), `manual_edge_grace` (14), `quick_takeoff_scale`
(3) in `recomp/src/ttk/manual_jump.inc`; `DNTTK_AIR_CONTROL=0..3` scales the
steering without a rebuild. Analysis scripts: `recomp/analysis/d08z-manual-jump`
(`measure.py`, `gap.py`, `pit.py`, `regress.sh`). Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`.
Nothing committed.

## 2026-10-01 - D08Y accepted (Done)

User confirmed the slot-5 jump and that stutter, audio slowdowns and freeze
frames are fixed (round 5: leased 150% CPU overclock, lip-launch jumps, jump
mantles, budgeted reach retries). Nothing committed yet. Next job: user's
choice; D08Z (optional manual modern jump) is in the backlog. User found
`--cpu-overclock 100` and `150` equally playable; 150 stays the default.

## 2026-10-01 - D08Y round 5: overclock leased to gameplay (Needs playtest)

Boot/loading audio slowdowns at 150% fixed: the overclock is renewed from the
player update and lapses outside gameplay; a safety net pauses it for 5 s if
emulation falls behind (`[TTK cpu]` log line). Binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`. If the user still
hears slowdowns, check the session log for `[TTK cpu]` lines first.

## 2026-10-01 - D08Y round 4: emulated CPU overclock (Needs playtest)

Playtest: play normally (first person, 16:9) for a few minutes, slot 5 and
elsewhere; freezes while running should be gone. The profile migrates to
schema 19 on launch (backup kept) with `cpu_overclock` 150. If a scene still
drops, try `run.py --cpu-overclock 175` and report it; `--cpu-overclock 100`
restores the original speed. Measure performance on the real GPU, not Xvfb:
SDL offscreen + `DNTTK_TEST_DRIVE`, see documentation/10-tooling.md and
`recomp/analysis/d08y-gap-jump/gpuperf.py`. Runtime change:
`recomp/patches/time-to-kill-zzzzzzzzz-cpu-overclock.patch`. Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`.
No commits until asked.

## 2026-10-01 - D08Y round 3: smoothing (Needs playtest)

Freeze frames during E reaches and the jump-mantle pop fixed; binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`.
Check: E jumps at slot 5 and other ledges feel smooth, no hitch, no snap into
the mantle. Tuning: `reach_core_budget`/`reach_extra_budget` and the try lists
in `ledge_reach_retry()`, glide in `mantle_glide()` (`ledge_reach.inc`).
Diagnose with `DNTTK_FRAME_TRACE=1` (frames over 25 ms or 3 ms of hooks).
D08Z (manual modern jump) is backlogged. No commits until asked.

## 2026-10-01 - D08Y round 2 (Needs playtest)

Playtest: slot 5, run at the gap and tap Space anywhere in the last few steps:
Duke should leap from the very lip and land (no E needed). Check the hitch
after E jumps is gone, and that bed/couch jumps in the apartment still fire
instantly. Also any other gap or pit: jumps pressed near it now leave from the
lip like the original. Code: `0x800780b4` hook (`modern_controls.cpp`, 768
threshold), `edge_jump_queued()` + `pc_input.cpp` hold (40 updates),
`ledge_reach.inc` (D08Y mantles), `shortcuts.inc` word copies. Private scripts:
`recomp/analysis/d08y-gap-jump` (`sweep.py`, `mjump.py`, `furniture.py`,
`latency.py`, `regress2/`). Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. No commits until asked.

## 2026-10-01 - D14 widescreen accepted (Done)

User: "im very happy with it! i accept!" Committed with the case study
(DuckStation stretches 2D by 4/3; native-wide does not). Next job: user's choice.

## 2026-10-01 - D14 widescreen (Needs playtest)

Modernized defaults to native 16:9 (profile schema 18, `run.py --widescreen
off|16:9|16:10|21:9|auto`). Fixes: dropped `nw_hud_corners` (it shifted world
polygons: the off-centre, torn preview), widened TTK's portal root rectangle
`0x800d2210` at the render's projection load, D11B near clip in third person,
HUD layout `0x800dd778` moved out while the status bar draws (new hooks
`0x8008BA30`/`0x8001FC44`, regenerated). Binary `3f726b91...af2c`. Next: user
playtest across levels at 16:9 (and 21:9/auto if wanted); real-GPU fps.
`build.py` stops at the runtime-patch check (`time-to-kill-stopped-window.patch`);
the generate/build steps were run directly (04-build-and-run.md).
DuckStation testing: the flatpak ignores an `XDG_CONFIG_HOME` override and uses
the player's config (resume-on-exit on); use the portable copy in
`recomp/analysis/d14-widescreen/r2/ds/app/bin` (`portable.txt`).
Nothing is committed yet.

## 2026-10-01 - Session locked in; next: D14

All session work accepted (D08U, D08W, D08A3, D08X Done). D14 16:9 preview is
off centre with geometry holes; start the next session from the ordered steps
in documentation/76-d14-widescreen-first-pass.md (A/B `native_wide = false`
first). Standing permission: close the player's open game and continue.

## 2026-10-01 - D08X accepted (Done)

User: "thats it, fully accepted this!!!". Binary `60161f81...369c`. Nothing committed yet.

## 2026-10-01 - D08X flush hang (Needs playtest)

E-jump ledge catches settle to the shimmy-aligned hang height (ledge top +
456). Binary `60161f81...369c`.

## 2026-10-01 - D08X crate-to-crate mantles (Needs playtest)

Jump mantles onto any climbable object are proximity-based with the original
line-up; crate-to-crate 18/18 at any angle to 45 degrees. Binary `ccb4eb2a...0107`.

## 2026-10-01 - D08X angle forgiveness (Needs playtest)

E reach retries the original acquisition with the heading turned +-25/+-51
degrees as well as lifted. Binary `8bb01385...1701`. Playtest: slot 11 crate
top to the ledge in front at an angle.

## 2026-10-01 - D08X higher grab confidence (Needs playtest)

E reach retries the original acquisition up to 480 higher, easing Duke into
the hang. Binary `43eda09c...0f61`. Playtest the drained-water ledges.

## 2026-10-01 - D08X first-tier crate hang (Needs playtest)

A jump catching a stacked crate into the original pole-style hang (mode 7,
A/D circled Duke round it) now lets go at once. Binary `c1c49119...6fe7`.

## 2026-10-01 - D08X crate hang fix (Needs playtest)

Fixed: object hang toggling flag-table bits broke the identity guard (all
Modernized controls lost); stuck crate hang now lets go (S, or W without a
climb). Binary `bf368b68...bd45`. New log line `[TTK identity] guard N` names
any future guard failure.

## 2026-10-01 - D08X crate mantle (Needs playtest)

Boxes room (private copy of user UI slot 11): an E running jump into a crate now
starts the original mantle mid-jump instead of bouncing. Binary `9bade2a5...ba9c`.
Playtest: Shift + W + Space with E held into the crates from a few steps back.
Standing permission (2026-10-01): close the player's open game and continue.

## 2026-10-01 - Playtest: D08U, D08W, D08A3 Done; D08X boxes follow-up

User accepted D08U, D08W and D08A3. D08X ledge grabs accepted; next work is the
boxes room (private copy of user UI slot 11 in recomp/analysis/d08x-boxes):
Shift + W + Space with E held should mount a crate mid-jump instead of bouncing.
16:9 preview: `DNTTK_WIDESCREEN=16:9 DNTTK_GAME_CONFIG=recomp/analysis/d14-widescreen/game-16x9-preview.toml run.py` (Modernized profile).

## 2026-09-30 - Autonomous chain: D08U (Needs playtest), then D08X, D08W, D08A3, D14 start

User authorized an unattended chain: D08U, D08X, D08W, D08A3, then start D14;
no commits; a combined playtest later. D08U: E at a ladder top lowers Duke onto
it (host blend over the original 156 transfer); S climbs down and steps off.
Binary `6129f2ab...5683`. Details: documentation/72-d08u-ladder-top.md.
D08X (Needs playtest): E held through a jump arms the original reach at
takeoff, so close ledges are caught. Binary `bd153dd5...65b3`. Details:
documentation/73-d08x-ledge-grab.md.
D08W (Needs playtest): shallow-water jumps follow the held direction.
Binary `7ca63455...3328`. Details: documentation/74-d08w-wade-jump-direction.md.
D08A3 (Needs playtest): switcher icons are now TTK's own HUD art from
/DATA/FONTS.RAW (pack rebuilt by build_ttk_inv_icons.py). Details:
documentation/75-d08a3-ttk-inventory-icons.md.
D14 (In progress, first pass): inert 16:9 activation plugin and package;
experiment shows TTK's 4:3 culling gaps at the widened edge; no player option
yet. Details: documentation/76-d14-widescreen-first-pass.md.
Final binary for the playtest: `db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`.

### Playtest checklist for this chain

1. D08U: walk to the edge of the sewer walkway next to the ladder (slot 12),
   press E. Duke should stow, turn and lower onto the ladder smoothly. Hold S:
   he climbs all the way down and steps off, pistol back out. Also try the top
   of the first alley ladder. Walking or running off the edge without E must
   still drop as before.
2. D08X: hold E and jump (W + Space) at a ledge within reach, for example the
   wall at the side of the slot-12 walkway. Duke should grab and pull up.
   Without E it bounces as before. Try running jumps and ledges elsewhere
   (apartment exterior, crystal-2); armed, hold E a moment before jumping.
3. D08W: in the subway shallow water (slot 1), A/D/S + Space should jump in
   that direction, including after running forward. Also the crystal-2 wade.
4. D08A3: press [ or ] to show the switcher; the icons should be TTK's own HUD
   art (health cross for medkit, jetpack, Bio Mask skull, goggles), green %
   and frame unchanged.
5. General: ladders, mantles, run-jump ladder transfers and Vanilla feel
   unchanged.

## 2026-09-30 - D04A accepted (Done)

User: "perfect, accept" on binary `198673f5...ded68e`. Escape pauses with a
free cursor and resumes with automatic recapture. Next job: the user chooses.

## 2026-09-30 - D04A Escape mouse release (Needs playtest)

Ad hoc user job: Escape sometimes left the mouse captured. Recapture raced a
missed Start tap; now Escape pulses Start and holds auto-recapture until the
pause stops gameplay offers. Only `recomp/src/ttk/pc_input.cpp` and
`recomp/tests/local/pc_input_native.cpp` changed. Needs the user's windowed
playtest; look for `Mouse released (Escape)` in the session log.
Playtest fix: the resume Escape no longer holds recapture (it had left the
mouse free after resuming). Rebuilt: binary `198673f5...ded68e`.

## 2026-09-30 - D07D, D08T1, D08V accepted (Done)

User: "awesome. accept". All three Done on binary `123c7910...693e`. D08X
(hold-E ledge grab) is now unblocked. Next job: the user chooses from the board.

## 2026-09-30 - D08V implemented (Needs playtest); D07D, D08T1, D08V await playtest

Camera-only lease through mantle/hang/pull-up, post-mantle 0/8 and 0/9 gait
and unowned 107/108 falls; `traversal_camera_ready()` replaces the tank
fallback there. Repro and sweep scripts in `recomp/analysis/d08v-sewer`.
Binary `123c7910...693e`. D08X (depends on D08V) stays blocked on the D08V
playtest. Next job: the user chooses from the board.

## 2026-09-30 - D08T1 implemented (Needs playtest)

Hold Grab (RMB/Alt) grabs pushables; E mantles them like any climbable.
New generated entry hook `0x80051890` (E's Cross masked at the idle grab);
`0x80051CF0` mask only while Grab owns Cross. Schema 17 (`grab`, `grab_alt`;
`original_aim` Unbound). `build.py` stops in the stale runtime patch-stack
check; regenerate with `psxrecomp_cli.py generate` directly (see
documentation/70). Binary `66097cf8...3015`.

## 2026-09-30 - D07D implemented (Needs playtest)

Red dot off by default in Modernized (schema 16, one-time migration with a
backup). `marker_hook` owns the dot at enqueue `0x8002BC18`/RA `0x80033DB0`
using S4 == Duke and the frame RA `0x8003543C`; no entry lease or LEVEL00
gate. Tests: `ttk-aim-test`, Python 85 OK, `pc_input_probe --controls red-dot`
on/off runs. Playtest: look for the dot with targets in third/first person,
held aim, jetpack, swimming, scripted cameras. The legacy `aim-options` route
fails in stale sampler/D04 checks (not changed). Binary `db5bf964...ec67b`.

## 2026-09-30 - D07D added (no red autoaim dot in Modernized)

New Todo D07D: the user finds the original red autoaim dot next to the modern
crosshair confusing. `red_dot` is off by default in every Modernized profile
(with profile migration), covering every state where `marker_hook` currently
steps aside; the option stays, and Vanilla keeps the dot. Next job: the user
chooses from the board.

## 2026-09-30 - D08T1 added (E always mantles, hold RMB to grab)

New Todo D08T1 from a user brief: pushable objects must not change traversal
controls. E always mantles; a new rebindable held Grab / Manipulate action
(default right mouse in Modernized, where precision aim is not needed; Vanilla
keeps it; Alt is the proposed second binding and the legacy-aim grab)
grabs, W/S push/pull, release lets go. Audit the Mouse2 /
`original_aim` paths first. Brief:
`documentation/69-d08t1-grab-manipulate-brief.md`. Next job: the user chooses
from the board.

## 2026-09-30 - D08X added (hold-E ledge grab)

New Todo D08X: in Modernized, holding E while jumping toward a ledge at
grabbing height should reliably catch it and mantle up, like run-jump + E on
ladders (D08J). Trace the original ledge eligibility (anims 134-142, 6/7
hanging dispatches) before choosing a fix. Next job: the user chooses from
the board.

## 2026-09-30 - D11C Done (user accepted)

User: "duke's head is back, mark as complete! well done". Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`. Next
job: the user chooses from the board.

## 2026-09-30 - D11C stale head-hide reclaim (Needs playtest)

`first_person_draw(duke,eye)` clears an unowned bit 0 on Duke's joint 9
record at Duke's draw entry (debug `fp.head_reclaims`), then hides it again
only in first person. The original never sets that bit (static scan of the exe
and the 30 unique overlays). A private slot-12 copy loads with the head drawn in
third person; first person still hides it. Awaiting the user's slot-12 check;
then D11C can be Done. Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

## 2026-09-30 - D17 expanded to high refresh rate rendering

User brief for Match Display / 30-240 / Unlimited rendering without changing
gameplay timing was folded into D17 (still Todo, depends on D01, D13). Brief:
`documentation/68-d17-high-refresh-brief.md`. Audit and plan first. Next job: the
user chooses from the board.

## 2026-09-30 - D16A cancelled; D08A3 added

D16A (HRP research) is Cancelled by the user; original TTK assets are the
direction. New Todo D08A3: extract original TTK medkit/biomask/jetpack/steroids
(and goggles) icons to PNG and rebuild `ttk-inv-icons.pack` so the switcher no
longer uses Duke3D tiles. Next job: the user chooses from the board.

## 2026-09-30 - D13 Done (user accepted)

User: "correct correct correct, D13 is good. I'd say let's approve it." Final
binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`. New backlog from the D13 playtest: D08U (top-of-ladder mount),
D08V (sewer mantle lease), D08W (subway shallow-water sideways jumps), D10B
(tight-space camera transparency/doors), D11C (savestate stuck head-hide
flag, slot 12). Next job: the user chooses from the board.

## 2026-09-30 - D13 F11 fullscreen key (Needs playtest)

F11 is the only default fullscreen key (`host_keymap.c`); Alt+Enter and Ctrl+F
are unbound. The D13 patch was regenerated and checked forward/reverse. Launcher
text says "F11:". Awaiting the user's confirmation of windowed start, F11 and
the remembered state; then D13 can be Done. Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D13 windowed default + remembered display (Needs playtest)

Schema 15 (`fullscreen_mode`), runtime `--fullscreen-mode` and
`--presentation-state` (written in `shutdown_runtime`), and
`absorb_presentation_state` / `save_presentation_state` in the launcher. The
user's profile is set to windowed at 4x (backup
`recomp/config/player-profiles.json.recovered-2a7d56cb...`). Awaiting the user's
confirmation of windowed start, Alt+Enter and the remembered state; then D13
Done. Binary `7bd2a001a295330397d45c73ca9bc2ca0bc575c0d889bab3259d06778b3a3d1c`.

## 2026-09-30 - D13 playtest follow-up (Needs playtest)

User confirmed 4x at 60 fps; Modernized default now 4x. Fixed Alt+Enter
(`host_keymap.c` exact modifier compare, now `mod_groups`) and SDL3 exclusive
(`psx_apply_fullscreen_display_mode` in main.cpp); both in the D13 runtime
patch, which was regenerated and checked forward/reverse. Awaiting the user's
confirmation, then D13 can be Done. New backlog from the same playtest: D08U
(top-of-ladder mount), D08V (sewer mantle lease), D10B (tight-space camera).
Slot 12 is `saves/local-play/openbios/state_800AB6FC_slot11.pst`; a private
copy is in `recomp/analysis/d13-resolution/cards/openbios/`
(`probes/start.sh RUN 11 ...`). `ptrace` attach is blocked on this machine, so
use temporary log lines instead of gdb attach. Binary `ecc9328065b8a6c3311423e1640936b1835e1517d35b923605609595e2f3a95d`.

## 2026-09-30 - D13 resolution and display (Needs playtest)

Runtime patch `recomp/patches/time-to-kill-zzzzzzzz-presentation-cli.patch`
adds `--internal-scale/--display/--window-width/--output-filter`; profile
schema 14 (`player_profiles.py`) saves them per profile and `run.py` passes
them (`presentation_args`; software forces 1x via `effective_scale`). Test
harness: `recomp/analysis/d13-resolution/` (`probes/start.sh RUN SLOT
[run.py args]`, `stop.sh`, `measure.py RUN`; port 9193, display :93, `WM=1`
starts metacity for fullscreen checks). `screenshot_hires` does not see the
OpenGL hr FBO; use window captures. Awaiting the user's desktop test (fps at
2x-4x on the GTX 1080 Ti, exclusive fullscreen). Pre-existing: patch-stack
check fails on `host_osd.c` drift vs the inventory-strip patch (see the D13
note). Binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`.

## 2026-09-30 - D08L Done (user accepted)

User: "im happy with that!" Final binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`. Next job: the user chooses
from the board (remaining Todo includes D09, D13, D19B, D20).

## 2026-09-30 - D08L inertial edge run-off (accepted)

Running falls over 768 that directly follow a ground stride now get the D08F
short-fall lease (`recomp/src/ttk/terrain.inc`, previous-animation gate 72..79,
`large_fall`, counter `run_offs`); all departures capped 10000 running / 2048
walking in the `0x8003ebf4` ballistic hook (`modern_controls.cpp`). Note: the
adapter never sees mode 9/0 on the first fall update (already 9/9), so do not
gate on it. Test state: `recomp/analysis/d08l-edge/apt-cards` slot 1
(apartment), 2 (on the bed), 3 (fire-escape platform, run west off the end);
probes port 9191, display :91 (`probes/start.sh`, `stop.sh`, `tr.py`).
Previous binary `before.bin` (`1452391c...`). Open follow-ups: other
exterior ledges, very high falls. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D12A Done (user accepted)

User: "its done! accepted. ... this has been yet another amazing feat of engineering." Final binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`. Next job: the user
chooses from the board (D12A is closed; remaining Todo includes D08L, D09, D13).

## 2026-09-30 - D12A thigh 0.13, E never kicks, Q chains (accepted)

Thigh drop default 0.13 (user's choice). `kick_convert` kicks only with the
attack held (E suppressed); Q queues during a kick, held Q repeats. Awaiting
the user's test. Binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D12A follow-up (Needs playtest)

After the user's first playtest: sphere at the crosshair trace hit
(`view_segment_query` in weapon_aim.cpp, within 480 horizontal) else 340;
held attack with Boot kicks in first person while moving; thigh drawn end
for end from the knee (`DNTTK_FP_KICK_THIGH=0` off). Garbage bags: alley
state in `recomp/analysis/d12a-kick/alley-cards` (slot index 4), bags
`0x801dcf44`/`0x801dcd64`. Private copies of the user's slots 0-4 in
`recomp/analysis/d12a-kick/user-cards` (index 3 = street near the subway
stairs; the pallet was not identified). Open: thigh framing options, the
pallet. Binary `e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`.

## 2026-09-30 - D12A first-person quick kick (Needs playtest)

Q in the eye view = host quick kick (`recomp/src/ttk/kick.inc`); Boot-selected
original requests (112..115) are converted at `0x800493a4`. Hit = original
sphere `0x800a979c` along the view on each update of frames 8..20; leg =
joints 14..17 from 115 poses via the D12 private-matrix path. Framing knobs
`DNTTK_FP_KICK`, `DNTTK_FP_KICK_TILT`, `DNTTK_FP_KICK_REACH`,
`DNTTK_FP_KICK_FREEZE`. Test state: `recomp/analysis/d12a-kick/fresh-cards`
savestate slot 3 (first street, god mode, pig cop ahead); probes use port 9181
and display :81 (`probes/start.sh`, `stop.sh`). Previous binary `before.bin`
(`941593c0...`), sources in `before-src/`/`after-src/`. Open: user playtest of
feel/framing/damage; knockback (enemy reaction 84) not reproduced. Binary
`93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`.

## 2026-09-30 - D08T pushable objects (Done, user-accepted)

E grabs / W,S push-pull / E lets go / Space climbs, for objects flagged
`0x08000000`; see `documentation/64-d08t-pushable-objects.md`. Test state:
private copy of the user's slot-5 alley savestate in
`recomp/analysis/d08t-push/cards` (slot index 4), sources before the change in
`.../before-src/`, previous binary `before.bin` (`65226a9d...`). New generated
hook `0x80051CF0` (regenerated with `psxrecomp_cli.py generate` as in the D12
note, then `cmake --build --preset local-dev` and `build_movie_overlay.py`).
Probes use port 9171 (`PORT` env). User playtest: "it works so much better than the original now. this is it rock solid. confidence level is very high." Possible
follow-up: other pushable objects/maps; host pre-alignment if the line-up angle
ever feels strict. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

## 2026-09-30 - D12 Done (user accepted); next D12A

User: "finally, we can mark this as accepted!!" First-person weapons are Done
on binary `65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next job chosen by the user: D12A, keep the kick (Q) in first
person. Per-weapon framing lives in the offset/tilt/scale tables in
`recomp/src/ttk/first_person.inc`; `DNTTK_FP_WEAPON_SLOT<n>`, `..._TILT<n>` and
`..._SCALE<n>` override them for one launch.

## 2026-09-29 - D12 framing tweaks; next job D12A

User accepted most weapons as they look; shotgun, gatling and throwing blades
retuned (per-slot tilt/offset tables in `first_person.inc`). Next job chosen
by the user: D12A, keep the kick (Q) in first person.

## 2026-09-29 - D12 twin cannons (Needs playtest)

Slot 8 (key 5) framed after the Devastator references; HUD now over the
weapon (viewmodel in OT slot 1). Tune per weapon without rebuilding:
`DNTTK_FP_WEAPON_SLOT8=x,y,z`, `DNTTK_FP_WEAPON_TILT8=p,y,r`,
`DNTTK_FP_WEAPON_SCALE8=f`. Club test state: private copy of the D11 route
cards in `recomp/analysis/d12-first-person-weapons/club-cards` (slot 1, main
room facing the mirror after a 180-degree turn). Binary
`b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`.

## 2026-09-29 - D12 first-person weapons (Needs playtest)

Real hand + weapon meshes drawn as a viewmodel in first person; see
`documentation/63-d12-first-person-weapons.md`. Test state: private copy of
the street savestate in `recomp/analysis/d12-first-person-weapons/cards`
(slot 1; `PSX_LOAD_SLOT=1`), sources backed up in `.../before/`. `build.py`
still stops at the runtime patch drift: after changing hooks run
`psxrecomp_cli.py generate` with `PSXRECOMP_GAME`/`PSXRECOMP_BIOS` set to
`build-recompiler/`, then `cmake --build --preset local-dev` and
`build_movie_overlay.py`. Before launching an isolated instance, stop any
earlier one on port 9177 (a stale instance answered one survey). Re-survey
seeds with `DNTTK_FP_WEAPON_TRACE=1` (fire each weapon standing, level view)
if placement math changes. Next: user playtest of placement/size/feel; then
holster/draw animation, left hand, projectile origin. Binary
`1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`. No commits.

## 2026-09-29 - D11B Done (user accepted); jetpack sprite confirmed

User: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available"). Next first-person job: D12 (hands and weapon). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

## 2026-09-29 - D11B occluder fade in first person (Needs playtest)

User: conservative default is the best-looking; keep `DNTTK_NEAR_CLIP=0` as
Vanilla rendering and `full` for research. Invisible subway door = original
occluder fade (`0x80031fa0` loop, overlap test `0x8002ee50`, fade distance
camera+0xa0, Duke rect camera+0xa8). New hook `0x8002EE50` hands the test an
empty rect in the eye view; `DNTTK_FP_OCCLUDER_FADE=1` restores it. Both are
logged as D19 menu toggles. Private copies of the user's F7 slots in
`recomp/analysis/d11b-near-clip/subway-cards` (file slot 01 = F7 slot 4,
02-04 = F7 slots 1-3). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`. No commits.

## 2026-09-29 - D11B third pass: conservative default (Needs playtest)

User saw dotted black lines on floors at 1080p with the subdividing mode. Test
at 1080p by capturing the window (`g.wshot`, `XRES=1920x1080` in
`recomp/analysis/d11b-near-clip/probes/launch.sh`, `look.py`); the runtime's
`screenshot_file` shows the software raster, not the OpenGL window. Default is
now `conservative` (clip only what the original gets wrong, no subdivision,
whole-polygon sort); `DNTTK_NEAR_MODE=full` keeps the subdividing mode;
`DNTTK_NEAR_CLIP=0` is off. Street test state:
`recomp/analysis/d11b-near-clip/street-cards` slot 1 (fresh game, street
spawn). Binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`. No commits.

## 2026-09-29 - D11B second pass (apartment artifacts; Needs playtest)

User reported comb/popping on the apartment wardrobe ("closet"). Fixed by
average-depth piece sorting, a -4 OT prop bias (actors excluded via
`first_person_actor_drawing()`), prop takeover to 3072, outward rounding of new
corners, and a per-frame packet budget (the render arena is a 139,744-byte
ring; see `documentation/62-d11b-near-clip.md`). Apartment test state:
`recomp/analysis/d11b-near-clip/apartment-cards` slot 1 (debug savestate on
private cards); probes in `recomp/analysis/d11b-near-clip/probes/`
(`closet.py`, `walkseq.py`, `roam.py`, `apt_route.py`). Compare with
`DNTTK_NEAR_CLIP=0`; `DNTTK_NEAR_TINT=1` colors host pieces by renderer. Binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`. No commits.

## 2026-09-29 - D11B near-wall clipping (Needs playtest)

Code: `recomp/src/ttk/near_clip.cpp` (+ `near_clip.h`), hooks `0x80010000`
and `0x80011020` added to `game.local.toml` and regenerated with the built
recompiler (`PSXRECOMP_GAME=build-recompiler/psxrecomp-game ... psxrecomp_cli.py
generate`), wired through `first_person_view_live()` /
`first_person_duke_drawing()` in `modern_controls`. New native target
`ttk-near-test`. `DNTTK_NEAR_CLIP=0` turns it off for A/B.

Findings worth keeping: the level is drawn by two hand-written renderers at the
start of the executable (world `0x80011020`, object `0x80010000`); the GTE RTP
ring's `ra` for them is garbage (both use `$ra` as a scratch register - 0x100 is
H). `0x8002ef90` is a horizontal grid surface, not walls. Near the eye one
texel spans 15-25 pixels, so any host subdivision must cut on whole-texel lines
or straight texture lines kink.

Probe helpers: `recomp/analysis/d11b-near-clip/probes/` (`sweep.py TAG X Z YAW`
places Duke on the private route cards and sweeps; `perf.sh`; private port
9177, display :77). Wall spots: club side wall (5486, -4096) yaw -61.3; entry
corridor (5442, -800) yaw -90. D11 binary backup
`recomp/analysis/d11-first-person/d11-accepted-dd7b85b4.bin`, toml backup
`game.local.toml.before-d11b`. Binary
`d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa`. No commits.

## 2026-09-29 - D11 first-person prototype (Done, user accepted)

Code: `recomp/src/ttk/first_person.inc` (header comment lists every original
address it relies on), wired in `modern_controls.cpp` (`fp_blend`,
`eye_jump`, `orbit_constraint` blend, hooks `0x800348d8`, `0x8001ca4c`,
`0x8002a038`, `0x800b4d9c`), `pc_input` action `camera_view` (P) placed after
`camera_shoulder` (wire payload now 39 entries), profile schema 13 `view`,
`--view`. New `game.local.toml` hooks `0x800348D8`, `0x8002A038`,
`0x800B4D9C`; regenerated with the built recompiler (one file per hook); the
launcher rebuilt the movie shard.

Findings worth keeping: camera `+0x42` is H = 160/tan(angle at `+0x40`/2) =
386; the camera sits H/2 behind its anchor; the world renderer flags vertices
nearer than H/2 and the joint draw skips joints nearer than H. Duke's model
record at `[player+0x40]`: 19 joints, record `+0x44+0x28*j`, byte 0 bit 0 skips
the joint, byte 2 level; joint 9 is the neck/head. The original camera eases
per axis through `0x8002a038` with rates from `0x800c0d4c`. Save states are
taken at scheduler boundaries (VSync waits), but the debug server reads mid
frame, so a debug read can see the relaxed 768 word; the runtime repairs any
captured value.

Probe helpers are in the session scratchpad (`g.py`, `route.py`,
`wallcheck.py`, `perf.sh`, `launch.sh`/`start.sh`/`stop.sh`; private port 9177,
display :77). Fixture copies: `recomp/analysis/d11-first-person/` (profile,
cards from D10, `route-cards` slot 1 = club main room, first person; the D10
binary backup `d10-accepted-84669bb6.bin`). Developer override
`DNTTK_FP_PROJECTION=160..386` for FOV/near tests. User accepted D11 after extended play. Next:
D11B (near-wall polygons) and D12 (hands and
weapon) follow; D11A was cancelled (the P toggle suffices). Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`. No commits.

## 2026-09-29 - D10 Done (user accepted); D08S cancelled

User: "it's done, fully accepted" after about five minutes of play. D11 (first-person
prototype) is now ready. No commits.

### D10 recenter / shoulder / saved distance implementation notes

D08S cancelled at the user's request (keep the scope text; may revisit). D10
options pass: actions `camera_recenter` (V) and `camera_shoulder` (H) sit
before `weapon_previous` in `input_bindings.def`, so they are edges, not guest
commands. Wire payloads now have 38 entries; the native test fixtures were
updated. The camera side is in `orbit_begin`/`orbit_constraint`
(`modern_controls.cpp`, debug JSON `shoulder_offset`, `recentering`,
`recenters`, `rest_pitch`). Persistence: runtime side file
`<settings>.camera-state`, merged by `run.py` after exit and at launch; profile
schema 12. The probe is in the session scratchpad (`d10_probe.py`; phases
main/relaunch/nudge/strafe; `SLOT=1|2`, `EXTRA` launcher flags). The fixture
copy is `recomp/analysis/d10-camera`. The debug `quit` returns "emu busy or
frozen"; SIGTERM the game child to exit cleanly. Duke's heading equals the view
on foot, so any recenter yaw work must target states where facing diverges.
Binary `84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`. No
commits.

## 2026-09-29 - D08R Done; D08Q1 faster Modern Ctrl descent (Done)

D08R accepted ("it's absolutely rock solid"). D08Q1: `k_jet_descend_velocity`
5500 in `jetpack.inc` (Modern only) gives 29.5 units/frame, the underwater
Ctrl dive rate from `/tmp/swimlab/deep4.log`. User: "verified working!!!". Probe script `descent` in the
scratchpad `jet_probe.py`; health is `player+0x32`. Binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`. No commits.

## 2026-09-29 - D08R selectable jetpack scheme (revision 2, now Done)

`--jetpack classic|modern` -> profile schema 11 control `jetpack` ->
`DNTTK_JETPACK`. Gate `jetpack_classic()` in `modern_controls.cpp`. Classic
shares the entire Modern control path (lease, `face_view`, WASD bridge, J,
fall grace); `jetpack_update()` in `jetpack.inc` returns before the host
vertical layer (hover, Ctrl descent, trim). Do not give Classic the original
camera or D-pad turning again: the user rejected that (revision 1) because
mouse turning failed and controls slipped out of modern. Probe harness copy
lives in the session scratchpad (D08Q `/tmp/jetlab/jet_probe.py` plus
`classic2`, `wcheck`, `turn2`, `tables`; `PORT`/`EXTRA` env). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`. No
commits until asked.

## 2026-09-29 - D08B accepted (Done)

User: "d08b can be marked as completely done man, its fine. accept it all"
and "i have tested all of them!". The user playtested the old D08B limits
(contact jumps, bed run-off slowdown, unopened-bed pipe bomb, oblique couch
entry, pig-cop ladder stall) and accepted them. Board and status updated; no
build or automated replay. D10 is the only
In progress job. No commits until asked.

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

Intro-only stutter after D23A. Cause: `psxrecomp-game --overlay-config-hash`
covers the `game.local.toml` hook list, so each new host hook moves the
movie shard's cache folder (`cache/SLUS-00583/gcc/linux-x64/cg10_..._gc<hash>_f0`)
and the loader silently interprets MOVIE.OVR. Now: `ttk-movie-shard` target in
the `local-dev` build preset, `run.py` `ensure_movie_shard()`, and
`fmv_poll.c` logs `ttk-fmv: native movie decoder active (19 functions)` or a
WARNING carrying `overlay_loader_last_msg()`. After adding a hook, just build
or launch normally. If the intro stutters: check the `ttk-fmv:` line in the
session log first. Launch tests in `tests/local/test_player_profiles.py`
must pass `--no-session-log` and stub `run.ensure_movie_shard`, or they
start the real game on the player's cards. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits until asked.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Done, user-accepted)

User: "that stuttering audio/slowness issue" before the D08Q playtest.
Measured with the runtime's telemetry in an isolated Xvfb instance
(`/tmp` harness; `audio_stats`, `phase_profile`, `phase_hot`, `frame`,
`ttk_input`): Modernized 47.5–49.3 fps + continuous underruns, Vanilla
60.0 clean, Sep‑27 binary 57.5. Cause: `identity()` walked all 20,305
guarded words via `psx_mod_read_word` on every call, 46 calls/frame ×
108 µs = 5 ms/frame (24 % wall). Fix: `code_identity.h` memcmp fast path
on `g_psx_ram` (exact per-word fallback keeps the apartment substitute),
`IdentityMemo` per `input_host_frame()` × `g_dirty_ram_code_gen` in
`modern_controls.cpp` and `weapon_aim.cpp`. Now 1 check/frame, 28 µs,
59.94 fps, 0 underruns, fill 267 ms. Debug JSON `identity_calls`,
`identity_checks`, `identity_us`. Tests: mocks define `g_psx_ram` /
`g_dirty_ram_code_gen`, `poke()`/`pokeb()` bump the generation; new memo
case. All three native suites PASS. If stutter returns: check
`identity_checks` ≈ 1/frame and `audio_stats.out.fill_ms` vs target first;
`perf` is unavailable (paranoid=4). Do not add per-call guard walks again;
new guards are now nearly free. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. No
commits until asked.

## 2026-09-29 — D08Q modern jetpack flight controls (Done, user-accepted)

Accepted: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."


User: jetpack uncontrollable except Space; WASD "blocked"; modern controls
fail. Isolated baseline on `79a3fd5b…` (turret-room / ledge states copied to
`recomp/analysis/d08q-jetpack/`, `dninventory` cheat, J, Space): original
mode 10 (anims 163–170) matched no lease → no mouse; WASD via tank fallback
relative to an unturnable body; idle gravity landed Duke in 1–2 s. New
`recomp/src/ttk/jetpack.inc` from the existing `8005a210` hook, camera-only
lease clause `jet` in `state()`, `jetpack_input_ready()` for `input_pad()`
(W/S → Up/Down, A/D → layout strafe pads) and `select_weapon` (J only in
flight), `locomotion_input_ready`/`airborne_input_ready` false in flight and
the 108 fall grace. Host writes: hover lock `+0x224 |= 0x08000000` + base
`+0x860 = Y` when idle; Ctrl → lock off, `+0x1f8 = 2800`, `+0x1e8 = 0`; WASD
→ lock off, `+0x1f8 = 0`, `+0x1e8 = 18` trim; `face_view()`. Do **not**
drive `+0x860` for descent or hover during WASD (57-jetpack-controls.md,
"Rejected on evidence"). Guards `8004aaf8`/272, `8004ac08`/472,
`8004ade0`/1972. Live probes: headings within 4° of the camera, level ±25,
25–29 units/frame, Ctrl 489/30 frames → 105, J off → 108 with live mouse →
ground run. ttk-input-test, ttk-aim-test, ttk-controls-test PASS (controls
test: use `analysis/pc-input/d08-camera-final/level00-guard-fixture.bin`;
the `d08p-turret` fixture is Modernized-patched and fails the Vanilla
assertion). Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. Playtest:
J, Space, mouse, WASD, release → hover, Ctrl → land, J in the air. If the
feel is off, the tunables are `k_jet_descend_velocity` (2800) and
`k_jet_level_trim` (18) in jetpack.inc; `DNTTK_JET_TRACE=1` logs each
update. No commits until asked.

## 2026-09-29 — D08P overlay scratch identity + mid-depth wade handler (Done)

User: the whole ledge area (F7 UI slot 3) kills run, mouse and jump for good;
depth is not the cause. Their `session-20260929-005059.log` showed
`[TTK lease] inactive (identity)` from the ledge onward. Live guard diff on the
isolated slot 3 copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's
trailing scratch vector, written by the zone script's `0x8007177c` hit test
from overlay code `0x800cb1a4`. The overlay guard now covers 9652 bytes
(code/tables), digest `274d71dd…`; `ttk_state_probe.py` matches; the controls
test proves the vector write keeps the lease and the last table word is still
guarded. Then the same water (depth 512, original mode 1 / clips 80/81,
Vanilla 8–14 units/frame) got its handler `0x800539f8` hooked — added to
`game.local.toml` `mod_function_entry_funcs`, regenerated (one generated line;
`build.py` itself stops at the pre-existing runtime patch-stack drift, so
generation was run directly with the built recompiler) — retargeting the
world root to camera-relative WASD in the land run band (gain 4, 80..120 per
tick). Guards added for `0x800486b0`/260, `0x800539f8`/452, `0x800788e0`/200.
Live: 45–58 units/frame, mouse steering, strafe, wade jump 98, 0 refusals,
turn/run/jump after the zone; slot 2 regression clean. Wading into a wall
idles (original handler); turning resumes. ttk-input-test, ttk-aim-test,
ttk-controls-test PASS. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`.
Isolated cards: `recomp/analysis/d08p-slot2/cards/openbios/` now holds copies
of state slots 01 and 02. User playtest: "it really works. perfectly" →
D08P **Done**; docs committed and pushed. Next session: pick the next job from
the board (no job auto-started).

## 2026-09-29 — D08P held keys across recapture, fresh capture offers (Needs playtest)

User: iteration 4 changed nothing for them (session log shows a fresh launch
and F7 slot 2 load 30 s before the report). Live Xvfb + xdotool probes
(`/tmp/d08p_live_probe*.py`, isolated `recomp/analysis/d08p-slot2/`, software
and OpenGL, debug load and the real `F7 → 2 → l` menu, the user's own profile)
all show the wade at 48–54 units/frame with mouse look; slow headings are the
original wall slide beside the spawn. Reproduced instead: a Shift/W held
through any recapture was wiped by `clear()` → walk gait + Walk pad until
re-pressed. Fixes: capture resyncs bound keys/mouse buttons from SDL device
state; capture offers expire after 8 frames so the original pause menu is not
captured; sustained tank fallback announces `ORIGINAL MOVEMENT (reason)`;
`run.py` mirrors stderr to `recomp/build-local/logs/session-*.log`. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`.
If the playtest still degrades, read the session log's `[TTK lease]` /
`[TTK input]` lines first. Turret itself not reached in probes (S doorway →
bridge → deep water; N platform → W walkway dead end). No commits until asked.

## 2026-09-28 — D08P waist-deep land locomotion (Needs playtest)

User: after speed recovered, the turret wade still crawled, mouse look was
dead in the water, and after leaving they only walked with no Shift. Isolated
slot 2 already uses land run anim 78 on D-pad forward; Modernized dropped
the lease on tank turns 70/71 and injected Walk. Waist-deep / mid water
now keeps the land camera lease, faces the view, converts 70/71 to run,
and does not inject Walk. ttk-input-test, ttk-aim-test, ttk-controls-test
PASS. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`.
Playtest F7 slot 2: run + mouse in the wade and after stepping out. No
commits until asked.

## 2026-09-28 — D08P identity reader stall (Needs playtest)

User: after the slot-2 recapture binary, everything ran slow including
audio. The apartment pair check ran on every word of every identity
guard, many times per frame. It now only inspects the two LEVEL00
apartment words. F7 recapture behavior is unchanged.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`.
Restart the player build; the running session will not pick this up.
No commits until asked.

## 2026-09-28 — D08P F7 slot 2 recapture (Needs playtest, iteration 3)

User: iteration 2 did not restore the turret room; F7 UI slot 2 (file
`state_800AB6FC_slot01.pst`, turret around the corner) lost Modernized
entirely. Isolated dump matches the flooded wade (depth `0x100`, anim 63,
normal camera, apartment words already patched). F10 left
`initial_capture` false; F7 released the mouse and did not request
recapture; the F7 menu could eat `capture_offer`. F7 now sets
`initial_capture` even if the cursor is already free; offers stay until
`allow_capture`; identity remaps the complete patched LEVEL00 pair.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS (including F7
recapture and slot-2 patched overlay). Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`.
Playtest: F7 load slot 2, mouse/WASD/strafe/crosshair without a manual
F10. No commits until asked.

## 2026-09-28 — D08P turret wade (Needs playtest, iteration 2)

The placed checkpoint is host F7 savestate slot 1, not a memory card. Isolated
dump: flooded turret room, depth `0x100`, anim 63, normal camera, no
`0x20000000`. First tank fallback made A/D turn (D-pad L/R → anims 71/70) and
classified 70 as free swim, which hid the crosshair and blocked Escape
recapture. Fixes: free swim only at original `≥0x281` or states 4/5; A/D
fallback L2/R2 strafe; recapture from `0x8005a210`; reticle on camera-only
locomotion. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`.
Playtest from that savestate: mouse, A/D strafe, crosshair, Escape resume.
No commits until asked.

## 2026-09-28 — D08P turret WASD freeze (Needs playtest)

Crystal-2 ceiling turret (after first crystal, broken-bridge water corridor)
dropped the modern camera lease. Captured WASD was inert because move actions
use pad 0 and Forward inject requires the lease. F10 could not restore WASD.
First fix: captured play without a lease feeds original D-pad tank bits
(iteration 2 replaced A/D turn with strafe). stderr
`[TTK input] WASD tank fallback` and `[TTK lease] inactive (...)`. Playtest
that room; if the window ignores keys entirely, that is a guest halt, not this
path. D07A/B, D08A, D08H and D10 distance closed on user play acceptance.
Binary `fd9e4c4d015673ec6fa1dcdedb3bcd60d9b74ddb4377786afb5a241d0f4e4c28`.
No commits until asked.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User: do not add a scuba device to this game. D08N is **Cancelled**. TTK
underwater air stays original and automatic; Bio Mask stays gas-only. No scuba
prototype, inventory slot, HRP scuba art, or toast. Historical notes that once
listed D08N as a swim follow-on are superseded. Next job: pick from the Todo
list (D08L, D09, and other open rows; not D08N). No code change. No commits.

## 2026-09-28 - game/ drop folder (tested)

First run of `run.py` / `build.py` creates `game/` with a README. Importer accepts
Redump USA `.cue`/`.bin` or CloneCD `.img` (hashes `708c0404...` and `230a34c2...`)
when the EXE pin matches. Europe/PAL `SLES-01515` prints a console error. User
tested the drop-folder flow. Original media is never overwritten.

## 2026-09-28 — D08O closed (accepted); nothing pending from the swim work

Playtest 3 on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb` accepted
everything: A/D strafe, Ctrl dive, underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the round-3 shallow → platform ledge jump, and the `biomask-small.png`
item-switcher icon. D08O is **Done**; the entries below are history. Do not
reopen the swim/ledge code without a reported regression (55 §6 rules 7–9).
Next job: pick from the Todo list in `MODERNIZATION_JOBS.md` (D08L, D09 are
open; D08N scuba is cancelled). No commits.

## 2026-09-28 — D08O.4 ledge jump: plain 98 arc, taller (Needs playtest)

Playtest on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.
Swimming (Ctrl dive, underwater WASD/Space/Ctrl, A/D strafe, mantle) and the
Bio Mask icon are **accepted** — do not touch. Only the ledge jump changed:

1. **Subway shallow water → platform** (west platform, room 21 → 57). From
   touching the wall, from a few steps back, running or standing, Space alone
   or with W: Duke does the ordinary directed jump (98) — not the tall 97 hop of
   round 2 — about a head higher than his normal shallow-water jump, and lands
   on the platform. At the wall / one step back he rises first and moves onto
   the platform only once his feet are above the lip (XZ held, then 3400/4000);
   from further back he travels from takeoff (1900 + 3.5·distance, max 6400).
   Landing is 98's normal landing (sometimes the hard 105). From ≳1650 units the
   ledge is out of probe reach and the plain 98 falls short — intended.
   `ledge_assists` in the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1`
   prints `ttk-ledge probe/launch/clear/abort/ballistic` and per-tick `ttk-swim`
   lines to stderr (`runs/<name>/runtime.log` in the lab).
2. If it still looks too high: `k_ledge_vy` (−9000; 98's own ≈−6900, rise scales
   ≈ VY/96·ticks — −8000 was not tried, apex margin at −9000 is ≈200 above the
   635 ledge). If it bumps the wall: `k_ledge_hold_distance` (320) / the hold
   speeds. If it overshoots into the back wall: `k_ledge_speed_per_unit` (3.5).
3. Regressions to keep: everything in the D08O.2/.3 entry below.

Mechanics (`swim.inc`): `swim_ledge_ahead` (probe 0/320/640/960/1280),
`swim_try_wade_jump` → `swim_start_airborne_jump(…,98)` + `delayed_vertical =
k_ledge_vy` (applied at the `8003ebf4` ballistic hook, `terrain_epoch` gated),
`swim_ledge_ballistic` (called from the same hook each ballistic tick: hold
XZ = 0 below the lip when `swim_ledge_hold_xz`, else exact XZ via
`swim_boost_want` / `swim_boost_horizontal`; clears `swim_ledge_pending` once the
foot is 32 above the top). `swim_update` drops a stale pending flag whenever
anim ≠ 98 or state ≠ 9. Lab: `/tmp/swimlab/ledge17.json` (15 trials),
`deep3.json`, `walk`. No commits.

## 2026-09-28 — D08O.2/.3 native dive + wade ledge assist (Needs playtest)

Playtest on binary
`71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5`:

1. **Ctrl at the surface** → Duke dives (original anim 133, `+0x22c` = 5).
   Underwater: **W/A/S/D** swim where the camera looks (thrust 128–130),
   **Space** ascends, **Ctrl** descends, looking up/down + W follows the view.
   Duke surfaces automatically (122, state 4); Ctrl again re-dives. If Ctrl is
   still inert: `swim_dives` in the debug JSON must increment; `swim_try_dive`
   refuses while `+0x224 & 0x241` or `+0x244` (timer) is non-zero.
2. **Subway shallow water → platform** (west platform with the hazard stripe,
   room 21 → 57). From touching the wall, from a few steps back, running or
   standing, Space alone or with W: Duke should hop high and land on the
   platform every time, with no bump-off (107) and no splash-cancel. Landing
   is the original hard-landing 105 (the 97 arc is higher than 98's). From
   >≈1300 units the plain 98 still falls short — intended. `ledge_assists` in
   the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1` prints
   `ttk-ledge launch/switch/abort` and probe lines to stderr.
3. Facing a plain wall in shallow water with W+Space → vertical hop (was a
   cancelled 98 splash).
4. Regressions to keep: A/D surface strafe (123/124), Space ascend only, E /
   mantle exit, shallow W/S/A/D wade, Space 98 in open water, Ctrl 105.

Mechanics (`swim.inc`): `swim_ledge_ahead` (standing probe `800797f4` from
0/320/640/960 along the view, world XZ written and restored inside the query),
`swim_try_wade_jump` launches 97 and arms `swim_ledge_pending`,
`swim_ledge_assist_update` (early hook, runs without the swim lease) switches to
98 when `+0x10` ≤ ledge top − 96, keeps VY via `delayed_vertical`/`delayed_takeoff`
(applied at the `8003ebf4` ballistic hook), XZ = 2200 + 3.2·distance (max 5400)
via `swim_boost_want`; `swim_reassert_boost` now also runs in flight.
Native swim: `swim_try_dive` (`80045564`), `swim_steer_underwater`,
`swim_thrust_input_ready` (Square injection in `pc_input.cpp`). Lab scripts live
outside the repo in `/tmp/swimlab` (`swim_lab.py`, `ledge*.json`, `deep3.json`).
No commits.

## 2026-09-28 — D08O implemented (Needs playtest)

Playtest the deep-water trio on binary
`d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e`:

1. **A/D** while swimming → camera-relative strafe (anims 88–93), no turning.
   If A/D still turn, dump `800d1a68/800d1b80[player+0x233]` — the strafe pad
   bits are resolved from those layout words (default L2=0x100 / R2=0x200).
2. **Ctrl** at the surface and mid-water → descend; **Space** → ascend to surface.
   Dive stops at `+0x1c8 − 0x1e0` (floor margin) or `surface + 0x1800`.
3. **Space** at a ledge no longer hops out; leave via **E** / original mantle.
   Shallow wade jump / ledge hop (`8003e2d0(97|98)`) is unchanged.

Regression already confirmed in the shallow subway savestate: W/S/A/D wade with
stable heading, Space wade jump 98, Ctrl crouch 105. ttk-controls-test and
ttk-input-test PASS. No deep-water savestate exists; deep zones (rooms 31, 35,
42, 64, 65, 68, 72) sit behind a keypad door (−7400,51700) and a fence
(40636,49390) from the subway state, and teleporting snaps Duke to geometry.

Code: `recomp/src/ttk/swim.inc` (`swim_strafe_pads`, `swim_apply_vertical`,
`swim_try_mantle` removed), `pc_input.cpp` free-swim block,
`modern_controls.h`, `tests/local/pc_input_native.cpp` stub. No commits.

## 2026-09-28 — D08O marked next (docs only)

**Next job to pick up: D08O** — deep free-swim polish.

Locked design in `documentation/55-swim-controls-research.md` (§1.2, §8) and
`documentation/54-swim-redesign.md`:

1. Fix A/D camera-relative strafe (W/S already OK)
2. Fix Ctrl descend from surface / mid-depth
3. Strip free-swim Space→ledge exit → **mantle / E only** (keep shallow wade jump)

D08M closed: shallow jump + ledge hop Done (do not reopen unless regression).
Space↑, W/S, E mantle accepted on binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

Hard rules: no invented `+0x20c`, no entry `+0x1f8` clear, no world-XYZ swim.

This session: documentation/design only — no `swim.inc` changes.

```sh
python3 recomp/tools/local/run.py
```

No commits.
