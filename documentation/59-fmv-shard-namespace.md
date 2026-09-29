# D23B - Intro FMV stutter: stranded native movie shard

## Report

"im still getting the stuttering, but only on the fmv at the beginning?"
(2026-09-29 08:42, after the D23A frame-budget fix). Follow-up: "make the
playback more robust ... this is our port, we dont need to rely ever, on the
tooling of the psx. if we can do it better, we should".

## Cause (measured)

The intro's MDEC/VLC decoder lives in `MOVIE.OVR`, an overlay the game loads
from disc. `tools/local/build_movie_overlay.py` compiles it into a native GCC
shard under `recomp/build-local/cache/SLUS-00583/gcc/linux-x64/`. The runtime
only looks in one folder, named `cg<N>_<codegen hash>_gc<config hash>_f0`.

The config hash is `psxrecomp-game --overlay-config-hash game.local.toml`. It
covers every field that *could* change overlay codegen, including the
`mod_function_entry` hook list. Every modernization job that added a host
hook (the 28 Sep jobs did) moved the namespace. The last shard was built on
27 Sep 19:19 (`gc01bd77ae`); the current config reads `gc15256f69`, which was
empty. The framework loader then runs every overlay function interpreted and
reports the mismatch only through the `overlay_loader_status` debug-port
command, never in stderr or the session log.

The movie hooks themselves live in the resident EXE, so the stranded shard's
code was still correct - the namespace is conservative, not wrong. That is a
framework trade-off left alone here; the port now keeps the shard current.

| State (isolated Xvfb probe, intro FMV, Modernized, OpenGL) | fps | audio underruns | output fill | interpreter share |
| --- | --- | --- | --- | --- |
| Stranded shard (`gc15256f69` empty) | ~52 | continuous | starved | ~30 % |
| Shard rebuilt for the current config | 59.92 | 0 | 267 ms | ~4 % |

The empty-ring poll acceleration (`src/ttk/fmv_poll.c`, doc 13) was working in
both runs. It was not the cause.

## Fix: three layers so it cannot go silent again

1. **Build.** `recomp/CMakeLists.txt` adds an always-run `ttk-movie-shard`
   target after `psx-runtime`, and the `local-dev` build preset now builds
   it. Plain `cmake --build --preset local-dev` (not only `build.py`) keeps the
   shard current. It skips with a message when the prepared disc, recompiler
   or GCC is missing; a real compile failure fails the build.
2. **Launch.** `tools/local/run.py` calls the same check before starting the
   game. That covers a `game.local.toml` hook edit with no rebuild. A failure
   warns and still launches.
3. **Runtime log.** `fmv_poll.c` logs the native-decoder state the first time
   the movie poll runs and whenever it changes:
   - `ttk-fmv: native movie decoder active (19 functions)`
   - `ttk-fmv: WARNING native movie decoder NOT loaded (...) [loader: ...]`,
     which carries the loader's own message (for a namespace problem, the
     `OVERLAY CACHE HASH MISMATCH` line naming both folders).

`build_movie_overlay.py` gained `--if-ready` (skip, exit 0, when
prerequisites are missing) and `--quiet` (one-line result, full compiler
output only on failure). It rewrites `analysis/movie-overlay/capture.json`
only when the recipe changes. Cost: about 0.2 s when current, about 1.4 s
for a rebuild.

## Verification (2026-09-29)

Headless runs with scratch memory cards, about 45 s each:

- Current shard: `movie shard: current`, then `native movie decoder active
  (19 functions)`.
- Shard folder moved aside, binary launched directly: the WARNING line with
  the loader's hash-mismatch text.
- Same state through `run.py`: `movie shard: rebuilt for the current config`,
  then `native movie decoder active (19 functions)`.
- `cmake --build --preset local-dev` prints `movie shard: current` after the
  link.
- `python3 -m unittest discover -s tests/local`: 64 tests OK (2 skipped).

The user confirmed the intro plays smoothly again after the 08:48 rebuild.

## Test-harness fix found on the way

Four launch tests in `tests/local/test_player_profiles.py` patched
`subprocess.call`, but `run.py` launches through `Popen` by default since
the session-log change. Running the suite started the real game against
`saves/local-play/`. Both cards had 0 occupied blocks and no savestate was
touched. The tests now pass `--no-session-log` and stub the shard check.

## Not done / further options

- The shard is still keyed by the framework's conservative config hash; a
  hook edit still costs a ~1.4 s rebuild at the next build or launch.
- A host-native MDEC/VLC decoder (replacing the guest decoder entirely) would
  remove the shard dependency. That is a larger fidelity project; the
  compiled original decoder already holds 60 fps here.

## If it returns

Look for the `ttk-fmv:` line in `recomp/build-local/logs/session-*.log`. If
it says NOT loaded, run `python3 tools/local/build_movie_overlay.py` from
`recomp/` and read its output.
