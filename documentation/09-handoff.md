# Next-session handoff

## 2026-10-03 - D17D shelf stabilization candidate (Needs playtest)

D17D now has a general near-polygon depth correction and a bounded static-prop
contact tolerance. Background 60/120/180 FPS captures show the club-exit lower
shelf intact through slow turns. The source has almost-coplanar placement;
integer screen snapping previously made the depth surfaces intersect as well.
No object-specific geometry, global culling, input or timing changes.

Read [87](87-d17d-contact-depth.md) for primitive identities, rejected partial
solutions, regression evidence and limits. True intersecting/coplanar geometry
is not universally solvable, and D17E/F/J stay separate Todo jobs. D17A/B remain
accepted. The player should confirm UI 9's lower shelf and ordinary 120/180 FPS
opening/club/apartment play. No next job started.

Candidate: `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
The complete runtime patch is exported and clean-pin verified. Vanilla remains
available; player saves/settings are unchanged. Tests were offscreen/private
Xvfb, not on the user's desktop. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17A/B accepted and implementation published

Read [86](86-d17-acceptance-and-regression-baseline.md) first. D17A and D17B
are **Accepted**, explicitly closed after natural play at 120/180 FPS. Do not
reopen the broad parents for residual defects. The board has D17D-J and D18C
with current/historical private save identities. No next job was started.
120 is the primary quality baseline, 180+ supported high refresh, 240+
robustness, Unlimited stress/debug. Do not impose a 120 ceiling or destabilize
accepted mouse response, opening/club/apartment, idle eye, PS1 character or FMVs.

Important repository correction: `recomp/` source/tools/tests/configuration are
now tracked at the root. Never restore its blanket exclusion. Framework/UI are
pinned root submodules; build applies the complete tracked
`recomp/patches/time-to-kill-accepted-source.patch`. Old incremental patches
are historical only. Export future framework edits with
`python3 recomp/tools/local/export_runtime_patch.py`; unchanged gitlinks do not
save local modifications. Media, generated C, builds, saves, analysis, research
and extracted retail assets stay ignored. Original disc dump is now under game/.

Accepted executable and local snapshot hashes are in note 86. The local binary
is not replaced by the separate clean-source verification build. Do not infer
new 120 numeric benchmarks from the earlier 180 synthetic tests; user acceptance
is a natural-play result. A field clock near 60 is not the Unique image rate:
120 can show 120 freshly rendered views without 120 simulation updates.


## 2026-10-03 - accepted baseline and bounded movement polish

Read [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md)
first. User accepts the previous build as the first polished/enjoyable baseline.
Preserved executable/source: local `polish-baseline`; new private states:
`cards-user8` (UI 12 club Shift+W/S; UI 10 now train-platform ledge; UI 9 furniture).
Never reuse old slot descriptions without checking the current manifest.

Retained change: `run.py` defaults PSX_GL_PERF off except when
PSX_REPLAY_PROFILE=1; explicit overrides survive. Only native addition is
DNTTK_POSE_TRACE=1 capture/completion timestamps. Accepted renderer algorithms,
90 ms world delay, mouse sampling and audio remain unchanged. Faster phase
correction and 130 ms buffering were tested and removed/rejected.

Slot 12 remains open: roughly 67 ms capture intervals plus about 38 ms
composition time and advance scheduling exhaust the coherent snapshot pair.
Regular present intervals and fresh late mouse rotation do not imply smooth
world translation. Next work should address snapshot availability without
blindly adding movement latency or speculative geometry. Slot 10 can drain
audio under movement stress; do not declare crackle solved from slot 1's
zero-underrun result. Full train sequence and actual audio-device listening
remain unverified. D17D Todo tracks isolated furniture/closet/subway artifacts;
slot 9's board defect appears at 60 Hz too. No visual workaround shipped.

100 Python tests pass; real movement/jump/fire and opening/dancer mouse routes
recorded. Opening/dancer camera steps stay under one degree, approximately
179-180 images/s and zero underruns. Original 12 states match hashes/mtimes.
Build `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`,
codegen `8bab543c`. Game closed; no commits or player-setting changes.


## 2026-10-03 - save-driven camera stability candidate (D17A/B Needs playtest)

Read [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md)
first. New private states are `cards-user7`, UI N = file N-1: 1/2 slow,
3 subway peripheral culling, 4 improved opening, 5 early club, 8 minor slowdown,
10 later club. Old reports' slot meanings are obsolete. Never test against the
player's originals. Original hashes are in `warmup-baseline/save-manifest.json`.

Preserve these fixes: camera ownership no longer expires after four fields;
late redraws get two presents' grace and unfinished expirations inform the
readiness estimator; mixed opaque/dual-source batches retain original order;
GPU profiling avoids blocking query readback; pinned fetched SDL3 X11 events
map milliseconds into SDL nanoseconds; known render/kick/aim scratch is reserved
before workers fork. SDL repair is idempotent and fails closed on source drift.

Fixed 180 Hz is substantially improved in measured real 500 Hz mouse sweeps.
Unlimited remains a failing adaptive-scheduler stress case (114 ms stale camera
sample); do not report that mode as solved. Subway culling was not reproduced
in the bounded full-width capture route, so no claimed fix. Further campaign
visibility, actual audio listening and subjective mouse acceptance need the
user's test. Codegen remains `8bab543c`. Final executable SHA-256:
`28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
Final checks: 100 Python tests and four native suites pass; 2,848 GP0 and
3,056 RGBA comparisons are exact. Third-person club, kick/shot, pause/resume,
repeated loads and the Vanilla route pass their bounded checks. Original save
hashes/mtimes match. No commits or player-setting changes. Game left closed.


## 2026-10-03 - latest opening/club candidate (D17B Needs playtest)

Read [83-opening-responsiveness.md](83-opening-responsiveness.md) first; it
supersedes the older performance conclusions below. The player's saved first
person / Match Display / CPU 100% settings are unchanged. The final cold route
has zero audio underruns or failed workers, camera p99 6.0 ms and max 8.3 ms
from the first valid redraw. Final Match Display club turning delivers 179.5
distinct images/s, p99 6.18 ms and realtime emulation. Unlimited is adaptive,
not guaranteed to outperform a fixed target. Minor UI 8/9/11 geometry pops
remain open; UI 10 is the good comparison. Full campaign and audio listening
still need the user's playtest. Do not mark D17B Done without that acceptance.

Important fixes to preserve: per-image code-guard invalidation and reconstruction
after restore; exact initialized math shard; deferred-cycle-aware pipeline
deadlines; timestamped interpolation; ordered translucent batching; worker
refresh on new DLLs rather than invalidated movie functions; atomic worker
start timestamps and guarded timeout subtraction. The old restore bitmap clear
was unsafe and must not be restored as a performance shortcut. The math shard
uses normal byte validation. CPU 150% is a different stress workload and did
not establish the stock-CPU result.

Codegen `8bab543c`; known old player and intermediate saves explicitly
compatible, wrong hash/ABI rejected. Final binary SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
Runtime and generator patch reverse checks pass. The 98-test Python suite,
new compiled guard/deadline tests, GP0/RGBA comparisons and Vanilla route pass;
see note 83 for the old structural-test limitation and RAM/MMIO interpretation.
Nothing committed. Tests use private cards/profiles under local analysis.
The player build is left closed; launch with `python3 recomp/tools/local/run.py`.

## 2026-10-03 - renderer quality pass (D17A/B Needs playtest)

Read [82-renderer-quality-pass.md](82-renderer-quality-pass.md) first. The new
candidate fixes verified replay writes outside the saved live framebuffer
band and related depth restoration defects. Camera/visibility provenance and
Unlimited scheduling also received systemic fixes. D17C idle behavior remains
unchanged. No claim of stable 180 distinct images/s in the club: the worker
CPU tail remains the main measured limit (p95 about 21 ms). Final real-display
180 target gives about 85 distinct images/s in the turning sample; Unlimited
about 100. Treat the earlier "refresh or two" response claims as superseded.

Surface checks: 227 copy-on-write + 232 fallback, zero mismatches. Normal
visual route: 24 sequences without detected backsteps. Endpoint route: 409/412
pixel-exact; three street images differ during turning and lack generation
labels to classify. Do not call this a clean endpoint pass. Use private card
copies and the quality scripts; no player saves were used. Next performance
work should measure worker dispatch/cycle/cache costs and actual display/input
latency, not optimize average FPS. Campaign playtest is still required.
No-input RAM/MMIO equivalence is 630/630; driven 60/180 has two transient
RAM mismatches (629/631), with MMIO exact and repeated 60/60 exact. Keep that
open; earlier synthetic-driver sensitivity is a clue, not a diagnosis.
Vanilla fresh-card capture route passes. The final real-mouse sweep still
has long low-motion/repeat runs; the retained baseline does too, with nearly
identical p99 presentation intervals. The yaw-threshold heuristic cannot
prove image freezes or input latency. See the A/B details in note 82.
Nothing committed. The player build
is left closed; launch with run.py.

