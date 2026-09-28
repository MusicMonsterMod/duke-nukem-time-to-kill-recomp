# Development log — 2026-09-25

## Initial inspection and decisions

Inspected the three original CloneCD files without writes. Enumerated 548 files, extracted executable metadata in memory, grouped 50 overlays into 30 unique contents, located candidate diagnostic references and extent-table pairs. Compared upstream disc metadata and found an exact boot-executable match but a whole-image mismatch. User chose Linux + Windows, recomp-first, extending the existing project.

## Implementation

Cloned the exact upstream commit and recursive submodules into `recomp/`; created branch `local/native-pc-bringup`. Added a separate documentation folder as requested. Installed Ninja and Capstone only into the local virtual environment.

Added a standalone C Mode 2 checker, standard-library ISO/CloneCD tooling, guarded local import, executable-analysis script, local identity profile, CMake preset, portable build wrapper, debug client, tests, and a no-game-input Linux/Windows tooling workflow.

## Integrity

All 191230 sectors passed structural checks, EDC, and applicable ECC P/Q. Form 2 EDC was populated in all such sectors. Prepared image is byte-identical to the original. Subchannel identity and size were recorded but subchannel semantics were not decoded.

## Generation and compilation

Both native emitters compiled from pinned source. OpenBIOS and resident game code generated successfully. Game output: 67 shards, 2561662 C lines, and 3833 dispatch entries. The generator noted 426 suppressed reserved-opcode/data-as-code warnings and several out-of-function branch/jump boundaries. These remain research issues.

Linux CMake configured with generated game code and linked the native runtime successfully. Local preset uses RelWithDebInfo, debug tools, no launcher UI, no Vulkan, and no new netplay. Windows has not been executed on this host.

## Headless boot observations

Started with the exact local profile and CUE, a separate blank card directory, OpenBIOS, debug port 9123, and overlay auto-compilation disabled. Runtime confirmed the expected BIOS identity and executable text guard. Framebuffer captures showed intro content, the title screen, and the main menu. Active-low Start input advanced to the menu.

Resident `dispatch_stats` reported zero misses at a title/menu checkpoint. However, `dirty_ram_stats` reported tens of millions of blocks and hundreds of millions of interpreted instructions. Therefore zero resident misses is not complete native coverage. No overlay compilation runs occurred at that checkpoint.

The first debug request used unsupported `get_state`; the server returned an error and closed the connection. Corrected the client to one JSON command per connection and used `frame` rather than `get_frame` for the live frame counter. These were tooling corrections, not game fixes.

## Tests

12 local tests passed, including real-sector corruption detection in scratch files. Upstream setup-executable-name parity test was also run. Raw logs remain local under `documentation/logs/`.

## Gameplay and windowed checks

A rapid Start/Cross route reached the first level with Duke visible outside the starting building. Pulsing D-pad Up and Right changed player position and facing in successive captures. HUD health fell from 100 to 92 and then 85 during the observed route. This verifies a narrow movement/damage slice, not jump/shoot/save/progression.

The runtime explicitly rejected the registered `pause` command; no paused-state assumptions were used afterward. The headless process exited with code 0 through TCP `quit`.

A separate windowed process initialized OpenGL 3.3 on NVIDIA, set the GL GPU pipeline up, and progressed through boot. It selected 59.94 Hz pacing on the 180 Hz display. Audio fidelity was not assessed.

The portable Python build wrapper was rerun end to end successfully, including revalidation/import, generation, configure, and relink.

## Final tooling review

Added protection against selecting an original IMG/CCD/SUB as the inspection report output, with a regression test. Final local suite: 13 tests passed. Allowed the tracked CMake preset through upstream's broad root JSON ignore rule. Documentation link targets and all JSON reports validated. Both runtime test processes exited with code 0; no test instance was left running.
# 2026-09-25 — FMV performance follow-up

Subsequent user feedback confirmed smooth video/gameplay but reported successive
voice clips. The [CD seek investigation](14-gameplay-voice-seek.md) found the
reader continuing through DISM02 when MUSIC1 was requested. The runtime patch
corrects explicit seek state; 21 recorded seek/read transitions now land on
their requested targets. Modern controls are confirmed as an eventual change
to movement/camera/aim semantics, beginning with third-person if useful.

The user reported unchanged slow video and garbled sound after the initial
experiments. Timing confirmed roughly 40–43 guest frames/s and audio buffer
starvation. A Time to Kill stream-poll hook with DMA destination exclusions,
plus offline compilation of the exact MOVIE.OVR, now measures 59.93/59.96
guest frames/s with no new audio underruns across 19- and 65-second intervals.
Seventeen local tests pass with the owned-disc integration tests enabled.

Added the root game manual, a reproducible movie-shard builder integrated
into the build wrapper, and a repeatable windowed FMV benchmark. Detailed
evidence, failed intermediate attempts, and remaining fidelity work are in
[the FMV pass](13-fmv-fidelity-pass.md). Full campaign/reference parity remains
unverified.


