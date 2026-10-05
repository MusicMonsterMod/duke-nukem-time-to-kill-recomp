# Local tool contracts

All implementation tools live under `recomp/tools/local/`. They are intentionally small and separate from the upstream framework.

## sector_check.c

Input: a raw Mode 2 image path. Output: one JSON object on stdout; up to 20 bad-LBA diagnostics on stderr. Exit 0 means no detected structural/EDC/ECC errors and no trailing bytes; 1 means invalid sectors or a partial trailing sector; 2 means usage or I/O failure.

The checker reads sequentially using a 2352-byte buffer. It supports Form 1 and Form 2, counts zero Form 2 EDC separately, and does not repair anything. The importer additionally enforces nonempty exact image identity and sector count; the checker is not a standalone disc identity system.

Build directly with a C99 compiler or use the adjacent CMakeLists. Windows compilation uses the same source through CMake. Tests use the original disc only read-only and corrupt temporary copies.

## disc_lab.py inspect

Input: a `.cue` (with its MODE2/2352 `.bin`), a raw `.bin`/`.img`, or CloneCD `.img`. Output: JSON manifest. Uses standard-library mmap and an ISO9660 reader specialized to the observed Mode 2 layout. The parser validates directory bounds and paired-endian fields. Multi-extent ISO reconstruction and arbitrary multi-track formats are not implemented; do not use this as a general disc conversion library.

Manifest schema version 1 includes image hashes, sector count, executable header, every file's extent and two hashes, XA submode counts, duplicate overlay groups, and candidate extent pairs. New fields can be additive; change the schema version for incompatible meaning changes.

`logical_sha256` and `raw_extent_sha256` deliberately have different meanings. The latter includes complete sectors and therefore their padding and headers. Never use a logical audio hash as a substitute for full raw identity.

## disc_lab.py import

With no image argument, creates `game/` if needed and searches it (then leftover `Duke Nukem*` folders, skipping `ignore/`) for a USA SLUS-00583 dump. Accepts a `.cue` (with its MODE2/2352 `.bin`), a raw `.bin`/`.img`, or CloneCD `.img`+`.ccd`+`.sub`. Requires a known image SHA-256 (CloneCD `230a34c2...` or Redump USA `708c0404...`) and executable identity `b5c3ba61...`, plus a fresh native validator run. A PAL dump or unknown hash prints `error: no valid USA SLUS-00583 dump found` plus accepted hashes. Subchannel length is checked only when a `.sub` is present.

Outputs: `time-to-kill.bin`, `time-to-kill.cue`, `SLUS_005.83`, and `import-receipt.json`. Refuses different existing prepared data. The image copy is checked after copying. Original files are never written. Interruption can leave a partial prepared copy; a subsequent import rejects it. Remove only that known partial prepared file after checking the error, then rerun. No automatic repair or destructive overwrite is performed.

## analyze_exe.py

Requires the exact known executable. Audits seed alignment and range, finds direct-call consistency and candidate string references, and optionally disassembles the initial startup region with Capstone. It does not claim CFG correctness or mark hooks safe to emit. Output is research metadata, not a replacement for a disassembler project.

## build.py

Portable Python orchestration for compiler tools, import, generation, CMake configure, and native runtime build. Stops on failure and uses explicit argv lists. Creates `game/` on first run. `--image` is optional; without it, the wrapper searches `game/` for a known USA dump. `--configure-only` still performs import and generation, then stops after runtime configuration. `--jobs` controls compilation parallelism.

The pinned repo and recursive submodules must already be present. The wrapper intentionally does not float or upgrade dependencies. The local virtual environment is added to PATH if found beside the checkout.

## debug_client.py

Sends an arbitrary JSON command to the runtime's loopback debug port, one connection per request. This is a developer control surface, including mutating commands such as pad input, memory writes, and quit. Use read-only diagnostics by default during observation; record all intentional input or memory changes in a test route.

The initial bring-up uses input injection and screenshot capture, never guest memory patching. `quit` asks the runtime to exit through its debug handler. An `ok:false` response makes the helper exit nonzero.

At this pinned runtime, `pause` is registered but explicitly rejects requests: the runtime requires ring-buffer diagnostics instead of synthetic paused snapshots. Do not build a test harness around pause/step without inspecting support.

## vanilla_regression.py

Linux-only bounded D01 input recorder. Requires a unique `--name`, the existing
candidate and prepared disc. Refuses an active game, occupied debug port or
existing run directory. Uses fresh cards and stores logs, captures, identities,
input frames and diagnostics in ignored `analysis/vanilla-regression/<name>`.
No player cards, guest memory writes or original media changes. Process exit 0
means the driver completed, not that gameplay assertions passed: review the
captured scenes using [the route](16-vanilla-regression-route.md). Headless runs
do not verify audible output or window-close behavior. The extended card/save
route remains separate and must not be inferred from the smoke route.


