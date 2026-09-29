# Modern controls and first-person mode: research design

## Product direction confirmed by the user

Modern shooter controls are a core goal of this Time to Kill project. This
means changing movement, camera and aiming behavior, not simply remapping
PlayStation buttons. A modern third-person control system can come first;
first-person play can build on the same independent movement/aim foundation.
The immediate milestone remains faithful original gameplay and audio.

The intended foundation is continuous mouse/right-stick camera yaw and pitch,
independent player movement and aim, responsive directional movement, and
weapon targeting consistent with the view. Preserve the game's collision,
traversal, combat rules and progression while investigating the changes needed
for that feel. Cover walking, strafing, backwards movement, crouching, jumping,
swimming and ledge interactions explicitly; adapting the old turn buttons is
not the acceptance criterion.

Use a modern third-person prototype to establish camera collision, aiming and
movement, then add a stable first-person view with appropriate body/weapon
visibility. Keep an original-behavior mode for comparison. Precise sensitivity,
acceleration, reticle and camera preferences can be tuned with the user once
the underlying game state and control routines are understood.

First-person mode is a later, optional gameplay feature. It is deliberately not enabled in the initial recomp profile. The original camera remains the compatibility baseline.

## Why camera position is insufficient

A third-person engine can use camera-relative movement, room/portal visibility, animation-derived weapon directions, auto-aim, collision-adjusted view placement, and fixed scripted shots. Moving the camera into Duke's head can reveal culled geometry, clip the player model, leave shots aimed away from the reticle, and produce motion sickness from skeletal animation.

The runtime still projects original geometry through PS1-era transforms. A host-side free camera cannot automatically correct the game's room visibility, actor updates, projectile simulation, or scripted state.

## Research targets

1. Player world position, facing, animation state, and collision body.
2. Camera position/orientation and the room identifier used for visibility.
3. View/projection matrix production and any near-plane clipping rules.
4. Input read, camera-relative movement conversion, and look behavior.
5. Weapon aim vector, auto-aim selection, muzzle origin, and projectile spawn.
6. Player-model draw submission and attachment transforms.
7. Scripted camera transitions, deaths, cutscenes, and menu state.

The camera diagnostic at `0x800137A0` and its candidate references are initial discovery anchors. Do not patch those addresses until their enclosing routines and callers are understood.

## Proposed first prototype

Use the verified player transform for a stable eye position. Separate aim yaw/pitch from animation pose; initially retain original physics and collision. Add a developer toggle to compare original and experimental camera behavior in the same route. Keep the view stable while walking and turning before adding a weapon presentation.

The hook contract should be game-owned and version-guarded: validated original image, active overlay identity if relevant, known original instruction bytes, and named semantics. Use native patch points supported by the pinned or explicitly upgraded runtime; do not modify generated C by hand.

Avoid committing to exact struct layouts or hook signatures until disassembly and runtime observations support them. The first implementation should expose only the verified fields it needs.

## Decisions deferred until evidence exists

- The implementation of independent mouse aim; adapting existing turn inputs alone does not meet the user's goal.
- Reticle policy and auto-aim behavior.
- Hide whole player body versus selectively hide head/upper body.
- Original weapon geometry versus new first-person weapon assets.
- Camera handling during ledge traversal, swimming, death, and scripted sequences.
- Whether original room culling can be adapted or needs a larger rendering change.

These are future product decisions, not blockers for the current native baseline.

## Acceptance

With the feature off, behavior must match the baseline. With it on, controls must be coherent, shots follow intended aim, visible geometry does not disappear at room boundaries, the player body does not obstruct the view, camera placement respects nearby surfaces, and scripted transitions return to a valid state. Test in multiple level/overlay families rather than only the starting area.


## 2026-09-26 research update

See [D03 live state research](18-player-camera-research.md) for guarded,
input-correlated player/camera addresses and remaining hook boundaries.
The `0x800137A0` anchor above is a string address, not a function entry.

## 2026-09-29 D11 prototype

The first playable eye-level view, its verified hooks, evidence and limits are
in [D11 first person](61-d11-first-person.md). Research questions answered
there: head visibility (joint skip bit during Duke's draw), body visibility
(original joint near cull), room culling (camera room follows the eye; no
missing rooms on the first-map route) and near-plane clipping (partly solved;
steep wall contact remains, job D11B).
