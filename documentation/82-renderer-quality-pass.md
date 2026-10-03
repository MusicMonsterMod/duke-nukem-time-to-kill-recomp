# D17A/B renderer quality pass - 2026-10-03

Historical engineering record. D17A and D17B were explicitly accepted on
2026-10-03; [note 86](86-d17-acceptance-and-regression-baseline.md) supersedes
the parent-job statuses and source-publication policy below.

This is a stabilization candidate, not acceptance of consistent 180 Hz in the
strip club or campaign-wide visual correctness. The user's playable build is
the comparison baseline. D17 remains accepted; D17C's accepted idle behavior
and the moving PS1 visual character were not redesigned.

## Systemic findings and changes

### Replay must leave live GPU surfaces intact

The expanded isolation checker hashes color, depth and stencil in the canonical
surface and every wide surface before and after a replay. The previous check
covered less state. It exposed writes outside the saved display band: the
replay center blit copied the full VRAM height while copy-on-write saved only
the displayed band. The replay blit now respects that band. The old code
produced 5 mismatches in 143 checks in one run; a post-fix run produced zero
in 126. The diagnosed mismatches were wide-surface color, not texture-cache
changes or proof of depth corruption in those particular captures.

Related depth-state defects found in review were fixed at the same boundary:

- Copy-on-write's backup blit disables scissoring. The depth clear now restores
  scissoring before clearing the requested area.
- The full-surface fallback now backs up/restores depth as well as color and
  stencil, matching the copy-on-write path's contract.
- Restoring the GPU snapshot reapplies the draw area, which marks a depth clear
  pending. The saved pending-clear state is now restored after that operation,
  so resuming live rendering does not accidentally begin another depth frame.

These are renderer-state fixes, not coordinate or object exceptions.

### Coherent camera, visibility and precise vertex provenance

The guest camera render routine derives the eye, rebuilds portal visibility,
then renders rooms and actors on each replay. This is not a design that simply
reuses the last live visibility list. Native actor draw paths load their
substituted matrix before their depth/frustum test. Some room membership and
object bounds remain discrete snapshot state; full coherence across every
object transition has not been proved.

For the existing special room-walk path, the eye is now derived from the
interpolated camera rotation/anchor before the room walk, using the verified
native routine at 0x80039c7c. Previously that walk could see a linearly
interpolated eye before composition recomputed it. Transform lookup now searches
the whole keyed substitution table, wrapping from its cursor; changed portal
traversal order no longer makes an otherwise available transform unfindable.

The runtime already had separate live and replay precise-vertex tables. A new
replay epoch check rejects precise UV/depth entries left over from an earlier
replay even if packet addresses and integer coordinates match. No stale-entry
hits have been observed in the measured route, so this is a defensive fix,
not an established explanation for the user's flicker.

### Scheduling, batching and deadline tails

Unlimited initially requests 1000 presents/s. The old 15-job per-game-frame
planning limit was exhausted during long native frames, after which the same
image was held. The planning capacity is now 96; the worker's actual outstanding
job limit remains 16. Native four-field frames are recognized rather than
rejected by a 60 ms interval bound. Display/Unlimited's already-paced rate is
no longer divided a second time when calculating interpolation coverage.
Unlimited may adapt through divisors up to 16, rather than stop at three.
Fixed/display targets still use divisors up to three.

Worker readiness now uses the recent 64-sample p95 instead of an average.
The camera sampling lead is bounded and changes gradually to avoid sudden
sampling-time jumps. This trades some input latency for fewer deadline misses;
it does not establish immediate mouse response under load.

A prefetch-only poll no longer flushes live GL batches every 0.5 ms. Actual
replay entry and due presentation retain the required flushes. Disabling the
existing whole-field GPU timing queries did not materially improve pacing in
an A/B check, so that behavior was left unchanged.

## What profiling establishes

At 4x internal resolution, CPU 100%, first-person dancers viewpoint:

- Worker redraws in a 1,934-sample instrumented run: median 12.14 ms,
  p95 21.344 ms, p99 22.744 ms. Actor rendering: median 4.526 ms,
  p95 8.277 ms, p99 8.563 ms; typically 49 actor visits and three rooms.
- Worker sampling finds guest dispatch and cycle/cache accounting prominent
  (about 10.5% entry lookup, 6.1% cycle step, 5.6% instruction-cache work,
  5.1% loads and 5.1% dispatch). Actors matter but are not the whole cost.
- A matched repeat of the original executable also settled at 60 presents/s
  and about 57 distinct images/s in the club. Earlier runs reached 90.
  A single before/after FPS reading would therefore be misleading.
- The candidate also varies between approximately 60 and 90 presents/s at
  the 180 target in these runs. This pass has not demonstrated a large CPU
  throughput improvement or a stable 180 distinct images/s club.

Final real-display samples (eight seconds per phase, 4x, CPU 100%, private
club dancer state):

| Target / motion | Presents/s | Distinct/s | Present p99 ms | Replay CPU p50/p99 ms | GPU span p50/p99 ms | Swap CPU p99 ms |
| --- | ---: | ---: | ---: | --- | --- | ---: |
| 180 / idle | 90.1 | 86.6 | 13.07 | 1.65 / 2.79 | 0.92 / 10.65 | 0.164 |
| 180 / turn | 89.9 | 85.4 | 12.99 | 1.64 / 2.87 | 1.20 / 9.69 | 0.184 |
| Unlimited / idle | 129.7 | 122.0 | 9.90 | 1.60 / 2.82 | 0.93 / 7.57 | 0.162 |
| Unlimited / turn | 108.0 | 100.3 | 11.88 | 1.68 / 2.87 | 1.09 / 9.06 | 0.151 |