## player_profiles.py / run.py

`run.py` creates `game/` on first parse, even for `--settings`. If the prepared
CUE is missing, it searches `game/` and imports a known USA dump before launch.
`run.py --settings` provides the launch-time terminal selector. `--mode`,
`--renderer`, `--reset-profile` and `--show-settings` support scripted use.
`--settings-file` isolates test preferences. See [schema/recovery](17-player-profiles.md).
Renderer choices are persistent; other runtime preferences remain shared.

## ttk_state_probe.py

Read-only guarded D03 candidate sampler, enabled by
`vanilla_regression.py --state-probe`. Validates executable hash, live resident
code and reciprocal object links; refuses mismatches. The runner also configures
host write-trace ranges and records store PCs without modifying guest state.
`wtrace_dump` uses `addr_lo/addr_hi`, unlike the `lo/hi` range-configuration
commands. [Research notes](18-player-camera-research.md) distinguish field
correlations from safe mutation hooks and describe hybrid trace limitations.


## correlate_overlays.py

Read-only comparison of a local 2 MiB gameplay RAM capture with the 30 unique
OVR payloads from the owned prepared disc. Nontrivial chunk matches nominate
load bases; full-byte comparison distinguishes exact residency from common-code
false positives. An exact match is not execution or transition proof. Raw RAM
and disc payloads stay local; reports contain only hashes/counts/addresses.

## 2026-09-26 — D03 hook-boundary acceptance

Completed read-only first-map research; see [the contracts](18-player-camera-research.md).
`vanilla_regression.py --state-probe` now brackets tracing with exact identity
checks and records selected function entries plus turn-increment writes.
`ttk_hook_contract.py` filters actor/caller/query ownership; ten D03 tests pass.
`summarize_state_research.py REPORT... --output FILE` emits compact call counts
and sequence evidence without raw RAM. Fresh `d03-hook-boundaries` (Modernized)
and `d03-acceptance` (Vanilla) runs exit 0. Final guard count is twelve.
No guest mutation or runtime rebuild. Fire-input captures show ammo 200;
ballistics remain explicitly unverified. D04 follows, with D05/D06 consuming
the walking/collision and camera-constraint contracts.

## D04 PC input tools

`pc_input.py` validates the shared action schema and atomic rebindings.
`run.py --bind ACTION=INPUT` (repeatable) saves Modernized bindings and returns;
`--show-bindings` prints them. `--settings` provides the same editor interactively.
Profile schema 2 migrates existing selection/renderer preferences with backups.

`pc_input_probe.py --name UNIQUE --mode vanilla|modernized` runs the actual
player on a private Xvfb display using xdotool keyboard/mouse events and a focus
sink. It refuses an active game, an occupied port or an existing evidence path.
Cards, profiles, runtime log, screenshots and assertion report are isolated in
`analysis/pc-input/`. It verifies SIO as well as the new read-only `ttk_input`
diagnostic; `--menu-back` tests menu return instead of level entry; navigation screenshots still need review. It uses dummy audio and
is Linux-only. This is not a listening, desktop-compositor or hardware-controller
test. See [D04 contracts](19-pc-action-input.md).


## D05/D06 controls verification

`pc_input_probe.py --name UNIQUE --controls movement|camera` extends the isolated
SDL route. Movement selects the original camera; camera uses the independent
orbit. `modern_controls_probe.py` checks displacement against view axes, run/walk
and probe counters, live camera/facing separation, release, focus and pause input
boundaries. It rejects death captures, retains earlier failures and saves an
owned-data overlay fixture in the ignored run directory. Review fire/ammo and
pause screenshots separately. These drivers refuse an existing game, use fresh
cards and never patch guest RAM.

`ttk-controls-test EXE OVERLAY_FIXTURE` exercises the actual C++ callbacks with
owned bytes and mocked runtime RAM services: Vanilla, identity/actor/caller/state
rejection, run/walk/deceleration probes, orbit constraints, duplicate look counts,
inversion/sensitivity and original-camera selection. Geometry tests verify
normalization and equivalent cumulative input under different batching. They
are not hardware refresh-rate or terrain tests.

## D07 weapon evidence

`pc_input_probe.py --controls weapons` records original ammo, constructor and
swept projectile paths, plus composed `present_shot` captures. `--near-cover`
adds a bounded approach; `--renderer opengl` checks the other exposed renderer.
`--weapon-aim original` exercises explicit fallback. `--synthetic-weapons` logs
inventory and selected-slot RAM writes in the owned isolated process; it never
saves and does not prove acquisition, models or animation. `--weapon-fixtures`
attempts normal key-based selection after granting inventory and rejects a wrong
slot. These are different evidence categories.

