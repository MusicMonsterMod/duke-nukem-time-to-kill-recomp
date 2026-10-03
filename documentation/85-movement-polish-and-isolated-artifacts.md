# Movement polish and isolated-artifact follow-up

Historical engineering record. D17A and D17B were explicitly accepted on
2026-10-03; [note 86](86-d17-acceptance-and-regression-baseline.md) supersedes
the parent-job statuses and source-publication policy below.

2026-10-03. The user accepts build `28554051...3dbe264` as a major
playability milestone: smooth opening, enjoyable club, responsive apartment,
and substantially improved closets/subway walls. Preserve that baseline.
This pass investigates remaining movement/audio polish without expanding into
isolated geometry fixes or changing the rendering architecture.

## Reproduction and boundaries

Latest player states were copied into private `cards-user8/openbios`.
UI N is file N-1. Slot 12 is now the club forward/backward reproduction;
slot 10 is now the train-platform/ledge area, not the previous late club.
Slots 1/2 retain construction-sign and train-control cases. Slots 3, 6, 9
and 11 are visual regression/backlog cases. Original state hashes/mtimes and
the accepted executable/source are preserved in `polish-baseline`.

`polish_motion.py` delivers real X11 key events: slot 12 alternates Shift+W
and Shift+S every 1.2 seconds; slots 1/10 turn around and alternate movement,
jumping and firing. Position samples verify that Duke actually moves. Runs
use private god mode to prevent death invalidating comparisons, CPU 100%,
first person, 4x, 180 Hz. Audio uses a realtime dummy sink; no actual-device
listening claim is made. The slot-2 sample is a short local interaction/movement
route, not proof of the full switch/train/pig-cop sequence.

An initial harness retained its synthetic key override, so real W/S events
were cleared. Those samples are excluded from movement evidence. One subsequent
launch collision interrupted an already private test; that incomplete sample
is also excluded. Final tests run sequentially with a started-process guard.
PNG capture and startup are outside the reported window (first two seconds
excluded). Sampling runs are bounded and dynamic actors mean they are not
bit-identical workloads.

## Slot 12: presentation is not the whole movement story

The actual Shift+W/S baseline maintains presentation p99 about 6.00 ms, yet
about 45% of traced intermediate frames are at interpolation alpha 1.0.
This is an endpoint-saturation diagnostic, not a direct count of perceptually
bad frames: stationary poses and identical endpoints can also clamp harmlessly.
Duke is moving in the captured route, however, and the exhausted history
provides a mechanism for intermittent translation holds despite regular presents.
No camera cuts, failed workers, worker replacement or clipping-budget exhaustion
was identified as the cause.

Opt-in capture/completion tracing shows roughly 67 ms median pose intervals.
In the short slot-12 readiness run, composition takes about 38 ms median and
43 ms p95 from capture to complete transform availability. Pose-clock phase
lag adds to this. The scheduler prepares redraws ahead of their presentations,
but interpolation requires two complete, coherent snapshots. The 90 ms world
buffer can therefore run out of usable future history before the next snapshot
is ready. Late mouse rotation remains independent of that world translation.

Two bounded experiments were rejected:

- Increasing world delay from 90 to 130 ms still left about one third of
  intermediate frames at an endpoint and added 40 ms of movement history delay.
- Faster phase correction reduced median clock misalignment but did not reduce
  endpoint saturation (about 47% in that run). It was removed from the source.

Neither experiment is shipped. The current 90 ms buffer, pose-clock correction,
mouse sampling, geometry/culling, interpolation and physics remain as accepted.
Further work should measure how to make coherent pose/transform pairs available
earlier, or prepare them closer to their useful presentation, before considering
latency-increasing buffering or speculative geometry. Slot 12 is not declared
fixed, and the new counters do not establish full input-to-photon latency.

## Audio and smaller stutters

The slot-1 construction-sign route includes verified movement, jumping and
weapon firing. Its baseline sample retains about 59.96 source fields/s and
180 distinct images/s, presentation p99 6.27 ms, with zero output underruns
in the measured window. This does not rule out a rare crackle, clipping in the
source mix, a different ledge path or behavior on the player's audio device.
No audio algorithm or buffer size was changed.

