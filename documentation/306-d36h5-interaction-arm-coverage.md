# D36H5 - Shared contact-arm coverage

2026-10-28. The user accepted the traversal rig in [note 305](305-d36h5-contact-arm-rig.md)
and selected expansion across interactions, highlighting current slots 6 and 7.
The user subsequently accepted the expansion and explicitly authorized
"lock it in, close us out". D36H5 is Done for the accepted contact-arm scope.
Labels: U user report/acceptance, S source/native tests, L isolated runtime observations.

## Accepted closeout

The implementation, tests, manual and engineering record are included in the
authorized MusicMonsterMod commit/push. No post-acceptance gameplay changes,
rebuild or launch. Broader unverified presentation coverage is retained as
D36H6 (Todo), not claimed completed by this acceptance. The isolated restoration
observation below remains unresolved. No next implementation job is selected.

## Shared ownership

S: all nine authenticated contact-action owners now use the same dedicated
first-person rig: Push, Use, Mirror, Ladder, Pole, Wall, Overhead, Mantle and
WaterExit. The draw hook and solver use the same `interaction_depth_bridge`
predicate. There is no second traversal-only list or numeric-pose allowlist.
The existing perspective resolver still authenticates player, camera, action,
attachment and committed-state ownership before admitting an arm draw.

The accepted virtual shoulders and authored elbow pole are unchanged. Native
hands remain the exact world contact targets; upper-arm and forearm matrices
are solved for the eye view. This preserves world depth, lighting and near
clipping. The rig changes presentation only: native joint storage, animation,
movement, callbacks, inventory and save state remain authoritative.

Equipment phases remain distinct. Equipment 1/2 retains the held-item/combat
presentation; equipment 0 enters the contact rig. This matters for cards and
crystals, where a carried item precedes an empty-hand committed gesture.
Third person, reflected Duke, other actors and Vanilla retain their existing
presentation. Q/door legs remain under their accepted independent owners.

## Coverage ledger

S: actual draw-hook tests cover both costume joint orders, reachable and
extended contacts, joined elbow/wrist endpoints, exact native hands and
unchanged gameplay matrices. The diagnostic native comparison and reflected
actor isolation are checked across all nine owners. Use fixtures include
116/118/266/267/268/270/275/277/279; Push includes 119/120/121; Mirror is 278.
Existing rejection tests retain unknown/death/script/camera safety boundaries.

L: runtime evidence uses private copied cards and settings, Xvfb and dummy
audio. Current slot numbers below describe this session, not older inventories.
Continuous recordings and state samples are retained locally.

| Family | Observed route or evidence |
| --- | --- |
| Toilet | Current slot 6, pose 275, 60/120; free look, completion, private save/load |
| Vending/NPC gesture | Current slot 7 vending machine and archived vending route, pose 277; 60/120, health effect and weapon return visible |
| Wheel | Archived wheel, pose 270 at 60/120; native cyclic gesture and completion |
| Keycard | Archived insertion, pose 266 at 60/120; held-item to contact-arm transition |
| Crystal | Archived insertion 267 plus authenticated holder take/return 268/267 at 60/120; full cycle completes |
| Lever | Archived lever, pose 118 at 60/120; gesture and completion |
| Door | Archived door, pose 116 at 60/120; native completion and independent door-leg checks |
| Statue | Archived statue, pose 279 at 60/120; native one-time effect byte changes 0 to 1 |
| Mirror | Archived grooming, pose 278 at 60/120; repeated use, nearby reflection and return |
| Push/grab/pull | Archived dumpster, 119/120/121 at 60/120; sustained reversal, look, release/weapon recovery and private save/load |
| Ladder/pole/mantle | Accepted 60/120 motion and FOV/aspect evidence in note 305 |
| Wall | Archived wall catch, climb/diagonals, upper/lower exit, drop, pause/save/load |
| Overhead | Archived catch, all directions, turn/free look, drop and private save/load |
| Water exit | Archived swim-to-exit route at 60/120; swimming 127/128, mantle 140 and ground. Brief WaterExit owner sampled at 60; sustained exit uses Mantle |

Local evidence roots are `recomp/analysis/d36h5-coverage/` and
`recomp/analysis/d36h5-cov-d35*/`. The former contains current-slot comparisons,
action recipes, build/test logs and per-run identities. The latter reuse
historical natural routes with separate writable cards. A script exit code
alone is not action coverage: observed poses/owners are checked separately.
Initial mouse/semantic-command push attempts missed the action; an initial
water run used the wrong archived card set and never entered water. Those
attempts are retained and excluded from successful coverage.

S: final player build, native controls/input/aim/near suites and animation-save
recovery checks pass. Repository privacy/content validation and diff whitespace
checks pass. All 41 player save/card/profile files preserve hashes and mtimes.
No framework source edit or dependency-pin change.

## Restoration exception

