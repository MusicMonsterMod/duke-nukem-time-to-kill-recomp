# Opening and strip-club responsiveness

Historical engineering record. D17A and D17B were explicitly accepted on
2026-10-03; [note 86](86-d17-acceptance-and-regression-baseline.md) supersedes
the parent-job statuses and source-publication policy below.

2026-10-03. D17B stabilization candidate, awaiting the user's full playtest.
This supersedes the performance findings in note 82. The earlier geometry
fixes and D17C idle-eye behavior are retained.

## Scope and configuration

Primary route: a completely new game, opening street, then the dancers.
Comparison: alley/apartment. Private copies of UI slots 8-11 cover the new
closet/furniture reports. All tests use isolated settings/cards and a private
debug port; player settings, saves, cards and original media are untouched.

Unless stated otherwise: first person, 4x internal resolution, emulated CPU
100%, real X11 display, 180 Hz target. This matches the player's CPU setting.
Audio uses the realtime dummy callback and underrun counters, not an audible
listening test. Mouse timings measure X11 event injection to the first traced
camera presentation, not input-to-photon latency.

Build: `recomp/build-local/Duke_Nukem__Time_to_Kill_Recompiled`.
SHA-256: `ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
Codegen identity: `8bab543c`. Nothing committed or pushed.

## Systemic findings and changes

### Scene cost and presentation scheduling

The original worker samples cost median/p95/p99 11.796/13.339/20.400 ms at the
dancers versus 2.281/4.019/4.301 ms in the apartment. Actor rendering was
4.415/5.057/7.965 ms versus 0.548/0.955/1.037 ms. The bad view visited 49 actors
and three rooms; the good view visited 14 actors and one room. This is additional
rendering/visibility work, not simply a mouse-sensitivity difference.

The previous scheduler predicted native framebuffer changes. Wrong predictions
and discarded jobs could halve 180 Hz to 90 Hz. A separate moving-average idle
budget refused ready redraws during startup, sometimes leaving only 40-70
camera updates/s while the presentation counter remained 180.

The new timeline selects consecutive completed pose snapshots by timestamp.
It retains eight snapshots and a 90 ms world/actor interpolation buffer, with
bounded clock-phase correction and no extrapolation. Camera rotation is sampled
near the presentation deadline, separately from that world-history delay. Pause
and inventory suspend the timeline; resume/load resets its history. One job owns
each presentation deadline even if a new snapshot changes the applicable pair.
Lead follows p99 worker readiness. Fixed targets retain their cadence; Unlimited
can adapt. Past average idle time no longer suppresses fixed-rate redraws.

The three workers generate frozen draw streams. GL submission and presentation
still run on the main/emulation thread; this is not a new independent GL thread.

### Native code and restored-state coherence

The game changes ADD to ADDU at `0x800B3DBC` and `0x800B3DC0`, inside frequently
used math routine `0x800B3D9C`. Fresh games correctly rejected the original
compiled body and interpreted it. Save restoration, however, cleared the
modified-text bitmap after copying RAM. The clean-page shortcut could then
accept stale original code. Worker image loads also lacked code-cache/guard
invalidation. Loaded scenes were therefore an unreliable proxy for fresh games.

Restoration now reconstructs modified-text tracking against immutable original
text. Every worker image load invalidates native/interpreter validation state.
The initialized 192-byte math routine is compiled through the normal overlay
compiler from a guarded owned-EXE recipe. Modified static-text pages can discover
that cached variant, but execution still requires the exact live code identity.
No original media or generated C is hand edited, and no byte guard is weakened.
The known render dependency is prepared before gameplay workers start.

The same-state diagnostic was extended to complete modified static code to its
return address rather than comparing one interpreted instruction with a whole
native function. Its replay clock now isolates deferred cycle accumulators.
GTE and multiply/divide deadlines include deferred instruction cycles instead
of a stale published counter. At CPU 100%, 37,748 native/interpreter math
comparisons passed, with zero divergences and one unsafe trace skipped.
Overclock compression is outside that diagnostic's replay model.

### Worker lifecycle and startup hitches

A late opening hitch survived the throughput fixes. The worker dependency
counter combined newly mapped DLLs with invalidated cached movie functions.
Retiring another movie function after gameplay began synchronously replaced
all workers. Only two DLLs existed while this combined counter reached 16.

Workers now refresh for newly mapped host libraries and mod-memory allocations.
Guest-code invalidation is handled by the corrected per-image code guards.
Workers first start in gameplay, avoiding unnecessary intro/menu forks.

There was also a timeout publication race. RUNNING was visible before the new
start timestamp, so a reader could use an old timestamp or subtract a start
newer than its cached clock sample. Worker identity/start are now atomic,
submission clears the old start, and timeout checks exclude unset/future starts.
The final cold run has no worker kills, deaths or failed jobs.

### CPU and GL submission

- Cache immutable compiled-entry lookups, including misses. Full key checks
  handle hash collisions; live code-validity checks remain separate.
- Skip timing-only instruction/cache accounting in frozen render workers.
  Live emulation remains timed. Complete GP0 comparisons check the result.
- Build the player with O3 while retaining debug symbols and the debug server;
  inline small constant-mask timing helpers without changing their algorithm.
- Batch compatible single-pass translucent triangles in their original order.
  Destination-mask and subtractive/two-pass cases remain isolated. Redundant
  wide-target/draw-offset assignments no longer flush this path.
- Honor the existing quiet-player setting for large optional forensic rings.

Removing the idle-budget gate alone caused audio starvation. Batching reduced
opening redraw submission from about 2.3 ms to 1.24 ms median and textured
batches from about 305 to 15 per guest field, but it alone was insufficient.
The code-state/native-path and lifecycle fixes were also required.

## Measurements

The bounded steady-scene comparisons at CPU 100% were:

| Measurement | Dancers | Apartment |
| --- | --- | --- |
| Distinct images/s, turning | 179.9 | 179.8 |
| Presentation p99 | 6.260 ms | 5.982 ms |
| Worker average | 8.098 ms | 2.114 ms |
| Replay CPU submission median/p99 | 1.499/1.657 ms | 0.735/0.839 ms |
| Swap-call CPU p99 | 0.205 ms | 0.134 ms |
| GPU query span median/p99 | 1.055/4.785 ms | 0.768/4.816 ms |
| Audio underruns | 0 | 0 |
| Mouse injection to camera presentation median | 17.85 ms | 12.48 ms |
| Mouse injection to camera presentation maximum | 24.82 ms | 15.48 ms |

Mouse results are twelve alternating steps per scene. GPU query spans include
command-stream gaps, and `frame_perf.emu_cpu_ms` includes pacing waits; neither
is a pure utilization measurement. Earlier CPU profiles identified substantial
driver submission and instruction-timing costs, not a generic input delay.

Final complete fresh boot: `response-cold-stable`. From the first valid redraw
onward, presentation median/p95/p99/max was 5.6/5.8/6.0/8.3 ms. There were no
intervals over 10 ms, including the first second. Steady camera delivery was
about 180 distinct images/s with realtime emulation, zero audio underruns and
zero failed/killed workers. This removes the preceding candidate's first-second
28.9 and 15.3 ms hitches. Nine total worker spawns include the initial group and
two early mod-memory refreshes before steady redraws.

Unlimited retains an adaptive cadence and is not claimed to be uniformly
smoother or faster than fixed 180. The final CPU-100% stress run retained
59.89 emulation fields/s and zero audio underruns, with 186.8 idle and 169.2
turning distinct images/s, presentation p99 6.35/6.41 ms, and zero failed/killed
workers. The final Match Display club repeat delivered 179.5 distinct images/s
while turning, 59.97 emulation fields/s, p99 6.18 ms, maximum 7.21 ms and zero
audio underruns or worker failures. `response-match-final` and
`response-unlimited-final` record those final-binary runs.

Two earlier `response-final-*` saved-scene runs accidentally inherited the
harness's CPU 150%, unlike the explicit 100% cold route. They showed remaining
overclock pressure and are not the stock-CPU acceptance comparison. The harness
now defaults comparisons explicitly to 100%; player preferences were not changed.

## Verification and limits

- Final complete worker timing comparison: 2,115 completed jobs across dancers
  and UI slots 8-11, zero failed jobs or differing GP0 streams.
- At least 4,224 full RGBA batching comparisons across those views, zero
  differing images. These diagnostics deliberately duplicate work and are
  separate from performance runs.
- Pause/resume and repeated loads pass. Private copies of old player states
  load; deliberately wrong codegen and ABI headers are rejected.
- Compiled tests cover modified-text restoration, native eligibility and
  preparation, batched pipeline deadlines, GTE serialization, replay-clock
  isolation and worker timeout boundaries. Continuation and pre-init loader
  guards pass; the generated lookup matches a linear oracle over 100,000 queries.
- Native control/aim/input/near-clip tests passed; the Python suite passed 98
  tests. Runtime and generator patch reverse-application checks pass.
- No-input 60/180 runs matched full RAM at all 543 common guest-cycle boundaries
  after a second load. Cumulative MMIO/write hashes include different pre-reload
  histories and are not claimed to match.
- Vanilla fresh-card intro/title/new-game/movement/inventory route completed;
  intro and gameplay captures were inspected. A final third-person club smoke
  check held realtime emulation, p99 6.40 ms and zero audio underruns or worker
  failures; its capture was inspected. Audio still needs human listening.

The old structural `test_interpreter_perf_guards.py` expects the obsolete
`g_psx_cycle_fast_limit` MMIO invalidation sequence. It also fails on retained
pre-pass source and is not counted as passing.

This does not establish full-campaign fidelity. The user's minor club furniture
and closet polygon/texture pops remain open under D17A; loading/capturing those
views does not prove their temporal artifacts are gone. No object-coordinate
workarounds were added. The stable idle-eye baseline and PS1 movement character
are preserved. The next acceptance step is the user's fresh-game full playtest.

## Reproduction and local evidence

Use `python3 recomp/tools/local/run.py` for the player build. The player's saved
Match Display, first-person and CPU-100% settings are unchanged.

Local evidence is under `recomp/analysis/d17-high-refresh/`:
`response-cold-stable`, `response-p99-club100`,
`response-deadline-apartment100`, `response-mouse-club`,
`response-mouse-apartment`, `response-regress`, and the timing/batching logs.
UI slots 8-11 are private zero-based files 7-10 in `cards-user6`.
`response_cold.py` boots isolated fresh cards; `quality.py` compares idle/turning;
`response_mouse.py` injects real X11 events. The pre-pass binary and source
copies remain in `responsiveness-baseline`.

See [10-tooling.md](10-tooling.md) for comparison switches and rebuild details.
