# D36J3 - Quick-kick transitions and FOV framing

2026-10-28. Done. User accepted revision 3 and explicitly authorized lock-in
and closeout, including implementation/documentation commit and push. Earlier
candidate wording below is historical; its evidence limits remain.

Evidence labels follow [D36](257-d36-first-person-presentation-audit.md):
S source/native regression, L private runtime evidence, A prior acceptance,
U/H user report or hypothesis. The accepted plain left-leg Q and separate native
right-leg door kick from [D36J](261-d36j-body-and-kick.md) remain the baseline.
[D36J2](304-d36j2-no-body-default.md) supplies the ordinary no-body policy.

## Revision 3 - Q uses the weapon presentation FOV

2026-10-28. User rejected revision 2 because the kick still changed appearance
across FOV 60/75/80/90 and explicitly requested the weapons' treatment.
Revision 3 removes the adaptive hip translation entirely. The entry/recovery
lowering remains; Q now uses the existing H=256 display projection shared by
weapons and display arms. World FOV changes only the world view, not Q's pose,
size or screen framing. This supersedes revision 2's retraction behavior.

S: `LimbPresentation::fixed_projection()` admits Kick-owned semantic left-leg
parts 1-4. The native right-leg door owner (parts 5-8/21), contact arms and body
remain on world projection. The existing near-clip renderer consumes the policy
without modifying shared GTE registers, UVs, depth or gameplay matrices. No new
allocation, replay state, asset or saved setting is added. Damage, hit point,
reach, sounds, attack timing and native door pose remain unchanged.

S: player build and controls/input/aim/near suites pass. Actual-hook tests verify
unchanged Q pose across 60/default/75/80/90 in both costume mappings. The actual
mesh-packet regression checks every Q part at H=160/177/191/209/256/277 and three
aspect margins: emitted screen positions/words are identical, with unchanged
UV/depth metadata and GTE state. Native door parts still respond to world H.
This directly tests the requested FOV independence, not just a hip position.

L: `locked13` compares full extension at 60/75/80/90 with actual records 4 and 8;
`locked7` compares chambering at those four values with record 4. The inspected
`locked-fov-comparison.png` shows matching leg/weapon framing while the world
changes. `locked219` repeats 60/90 full extension at 21:9 and requested 120.
It completes 514 worker jobs with zero worker failures.

`locked-motion` uses the no-menu harness at requested 120: 60/pistol, 90/pistol
and 90/record 8, pitch sweeps, repeated Q, walk/run/crouch, airborne 96/97 and
landing 105. First -> third -> first and private save/load end in the expected
perspective with no active kick left behind. 1763 replay sessions / 2182 worker
jobs complete with zero session/worker/plugin failures. Packet peak is
139304/139744 bytes with zero packet skips/copy overflows: limited headroom,
not a campaign budget guarantee. No synthetic-menu failure is counted as a
pass; pause/resume and live door/Vanilla evidence below remain earlier checks.

The capped timing repeat measures 59.88-59.95 guest FPS and 16.683 ms average
frame time. Recording/diagnostic evidence does not establish sustained 120 FPS
or uncapped performance. All 41 player files retain hashes/mtimes. No framework
changes, commit or push. User visual acceptance and broader coverage remain
pending. The earlier evidence/limitations below remain labelled by revision.

Current executable:
`03bc00541d1db674ee9811c03b32042e6132b548dad2aa7fd14351c7ae881abe`.

## Revision 2 - Adaptive hip anchor (rejected)

2026-10-28. User reported that full extension at FOV 90 exposes the detached
hip end and requested progressive retraction as FOV increases. L: matched
`wide-before-{default,80,90}-key2.png` captures reproduce the exposed end at 90.
The first candidate only changed entry/recovery, leaving this interval untouched.

S: Q now keeps its animated hip's default screen anchor for projections wider
than the original H=256. Every left-leg segment receives the same camera-plane
translation: hip X/Y (including the existing entry/recovery drop) multiplied by
`max(0, 256/H - 1)`. Depth, rotations, segment lengths and native matrices stay
unchanged. The visible extension retracts toward the lower-left anchor as FOV
widens. Default and narrower views retain their prior framing. This is driven
by the existing render-snapshot-aware `eye_projection()`, so replay uses its
captured FOV rather than a later live setting. There is no new persistent state.
Native door presentation is explicitly excluded. Weapons, camera freedom,
hit point/reach, damage, attack timing and sound remain unchanged.

S: player build and controls/input/aim/near suites pass. The actual-hook test
now checks 60/default/75/80/90 across both costume maps: default strike position,
constant projected hip anchor at wider FOV, unchanged depth, entry/recovery
clearance and unchanged native matrices. Existing gameplay and door checks pass.