L: the first 120-Hz overhead route completed its traversal/look stages but
halted after a mid-hang diagnostic save/load with GPU GP0 command `0x1F`.
The save resumed at `0x8002dc1c`. A native-arm comparison completed the same
route and restoration, but saved at a different instruction, so it does not
establish the cause or exonerate the rig. First-person 60 restoration passes;
twelve subsequent 120-Hz rig save/load cycles and a full first-person overhead
route repeat (`eye120-repeat`) also pass. The original failing
save was overwritten by the later private route, so an exact-state comparison
is unavailable. Stress-run saves are now preserved per iteration. This is an
unresolved isolated observation, not a confirmed arm regression or a fixed bug.

## Scope that remains open

This closes the separate arm-policy omission for all established contact
owners. It does not establish exhaustive campaign interaction coverage.
The [D35K inventory](238-d35k-interaction-inventory.md) still identifies rare
selector/callback variants without confirmed natural routes: underwater
class-6 pose 131, class-9 pose 269, class-12/type-368 pose 176, alternate
toilet/statue types, callback classes 14/16/17/18 and nine alternate-camera
event subtypes. Shared animation or plausible table data is not proof of
equivalent contacts. Their current camera safety policy remains intact.

D36H6 retains the broader weapon/upgrade/item, empty-hand locomotion,
flight/swim and effect/lighting review requirements. Those domains have
separate accepted presentation owners; they are not silently converted into
world-contact arms. The user accepted the delivered contact-arm scope; this
does not certify the broader unrun matrix.

## How to handle a future arm report

The user can simply report that the arms look wrong and describe the activity
or current save slot. Do not require them to know an owner, pose number or job
ID. Slot contents change; identify the current scene before reusing a recipe.
Use this note and [note 305](305-d36h5-contact-arm-rig.md) as the starting point.

1. Reproduce with private settings and copied saves/cards. Record the actual
   action, lower/upper animation, equipment, camera owner, costume, look angle,
   FOV, aspect and frame rate. Capture continuous entry/action/recovery motion
   and a useful still. Preserve any failing private save before another run.
2. Identify the presentation owner before choosing a fix. Contact gestures use
   `LimbOwner::Interaction`; combat/held-item display, empty-hand locomotion,
   swimming/flight and Q/door legs have separate policies. Read the ownership
   table in [audit 257](257-d36-first-person-presentation-audit.md). A symptom
   at a hand is not proof that the weapon grip or contact solver is at fault.
3. For contact arms, inspect `first_person_contact_arm` and
   `interaction_depth_bridge` in `recomp/src/ttk/first_person.inc`, and
   `recomp/src/ttk/contact_arm_ik.h`. Semantic parts 10/13 are upper arms,
   11/14 forearms, 12/15 hands; resolve the costume's joint mapping, never
   assume those semantic numbers are native matrix indices. Equipment 0 uses
   the rig; equipment 1/2 deliberately retains item/combat presentation.
4. Compare the same motion using the one-launch diagnostic below. Normal
   Modernized uses the accepted rig. The comparison restores native arm poses
   without changing a saved preference. In diagnostics add private settings
   and card arguments, as in the local reproduction scripts.

   ```sh
   DNTTK_FP_CONTACT_ARMS=native python3 recomp/tools/local/run.py
   ```

5. Preserve the design invariants: hands keep their native world matrices;
   shoulder roots follow the eye; elbows use an authored outward/down/back
   pole; length extension is axial and does not widen the arms. World depth,
   lighting and clipping remain authoritative. Do not fix an exposed end by
   hiding useful segments, narrowing FOV, restricting look or restoring the
   unwanted body. Authenticate a newly found interaction in the perspective
   resolver rather than bypassing guards or adding arbitrary numeric poses.
6. Add a regression for the reproduced defect beside the `contact_rig` actual
   draw-hook checks in `recomp/tests/local/modern_controls_native.cpp`. Retest
   both costume mappings, joined elbow/wrist endpoints, exact hand contacts,
   unchanged native matrices, first/third/reflections/Vanilla, equipment
   handoff, look extremes, 60/120 replay and private save/load. Run the native
   controls/input/aim/near suites and
   `python3 recomp/tools/local/test_animation_save_restore.py` as appropriate.
   Inspect the real player build in motion; arithmetic tests alone do not
   establish visual quality.
7. Append labelled source/runtime/user evidence and the precise regression
   route here or in a linked follow-up note. Retain before/after motion,
   rejected approaches, binary identity and explicit coverage limits. Obtain
   visual acceptance before closeout; commit/push still needs explicit user
   authorization for that future change.

The original accepted rig's parameters and rejected experiments are explained
in note 305. Local scripts are evidence/reproduction aids, not portable retail
assets: `recomp/analysis/d36h5/{climb,motion}.py`,
`recomp/analysis/d36h5-coverage/actions.py` and the `d36h5-cov-*` route scripts.
Keep original media and player saves out of Git. The implementation and native
regression tests are tracked, so the solution remains understandable even when
local capture directories are unavailable.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.
