# D14H - Jetpack arm projection and a body that turns with the view

2026-11-02. Done, user accepted revision 2: "that's a winner. lock it in and
close it out". Commit/push closeout authorized. Coverage limits below remain
in force. Evidence labels follow the accepted D36 audit
([note 257](257-d36-first-person-presentation-audit.md)): S source/native
tests, L isolated runtime observations, U user report.

## Problem

U (backlog intake): in first-person jetpack flight Duke's arms do not appear
to use the safe viewmodel projection introduced with the wider field of view.

S: D36K1 ([note 273](273-d36k1-jetpack-arms.md)) claims empty-handed arm
parts 10-15 during `PerspectiveAction::Jetpack` as `LimbOwner::Body`: world
depth, clipping admission and fade correction, but world projection. D14B
([note 297](297-d14b-weapon-projection.md)) gave the fixed H=256 projection
only to display owners, and D36J5 later gave it to ordinary legs. D36K1 was
accepted when world H was 256; D14F then made FOV 90 (H=160) the default. So
in flight the legs and any held weapon draw at H=256 while empty-handed arms
draw at the world H, about 62% of their accepted framing at the default FOV
and 36% at FOV 120.

L (private copy of the D36 audit cards, slot 8 LEVEL05, offscreen, 60 Hz,
baseline executable `3afecaf4...7bd6f4`): verified equipment 0 and poses
hover 164, forward 165, backward 166 and strafe 170. At FOV 90 and 120 the
arms shrink toward screen centre; looking down in forward, strafe and hover
flight exposes the open upper-arm ends with the hands floating beside them,
out of scale with the fixed-projection legs.

The armed case was already correct: a held weapon or item takes the D14B
`DisplayHand`/`DisplayArm` owners, and the baseline armed stills keep one size
at every FOV.

## Change

A new `LimbOwner::FlightArm` has the `Body` presentation (world depth, safe
clipping admission with the joint-depth bridge, proximity-fade correction,
per-joint cleanup) and joins `fixed_projection()`. The D36K1 flight branch in
`first_person.inc` claims it instead of `Body` for the same parts and
conditions. Native flight poses, the absence of the ground-only 120-unit
lowering, armed display arms, swimming/contact rigs, ground arms, third
person, reflections and Vanilla are unchanged. Projection only: no gameplay
matrix, GTE, animation, setting, save, framework or generated-C change.

The contact rig (D14G's approach for swimming) was not needed: in flight the
native arm chain is attached to a body pose that already keeps the shoulders
below the view at H=256, which is the presentation the user accepted for
D36K1. Matching the legs' fixed projection restores that framing at every FOV.

## Verification

S: player build. `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels),
`ttk-aim-test`, `ttk-input-test` (Xvfb) and default `ttk-near-test` pass. The
D36K1 flight group (all eight flight animations, three schemes, both costume
maps, equipment 0/1/2) now requires `FlightArm` with fixed projection and
world clipping for empty-handed arms, never `Body` in flight. The D14B packet
group includes `FlightArm` at H=160-277: identical screen output, UV and depth.
That group only runs with `DNTTK_UNIFIED_GEOMETRY=0`, where an unrelated
D17AH sky assertion fails identically at HEAD; it was run once in a scratch
build with that single assertion bypassed (passes), then the test was restored.

L (`recomp/analysis/d14h/`, local only; player saves, cards and profile
hash-checked unchanged after every run):

- `base60` and `cand60`: FOV 60/90/120 (baseline 90/120), hover, backward,
  forward and strafe flight at level and two downward pitches, hover looking
  down, armed backward flight. `runs/compare.png` and `runs/detail120.png`.
  Candidate arms keep the same size and framing at all three FOVs; backward
  flight looking down shows connected arms in scale with the legs; the open
  upper-arm ends move to the screen edges as at the original projection.
  Armed flight is unchanged.
- `cand120`: the same route at 120 Hz, FOV 120. Same framing; three replay
  workers, 2254 jobs, zero failed jobs and zero worker deaths.

Limits: one LEVEL05 route, default costume, sequential samples rather than
atomic frames, no live save/load in flight this run (D36K1 covered that path,
which this change does not touch), not a 180 Hz performance check. At steep
downward pitch in forward/strafe flight a slice of upper arm can still sit at
the lower screen corners, as in the accepted D36K1 presentation. Human motion
review is required.

Candidate SHA-256:
`870aded75d0e48d8b9ccf1357434ab6c61612ceb1289d99faebfc046c57f09a5`.

## Revision 2 - the body turns with the mouse (all first person)

U (first candidate playtest): "they dont seem to move with duke if you turn
left and right quickly, thats what the issue is"; "duke's whole body in fact
really"; "and crouching i suppose too"; asked whether Duke's body turn can be
locked to the mouse, holstered and standing too.

S: gameplay already locks it. `face_view` writes the view yaw into Duke's
heading at every game update. The lag is presentation: the D17B late camera
re-aims every presented redraw (120/180 Hz) at the newest mouse yaw about the
eye, while Duke's skeleton stays interpolated between 30 Hz game frames. On a
fast turn his body, legs and world-posed arms trailed the view and caught up
at the next game frame. Display weapon arms are built from the camera, so
armed hands never showed it.

Change (`frame_replay.cpp`, `apply_late`): in first person the late redraw
also carries a body camera, the interpolated game yaw at the late pitch about
the same eye. The worker swaps it into the camera rotation and view matrix
only for Duke's own actor draw (`replay_body_camera`, from
`weapon_draw_bounds`) and restores the late camera at the next object-list
step or actor entry. Viewing Duke through his interpolated yaw is identical to
turning his whole body to the late yaw about the eye, so every first-person
state (standing, walking, crouching, jetpack, swimming, contact actions,
holstered or armed) follows the mouse between game frames. Heading, aim,
collisions, the world, reflections, other actors, third person, Vanilla and
non-late presentation are unchanged. `DNTTK_LATE_BODY=0` (developer, one
launch) restores the old draw. Duke stays excluded from recorded-transform
substitution, so his joints are composed with this camera live.

L (`analysis/d14h/turn.py`, `runs/turn-compare.png`; 120 Hz, slot-8 copy):
`DNTTK_LATE_TEST_YAW_FILE` turns every redraw 0/+25/-25 degrees past the game
camera, freezing an extreme fast turn. Old build: jetpack arms and legs swing
across the screen. New build: arms, legs and the crouched knee keep exactly
the same screen position while the world turns; armed hands unchanged. Both
runs: zero failed replay jobs, zero worker deaths. Native controls (with a
test stub), aim, input and near suites pass.

Limits: a static yaw offset, not a live mouse flick; 120 Hz rather than the
player's 180 Hz (same path); world occlusion of the body uses the turned pose
(about one game frame of turn at most). The user's live turn review is the
acceptance test.

Executable SHA-256 (revision 2):
`e20f64960237792afb450a9c0d6db47c797083e131753c226a4b238679458457`.

## Playtest

First person: flick the mouse left and right quickly while standing,
crouching and flying (empty hands and with a gun), looking down at Duke's
arms and legs; they should stay with the view. With empty hands in flight,
also try `fov 60`, `fov 90` and `fov 120` in the console (session only).

```sh
python3 recomp/tools/local/run.py
```