## 2026-10-03 - fifth pass: loss of control, late camera rebuilt (Needs playtest)

Ask the user for a fresh-game playtest: turning smoothness (start area and
club), loads of several saves in a row, holstered weapon + fire. Not
reproduced: UI 1 table artifact, UI 6 closet popping; ask for a screenshot or
a description. Measure turning with `realsweep.sh` + `holds.py` (STEP_S=0.001
PX=1) on the real display only. New switches: `DNTTK_LATE_SAMPLE`,
`DNTTK_LATE_LOG`; removed: `DNTTK_LATE_OVERRUN`, `DNTTK_LATE_JIT`. Nothing
committed.

## 2026-10-03 - D17A see-through fixed, D17B dancer jerk (Needs playtest)

Ask the user to replay UI 1-4 (`cards-user4` copies): doorway, statue,
apartment (no see-through), and sweeping across the dancers. Idle stability is
accepted: do not change the D17C eye. If turning still hitches, measure with
`realsweep.sh` + `staleclass.py` (real window and mouse); the remaining limit
is worker latency at 4x, and the next step would be reprojection with the
world and the HUD/weapon captured separately. New switches:
`DNTTK_LATE_OVERRUN`, `DNTTK_LATE_DUE_FREE`; lead default 16 ms. Nothing
committed.

## 2026-10-02 - D17A/B/C third pass (Needs playtest)

Ask the user to replay UI 1-5 (`cards-user3` copies) at Match Display: club
turning with the dancers (late camera, paced), idle stability and view bob
settings, closet, collar, apartment wall. If turning still feels heavy at
every 2nd/3rd refresh, next is present-time reprojection on the GPU. Developer
switches added: `DNTTK_LATE_PACING`, `DNTTK_VIEW_BOB` (numeric scale),
`PSX_HOST_DEPTH`, `PSX_HOST_UV`. Open: Duke stuck after the earlier UI 3 load.
Nothing committed.

## 2026-10-02 - D17A precise near geometry, D17B late camera (Needs playtest)

Ask the user to replay the strip club (dancers in view, turning) and the UI 4-9
states at Match Display. If it feels heavy, compare `DNTTK_LATE_CAMERA=0` and
the 150% CPU option. Developer switches: `DNTTK_LATE_CAMERA`,
`DNTTK_LATE_LEAD_MS`, `DNTTK_LATE_PEEK`, `DNTTK_LATE_JIT`, `DNTTK_LATE_TEST_YAW`,
`DNTTK_NEAR_MODE=conservative|full`, `DNTTK_NEAR_PRECISE`,
`DNTTK_NEAR_SPLIT_PX`, `DNTTK_NEAR_SPLIT_RATIO`, `DNTTK_REPLAY_NEAR_STATE`,
`DNTTK_REPLAY_LOG`. Open: Duke stuck after loading UI 3 not reproduced; cost on
the real display unmeasured (offscreen 93-180 distinct/s). Nothing committed.

## 2026-10-02 - D17C parked; D17A in progress

D17C parked (user saw no difference with the bob off). D17A: in-between
fidelity proven (alpha 0/1), near-wall flicker reproduced in the club at
16:9 first person and localized to the conservative world near clip's piece
ordering (documentation/81-d17a-instability.md). Next: insert the taken
polygon's pieces at its own position in the mesh order; test with
`recomp/analysis/d17-high-refresh/cwtest.py TAG` (STEER=1, CARDS=cards-club,
SLOT=1, OC=100, SCALE=2; target 0 back-and-forth) and `deferab.py` stills
(apartment/street, no vanished walls). Ask the user for F7 savestates at the
other reported places. Nothing committed.

## 2026-10-02 - D17 accepted; D17A/B/C added; D17C experiment (Needs playtest)

**Next session starts with D17C** (user's choice): get the user's playtest
verdict on `DNTTK_VIEW_BOB=off`, then keep it off, make it an option, or
redesign it. After that D17A or D17B.

D17 is Done (user-accepted). Next jobs on the board: D17A (instability and
black areas at the places the user listed), D17B (mouse-to-present latency
measurement, then a late camera update if the 30 Hz camera is the cause), D17C
(view bob). D17C test: `DNTTK_VIEW_BOB=off python3 recomp/tools/local/run.py`
(first person; environment only, not saved). Measure with
`recomp/analysis/d17-high-refresh/bobframes.py KEYS SECS` (per-field root and
eye height; STEER=1). Nothing committed.

## 2026-10-02 - D17 second playtest fixes (Needs playtest)

Recheck at Match Display (180 Hz): the street, the apartment (sink, wardrobe,
light switch), the subway past the EXIT sign and the power button should now be
smooth and stable; backtick console `fps` should show about 180 FPS, a Unique
count near it and Game 30. Developer switches added: `DNTTK_TEST_INPUT=FILE`
(scripted input), `PSX_REPLAY_COW=0` / `PSX_REPLAY_COW_CHECK=1`. Harness:
`steer.py`, `monitor.py`, `popsweep.py`, `seqcheck.sh`, `shift2.py`,
`CARDS=cards-apartment|cards-subway` (private copies of the D11B states).
Offscreen runs keep the GPU at its P5 clocks (no display), so they are a worst
case. Open: inherent snapping and original-mesh cracks; `ttk-input-test` needs
a live desktop session. Nothing committed.

## 2026-10-02 - D17 playtest review fixes (Needs playtest)

Recheck the first playtest's problems at the saved rate (`run.py`; the profile
has 120): the strip-club street and other heavy outdoor areas should now be as
smooth as the sewer with no audio artifacts; walking through doors should not
show other rooms; the intro and level movies should look like 60 Hz (no
flicker, normal brightness); after a first-person quick kick (Q) the leg should
be gone. Player sessions now run with `PSX_FORENSICS=0` (launcher; use
`--diagnostics` or `PSX_FORENSICS=1` for the forensic rings). New developer
switches: `PSX_GL_STATE_CACHE=0`, `PSX_PROF=FILE` with `PSX_PROF_CALLERS=1` or
`PSX_PROF_REPLAY=1` (profiler, `symprof.py`), `DNTTK_ROOM_WALK=always|off|probe`,
`DNTTK_TEST_KICK=<frames>` (with `DNTTK_TEST_DRIVE`). Harness additions in
`recomp/analysis/d17-high-refresh`: `slotscan.sh` (per-slot coverage and load),
`mipsdis.py` (static code from the generated C), `genpatch.py` (regenerates the
runtime patch). Open: vertex snapping and affine warp with small camera steps
(geometry correction off), third-person camera lerp around wall corners, fewer
in-betweens than asked above 120 Hz in the heaviest scenes. Nothing committed.

## 2026-10-02 - D17 high refresh rate (Needs playtest)

Playtest on the 180 Hz monitor: `run.py --frame-rate display` (saved;
`--frame-rate 60` returns to the original). Check: camera turns, running and
enemies look smoother than at 60 with no change in game speed; no tearing in
windowed and fullscreen (if exclusive fullscreen tears, try
`PSX_REPLAY_VSYNC=1`); `--frame-rate 30` and `120/144/240` also behave;
first person, widescreen, menus, movies and level loads; any flicker between
real and in-between frames (a mismatch) or stutter in busy scenes. Developer
switches: `DNTTK_FRAME_INTERP=off` (pacing only), `DNTTK_INTERP_STEPS`,
`DNTTK_REPLAY_BUDGET_MS`, `DNTTK_REPLAY_SLACK_MS`, `DNTTK_REPLAY_WORKERS`,
`DNTTK_REPLAY_TEST=alpha0|alpha1` (fidelity), `PSX_FP_RAM_HASH=1`
(equivalence). Harness: `recomp/analysis/d17-high-refresh` (`d17.py`,
`trip.sh`, `equiv.py`, `cadence.sh`). Runtime patch
`time-to-kill-zzzzzzzzzz-render-replay.patch` (new files render_replay.c/h,
render_worker.c). Regenerated hooks: `0x80026164`, `0x800632B0`,
`0x80031D10`, `0x80032E78`, `0x80031C14`, `0x8001FBA0`. Open: driven-input
equivalence at 180/unlimited (one pad sample), vsync on the real display,
moving room geometry. Pre-existing: `ttk-near-test` link error
(`frame_trace_*`). Nothing committed.

## 2026-10-01 - D08J1 accepted (Done)

User playtest: "genuinely working solidly." Committed. Next job: user's choice.

## 2026-10-01 - D08J1 overhead ladder grab (Needs playtest)

Playtest at save slot 6: hold E and run (or walk) at the ladder; Duke should
leap and grab it with no Space, then W climbs. Also: stop under it and tap E;
start a little to one side; try with a weapon drawn (E stows first; pressed
late while running, Duke stops at the wall and then leaps straight up).
Without E he should stop at the wall as before. Tuning:
`ladder_leap_distance()` windows (750/800/450), `leap_side` (180),
`leap_step` (70), rise limits 300..1400 in `recomp/src/ttk/ladder_top.inc`;
`DNTTK_TRAVERSAL_TRACE=1` logs `ttk-ladder-leap`. Scripts:
`recomp/analysis/d08j1-ladder-run` (`sweep.py`, `leap2.py`, `regress.sh`).
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Nothing committed.

