# D10G4 - Shoot what you see: the crosshair meets enemies where drawn

2026-11-05. Status: Done (accepted 2026-11-05: "it's aceeptable as it is"). Follows D10G3
([note 386](386-d10g3-shot-view.md)) and the 2026-11-05 latency
investigation (local `recomp/analysis/d10g-latency/REPORT.md`, candidate 1).
Executable: `4f0a0d38419624ed9e95646153210f6102d9b7a7d263432c3a5090da4040c8fb`.

## Problem

At 120/180 Hz the late-view image draws the world at the presented world time,
about 90 ms in the past (`timeline_pair`), while the crosshair's screen-ray
query (`0x8006d980`) tests actors where they are now. D10G3 already aims from
the view on screen, so a still target is exact, but a pig cop running across
the view at about 1900 u/s is drawn about 171 u behind where the query sees it:
the crosshair sits on the enemy and the query meets the wall behind it. The
same mismatch made the D10G1 targetable-actor exemption and the D22D beam lock
see a wall instead of a moving enemy.

## Change

- `frame_replay.cpp`: when a present shows a late-view redraw, it also records
  where that image drew each object: the base frame's object poses (position
  +4..+12; Duke, his held/climbed objects and every drawn actor) blended toward
  the next frame at the redraw's alpha, exactly as `build_params` does (1536 u
  jump guard). `shot_view_poses` hands them to the shot with the D10G3 view.
- `weapon_aim.cpp` (Modernized view aiming, only when the D10G3 screen view
  applied): for the one screen-ray query, every drawn actor other than Duke and
  his held/climbed objects that moved between 1 and 1536 u since it was drawn
  is moved back to its drawn position: position +4 and the world joint
  translations at +0x3c that the hit tests also read (the actor collision
  `0x80076984` -> `0x80076330` reads +4, the +0x40 model box, the +0x44 sphere
  list and the joint matrices). Every word is restored right after the query.
  When the ray meets one of those actors, the target point is moved by that
  actor's shift, so the round aims at the same point on the enemy where it
  stands now. The body-to-muzzle cover check, the assist lock, D10G1
  point-blank and the round itself all meet current positions, unchanged.
- D10G1's "a targetable actor under the crosshair keeps the exact point" and
  the D22D beam lock now see the drawn enemy, because both read the query's
  actor.
- No rewind at 60 Hz, with no shown late view, a stale or cut one, free look,
  game-owned cameras (the D10G3 fallbacks), Vanilla or `original` aiming.
  Native weapon timing, muzzle, spread, projectile and damage are untouched.
- A/B: backtick console `shootseen [on|off|default]` (session only),
  `DNTTK_SHOOT_SEEN=0` at launch. Diagnostics: `ttk-aim-shot ... seen=<actors
  rewound> actor=<hit actor> moved=<its shift>` and, with `DNTTK_AIM_TRACE=1`
  only, `now=<kind>,<actor>` (the same ray against current positions) and
  `near=<actor>,<drawn miss>,<now miss>,<moved>` (the nearest living moving
  actor's ground-plane miss); aim debug JSON `shoot_seen`, `seen_shots`,
  `seen_rewinds`, `last_seen_actors`, `last_seen_shift`.

## Evidence

`ttk-aim-test` (real adapter, collision fixtures): new D10G4 group. An actor
800 ahead, now 300 to the side, drawn on the crosshair: the query sees it (and
its joint translation) at the drawn place, the round aims at x/z 0.308 (the
same point on it now), and its position and joint words are restored. Off
(console and `DNTTK_SHOOT_SEEN=0`), no screen view, a stale view and a
1536 u jump keep the current positions (-0.100); a still actor is hit as before
(-0.154). Every earlier group still passes.

In game (offscreen, private apartment card copies, level 1, pistol;
`recomp/analysis/d10g-latency/seen_slide.py`): a spawned idle pig cop is moved
sideways across a fixed crosshair at about 1900 u/s by debug RAM writes to its
position (pig cop joint translations read 0, so its position is what the hit
test uses). Per shot, the rewound query against the same ray on current
positions:

| Shot (180 Hz, one run, sliding pig cop) | Drawn miss (u) | Now miss (u) | Rewound query | Current-position query |
| --- | --- | --- | --- | --- |
| 1 | 23 | 187 | hit | miss |
| 2 | 49 | 108 | hit | hit |
| 3 | 151 | 363 | hit | miss |
| 4 | 189 | 143 | hit | hit |
| 5 | 222 | 91 | miss | hit |
| 6 | 257 | 117 | miss | hit |
| 7-14 | 234-350 | 17-591 | miss | hit when now miss under about 280 |

The rewound query hits exactly when the drawn position is within about 190 u
of the crosshair ray and follows the drawn position, not the current one; the
shifts were 116-342 u (about 90 ms at the slide speed, with host write jitter).

| Check | Result |
| --- | --- |
| 60 Hz, sliding pig cop | 7/7 shots on the game camera, `seen=0` (no rewind) |
| 180 Hz, `DNTTK_SHOOT_SEEN=0` | 11/11 screen view, `seen=0` |
| D10G3 turn 72 deg/s, 180 Hz (`rays.py`) | median 0.00 deg, 19/19 screen view, one 1.2 deg present-boundary shot (as before) |

Player files hash-checked unchanged after every run.

## Not verified / limits

- A real running enemy under the player's crosshair: host-side tracking of
  woken pig cops (`seen_track.py`) never kept a living runner under the
  crosshair (the woken ones mostly stand and shoot), so the moving case was
  measured only with the slide fixture. The user judges fairness at 180 Hz on
  the real display.
- Joint translations are shifted with the position (a pure translation); a
  turn or animation change between the drawn and current poses is not
  rewound. Rooms are not rewound: an actor drawn in another room than it now
  stands in is tested in its current room's list.
- The native 100-200 ms press-to-shot delay (cause 1) remains: a click-and-stop
  shot still leaves later than the press. Lag compensation cannot remove it.
