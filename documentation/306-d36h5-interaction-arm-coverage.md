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

## D36H8 - reaching arms over the neck (2026-11-03, accepted)

U: run-jump across the slot-11 sewer water with E held; the raised empty arms
of the airborne reach flickered before the catch (user video, looking down).
L: reproduced on a private copy at 120 Hz (7 presents on, 13 off, 15 on);
Classic the same; Duke's room stays listed and his own camera skips the
actor cull.

Owner: not this rig. The reach 109 keeps the perspective action `ground`, so
its empty arms (parts 10-15, equipment 0) are D36H4 body-path limbs, hidden by
`first_person_hide_body_joint` whenever the D14E1 eye-over-pelvis envelope
fails. Draw-time trace: the reach leans the body and the eye ran 106-117 units
ahead of the pelvis (limit 96) while staying 16-24 units over the neck.

S: `first_person_arms_eye_inside` admits empty arms while the eye is over the
neck joint (horizontal 128, 0-224 above), in addition to the pelvis envelope.
Body/legs keep the pelvis rule; a displaced eye fails both; armed arms, the
contact rig, third person and Vanilla are unchanged. Draw matrices only, no
state, the same in replay. Regression: the D14E1 group pins the neck away for
its displaced-eye cases and adds the reach (eye 116 ahead of the pelvis, over
the neck: arms drawn as Body, body hidden; neck away: arms hidden; armed arms
unchanged), both costumes, H 256 and 160.

L: after the fix the same route shows the arms continuously through the reach
into the catch and mantle at 120 Hz (two runs) and 180 Hz. Evidence and route
scripts: `recomp/analysis/d36h8/` (local). Executable
`21e023029b3487de97d149e736a9b8f964213cbb588c5f0b59e3b4bc9792bd1b`.
U: accepted after the playtest ("perfect. lock it in and close us out").

Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## D36H9 - ledge hang: arm flicker and legs-only view (2026-11-04, accepted)

U: slot 9, first person, unarmed: jump catch, hang, look down and turn along
the wall; an upper arm popped in and out at the side, and only the legs (and
holstered pistol) showed below the eye. L: reproduced on private copies at
60/120/180 (Xvfb recordings, per-frame skin-pixel counts) on the D08J7B and
D08J7C builds alike.

Owner: the contact rig (`first_person_contact_arm`, Mantle). Three causes,
found in order with a temporary draw trace (removed):

1. Draw ownership was lost mid-draw. The hang draw iterates a list
   (0x8001ca4c, ra 0x80075810) inside Duke's own draw; the hook treated every
   such call as the object loop's next step and cleared `duke_drawing`, so the
   arm meshes went through the generic scene path. S: a call below the
   actor-entry stack frame (`actor_draw_sp`) is nested and keeps ownership.
2. Camera-yaw shoulders swept through the view. The virtual shoulder anchor
   (lateral, 150 down, 64 back) is in camera axes; with the hands fixed on the
   lip, turning the view slid it from +190 to -220 across the eye and the upper
   arm crossed the view as a near slab. S: in Mantle the anchor frame follows
   Duke's drawn heading with the camera pitch (identical at zero look offset).
3. The eye itself alternated. The native solver casts four rays from Duke's
   pivot to the requested point (eye plus the native forward distance); looking
   down from a hang that point lies in the lip and the nearest hit (0x8003a5e0)
   alternated between two positions 84 units apart per tick, moving the eye and
   the camera-attached arms. A new hook there would change generated code
   (savestates), so: once the Mantle eye is fully blended in, the existing
   0x8002a038 easing hook takes our requested delta (rotated by the camera rows,
   as 0x800b444c does). Without the solver's push-back the hang eye sat over the
   root and lost the accepted framing (both hands on the lip), so a ledge hang
   (mode 6/7, anims 147-153) places the eye 128 units behind the root along
   Duke's heading, matching the native look-ahead framing. Overhead bars, water
   exits, blends, third person and Vanilla keep the native solve.

Legs (user decision, same day): keep the dangling legs and fade them only
while looking down, like Duke in a tight third-person spot. The game's own
close-camera blend (B+F/4) is effectively a binary hide, so the legs use the
third-person body's screen-door coverage (`first_person_legs_coverage`, all
faces host-drawn): solid to 45 degrees down (where they enter the view), gone
at the 60-degree look limit, in the hang and the pull-up (140); solid standing.

L: slot 9 slow pans at 60 and 180 show no skin-pixel jumps (before: alternating
every tick at the screen corner); the pull-up shows the arms continuously.
D35G alley hang at 60/120/180: look-down, pan, shimmy and pull-up clean; legs
hidden hanging and mid pull-up, solid standing; third person unchanged; Vanilla
(--vanilla) hang unchanged. D36H8 slot-11 run-jump mantle at 120/180: catch and
mantle continuous, identical to an A/B run with the new switches off.
Developer A/B switches: `DNTTK_FP_HANG_EYE=native`, `DNTTK_FP_CONTACT_ARMS=camera`,
`DNTTK_FP_HANG_LEGS=solid`. Native controls 147 groups (new D36H9 group:
nested list call keeps ownership, the loop step ends it; the contact-rig group
faces Duke to the identity camera), input, aim and near suites, Python profile
and input suites pass. Scripts: `recomp/analysis/d36h9/` (local).

