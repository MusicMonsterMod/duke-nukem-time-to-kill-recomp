# D36J5 - Optional first-person legs

2026-10-28. Done, user-accepted; closeout authorized.

User accepted the result and authorized closeout on 2026-10-28: "great work. lock it in and close us out". D36J5 and D36H7 are Done. Recorded verification and campaign limits remain unchanged; D19X/D19Y remains pending separately.

## Behavior and ownership (S: source/native tests)

Modernized first person restores ordinary leg parts 1-8 and the right-thigh
attachment 21 throughout every supported first-person action. Root/pelvis 0,
chest 9 and backpack 20 remain hidden. This restores the ordinary locomotion
legs without reopening the torso fitting work. Traversal, swimming, flight and
interaction eyes now admit the same legs; contact-arm, reflection and third-person
owners retain their existing poses and camera policies.

A Legs descriptor retains native animation, polygon clipping, depth and
proximity-fade correction. Its mesh projection uses the same H=256 path as
DisplayHand, DisplayArm and Kick. It does not become an always-on-top weapon
layer. No bone scaling, animation retargeting, gameplay matrix edits, new hooks,
retail assets or framework changes. Semantic part lookup handles costume order.
The existing displaced-eye guard continues to reject unsafe locomotion legs.
Action eyes use their authenticated perspective gates instead of the upright
eye-over-pelvis envelope, which is not valid for climbing or swimming.

Q and door-kick owners still run before ordinary-body admission, so they replace
their corresponding leg chain and keep their accepted poses and projection.
Turning ordinary legs Off does not disable these attacks or their presentation.

OPTIONS > ADVANCED > CAMERA AND AIM > FIRST-PERSON LEGS defaults On. It uses
the existing draft/Confirm/Cancel/Defaults and menu-sidecar path. Profile schema
59 adds controls.first_person_legs; missing/invalid values recover to On without
resetting other preferences. The launcher supplies DNTTK_FP_LEGS; Vanilla gets
Off and retains its original presentation. No player settings are edited by tests.
DNTTK_FP_BODY=visible remains the earlier full-body diagnostic; hidden/invalid
nonempty body overrides still select no ordinary body. No additional public CLI.

## Verification

- S: player build and native controls/input/aim/near suites pass. Controls include
  both costume mappings, default/On/Off, hidden torso, unchanged native matrices,
  scope cleanup and the pre-existing no-body/interaction/kick/Vanilla guards.
  The full controls run includes the owned campaign overlay fixtures.
- S: emitted mesh-packet tests include Legs across H=160,177,191,209,256,277 and
  three aspect margins. Coordinates and packet words stay invariant while UVs
  and depth remain unchanged. World body/contact comparisons still change with H.
- S: 109 profile/launcher tests pass, including schema-58 migration, invalid-value
  recovery, On/Off sidecar persistence, FOV retention and launcher propagation.
- L: private 60-setting real-key menu run verifies row placement, Confirm Off,
  cancelled On, confirmed Defaults On, Q return and third/first view return.
- L: a subsequent real-mouse downward sweep and real-key menu run confirms
  visible legs at FOV 90, disappearance on confirmed Off and return on confirmed
  Defaults, with torso absent in all three inspected captures.
- L: controlled downward 120-setting captures show legs without torso at FOV
  60/default/80/90. Native leg framing matches while the world projection changes.
  Bounded walk, crouch, jump and Q samples were captured. Replay reports 1028
  completed worker jobs, zero worker failures and zero failed sessions.
- L: all 41 player save/card/profile files retain hashes and mtimes. Private
  copies/settings and Xvfb were used. Evidence stays in recomp/analysis/d36j5/.
- Repository/privacy check passes. No framework changes or patch export needed.

## Limits and excluded attempts

The first real-key End capture did not establish a downward legs view. Two
attempts combining synthetic gameplay input with menu keys failed to produce
a menu sidecar and are excluded from menu evidence. The completed real-key menu
run and dedicated downward gameplay run above are separate evidence.

The 120 presentation setting is not a sustained-120-fps or audio claim; the
runtime reduced presentation cadence during this diagnostic. No exhaustive
campaign/action/costume visual sweep, new software-renderer check or physical
controller review. Costume mapping and isolation are native-test evidence.
Human visual acceptance was received at closeout on 2026-10-28.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.


## Revision 2 - Maximum-downward running hip exposure

User reports the open torso join while running and clarifies maximum downward
look. The ordinary leg draw now translates the complete chain and thigh
attachment 48 units backward along the eye's horizontal heading. This is a
rigid draw-only offset, independent of FOV and animation phase. Bone rotations,
relative limb positions, foot height and the gameplay skeleton stay unchanged.
Q/door owners precede this adjustment; torso suppression and the saved toggle
remain intact. The visible-body diagnostic also retains its comparison pose.

