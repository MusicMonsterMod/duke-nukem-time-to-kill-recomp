# D17A/D17B acceptance and high-refresh regression baseline

2026-10-03. **D17A Accepted. D17B Accepted.** The user explicitly closed both
broad systemic jobs after natural play through a significant portion of level 1
at 120 FPS and 180 FPS. At 120 the experience was excellent; at 180 it remained
good and enjoyable. Mouse response, combat, opening, club and apartment no
longer made the renderer the focus of play. Residual defects are focused bugs,
not a reason to leave the parents open.

This is human gameplay acceptance, not a claim of perfect frame delivery,
measured input-to-photon latency, or complete campaign coverage. Older notes'
Needs playtest labels and unfinished-work statements describe their historical
iteration; this explicit acceptance supersedes those parent-job statuses.

## Quality strategy

| Presentation target | Engineering role |
| --- | --- |
| 120 FPS | Primary quality and regression target. Protect smooth pacing, immediate mouse feel, audio/FMV stability, visibility, geometry and textures, consistent demanding-scene controls and intended PS1 character. |
| 180 FPS+ | Excellent, responsive and fully playable high-refresh support. Treat remaining extreme-rate imperfections pragmatically without compromising 120. |
| 240 FPS+ | Robustness and compatibility. Preserve architecture support and frame-rate independence; later user testing on a 240 Hz display remains outstanding. |
| Unlimited | Stress/debug mode for exposing timing, starvation and scheduling defects, not the quality standard for every scene. |

This is not a 120 FPS technical ceiling or a change to the player's saved
setting. Simulation, movement, weapons, audio and movies must not accelerate
with presentation rate. Fix genuine systemic bugs exposed at higher rates,
but do not destabilize the accepted foundation to chase extreme-rate perfection.
Vanilla remains available and Modernized optional.

For a future fix: reproduce at 120 first, preserve a dated private state copy,
measure source timing, actual presented-image cadence, input/camera timing,
CPU/worker/GPU work and audio where relevant; compare 180 and use 240/Unlimited
for robustness. Verify the relevant opening/club/apartment/movie regressions.
Do not accept average FPS alone, change world buffering blindly, or call a
dummy audio sink an actual-device listening test. No broad refactor is part
of this closeout.

## What 30, 60 and 120 mean here

Three different clocks must not be conflated:

- Guest NTSC fields/VBlanks run at approximately 59.94 Hz at realtime speed.
- Original game compositions normally produce around 30 new game images/s,
  and fewer in demanding scenes. Simulation/actor updates retain original
  timing; a guest field is not automatically a new scene image.
- Host presentations can run at 60, 120, 180 or another selected rate. Above
  60, this implementation redraws intermediate camera/world/actor views using
  the game's renderer in workers, with more recent mouse rotation.

The original-style 60 setting usually repeats roughly 30 game images across
roughly 60 presentations. That part of the user's intuition is correct.
**120 is not a hidden 60 FPS mode or a doubled counter.** It requests a present
about every 8.33 ms and can deliver roughly 120 distinct intermediate rendered
views/s. It does not make game logic run at 120 updates/s, nor invent additional
animation detail absent from the snapshots. Repeated views can still occur
under load. A fully stationary image can also be visually identical despite
being freshly rendered.

The FPS overlay separates `FPS` (presentations), `Unique` (distinct image
submissions including redraws), `Game` (game compositions), `Guest` (fields)
and `RT` (realtime speed). `Unique` is not a pixel-difference or independent
simulation-update count. Use these counters and frame traces together.
Code inspection confirms the 60 option bypasses high-refresh replay and rates
above 60 enable it; the target is not divided by two. Prior distinct-image and
capture measurements are in notes 80 and 83-85. No new 120 benchmark is claimed
by this closeout; the latest 120 evidence is the user's
natural playtest.

## Accepted implementation

Preserve the following proven changes rather than restoring earlier shortcuts:

- Coherent render snapshots and transforms, camera/vertex provenance checks,
  near-plane handling, and live/replay color/depth isolation. Per-frame host
  depth reset removed major see-through failures. Exact endpoint checks and
  complete command/image comparisons provide bounded evidence, not universal
  visibility proof.
