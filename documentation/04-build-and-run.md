# Build and run

## Source checkout

The implementation is now tracked under `recomp/`. Clone with
`git clone --recurse-submodules https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp.git`,
or run `git submodule update --init --recursive` in an existing clone.
The pinned framework and UI are dependencies, not missing unpublished title code.
The build automatically applies `recomp/patches/time-to-kill-accepted-source.patch`.
Historical incremental patches are archived and not applied.

No retail game data, generated game code, binaries, player saves or research are
in the repository. Put your owned disc under `game/`; all content there except
README.md is ignored. The launcher needs a successful local build first.
Optional extracted Duke fonts/inventory packs are not distributed; the existing
conditional CMake asset steps use them only when supplied locally. They are not
required inputs to the base native build. Do not infer that a source-only clone
contains those optional visual assets from the accepted local playtest.


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


## Fonts, icons and UI art (D24A)

No Duke Nukem 3D art and nothing under `research/` is used. The normal CMake
build makes, beside the executable:

- `ttk-fonts.pack` / `ttk-fonts.json`: Time to Kill's own fonts (TTK Big and
  Medium Italic from `/DATA/FONTS.RAW`, the system 8x8 font from `SLUS_005.83`),
  extracted from the prepared disc (`recomp/disc/`) by
  `tools/local/build_ttk_fonts.py`.
- `ttk-inv-icons.pack`: the HUD item icons from the same disc file, plus the
  project's own selection frame (`recomp/assets/ui/item-frame.png`), by
  `tools/local/build_ttk_inv_icons.py`.
- `ttk-inv-digits.pack`: the switcher digits from the tracked CC0 3x5 Microfont
  (`recomp/assets/ui/fonts/microfont/`), by `tools/local/build_ttk_inv_digits.py`.

All are deterministic, use only Python's standard library, and are rebuilt when
their inputs change. The disc-derived packs are retail-derived build output:
never commit or distribute them. Without a prepared disc, the runtime uses its
generic host font and the switcher shows no icons. Keep the packs beside the
executable when copying a local build.

## Jetpack scheme (D08R)

`run.py --jetpack classic|modern` saves the Modernized jetpack scheme
(profile schema 11; older files migrate with a backup). Append
`--show-settings` to save without launching. Modern is the default D08Q
flight; Classic keeps the modern controls with the original burst physics. Vanilla ignores it. See
[57-jetpack-controls.md](57-jetpack-controls.md).

## Resolution and display (D13)

Profile schema 14 saves presentation per profile. `--internal-scale 1..4`
renders the 3D scene at that multiple of the PSX resolution (OpenGL; the
software renderer always runs at 1x). `--display windowed|borderless|exclusive`,
`--window-width 0|640..7680` (0 fits the display) and `--output-filter
linear|nearest` only change how the finished image is shown. `--settings`
choice R edits all four. Append `--show-settings` to save without launching:

```sh
python3 recomp/tools/local/run.py --mode modernized --internal-scale 3 --display borderless --show-settings
```

The launcher passes these as runtime CLI flags from the reviewed patch
`time-to-kill-zzzzzzzz-presentation-cli.patch`; they override
`build-local/settings.toml`. The same patch makes **F11** the only default
fullscreen key (Alt+Enter and Ctrl+F are no longer bound), fixes modifier
matching for rebound hotkeys and makes `exclusive` a real exclusive mode under SDL3. Modernized
defaults to 4x, Vanilla to 1x. Both open windowed by default. At exit the
runtime reports the window state (windowed or fullscreen, borderless or
exclusive, width) and `run.py` saves it to the played profile (schema 15). See [67-d13-resolution-display.md](67-d13-resolution-display.md).

## Widescreen (D14)

Profile schema 18 adds the Modernized `widescreen` choice: `off`, `16:9`
(default), `16:10`, `21:9` or `auto` (follows the window from 4:3 to 21:9).
`run.py --widescreen VALUE` saves it (`--settings` choice W); the launcher
passes `DNTTK_WIDESCREEN` (always `off` for Vanilla) and the preloaded
`dnttk.presentation.widescreen` plugin selects the aspect. `game.local.toml`
`[widescreen] gte_game_mode = true` is inert at 4:3. Regenerated hooks
`0x8008BA30` and `0x8001FC44` anchor the HUD. See
[76-d14-widescreen-first-pass.md](76-d14-widescreen-first-pass.md).

