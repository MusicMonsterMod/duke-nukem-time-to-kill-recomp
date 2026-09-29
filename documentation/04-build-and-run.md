# Build and run

## Prerequisites

Linux: C/C++ compiler (tested GCC 13.3), CMake 3.20+, Python 3.11+, Ninja, pkg-config, and the development libraries required by SDL3/OpenGL on the host. This machine already had the required system libraries; no system packages were changed.

Windows: use a native x64 C/C++ toolchain supported by upstream (MSYS2 MinGW64 or the upstream portable toolchain), CMake, Python, and Ninja in the same shell environment. Windows execution has not been tested here. Do not mix an MSVC build directory with MinGW tools.

The local Python environment was created at the workspace root:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r recomp/requirements-local.txt
```

On Windows use `.venv\Scripts\python.exe`. Capstone is optional executable-analysis tooling; Ninja is used by the build.

## Rebuild from original inputs

The first run of `run.py` or `build.py` creates `game/` (with a README) if it is missing. Drop your owned USA SLUS-00583 dump there, then from the workspace root:

```bash
.venv/bin/python recomp/tools/local/build.py --jobs 4
```

`--image` is optional. If omitted, the tools search `game/` (then leftover `Duke Nukem*` folders) for a valid USA SLUS-00583 MODE2/2352 dump. A `.cue` is preferred when you pass `--image` yourself. A Europe/PAL (`SLES-01515`) disc or an unknown USA hash prints `error: no valid USA SLUS-00583 dump found` plus the accepted SHA-256 values and stops.

The wrapper:

1. Builds the read-only sector checker using CMake.
2. Checks image/executable identity and sector integrity. CloneCD `.ccd`/`.sub` are validated when present.
3. Creates a byte-identical BIN copy, CUE, boot executable, and import receipt in ignored `disc/`.
4. Builds `psxrecomp-game` and `psxrecomp-bios` from pinned source.
5. Generates game C and the bundled OpenBIOS backend through the upstream CLI with `game.local.toml`.
6. Configures and builds the `local-dev` CMake preset.

All subprocesses use argument arrays. Paths with spaces are supported. No shell-specific expansion is required by the Python wrapper. Failed commands stop the pipeline.

Existing prepared data must match before reuse. The importer rejects different prepared image/CUE/executable content rather than replacing it. Original inputs remain untouched. The local profile is separate from upstream `game.toml` and does not inherit its netplay fingerprint claim.

## Incremental compile

From `recomp/`, with Ninja on PATH:

```bash
cmake --preset local-dev
cmake --build --preset local-dev --parallel 4
```

The preset uses RelWithDebInfo, debug tools, OpenGL, no Vulkan, no netplay, and no launcher UI. It links generated game code when the generated dispatch marker exists. If generation is missing, upstream can instead build a setup host; inspect configure output before calling a successful link a playable build.

Expected local binary:

```
recomp/build-local/Duke_Nukem__Time_to_Kill_Recompiled
```

Windows appends `.exe`. The Linux candidate is approximately 131 MiB including debug information.

## Run the candidate

Convenient wrapper from workspace root: `python3 recomp/tools/local/run.py`. The first invocation creates `game/` even for `--settings`. Ordinary launch uses `recomp/saves/local-play` for writable state. If `recomp/disc/time-to-kill.cue` is missing, the wrapper searches `game/` and imports a known USA dump without touching the original files. The explicit command below is useful for reproducing the initial probes.

From `recomp/`:

```bash
build-local/Duke_Nukem__Time_to_Kill_Recompiled --game game.local.toml --disc disc/time-to-kill.cue --no-launcher --debug-port 9123 --memcard-dir saves/local-dev
```

Use `--headless` for diagnostic runs without a visible window/audio. `--renderer software` selects the software renderer for comparison. Headless mode is not an audio or window-system test.

**Path behavior observed:** the runtime resolves a relative writable-state path under its executable directory. Thus `--memcard-dir saves/local-dev` becomes `recomp/build-local/saves/local-dev`, not `recomp/saves/local-dev`. Use an absolute path when isolation must be explicit. Always use separate cards for experimental settings and baseline playtests.

## Diagnostic requests

The local helper sends one request per connection; the pinned server closes the connection after its response. Debug service is on loopback.

```bash
python3 tools/local/debug_client.py '{"cmd":"frame"}'
python3 tools/local/debug_client.py '{"cmd":"bios_info"}'
python3 tools/local/debug_client.py '{"cmd":"dispatch_stats"}'
python3 tools/local/debug_client.py '{"cmd":"autocompile_status"}'
python3 tools/local/debug_client.py '{"cmd":"overlay_loader_status"}'
python3 tools/local/debug_client.py '{"cmd":"quit"}'
```

`get_frame` requires a frame argument and retrieves a recorded frame. Use `frame` for the current counter. There is no `get_state` command in this pinned server. Supported command definitions live in `psxrecomp/runtime/src/debug_server.c`; current upstream web documentation can differ.

Screenshots should go to ignored `analysis/` using an absolute path. For example pass `{"cmd":"screenshot_file","path":"/absolute/path/recomp/analysis/local-smoke/frame.png"}`. The output is a canonical framebuffer capture, not proof of presented-window behavior.

## Native movie code

The build wrapper first applies the reviewed runtime fixes in `recomp/patches/`.
On an existing checkout, `python3 tools/local/apply_runtime_patches.py` performs
the same idempotent check. A conflicting runtime revision fails rather than
discarding edits. The [CD seek fix](14-gameplay-voice-seek.md) explains the current patch.

The ordinary local build now also runs `tools/local/build_movie_overlay.py`,
which needs GCC and compiles this revision's MOVIE.OVR from the prepared disc.
To rebuild that stage alone from `recomp/`:

```bash
python3 tools/local/build_movie_overlay.py
```

Keep `build-local/cache/` alongside the executable. Deleting it falls back to
interpretation and can reintroduce slow movies/audio starvation. The shard's
cache folder is keyed by `game.local.toml` (including its hook list), so the
`local-dev` preset (`ttk-movie-shard` target) and `run.py` both re-check it
with `--if-ready --quiet` on every build and launch; the session log reports
`ttk-fmv: native movie decoder active` or a WARNING
([59-fmv-shard-namespace.md](59-fmv-shard-namespace.md)).
`build.py --skip-movie-overlay` is available for diagnostics. See [movie measurements](13-fmv-fidelity-pass.md).

## OpenBIOS backend

The pinned framework includes a tracked OpenBIOS image and profile. The generator built its backend successfully; the first runtime probe reported the expected SHA-256 and matching loaded wordsum. No retail BIOS was needed for this build.

That result establishes the selected backend, not universal OpenBIOS compatibility. If later behavior fails, compare with a separately supplied compatible retail BIOS using the pinned framework's documented generation path; do not download an unverified BIOS or infer a fix from another game's settings.


## Persistent launch profiles

`python3 recomp/tools/local/run.py --settings` opens the terminal selector and
returns without launching. Ordinary `run.py` uses the saved selection;
`--mode vanilla` or `--mode modernized` selects, saves and launches directly.
Modernized offers a guarded first-map movement/camera preview; weapon aiming stays original.
`--renderer` now persists per profile. See [profile behavior and recovery](17-player-profiles.md).

## PC action input (D04)

Modernized uses the native action module when launched through `run.py`.
`--settings` choice 6 edits bindings; `--bind ACTION=INPUT` saves and returns.
`--show-bindings` lists the map. Schema 1 profiles migrate to schema 2 with a
backup. See [input contexts, testing and limits](19-pc-action-input.md).

Native SDL checks can be built separately without running a game:

```sh
cmake --build recomp/build-local --target ttk-input-test --parallel 4
xvfb-run -a recomp/build-local/ttk-input-test
```

The reviewed runtime patches form an ordered stack. The applicator validates
already-installed overlapping fixes on temporary source copies, preserving live
local edits. Normal build/import steps and the movie-cache check remain unchanged.


## Modernized movement and camera (D05/D06)

Modernized uses camera-relative WASD and an independent mouse orbit in verified
first-map normal locomotion states. `--settings` choice 7 edits camera selection,
sensitivity and Y inversion; `--camera original` keeps the original camera with
Modernized input. Append `--show-settings` to save without launching. Schema 3
backs up and migrates existing profiles, retaining bindings and renderer choices.
See [contracts, guards and remaining playtests](20-modern-movement-camera.md).

The owned-data guard harness target is `ttk-controls-test`. It takes the prepared
executable and the exact overlay fixture saved by an isolated SDL controls route.
These checks complement gameplay review; they do not certify terrain collision.

## Weapon aiming preview (D07)

`--settings` choice 8 or `--weapon-aim view|original` selects Modernized weapon
behavior at launch. Append `--show-settings` to save without launching. Schema 4
backs up and migrates earlier preferences without discarding their bindings,
renderer or camera settings. Vanilla always uses original aiming. The
[coverage note](21-modern-weapon-aiming.md) describes supported projectile paths
and explicit beam/hitscan gaps. `ttk-aim-test EXE` checks the real adapter against
controlled collision fixtures; it is not a retail-map playtest.

## D07 follow-up preferences

The current Schema 5 launcher adds independent assistance, crosshair and red-dot
controls. `--settings` choice 9 edits these for Modernized. For example:

```sh
python3 recomp/tools/local/run.py --mode modernized --weapon-aim view --aim-assist original-lock --crosshair on --red-dot off --show-settings
```

This saves preferences without starting a game. Assistance is experimental and
limited to the supported weapon paths; Vanilla remains original. See the current
[manual](../GAME_MANUAL.md) and [follow-up contracts](24-d07-controls-and-aim-options.md),
which supersede the older preview descriptions above.


## Local Duke font assets

When the supplied font ZIP and both PK3 metadata archives are present under
`research/fonts/`, the normal CMake build generates `ttk-fonts.pack` and
`ttk-fonts.json` beside the executable. Conversion is deterministic and uses only
Python's standard library. These are local derived art/provenance, kept in the
ignored build tree. Keep both with a copied local build. Originals are read only;
no substitute font is downloaded. Without a valid pack, the existing generic
host font remains available. Vanilla and original TTK text are unchanged.
