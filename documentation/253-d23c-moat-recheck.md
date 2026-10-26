# D23C - Medieval moat recheck

Status: Done (user-accepted), 2026-10-26. No gameplay changes or rebuild.

## Finding and recommendation

The user reports the October 5 slowdown is now almost impossible to reproduce;
only a tiny frame drop remains when loading UI slot 2, jumping into the water
and swimming forward. Later optimizations may have improved it, but attribution
to a particular change is not established.

Three private slot-2 repeats and a fresh level-6 check did not reproduce the
original sustained slowdown. The user accepted retirement of that original report, retaining the brief
water-entry cadence dip below as an observation.
This is not a claim that every hitch is fixed or its historical cause is known.
No dominant host bottleneck was established and no optimization is proposed.
The user explicitly authorized closeout; see the acceptance entry below.

## Method

Accepted executable: `9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
Existing D23 audit harness adapted privately under
`recomp/analysis/d23c-20261026/audit.py`; results in its `run/` and `summary.json`.
[Measurement summary](reports/d23c-moat-recheck.json).

Same D23 reference settings: Modernized, 120 Hz, first person, fast timing,
150% emulated CPU, 16:9, 4x internal resolution, OpenGL offscreen on the real
GPU, SDL dummy audio. The player's saved frame-rate preference is AUTO; fixed
120 matches D23 and does not certify AUTO/display/compositor behavior.

UI slot 2 (file index 1) and its associated files were copied to private writable
storage, without importing player cards. All three loads succeeded and reported
level 6, FAMILY JEWELS. Each repeat used 10 seconds idle, then 1 second W+Space
and 19 seconds W. Control telemetry registered swimming and positions progressed
along the moat; the post-window capture confirmed the moat view. Enemies were
not disabled. Enemy counts/types were not independently enumerated, so this is
not a controlled Necro population comparison.

Fresh console travel to level 6 took 5.90 seconds. Its separate check used 45
seconds of modest lateral movement/view sweeps, then 30 seconds forward with an
initial jump. The forward route ended against dark geometry; it is a bounded
opening-area route, not exploration of the whole level. Screenshots were taken
outside scored windows. Private-process invulnerability was set before the
routes; slot loads restored their own gameplay state.

## Measurements

| Route | Emulated fields/s | Presents/s | Present p99 | Worst present interval |
| --- | ---: | ---: | ---: | ---: |
| Slot 2 water, three repeats | 59.925-59.976 | 120.000-120.101 | 8.784-8.806 ms | 11.251 ms |
| Fresh level 6, view/movement | 59.949 | 120.053 | 8.762 ms | 9.324 ms |
| Fresh level 6, forward | 59.929 | 120.023 | 8.734 ms | 9.454 ms |

The underlying game-image cadence briefly fell from about 20 to 15-16 images/s
in the roughly 2-4 second segment of each water route, then returned to about
20. Whole water windows averaged 19.38-19.43 images/s. This is a reproducible
brief guest-image cadence change, even though interpolated presentation and
emulated fields remained steady. It may explain the user's tiny perceived drop;
its cause was not isolated. Do not call the route entirely hitch-free or confuse
120 presents/s with 120 original game updates/s.

All scored samples stayed in active, unpaused gameplay. No adaptive presentation
reductions, emulation sheds or warmed output underruns occurred. Dummy audio
accumulated source overflow drops, the known D23 measurement-path limitation;
there was no real-device/listening test. Memory samples are retained but these
short, warming routes are not a memory-stability audit.

Contemporaneous CPU, I/O and memory pressure avg10 were 0.00% in all recorded
samples; no backup/update process matched the host checks. The recent player's
07:01-07:04 session log confirms slot 2 loads and level 6 travel, but contains
no event-resolution profile for the tiny drop. The journal shows a Timeshift
check completing at 07:00:01, before that session; it does not establish backup
contention during play. Current idle readings cannot reconstruct historical
conditions or identify the October report's cause.

## Preservation and limits

All hashed player save/card/settings files retained their contents, sizes and
modification times. Graceful shutdown: 0.067 seconds, exit 0. No game processes
remain from this run. No gameplay/framework edits, rebuild, commit or push.
Existing framework changes and pinned dependencies were preserved.

No Vanilla rerun was needed for an unchanged executable. D23 remains its accepted
baseline. No on/off enemy, function-level bottleneck attribution, physical-display
or full-campaign claim is made. Reopen with a repeatable scene if the sustained
slowdown returns; preserve this route and measure before optimizing.


## Acceptance and closeout - 2026-10-26

User: "I'm happy to close it out". D23C is Done. Acceptance retires the
original sustained-slowdown report based on current playtest and bounded
measurements; it does not assert that the brief water-entry dip was fixed or
that the historical cause is known. Documentation and evidence-summary commit
and push are authorized. The private exploratory harness and raw captures stay
local. No gameplay changes, rebuild or post-acceptance launch; existing framework
changes and dependency pins preserved. No next job started.

Launch: `python3 recomp/tools/local/run.py`.
