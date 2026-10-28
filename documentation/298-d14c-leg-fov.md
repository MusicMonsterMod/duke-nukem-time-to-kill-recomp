# D14C - First-person leg presentation across FOV

2026-10-28. Done, user-accepted; implementation and documentation commit/push
closeout authorized as MusicMonsterMod.

Evidence labels follow [D36](257-d36-first-person-presentation-audit.md):
S = source/native tests, L = private runtime observation, A = prior acceptance,
U/H = user report or unresolved hypothesis.

## Findings

U: after accepting D14B's weapon/arms, the user reported an uncertain alteration
in the leg view. The intake did not identify ordinary legs, Q or the door kick.

L: current UI slot 4 is the LEVEL00 street facing Bootylicious. Its private
source save has SHA-256
`df0a976a2c8503389877880ff8427d925c6109e48ef477560c8a0bbf7ff5eeab`.
Standing at the same position and maximum downward pitch, FOV 90 shows more
of the legs and moves their projected outline toward the image centre. It also
made the jeans substantially darker than at default or 60.

S/L: the framing change follows normal projection. Ordinary legs retain native
matrices and world depth. H is 277 at 60, 256 at default and 160 at 90, so
screen displacement from the projection centre scales by 1.08203125, 1 and
0.625. It does not imply a changed eye position or a changed mesh shape.
The candidate's filtered native mesh trace contains 12 exactly repeated left-thigh
poses across all three settings. In the retained example, all 22 camera-space
vertices match exactly, and background colour is [2640,2640,2640] in all three.
This is matched geometry/lighting evidence, complementary to the screenshots;
the screenshots themselves are sequential and not exact animation-phase matches.

S/L: the excessive darkening is a confirmed defect. The native root can pass
its whole-joint H cutoff at wide FOV and enable close-camera fade at
`0x80034e18`. The existing limb correction clears the fade's command/tpage,
flat colour and GTE back colour, but the native loop retains faded brightness
in s2. Later per-joint shading at `0x8003505c` reuses that register and overwrites
the correction. The body therefore becomes opaque but incorrectly dark.
Before correction, standing traces show background channels 2640 at 60/default
but 384 at 90, despite the same actor environment brightness of 165.
Crouched samples similarly show 2368 versus 512. After correction, the channels
are 2640 for standing and 2368 for crouching at all three FOVs in this scene.

S/A: Q still uses the accepted plain left leg, with its camera-relative pose
and viewmodel depth. Native door 116 separately uses the right leg and thigh
attachment. Both retain world projection; neither is selected by D14B's fixed
weapon/arm projection. Fixed-phase Q captures at frame 13 show its expected
FOV-dependent framing. There is no evidence here requiring fixed-FOV legs,
new geometry, a changed kick anchor, or a moved/restricted camera.

H: the lighting defect is independently reproduced and the user accepts the
result. Acceptance does not establish that it explains every aspect of the
original uncertain report. Preferences about how much native thigh/body is
visible are not diagnosed as deformation.

## Bounded correction

`first_person.inc` now supplies the actor's environment brightness to native
s2 only while correcting the recognized proximity fade in an authenticated
primary-Duke joint draw. The actor, camera, caller, stack and saved camera
pointer must match. The native per-joint shader still owns its tint decisions.
The old s2 value restores before the next joint in that same actor stack frame;
unrelated transforms and attached meshes do not have their registers rewritten.
Actor boundaries, load/world reset and replay preparation discard this draw-local
record. The native actor epilogue restores its caller's callee-saved register.

The existing transparency exclusions remain. Correction also applies to an
already eligible display or contact limb when it encounters the same fade;
this avoids leaving shared shading inconsistent with the body. The complete
native actor routine is already covered by the existing executable identity guard.

No projection, eye placement, pose, mesh, gameplay matrix, near plane, joint
admission rule, world-depth ordering, kick damage/reach/timing or weapon framing
changes. No new guest allocation, save format, hook address, generated C change,
framework/UI edit or dependency-pin change. Existing framework edits are preserved.

## Verification and limits

