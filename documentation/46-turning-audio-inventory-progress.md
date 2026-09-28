# Selected follow-up progress — 2026-09-27

Scope/order: D10A, D18A, D08I, D08A1, per note45. D08K remains blocked;
D19B deferred. Preserve accepted aiming/furniture/ladder/silent-cheat work.

## Resumable result

Final binary SHA256: `a21f947840c945be504b23e112f7dececa86dda4a25d7d245b3debb25028bf52`.
D08A1 is implemented and Needs playtest. D10A and D08I remain Needs playtest
without speculative behavior changes. D18A remains In progress: club starvation
is reproduced, but its root bottleneck and a verified repair are unresolved.
D08K remains blocked and D19B deferred. The chronological checkpoints below
include excluded attempts; later results supersede their pending language.

Current and older controlled Vanilla spawn captures both completed alive with
zero new starvation/overflow during ambience and gunfire. Older evidence:
`analysis/pc-input/iteration46-audio-older-vanilla-fixture2`, approximately
8.02/8.06 seconds, 483/484 guest frames. These are spawn comparisons with the
explicit private invulnerability fixture, **not matched club comparisons**.
They do not show that Vanilla is generally free of crackle. Club no-poll captures
remain the positive reproduction; isolated voice and the player's precise ledge
combat route remain unverified. Accelerated boot audio is not evidence of
normal gameplay speedup; paced club measurements were below normal frame rate.

Resume D18A with matched club captures across profiles/builds and targeted
producer/frame-time attribution. Retain original source PCM and output-monitor
captures, investigate voice separately, and verify a proposed change against the
positive club reproduction before changing queues, resampling or mixer behavior.
Host profiling permission limits remain; no security settings were changed.

Player checks: rapid physical mouse sweeps while Shift-running and firing; the
exact close-bed Shift+W+Space position/timing; Enter on the visible highlighted
gadget (including held Enter past strip expiry), U/custom use, then fresh Enter
with the strip hidden for ordinary pause. Audio listening remains required after
a real repair; this build does not claim to fix it.

Launch with the saved profile:

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

## Final verification and cleanup

Normal image/build/ordered patch workflow passed. All six native suites passed;
60 Python tests ran successfully with two conditional skips. Native input was
rerun after the final mouse-button prompt-name correction. OpenGL/default and
Software/custom inventory routes passed as detailed below.

The exact final a21f9478 build completed the separate **unmodified Vanilla**
14-checkpoint headless route with exit0 and no driver error:
`analysis/vanilla-regression/iteration46-final/report.json`. Jump, original
inventory, movement/turn and post-fire checkpoint captures were reviewed. The
post-fire still alone does not prove a shot; this bounded route is not a campaign
or audio-quality certification. Run command:
`python3 recomp/tools/local/vanilla_regression.py --name iteration46-final --mode vanilla --state-probe`.

`analysis/iteration46/verification.json` confirms all 11 recorded player
settings/card/state files unchanged and exactly six intended source/test edits.
Temporary runtime timing instrumentation was removed; no audio, aiming,
traversal, ladder or cheat behavior code changed. Original assets and unrelated
edits were preserved. All owned test games exited; only the private audio module
536870913 was unloaded. `analysis/iteration46/cleanup.json` records no remaining
owned game or private sink. No commits were made.

## Entry checkpoint

No player game was running at entry. Existing dirty root/submodule work was
inventoried; baseline source copies and hashes are under private
`recomp/analysis/iteration46/before` and `source-before.json`. The delivered
executable was copied to `iteration46/baseline-player`. Player configuration/card
hashes are in `iteration46/preservation.json`. Original assets were not modified.
No generated C will be edited. All test games use their own Xvfb display,
`analysis/pc-input/iteration46-*` cards/profiles and port9164.

D10A in progress. Actual SDL input native test now compares 1600-pixel single
packets with 80 × 20-pixel packets, with/without Shift. Both receipt totals agree.
This is input evidence only, not desktop mouse feel or camera acceptance.
Live baseline driver/route: `iteration46/driver.py`, `turn.py`; evidence/log:
`iteration46-turn-baseline` and `iteration46/turn-baseline.log`. Driver owns and
cleans its process tree. Tests use unchanged player binary, dummy audio for D10A.
No turning cause or fix established yet.

## D10A first comparison

Sixteen baseline street comparisons completed: 20x20, 1 × 400, +/-800 pixel sweeps
in standing, Shift-only, W and Shift+W. Requested yaw matched every injected
distance to diagnostic rounding (<0.000001 rad). The actual camera matrix was
within 3.82 degrees at the short post-sweep sample; running samples included original
bump94. No reproduced lost-turn failure yet; no camera behavior changed. Native
SDL packet batching/Shift comparison passed. Live evidence is in
`analysis/pc-input/iteration46-turn-baseline/turn-{rows,results}.json`.