`summarize_weapon_aim.py RUN --output JSON` correlates launch vectors, original
flight segments and original collision-consumer impact positions. It reports
errors geometrically, without labelling every target discrepancy a failure:
near cover, moving actors and original ballistics can legitimately intervene.
See [D07 contracts and coverage](21-modern-weapon-aiming.md).

## D07A presentation verification

`pc_input_probe.py --controls presentation` records moderate up/down pitch,
walking fire and composed reticle frames. `--controls facing` exercises natural pistol draw/holster, view
facing, directional movement, firing and original precision aim. Use a unique
name and private cards as with existing routes. `--controls facing-research`
records original H/Shift/fire state correlations without guest writes; its raw
state files remain in ignored analysis. `facing_probe.py` uses a short H tap:
holding it opens the original weapon selector. Heading comparisons after look
allow the original camera constraint/update phases to settle. Native fixtures
cover facing and arm argument ownership, original options and equipment-based
reticle eligibility. See [D07A evidence and limits](22-view-facing.md).

### D07A wall/run follow-up

`pc_input_probe.py --name UNIQUE --controls wall` uses real SDL input and private
cards to approach the first-map wall, rotate and back away, check Shift/Caps Lock
against SIO and guest speed flags, and sweep camera yaw in both directions.
It now rejects death-contaminated samples. Retain the initial long sweep as
excluded evidence: it reached death late in the second sweep despite exit 0.
The route does not certify the strip-club interior or every collision state.
Updated movement/presentation routes explicitly accommodate walk-by-default.
Native input tests also cover custom speed bindings, repeat, capture and Vanilla;
native control fixtures cover idle clearance plus camera-only bump eligibility.

The final wall route sweeps while Duke is still against the wall, before backing
away. It requires the final-orientation hook counter to advance, each sweep step
to follow the mouse turn sign, and sampled view/request yaw error below 20°.
This caught an orientation pin missed by the earlier street sweep. Capture/state
recovery also has a native test with visible orientation differing from the boom.
The final replay records exact driver/binary hashes and rejects death. Intermediate
wall-entrance and inactive-hook failures remain in the evidence, with their causes.

## D07 continuation probes and settings

Schema 5 adds a separate `interact` action and `--aim-assist off|original-lock`,
`--crosshair on|off`, `--red-dot on|off`. All are persistent through `run.py`;
`--show-settings` saves without launching. Test commands always pass a separate
`--settings-file` and private cards. Migration preserves originals with backups.

`pc_input_probe.py --controls continuation` observes dummy-device audio counters,
then asserts armed E suppression, walk/run changes, backward running-jump direction
and the airborne normal-camera lease. `DNTTK_TRAVERSAL_TRACE=1` logs the original
ballistic callback arguments/state when diagnosing a refusal; ordinary play does
not enable it. Audio measurements are timing evidence, not listening tests.

`--controls aim-options --aim-assist original-lock --red-dot off` performs a
natural pistol aim scan and traces original actor contacts. It explicitly grants
one private inventory fixture to verify numbered requests through original
selection/draw logic, never overwrites equipped slot/model, and never saves.
Use `--crosshair off` separately to test independent marker visibility.

`--automatic-entry --controls red-dot --red-dot off|on` (D07D, `red_dot_probe.py`)
runs natural pistol scans in third person, held Mouse2 and first person, with no
guest writes and no state sampler, and asserts the marker hook's hidden count per
segment against the setting. Captures are kept but are not visual proof.
The trace's actor +0x204 word is raw research data, not a verified health offset.
See [D07 follow-up contracts](24-d07-controls-and-aim-options.md).

## Scene-lifetime regression and exit decoder

`ttk-scene-test OWNED_EXIT_RAM` exercises the actual guarded reset hook against
an isolated 2 MiB crash snapshot. It checks RAM equivalence with the owned
original MIPS no-callback list-release routines, plus rejection and repeat cases.
The snapshot is ignored local game data, not a distributable fixture.
`inspect_object_exit.py` now reports whether the dispatched object overlaps a
current animation slot and locates its scene-list node. See
[the column-area investigation](29-column-exit.md).

## D08A controls verification

`inspect_controls_inventory.py` decodes weapon-name IDs through the owned EXE's
actual record and string-pointer tables, with an exact image identity check. Its
report is `documentation/reports/d08a-inventory-identities.json`; it does not infer
slots from the manual. `pc_input_probe.py --controls shortcuts --automatic-entry
--renderer opengl --name <unique-name>` runs the isolated SDL route in
`shortcuts_probe.py`, with private preferences/cards. Original equipment/crouch
checks use logged health top-ups before explicit RAM inventory fixtures; fixture grants do not
establish acquisition or campaign acceptance. Earlier failed replays are retained.

