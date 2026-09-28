# Time to Kill movie playback: diagnosis and native overlay

This pass addresses the user's report of slow-motion FMVs and garbled audio in
the Linux SLUS-00583 build. The measured candidate now sustains the NTSC guest
clock with no new audio underruns in two intro tests. This is performance
evidence, not a claim of pixel/sample identity with a PlayStation.

## What actually failed

The earlier windowed intro ran around 40–43 guest frames per wall-clock second
against a 59.94 Hz target. The host continued requesting audio at 44100 samples
per second while the guest produced it too slowly. Audio buffer occupancy
fell to roughly 17 ms against a 180 ms target, and the underrun counter kept
increasing. Improving pictures alone could not fix that starvation.

Most sampled execution time was attributed to resident function `0x800B81BC`.
Disassembly identifies it as the stream ring's `StGetNext` polling routine,
not the variable-length decoder. Its caller at `0x800AB370` retries while
decrementing `s0`; the initial timeout is `0x200000`. The ring index is at
`0x800E811C` and its base pointer at `0x800E8130`. Each descriptor is 32 bytes.
Status 1 wraps the ring; status 2 consumes an available entry. The other-status
path returns 1 without writing memory.

Generic idle skipping did not recognize this cross-function loop. An initial
title hook stopping at every internal device deadline also failed to deliver
full speed. Diagnostics then showed three or four RAM writes during many
47-cycle iterations: ongoing MDEC DMA was writing decoded pixels. A generic
"no RAM writes" condition rejected almost every useful skip even though those
writes did not affect the descriptor being polled.

## Title-specific wait acceleration

`recomp/src/ttk/fmv_poll.c` registers a function-entry hook using the existing
plugin API. `game.local.toml` requests emission of that entry hook. Generated
code is regenerated normally; no hand-edited generated shard is required.

The implementation requires the exact resident caller and complete callee
instruction words, the expected return address, a valid RAM descriptor, and
an empty ring entry. It observes three equal iteration durations, unchanged
CPU register/timing state except the timeout decrement, and no intervening
MMIO. It disables itself for exception execution, precise/lockstep/self-check
and netplay modes.

Before advancing time, it synchronizes devices, rechecks the descriptor and
code, rejects pending interrupts, and checks all active DMA channels. Only
known MDEC/CD sequential transfers may proceed. Their remaining write ranges
must avoid the poll code, caller, ring pointer/index, and current descriptor.
Reverse/wrapping transfers and other active channels reject acceleration.
MDEC input DMA is read-only and may proceed.

Skipped iterations leave the timeout above zero and stop strictly before the
next observable device event. The runtime still services intermediate hardware
events on the guest clock. The following generated instructions handle the
actual interrupt or newly available stream data. The hook does not change XA
sample rates, CD sector cadence, MDEC decoding, or video presentation rate.

The iteration arithmetic and DMA exclusion rules have focused boundary tests,
including 20000 deterministic randomized time-bound checks. These tests do not
replace differential execution against a reference machine. Disable the hook
for comparisons with `DNTTK_FMV_POLL=0`.

## Compiling MOVIE.OVR

The local disc contains `/DATA/MOVIE.OVR;1` at LBA 23959, length 6884 bytes.
Its SHA-256 is
`eacbb6eadd142bc6d28df17b13923722a146355ec12f47df9278ca2c19028e52`.
Runtime RAM observations establish its unrelocated base as `0x800CA968`.
Code occupies `0x800CA9BC` through `0x800CB4AF`; subsequent bytes include
tables and compressed initialization data and are excluded from code discovery.

Nineteen entries are supplied from checked direct calls, function prologues,
and a leaf-routine boundary. In particular, resident calls at `0x800AB330` and
`0x800AB64C` target the hot VLC decoder at `0x800CB07C`. Calls at `0x800AADB4`
and `0x800AB630` target table initialization at `0x800CB3CC`.

`tools/local/build_movie_overlay.py` reads the user's prepared raw disc, checks
the boot EXE and movie hashes, and constructs the offline overlay recipe in
ignored `analysis/movie-overlay/`. It invokes the pinned overlay compiler in
CPS mode with guest cycle accounting. The build reported 19 functions, zero
unsupported-instruction TODOs, and zero unknown/bad targets. The resulting
native shard lives under `build-local/cache/SLUS-00583/` in the runtime's
platform/configuration/ABI namespace. Runtime byte guards control dispatch;
the shard is not installed as an unconditional address override.

The local profile now enables overlay-cache loading. Live auto-compilation
remains off. The loader reported 19 registered candidates and actual native
dispatches. Sampled interpreter time in the movie fell from approximately
25–30% to 3–4%. Other overlays still use interpreter fallback.

## Measurements

These are **guest VBlank rates**, not distinct movie-image rates. The tested
intro decodes approximately 15 movie frames per second at normal guest speed.
Runs use windowed OpenGL on this machine; they are not a controlled hardware
performance comparison across different PCs.

| Candidate | Guest frames/s | Audio result |
|---|---:|---|
| Original measured baseline | about 40 | buffer starvation |
| First polling hook | 43.18 | underruns continue |
| Observable-event hook, generic store guard | 41.68 | underruns continue |
| DMA-disjoint polling | 54.54 | fewer underruns, still too slow |
| Polling plus native movie overlay, 19.37 s | 59.93 | zero new underrun samples |
| Repeated longer run, 65.37 s | 59.96 | zero new underrun samples |

The longer run advanced from guest frame 909 to 4828 and MDEC decode count
128 to 1108. Audio occupancy went from 194.3 to 161.0 ms, without underruns.
Startup overflow-drop counters were nonzero but remained unchanged throughout
both measured movie intervals; this pass does not claim all audio counters
are zero. Host correction remained close to zero instead of staying clamped
at the starvation limit.

Evidence: `reports/fmv-native-overlay.json`, `reports/fmv-native-long.json`,
earlier `fmv-*` reports, and matching build/run logs in `logs/`. The repeated
benchmark starts a fresh process, uses isolated memory cards, and records
guest progress, audio occupancy, MDEC state, execution profile, and loader state.

## Reproduce

From `recomp/`, after building the runtime:

```sh
python3 tools/local/build_movie_overlay.py
python3 tools/local/benchmark_fmv.py --name fmv-check --samples 65
python3 tools/local/run.py
```

The ordinary `tools/local/build.py` now includes the movie-shard build. GCC is
required for that stage. `--skip-movie-overlay` deliberately omits it for
diagnostics. Rebuild the shard after changing runtime ABI/configuration; stale
cache namespaces will not be selected. Linux is the measured platform;
Windows playback remains unverified.

## Remaining fidelity work

The final build also passed a windowed intro/title-to-first-level smoke check.
Controller turning/movement inputs were exercised; local screenshots are in
`recomp/analysis/fmv-baseline/post-fix-*.png`. The gameplay loader report records
14 invalidated movie entries and 2004 blocked stale-dispatch attempts after
overlay replacement, demonstrating that movie code is not blindly retained
over different level code. This is a useful safety observation, not complete
proof of every overlay transition. All 17 local tests passed with owned-disc
integration tests enabled (`DNTTK_IMAGE` set to the prepared raw BIN).

The measured slowdown/audio-starvation issue is addressed in this candidate.
User listening and visual confirmation remain necessary. Next establish
frame-aligned pixel and PCM comparisons against a trusted reference, exercise
every movie and skip/transition path, and complete gameplay/save/load tests.
Do not infer full-game 1:1 behavior from an intro benchmark or native function
count. The root [game manual](../GAME_MANUAL.md) documents the actual controls;
first-person camera work remains a later milestone.