Private continuation `iteration46-live`: port9164, separate Xvfb/cards/settings,
real PulseAudio SDL backend intended for temporary sink `dn_ttk_iteration46`.
Initial PULSE_SINK request was ignored: own test PID119711/stream5108 went to physical sink56. The user heard crackle/apparent speedup during accelerated boot navigation. Moved only that verified private stream to sink5102 and confirmed it. No player/other application stream was moved. Module ID is stored in `iteration46/audio-module-id`;
unload that exact module after tests. `session.py` runs one job at a time from
the private session's `job` file; create `finish` to let its owner close the game.
This routing can capture software output; it cannot certify physical-speaker quality.

User live feedback: background test audio was “really crackly” and perhaps sped up. This is fresh audible evidence, but boot/navigation turbo was active in that interval; separate paced gameplay capture is required before attributing cause.

D10A club entry reached naturally (door5 route and reviewed `inside-club5.png`).
24 additional sweeps started in the club but moved back onto the street; do not count them all as interior. A second focused 16-case fast-turn route remained inside (Z -1500..-4523), with walking/running and right mouse released/held; all matched requested yaw within 0.000001 rad. Original bump95
was included. No turn-loss reproduction or supported behavior repair established.
Keep D10A Needs playtest with physical mouse/high-rate/compositor and combat limits.

D18A now recording paced club-area ambience versus gunfire through all three
runtime taps and the verified private PulseAudio monitor. `audio.py` uses 480
guest frames per segment and records elapsed wall time and counter differences.

## D18A paced capture checkpoint

Actual software audio-server monitor WAVs and runtime taps captured privately.
`iteration46-live/audio-results.json`: club ambience 481 frames/8.355s, 10,741
starved output samples; gunfire 481 frames/9.325s, 55,888 starved output samples.
The bridge counter counts **samples**, not separate audible clicks. No new
overflow drops in either interval; correction at -0.5% attempting to recover fill.
The source/host/monitor PCM peaks stay below full-scale. `audio-metrics.json`
records peaks/RMS/step checks; that does not establish absence of all source artifacts.
`*-profile.json` shows broad time attribution, not function-level root cause.
Host perf and ptrace profiling were denied; system security was not changed.
No audio repair shipped; do not enlarge buffers/mute sources to disguise missed
deadlines. Physical-device listening, exact ledge combat, isolated voice, Vanilla
and older-build matched comparisons still required.

The first street capture used correct audio-server monitor output but an incorrect
implicit runtime tap start (oldest ring slice). Retained under `street-audio-first`;
those tap files are excluded. The club rerun uses explicit absolute sample starts.

D08I now reproducing from a natural club-exit/apartment ladder route; no gameplay
source change yet. D08A1 implementation follows that comparison.

D08I apartment reached through natural movement and first exterior ladder;
`fresh-route.json` / `inside.png` in `iteration46-live`. Close-bed sweep `bed.py`
compares preheld Shift with simultaneous direction/jump, no Shift, W three/eight
frames first, and jump three frames first. Diagnostic keys use persisted isolated
J binding (Space remains player default); raw input order is recorded.

## D08I bounded result / D08A1 baseline

Twenty close-bed attempts completed across three sweeps. First attempt began at
Z9199 and is excluded from close-contact acceptance. Remaining 19 at the bed
(walking contact, repeated contact, running contact; Shift preheld; W offsets
0/1/3/8/12/20/40 and jump-first -3; no-Shift controls) all transitioned from97
to98 after clearance and landed forward. No lost-direction reproduction. Native
and prior mechanics remain unchanged; no speculative jump repair. Keep Needs
playtest for exact player's geometry/timing. `bed*-{rows,results}.json` records
receipt, original state, support/foot, velocity, landing and handoff counters.

D08A1 failing native test recorded in `native-enter-baseline.log`: visible strip
Enter releases capture/pauses instead of enqueuing item_use. Candidate shares the
strip visibility predicate, routes unmodified Enter to existing item_use, and
retains press ownership through expiry/focus/capture until release. U/custom
bindings remain independent; prompt uses the actual saved item_use binding.
Native held/repeat/focus/expiry/capture/menu checks and inventory rasterizer pass.
Live baseline/candidate and both-renderer review pending.

An opt-in code-identity timing diagnostic (`DNTTK_IDENTITY_PROFILE`) is temporarily
compiled for D18A attribution; it does not cache permission or change behavior.
The old live process still maps the unchanged baseline executable.

Further D18A control: apartment ambient/gunfire with **no repeated debug polling**
(`audio-nopoll.py`, latest `iteration46-live/audio-results.json`) ran 483/482
frames over 8.029/8.025 s with zero added starved samples or overflow. The club
capture and no-poll apartment capture are different locations, so this does not
isolate polling as the cause or close the reported club/ledge audio issue.
Keep the distinction; a matched no-poll club comparison remains necessary.