The native controls test additionally covers semantic availability, upgraded
ammo, zero-ammo remote eligibility, original animation/time-budget contracts,
clearance rejection and distance smoothing/clamps. Native fixtures model call
contracts, not original terrain. Input and profile tests cover held release,
scroll bursts, Alt exclusion, custom bindings and explicit schema-7 migration.
See [the focused engineering note](33-controls-shortcuts.md) for original paths
and limitations, and [the final report](reports/d08a-controls.json) for results.

## D08C standing-jump replay

`pc_input_probe.py --controls standing --automatic-entry --renderer opengl --name NAME`
uses `standing_jump_probe.py` on a private display/profile/card directory. It sends
real SDL direction/jump events (the test profile rebinds jump to J), samples original
preparation/flight/landing and checks directional animation plus velocity alignment
and once-only correction. Cardinal/diagonal and Space-only cases are included.
Every health-only longevity fixture is logged; no movement/state/position writes.
`DNTTK_STANDING_BASELINE=1` observes only the first case without requiring the fix.
The normal active-game refusal and cleanup apply. See [D08C](34-standing-directional-jumps.md).

D08C is accepted Done on 2026-09-27. `DNTTK_STANDING_REVERSE=1` selects the
eight-case final input-order route: six Space→direction cases, Space alone and
W→Space. It samples preparation before late direction is pressed. Signed nominal
waits and grouped event summaries are not exact per-key latency measurements.
Current accepted evidence is [the input-order report](reports/d08c-input-order.json);
the earlier standing/running reports retain their historical binary identities.

### Strict traversal exploration capture

For an isolated exploration run, `DNTTK_PROBE_EXPLORE=1` enables request/response
operations. Add `DNTTK_REQUIRE_AUTO_CAPTURE=1` to require gameplay auto-capture and
disable the helper's F10 fallback. Exploration rejects a dead player and verifies
pause/resume globals; `exploration_operations` records operations, and ordinary
exploration records any helper F10 repairs in `capture_repairs`. Driver exit zero
alone does not establish successful traversal; review state and captures.


## Feedback replay and Duke font checks

`pc_input_probe.py --isolated-concurrent` permits an explicitly private Xvfb probe
alongside a player. It still refuses player port 9123 and busy ports; profiles,
cards and overlay captures remain under the private run directory. Never point
probes at a player's process/display/cards. Traversal observations include original
foot/floor/time-step evidence in the iteration-40 helper captures. Probe jump is
rebound privately to J to distinguish it from jetpack input; player bindings stay
unchanged.

`build_duke_fonts.py ARCHIVE OUTPUT.pack` converts the SHA-pinned supplied indexed
PNG archive using the standard library, retaining both PK3 metadata records in
`OUTPUT.json`. CMake invokes it when the local sources exist. The pack is local
owned artwork under the ignored build tree, not embedded source. `ttk-font-test
PACK [SHEET.ppm]` checks the actual shared renderer at three widths; `PACK invalid`
checks clean rejection. `DNTTK_FONT_PACK` overrides only runtime font loading for
isolated missing/corrupt-pack probes. It is not a saved player preference.

See [the iteration report](40-feedback-implementation.md) for actual reproductions,
excluded route attempts, final suites, and limits. Do not infer campaign or user
feel acceptance from the native fixtures.

## Selected feedback verification (iteration42)

`ttk-inventory-test build-local/ttk-fonts.pack` checks the actual temporary tile
rasterizer (240/304/624/1008 available widths, transparent pixels/text/arrow,
expiry and mode/capture/menu/epoch invalidation). Build through the existing
CMake target. `ttk-controls-test` now includes original-menu shared selection,
cycle-only inventory immutability, zero/one/many/depletion, bounded edge grace and
E-owned airborne stow including final generic blend0/1. `ttk-input-test` includes
silent cheat entry, bounded Select delivery after capture release and stow leases.

From the workspace root, `python3 recomp/tools/local/audit_crouch_assets.py` reads
the owned executable/disc and writes a local derived DB00 animation audit under
`recomp/analysis/iteration42`. It records exact asset hashes and reader-derived
frame/root/joint data; it is **not** a validated skeletal importer or animation
writer. See [crouch findings](43-crouch-walk-audit.md).

The implementation/reproduction record is [44](44-traversal-inventory-implementation.md);
the resumable active-state record is [42](42-traversal-feedback-progress.md).
All live helpers in analysis/iteration42 operate on private Xvfb processes/cards
and non-player debug ports. Do not overlap helper inputs into the same session.

