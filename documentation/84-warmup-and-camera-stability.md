# Save-driven warm-up and camera stability pass

Historical engineering record. D17A and D17B were explicitly accepted on
2026-10-03; [note 86](86-d17-acceptance-and-regression-baseline.md) supersedes
the parent-job statuses and source-publication policy below.

2026-10-03. D17A/B quality pass following the user's next playtest.
The improved opening, playable apartment, stable idle eye and FMVs from note 83
are the baseline. This investigation measures camera motion as well as presents.

## Reproduction

The latest player states were copied to the private `cards-user7` directory.
UI slots 1/2 are slowdown cases, 3 is the subway peripheral-wall report, 4 is
the accepted opening comparison, 5 is the early club, 8 the minor slowdown,
and 10 the later club. These replace the meanings of those slot numbers in
older reports. Player saves, cards and settings were not used as writable tests.

`warmup_probe.py` starts a new process per state and sweeps real X11 mouse
input, reversing every two seconds. Final comparisons inject one count every
2 ms, use first person, 4x resolution, CPU 100% and 180 Hz unless specified.
Long runs last 65 seconds. The private process sets the original invulnerability
flag to keep death from invalidating a long performance comparison; enemies
and their processing remain active. No cheat is persisted to player state.

The initial synthetic-input experiment died during its long club observation.
It is not evidence that the renderer warmed up. Synthetic input also bypasses
the real event timestamp history; it is unsuitable for accepting mouse feel.
The new harness records input/camera traces, source fields, audio counters,
worker lifecycle, CPU phases, GL batch counts and asynchronous GPU timing.

## Causes and changes

### A slow game frame incorrectly reset the camera

`orbit_begin` considered a gap over four guest fields to be lost camera
ownership. The dancers legitimately produced five-field updates. Even with
180 evenly spaced presentations, those updates reseeded yaw from an older
original-game camera and reset accumulated mouse motion. Diagnostics reproduce
`gap=5 valid=1 epoch=3/3` repeatedly in the early club.

The camera now preserves its look across slow updates. Explicit input epochs,
savestate loads and camera-lease releases still invalidate it. Native callback
regression tests cover five-, eight- and twelve-field gaps against a deliberately
stale game camera, verifying that the full requested mouse turn survives.
Existing recapture/state-ownership tests also pass.

### Late redraws were discarded before their cost could be measured

The scheduler dropped unfinished work immediately after its deadline. Slow
results were missing from the readiness estimator, which then continued using
a lead suitable for the cheaper preceding view. A measured failure repeated
one camera image until its sample was 114 ms old, then jumped to current input.
The presentation intervals alone still looked healthy.

Just-late results now remain eligible for two presentation intervals. Expired
unfinished work contributes its elapsed time as a lower bound to the readiness
estimate before release. This bounds the queue while allowing a slightly late
image to become the next useful camera update instead of being discarded.

A 65-second, 500 Hz mouse-input comparison during development found:

| Camera measurement | Before camera/scheduler repair | After both |
| --- | ---: | ---: |
| Adjacent camera turns over 1 degree | 54 | 0 |
| Largest adjacent turn | 7.66 degrees | 0.79 degrees |
| Worst traced camera-sample age | 31.06 ms | 25.53 ms |
| Presentation interval p99 | 6.08 ms | 6.05 ms |

The intermediate camera-only build still had a 114.32 ms stale-image episode
and two turns over one degree. Adding the scheduler repair removed those in
the measured 65-second run.

Both runs already contained the batching/timestamp fixes and used the same
sweep protocol. This isolates a defect that average FPS could not describe.
These are software trace measurements, not input-to-photon latency.

### Opaque/translucent boundaries fragmented GL submission

The slowdown states alternated opaque and translucent primitives, splitting
otherwise compatible draw streams into hundreds of batches. Main-thread CPU
sampling in slot 2 attributed 29% to the NVIDIA GL driver, including draw,
framebuffer switching and timer-query work. Workers were comparatively cheap.
At 60 Hz without replay, the same state retained realtime emulation.

The title's ordered dual-source path now admits opaque primitives too. Their
per-primitive blend code produces source * 1 + destination * 0; translucent
primitives retain their existing factors and original submission order.
Destination masking and subtractive/two-pass draws remain isolated. This is
active only with the title's existing high-refresh ordered-batching opt-in.
Vanilla and the conservative comparison path remain available.

Initial slot-2 comparison restored about 49.1 to 59.9 emulation fields/s and
removed measured audio underruns. The ending batch window fell from about 315
to 69 batches/field; redraw submission p99 fell from 6.61 to 3.12 ms.
The full RGBA oracle compared over 3,200 redraws across all seven supplied
states without differing pixels. This diagnostic duplicates work and is not
used as a performance measurement.

Frame GPU profiling now checks completion before reading query results. Its
statistics can skip an unfinished sample instead of synchronously waiting for
the GPU on the game thread.

### Mouse event timestamps and first-use allocations

The pinned SDL3 X11 raw-motion code passed the server's millisecond timestamp
directly to SDL's nanosecond event API. Its other X11 timestamp helper used
pump time, losing report spacing when events arrived in batches. The fetched
source now uses one wrapping-server-clock conversion into SDL nanoseconds.
The configure-time repair is idempotent and rejects unrecognized source; it
does not edit an installed SDL library. Compiled tests check queued events,
32-bit wrap, older reports and fail-closed application.

