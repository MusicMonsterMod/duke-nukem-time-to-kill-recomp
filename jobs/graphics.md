# Time to Kill jobs - Graphics

Job descriptions and work logs for D13, D15, D16, D17, D40, D46, D52 and D53.
Status and dependencies are on [the job board](../MODERNIZATION_JOBS.md); the
dated session log is in the `log-YYYY-MM.md` files beside this one. Moved
verbatim from the board on 2026-11-10 (D58), one section per job in ID order.

## Background

### Focused D17 follow-ups: shared constraints

Current statuses are in the board above; accepted fixes remain regression
baselines for unfinished follow-ups. Primary quality/regression target is
**120 FPS**; also compare 180 FPS. 240 FPS+ is
robustness/compatibility, Unlimited stress/debug. No 120 FPS technical ceiling.
Preserve accepted mouse feel, FMVs/audio, idle stability, gameplay speed, FPS
reporting, culling improvements, PS1 character and optional Vanilla. Profile
before changing architecture. Do not trade away 120 FPS quality to perfect an
extreme rate. Fix a demonstrated systemic defect at its appropriate level.

Current reproduction copies: local `cards-user8`, UI slot N = file N-1.
Never write to original player saves/cards. Save numbers are not permanent
identifiers: preserve the dated snapshot/manifest and location in each report.
Older generations are explicitly named below. Evidence and baseline identity:
[86](documentation/86-d17-acceptance-and-regression-baseline.md).

## Jobs

### D13 — Higher internal resolution and display scaling

Explicit user priority: selectable high-resolution settings. Inspect and reuse applicable renderer capabilities, then expose tested internal-resolution choices, fullscreen/window modes and output scaling. Distinguish rendering more scene detail from enlarging a low-resolution image. Preserve original-resolution presentation.

**Acceptance:** comparison captures demonstrate the effect, settings survive restart, UI/FMV sizing remains correct and tested performance is recorded. Verify the active build actually supports each offered option.

**Done (2026-09-30, user-accepted):** "correct correct correct, D13 is good.
I'd say let's approve it." Per-profile internal scale 1-4x (OpenGL; the
software renderer stays 1x), windowed/borderless/exclusive display, window
width and linear/nearest output filter, via `run.py --settings` choice R or
flags. Vanilla defaults to 1x, Modernized to 4x (user choice). Alt+Enter and
exclusive fullscreen were fixed after the first playtest. Evidence and limits:
[D13 note](documentation/67-d13-resolution-display.md).

### D15 — Optional geometry and texture precision

**Accepted (user playtest, 2026-10-04).** Geometry stability, slot 4 floor seams,
slot 1 idle polish, slot 5 black/missing left surfaces, slot 6 closet/furniture
and the previous slot 8 opacity case are accepted. [Implementation, regression
causes, tests and limits](documentation/96-d15-accepted-precision.md). The newly
replaced slot 8 subway bands are D17P and do not block D15.

Investigate renderer support for reducing vertex jitter and perspective distortion, preserving authentic behavior as an option. Compare effects on animated geometry, effects and seams before enabling enhancements by default.

**Acceptance:** captures show actual improvements and any remaining artifacts; toggles restore original presentation; collision, visibility and game timing remain unaffected. Unsupported renderer features are documented rather than simulated by ineffective settings.

### D16 - Texture manipulation and replacement foundation

**Status: Done (bounded foundation accepted 2026-10-27). Dependency: D13 (Done).**
Real 40x40/160x160 Photon-to-game panel replacement is implemented. Exact source
and palette gating, original fallback and private 60/120 save/load checks pass.
[Note 278](documentation/278-d16-texture-foundation.md) records the artifact,
source provenance, tests and explicit limits. The user accepted the visible club
example and explicitly deferred broader scripting/modding/texture authoring.
Commit and push authorized. This acceptance does not claim whole-game coverage.

Establish control over the game's texture path so original textures can be
identified, inspected, manipulated and replaced deliberately. HD textures and
user-created replacements are possible future uses; this job builds the technical
foundation and proves it with a controlled small-scale example. It does not build
a modding pipeline or produce a full texture pack.

**Scope:**

1. Map the texture lifecycle from disc/source assets through decoding, palettes,
   uploads, VRAM/texture-page reuse and renderer sampling. Identify where a
   replacement can be applied safely. Catalogue world, actor/weapon, sprite/UI
   and dynamic texture paths, with explicit coverage and unsupported cases;
   one successful world texture must not imply universal support.
2. Establish reliable texture identity and inspection/export for a small sample.
   Account for level/overlay changes, shared pages, subregions and palette variants;
   a transient VRAM address alone must not accidentally replace unrelated art.
3. Implement a bounded replacement path supporting deliberate texture edits and
   a higher-resolution sample while preserving intended UV mapping and native
   behavior. Determine the required handling of palettes, animation, transparency,
   semi-transparency, wrapping and texture updates from evidence. Keep original
   disc assets intact and provide a deterministic original-texture fallback.
4. Use Photon for the controlled authoring example: retain an editable source,
   export the runtime image, load it in game and revise it through the same path.
   Keep the runtime image interface independent of Photon. Read the accepted art
   guide before material/art work and validate appearance in game.
5. Document the integration points, supported image inputs, identity rules,
   limitations and reproducible local workflow so later texture work can build on
   this foundation without repeating the investigation. Texture filtering,
   internal render resolution and replacement resolution remain separate concerns.

**Acceptance:**

- A small, named sample completes the inspect/export -> Photon edit -> runtime
  replacement -> in-game verification loop, including a higher-resolution variant.
  Choose sample assets after researching the real texture paths.
- Evidence shows only the intended textures change, with exact source/palette
  gating, original fallback and save/load restoration checks. Per the user
  acceptance on 2026-10-27, natural transitions and broader animated/cutout
  specimens are deferred coverage for future expansion, not blockers to this
  bounded foundation. Record those untested paths explicitly.
- Disabling replacements, or encountering missing/invalid replacement data, restores
  original textures predictably. Vanilla retains original behavior. Verify relevant
  renderer paths and bounded memory/performance cost, with limitations stated.
- The result establishes reusable texture handling rather than a hard-coded visual
  swap for one scene. Document what is needed to extend coverage to other classes.
- Record sample provenance. Extracted retail textures and derived local authoring
  references remain local under repository asset rules.

**Outside this job:** user-facing mod tools, pack manifests/installers, mod discovery,
priority/conflict systems, distribution, a full HD pack, model replacement and
reviving cancelled D16A HRP work. Further asset production and modding infrastructure
require separately selected work. Filtering options can be evaluated during the
investigation, but are not the purpose or proof of this foundation.

### D16A — HRP assets and first-person weapon research (later)

**Cancelled (2026-09-30, user decision).** The first-person weapons and other
presentation work built from the original TTK meshes and assets is the chosen
direction; HRP import or adaptation is unnecessary. The only follow-up kept is
replacing the Duke3D-sourced switcher icons with original TTK art, tracked as
**D08A3**. The text below is kept for history.