- Timestamped world interpolation with recent mouse rotation, fixed-rate
  presentation scheduling, bounded worker queues, adaptive readiness and grace
  for just-late redraws. Five-field game updates no longer incorrectly reset
  accumulated mouse look. Explicit ownership/epoch/state-load resets remain.
- Correct restored-code guard reconstruction and worker code-state invalidation;
  guarded compiled math acceleration; no unchecked byte-guard bypass. Stable
  worker startup timing and prepared scratch allocations reduce first-use
  replacement stalls.
- Ordered translucent and compatible opaque batching retain primitive order;
  mask/subtractive exceptions remain isolated. GPU profiling does not block on
  unfinished query results. Ordinary launches disable per-batch GPU timer
  diagnostics; explicit profiling remains available.
- Correct pinned SDL3 X11 event timestamp units/clock mapping, preserving queued
  mouse report timing. Configure-time repair is idempotent and fails closed
  on an unrecognized dependency revision.
- Stable FMV path and source timing, correct FPS categories, accepted idle-eye
  stability, and intentional PS1 movement character. No general geometry
  smoothing, speculative camera reprojection or new audio buffering was added
  during the last polish pass.

The 90 ms world history and original pose-clock correction remain unchanged.
The tested 130 ms buffer and stronger correction were rejected and are not in
the accepted implementation. Slot 12's residual movement problem is not being
hidden by claiming these experiments fixed it.

Engineering history and evidence:
[81](81-d17a-instability.md), [82](82-renderer-quality-pass.md),
[83](83-opening-responsiveness.md), [84](84-warmup-and-camera-stability.md),
[85](85-movement-polish-and-isolated-artifacts.md).
Earlier severe failures and superseded metrics remain historical, not current
acceptance criteria. Final previous-pass checks: 100 Python tests; bounded
opening/dancer real-mouse regression around 179-180 images/s with zero audio
underruns and no adjacent camera turns over one degree. Earlier final-source
GP0/RGBA oracles and native suites are recorded with their build scope in 84.
No renderer refactor or gameplay change is part of closeout. A separate clean-source build verifies publication completeness; it does not replace the accepted player binary or constitute another playtest.

## Residual reproduction ledger

The job board is authoritative. None of these jobs is started by closeout.

| Job | Reproduction and evidence |
| --- | --- |
| D17D | Current UI 9 club-exit lower wooden board; also visible at 60 Hz and in original/intermediate images. Suspected local order/intersection/clipping, exact primitive cause open. |
| D17E | Current UI 3 subway peripheral walls near the camera, corridor/stairs. User reports major improvement; bounded full-width routes did not reproduce the missing sections. |
| D17F | Current UI 6/11 apartment closet lower/edge artifacts; older cards-user6 UI 8 club closet and UI 11 apartment closet. Secret reveal now user-reported stable. |
| D17G | Current UI 12 club Shift+W / Shift+S translation jerk. Trace supports insufficient complete interpolation history in the 180 stress case; quantify at 120 before generalizing. |
| D17H | Current UI 2 switch, leave doorway while train moves, approach pig cops; current UI 10 turn/move/jump at train-platform ledge. Full train route remains to be measured. |
| D17I | Unlimited stale-camera episodes: older cards-user7 UI 5 early club / UI 10 later club, current UI 12 stress. Keep separate from normal 120 acceptance. |
| D17J | Bounded verification of older apartment light-switch/bed-secret, alley burnt-car platform, pavement, club balcony stairs/bar stools, warm-up reports and unclassified endpoint captures. Do not presume still broken. |
| D18C | Current UI 1 construction signs: turn, approach, fire while moving, jump ledge; UI 10 train ledge and UI 12 movement as stress comparisons. Rare slot-1 crackle unconfirmed by counter tests; starvation measured in some slot-10/12 runs. |

