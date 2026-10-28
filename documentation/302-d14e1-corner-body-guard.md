# D14E1 - Reject external views of the incomplete first-person body

2026-10-28. Done, user-accepted. Implementation and documentation commit/push
closeout explicitly authorized as MusicMonsterMod. The selected correction
addresses slot 7 exposing a headless/armless torso in a corner.
Evidence labels follow D36: S source
and native tests, L private runtime observations, A prior acceptance, U report.

## Reproduction and cause

L: a private copy of current UI slot 7 reproduces the reported torso at default,
80 and 90 FOV. The saved corner places the camera at (-2771,-10191,18927) and
Duke's actor root at (-2823,-9727,19111): approximately 191 units of horizontal
displacement and 464 units of vertical clearance. These are game coordinates.
The normal slot-4 standing comparison has about 3 horizontal units and 286
vertical units; crouching has about 3 and 224 respectively. Animated matrix
samples vary slightly from actor-root samples.

S/L: the native collision camera can resolve above and beside the actor at
this corner. D14E only adjusted torso recline using vertical eye clearance;
it did not reject a camera that was outside the body. Thus head hiding and
camera-relative arms left the native torso visible as an incomplete external
model. The saved scene and backing out/re-entering reproduce the relationship.
This is a presentation safety omission in D14E, not a missing retail head mesh.

## Correction

Before admitting ordinary body geometry, every authenticated primary first-person
joint draw checks the actual camera against the current semantic pelvis matrix.
The admitted volume has a horizontal radius of 96 native units and vertical
clearance 128-352. These conservative presentation bounds accommodate the
accepted normal eye, crouch and bounded bob while excluding the reported corner.
The decision has no FOV, level, costume-index or save-slot special case.

Outside that volume, semantic body parts 0-9, backpack 20 and thigh attachment 21
receive HiddenBody ownership. Unarmed locomotion arms are also suppressed so
that a detached world-space arm chain cannot replace the exposed torso. Contact
arms keep their existing action ownership. Camera-relative weapon arms and the
separately owned Q/native-door kick remain on their established paths.

HiddenBody uses the camera matrix as the joint's temporary draw transform. Its
origin is therefore at the eye, giving native whole-joint rejection depth zero;
no near-depth bridge or host polygon takeover is granted. This requires no
allocation and cannot fail open because scratch memory was unavailable. Native
skeleton matrices, head flags, collision, camera position and gameplay events
are not edited. The joint scope retires at the next transform/draw boundary.

The rule reads current draw matrices, including interpolated replay matrices,
and recomputes on each draw. There is no visibility latch, saved state or worker
snapshot field. Returning inside the admitted volume immediately restores the
accepted D14E body path. Third-person and mirror ownership do not enter the new
rule; Vanilla retains its existing gates. No framework, asset, generated C,
additional hook or default/FOV-range change is involved.

## Verification

S: final player build and controls/input/aim/near suites pass. Controls exercise
owned LEVEL00/LEVEL01 and all-level fixtures. New tests cover both costume maps,
standing/crouched safe eyes, displacement in each horizontal direction, excessive
height and insufficient height, the reported corner delta, armed/unarmed parts,
H=277/256/191/160 plus synthetic H=128, disabled depth bridging, unchanged native
matrices and scope restoration. Existing tests retain kick hit-count/arguments,
contact arms, reflection, third-person, Vanilla and load/replay contracts.

The older synthetic skeleton fixtures left the pelvis at the world origin while
placing the eye elsewhere. Their initial failure under this new position rule
was corrected by locating the fixture pelvis under its synthetic eye/waist;
no production guard was weakened to accommodate impossible fixture geometry.

L: private evidence lives under `recomp/analysis/d14e1/`:

- `baseline`: reported slot 7 at default/80/90 and look/back-out/re-entry samples.
- `candidate`: initial guard with the same route; unsafe body disappears and
  the connected body returns after backing out. This first experiment used
  scratch; the final code uses the allocation-free camera-matrix rejection.
- `final120`: final code, 16:9 and 120 presentation setting, the same saved scene
  and route. 417 replay sessions / 459 worker jobs, zero failures.
- `wide120`: 21:9, 60/default/80/90, corner yaw/pitch extremes and Q, back-out,
  crouch, re-entry and ordinary slot-4 downward-body controls. Sixty route samples
  plus private save/load and four physical P/Escape transitions. First -> third
  -> first is observed with projection 160 -> 386 -> 160. This run completes
  2,269 replay sessions / 2,594 worker jobs, zero session/worker/plugin failures.
- `aspect43`: final code at 4:3/60 presentation setting, reported saved view at
  default/80/90 and the look/back-out/re-entry route.
- `comparison.png` and `coverage-sheet.png`: retained before/after and sampled
  route views. Ground shadow remains visible in the corner; it is not torso
  geometry. Screenshots and retail-derived material remain local/untracked.

All completed harnesses assert SHA-256 and mtime preservation for all 41 player
save/card/settings files. Writable state uses private copies. Diagnostic games
are stopped. Sequential captures are not atomic camera/pose measurements; the
replay setting is not a sustained-120-FPS benchmark. Sparse captures and native
contracts do not establish exhaustive campaign or every future custom-model
coverage. The external-body rejection is a persistent safety rule, not a claim
that every possible first-person visual issue has been certified impossible.

Final executable SHA-256:
`689a315f328b4af50586db921d0f05738c79595ed4216f66f8704a2eb94df399`.

## Player check

```sh
python3 recomp/tools/local/run.py
```

Load slot 7, look around and down, back out, then walk into the corner again.
Try `fov 60`, `fov default`, `fov 80` and `fov 90`. The incomplete body should
not be presented when the obstruction camera moves outside Duke; the ordinary
connected downward body returns on leaving the corner. Check P both ways and Q.


## Acceptance and closeout - 2026-10-28

User: "lock it in and close it out". D14E1 is Done by user acceptance.
The displaced-eye body guard is accepted; implementation, regression tests,
manual and documentation commit/push closeout are explicitly authorized as
MusicMonsterMod. Prior build, native-suite and bounded runtime evidence remain
with the coverage limits above. Acceptance does not establish exhaustive
campaign coverage or change the supported FOV range.

No further gameplay change, rebuild or game launch during closeout. Player
saves/settings remain untouched. No next job started.
Launch: `python3 recomp/tools/local/run.py`.
