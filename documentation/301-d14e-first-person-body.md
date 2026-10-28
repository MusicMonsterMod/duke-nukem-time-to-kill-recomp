# D14E - Connected first-person body across FOV

2026-10-28. Done, user-accepted. Implementation and documentation commit/push
closeout explicitly authorized as MusicMonsterMod.

Evidence labels follow [D36](257-d36-first-person-presentation-audit.md):
S = source/native tests, L = private runtime observations, A = prior acceptance,
U/H = user report or hypothesis. Captures, copied cards and retail geometry stay
local under `recomp/analysis/d14e/`.

## Problem and design target

U: higher FOV makes Duke appear to run on disembodied legs. The user requests a
professional first-person body baseline, considering 80/90 as potential future
defaults and higher future limits. This job does not change the accepted default
or the supported 60-90 range. A wider range requires its own observed acceptance.

A: D14B weapon projection, D14C leg lighting, D36G-K ownership/arms/legs/actions
and D36I1 mirror continuity are accepted. The D36 audit remains the regression
contract. New torso presentation must not move gameplay joints, the eye, aim,
collision or foot contact, and must not leak into reflections or Vanilla.

## Published production practice

Research accessed 2026-10-28. These are documented techniques and design lessons,
not a claim that the named games are flawless or that their proprietary source
was inspected. No reference game was run for this job.