Native controls regression checks both costume maps, default/On/Off, all tested
FOV projections, preserved rotations/Y and the shared backward offset, while
confirming the original matrices remain untouched. Player build, full native
controls and emitted-packet near suite pass. An initial 80-unit offset hid too
much of the idle legs and was reduced to 48; it is not the delivered candidate.
Private maximum-downward running/backward/crouch sequences at default and 90
FOV are retained in recomp/analysis/d36j5r2/. User appearance review was accepted at closeout; this is not exhaustive terrain/campaign coverage.


## Revision 3 - All supported first-person actions

User selected restoration for climbing, toilet use and all other documented
leg-use contexts. The source of the coverage inventory is the accepted D36A/C/K
matrix in note 257, the interaction-selector inventory in note 238, the contact
coverage ledger in notes 305/306 and the broader flight/swim work in note 307.
Historical slot numbers in those records are not assumed to describe current
player saves. Tests use private archived copies and record actual action owners.

Leg admission is now shared at the authenticated primary Duke joint hook before
individual arm policies. It is no longer a Ground/Committed/Fall whitelist.
The upright displaced-eye envelope remains for locomotion, while authenticated
action eyes can render legs outside that envelope. Unknown/scripted/death and
third-person fallbacks remain under the existing perspective resolver.
The fixed projection, 48-unit backward draw translation, world depth, native
animation, torso hiding and saved On/Off setting apply consistently. Q and door
chains retain their earlier priority. No additional setting or schema change.

### Complete policy inventory

| Documented family | Owner / identifiers | Revision-3 policy |
| --- | --- | --- |
| Idle, walk, run, strafe, wade, crouch/walk | Ground; 63 and 178, gait handlers | Shared legs, locomotion displaced-eye guard |
| Idle gestures, held inventory | Committed; 64-69 and existing inventory exception | Shared legs, existing camera ownership |
| Jump, landing, fall, wall bump | Ground/Committed/Fall; 96-109 with owned-state checks | Shared legs; no jump/gameplay edits |
| Ladder | Ladder; 185-190, supported mounts/exits | Shared legs with authenticated ladder eye |
| Chain/pole | Pole; 191-196, supported mounts/exits | Shared legs with authenticated pole eye |
| Climbable wall | Wall; 197-206 and owned transitions | Shared legs with authenticated wall eye |
| Ceiling hang / monkey bars | Overhead; mode 6/7, 147-151 | Shared legs with authenticated overhead eye |
| Mantle, hang, shimmy | Mantle; 134-153 with owned mode/history | Shared legs with authenticated mantle eye |
| Water climb-out | WaterExit then Mantle | Shared legs across owner handoff |
| Surface and underwater swim | Swimming; modes 4/5 and water predicates | Shared animated swim legs |
| Jetpack | Jetpack; 163-170, all flight schemes | Shared flight legs |
| Grab, push, pull | Push; 119-121 | Shared legs; contact arms unchanged |
| Toilet | Use; 275 | Shared legs; native callback unchanged |
| Vending / NPC use | Use; 277 | Shared legs; native callback unchanged |
| Wheel / lever | Use; 270 / 118 | Shared legs; native callback unchanged |
| Keycard / crystals | Use; 266 / 267-268, equipment-to-contact handoff | Shared legs through held/contact phases |
| Statue | Use; 279 | Shared legs; native one-time effect unchanged |
| Mirror grooming | Mirror; 278 | Shared primary-eye legs; reflected full Duke unchanged |
| Q and hinged door kick | Kick; host Q / native 116 | Existing kick chain first, shared policy for other leg |
| Rare selector/callback variants | Note 238 classes 6/9/12/14/16/17/18 and alternate cameras | Shared legs whenever the existing authenticated eye is active; no new camera eligibility inferred |

S: expanded actual-hook tests cover On/Off, every semantic leg/attachment part,
torso suppression, both costume maps, fixed/world-depth ownership and native
matrix preservation across all nine contact owners, swimming and jetpack.
Existing action-choice, third-person, reflection, script and Vanilla gates remain.
Player build and native controls/input/aim/near suites pass. The existing setting
and profile persistence paths did not change.

L: runtime evidence and exclusions are appended below. This complete policy
inventory is not a claim of newly replaying every campaign callback variant.


Revision-3 runtime evidence (L): private current toilet/vending routes at 60/120
observe 275/277 and completion; the 60 run includes maximum-downward input.
Archived wheel/crystal/card routes at 60 observe 270, 267 and 266 with their
existing owners. At 120, ladder motion observes 186-189, mantle observes 149/140
and ground return, and the corrected attached pole fixture observes 192/196 and
return. The inspected ladder downward capture visibly contains legs without
torso. Some stationary/action poses remain below the screen or world-occluded;
admission does not force legs on top of the environment.