## Performance measurement (D08Y, 2026-10-01)

Do not judge frame pacing under Xvfb: software OpenGL (llvmpipe) and the
software renderer make the host the bottleneck, especially in widescreen.
Measure on the real GPU without a window: run the player with
`SDL_VIDEODRIVER=offscreen` (NVIDIA EGL; the log shows
`OpenGL context created (... NVIDIA ...)`), a private profile and cards, and
drive it with `DNTTK_TEST_DRIVE=<mouse counts per frame>` (diagnostics only:
the input layer acts captured and holds Shift + W with a constant turn).
`DNTTK_FRAME_TRACE=1` logs logic frames over 25 ms (`DNTTK_FRAME_TRACE_MS=0`
logs all) with the Modernized hook time and the costliest hooks; a logic
frame is normally two video fields (33.4 ms). Runtime telemetry:
`phase_profile`, `phase_hot` (diff two snapshots), `frame_perf`. Reference
script: `recomp/analysis/d08y-gap-jump/gpuperf.py` (local only).


## Jump feel measurements (D08Z)

`recomp/analysis/d08z-manual-jump` (local only): `measure.py` traces press to
airborne, flight and velocity per field for running, standing, walking and
steered jumps on a private copy of UI slot 5 (`JUMP=assisted|manual`);
`gap.py` presses Space at a given Z or N fields after a run-off and reports
whether Duke ends across the gap; `regress.sh STYLE` reruns the D08X/D08Y/D08U/
D08W routes with that jump style (it rewrites those folders' private test
profiles and analysis files; back them up first). `ttk_controls` reports
`jump_style`, `air_steers`, `coyote_jumps`, `quick_takeoffs` and `air_cap`.
`DNTTK_AIR_CONTROL=0..3` scales the manual air steering rate for tuning.

## D17 frame-rate harness

`recomp/analysis/d17-high-refresh/` (ignored) runs the game on the real GPU
offscreen with private profile, cards and savestate copies (port 9317).
`d17.py up TAG RATE` / `stats` / `dump NAME` / `down`; `trip.sh TAG` dumps
triples (real image k, the redraw shown for it, real image k+1) and prints
their mean pixel distances (`DNTTK_REPLAY_TEST=alpha0|alpha1` makes a redraw
equal image k or k+1); `equiv.py run TAG RATE` saves the per-frame guest
fingerprints (`PSX_FP_RAM_HASH=1` adds RAM contents) and `equiv.py compare A
B` aligns two runs by cycle count (use `OC=100`: the overclock lease is
wall-clock based); `cadence.sh RATES...` prints present intervals, image
shares, guest rate and underruns; `slotscan.sh SLOTS...` (with `R=RATE`)
prints redraw coverage, cadence, guest work and pacer slack per savestate slot.
`mipsdis.py LO HI` disassembles static game code from the generated C (venv
Python with capstone); `genpatch.py` regenerates the D17 runtime patch from the
baseline copies. Debug commands `render_replay` and `replay_dump` (a path
ending in `-seq` dumps the next 24 presents); see
documentation/80-d17-high-refresh-audit.md.

Scripted offscreen play (D17 third pass): `STEER=1` makes `d17.py up` pass
`DNTTK_TEST_INPUT=steer.input`; `steer.py` writes held keys and mouse counts to
it (`face(yaw)`, `inp('w shift dx=2')`). `monitor.py SECONDS [keys]` prints
per-second guest fields, game images, presents, distinct images, redraw
sessions and GPU load; `popsweep.py TAG [yaws]` walks into walls and
strafe-turns while dumping present sequences and listing one-present pops;
`seqcheck.sh TAG` (alpha 0) checks every redraw against its real image;
`shift2.py TAG...` prints the image shift between consecutive presents.
`CARDS=cards-apartment` or `cards-subway` selects private copies of the D11B
states. `PSX_REPLAY_COW_CHECK=1` verifies every redraw restores all surfaces
exactly; the plugin status has a per-present trace.

Emulation-thread profiler (developer): `PSX_PROF=FILE` samples the main thread
every millisecond of its CPU time and writes the counts at exit;
`PSX_PROF_CALLERS=1` also attributes time spent in the GL driver, libc and the
vDSO to the calling runtime function; `PSX_PROF_REPLAY=1` samples only inside
D17 redraw feeds (every 0.2 ms). `symprof.py FILE [N]` symbolizes it. Player
sessions run with `PSX_FORENSICS=0` (the runtime's always-on forensic rings
quiet, no per-frame VRAM readback); set `PSX_FORENSICS=1` or launch with
`--diagnostics` to keep them.