L: `wide-after` captures full extension at default/60/75/80/90 with actual
pistol record 4. Inspected FOV-90 output no longer exposes the hip end; the boot
is slightly lower/left. `wide219` covers 60/90 with records 4/8 at requested 120;
`wide43` covers FOV 90 at 60. The inspected ultrawide record-8 strike keeps the
hip outside the viewport. `wide-west` repeats the LEVEL01 cowboy route at 120.
Replay: `wide219` completes 754 worker jobs and `wide-west` 1388, with zero
session/worker/plugin failures.

`wide-motion120` records 80/pistol, 90/pistol and 90/record 8, with active Q,
pitch sweeps, walking/running/crouching and airborne poses 96/97/105. Consecutive
recorded frames were inspected for entry, extension and recovery. As before,
its final synthetic-menu assertion fails; preceding movement evidence is
retained, but it is not a new passing pause/resume test. The separate real-key
lifecycle evidence below belongs to revision 1. No input/lifecycle code changed.

The repeated capped cost sample (`wide-cost`) measures 59.89-59.95 guest FPS,
16.683 ms average total frame time and 14.918 ms emulator CPU average. As with
the earlier comparison, this does not isolate the small draw arithmetic cost or
establish uncapped performance. All 41 player files retain hashes and mtimes.
No framework changes, new assets, commit or push. User visual acceptance remains
pending, including the amount of visible retraction and broader combinations.

Revision-2 executable:
`1f9795ad2f453b1f6251b9e83be2d7ecf00dd01797058718ec34bececeb99063`.

The sections below retain revision-1 evidence and limits. Their unchanged
strike-framing statements are historical and superseded above for wider FOV.

## First candidate - entry/recovery only

L: the unchanged phase-0 Q pose already exposes a blue thigh patch at the lower
edge at FOV 90, although ordinary idle has no leg. Matched frozen-pose captures
are `recomp/analysis/d36j3/entry0-90-key2.png` and
`fixed-entry0-90-key2.png`. Default/60/80/90 comparisons are retained alongside
them. Consecutive recorded frames show the corresponding entry/recovery edge
transition; this is distinct from the accepted large chamber/extension pose.

S: `kick_leg_joint` previously admitted the recorded resting pose immediately
and removed it when Q finished. The candidate adds one shared camera-down
translation to all four displayed left-leg parts: 160 units at rest, smoothly
reduced to zero by chamber frame 7, zero through frame 21, then smoothly returned
to 160 by frame 27. This lets the complete chain enter and leave below the screen
without rotating, scaling, replacing or separately moving a limb segment.
The source retains the original phase interpolation and left-mesh retargeting.

Strike/hold framing and the weapon/arm transforms are unchanged. No fixed-FOV
kick projection, pitch restriction, new mesh/material, player setting or
serialized state is introduced. The existing draw scratch is reused. The native
door branch is excluded. Gameplay matrices, collision, crosshair hit point,
reach, damage, callbacks, attack/impact timing and sound routines are untouched.
Third person, reflections and Vanilla retain the existing guarded owners.

The review did not justify moving the accepted strike pose or forcing the boot
behind every weapon. Sampled bulky-weapon overlap remains on the left, with the
weapon identifiable; final judgment of the chamber size and motion belongs to
the user. This bounded repair does not claim to eliminate every possible
weapon overlap or certify all costume/effect combinations.

## Native verification

S: the added actual-hook regression fails against the original source at its
entry-clearance assertion and passes with the repair. It samples entry, strike
and recovery with both costume joint orders, checks the unchanged strike anchor,
and verifies that native joint matrices remain untouched. The origin clearance
assertion is not a substitute for the retail-mesh runtime captures.

Player executable builds. Controls (owned EXE, LEVEL00 guard fixture, LEVEL01
and all overlay fixtures), input, aim and near-clipping suites pass. Existing
checks retain 13 hit-sphere calls during frames 8-20, their exact arguments,
sound/impact behavior, native door ownership, cancellation and save/replay
contracts. No framework or generated C edits were made.

## Private runtime coverage

L: recipes, state sidecars, continuous recordings and frame sheets are local
under `recomp/analysis/d36j3/`. Xvfb/OpenGL, private settings/cards and dummy audio
were used. State reads and screenshots are sequential, not atomic. The 30-FPS
recordings and inspected consecutive-frame sheets support bounded motion review;
they do not replace human visual acceptance or certify every display frame.

- `motion60` is the unchanged baseline. `fixed60` and `fixed120` exercise the
  candidate. Default/60/80/90 use the pistol; FOV 90 additionally uses actual
  records 7, 8 and 11, plus equipment 0. Actual recorded poses include 63, 72,
  76, 78, 96, 97, 105, 176 and 178. Pose 176 is an observed native transition,
  not relabelled as crouch. Airborne Q is observed with pose 97. Held/repeated Q,
  pitch sweeps, recovery, private save/load and first/third/first are retained.