- DICE's [Mirror's Edge production session](https://www.gdcvault.com/play/1012172)
  describes full-body first-person movement and an iterative animation pipeline.
  Its [Catalyst explanation](https://www.ea.com/en-gb/amp/news/the-world-from-faiths-perspective)
  explicitly describes checking captured movement from the head camera so actions
  read clearly. Applied here: judge the rendered first-person silhouette and
  motion, not the unchanged third-person model in an external viewer.
- Bungie's [Destiny animator discussion](https://www.bungie.net/7/en/News/article/12707/undefined)
  emphasizes responsive physical action and carefully authored camera motion.
  It is evidence about animation priorities, not a disclosure of Destiny's leg
  renderer. Applied here: retain responsive controls and the accepted stable eye;
  adding the torso must not reintroduce native animated head motion.
- Epic's [first-person rendering documentation](https://dev.epicgames.com/documentation/unreal-engine/first-person-rendering)
  separates custom-FOV first-person geometry from its world representation and
  discusses retaining world-connected feet. Applied here: preserve D14B weapon
  framing and world-space legs, while reflections retain the native complete
  actor. Copying Unreal's renderer is unnecessary for this PS1 title.
- Lena Piquet's [implemented full-body camera](https://www.froyok.fr/blog/2018-06-true-first-person-camera-in-unreal-engine-4/)
  documents additive pose control, update ordering and camera stabilization.
  It illustrates Riddick, Syndicate and Mirror's Edge as references, but does
  not establish their internal implementations. Applied here: presentation
  derives from the current draw state, including replay, without a delayed
  simulation latch or a second independently ticking skeleton.

The inference for this title is a hybrid: accepted camera-relative combat arms,
native contact arms and lower-body motion, plus a narrowly owned torso pose.
A literal camera placed over untouched retail geometry is not enough. A new
mesh would also require costume/style/texture work without fixing visibility
ownership by itself. Reuse the native costume surfaces first.

## Source diagnosis and iterations

S: the authenticated Duke model has 19 joints and semantic part mappings. Part 0
is the pelvis, part 9 the torso, 1-8 the legs and 21 the thigh attachment. The
accepted ordinary-body bridge admitted 1-8/21 but excluded 0/9. Native whole-joint
rejection happens before the polygon clipper and depends on projection H.
The owned-disc D36 census includes pelvis/torso meshes in all ten groups.

L: `baseline` reproduces exposed thigh ends at the current private slot-4 street
scene. `admission` admits 0/9 unchanged: the chest fills roughly half the downward
view and obscures the legs. This rejects a visibility-only solution.

L: `lean25` rotates the torso backward at its native waist origin. Standing
improves, but crouch brings the chest across the eye and near plane. This rejects
a single fixed lean as the complete solution. `fit` instead derives upright
heading from the pelvis and increases clearance with the actual eye-to-waist
height. Standing and crouched captures at 60/default/80/90 now show a connected
vest/belt/leg composition. Movement and broader verification follow below.

These captures are sequential, not matched animation-phase images. The private
runs use Xvfb/OpenGL and dummy audio; they do not establish sound quality or
sustained performance from the requested presentation rate alone.


## Implementation contract

- Ground, Committed and Fall body draws admit semantic pelvis 0 and torso 9
  alongside accepted legs/accessory 1-8/21. Head hiding, kick ownership and all
  existing action eligibility gates remain authoritative. Flight/swim/traversal
  and interaction torso policies are not expanded by this locomotion change.
- The torso keeps its native waist translation. A normalized horizontal pelvis
  heading supplies its basis; render-only recline varies from 35 to 60 degrees
  as vertical eye-to-waist clearance falls from 282 to 212 native units. These
  are in-game composition calibrations, not real-world anatomical dimensions.
  The native torso tilt is replaced only in this draw. Native matrices remain
  byte-identical; feet, gameplay attachments, hit queries and camera are untouched.
- The same pose is used at every FOV. Torso, pelvis and legs keep world projection,
  near-plane polygon clipping and world depth. D14B display arms/weapons retain
  their existing fixed projection and ownership. The fit adds no FOV branch or
  saved preference. Invalid root mapping or a near-vertical root leaves the
  native torso pose; unusual acrobatic poses require separate visual coverage.
- Reuse the existing reserved 32-byte arm scratch. Every draw fully overwrites it
  before use, including replay workers. No allocation, serialized state, render
  state field, second skeleton, mesh asset, new guest hook or framework change.
  Reflection/third-person/Vanilla paths retain their own representation.
- L/S: admitting the pelvis exposed its native fade initialization after the
  transform hook. `pelvis` traces show back colours 544/512/512/496 at
  H=277/256/191/160, with subtractive command 0x02000000. The guarded mesh-entry
  correction now catches this late root fade. `pelvisfixed` shows 2640 in standing
  and 2368 in crouched samples at all four H values, with command 0. Actor,
  semantic root, mesh pointer, caller, stack-camera and effect guards constrain
  the correction. Existing context/GTE restoration retires it at the next joint.

The waist hinge is a deliberate first-person composition, not an animation or
asset replacement for third person. It preserves native costume material and
texel density. The [accepted art guide](265-native-mesh-art-style.md) was reviewed;
no new material or geometry authoring is needed for this candidate.

## Verification and limits

S: player build and controls/input/aim/near suites pass, including owned LEVEL00,
LEVEL01 and all-level controls fixtures. Added checks cover both costume joint
orders, pelvis/torso admission, fixed waist translation, unchanged native matrix
arrays, monotonic clearance fit, FOV-independent output (H 277/256/191/160 and a
synthetic 128), native world depth, root fade/effect/caller/mesh/stack guards and
context/GTE restoration. Existing suites retain Q hit-count/arguments, native
door behavior, view/load/replay policies, reflections, third person and Vanilla.
Synthetic H=128 is not a player-supported or visually accepted FOV extension.

L: `coverage120` contains 113 samples at a 120 presentation setting and 21:9:
60/default/80/90; movement, crouch, jump preparation/air/recovery, look extremes,
near-wall/floor views, seven additional copied slots, private save/load, physical
P first/third/first and Escape pause/resume. Sampled lower animation IDs are
63, 72, 74, 76, 78, 178, 96, 97 and 105. Actual mode/pose fields accompany each
capture. This run precedes the final pelvis-only light correction; torso geometry
is final. It completes 3,184 replay sessions and 3,795 worker jobs, zero failed
sessions/jobs/plugin failures. Packet peak is 139,680 of 139,744 bytes across
these scenes, host geometry peak 49,176 bytes, with zero packet skips, copy
overflows or world/object budget hits. This nearly full arena is not evidence
of generous remaining headroom. No new geometry allocation is introduced.

L: final `pelvisfixed` covers standing/crouch and Q at 16:9 with all four FOVs;
`final43` repeats the same matrix at 4:3. `motion60` retains a 30-FPS X11 recording
and 18 movement/turn/look/jump/Q samples at 90. Reviewed one-second video samples
and individual captures show the torso remaining below the play view. Equipment
state confirms empty hands (equipment 0), crossbow (record 8) and dynamite
(record 12); the labelled holster attempt retained the pistol and is excluded
from holster coverage. `handoffs` separately verifies physical-key crossbow and
dynamite selections, and a vending action (lower 277, equipment 0, Use policy).
Other attempted use routes did not reach their action and are not counted.
`equipment120` confirms final-build crouched empty hands and return to pistol
at 90. Its four misleadingly named `unarmed` captures remain armed crouch
samples: the private C binding invoked crouch. Requested group 5/6 switches in
that crouched run also remained pistol and are excluded from weapon coverage. These recordings/captures and requested presentation settings are
not a sustained-120-FPS or complete motion acceptance claim.

Harness corrections: the initial `motion120` enabled expensive COW diagnostics
and its short input holds did not reach the intended downward look. It was
stopped and is excluded from visual coverage; `coverage120` uses stronger look
input and ordinary replay diagnostics. An aim-suite invocation omitted its
required EXE argument and was rerun correctly. The new root-fade fixture initially
inherited a previous fade before simulating native initialization; resetting the
fixture's entry context exercises the intended late-fade event and passes.
These are recorded harness/test setup failures, not successful coverage.

## Remaining review boundary

D14E is a locomotion-body candidate, not a certification of every campaign frame
or a completed D36L combined acceptance pack. All ten costumes have the existing
source mapping evidence; runtime samples do not certify all ten. Every stair,
acrobatic root pose, action interruption, lighting/power-up combination, software
renderer, live aspect resize and 180/240/Unlimited presentation remain unverified.
Native tests protect intentional effects; that is distinct from visual playtests.
Existing black ground-edge triangles also appear in the retained pre-change
crouched baseline and are not claimed repaired by this body job.

Full-body traversal/interaction retargeting, shoulder-to-torso blending at future
wider FOV, new default FOV and extending the supported slider/console range are
separate decisions. Their accepted current limb paths remain intact. Human motion
and framing acceptance remains necessary before calling this release-ready.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Use `fov 80`, `fov 90`, `fov 60` and `fov default` in the backtick console. Look
down while moving, crouching, jumping and quick-kicking; compare with empty
hands and longer weapons. P checks third/first-person restoration. FOV remains
session-only and the accepted default is unchanged.


## Candidate handoff

Final executable SHA-256:
`fd375ac91717866c4907543e6d64e77fee12c5f550e17d8a794021358a175d6c`.
Source intake commit: `d554c0b71add91c9e189bc8c1f5534d32fcf1b13`.
Existing framework/UI pins and local framework modifications are preserved;
this job does not author framework changes. No generated C or retail assets edited.

The final-build 120-setting equipment run completes 657 replay sessions
and 828 worker jobs with zero failed sessions/jobs/plugin failures.
All completed harnesses assert player save/card/settings SHA-256 and mtime
preservation; writes use private copies. The failed initial diagnostic also
runs its cleanup/integrity check. Test processes have exited. Build, native
suites, whitespace and repository/privacy checks pass. No acceptance, commit
or push performed. `before-after.png` is a labelled standing/crouch comparison
under the local evidence directory; retail-derived screenshots remain untracked.


## Acceptance and closeout - 2026-10-28

User: "acepted - lock it in and close it out". D14E is Done by user acceptance.
The connected locomotion body, clearance-based torso pose and scoped pelvis
lighting correction are accepted. Implementation, tests, manual and documentation
commit/push closeout are explicitly authorized as MusicMonsterMod.

The preceding candidate observations and coverage limits remain historical
verification evidence. Acceptance does not certify every costume/action, change
the default FOV or extend the supported range. No further gameplay change,
rebuild or game launch is needed for closeout. Player files remain untouched;
no next job started. Launch: `python3 recomp/tools/local/run.py`.