Live Enter baseline confirmed while highlighting jetpack: Enter opened pause and
released capture (`inventory-enter-baseline.json`, screenshots). Private baseline
process closed through its driver. Candidate GL inventory route uses dummy audio
and opt-in identity timing; player files remain untouched.

## Inventory live verification and release build

First GL run passed activation, no-item, held timeout, fresh Enter pause, U and
original menu; its helper incorrectly expected automatic capture after an explicit
F10 opt-out. That assertion was a driver error; the original manual F10 policy
was correct. The corrected full GL route (`iteration46-enter-final-gl`) passes all
checks including explicit recapture and focus recovery. 320/640/1024 captures
reviewed: original tile geometry/arrow/name/charge retained, one use-hint row added.
Software custom O/P/L full route is running on the release binary.

Opt-in identity timing measured roughly 7–11% of wall time in sampled modern
checks; it does not by itself establish the audio bottleneck. Diagnostic source
was saved privately then removed. All live bytes remain checked on every identity
invocation. Full owned-disc import/generation/ordered patch/build/movie-cache
workflow passed (`full-build.log`), followed by removal-of-diagnostic rebuild
(`build-release.log`). Final source edits are only pc_input.cpp/.h,
inventory_hud.cpp/.h and the two corresponding native tests, plus documentation.
No movement, aiming, terrain, traversal, cheat or audio behavior source changed.

Release SHA256: b1472b3f36ac7cad6d4fb0a450258dd2581a48f4515b0b408b91a0f52b7b9340.
Six final native suites pass; Python 60 tests pass with 2 conditional skips. The
full GL pass used the same inventory implementation with disabled diagnostic
code still present (hash14803060...); exact-release GL smoke is included in the
next private audio route. Keep build identities distinct in reports.

Software full route on exact release hash passes with O/P/L custom bindings;
reviewed 320-pixel capture displays Enter / L: Use and unchanged tile styling.
GL full route and Software cover no-item, held beyond timeout, fresh Enter pause,
toggle on/off, U/L, original inventory, explicit capture release and focus return.
No settings migration/rebinding was added.

Audio routing retry: SDL also overrides the requested app-name hint; first
`iteration46-audio-club-nopoll` aborted before input checks when stream lookup
failed. Driver now identifies only the process with this exact private cards
argument, moves that stream, and records its ID before any navigation. Retry
`iteration46-audio-club-nopoll2` verified stream6413 on private sink5102.

## Final club checks

`iteration46-audio-club-nopoll3`: exact b1472b3f... GL Enter on/off smoke passed
after waiting for original activation to settle and re-highlighting (the first
smoke retry attempted the second action too soon; it is excluded). Eight further
fast ±48/96-degree sweeps while actually firing and walking/running stayed inside
the club, matched yaw within 0.000001 rad and registered five aimed shots. Total
D10A comparisons 64 across all routes; no lost yaw reproduced. Screenshots confirm
club location and ammo decrease; this is not physical high-rate mouse acceptance.

The no-poll club capture still starves: ambience 19,851 and gunfire 42,751 output
samples over about 8.02 s each, with no overflow. Source generation was below normal
wall-clock rate (about56 and52 guest frames/s), not faster; repeated frame polling
alone does not explain the club failure. WAVs and per-phase counters retained.
No full-scale PCM rails; pre-volume mixer clipping/source artifacts remain unruled
out. These measurements establish a timing contributor, not its exact hot function.

Final review corrected the new prompt's mouse names to match existing binding
tokens: SDL button3 is Mouse2 (right), SDL button2 is Mouse3 (middle). Added native
name checks; no input mapping changed. Final rebuilt SHA256:
`a21f947840c945be504b23e112f7dececa86dda4a25d7d245b3debb25028bf52`.
Only this prompt-name correction differs from the b147 live UI/club build.
Final native input suite passes again; other native/source paths are unchanged.

Vanilla audio comparison correction: the first spawn run died before its gunfire
interval (animation215; reviewed continue-screen capture). Its gunfire/quiet
result is excluded from playback acceptance. Repeating current/older Vanilla
with a declared private RAM fixture: original invulnerability halfword800c3cc6
set to1 before resume, after checking live player/camera ownership. No source,
asset, player settings or saves change. `audio-fixture.json` records exact writes;
start/end animation checks reject death. This is a controlled audio comparison,
not unmodified Vanilla campaign acceptance. A separate unmodified Vanilla route
will remain the gameplay regression evidence.

Current Vanilla controlled spawn capture passes: about8.02/8.06s ambient/gunfire,
zero new starved samples and zero overflow, living-player checks retained. The
fixture is explicitly different from natural unmodified gameplay. The earlier
local build is `analysis/d08-responsive-input/player-before`, copied privately:
SHA256 `a607e052a401a04a6184af31f557c5a3719bc85ad353117aa95958160e7b78dd`.
Its first launch lacked executable-adjacent bundled BIOS and exited before
playback; retry uses read-only links to the existing bundled BIOS/cache/font
files, without replacing the player executable.