S: player build and controls/input/aim/near-clipping suites pass. Controls use
owned LEVEL00/LEVEL01 and all-level fixtures. The added test follows brightness
through the transform hook and subsequent native-shader input, checks restoration
at an unowned torso joint, and verifies an unrelated callee's s2 is untouched.
Existing tests cover intentional transparency flags, costume joint ordering,
clipping, actor/caller/code/overlay guards, load/replay, Vanilla, aim, the exact
Q hit-call count/arguments and separate native-door no-extra-attack behavior.

L: scripts, captures, RAM observations and traces remain local under
`recomp/analysis/d14c/`:

- `initial` and `shorttrace`: baseline 60/default/90 standing/crouched and Q
  observations. The first longer unfiltered trace hit its record cap; the
  bounded `shorttrace` pass supplies all three settings.
- `candidate` and `matched`: corrected 16:9 standing/crouched samples and
  filtered repeated-pose trace. `matched-pose.json` retains the exact common
  vertices; `matched-summary.json` records the projection and light comparison.
- `coverage120`: 86 samples at a 120 presentation setting and 21:9, with all
  three FOVs, walk/run, crouch/crouch movement, jump preparation/air/landing,
  pitch sweep, active Q/recovery, nearby wall/floor and seven additional copied
  saves. Two distinct costume part orderings are observed. Actual sampled
  animations include 63, 72/74/76/78, 178, 96/97 and 105. These are sparse
  motion samples, not every animation frame or a full campaign certification.
- Physical P keys verify third-person projection 386 and return to first-person
  160; Escape pause/resume and a private save/load preserve the FOV selection.
  The run completes 2949 replays, 3240 worker jobs, zero failed sessions/jobs
  and zero plugin failures. Diagnostic capture causes pacing reductions; this
  is restoration/stability evidence, not a sustained-120-FPS benchmark.
- `door60`: private historical door fixture reaches native animation 116 and
  returns to 63 at every FOV. Right-leg draws advance; Q stays inactive and
  its damage-hit counter stays zero. Door timing remains native.
- `aspect1610` and `aspectauto`: static standing and Q samples at all three
  FOVs in 16:10 and the current automatic aspect. No live resize was performed.
- `frozen43`: standing/crouched and Q at 4:3. The existing diagnostic-only kick
  freeze holds Q frame 13 across 60/default/90; normal timing is covered by
  the other runs and native suite. An initial launch used invalid aspect flag
  `4:3` and was rejected before gameplay; the corrected launcher value is `off`.

All diagnostic game/Xvfb processes have exited.
Each completed harness checks SHA-256 and mtime preservation of player saves,
cards and profile. All writable test state uses private copies. Sequential
RAM/image reads are not atomic captures. The matched vertex trace specifically
removes the idle-phase ambiguity for the sampled thigh; it does not certify
every body part or every action at an identical phase.

User visual acceptance is recorded below. Every
costume/action/lighting/power-up combination, continuous motion, software
rendering and live adaptive resizing are not newly certified. Native world
occlusion and floor contact retain their original rendering path; sampled
near-wall/floor scenes do not establish all portal/contact angles. The accepted
D36J plain-left-Q and D14B weapon framing remain the reference.

Baseline executable:
`bc1329ece5c94fa033149d96527aa9c2e84525add340681b931fb8eb20b86f53`.
Candidate executable:
`24f3de2acb9c547b3305e1b91243e885b8be5a4b038539ff52f84bc6179b2794`.

## Playtest

From the workspace root:

```sh
python3 recomp/tools/local/run.py
```

In first person, compare `fov 60`, `fov default` and `fov 90` while looking down,
standing, crouching and moving. Check Q and a kickable door. The body should
retain natural world perspective without the reproduced wide-FOV darkening.
Weapon/arms keep D14B's accepted framing. FOV remains session-only.


## Acceptance and closeout - 2026-10-28

User: "lock it in and close it out!!" The investigation and bounded lighting
correction are accepted; D14C is Done. Authored implementation, regression tests,
manual, board and status/handoff updates are authorized for commit and push as
MusicMonsterMod. The preceding verification and coverage limits remain intact.
No additional gameplay change, rebuild, game launch or player-file write during
closeout. Existing framework modifications remain untouched. No next job started.
