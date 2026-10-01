# D08W - Shallow-water jumps in the held direction

Status: **Done** (user-accepted 2026-10-01). Modernized only; Vanilla unchanged.

## Reproduction

On a private copy of the user's UI slot 1 (runtime slot 0): the first subway
tunnel, Duke in ankle-deep water (surface -5632, floor -5504), with private
cards and profile in `recomp/analysis/d08w-subway-jump`. Real keys on Xvfb, jump
displacement measured in camera axes (`sidejump.py`):

| Input + Space | Before (forward, right) |
| --- | --- |
| A, D, W or S standing | about (1600, 10) every time |
| Shift-run forward, then A or D + Shift | about (1650, 10) |
| walking or running A first | (1586, 89) |

Every wade jump went forward, including S.

## Cause

Wade jumps are host-owned (`swim_try_wade_jump` in `swim.inc`). The launcher
calls the original `0x8003e2d0`, which aims the jump along Duke's facing, then
`swim_boost_horizontal` was meant to aim it along the held camera-relative
direction. It only did that when the original left no horizontal speed.
Otherwise it rescaled the original's velocity to the wade minimum and kept its
facing direction (with view aiming Duke faces the camera, hence "forward").

A second, smaller issue: the D08O ledge and climb assists (which jump toward
what lies ahead of the view) accepted any input except S, and the climb probe
accepts flat water ahead, so strafing jumps were routed through them too.

## Change (`swim.inc`)

- `swim_boost_horizontal` always aims along the held direction, at the faster
  of the original launch speed and the wade minimum (walk 3600, run 4800). The
  D08O ledge assist's exact speed is unchanged.
- The ledge and climb assists serve Space alone or a jump with W held.
  Strafing (also after running forward) and backing up use the ordinary
  directed jump.

## Evidence (binary `7ca634554b3a2a6b4be49361c92171ba6efbe9c541b3bc1b7d3a5f31056d3328`)

Subway, camera turned so strafing runs along the open tunnel (`after-turn`):

| Input + Space | After (forward, right) |
| --- | --- |
| A standing | (10, -1612) |
| Shift-run forward, then A + Shift | (10, -1612) |
| Walking A | (-17, -1716) |
| S standing | (-1650, 17) |
| W, D | short: a train stands on that side |

Original camera (`after`): W (1554, 10), S (-1650, 17), run forward then D
(-12, 1238); A is blocked by the train on the left.

Crystal-2 flooded corridor (private copies of UI slots 2 and 3, depth 256):
jumps follow the held direction (A (-4, -1654), W (1690, -20), run then A
(2, -1311), D (9, 1467), S (-1709, -16)); cases into an adjacent wall do not
travel, as before. Native controls/input/aim suites PASS; Python 87 OK.

## Limits

- The D08O subway-platform ledge assist was not triggered in these runs (no
  platform in reach from the saved position); its code path keeps its exact
  speed and only its trigger is narrowed to Space or W.
- Space alone in shallow water keeps its current behavior (the climb assist
  still turns it into a short forward jump when flat water is ahead).
- Splash and sound behavior was not listened to (dummy audio); the original
  launch still runs.