## 2026-10-01 - D08Z accepted (Done)

User: "i love it. lock it in." Player profile stays on `jump: manual`
(assisted remains the default for new profiles). Next job: user's choice.

## 2026-10-01 - D08Z manual jump style (Needs playtest)

Playtest with `run.py --jump manual` (saved; `--jump assisted` returns to the
default). Check: running jumps leave on the press, the slot-5 gap by timing,
late presses just after an edge, steering with the mouse + W and braking with S
mid-air, quicker standing jumps, E grabs/mantles still work. Tuning knobs:
`air_accel_fraction` (0.14), `manual_edge_grace` (14), `quick_takeoff_scale`
(3) in `recomp/src/ttk/manual_jump.inc`; `DNTTK_AIR_CONTROL=0..3` scales the
steering without a rebuild. Analysis scripts: `recomp/analysis/d08z-manual-jump`
(`measure.py`, `gap.py`, `pit.py`, `regress.sh`). Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`.
Nothing committed.

## 2026-10-01 - D08Y accepted (Done)

User confirmed the slot-5 jump and that stutter, audio slowdowns and freeze
frames are fixed (round 5: leased 150% CPU overclock, lip-launch jumps, jump
mantles, budgeted reach retries). Nothing committed yet. Next job: user's
choice; D08Z (optional manual modern jump) is in the backlog. User found
`--cpu-overclock 100` and `150` equally playable; 150 stays the default.

## 2026-10-01 - D08Y round 5: overclock leased to gameplay (Needs playtest)

Boot/loading audio slowdowns at 150% fixed: the overclock is renewed from the
player update and lapses outside gameplay; a safety net pauses it for 5 s if
emulation falls behind (`[TTK cpu]` log line). Binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`. If the user still
hears slowdowns, check the session log for `[TTK cpu]` lines first.

## 2026-10-01 - D08Y round 4: emulated CPU overclock (Needs playtest)

Playtest: play normally (first person, 16:9) for a few minutes, slot 5 and
elsewhere; freezes while running should be gone. The profile migrates to
schema 19 on launch (backup kept) with `cpu_overclock` 150. If a scene still
drops, try `run.py --cpu-overclock 175` and report it; `--cpu-overclock 100`
restores the original speed. Measure performance on the real GPU, not Xvfb:
SDL offscreen + `DNTTK_TEST_DRIVE`, see documentation/10-tooling.md and
`recomp/analysis/d08y-gap-jump/gpuperf.py`. Runtime change:
`recomp/patches/time-to-kill-zzzzzzzzz-cpu-overclock.patch`. Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`.
No commits until asked.

## 2026-10-01 - D08Y round 3: smoothing (Needs playtest)

Freeze frames during E reaches and the jump-mantle pop fixed; binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`.
Check: E jumps at slot 5 and other ledges feel smooth, no hitch, no snap into
the mantle. Tuning: `reach_core_budget`/`reach_extra_budget` and the try lists
in `ledge_reach_retry()`, glide in `mantle_glide()` (`ledge_reach.inc`).
Diagnose with `DNTTK_FRAME_TRACE=1` (frames over 25 ms or 3 ms of hooks).
D08Z (manual modern jump) is backlogged. No commits until asked.

## 2026-10-01 - D08Y round 2 (Needs playtest)

Playtest: slot 5, run at the gap and tap Space anywhere in the last few steps:
Duke should leap from the very lip and land (no E needed). Check the hitch
after E jumps is gone, and that bed/couch jumps in the apartment still fire
instantly. Also any other gap or pit: jumps pressed near it now leave from the
lip like the original. Code: `0x800780b4` hook (`modern_controls.cpp`, 768
threshold), `edge_jump_queued()` + `pc_input.cpp` hold (40 updates),
`ledge_reach.inc` (D08Y mantles), `shortcuts.inc` word copies. Private scripts:
`recomp/analysis/d08y-gap-jump` (`sweep.py`, `mjump.py`, `furniture.py`,
`latency.py`, `regress2/`). Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. No commits until asked.

## 2026-10-01 - D14 widescreen accepted (Done)

User: "im very happy with it! i accept!" Committed with the case study
(DuckStation stretches 2D by 4/3; native-wide does not). Next job: user's choice.

## 2026-10-01 - D14 widescreen (Needs playtest)

Modernized defaults to native 16:9 (profile schema 18, `run.py --widescreen
off|16:9|16:10|21:9|auto`). Fixes: dropped `nw_hud_corners` (it shifted world
polygons: the off-centre, torn preview), widened TTK's portal root rectangle
`0x800d2210` at the render's projection load, D11B near clip in third person,
HUD layout `0x800dd778` moved out while the status bar draws (new hooks
`0x8008BA30`/`0x8001FC44`, regenerated). Binary `3f726b91...af2c`. Next: user
playtest across levels at 16:9 (and 21:9/auto if wanted); real-GPU fps.
`build.py` stops at the runtime-patch check (`time-to-kill-stopped-window.patch`);
the generate/build steps were run directly (04-build-and-run.md).
DuckStation testing: the flatpak ignores an `XDG_CONFIG_HOME` override and uses
the player's config (resume-on-exit on); use the portable copy in
`recomp/analysis/d14-widescreen/r2/ds/app/bin` (`portable.txt`).
Nothing is committed yet.

## 2026-10-01 - Session locked in; next: D14

All session work accepted (D08U, D08W, D08A3, D08X Done). D14 16:9 preview is
off centre with geometry holes; start the next session from the ordered steps
in documentation/76-d14-widescreen-first-pass.md (A/B `native_wide = false`
first). Standing permission: close the player's open game and continue.

## 2026-10-01 - D08X accepted (Done)

User: "thats it, fully accepted this!!!". Binary `60161f81...369c`. Nothing committed yet.

## 2026-10-01 - D08X flush hang (Needs playtest)

E-jump ledge catches settle to the shimmy-aligned hang height (ledge top +
456). Binary `60161f81...369c`.

## 2026-10-01 - D08X crate-to-crate mantles (Needs playtest)

Jump mantles onto any climbable object are proximity-based with the original
line-up; crate-to-crate 18/18 at any angle to 45 degrees. Binary `ccb4eb2a...0107`.

## 2026-10-01 - D08X angle forgiveness (Needs playtest)

E reach retries the original acquisition with the heading turned +-25/+-51
degrees as well as lifted. Binary `8bb01385...1701`. Playtest: slot 11 crate
top to the ledge in front at an angle.

## 2026-10-01 - D08X higher grab confidence (Needs playtest)

E reach retries the original acquisition up to 480 higher, easing Duke into
the hang. Binary `43eda09c...0f61`. Playtest the drained-water ledges.

## 2026-10-01 - D08X first-tier crate hang (Needs playtest)

A jump catching a stacked crate into the original pole-style hang (mode 7,
A/D circled Duke round it) now lets go at once. Binary `c1c49119...6fe7`.

## 2026-10-01 - D08X crate hang fix (Needs playtest)

Fixed: object hang toggling flag-table bits broke the identity guard (all
Modernized controls lost); stuck crate hang now lets go (S, or W without a
climb). Binary `bf368b68...bd45`. New log line `[TTK identity] guard N` names
any future guard failure.

## 2026-10-01 - D08X crate mantle (Needs playtest)

Boxes room (private copy of user UI slot 11): an E running jump into a crate now
starts the original mantle mid-jump instead of bouncing. Binary `9bade2a5...ba9c`.
Playtest: Shift + W + Space with E held into the crates from a few steps back.
Standing permission (2026-10-01): close the player's open game and continue.

## 2026-10-01 - Playtest: D08U, D08W, D08A3 Done; D08X boxes follow-up

User accepted D08U, D08W and D08A3. D08X ledge grabs accepted; next work is the
boxes room (private copy of user UI slot 11 in recomp/analysis/d08x-boxes):
Shift + W + Space with E held should mount a crate mid-jump instead of bouncing.
16:9 preview: `DNTTK_WIDESCREEN=16:9 DNTTK_GAME_CONFIG=recomp/analysis/d14-widescreen/game-16x9-preview.toml run.py` (Modernized profile).

## 2026-09-30 - Autonomous chain: D08U (Needs playtest), then D08X, D08W, D08A3, D14 start

