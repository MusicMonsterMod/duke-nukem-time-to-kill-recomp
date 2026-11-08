# D10H8 - First-person world drawn nearer the present

2026-11-08. Status: Done (accepted and closed 2026-11-08: "it's a whole tonne better!", then "lock it in, close it out"; default changed to 16 ms after the user's A/B). Follows D10H5 and D10H7
([note 392](392-d10h5-adaptive-world-delay.md),
[note 393](393-d10h7-submit-lead-margin.md)) and D49D
([note 418](418-d49d-holes-with-smoke.md)). Lab scripts (local):
`recomp/analysis/d10g-latency/wd8.py` (timing) and `wd8_shots.py` (present
captures); results in `wd8.jsonl`.

## Why hits trailed the crosshair

At 120/180 Hz a present at time t shows the world at t - delay, blended
between game frames n-1 and n. The redraw replayed the older frame's display
list (n-1) with blended positions. Anything new in frame n (a bullet hole and
its smoke, a muzzle flash, an ammo count, the weapon's next frame) therefore
appeared only once the world clock passed frame n's pose time: one whole world
delay (about 65-75 ms) after the game made it. While turning at 360 deg/s that
is about 25 degrees of trail between the crosshair and a fresh wall hit.

EDuke32 (`G_DrawRooms`) draws its newest sprite list and interpolates the
positions back between the last two tics. New sprites show as soon as their
tic exists; only movement is blended.

## Change (`recomp/src/ttk/frame_replay.cpp`, `modern_controls.cpp`)

First person only, present timeline (Modernized at 120/180 Hz):

- **Newer list.** The redraw replays the newer frame's display list (n) with
  the same blended camera, object poses, actor joints and transforms as
  before. New hits, smoke, flashes, HUD and weapon frames appear one game
  frame sooner; positions are unchanged. The job is still owned by the older
  frame, which is released first (oldest slot reused first), so the newer
  slot is always held. `render_state_blend_newer` gives the draw-time state:
  the newer frame's discrete state (weapon, flags), with each value the old
  path blended between ticks (bob, run arms, recoil easing, kick and door
  frames, hand channel, FOV blend) blended the same way under the same
  conditions. A new shot's recoil starts with its muzzle flash at the newer
  tick.
- **16 ms limited extrapolation** (8 in the first build). The world time may
  run up to 16 ms past the newest pose, and the adaptive delay target is 16 ms
  lower. Positions past the
  newest pose run on at the speed both of the last two game steps agree on
  (minmod per axis, `lerp_x`): steady motion continues; a stop, a reversal or
  a one-step jump (stair step, collision push-out) holds at the newest pose
  instead of overshooting. Rotations hold. An extrapolated eye walks the
  portal rooms (flag 2). D10G4's shown positions use the same rule.
  Extrapolation is only as large as the delay is below the D10H5 pair target,
  so after a switch to third person the world does not hold while the delay
  slews back up.
- Switches (developer, environment): `DNTTK_WORLD_DRAW=pair` is the D10H5/D10H7
  draw; `newer` the default; `step` draws the newest complete frame unblended.
  `DNTTK_WORLD_EXTRAP_MS=<0..50>` (default 16). `render_replay`
  `plugin.timeline` reports `world_draw`, `extrap_ms` and `extrapolated`.

Third person, Vanilla, 60 Hz (no redraws), frontend and loading images and
gameplay are unchanged.

## Results (offscreen, private apartment card copies, first person)

`appear` is the age of a newly made frame's contents on the first present that
shows them (a new hit's delay); `trail` is that at a 360 deg/s turn. `dev`
counts presents whose eye step deviates over 8 units from the smooth blended
path (0 by construction for pair).

| Run (8 s, 180 Hz unless noted) | World age | Appear median (p95) | Trail | dev > 8 / s | Starved |
| --- | --- | --- | --- | --- | --- |
| Strafe-turn, pair (old) | 67.2 | 70.8 (73.3) | 25.5 deg | 0 | 3 / 1459 |
| Strafe-turn, newer, e0 | 68.0 | 37.1 (44.6) | 13.3 deg | 0 | 3 / 1448 |
| Strafe-turn, newer, e8 | 60.1 | 28.1 (37.5) | 10.1 deg | 0 | 2 / 1448 |
| Strafe-turn, newer, e16 | 50.9 | 22.7 (33.8) | 8.2 deg | 0 | 3 / 1451 |
| Strafe-turn, step | 37.3 (19-56) | 21.6 | 7.8 deg | world jitter p95 27.8 ms | 0 |
| Strafe-turn 120 Hz, pair | 73.0 | 76.9 (81.6) | 27.7 deg | 0 | 2 / 966 |
| Strafe-turn 120 Hz, newer, e8 | 62.6 | - | - | - | 2 / 966 |
| Level 1 run-strafe, pair | 63.1 | 65.9 (81.5) | 23.7 deg | 0 | 0 / 1462 |
| Level 1 run-strafe, newer, e8 | 56.9 | 25.8 (42.8) | 9.3 deg | 0 (max 1.8 u) | 0 / 1442 |
| Level 8 run-strafe, pair | 54.4 | 57.2 (63.1) | 20.6 deg | 0 | 0 / 1468 |
| Level 8 run-strafe, newer, e0 | 53.3 | 22.9 (25.3) | 8.2 deg | 0 | 0 / 1469 |
| Level 8 run-strafe, newer, e8 | 45.6 | 16.6 (23.0) | 6.0 deg | 0 (max 6.3 u) | 0 / 1468 |
| Level 8 run-strafe, newer, e16 | 39.1 | 17.0 (23.8) | 6.1 deg | 3.05 (max 28 u) | 0 / 1468 |
| Level 1 run-strafe, default e16 | 47 (delay) | 18.5 (37.1) | 6.7 deg | 2.5 (max 28 u) | 0 / 1452 |

- The newer list alone halves the hit delay with no change to motion.
- The floor for a hit is lead + composition (about 17-22 ms): a frame cannot
  be shown before it is composed and its redraw submitted. e8 reaches it in
  fast scenes; e16 adds world freshness but no hit freshness there, and costs
  smoothness (about 3 visible eye-step deviations per second on level 8).
- Plain extrapolation (before the minmod limit) overshot up to 125 units on
  level 1 stairs and push-outs; the limit brought that to 21 at e16 and under
  7 at e8.
- Step (no blend) has the lowest world age but moving things step at the game
  rate (world clock jitter p95 28 ms): rejected.
- Pace divisor 1, no pace changes, 0-2 repeats in every run. Third person with
  the defaults: delay 68.9 ms, no extrapolation (gated).
- Present captures firing pistol then shotgun while turning and strafing
  (`wd8_shots.py`, newer and pair): weapon, muzzle flash, smoke, holes and HUD
  draw normally; in the newer draw the fresh pistol hits sit closer to the
  crosshair.
- `ttk-aim-test recomp/disc/SLUS_005.83` and `ttk-input-test` pass.

Against EDuke32 (newest sprites, positions one tic behind, so new sprites 0-33
ms): new hits now show about 17-28 ms after their game frame, positions about
46-60 ms. The remaining gap in positions is the game's own 30 Hz frame plus
its composition and the redraw lead.

## Default 16 ms (user, 2026-11-08)

The first build defaulted to 8 ms. User A/B in play: "it's a whole tonne
better! ... is it a little more in sync with your "more extrapolation"
setting on?", then, asked to watch for eye-step deviations on stairs and while
strafing: "i'm not experiencing any wobble at all ... so let's go for 16 as
default then". The deviations the lab counts (up to about 28 units on single
presents) were not visible to the player. Executable
`289bc12019668f18782a8be599e9934e0baa4a4ef89ad4c40b06ab8599e666f9`.

## Not verified / limits

- Accepted in play on the real 180 Hz display (user, 2026-11-08); no wobble
  seen at 16 ms. A/B remains: `DNTTK_WORLD_DRAW=pair`, and
  `DNTTK_WORLD_EXTRAP_MS=0` or `8`.
- Objects removed in the newer frame vanish one game frame sooner; objects
  new in it are drawn at their newer position (no blend) for that frame.
- Joint and transform rotations hold during the at most 16 ms of
  extrapolation; translations continue (minmod), so a fast-turning enemy can
  lag its own rotation by up to 16 ms.
- Heavier scenes (20 fps) not measured; the 90 ms ceiling is unchanged.
- The energy beam's D49/D49A follow (glow and sparks) is kept as accepted.