Limits: the fade is pitch-based, not per-limb visibility; the legs return
solid at the end of the pull-up rather than as the knee reaches the lip (the
clip's start frame is not readable statelessly); the 128-unit setback is a
measured constant from slot 9 and the alley, not per-level geometry; without
the GL coverage path (`psx_mod_present_replay_available`) the legs stay solid;
only Polymorph was checked. Executable
`a87d304b36c72636bab815b3367ecc7ec3e974701ce7d22d63f17a9034a78ec4`.


Revision 1 (2026-11-04). User: "the leg view just isnt it.
can you revert it back to the hanging legs while i make a decision. and the
arms seem to have changed too because now i can really badly see Duke's
disembodied shoulders". Both reverted to the earlier presentation: the legs
are solid dangling legs again (fade kept only as developer A/B
`DNTTK_FP_HANG_LEGS=fade`), and the contact-rig shoulders are camera-anchored
again (body-heading frame only as `DNTTK_FP_CONTACT_ARMS=body`). Kept: the
draw-ownership fix and the steady hang eye. Slot 9 slow pans at 60/180 still
show no per-tick arm flicker; native controls 147 groups pass. The leg
treatment waits for the user's decision. Executable
`0434285d96aba58fdb4980f75fd122a3fdb76f5206127a32120f0bb22c5b4ed6`.

Revision 2 trial (2026-11-04). User: "when hanging, all i
want you to do is try: moving the legs back a few inches on my screen. lets say
4 inches back as i perceive it." In a ledge hang the dangling leg rig moves a
further 140 units back along the view heading (the D36J5 leg setback, plus
140): at FOV 90, the legs' depth and a 27-inch 16:9 screen, about 4 inches
toward the bottom of the screen. Looking straight down (the 60-degree limit)
they now sit just below the bottom edge; before, their top was about 29% up
the screen. Developer tuning only: `DNTTK_FP_HANG_LEGS_BACK=<units>`. Pull-up,
standing legs, arms and eye unchanged. Executable
`074ace53ff13ae7826f85864ace7ae2af884c2254a248c187ef365d6dd08ed7e`.
Iterated the same day: "bring em forward an inch", then "basically a sliver
of leg is all it needs to show". Measured leg top above the bottom edge at the
60-degree look: 0 units 25.5%, 30 15.3%, 60 9.7%, 80 4.5%, 88 2.3%, 92 1.0%,
105 and 140 none. Now 88 units (a sliver). Executable
`89fe6df1b40ee15028a96d25041714ba39a988eba9c7c01de6d1d3d2e47b2ebe`.

U: accepted ("thats it, thats a winner. lock it in, close it out"; "the job was
way simpler than we thought"). Shipped: ownership fix, steady Mantle eye, legs
88 units back. Fade and body-heading shoulders rejected (A/B switches only).

## D08J7D - armed off hand on the lip (2026-11-04, accepted)

The contact rig now also serves the pistol's free arm (parts 10-12, equipment
2) in the armed sidearm hang and the D08J7C one-handed pull-up, while the hand
is on the lip and ahead of the eye; its drawn height is pinned to the lip.
Other armed Mantle cases (D35G1 step-ups) keep the hidden arm. Details and
evidence: [note 378](378-d08j7-mantle-fire-research.md) D08J7D section.

## D36H10 - water exit: vibrating arms (2026-11-05, accepted)

U: UI slot 12, first person, climbing out of the water: the arms vibrate.
L: private copy at 180 Hz with an in-draw trace of the contact hands. The eye
moves smoothly; the native hand targets themselves swing about 200 units
forward and back every 2-3 game updates through the pull-up 140, shoulders
included, so the whole upper body rocks.

Owner as drawn: the contact rig (WaterExit, then Mantle). Not at fault: the
cause is the D08J7B 1.6x pull-up. Duke's pose is built from four animation
channels (0 legs, 1 right arm, 2 left arm, 3 chest/head); the speed-up scaled
the runner time of channels 0 and 1 only. Channels 2 and 3 ran the original
clock and the game then puts them on channel 1's frames by offset
(0x8005a11c), so each time channel 1 crossed into a frame first they kept a
stale previous frame and jumped up to two frames ahead (channel 1 `1500>1501`
while channel 2 showed `1509>1511`, then `1509>1512`). At native speed all
four advance in lockstep and the arms make one smooth heave per frame. The
D35G alley ledge pull-up had the same desync.

S: `pullup_advance` (`climb_fire.inc`) scales channels 2 and 3 as well (runner
returns 0x8005a75c / 0x8005a7bc, deltas sp+0x14 / +0x16), once per update,
only while that channel plays 140, so an armed gun arm on its own clip stays
unscaled. Root motion, events, timing and end position are unchanged. The
contact rig, eye and hang presentation are untouched. Regression: the D08J7B
native group runs channels 2 and 3 (scaled once, other clip, wrong caller,
Vanilla). Developer A/B: `DNTTK_PULLUP_SYNC=off`. Diagnostic:
`DNTTK_PRESENT_TRACE=1` now also logs `contact_hand_l/r`.

L: body-frame hand direction reversals during 140: 10 forward / 10 up before,
4 / 6 after (native speed 7 / 4); recordings show the arms coming down
steadily instead of snapping to the top corners. Same end position and yaw,
clip 66 game frames in both. Executable
`5ab597cc778598dc501fd281499ebb77bedfd937dbef0ad153d655643d0b590e`.
Limits: recorded at 180 only; slot 9 hang and armed one-handed pull-up not
re-recorded; third person not visually judged (dark spot). Evidence:
`recomp/analysis/d36h10/` (local).

U: accepted ("fixed! lock it in and close it out. excellent work").