The short slot-2 sample retains about 59.97 fields/s, 179.33 images/s and
presentation p99 5.99 ms, with zero output underruns. The full moving-train
encounter remains a regression route for continued playtesting.

Slot 10 does reproduce source-time pressure and output starvation under the
scripted movement/jump/fire stress: about 57.29 fields/s despite 175.86 distinct
images/s and a 6.06 ms presentation p99. The output counter advances by 29,253
missing sample frames after the first two seconds. This counter counts audio
sample frames, not 29,253 audible crackles. Slots 10/12 can recover during a
run as workload changes. This is evidence of insufficient timely audio
production in these stressed runs, not proof that the user's slot-1 crackle
has the same cause. Existing audio safeguards remain intact.

## Small retained change

Ordinary `run.py` launches no longer enable per-batch OpenGL timer queries.
These diagnostics issue timestamp/query work throughout rendering and are not
needed to display the game. The runtime and renderer are unchanged by this
launcher setting. `PSX_GL_PERF=1` explicitly restores them; an explicit 0/1 is
always preserved. `PSX_REPLAY_PROFILE=1` enables them by default for profiling.

Paired diagnostic-off runs reduced observed source starvation in slots 10/12,
but run-to-run scene variation is material and some underruns remain. Do not
claim this change solves the movement or audio reports. It removes unnecessary
normal-play diagnostic work without changing primitive submission, blending,
visibility, audio processing or frame-rate selection.

The only title-source addition is opt-in `DNTTK_POSE_TRACE=1`, reporting capture
and composition-completion timestamps plus eye position. With the variable
absent, no trace is emitted. The accepted renderer behavior is preserved.

## Isolated visuals: separate D17D backlog

D17D is now Todo on the job board. Slot 9's lower wooden board is fragmented
in both original and intermediate captures. Near-clip budget hits, copy
overflows, packet skips and mesh fallbacks are zero in the inspected sample.
This points investigation toward local polygon clipping, depth/order or surface
intersection rather than texture streaming. Exact primitive provenance remains
unverified; no coordinate patch, culling change or mesh modification was made.

Retain slot 3's peripheral subway route and slots 6/11 closet edges in D17D's
regression set. The user's substantial visual improvement is recorded as
acceptance evidence for the baseline, not proof that every isolated artifact
is gone. D17D must preserve that baseline when addressed separately.

## Verification and delivery

The existing 100-test Python suite passes. Incremental build succeeds.
Final runs use ordinary-launch GPU diagnostics defaults. All figures exclude
the first two seconds; source/audio rates use the sampled window. The two
mouse sweeps deliver real 500 Hz input. Movement routes use repeated real keys.

| Case | Source fields/s | Distinct images/s | Present p99 / max ms | Missing output sample frames |
| --- | ---: | ---: | ---: | ---: |
| Slot 12 Shift+W/S | 58.51 | 175.95 | 5.98 / 12.54 | 16,225 |
| Slot 10 move/jump/fire | 59.27 | 177.99 | 6.01 / 6.60 | 651 |
| Slot 1 move/jump/fire | 59.96 | 179.97 | 6.30 / 7.94 | 0 |
| Slot 4 opening mouse | 59.95 | 180.02 | 6.03 / 7.75 | 0 |
| Slot 5 dancers mouse | 59.94 | 178.95 | 6.00 / 7.13 | 0 |

The slot-12 endpoint share varies materially between runs and reaches 65%
in this final sample. It remains unresolved. The source/audio results must
not be described as uniformly realtime or crackle-free. The opening and club
mouse sweeps have no adjacent camera turns over one degree (max 0.62/0.72),
with three workers and no worker failures. This preserves the tested mouse
baseline but does not accept all translation or audio behavior.

The furniture defect is also present in the inspected 60 Hz full-width image,
so it is not exclusive to intermediate high-refresh images. The final subway
left-wall route remains visually intact in the inspected capture; user-reported
peripheral improvement is retained, without declaring all culling fixed.
All twelve original player save hashes and mtimes match. Root/title whitespace
checks pass. No game is left running. Original media/generated C/player state were not edited. Codegen remains
`8bab543c`; no save-format change.

Executable SHA-256:
`1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