## 2026-09-26 — D01 Vanilla regression recording

User authorized autonomous progression through the prescribed job order. Added
`tools/local/vanilla_regression.py` and the D01 route/evidence document. Preserved
existing local runtime/submodule changes and tested only fresh isolated cards.
Captured and retained startup timing failures, corrected the test's initial
weapon-state assumption, and verified the bounded gameplay inputs visually.
The longer progression/save/audio/window acceptance remains outstanding; D01
is Needs playtest and dependent jobs have not been marked started or complete.
See `reports/d01-regression.json` for attempts and build identity. No rebuild or
runtime/gameplay modification occurred.


## 2026-09-26 — Vanilla sign-off, D02 profiles and D03 research

User playtest confirmation closes D01 while preserving individual instrumented
coverage limits. Implemented D02 persistent launch profiles and the terminal
selector; nine targeted tests plus an actual PTY settings session and two
fresh-card launcher replays passed. Vanilla/OpenGL and Modernized/software use
original gameplay; the preview message explicitly says modern input is absent.
Settings recovery preserves original bytes, reset is per profile, and runtime
TOML/player cards remain outside the profile writer.

Continued D03 with guarded read-only state snapshots and filtered write traces.
Four guard tests pass. Player/camera pointers, position and heading correlate
with captured inputs; write PCs distinguish ordinary integration, heading
smoothing and jump movement. Input/collision ownership and active-overlay hooks
remain unverified, so D03 stays In progress. No generated C, native runtime or
existing submodule patch was changed. See engineering notes 17/18 and the
D02/D03 reports for reproducible evidence and remaining boundaries.


## 2026-09-26 — D03 producer/overlay continuation

Recorded user confirmation of the terminal launcher and Modernized selection.
Resumed D03 with a fresh-card producer trace, preserved personal preferences,
and established exact LEVEL00 residency at 0x800CA968 (9668 matching bytes).
Added a conservative first-map overlay identity check to the read-only sampler.
Five guard tests and captured-RAM replay pass; the final guard itself has not
yet been exercised in another live run. Input/collision/callback ordering
research remains active; no mutation hook or modern control is advertised.

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


## 2026-09-26 — D04 complete

Implemented the Modernized PC action path, independent movement/look data,
profile schema 2 bindings and terminal/CLI rebinding, fixed menu navigation and
explicit capture/release. Added focus gating for original physical input in
both modes. No guest code or movement/camera state mutations. Runtime patch
stack reproduction and idempotence are verified; player/native SDL tests build,
42 Python tests pass (two owned-disc integration skips), input/SIO assertions
pass in both profiles, and final Modernized SDL navigation reaches gameplay.
The existing Vanilla regression route exits 0 with reviewed firing, jump,
inventory and movement captures. Original media and player cards remain untouched.
See [contracts and limitations](19-pc-action-input.md) and
[full evidence](reports/d04-input.json). D05/D06 remain unstarted.


## 2026-09-26 — D05/D06 Modernized control slice

Added guarded first-map camera-relative locomotion and independent mouse orbit,
retaining original collision/camera stages and Vanilla. Covered run, walk and
stop-animation paths; tightened death/animation ownership after screenshot review
caught a numerical false positive. Schema 3 preserves existing preferences while
adding original-camera choice and sensitivity/inversion. See [contracts and
acceptance gaps](20-modern-movement-camera.md), [D05](reports/d05-movement.json) and
[D06](reports/d06-camera.json). Final Modernized and Vanilla isolated routes pass
on one binary hash; 46 Python tests pass with two skips and both native harnesses
pass. Manual/status/handoff updated. Both jobs are Needs playtest; stopped at D06.


## 2026-09-26 — D05/D06 user acceptance

The user confirms working WASD movement and independent third-person mouse camera
and explicitly signs off both jobs as Done. Updated board/status/handoff/manual
and evidence while retaining specific coverage limits. D07 is ready but remains
Todo; the user will invoke it after clearing the conversation. No game process,
build, code or player-card changes were made for this documentation update.

## 2026-09-26 — D07 weapon aiming preview

Added guarded view-to-muzzle shot direction, world-query parallax/near-cover
handling, Software/OpenGL crosshair, schema-4 aiming choice and isolated weapon
observers/fixtures. No generated C edits or player-card tests. See
[the D07 note](21-modern-weapon-aiming.md) and [evidence](reports/d07-aiming.json)
for automated results, failed routes, weapon coverage and required playtests.
D07 remains Needs playtest with beam/hitscan and natural-weapon gaps; stopped
within D07 and retained D01–D06 user acceptance.

## 2026-09-26 — D07 user acceptance

Recorded explicit user acceptance of the bounded D07 iteration, including the
known body/gun alignment limitation. D07 marked Done; existing evidence gaps
retained. Added D07A (Todo) for view-aligned facing/weapon presentation before D08.
Documentation only; no game launch, settings mutation or new verification run.