Note (2026-10-01): `build.py` currently stops in `apply_runtime_patches.py` on
`time-to-kill-stopped-window.patch`. To regenerate after a hook-list change,
run the remaining steps from `recomp/` with `PSXRECOMP_GAME` and
`PSXRECOMP_BIOS` pointing at `build-recompiler/`: `psxrecomp/psxrecomp_cli.py
generate --config game.local.toml --project-root . --disc disc/time-to-kill.cue`,
then `cmake --preset local-dev`, `cmake --build --preset local-dev` and
`tools/local/build_movie_overlay.py`.

## Draw distance (D17P)

Profile schema 24 adds the Modernized `draw_distance` choice: `extended`
(default; render-only far limits doubled, portal rectangles 2 native pixels
wider, exact PGXP NCLIP signs when precision is Corrected) or `original`.
`run.py --draw-distance VALUE` saves it (`--settings` choice D); the launcher
passes `DNTTK_DRAW_DISTANCE` (always `original` for Vanilla). Three hooks
(`0x8006276C`, `0x80062B48`, `0x8002FFEC`) were added to `game.local.toml` and
regenerated (see the D14 note above). Framework headers hashed into the codegen
tag (`runtime/codegen_hash_sources.cmake`, including `cpu_state.h`) must not
change for title features: a new tag rejects every existing savestate. See
[note 97](97-d17p-distant-bands.md).

## World subdivision (D17N)

With Modernized texture precision `corrected`, world polygons with exact
projections skip the original screen-space subdivision and are drawn whole.
Two hooks (`0x800114EC`, `0x8001160C`) were added to `game.local.toml` and
regenerated (see the D14 note above). `DNTTK_WORLD_SUBDIVISION=1` (developer)
keeps the original subdivision. See [note 98](98-d17n-world-subdivision.md).

## Fire while swimming (D08O1)

Two hooks (`0x800455BC`, the swim state handler, and `0x80055E80`, the next
player-update step) were added to `game.local.toml` and regenerated (see the
D14 note above). `build.py` currently stops on a stale `build-tools` CMake
cache from the old workspace path; regenerate with `psxrecomp_cli.py
generate` and build with `cmake --build --preset local-dev`. See
[note 107](107-d08o1-swim-fire.md).

## Weapon forward while swimming (D08O2)

One hook (`0x800411B8`, the last call before the player update picks Duke's
model build) was added to `game.local.toml` and regenerated the same way. See
[note 118](118-d08o2-swim-weapon-forward.md).

## Natural swim-fire pose (D08O2A)

One hook (`0x800B42EC`, the matrix composition Duke's model builds use for
each joint) was added to `game.local.toml` and regenerated the same way. It
has its own lightweight callback because the routine is shared by every
model. See [note 119](119-d08o2a-natural-swim-fire-pose.md).

## Climbing down chains and poles (D08J5)

One hook (`0x8003964C`, the bone lookup the pole/chain down probe calls) was
added to `game.local.toml` and regenerated the same way (one generated line;
codegen hash unchanged). Like D08O2A it has its own lightweight callback: only
the probe's call (return address `0x8007d7fc`) goes further. See
[note 125](125-d08j5-chain-descent.md).

## Frame rate (D17)

Profile schema 21 adds the Modernized `frame_rate` choice: `display`, `30`,
`60` (default, the original presentation), `120`, `144`, `165`, `180`, `240`
or `unlimited`. `run.py --frame-rate VALUE` saves it (`--settings` choice F);
the launcher passes `DNTTK_FRAME_RATE` (always `60` for Vanilla) and the
preloaded `dnttk.presentation.frame_rate` plugin selects replay presentation.
Above 60, in-between images are redrawn by forked worker processes (Linux);
the runtime patch is `time-to-kill-zzzzzzzzzz-render-replay.patch`. Six hooks
were added to `game.local.toml` and regenerated (see the D14 note above for
the regeneration commands). Debug commands: `render_replay` (cadence, workers,
plugin counters) and `replay_dump path=BASE`. Player launches also set
`PSX_FORENSICS=0` (the runtime's always-on forensic rings and per-frame VRAM
readback off); `--diagnostics` or an explicit `PSX_FORENSICS=1` keeps them.
The runtime patch is regenerated from its baseline copies by
`recomp/analysis/d17-high-refresh/genpatch.py`. See
[80-d17-high-refresh-audit.md](80-d17-high-refresh-audit.md).

## Jump style (D08Z)

Profile schema 20 adds the Modernized `jump` choice: `assisted` (default, the
original lip launch and fixed arc) or `manual` (jump on the press, edge grace,
air steering, quicker standing takeoff). `run.py --jump VALUE` saves it
(`--settings` choice J); the launcher passes `DNTTK_JUMP` (always `assisted`
for Vanilla). See [78-d08z-manual-jump.md](78-d08z-manual-jump.md).