D17A (documentation/81-d17a-instability.md): `a1slots.sh TAG SLOTS` (alpha 1
consistency per slot), `a0check.py`, `a1check.py`, `altcheck.py`
(back-and-forth presents), `backsteps.py` (present trace order), `cwtest.py`
(club wall spot, `CARDS=cards-club SLOT=1`), `holes.py`, `goto.py`,
`sheet.py`/`crops.py` (dumps are bottom-up; sheets flip them), `deferab.py`.
`DNTTK_REPLAY_LOG=1` logs every redraw job (alpha, camera, near clip counts);
`render_replay` reports `multi_area`. `d17.py up` raises
`PSX_STARVATION_TIMEOUT_US` (large dumps stall the emulation thread).
Second pass: `latency.py TAG HEADINGS N` (mouse step to first moved present),
`turnrate.py`, `dancers.py` (per heading: guest fields, game frames, distinct
images, sessions), `yawtrace.py` (presented late yaw order), `still3.sh` (user
states in several near clip modes, `CFGS`), `wdiff.py` (write-trace diff of
player/camera/pad), `CARDS=cards-user2` (user states UI 1-9, file N-1).
`render_replay` reports `host_vertices` [stored, hits, no source, address miss,
word miss]; the plugin status reports `late_*` counters.

Third pass: `eyetest.py` (first-person eye and rotation while idle, walking,
running), `nwstart.py` (boot to a new game on private cards), `realdrive.sh` /
`realseq.sh` (real X11 window, `VIDEO=x11 DISPLAY=:0`, real input via
`xdotool`), `CARDS=cards-user3` (user states UI 1-5, file N-1). A sequence
dump needs a name ending in `-seq` (`d17.dump('X-seq')`, then
`a0check.check('X')`). The plugin status reports `pace_div` and
`pace_changes` (late camera pacing).
Fourth pass: `turnseq.sh TAG SLOT` (driven turning and walking, 4 present
sequences), `realsweep.sh TAG [env]` (real window and xdotool mouse sweeping
left/right; `TOOL=staleclass.py` classifies repeated images while turning,
default `smooth.py`), `smooth.py`, `stale.py`, `jerk.py`, `sweep.py`. The
present trace carries the present time and the planned alpha of the shown
image; `render_replay.host_depth` = [depth-tested triangles, depth clears];
plugin status `late_overruns`, `ahead`.
Fifth pass: `stuck.sh RATE [env]` (start on SLOT, load LOADS, hold W after
each: distance and animation), `circletest.sh TAG` (real window: holster, fire
click/hold, E; reports `+0x224 0x200`), `loadstall.sh` (fields/s after loads),
`holds.py` (held presents while turning; use with `realsweep.sh`, `STEP_S=0.001
PX=1` for a fine real mouse), `yawsmooth.py`, `rawtrace.py`,
`staleclass.py` (now prints njobs). `DNTTK_LATE_LOG=1` logs every late redraw
submission (frame, target present, alpha, flip pending). Status: `lead_ms`,
`ready_ms`. A private street savestate: `cards-fresh` file 0.


2026-10-03 opening responsiveness pass (details in
[83-opening-responsiveness.md](83-opening-responsiveness.md)):

- `tools/local/build_math_overlay.py` builds the exact initialized 192-byte
  geometry-math variant from the verified owned EXE, through the overlay
  compiler. CMake and `run.py` ensure this shard alongside the movie shard.
  Its generated code is never hand edited; normal runtime byte guards apply.
- Modernized high-refresh launches enable compatible ordered translucent
  batching (`PSX_GL_ORDERED_SEMI_BATCH=0` is the comparison). Vanilla does not.
- `DNTTK_PRESENT_TIMELINE=0` compares the previous scheduling path;
  `DNTTK_WORLD_DELAY_MS` adjusts the world-history buffer (default 90 ms,
  clamped 50-130). Camera rotation is sampled much nearer presentation.
- `PSX_REPLAY_UNTIMED=0` compares timing-faithful workers;
  `PSX_REPLAY_VALIDATE_TIMING=1` compares both workers' complete GP0 streams.
  `PSX_REPLAY_VALIDATE_BATCH=1` compares complete RGBA redraws with batching
  on/off. These expensive diagnostics are not normal performance runs.
- `PSX_REPLAY_PROFILE=1` records redraw CPU/GPU-query spans and worker refresh
  causes; `PSX_PROF_FIRST_FRAME` / `PSX_PROF_LAST_FRAME` bound CPU sampling
  when using `PSX_PROF`. GPU query spans can include command-stream gaps.