Near-clip scratch, first-person/kick matrices and aiming scratch are reserved
before workers start.
Their first use previously changed the mod-memory watermark and synchronously
replaced the worker group. The club comparison now starts three workers once,
versus nine spawns previously, with no failed jobs or worker restarts.

## Warm-up interpretation

Long baseline observations did not show continuing native-library loads or
framebuffer creation after startup. Slot 8 did not reproduce a minute-long
monotonic reduction in worker cost. The early and late club saves also contain
different views/workloads: the first observation drew 62 actors/objects in
slot 5 versus 47 in slot 10. They are useful gameplay comparisons, not identical
cache-state experiments.

The observed camera timeout and censored readiness estimate explain why the
same input path could feel worse in an expensive view and improve when its
workload changed. First-use scratch allocations were a separate measured hitch
and were moved earlier. This does not establish caching as the cause of every
reported self-healing slowdown, nor claim that a saved-state launch reproduces
every first-visit transition of a complete campaign session.

## Subway and remaining visual work

A full-width capture route loads slot 3, levels the view, approaches the left
wall and walks along it. The inspected 60 and 180 Hz routes did not reproduce a missing
wall section. Near-clip budgets, packet skips and mesh fallbacks stayed clear
on that route. Full-width compositor captures were used because the canonical
512-pixel screenshot omits the very margins under investigation.

The peripheral culling report remains open. No coordinate-specific workaround,
blanket culling disable, extra geometry smoothing or PS1-art replacement was
added. The closet/furniture temporal artifacts from earlier reports also remain
open. A clean bounded capture does not establish full-campaign visibility.

## Verification and delivery

Final saved-state sweeps use 500 Hz real mouse events. The table omits the
first second after state restoration; slots 8/10 run 65 seconds, others 25.

| UI slot | Source fields/s | Distinct images/s | Present p99 / max, ms | Audio underruns |
| --- | ---: | ---: | ---: | ---: |
| 1 | 59.90 | 179.58 | 6.35 / 13.45 | 0 |
| 2 | 60.00 | 179.69 | 6.15 / 7.32 | 0 |
| 4 | 59.93 | 179.80 | 6.01 / 7.53 | 0 |
| 5 | 59.90 | 179.30 | 6.00 / 7.14 | 0 |
| 8 | 59.94 | 179.55 | 6.02 / 9.47 | 0 |
| 10 | 59.96 | 179.74 | 6.06 / 7.26 | 0 |

No adjacent traced camera turn exceeded one degree in these final sweeps.
Rare host presentation outliers remain, including the 13.45 ms slot-1 sample.
The table was measured before the last incremental scratch reservation: slot 4
revealed a first-kick allocation growing the watermark by 144 bytes. The final
build also reserves those buffers and the known aiming/interaction scratch.
Repeated final-build slot 4/5 sweeps start exactly three workers, with no deaths,
kills or failed jobs, and no adjacent turn over one degree.

A fresh boot through the movie/menu into gameplay has zero audio underruns,
three worker spawns and no failed/killed workers. From its first valid camera
redraw, presentation p99 is 6.07 ms and maximum 7.83 ms. The opening capture
was visually inspected. Audio counters use a realtime dummy output; listening
on the player's actual audio device remains part of playtesting.

Unlimited remains a useful failing stress test, not an accepted smooth mode.
Its final 25-second club run retains 59.94 source fields/s and zero underruns,
but produces about 160.7 distinct images/s, eight traced turns over one degree
and a worst camera-sample age of 114.08 ms. Presentation p99/max are 7.35/8.45 ms
after the first second. Adaptive cadence/deadline pressure remains a follow-up;
the fixed 180 Hz result must not be generalized to Unlimited.

Native controls, input, aim and near-clip tests pass on the final sources,
including the new slow-update camera cases. The Python suite passes 100 tests.
Final-source verification also includes:

- 2,848 reported timing-oracle checks across all seven states, all exact GP0
  streams; 3,056 full-RGBA batching checks, zero differing images.
- Third-person club sweep: 60.01 source fields/s, 179.58 distinct images/s,
  present p99/max 6.00/7.09 ms after the first second, zero audio underruns,
  no adjacent camera turn over one degree and three workers without failures.
- First quick kick and two shots in UI 4 preserve the three-worker group.
  Pause stops redraws; resume restarts them. Repeated loads of UI 5/10/2/4
  complete without failed or replaced workers. An initial single-click test
  did not fire; the verified test holds Mouse1 for 0.5 seconds and checks the
  actual shot counter, rather than assuming delivery.
- Vanilla fresh intro/menu/gameplay/control route exits cleanly. Intro movie
  and opening gameplay captures were inspected. This is bounded compatibility
  evidence, not full-campaign or actual-device audio acceptance.
- Runtime and generator patch reverse checks pass; root, local title and
  framework diff whitespace checks pass. All twelve original player states
  match their recorded SHA-256 and modification times. Game left closed.

D17A/B remain Needs playtest. The candidate improves the measured fixed-180 Hz
baseline substantially; Unlimited scheduling and unreproduced visual reports
remain explicit follow-up work.
Local evidence lives under `recomp/analysis/d17-high-refresh/warmup-*`.
The preserved pre-pass executable/source and original state hashes are under
`warmup-baseline`; the player originals remain unchanged.

Build SHA-256: `28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
Codegen identity remains `8bab543c`; save-header compatibility is unchanged.
No generated recompilation C or original media was hand edited.
