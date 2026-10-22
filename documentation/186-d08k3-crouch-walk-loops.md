# D08K3 - Crouch-walk animation loops

Job: [D08K3](../MODERNIZATION_JOBS.md) (split from D08K, 2026-10-22). Status:
Done (user-accepted 2026-10-22; the look continues in D08K6). Depends on D08K1.

## Starting point

- Every crouch pose in the original (176 transition, 177, 178 idle, 179 low
  pose, 180) is the same **one-knee kneel**; there is no squat to walk from,
  and directional crouch input rolls (181-184). A crouch walk is a new gait.
- The original walks are half cycles of 4 keyframes, one step each: forward
  72 (legs clip 4, footstep event 4) then 74 (clip 8, event 2), backward 82 then
  83; strafes 90 / 91 are whole 6-keyframe cycles (about 785 units sideways).
  Event bit 1 ends a clip. Measured with the same metric as below, their
  planted feet slide 23-40 units a step on average (worst 51-99).
- The legs channel (channel 0) owns ROT joints 0-8 and 21; arms and torso are
  other channels, so aiming and firing stay on their own tracks.

A first attempt layered the walks' joint motion onto the kneel. It failed: with
the thighs pointing forward the walk's sideways hip swing becomes a twist,
forward travel shrank to 117 per cycle and the strafes went diagonal.

## The gait (`recomp/tools/local/ttk_crouch_walk.py`)

Two-bone inverse kinematics on Duke's own skeleton (hierarchy 1, read from the
disc):

- leg chain per side: hip (ROT 1 / 5) -> knee (ROT 2 / 6, a pure hinge about
  local x, as in every original walk) -> ankle (ROT 3 / 7) -> toe (ROT 4 / 8);
  thigh 227, shin 258, the idle ankle 36 over the floor;
- pelvis 330 over the floor (root rotation identity, as in the kneel), a 8-unit
  bob twice a cycle;
- each foot: 60% stance (planted, sliding back under the body by exactly the
  body's travel), 40% swing (lifts 55 and steps ahead), the legs half a cycle
  apart; stance width 1.15 x the idle's;
- the knee hinge is solved for the hip-ankle distance, the hip turns the leg
  onto the ankle target with the knee pointing forward, and the ankle keeps the
  standing idle's flat, forward foot (also when strafing);
- 8 keyframes a cycle, 39 ticks each (step 150, about 200 units/s, the cycle
  travels 250); root travel per keyframe equals the planted foot's slide;
- footstep events at each touchdown (4 first leg, 2 second), 1 on the clip's
  last frame; forward and backward are split into two half-cycle clips like the
  originals (each with an absolute first root height).

The output is derived from data on the owned disc and is generated locally
(`ttk_crouch_walk.py OUT_DIR`), never committed. D08K4 will build its pack the
same way at build time.

## Evidence

Private outputs under `recomp/analysis/d08k3-gait/` and lab
`recomp/analysis/d08k3-lab/` (port 9404, private cards/savestates).

| Check | Result |
| --- | --- |
| Planted-foot slide (keyframes + 3 slerp steps between) | forward / backward max 1.6, mean 0.2; strafes max 1.7, mean 0.1 (originals 23-40 mean) |
| Planted foot over the floor | 2.5-4.8 units at every sample |
| Highest vertex | 647 with the crouch idle's upper body, 654 with the walk's arms; the D08K2 hull became 0x2c0 (704) |
| Renders, 8 keyframes, side and back (`v2-strip.png`) | low duck-walk, knees forward, alternating steps; strafe steps sideways facing forward |
| In game: preview pack replacing the walk legs clips (72, 74, 82, 83, 90, 91) | walking (W) plays the crouch walk in both halves, pelvis held low, feet stepping (`d08k3-lab/shots/prev3t-sheet.png`); first person keeps the eye at crouch height |
| Blender | `v3/duke-crouch-walk.blend`: rigged, textured Duke with the six clips and the crouch actions, for review or hand edits (re-import with `ttk_anim.py blender-import`) |
| Test | `test_ttk_anim.py` `test_crouch_walk_gait`: 4 directions, 8 frames, events, slide under 4, feet on the floor, top under the hull, every value encodable, half-clip split |

## Limits

- Modernized movement sets its own pace (D08AA), so in the preview Duke
  travelled about 1,100 units/s with crouch feet: D08K4 must drive the crouch
  walk at the clips' 200 units/s (or scale the clips' ticks to the chosen
  speed) to keep the planted feet planted.
- Entry from and exit to the kneel (178) are the game's own channel blends;
  they were not judged visually yet (D08K4).
- Diagonals are blends of the four directions in D08K4, not separate clips.
- The look is the user's call: stance width, pelvis height, step and lift are
  tool options.