- Local analysis scripts: `response_cold.py` boots private fresh cards through
  the title/difficulty route, profiles the first gameplay seconds and saves
  private slots 0/11. `quality.py` compares idle/turning scenes (explicit CPU
  100% default; `OC=150` is a different workload), `response_mouse.py` measures
  X11 injection to traced camera presentation, `response_regress.py` checks
  pause/resume, incompatible headers and private UI slots 8-11.
  The other `response_*validate.py` scripts run the comparison diagnostics.

Runtime patch additions include restore/native-code guard reconstruction,
worker-image code-cache invalidation, cycle-deadline accounting, batching and
worker timing. The separate dispatch-cache patch contains the generator
lookup change and its compiled oracle test. All these remain local to recomp.


2026-10-03 next-save pass ([note 84](84-warmup-and-camera-stability.md)):

- `warmup_probe.py SLOT TAG SECONDS` uses private `cards-user7`, CPU 100%,
  4x, real X11 and independent 500 Hz one-count mouse events reversing every
  two seconds. File slots are zero-based. `RATE=unlimited` stresses scheduling;
  `VIEW=third` checks third person. Never overlap a timed game with builds/tests.
- `warmup_subway.py TAG RATE` captures the full compositor width along UI 3's
  left-wall route. Canonical 512-pixel screenshots omit the margins under test.
- `DNTTK_CAMERA_RESEED_TRACE=1` records camera validity, field gaps and epochs.
  Present trace yaw has four decimals and timestamps six; compare yaw jumps
  and camera-sample age alongside presentation intervals and distinct images.
- `PSX_GL_MIXED_BATCH=0` disables the new mixed opaque/translucent batching
  while preserving the earlier ordered-translucent path for A/B comparisons.
- `tools/local/fix_sdl_x11_time.py` applies the exact pinned SDL source repair
  at configure time. Its compiled clock tests cover wrap and queued reports.
  It does not edit installed SDL, and rejects unknown source fragments.
- `warmup_final_timing.py` / `warmup_final_batch.py` compare complete GP0/RGBA
  streams across the new seven states. Oracle duplication is not a speed test.
- `warmup-baseline` preserves the pre-pass executable and original save hashes.
  The player's cards and state files remain outside writable test directories.


2026-10-03 bounded movement polish ([note 85](85-movement-polish-and-isolated-artifacts.md)):

- `polish_motion.py SLOT TAG SECONDS` uses new private cards-user8 and real
  X11 Shift+W/S (slot 12) or turn/move/jump/fire (slots 1/10). Do not enable
  DNTTK_TEST_INPUT while testing real keyboard events: it clears those keys.
- `DNTTK_POSE_TRACE=1` logs `[pose]` capture time/field/eye and `[pose-ready]`
  complete-transform time. Compare arrival time, pair interval and interpolation
  endpoint saturation as well as presentation cadence. Alpha 1 alone is not
  proof of a perceptual hitch when the world pose is stationary.
- `run.py` now defaults PSX_GL_PERF=0 for ordinary launches, or 1 when
  PSX_REPLAY_PROFILE=1. Explicit overrides survive. `frame_perf` has no samples
  when GL diagnostics are disabled; consumers must tolerate that.
- `polish_final.py` runs the bounded current-save routes sequentially. No
  builds or CPU-heavy tests overlap live measurements. `polish-baseline`
  preserves accepted executable/source and original state hashes/mtimes.


2026-10-03 accepted-source closeout:

- Root Git now owns `recomp/src`, tools/tests/configuration and authored patches.
  Framework/UI are pinned submodules. Clone recursively; apply the complete
  `time-to-kill-accepted-source.patch` through the build wrapper. Archived patches
  under `patches/history` are not applied. The old incremental stack was not
  sufficient to recreate the working framework from its pin.
- `tools/local/export_runtime_patch.py` uses a temporary index to export all
  tracked framework edits and reviewed new runtime/recompiler source, preserving
  the real index. A pristine pinned checkout plus the exported patch matched all
  37 accepted modified/new framework files. Repeat this check after changes.
- CI forbids media/generated output/player data, requires essential title source,
  and permits only the two declared dependency gitlinks. A blanket recomp ignore
  is an error. Never replace source with an upstream pointer that omits our edits.
- Primary regression target is now 120 FPS; 180 support, 240 robustness and
  Unlimited stress remain distinct obligations. See acceptance note 86.


2026-10-03 D17E/K/L ([note 88](88-d17e-k-l-visuals.md)):

- `recomp/build-local/ttk-near-test recomp/disc/SLUS_005.83` (from workspace
  root) additionally authenticates the owned executable and
  exercises actual hook/packet contracts: world source ordering, compact-prop
  caller and extent gates, opaque grouping, translucent ordering, Vanilla
  exclusion and altered-code rejection. Without the argument it runs the
  existing projection and plane math tests only.