Current private reproduction generation is `cards-user8`, with UI N mapped to
zero-based file N-1. `polish-baseline/save-manifest.json` preserves original
hashes. Historical `cards-user7`: UI 1/2 severe slowdowns, 3 subway, 4 improved
opening, 5 early club, 8 minor initial slowdown, 10 later club. Historical
`cards-user6`: UI 8 club closet, 9 furniture, 10 good apartment, 11 lower closet.
`cards-user5`: UI 1 table/furniture, UI 6 closet. Do not overwrite these private
generations or infer location from a reused slot number.

No new bugs are filed for already repaired mouse reseeding, severe idle
breathing, major see-through walls, stable FMVs or FPS reporting. Preserve them
as regressions. Normal moving-camera PS1 wobble is intentional unless evidence
identifies a separate defect. Unmatched captures without generation labels
remain unclassified rather than being promoted into confirmed failures.

## Baseline identity and local preservation

Accepted executable SHA-256:
`1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
Codegen identity: `8bab543c`. No save-format change.
Local title upstream HEAD: `b2d21e3231d19958b7b04af065ce5397265b8559`.
Local framework HEAD: `08ec704a974b1f3a16335b4afeb340b9eff19926`.
These upstream commits alone do not contain the authored local implementation.

Local preservation directory:
`recomp/analysis/d17-high-refresh/accepted-2026-10-03/` contains the executable,
a source/tools/tests/patches/configuration archive, tracked-tree diffs and
`manifest.json`. Source archive SHA-256:
`8b1133c5857fd13181f8d731e49cf43213f65b2efe3c16d310e55d8fed7a8305`.
It excludes player states/cards and original media; required local assets and
media retain their existing locations. This is a local rollback snapshot, not
a claim that upstream Git alone can rebuild the accepted binary.

Original disc dump moved intact into
`game/Duke Nukem - Time to Kill [U] [SLUS-00583]/` at the user's request.
All five files retained their SHA-256 hashes. Every file under game except
README.md is ignored. The prepared runtime copy stays under recomp/disc so
existing launches are unaffected; the old import receipt is historical
provenance and may name the former source path.


## Source publication correction

During closeout the user explicitly revoked the old documentation-only policy.
`recomp/` is crucial implementation source and is now tracked in the root repo.
The root ignore rules, AGENTS.md, both agent skills, README and CI checker were
corrected. Title source is committed directly; pinned upstream framework/UI
submodules plus the complete tracked framework patch preserve dependency changes.
A clean pinned framework checkout plus the patch matched all 38 modified/added
framework files byte-for-byte. The previous incremental stack failed from the
pin and is archived, not applied. Original dependency Git metadata is preserved
locally; no authored working-tree changes were discarded in the conversion.

Source publication does not include original media, generated game code, builds,
saves/cards, analysis dumps, research or extracted retail assets. These exclusions
must not become a blanket exclusion of authored source again. The remote commit
must contain `recomp/src/ttk/frame_replay.cpp`, `modern_controls.cpp`, launcher,
build tools and complete patch, not merely a `recomp` gitlink.


Clean build verification additionally found three authored runtime regressions
missing from the framework's test registration. Their CMake registrations are
included in the complete patch; this is build/test wiring, not a gameplay change.
Root CI now applies the patch from pinned source and runs the source regression
suite without retail inputs. The source-publication policy explicitly supersedes
older historical statements that code stayed local.


Publication validation: the complete patch reproduces all 38 modified/new
framework files from the publicly reachable pinned commit (37 accepted source
files plus test registration). The 100-test no-media Python run passes with
2 disc-dependent skips; the three compiled restore/deadline regressions pass
against a clean patched dependency. Repository hygiene requires essential source
and excludes player/media/output paths. Patch context whitespace is preserved
explicitly rather than altered to satisfy a prose whitespace check.

A separate clean source checkout built successfully with the owned disc input,
fresh pinned dependencies and the complete patch. The build/import/codegen
workflow exited successfully, including the player executable and overlay
processing. This checks source reproducibility without replacing or launching
the accepted player executable. It is build evidence, not another gameplay
playtest; the user's 120/180 FPS acceptance remains the gameplay evidence.
