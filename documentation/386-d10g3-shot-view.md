# D10G3 - Shots aim from the view on screen

2026-11-05. Status: Done (user: "ok yes, it feels great, it feels like an improvement", then "close us out"). Follows the D10G report
([note 384](384-d10g-aim-mouse-feel.md)) and the 2026-11-05 latency
investigation (local `recomp/analysis/d10g-latency/REPORT.md`, causes 3 and 4).
Executable: `b28bb7358fa6166b2b07a292f87c3d3f75671708f47a02d60ac45d8e45bf34cc`.

## Problem

The shot entry `0x8003c500` runs inside the game update before the camera
update, so the view adapter aimed with the previous update's camera
(`camera+0x0c` forward, `camera+0x14` eye). At 120/180 Hz the screen shows the
D17B late view instead: the rotation about 34 ms newer (the newest mouse look
minus the present lead) and the eye at the presented world time, rotated about
the orbit pivot. While turning, the round left about 2.4 degrees behind the
crosshair at 72 degrees/s; while strafing, the shot eye ran about 64 ms of
Duke's sideways motion ahead of the eye on screen (about 42 u walking, about
190 u running).

## Change

- `frame_replay.cpp`: every redraw job whose late view applied records the
  camera it draws (eye after the late rotation, forward = rotation row 2).
  When a present shows that job, the view becomes the shown view (a repeat
  keeps it; a present with no redraw clears it).
- `weapon_aim.cpp` (view aiming, Modernized): the shot takes eye and forward
  from the shown view, before free look (D10E keeps its own eye), the screen
  ray, cover checks, D10G1 point-blank and D22D beam locks, which all follow
  the new ray unchanged. Fallback to the game camera when there is no shown
  late view (60 Hz, game-owned cameras such as climbs, frontend), when it is
  older than 60 ms, when its eye is over 1536 u from the game eye, or turned
  over 100 degrees from it (a cut).
- Native weapon timing, muzzle, spread, projectile and damage are untouched.
  Vanilla and `original` aiming are unchanged.
- A/B: backtick console `shotview [on|off|default]` (session only),
  `DNTTK_SHOT_VIEW=0` at launch. Diagnostics: `ttk-aim-shot ... view= eye=
  forward= shift= turn= age=` and `shot_view`, `screen_view_shots`,
  `screen_view_refused`, `last_view_*` in the aim debug JSON.

## Evidence (offscreen, private apartment card copies, pistol)

`rays.py` / `rays_eval.py` in `recomp/analysis/d10g-latency/` (runs `g3-*`);
the evaluator now uses the eye the shot logs. Player files hash-checked
unchanged after every run.

| Run | Before (2026-11-05 report) | After |
| --- | --- | --- |
| Turn 72 deg/s, 180 Hz | shot 2.42 deg behind the screen (33.8 ms) | median 0.00 (-0.03..+0.02), 19/19 screen view |
| Turn 72 deg/s, 120 Hz | 2.35 deg (32.7 ms) | median 0.00, one shot 1.2 (present boundary) |
| Turn, 180 Hz, first person | - | median 0.00, one shot 1.2 |
| Walk strafe 684 u/s, 180 Hz | eye 42 u off the screen eye | 0.5 u (-2.5..3.1) |
| Run strafe, 180 Hz | eye about 186 u off | about 1 u (shot eye moved 181-213 u back to the screen eye) |
| Still, 180 Hz | 0.01 deg | 0.01 deg, 1 u |
| Turn, 180 Hz, `DNTTK_SHOT_VIEW=0` | - | 2.49 deg (34.7 ms): the old behaviour |
| Walk strafe, 60 Hz | - | 19/19 shots on the game camera (no redraws at 60 Hz) |

The two 1.2 degree outliers are 16-17 ms: a shot on a present boundary, where
the evaluator's choice of the shown present and the game's differ by one
present. The turn runs' eye column in `rays_eval.py` compares against the
un-rotated interpolated eye and so reads about 145 u; that is the late view's
orbit about the pivot, not an error.

`ttk-aim-test` (real adapter, collision fixtures): new D10G3 group (screen
eye 300 to the side and screen forward turned 30 degrees are followed; stale,
cut, switched-off, `DNTTK_SHOT_VIEW=0` and no view keep the game camera;
console parse and refusal) plus every earlier group passes.

## Not changed / not verified

- Moving enemies still trail by the 90 ms world delay (cause 2) and the native
  100-200 ms press delay (cause 1). The "shoot what you see" lag
  compensation is D10G4.
- Real mouse at 180 Hz on the player's display, real enemies, other weapons in
  play (the change sits before all gun, throw and continuous-weapon paths).
