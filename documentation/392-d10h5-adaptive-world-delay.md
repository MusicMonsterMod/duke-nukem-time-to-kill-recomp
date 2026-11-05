# D10H5 - Adaptive world delay at high refresh

2026-11-05. Status: Done (accepted and closed 2026-11-05). Research in D10H
([note 388](388-d10h-duke3d-weapon-feel.md)); lab of D10H1
([note 389](389-d10h1-weapon-feel-lab.md)); latency report
`recomp/analysis/d10g-latency/REPORT.md` (local).

At 120/180 Hz (Modernized, late camera, present timeline) everything on screen
except the camera rotation was drawn a fixed 90 ms in the past. Duke 3D draws
its world at most one tic (0-33 ms) behind. The world delay is now adaptive.

## What limits the delay (measured and code)

A present at time t shows the world at t - delay, interpolated between pose
frames n-1 and n (`timeline_pair`). The redraw is submitted a lead before t
(10-16 ms here) and needs both frames complete, so frame n must have finished
its composition (`composition_end`) by then:

    delay >= (frame n ready - frame n-1 pose time) + lead (+ submit granularity)

"Pair age" (frame n ready - frame n-1 pose time) from 18 earlier traced lab
runs (`DNTTK_POSE_TRACE`, apartment fixture, 120/180 Hz): median 37-40 ms,
p95 41-52 ms, p99 49-58 ms. It is one game interval (33 ms at 30 fps, 50 ms at
20) plus the composition (median 4-7 ms, p99 15-24 ms). With the lead, the
safe delay in those scenes is about 65-75 ms. The D10H estimate of 55-65 ms
left the composition out.

If the pair is not complete, the timeline uses the previous pair at alpha 1:
the world holds for that present, then catches up (the judder kind). New
counter: `timeline.starved` (gameplay presents whose world time was past
their pair). Fixed delays, strafe and turn, 180 Hz, about 1450 presents:

| Fixed delay | Starved presents |
| --- | --- |
| 90 (old default) | 0 |
| 70 | 1 (0.07%) |
| 65 | 10 (0.7%) |
| 60 | 98 (6.7%) |
| 55 | 169 (11.7%) |

## Change (`recomp/src/ttk/frame_replay.cpp`)

- Every gameplay composition end records its pair age (128-frame window,
  about 4 s, p99; a single hitch is ignored).
- Target delay = p99 pair age + current lead + one present period, clamped
  50-90 ms. 90, the old fixed value, is the ceiling, so a heavy scene can only
  return to the old behaviour, never go later.
- The delay is slewed with elapsed time: at most 10% slower world clock while
  it grows and 3% faster while it shrinks, so the world never jumps back or
  forward. Startup uses 90 until 8 pair ages exist.
- `DNTTK_WORLD_DELAY_MS=<ms>` still fixes it (50-130); `DNTTK_WORLD_DELAY_MS=90`
  is the A/B against the old build.
- `render_replay` `plugin.timeline` reports `world_delay_ms`,
  `world_delay_wanted_ms`, `world_delay_fixed`, `pair_age_p99_ms`, `presents`
  and `starved`.
- Modernized only: the present timeline is the Modernized provider
  (`input_modernized()`); Vanilla, 60 Hz (no redraws), frontend and loading
  images (their own delay) are unchanged. Shot view (D10G3) and shoot what you
  see (D10G4) use the image on screen, so they follow automatically.

## Results (offscreen, private apartment card copies, `world_delay.py`)

World age is measured per present from the present trace and `[pose]` times.

| Run (8 s) | World age median (p5-p95) | Starved |
| --- | --- | --- |
| Strafe + turn, 180, fixed 90 | 90.0 (89.9-90.3) | 0 / 1454 |
| Strafe + turn, 180, adaptive | 70.3 (66.3-74.0) | 1 / 1463 |
| Run-strafe + turn, 180, adaptive | 66.3 (58.7-69.7) | 0 / 1471 |
| Strafe + turn, 120, adaptive | 75.1 (69.9-80.4) | 0 / 969 |
| Level 1, 180, adaptive | 71.6 (69.5-76.8) | 0 / 1459 |
| Level 4 route (load failed, apartment), 180 | 70.8 (66.9-74.6) | 0 / 1460 |
| Level 8, 180, adaptive | 63.2 (57.2-68.1) | 0 / 1478 |

About 20-27 ms less world latency at 180 Hz (about 15 at 120), with starved
presents at or near the fixed-90 level. Pace divisor stayed 1 with no rate
changes and 0-2 repeats in every run, as at fixed 90.

D10G4 slide fixture (`seen_slide.py`, level 1, 180): 18 of 19 shots on the
screen view, and the rewound hit still follows the drawn position (4 of 5 hits
with the drawn miss under 190 u). Its drawn-to-current shift median (162 vs
164 u before) is dominated by the fixture's host RAM-write jitter and does not
resolve a 20 ms change.

## Not verified / limits

- Accepted in play on the real 180 Hz display (user, 2026-11-05: all of
  Time to Kill, challenge 1 and part of Duke Hill, "didnt experience a single
  issue"). A blind fixed 70 / 90 comparison was not completed.
- Follow-up D10H7: submit redraws at their exact lead, dropping the
  one-present margin (about 3-6 ms).
- No scene heavier than these was measured; at 20 fps the target reaches the
  90 ms ceiling (50 ms interval + composition + lead) and behaves as before.
- Rising after a sudden load increase takes up to about 200 ms at the 10%
  slew; presents in that time can be starved.
- Real display and mouse latency not measured.