Presentation quantiles cover the last 600 presents; replay and swap quantiles
cover the last 256 samples. These windows differ from the eight-second count
interval and include adaptation. The native game produced about 20 frames/s.
Emulation-thread wall time averaged about 16.5 ms per field, including GL work.
No GPU timing queries were dropped. Short swap-call tails do not exclude
compositor/display queuing. Three worker cancellations/failures were already
present before the 180 run's measurement windows; their counters did not
increase during either measured phase.

A final real X11 mouse sweep (`holds.py`, 12 s, PX=1, STEP_S=0.001,
Match Display, 4x) did not establish a responsiveness improvement. Candidate:
933 traced presents, 193 held by the heuristic, 1.0 runs of two-or-more held
presents/s, longest 1,300 ms, p99 presentation interval 18.638 ms. Original
baseline under the same script: 846 presents, 230 held, 0.8 runs/s, longest
1,984 ms, p99 18.639 ms. Both had two pacing changes. This heuristic treats
less than 0.05 degrees of yaw change as held and includes mouse sweep reversals;
it is not proof of a frozen identical image for those durations. Keep the
long low-motion runs open and capture input events plus snapshot-generation
and presented-image identity together before assigning their cause. One
short A/B pair is insufficient to establish regression or improvement.

`frame_perf` emulation-thread wall time includes GL work on that thread; it is
not a separate pure CPU rendering measurement. Its whole-field GPU query can
include command-stream bubbles and must not be interpreted as GPU utilization.
New opt-in `PSX_REPLAY_PROFILE=1` uses nonblocking timestamp queries around
individual replays, records CPU replay wall time and swap-call wall time, and
exports p50/p95/p99/max through `render_replay.quality`. GPU timestamps still
include bubbles. Swap-call duration is not scanout or mouse-to-photon latency.
Worker logging uses `DNTTK_REPLAY_PROFILE=1`; process sampling uses
`PSX_PROF_WORKERS=/tmp/prefix`. These diagnostics are off by default.

## Regression evidence and limits

Private savestate/card copies only; player preferences, cards and retail media
were not modified. The source and binary baseline were retained locally.

- Final 2x surface isolation: zero mismatches in 227 copy-on-write checks
  and zero in 232 full-surface fallback checks across doorway/closet loads.
  The first multi-slot script timed out because hashing slows emulation; a
  bounded reload route replaced its real-time frame-wait assumption.
- Native near-plane, controls, aim and input tests pass. The input test needed
  its missing `hold_button_sampled()` stub to link against the existing input
  implementation. The owned-image Python suite passes all 98 tests.
- Gameplay RAM/MMIO equivalence at 60 versus 180: no-input route 630/630
  aligned frames identical. Driven route 629/631 identical: RAM differs at
  frames 43-44, MMIO identical throughout, then RAM converges again. Repeated
  driven 60 versus 60 is 631/631 exact. The synthetic driver has an earlier
  documented frame-43 pad discrepancy, but this run's differing addresses
  were not traced. Keep the driven result open; do not call it full gameplay
  equivalence or infer that convergence proves harmlessness.
- Vanilla fresh-card route `d17-quality-vanilla`: exit 0; intro, title,
  gameplay, weapons, inventory and movement captures reviewed as normal.
  Compiled FMV decoder active. This was a headless route, not an audible
  audio-output or full-campaign test.
- Endpoint diagnostic (interpolation and late camera disabled): 409 of 412
  comparisons pixel-exact across eight viewpoints. Three fresh-street redraws
  differed during turning. Inspection shows a changed camera image, not a
  local hole; capture labels do not identify snapshot generation, so these
  remain unclassified, not a clean endpoint pass.
- Normal interpolation: 24 sequences of 24 presented images across the club
  doorway, statue, apartment, closet, subway and street. No back-and-forth
  image jumps detected by the existing image-motion check. Contact-sheet
  inspection found no obvious large holes. This does not rule out individual
  triangle, texture-edge or intermittent object failures.

Artifacts and repeatable scripts are local under
`recomp/analysis/d17-high-refresh/`: `quality.py`, `quality_regress.py`,
`quality_final.py`, `quality-*.json`, logs and `shots/`. Capture dumps and
surface hashing stall rendering; their frame pacing is not performance evidence.

## Remaining work

The key remaining performance problem is worker CPU tail latency and scheduling
under contention. Optimize that path with profiles before replacing guest
cycle/cache behavior or adding GPU reprojection. An actual display/mouse
latency measurement is still required; scripted turning and present timestamps
do not establish mouse-to-photon response. Visibility transitions, transparency,
overlap/depth ordering and precision around intersections still need broader
campaign testing. The three unmatched endpoint captures need generation-tagged
capture diagnostics if they reproduce as a visible hitch.

This pass does not establish that all reported texture flicker has one cause.
It establishes and fixes a live-surface isolation defect, removes several
inconsistent-state opportunities and improves stress-test observability.

## Build and launch

Runtime patch regenerated with 19 files; reverse-apply check passes against
the current runtime. No generated recompilation C was edited. The build keeps
the compiled FMV overlay. Source and player executable remain local; no commit
or push was made.

Executable SHA-256:
`a81d2d2869ee0fcd116b741a34984cf7e6e6b189de6431362966d7100263df41`.

Launch the candidate using the player's saved preferences:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Use F10 to capture the mouse if necessary. Playtest fixed/Match Display as well
as Unlimited, stationary and turning in the club, and room transitions in the
known problem saves. The player build is not left running after tests.