User authorized an unattended chain: D08U, D08X, D08W, D08A3, then start D14;
no commits; a combined playtest later. D08U: E at a ladder top lowers Duke onto
it (host blend over the original 156 transfer); S climbs down and steps off.
Binary `6129f2ab...5683`. Details: documentation/72-d08u-ladder-top.md.
D08X (Needs playtest): E held through a jump arms the original reach at
takeoff, so close ledges are caught. Binary `bd153dd5...65b3`. Details:
documentation/73-d08x-ledge-grab.md.
D08W (Needs playtest): shallow-water jumps follow the held direction.
Binary `7ca63455...3328`. Details: documentation/74-d08w-wade-jump-direction.md.
D08A3 (Needs playtest): switcher icons are now TTK's own HUD art from
/DATA/FONTS.RAW (pack rebuilt by build_ttk_inv_icons.py). Details:
documentation/75-d08a3-ttk-inventory-icons.md.
D14 (In progress, first pass): inert 16:9 activation plugin and package;
experiment shows TTK's 4:3 culling gaps at the widened edge; no player option
yet. Details: documentation/76-d14-widescreen-first-pass.md.
Final binary for the playtest: `db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`.

### Playtest checklist for this chain

1. D08U: walk to the edge of the sewer walkway next to the ladder (slot 12),
   press E. Duke should stow, turn and lower onto the ladder smoothly. Hold S:
   he climbs all the way down and steps off, pistol back out. Also try the top
   of the first alley ladder. Walking or running off the edge without E must
   still drop as before.
2. D08X: hold E and jump (W + Space) at a ledge within reach, for example the
   wall at the side of the slot-12 walkway. Duke should grab and pull up.
   Without E it bounces as before. Try running jumps and ledges elsewhere
   (apartment exterior, crystal-2); armed, hold E a moment before jumping.
3. D08W: in the subway shallow water (slot 1), A/D/S + Space should jump in
   that direction, including after running forward. Also the crystal-2 wade.
4. D08A3: press [ or ] to show the switcher; the icons should be TTK's own HUD
   art (health cross for medkit, jetpack, Bio Mask skull, goggles), green %
   and frame unchanged.
5. General: ladders, mantles, run-jump ladder transfers and Vanilla feel
   unchanged.

## 2026-09-30 - D04A accepted (Done)

User: "perfect, accept" on binary `198673f5...ded68e`. Escape pauses with a
free cursor and resumes with automatic recapture. Next job: the user chooses.

## 2026-09-30 - D04A Escape mouse release (Needs playtest)

Ad hoc user job: Escape sometimes left the mouse captured. Recapture raced a
missed Start tap; now Escape pulses Start and holds auto-recapture until the
pause stops gameplay offers. Only `recomp/src/ttk/pc_input.cpp` and
`recomp/tests/local/pc_input_native.cpp` changed. Needs the user's windowed
playtest; look for `Mouse released (Escape)` in the session log.
Playtest fix: the resume Escape no longer holds recapture (it had left the
mouse free after resuming). Rebuilt: binary `198673f5...ded68e`.

## 2026-09-30 - D07D, D08T1, D08V accepted (Done)

User: "awesome. accept". All three Done on binary `123c7910...693e`. D08X
(hold-E ledge grab) is now unblocked. Next job: the user chooses from the board.

## 2026-09-30 - D08V implemented (Needs playtest); D07D, D08T1, D08V await playtest

Camera-only lease through mantle/hang/pull-up, post-mantle 0/8 and 0/9 gait
and unowned 107/108 falls; `traversal_camera_ready()` replaces the tank
fallback there. Repro and sweep scripts in `recomp/analysis/d08v-sewer`.
Binary `123c7910...693e`. D08X (depends on D08V) stays blocked on the D08V
playtest. Next job: the user chooses from the board.

## 2026-09-30 - D08T1 implemented (Needs playtest)

Hold Grab (RMB/Alt) grabs pushables; E mantles them like any climbable.
New generated entry hook `0x80051890` (E's Cross masked at the idle grab);
`0x80051CF0` mask only while Grab owns Cross. Schema 17 (`grab`, `grab_alt`;
`original_aim` Unbound). `build.py` stops in the stale runtime patch-stack
check; regenerate with `psxrecomp_cli.py generate` directly (see
documentation/70). Binary `66097cf8...3015`.

## 2026-09-30 - D07D implemented (Needs playtest)

Red dot off by default in Modernized (schema 16, one-time migration with a
backup). `marker_hook` owns the dot at enqueue `0x8002BC18`/RA `0x80033DB0`
using S4 == Duke and the frame RA `0x8003543C`; no entry lease or LEVEL00
gate. Tests: `ttk-aim-test`, Python 85 OK, `pc_input_probe --controls red-dot`
on/off runs. Playtest: look for the dot with targets in third/first person,
held aim, jetpack, swimming, scripted cameras. The legacy `aim-options` route
fails in stale sampler/D04 checks (not changed). Binary `db5bf964...ec67b`.

## 2026-09-30 - D07D added (no red autoaim dot in Modernized)

New Todo D07D: the user finds the original red autoaim dot next to the modern
crosshair confusing. `red_dot` is off by default in every Modernized profile
(with profile migration), covering every state where `marker_hook` currently
steps aside; the option stays, and Vanilla keeps the dot. Next job: the user
chooses from the board.

## 2026-09-30 - D08T1 added (E always mantles, hold RMB to grab)

New Todo D08T1 from a user brief: pushable objects must not change traversal
controls. E always mantles; a new rebindable held Grab / Manipulate action
(default right mouse in Modernized, where precision aim is not needed; Vanilla
keeps it; Alt is the proposed second binding and the legacy-aim grab)
grabs, W/S push/pull, release lets go. Audit the Mouse2 /
`original_aim` paths first. Brief:
`documentation/69-d08t1-grab-manipulate-brief.md`. Next job: the user chooses
from the board.

## 2026-09-30 - D08X added (hold-E ledge grab)

New Todo D08X: in Modernized, holding E while jumping toward a ledge at
grabbing height should reliably catch it and mantle up, like run-jump + E on
ladders (D08J). Trace the original ledge eligibility (anims 134-142, 6/7
hanging dispatches) before choosing a fix. Next job: the user chooses from
the board.

## 2026-09-30 - D11C Done (user accepted)

User: "duke's head is back, mark as complete! well done". Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`. Next
job: the user chooses from the board.

## 2026-09-30 - D11C stale head-hide reclaim (Needs playtest)

`first_person_draw(duke,eye)` clears an unowned bit 0 on Duke's joint 9
record at Duke's draw entry (debug `fp.head_reclaims`), then hides it again
only in first person. The original never sets that bit (static scan of the exe
and the 30 unique overlays). A private slot-12 copy loads with the head drawn in
third person; first person still hides it. Awaiting the user's slot-12 check;
then D11C can be Done. Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

## 2026-09-30 - D17 expanded to high refresh rate rendering

User brief for Match Display / 30-240 / Unlimited rendering without changing
gameplay timing was folded into D17 (still Todo, depends on D01, D13). Brief:
`documentation/68-d17-high-refresh-brief.md`. Audit and plan first. Next job: the
user chooses from the board.

## 2026-09-30 - D16A cancelled; D08A3 added

D16A (HRP research) is Cancelled by the user; original TTK assets are the
direction. New Todo D08A3: extract original TTK medkit/biomask/jetpack/steroids
(and goggles) icons to PNG and rebuild `ttk-inv-icons.pack` so the switcher no
longer uses Duke3D tiles. Next job: the user chooses from the board.

## 2026-09-30 - D13 Done (user accepted)

User: "correct correct correct, D13 is good. I'd say let's approve it." Final
binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`. New backlog from the D13 playtest: D08U (top-of-ladder mount),
D08V (sewer mantle lease), D08W (subway shallow-water sideways jumps), D10B
(tight-space camera transparency/doors), D11C (savestate stuck head-hide
flag, slot 12). Next job: the user chooses from the board.

## 2026-09-30 - D13 F11 fullscreen key (Needs playtest)

F11 is the only default fullscreen key (`host_keymap.c`); Alt+Enter and Ctrl+F
are unbound. The D13 patch was regenerated and checked forward/reverse. Launcher
text says "F11:". Awaiting the user's confirmation of windowed start, F11 and
the remembered state; then D13 can be Done. Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D13 windowed default + remembered display (Needs playtest)

Schema 15 (`fullscreen_mode`), runtime `--fullscreen-mode` and
`--presentation-state` (written in `shutdown_runtime`), and
`absorb_presentation_state` / `save_presentation_state` in the launcher. The
user's profile is set to windowed at 4x (backup
`recomp/config/player-profiles.json.recovered-2a7d56cb...`). Awaiting the user's
confirmation of windowed start, Alt+Enter and the remembered state; then D13
Done. Binary `7bd2a001a295330397d45c73ca9bc2ca0bc575c0d889bab3259d06778b3a3d1c`.

## 2026-09-30 - D13 playtest follow-up (Needs playtest)