- `aspect43` uses widescreen `off` at 60; `aspect219` uses 21:9 at 120. Both
  sample FOV 60/pistol and FOV 90/record 8 with movement and Q. These are sampled
  combinations, not the full Cartesian product of FOV, aspect and equipment.
- `costumes120` visits challenge stages 23/24/25 with actual records 28/29/27.
  All three retain the first joint ordering; they do not establish cowboy
  coverage. Active Q is confirmed in state; some early presented captures
  still show the prior frame. Held-Q captures and video provide settled views.
- `west120` visits LEVEL01 with its verified cowboy part-to-joint table,
  pistol and live Q across look/recovery/crouch requests. The recording and
  inspected crouched frame retain the actual plain left leg. It completes
  7557 replay jobs with no failures; this is one costume route, not all ten.
- `recomp/analysis/d36j3-door/physical-door*` reuses the historical private
  hinged-door fixture with ordinary keyboard input. It reaches native 116
  (17 samples), retains first person, draws the separate right leg, returns to
  63 and makes zero Q hit/sound calls. The close door obscures much of the
  view; the inspected strike image is not full-motion visual certification.
- `vanilla` uses an isolated one-launch original profile: the inspected scene
  retains third-person Duke, Modernized is false and Q activates no host kick.
  This is an exclusion smoke check, not a new Vanilla campaign acceptance.
- The synthetic-input pause attempts stayed in the menu and are excluded from
  resume acceptance. `fixed60` and `aspect219` end with harness assertions;
  their preceding motion/images remain evidence, not fully passing scripts.
  Separate `keys60` and `keys120` disable synthetic input and use ordinary
  keyboard events: pause suspends gameplay, Return resumes, Q increments the
  attack count again, and P switches third/first successfully.
- `fixed120`: 4001 replay sessions and 4794 completed worker jobs, zero failed
  sessions/jobs/plugin failures. Packet peak 116488/139744 bytes, zero packet
  skips or copy overflows. Requested 120 is not sustained measured 120 FPS;
  adaptive pacing is active. `costumes120`: 2952 completed jobs, zero failures.

Initial `phase7` weapon-key attempts retained the pistol and are excluded from
multi-weapon coverage. Corrected `matched7` uses explicit selection actions and
records actual equipment IDs. Initial synthetic door input did not reach 116
and is excluded. Failed harness attempts are retained rather than counted as
passes. Pre-existing black ground-edge triangles occur in the baseline too and
are not claimed fixed by this work.

## Bounded timing and remaining limits

L: three 3-second held-Q windows on the same private FOV-90 route, at a 60 cap
and without video capture, measure 59.87-59.95 guest FPS before and 59.88-59.97
after. The final frame average is 16.683 ms in both. Reported emulator CPU
averages are 14.770 and 14.820 ms respectively; this small capped comparison
cannot isolate the arithmetic cost or establish uncapped/campaign performance.
No new allocations, skeleton submissions or persistent replay data are added.

All 41 player save/card/profile files retain hashes and mtimes in the harness
checks and the final comparison after the door run. Diagnostic processes have
exited. Natural damage interruptions, all ten costume models, every weapon/
upgrade/effect combination, software rendering and 180/240/Unlimited are not newly certified. Native
exclusion checks pass.
Dummy audio supplies no listening-quality evidence. User motion/appearance
acceptance remains pending.

Baseline executable:
`134c98ee73f2224768cda39d36e3c43b94113c4154fde5524fe8e0df8fbb68c1`.
Candidate executable:
`8088fb953881a09956e4cf60065267c8040dd1e3f32abd52cd803f2fa1286a77`.

## Playtest

Check Q start/recovery and repeated Q, especially at FOV 80/90 while looking
down, jumping and holding a bulky weapon. The full kick and weapon framing
should feel familiar; the resting thigh should no longer pop at the bottom
edge. Check the native hinged-door kick separately. No settings change is
required to use this build. Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```


## Acceptance and closeout - 2026-10-28

User: "thats it! lock it in and close it out". Revision 3 is accepted: Q shares
the weapon display projection across world FOV, with smooth entry/recovery.
Revision 2's adaptive translation remains rejected and removed. D36J3 is Done
for this delivered scope; acceptance does not certify the unrun combinations
listed above. No subsequent gameplay change, rebuild, launch or player-file
write is needed. Existing build/native/runtime evidence remains authoritative.

Authored implementation, native tests, manual and documentation are included
in the authorized MusicMonsterMod commit/push. The pre-existing framework delta
is re-exported and verified against its pinned clean dependency. Repository
privacy and diff checks pass. No next job selected.