Investigate the [Duke Nukem 3D High Resolution Pack](https://hrp.duke4.net/)
as a future source/reference for weapons, interface, HUD/overlays and optional
3D model replacement. Prioritize first-person weapon presentation for D12.
This is later research, not authorization to import assets during controls work.
Inventory candidate weapon models, skins, animations and definitions; map them to
Time to Kill weapons and identify mismatches, missing states and conversion work.
Assess renderer integration, muzzle alignment, clipping, widescreen placement,
performance and disc-asset fallback. Review per-asset provenance, attribution and
applicable art terms before proposing redistribution or adaptation.

**Acceptance:** a sourced compatibility/rights matrix and a bounded prototype
proposal distinguish reusable assets, visual references and new work. Record
unresolved permission and technical questions; no assumption that an EDuke32 pack
loads directly into this recomp. Do not import or adapt HRP scuba gear as a TTK
item (D08N cancelled). Implementation requires a later selected job.
Keep additional user-supplied research topics as separate scoped backlog entries.
See [external research register](documentation/23-external-research.md).

### D17 — Presentation smoothness without faster simulation

**Done (user-accepted 2026-10-02):** on the 180 Hz monitor at Match Display
the user saw about 180 FPS with very good stability, correct FMVs and an
accurate fps readout, and accepted the job. Remaining refinements are separate
jobs: geometry/texture instability (D17A) and mouse responsiveness and input
latency (D17B; the acceptance's input-latency measurement moves there).

**Needs playtest (2026-10-02, second pass):** frame-rate option with Match
Display, even display-rate presents and in-between frames redrawn by TTK's own
renderer in worker processes (camera, Duke and object-loop transforms
interpolated; guest timing untouched). After the first playtest: emulation-thread
costs cut (forensic rings off in player sessions, GL binding cache, cheaper
presentation tick and SPU query), one in-between per display refresh (120 Hz: 3
per game frame) reaching nearly every present in all ten test slots, doorway
rooms from the game's own portal walk, FMV and quick-kick fixes. Design,
evidence and limits: [D17 notes](documentation/80-d17-high-refresh-audit.md)
(section 13 for the second pass, section 14 for the third: no repeated images,
copy-on-write redraw saves, exact rotation blends, near-clip seams, fps readout).

**Scope expanded 2026-09-30 (user brief):** arbitrary / high refresh rate
rendering. Full brief: [D17 high refresh brief](documentation/68-d17-high-refresh-brief.md).
Render frequency must be separated from gameplay timing: rendering at 180 FPS
must not run game logic 3x faster, and 30 FPS must not slow it down. Frame-rate
option: **Match Display** (preferred default; the refresh rate of the display
actually presenting the window, handling moves between monitors, never a
hardcoded 180 Hz), **30 / 60 / 120 / 144 / 165 / 180 / 240 FPS**, and
**Unlimited**. Use interpolation (player/camera, actors, projectiles, moving
sectors) where the data permits and it cannot change gameplay; mouse look must
not feel quantized to a lower rate; frame delivery must be evenly paced, not just
a good average. Audit every system listed in the brief rather than assuming
frame independence. Current 60 FPS is the known-good baseline and must not be
destabilized; 30 FPS stays as a compatibility and regression mode.

**First step when selected:** audit and a short technical plan (what sets
simulation frequency, render frequency, remaining frame-dependent systems,
current display refresh detection, where interpolation or delta-time
conversion is needed, and a safe staged path) before any timing-model change.
Until D19 menus exist, the option follows the D13 pattern (profile setting,
launcher/runtime flag). Vanilla keeps its original timing.

**Acceptance (expanded):** a repeatable numeric comparison at 30 / 60 / 120 /
180 / 240 FPS (known-distance walk/run, jump, fall, jetpack, camera rotation,
auto and semi-auto fire, projectile travel, enemy movement, doors/lifts, animations,
timers/scripts) shows equivalent gameplay; measured frame times match the target
cadence; 60 FPS behaviour is unchanged; plus the original criteria below.

Measure unique rendered frames, guest timing, host presentation and input latency independently. Investigate interpolation only where the game's data permits it; reset interpolation across teleports, room loads and camera cuts.

**Acceptance:** measured pacing improves without speeding up movement, scripts, audio or cutscenes; discontinuities do not smear or blend incorrectly. Report achieved unique-frame cadence honestly: a 60 Hz guest clock is not proof of 60 unique game images per second.

### D17A - High-refresh texture/geometry instability: popping, flicker, black areas

**Accepted by the user, 2026-10-03**, after natural first-level play at 120 and
180 FPS. Broad systemic work is closed. Remaining focused issues are tracked
in D17D-J and D18C; they do not reopen this parent. The following original
brief and criteria are historical. See [acceptance and baseline](documentation/86-d17-acceptance-and-regression-baseline.md).

User report (2026-10-02, third playthrough at 180 Hz, after D17 was accepted):
parts of the scene vanish for a moment and show black underneath. Places:
the apartment right after using the light switch (looking right and walking
toward where the bed moves from, black patches on the wall to the left); the
wardrobe ("closet") still flickers; on the highest alley platform looking
toward the burnt-out car, strong flicker of the brick wall and the platform for
a couple of seconds before it settles; occasional popping on the street
pavement; the subway, where vanished parts show as black areas or squares near
the edges of the image; the strip club stairs to the balcony, and bar stools
that disappear and return. Also a slight overall texture "vibration" or
shimmer. The user notes it often appears around polygon edges and vertices
(descriptive, not a diagnosis).

Known from D17 (documentation/80 sections 13-14), to verify rather than assume:
- Redraws match the real image exactly at alpha 0, and the D17 tools
  (`popsweep.py`, `seqcheck.sh`, `shift2.py`, scripted input) find mostly
  hairline seams and texel sparkle, not whole surfaces vanishing. The reported
  black areas have not been reproduced yet: reproduce first, at the reported
  places, at 60 Hz as well as above (is it high-refresh specific?).
- Native-pixel vertex snapping and affine textures are more visible with six
  distinct images per game frame; geometry/texture correction (D15) cannot
  resolve TTK's CPU-built packets as implemented.
- Near clip (D11B) takes over polygons beside the eye; its pieces meet the
  original polygons at seams, and its packet budget can refuse polygons.
- Black areas near the image edges may be widescreen margins (D14) where the
  game culls to its 4:3 view, or the in-between camera seeing past the real
  camera's cull; the strong flicker that settles after a couple of seconds may
  be a streaming or culling transition.

**Acceptance:** each reported place reproduced (or shown not to reproduce) with
a recorded test; causes identified and fixed or explained with evidence;
no surfaces vanish to black at 60 Hz or above in those places; any remaining
PS1-inherent artifact is documented, with an option (D15) where one is
feasible. Vanilla unchanged.

### D17B - Mouse responsiveness and input latency at high refresh rates

**Accepted by the user, 2026-10-03**, after natural first-level play at 120 and
180 FPS. Broad systemic work is closed. Remaining focused issues are tracked
in D17D-J and D18C; they do not reopen this parent. The following original
brief and criteria are historical. See [acceptance and baseline](documentation/86-d17-acceptance-and-regression-baseline.md).

User report (2026-10-02): frame rate is very stable, but turning with the mouse
(entering the strip club and turning to shoot, and later just walking and
turning) feels slightly delayed or floaty. Mouse input must feel immediate:
no perceptible latency or smoothing between the mouse and the camera.

To measure, not assume: mouse event to camera change to photons, at 60 Hz
(D17 off) and at 120/180 Hz, in first and third person. Candidates:
- The camera turns once per game update (30 per second); in-between images
  interpolate between two game frames, so the newest mouse motion reaches the
  screen only with the next game frame and arrives smoothed.
- Replay presentation draws in-betweens ahead into a cache; a prepared image
  can be up to one present old when shown.
- Swap interval, driver frame queueing and exclusive fullscreen behaviour.
- Input sampling once per field and any smoothing in the modern controls.
The runtime has a latency ring (`latency_ring_mark`) and the plugin a present
trace; extend them to a mouse-to-present measurement. A likely direction, if
the camera update is the cause: apply the newest mouse yaw and pitch to every
presented image (late camera update at present time) while gameplay keeps its
original 30 Hz aim.

**Acceptance:** measured mouse-to-present latency at high refresh is no worse
than at 60 Hz and as low as the pipeline allows; the user confirms aiming feels
immediate in first and third person; gameplay timing unchanged.

### D17C - View bob: disable experiment, then a stable modern camera

**Done (user-accepted 2026-10-03).** The user confirms the camera is much more stable and is happy with the current look. This closes the remaining camera-look playtest; the implemented camera and bob options are unchanged.

User request (2026-10-02): TTK's walking view bob looks wrong in the recomp,
especially at high refresh: rather than Duke's head moving, the floor and walls
seem to breathe, swell and melt (with the PS1 geometry wobble). First step: a
clean test with no view bob at all, to judge whether the world feels more
stable, whether the breathing goes away, whether it interacts with the D17A
instability, and whether any rendering work is tied to the bob. Longer term the
recomp should present a stable modern 3D view where appropriate.

**Acceptance (experiment):** a switch that removes the first-person view bob
entirely, verified by measurement; the user playtests it and decides the next
step (keep off, make it an option, or redesign).

### D17D - Club-exit furniture: lower wooden board popping

**Accepted (2026-10-03).** User confirms the UI 9 prop is fixed, along with
the UI 11 closet and the closet inside the strip club. This accepts the
source-plane depth correction and bounded static-prop contact tolerance.
The related closet report D17F is also closed on this explicit playtest.
New blood/tabletop-prop reports are separate D17K/L follow-ups.
[Cause, implementation and evidence](documentation/87-d17d-contact-depth.md).

Original report: UI 9 (`cards-user8`, file 08); inspect the lower wooden
board of furniture near the strip-club exit, stationary and during slow turns.
Also reported in `cards-user6` UI 9 and `cards-user5` UI 1. Triangular fragments
occur in both original and intermediate captures, and at 60 Hz. No clipping
budget, packet or copy exhaustion in the inspected sample. The investigation
confirmed near-coplanar placement plus snapped-depth surface intersections;
see the current evidence above.

**Acceptance:** identify the offending primitives and cause, fix this object
without coordinate hacks or blanket culling changes; capture before/after at
60/120/180, then preserve the opening/club regression baseline. No speculative
mesh change or general renderer rewrite. User confirms the lower board stable.

### D17E - Subway peripheral wall visibility near the camera

**Accepted (user playtest, recorded 2026-10-04).** Current UI 3 (`cards-user8`, file 02; earlier `cards-user7` UI 3).
Walk along the corridor close to either wall, turn near view edges, and walk
up/down stairs. Original report: blank/missing peripheral wall sections,
not conventional tearing. User now reports substantial improvement; bounded
60/180 full-width routes did not reproduce the missing section.

**Acceptance:** reproduce on the accepted build at 120 first, or document
repeatable non-reproduction and seek closure; distinguish widescreen margin,
near clipping and visibility state using full-width compositor captures.
Verify 60/120/180 and wall approach/retreat without regressing fixed walls.
No claimed fix solely from a canonical 512-pixel image that omits the margins.

**Candidate/evidence:** Full-width wall approach/retreat, edge turns and stair routes at 60/120/180 did not reproduce the old missing-wall report. User repeatedly walked the subway corridor and watched peripheral edges without reproducing the old gaps; the result is accepted. No E-specific visibility workaround shipped. Future similar reports get separate targeted jobs. See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17F - Apartment/club closet edge and lower-geometry artifacts

**Accepted (2026-10-03, user playtest of the D17D implementation).** The user
explicitly confirms the UI 11 closet and the closet inside the strip club
are fixed. Preserve these accepted surfaces. Tabletop props seen from UI 11
are a distinct new report, D17L; do not reopen the closet job for them.

Historical reproduction: UI 6 and 11 (`cards-user8`, files 05/10): apartment closet,
lower geometry/textures and intersections. Current apartment/secret reveal
is user-reported dramatically improved, stable and pleasant. Preserve it.
Older `cards-user6` UI 8: club closet disappearing walls; UI 11: apartment
lower closet popping; UI 10: good apartment comparison. `cards-user5` UI 6
is an earlier closet reproduction. These old snapshots are not current slot 8.

**Acceptance:** separate currently reproducible minor edge/polygon defects
from fixed see-through doors/walls; classify clipping/order/depth versus actual
texture changes; bounded fix with closet and secret-reveal regression at 120
and 180. Do not smooth away normal moving-camera PS1 character.

### D17G - Strip-club translation jerk during Shift+W / Shift+S

**Done (2026-10-22), no action needed.** The user retested the strip-club area on the current build and could not reproduce the issue: "The graphics and stability is perfect in this area now." They explicitly requested completion. This supersedes the historical Todo and reproduction notes below. No specific adjacent fix is established as the cause of resolution; frame rate and camera were not reported. No implementation change, rebuild or automated test was needed.

**Historical report:**

**Todo. Primary remaining movement polish.** Current UI 12 (`cards-user8`,
file 11): repeatedly Shift+W, Shift+S. User describes a minor polish issue;
club mouse rotation and general playability are accepted. Compare current
UI 4 opening, UI 5 dancers, UI 8 street, apartment/alley and a fresh entry.

Trace evidence at 180: regular presents can coincide with exhausted world
interpolation history. Roughly 67 ms pose intervals and 38 ms composition
completion delay consume the 90 ms history plus advance-preparation margin.
Alpha 1 alone is not proof of a hitch. Higher world delay (130 ms) and stronger
clock correction were tested and rejected; neither is shipped. No worker
replacement/camera-cut diagnosis was established for this case.

**Acceptance:** reproduce and measure at 120 before assuming the 180 stress
result applies; correlate translation holds, input timestamps, complete pose
availability, CPU/worker/GPU timing and audio. Improve the bounded route without
adding unreviewed movement latency or speculative incoherent geometry. Preserve
late mouse response, original gameplay timing, 180 support and PS1 identity.

### D17H - Train-control and platform/ledge movement stutters

**Done (2026-10-22), no action needed.** After receiving the train-switch and platform/ledge test route, the user could not reproduce the issue and reported: "play is totally smooth". They explicitly requested completion with no action needed. This supersedes the historical Todo and reproduction notes below. Frame rate was not reported; no specific resolving change is established. No implementation change, rebuild or automated test was needed.

**Historical report:**

**Todo.** Current UI 2 (`cards-user8`, file 01): activate train switch, leave
the door while the train moves, approach the pig cops at the far end. Current
UI 10 (file 09): turn around, walk forward, jump onto the ledge and move there.
UI 10 now means the train-platform area, not the previous late club state.
User reports subtle stutters, not an unplayable scene. The full moving-train
sequence has not been established by the short scripted interaction sample.

Slot 10's scripted jump/fire stress can reduce source throughput and starve
output audio despite high presentation FPS. Coordinate with D18C for audio;
keep this job focused on frame times and movement. Preserve original slowdown
UI 1/2 from `cards-user7` as historical stress comparisons, not proof of a
current severe regression.

**Acceptance:** perform the actual switch/door/train/pig-cop and ledge routes
at 120; capture spikes and source workload, then verify any local fix at 120
and 180 with unchanged train/AI/gameplay timing and opening/club baseline.

### D17I - Unlimited stale-camera scheduling episodes

**Done (2026-10-22), no action necessary.** After receiving the manual Unlimited camera test instructions, the user explicitly requested: "mark this as complete and no action necessary". Closed on user acceptance; no detailed test observations or settings were supplied. This supersedes the historical Todo below, without claiming a measured timing improvement or a specific resolving change. No implementation change, rebuild or automated test was needed.

**Historical report:**

**Todo. Lower priority stress/debug.** Use `cards-user7` UI 5 early dancers,
UI 10 later club, and current `cards-user8` UI 12. Earlier Unlimited capture:
about 161 distinct images/s, 114 ms stale camera sample and catch-up turns,
while emulation/audio could remain realtime. This is not a requirement to
produce unlimited perfect images, nor evidence that 120 is unacceptable.

**Acceptance:** investigate adaptive cadence/deadline pressure and bounded
queues with frame-age traces, improve correctness/recovery under overload,
retain frame-rate independence at 240+, and prove no 120/180 regression.
Do not impose a 120 ceiling or retune normal play solely for this stress mode.

### D17J - Verify older isolated visual reports against accepted baseline

**Done (2026-10-22), no action necessary.** After receiving the location-by-location visual checklist, the user confirmed "these environments are all stable now" and explicitly requested completion. This supersedes the historical Todo below. Frame rate and camera were not reported; no specific resolving change is established. No implementation change, rebuild or automated test was needed.

**Historical report:**

**Todo. Bounded reproduction triage only.** Preserve reports without reliable
current slot identity: apartment after light switch/bed-secret reveal (black
wall patches); highest alley platform toward burnt-out car (brick/platform
flicker); street pavement/edge/intersection sparkle; club stairs to balcony
and bar stools disappearing. Historical source: note 81 / original D17A brief.
The apartment secret reveal is now user-reported stable: do not reopen it as
a confirmed current failure without reproduction.

Retain older `cards-user7` UI 8 initially slow area and UI 5 early/UI 10 late
club comparisons for first-visit versus revisit observations. No minute-long
shader-cache explanation was established. First-use scratch reforks and the
five-field camera reset were fixed, so do not treat those causes as still open.
Three earlier unmatched moving endpoint captures in note 82 lack generation
labels and are unclassified, not proof of a current visible defect.

**Acceptance:** bounded 120-first capture routes classify each as reproduced,
not reproduced, fixed/user-confirmed, or characteristic PS1 motion. Record
current state identity and primitive provenance for any reproduced defect,
then create a single scoped fix job or assign D17D-F; no broad implementation
under this triage job. Preserve historical evidence rather than guessing slots.

### D17K - Coplanar ground blood in save slot 12

**Accepted (user playtest, recorded 2026-10-04).** User report on 2026-10-03 after
accepting D17D: load UI save slot 12 (file 11) and look at the blood on the
ground directly in front of Duke. The user describes it as coplanar blood.
The initial report did not establish the cause; the candidate addresses a
traced mixed-path ordering defect.

Before testing, verify the current save identity and make a dated private copy;
do not assume its bytes still match the earlier `cards-user8` snapshot. Use
background/offscreen tests and private cards/profiles. Inspect the blood
primitive/decal path, transparency, depth and ordering relative to the floor.
Do not extend the static-prop contact bias blindly to blood or all sprites.

**Acceptance:** identify the offending primitives and establish whether this
is renderer depth/order handling or genuinely ambiguous coplanar placement;
implement a bounded fix if justified, or document an unavoidable placement
limit. Verify 60/120/180, transparent blending and nearby geometry while
preserving the accepted UI 9 prop, UI 11 closet and strip-club closet. User
confirms the result. D17D/F remain accepted.

**Candidate/evidence:** The candidate restores the source floor polygon ordering key so enhanced floor pieces do not paint over native blood. User confirms stable ground splatters, clearly visible varied patterns and excellent appearance. Preserve the current behavior. The original blood state is preserved privately; current UI 12 now holds the separate D17N wall case. See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17L - Tabletop props cut off on approach/retreat in save slot 11

**Done - user-accepted 2026-10-04, implementation `1e6fb0a`.** Runtime capture identifies a later native table-top quad overwriting the depth-tested cup. The new bounded correction and 60/120/180 evidence are in [note 95](documentation/95-d17l-tabletop-depth.md). The earlier unresolved candidate and retest below are historical. User report on 2026-10-03: load UI
save slot 11 (file 10), look at the props sitting on the table directly ahead,
and walk forwards and backwards. The props get cut off and reappear as Duke
moves closer. This concerns the objects on the table, not the now-accepted
closet in the same save. The earlier pre-repair retest showed the isolated cup disappearing/flickering, largely unchanged; other cups and the bowl on the bar are stable.

Verify the current save identity and use a dated private copy in background
tests. Trace the affected props through approach and retreat, including
visibility/fade, near clipping, host takeover and depth/order transitions.
Distinguish an entire object disappearing from individual polygons being cut.

**Acceptance:** reproduce and identify the responsible path, make a bounded
fix without coordinate hacks or blanket culling changes, and capture the
approach/retreat at 60/120/180. Preserve legitimate occlusion, the accepted
UI 9 prop and both accepted closets, weapons and original PS1 movement
character. User confirms stable tabletop props. D17D/F remain accepted.

**Historical candidate/evidence (superseded by note 95):** The candidate gives compact static props consistent depth beyond the near radius and groups opaque faces to control draw cost. The user does not accept this candidate as resolving the isolated cup. Read-only mesh inspection establishes polygonal table/cup geometry and shared cup meshes across table/bar instances, but does not yet establish the residual flicker cause. Trace the specific failing instance and supporting surface across a bad frame before changing behavior. See [follow-up inspection](documentation/89-d17-playtest-followup.md). See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17M - Shotgun ammo visible through ladder platform (current slot 10)

**Done (2026-10-22).** User: "lock it in and close it out, fix fully accepted."
Implementation and documentation commit/push explicitly authorized. The updated
slot-10 upper-ladder report is separate D17M1 and does not reopen accepted ammo
occlusion. [Note 200](documentation/200-d17m-platform-ammo.md).

Historical candidate evidence follows.

**Needs playtest (2026-10-22).** Reproduced on the current private slot-10 copy.
The candidate adds bounded thin-platform depth coverage and compact pickup
depth for Corrected textures, preserving Original textures and Vanilla.
Build/native renderer tests and offscreen checks are recorded in
[note 200](documentation/200-d17m-platform-ammo.md). User ladder ascent/descent
and unobstructed pickup acceptance remain. No commit or push.

Historical intake follows; its slot number is not the current reproduction.

**Todo - separate backlog bug (2026-10-04).** Load UI save slot 9 (file 08)
and begin climbing the ladder. The shotgun shell/ammo pickup is visible
through the platform it sits on; the platform should occlude it from below.
This is a new report, not the accepted old UI 9 shelf/prop case.

**Investigation:** verify save identity, use a private copy, and trace pickup
and platform primitives. Compare culling, depth/occlusion, sprite handling,
clipping and render order; the symptom does not establish the cause.

**Acceptance:** legitimate platform occlusion throughout ladder ascent and
descent, with the pickup visible when unobstructed. Check nearby geometry,
60/120/180 and Vanilla/Modernized; preserve accepted D17C/D/E/F/K behavior.
No blanket sprite-depth or visibility bypass. User confirms the result.
See [save identity and regression baseline](documentation/89-d17-playtest-followup.md).

### D17M1 - Upper ladder visible through platform (updated UI slot 10)

**Done (user-accepted 2026-10-22).** Fix accepted and closeout authorized; [note 201](documentation/201-d17m1-ladder-platform.md). User reports the upper ladder showing through the
platform when looking slightly upward; small movements and pitch changes
make it easy to see. Intake uses a private copy of the updated UI slot 10
(file 09), SHA-256
`1b709040b1614b4abd6418021cf7bcea99801bf121a611154ff4ee004ce2c9e5`.
This is a separate ladder/platform pair from D17M's accepted ammo occlusion.

**Quick research:** offscreen first-person inspection at 120 with the accepted
D17M executable; local captures in
`recomp/analysis/d17m-platform/shots/ladder-*.png`. Code inspection shows
D17M extends compact pickup depth only to 320 local units, ordinary compact
props remain limited to 256, and thin-platform coverage is gated. A ladder
could remain on the native ordering path; this is a plausible cause, not a
traced diagnosis. A bounded renderer correction appears feasible. No ladder
fix, code changes, rebuild or broad investigation during this intake.

**Next investigation:** authenticate ladder and intervening platform packets,
then compare native ordering, host depth participation and geometric overlap.
Distinguish truly hidden ladder portions from rails physically in front of or
extending above the platform. Reproduce by loading this updated state, looking
up/down and shifting slightly left/right beneath the ladder platform.

**Acceptance:** hidden upper ladder portions occluded through nearby movement
and ladder ascent/descent; legitimately exposed rails/rungs remain visible.
Check both camera views, 60/120/180, nearby geometry and accepted D17M ammo
occlusion. Preserve Original texture and Vanilla behavior, climb controls and
performance. Use private copies only. User confirmed the fix and authorized closeout on 2026-10-22.

### D17N - Diagonal wall artifacts during movement in save slot 12

**Todo - separate backlog bug (2026-10-04).** Load UI save slot 12 (file 11)
and repeatedly walk forwards/backwards. Watch the wall directly ahead:
jagged diagonal lines appear during movement and the surface looks as if it
vibrates. Other nearby walls show similar behavior. This new state is not
the accepted D17K blood reproduction.

**Investigation:** trace affected source faces and output primitives on a
private save copy. Distinguish polygon/triangle boundaries, clipping,
interpolation, precision, native PS1 geometry behavior and other render-path
causes. Do not assign a technical cause from the visual description alone.

**Acceptance:** substantially steadier wall surfaces on repeated W/S routes
at 60/120/180, preserving the deliberately retained PS1 visual character,
textures, legitimate occlusion and accepted D17C/D/E/F/K behavior. Compare
Vanilla/Modernized and nearby walls; user confirms the improvement.
See [save identity and regression baseline](documentation/89-d17-playtest-followup.md).

**Accepted (user playtest, 2026-10-04).** After substantial play the user
reported a major improvement to the zigzag/vibrating texture effect, most
obviously in UI slot 12 and across the game: "The result is excellent." (The
user's message labelled this job D17P; it is D17N. D17P remains the earlier
accepted draw-distance job.) Accepted executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Do not extend D17N; remaining subtle flicker is D17Q.

Candidate record: Cause: the original world renderer sends
every polygon within `0x2000` units to a screen-space subdivision
(`0x80012960`/`0x8001205c`) whose midpoints are integer screen averages with
rounded UVs and no projection data, so D15's perspective-correct path skipped
them and each piece was drawn affine; the bends follow piece diagonals and move
as the eye moves. This is general, not slot 12 specific. With Corrected texture
precision, polygons whose corners all have exact projections and fit the GPU
limit are now drawn whole (new hooks `0x800114EC`/`0x8001160C`, guarded by the
existing renderer digest). Offscreen evidence: straight panel borders across
120 Hz walking sequences; 12-slot sweep with exact-projection share 0.29-0.43
rising to 0.73-1.00, lower packet use, unchanged game rate, zero budget hits or
replay misses; Original textures and Vanilla unchanged. Tests pass. Candidate
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`. User to
confirm on W/S routes in UI 12 and elsewhere. See
[cause, implementation and evidence](documentation/98-d17n-world-subdivision.md).

### D17O - Unstable sky appearance when looking up (slot 10)

**Done, user-accepted (2026-10-04).** Autonomous investigation verified that the
original sky uses camera-relative meshes. High-refresh object interpolation
incorrectly recorded their matrices under the last world object, replacing the
current eye translation with an older one. A guarded exemption for the three
resident sky calls now retains the original camera-relative draw path and
animated layers. Sampled displacement fell from up to 1075.397 units to zero.
Private ground/flight captures cover 60/120/180 and multiple yaw/pitch views;
the user explicitly accepted the result: "amazing work!!! accepted. well done."
Preserve implementation `25734d1` and its recorded coverage limits. See [architecture, evidence and limits](documentation/92-d17o-sky-intake.md).
The report concerns UI slot 10 (file 09) and is not jetpack-specific.

**Scope:** reproduce the look/feel and establish how TTK draws and animates
its sky from owned game code/assets and authenticated active overlays. Trace
camera-relative/world coordinates, primitives, UV/texture motion, clipping,
ordering and update cadence. Compare original guest frames and presented
high-refresh frames before attributing the effect to a skybox, interpolation,
precision or camera motion. Implement a bounded stabilization if evidence
supports it, retaining intended art/parallax/animation and Vanilla behavior.

**Reproduction:** preserve the current UI 10 identity, then use writable private
cards/profiles for stationary look-up, slow yaw/pitch, walking and ground/flight
comparisons. Start at 60/120, then 180; inspect the full presented sky/horizon,
first/third person and another sky view if available. Never use the player's
cards for tests or infer identity from an older slot-10 fixture.

**Acceptance:** documented original sky architecture and verified cause;
before/after evidence of steadier sky motion without broken horizon seams,
world occlusion or lost intended animation; bounded tests/regressions covering
accepted D17C/D/E/F/K and D08Q2/Q3 behavior. Record timing cost and remaining
coverage limits. User confirms the appearance; otherwise Needs playtest.
Heavily document source/asset findings, verified addresses, reproduction data,
rejected hypotheses, implementation rationale and measured versus observed
results. Update status/handoff/manual as appropriate; commit and push source
and documentation, excluding all retail assets and local captures/saves.

[Detailed intake, private save identity and investigation plan](documentation/92-d17o-sky-intake.md).

### D17P - Distant horizontal black bands in the new subway slot 8

**Done (user-accepted 2026-10-04):** "i fully accept this fix." Implemented after accepted baseline `8cfd16f`. Separate follow-up
after D15 acceptance and its authorized commit/push. Do not reopen D15 or
confuse the replacement slot with the accepted old slot 8 door opacity case.

**Primary reproduction:** load the new UI save slot 8 in the subway corridor.
Walk forwards while watching distant geometry/textures. Multiple horizontal
black lines/bands appear farther down the corridor and seem to follow the
distant scene, as if sections disappear along horizontal boundaries.

**Investigation:** "possible culling" is a visual report, not an established
cause. Compare visibility/culling, clipping, polygon gaps, depth behavior,
geometry correction, texture rendering, precision and other rendering paths.
Use Original/Corrected geometry and textures independently, primary 120 Hz,
and lower-rate comparisons where useful. Trace the implicated primitives;
review prior fixes and R01 concepts without transplanting unrelated code.

**Acceptance:** eliminate the distant bands during approach and nearby view
changes, explain the responsible rendering path with evidence, and preserve
accepted D15 stability, floor seams, idle behavior, closet/furniture opacity,
Vanilla and optional Modernized modes. Recheck replay consistency and 120 Hz
cost; obtain a focused user playtest before acceptance.

New slot SHA256: `7bf643dcfff92379ce5b9f06789eedf0a1c8c9cd4a46c1cfcde8670d9dba5743`.
UI slot 8 = debug slot 7 / `slot07.pst`. Private intake and player-file manifest
are local at `recomp/analysis/d17p-subway-bands/`. Retail state stays untracked.
[Accepted baseline and distinct old/new state identities](documentation/96-d15-accepted-precision.md).

**Initial investigation:** private approach captures compare all four geometry/texture
precision combinations at 120 Hz and Corrected/Corrected at 60 Hz. A distant
horizontal ceiling discontinuity is visible with both Original and Corrected;
this does not yet establish the cause or identify every reported band. Local
evidence: `recomp/analysis/d17p-subway-bands/intake.json` and `probe.json`.
No renderer changes; player files unchanged. Next: longer approach and matched
primitive tracing before choosing a fix.

**Cause and accepted fix:** Vanilla shows the same bands; they are
three original limits, not a D15 regression. (1) The far limit: rooms past the
render context's `+0x74` (28072/33280 in the subway) are never drawn, and the
fog fade before it is squeezed into a pixel or two, so the corridor ends in a
hard black box. (2) Integer portal rectangles a pixel or two short of distant
openings drop whole strips of the next section's ceiling (rectangle outcodes in
`0x8001160c`), leaving full-width black lines. (3) One-pixel-tall distant faces
whose integer NCLIP is zero or wrong-signed are skipped, leaving jagged partial
lines. Proven with packet rasters, live limit reads, a far-limit experiment and
per-mesh face traces with context limits and rectangles.

New Modernized **Draw distance** option (`extended` default, `original`;
schema 24, `run.py --draw-distance`, settings **D**; Vanilla always original):
doubles the render-only limits (level globals and gameplay checks untouched),
widens portal rectangles by 2 native pixels, and enables the runtime's precise
NCLIP culling (exact PGXP sign, only with Corrected geometry or textures).
Hooks `0x8006276C`/`0x80062B48`/`0x8002FFEC`, guarded code ranges, framework
patch exported and verified on the clean pin. Matched 4x views show a continuous
ceiling and the real corridor end. All 12 private slots: unchanged game rate,
ring peak at most 50%, no budget hits or replay misses; 120 Hz cost unchanged.
Python 106 (2 skips) and native controls/near/input/PGXP/GTE tests pass.
The user's playtest accepted the fix.
[Cause, implementation, evidence and limits](documentation/97-d17p-distant-bands.md).

### D17Q - Residual wall/surface flicker: Corrected-renderer stability polish

**Done, user-accepted (2026-10-04).** After extended play the user wrote
"the whole job is 100% accepted", and praised how cleanly the game now
plays. Accepted executable
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960` is the new regression baseline. Candidate record follows; see
[cause, fix and evidence](documentation/99-d17q-uv-seams.md). The remaining diagonal line
(UI 12 panel at the crosshair on load, flickering on W/S) was a one-texel UV
seam. The GL/VK backends applied the 2D mirrored-sprite UV bump per triangle,
and a wall triangle with an exactly vertical integer edge qualified while its
partner did not, so the two halves of one polygon sampled one texel apart. As
movement changed the integer corners, the seam toggled. Positions, depths and
UVs were verified consistent; offline exact rendering is seamless. Fix:
perspective-corrected 3D triangles use full UV limits and no sprite bump
(`gpu_uv.h`, GL and VK; `PSX_UV_3D_LEGACY=1` compares). Original textures and
Vanilla are pixel-identical; 12-slot 120 Hz sweep and 60/180 Hz: zero replay
misses; tests pass; codegen hash unchanged. **User to confirm** steadier walls
on W/S routes in UI 12 and elsewhere (confirmed). Originally requested by the
user after accepting D17N. With the systemic
subdivision zigzag gone, some walls and surfaces can still show subtle
flickering or residual instability during movement. This is final polish, not
a reopening of D17N.

**Objective:** investigate the remaining wall/surface flicker and determine how
close Corrected geometry and textures can get to genuinely stable rendering
during movement, while Original textures, Vanilla and the software renderer
keep the original PS1 behaviour.

**Investigation:** do not assume D17N's cause. First establish, with matched
private captures and primitive/packet evidence, what actually moves or changes
between frames on affected surfaces. Candidate directions only, not diagnoses:
residual PS1 coordinate quantisation, geometry transformation, clipping and
near-plane handling, polygons still on the subdivided/fallback path (the D17N
`subdivided_kept` counter and polygons without exact projections), vertex and
depth precision, texture coordinates (including integer UVs and texel
snapping), culling/visibility, high-refresh interpolation and other renderer
behaviour. Reuse the method and history of
[D17N](documentation/98-d17n-world-subdivision.md),
[D17P](documentation/97-d17p-distant-bands.md),
[D15](documentation/96-d15-accepted-precision.md),
[R01](documentation/93-disruptor-reference-research.md) and the D17A/D11B
clipping notes where relevant.

**Acceptance:** identify the remaining sources with evidence; implement bounded
Corrected-path improvements where justified, or document unavoidable limits.
Verify 60/120/180 W/S and turning routes across the private slots with no
game-rate, budget or replay-miss regressions, preserve the accepted
D15/D17N/D17P and D17C/D/E/F/K/L/O baselines and the Original/Vanilla paths,
and the user confirms steadier surfaces in play.

### D17R - Sky turns black toward the edges when looking up (game-wide)

**Done (user-accepted 2026-10-21: "Perfect!! Lock this in.").** Executable
`fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d`
is the regression baseline. Follow-up sky artifacts are D17R1.

User report, 2026-10-04 (D22A playtest). UI slot 9: look up. The
central sky renders correctly, but toward the left and right edges of the view
it turns black/dark, as if the edges had become night.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Investigate the underlying sky rendering, not a level patch. Directions, not
assumed causes: the original sky/background geometry and coverage, widescreen
and FOV expansion (D14), clipping, projection, culling, geometry limits and the
original renderer's assumptions about the visible horizontal field, including
our first-person presentation. Reuse D17O ([note 92](documentation/92-d17o-sky-intake.md)),
R01 and the renderer notes.

**Also seen, 2026-10-05 (user, after D22C):** UI slot 2 (savestate file 01,
SHA-256 `756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`,
written 2026-10-05 09:10), first medieval level: "strange behaviour with the
sky that's directly ahead of you". The user suspects the same cause as the
black peripheral sky and asked to record it here rather than as a new job.
Confirm whether it is the same symptom (edges going black) or a different
sky artifact before assuming one cause; split it out if it is not.

**Also reproduced, 2026-10-05 (user, D22B game-wide playtest):** the Roman /
HOG HEAVEN environment (`level 10`) shows the same black sky toward the
peripheral edges of the view. The problem is game-wide, not specific to
LEVEL01's sky.

**Acceptance:** slot 9 and HOG HEAVEN show a continuous sky to both edges in 16:9 and the
original aspect, first and third person; the first map's accepted sky (D17O
slot 10) is unchanged; the solution is shared sky rendering that holds across
levels; the user confirms.

**Work log, 2026-10-20 (Needs playtest).** Cause: the D14 widescreen columns.
4:3 is clean; in 16:9 the dark areas start at the 4:3 boundary. The shared
sky routine `0x800388e4` runs after the screen-mode call has reset the root
rectangle to 4:3, so its flat sky-colour fill stops there (black top
corners), its camera-facing backdrop is authored only for the original field
of view (a slanted edge with clouds over black past it), and its object-clip
rectangle rejects cloud and horizon pieces wholly in the margins. New
`recomp/src/ttk/sky_widescreen.cpp` widens all three by the live margin on
existing hooks (`0x8002bc18`, `0x800292a0`, `0x80010000`, sky call sites
only), gated by the D17O routine digest, one player and a live margin;
Vanilla and 4:3 are unchanged. Offscreen evidence: slot 9 (file 08 copy,
hash above) clean at all headings tried in first and third person, 16:9 and
21:9, 60 and 120 FPS (849 replays, 0 failed); medieval slot 2 corners go
from black to the level's fill; D17O slot 10 keeps every sky offset at
0,0,0. New `ttk-sky-test` passes; near, aim and input suites pass. HOG
HEAVEN runs the same routine with all sky layers, but its outdoor view was
not reached offscreen. The medieval "strange sky directly ahead" was not
separately reproduced. Executable
`fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d`.
[Cause, change and evidence](documentation/172-d17r-sky-edges.md).

### D17R1 - Sky artifacts when looking up/down and turning

**Done (user-accepted 2026-10-21: "lock it in and close it out").** Executable
`9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f`
is the regression baseline. User report, 2026-10-21, while accepting D17R ("Improvements to
the new skybox rendering"):

- UI slot 2 (savestate file 01, SHA-256
  `756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`,
  written 2026-10-05 09:10, first medieval level): look up and down slowly in
  first person and the sky shows artifacting. It gets stronger when looking up
  about 45 degrees and then turning left and right, and the camera may jerk
  there.
- UI slot 8 (savestate file 07, SHA-256
  `115c923ca9179ae3e3d947b4c6e447b0b44ff827bc0aec31542bbab5f1d2a937`,
  written 2026-10-21 06:38, level 1): look left and right, or all around: a
  similar artifact, without the camera jerking.

Test only on dated private copies; verify these hashes first, since slot
numbers get reused. Investigate before assuming a cause: compare 4:3 against
16:9 and `DNTTK_SKY_WIDE=0` (the D17R widening), 60 against higher frame rates
(D17 replays, the D17O camera-relative sky), and Vanilla. The camera jerk may
be a separate frame-pacing or input problem (D17B family) that only shows in
the sky; split it out if so. Reuse note 92 (D17O) and note 172 (D17R).

**Acceptance:** slots 2 and 8 show a stable sky while looking up/down and
turning in first person, 16:9 and 4:3, at 60 and the saved high frame rate;
no camera jerk; D17O slot 10 and D17R slot 9 unchanged; Vanilla unchanged;
the user confirms.

**Work log, 2026-10-21 (Needs playtest).** The user's recording
(`research/screencaps/Video_2026-10-21_07-02-29.mp4`) showed the clouds cut
into stair-stepped, screen-aligned blocks. Reproduced offscreen on the slot 2
save at 60 and 120 FPS with the player's extended draw distance (whole with
original distance). A GP0 dump showed the cloud quads were not drawn, not
covered. Cause: the D11B near clip, which draws the eye view's near polygons
(the sky's clouds included), wrote its packets into the game's render ring
with a 60% per-frame cap; with extended distance the game alone used 61% of
the ring (85.6 KB), so the sky, drawn last, fell back to the original path
about 60 times a second and its near cloud quads were dropped. Change: the
near clip's host packets now have their own 512 KB arena in the GPU DMA
aperture (four 128 KB regions, one per frame), and the game's ring is no
longer touched; framework `gpu.c` keys aperture addresses apart from main RAM
in the host precise-vertex table (patch exported, applies clean to the pinned
framework). Evidence: 0 fallbacks (was 123 per 2 s), clouds whole; 120 FPS
sweeps of slot 2, LEVEL01 slot 8, D17R slot 9 and D17O slot 10 with 0
fallbacks, 0 packet skips and 0 failed redraws; Vanilla unchanged; near (both
precisions), sky, aim and input suites pass. The camera jerk was not
reproduced or isolated. Executable
`d632379685d59dd1fa953f088459ac69af1bff6022ef5bd8886ac2a0bbf27f03`.
[Cause, change and evidence](documentation/173-d17r1-sky-blocks.md).

**Work log, 2026-10-21 (regression fixed, Needs playtest).** User, first
playtest: "quite a lot of graphically breaking issues ... this is a no go".
The high-refresh redraw workers walked the ordering table with a loop that
also ended at any bit-23 link, and every new arena address has bit 23 set:
each in-between image stopped at the first host packet and showed only the
sky fill over stale picture (no weapon, no HUD). Still screenshots show real
frames, so the first checks missed it. Framework `render_replay.c` now ends
the walk like the live walker (`0xFFFFFF`, or a bit-23 link outside the
allocated aperture). Forced-turn redraw and walking/turning replay sequences
on slot 2, LEVEL01 slot 8 and D17O slot 10 are whole, 0 failed; turn pops
match the baseline executable. Patch re-exported and verified. Executable
`9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f`.

### D17R2 - Sky turns black at certain viewing angles (slot 8)

**Done (user-accepted 2026-10-22).** Current slot 8 is LEVEL06 room 28.
Turning away from sky-flagged room 22 makes the portal walk return no sky,
so the entire sky pass is skipped. Modernized now keeps the loaded sky as
background after the original far-limit decisions, with guarded caller,
camera, stack and code identity. Vanilla and empty-sky levels are unchanged.

Build and sky/near contracts pass. Current-slot 60/120 motion, 48-view sweep,
4:3/third-person/original precision-distance samples and prior sky scenarios
pass; user fully accepted the result. [Note 197](documentation/197-d17r2-angle-dependent-black-sky.md).
Accepted executable `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.

**Acceptance:** slot 8 left/up view stays correct; stable yaw/pitch sweeps and
moving turns; prior sky improvements intact; no rendering, culling or
performance regressions; 60/high-refresh presented checks and user playtest.
Preserve Vanilla and existing display/camera modes.

**Dependencies:** D17O, D17R, D17R1 (Done).

### D17S - Auto frame-rate default: follow the display up to a cap

**Done (user-accepted 2026-10-22).** AUTO is the default for new Modernized
profiles. Existing explicit choices are preserved. The compatibility-first
candidate uses `refresh / ceil(refresh / 120)`: the highest refresh divisor
at or below 120 FPS, without assuming variable-refresh support. Examples:
60 -> 60, 120 -> 120, 144 -> 72, 165 -> 82.5, 180 -> 90, 240 -> 120.
Fractional refresh is retained; unknown refresh falls back to 60. This
supersedes the earlier undecided 144/165 exceptions. AUTO follows display
changes and applies from the in-game menu. Vanilla remains unchanged.

**Acceptance:** compiled production resolver and live setter tests cover the
listed modes and display changes; build, profile tests and offscreen live
menu transitions pass. The user fully accepted the result on 2026-10-22. Physical monitor transitions
and the broader hardware matrix remain unmeasured by the agent. [Note 203](documentation/203-d17s-auto-frame-rate.md).

### D17T - Smooth title menu and attract demo at the saved frame rate

**Done (user-accepted 2026-10-18).** User: "i can confirm this works ... it's a good first start." Note [140](documentation/140-d17t-frontend-smoothness.md). User request: the title menu and the attract
demo behind PRESS START are the first thing a player sees, and they look like
they run at about 30 fps. Present them at the player's saved Modernized frame
rate (`controls.frame_rate`, e.g. 120 or Match Display) so the first
impression is smooth. Vanilla stays at its original 60 Hz presentation, the
intro FMV keeps its native rate, and demo/menu logic timing must not change.

**Investigation (2026-10-18, before implementation):**

- Code: D17 render replay is gated to gameplay. `gameplay_context()`
  (`recomp/src/ttk/modern_controls.cpp:342`) requires game mode
  `0x800bcbb0 == 1` and no pause (`0x800be568`, `0x800d2540`); mode 0 is the
  frontend/attract dispatcher (`0x8001bb84`). `composition_hook`,
  `timeline_provider` and `start_workers_once` in
  `recomp/src/ttk/frame_replay.cpp` all refuse outside it, so the replay
  workers are not even started until the first level. Note 80 section 3 lists
  "FMV, menus, loading, 2D screens: Step (interpolation off)" by design.
- Measured (offscreen real GPU, D17 harness `SLOT=none`, private profile,
  Modernized at 180, no input): after about 150 s of intro FMV the game enters
  mode 0 with full 3D scenes (Duke running, PRESS START and the logo). Polling
  the display buffer index `0x800bdbd0` gives **9.7-10 guest images per second
  (one every 6 video fields)**, each repeated about 6 times at 60 Hz. That is
  slower than gameplay (30, or 20 on overrun), so it reads as worse than 30 fps.
- Not yet known, and the first step: why the demo is 10 Hz. Either the
  frontend paces itself (a fixed VSync divisor, as on the original console)
  or it is CPU-bound because the CPU overclock and D23F fast timing are leased
  only from the player update during Modernized gameplay
  (`overclock_lease`, `modern_controls.cpp:724`). Compare against Vanilla and
  the original; check the frontend's per-frame delta and VSync calls.
- Risk: the attract demo is probably recorded input playback. Making the
  guest render more frames (faster CPU) could change its pacing or desync the
  demo. Interpolated redraws (D17 replay) leave the guest untouched and are
  the safe route; any change to guest speed needs a demo equivalence check.

**Result (2026-10-18, implemented):** the 10 Hz is the original's own pace:
the flip `0x8001fcbc` waits at least 6 fields in mode 0 (2 in gameplay). A
30 Hz frontend experiment desynced the attract demo at once (recorded
per-frame input), so the guest pace stays and the frontend gets D17
in-between images instead: 118-119 distinct presents a second at 120 (179 at
180, 239 at 240) with the guest still at 10. The user linked the menu's
clunky feel (D19B): the tick reads the pad once per 6 fields, so short taps
were lost (45 ms keyboard taps: 7 of 12); in Modernized a new frontend press
is now held for 6 fields (12 of 12, one step each). Interim until our own
menus (D19). Details in note 140.

**Original plan:** allow render replay in a frontend context
(mode 0, demo running, no FMV), start the workers there, and interpolate the
demo camera and actors between guest images with the non-late path (the game
owns this camera: no mouse look). Check that the attract frames go through the
same composition `0x80026164` (and the `0x800c27bc == 1` gate) and the demo
Duke/actor slots; add discontinuity rules for demo camera cuts, demo scene
changes, loads and the switch into the menu. 2D menu text and logo stay
crisp at their latest state. A 100 ms image interval means larger blends than
gameplay; judge quality at 120 and 180.

**Acceptance:** in Modernized with a saved rate above 60, the attract demo and
the title menu present distinct interpolated images at that rate (measured:
presents and distinct images per second), with demo logic unchanged (guest
fingerprints match a 60 Hz run of the same boot) and no pops at cuts or
ghosting on text; menu input as responsive as now; Vanilla, FMV and gameplay
D17 unchanged; the user judges smoothness on their display.

### D17U - Smooth loading screens (spinning Duke logo) at the saved frame rate

**Done (user-accepted 2026-10-18).** User: "i accept!! absolutely perfect." Note [141](documentation/141-d17u-loading-screens.md). User request after accepting D17T: the loading
screens (the spinning Duke logo) still look low frame rate. Make them present
smoothly at the saved Modernized rate, as D17T did for the title, demo and
menu. Vanilla and loading speed/behaviour stay original.

First steps: identify the loading screen's mode and draw path (D17T saw 2D
screens between demo scenes at about 10 images/s with no view composition);
measure its guest image rate; find what the logo's spin is driven by (a
per-frame angle, a field counter, or CD progress) and whether it is drawn
through the GTE (3D) or as 2D sprites. Then choose: replay with an
interpolated spin angle, a host redraw of the same logo at present rate, or
another method. Never slow loading or change what the guest does while it
loads; keep the CD/audio timing intact.

**Acceptance:** the spinning logo presents distinct, evenly spaced images at
the saved rate (measured), loading takes no longer than before (measured),
no visual glitches at the start/end of a load, Vanilla and 60 unchanged; the
user judges it on their display.

**Result (2026-10-18, implemented):** loading screens are drawn by three
callers (the blocking load loop `0x8001c440`, the frontend tick's load branch
and the level start) as a static backdrop plus the logo `0x80025554`, a GTE
model turned about Y by the angle `0x800beaf8` plus the field delta; 10
images/s between demo scenes, 30 in level loads. The logo draw is now captured
as a loading image and redrawn by the D17 workers with an interpolated angle
(hook `0x80025554`, regenerated, codegen hash unchanged). At 120: DUKE HILL
load 119 distinct presents/s (5 before), 6.82 s (6.84 before); demo scene
loads 112; 180 and 240 scale; 60, Vanilla and `DNTTK_LOADING_INTERP=off`
original. Details in note 141.

### D17V - Modern view bob: steady Duke 3D-style tempo

**Done (user-accepted 2026-10-19).** The user: "total victory. i even played it
on strong and i love it." Executable
`a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193` is the
regression baseline. Duke 3D's fixed-tempo |sin| dip (deeper, not faster, when
running) on vblank time, plus a glide that evens out Duke's stride surge.
[Cause, implementation and evidence](documentation/151-d17v-view-bob.md).

User request (2026-10-19): the first-person running bob looks like a sped-up
walking bob and reads as erratic. Review how the bob works and replace it with
a smoother, more realistic modern bob that emulates the view bob of Duke
Nukem 3D (reference: the user's eDuke32 recording `research/screencaps/running.mp4`,
walking first, then running). Duke 3D's weapon motion while moving is out of
scope. The view's smoothness and stability are what matter.

**Acceptance:** the Modernized first-person bob keeps one steady tempo from
walking to running, grows deeper (not faster) with speed, has no sideways
jitter, and settles smoothly when Duke stops or lands. The existing
off/subtle/on/strong setting still scales it, and Vanilla is unchanged.
Measured on driven walk and run routes, then the user playtests and accepts
the feel.

### D17W - Overturned minecart white triangles in first person

**Done (user-accepted 2026-10-20: "that cart is looking handsome!!").**
Executable `8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c`
is the regression baseline. Cause found and fixed in the near clip
(`recomp/src/ttk/near_clip.cpp`, no framework change).

Cause: the cart model (object mesh `0x80129f58`) lays two faces on the same
four corners of its back panel: a textured FT4 (records `0x8012a1d0`,
`0x8012a1e0`) and a flat F4 (`0x8012a538`, `0x8012a540`, palette color 0,
white). The original object renderer (`0x80010000`, F4 path `0x80010810`)
gives both one AVSZ slot and inserts at the slot head, so the earlier record,
the texture, is painted last and covers the flat face. In the eye view the
D17A precise path draws every prop polygon within 3072 units itself, and each
host piece took its own slot from its own average depth. Flat and gouraud
triangles are never depth-tested in the GL renderer (painter order only), so
flat pieces in nearer slots painted large white triangles over the panel.
`DNTTK_NEAR_CLIP=0` and third person were clean, and `DNTTK_NEAR_TINT=1`
showed the white areas as host object pieces. A depth push alone did not help
(flat pieces have no depth), which confirmed the ordering cause.

Fix (systemic, every object mesh drawn by the near clip, not this model): for
each pair of host-drawn faces of one mesh that are coplanar (normals within
0.9999, corners within half a unit of the plane) and overlap (separating axes
in the plane, more than half a unit), the face the original paints first
(farther slot, or the later record in one slot) is ranked behind the other.
Every face of such a group sorts by the group's average depth and is emitted
in rank order, so the ordering table paints them as the original did. A
ranked face is also pushed back 1/4096 of its depth per rank for the depth
test of textured pieces. `DNTTK_NEAR_COPLANAR=0` (developer) turns it off for
comparisons. Third person and Vanilla never reach this code.

Evidence (offscreen, OpenGL 4x, 16:9, 120 Hz, first person, private copy of
the player's slot 10 in `recomp/analysis/d19k-load/cards-mine`; screenshots
in `recomp/analysis/d17w-minecart/`, local). Executable
`8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c` (the slot 10
shots were taken one build earlier, identical but for the developer switch):
- Slot 10 cart: clean at the load position, walked closer, backed off, turned
  left and right, looking down, strafed to the side (`fixed-*.png`). Before
  the fix: white triangles at the same positions.
- Trace (`DNTTK_PRIMITIVE_TRACE`): the two F4 records carry rank 1 and their
  pieces share the panel's slot.
- A/B on 12 other private savestates (copy of the D22D cards, levels 0 to
  2 and the club), coplanar on and off: identical except animated actors
  (chickens, dancers) and 14 pixels of noise.
- Vanilla on slot 10: unchanged.
- `ttk-near-test` (with the owned EXE): new case, a textured and a flat face
  on the same corners in both record orders; the original's winner keeps the
  nearer depth, both share one slot and the other face is drawn first. It
  fails without the fix. Python 160 OK (2 skipped), `ttk-input-test`,
  `ttk-live-env-test`.

Remaining: world meshes (`0x80011020`) are not
ranked; no coplanar world case is known. Flat/gouraud host pieces still have
no depth in the GL renderer; ordering, not depth, protects them.

**Opened 2026-10-20 (brief research).** User: "load up slot 10
and take a look at the overturned minecart right in front of you and it is
artifacting like crazy it seems to have white triangles all over it where for
some reason it's texture is not showing properly ... I haven't seen anything
like this since the very early days of development so there may be something
unique about this object."

Reproduction: player UI slot 10 (file `state_800AB6FC_slot09.pst`, sidecar
level 3), copied privately to `recomp/analysis/d19k-load/cards-mine`; load and
look straight ahead. Screenshots (local):
`recomp/analysis/d17w-minecart/`.

Brief findings (2026-10-20, offscreen OpenGL 4x 16:9 120 Hz, executable
`15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b`):
- **First person (the player's profile):** the green back panel of the cart
  in front shows two or three large flat white triangles over its texture;
  the coal load and the other cart beside it are textured normally.
- **Same with GEOMETRY ORIGINAL:** identical triangles, so not the D15 / D17N
  corrected-geometry subdivision.
- **Third person, same savestate:** the cart is clean.
- So the fault depends on the camera position or first-person processing,
  not on the object alone. Leads, in order: the D11B near-wall clipping of
  textured polygons (a split polygon losing its texture page, CLUT or UVs:
  white is an untextured / zero-colour fallback); a polygon type unique to
  this model (e.g. a textured quad or gouraud-textured primitive that only
  this object uses at that distance); the game's own subdivision of large
  near polygons in first person; a texture page or CLUT that is not resident
  in VRAM from this view. Not yet checked: the software renderer, Vanilla
  (no first person; move the original camera close), the GPU command packets
  of those triangles (`render_replay` / GPU trace), and whether the white area
  changes with distance.

**Acceptance:** from player UI slot 10 in first person the cart is fully
textured from every distance and angle; the fix is systemic (the polygon class
or clipping path, not this object); other first-person near-wall clipping
(D11B) unchanged; Vanilla unchanged; the user judges it.

### D17X - Small white triangles on posts and soil in first person

**Done (user-accepted 2026-10-22).** Both slot 6 and slot 7 are accepted.
Slot 7 reproduced as triangular soil cutouts exposing brighter ground during
movement. Warped host quads now retain the native diagonal and per-triangle
plane depth, preserving the mound's floor contact. Both fixes are in the
accepted build. [Note 202](documentation/202-d17x-posts-soil.md).
Originally queued on 2026-10-21. The user thinks it is similar to D17W (the minecart's
white triangles).

- UI slot 6 (savestate file 05, SHA-256
  `9d995e41fe1a9ddda0b500664941ea965970dbc7557c483757f05491019fe594`,
  written 2026-10-21 06:30, level 6): small white triangles appear on the
  wooden posts holding up the boat in front, most visible in first person
  while walking slowly backwards and forwards.
- UI slot 7 (savestate file 06, SHA-256
  `85b08d702946a955ec1f118f53d518dcc0cee8d4107636e63027cacf4a93323e`,
  written 2026-10-21 06:34, level 1): the same kind of artifact on the soil
  from the grave dig, in its corners, when walking back and forth.

Test only on dated private copies; verify these hashes first. Investigate
before assuming the D17W cause: check `DNTTK_NEAR_CLIP=0`, third person and
`DNTTK_NEAR_TINT=1` (host near-clip pieces), whether the posts and soil are
object meshes or world geometry, and whether the D17W coplanar-face ordering
covers them. Fix the system, not these models.

**Acceptance:** no white triangles on the slot 6 posts or slot 7 soil while
walking slowly in first person, 16:9 and 4:3, at 60 and the saved high frame
rate; D17W slot 10 cart unchanged; Vanilla unchanged; the user confirms.

### D17Y - Slot 8 fire-platform flicker and jumping castle-wall disappearance

**Done - user accepted and authorized closeout (2026-10-23).**
User reported two rendering problems from their current UI save slot 8:

- Load slot 8, jump and look on top of the fires to Duke's right. The
  platform supporting those fires flickers heavily.
- From the loaded orientation, turn approximately 90 degrees right and jump
  vertically. The castle walls at the top of the screen disappear during
  the jump, then return when Duke lands.

Both routes reproduced on a private dated copy. The fire artifact came from
animated faces using the game's empty missing-texture descriptor and sampling
the live framebuffer; Modernized now filters those faces from a private mesh
copy. The wall strips were clipped by the top of a direct portal into a taller
sky-bearing room; Extended draw distance now preserves that open upper edge
without opening downstream apertures. See [note 206](documentation/206-d17y-slot8-rendering.md).

Build and native regression suites pass. First/third-person, 4:3/16:9,
60/120 FPS jump captures pass; saved Auto also checked offscreen (60 FPS
fallback without a physical display). User confirmed the graphical oddities are completely resolved and fully
accepted the fix: "lock it in. close it out". Player settings/saves were
unchanged by implementation/testing; implementation and documentation are
included in the authorized closeout.

**Acceptance:** reproduce both routes; keep the fire platform stable and the
upper castle walls visible throughout takeoff, apex and landing. Check both
camera views, 4:3/16:9, 60 FPS and the saved high-refresh setting; retain
correct occlusion, nearby geometry, existing sky/depth fixes and Vanilla.
Use a systemic fix supported by evidence, and obtain user playtest acceptance.

**Dependencies:** D17A, D11B, D14 (completed/accepted).

### D17Z - Slot-12 medieval corridor wall glitch

**Status: Done (user-accepted with D17AA; closeout authorized 2026-10-26, commit 5c8b5de). User re-confirmed the renderer state in play 2026-10-30 after the D17AM unified-geometry default. Depends on D11B/D17N/D17Q (accepted).** Candidate
preserves the native triangle diagonal for Corrected-texture world quads before
near clipping, preventing uneven wall UVs from remapping at takeover. Build,
focused interpolation/clipping regression and private 60/120 corridor routes pass;
Original/Vanilla comparison evidence and limits are in [note 266](documentation/266-d17z-medieval-wall-textures.md).
Acceptance still requires the user's matched slot-12 corridor review.
**When fixed, remind the user that `research/screencaps/medieval-glitch-wall.mp4`
is there for comparison, both in the playtest handoff and closeout.**

### D17AA - Slot-12 wall tear exposes the pushable block

Status: In progress. User selected implementation immediately after backlog
creation on 2026-10-26. Load a private copy of current UI slot 12, take one step
forward and identify the wall tear exposing the pushable block. Reproduce and
trace the wall/block primitives before selecting a fix. Preserve accepted D17Z
texture continuity, world/prop occlusion, pushable-block gameplay, precision and
clipping, first/third person, Original/Vanilla and 60/120 replay behavior.
Acceptance: the wall remains opaque on the same step/retreat/turn route, without
hiding legitimately visible block surfaces or introducing missing geometry;
focused regression and matched before/after visual evidence required.

### D17AB - Slot-5 mirror sky leak, HUD loss and palette corruption

**Status: Done (user-accepted; closeout, commit and push authorized 2026-10-27).**
User reports outside clouds rotating overhead while indoors facing the mirror;
walking/jumping close with pistol (key 2) briefly hides HUD elements. After
DNSTUFF, selecting freeze thrower (key 0), firing toward the mirror and repeatedly
jumping/firing can cause severe multicolored rendering and a persistent shifted
world palette. These are user reports until independently reproduced.
Scope: reproduce current UI slot 5 using private save/card/settings copies;
trace mirror render/clipping, ordering, framebuffer/CLUT writes and high-refresh
replay; implement a bounded correction supported by evidence.
Acceptance: indoor mirror approach has correct sky occlusion, stable HUD and no
palette corruption through pistol and freeze-shot jumps, repeated approaches,
load/reload and 60/120 presentation. Preserve reflected geometry/opacity,
intentional freeze effects, outdoor sky, first/third views and Vanilla behavior.
User accepted the delivered mirror fixes: "lock it in and close us out!".
Implementation, regression tests and documentation commit/push are authorized.
[Note 280](documentation/280-d17ab-mirror-rendering.md) retains the cause,
old/fixed comparison and bounded verification. Existing framework changes are
unchanged; no new sky suppression or gameplay controls.

### D17AC - Slot-11 phone disappears on approach

**Status: Done (user-accepted; closeout authorized 2026-10-28). Related: D17L.**
User reports the phone disappears as the player approaches it in current UI
slot 11. Verify current save identity with private copies, then reproduce
approach/retreat and inspect object visibility, near clipping, culling,
depth/order and contact with adjacent surfaces. A culling fault or touching
sprite is a hypothesis, not an established cause.
Acceptance: identify the responsible path, implement a bounded correction and
verify stable phone visibility while preserving legitimate occlusion and nearby
props in relevant views and refresh rates. D17L's historical slot-11 tabletop
repair is related evidence, not proof this report has the same cause.

Accepted fix: the phone rear panel is embedded roughly 10-11 units in its wall.
A shape-verified, opaque-panel-only 16-unit depth allowance preserves its
visibility without changing raster geometry or gameplay. Build, native packet
contracts, 60/120 private routes and replay restoration checks pass within
[note 292](documentation/292-d17ac-phone-wall-depth.md)'s limits. User accepted
the fix and explicitly authorized implementation, tests and documentation
commit/push closeout on 2026-10-28.

### D17AD - Tile loading background across widescreen resolutions

**Status: Done (user-accepted; closeout authorized 2026-10-28). Related: D17U.**
User reports the background behind the giant spinning nuke loading symbol,
including menu transitions, remains 4:3 with black side bars. Inspect supplied
`screencaps/loading-4-3.png` (resolve its actual local location when selected).
Extend/tile the background without stretching its artwork, adapting to the
active resolution/aspect ratio. Preserve spinner geometry, animation timing,
menu/loading composition and Vanilla.
Acceptance: seamless non-stretched coverage at 4:3, 16:9, 16:10, ultrawide and
window resize/custom resolutions; no uncovered side bars, seams or regression
to the accepted smooth loading spinner. Screenshot inspected during implementation.

Accepted fix: guarded copies of the original tiled backdrop cover both wide
margins without stretching the art or changing the spinner. Adaptive 4:3
retains clipped copies for subsequent window growth. Build/native contracts,
fixed-aspect captures and save-recovery checks pass within
[note 293](documentation/293-d17ad-loading-background.md)'s limits.
User accepted: "great work! lock it in and close us out". Implementation,
tests and documentation commit/push are authorized. No next job started.

### D17AE - Slot-11 ceiling artifact above door

**Status: Done (user-confirmed 2026-10-29; follow-up D17AL). Related: D14F, D17AG, D17AK.**
Private copy of current UI slot 11; walk backward/forward at 90. Identify ceiling geometry/render operation; stable ceiling without missing or incorrectly visible adjacent geometry.

User symptom (2026-10-29, re-created as a brand-new-game save in slot 11):
the outdoor sign's support post shows through the top of the doorframe
(research/screencaps/flickering.mp4). Cause: native painter order. The header
is a world face sorted by its farthest corner (slot 105); the post is a prop
sorted by its average depth (slot ~103), so it paints over the nearer header.
Reproduced with host near clipping disabled and in native captures.

Changes:

1. Scene depth (`near_clip.cpp`, `DNTTK_SCENE_DEPTH`, default on in
Modernized). After each world/scenery mesh draws, its own opaque ring packets
are matched to its projected corners and given true view depth through the
host-vertex channel; the drawing-chain close (`0x8001fba0`) flushes the last
mesh. Packets, ordering slots, UVs and colours are unchanged. Translucent
packets, sprites, sky, water, the first-person layer and Vanilla keep painter
order.
2. Framework (`render_replay.c`): replay workers no longer let later PGXP
metadata replace a plugin's host depth for the same vertex word, matching the
live path's precedence. Without this, 120-FPS in-between images still showed
the post. Patch exported and verified (55 files match the clean pin).
3. Reverted Sonnet's removal of the world backface cull: the retail world
renderer does call NCLIP (`0x800115e4`, `0x80011734`, `0x80011764`,
`0x80011788`). The FOV-independent guard band is kept; with it, the original
void route and the new save show an intact ceiling.

Validation: post hidden in every capture at FOV default/60/90/120, 60 and
120 FPS, standing and walking (16-frame rocks, 8-step walks); new
`ttk-near-test` contract plus near, controls, input, aim, gas, loading and
live-env suites pass; 120-FPS replay 0 misses/failures, about 5% more worker
time. Same change fixes D17AG. User confirmed the slot-11 fix in play
(2026-10-29). Known regression: embedded payphone, tracked as D17AL.
See `documentation/322-d17ae-ceiling-investigation.md`.

### D17AF - Latest slot-10 ground water disappears

**Status: Done (user-confirmed 2026-10-29: "the water isn't disappearing underfoot that problem is solved"; [note 326](documentation/326-d17am3-am4-unified-default.md)). Related: D14F, D17AK.**
D17AM4 (new slot 10 save, 2026-10-29 19:38): the native water builder skips
every tile with a corner nearer than H/2 (0x8002f70c), so the tiles around
Duke never reached the clip hook. Near flags are now cleared while the eye
view is live and those tiles are clipped, depth-tested 32 units back (keeps
the channel walkway's grating over the water, as the original paints it).
Preserve latest resaved UI slot 10 without distracting blood. Slow backward/forward movement; distinguish visibility, clipping, depth, transparency and texture state. Correct water/surrounding surface relationships throughout.

Cause: GTE corner saturation on close water tiles; native water FT4 builder at
`0x8002bc18` / RA `0x8002fbb4` is independent of the mesh pipeline, so ordinary
near-clip did not intercept it. Changes: `water_clip.inc` added; hooked in
`near_clip.cpp` at line 1406. Contract tests passing (all groups). Opt-in via
`DNTTK_WATER_CLIP=1` pending gameplay validation. Remaining: gameplay verify in
slot-10, regression check in other water levels, promote from opt-in to default
after validation. Note 321: [documentation/321-d17af-water-clip.md](documentation/321-d17af-water-clip.md).

### D17AG - Slot-6 holes in wooden panelling

**Status: Done (user-confirmed 2026-10-30; earlier: selected 2026-10-29). Related: D14F, D17AE, D17AK.**
Private current UI slot 6; pan left/right at panelling above caravan/crate. Continuous correct visibility without revealing hidden surfaces.

Cause: native painter order (note 320). Fixed by the D17AE scene-depth change:
at the slot-6 load view the canopy holes present with `DNTTK_SCENE_DEPTH=0`
are gone at FOV 64, 90 and 120, 60 and 120 FPS. Turn sweeps across slots
5/6/8/10/12 show no new artifacts. Remaining: user pan playtest. See note 322.

### D17AH - Slot-8 sky glitching and popping

**Status: Done (user-confirmed 2026-10-29: "the very best" the sky has looked; [note 326](documentation/326-d17am3-am4-unified-default.md)). Related: D14D, D17AK.**
The painter-order sky candidate (note 319) holds under unified geometry:
slot 8 clouds continuous while turning at FOV 90/120, 120 FPS. No change.
Private current UI slot 8; move/rotate. Inspect previous sky fixes and assumptions first. Stable sky plus representative other sky variants and regression checks.

Reproduction/fixture: pending inventory. Cause: unconfirmed. Changes: none at
registration. Validation: pending. Remaining: reproduction, diagnosis, correction
and appropriate visual/regression evidence; preserve player originals.

### D17AI - Slot-12 wall near pushable cube disappears

**Status: Done (user-confirmed 2026-10-30; earlier: not reproduced in D17AM4, [note 326](documentation/326-d17am3-am4-unified-default.md)). Related: D17Z, D17AA, D17AK.**
Old slot-12 fixture copy: forward, backward and turning routes past the
cube at FOV 90/120 show the wall intact in both renderers. Needs the
user's check before closing.
Private current UI slot 12; backward/forward movement. Stable wall with correct cube and surrounding depth/visibility.

Reproduction/fixture: pending inventory. Cause: unconfirmed. Changes: none at
registration. Validation: pending. Remaining: reproduction, diagnosis, correction
and appropriate visual/regression evidence; preserve player originals.

### D17AK - Shared renderer investigation and FOV quality sweep

**Status: Done (user-confirmed 2026-10-30; earlier: D17AM3 sweep is its evidence, [note 326](documentation/326-d17am3-am4-unified-default.md)). Related: D14A, D14B, D14D.**
D17AM3/AM4: all 12 slots and 21 level-select levels at FOV 64/90/120, 60/120
FPS, 16:9/4:3, first/third person, Bio Mask. Shared causes found: D20H
(host packet region after mid-frame loads) and the D17AF water rejection.
Cycle available private saves; capture and trace seams/missing surfaces/popping. Establish shared causes from evidence only, coordinate shared renderer edits, prefer principled minimal corrections. Replacement renderer is outside scope.

Reproduction/fixture: pending inventory. Cause: unconfirmed. Changes: none at
registration. Validation: pending. Remaining: reproduction, diagnosis, correction
and appropriate visual/regression evidence; preserve player originals.

Shared acceptance: previous code/config default, 90 and 120 FOV; intermediate
thresholds as needed. Projection/viewport changes require primary and contrasting
supported aspects. Preserve weapons/hands, HUD/menus, occlusion/transparency/sky,
save compatibility, Vanilla and accepted 120-Hz gameplay. Record exact fixtures,
attempt counts, comparable captures, automated and live results separately.
Unavailable fixtures and unreproduced symptoms remain explicit limitations.

### D17AL - Embedded prop contact depth (slot-12 payphone regression)

**Status: Done (user-accepted; closeout authorized 2026-10-29). Related: D17AE, D17AC.**
User slot 12 (new save, 2026-10-29): walking toward the payphone, its front
unit is missing until close, then pops in. Confirmed caused by D17AE scene
depth: with `DNTTK_SCENE_DEPTH=0` the unit shows at every step. The payphone
is embedded about 10-11 units in its wall (D17AC); the original painted it
over the wall, the host near path (props under 3072) already grants its
16-unit contact allowance, but scene depth gives distant native packets exact
depth with no allowance. Fix game-wide: scene-depth prop packets use the same
contact rules as host prop pieces (`DNTTK_PROP_CONTACT`, the verified phone
back allowance), so a prop's appearance cannot change at the handover.
Acceptance: phone stable from far to touching at FOV 64/90/120, 60 and 120
FPS; slot 11 post and slot 6 canopy stay fixed; props flush with walls do not
start showing through walls; Vanilla unchanged. Private copy of current UI
slot 12; old D17AI slot-12 fixture is kept in `recomp/analysis/d17ak/fixtures`.

Candidate: scene-depth prop packets now use the same contact allowance as
host prop pieces (one `prop_contact_shift` rule in `near_clip.cpp`, the phone
back identified by the existing D17AC shape check). Private slot-12 approach:
front unit missing in steps 0-3 with the change off, whole at every step with
it on, at FOV 64/90/120 and 60/120 FPS. Slot 11 post and slot 6 canopy
unchanged; 120-FPS replay 0 misses; near contracts pass. Developer switch
`DNTTK_SCENE_PROP_CONTACT=0`. See
[note 323](documentation/323-d17al-prop-contact-depth.md).

### D17AM - Unified host geometry path (parent)

**Status: Done (D17AM1-AM4 done; user playtest 2026-10-29). Related: D17A, D17AE, D17AK, D11B.**
Goal: every world and prop polygon is drawn by the host at every distance,
with depth, perspective texturing and the original's clipping/colour/UV
rules, retiring the 1536/3072 handover and the per-case depth rules that
exist only because of it. Done when D17AM1-AM4 are done.

#### D17AM1 - Inventory and unified rule design (report first)

**Status: Done (user-approved 2026-10-29: "go entirely with your recommendation, end to end"; [note 324](documentation/324-d17am1-unified-geometry-design.md)).** Catalogue every current threshold and special
case in `near_clip.cpp` (perspective_z/object_perspective_z handover, platform
depth, compact/pickup props, static box tops, ladders, phone back, coplanar
rank, source order and object bias, warped quads, world subdivision bypass,
scene depth, sky painter, viewmodel) with what each protects and its
evidence. Define the single rule set (contact tolerance, decal/coplanar
order, translucency, sort keys) and measure the cost of full takeover: host
packet bytes per frame against the 128 KB arena, CPU per frame, 120-FPS worker
time, on the heaviest fixtures (canal, outdoor town, club). Present for
approval before implementation.

#### D17AM2 - Implementation behind a developer switch

**Status: Done (user-accepted 2026-10-29; [note 325](documentation/325-d17am2-unified-geometry.md); developer switch `DNTTK_UNIFIED_GEOMETRY=1`).** Implement the approved path with a
developer switch to compare against the current renderer. Resize/restructure
the host packet budget safely (savestate-carried mod memory, D20C region
rules), keep replay precedence and stale-entry rules, and keep the
first-person, Bio Mask, sky and water layers unchanged.

#### D17AM3 - Regression sweep and default switch

**Status: Done (user playtest 2026-10-29, commit authorized; [note 326](documentation/326-d17am3-am4-unified-default.md)).** All private fixtures and level-select levels:
static, walk and turn routes at FOV 64/90/120, 60 and 120 FPS, 16:9 and 4:3;
embedded props, decals, blood, coplanar floors, ladders/platforms, pickups,
actors. Performance within D23 budgets. Then make it the Modernized default
and remove superseded special cases. Vanilla unchanged.

#### D17AM4 - Re-triage the remaining renderer batch

**Status: Done (user playtest 2026-10-29, commit authorized; [note 326](documentation/326-d17am3-am4-unified-default.md); follow-ups D17AN-D17AP).** Re-test D17AF (water), D17AH (sky), D17AI
(slot-12 wall near cube, old fixture copy), D20H and D17AK against the unified
path; close what it fixes and keep only genuine effect/sky/water bugs.

### D17AN - Slot-12 water showing through the bridge deck

**Status: Done (user-confirmed 2026-10-30, [note 327](documentation/327-d17an-ao-ap-renderer-followups.md)). Related: D17AF, D17AM.**
Cause: water tiles kept painter order; a tile under the bridge sorted nearer
than the deck. Fix: with unified geometry every water tile is depth-tested at
its surface 32 units deep (the D17AF rule); a tile taken only for depth
leaves the 48 KB far reserve to near meshes, else stays native (canal).
Bridge clean in capture; contracts, 33-scene A/B, routes and timing pass.
User (Family Jewels, new UI slot 12, 22:23): the water texture clips through
the ground texture of the bridge, especially at the start of the bridge.
Private copy: `recomp/analysis/d17am3/playtest-fixture` (hashes in
`playtest-originals.sha256`). Check the D17AM4 near-tile water depth and the
native water painter order against the deck; Vanilla unchanged.

### D17AO - Slot-11 ceiling hole showing the sky

**Status: Cancelled - user decision 2026-11-03: no action required (2026-10-30: authored level gap, acknowledged, deferred) (diagnosed 2026-10-30, [note 327](documentation/327-d17an-ao-ap-renderer-followups.md)). Related: D17AM, D17AH.**
Not a renderer regression: the wedge appears in unified, earlier and native
renderers and in the pre-afd4ad6 build. A trace shows an authored gap: two
room meshes (`0x80192f94`, `0x801bb994`) leave a lens-shaped strip open in
the ceiling and the sky dome shows through. The original camera evidently
never sees it; the independent camera does. Options: leave as authored, a
level-data patch for this spot, or a generic gap-sealing rule.
User (Family Jewels, new UI slot 11, 22:21): a hole in the ceiling of this
area shows the sky, most visibly when jumping; never seen before. Compare
with `DNTTK_UNIFIED_GEOMETRY=0` and the original renderer (clip-rectangle
cull, far silhouettes, downgrades) before changing anything. Same private copy.

### D17AP - Slot-9 grave-dig soil corners flashing again

**Status: Done (user-confirmed 2026-10-30, [note 327](documentation/327-d17an-ao-ap-renderer-followups.md)). Related: D17X, D17AM.**
Cause: unified geometry drew a warped quad wholly on screen as one packet
with no depth plane, bypassing D17X. Fix: warped quads are drawn as their two
native halves with per-triangle plane depth. Notches gone in the pan capture.
User (Old West, new UI slot 9): the corners of the dug grave's dirt flash when
panning, as in D17X (accepted 2026-10-22), now with unified geometry. Read
D17X first; likely coplanar or contact depth between the soil and the ground.
Same private copy.

### D17AQ - Slot-10 jetpack height: level sections pop in and out

**Status: Done (user closed 2026-11-01: "close D17AQ"; B max limit, A open sky (Classic), dormant enemies, load crackle fix; [note 335](documentation/335-d17aq-draw-reach-report.md)). Dependencies: D17AM, D14F.** Opened 2026-10-30
from the user's playtest: in UI slot 10 (saved 2026-10-30 12:24) Duke hovers
high above the level with the jetpack; turning left and right makes parts of
the level appear and disappear, a little at a time. The user asks whether this
is an original PlayStation drawing limit. Measured during D23G: slot 10 holds
120 presents/s at 0.66 of the emulation thread, so it is not a performance
problem; a still from the save shows the centre drawn and both sides empty.
Report first: identify whether the game's room/portal visibility, its view
cone or the widescreen portal bounds decide what is skipped at this height,
compare Vanilla 4:3 and Modernized FOV 60/90 from the same private save, and
say what a game-wide fix would involve before implementing anything.
Acceptance (report): cause identified with evidence and a proposed fix the
user approves.

**User follow-up (2026-10-30).** Slot 10 still shows the same reach. Because
Modernized now shows so much more, enemies visibly come into existence only as
Duke gets close (Necros on the long moat stretch). From one end of the long
moat (UI slot 2) the far end is completely black: the level does not draw it.
Include in the report: the far-end cutoff in slot 2 (draw-distance limit,
room/portal visibility or fog), and the enemy activation/spawn distance versus
the now-visible range, with what extending each would cost (frame rate
included, given D23G).

**Work log 2026-10-30 - report ([note 335](documentation/335-d17aq-draw-reach-report.md)).**
Private copies of UI slots 10 and 2 (both level 6), offscreen, player files
hash-checked. Slot 10: Duke hovers at Y -1898 in sky room 1, above the moat
walls (tops near Y 1000). The original portal walk lists neighbours only
through portals and clips them to the portal rectangles, so sky rooms seen
over the walls are never drawn and portal rooms are cut mid-screen and drop
out as Duke turns. Same at FOV 60, 90 and 4:3; Vanilla reaches the same
height and shows the same void. A removed developer experiment listing every
sky room made the view whole (thread 0.66 to 0.82 at 120 Hz, ring peak 64540
to 115872 of 139744); the remaining black is authored void. Slot 2: the moat
end is 50000-63000 away, past the Extended limit 51104; at the GTE maximum
65535 it appears, cost within noise. Enemies: the spawn/despawn radius is the
original mesh far limit (25552 in level 6, set at load at `0x800da474`; cap
12 live of 51 spawners), so they spawn in view under Extended draw distance.
Proposed (Modernized Extended only): A render-only open-sky visibility, B
limit to 65535, C1 spawn radius to the Extended limit with despawn
hysteresis (gameplay change) or C2 fade-in. No player code changed; the
player executable is byte-identical. Next: user picks which fixes to build.

**Work log 2026-10-30 - B, A, C1 implemented (Needs playtest).** User: "proceed
with your exact recommendation ... open her up". Modernized Extended only:
B raises the room/mesh limit to 0xffff in every level; A opens sky rooms to
the whole view (or appends them) while the camera is in a sky room and Duke
can see them over the camera room's walls (mesh-piece bounds sight test);
C1 adds entry hook `0x80099164` (regenerated, codegen hash unchanged) so
enemies spawn out to the extended limit and are removed 4096 beyond it, with
exact distance and no game variable written. Native controls (117 PASS, new
D17AQ groups), near, input and profile tests pass; 12-slot sweep shows no
lost geometry; Vanilla unchanged. Cost at 120 Hz/150%: slot 10 hover 30 to 22
game frames/s (open sky shows 4-8 more rooms), slot 2 ground 30.2 to about 28;
presents stay 120. Level cap (12 in level 6) now fills at once with distant
enemies. Playtest: slots 10 and 2, outdoor levels, combat feel.

**Work log 2026-10-30 - C1 withdrawn; dormant enemies (Needs playtest).** User:
C1 changed the flow; wants Duke 3D behavior (present, no reaction until near
or shot) and reported a very slow slot-12 load. C1 removed. New
`enemy_reach.inc` (hooks `0x80099164`, `0x80099530`, `0x80025944`,
`0x800632b8`, `0x8006cefc`): spawn to the extended limit and remove 4096
beyond it with the original cap and replacement; beyond the original radius a
spawned enemy's class controller is skipped (class 0 for that call only)
until it is hurt; post-load heal from the type table. Distant enemies stayed
still in slot 2, a hurt one woke, live never exceeded the cap in 12 slots,
cost about 0.2 game frames/s. Slot 12 loads in about 21 ms here; the user's
log has no slot-12 load and host checks found no contention, so cause
unknown; asked the user how it was loaded.

**Work log 2026-10-30 - slot 10 slowdown (Needs playtest).** User corrected
the slow slot to 10. Cause: open sky at hover, 30 to 23.6 game frames/s at
the player's settings (game runs slow). Fixed in three steps: rectangles of
the seen pieces only, a null mesh to the original world routine when the host
takes every face (all scenes), and the object pass back on the portal walk's
list (new hook `0x800374cc`): 28.1 at hover, slot 2 at parity (30.1). Tests
and a 12-slot sweep pass (note 335, revision 3).

**Work log 2026-11-01 - slot 10 load crackle (Needs playtest).** User: "pay
attention to the audio crackle when you load this one". Their 17:31 session
loaded slot 10 and logged the emulation behind real time (46 frames/s) and a
step to every 2nd refresh; the host was idle (load 0.37, Timeshift's 17:00 run
had ended, no memory or I/O pressure). Reproduced offscreen at the player's
settings (180 Hz, 4x, CPU 150%, Polymorph): after the load the vista ran at
44-50 fields/s and the 180 ms audio ring ran dry (5374 dry pulls in about
1 s) inside the 3 s after a load that the governor does not judge (D17AR3
part 4); the step down came only after it. Fix (Modernized only, our code,
no framework change): `overclock_lease` reads the ring through
`psx_audio_out_stats`; once the ring has filled since the last load or
gap, a fall under half its target sheds one present step at once
(`replay_audio_low`, `audio_sheds` in `render_replay`), the settle window
included; a further step waits 1.5 s and needs a lower ring. Slot 10: 0
dry pulls in three load runs and a turning run (ring low 78-88 ms, game
back to 60). Slot 2 also sheds once after its load and avoids continuous
crackle: with shedding off it runs 41-51 fields/s at 180. The other 10
fixture slots: no shed, no dry pull, about 175-184 presents/s. Vanilla
unchanged (no governor line, no dry pull). `ttk-input-test`,
`ttk-near-test`, `ttk-controls-test` (119 PASS; two stubs added) and
`test_auto_frame_rate.py` pass. Note 335 revision 4. Not addressed: the
vista still runs at 90 presents/s at 180 on this machine (D17AR5 had held
180 there; not re-measured against that build).
User follow-up: right after the load it "churns", then is fine. Measured
(three loads per session, slot 10, 180 Hz): every load draws its first 3-5
game frames on the emulation thread (12-24 ms of host world pass each, 47-113
ms frames) until the first post-load worker redraw exists; then the 180 Hz
redraw feed on the same thread keeps the vista under 60 fields/s until the
step down (about 1 s). Engaging redraw-only after 2 instead of 8 redraws
changed nothing measurable (the first redraw is what is waited for) and was
reverted. Moving the redraw feed off the emulation thread is the change that
would remove both; not started, awaiting the user's choice.

**Work log 2026-11-01 - crackle check revised (Needs playtest).** User:
crackle unchanged. Their 18:22 log: the slot-2 load stepped down only by the
old slow-second rule at 35.8 frames/s, no `audio running low`. Cause: the
check armed only once the ring had refilled to 90% after the load, and a
start that slow never refills it. Now it also arms 0.5 s after a load or
gap, sheds under a third of the target (60 ms; UI slot 5 dipped to 94 ms
without running dry), and a second step waits 0.75 s and needs the ring
still falling. Emulating the slower start (CPU 200%): dry pulls about 7800
to 0-1255 per load, two steps. Player settings: slot 10 0 dry pulls in 3 of
4 loads (566 in one); 15-load sweep of all fixture slots, others 0; UI slot
5 stepped once at 45 ms (near-dry). Native tests (119 PASS) and
`test_auto_frame_rate.py` pass. The remaining slow first second needs
D17AW.
User retest (18:51 log, video): crackle "not perfect, it might be a tiny bit
better". The log shows the revised check acting (shed at 29 ms after the
first load, 35 ms after the second); a 180 probe failed and the second load
went to every 3rd refresh (72 presents/s over the minute). The session was
being screen-recorded, which also loads the machine. Distant black areas
in the same video are recorded as D17AX.

**2026-11-01 user report (during the D17AX revision playtest, slot 10).**
"no slowdowns, no crackle, its stable". The Revision 4b crackle check was in
that build and is committed with D17AX. Status left as Needs playtest for
the user to close.

**2026-11-01 closed.** After D17AW (render thread) was accepted, the user:
"close D17AQ". The user reported no crackle on the D17AX and D17AW builds;
their D17AW session log shows one audio-low step at the load and 180
presents/s afterwards. D17AQ is Done. Retained: the slow first second after
loading the slot 10 vista (both builds about 45 fields/s, D17AW note 353).

### D17AR - Host-owned world visibility (parent)

**Status: Done (D17AR1-AR4 done 2026-11-01). Dependencies: D17AM, D17AQ.** Goal: the host owns world
visibility. Each level's room geometry is read from the game's own data once,
kept host-side in world space, and drawn by frustum, distance and depth from
the real camera every frame, independent of the portal walk and its
rectangles. The game keeps all simulation: Duke, enemies, props, doors,
pickups, effects. Expected to remove portal pop-in and edge cut-offs in every
level, and to recover the guest cost of the original room walk. Vanilla
unchanged. Done when D17AR1-AR4 are done.

#### D17AR1 - Inventory, data-format verification and design (report first)

**Status: Done (user: "the design is approved", 2026-10-30; [note 336](documentation/336-d17ar1-host-visibility-design.md)).**
Work log 2026-10-30: verified on slot 2 (level 6) that world vertices are
room min + (1024*b0, 128*b1, 1024*b2) and the room transform is
R * (room min - eye) (134 traced meshes, max error 2.8); the 120 Hz redraws
re-run the composition through the same hooks. Design: host world cache and
pass reusing the D17AM emission with a synthesized GTE state, guard-banded
frustum, room-graph and over-wall selection, props/objects fed the host list,
game room walk emptied. Five proofs gate D17AR2. Approved; D17AR2 starts in a
fresh session. D17AS (water triangle) deferred by the user.
Original scope: Verify against the live game: room/piece/mesh
formats and the exact room transform, per-room and per-vertex lighting and
colour tables, fog, texture and animated-texture lookup, transparency and
water, special pieces (sky rooms, mirrors, moving or hidden geometry, secret
areas, level overlays), what the room walk does besides drawing (state the
props/actors and object passes depend on), late-camera redraw interaction,
and the cost model (host draw vs the guest room walk). Present the design and
the stages for approval before implementation.

#### D17AR2 - Implementation behind a developer switch

**Status: Done (closed 2026-11-03; visual playtest passed 2026-10-30; performance carried to D17AR3; [note 337](documentation/337-d17ar2-host-visibility.md)).**
Work log 2026-10-30: host world pass behind `DNTTK_HOST_VISIBILITY=1`
(default off; developer console `hostvis on|off`). At the room walk the host
selects rooms (host portal walk with a 15% guard band, sky rooms over the
walls, plus every game-listed room) and pieces, and draws them through the
D17AM emission with the walk's GTE/context state; the game's walk keeps only
pieces the frame budget sends back; props get the host list, the object pass
the game's. Proofs: 84 matched views over all 21 selectable levels, 81 within
animation noise, the rest an explosion and newly drawn far geometry; the slot-10
jetpack pop-in and the castle cut-off are filled. No world piece data changes
in any level; single walk/camera-render caller on the disc; no refusals,
props list at most 15. Fixed during the sweep: portal corners are a 2x2 grid
(level 21 lost a room). Cost: about 4-5% fewer game frames/s at 120 Hz in the
moat views (drawing about 2.6 ms a frame; the expected guest saving was mostly
already taken by D17AQ). Level 28: draw-distance limits refused, host pass
never engages. Remaining for D17AR3: performance, level 28, FOV/aspect/view
modes and routes.
Work log 2026-10-30 (revision 2, after the user's review and live session):
far-end holes were distant pieces the frame budget sent back to the original
renderer (drawn black; 24-37% of host pieces). Host pieces now draw nearest
first and leave only a measured props/actors share; host frames drop the far
reserve for props; the rectangle test is on for host pieces; the host packet
arena is two 256 KB regions (about 350 KB a frame, same allocation, savestate
mirror tagged for both layouts). Pieces handed back 0 at both fixtures, no
prop fallbacks, 84-view sweep clean, Bio Mask and savestates checked. 120 Hz:
slot 10 29.35 off / 28.35 on, slot 2 29.2 / 29.25 game frames/s (28.0 and
28.5 before the job).
Work log 2026-10-30 (revision 3): user playtest saw no change. At `auto`
(120 Hz) most frames are redraw-worker frames, and `hostvis on` reached only
the main process (the launch had no environment switch). The switch now rides
in RenderState to the workers; 120 Hz console-toggle comparisons show the
moat's far end filled and the slot-10 castle wall continuous.
Playtest 2026-10-30 (revision 3, `hostvis on` reaching all four processes):
user: "Visually, fantastic. performance-wise, it needs a lot of work to make
this silky smooth! ... nothing is popping in and out of existence anymore, so
this is certainly the right direction." Counters: about 312000 host pieces,
none handed back, no fallbacks; 75-90 presents/s most minutes with brief
emulation-behind drops (also with the switch off; main thread at 100% of a
core, no host contention). Open: smoothness (D17AR3); the user's launch with
`DNTTK_HOST_VISIBILITY=1` did not reach the game's environment (to check).

#### D17AR3 - Regression sweep, performance and default switch

**Status: Done (closed 2026-11-03; parts 1-2 accepted 2026-10-30; part 3, default
switch, level 28, regression sweep and its revision 2, accepted 2026-10-31;
part 4, AUTO at the display's full refresh, accepted 2026-10-31, [note 341](documentation/341-d17ar3-auto-full-refresh.md);
renderer named Polymorph; [note 338](documentation/338-d17ar3-host-visibility-performance.md),
[note 339](documentation/339-d17ar3-redraw-only-live-frames.md),
[note 340](documentation/340-d17ar3-host-visibility-default.md)).**
User priority after the D17AR2 playtest: make host
visibility silky smooth at 120 Hz (slot 10 and the moat dip to every second
refresh). Also: level 28 never engages; check why the user's
`DNTTK_HOST_VISIBILITY=1` launch did not reach the game's environment. All private fixtures and level-select levels, static/walk/
turn/jetpack routes, FOV 60/90/120, 4:3 and 16:9, 60 and 120 Hz.
Work log 2026-10-30 (part 1, performance): at slot 10 most redraws were
dropped (their GP0 record overflowed 192K words: 8% while turning, all of them
at the loaded view, where the screen repeated the live 30 fps image). Redraw
buffer 1M words; precise vertices in the GPU DMA aperture indexed directly
instead of a probed hash (the emulation thread's largest single cost); world
coplanar pairs cached per mesh (host piece 48 to 30 us); no unused mesh copy;
cached environment switches. Checks: coplanar cache 0 differences in 67,735
meshes, direct table identical over 46 million lookups, D17AR2 84-view sweep
unchanged, `ttk-near-test` passes, patch re-exported. Same conditions against
the accepted D17AR2 build (120 Hz, scale 4, turning, seconds at full rate of
24): slot 10 host on 10 to 21 (failed redraws 574 to 0, present p95 15.5 to
9.2 ms), slot 2 host on 3 to 21, host off 20 to 24 and 12 to 20. Standing in
the heaviest views (slot 10 loaded view, parts of slot 2) still falls to every
second refresh: the emulation thread needs 110-128% of a core there. A
throwaway experiment that skipped the live frame's host world and GPU drawing
(at 120 Hz every present is a redraw) reached full rate there (slot 10 60.0
fields/s from about 47); making that real is a design change awaiting the
user's decision. `run.py` passes `DNTTK_HOST_VISIBILITY` through (all four
processes log on); the user's earlier launch could not be reproduced.
Playtest 2026-10-30 (part 1): "it's feeling really good. quality has increased
by this massive performance hike. especially on my absurd state save on slot
2. that's actually really smooth now." Part 1 accepted and committed. Next
(part 2): redraw-only live frames (note 338, last section), then "let's really
see how far we can push this renderer". The user also wants the renderer
named: the in-game renderer choice will be Classic and our renderer (name to
come from the user; EDuke32's are classic, Polymost and Polymer).
Work log 2026-10-30 (part 2, redraw-only live frames, Needs playtest): at 120
Hz an engaged live gameplay frame keeps its host world selection but emits no
pieces, and the GPU DMA lists it sends after its flip drop their drawing
primitives (state, uploads, copies and DMA timing unchanged); the presenter
never shows that image (keeps the screen; thumbnails from the last redraw).
Engaged after 8 redraw presents in a row, only in gameplay with the late
camera and live workers; 3 misses in a row, a failed redraw or a savestate
load draw again; never at 60 Hz, Vanilla, menus, pause, loading, split screen.
TTK sends each frame as about 9 lists chained after the flip, so the flip is
the frame boundary. `DNTTK_REDRAW_ONLY=0` / console `redrawonly off`
(developer); `PSX_LIVE_SKIP_CANARY=1` check. Canary over all 10 fixture slots,
pause, thumbnail, save/load: no stale pixel shown; an early garbled-texture
frame after loads (re-presenting a cache entry) fixed by not swapping. Same
build off/on, 120 Hz, scale 4, seconds at full rate of 24, host visibility on:
slot 10 turning 20 to 24, standing at the loaded view 1 to 24 (p95 26.7 to 8.8
ms), slot 2 turning 11 to 24, standing 20 to 24; host off slot 2 20 to 24.
Forced full rate: slot 10 loaded view 47.7 to 60.0 fields/s. `ttk-near-test`
passes; patch re-exported and verified. Not automated: pause resume,
inventory, level changes, split screen (playtest).
Playtest 2026-10-30 (part 2): "it feels unbelievably smooth." Part 2 accepted
and committed. D17AR3 continues: host visibility default switch, level 28, the
full regression sweep, then pushing the renderer further and naming it.
Work log 2026-10-30 (part 3, Needs playtest): host world visibility is on by
default in Modernized (Extended draw distance, unified geometry, near path
live); `DNTTK_HOST_VISIBILITY=0` (developer) turns it off, console `hostvis`
still overrides. Level 28: its own alternate room limit is 69511, past
0xffff, and the draw-distance check refused it every frame; now accepted and
kept as the game sets it, so the extended limits and the host pass run there
(about 61 host pieces a frame, none handed back). Sweep: 21 levels x 5 views x
5 configurations (FOV 60/90/120, 4:3 and 16:9, first and third person, 60 Hz
matched host off/on/off): engaged in all 525 views, no refusals, no pieces
handed back, no piece data changes; the 8 views above noise are gameplay
events and animation, checked by eye. Fixture routes (10 slots, still/turn/
walk, scale 4, default build, no environment): 120 Hz 180 of 180 seconds at
full rate (same as host off), 60 Hz all engaged, 120 Hz FOV 120 4:3 180 of
180 (one route without host frames: Duke killed, death camera at 4:3 uses the
game's walk); redraw-only engaged in every 120 Hz route. Vanilla: no
draw-distance or host frames. `ttk-near-test`, `ttk-input-test` pass; no
framework changes. Seen, not visibility: level 28 sky drawn as a box with
black around it in some views (host off the same).
Playtest 2026-10-31 (part 3): performance and visuals "looking great", with
regressions: muzzle-flash light stuck on the ground (slot 12), the slot-4
club garbled, slow and then halted (GP0 0x1F). Work log 2026-10-31 (revision
2, Needs playtest): the club failure dates from D17AR2's two-region packet
arena (same with every switch off; fine before D17AR2): in the club three
passes a frame each started a new region, so the frame overwrote its own
packets. Now the first reset after a flip picks the frame's region and later
ones append (carried to the workers). The stuck light: a redraw-only live
frame drew no pieces, so the renderer's vertex light-mark clear never ran in
live RAM; it now applies that clear to its selected pieces. Club at 120 Hz
scale 4: 16 of 16 seconds at full rate, drawn correctly; light clears after
firing. Canary, level sweep, 120 Hz routes and `ttk-near-test` clean. Note
340.
Playtest 2026-10-31 (part 3, revision 2): accepted. User: "there's no issue
there with the club ... no problems there with the muzzle flash either";
the club back room "this rendition is perfect"; slot 10 at jetpack height
"there is not one single indication of slow down ... we've really really
succeeded"; "I'm extremely happy with the work and I accept this". D17AR3
continues: pushing the renderer further and naming it.
Closeout 2026-10-31: part 3 committed and pushed (`49085e8`). The user named
the renderer **Polymorph** ("we're morphing the locked in n-space engine now
to something far beyond it's 1998 bounds") and chose full 180 Hz as the push,
then gave AUTO to the agent: "I'm allowing you to now own this. I want the
best compatibility for the widest array of monitors."
Work log 2026-10-31 (part 4, accepted the same day: "this is great work. fully
accepted"): the player's 180 Hz display
had AUTO at 90 (D17S `refresh / ceil(refresh / 120)`). AUTO now gives every
refresh up to 240 Hz its own picture (an even fraction above: 360 -> 180);
the existing governor drops evenly to every 2nd or 3rd refresh where a scene
cannot keep up. Savestate loads no longer count as emulation behind for 3 s;
after stepping down, the governor retries the faster rate only when the
redraws are clearly cheaper (under 75% of the cost at the step down) or after
30 clean seconds, and a failed retry returns after one slow second. Results
(fixture routes, scale 4): final build 120 Hz 120 of 120 route seconds at full rate, 180 Hz 115 of 120 (only slot 10's vista); 144 and 165 full everywhere but slot 10's jetpack
vista standing; 180 full except that vista (steps to 90, sometimes 60);
240 full in 7 of 10 slots, the rest stepping to about 120. Cause in the vista
(perf): drawing finished worker redraws into the GL renderer takes 46% of the
main thread (3.5 ms a redraw), mostly per-primitive batcher work; workers,
scale and GPU are not limits. That is D17AR5. Tests: auto-rate, profile,
`ttk-near-test`, `ttk-input-test`, `ttk-controls-test` pass; patch
re-exported and verified. Note 341.

#### D17AR4 - Retire superseded portal patches

**Status: Done (user-accepted 2026-11-01; closeout authorized; [note 344](documentation/344-d17ar4-portal-patches.md)).** D17AQ open sky, D17P/D17Y portal widening and similar
rules, once host visibility covers them. User decision 2026-11-01: keep them
for the Classic renderer; Polymorph must not depend on them.
Work log 2026-11-01 (Needs playtest): the three rules already ran only on
frames the host pass did not draw, but Polymorph's over-the-walls selection
read the Classic switch `DNTTK_OPEN_SKY`, and Polymorph did not cover D17Y:
third person in the slot-8 castle jump (wall view), room 22's rectangle top
fell to -33 and the tower tops showed sky, as in Classic with the rule off.
Now one predicate (`upper_edge_open`) serves Classic's rule and the host
portal walk, which gives that direct neighbour the camera room's top;
`DNTTK_OPEN_SKY` is Classic-only; Classic code is labelled. Developer audit
`DNTTK_HOST_VIS_AUDIT=1` compares Classic's rectangles with the host's each
frame: open tops past the host 35 of 447 before, 0 after; only the 2-pixel
margin remains (exact face tests make it unnecessary; subway clean).
Matched routes (Polymorph / Classic / Classic rules off): jetpack moat castle
whole / whole / cut; subway clean / clean / dark outline; castle tower tops
drawn / drawn / sky; also at 120 Hz. 21-level sweep: no refusals or pieces
handed back, 83 of 84 views within noise, one an enemy's muzzle flash.
`ttk-near-test`, `ttk-input-test` pass; `ttk-controls-test` stops at the
existing `movement_ready()` assertion. No framework changes. Note 344.
Playtest 2026-11-01 (slot 8 replaced at the user's request with the D17Y
castle copy; their push/pull save backed up under `recomp/saves/backups/`):
third-person castle jump in Polymorph and Classic, logs clean. User: "ready
to accept this, and then you can close us out". Committed and pushed.

#### D17AR5 - Polymorph: cheaper redraw feed for 180 Hz in the heaviest views

**Status: Done (user-accepted 2026-10-31; closeout authorized).** Slot 10's jetpack vista falls to half the refresh at 180
Hz: each finished worker redraw costs the main thread about 3.5 ms to draw
into the GL renderer (46% of the thread; textured quad path, command
dispatch, batch flushes, widescreen helpers; note 341 profile). Goal: about a
third cheaper, or the feed off the emulation thread, so the vista holds 180
with game speed at 60. Measure with `recomp/analysis/d17ar3p5/` probes.
Work log 2026-10-31 (Needs playtest): the feed had no single hot spot; it
repeated small costs for about 46,000 GP0 words and 7,600 triangles a redraw.
Exact framework changes (same pictures, less work): recorded GP0 runs go in as
whole commands instead of word by word; display info, the native-4:3
classifier and the sprite-tag lookup are answered once while their inputs
cannot change; the widescreen census ring records live primitives only;
textured vertices are 56 bytes instead of 88 (small integers as
non-normalized integers, shader unchanged); a redraw session saves the 1 MB
CPU VRAM array only when something writes it. Slot 10's vista standing at 180
(scale 4, shedding off, three alternating pairs): main thread per redraw 4.0
-> 3.07 ms (24% lower; feed alone 20%), game 43.5 -> 59.1 fields/s, 158 ->
179 presents/s. Default governor: two long holds kept 180 every interval
(75 s, 45 s) at game 60; the governed route held 180 standing and turning in
4 of 5 runs (one stepped to 90 at 57.0 frames/s just after the load). Margin
over the governor's 57 line is about 1-3 frames/s with the desktop busy.
Route sweep: 120 Hz 120 of 120 seconds full; 180 Hz 111 of 120 (part 4 build
the same evening: 92 of 120). Vanilla renders normally. `ttk-near-test`,
`ttk-input-test`, `test_auto_frame_rate.py` pass; `ttk-controls-test` gets
its missing `live_frame_redraw_only` stub and then stops at an existing
`movement_ready()` assertion that the part 4 executable fails too. Patch
re-exported and verified. Not moved off the emulation thread. Note 342.
Playtest 2026-10-31: accepted. User: "fully accepted/ lock it in, close it
out". Committed and pushed.

#### D17AR6 - Renderer choice in OPTIONS: Classic and Polymorph

**Status: Done (user-accepted 2026-11-01; closeout authorized).** The user named our renderer Polymorph (2026-10-31). Offer
Classic (the original renderer) and Polymorph in OPTIONS > DISPLAY and the
launcher, mapping to the existing switches (host visibility, unified
geometry, near path, redraw-only); Vanilla stays Classic.
Work log 2026-10-31 (Needs playtest): OPTIONS > DISPLAY has RENDERER:
POLYMORPH / CLASSIC (next launch); the launcher has `--world-renderer
polymorph|classic` (profile field `world_renderer`, default polymorph).
Classic turns off unified geometry, host visibility and redraw-only (developer
values still win). The near path stays on: with it off, first person broke
(D36 arms changed, close walls vanished in slot 10), while with it on Classic
first person matched Polymorph in nine views. Vanilla sets none of the
switches and is unchanged. The ADVANCED OpenGL / software row is renamed
BACKEND. Real-key round trip (confirm CLASSIC, quit, relaunch): host frames 0,
unified off, redraw-only off, near on; back to Polymorph with the flag. Profile
tests (112), `ttk-near-test`, `ttk-input-test`, `test_auto_frame_rate` pass.
No framework changes. Not live (a later option). Note 343.
Accepted 2026-11-01. User: "lock it in, close us out". Committed and pushed.

### D17AS - Slot-2 moat: large dark triangle in or under the water

**Status: Done - closed as original level design (user decision 2026-11-01: option 1, leave as authored; no fix planned; [note 347](documentation/347-d17as-moat-stray-triangle.md)). Dependencies: D17AM.**
Work log 2026-11-01: reproduced from the clip's route (wade forward, dive,
turn) on a private fixture copy. Identical with the default, earlier,
native (`DNTTK_NEAR_CLIP=0`), host-visibility-off and water switches off, so
not a renderer regression. Both symptoms are one face, `0x8018dd5c` of room
mesh `0x8018dad0`, drawn by the game's room walk: a lone triangle hanging
from the water surface on the channel's west wall to 2048 units deep across
the channel, in the plane where this room meets the next. It shares its deep
corner with no other face, and the next room continues the channel through
that plane, so it closes nothing. The user's theory holds; same class as
D17AO. Recommendation: leave as authored (or one small level-data job for
D17AO and D17AS together). No code changed.
User decision 2026-11-01: option 1. The triangle is part of the original
level design; it stays as authored and the job is closed with no fix planned.
Original report: User clip
`research/screencaps/triangle.mp4` (17.9 s, 60 fps, 1456x820): around 1.5 s
a large sharp-edged dark triangle covers the moat floor while Duke stands in
the water, and around 7-9 s a dark vertical wedge crosses the centre of the
view underwater. Not reproduced by a scripted look-down/turn route from slot
2 (the dark region there was close textured rock, identical with D17AQ off).
Not known whether it predates D17AQ. Next: reproduce from the clip's route
(wade into deeper water and dive), compare with D17AQ off and with
`DNTTK_UNIFIED_GEOMETRY=0`, and trace the triangle's source.

### D17AT - Ground decals and small props stable on any surface

**Status: Done (user-accepted 2026-11-01: "this is 100% acceptable ... accept this job as closed"). Design approved by the user 2026-11-01 ("i approve the
recommended design of the proposed fix in 3 stages"; water option A)
([note 348](documentation/348-d17at-decal-stability-report.md)). Dependencies: D17AM, D17K.** User: the shells
on the ground in slot 12 (file 11, medieval canal) clip in and out as the
water texture animates; ground blood in slot 11 (file 10) pops in and out
when walking left and right over it; the blood in slot 7 (file 6, Family
Jewels castle tower, across the jump) flickers. "I want our renderer to
absolutely own that ... to never have this problem whatsoever." D17K fixed one
case (floor ordering key); this is the general rule. Find for each case which
path draws the decal or prop (game OT, host world pass, scene depth), where
it is coplanar or nearly so with the surface under it, and what decides the
order frame to frame (animated water texture swaps, host depth, OT slot). Then
design one host rule that keeps a decal or small prop lying on a surface
drawn over it at every distance, angle and frame rate, in every level, without
showing it through walls. Private copies of the three saves; verify at 60 and
120 Hz.

**2026-11-01 report (note 348).** All three reproduced offscreen on private
copies. Blood is a per-frame decal queue (`0x8009b7fc` queues,
`0x8009b1c0` draws) that the host never took over: native painter-only
packets over host floors with depth, so a floor polygon the original sorts
nearer cuts the blood (also with the game's own packets: not a Polymorph
regression). Slot-12 shells: the shell-box prop lies 15 above to 140 below
the water plane; host water writes depth (D17AN, 32-unit push) on its
non-transparent texels, so the cut follows the animated texture; with
`DNTTK_WATER_DEPTH=0` the boxes are whole. Proposed rule: decals as host
geometry with plane depth and a distance-scaled contact allowance (D17AT2);
water tested but not written via a small framework depth mode (D17AT3);
sweep (D17AT4). No code changed.

**2026-11-01 implementation (Needs playtest; note 348).** Correction: the
blood is not the sprite queue but small flat meshes from the small-model
renderer `0x8002e02c`. D17AT2: new hooks `0x8002D848`/`0x8002BD48`
(regenerated C, codegen hash unchanged); each model is re-projected and
checked against the game's records, near flags cleared, and every face is
drawn as a host prop polygon (plane depth, two-unit contact allowance, game
slot and material) with its native packet emptied (`model_depth.inc`,
`DNTTK_MODEL_DEPTH=0` to compare). D17AT3: framework
`psx_mod_gpu_host_vertex_test_only` (patch exported, verified on a clean pin);
water tiles test depth without writing it (`DNTTK_WATER_WRITE=1` to compare).
Offscreen: slot-12 shell boxes whole in every frame, slot-11 pool identical
frame to frame (it now ends at a 128-unit step it used to spread over), slot-7
ledge blood host-drawn; moat, bridge and grating regressions clean; 120 Hz no
replay failures; no measurable cost at 180; Vanilla untouched;
`ttk-near-test` PASS with new contracts. Python suite has one pre-existing
failure (`test_primitive_trace`, same at HEAD). Player check at 180 Hz needed.

**2026-11-01 follow-up D17AT5 (Needs playtest; note 348).** User: the
shimmer is only where blood blobs overlap. Measured z-fighting between blobs of
one pool (depth winner changed 5x more often than edge motion explains). Flat
small models are now decals: depth-only pass 2 units up (new framework mode
`psx_mod_gpu_host_vertex_depth_only`) and a test-only colour pass 6 units up,
so blobs keep the game's painter order among themselves while floors and
nearer walls still resolve by depth. No measurable cost at 180. Blob parts
that run under a wall stay cut (deliberate). Compare: `DNTTK_DECAL_LIFT=0`.

**2026-11-01 follow-up D17AT6 (Needs playtest; note 348).** User: blood
disappears at the sides of the view when turning. Cause: the blood-object
draw (`0x800a2690`) tests the room bitset the game's portal walk fills, while
Polymorph shows more rooms than that walk lists; a pool in such a room was
skipped whole (434-840 draws per turning route on the user's save). Fix: a new
hook sets the room bit for that one object's test when the host showed the
room, cleared before anything else runs. Decal offsets are now depth scales
(robust where a shifted plane fell back). The game's screen cull was ruled
out (redraws use the late camera) and left as is. No measurable cost at 180.

**2026-11-01 follow-up D17AT7-9 (Needs playtest; note 348).** From the
user's clip `bloody.mp4`: blood vanishing at the view edges came from two game
culls sized for the 4:3 view (a side cull at about half-width 341 against 480,
and an oversize-face cull of 120000 in the facing test), now bypassed for
live blood models with the host clipping and testing facing in view space;
the remaining shimmer came from blob order following view depth, which
changes while turning; decals now sort by straight-line distance, so turning
never reorders them (measured winner changes at fixed points: 1631 to 831,
with 802 explained by edge motion alone). Compare: `DNTTK_MODEL_SIDE_CULL=1`,
`DNTTK_DECAL_SLOT=0`.

**2026-11-01 follow-up D17AT10 (Needs playtest; note 348).** User clip
`Video_2026-11-01_12-31-22.mp4`: edges fixed, harsh flicker while turning.
Single-image dropouts of wall splats that sit up to ~2 units inside their
wall, introduced by D17AT9's distance slots (the wall could be drawn after
the splat). The decal depth pass now writes 4 units up (was 2), so the order
no longer matters; no dropouts in 15 fast-turn runs, blob order still stable.

**2026-11-01 follow-up D17AT11 (Needs playtest; note 348).** User clip
`Video_2026-11-01_12-57-03.mp4`: "significantly more stable ... nothing is
disappearing"; minimal glitching left where splats meet surfaces. Each blood
model's translation was truncated to whole units by the game, so its plane
jittered by up to a unit per frame; the host now places blood models with
the exact translation (object and camera positions through the camera
matrix). No dropouts, order stable, no measurable cost.

**2026-11-01 follow-up D17AT12 (Needs playtest; note 348).** User: blobs
still swap which is on top while panning (all three saves). Blobs crossed
distance-slot boundaries as the eye moved (about 7% of comparisons). All
blood decals now share one ordering slot, so their order is always the
game's object order. A ranked depth rule was tried and left off.

**2026-11-01 accepted and closed.** User: "I want to now accept this this is
100% acceptable ... it's not perfect however ... accept this job as closed";
"slot 7 does not exhibit any issues anymore however in slot 11 and 12 you can
see a very very slight shimmering"; "this is literally the best it's ever
looked it looks better than the base game". Remaining shimmer moved to D17AU.

### D17AU - Polymorph decals: no shimmer at all

**Status: Cancelled (user satisfied, 2026-11-01). Dependencies: D17AT.** User, accepting D17AT (2026-11-01):
"the next job is to aim for perfection with no shimmering whatsoever ... push
polymorph renderer to the next level to allow for blood splatters to look
robust." Reference clip `research/screencaps/Video_2026-11-01_14-22-44.mp4`:
slot 7 (UI) clean; slots 11 and 12 show a very slight shimmer of the pools
while panning. Start from note 348 (what D17AT learned: the small-model
renderer `0x8002e02c`, decal passes, exact placement, slots, the measures
that could and could not see the problem). Candidate directions, report
first: measure the residual per pixel with a deterministic camera path in
the redraw workers rather than video; separate texture crawl (texel
aliasing in motion, every renderer) from blob order changes; consider a
true decal composite (blobs of one surface resolved together in a stable
order, for example by object id in a stencil or a per-surface layer) instead
of depth offsets; and texture filtering or supersampling for blood only, if
the residual is aliasing. Classic and Vanilla unchanged; verify at 180 Hz.

**2026-11-01 report (In progress, awaiting choice; note 349).** Measured on
a fixed redraw yaw path (new developer switch `DNTTK_LATE_TEST_YAW_FILE`;
runs repeat to the pixel), scale 4, 180 Hz. Three parts: (1) decal corners
still use the game's GTE screen positions (whole-unit truncation), so blobs
slide against each other by up to 0.1 native px per image; developer switch
`DNTTK_DECAL_EXACT=1` projects them exactly and cuts pan-induced pops by a
third to a half (slot 11 close 0.09% to 0.06%, slot 12 about 0.26% to
0.12%); (2) slot 12's pool lies under the canal's animated translucent
water, drawn the same in Vanilla: authored, most of the change seen there;
(3) the rest is aliased speckle edges (cutout texels), which only
coverage-based edges can remove. Proposed: D17AU2 exact corners by default;
D17AU3 anti-aliased decal edges with a decal composite (framework, design
report first). Nothing changes in play until a stage is chosen.

**2026-11-01 cancelled.** User, after playing: "lets shelve this. it looks so
good right now ... we have bigger fish to fry ... im genuinely so happy with
how the blood looks now, its better than ever". D17AU2 and D17AU3 are not
built. The report (note 349) and both developer switches stay, off by
default (`DNTTK_DECAL_EXACT`, `DNTTK_LATE_TEST_YAW_FILE`), for reference if
the job is ever reopened. Nothing changed in play.

### D17AV - Polymorph: flickering triangle on corridor walls (slot 8)

**Status: Done (user-accepted 2026-11-01: "it is now fixed! great work. superb"; [note 351](documentation/351-d17av-coplanar-wall-flicker.md)). Dependencies: D17AM, D17W.** User (2026-11-01): in a
new state in UI slot 8 (file 7), looking left and right shows a flickering
triangle on a section of corridor wall. "I don't want this to happen in our
renderer whatsoever." Find the cause, fix it in Polymorph and check for other
places in the game with the same problem. Classic is frozen: from now on
renderer work targets Polymorph only (user, 2026-11-01). Vanilla unchanged.

**2026-11-01 implementation (Needs playtest; note 351).** Each corridor wall
panel (level 6, room 22) is several faces in the same place, quads and
triangles with different textures. Polymorph gives such faces a small depth
step by rank (D17W/D17AM2), and three faults let them fight or swap: a
triangle stored as a quad with a repeated corner got no plane and was never
ranked (the left panel's stripes); counted ranks could tie along a chain; and
when a triangle and the quad it covers tied in the original's farthest-corner
order, record order chose, so turning made the triangles come and go (the
right panel; Classic does the same). Fixed in `near_clip.cpp`: repeated
corners dropped before the plane is made, rank = one more than the highest
face over it, and on a tie the face built from the other's corners wins.
Traced 56-step pan: each panel keeps one winner in every frame (93 and 37
traced frames). Static sweep of every selectable level: repeated-corner pairs
only on level 6 (5); tie-prone chains in 20 meshes; 144 subset pairs with
different textures on levels 1, 2, 3, 5, 6, 7, 8, 9, 11 and 25, all covered by
the same rules. `ttk-near-test` and `ttk-controls-test` pass. Not seen: the
other levels' places in game, 120/180 Hz, third person.

**2026-11-01 follow-up (Needs playtest; note 351).** User: "the issue is
still present but it's not as flickery" (`Video_2026-11-01_16-30-27.mp4`).
The order now held; the remaining triangle is a shading break: one corner of
each panel is lit, and the GPU shades a quad as two triangles on a diagonal,
so the light stops at a hard edge (as in the original). The panels' extra
faces are exact duplicates of the quad's texture map, and the left panel's
front is two triangles plus a back-side triangle showing through (repeated-
corner quads pass the cull from both sides). Polymorph now shades such quads
smoothly (four triangles around a centre with the mean colour, only where
one texture map covers the face), joins flat triangle pairs into such quads,
keeps the whole face over exact duplicates, and culls repeated-corner quads
as their triangle. Slot 8: both panels one even wall. A/B on slot 8 and
levels 1 and 5: no texture or edge changes. Cost at 180 Hz: worker job
average 9.0 to 9.5 ms, peak and cadence unchanged. Developer
`DNTTK_SMOOTH_QUADS=0`, console `smoothquads`.

**2026-11-01 accepted and closed.** User: "it is now fixed! great work.
superb", then authorized the closeout. D17AV is Done. Retained limits: a
possible cross-room ceiling hairline seen briefly after turns (not tied to
this change), other levels' lit quads not visited one by one, not measured
on the player's 180 Hz display.

### D17AW - Polymorph: present redraws from a host render thread

**Status: Done (user-accepted 2026-11-01: "it plays beautifully ... fully accept"; [note 353](documentation/353-d17aw-render-thread-report.md)). Dependencies: D17AR5, D17AQ.** The emulation
thread also feeds every finished worker redraw into the GL renderer (about
3 ms each, 46% of the thread at 180 Hz in slot 10; GPU about 7% busy). In
heavy views (slot 10 jetpack vista, slot 2 moat) that keeps the game under
60 fields/s at 180 Hz, so the governor steps to 90, and every savestate load
churns for about a second first (D17AQ work log 2026-11-01). Goal: a host
render thread owns GL and presents the redraws, so the emulation thread only
emulates. Report first: GL context ownership, VRAM and redraw-cache handoff,
presenter timing, savestate loads, the first post-load frames, menus/FMV,
failure and fallback, measured headroom. Polymorph only; Classic frozen;
Vanilla unchanged. Acceptance: slot 10 and slot 2 hold 180 at 60 fields/s,
loads without a slow second, no audio dropouts, 21-level sweep and native
tests pass, user playtest.

**2026-11-01 stage 1 report (awaiting decision; note 353).** Slot 10 vista
at 180 (shedding off): the emulation thread is at 100% and half of it (50-51%)
is the redraw feed, so the game runs 41-43 fields/s. Since D17AX the vista
redraw is about 79,500 GP0 words and 5.2 ms (D17AR5: 46,000 and 3.07 ms); the
distant pieces that used to fall back are now drawn every redraw, and the
D17AR5 margin is gone. Turning: 2.5 ms, 56-57 fields/s. GPU 24-49% busy; the
driver is 13% of the thread. Without the feed the game needs about 72% of a
thread at 60 fields/s (still vista). One render thread would need about 94%
at 180 in the still vista; two about 47% each. After loads the slow stretch
is the feed saturation until the step down, plus about 0.2 s of live-drawn
frames. Options: R1 all GL on a render thread (not recommended: the live
command path reads guest RAM); R2 redraw renderer instances on their own
threads with shared GL contexts, per-thread renderer state, a per-frame
snapshot (VRAM blit, GPU snapshot, RAM reads from the worker slot image),
presentation on the render thread (recommended); R3 workers send ready
vertex batches (later, if needed). Staged plan 2a/2b/3/4 in note 353. No
code changed.

**2026-11-01 implementation (Needs playtest; note 353).** User: "im so ready,
lets go" (R2). Polymorph's redraws are drawn on a host render thread with its
own renderer instance (the renderer sources compiled again with every global
renamed, `runtime/src/render_copy/`), a shared GL context and a per-publish
snapshot of the VRAM surfaces, CPU VRAM, GPU emulator and widescreen state.
The emulation thread queues redraws and adopts finished images. Title: cache
victims skip images being drawn, leads include the render latency, a due
present waits briefly for an image already being drawn, and the slack/budget
governors no longer hold queueing back. Classic and `DNTTK_RENDER_THREAD=0`
keep the single-thread path; Vanilla unchanged.
- Same pictures: 1646 of 1646 sampled images exact against the single-thread
  path (`PSX_RT_VALIDATE=1`, all 10 fixture slots, still and turning).
- Slot 10 vista at fixed 180: game 41-43 to 59.9 fields/s, presents 155 to
  180, emulation thread 100% to about 70%, feed share 50% to 0.
- Route sweep at 180 with the governor: 162 of 180 seconds at full rate
  (D17AX build: 130); every slot but the vista holds 180, slot 2 99 to 180.
- Slot 2 load: 0 audio underruns (was 504).
- Limits: the vista standing still holds 180 presents but about 110 fresh
  images/s, limited by worker jobs (21-23 ms each, 3 workers; more workers
  are slower on 8 cores); the slow first second after loading the vista
  remains (both builds about 45 fields/s). One render thread by default
  (`PSX_RENDER_THREADS=2` gives no gain here).
- Tests: controls 119 PASS, near, input, auto rate; clean quit; the patch
  applies to the pinned commit; codegen hash unchanged.

**2026-11-01 accepted and closed.** User: "perfect. it plays beautifully ...
fully accept. lock it in, close us out". Their session log (slot 2 load, about
7 minutes): one step to every 2nd refresh at the load (audio ring 28 ms),
back to every refresh, then 180.0 presents/s in every full minute with no
further changes; window-close exit, no crash; RSS 408 to 472 MB, levelling
off. D17AW is Done. Retained limits: the slot 10 vista standing still is
limited by worker job cost (about 110 fresh images/s); the slow first
second after loading the vista; 120 Hz, the 21-level sweep with this build
and the second instance's GPU memory not measured.

### D17AX - Polymorph: draw the whole level from jetpack height (slot 10)

**Status: Done (user-accepted 2026-11-01: "100% accepted. no slowdowns, no crackle, its stable, no flicker"; [note 352](documentation/352-d17ax-jetpack-vista-report.md)). Dependencies: D17AR, D17AQ; related D17AW.**
Video review (2026-11-01): hovering in slot 10 (level 6) facing the castle,
the band between the front wall's top and the mountain backdrop is black
(the castle interior and anything beyond the wall), and turning left or right
the level reads as a floating island: everything past the moat banks is
black down to the screen edge. Same class as the original D17AQ report
(centre drawn, sides empty), which concluded the rest was authored void
after listing every sky room; that was never shown for every room or with
host visibility. Report first, per black region (castle interior, both
side banks, behind the camera):
- Geometry the level has but the host pass does not select (host portal
  walk, over-the-walls test, guard band, far limit 65535, piece budget, the
  original room data's rectangles) - prove it with a developer mode that
  draws every room and piece of the level in range.
- Geometry the level never authored (the PlayStation camera never saw it).
  Propose fills for the user to choose: extend terrain to the backdrop,
  a horizon skirt matched to the sky/mountains, distance fog or haze, or
  authored meshes per the D08A28 style guide.
Measure cost at 180 Hz with D17AW in mind (the player's machine already
steps slot 10 to every 2nd/3rd refresh). Then implement the chosen fix
game-wide (every outdoor level from jetpack height, not a slot-10 patch),
Polymorph only; Classic frozen; Vanilla unchanged. Acceptance: slot 10
turning a full circle at hover shows no black ground or wall gaps the user
objects to, slot 2 moat and other outdoor levels checked, 21-level sweep
without lost geometry, user playtest.

**2026-11-01 report (awaiting decision; note 352).** Full-turn captures at
hover with a new developer switch `DNTTK_HOST_VIS_ALL` (`1`, `sky` or a room
list; rooms drawn whole-view, kept out of the props list; debug `list` of
selected rooms). Behind the camera and past both moat banks: authored void.
Level 6 has no room beyond room 1's outer bank (Z -6144) or the moat rooms'
outer edges, and forcing any room group adds nothing there. The castle band:
the keep tower and gatehouse exist in non-sky rooms 33, 34, 38 and 43-49,
which the over-the-walls test never offers because it considers sky rooms
only. The rest of the band is void below the sky model's black mountain
ring. Every sky room with geometry in view is already selected; the guard
band, the 65535 limit and the room rectangles hide nothing. Side finding:
every room whole-view fills the near-clip arena (139616 of 139744 bytes),
and pieces falling back to the game's renderer overran its packet area
(garbage textures, Duke dropped, GPU stopped on 0x1F; reproduced twice). Arena
peak: 30% current, 59% with all interior rooms. Cost at 180 Hz (player
settings) with all interior rooms: within run-to-run noise, fields at 60.
Sky rooms exist in levels 1, 5, 6, 8, 10, 11, 21 and 28. Proposed: A
(over-the-walls for every room, capped before the arena fills, never handed
back to the game's renderer), then B1 (backdrop continued below the
horizon) with B2 (distance haze); B3 horizon skirt or B4 authored terrain
optional. `ttk-controls-test` (119 PASS), `ttk-near-test` and
`ttk-input-test` pass. The player-facing build is unchanged apart from the
developer switch and debug field.

**2026-11-01 implementation (Needs playtest; note 352).** User: "i think B4
is too extreme, at least at this point in time. but other than that, let's
go with your most educated and informed recommendation" (A, B1, B2; B3
left for later).
- A: from a sky room, every room is offered to the over-the-walls test. Non-sky
  ("scenery") pieces also need a sight line to their top that clears every
  regularly drawn piece's bounds. They are drawn last, dropped (never handed
  to the game's walk) when unaffordable, and kept out of the props list and
  shown set.
- More packet space: the vista already exceeded the 350 KB region, and the
  1 MiB GPU aperture was full. The framework gains a second 1 MiB aperture
  below it (0x80E00000), which each near-clip region uses for another
  512 KB. It is carried in savestates as an optional tail older builds skip,
  and in replay images. The first aperture's layout and the codegen hash
  (0x8bab543c) are unchanged, and existing saves load (`modmem=restored`).
  Two post-load faults found while testing were fixed: the missing world for
  a few frames, and texture corruption after the third cycle.
- B1: a mist band below the horizon in the level's own sky tone (sampled
  from VRAM), under the mountain ring.
- B2: GL distance haze towards the band's horizon colour for host-depth
  primitives, from the original fog start to the extended limit, at most
  55%. B1 and B2 are on only in sky rooms and ease over 0.5 s.
- Evidence: in slot 10 the keep tower and gatehouse show, and black below
  the mountains is now mist. 153 pieces a frame, no fallbacks or drops
  (original-renderer fallbacks 1404 to 0). Slot 2 is unchanged, and a 21-level
  sweep against the old behaviour lost no geometry. Three save/load cycles
  are clean. Vanilla is unchanged. Tests: controls 119 PASS, near, input,
  framework aperture; the patch is exported and verified on the pinned
  commit.
- Cost at 180 Hz (player settings): about 2 fields/s more in the slot 10
  vista (raw 57 to 54-55). With the governor it runs at every second refresh
  (90 presents/s), with fields at 60; D17AW is the fix for that.
- Developer switches: `DNTTK_OVER_WALLS_ROOMS`, `DNTTK_SCENERY_OCCLUSION`,
  `DNTTK_EXTRA_ARENA`, `DNTTK_MIST`, `DNTTK_HAZE`, `DNTTK_HOST_VIS_ALL`.
- Not seen: other outdoor levels from the jetpack (scripted flights did not
  climb), the gatehouse notch filled (it stays black), the player's display.

**2026-11-01 revision (Needs playtest; note 352 Revision 2).** User after
the playtest: the fog "doesnt work for me ... visually its not it", and the
gatehouse is "probably unnecessary"; "all i really wanted was for a couple
of those draw distance issues ... to not flicker". Then: "let's keep the
high KB per frame, and if anything go a lot higher". Mist and haze are for
a later, better-integrated job; the notes stay in note 352.
- Removed: A (non-sky rooms over the walls, per-piece occlusion), B1 (mist
  band, sky sampling), B2 (haze API and shader terms).
- Cause of the flicker: in the slot 10 vista the 350 KB region overflowed in
  about half the frames, and the distant pieces that did not fit went back to
  the original renderer as black holes, changing with the view. Consecutive
  turning shots show a far wall section missing in one frame and present in
  the next (old size); with the larger region it is present in every frame,
  and nothing goes to the original renderer.
- Kept and grown:
  - Second GPU DMA aperture, 2 MiB at 0x80D00000 (3 MiB at 0x80C00000 stopped
    the BIOS at boot). Each frame region gains 1 MiB (1.35 MB in all, about
    2.8 times the vista's peak); the two regions alternate 64 KB chunks.
  - Replay images and the "XTR2" savestate tail copy only the part ever
    written. Older saves load untouched; the codegen hash is unchanged.
- Evidence: three save/load cycles clean; 21-level sweep identical at heading
  0; controls 119 PASS, near, input and the framework header test pass; the
  patch is verified on the pinned commit.
- Cost at 180 Hz: raw 54.6-56.7 fields/s against 55.2-57.3 (about 1
  field/s, within spread).
- Developer: `DNTTK_EXTRA_ARENA=0`, `DNTTK_HOST_VIS_ALL`.

**2026-11-01 accepted and closed.** User: "100% accepted. no slowdowns, no
crackle, its stable, no flicker. lock it in, close it out!" D17AX is Done.
Retained limits: other outdoor levels' jetpack vistas not flown by script;
the authored void past level edges stays black; mist and haze are a future
job (design kept in note 352); the slot 10 vista still needs D17AW to hold
180 presents/s on this machine.

### D17AY - Full-refresh presentation: no draw-time reads of 30 Hz tick state

**Status: Done (user-accepted 2026-11-02). Dependencies: D17 (render replay), D14H, D35D1.** The game
logic, physics and animation timing run at the original 30 Hz tick and stay
there; raising the simulation rate is not the goal. Render replay already
interpolates the camera, object transforms and Duke's joint matrices to the
display rate (120/180 Hz). The defect class is host presentation code that,
at draw time, reads gameplay state that is only updated per tick (player
position +4..+12, facing +0x1c, held/attached object positions, timers) and
combines it with interpolated data. D35D1 revision 2 was one instance: the
push grip anchored to Duke's last-tick position stepped against the
interpolated eye (up to 29 units in redraws) and vibrated; D14H's body camera
versus the late camera was a second, rotational instance.

Scope:
1. Inventory every draw-time hook in `recomp/src/ttk/` (first-person eye and
   body/limb rigs, contact arms, weapon/viewmodel and display-hand placement,
   Q/door legs, swim/jetpack arms, HUD and world markers, crosshair/red dot,
   reflections, the D14H body camera) and list the tick-rate state each reads.
2. Provide shared interpolated values for redraws (for example player
   position/facing and the held or attached object) through the render-replay
   snapshot, so fixes reuse one source instead of per-feature workarounds.
3. Add a per-draw diagnostic that reports a presentation quantity stepping at
   the tick rate against the interpolated view, and use it at 120 and 180 Hz
   (the player's display is 180 Hz).

**Acceptance:** an inventory with each reader classified (fixed, already
interpolated, intentionally tick-rate); measured before/after evidence for
every fix at 120 and 180 Hz; Vanilla and 60 Hz unchanged; the user's
playtest of the reported cases.

### D17AZ - The slot-12 train vanishes at the edge of the view

**Status: Done ([note 365](documentation/365-d17az-object-reach.md)), accepted 2026-11-02. Dependencies: D17AR (host world visibility), D17AR4, D17AX.**
Priority medium. Renderer work targets Polymorph (Classic is frozen).

Observed (user, 2026-11-02): load UI slot 12 and slowly turn right. The train
disappears before it leaves the view. The user compares it with EDuke32, where
a sprite or voxel that spans several sectors disappears at the edge of the
view when the sector it belongs to is culled.

Brief investigation (documentation and code notes only; nothing run; every
cause is a **hypothesis to confirm**): the user's guess holds up. In TTK the
"rooms" do work like sectors for drawing. Each frame the portal walk
`0x8006276c` lists the rooms whose portal openings are on screen (at most 32,
each with the screen rectangle of its opening). The game's room walk, the
props/actors pass `0x8002ffec` and the object pass `0x800374cc` all draw from
that list ([note 336](documentation/336-d17ar1-host-visibility-design.md)).
D17AR moved **world geometry** to a host decision. The props/actors pass gets
the host room list. The **object pass still gets the game's own portal list**,
because its handlers affect play ([note 337](documentation/337-d17ar2-host-visibility.md)).
Likely causes, in order:
1. The train is drawn through the object or props pass. It is linked to one
   room (or to the room of its origin). When that room's opening leaves the
   screen, the room is not listed and the whole train is skipped, even
   though its body still reaches into rooms that are on screen. This is the
   EDuke32 sector case.
2. A per-object cull against the listed room's rectangle or the frustum uses
   the object's origin and a small radius instead of its real extent. A long
   train is then dropped once its origin leaves the view.
3. Less likely: the D17AM3 far reserve hands it back to the original
   renderer, or a far limit cuts it (D17P, D17AX).

Investigation: identify the train's owner in slot 12 (level, object or prop
record, type, room link, bounds/radius). Trace the frame where it vanishes:
is its room listed (game and host walk)? Is it submitted and then culled, and
by which test? Compare Polymorph with Classic and with `hostvis off`, in first
and third person and at FOV 90/120. Then sweep the other levels for large
multi-room objects (trains, vehicles, doors, lifts, big props) that show the
same pop.

Direction (to choose after the report): a game-wide rule, not a slot-12 patch.
Draw an object when its real bounds intersect the view frustum and any room
visible to the host, whichever room it is linked to. Keep the object pass's
gameplay handlers on the game's own list (as D17AR requires); only the drawing
decision widens. Classic and Vanilla stay unchanged.

Acceptance:
1. Slot 12: turning slowly in either direction, the train stays drawn until
   its real extent leaves the screen. First and third person, FOV 90 and 120,
   at 180 Hz.
2. The object pass's gameplay behavior (handlers, triggers, AI) is unchanged.
3. A level sweep finds no other object popping at the view edge, or lists the
   remaining cases with their causes.
4. Polymorph frame rate holds in the heaviest views (D17AR5); Classic and
   Vanilla are unchanged.

### D17BA - The burning barrel's inside is missing up close

**Status: Done ([note 366](documentation/366-d17ba-open-models.md)), accepted 2026-11-02. Dependencies: none (renderer work targets Polymorph).**
Priority medium.

Observed (user, 2026-11-02): load UI slot 12 (saved 2026-11-02 20:33) and walk
up to the burning barrel. Its back is not drawn; the rest only appears once
Duke walks around it.

Brief look (offscreen, private copy of the save; nothing changed; every cause
is a **hypothesis to confirm**): the barrel is a placed static object (type
220, renderer kind 1 `0x80031d10`, model `0x8012cb90`, local box about 320 x
750 x 280, room 22), with the flame drawn above it. From about 500 units and
looking down, the barrel's open top shows the floor and the wall behind it
where the far inside wall should be (`recomp/analysis/d17az-barrel/barrel-sheet.png`).
Likely causes, in order:
1. The user's hypothesis: the model's faces are one-sided. From outside, the
   far side's inner faces point away from the camera and are back-face culled
   (NCLIP), so the open top shows through to the background. The original
   PlayStation view (low, far camera) rarely looked into it.
2. The host culling decision (D17 precise culling, `DNTTK_PRECISE_CULLING`)
   or the near-clip emission drops faces the original NCLIP would keep.
3. The model has no inner faces at all (only an outer shell and no lid), so
   only added geometry or a cap would close it.

Investigation: compare Polymorph, `DNTTK_PRECISE_CULLING=0`, Classic and
Vanilla in the same view. Trace the barrel's polygons (primitive trace) for
culled versus absent faces. Then sweep the levels for other open models
(barrels, bins, pipes, crates) with the same look.

Direction (to choose after the report): a game-wide rule, not one barrel. For
example, draw both sides of the faces of open models (flagged per model from
the game's own data), with the inside showing the same texture, if cause 1 or
2 holds. Classic and Vanilla stay unchanged.

Acceptance:
1. Slot 12: walking up to and around the barrel, its far side and inside are
   always drawn; first and third person, 180 Hz.
2. No double-sided faces show through solid models elsewhere (z-fighting,
   wrong-side lighting); the level sweep lists every model the rule changes.
3. Polymorph frame rate holds; Classic and Vanilla unchanged.

### D17BB - Other rooms' wall pieces show over the castle walls

**Status: Done (region rule; accepted 2026-11-02; [note 367](documentation/367-d17bb-over-wall-leaks-report.md)). Dependencies: D17AR (done). Renderer work targets Polymorph.**
Priority high (visible in normal play, not only from the jetpack).

Observed (user, 2026-11-02): load UI slot 12 (castle courtyard, level 6) and
look over the castle walls. Fragments of other walls show above the
battlements: a spire, a tilted merlon-like block, small spikes and a dark
roof stub. It looks glitched.

Cause (confirmed, note 367): the over-the-walls test in host visibility
(`host_vis_select`, from D17AQ's open sky) offers every sky room whose piece
tops seem to clear the camera room's walls. From the courtyard (room 8) it
admits moat rooms 2, 3 and 4, which the original portal walk never lists
there. Room 2 holds the moat-side skin of the same wall, with its own spire
and merlons; what rises above the inner wall is drawn. The test compares
bounding boxes only where the sight line crosses the room edge, and an edge
point with no piece counts as open. The game's own room list shows a clean
wall. Classic's D17AQ rule does the same (Classic frozen).

Options (note 367): A, over the walls only while the eye is above the
camera room's walls (with hysteresis); B, an over-wall room's pieces that
form the far skin of a shared wall stay hidden from inside; C, a per-level
visibility rules file of room pairs with a developer console to author it
live (override layer); D, a precomputed visibility set (not recommended now).
Recommended: A, then B if raised views still show skins, C for single cases.

Acceptance:
1. Slot 12: no foreign wall pieces above the courtyard walls at any heading,
   first and third person, FOV 90 and 120, 180 Hz.
2. Slot 10 jetpack vista and slot 2 moat: what D17AQ, D17AR and D17AX fixed
   stays fixed (no pop-in, no lost towers or banks).
3. Sweep of the outdoor levels (1, 5, 6, 8, 10, 11, 21, 28) at ground level
   and from raised spots: every remaining over-the-walls admission listed and
   checked; no regression elsewhere.
4. Polymorph frame rate holds; Classic and Vanilla unchanged.

### D17BF - Sleeping enemies: visible, not live, as in Duke 3D

**Status: Done (user-accepted 2026-11-09: "it's absolutely perfect!! i can see everything i want in the security cams"; closeout authorized 2026-11-09: "lock it in and close it out"; [note 431](documentation/431-d17bf-sleeping-enemies-report.md)). Delivered: sleeping copies drawn (step 1) and the slot churn fix. Hits waking copies, blasts/flames and the D17AQ review moved to D17BF1.** Dependencies: D17AQ, D17AZ, D52 (Done).**
TTK creates spawn-point enemies only near Duke, up to a per-level cap of
live enemies (`0x800c56a8`, 12 in level 0); every other spawn point is a
stored position (`*0x800de720`, 48-byte records: +0 type, +4 live actor, +8
position, +0x14 flags). So security monitors and distant views show empty
rooms, and D17AQ spends real slots on far dormant enemies just to show them.
User: "we dont want the enemies live, we want them like duke3d. not live,
but visible"; on a shot creating the real enemy: "this is basically how
duke3d works yes".

Wanted (Modernized; Vanilla and the "original" draw distance unchanged):
- **Sleeping copies.** For each spawn point with no live enemy (and not
  killed or disabled), the host draws that type's model at the record's
  position and heading in a standing pose, through the game's own actor
  renderer with a stand-in actor in mod memory: no game slot, no AI. The
  record is the truth, so a removed enemy's saved position moves its copy.
- **Everywhere the world is drawn.** Security monitors and other game-owned
  views (D52) and distant views; room visibility as the game's (D17AZ-style
  reach draws), nearest first under a per-frame budget, lit like real
  enemies (D41), steady in 120/180 Hz redraws.
- **Handover.** When the game spawns the real enemy, its copy disappears the
  same frame at the same spot and, ideally, pose.
- **Hits wake them, as in Duke 3D.** A bullet, blast or projectile that hits
  a sleeping copy creates the real enemy there (at the cap the farthest
  dormant enemy gives way, as the original rule does) and the hit is
  applied through the game's own damage path, so it reacts and wakes for
  good. Auto-aim may target copies as Duke 3D allows. If every slot holds an
  awake enemy near Duke the shot passes and the copy sleeps (verify how
  often that can happen).
- **Combat unchanged.** Enemies near Duke, their count and their AI stay
  the original's; revisit whether D17AQ's far dormant actors are still
  needed once copies cover distant views.

Start report-only: pose source (the game's pose routine on the stand-in, or
a captured standing pose of the type), how killed and disabled records are
marked (flag 0x10, script-held +0x16 rows), visibility and draw budget, and
the hit paths (hitscan, radius damage, projectiles in flight, auto-aim; see
D10H3 and D49). Then a prototype on the slot 9 monitors (UI slot 9, file 8,
private copies only). Acceptance: enemies visible asleep on monitors 2 and
3 and in a distant view, no pop on handover, a shot copy wakes as a real
damaged enemy, combat counts unchanged, Vanilla unchanged, user playtest.

**Work log (2026-11-09, report).** [Note 431](documentation/431-d17bf-sleeping-enemies-report.md).
No game code changed. Slot 9 (private copy, level 0): 71 rows, 12 live
(cap 12), 25 eligible for a copy (`flags & 0x17 == 0`), 15 dead (0x4,
carried from the actor, inferred), 18 disabled (0x10). The actor renderer
`0x80032e78` reads no lists and rebuilds the joints each call from the
skeleton at +0x40, so a stand-in actor in mod memory can be drawn the D17AZ
way (CPU copy, render state saved); pose from the type's skeleton template
or, if that is a bind pose, a captured idle pose. Rows carry facing (+0x18
quaternion). TTK rounds are projectiles: the existing sweep hook
`0x8006dd2c` can wake copies for rounds and rockets; blasts (`0x800a9bf8`)
and flames (`0x800a9ee0`) need new entry hooks; beam lock, auto-aim and kick
are host-side. Found: `spawn enemy 26` fails in level 0 (also without
D17AQ). Next: user approves the plan (render-only prototype first).

**Work log (2026-11-09, step 1 drawing, Needs playtest).** User: "yes
proceed". New `recomp/src/ttk/sleeping_enemies.inc` ([note 431](documentation/431-d17bf-sleeping-enemies-report.md)
section 8), Modernized Extended only; no new hook, no regenerated code.
Up to 48 nearest eligible rows get a stand-in actor in mod memory, built by
the creation's own type setup `0x80095504` (flag 0x10 during the call, no
list entry), placed at the row or its position slot, facing from the row,
resting height learned per type from live enemies; listed in a mod-memory
table the redraw workers also see; drawn by `0x80032e78` at the object pass
in the pass's rooms and host-shown rooms, nearest 16 a frame. Slot 9: copies
on both monitor views (empty with `DNTTK_SLEEPING_ENEMIES=0`), steady in a
180 Hz burst; slot 2: 20 handovers at the copy's x/z and facing, height 0
(type 57) / 17 (Necro) off; about 0.45 ms per visible copy, fields 60/s
unchanged; Vanilla builds nothing. `ttk-near-test`, `ttk-aim-test`,
`ttk-input-test` pass; `ttk-controls-test` link failure predates this.
Manual updated. Remaining: hits waking copies (step 2), blasts and flames
(step 3), D17AQ review (step 4), distant view seen on screen, user playtest.

**User playtest (2026-11-09).** "it's absolutely perfect!! i can see
everything i want in the security cams." Slot 2, watched read-only on the
debug port: the distant static Necro the user saw was a live D17AQ dormant
enemy (row 0, 32539 away), not a copy; two rockets took it 8000 to 1048
(woke, `woken` 1) to 111, then dead. After the kill the D52 tie rule let a
hovering Necro (row 28) and a type 57 (row 38) trade the last slot every 19
frames: actor positions were compared with row positions, and the Necro
hovers about 1100 above its row. Fix (`enemy_reach.inc`): the enemy that
would give way counts at the nearer of where it stands and its row's place,
so a replacement can never be reversed. Offscreen slots 2 and 9: 0 spawner
changes in 6 s windows, live 12 of cap 12. User repeated the kill on the
new build (watched read-only): rockets 8000 to 1154, dead at frame 2642;
the freed slot went to row 18, then nearer rows 23 and 29 within 10 frames
(each replacement brings a nearer enemy), then no change for the remaining
975 frames until the user closed the game (before: a swap every 19 frames).

### D17BF1 - Sleeping enemies wake when hit

**Status: Todo (split from D17BF at its closeout, 2026-11-09). Dependencies: D17BF (Done).**
Steps 2-4 of the D17BF plan ([note 431](documentation/431-d17bf-sleeping-enemies-report.md)
sections 5 and 7). A round, rocket, kick, beam lock or auto-aim that would
hit a sleeping copy creates the real enemy at the copy (at the cap the
farthest live enemy gives way, as the original rule does) and applies the
hit through the game's own damage path, so it wakes for good. Rounds and
rockets through the existing sweep hook `0x8006dd2c`; kick, beam lock and
auto-aim host-side; then blasts (`0x800a9bf8`) and flames (`0x800a9ee0`),
which need new entry hooks (check the codegen hash so savestates still
load). Check first that host-called creation works where `spawn enemy 26`
fails (level 0). Then review whether D17AQ's far dormant live enemies are
still needed now that copies cover distant views. Modernized Extended only;
Vanilla unchanged. Acceptance: a shot copy wakes as a real damaged enemy in
slot 2 and slot 9, combat counts unchanged, user playtest.

### D17BG - Sandstone room ceiling face drops out to black

**Status: Done (user-accepted 2026-11-10: closed as not a player fault; software OpenGL notice added).** Found in D41F (note 439):
a ceiling face in the sandstone room (UI slot 8) showed black on some frames.
It happens only in the offscreen test runs (Xvfb, Mesa llvmpipe): llvmpipe
samples wrong texels for a perspective-textured triangle it clips at the
framebuffer edge, in one of the two display buffers. On the real display
(NVIDIA) the face showed in every sample (30 and 180 Hz). No code changed.
Closed as not a player fault; a software OpenGL now gets a launch-log line
and a one-time main menu notice (install the graphics driver). [Note 440](documentation/440-d17bg-ceiling-dropout.md).

### D40 - Switch renderer live in game (EDuke32-style)

**Status: Done (user-accepted 2026-11-03; closeout authorized; [note 371](documentation/371-d40-live-renderer-switch.md)). Dependencies: D17AR6 (renderer choice), D39G (Done).**
User, 2026-11-03: "i want the ability to switch renderer on the fly like in
eduke". In EDuke32 the renderer (`setrendermode` / the video menu) changes
immediately, mid-game. Here OPTIONS > DISPLAY > RENDERER (Polymorph / Classic)
applies only on the next launch.

Known from intake (not yet audited): `run.py` passes the choice as
`DNTTK_WORLD_RENDERER` and, for Classic, defaults `DNTTK_UNIFIED_GEOMETRY`,
`DNTTK_HOST_VISIBILITY` and `DNTTK_REDRAW_ONLY` to 0. Several readers latch the
environment once in function statics (`screen_flash_classic()`, host
visibility, `frame_replay.cpp`), and the near-clip packet arenas, redraw
workers and render replay state are sized and chosen per frame from them.

Scope:
1. Survey every consumer of the four switches (and anything derived: arenas,
   replay workers, present tint, unified geometry caches, host visibility
   state) and classify each as switchable per frame, at a frame boundary, or
   needing a level reload.
2. One runtime renderer state (in RenderState / mod memory as appropriate)
   replacing the latched getenv reads; environment and profile only seed it at
   launch. Explicit developer overrides keep winning.
3. The OPTIONS entry applies on Confirm, mid-level, at a clean frame boundary
   (no torn or black frame, no packet arena overflow, redraw workers resynced),
   and still saves the preference. Optionally a console command (`renderer
   polymorph|classic`, like EDuke32's `setrendermode`) and a debug port command
   for tests.
4. Flashes follow the switch: a Polymorph tint in flight stops cleanly on
   Classic; Classic's frozen quads stop on Polymorph (D39G).

**Acceptance:** switching both ways repeatedly mid-level, in third and first
person, while moving, firing, frozen and during a flash, at 60/120/180 Hz,
shows the chosen renderer from the next frame with no crash, stall, missing
geometry or stale state; savestates load correctly after a switch; the choice
persists; Vanilla stays Classic and unchanged; native regressions; user's
playtest.

### D46 - Apartment mirror shows the outside when Duke turns left

**Status: Done (user-accepted 2026-11-08: "fully accepted. excellent work! absolutely superb. lock it in, close it out"; [note 414](documentation/414-d46-mirror-portal-depth.md)). Dependencies: none known.** User, after the
D41H playtest: "slot 8 will load you into the apartment, if you turn duke to
the left, you'll notice that the mirror pops into the outside. i never
noticed this before". Unknown whether it is a regression and from which job
(candidates to check: the D41C/D41H host-drawn lit Duke and models, D17
host visibility and unified geometry, the D40 renderer switch); compare with
dynamic lights off, `light smooth off`, `DNTTK_DUKE_LIGHT=0`, the Classic
renderer and Vanilla, and older builds if needed. Work only on a private copy
of the player's slot 8 (never the player's saves or cards). Acceptance:
reproduce on the copy, name the owner of the wrong mirror view with evidence,
fix it game-wide (not for this mirror only) and matched shots before and after
turning left; user playtest.

Work log 2026-11-08: reproduced offscreen on a private copy of UI slot 8
(file 7): from about yaw 55 the mirror shows the outdoor wall behind the
bathroom. Same in Classic, with lights off, original draw distance and the
other host culling switches off. Owner: the game's portal walk listed street
rooms 5 and 6 (blanking room 6's pieces restored the reflection), because the
portal rectangle test uses `camera+0xa0` (the depth of Duke) as its
near-portal limit, and in the eye view Duke's draw returns before storing it
(nearer than H), so it stayed at the third-person 5386. `camera_target_near`
(first_person.inc) completes the original `max(depth, H)` for that case:
`camera+0xa0` = H (160) for the main camera's own target, Modernized only,
`DNTTK_TARGET_DEPTH=0` to compare. Matched before/after shots over a full
left turn (60, 180 Hz, Classic): the mirror keeps its reflection; the street
through the doorway is unchanged; third person and Vanilla identical.
`ttk-near-test`, `ttk-input-test`, `ttk-aim-test` pass; `ttk-controls-test`
stops at a pre-existing assertion with or without the change. Remaining:
user playtest; other mirrors and first-person doorways elsewhere not
surveyed.

### D52 - Security camera screens: low frame rate and glitches over them

**Status: Done (user-accepted 2026-11-09: "I can confirm the cams are now stable, and cams 2 and 3 show no enemies"; closeout authorized 2026-11-09; [note 430](documentation/430-d52-security-cameras.md)). Dependencies: none known. Follow-up: D17BF.**
User: "slot 9, the security cameras display at a seemingly low framerate,
and display graphical glitches over them. check the 3 security cams and then
investigate why this is happening, and then fix the security camera". Work
only on a private copy of UI slot 9 (file 8); never the player's saves or
cards. Find the three camera screens, measure how often each screen's image
changes against the game and display rates, capture the glitches, and name
the owner of each fault with evidence (original game behaviour, the 120/180
Hz redraw workers, Polymorph, D17 host visibility or culling, D41 lighting,
the D46 target-depth fix). Fix it game-wide (every security camera, not just
these three), Modernized default; Vanilla unchanged in play. Acceptance:
matched before/after shots of all three screens at 60 and 180 Hz, a measured
screen update rate, and the user's playtest in slot 9.

**Work log (2026-11-09, Needs playtest).** Lab `recomp/analysis/d52/lab.py`,
private copy of UI slot 9. The monitors open with the action (left mouse);
the view is the alternate camera `0x800d7010` (Duke +0x224 bit
`0x20000000`, D35K). Two faults. (1) Frame rate: the composition draws from
`0x800da488`, the alternate camera or the player's camera
(`0x8002620c..0x8002626c`), but the 120/180 Hz redraws interpolated only
`0x800d6eb0`, and recorded objects only drawn through it. So the security
picture moved at the game's 30 fps, and with the late camera the newer- and
older-list redraws (D10H8) alternated between two camera positions. Old
build, 180 Hz: pan in whole 0/3/6 px game-frame steps, 7 of 39 shot pairs
identical. (2) Glitch: a dark slab across the middle camera's picture, from
the object near clip drawing faces wholly nearer than H, which the original
drops: the camera's own housing (only `DNTTK_NEAR_CLIP=0` or `=world`
removes it). Fix: `composition_camera()` and `game_camera_view()`
(near_clip.cpp); redraws capture, interpolate and write the drawn camera
(switches are cuts; late mouse view, body camera and room walk only for the
player's camera), and record objects drawn through it; in a game-owned view
the object near clip leaves all-near faces to the original. After, 180 Hz:
no identical pairs, 1-2 px in-between steps, about 500 game-camera blends a
run, first and third person and Classic; the slab is gone in matched shots
at camera heading 350; exit back to Duke's view clean; ordinary play takes
the same paths (`game_camera_blends` 0). The right camera's see-through
block is the original's water. `ttk-near-test`, `ttk-aim-test` pass;
`ttk-input-test` (no D52 file) hung on the desktop display after its
rebuild. Remaining: user playtest; Vanilla not compared on screen (pad
injection did not move Duke); other levels' cameras and `0x800da488`
scripted views not visited.

**Work log (2026-11-09, revision 2, Needs playtest).** User: "while it
currently looks better than before with the higher frame rate, the enemies
on the security feed seem to glitch in and out. this seems to affect only
monitors 2 and 3". Reproduced at 180 Hz on the middle monitor: an enemy in
2 of every 8 shots while the camera barely moves; the same at 60 Hz (no
redraws), with every host renderer switch off, in Classic and in the build
from before D52. A per-tick list of the actors the game drew showed actor
`0x801cff94` (draw kind `0x80032e78`) in 3 ticks of 18, its position and
room different almost every tick (rooms 43, 76, 31, 48, 34, 67, 54...),
each one a spawner record (`*0x800de720`, type 26 and 123). With
`DNTTK_ENEMY_REACH=0` it stopped. Cause: D17AQ dormant enemies spawn far
spawners while the live cap (12 in level 0) is full, so the spawn check
`0x8009926c` asks the replacement `0x80099048`, which keeps the farthest
live enemy by the `0x800675d4` distance and replaces it unless the new
spawner is farther. That distance is `0x7fffffff` once an axis differs by
32768, so every far spawner tied with the far enemy and replaced it: one
actor round-robined between them, and an enemy in a watched room existed
one tick in six. Vanilla could not be compared on screen (the savestate
loads frozen there), but the original's spawn radius (about 25552) never
reaches saturated distances. Fix (`recomp/src/ttk/enemy_reach.inc`, in the
existing range-test hook, Modernized Extended only): with the cap full, a
spawner counts as in reach only if the enemy that replacement would remove,
worked out on the host with the original's saturated distances, is farther
from Duke than the spawner by exact distance; every replacement now brings
a nearer enemy in, so none can repeat. Guards added for `0x80099048` and
`0x800675d4`; counter `churn` in the reach debug JSON. No new game hook, no
regenerated code. After: the actor keeps one position, the middle monitor's
enemy pixels stay 0 in 40 shots at 180 Hz, the right monitor draws the same
16 objects every tick; slot 9 walking and turning: cap 12, live 12, 10
dormant, 474 tie replacements refused. Those far rooms now show no enemy,
as in the original. `ttk-near-test` and `ttk-aim-test` pass;
`ttk-controls-test` does not link (`gpu_vram_peek` from
`level_lights.inc`, D41D1, before this change). Strips in
`research/before-after/D52/enemy-flicker-*.png` (local). Remaining: user
playtest.

### D53 - Security monitors, Duke 3D style: camera overlay, screen art, light, toggle

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; revisions 1-7: Duke 3D TV and camera frame, edge culling fix, keypad and power-switch glow, FEED screen text, brighter screens, glow ring; [note 432](documentation/432-d53-security-monitors.md)). Dependencies: D52 (Done), D41D1 (Done). Follow-ups: D53A, D41K.**
User, slot 9: "In duke3d, there was a recognizable UI over the security
cameras, you knew you were looking at the security cam view. Duke TTK doesnt
really have that." And: "Many times have I just walked past those screens in
the subway because that design just doesnt play into my mental model at all.
So let's bring this back to duke!" Scope, game-wide (every security monitor
and camera view, not slot 9 only):

- **Camera overlay.** While a monitor view is up (composition camera is the
  alternate camera, D52), draw the Duke 3D camera overlay: four corner
  brackets, the "REC" lettering in the top left bracket and the blinking red
  record light (EDuke32 `G_DrawCameraText`, CAMCORNER 2482/2483 and CAMLIGHT
  2484, blink on `totalclock & 16`). Original TTK-style art, made in Photon
  Studio, not the retail tiles.
- **Monitor screen art.** Replace the dark glass of the TTK monitor face with
  a screen in the spirit of Duke 3D's VIEWSCREEN (tile 502: black terminal
  with "SECURITY" and status lines) inside a TV-style bezel like tile 638,
  so the screens read as screens. Original art per the D08A28 texel density
  and style guide (note 265); verify in game.
- **Screen light.** The screens give off a soft cool light (D41D1 texture
  emitters).
- **Settings toggle.** "Security monitors: TTK / Duke 3D" (Modernized default
  Duke 3D). TTK draws the original screens and no overlay. Vanilla
  unchanged.

Process: research (EDuke32 source and the "securitycam" save, user screenshot
`research/screencaps/Screenshot_2026-11-09_11-56-20.png`, reference tiles
`research/screencaps/tile0502.png`, `tile0638.png`), then a design proposal
artifact for user approval, then buildout. Acceptance: user playtest in slot
9 with both toggle values.

**Work log (2026-11-09, Needs playtest).** Proposal artifact approved: TV
face B, overlay in the 4:3 area, screen light. User during the buildout: TVs
"a little bigger than those in TTK"; then "go a lot closer to how duke3d's
security screen worked ... even match the colors"; the bezel's bottom "doesnt
connect/line up well. study the duke3d one even more"; and "lose the subway
thing ... i want this to be a generic asset". Built: `security_monitor.cpp`
swaps the LEVEL00 monitor mesh (found by shape) for a guest copy of a 620x620
TV mesh at the near-clip object entry, drawn from the second texture bank
(TV atlas at x 0..255, shared with the D17AO1 sky); the original is restored
whenever the host path cannot take it. Glass unlit by room and dynamic light,
cursor blink, faint flicker, an olive light per TV (0.9, radius 1800), and
the camera frame in the present pass (framework patch). Art: original,
Photon Studio (`recomp/assets/ui/security-monitor`), painted in Duke 3D's
style and palette values, no tile pixels. Option: OPTIONS > GAMEPLAY >
SECURITY MONITORS, schema 72, `--security-monitors`. Lab (private slot 9
copy): DUKE 3D at 60/180 Hz, first and third person, camera frame and blink,
TTK and Vanilla originals; `ttk-near-test`, `ttk-aim-test`, profile and UI
art tests pass; `ttk-controls-test` still fails to link (`gpu_vram_peek`,
D41D1, known). Before/after in `research/before-after/D53/` (local).
Remaining: user playtest in slot 9 both ways; light strength tuning; other
levels' monitor meshes (LEVEL05/09) unverified.

**Revision 2 (2026-11-09, Needs playtest).** User after the first
playtest: the TV "disappears" entering peripheral vision, the box sides look
like stretched pixels, the record dot's white glow does not work, and the
console by the door should light at its controls. Cause of the vanish
confirmed: props are culled by their own bounds (+0x4c), TTK's smaller panel;
monitor props now carry the TV's bounds in the Duke 3D look (composition
hook, restored for TTK). New 128x32 / 32x128 side strips, flat recess, glow
removed from the record light, and a green console light (type 17 console,
found by shape, 1.8 / radius 900, DYNAMIC LIGHTS). Lab: matched 180 Hz edge
sweep with and without the bounds fix, monitor use still works, console and
side before/after in `research/before-after/D53/`; `ttk-near-test` passes.

**Revision 3 (2026-11-09, Needs playtest).** User: only the console's
controls should glow, and the TV light went off far from the control room.
The controls are a separate one-quad prop (type 477 keypad on the console's
right section); it is now drawn self-lit (a guest copy as a gouraud quad at
colour 255, no dynamic light) when found lying on the console; the console
light is removed (point lights barely brighten these props). The 9000-unit
TV light range cut is removed. Lab: keypad 1.7x-2x brighter, desk unchanged;
3 TV lights published along a walk to the far door.

**Revision 4 (2026-11-09, Needs playtest).** Screen text made generic (any
number of cameras): of four mocked-up row sets the user chose B, "it's gotta
be B feed!": FEED LIVE / LINK SECURE / TAPE REC. Atlas rebuilt in Photon,
checked in game (slot 9).

**Revision 5 (2026-11-09, Needs playtest).** User, slot 10: make the power
button and the sign above it emit light, "make it look really cool". Usable
keypads (type 477, subtype set) glow on walls as on the console; the
lightning power sign (type 481) beside one is self-lit with an electric
breath and crackle, and a pulsing electric blue light throws a halo on the
wall. Lab at the user's slot 10 save: before/after and pulse strip in
`research/before-after/D53/power-*.png`; console keypad unchanged.

**Revision 6 (2026-11-09, Needs playtest).** User, slot 9: brighter
screens, "just the screens". Screen faces' self-lit level 140 to 230
(`DNTTK_SCREEN_LEVEL`); bezel, box and wall light unchanged. Matched shots:
text about 1.6x brighter, bezel and wall equal.

**Revision 7 (2026-11-09, Needs playtest).** User: the border is too dark
when the lights go off; "a touch of light haze around it". Added a self-lit
additive glow ring: a haze gradient layer in the atlas (Photon) and four
mitred strips in front of the bezel, sorted just nearer than the TV's opaque
faces (translucent faces are painter-only). Lab: full ring, steady at 180 Hz,
third person fine. Also stood up D41K (props in dynamic light).

### D53A - Security monitors keep a low-res live picture as you walk away

**Status: Todo (split from D53 at intake, 2026-11-09). Dependencies: D53 (Done).**
User: "in duke3d, when you arent watching the screen anymore, it shows a low
res image on the screen still as you walk away until it resets. i'd love
this, so much". EDuke32: after the use key, the VIEWSCREEN sprite stays
active and keeps rendering its camera into a small tile (refresh every
`ud.camera_time` clock ticks, 4 by default, about 30 per second) while the
player is within `VIEWSCREEN_ACTIVE_DISTANCE` 8192 units; past that the
screen resets to tile 502 (`actors.cpp` VIEWSCREEN case, `sector.cpp`
viewscreen refresh, `game.cpp` tsprite swap). For TTK: after leaving a
camera view, draw that camera's view at low resolution into the monitor's
screen face until Duke walks out of range, then back to the D53 idle
screen. Needs a second scene pass to an offscreen target in Polymorph.
Duke 3D toggle value only. Acceptance: user playtest in slot 9.