User confirmed 4x at 60 fps; Modernized default now 4x. Fixed Alt+Enter
(`host_keymap.c` exact modifier compare, now `mod_groups`) and SDL3 exclusive
(`psx_apply_fullscreen_display_mode` in main.cpp); both in the D13 runtime
patch, which was regenerated and checked forward/reverse. Awaiting the user's
confirmation, then D13 can be Done. New backlog from the same playtest: D08U
(top-of-ladder mount), D08V (sewer mantle lease), D10B (tight-space camera).
Slot 12 is `saves/local-play/openbios/state_800AB6FC_slot11.pst`; a private
copy is in `recomp/analysis/d13-resolution/cards/openbios/`
(`probes/start.sh RUN 11 ...`). `ptrace` attach is blocked on this machine, so
use temporary log lines instead of gdb attach. Binary `ecc9328065b8a6c3311423e1640936b1835e1517d35b923605609595e2f3a95d`.

## 2026-09-30 - D13 resolution and display (Needs playtest)

Runtime patch `recomp/patches/time-to-kill-zzzzzzzz-presentation-cli.patch`
adds `--internal-scale/--display/--window-width/--output-filter`; profile
schema 14 (`player_profiles.py`) saves them per profile and `run.py` passes
them (`presentation_args`; software forces 1x via `effective_scale`). Test
harness: `recomp/analysis/d13-resolution/` (`probes/start.sh RUN SLOT
[run.py args]`, `stop.sh`, `measure.py RUN`; port 9193, display :93, `WM=1`
starts metacity for fullscreen checks). `screenshot_hires` does not see the
OpenGL hr FBO; use window captures. Awaiting the user's desktop test (fps at
2x-4x on the GTX 1080 Ti, exclusive fullscreen). Pre-existing: patch-stack
check fails on `host_osd.c` drift vs the inventory-strip patch (see the D13
note). Binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`.

## 2026-09-30 - D08L Done (user accepted)

User: "im happy with that!" Final binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`. Next job: the user chooses
from the board (remaining Todo includes D09, D13, D19B, D20).

## 2026-09-30 - D08L inertial edge run-off (accepted)

Running falls over 768 that directly follow a ground stride now get the D08F
short-fall lease (`recomp/src/ttk/terrain.inc`, previous-animation gate 72..79,
`large_fall`, counter `run_offs`); all departures capped 10000 running / 2048
walking in the `0x8003ebf4` ballistic hook (`modern_controls.cpp`). Note: the
adapter never sees mode 9/0 on the first fall update (already 9/9), so do not
gate on it. Test state: `recomp/analysis/d08l-edge/apt-cards` slot 1
(apartment), 2 (on the bed), 3 (fire-escape platform, run west off the end);
probes port 9191, display :91 (`probes/start.sh`, `stop.sh`, `tr.py`).
Previous binary `before.bin` (`1452391c...`). Open follow-ups: other
exterior ledges, very high falls. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D12A Done (user accepted)

User: "its done! accepted. ... this has been yet another amazing feat of engineering." Final binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`. Next job: the user
chooses from the board (D12A is closed; remaining Todo includes D08L, D09, D13).

## 2026-09-30 - D12A thigh 0.13, E never kicks, Q chains (accepted)

Thigh drop default 0.13 (user's choice). `kick_convert` kicks only with the
attack held (E suppressed); Q queues during a kick, held Q repeats. Awaiting
the user's test. Binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D12A follow-up (Needs playtest)

After the user's first playtest: sphere at the crosshair trace hit
(`view_segment_query` in weapon_aim.cpp, within 480 horizontal) else 340;
held attack with Boot kicks in first person while moving; thigh drawn end
for end from the knee (`DNTTK_FP_KICK_THIGH=0` off). Garbage bags: alley
state in `recomp/analysis/d12a-kick/alley-cards` (slot index 4), bags
`0x801dcf44`/`0x801dcd64`. Private copies of the user's slots 0-4 in
`recomp/analysis/d12a-kick/user-cards` (index 3 = street near the subway
stairs; the pallet was not identified). Open: thigh framing options, the
pallet. Binary `e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`.

## 2026-09-30 - D12A first-person quick kick (Needs playtest)

Q in the eye view = host quick kick (`recomp/src/ttk/kick.inc`); Boot-selected
original requests (112..115) are converted at `0x800493a4`. Hit = original
sphere `0x800a979c` along the view on each update of frames 8..20; leg =
joints 14..17 from 115 poses via the D12 private-matrix path. Framing knobs
`DNTTK_FP_KICK`, `DNTTK_FP_KICK_TILT`, `DNTTK_FP_KICK_REACH`,
`DNTTK_FP_KICK_FREEZE`. Test state: `recomp/analysis/d12a-kick/fresh-cards`
savestate slot 3 (first street, god mode, pig cop ahead); probes use port 9181
and display :81 (`probes/start.sh`, `stop.sh`). Previous binary `before.bin`
(`941593c0...`), sources in `before-src/`/`after-src/`. Open: user playtest of
feel/framing/damage; knockback (enemy reaction 84) not reproduced. Binary
`93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`.

## 2026-09-30 - D08T pushable objects (Done, user-accepted)

E grabs / W,S push-pull / E lets go / Space climbs, for objects flagged
`0x08000000`; see `documentation/64-d08t-pushable-objects.md`. Test state:
private copy of the user's slot-5 alley savestate in
`recomp/analysis/d08t-push/cards` (slot index 4), sources before the change in
`.../before-src/`, previous binary `before.bin` (`65226a9d...`). New generated
hook `0x80051CF0` (regenerated with `psxrecomp_cli.py generate` as in the D12
note, then `cmake --build --preset local-dev` and `build_movie_overlay.py`).
Probes use port 9171 (`PORT` env). User playtest: "it works so much better than the original now. this is it rock solid. confidence level is very high." Possible
follow-up: other pushable objects/maps; host pre-alignment if the line-up angle
ever feels strict. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

## 2026-09-30 - D12 Done (user accepted); next D12A

User: "finally, we can mark this as accepted!!" First-person weapons are Done
on binary `65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next job chosen by the user: D12A, keep the kick (Q) in first
person. Per-weapon framing lives in the offset/tilt/scale tables in
`recomp/src/ttk/first_person.inc`; `DNTTK_FP_WEAPON_SLOT<n>`, `..._TILT<n>` and
`..._SCALE<n>` override them for one launch.

## 2026-09-29 - D12 framing tweaks; next job D12A

User accepted most weapons as they look; shotgun, gatling and throwing blades
retuned (per-slot tilt/offset tables in `first_person.inc`). Next job chosen
by the user: D12A, keep the kick (Q) in first person.

## 2026-09-29 - D12 twin cannons (Needs playtest)

Slot 8 (key 5) framed after the Devastator references; HUD now over the
weapon (viewmodel in OT slot 1). Tune per weapon without rebuilding:
`DNTTK_FP_WEAPON_SLOT8=x,y,z`, `DNTTK_FP_WEAPON_TILT8=p,y,r`,
`DNTTK_FP_WEAPON_SCALE8=f`. Club test state: private copy of the D11 route
cards in `recomp/analysis/d12-first-person-weapons/club-cards` (slot 1, main
room facing the mirror after a 180-degree turn). Binary
`b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`.

## 2026-09-29 - D12 first-person weapons (Needs playtest)

Real hand + weapon meshes drawn as a viewmodel in first person; see
`documentation/63-d12-first-person-weapons.md`. Test state: private copy of
the street savestate in `recomp/analysis/d12-first-person-weapons/cards`
(slot 1; `PSX_LOAD_SLOT=1`), sources backed up in `.../before/`. `build.py`
still stops at the runtime patch drift: after changing hooks run
`psxrecomp_cli.py generate` with `PSXRECOMP_GAME`/`PSXRECOMP_BIOS` set to
`build-recompiler/`, then `cmake --build --preset local-dev` and
`build_movie_overlay.py`. Before launching an isolated instance, stop any
earlier one on port 9177 (a stale instance answered one survey). Re-survey
seeds with `DNTTK_FP_WEAPON_TRACE=1` (fire each weapon standing, level view)
if placement math changes. Next: user playtest of placement/size/feel; then
holster/draw animation, left hand, projectile origin. Binary
`1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`. No commits.

## 2026-09-29 - D11B Done (user accepted); jetpack sprite confirmed

User: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available"). Next first-person job: D12 (hands and weapon). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

## 2026-09-29 - D11B occluder fade in first person (Needs playtest)

User: conservative default is the best-looking; keep `DNTTK_NEAR_CLIP=0` as
Vanilla rendering and `full` for research. Invisible subway door = original
occluder fade (`0x80031fa0` loop, overlap test `0x8002ee50`, fade distance
camera+0xa0, Duke rect camera+0xa8). New hook `0x8002EE50` hands the test an
empty rect in the eye view; `DNTTK_FP_OCCLUDER_FADE=1` restores it. Both are
logged as D19 menu toggles. Private copies of the user's F7 slots in
`recomp/analysis/d11b-near-clip/subway-cards` (file slot 01 = F7 slot 4,
02-04 = F7 slots 1-3). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`. No commits.

## 2026-09-29 - D11B third pass: conservative default (Needs playtest)

User saw dotted black lines on floors at 1080p with the subdividing mode. Test
at 1080p by capturing the window (`g.wshot`, `XRES=1920x1080` in
`recomp/analysis/d11b-near-clip/probes/launch.sh`, `look.py`); the runtime's
`screenshot_file` shows the software raster, not the OpenGL window. Default is
now `conservative` (clip only what the original gets wrong, no subdivision,
whole-polygon sort); `DNTTK_NEAR_MODE=full` keeps the subdividing mode;
`DNTTK_NEAR_CLIP=0` is off. Street test state:
`recomp/analysis/d11b-near-clip/street-cards` slot 1 (fresh game, street
spawn). Binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`. No commits.