- `DNTTK_SOURCE_ORDER=0` compares piece-average world ordering;
  `DNTTK_PROP_DEPTH=0` compares the previous compact-prop near-radius behavior.
  These are developer comparisons, not player settings. Broad all-prop
  coverage was rejected; the shipped rule is 256 local units per axis.
- Private evidence is under `analysis/d17-high-refresh/ekl-*`; current copies
  are `cards-ekl-20261003`, with hashes/mtimes in `ekl-save-manifest.json`.
  UI 11/12 have changed since `cards-user8`; use the dated identity, not only
  a slot number. Full-width compositor captures are required for D17E.


2026-10-04 D17N ([note 98](98-d17n-world-subdivision.md)):

- `analysis/d17n-20261004/` (ignored): `intake.py` (slot 12 W/S screenshots),
  `pk.py` (packet dump with `geom_correction` counts), `trace.py` (per-mesh
  primitive trace plus dumps), `walk.py` (`MODE=keep|whole` 24-present
  sequences), `sweep.py` (12 slots, old versus candidate: rates, exact-share,
  ring peak, counters) and `modes.py` (60/180 Hz, Original textures, Vanilla).
  Private cards are a dated copy of the player's files; port 9241.
- `DNTTK_WORLD_SUBDIVISION=1` keeps the original world subdivision. The
  `ttk_input` near report has `whole_polygons` and `subdivided_kept`.
- `ttk-near-test disc/SLUS_005.83` now also runs whole-polygon contracts.


2026-10-04 D17Q ([note 99](99-d17q-uv-seams.md)):

- `analysis/d17q-20261004/` (ignored): `walk.py` (idle plus W/S 24-present
  sequences), `pk.py` (packet dump), `vram.py` (panel texture/CLUT),
  `hires.py`, `sweep.py` (`MODE=legacy|fix`, 12 slots), `census.py` (quads
  whose two halves disagree under the old UV model) and `modes.py` (60/180 Hz,
  Original textures and Vanilla, legacy versus fix). Private cards are a dated
  copy of the player's files; port 9242.
- `PSX_UV_3D_LEGACY=1` restores the per-triangle 2D mirrored-sprite UV model
  on perspective-corrected 3D triangles (developer comparison only).
- To identify which panel a screen region is, remember the 16:9 wide surface
  is offset by 85 native px from canonical VRAM x.



2026-10-04 D22A ([note 100](100-d22a-portal-level-identity.md)):

- `analysis/d22a-20261004/` (ignored): `route.py` (private slot 06 -> portal
  -> Cross -> LEVEL01 control checks), `repeat.py` (portal twice plus a long
  wander), `slots.py` (12-slot lease sweep), `vanilla.py`, `ramdump.py`,
  `levels.py` (LEVELxx.OVR survey) and `selfwrites.py`. Private cards are a
  dated copy of the player's files; port 9243. `setup.py` rebinds Fire to Q in
  the private profile so scripted input can shoot.
- `ttk-controls-test EXE LEVEL00_FIXTURE [LEVEL01.OVR [LEVELS_DIR]]`: the
  optional owned LEVEL01.OVR (extract it locally from the disc) adds the D22A
  level-identity group; LEVELS_DIR, a directory of every owned LEVELxx.OVR,
  adds the D22B group over all 21 selectable levels.

## Level overlay guards (D22B)

`tools/local/level_overlay_guards.py [--disc BIN] [--check]` reads the owned
disc (read-only) and derives the Modernized lease's per-level body guard for
the 21 levels the original level select offers. It verifies the common
layout (tag, strings, jump tables, code ending at the last `jr $ra`, then one
data block whose first reference is the code end), finds the trailing bytes
each level writes (its own `sb`/`sh`/`sw` and the hit vectors it passes to
`0x8007177c`), and fails on a write inside code, data referenced inside code,
or unwritten bytes inside the written tail. Without `--check` it prints the
`level_overlays[]` table for `src/ttk/control_guards.inc`; with `--check` it
compares that file and exits 1 on any difference. Pure standard library;
`tests/local/test_level_overlay_guards.py` covers synthetic overlays and, when
the disc is imported, the owned-disc check. See
[note 102](102-d22b-every-level.md).

- `analysis/d22b-20261004/` (ignored): `survey.py` (`level N` for every level,
  scripted Modernized play, lease/view sampling, overlay RAM diffs, screenshots
  of unfamiliar lease drops), `state_probe.py` (animation/mode timeline in one
  level), `trans.py` (statistics screen, savestate, pause), `slots.py`,
  `vanilla.py`, `steps.py` (Vanilla pad gait probe), `death.py` /
  `death_x.py` (death attempts; Xvfb display 95 for real keys) and `final.sh`.
  Port 9247, private card copy, Fire on Q in the private profile.
