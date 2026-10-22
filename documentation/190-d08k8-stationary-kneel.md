# D08K8 - Stationary one-knee crouch

Status: Done (user-accepted 2026-10-22).

User: "this is perfect!! lock it in, close it out". The user accepted the
player-facing result and authorized documentation, commit and push closeout.

## Implementation

- `ttk_crouch_walk.py`: host idle clips 5/15 now hold the original animation
  178 pose, with separate lower and upper joint sets and a 282-unit pelvis.
  The four moving clips are unchanged.
- `crouch_walk.inc`: transitions start from the last displayed joint pose
  and pelvis height, with a six-update smoothstep blend. Restarting during
  a transition captures the intermediate pose. Movement starts immediately;
  movement speed, collision probes and stand-up rules are unchanged.
- Stationary firing uses the original crouched aim pose. The moving firing
  hunch and shoulder compensation ramp together between zero and 50 degrees,
  preserving their opposing rotations. Channel-mask sanitation remains.
- `anim_override.cpp`: a loaded savestate's host animation tables are restored
  and rebuilt from the current pack. The first lab run exposed old embedded
  idle clips being adopted despite the new pack being loaded from disk.

## Evidence

Executable: `d798757cb2de56f946074165423faea379a1711d51f81b60cf00c70d8780e1f3`.

Private lab: `recomp/analysis/d08k8-lab/`, port 9408, copied test cards and
private settings, offscreen OpenGL. No player saves were written.

- `kneel2`: third person, 60 Hz, slot 10 dumpster; held idle stays on the
  kneel frame and at pelvis height 282, forward/backward movement, stops,
  held fire, release to standing 63, then ordinary walking 72. Captures
  inspected against CROUCH ORIGINAL's kneel.
- `rapid`: 120 Hz, short forward/left/right inputs with stops between them;
  crouch remains 178 and release stands. This is a functional and captured
  pose check, not user approval of the transition's look. The attempted
  scripted holster input was not independently verified, so unarmed gameplay
  coverage was not independently established in the lab; the subsequent user
  acceptance closes this job.
- `original`: CROUCH ORIGINAL still enters the original roll 181 on Ctrl+W.
- Animation Python suite: 15 tests pass, including original kneel quaternion
  matching, separate joint sets and encoded root-command validation.
- Native input, weapon aiming and controls suites pass (controls with the
  D07 final-camera LEVEL00 fixture and D22A LEVEL01 overlay). An initial
  controls invocation used a nonexistent LEVEL01 path and was rerun with the
  correct fixture. Vanilla guard checks pass; no full Vanilla route was run.
- Player build succeeds. No framework source changed in this job.

## Player review

CROUCH MODERN, third person: hold Ctrl while still, start/stop in every
movement direction, rapidly interrupt those transitions, release to stand,
and fire while still and moving. Try armed and holstered. Judge the kneel
and the transition in motion. D08K5's game-wide coverage remains separate.

## Acceptance

User playtest accepted the result on 2026-10-22. The executable above is the
new accepted regression baseline. D08K9, D08K10 and D08K5 remain separate jobs.
