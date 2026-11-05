# D10H7 - Redraw submit lead and the world delay margin

2026-11-05. Status: Done (accepted and closed 2026-11-05). Follows D10H5
([note 392](392-d10h5-adaptive-world-delay.md)); lab scripts under
`recomp/analysis/d10g-latency/` (local): `world_delay.py`, `delay_sim.py`.

D10H5's adaptive world delay is p99 pair age + lead + one present period.
The job assumed the period term covered redraws submitted up to a present
early. Measuring first showed the opposite.

## What the submit timing actually is

New `render_replay` `plugin.timeline` counters over the last 1024 gameplay
submits: `ahead_ms` (target - submit: min, p50, p99, max) and `need_ms` (target
- newest complete pose time: p50, p99, p99.9, max; a submit is starved when the
world delay is below its need). `DNTTK_POSE_TRACE` also logs every submit as
`[submit] host_ms t lead delay need period`.

- `timeline_provider` submits a deadline at the first provider tick where
  `t - now <= lead`, and the provider ticks about once per present. So the
  actual lead is never above the planned lead: it is lead minus 0 to one
  period (180 Hz runs: ahead p50 12-15 ms, min 6-11, with lead 11-17).
- The delay needed for a submit is (target - submit) + (submit - newest
  complete pose), at most lead + pair age. The period term was therefore not
  covering submit granularity; it was tail margin on the p99 pair-age
  estimate. Queueing at exactly t - lead would make submits earlier, so the
  need would grow, not shrink. That was not built.

## Margin sweep

`delay_sim.py` replays delay policies against the traced submits, which is
valid because the need does not depend on the delay. The D10H5 policy simulates
to the recorded delays (74.8 / 68.0 / 78.5 against 74.8 / 68.0 / 78.3 ms).
Simulated over five traced runs (13101 gameplay presents; 180 Hz strafe-turn,
run-strafe, level 1 and level 8, and 120 Hz strafe-turn):

| Margin (x period) | Starved | Share |
| --- | --- | --- |
| 1 (D10H5) | 1 | 0.008% |
| 0.75 | 2 | 0.015% |
| 0.5 | 5 | 0.038% |
| 0.25 | 8 | 0.061% |
| 0 | 14 | 0.107% |

The trade is smooth, with no free reduction. Both a direct p99.9-of-need
policy and a per-frame worst-need policy starved more for the same delay.

Live offscreen runs, 12 s each, private apartment card copies
(`world_delay.py`, `MARGIN=`):

| Run | m1 delay / starved | m0.5 | m0 |
| --- | --- | --- | --- |
| Strafe-turn 180 | 74.8 / 1 | 68.4 / 1 | 67.3 / 3 |
| Run-strafe 180 | 68.0 / 0 | 63.0 / 1 | 61.7 / 2 |
| Strafe-turn 120 | 78.3 / 0 | 73.6 / 1 | 69.7 / 2 |
| Level 1, 180 | 74.8 / 0 | 70.2 / 0 | 69.2 / 5 |
| Level 8, 180 | 66.2 / 0 | 62.3 / 0 | 59.7 / 0 |
| Total starved | 1 / 10776 | 3 / 10159 | 12 / 10166 |

Pace divisor 1, no pace changes and 0-3 repeats in every run. Delays vary by
a few ms between sessions with pair age, so the per-run columns are not a strict
paired comparison. The accepted D10H5 build measured 9 / 11700 (0.077%) in
its own lab runs (note 392 jsonl).

## Change (`recomp/src/ttk/frame_replay.cpp`)

- The margin is now half a present: delay target = p99 pair age + lead +
  0.5 x period (2.8 ms at 180 Hz, 4.2 at 120), about 3 ms less world delay at
  180 Hz and about 4 at 120. Two more default-build runs: strafe-turn 180 71.9 ms,
  2 / 2173 starved; 120 73.0 ms, 1 / 1449. Over all seven 0.5 runs: 6 / 13781
  (0.044%), within the D10H5 level.
- `DNTTK_WORLD_DELAY_MARGIN=<0..2>` scales the term: 1 is the D10H5 build,
  0 is no margin (0.12% starved, about one held present every 5 s at 180 Hz).
  `DNTTK_WORLD_DELAY_MS` still fixes the whole delay.
- Modernized only (present timeline); Vanilla, 60 Hz, frontend and loading
  images unchanged. Not in scope: a lower percentile, early partial frames,
  extrapolation.

## Not verified / limits

- Accepted in play on the real 180 Hz display (user, 2026-11-05: "it works
  absolutely fine, it's so snappy now"). A starved present holds the world for
  one present and then catches up. At 0.044% that is about one every 13 s at
  180 Hz, against about one a minute measured today at margin 1.
- Not measured in heavier scenes; the 90 ms ceiling is unchanged.
- Vanilla was not run; its code path does not reach the timeline provider.