## 2026-09-29 - D11B second pass (apartment artifacts; Needs playtest)

User reported comb/popping on the apartment wardrobe ("closet"). Fixed by
average-depth piece sorting, a -4 OT prop bias (actors excluded via
`first_person_actor_drawing()`), prop takeover to 3072, outward rounding of new
corners, and a per-frame packet budget (the render arena is a 139,744-byte
ring; see `documentation/62-d11b-near-clip.md`). Apartment test state:
`recomp/analysis/d11b-near-clip/apartment-cards` slot 1 (debug savestate on
private cards); probes in `recomp/analysis/d11b-near-clip/probes/`
(`closet.py`, `walkseq.py`, `roam.py`, `apt_route.py`). Compare with
`DNTTK_NEAR_CLIP=0`; `DNTTK_NEAR_TINT=1` colors host pieces by renderer. Binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`. No commits.

## 2026-09-29 - D11B near-wall clipping (Needs playtest)

Code: `recomp/src/ttk/near_clip.cpp` (+ `near_clip.h`), hooks `0x80010000`
and `0x80011020` added to `game.local.toml` and regenerated with the built
recompiler (`PSXRECOMP_GAME=build-recompiler/psxrecomp-game ... psxrecomp_cli.py
generate`), wired through `first_person_view_live()` /
`first_person_duke_drawing()` in `modern_controls`. New native target
`ttk-near-test`. `DNTTK_NEAR_CLIP=0` turns it off for A/B.

Findings worth keeping: the level is drawn by two hand-written renderers at the
start of the executable (world `0x80011020`, object `0x80010000`); the GTE RTP
ring's `ra` for them is garbage (both use `$ra` as a scratch register - 0x100 is
H). `0x8002ef90` is a horizontal grid surface, not walls. Near the eye one
texel spans 15-25 pixels, so any host subdivision must cut on whole-texel lines
or straight texture lines kink.

Probe helpers: `recomp/analysis/d11b-near-clip/probes/` (`sweep.py TAG X Z YAW`
places Duke on the private route cards and sweeps; `perf.sh`; private port
9177, display :77). Wall spots: club side wall (5486, -4096) yaw -61.3; entry
corridor (5442, -800) yaw -90. D11 binary backup
`recomp/analysis/d11-first-person/d11-accepted-dd7b85b4.bin`, toml backup
`game.local.toml.before-d11b`. Binary
`d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa`. No commits.

## 2026-09-29 - D11 first-person prototype (Done, user accepted)

Code: `recomp/src/ttk/first_person.inc` (header comment lists every original
address it relies on), wired in `modern_controls.cpp` (`fp_blend`,
`eye_jump`, `orbit_constraint` blend, hooks `0x800348d8`, `0x8001ca4c`,
`0x8002a038`, `0x800b4d9c`), `pc_input` action `camera_view` (P) placed after
`camera_shoulder` (wire payload now 39 entries), profile schema 13 `view`,
`--view`. New `game.local.toml` hooks `0x800348D8`, `0x8002A038`,
`0x800B4D9C`; regenerated with the built recompiler (one file per hook); the
launcher rebuilt the movie shard.

Findings worth keeping: camera `+0x42` is H = 160/tan(angle at `+0x40`/2) =
386; the camera sits H/2 behind its anchor; the world renderer flags vertices
nearer than H/2 and the joint draw skips joints nearer than H. Duke's model
record at `[player+0x40]`: 19 joints, record `+0x44+0x28*j`, byte 0 bit 0 skips
the joint, byte 2 level; joint 9 is the neck/head. The original camera eases
per axis through `0x8002a038` with rates from `0x800c0d4c`. Save states are
taken at scheduler boundaries (VSync waits), but the debug server reads mid
frame, so a debug read can see the relaxed 768 word; the runtime repairs any
captured value.

Probe helpers are in the session scratchpad (`g.py`, `route.py`,
`wallcheck.py`, `perf.sh`, `launch.sh`/`start.sh`/`stop.sh`; private port 9177,
display :77). Fixture copies: `recomp/analysis/d11-first-person/` (profile,
cards from D10, `route-cards` slot 1 = club main room, first person; the D10
binary backup `d10-accepted-84669bb6.bin`). Developer override
`DNTTK_FP_PROJECTION=160..386` for FOV/near tests. User accepted D11 after extended play. Next:
D11B (near-wall polygons) and D12 (hands and
weapon) follow; D11A was cancelled (the P toggle suffices). Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`. No commits.

## 2026-09-29 - D10 Done (user accepted); D08S cancelled

User: "it's done, fully accepted" after about five minutes of play. D11 (first-person
prototype) is now ready. No commits.

### D10 recenter / shoulder / saved distance implementation notes

D08S cancelled at the user's request (keep the scope text; may revisit). D10
options pass: actions `camera_recenter` (V) and `camera_shoulder` (H) sit
before `weapon_previous` in `input_bindings.def`, so they are edges, not guest
commands. Wire payloads now have 38 entries; the native test fixtures were
updated. The camera side is in `orbit_begin`/`orbit_constraint`
(`modern_controls.cpp`, debug JSON `shoulder_offset`, `recentering`,
`recenters`, `rest_pitch`). Persistence: runtime side file
`<settings>.camera-state`, merged by `run.py` after exit and at launch; profile
schema 12. The probe is in the session scratchpad (`d10_probe.py`; phases
main/relaunch/nudge/strafe; `SLOT=1|2`, `EXTRA` launcher flags). The fixture
copy is `recomp/analysis/d10-camera`. The debug `quit` returns "emu busy or
frozen"; SIGTERM the game child to exit cleanly. Duke's heading equals the view
on foot, so any recenter yaw work must target states where facing diverges.
Binary `84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`. No
commits.

## 2026-09-29 - D08R Done; D08Q1 faster Modern Ctrl descent (Done)

D08R accepted ("it's absolutely rock solid"). D08Q1: `k_jet_descend_velocity`
5500 in `jetpack.inc` (Modern only) gives 29.5 units/frame, the underwater
Ctrl dive rate from `/tmp/swimlab/deep4.log`. User: "verified working!!!". Probe script `descent` in the
scratchpad `jet_probe.py`; health is `player+0x32`. Binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`. No commits.

## 2026-09-29 - D08R selectable jetpack scheme (revision 2, now Done)

`--jetpack classic|modern` -> profile schema 11 control `jetpack` ->
`DNTTK_JETPACK`. Gate `jetpack_classic()` in `modern_controls.cpp`. Classic
shares the entire Modern control path (lease, `face_view`, WASD bridge, J,
fall grace); `jetpack_update()` in `jetpack.inc` returns before the host
vertical layer (hover, Ctrl descent, trim). Do not give Classic the original
camera or D-pad turning again: the user rejected that (revision 1) because
mouse turning failed and controls slipped out of modern. Probe harness copy
lives in the session scratchpad (D08Q `/tmp/jetlab/jet_probe.py` plus
`classic2`, `wcheck`, `turn2`, `tables`; `PORT`/`EXTRA` env). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`. No
commits until asked.

## 2026-09-29 - D08B accepted (Done)

User: "d08b can be marked as completely done man, its fine. accept it all"
and "i have tested all of them!". The user playtested the old D08B limits
(contact jumps, bed run-off slowdown, unopened-bed pipe bomb, oblique couch
entry, pig-cop ladder stall) and accepted them. Board and status updated; no
build or automated replay. D10 is the only
In progress job. No commits until asked.

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

Intro-only stutter after D23A. Cause: `psxrecomp-game --overlay-config-hash`
covers the `game.local.toml` hook list, so each new host hook moves the
movie shard's cache folder (`cache/SLUS-00583/gcc/linux-x64/cg10_..._gc<hash>_f0`)
and the loader silently interprets MOVIE.OVR. Now: `ttk-movie-shard` target in
the `local-dev` build preset, `run.py` `ensure_movie_shard()`, and
`fmv_poll.c` logs `ttk-fmv: native movie decoder active (19 functions)` or a
WARNING carrying `overlay_loader_last_msg()`. After adding a hook, just build
or launch normally. If the intro stutters: check the `ttk-fmv:` line in the
session log first. Launch tests in `tests/local/test_player_profiles.py`
must pass `--no-session-log` and stub `run.ensure_movie_shard`, or they
start the real game on the player's cards. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits until asked.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Done, user-accepted)

