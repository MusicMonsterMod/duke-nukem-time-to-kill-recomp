# D08J2 - Poles and chains: A/D sidestep the wrong way

Status: **Accepted** (2026-10-07, user: "fully and completely accept it all"). Modernized only; Vanilla unchanged.

User request: "when climbing poles or chains, left and right are inverted, A
turns right, and D turns left! i would like the opposite to happen." Test
location: the user's UI slot 3 (savestate file 02), the medieval chain.

## Reproduction (UI slot 3 = savestate file 02)

Private copy in `recomp/analysis/d08j2-20261007/cards` (SHA-256
`d85450def6811c21e51cad82767c19179a34b184dc05c5597ea3e6f6a53f7416`, identical
to the player's file, which was only read). Duke stands on a crate under a
hanging chain (type 841, flags `0x100412`). W + E + Space catches it:
mode 3, 154, then the hang-climb 193/194 with the chain in `+0x17c`.

Before the fix, holding A for 40 frames: yaw 2048 -> 1082, Duke moved from
(-2544, 26900) to (-2768, 26694). Facing -z, Duke's own left is +x (on the
ground in Modernized, A strafes him to +x with the camera behind), so he went
round the chain to his right: on screen, A moved him right.

## Original contract

- The climb handler `0x80043eb8` dispatches on the animation through the
  table at `0x80013dd0`. 192/193 run `0x8004421c`, 194/195 run
  `0x80044398`; both call `0x800439e4`. Plain ladder rungs (186..189 at
  `0x80043fd0`/`0x80044128`) never call it. Climbing walls (198..205) have
  their own shimmy (204/205) and were not changed.
- `0x800439e4` reads two button tables (`0x800d1a98`, `0x800d14e8`) and
  sidesteps Duke with `0x80073b7c` (directions 2/3) rotated by his yaw; for
  an object without type flag `0x100`, `0x80043c08` then turns him back to
  face it, so he circles poles and chains.
- Vanilla, private run, original D-pad pressed through the debug port:
  Left 40 frames: yaw 2048 -> 1402, (-2544, 26900) -> (-2732, 26798), Duke
  goes to his right; Right brings him back. So the direction is the
  original's own (Left turns his yaw left, which carries him the other way
  round the object); the Modernized mapping (A = D-pad Left) only passed it on.

## Change

`ladder_top.inc` `pole_sidestep_ready()`: Modernized traversal input, mode 3,
animation 192..195. `pc_input.cpp` (the attached-traversal block) sends A as
D-pad Right and D as D-pad Left there; everywhere else A/D keep Left/Right.
The selection is by the original hang-climb state, not by level or object, so
every pole and chain using it is covered. The arrow keys (the unrebindable
original-button hatch) are untouched.

## Evidence (binary `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`)

Private Xvfb runs, real keys, dummy audio, isolated profile and cards
(`recomp/analysis/d08j2-20261007/`, `ad2.py`, `van.py`):

| Check | Result |
| --- | --- |
| Modernized third person, A 40 frames | yaw 2048 -> 2883, (-2544, 26900) -> (-2329, 26739): to his left (screen left) |
| then D, D, A | D: back and on to his right (yaw 2140, 1360); A: back to his left (2165) |
| First-person profile | the climb keeps the third-person view; same directions (A: 2048 -> 2756, to +x) |
| W / S on the chain | W climbs (Y 2082 -> 1737), S climbs down (1706 -> 2323) |
| Vanilla | original D-pad Left still sends Duke to his right (above) |
| Suites | `ttk-input-test` (new D08J2 case: ladder A/D = Left/Right, pole A/D swapped), `ttk-controls-test` (LEVEL00 `d08-camera-final` fixture, LEVEL01, all levels), `ttk-aim-test`, `ttk-near-test` PASS; Python 121 OK (2 skipped: owned-disc tests) |

## Limits

- One chain (UI slot 3) tried live. Other poles and chains are covered by
  the shared state, not individually played.
- The first-person profile shows the original climb view on the chain (no
  eye view there); D08J3 is the camera follow-up.
- Climbing walls (198..205) keep the original Left/Right; not reported and
  not examined live.
