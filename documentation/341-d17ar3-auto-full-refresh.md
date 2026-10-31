# D17AR3 part 4 - Polymorph: AUTO at the display's full refresh

2026-10-31. Fourth D17AR3 item ("how far we can push this renderer"). The
user named our renderer **Polymorph** ("It can handle polygons of course! and
we're morphing the locked in n-space engine now to something far beyond it's
1998 bounds"). The in-game choice will be Classic and Polymorph (job D17AR6).
The user then gave this part its goal: "I'm allowing you to now own this. I
want the best compatibility for the widest array of monitors." Status: Accepted
2026-10-31 ("this is great work. fully accepted").

Evidence is local under `recomp/analysis/d17ar3p5/` (`routes.py` from part
3 with a rate-relative full-rate threshold, `probe9.py`, `probe9b.py`,
`probe9c.py` (perf), `hold9.py`; logs, JSON). Every run used private copies
of the D17AQ fixture card and savestates, offscreen video and dummy audio,
and hash-checked the player's saves and profile before and after.

## Finding: the player was at 90, not 120

The player's display runs at 180 Hz (`xrandr`: 1920x1080 at 180, also 120
and 60). D17S AUTO chose `refresh / ceil(refresh / 120)`, so every session
since at least 2026-10-30 presented 90 pictures a second (session logs:
`[TTK pace] minute: 90.0 presents/s`). The player had been choosing 120 for
its stability; at a 30 fps game, 60 Hz shows each image twice while 120 gives
every refresh its own redraw, which is why 120 felt like "how 60 fps should
feel".

## Headroom (before the change)

Fixture routes, scale 4, 16:9, default build, 6 s per route:

| Rate | Full rate (still / turn / walk, 10 slots) | Exception |
| --- | --- | --- |
| 120 | all but one | fixture 9 still (slot 10 jetpack view) |
| 144 | all but one | fixture 9 still |
| 180 | 25 of 30 | fixture 9 all routes; fixtures 1 and 4 still (first route after a load) |

Slot 10's loaded view at 180 Hz, shedding and late pacing off: 47.6 game
frames/s standing, main thread at 100% of a core; turning held 61 and 178
presents/s at 83%. More redraw workers (4, 5, 6) changed nothing (worker
redraws averaged 15 ms; 3 workers keep up). Internal scale 1, 2 and 4 gave
the same rate (not GPU-bound; GPU at 7%).

Main-thread profile (perf, DWARF call graphs, 180 Hz, standing): 46% of the
thread draws finished worker redraws into the GL renderer
(`render_replay_feed`, about 3.5 ms a redraw standing, 2.0 ms turning), the
rest is the live game. Inside the feed: the textured quad path 17%
(`gpu_textured_triangle` 6%, `prepare_precise_triangle` 4%), command
dispatch 14%, batch flushes and the GL driver 7.5%, widescreen helpers
(`gpu_display_info_ref` 2.6%, `ws_*`) about 5%. No single hot spot; holding
180 there needs the feed about a third cheaper (job D17AR5).

## Changes

- **AUTO up to 240** (`replay_rate.h`): `refresh / ceil(refresh / 240)`.
  Every refresh up to 240 Hz gets its own picture; faster displays use an
  even fraction (360 -> 180, 480 -> 240, 500 -> 166.7). Unknown refresh
  still falls back to 60. Fixed choices, Display, Unlimited, 60 and Vanilla
  are unchanged. Menu hint, launcher help, settings text and manual updated;
  `test_auto_frame_rate.py` covers the new table.
- **Savestate loads are not judged** (`modern_controls.cpp`): the 3 s after
  a savestate load no longer count as "emulation behind" (the catch-up halved
  presents after loads: fixtures 1 and 4 at 180).
- **Probes only when lighter** (`frame_replay.cpp`): after stepping down,
  the governor steps back up after 5 clean seconds (times the backoff) only
  when the second's average redraw feed is under 75% of the cost when it
  stepped down (the view turned away from the heavy vista); otherwise after 30
  clean seconds (times the backoff). A probe that falls behind goes back after
  one slow second instead of two. Before, standing in the heavy view retried
  180 every 5/10/20/40 s and slowed the game to about 48 for 1-2 s each time.

## Results

Final build, fixture routes (game frames/s about 60 throughout unless noted):

- 120: 120 of 120 route seconds at full rate, all 10 slots (before: slot
  10's vista standing after a load stepped down); 0 failed redraws.
- 180: 115 of 120; the misses are slot 10's vista only (standing steps to
  about 115 presents/s, game 57; turning 166). 0 failed redraws.
- 165: 29 of 30 routes at full rate; fixture 9 standing steps to about 90.
- 180: fixtures 1 and 4 now full on every route; fixture 9 standing and part
  of turning step down evenly (every 2nd, sometimes 3rd refresh).
- 240: 23 of 30 at full rate; fixtures 1, 2 and 9 step to about 120 in parts
  (fixture 1 game rate 56-57 while stepping).
- 40 s standing in slot 10's view at 180: steps to 90 at the start, stays
  there (occasional single-second dips in the harness, which polls the debug
  server), and to 60 once.

Tests: `test_auto_frame_rate.py`, frame-rate and profile unit tests,
`ttk-near-test`, `ttk-input-test`, `ttk-controls-test` (clean
`d08-camera-final` fixture) pass. Patch re-exported and verified against the
pinned clean framework.

## Limits

- Physical monitors other than the player's 180 Hz display are not tested;
  rates were forced offscreen (`--frame-rate 144/165/180/240`).
- Slot 10's jetpack vista and similar heavy views run at half the refresh at
  180 and above on this machine (i7-5960X). D17AR5 targets that cost.
- Stepping between full and half rate when turning toward and away from a
  heavy vista is new at 180; the player's previous AUTO never left 90.