User: "that stuttering audio/slowness issue" before the D08Q playtest.
Measured with the runtime's telemetry in an isolated Xvfb instance
(`/tmp` harness; `audio_stats`, `phase_profile`, `phase_hot`, `frame`,
`ttk_input`): Modernized 47.5–49.3 fps + continuous underruns, Vanilla
60.0 clean, Sep‑27 binary 57.5. Cause: `identity()` walked all 20,305
guarded words via `psx_mod_read_word` on every call, 46 calls/frame ×
108 µs = 5 ms/frame (24 % wall). Fix: `code_identity.h` memcmp fast path
on `g_psx_ram` (exact per-word fallback keeps the apartment substitute),
`IdentityMemo` per `input_host_frame()` × `g_dirty_ram_code_gen` in
`modern_controls.cpp` and `weapon_aim.cpp`. Now 1 check/frame, 28 µs,
59.94 fps, 0 underruns, fill 267 ms. Debug JSON `identity_calls`,
`identity_checks`, `identity_us`. Tests: mocks define `g_psx_ram` /
`g_dirty_ram_code_gen`, `poke()`/`pokeb()` bump the generation; new memo
case. All three native suites PASS. If stutter returns: check
`identity_checks` ≈ 1/frame and `audio_stats.out.fill_ms` vs target first;
`perf` is unavailable (paranoid=4). Do not add per-call guard walks again;
new guards are now nearly free. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. No
commits until asked.

## 2026-09-29 — D08Q modern jetpack flight controls (Done, user-accepted)

Accepted: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."


User: jetpack uncontrollable except Space; WASD "blocked"; modern controls
fail. Isolated baseline on `79a3fd5b…` (turret-room / ledge states copied to
`recomp/analysis/d08q-jetpack/`, `dninventory` cheat, J, Space): original
mode 10 (anims 163–170) matched no lease → no mouse; WASD via tank fallback
relative to an unturnable body; idle gravity landed Duke in 1–2 s. New
`recomp/src/ttk/jetpack.inc` from the existing `8005a210` hook, camera-only
lease clause `jet` in `state()`, `jetpack_input_ready()` for `input_pad()`
(W/S → Up/Down, A/D → layout strafe pads) and `select_weapon` (J only in
flight), `locomotion_input_ready`/`airborne_input_ready` false in flight and
the 108 fall grace. Host writes: hover lock `+0x224 |= 0x08000000` + base
`+0x860 = Y` when idle; Ctrl → lock off, `+0x1f8 = 2800`, `+0x1e8 = 0`; WASD
→ lock off, `+0x1f8 = 0`, `+0x1e8 = 18` trim; `face_view()`. Do **not**
drive `+0x860` for descent or hover during WASD (57-jetpack-controls.md,
"Rejected on evidence"). Guards `8004aaf8`/272, `8004ac08`/472,
`8004ade0`/1972. Live probes: headings within 4° of the camera, level ±25,
25–29 units/frame, Ctrl 489/30 frames → 105, J off → 108 with live mouse →
ground run. ttk-input-test, ttk-aim-test, ttk-controls-test PASS (controls
test: use `analysis/pc-input/d08-camera-final/level00-guard-fixture.bin`;
the `d08p-turret` fixture is Modernized-patched and fails the Vanilla
assertion). Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. Playtest:
J, Space, mouse, WASD, release → hover, Ctrl → land, J in the air. If the
feel is off, the tunables are `k_jet_descend_velocity` (2800) and
`k_jet_level_trim` (18) in jetpack.inc; `DNTTK_JET_TRACE=1` logs each
update. No commits until asked.

## 2026-09-29 — D08P overlay scratch identity + mid-depth wade handler (Done)

User: the whole ledge area (F7 UI slot 3) kills run, mouse and jump for good;
depth is not the cause. Their `session-20260929-005059.log` showed
`[TTK lease] inactive (identity)` from the ledge onward. Live guard diff on the
isolated slot 3 copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's
trailing scratch vector, written by the zone script's `0x8007177c` hit test
from overlay code `0x800cb1a4`. The overlay guard now covers 9652 bytes
(code/tables), digest `274d71dd…`; `ttk_state_probe.py` matches; the controls
test proves the vector write keeps the lease and the last table word is still
guarded. Then the same water (depth 512, original mode 1 / clips 80/81,
Vanilla 8–14 units/frame) got its handler `0x800539f8` hooked — added to
`game.local.toml` `mod_function_entry_funcs`, regenerated (one generated line;
`build.py` itself stops at the pre-existing runtime patch-stack drift, so
generation was run directly with the built recompiler) — retargeting the
world root to camera-relative WASD in the land run band (gain 4, 80..120 per
tick). Guards added for `0x800486b0`/260, `0x800539f8`/452, `0x800788e0`/200.
Live: 45–58 units/frame, mouse steering, strafe, wade jump 98, 0 refusals,
turn/run/jump after the zone; slot 2 regression clean. Wading into a wall
idles (original handler); turning resumes. ttk-input-test, ttk-aim-test,
ttk-controls-test PASS. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`.
Isolated cards: `recomp/analysis/d08p-slot2/cards/openbios/` now holds copies
of state slots 01 and 02. User playtest: "it really works. perfectly" →
D08P **Done**; docs committed and pushed. Next session: pick the next job from
the board (no job auto-started).

## 2026-09-29 — D08P held keys across recapture, fresh capture offers (Needs playtest)

User: iteration 4 changed nothing for them (session log shows a fresh launch
and F7 slot 2 load 30 s before the report). Live Xvfb + xdotool probes
(`/tmp/d08p_live_probe*.py`, isolated `recomp/analysis/d08p-slot2/`, software
and OpenGL, debug load and the real `F7 → 2 → l` menu, the user's own profile)
all show the wade at 48–54 units/frame with mouse look; slow headings are the
original wall slide beside the spawn. Reproduced instead: a Shift/W held
through any recapture was wiped by `clear()` → walk gait + Walk pad until
re-pressed. Fixes: capture resyncs bound keys/mouse buttons from SDL device
state; capture offers expire after 8 frames so the original pause menu is not
captured; sustained tank fallback announces `ORIGINAL MOVEMENT (reason)`;
`run.py` mirrors stderr to `recomp/build-local/logs/session-*.log`. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`.
If the playtest still degrades, read the session log's `[TTK lease]` /
`[TTK input]` lines first. Turret itself not reached in probes (S doorway →
bridge → deep water; N platform → W walkway dead end). No commits until asked.

## 2026-09-28 — D08P waist-deep land locomotion (Needs playtest)

User: after speed recovered, the turret wade still crawled, mouse look was
dead in the water, and after leaving they only walked with no Shift. Isolated
slot 2 already uses land run anim 78 on D-pad forward; Modernized dropped
the lease on tank turns 70/71 and injected Walk. Waist-deep / mid water
now keeps the land camera lease, faces the view, converts 70/71 to run,
and does not inject Walk. ttk-input-test, ttk-aim-test, ttk-controls-test
PASS. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`.
Playtest F7 slot 2: run + mouse in the wade and after stepping out. No
commits until asked.

## 2026-09-28 — D08P identity reader stall (Needs playtest)

User: after the slot-2 recapture binary, everything ran slow including
audio. The apartment pair check ran on every word of every identity
guard, many times per frame. It now only inspects the two LEVEL00
apartment words. F7 recapture behavior is unchanged.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`.
Restart the player build; the running session will not pick this up.
No commits until asked.

## 2026-09-28 — D08P F7 slot 2 recapture (Needs playtest, iteration 3)

User: iteration 2 did not restore the turret room; F7 UI slot 2 (file
`state_800AB6FC_slot01.pst`, turret around the corner) lost Modernized
entirely. Isolated dump matches the flooded wade (depth `0x100`, anim 63,
normal camera, apartment words already patched). F10 left
`initial_capture` false; F7 released the mouse and did not request
recapture; the F7 menu could eat `capture_offer`. F7 now sets
`initial_capture` even if the cursor is already free; offers stay until
`allow_capture`; identity remaps the complete patched LEVEL00 pair.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS (including F7
recapture and slot-2 patched overlay). Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`.
Playtest: F7 load slot 2, mouse/WASD/strafe/crosshair without a manual
F10. No commits until asked.

## 2026-09-28 — D08P turret wade (Needs playtest, iteration 2)

The placed checkpoint is host F7 savestate slot 1, not a memory card. Isolated
dump: flooded turret room, depth `0x100`, anim 63, normal camera, no
`0x20000000`. First tank fallback made A/D turn (D-pad L/R → anims 71/70) and
classified 70 as free swim, which hid the crosshair and blocked Escape
recapture. Fixes: free swim only at original `≥0x281` or states 4/5; A/D
fallback L2/R2 strafe; recapture from `0x8005a210`; reticle on camera-only
locomotion. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`.
Playtest from that savestate: mouse, A/D strafe, crosshair, Escape resume.
No commits until asked.

## 2026-09-28 — D08P turret WASD freeze (Needs playtest)