Excluded: the first pole attempt used archived slot 1 and observed only ground/
fall; attached slot 0 replaced it. The first push recipe used semantic grab
input and missed 119-121; it is not successful push coverage. Earlier current
interaction captures used a smaller pitch sweep; maximum-downward captures
were repeated separately. Native tests cover the remaining documented action
owners; wall/overhead, mirror/statue, swim/flight and rare campaign variants are
not newly certified by this bounded runtime pass. All completed private runs
preserve the player's 41 files. Existing arm/action evidence remains historical.

The corrected 60-setting push route uses the documented bound-key recipe and
observes all 119/120/121 phases, repeated push/pull, downward look, release and
private save/load recovery. Player files remain unchanged. The earlier missed
semantic-input attempt remains excluded.


## Revision 4 - Crouch locomotion legs

U: crouch running loses the legs. L: private maximum-downward crouch movement
captures reproduce missing legs. Recorded running eye offsets reach about
130 units horizontally from the pelvis; crouch-walk eye height can fall to
108-123 units. Both violate the old upright-only 96-radius / 128-minimum-height
body guard. Samples are sequential diagnostic reads, not atomic frame dumps.

S: leg admission for grounded crouch poses 176/178 now measures horizontal
clearance from the pelvis-to-neck segment, with the existing 96-unit margin.
The minimum height is 64 for this validated crouch footprint; the 352 maximum
remains. Neck lean must be at most 192 and neck rise 0-256, otherwise the upright
guard is retained. Modes/history must both be grounded (0/1). The test uses
current draw matrices, including replay, without a new saved flag or input latch.
Standing and empty-arm guards, torso hiding, the 48-unit backward translation,
projection and crouch animation/movement are unchanged. This is not a blanket
crouch exemption from displaced-camera rejection.

S: player build, full native controls and near suites pass. New actual-hook
checks cover low eyes, forward lean, side/forward displaced-camera rejection,
vertical bounds, both costume maps, two projections, hidden torso and untouched
native matrices. Upright comparison cases still reject the crouch-only envelope.

L: before 60 / candidate 120 private runs sample crouch idle, walk, run, reverse,
strafe and standing recovery at default/90 FOV with maximum downward look.
Candidate downward run/walk captures visibly retain leg geometry where
the baseline running capture loses it. Strafe samples include both visible leg
edges and poses below the picture; admission does not force an offscreen limb
into view. Poses 176/178 are recorded; all 41 player
files retain hashes and mtimes. Evidence is recomp/analysis/d36j5r4/, with the
baseline preserved in before/. This does not newly certify every low ceiling,
terrain/costume combination or sustained presentation rate. User review accepted at closeout.


## Revision 5 - Current slot 7 ladder-end hang

U: looking directly down at the end of a ladder exposes disembodied legs.
L: the current private copy of UI slot 7 is pose 207, mode/history 3/3, owner
Ladder. Its animated hips swing forward enough that the ordinary 48-unit
setback exposes their open upper ends at maximum downward pitch. The original
private save is retained under recomp/analysis/d36j5r5/cards/; the earlier slot-7
vending evidence describes a different historical save, not this reproduction.

S: for authenticated Ladder leg draws only, the entire ordinary leg rig now
moves backward enough to place both semantic hip roots below the fixed display
picture (`camera_y - 0.5*camera_z >= 48`). The usual 48-unit minimum remains;
the bounded maximum is 192. Both chains and the right-thigh attachment share
one translation, preserving bone relationships, rotations and foot height.
The calculation uses native hip matrices and the current draw camera, so it
is independent of world FOV and needs no new animation/replay state. Other
action/locomotion owners, Q/door chains, torso hiding and the setting are unchanged.

S: recorded-pose calculation requires about 110 units in this slot-7 hang;
the earlier ordinary ladder downward sample still requires only the existing
48. Player build, full native controls and near suites pass. The new actual-hook
regression uses the hang geometry, both costume maps and H=160/256, checking
hip clearance, identical whole-leg/attachment shifts, unchanged level-look
framing and original skeleton bytes. Existing crouch and arm regressions pass.

L: before/after 60 captures at default/90 and down/up/side looks are retained
locally. The default/90 downward pairs match camera and native matrices exactly
and remove the exposed upper ends; visible knee edges can remain at the bottom
of the picture. This does not force offscreen legs into view. Matched-input
status for every comparison is recorded in matched-inputs.json. A 120-setting
motion run observes hang 207, exit 210, fall/ground recovery, look sweeps and
private save/load. All player files preserve hashes/mtimes. These are bounded
scene/pose checks, not every ladder or sustained-rate certification. User review
accepted and closeout authorized 2026-10-28; no framework changes.