Crystal-2 ceiling turret (after first crystal, broken-bridge water corridor)
dropped the modern camera lease. Captured WASD was inert because move actions
use pad 0 and Forward inject requires the lease. F10 could not restore WASD.
First fix: captured play without a lease feeds original D-pad tank bits
(iteration 2 replaced A/D turn with strafe). stderr
`[TTK input] WASD tank fallback` and `[TTK lease] inactive (...)`. Playtest
that room; if the window ignores keys entirely, that is a guest halt, not this
path. D07A/B, D08A, D08H and D10 distance closed on user play acceptance.
Binary `fd9e4c4d015673ec6fa1dcdedb3bcd60d9b74ddb4377786afb5a241d0f4e4c28`.
No commits until asked.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User: do not add a scuba device to this game. D08N is **Cancelled**. TTK
underwater air stays original and automatic; Bio Mask stays gas-only. No scuba
prototype, inventory slot, HRP scuba art, or toast. Historical notes that once
listed D08N as a swim follow-on are superseded. Next job: pick from the Todo
list (D08L, D09, and other open rows; not D08N). No code change. No commits.

## 2026-09-28 - game/ drop folder (tested)

First run of `run.py` / `build.py` creates `game/` with a README. Importer accepts
Redump USA `.cue`/`.bin` or CloneCD `.img` (hashes `708c0404...` and `230a34c2...`)
when the EXE pin matches. Europe/PAL `SLES-01515` prints a console error. User
tested the drop-folder flow. Original media is never overwritten.

## 2026-09-28 — D08O closed (accepted); nothing pending from the swim work

Playtest 3 on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb` accepted
everything: A/D strafe, Ctrl dive, underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the round-3 shallow → platform ledge jump, and the `biomask-small.png`
item-switcher icon. D08O is **Done**; the entries below are history. Do not
reopen the swim/ledge code without a reported regression (55 §6 rules 7–9).
Next job: pick from the Todo list in `MODERNIZATION_JOBS.md` (D08L, D09 are
open; D08N scuba is cancelled). No commits.

## 2026-09-28 — D08O.4 ledge jump: plain 98 arc, taller (Needs playtest)

Playtest on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.
Swimming (Ctrl dive, underwater WASD/Space/Ctrl, A/D strafe, mantle) and the
Bio Mask icon are **accepted** — do not touch. Only the ledge jump changed:

1. **Subway shallow water → platform** (west platform, room 21 → 57). From
   touching the wall, from a few steps back, running or standing, Space alone
   or with W: Duke does the ordinary directed jump (98) — not the tall 97 hop of
   round 2 — about a head higher than his normal shallow-water jump, and lands
   on the platform. At the wall / one step back he rises first and moves onto
   the platform only once his feet are above the lip (XZ held, then 3400/4000);
   from further back he travels from takeoff (1900 + 3.5·distance, max 6400).
   Landing is 98's normal landing (sometimes the hard 105). From ≳1650 units the
   ledge is out of probe reach and the plain 98 falls short — intended.
   `ledge_assists` in the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1`
   prints `ttk-ledge probe/launch/clear/abort/ballistic` and per-tick `ttk-swim`
   lines to stderr (`runs/<name>/runtime.log` in the lab).
2. If it still looks too high: `k_ledge_vy` (−9000; 98's own ≈−6900, rise scales
   ≈ VY/96·ticks — −8000 was not tried, apex margin at −9000 is ≈200 above the
   635 ledge). If it bumps the wall: `k_ledge_hold_distance` (320) / the hold
   speeds. If it overshoots into the back wall: `k_ledge_speed_per_unit` (3.5).
3. Regressions to keep: everything in the D08O.2/.3 entry below.

Mechanics (`swim.inc`): `swim_ledge_ahead` (probe 0/320/640/960/1280),
`swim_try_wade_jump` → `swim_start_airborne_jump(…,98)` + `delayed_vertical =
k_ledge_vy` (applied at the `8003ebf4` ballistic hook, `terrain_epoch` gated),
`swim_ledge_ballistic` (called from the same hook each ballistic tick: hold
XZ = 0 below the lip when `swim_ledge_hold_xz`, else exact XZ via
`swim_boost_want` / `swim_boost_horizontal`; clears `swim_ledge_pending` once the
foot is 32 above the top). `swim_update` drops a stale pending flag whenever
anim ≠ 98 or state ≠ 9. Lab: `/tmp/swimlab/ledge17.json` (15 trials),
`deep3.json`, `walk`. No commits.

## 2026-09-28 — D08O.2/.3 native dive + wade ledge assist (Needs playtest)

Playtest on binary
`71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5`:

1. **Ctrl at the surface** → Duke dives (original anim 133, `+0x22c` = 5).
   Underwater: **W/A/S/D** swim where the camera looks (thrust 128–130),
   **Space** ascends, **Ctrl** descends, looking up/down + W follows the view.
   Duke surfaces automatically (122, state 4); Ctrl again re-dives. If Ctrl is
   still inert: `swim_dives` in the debug JSON must increment; `swim_try_dive`
   refuses while `+0x224 & 0x241` or `+0x244` (timer) is non-zero.
2. **Subway shallow water → platform** (west platform with the hazard stripe,
   room 21 → 57). From touching the wall, from a few steps back, running or
   standing, Space alone or with W: Duke should hop high and land on the
   platform every time, with no bump-off (107) and no splash-cancel. Landing
   is the original hard-landing 105 (the 97 arc is higher than 98's). From
   >≈1300 units the plain 98 still falls short — intended. `ledge_assists` in
   the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1` prints
   `ttk-ledge launch/switch/abort` and probe lines to stderr.
3. Facing a plain wall in shallow water with W+Space → vertical hop (was a
   cancelled 98 splash).
4. Regressions to keep: A/D surface strafe (123/124), Space ascend only, E /
   mantle exit, shallow W/S/A/D wade, Space 98 in open water, Ctrl 105.

Mechanics (`swim.inc`): `swim_ledge_ahead` (standing probe `800797f4` from
0/320/640/960 along the view, world XZ written and restored inside the query),
`swim_try_wade_jump` launches 97 and arms `swim_ledge_pending`,
`swim_ledge_assist_update` (early hook, runs without the swim lease) switches to
98 when `+0x10` ≤ ledge top − 96, keeps VY via `delayed_vertical`/`delayed_takeoff`
(applied at the `8003ebf4` ballistic hook), XZ = 2200 + 3.2·distance (max 5400)
via `swim_boost_want`; `swim_reassert_boost` now also runs in flight.
Native swim: `swim_try_dive` (`80045564`), `swim_steer_underwater`,
`swim_thrust_input_ready` (Square injection in `pc_input.cpp`). Lab scripts live
outside the repo in `/tmp/swimlab` (`swim_lab.py`, `ledge*.json`, `deep3.json`).
No commits.

## 2026-09-28 — D08O implemented (Needs playtest)

Playtest the deep-water trio on binary
`d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e`:

1. **A/D** while swimming → camera-relative strafe (anims 88–93), no turning.
   If A/D still turn, dump `800d1a68/800d1b80[player+0x233]` — the strafe pad
   bits are resolved from those layout words (default L2=0x100 / R2=0x200).
2. **Ctrl** at the surface and mid-water → descend; **Space** → ascend to surface.
   Dive stops at `+0x1c8 − 0x1e0` (floor margin) or `surface + 0x1800`.
3. **Space** at a ledge no longer hops out; leave via **E** / original mantle.
   Shallow wade jump / ledge hop (`8003e2d0(97|98)`) is unchanged.

Regression already confirmed in the shallow subway savestate: W/S/A/D wade with
stable heading, Space wade jump 98, Ctrl crouch 105. ttk-controls-test and
ttk-input-test PASS. No deep-water savestate exists; deep zones (rooms 31, 35,
42, 64, 65, 68, 72) sit behind a keypad door (−7400,51700) and a fence
(40636,49390) from the subway state, and teleporting snaps Duke to geometry.

Code: `recomp/src/ttk/swim.inc` (`swim_strafe_pads`, `swim_apply_vertical`,
`swim_try_mantle` removed), `pc_input.cpp` free-swim block,
`modern_controls.h`, `tests/local/pc_input_native.cpp` stub. No commits.

## 2026-09-28 — D08O marked next (docs only)

**Next job to pick up: D08O** — deep free-swim polish.

Locked design in `documentation/55-swim-controls-research.md` (§1.2, §8) and
`documentation/54-swim-redesign.md`:

1. Fix A/D camera-relative strafe (W/S already OK)
2. Fix Ctrl descend from surface / mid-depth
3. Strip free-swim Space→ledge exit → **mantle / E only** (keep shallow wade jump)

D08M closed: shallow jump + ledge hop Done (do not reopen unless regression).
Space↑, W/S, E mantle accepted on binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

Hard rules: no invented `+0x20c`, no entry `+0x1f8` clear, no world-XYZ swim.

This session: documentation/design only — no `swim.inc` changes.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

No commits.
