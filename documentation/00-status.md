# Current status - 2026-10-04

## 2026-10-23 - D08T3 accepted and closed; D08L1 Todo

User accepted Free Push after final testing of slots 9-12 and explicitly
requested closeout and commit/push. D08T3 is Done: free dumpster movement to the
ladder, box push/pull, stable enemy collision and supported-edge refusal all
confirmed. Accepted executable: `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
No further gameplay changes or rebuild after acceptance. Authored source,
native regression tests, manual and [note 208](208-d08t3-manual-push-pull.md)
are included in closeout; player saves and media unchanged.

**Next selectable job: D08L1 (Todo)** in [the job board](../MODERNIZATION_JOBS.md#d08l1---modern-locomotion---allow-walking-and-crouch-walking-off-ledges).
After pushing the slot-12 dumpster to the ladder and mantling it, low overhead
clearance forces crouching and Duke cannot crouch-walk off. Record this as a
separate locomotion softlock, not a Free Push defect. Investigate original vs
modern ledge restrictions, grounded/crouched support and falling transitions.
Allow standing and forced/voluntary crouched walk-offs while preserving object
ledge safety, jumps, mantles, collision and Vanilla. Not implemented or started.

Candidate history below predates this acceptance.



## 2026-10-23 - D08T3 candidate (Needs playtest)

Review revision: reproduced and fixed the held-W grab timing failure that
escaped the first pass, plus the dumpster crossing its ladder. Flat panels
and complete room object lists now block the dumpster before contact. Final
private routes on slots 9-12 pass held-W grabbing, neutral hold, reverse,
sustained boundary refusal and release; native regressions cover both bugs.
Earlier pause/focus/mantle evidence below is from the first candidate.

Manual held push/pull and supported-edge checks are built. W/S stop immediately
on release, Grab stays held, reverse works, and a blocked object stays held.
Slot 9 stops at its ledge; slot 8's already-floating block is not relocated.
Build/native suites and isolated gameplay checks pass for the supplied objects,
release/pause/focus, saved moving pose, repeated loads and E mantle. User feel,
sound and puzzle-completion acceptance remain. See
[note 208](208-d08t3-manual-push-pull.md) for evidence and limits.

Candidate: `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
No player-save/settings or framework changes. No acceptance closeout, commit or
push. Launch: `python3 recomp/tools/local/run.py`; automatic capture.


## 2026-10-23 - D26B accepted and closed

User accepted the view/control ownership candidate and explicitly requested
"lock it in, close it out". D26B is Done. The authored camera/input changes,
native regression tests, manual and evidence notes are included in the
MusicMonsterMod closeout. No new implementation or build after acceptance.

Accepted executable:
`a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec`.
[Note 207](207-d26b-view-ownership.md) records the ownership contract, verification,
deliberate overrides and remaining coverage limits. Runtime patch re-exported
unchanged and verified against the clean pinned framework (51 files match).
No framework/pin or player-save changes. No next job started.
Launch: `python3 recomp/tools/local/run.py`. Automatic capture, no F10 step.


## 2026-10-23 - D26B view/control ownership candidate (Needs playtest)

D26B now covers the user's requested ownership of view/control choices across
console, menus, focus, saves/loads and transitions. Reproduced console/pause
camera teardown on input release. Player preferences now survive inactive
snapshots; authenticated camera presentation is separate from action/capture
permission. Loads and level teardown discard old world-specific camera state;
newly acquired supported cameras start in the selected view, including jumps.

Build and native input/aim/controls suites pass, with all 21 level guards. Six
live first/third-person runs at 60/120/240 FPS cover console, pause, focus, F7,
cross-perspective loads and rapid load/Escape. Final live level transitions,
same-level restart and Vanilla save/pause checks pass. See
[note 207](207-d26b-view-ownership.md) for evidence and explicit coverage limits.
Legacy/Original camera and unsupported traversal retain deliberate overrides;
they do not overwrite the saved view. User gameplay acceptance remains.

Candidate: `a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec`.
No commit/push or acceptance closeout. No framework or player-save changes.
Launch: `python3 recomp/tools/local/run.py`. Automatic capture, no F10 step.

## 2026-10-23 - D17Y accepted and closed

User confirmed both slot-8 graphical oddities are completely resolved, fully
accepted the fix and explicitly requested "lock it in. close it out". D17Y is
Done. The missing-animation face filter and direct open upper portal correction
are accepted, with their authored implementation and regression tests included
in the closeout. See [note 206](206-d17y-slot8-rendering.md) for diagnosis,
validated build, capture matrix, native tests and coverage limits.

Runtime patch re-exported unchanged and verified against the clean pinned
framework. No new framework edits or player settings/save changes. No next
job started; wait for the user's selection. Launch:
`python3 recomp/tools/local/run.py`. Gameplay capture remains automatic.


## 2026-10-23 - D17Y rendering candidate (Needs playtest)

Both current slot-8 reports reproduced and corrected in focused captures.
The fire platform was covered by missing-animation faces sampling the live
framebuffer; Modernized now filters those faces from a private mesh copy.
The wall strips were clipped at an open upper portal boundary; Extended draw
distance now extends only the direct taller sky-room neighbor's top edge.
Downstream apertures, valid flame textures and original meshes are preserved.

Build and native near/controls regression suites pass. First/third-person,
4:3/16:9, 60/120 FPS captures pass. Saved Auto was checked offscreen, where
it resolves to 60 without a physical monitor. User playtest and physical-display
Auto remain. See [note 206](206-d17y-slot8-rendering.md) for diagnosis, private
save identity, executable hash, evidence and limits. Player profile and all
39 save/card files unchanged. No framework edits, commit or push.

Launch: `python3 recomp/tools/local/run.py`. Resume captures automatically;
no F10 step. Await the user's slot-8 retest before acceptance/closeout.


## 2026-10-23 - D04B accepted and closed; D17Y queued

User accepted D04B ("way better") and authorized the bounded follow-up and
implementation/documentation commit/push. A brief post-load Escape interruption
still recovers automatically; no claim that every split-second interruption is
gone. Quick win: confirmed menu exit bypasses the generic 12-frame capture
cooldown, while requiring a new authenticated gameplay offer. Build and input
regression suite pass; focused live timing checks are in [note 205](205-d04b-control-recovery.md).
Final executable: `41bb0257acb5bd2e89677b5062fad2c74da0ed0e94d08cca546ae9847dd564c5`.
Broader prior evidence and coverage limits remain in that note.

D17Y is Todo: current UI slot 8, jumping while looking at the fire platform to
Duke's right causes severe flicker; turning 90 degrees right and jumping makes
the upper castle walls disappear until landing. User report only; not started.
Choose a job before any further implementation. Framework patch re-exported
unchanged and verified against the clean pin (51 files). No player saves/cards
changed. Launch: `python3 recomp/tools/local/run.py`; no F10 step required.


## 2026-10-23 - D04B control recovery candidate (Needs playtest)

Quickload plus rapid Escape reproduced a permanent host capture hold on the
accepted D17S baseline. The candidate ends that hold on explicit menu
acknowledgement or fresh gameplay after the Start pulse. F10 is inert; no
implicit tank fallback. OPTIONS > CONTROLS > CONTROL SCHEME is the persistent
MODERN / LEGACY choice (schema 40, default Modern). Identity guards retained.

Build, native input/aim/controls suites (all 21 overlay guards), 100 Python
tests, 36 timing sequences across 30/60/120/180/240/Unlimited and both views,
real menu/save/load/console routes, and Vanilla save/pause checks pass.
[Note 205](205-d04b-control-recovery.md) records evidence and remaining player
acceptance / live death and traversal limits. Candidate:
`31b9ed43bc8738ad4eeb968af629d3be423ff2172a97cfa0d22305766bed950c`.
D17S remains the accepted baseline. No commit/push or player-save changes.
Launch: `python3 recomp/tools/local/run.py`. No F10 step required.


## 2026-10-23 - D19Q2 accepted (Done); D04B queued (Todo)

User accepted the paused-save investigation and explicitly requested commit
and push. [Note 204](204-d19q2-paused-saves.md) retains evidence and limits.
No gameplay changes or rebuild; D17S executable remains the baseline.

New separate report: loading then pressing Escape shortly afterward can lose
Modern controls. D04B is Todo: Modern default, persistent settings-only
Modern/Legacy choice, no implicit Legacy fallback, no F10 scheme switching or
manual recovery requirement, reliable pause/focus/load transitions. Detailed
acceptance is on the board. D04B has not started; choose it explicitly before
implementation. D19Q2 testing does not disprove this new timing report.

Documentation-only closeout authorized as MusicMonsterMod. Player saves/cards
unchanged. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-23 - D19Q2 paused-save investigation (Needs review)

No confirmed regression on the unchanged D17S executable `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
The reported UI slot 10 preserves pause on debug load, but quickload resumes.
SAVE GAME/F2, gameplay quicksave, F3/title loads, F7/debug and Vanilla routes
checked on private copies. Real keyboard loads resume and recapture correctly.
Legacy/debug paused saves without `resume 1` deliberately remain paused.
[Note 204](204-d19q2-paused-saves.md) records the matrix and limits.
No gameplay change, rebuild, player-save modification, commit or push.
Awaiting user review; do not mark Done or perform acceptance closeout yet.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17S accepted (Done)

User fully accepted AUTO and explicitly authorized closeout. D17S is Done.
New Modernized profiles use AUTO: a whole display-refresh divisor capped at
120 FPS; existing choices and Vanilla remain intact.
[Note 203](203-d17s-auto-frame-rate.md) retains evidence and measurement limits.
Accepted executable: `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
No gameplay changes or rebuild. Framework patch re-exported and clean-pin
verification matches all 51 changed/new files. Implementation, tests and
documentation included in the authorized commit/push as MusicMonsterMod.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17S AUTO frame rate (Needs playtest)

AUTO added to FRAME RATE and the launcher, default for new Modernized profiles;
existing saved choices retained. Compatibility-first target is the highest
display-refresh divisor at or below 120 FPS (180 Hz -> 90, 240 Hz -> 120),
unknown display -> 60. Live display changes re-evaluate the target.
Build, compiled rate/setter tests, 88 profile tests, isolated offscreen live
menu transitions and clean-pin patch verification pass. Physical monitor
pacing and user acceptance remain. [Note 203](203-d17s-auto-frame-rate.md).
Candidate: `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9`.
Launch: `python3 recomp/tools/local/run.py --frame-rate auto` (saves AUTO).
No commit or push.


## 2026-10-22 - D17X accepted (Done)

User fully accepted both fixes and explicitly authorized closeout.
D17X is Done: boat-support flat host depth and native warped-quad triangles
for soil floor contact. [Note 202](202-d17x-posts-soil.md) retains test evidence
and limits. No further gameplay changes or rebuild. Accepted executable:
`e5a6a69cf7bf8ff9d72581ee2171b1b8c7f64546f71809b846b0b3de3f3e7b4a`.
Framework patch exported and clean-pin verification matches all 50 files.
Implementation, tests and documentation included in the authorized commit/push
as MusicMonsterMod. No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17X soil revision (Needs playtest)

User confirmed slot 6 fixed. Slot 7 now reproduces as triangular soil cutouts
exposing brighter ground during movement; the earlier nonreproduction note
is superseded. Warped host quads retain the native diagonal and per-triangle
plane depth/contact correction. World ordering and the boat fix are retained.
Build and owned geometry/ordering tests pass, as do the GL batch tests.
Inspected 60/120 FPS, 16:9/4:3 routes and continuous redraw sequences retain
the soil edges; boat, minecart and six older scenes checked.
[Note 202](202-d17x-posts-soil.md) records details and limits.
Candidate: `e5a6a69cf7bf8ff9d72581ee2171b1b8c7f64546f71809b846b0b3de3f3e7b4a`.
Recheck UI slot 7, first person, slowly forward/back and sideways near the
soil corners. F10 if needed. Launch: `python3 recomp/tools/local/run.py`.
No acceptance closeout, commit or push.


## 2026-10-22 - D17X candidate (Needs playtest)

Boat-support white slivers reproduce on private UI slot 6 and are corrected
by honoring opaque host flat/gouraud depth in GL. UI slot 7 soil white corners
did not reproduce on the current baseline; their cause remains unestablished.
Build, compiled GL batch checks and owned near tests pass. Offscreen 60/120,
16:9/4:3 routes cover both saves and the preserved D17W cart; six older scenes
were inspected. [Note 202](202-d17x-posts-soil.md) records evidence and limits.
Candidate: `8ef0ab59d8889f80a1f89e58282c8e30a4c62a602359ebc34d3bd60ae060c4a3`.
Framework patch exported and verified against the clean pin (50 files).
Player saves/profile unchanged. No acceptance closeout, commit or push.
Player check: first person, slowly forward/back at the slot 6 posts and slot 7
soil corners. F10 if capture is needed. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17M1 accepted (Done)

User confirmed the ladder/platform fix and explicitly authorized closeout.
D17M1 is Done on user gameplay acceptance; [note 201](201-d17m1-ladder-platform.md)
retains automated evidence and the scripted traversal limitation.
Accepted executable: `1381185d45a6b0a68b5f8529fe6f4e812e5130c645a9a313319cb0d37186559f`.
No new gameplay changes or rebuild. Framework patch export unchanged;
clean pinned-dependency verification matches all 50 patched files.
Implementation, tests and documentation included in the authorized commit/push.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17M1 candidate (Needs playtest)

Plain ladder upper panels now share depth with the intervening platform while
retaining their original draw ordering. Authenticated type flags select the
correction, not a slot or mesh address. Original textures, Vanilla and climb
controls retain their existing paths. Build, renderer/trace tests and offscreen
60/120/180 camera sweeps pass; older-scene and performance checks recorded in
[note 201](201-d17m1-ladder-platform.md). Player ascent/descent acceptance remains.
Candidate: `1381185d45a6b0a68b5f8529fe6f4e812e5130c645a9a313319cb0d37186559f`.
Use updated UI slot 10 with Corrected textures; F10 if capture is needed.
Launch: `python3 recomp/tools/local/run.py`.
No framework edits, acceptance closeout, commit or push.


## 2026-10-22 - D17M accepted; D17M1 ladder backlog

User fully accepted D17M and explicitly authorized closeout, commit and push.
D17M is Done; accepted executable remains
`8a034f815fc3abf27c27f843dfcc27e01444b4a5e383fa177fe7370ed98412da`.
No rebuild or further gameplay edits. Framework patch export unchanged;
verification against the clean pinned dependency matches all 50 patched files.
[Note 200](200-d17m-platform-ammo.md) retains test evidence and limits.

Brief inspection of a private copy of updated UI slot 10 supports a separate
D17M1 Todo for upper-ladder/platform visibility. The compact-object depth gates
are a plausible lead, not an established cause. Board entry records save hash,
reproduction and acceptance. No implementation started. D19Q2 paused-save
investigation also remains Todo. Earlier D17G/H/I/J triage and D17M code/tests
are included in the explicitly authorized commit/push.

Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-22 - D17M platform ammo correction (Needs playtest)

Current reproduction is UI slot 10. The candidate gives thin overhead
platforms depth and narrowly extends compact depth coverage to walk-over
pickups in Corrected textures. Original textures and Vanilla keep their
existing paths. Owned-EXE renderer tests pass in five settings combinations;
primitive-trace tests pass in both texture modes. Offscreen captures cover
first person at 60/120/180, third person at 120, elevated pickup visibility,
and six older regression scenes. The final 180-target stationary samples
measure 178.6/s at the old table and 180.3/s at the platform, with zero dummy-
audio underruns. User ladder ascent/descent and gameplay acceptance remain.
Player saves/cards/profile unchanged. No framework/generated-source edits,
commit or push. [Note 200](200-d17m-platform-ammo.md) records bounds and evidence.
Candidate: `8a034f815fc3abf27c27f843dfcc27e01444b4a5e383fa177fe7370ed98412da`.

Added D19Q2 Todo at the user's request for recurring paused-save behavior.
It remains deferred; diagnostic loading into a pause menu alone does not
prove a player SAVE/LOAD regression. No other job started.

Load slot 10 with Corrected textures for playtest:
`python3 recomp/tools/local/run.py` (F10 if mouse capture is needed).

## 2026-10-22 - D17J complete, no action necessary

The user confirmed the apartment, alley platform, pavement, club stairs and
bar-stool environments are all stable and explicitly requested completion
with no action necessary. D17J is Done on user gameplay evidence. Frame rate
and camera were not reported; no specific resolving change is established.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. D17M is the next
unfinished D17 report for manual review; implementation has not started.


## 2026-10-22 - D17I complete, no action necessary

After receiving the manual Unlimited camera test instructions, the user
explicitly requested completion with no action necessary. D17I is Done
on user acceptance. No detailed test observations or settings were supplied;
no measured timing improvement or specific resolving change is claimed.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. No next job started.


## 2026-10-22 - D17H complete, no action needed

The user could not reproduce the old train-control and platform/ledge stutters
after receiving the manual test route, reporting "play is totally smooth".
They explicitly requested completion with no action needed. D17H is Done
on user gameplay evidence. Frame rate was not reported; no specific resolving
change is established. No gameplay changes, rebuild or automated tests.
The accepted D12D executable remains the baseline. Documentation only;
no commit or push. No next job started.


## 2026-10-22 - D17G complete, no action needed

The user retested the old strip-club translation-jerk report on the current
player build and could not reproduce it, reporting perfect graphics and
stability in that area. They explicitly requested D17G be marked complete
with no action needed. D17G is Done on user gameplay evidence; frame rate
and camera were not reported, and no specific resolving change is established.
No gameplay changes, rebuild or automated tests. The accepted D12D executable
remains the baseline. Documentation only; no commit or push. No next job started.


## 2026-10-22 - D12D accepted (Done)

User: "fully accepted!! lock it in, and close us out". Upgraded first-person
weapon framing is accepted. Implementation, tests and documentation commit/push
explicitly authorized. Accepted regression baseline:
`bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0`.
[Note 199](199-d12d-upgraded-viewmodels.md) retains tests and coverage limits.
No gameplay rebuild; framework export unchanged and verified against the clean
pinned dependency (all 50 patched files match). No next job started.
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D12D upgraded viewmodels (Needs playtest)

Challenge Stage 4's held record 29 had no first-person framing or firing
seed. The three separate upgraded records now share their base presentation
slots (27/28/29 -> 9/7/8), with original weapon gameplay records untouched.
Candidate `bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0`.
Build and native input/aim/controls suites pass; live offscreen captures cover
stage 4 at 60/120 Hz, stages 23/25, all six campaign upgrades standing/firing/
crouched, and third-person/Vanilla exclusion smoke checks.
[Note 199](199-d12d-upgraded-viewmodels.md) records evidence and limits.
User visual acceptance remains. No framework changes, commit or push.
Launch: `python3 recomp/tools/local/run.py`; P first person, F10 if needed.


## 2026-10-22 - D08K5 and D08K accepted (Done)

User: "total acceptance! lets lock it in and close this one out!!!"
Final coverage accepted; D08K5 and the D08K crouch-walking parent are Done.
Documentation, commit and push closeout explicitly authorized. No gameplay
changes or rebuild. Accepted executable remains
`c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
[Note 198](198-d08k5-crouch-coverage.md) retains verification and coverage limits.
No next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D08K5 coverage ready for final acceptance

D08K5 Needs playtest (final acceptance), D08K still awaiting it. No gameplay
changes; accepted D17R2 executable remains `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
Fresh multi-era/camera, combat, saves, traversal and death/Continue checks,
all three native suites and reviewed Vanilla route pass. Prior terrain
coverage and fixture-only low-ceiling evidence are distinguished in
[note 198](198-d08k5-crouch-coverage.md). User reports extensive happy use.
No request for a full campaign replay. No acceptance closeout, commit/push
or next job started. Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-22 - D17R2 accepted (Done)

User fully accepted D17R2 and explicitly requested closeout. Accepted executable
`c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
The angle-dependent black sky is fixed; [note 197](197-d17r2-angle-dependent-black-sky.md)
records implementation, tests and coverage limits. Implementation, tests and
documentation commit/push authorized. Framework export unchanged; verified
against the clean pinned dependency, all 50 patched files match. No rebuild
for documentation closeout. No next job started; wait for user selection.


## 2026-10-22 - D17R2 sky-pass omission (Needs playtest)

Current UI slot 8 is LEVEL06 room 28. Turning away from a sky-flagged
neighbour culls that neighbour and skips the entire original sky pass.
Modernized now draws the loaded sky background independently of that flag,
after its original far-limit use. No world visibility or Vanilla changes.
Candidate `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
Build, native sky/near contracts, current-slot 60/120 motion and angle sweep,
4:3/third-person/original precision-distance samples and previous sky views
pass. [Note 197](197-d17r2-angle-dependent-black-sky.md) records evidence and
limits. User slot-8 left/up playtest remains. No framework changes or commit/push.
Launch: `python3 recomp/tools/local/run.py` (F10 if capture is needed).


## 2026-10-22 - D08AE accepted; D17R2 queued

User fully accepted D08AE after testing the reported deep-water locations
and shallow water: "all solid". D08AE Done; accepted executable
`c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.
[Note 196](196-d08ae-deep-water-entry.md) retains tests and coverage limits.
Commit/push of authored implementation and documentation explicitly authorized.
Framework export unchanged; patch verified against the clean pinned dependency
and all 50 patched files match. No gameplay rebuild for closeout.

D17R2 Todo: current UI slot 8, look left then slightly upwards; sky turns
black. [Note 197](197-d17r2-angle-dependent-black-sky.md) records source-only
findings, candidate matrix/clipping/packet paths, prior D17O/D17R/D17R1 work
and acceptance criteria. No live reproduction or confirmed cause, no rendering
code changed, no build/launch. Wait for selection before implementation.


## 2026-10-22 - D08AE deep-water entry (Needs playtest)

Deep entry now releases shallow-flight protection when native queried local
volume depth exceeds 640. Original body/neck entry and effects take over;
no new drag constants, velocity writes, hooks or framework edits.
Candidate `c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.
Private slot 9/2 entry matrix and 24 verified-depth routes pass, together with
shelf/slope, gait/crouch, swim-fire/dive/ascent, ORIGINAL and Vanilla checks.
Build, native controls over 21 overlays, input/aim and hook contracts pass.
All 39 original player manifest files unchanged. [Note 196](196-d08ae-deep-water-entry.md)
records evidence, sampling limits and an existing crouch-branch warning.
User entry-feel review remains; D08AD stays the accepted baseline.
No acceptance, commit/push or next job. Launch from this workspace with
`python3 recomp/tools/local/run.py`, test pool UI slot 9 and moat UI slot 2.


## 2026-10-22 - D08AD accepted; D08AE queued

User accepted D08AD: "the animation is fixed. lock it in and close it out."
The accepted executable remains
`9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
Implementation, documentation, commit and push authorized after creating
D08AE: restore deep-water entry resistance and timely swim handoff.

D08AE stays Todo, investigation only. Private current UI slots 9 (pool) and
2 (moat) reproduce submerged airborne descent almost to the bottom under
MODERN. ORIGINAL enters swimming much earlier. Cached support equals water
surface, despite deep native volume: D08AC's shallow-flight bypass predicate
is strongly implicated. No deep-water fix applied; do not automatically start
it. [Note 195](195-d08ad-deeper-water-animation.md) and
[note 196](196-d08ae-deep-water-entry.md). Prior tests of already-swimming
states did not establish entry fidelity. Framework export unchanged and
verified against the pinned clean dependency; no gameplay rebuild needed.


## 2026-10-22 - D08AD deeper-water animation fix (Needs playtest)

Walking's terrain probe invalidated the D08AC patch scope before native
dispatch, resetting the walk clip every update. A separate native completion
selector restarted the same running stride half. Arm the scope after the
probe and retain native land stride choices at completion. No physics or
backpack-specific changes. [Note 195](195-d08ad-deeper-water-animation.md).

Candidate `9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`:
full 72/74 and 76/78 cycles at 60/120 Hz, private updated slot-10 route and
visual captures; native controls over 21 overlays, input, aim and hook-contract
tests pass. Paired jump heights, crouch, deep swimming, ORIGINAL and Vanilla
regressions checked. All original save/card hashes unchanged. User playtest
pending; no acceptance, commit or push. Existing framework edits preserved.


## 2026-10-22 - D08AD queued (backlog/source investigation only)

Added one Todo follow-up for frozen third-person walking and abbreviated
running in the user's updated deeper-water UI slot 10. D08AC movement/jump
physics and D08K11 crouching remain accepted. [Note 195](195-d08ad-deeper-water-animation.md)
records depth classification, legacy 80/81 conversion and possible repeated
track resets; these are source findings and hypotheses, not a reproduced
root cause. No gameplay edits, build, launch or save/card access. Implement
only when selected; no commit/push authorized by this backlog request.


## 2026-10-22 - D08AC accepted (Done)

User: "perfect!!! lock it in and close us out". Grounded shallow/mid-water
land movement, jumps, air control and landing accepted. SWIMMING MODERN
uses shared land mechanics; ORIGINAL retains its legacy wade jumps and
ledge assistance. Deep swimming and Vanilla remain unchanged.
[Note 194](194-d08ac-shallow-water-locomotion.md) records implementation,
paired traces, native suites and live regressions. Executable
`30ca9b3fa5bd0a6350dd8b7ef1a509914ecfb3e60dcf74ec9ebba0982b7eb838`
is the accepted regression baseline. Broad campaign/shoreline coverage
remains an ongoing regression concern, not a claim of exhaustive testing.
Implementation and documentation commit/push authorized. No next job started.


## 2026-10-22 - D08K11 accepted (Done)

User: "awesome! i accept the behaviour here, very good. lock it in and close it out."
Shallow/mid-water Modern crouch and crouch walking accepted; documentation,
commit and push authorized. [Note 193](193-d08k11-shallow-water-crouch.md).
Executable `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8`
is the accepted regression baseline. D08K5's broad collision/campaign sweep
remains separate. No next gameplay job started.

Closeout CI sweep: [run 39793587393](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/runs/39793587393)
failed importing `ttk_anim` because NumPy was not installed. Local requirements
now include NumPy/Pillow and CI installs them before testing. A clean source-only
checkout with a fresh Python environment passes 180 tests (23 owned-input/tool
skips), the media-validator build, and all three framework regression scripts.
The exported framework patch is unchanged and applies idempotently to the pinned
clean dependency. This CI fix changes no gameplay or accepted executable.


## 2026-10-22 - D08K11 shallow-water crouch (Needs playtest)

Modern crouch/crouch walk now works in grounded shallow and mid-depth water,
including native mode 1; the original mid-water stand-up veto is bypassed
only during an owned crouch dispatch, then restored. Deep-water handoff
uses the original clearance-checked stand-up and native swim rules.
Build and native input/aim/controls pass; private slot 10, direction/fire/
stand-up checks, both cameras and water options, controlled depth fixtures
and separate swimming/Original/Vanilla regressions completed. Natural
shoreline and player feel review remain. [Note 193](193-d08k11-shallow-water-crouch.md).
Candidate `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8`.
D08K10 remains the accepted baseline. No commit/push or next job.


## 2026-10-22 - D08K10 accepted (Done)

User: "perfect!! lock it in, close it out. great work". First-person crouched
weapon alignment, including revision 2's high-refresh crouch walking, accepted.
Documentation, commit and push authorized. [Note 192](192-d08k10-crouched-viewmodel.md).
Executable `72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`
is the accepted regression baseline. No next job started; no rebuild needed
for documentation closeout.


## 2026-10-22 - D08K10 revision 2: replay crouch poses (Needs playtest)

User confirmed idle but reported bad weapon poses while crouch walking.
Reproduced at 120 Hz: render workers lacked the crouch eligibility state
and used stale fork-time values. Each published frame now includes it.
Player build and native controls pass, including a forked-worker snapshot
regression. Actual presented captures verify all six weapons while moving
forward/sideways/backward. The earlier high-refresh verification claim
was too broad; this revision checks the worker's visible output.
[Note 192](192-d08k10-crouched-viewmodel.md). Candidate
`72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`.
User retest pending. No framework edit, commit/push or next job started.


## 2026-10-22 - D08K10 crouched first-person weapons (Needs playtest)

Animated weapons and held items now read the right arm's animation channel
for the eye-view copy during Modern crouching, avoiding the crouch body's
shoulder compensation. Standing and seeded guns retain their framing;
third-person animation, Original crouch and Vanilla paths are unchanged.
Build and native input/aim/controls pass; private 60/120 Hz weapon checks
completed. User review of appearance and transitions remains.
[Note 192](192-d08k10-crouched-viewmodel.md). Candidate executable
`fd7ed0ef639a96d79cfdb27b4cf18e2c72e3128a856575e125198f4418abced0`.
D08K9 remains the accepted baseline. No commit/push or next job started.


## 2026-10-22 - D08K9 accepted (Done)

User: "lock it in, close it out!" Held crouch on touchdown and the final
CROUCH-toggle/menu revision accepted. LANDING is hidden from Advanced;
crouch-on-landing follows CROUCH alone. Documentation, commit and push
closeout authorized. [Note 191](191-d08k9-crouch-on-landing.md). Executable
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`
is the accepted regression baseline. No next job started.


## 2026-10-22 - D08K9 crouch on landing (Needs playtest)

Crouch held through a jump takes over in the soft touchdown update, with
a flight-to-kneel/walk blend and no recovery wait or intermediate gait.
Private slot 4: 24/24 cases across both jump styles/cameras at 120 Hz;
60 Hz follow-ups, release/late press and damaging-fall fixture pass. Native
input/aim/controls and player build pass. User review of feel is pending.
[Note 191](191-d08k9-crouch-on-landing.md). Candidate executable
`5bff834e83785cb7d7023001198aebba1d66df881de7d05cbe13d051b081b091`.
D08K8 remains the accepted baseline. No commit/push or next job started.
Functional behavior accepted by the user; follow-up hides LANDING from
Advanced and makes crouch-on-landing follow CROUCH alone. Build, native
controls and live LANDING ORIGINAL check pass. Revised candidate
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`.


## 2026-10-22 - D08K11 opened (Todo, backlog only)

[D08K11: shallow-water crouching](../MODERNIZATION_JOBS.md#d08k11---enable-modern-crouching-and-crouch-walking-in-shallow-water)
records the user's slot 10 flooded-subway report and a source-only investigation.
Crouch entry and walk eligibility explicitly reject the broad water predicate;
176 also serves as a water-dip animation. Depth/state, collision, input ownership
and head-submersion handling need a later private-copy runtime investigation.
No gameplay changes, build or launch; the D08K8 accepted build remains current.
Include D08K11 before D08K5's final coverage; no implementation job started.


## 2026-10-22 - D08K8 accepted (Done)

User: "this is perfect!! lock it in, close it out". Stationary kneel and
transitions accepted; closeout authorized. [Note 190](190-d08k8-stationary-kneel.md).
Executable `d798757cb2de56f946074165423faea379a1711d51f81b60cf00c70d8780e1f3`
is the accepted regression baseline. Next suggested: D08K9, D08K10, then D08K5.
No next job started; no rebuild needed for documentation closeout.


## 2026-10-22 - D08K8 stationary kneel (Needs playtest)

CROUCH MODERN now holds the original one-knee pose, blending from the last
shown pose into and out of walking. Saved animation tables refresh on load.
Build and focused tests pass; private 60/120 Hz checks completed. User visual
acceptance and unarmed gameplay review remain. See [note 190](190-d08k8-stationary-kneel.md).
Executable `d798757cb2de56f946074165423faea379a1711d51f81b60cf00c70d8780e1f3`.
No commit or push; D08K7 remains the accepted baseline.


## 2026-10-22 - D08K7 accepted (Done)

- User: "confirmed working! lock it in and close it out"
- D08K7 Done (note 189). Executable
  `a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963` is the regression baseline. Next suggested: D08K8, D08K9, then D08K5.

## 2026-10-22 - D08K7 stuck crouch on objects (Needs playtest)

- Slot 10 dumpster: crouch walks over the top and stops at the edge where the
  standing walk stops; releasing Ctrl stands Duke; W never rolls in CROUCH
  MODERN. Objects are judged as the original walker (`0x8007926c`) judges
  them; a refused stand-up keeps the crouch walk on the directions and stands
  Duke once there is room. Note 189. Executable
  `a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963`.
- Next suggested: D08K8 (kneel as the still crouch), D08K9, then D08K5.

## 2026-10-22 - D08K6 accepted (Done)

- User: "that's it!! i'm happy with this end to end. the anim is now fully complete. the technical implementation is now fully complete. this is totally playable in every way at this point. lock it in, close it out"
- D08K6 Done (note 188). D08K waits only on D08K5. Executable
  `789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03` is the
  regression baseline.

## 2026-10-22 - D08K6 revision 4: variant C, head up, hunched firing (Needs playtest)

- Default crouch is variant C. Head looks ahead (an arm-range bug left it on
  the kneel's head when armed). Firing keeps the hunch: the chest's aim
  look-at is tilted down 50 degrees and the shoulders compensate, so the gun
  stays on the crosshair. Note 188 revision 4. Executable
  `789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03`.

## 2026-10-22 - D08K6 revision 3: crouched shots aimed up fixed (Needs playtest)

- While firing, the original aim build points the chest at the view; our lean
  and arm compensation on top tipped the gun 49-58 degrees up. The upper body
  now uses its channels' own pose while the aim bit is set. Crouched fire is
  level, also while crouch walking. Note 188 revision 3. Executable
  `cd48d3e3d7cd5f0b35f3364b2179d0a4209b1ca3ac79b67ec6354312f92db900`.

## 2026-10-22 - D08K6 revision 2: arms straight down after crouch fixed (Needs playtest)

- Armed crouch left both arms at identity in every later animation: channel
  joint masks grew (left arm/chest channels gained the right arm and chest)
  and the weapon-arm compensation compounded on stale records. Masks trimmed
  to each channel's original joints; weapon arm from its channel frames.
  Note 188 revision 2. Executable `6ca0122ee58dce894263394fa0d1ba7623763e8bba5aa36f6967bf9c6d77dbcf`.

## 2026-10-22 - D08K6 tactical crouch walk (Needs playtest)

- New crouch-walk look: hips higher (0.62 leg lengths), feet under the hips,
  chest 48 degrees over the knees, head level, fists up front; upper body
  written after every blend, so the armed left-arm shake (animation 178's
  upper channels churning) is gone. Note 188 also records everything needed
  for future custom animations and models. Variant packs a/b/c/d under
  `recomp/analysis/d08k6/packs/` for `DNTTK_ANIM_PACK`.
- Executable `dda1d28f2626ffa5b0464d0adcc83bb99483dc3879ef9e1009069d342382a3be`.
  Limits: pistol idle hangs at the thigh; first person not judged.

## 2026-10-22 - D08K1-D08K4 accepted (crouch walking, technical)

- User: "now it's functionally working. the job is fulfilled on a technical
  level ... close out the done tasks". D08K1-D08K4 Done (notes 184-187).
  Executable `993c2c705d1f86fa27c1608ed92ba0574a1ade50097c909918b656b65e1c46c5` is the regression baseline.
- D08K6 opened (Todo): crouch-walk look (Solid Snake, not "on the toilet") and
  the slight left-arm shake while crouch walking.

## 2026-10-22 - D08K4 Modernized crouch walking (Needs playtest)

- CROUCH MODERN (default) walks low with held crouch plus a direction, Duke
  facing the view; CROUCH ORIGINAL rolls; Vanilla unchanged. Profile schema 39.
  Clips built from the disc at build time. Note 187. Executable
  `862d7b9b8a98c91c55427ada9a49e402ef2d3083be99450a5e7475e02fc81c32`.

## 2026-10-22 - D08K3 crouch-walk loops (Needs review)

- New IK gait for a crouch walk (the original has only a kneel): forward,
  backward and strafes, planted feet (slide under 2 units vs 23-40 in the
  original walks), footstep events, generated locally from the disc by
  `ttk_crouch_walk.py`. In-game preview by pack; Blender file for review.
  D08K2 hull raised to 0x2c0 for the walk's 654 height. Note 186.

## 2026-10-22 - D08K2 crouch-height collision (Needs review)

- A crouch probe asks the original move queries (object probe, floor and
  ceiling, head ray, slope) with a 0x2c0 crouch hull instead of the standing
  0x370, refuses steps up or drops over 0x100, and follows the floor. Debug
  console `crouchwalk` / `crouchgo` (test mover) and `crouchscan`; Modernized
  only, nothing changes without them. Stand-up keeps the 0x370 rule.
- Finding: no crouch-only ceiling or overhang at any selectable level start
  (levels 0-3, 5-12); the walkthrough agrees, so the hull is a safety rule and
  the feature's value is precise combat movement. Live: walls, props, curbs,
  stairs and two edges behave; clearance gate shown with a test hull; native
  fixture covers the crouch-only case. Note 185. Executable
  `3e58625a743856ff8e1832cfc5baac9c86e5d797b80cf2fa21e1f94e6d61b8e0`.

## 2026-10-22 - D08K1 animation pipeline (Needs review)

- Duke's skeleton, meshes, textures and animations are decoded from the disc
  (HIE/MOD/ANM/FRM/ROT, A/B.RAW) by `recomp/tools/local/ttk_anim.py`; every
  clip of DB00-DB09 (9,481) re-encodes byte for byte. An animation's four
  clips are body channels (legs/root, right arm, left arm, chest+head), so a
  crouch walk needs only a new legs clip.
- Renders and offline FK match live memory (joints within 4.4 units, blended
  quaternions within 8/4096). Blender bridge (`ttk_anim_blender.py`) exports
  rigged, textured Duke with clips as actions and imports edits back.
- Runtime override packs (`recomp/src/ttk/anim_override.cpp`, Modernized
  only, `DNTTK_ANIM_PACK` or `ttk-anim.pack`; none ships): a Blender
  re-encoded idle plays within 2.39/4096 of the original; a demo pack makes
  Duke idle kneeling; savestates adopt or restore it. Vanilla unaffected
  (route reviewed). Note 184. Executable
  `5998cf36936d588c5433f3cc5e9cc001308a8717e78a4b9388ec029f3f6d3cfe`.

## 2026-10-22 - D20E accepted (ENEMIES KILLED X OF Y)

- User: "lock it in, this is complete. close it out". D20E Done. Note 183. Executable
  `156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef` is the regression baseline.

## 2026-10-22 - D20E statistics research: ENEMIES KILLED X OF Y (Needs playtest)

- The statistics screen's kills line now reads `X OF Y`. Y uses the level
  load's own rule for its enemies-left count (placement table, same totals on
  7 levels and on every difficulty); the live count is not used because
  dancers are counted but never subtracted. Kills survive death and Continue.
- Accuracy dropped by the user (shots `+0x3b2` and hits `+0x3b4` documented
  only); damage taken parked (no original counter). Note 183.
  Executable `156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef`.

## 2026-10-21 - D18D accepted (music after Continue)

- User: "i can confirm the music started with no further issue. lock it in,
  close it out". D18D Done. Note 182. Executable
  `23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a` is the regression baseline.

## 2026-10-21 - D18D music after Continue (Needs playtest)

- Cause: the runtime finished the near CD seek at Continue (drive paused a
  few sectors from the music's resume point) in 0x800 cycles, inside
  libcd's `CdControl`, before Time to Kill installed its seek callback, so
  the music player stuck in state 1, CD muted, until a voice line.
- Fix in the framework (`cdrom.c`, exported patch): near SeekL/SeekP use
  Beetle's seek time (min 20000 cycles, plus the paused-drive restart).
  Modernized and Vanilla resume music two frames after CONTINUE; level-start
  voice-to-music and intro FMV unchanged; about 37 ms more per level load.
  Not yet heard on the real audio device. Note 182. Executable
  `23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a`.

## 2026-10-21 - D11G cancelled (cannot reproduce)

- User: "cancel the job. i cant seem to replicate it either now. if it comes
  back i'll let you know but close it as cant replicate". No code changed.
- Six routes from a private copy of UI slot 11 all started the next level in
  first person with no input (debug load, pause LOAD GAME, title LOAD GAME,
  F3, statistics SAVE GAME then CONTINUE; the player's profile at 120 fps in
  a window and on the real GPU). If it returns, keep the session log: the
  likely causes are capture-related (focus lost during the load, an F10
  opt-out) or a level-start state that holds the camera lease inactive.
  Regression baseline unchanged:
  `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.

## 2026-10-21 - D20D accepted (richer statistics); D11G opened

- User: "accepted! ... close this work out, fully accepted". D20D Done.
  Note 181. Executable
  `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d` is the regression baseline.
- D11G (Todo, not actioned; next up): a new level can start in third person
  although first person is selected (player UI slot 11, next level); a left
  click brings first person back.

## 2026-10-21 - D20D richer statistics screen (Needs playtest)

- Approved layout: level name, DIFFICULTY:, kills, time, BEST TIME (NEW
  BEST! when beaten), continues, secrets, MISSION ITEMS, challenge item,
  HEALTH / ARMOR. Best times per level and difficulty in
  `ttk-best-times.txt` beside the savestates. `!` and `%` now drawn in the
  game's own small font; the font picker shows ( ) % and the double quotes
  (section 16). Offscreen checks pass, Vanilla unchanged. Note 181.
  Executable `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.

## 2026-10-21 - D20F accepted (statistics screen list)

- User: "thats so much better! lock it in, close it out, 10/10". D20F Done.
  Note 180. Executable
  `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496` is the regression baseline.
  D20D (richer statistics, builds on this layout) and D20E stay Todo.

## 2026-10-21 - D20F statistics screen list (Needs playtest)

- CONTINUE and SAVE GAME are a selectable list under the statistics lines
  (lines moved up 25); Up / Down / wheel, Enter chooses, V still saves;
  Escape or a save returns on SAVE GAME; CONTINUE only before the final
  ending. The original's wait is about 1 s, not 5 s (note 179 corrected).
  Offscreen checks pass, Vanilla unchanged. Note 180. Executable
  `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496`.

## 2026-10-21 - D20B accepted (statistics screen); D20F opened

- User: "i can confirm its now working and positioned correctly with the overlap ... i think we can close out the current work, just to break this task down a bit, as it is indeed working fine now." D20B Done. Note 179. Executable
  `f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.
- D20F (Todo, not actioned): CONTINUE and SAVE GAME as a selectable list on
  the statistics screen (V is legacy and not discoverable on keyboard and
  mouse). D20D (richer statistics) and D20E (research) stay Todo; D20D now
  builds on D20F.

## 2026-10-21 - D20B first playtest: SAVE GAME date overlap fixed (Needs playtest)

- User: works functionally; on SAVE GAME from the statistics screen the date
  overlapped the line under the picture (their 1444x1020 window). The host
  image used the 16:9 fit while menu frames there are presented at 4:3; it now
  follows the presented rect (framework, patch exported). Also covers the
  title LOAD GAME in tall windows. Note 179. Executable
  `f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.

## 2026-10-21 - D20B statistics screen rebuilt (Needs playtest); D20D, D20E opened

- Option B, part 1 (user: remove the built-in save, "lets go for option B"):
  after a one-player level the statistics screen is ours in the original's
  look; V (SAVE) opens SAVE GAME (savestate slots), never the memory card;
  CONFIRM goes on. Saves there show the level's last picture and LEVEL
  COMPLETE on LOAD GAME and load back onto the screen. Vanilla and two-player
  unchanged. Hooks `0x8001FCBC`, `0x800270E8`; codegen hash unchanged.
  Note 179. Executable
  `b217e8898cbfa52a2baabe0efa01d13a3cbea83d9b016950fff636a6757bbbf4`.
- Testing aid: console `endlevel` ends the level as its exit does. Executable
  `c4bc40729ab7a1efaf7ac90b3ec8492bbdc7bd2a7ad2c9cf123c574a182bc82a`.
- Playtest: backtick, `endlevel`; wait for the prompts, V to save, Escape /
  save back, Enter to continue; then load that slot from LOAD GAME.
- D20D (richer lines, best times) and D20E (enemy total, accuracy, damage
  research) opened, not actioned.

## 2026-10-21 - D20C and D20A accepted

- User: "rock solid. it's working! the artifacting is no longer present upon
  load. i recreated the steps here and it's all fine. lock it in, close it
  out." D20C and D20A Done (one cause: savestates now carry mod memory).
  Note 178. Executable
  `03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`.

## 2026-10-21 - D20C wrong colours after a load fixed (Needs playtest); D20A same cause

- Cause: savestates did not carry mod memory (Expansion 1 and the GPU-DMA
  aperture). The first-person near clip's packet arena is in the aperture and
  linked from the game's ordering table, so a load could walk another frame's
  packets: scrambled textures, wrong colours kept in VRAM, or a GP0 halt.
  D20A's halting list `0x80F40BD0` is in that arena: same bug.
- Fix: framework section `BS_SEC_MODMEM` (restored on matching layout; the
  aperture is otherwise filled with end-of-list headers, also for older
  saves); near clip reserves its arena at startup and resumes past the
  loaded region. Codegen hash unchanged. Note 178. Executable
  `03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`.
- Playtest: load the player's slot 1 several times (F9 / LOAD GAME), in first
  person; also F6 right after F9, restart, load.

## 2026-10-21 - D20 accepted (quick save and quick load); D20C opened

- User: "its all working!" D20 Done. Note 177.
- D20C (Todo, not actioned): colours sometimes wrong after loading a
  savestate (their slot 1, intermittent; "try loading several times").

## 2026-10-21 - D20 quick save and quick load (Needs playtest); D20A, D20B opened

- EDuke32-style, Modernized one-player: F6 quick save over the last slot
  used (saved to or loaded from; none at launch: SAVE GAME opens), F9 quick
  load (no slot: LOAD GAME), F2 / F3 open SAVE / LOAD GAME from play (Escape
  back to play), console `quicksave` / `quickload`, KEY BINDINGS SAVES rows,
  launcher schema 38. F6 port swap off in Modernized. Note 177. Executable
  `20f8edc1d1d6260e59747df7d31ae4a438f4081a8eb19ee402ea9924f1b3e930`.
- D20A (framework, pre-existing): a save within ~1.5 s after a load can halt
  the emulator when loaded after a restart (GP0 0xFA). D20B: the original
  memory card saves, split from D20.

## 2026-10-21 - D19T1 accepted (no quotes on DELETE SAVED GAME:)

- User: "perfect. again lock it in close it out." D19T1 Done.

## 2026-10-21 - D19T1 no quotes on DELETE SAVED GAME: (Needs playtest)

- Ad hoc user request: the slot name line on DELETE SAVED GAME: (LOAD GAME
  and SAVE GAME) is drawn without quotes. D19T quote glyphs kept. Built, not
  launched.

## 2026-10-21 - D19T accepted (real double quotes)

- User: "that's awesome. lock it in close it out." D19T Done: real opening
  and closing double quotes in the menu font and the font pack (steel name
  line on DELETE SAVED GAME:). Note 176.

## 2026-10-21 - D19T real double quotes in the menu font (Needs playtest)

- `"` in Modernized menu text draws as real double quotes in both menu
  sizes, built from the font's own apostrophe (opening one turned half
  round), and in the font pack (Console steel: the DELETE SAVED GAME: name
  line, after the user's first playtest). Test with `DNTTK_TEST_MENU_TEXT`.
  Note 176. Executable
  `52f5f1a3ae67040f6e9d52d544ba914e7115c1e38985a23ce066937c22aea29c`.

## 2026-10-21 - D19S accepted (delete saves from LOAD GAME); D19T opened

- User: "perfect. lock it in." Delete on a used LOAD GAME / SAVE GAME slot
  asks DELETE SAVED GAME: with the name in Console steel (NO first); YES
  removes only that slot's files. Note 175.
- D19T opened (Todo): real double quote marks in the game's menu font.

## 2026-10-21 - D08Z5 accepted

- User: "perfect!!! lock it in close it out." D08Z5 Done; executable
  `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da` is the regression baseline.

## 2026-10-21 - D08Z5 second playtest: slope jump climbs (Needs playtest)

- User: "i dont like how this feels ... i prefer the idea of when jumping again, duke is actually trying to jump up a gradient slope a little higher ... What we've got right now is that dukes jump feels almost cancelled out". The Modern slope jump (98) never rose: Duke took off
  pressed into the face, so the air sweep `0x8007a98c` called it floor on the
  first update and landed him. Modern now keeps the jump's rise, drifts up the
  slope only as fast as the arc clears the face (k 0.25 of the rise against
  the face's tilt `+0x2be`), and while rising tests each update with the
  original sweep (drift, straight up, or lifted up to 160 units). A jump at
  the bottom of the face gets the same rise. Slot 10 car: jumps rise about
  430 units and land 90 to 390 higher on the face; tapped runs ended on the
  car in 3 of 6. Executable `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da`. [Note 174](174-d08z5-wall-backflip.md).

## 2026-10-21 - D08Z5 first playtest: backflip still there, fixed (Needs playtest)

- User: "this absolutely doesnt seem to have provided any kind of fix ...
  there is even a chance duke will slip out of first person". The player's
  logs showed animations 99/100, not the launch the first delivery removed:
  Jump pressed while Duke slides on the car face, facing up the slope, makes
  the slide handler's jump branch (`0x800547ac`) start the backflip 99/100
  (up to about 2400 units back). The first harness only tapped Space before
  the landing, so it never reached it. Modern now starts the original's own
  slope jump 98 in Duke's facing instead (armed by the branch's facing test
  `0x8005446c`, ra `0x800547fc`; rewritten at the initializer `0x800493a4`).
  The camera lease now also covers 99/100 from mode 0 or 9 (it dropped first
  person in ORIGINAL). Mashing Space into the slot 10 car: 18 Modern runs, no
  99/100, 17 slide jumps turned into 98; ORIGINAL keeps the backflip with the
  camera held. Executable `cd458c38c3ab3b22658ef1b0896fc3bed36df2cf02a8976ce391d2514bd261d2`. [Note 174](174-d08z5-wall-backflip.md).

## 2026-10-21 - D08Z5 wall backflip option (Needs playtest)

- The "backflip" off the slot 10 police car is the original steep-slope
  slide (mode 2, slip pose 145) followed by a backwards launch from the fall
  table `0x800c27c8` when the face ends (525 to 812 units back). New
  ADVANCED > MOVEMENT > WALL BACKFLIP, MODERN (default) / ORIGINAL: Modern
  keeps the slide but zeroes that launch (`wall_backflip.inc`), so Duke drops
  where the face ends (70 to 410 back). Profile schema 37, `run.py
  --wall-backflip`, Vanilla original. Hooks `0x8005446C`, `0x8003F060`
  regenerated (codegen hash unchanged). Executable
  `191646ef67d6890ec435ddc771a9f31c9da349eb6e83d8a8c5b341860a4d3647`.
  [Note 174](174-d08z5-wall-backflip.md).
- Separate finding: the player's slot 10 save halts the emulator on some
  reloads in one session (GPU GP0 unknown command). Not investigated.

## 2026-10-21 - D17R1 accepted

- User: "lock it in and close it out". D17R1 Done; executable
  `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f` is the
  regression baseline.

## 2026-10-21 - D17R1 first playtest: redraws broken, fixed (Needs playtest)

- User: "quite a lot of graphically breaking issues". The redraw workers'
  ordering-table walk ended at any bit-23 link, and every new host-packet
  arena address has bit 23 set, so high-refresh in-between images lost
  everything after the sky. Framework `render_replay.c` now ends the walk as
  the live GPU walker does. Moving replay sequences checked whole. Executable
  `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f`.
  [Note 173](173-d17r1-sky-blocks.md).

## 2026-10-21 - D17R1 sky cloud blocks fixed (Needs playtest)

- The stair-stepped holes in the clouds came from the near clip's packet
  budget: with the extended draw distance the game filled 61% of its render
  ring, the near clip's 60% cap was spent, and the sky (drawn last) fell back
  to the original path, which drops its near cloud quads. The near clip now
  has its own packet arena in the GPU DMA aperture; framework `gpu.c` keys
  those addresses apart from RAM. 0 fallbacks in the canal and four sweeps.
  Camera jerk not reproduced. Executable
  `d632379685d59dd1fa953f088459ac69af1bff6022ef5bd8886ac2a0bbf27f03`.
  Awaiting the user's playtest. [Note 173](173-d17r1-sky-blocks.md).

## 2026-10-21 - D17R accepted; D17R1 and D17X opened

- User: "Perfect!! Lock this in." D17R Done; executable
  `fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d` is the
  regression baseline.
- New Todo jobs: D17R1 (sky artifacting when looking up/down and turning,
  medieval UI slot 2 with possible camera jerk, UI slot 8) and D17X (small
  white triangles on boat posts, UI slot 6, and grave-dig soil, UI slot 7, in
  first person; backlog, not started).

## 2026-10-20 - D17R sky edges in widescreen (Needs playtest)

- Cause: the sky routine runs after the root rectangle is back to 4:3, so its
  sky-colour fill, camera-facing backdrop and clip rectangle stopped at the
  original picture's edges in 16:9. `sky_widescreen.cpp` widens them by the
  widescreen margin; 4:3 and Vanilla unchanged. Executable
  `fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d`.

## 2026-10-20 - D26G accepted

- User: "lock it in and close it out!!". D26G Done.

## 2026-10-20 - D26G console key repeat (Needs playtest)

- Cause: the console dropped SDL key repeat events for Backspace and
  characters. Fixed in the framework (exported patch). Executable
  `5ac80ba1394f8942b4998159d79fd515e09e959d73aea456e65b4eb95b7b03da`.

## 2026-10-20 - D17W accepted

- User: "that cart is looking handsome!!". D17W Done; executable
  `8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c` is the
  regression baseline.

## 2026-10-20 - D17W minecart fixed (Needs playtest)

- Cause: the cart lays a flat white F4 on the corners of its textured back
  panel. The original paints the texture last in one slot; the eye-view near
  clip sorted each piece by its own depth and flat pieces have no depth test,
  so white triangles painted over it. Coplanar overlapping faces of a mesh
  now keep the original's paint order (near clip, no framework change).
  Executable `8169621316e555cd08b72367c835c1189e9a8d5ae68216497e2b04391cf4700c`.

## 2026-10-20 - D19Q1 accepted

- User: "confirmed working!". D19Q1 Done.

## 2026-10-20 - D19Q1 fixed (Needs playtest)

- Cause: `savestate_grab_thumb` read CPU VRAM without syncing the OpenGL
  renderer, so the pause picture was the image at the previous save. Fixed in
  the framework (exported patch); reproduced before, clean after. Executable
  `92178369fc5153178009edf801bd5c07299fee5d6d9bab927ea6ca2b6fc3e043`.

## 2026-10-20 - D19Q1 opened (SAVE GAME thumbnail shows the menu)

- Player UI slot 1's picture is the OVERWRITE THIS SAVE? page; first person
  pauses on a third-person view, untested by D19Q. Research in the job.

## 2026-10-20 - D19Q accepted

- User: "im happy with this". D19Q Done; executable
  `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b` is the regression baseline.

## 2026-10-20 - D17W opened (minecart white triangles)

- New Todo D17W: player UI slot 10, the overturned minecart shows white
  untextured triangles in first person only (also with GEOMETRY ORIGINAL;
  clean in third person). Leads in the job (D11B near clip first).

## 2026-10-20 - D19Q SAVE GAME (Needs playtest)

- Pause list SAVE GAME opens LOAD GAME's page with every slot open; empty
  saves at once, used asks OVERWRITE THIS SAVE? (NO first); SAVED TO SLOT NN
  or THE GAME COULD NOT BE SAVED. Thumbnail = play when the pause opened;
  LOAD GAME on such a save resumes play. Framework: atomic slot writes,
  thumbnail grab / write. Identity guard no longer covers the card-restored
  GAME SETTINGS values. Executable
  `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b`,
  [note 171](171-d19q-save-game.md).

## 2026-10-20 - D19R done: startup card wait at host speed

- The user delegated testing and closeout. States 1 and 2 (card probe, file
  read) run their VBlanks unpaced with the framework idle skip on for that
  window only (new `psx_mod_request_unpaced`, exported patch; codegen hash
  unchanged). States 1-2: about 1.9 s to 0.24 s; black after the movies about
  0.35 s. Settings still load; blank and fresh cards continue; Vanilla
  unchanged. Executable `2fb1bd3cc0e8ee78fd94c41da6c66c39a5fe7d5ff4eddd813e321bc2f2833408`,
  [note 170](170-d19r-card-wait.md).

## 2026-10-20 - D19K accepted; D19R opened

- User: "confirmed now working as intended. lock it in." D19K Done; executable
  `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9` is the regression baseline.
- New Todo D19R (startup card probe hang, research in the job): state 1's
  card probe blocks the main loop for about 1.5 s (90 fields) in the BIOS card
  routines, black screen; options listed (unpaced card wait, keep the title
  image, defer the probe, host settings read).

## 2026-10-20 - D19K revision 2: LOAD GAME opens on slot 01 (Needs playtest)

- User: "superb, everything is working as intended!"; LOAD GAME now opens on
  slot 01 instead of the newest save. Executable `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9`.

## 2026-10-20 - D19K revision 1: steel text, startup card check unseen; D19Q opened (Needs playtest)

- LOAD GAME's save time and the in-game warning (now UNSAVED PROGRESS WILL BE
  LOST) in Console steel TTK Medium Italic with its shadow, drawn by the host
  with the thumbnail. Modernized startup no longer shows CHECKING MEMORY CARD,
  CHECKING SAVE GAME FILE or the no-card message; the card reads and the
  settings unpack still run (hook `0x8008B454` regenerated, codegen hash
  unchanged). New Todo D19Q (SAVE GAME screen to match). Executable
  `2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`, [note 169](169-d19k-load-game.md).

## 2026-10-20 - D19K LOAD GAME from our savestates (Needs playtest)

- LOAD GAME on the title, pause and death lists now opens our page (layout A
  of the mockup): the 12 savestate slots by level name, newest selected, the
  chosen slot's thumbnail, era and save time beside them, then LOAD THIS
  GAME? and a load like F7's. Empty and other-build slots are greyed; a failed
  load keeps the game and says so. The memory card LOAD stays Vanilla's.
  Framework: thumbnail draw in both present paths; `boot_state_check_buffer`
  no longer passes every slot when given no reason buffer. Offscreen loads
  from pause, title and death; Vanilla original; tests pass; patch verified
  against the pin. Executable `2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`,
  [note 169](169-d19k-load-game.md).

## 2026-10-20 - D08Z4 accepted

- User: "thats the one! lock it in and close it out!!". D08Z4 Done; executable
  `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778` is the regression baseline.

## 2026-10-20 - D08Z4 falls keep the eye view (Needs playtest)

- Slot 6 cop car: the original slope slide on the car body alternates 145/108/105,
  and each 108 counted as an unowned fall, which first person showed from the
  orbit (the "brief third person"). Unowned falls (107/108) now stay in the
  eye, game-wide; mantles, hangs and climbs keep the orbit. 40 scripted jumps
  (gaits, headings, both jump styles, both wall-contact modes) stay in the eye;
  E-held mantles still use the orbit; third person and Vanilla unchanged;
  native suites pass. Executable `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778`,
  [note 168](168-d08z4-falls-keep-eye.md).

## 2026-10-20 - D34 accepted

- User: "amazing read". D34 research Done; follow-ups D34A-D34G stay
  proposed in note 167 for another day.

## 2026-10-20 - D34 multiplayer research (Needs review)

- Research only (no code, build or game run). [Note 167](167-d34-multiplayer-research.md):
  two-player mode is one shared simulation drawing one viewport per game
  frame; our build has no port-2 pad (`MAX_PLAYERS 1`) and every Modernized
  system is player-1-only, so two-player play falls back to the original
  controls and presentation. Recommended route: deterministic input sync of
  the game's own two-player mode (framework `recomp-net`), own view full
  screen through D17 replay, self-hostable relay. Proposed follow-ups
  D34A-D34G await the user's review. Executable unchanged
  (`6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33`).

## 2026-10-20 - D19N accepted

- User: "lock it in and close it out!!". D19N Done; executable
  `6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33` is the
  regression baseline. Backlog: D19K, D19F, D26G.

## 2026-10-20 - D19N review pass (Needs playtest)

- Fixed the title CONTROLS screen and ADVANCED going blank when a slider was in
  view at a high frame rate (slider objects now in guest RAM, the original
  SOUND screen's). GAME SPEED's slider has room; AUTO AIM moved to ADVANCED >
  ORIGINAL GAME beside RED DOT; USE ITEM is Enter only (profile schema 36).
  Executable `6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33`, [note 166](166-d19n-options-layout.md).

## 2026-10-20 - D19N OPTIONS, the rest under ADVANCED (Needs playtest)

- Layout A as approved: OPTIONS is SOUND (original), CONTROLS, DISPLAY,
  GAMEPLAY and ADVANCED (one list under headings). AUTO AIM (profile, default
  OFF) replaces AIM ASSIST and sets the game's auto aim; GORE, STATUS BARS and
  GAME SPEED stay the original's values on the card; CONFIGURE CONTROLLER is
  gone from Modernized, which forces the default pad layout (a card copy keeps
  Vanilla's layout and auto aim in saves). Dependent rows are dimmed.
  Hooks `0x80082F64`, `0x80089920` regenerated (codegen hash unchanged).
  Executable `ce2b297e1f854aad818b5318b3636183986742973b94c1bed6d35bad2bcff6a9`,
  [note 166](166-d19n-options-layout.md). Offscreen and tests as described.
  Awaiting the user's playtest.

## 2026-10-20 - D24D + D24E accepted

- User: "its perfect! lock it in close it out". D24D and D24E Done; font pack
  `2609f2e6077a080c25d327582471ca1a19ab816ca1e078a44ddadeaaafe7316d`,
  executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`
  unchanged. Backlog: D19K, D19F, D19N, D26G.

## 2026-10-20 - D24D + D24E TTK-font ( ) and % (Needs playtest)

- `<` `>` were already native. ORIGINAL MOVEMENT uses `( )`, which the disc
  fonts lack, so they are now built from each font's own `O`. `%` is `./.`
  from its own `/` and periods. Only the font pack changed (executable
  `bbfaf117...` unchanged). Font tests pass, and offscreen MEDKIT 100% and
  console `(PAREN) 50%` show them in the TTK fonts.
  [note 165](165-d24d-d24e-font-glyphs.md). Awaiting the user's check.

## 2026-10-20 - D19G1 accepted

- User: "it works! i can confirm it works. lock it in close it out". D19G1
  Done; executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29` is the regression baseline. Backlog: D19K, D19F,
  D19N, D24D, D26G.

## 2026-10-20 - D19G1 FRAME RATE live after a 60 launch (Needs playtest)

- In-game FRAME RATE now applies on CONFIRM from a 60 launch (replay
  presentation and its render workers start mid-session) and 60 stops it from
  any launch, back to the original presentation. Software renderer: NEXT
  LAUNCH. Framework request `psx_mod_request_present_replay` in the tracked
  patch. Executable
  `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`,
  [note 164](164-d19g1-live-frame-rate.md). Offscreen as described; tests
  pass. Awaiting the user's playtest.

## 2026-10-20 - D08T4 accepted

- User: "lock it in, close it out, another winner". D08T4 Done; executable
  `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4` is the
  regression baseline. Backlog: D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D08T4 slot 10 dumpster (Needs playtest)

- Fixed by design: the level marks the floor at that dumpster as no-grab
  (original idle grab `0x80051890` checks bit 0 of byte 4 of Duke's floor
  record `*(player+0x48)`); Vanilla cannot grab it either, only climb it. Our
  grab hint ignored that gate. Now the hint and Grab apply only where the
  original can grab, game-wide; alley dumpster and medieval block unchanged,
  Vanilla unchanged. Executable
  `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4`,
  [note 163](163-d08t4-fixed-pushables.md). Native tests pass. Awaiting the
  user's playtest.

## 2026-10-20 - D10D accepted

- User: "perfect, lock it in, close it out". D10D Done; executable
  `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892` is the
  regression baseline. Backlog: D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D10D keyboard turning (Needs playtest)

- Left / Right arrows turn the view (and Duke) everywhere in Modernized play:
  on foot, first and third person, swimming, jetpack; Duke 3D rates (slow
  first 1/8 s, then about 79 degrees/s walking, 158 running), smooth at high
  refresh. Captured, the arrows no longer reach the original D-pad, so the
  jetpack stops strafing on them; the tank fallback turns Duke with them.
  KEY BINDINGS CAMERA rows TURN LEFT / TURN RIGHT, `--bind`; launcher schema
  35. Lab offscreen (level 1 slot 9) as described. Executable
  `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892`, [note 162](162-d10d-keyboard-turning.md). Native and Python tests
  pass. Awaiting the user's playtest.

## 2026-10-20 - D10C accepted

- User: "awesome!! ... lock this current one in, and close it out". D10C Done;
  executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19` is the
  regression baseline. New Todo: D10D (Left / Right arrows turn left / right
  in Modernized). Backlog: D10D, D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D10C Duke 3D look keys (Needs playtest)

- Page Up / Page Down look up / down while held and ease back on release;
  Home / End tilt and keep the view (a Page Up / Down tap brings it back).
  First and third person, mouse look on top, smooth at high refresh (late
  camera). KEY BINDINGS CAMERA rows and `--bind`; launcher schema 34 adds the
  keys to older profiles. Lab offscreen (level 1 slot 9): holds, locks,
  taps, mouse and V as described; KEY BINDINGS swap saved. Executable
  `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19`,
  [note 161](161-d10c-look-keys.md). Native and Python tests pass. Awaiting
  the user's playtest; open question whether center should be level.

## 2026-10-20 - D07F accepted

- User: "confirmed working. lock it in and close it out". D07F Done;
  executable `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef` is the
  regression baseline. Backlog: D10C, D19G1, D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D07F airborne shots follow the crosshair (Needs playtest)

- Jumps and falls hold only the camera-only lease; shots there now use the
  view aim (`airborne_shot_ready()`). Lab: 24 airborne shots (RPG, shotgun,
  pistol; manual and assisted; first person; up and down), 0 misses, rockets
  explode at the crosshair point. Executable
  `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef`,
  [note 160](160-d07f-airborne-shots.md). Native and Python tests pass.
  Awaiting the user's playtest.

## 2026-10-20 - D19G accepted

- User: "looking good!! lock it in and close this job out". D19G Done;
  executable `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da` is the
  regression baseline. New Todo: D07F (airborne shots ignore the crosshair),
  D10C (Duke 3D look keys), D19G1 (frame rate live after a 60 launch).
  Queued: D19K, D19F, D19N, D24D, D26G.

## 2026-10-20 - D19G KEY BINDINGS and live settings (Needs playtest)

- PC CONTROLS > KEY BINDINGS: every Modernized action in five groups; Enter,
  then a key or mouse button binds it at once and saves it (a taken key
  swaps, Escape cancels, Backspace clears, V asks RESTORE DEFAULT KEYS?).
  Live on CONFIRM now: mouse sensitivity and invert, display mode,
  widescreen, camera, view, shoulder, camera distance, weapon aim, and frame
  rate unless launched at 60. Design approved from a Photon mockup built from
  the disc's own interface art. Offscreen and Xvfb checks, Python 153 OK,
  native suites, patch and `check_repo.py` pass. Executable
  `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da`,
  [note 159](159-d19g-options-stage-2.md). Not verified: the real display
  (borderless / exclusive, window reshape) and the user's feel.

## 2026-10-20 - D33 accepted

- User: "perfect!! lock it in, close it out". D33 Done; executable
  `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-20 - D33 Modernized by default, `--vanilla` one launch (Needs playtest)

- Plain `run.py` always launches Modernized; `run.py --vanilla` plays Vanilla
  for that launch only (never saved). `--mode` removed; the in-game PROFILE
  row removed; a saved Vanilla choice moves to Modernized once with a notice.
  Offscreen checks, Python 152 OK, native suites and `check_repo.py` pass.
  Executable `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2`,
  [note 158](158-d33-default-profile.md). Not verified: the user's launches.

## 2026-10-20 - D08AB accepted

- User: "solid lock it in and close it out". D08AB Done; executable
  `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AB SWIMMING MODERN / ORIGINAL option (Needs playtest)

- New SWIMMING row in OPTIONS > MODERNIZED (after LANDING): MODERN (D08AA
  pace, default) / ORIGINAL (pre-D08AA water speeds with modern controls,
  including the forced run in shallow water - user choice). Live, saved
  (schema 33), `run.py --swimming modern|original`; Vanilla always original.
  Lab: ORIGINAL reproduces the D08AA "before" table (surface 26 / 6 / 15,
  underwater 27.6, subway forced run), MODERN unchanged, a mid-swim menu
  change applies at once both ways; regressions and suites pass. Executable
  `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508`,
  [note 157](157-d08ab-swimming-option.md). Not verified: the user's
  playtest. Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AA accepted; D08AB queued

- User: "this is perfect. swimming now feels great! it's responsive, and the gameplay allows the player to dodge enemy fire and not face these jarring speed changes when swimming." D08AA Done; executable
  `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366` is the
  regression baseline. New Todo D08AB: a SWIMMING MODERN / ORIGINAL row in
  PC CONTROLS (user request). Queued: D08AB, D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D08AA one walk/run pace on land and in water (Needs playtest)

- Shallow water forced the run; the mid-depth wade walked at twice the land
  walk; surface strokes (~26 forward, ~6 sideways) and underwater thrust
  (~28) ignored Shift. One shared pace (the land baselines, walk ~10.5 and
  run ~50 units per vblank) now drives all of them: shallow uses the land
  gait with Shift, the mid-depth root follows the pace step, and a new hook
  at the swim tail's rotation (`0x80066e34`, ra `0x80046ec8`) rescales the
  original stroke velocity; Space / Ctrl keep their vertical speed. Measured
  walk 9.9-11.1 and run 49-51 in every state; swim regressions and Vanilla
  unchanged; suites pass. Executable
  `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366`,
  [note 156](156-d08aa-unified-pace.md). Not verified: the user's playtest.
  Queued: D19K, D19G, D19F, D19N, D24D, D26G.

## 2026-10-19 - D19O and D19L accepted

- User: "accepted! lets bring this one to a solid close and lock it in". D19O
  and D19L Done; executable
  `114a77530e92839c095aeb833078a84823b312417e7256c5c476460c1d7f38b2` is the
  regression baseline. Queued: D19K, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D19O round 2 and D19L: the menu stays up through demo scene loads (Needs playtest)

- User playtest: judder at scene changes and loading screens; held Down
  stopped when the demo scene changed. Our menu skipped the guest's second
  pass of an image at two load hand-overs (gone about 100 and 200 ms), the
  presenter showed the older real image in a long logo pause, and held keys
  did nothing for the 1.9 s blocking load (then a burst). Now the menu is
  drawn on every image, moves during the whole load, and the repeat never
  bursts; load timing unchanged. Covers D19L's acceptance too. Executable
  `114a77530e92839c095aeb833078a84823b312417e7256c5c476460c1d7f38b2`,
  [note 155](155-d19o-menu-flashback.md#round-2-the-users-playtest-2026-10-19).
  Not verified: the user's playtest.

## 2026-10-19 - D19O menu flash after a stall or demo/logo switch (Needs playtest)

- The flash came after a replay timeline reset (a host stall; every demo/logo
  switch during a demo scene load), not from each menu change: D19J measured
  it right after its own dump stall. For about 100 ms the real image, one to
  two images behind the redraws, was shown. The newest menu image is now
  redrawn on its own until a pair exists, and the last redraw stays through
  the reset. Executable
  `226719899432b4b43694269a108943d1823e25fbe87c13089489278d64af5161`,
  [note 155](155-d19o-menu-flashback.md). Not verified: the user's playtest.

## 2026-10-19 - D07E accepted

- User tested Modernized `view` and `original` weapon aiming: "it plays
  solidly ... both work as intended beautifully". D07E Done; executable
  `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5` is the regression baseline.
  Queued: D19O, D19K, D19L, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D07E stale lease and original-aim facing (Needs playtest)

- Modern shots and movement lost their lease when the game ran below 15 fps
  (upgraded energy stream: 34 of 107 shots used the original auto-aim). The
  lease window is now one game frame, 4..10 vblanks (4 at the normal 20 fps).
- `original` weapon aiming: Duke now faces the camera (user choice), so the
  original shot, auto-aim and energy lock follow where he looks instead of the
  save's heading. Executable
  `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5`,
  [note 154](154-d07e-frame-lease.md). Not verified: the user's playtest.

## 2026-10-19 - D19P accepted

- User: "lock it in. close it out! it's great" D19P Done; executable
  `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63` is the
  regression baseline. Queued: D07E, D19O, D19K, D19L, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19P death screen QUIT TO MENU (Needs playtest)

- Death list QUIT is now QUIT TO MENU, asking QUIT TO MENU?; YES still goes
  through the game over screen to the title. Fixed on the way: the D19
  identity guard covered the selected item `0x800c3f94`, so Modernized menus
  could stay original for a session. Executable
  `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63`,
  [note 153](153-d19p-quit-to-menu.md). Not yet verified: YES in level 1 and
  first person, the user's playtest.

## 2026-10-19 - D22D accepted

- User: "it's perfect!!" D22D Done; executable
  `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98` is the
  regression baseline. New Todo D19P (death screen: QUIT becomes QUIT TO
  MENU beside QUIT GAME). Queued: D19P, D07E, D19O, D19K, D19L, D19G, D19F,
  D19N, D08AA, D24D, D26G.

## 2026-10-19 - D22D energy weapon damages enemies (Needs playtest)

- Cause: a D07C Modernized regression. The beam updater `0x800733C8` damages
  only the actor in projectile `+0x14`, and D07C cleared it on every
  view-aimed beam. Vanilla (same save, same enemy) locked and killed it; the
  Modernized baseline never damaged it.
- Fix: the first beam locks onto the targetable actor under the crosshair,
  within the original reach (6000), or none (a miss); the upgrade's extra
  beams keep the original's other locks inside 45 degrees of the view. The
  completion no longer races the host input sequence. Executable
  `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98`,
  [note 152](152-d22d-energy-beam.md).
- Lab: levels 1 and 0, two enemy types, first and third person, kills; 8
  degrees off is a miss; upgraded extra beam; Vanilla unchanged.
  `ttk-aim-test` links again with a D22D case; controls, input, Python 146
  OK. New Todo D07E (stale-lease shots use the original auto-aim; `original`
  weapon aim keeps a stale aim vector). Not yet verified: the user's playtest.

## 2026-10-19 - D17V accepted

- User: "total victory. i even played it on strong and i love it." D17V Done;
  executable `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193`
  is the regression baseline. Queued: D19O, D19K, D19L, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D17V modern view bob (Needs playtest)

- Modernized first person: the bob now keeps Duke 3D's steady tempo (about
  1.5 dips a second) walking and running, dipping deeper rather than faster
  when running, with no sway; the eye glides evenly instead of riding Duke's
  stride surge. Off restores the plain D17C eye. Executable
  `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193`,
  [note 151](151-d17v-view-bob.md). Pre-existing: `ttk-aim-test` lacks the
  D19J `psx_mod_host_call_*` stubs and does not link.

## 2026-10-19 - D19J accepted

- User: "fully accept". D19J Done; executable
  `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d` is the regression baseline.
  Now unblocked: D19K (needs D24B, accepted) and D19L. Queued: D19O, D19K,
  D19L, D19G, D19F, D19N, D08AA, D24D, D26G.

## 2026-10-19 - D19J title main menu re-implemented (Needs playtest)

- Modernized: after PRESS START our main menu in the original's look (ONE
  PLAYER, TWO PLAYER, LOAD GAME, OPTIONS, QUIT GAME), plus CHOOSE DIFFICULTY
  LEVEL, TWO PLAYER OPTIONS and CHOOSE CHARACTER, drawn with the originals'
  calls on state 12 / sub-screen 12 (new entry hook `0x80022324`). Starting a
  game hands the page to the original handler with one Cross press. One
  keyboard picks both two-player characters. LOAD GAME stays original
  (D19K). Framework: saves and loads wait for plugin guest calls
  (`psx_mod_host_call_begin/end`); a save inside a menu call had crashed the
  run. Executable
  `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d`,
  [note 150](150-d19j-title-menu.md). New Todo D19O (frontend redraw
  flash-back, also in the original menus). Still queued: D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19M accepted

- User: "perfect!! thats working". D19M Done; executable
  `de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43` is the regression baseline.
  Title main menu QUIT GAME comes with D19J. Queued: D19J, D19G, D19F, D19N,
  D08AA, D24D, D26G.

## 2026-10-19 - D19M QUIT GAME (Needs playtest)

- QUIT GAME is the last item of the pause list and the death list (the death
  list's QUIT still means game over); QUIT GAME? YES / NO, NO highlighted;
  YES sends the window close event, exit code 0, settings kept. Title main
  menu gets it in D19J (user choice). Executable
  `de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43`,
  [note 149](149-d19m-quit-game.md). Still queued: D19J, D19G, D19F, D08AA,
  D24D, D26G.

## 2026-10-19 - D19I accepted; D19M and D19N added

- User: "it's working as intended". D19I Done; executable
  `f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c` is the regression baseline.
  New Todo: D19M (QUIT GAME item that closes the program, YES / NO prompt),
  D19N (merge the original OPTIONS screens with ours: one OPTIONS screen,
  the rest under ADVANCED; first-pass overlaps: AUTO AIM vs our aim rows,
  CONFIGURE CONTROLLER vs PC CONTROLS / D19G, GAME SPEED vs CPU SPEED; design
  artifact first). Still queued: D19J, D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D19I in-game menu text at full brightness (implemented)

- Our pause list, death list, briefing, prompts and pause OPTIONS were drawn
  at the view composition, into the world's drawing chain, under the paused
  world's darkening quad (`0x80025f28`) and behind Duke. In gameplay they are
  now drawn in the pause-dispatch tick, after the quad, where the original
  draws its pause text; title menus unchanged. Text measures 255 red like the
  original (briefing exactly the original's 255, 176, 34; was 135, 95, 25).
  World darkening unchanged; 120 Hz presents never lose the text. Executable
  `f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c`,
  [note 148](148-d19i-menu-brightness.md). Still queued: D19J, D19G, D19F,
  D08AA, D24D, D26G.

## 2026-10-19 - D19H accepted; D19I-D19L added

- User: "accepted!! this works and it's functional". D19H Done; executable
  `681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2` is the regression baseline.
  New Todo: D19I (in-game menu text drawn at half brightness, likely under
  the paused-world quad), D19J (title main menu re-implemented like
  OPTIONS), D19K (LOAD GAME from our savestates inside the menus, replacing
  the memory-card LOAD in Modernized), D19L (menus stay up during demo scene
  loads). Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D19H pause menu and death screen as lists (Needs playtest)

- Modernized: Escape opens RESUME / MISSION BRIEFING / OPTIONS / LOAD GAME /
  RESTART LEVEL / MAIN MENU (layout B); the death screen lists CONTINUE / LOAD
  GAME / RESTART LEVEL / QUIT (challenge: REPLAY LEVEL / GIVE UP), with YES / NO
  prompts, in the original menu style (game's own draw calls, screen values 8
  and 9). Each choice runs the original handler's and main loop's own
  sequence; LOAD uses the original screens. Menu taps come from key events.
  D19's glyph guard no longer covers menu variables. Offscreen: every choice,
  challenge stage 1, Vanilla and title OPTIONS match the originals; suites
  pass. Executable
  `681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2`,
  [note 147](147-d19h-pause-menu.md). Continues kept as the original's (user
  expects to scrap them later). Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D08O4 accepted

- User: "this is awesome. fully accepted!". D08O4 Done; executable
  `3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d` is the regression baseline.
  Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-19 - D08O4 short Ctrl dive stops on release (Needs playtest)

- The surface dive (original anim 133) is a fixed ~30-update thrust along
  body pitch that ignores Square; after Ctrl was released the host pitched
  Duke back to the view, so the dive carried him forward. With no swim input
  held, the host now ends 133 the way the original handler does (anim 127).
  Slot 3 copy: drift after release 1474 -> 9 units (3-update tap), 796 -> 10
  (15-update hold); W, W + fire, Space surfacing, Ctrl+W and Vanilla
  unchanged; suites pass. Executable
  `3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d`.
  Still queued: D19G, D19F, D08AA, D24D, D26G.

## 2026-10-18 - D08O2B accepted; D08O4 added

- User: "it certainly is working, duke shoots in the right direction". D08O2B
  Done; executable
  `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25` is the
  regression baseline. New Todo D08O4: in slot 3 a brief Ctrl dive keeps
  carrying Duke forward with the dive/forward stroke after release (lab: about
  850 units of horizontal drift after a 15-frame tap). Queued: D08O4, D19G,
  D19F, D08AA, D24D, D26G.

## 2026-10-18 - D08O2B swim-fire on every Duke model (Needs playtest)

- Slot 3 (level 2 sewer, file 02) uses a Duke model whose joints are ordered
  differently from level 6, where D08O2A was tuned by record number; the hook
  turned a leg instead of the shoulders. Shoulders/chest are now found by part
  (`duke_model.inc`, shared with first person). Slot 3 copy dive + W + fire:
  gun hand 85 -> 221; level 6 regression matches; Vanilla no arming; suites
  pass. Executable `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25`,
  [note 146](146-d08o2b-swim-fire-costumes.md). Still queued: D19G, D19F,
  D08AA, D24D, D26G.

## 2026-10-18 - D08G5 accepted

- User: "perfect!!". D08G5 Done; executable
  `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79` is the regression baseline.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08G5 partial cheat entry keeps mouse look (Needs playtest)

- While `dn...` was pending, the input layer dropped all mouse events (so the
  camera stalled) and the key that broke the sequence was eaten. Now typing
  owns the keyboard only: mouse look and wheel stay live, a click cancels the
  prefix and acts, and a breaking key (W, Space, ...) acts at once.
  `ttk-input-test` PASS (new check fails on the old source). Executable
  `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79`.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08A22 accepted

- User: "fully accepted!!". D08A22 Done; executable
  `67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546` is the regression baseline.
  Still queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.

## 2026-10-18 - D08A22 strips scale with the picture (Needs playtest)

- The inventory/mission switcher strips and the mission card take their scale
  from the fitted game picture (picture height / 480, fractional, at least 1x)
  instead of an integer floor of the window height, and sit inside the
  pillarbox/letterbox. Strip icons now match the native HUD icons at every
  window size (offscreen: 18 vs 18 px at 1440x1080, 10 vs 10 at 800x600;
  OpenGL 4:3 and 16:9, Software). Framework patch re-exported and verified.
  Executable `67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546`.

## 2026-10-18 - D19 accepted: in-game OPTIONS extended

- Modernized OPTIONS (title and pause) adds PC CONTROLS, DISPLAY and
  MODERNIZED, drawn by the game's own menu code; settings save to the
  profile, eight gameplay options apply at once. User: "fully accept!!".
  Executable `fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b`. [Note 145](145-d19-game-menu.md).
  Follow-ups: D19G, D19F, D19F1.

## 2026-10-18 - D19D accepted

- User: "amazing work". D19D Done; executable
  `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe` is the regression baseline.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D19D PC menu keys (Needs playtest)

- Modernized menus: **Escape/Backspace** back out one level (Triangle),
  **Enter** confirms (Cross; Start on PRESS START), **V** is Square, the
  **mouse wheel** moves menu cursors. Escape never quits: it resumes in the
  pause menu (Triangle there is QUIT) and does nothing on PRESS START and the
  death screen. CONFIGURE CONTROLLER (assigns any pressed button) takes Start
  for both keys until its prompt. Fixes Escape confirming the death screen's
  REALLY QUIT?. Gameplay Escape/Enter, movies, F7 and Vanilla unchanged.
  Executable `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe`. [Note 144](144-d19d-menu-keys.md).
  Accepted 2026-10-18.

## 2026-10-18 - D30 recomp.board listing (Done)

- The project's row on [recomp.board](https://recomp.fyi/game/duke-nukem-time-to-kill-ps1)
  was written by the board's scan and unclaimed, with an empty toolchain,
  status and targets and a wrong Europe SLES-01515 "works from" entry.
  `.recomp.json` at the repository root now claims and writes it: USA
  SLUS-00583 only, Psxrecomp, in-progress, Linux, maintainer, links, help and
  notes. Validated against `https://recomp.fyi/schema/v1.json`. README has a
  recomp.board badge and link. The row shows claimed after the board's next
  daily read. Update the file when status or targets change (D22, D24, D25).
  No game code changed; nothing built or launched.

## 2026-10-18 - D19E accepted

- User: "excellent work". D19E Done; executable
  `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf` is the regression baseline.
  The interim original-menu work is closed out. Still queued: D08O2B, D08AA,
  D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19E menu responsiveness close-out (Needs playtest)

- Menu taps are no longer lost while the loading spinner plays or on LOAD
  GAME with a saved game. A Modernized menu press is held until the game has
  read it (its own pad history), and taps made meanwhile are queued. Menus
  over the spinner are drawn from the newer image, so press to screen there
  is 100-113 ms mean (was 191-220). 90 of 90 taps over the spinner (was 24 of
  30); every screen in the sweep took every tap exactly once at 120, 180 and
  60. Demo menus, smoothness, load timing, gameplay and Vanilla unchanged.
  Executable `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf`. [Note 143](143-d19e-menu-closeout.md).
  Limits: the original hides the menu for the first 1.8 s of a demo scene
  load (presses land when it returns); menu text still at 10 Hz. Accepted 2026-10-18. Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19C accepted; D19E queued

- User: "its more responsive when the demo is playing, but when the loading
  spinner plays it is less responsive. lets commit at this point." D19C
  Accepted for demo-backed menus; executable
  `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828` is the regression baseline.
  New Todo D19E: menus over the loading spinner are less responsive; fix
  that and sweep every menu screen for best response (close-out). Still
  queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D19C more responsive original menus (Needs playtest)

- Menu presses reach the screen about 100 ms sooner at the saved Modernized
  rate: main menu press-to-screen 220 -> 124 ms mean (24 of 24 taps one step
  each). Frontend in-betweens are drawn from the newer image of each pair,
  the world blended back toward the older poses; the guest, its 10 Hz tick
  and the demo behind the menu are untouched. Smoothness, gameplay start, 60
  and Vanilla unchanged. Executable
  `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828`.
  [Note 142](142-d19c-menu-response.md). Awaiting the user's playtest.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.

## 2026-10-18 - D17U accepted

- User: "i accept!! absolutely perfect." D17U Done (smooth spinning-logo
  loading screens at the saved Modernized rate); executable
  `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc` is the regression baseline. Still queued:
  D08O2B, D08AA, D08A22, D24D, D26G, D19C, D19D.

## 2026-10-18 - D17U smooth loading screens (Needs playtest)

- The spinning-logo loading screens now get D17 in-between images at the
  saved Modernized rate: level loads 119 distinct presents/s at 120 (30 guest
  images, 5 counted before), demo scene loads 112 (guest 10); 180 and 240
  scale. Load time unchanged (6.82-6.84 s); Vanilla and 60 original. Hook
  `0x80025554` added and regenerated (codegen hash unchanged). Executable
  `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc`.
  [Note 141](141-d17u-loading-screens.md). Awaiting the user's playtest.
  Still queued: D08O2B, D08AA, D08A22, D24D, D26G, D19C, D19D.

## 2026-10-18 - D17T accepted; D17U, D19C, D19D queued

- User: "i can confirm this works ... it's a good first start." D17T Done;
  executable `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`
  is the regression baseline. New Todo: D17U (smooth loading screens, the
  spinning Duke logo), D19C (more responsive original menus), D19D (Esc backs
  out of sub-menus; PC-friendly menu navigation). Still queued: D08O2B,
  D08AA, D08A22, D24D, D26G.

## 2026-10-18 - D17T smooth title menu and attract demo (Needs playtest)

- Frontend (mode 0) runs 10 images/s by the original's flip (6 fields); a
  30 Hz experiment desynced the attract demo, so the guest pace stays.
  Frontend images now get D17 in-betweens at the saved Modernized rate
  (118-119 distinct/s at 120), and short menu taps are held for 6 fields
  (45 ms taps 12/12, 7/12 before). Executable
  `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`.
  [Note 140](140-d17t-frontend-smoothness.md). Awaiting the user's playtest
  on their display. Still queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-09 - D08G6 accepted (`dntlod`)

- User: "accepted!" D08G6 Done. Fun typed cheat shows "Play Zaxtor's
  Oblivion!" only. Executable
  `74cec66e398217530d0b8abc3fa0ccfe06308f6b73a17e5aafa628b3f0f2a540` is the
  regression baseline. Queued: D08O2B, D08AA, D08A22, D24D, D26G.

## 2026-10-09 - D08AA queued (unify walk/run speeds)

- User: surface swim too slow; underwater and shallow/ankle-deep water force
  run without Shift. New Todo D08AA - unify walking/running horizontal speeds
  across land, shallow water, surface and underwater to the current land
  baselines (Shift walk/run); shared path preferred; do not retune the
  baselines here; keep swim physics, vertical move, transitions and swim-fire.
  Test: UI slots 2-3 (swim), slot 8 (subway ankle-deep). Still queued:
  D08O2B, D08A22, D24D, D26G.

## 2026-10-09 - D08O2B queued (swim-fire aims down again)

- User: on the level in UI slot 3, swimming and firing points the weapon down
  again; UI slot 2 still shoots forward on the same build. New Todo D08O2B
  (depends D08O2A). Still queued: D08A22, D24D, D26G.

## 2026-10-09 - D08A20 accepted (option A square)

- User: "fully accepted. ... this is it, excellent!!!" D08A20 Done; executable
  `a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95` is the
  regression baseline. [Note 139](139-d08a20-challenge-hud.md).
- Layout: 14x14 challenge icon in the native box's left cell (option A). Font
  picker section 15 marks A accepted; section 8 shows E as shipped.
- Still queued: D08A22, D24D, D26G.

## 2026-10-09 - D08A21 accepted; challenge icons tracked; D08A22 queued

- User: "completely approve this change." D08A21 Done; executable
  `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21` is the regression baseline.
- Challenge item icons tracked: `assets/ui/items/hud-challenge.png` (14x14 art)
  and `16px/hud-challenge.png` (from the user's `challenge-16.gif`), both with
  15-colour copies; items.json lists CHALLENGE ITEM (not drawn until D08A20).
- New: D08A22, host switcher strips scale with an integer floor of
  output height / 480, so they read small in small windows.

## 2026-10-08 - D08A21 14x14 HUD icons swapped in (Needs playtest)

- The user's 14x14-in-16x16 medkit, steroids, invincibility, invisibility and
  Double Duke icons (`research/inv/items-14/`) are now the active art in
  `recomp/assets/ui/items/` under the same names; 15-colour copies remade with
  `reduce_icon_15col.py`; the five cell/CLUT tables in
  `recomp/src/ttk/gadget_hud.inc` regenerated.
- The full 16x16 versions are kept, unused by the build, in
  `recomp/assets/ui/items/16px/` (user: for a future interface).
- Verified offscreen (private card copy): power-up, steroids and medkit boxes
  now leave a gap inside the frame; the switcher strip shows the new medkit and
  steroids. `test_ui_art` / `test_ttk_inv_icons` OK (new border test).
- Executable `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21`.
- Also 2026-10-08: backlog D08Z4, D08A20 (challenge icon delivered, top-right
  mockups in the font picker), D12D, D18E, D22D, D24E.

## 2026-10-08 - D08A12 accepted

- User: "perfectly done." D08A12 Done; executable
  `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d` is the regression baseline.
- Still queued: D24D, D26G.

## 2026-10-08 - D08A12 modern dynamite (implemented)

- **Original:** drawing dynamite lights it (fuse 4000 at the draw completion
  `0x8004e5ac`, burned by `0x8004dea4` case `0x8004e018`, about 3.7 s). The
  case ignores weapon requests and the holster button, so a switch waits for
  the throw or the blast.
- **Modern (`dynamite` = `modern`, default):**
  - A drawn stick stays unlit: fuse held full, sparks dark, sizzle refused.
  - 3, other weapon keys, the wheel, X or E's holster stow it, as for the Holy
    Hand Grenade.
  - Fire (Cross) lights it with a full fuse, and the original runs from then
    on.
- **Option:** profile schema 32, `run.py --dynamite original` (menu T).
  Vanilla stays original.
- **Hooks:** three new entry hooks, regenerated; codegen hash unchanged
  (savestates load).
- **Verified live (private copies):** held 6 s unlit, stow, wheel
  pass-through, a savestate round trip, then a lit throw and its explosion.
  Original: the stick exploded in hand and the switch waited.
- **Follow-up:** after a throw the next stick is drawn (unlit) once fire is
  let go; none when no sticks are left. Live: four throws in a row redrew.
- **Executable:** `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d`.
  [Note 138](138-d08a12-modern-dynamite.md). Nothing committed.

## 2026-10-08 - D08A19 accepted

- User: "well thats absolutely superb. I fully accept, this is the sort of design extension i wanted, introducing something the original game never had, and it looks completely at home. I'd call this feature finished." D08A19 Done; executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8` is the regression baseline.
  Still queued: D24D, D26G.

## 2026-10-08 - D08A19 used mission items (Needs playtest)

- User art delivered (`mark-used.png`, 8x8 green tick); font picker section 14
  shows it. Used mission items keep their full-colour icon with the tick in the
  strip and on the card, status USED (gold), counted as collected; Enter on one
  says ALREADY USED.
- "Used" = bit 0 cleared while Duke holds that item (animation 47); marker
  `0x4000` in the item's own flag, so saves keep it and a level reset clears it.
- Verified live in level 0 (red crystal at its holder) and by staged writes in
  level 6; savestate round trip; Vanilla never marked. Executable
  `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8`.
  [Note 137](137-d08a19-used-mission-items.md). Nothing committed.

## 2026-10-08 - D08Z3 accepted; D24D, D26G, D08A19 queued

- User: "Completely accept this work as complete, working, and done." D08Z3
  Done; executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e` is the regression baseline.
- Queued: D24D (`<`/`>` message fallback), D26G (console key repeat),
  D08A19 (USED status and green tick; placeholder in font picker section 14,
  8x8 `recomp/assets/ui/items/mark-used.png`).

## 2026-10-08 - D08Z3 modern landing (Needs playtest)

- Stand-still after landing = the original recovery 105 (straight-up jump,
  direction released before touch-down, bounce), about 0.6 s with no input,
  in Vanilla too. Modernized `landing` = `modern` (default): a held direction
  walks/runs off the update after the touch-down, a jump press hops again;
  `run.py --landing original` restores it. Schema 31 (the player's profile
  migrates on next launch).
- Measured: `original` 36..44 fields to move, `modern` 3..8; routes
  unchanged. Executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e`.
  [Note 136](136-d08z3-modern-landing.md). Nothing committed.

## 2026-10-08 - D08Z2 accepted

- User: "accepted!!". D08Z2 Done (armed scramble plays full body; the D08Z1
  safety net no longer freezes the legs). Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression
  baseline. Next: D08Z3.

## 2026-10-08 - D08Z2 revision 1: legs frozen after a corner bump (Needs playtest)

- User: after a bump at the statue (UI slot 8) Duke's lower half stays locked
  in one pose whatever he does (UI slot 6).
- Cause (D08Z1 safety net): its 107 undo ran at the next animation runner
  call, after the update had started 107 on the tracks; track 3's OR-only
  joint mask kept 107's leg joints (`0x1c00` -> `0x1cef`), freezing the legs,
  also in savestates. Reproduced at the monument corner; the original bounce
  never does it.
- Change: undo at the bounce's contact sound `0x8006bbd8` (ra `0x80055a10`),
  right after the 107 write; new opt-in hook, regenerated, codegen hash
  unchanged, savestates load.
- Evidence: three corner catches, mask stays `0x1c00`, legs stride after;
  native 45 groups, Python 136 OK. Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986`.
  [Note 135](135-d08z2-mantle-upper-body.md#revision-1-legs-frozen-after-a-corner-bump).
- The player's new UI slot 6 keeps the frozen legs (saved polluted state).
  Nothing committed.

## 2026-10-08 - D08Z2 mantle upper body (Needs playtest)

- Cause: the D08Y low-lip step-up (no E) could start with a weapon drawn, and
  since D08Z1 it starts from every wall slide. The original mantles only with
  a free upper body (`0x80051cf0` refuses on upper-table bit 8) and sets only
  the lower animation, so the legs played 134 while the upper track held the
  static weapon-ready pose 20. Holstered E mantles, crate mantles and the
  slot-12 grab and pull-up were full body and matched `original` frame for frame.
- Change: `jump_mantle_start` (one start for the crate, lowered-catch and
  step-up mantles) hands a weapon pose on the upper track to the mantle;
  `mantle_upper_restore` puts the kept upper block back when the mantle ends
  (without it the weapon stayed lowered and would not fire).
- Evidence: armed slot-5 step-ups play full body, weapon pose back one update
  after, Mouse1 fires; D08Z1 regression routes same outcomes; native 45
  groups (D08Y group extended), Python 136 OK. Executable `9de8da617862f53e545ac3c8be0f23623979fea2265944e2f3cdfaa3ec3f96b0`.
  [Note 135](135-d08z2-mantle-upper-body.md).
- Open: the user's playtest. Nothing committed.

## 2026-10-08 - D08Z1 accepted; D08Z2, D08Z3 queued

- User: "mechanically this feels significantly better, where we can call the actual job as accepted." D08Z1 Done; executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34` is the regression baseline.
- D08Z2 (next): the mantle now plays as a static leg pose, a regression from
  D08Z1. D08Z3: occasionally Duke is stuck for about a second after landing.
  Leads in the job board.

## 2026-10-08 - D08Z1 revision 1: monument corner (Needs playtest)

- The slot-6 monument corner bounced (convex corners defeated the wall clip).
  Jumps now turn around a corner in 15-degree steps, and any original bounce
  that still happens is turned into a slide, with its 107 undone in the same
  update. Slot 6 corner: no 107 in 21 jumps; earlier routes unchanged.
  Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. [Note 134](134-d08z1-jump-wall-slide.md).

## 2026-10-08 - D08Z1 jump wall slide (Needs playtest)

- Modernized jumps no longer bounce off walls: a wall only stops the motion
  into it, so Duke keeps his arc and slides along it (EDuke32 style); rising
  into a ceiling stops the rise and keeps the forward speed. No 107 stagger,
  rumble or contact sound. `run.py --jump-walls original` (menu K) restores
  the bounce; Vanilla unchanged. Profile schema 30.
- Verified offscreen: no 107 in 48 wall jumps over three levels, both jump
  styles; D08X/D08Y/D08U/D08W/D08Z routes keep their grabs, mantles and
  scrambles; native 45 groups, Python 136 OK, `check_repo.py`. Executable
  `c91a3ef128d9f98d923efe858fef75f432b8d729b2ffe5f16654f0034a56b627`. [Note 134](134-d08z1-jump-wall-slide.md).
- Needs the user's playtest. Nothing committed.

## 2026-10-08 - D24C accepted

- User: "all accepted". D24C Done: TTK-font `!` and `>` console prompt.

## 2026-10-08 - D24C TTK-font `!` and `>` console prompt (Needs playtest)

- Messages, headings and the console now draw `!` from the TTK font's own I
  and period (build-time, `build_ttk_fonts.py`) instead of the 8x8 system font.
- The console prompt and echoed commands start with `>` instead of `]`.
- Verified in a private offscreen run (`dnstuff`, console); suites pass. Needs
  the user's look in their own window.

## 2026-10-08 - D08A18 mission item use with Enter (Done, user-accepted)

- In Modernized, Enter / U on the browsed mission item tries to use it.
  - At the lock that takes it, the normal E interaction runs, so the original
    pulls the item out, opens the lock and uses it up.
  - Anywhere else a cheat-style notice appears, CAN'T USE THIS HERE (or NOT
    FOUND YET), and nothing changes.
- The original has no inventory use for mission items. See note 133 for the
  lock contract.
- Offscreen evidence: level 0 crystal holder use, both refusals. Suites pass.
  Vanilla unchanged.

## 2026-10-08 - D08A17 accepted

- User: "confirmed it's all working as intended!" D08A17 Done; executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline.

## 2026-10-08 - D08A17 power-up countdowns (Needs playtest)

- Invincibility (`+0x8a0`), invisibility (`+0x89c`) and Double Duke (`+0x89e`)
  are 6000-unit timers (20 s) set by their pickups and drained by Duke's
  update; death zeroes them and Continue gives 1500 invincibility (original).
  In Modernized each running one now has a lit box with the user's D08A16
  icon counting down in percent, stacked above the selected gadget, an
  active jetpack and running steroids (`gadget_hud.inc`). User choices:
  percent, the Continue protection shows, no expiry warning.
- Executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`;
  private Xvfb evidence and suites pass;
  [note 132](132-d08a17-powerup-countdowns.md). Waiting for the user's
  playtest. Not committed.

## 2026-10-08 - D08A14, D08A15, D08G4 accepted; D08A16, D08A17 queued

- User: "fully accepted and ready to close it out." D08A14, D08A15 (with the
  pickup correction) and D08G4 Done. Executable
  `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.
- Queued: D08A16 (power-up coin icons for invincibility, invisibility and
  Double Duke; human design ticket, placeholders in the local font picker
  when it begins) and D08A17 (HUD countdowns for the three power-ups, after
  the icons). Not started.

## 2026-10-08 - D08A14, D08A15, D08G4 (Needs playtest)

- **D08A14:** death ends steroids in Modernized `portable`: running and held
  doses are cleared while Duke is dead (user chose that a held dose is lost
  too), so after Continue there is no box, heartbeat or switcher entry.
  `original` and Vanilla keep them. [Note 127](127-d08a4-portable-steroids.md#d08a14---death-ends-steroids).
- **D08A15:** the heartbeat restarts with a beat at once when `dnhyper`
  refreshes steroids mid-run. Corrected by the user: a steroids pickup mid-run
  now stops the run and leaves a full held dose (Duke 3D feel), like the
  inventory cheats; executable `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472`. Cause: a refresh to 9000 never read
  above the run's first reading (9000 minus one drain step), so the old beat
  count held the next beat back. [Note 129](129-d08a10-steroids-heartbeat.md#d08a15---restart-mid-run-beats-at-once).
- **D08G4:** `dnstuff`, `dnitems`, `dninventory` set armor to 100 (10000, the
  original full armor value); running steroids stop and a full dose is held
  (user rule when selected). [Note 38](38-debug-cheats.md).
- Final executable `a4cd7a4f8efcfc44ef86e816adc3b245fcde9a733acd1e9f503fdf51e0235ef0`.
  Suites: `ttk-controls-test` 44 groups (new D08A14, D08G4), input,
  inventory, Python 131 OK (2 skipped), overlay guards, `check_repo.py`.
- **Not yet verified:** the user's playtests. Nothing committed.

## 2026-10-08 - D08A13 accepted; D08A14, D08A15 queued

- User: "i accept that this works." D08A13 Done; executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05` is the
  regression baseline.
- Queued: D08A14 (death and Continue end steroids and remove them from the
  inventory) and D08A15 (heartbeat silent for a few seconds when steroids
  restart mid-run). Not started.

## 2026-10-08 - D08A13 steroids independent of damage (Needs playtest)

- **Found:** the coupling is original. Duke's damage handler `0x800a40a8`
  checks running steroids first (`0x800a4154`): the hit is cancelled entirely
  (no armor or health loss) and costs the steroids 1500 of 9000. The armor
  element only displayed the countdown.
- **Changed (Modernized, `steroids` `portable`):** new entry hook
  `0x800A40A8` (one regenerated line, codegen hash unchanged, savestates
  load) turns the running bit off for Duke's damage call only. The hit takes
  the normal path (armor 75%, health, death and its return value), and the
  timer is untouched. The bit returns at the next TTK hook, before the drain,
  kick, status bar or heartbeat read it. Vanilla, `original` and
  `DNTTK_STEROID_SHIELD=original` keep the shield.
- **Evidence:** pig cop on savestate slot 3: 5-10 hits per run, timer drains
  5 per frame regardless (60 and 120 fps, armor, `dnhyper`, R dose,
  savestate); Vanilla 4 absorbed hits of 1500. Death and Continue resume the
  remaining time. Suites, Python 131, overlay guards, Vanilla route. Executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05`.
  [Note 127](127-d08a4-portable-steroids.md#d08a13---steroids-independent-of-damage).
- **Note for the user:** steroids no longer protect Duke in Modernized.
- **Not yet verified:** the user's playtest; explosions, falls and drowning
  in isolation; natural pickups.

## 2026-10-08 - D12C accepted

- User: "perfect!! fully accepted." Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f` is the regression baseline.
- D12C: a connecting kick (first or third person, Modernized) plays the wall
  bump thud `0x2007` one octave up at three times its volume, once at the
  first contact; misses stay silent. [Note 131](131-d12c-kick-impact.md).

## 2026-10-08 - D12C kick impact sound (Needs playtest)

- **What:** in Modernized, a kick that connects (an enemy, a breakable or
  other object, a wall) plays the game's own wall-bump thud `0x2007` once, at
  the first contact; empty-air, missed and out-of-range kicks stay silent.
  First-person quick kick and the third-person original kick.
- **How:** the damage sphere `0x800a979c` stores the attacker in its own
  frame only when it strikes something; host calls read that slot. Walls use
  the original segment query where the boot reaches. Third person: new entry
  hook `0x800A979C` (one regenerated line, codegen hash unchanged, savestates
  load) makes the kick's sphere call on the host and leaves the original call
  touching nothing, so damage is applied once. `DNTTK_KICK_IMPACT=off` turns it off.
- **Evidence:** private runs on the first street and the alley (pig cop, club
  door, garbage bag, head-on wall, empty air, floor, out of range); suites,
  Python 131, Vanilla route. Executable
  `3873288acd438d2e367ca2c0fffa20f0953134de61af7715522fec0ffcc87c98`.
  [Note 131](131-d12c-kick-impact.md).
- **Not yet verified:** the user's ear in play; a multi-kick crate; other levels.
- **Revision (same day):** the user found it very quiet; 92% of the thud's
  energy is below 150 Hz. It now plays one octave up (user's pick):
  `0x8006b73c` with pitch byte 48, SPU pitch `0x7E8`. Executable
  `b62af2b911cfdcc7c300ac025f00f1218b6f8c7439bbd8eda460ce9243283e3d`.
- **Revision 2:** user asked to double its volume: the impact's own sound
  object volume `+0x6a` is doubled (5192 -> 10384); live voice `0xC2A` ->
  `0x1855`. Executable `de112f13e75afef2e31543f48f09ec58c49ba4f6ada9a93b914d2e66cf0d9dbd`.
- **Revision 3:** multiplier 3 by default; the silent first kick at a wall
  was the routine refusing a `0x2007` already playing (Duke's quiet wall
  bump). A playing `0x2007` is now stopped first (`0x80068900`). Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f`.

## 2026-10-08 - D27A accepted; D12C queued

- User: "i fully accept this. great work. i have wanted this one for a long
  time so this minor change has a huge impact on me as a player." Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829` is the
  regression baseline.
- New Todo D12C: kick impact sound driven by the real kick hit result (wall,
  crate, actor), using the existing wall-collision thud (not Duke's grunt);
  empty-air and out-of-range kicks stay silent; no duplicates over targets'
  own damage sounds; kick gameplay unchanged.

## 2026-10-08 - D27A Modern Shift/run is silent (Needs playtest)

- **What:** in Modernized, pressing, holding and releasing Shift no longer
  plays the walk/run click (`0x0001`); running is unchanged and the D08A10
  steroids heartbeat still plays the same click on its beat.
- **How:** a new entry hook on the player update `0x800412A4` stores the run
  state the original toggle is about to set at `player+0x27b` first, so the
  original sees no change and never clicks (`run_click.inc`; one regenerated
  line, codegen hash unchanged, savestates load). Vanilla's L1 toggle still
  clicks. `DNTTK_RUN_CLICK=original` keeps the click.
- **Evidence:** SPU KEYON counts of the click sample per leg: Shift taps 10 -> 0,
  hold/release 2 -> 0, walking taps 8 -> 0; beat identical with or without
  Shift (19 beats, 15/15/18 fields). Suites and Python 131 pass. Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`.
  [Note 130](130-d27a-silent-shift.md).
- **Not yet verified:** the user's ear in play.

## 2026-10-08 - D08A11 accepted; D27A and D08A12 queued

- User: "this job is fully accepted." Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892` is the
  regression baseline.
- New Todo D27A: Modern Shift/run silent. The sound is the known walk/run
  toggle click `0x0001` via `0x8006bbd8(1)` (note 129); remove it from the
  Modern Shift path only and keep the D08A10 heartbeat, which uses it.
- New Todo D08A12: Modern dynamite - selecting it (6 twice, mouse wheel) must
  not commit Duke to a fuse; research the original equip/arm/fuse state
  machine first; Dynamite Behaviour Modern (Modernized default) / Original.

## 2026-10-08 - D08A11 `dnhyper` in the steroids box (Needs playtest)

- **What:** with portable steroids, `dnhyper` now counts down in the D08A8
  steroids box (lit pill icon, stacked or selected like a used dose) and the
  armour element shows only armour. It ends with nothing held.
- **Cause and fix:** `dnhyper` sets the running bit without the owned bit;
  the HUD predicates in `steroids.inc` now treat the running bit alone as
  running steroids, and the status-bar hide/restore no longer depends on the
  owned bit (savestate-safe). A pickup during a `dnhyper` run selects steroids
  (`pickup_select.inc`). R still takes only a held dose.
- **Evidence:** 60 and 120 fps private runs (no armour, armour 50, savestate
  reload, end, pickup refresh, R dose regression, `original` profile); suites
  and Python 131 pass. Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892`.
  [Note 127](127-d08a4-portable-steroids.md) (D08A11 section).
- **Not yet verified:** the user's look in play.

## 2026-10-08 - D08A10 accepted; D08A11 queued

- User: "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!" Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca` is the regression baseline.
  [Note 129](129-d08a10-steroids-heartbeat.md).
- New Todo D08A11: `dnhyper` shows its countdown in the original armor
  element instead of the D08A8 steroids box. Likely cause: `dnhyper` sets the
  running bit without the owned bit that the HUD predicates require.

## 2026-10-08 - D08A10 sound fix: the Shift click (Needs playtest)

- User on the first build: "the beat is right but the sound is wrong. i want
  the sound specifically when you press shift". Found it: sound `0x0001`, the
  walk/run toggle click, played through the non-positional call
  `0x8006bbd8(1)` (SPU sample `0x012F0`, pitch `0x228`-`0x22F`). The beat now
  makes that exact call; same sample and pitch as a real Shift press in levels
  0 and 6, rhythm unchanged. Suites pass. Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`.
  [Note 129](129-d08a10-steroids-heartbeat.md).

## 2026-10-08 - D08A10 steroids heartbeat (experimental, Needs playtest)

- **What:** in Modernized with portable steroids, a low heartbeat thump (the
  game's own sound `0x1012`, Duke's bank 1) plays at Duke while steroids run,
  225 bpm as Duke 3D's `DUKE_HARTBEAT` (reference video: 268 ms between beats).
  Stops with the effect, a damage cut skips ahead, death and level change end
  it. `DNTTK_STEROID_BEAT=off` silences it, `=<id>` tries another sound (the
  Shift-run footstep is `0x2000`). Console `sfx <id>` auditions any sound.
- **How:** `steroids_beat.inc`, one beat per 80 units of the steroids timer
  (300 units/s) from the existing authenticated player-update hook; the
  original sound call on a private stack. No new hooks or codegen change.
- **Evidence:** identical rhythm at 60 and 120 fps; pickup + R, savestate load,
  damage cuts, `level 6`, level 6 play, `original`/`off`; suites and Python 131
  pass. Executable
  `d0d1f09355d69f4ea3ff1a1a6ea7737bf261f512d83443b3d0f3edf20c861acf`.
  [Note 129](129-d08a10-steroids-heartbeat.md). Previews:
  `recomp/analysis/d08a10-beat/preview/` (local).
- **Not yet verified:** the user's ear (keep, change sound, or revert); the
  rhythm steps 250/250/300 ms in 20 Hz scenes; death while running.

## 2026-10-08 - D08A9 accepted; D08A10 queued (experimental)

- User: "fully accepted, working beautifully." Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6` is the
  regression baseline. [Note 128](128-d08a9-pickup-selection.md).
- New Todo D08A10 (experimental, may be reverted): while steroids run, a
  heartbeat-like sound at 226 bpm as in Duke 3D, using a game sound (an
  existing heartbeat if any, else Duke's Shift-run sound). Reference video
  `research/screencaps/Video_2026-10-07_23-59-03.mp4`.

## 2026-10-07 - D08A9 picked-up gadget becomes the selection (Needs playtest)

- **What:** in Modernized, picking up a jetpack, Bio Mask, goggles, medkit or
  steroids (or topping one up) makes it the `[ / ]` selection and the HUD box
  at once, as in Duke 3D; Enter / U then use it. Keys, mission items, full
  gadgets left on the ground and `dninventory` leave the selection alone.
- **How:** `pickup_select.inc` compares items 1-5 around the original pickup
  dispatcher (`0x80081a48` entry, `0x8001ca4c` ra `0x8007fe78`; existing
  hooks) and selects through the guest menu ID `0x800c3f94`. Waits while a
  gadget is mid-activation. No new hooks or codegen change.
- **Evidence:** private Xvfb runs in levels 0 (60 fps) and 6 (120 fps), new
  native D08A9 group, suites and Python 131 pass. Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`.
  [Note 128](128-d08a9-pickup-selection.md).
- **Not yet verified:** natural (non-spawned) pickups, death/Continue, the
  user's feel.

## 2026-10-07 - D08A8 accepted; D08A9 queued

- User: "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8` is the regression baseline.
  [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A9: when Duke picks up a gadget (jetpack, Bio Mask, goggles,
  medkit, steroids), it becomes the `[ / ]` selection and the HUD box, as in
  Duke 3D. Mission items and keys leave the selection alone.

## 2026-10-07 - D08A8 steroids countdown in the steroids box (Needs playtest)

- **What:** in Modernized (`steroids` `portable`), running steroids count down
  in their own HUD box with the pill icon, lit: in the item slot while selected
  (they now stay selected after R, so the box counts down in place), otherwise
  one box row above (above an unselected jetpack that is on). The armour element
  on the left shows only armour, and stays hidden with no armour.
- **How:** status bar element 2 shows steroids whenever `+0x364` bit 1 is on.
  For the status bar draw only, bit 1 is off; it comes back at the first hook
  after the status bar returns (`0x8001fc44`, or new lightweight hook
  `0x8002E850`; one regenerated line, savestates still load).
- **Evidence:** private Xvfb runs (GL/Software, 4:3/16:9, 60/120 fps), Vanilla
  unchanged; native suites and Python 131 pass. Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`. [Note 127](127-d08a4-portable-steroids.md).
- **Accepted** (see the entry above).

## 2026-10-07 - D08A4 accepted; D08A8 queued

- User: "mechanically, the steroids work perfectly. you pick them up, you can
  press r to run, and thats it ... i accept this job as complete now as it's
  functional". Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5` is the
  regression baseline. [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A8: the running countdown uses the original armor element and
  icon; show it in the steroids box with the pill icon instead (reference
  `research/screencaps/ttk-roids.png`).

## 2026-10-07 - D08A4 built: portable steroids, EDuke32 style (Needs playtest)

- **What:** in Modernized (`steroids` `portable`, the default), picking up
  steroids keeps them: STEROIDS shows as before, and they appear in the `[ / ]`
  switcher after the medkit and in the HUD item box with the user's own
  pill-bottle icon. **R**, or Enter/U with steroids selected, takes them
  (`USED STEROIDS`, the pickup sound, the original effect). One at a time: more
  steroids stay on the ground while one is held; a pickup while they run
  refreshes them, as in the original. `original` and Vanilla keep the original
  rule. `run.py --steroids portable|original`, `--settings` S (profile schema 29).
- **How:** the original still has a held steroids item (item 4, `+0x364` bit 0
  with its amount `+0x366`); only its pickup case `0x800827e8` runs it at once.
  The pickup is turned into a held item at the pickup tail's own sound call (new
  entry hook `0x8006B73C`, one regenerated line; savestates still load). A first
  design with a count in spare flag bits was replaced after a write trace showed
  the level-end snapshot `0x80083348`, which the card save writes, keeps only
  bit 0.
- **Evidence:** private copies of the user's cards and savestates, levels 0 and
  6: pickup held, second pickup left, R runs it, refresh while running;
  savestate reload, level completion with the stats-screen card save, loading
  that save, and death with Continue keep it; `original` and Vanilla unchanged;
  suites pass. Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5`.
  [Note 127](127-d08a4-portable-steroids.md). Next: the user's playtest.

## 2026-10-07 - D08V1 accepted

- User: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area". Their playtest log `session-20261007-202257.log` shows
  two chain top exits (196) with no identity failure.
  Executable `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`
  is the regression baseline. [Note 126](126-d08v1-control-loss.md).

## 2026-10-07 - D08V1 built: Modernized controls never lost to a chain exit (Needs playtest)

- **What:** climbing a chain or pole out at the top (the original exit 196,
  which the user reached after a mid-air catch) no longer switches Modernized
  controls off for the rest of the session. If the game's code check ever fails
  for real, the screen says `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`,
  the mouse turns Duke and WASD move him, and Modernized returns when the check
  passes. Vanilla unchanged.
- **Cause:** the original pole/hang side probe `0x800439e4` toggles bit 0x40 of
  the flag-table entry of Duke's current animation. A code guard covered that
  table, so the game's own write (`0x61 -> 0x21` on entry 196) read as tampered
  code. The fallback had no way to turn.
- **Fix:** the table is now a masked state guard (bit 0x40 on all 280 entries,
  and entry 265's low half for `BONUS.OVR`), with every other bit still
  authenticated. New `guard_writer_audit.py` (static, delay-slot aware, and live
  via `--port`) found no other reachable writer into any guard set.
- **Evidence:** private copy of UI slot 3. The baseline reproduced the lockout
  on the first top exit. Fixed: top exit, catch jumps, a missed jump and fall
  keep the lease; the live audit is clean; a forced loss turns with the mouse,
  moves and recovers; Vanilla host idle; suites pass. Executable
  `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`.
  [Note 126](126-d08v1-control-loss.md). Next: the user's playtest.

## 2026-10-07 - D08J5 accepted; D08V1 queued

- User: "that definitely works, and i accept it ... it works perfectly."
  Executable `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`
  is the regression baseline. [Note 125](125-d08j5-chain-descent.md).
- New job D08V1: after a missed jump that caught the chain in the air, the
  code guard on the upper-body animation flag table (`0x800c2a8c`) tripped on
  entry 196 (the original cleared bit 0x40 at run time) and Modernized
  controls stayed off for the rest of the session. Log copied to
  `recomp/analysis/control-loss-20261007/`. Modern controls must never be lost
  this way.

## 2026-10-07 - D08J5 built: climb down chains and poles (Needs playtest)

- **What:** in Modernized, at the top of a chain or pole press **E**
  (`E TO CLIMB DOWN` shows): Duke grabs it, swings round to the far side and
  hangs facing the platform. **S** climbs all the way down and steps off at the
  bottom; **W** climbs back up; **Ctrl** or **Space** lets go anywhere; S at an
  end with nothing below lets go. Vanilla unchanged.
- **Cause:** the original has no top mount for poles/chains and the jump from
  the slot-3 platform overshoots. Its down probe also accepts a floor above
  Duke, so on the platform side near the top Down climbed him back out
  (original, Vanilla too); a new codegen hook `0x8003964C` stops that in
  Modernized (regenerated one line; savestates still load).
- **Evidence:** private copy of UI slot 3: mount, full descent and step-off in
  third and first person, W exit, Ctrl/Space, A/D, platform-side descent;
  Vanilla and the slot-5 ladder regression unchanged; suites pass. Executable
  `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`.
  [Note 125](125-d08j5-chain-descent.md). Next: the user's playtest.

## 2026-10-07 - D08J3 accepted; D08J5 queued

- User: "we have an absolute winner once again!! it's working well, very fun,
  looks great." Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520` is the regression baseline.
  [Note 124](124-d08j3-climb-camera.md).
- Backlog: D08J5, climbing down chains (UI slot 3, re-saved with a chain in
  front of Duke).

## 2026-10-07 - D08J3 follow-up: climbs centre the view (Needs playtest)

- User: "it works perfectly!" and asked that climbs centre the view whatever
  the over-the-shoulder setting. Built: the shoulder offset eases to centre
  during ladder/pole/chain climbs and back afterwards; the saved preference is
  unchanged. Chain run and suites pass. Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`. [Note 124](124-d08j3-climb-camera.md).

## 2026-10-07 - D08J3 built: free camera on ladders, poles and chains (Needs playtest)

- **What:** in Modernized, the mouse camera stays free while Duke climbs
  ladders, poles, chains and climbing walls (orbit, look up/down, Alt+wheel,
  V recentre). On poles and chains A/D go left/right on screen from any
  camera side. Vanilla unchanged.
- **Finding:** the climb keeps the normal camera; in mode 3 the original only
  swaps in a high look-down boom, and the host orbit lease stopped there.
- **Evidence:** private copies of UI slots 5 (ladder) and 3 (chain): full
  orbit while climbing, climbing unchanged, A/D screen-relative behind, front
  and side; ladder top/enemy/descent regression and Vanilla unchanged; suites
  pass. Executable `2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`.
  [Note 124](124-d08j3-climb-camera.md). Next: the user's usability playtest.

## 2026-10-07 - D08J4 accepted

- User: "it's a complete winner for me. I totally accept." Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa` is the regression baseline. [Note 123](123-d08j4-ceiling-hang.md).

## 2026-10-07 - D08J4 built: climbable ceilings (Needs playtest)

- **What:** in Modernized, hanging from a climbable ceiling (monkey bars),
  WASD move Duke relative to the camera like walking and he no longer lets go
  in mid-travel; at an end he keeps hanging. Ctrl or Space lets go. Vanilla
  unchanged.
- **Cause:** the falls were ours: the D08X object-hang release let go after W
  had been held about 40 updates (and on S). The tank steering is original
  (mode 7: Up/Down along the facing, Left/Right turn).
- **Evidence:** private copy of UI slot 3: camera-relative travel in all
  directions, no drops, ends hold, Ctrl/Space let go; suites pass. Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`.
  [Note 123](123-d08j4-ceiling-hang.md). Next: the user's playtest, including
  whether the red-carpet ledge can be reached.

## 2026-10-07 - D08J2 accepted; D08J4 queued

- User: "fully and completely accept it all." Executable
  `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the regression
  baseline. [Note 122](122-d08j2-pole-chain-sidestep.md).
- Backlog: D08J4, ceiling monkey-bar climbing drops Duke at the wrong points
  (UI slot 3, re-saved at the top of the chain).

## 2026-10-07 - D08J2 built: pole and chain A/D direction (Needs playtest)

- **What:** in Modernized, while Duke hangs on a pole or chain, A takes him
  round it to the left and D to the right on screen. Vanilla unchanged.
- **Cause:** original. Left/Right on the hang-climb (192..195) reach the
  sidestep `0x800439e4`; D-pad Left carries Duke to his own right, then
  `0x80043c08` turns him to face the object. A was D-pad Left.
- **Evidence:** private copy of UI slot 3 (the chain): A/D in third person and
  the first-person profile, W/S climbing; Vanilla D-pad unchanged; suites pass.
  Executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`.
  [Note 122](122-d08j2-pole-chain-sidestep.md).

## 2026-10-07 - D08U2 accepted

- User: "fully accepted. this is exactly what i wanted. well done!" Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e` is the
  regression baseline. [Note 121](121-d08u2-ladder-top-enemy.md).

## 2026-10-07 - D08U2 built: climb off a ladder past an enemy (Needs playtest)

- **What:** in Modernized, an enemy (or other character) standing near a
  ladder top no longer keeps Duke on the top rung: holding W, he climbs off
  with the original exit and stops short of the enemy. Walls and other
  objects still block; the enemy is not pushed (user: optional). Vanilla
  unchanged.
- **Cause:** original. The exit test `0x8007d240` refuses when its forward
  probe `0x8007cde8` meets any cell object, characters included.
- **Evidence:** private copies of UI slots 5 and 12: slot 5 in both views,
  the second alley ladder with an enemy respawned on its landing, D08U/D08U1
  mounts and descents unchanged; suites pass. Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`.
  [Note 121](121-d08u2-ladder-top-enemy.md).

## 2026-10-07 - D08U2 queued

- Backlog: D08U2, an enemy standing at a ladder top stops Duke climbing off;
  Duke should push it back. Test location: the user's UI slot 5.

## 2026-10-07 - D08O3 accepted; D08J2 and D08J3 queued

- User: "fully passed my playtest. feels great to play. i accept this job
  now as complete!!" [Note 120](120-d08o3-e-in-water.md).
- Backlog: D08J2 (poles and chains: A turns right, D turns left; make A
  left and D right; chain test location: the user's UI slot 3) and D08J3 (free camera while climbing ladders, poles
  and chains; needs investigation and usability testing).

## 2026-10-07 - D08O3 built: E in water keeps the weapon (Needs playtest)

- **What:** E while swimming still stows the weapon (needed to climb out at
  a ledge), but without a climb-out Duke draws it again automatically,
  without stopping his swimming. Land E and Vanilla unchanged.
- **Evidence:** private level 6 lab: walls, ledge, underwater, 11 swimming
  runs; regressions and suites pass. [Note 120](120-d08o3-e-in-water.md).

## 2026-10-07 - D08O2A accepted

- User: "this is actually rock solid ... I'm happy with it and its 100%
  playable." Known minor issue: one final shot at the end of a burst can
  fire with the weapon pointing down (seen in the user's capture).

## 2026-10-07 - D08O2A built: natural swim-fire pose v2 (Needs playtest)

- **What:** in Modernized, swimming underwater while firing now raises only
  Duke's arms and weapon toward the crosshair; the chest, head and legs keep
  the swim stroke (v1 stood the whole upper body up). Vanilla unchanged.
- **How:** new hook on the matrix composition `0x800b42ec` re-orients the
  two shoulders to their floating-fire pose relative to the view, at either
  model build; no aim bit or other game memory is written.
- **Evidence:** private level 6 lab, six weapons x four directions, 120 fps:
  gun hand near the floating-fire value, chest unchanged from plain
  swimming; floating, surface, ground and Vanilla unchanged; suites pass.
  [Note 119](119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O2 accepted (v1); D08O2A queued

- User: "it works and i accept this as a v1". The more natural pose (torso
  stays in the stroke, arms raised to fire, head looking up) is Todo D08O2A.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D08O2 built: weapon forward while swimming and firing (Needs playtest)

- **What:** in Modernized, swimming underwater while firing turns Duke's
  upper body and weapon toward the crosshair, as when he fires floating still;
  the legs keep the stroke. Vanilla unchanged.
- **How:** new hook `0x800411b8` sets the original aim bit `0x100` for Duke's
  model build only (cleared at the neck's aim call), so the original arm-aim
  builder runs during the stroke and the D07A hook gives it the view.
- **Evidence:** private level 6 lab, six weapons x four directions, gun hand
  along the view equals the floating-fire pose; 60 and 120 fps; Vanilla,
  ground and surface unchanged; suites pass.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D23E and D24A accepted

- User: "we can mark D23E and D24A both as accepted. we've proven these are
  now working significantly better". D23E (busy-scene stutter,
  [note 104](104-d23e-busy-scene-stutter.md)) and D24A (open assets,
  [note 112](112-d24a-open-assets.md)) are Accepted. No code changed.

## 2026-10-07 - D08A7 accepted: custom medkit gadget icon

- User: "amaazing work. i accept" (Done). The medkit shows the user's sprite
  in the switcher and the HUD box; health keeps the cross.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A7 built: custom medkit gadget icon (Needs playtest)

- **What:** in Modernized the medkit's inventory picture is the user's own
  sprite, full colour in the `[` / `]` strip and 15 colours in the D08A6 HUD
  box. The health box keeps the cross. Vanilla unchanged.
- **How:** the strip pack takes item 5 from `assets/ui/items/gadget-medkit.png`;
  the HUD box draws a medkit record whose 4bpp cell and palette sit in rows the
  disc's HUD sheet leaves empty (cell 960,205, palette 1008,206), loaded by a
  GP0 `A0` packet in the HUD ordering table each frame the box is drawn.
- **Evidence:** live VRAM survey (band empty in intro, title, pause, Select,
  six levels; the game fills x 960-991 from row 223 at run time), GL 4:3 /
  16:9, Software 4:3, 120 fps with savestate reload, Vanilla no upload.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A6 accepted: selected gadget on the HUD

- User: "i fully accept!" (Done). The gadget picked with `[` / `]` always has
  its original-style box over ammo, lit when on, dim when off; it follows the
  selection at every frame rate. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 playtest fix (Needs playtest)

- User saw the medkit box stay after hovering the jetpack, with both shown.
  Cause: at 120 fps the D17 worker redraws replay the status bar on a copy of
  guest memory, where the host selection was stale. The box now reads the
  original menu ID `0x800c3f94`, kept equal to the selection.
- `[` / `]` now hover in every gameplay state (flight, swimming, firing); the
  selected jetpack lights in place without sliding out of the ammo box.
- Verified from a copy of the user's savestate with the user's settings,
  three runs. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 built: selected gadget on the HUD (Needs playtest)

- **What:** in Modernized the gadget chosen with `[` / `]` (the one Enter / U
  uses) always has a box at the original item slot over ammo: the HUD's own
  box, icon and red digits with its charge. Lit when on, dim digits when off,
  medkit (health cross) always lit. An unselected jetpack that is on moves
  one row up; Bio Mask / goggles keep their original place left of ammo.
- **How:** the status bar's own calls, from its entry hook after the D14 shift
  (`src/ttk/gadget_hud.inc`); dim digits are the original glyph packets at
  colour 0x31. The whole HUD layout is now saved and restored each draw.
- **Evidence:** private Xvfb runs: every selection, cycling, N / J / Enter,
  stacking, savestate load, level change, GL 4:3 / 16:9, Software 4:3, Vanilla
  unchanged; native tests and Python 120 OK.
- **Open:** user look confirmation; death / Continue and resize not run;
  Software 16:9 cuts the right HUD with or without this job (existing).
  [Note 115](115-d08a6-selected-gadget-hud.md).







## 2026-10-07 - D08A5 accepted: mission inventory on , / .

- User revision after the first try: mission items are their own **mission
  inventory** on `,` / `.` (previous / next, the < > keys), in the switcher's
  place with the item card at the top. `[` / `]` show gadgets only. Enter / U
  while it is open just close it, so an instinctive Enter no longer toggles
  the jetpack. The `\` binding is gone.
- Original strafe (Comma/Period, PS L1/R1) is unbound by default; profile
  schema 28 moves the keys and drops schema 27's `\` action.
- Verified in native tests, Python 120 OK and a private Xvfb run (browse,
  wrap, Enter swallow, `[` back to gadgets). User: "i fully accept!" (Done).
  Open for later: controller input for both inventories.
  [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - D08A5 built: mission item tracking (Needs playtest)

- **What:** design E is in the game (Modernized only). `[` / `]` show a mission
  row under the gadgets in every level with mission items; `\` browses it and
  shows the item card at the top (FOUND / NOT FOUND YET), clearing 2.5 s after
  the last press. The gadget frame is the tile0020 orange swap.
- **Playtest fixes:** `\` now works on UK keyboards (the key left of Z, SDL
  `NONUSBACKSLASH`); mission and gadget frame borders are even (pixel-centre
  stretch) and the panel outline has margin below it.
- **Profiles:** schema 27 adds the `\` binding to saved profiles (custom
  bindings kept); older launchers refuse the migrated file.
- **Data:** the original Select inventory's own list (name function
  `0x80087d4c`, flag `player+0x354+4*i` bit 0). Crystals are items 11-13 and
  level 6's skeleton keys items 7-8, not the ticket's guesses.
- **Evidence:** native and Python suites pass; private Xvfb runs show the row,
  card, timeout, a live jewel pickup (1/5), level travel and no row in level 8;
  Vanilla unchanged. `ttk-controls-test` fails on stale fixtures with or
  without the change.
- **Open:** user look confirmation in play; controller D-pad focus is not
  implemented (no controller path opens the switcher); the block sits about
  26 px higher than the mockup. [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - Mission item icons in; tracking design E locked; D08A5 queued

- **Icons:** the user's original 16x16 icons are tracked in
  `recomp/assets/ui/items/`, with `items.json` mapping all 15 mission items
  and steroids to a file, the game's object types and levels:
  - one keycard drawing serves all seven key-slot items;
  - Scrap of Paper also serves Torn Paper (user-confirmed);
  - every other item has its own drawing.
  `test_ui_art.py` checks coverage and format.
- **Keycards:** every key-slot key (types 153/154, named per level) is the same
  flat keycard object in the game, including medieval level 7. A pig carries
  and drops it there. Checked in RAM.
- **Tracking design:** the user compared four mockups (A panel, B HUD tracker,
  C pickup card, D switcher row) in the research page and iterated a fifth,
  **E**, which is locked: the mission row under the `]` switcher, `\`
  browsing, and a card at the top. Exact spec in **D08A5** on the board.
- **Next session: build D08A5.** Start from the board entry. The working
  mockup code (layout, palette swaps, card) is `drawE()` and `swapped()` in
  `recomp/analysis/d24a-fonts/picker.template.html` (local).
  - Switcher code: `src/ttk/inventory_hud.cpp`; packs via
    `tools/local/build_ttk_inv_icons.py`.
  - Item names and found state: the player's mission flags (`+876`..`+916`,
    see the job). Level crystals (items 11-13) are confirmed by D26F
    ([note 117](117-d26f-crystals.md)): taken from holders, not walk-over
    pickups; `spawn 761` (generic crystal) sets item 14 instead. D26F (accepted)
    adds `spawn 1761/2761/3761`: the real crystals as custom
    walk-over pickups (recipe in note 117).

## 2026-10-06 - D24A, D24B, D26E: open assets, TTK fonts, savestate menu, debug spawn

User, end of session: "im happy with all progress tonight. the document is
superb and an amazing piece of research."

- **D24A (Needs playtest: fresh-clone test).** No Duke Nukem 3D art or
  `research/` input remains:
  - TTK disc fonts (Big and Medium Italic, system 8x8) are built from the
    player's disc;
  - switcher digits are the CC0 3x5 Microfont;
  - the selection frame and crosshair are the user's original art;
  - HUD icons are built from the disc in every build;
  - messages and the console draw at half scale, in the "Console steel"
    palette with a navy shadow.
  The README credits PSXRecomp, Alexbeav's PS1 Recomps, recomp-ui and
  Microfont. Remaining: the fresh-clone test against the pushed commit.
  [Note 112](112-d24a-open-assets.md).
- **D24B (Accepted).** The F7 savestate menu uses the TTK fonts and disc art,
  with level names recorded per slot. [Note 113](113-d24b-savestate-menu.md).
- **D26E (Accepted).** The console has `spawn <item>` and `items`, now working
  from the real console and along the camera view; the half-sunk landing is
  the game's own drop. It also gained scrollback (512 lines, PgUp/PgDn/wheel),
  Up/Down command recall and `history` (D26C, accepted).
  [Note 111](111-d26e-debug-spawn.md).
- **Research page:** the local UI and font research page (fonts, palettes, UI
  sprites, BS stills, per-level mission items, design slots, savestate mockup)
  is `recomp/analysis/d24a-fonts/ttk-font-picker.html`. It is retail-derived
  and local only; `research/TTK-UI-and-Font-Research.html` links to it.
- **Next:** the user designs the open icons (15 mission items and keys, plus
  steroids) for the placeholders on that page.

## 2026-10-06 - D24A queued (public clone completeness)

A fresh-clone test of `main` built and played the same as the working copy,
but lacked the TTK inventory icons (disc-derived, not built by `build.py`),
the Duke message font and the green digits (Duke 3D art in `research/`). New
Todo D24A covers moving them into a proper build route. It notes that Duke 3D
art cannot be committed without a licence, and lists the options (derive from
the TTK disc, an original font, or user-supplied files).

## 2026-10-06 - D08Q6 accepted

User accepted the Modern jetpack fix ("i accept this as fixed! the test passes
and this feels great"): revision 5b, with height held, the original's speed,
flame and poses. Not committed.

## 2026-10-06 - D08Q6 revision 5b: original speed, flame and poses (Needs playtest)

Modern flight now moves at the original jetpack's top speed (about 10x the
5a cut) and shows the original flame (moving, climbing and hovering) and lean
poses. The original's flight inputs are fed again and their thrust is
cancelled so the host still owns the velocity. Height hold, landing, J, fire,
switching and fuel are verified live; Classic and Vanilla are unchanged; all
suites pass. Next: the user's playtest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 5: Modern flight model rewritten (Needs playtest)

Root cause found: the original root apply `8004ac08` throws away horizontal
motion whenever the vertical root is 0, so a precisely held height left WASD
dead unless Space or Ctrl was held. Modern flight was rebuilt from scratch.
The host owns the flight velocity: eased camera-relative WASD/arrows,
Space/Ctrl, height hold with the floor cap, and a +-1 vertical step while
moving. The original keeps collision, landing, cut-outs, firing and fuel
(drained at the original rate). Verified live with a copy of the user's
profile; all suites pass. Duke keeps the hover pose while moving. Next: the
user's playtest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 4: Right Shift dropped capture; idle creep (Needs playtest)

From the user's session log: pressing Right Shift sent the original Select,
which silently released Modernized capture mid-flight. The host stopped with
the hover lock on, so WASD barely moved until Space. Right Shift now counts as
Shift and never sends Select while captured (game-wide; on foot it runs like
Left Shift). The idle creep was a stalled horizontal coast, now stopped while
hovering. Both were reproduced and fixed live. Suites pass. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 3: Shift and arrow keys in flight (Needs playtest)

Shift no longer reaches the original hover toggle in Modern flight; that
toggle was dropping Duke 128 units against the new hover. The arrow keys now
fly like WASD; before, they sent raw D-pad input that the hover pin throttled.
`dnkroz` keeps the jetpack the whole time god mode is on. Live: arrows fly,
Shift has no effect on height, and the largest step is 7 units. Suites pass.
Next: the user's retest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - `dnkroz` now gives the jetpack (D08Q4 follow-up)

`dnkroz` turning god mode on now also grants the jetpack (full fuel) when Duke
lacks one, at the user's request during the D08Q6 retest. Unlimited fuel for
an owned pack already worked. Cheats are still refused in mid-air. Verified
live and in the native suite. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08Q6 revision 2: smooth hover (Needs playtest)

The user found revision 1 jerky. Its hover twitched +17/-25 about every 6
updates, because phase 0 left one timestep of the steep part of the bob sine and
the error-driven slew echoed it. Revision 2 parks the bob at its flat peak and
slews the base gently. Hover is flat and stops settle over 3 updates, while the
altitude still holds. Suites pass. Next: the user's retest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 Modern jetpack altitude hold (Needs playtest)

Modern flight no longer creeps upward. The altitude captured when Space or
Ctrl ends is held through flight, turning, looking up/down and hovering.
Level flight steers back to it, hover pins to it with the original bob off, and
it is capped at the original floor approach height over higher ground. The lab
circuit ends within 9 units (before: about 460 units of climb in six legs).
Ctrl landing, J fall, firing, weapon switching, Classic and Vanilla are
unchanged. Suites pass. Next: the user's LEVEL01 playtest. Not committed. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q5 accepted; D08Q6 queued

User accepted weapon switching in jetpack flight ("excellent!! i accept.").
New Todo D08Q6: in Modern flight Duke's altitude creeps upward over long
circuits (LEVEL01 apartment/alley, `dnkroz`, much looking up); height should
stay fixed unless Space or Ctrl is used. Not committed.

## 2026-10-06 - D08Q5 weapon switching in jetpack flight (Needs playtest)

Number keys, the wheel, semicolon/apostrophe and X now switch weapons while
flying (Modern and Classic). The original mode-10 state already completes the
ground weapon request with the normal holster/draw; our shortcut layer had
stopped at the J check. Verified live with every weapon, firing after each
switch; swim rules, ground and Vanilla unchanged; suites pass. Dynamite's own
blast still knocks Duke out of flight (original). Next: the user's playtest.
Not committed. See [note 108](108-d08q5-jetpack-weapon-switch.md).

## 2026-10-06 - D08G3 and D08Q4 accepted; D08Q5 queued

User accepted `dnupgrade` and the Duke3D-style `dnkroz`. New Todo D08Q5:
weapon switching with number keys and the wheel while flying the jetpack.
Committed and pushed.

## 2026-10-06 - D08G3 `dnupgrade` and D08Q4 `dnkroz` (Needs playtest)

`dnupgrade` upgrades weapons 4/5/7/8/9/10 through the original upgrade bit and
the persistent mask `+0x85f` (which saves and pickups reapply), carrying ammo
to the Laser Gatling, Incendiary RPG and HiTemp Flamethrower records.
`dnkroz` now raises health to 100 (never lowering an Atomic Health surplus;
the original pickup already caps at 200) and keeps an owned jetpack full in
every scheme. Verified live in levels 6 and 12, Modern and Classic flight, a
savestate round trip and Vanilla, on private copies; suites pass. Next: the
user's playtest. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08O1 accepted; D08O2 queued

User accepted fire while swimming ("mechanically, it does exactly what it's
meant to"). Remaining issue, now Todo D08O2: the weapon points downwards while
Duke swims and fires; make it point forward. D08Q4 scope extended to Duke3D's
`dnkroz` (health to 100, unlimited jetpack, Atomic Health to 200). Committed
and pushed.

## 2026-10-06 - D08O1 fire while swimming (Needs playtest)

Underwater the original tests fire before the swim thrust, so holding fire
stopped Duke. Modernized now keeps him swimming while he fires (fire hidden
from the swim handler only, restored right after; new hooks `0x800455bc`,
`0x80055e80`, codegen hash unchanged). Swim shots use view aiming and the
crosshair; weapon keys work in water, limited to the weapons the original
allows there. Verified live in level 6 on private copies; Vanilla, ground and
jetpack firing unchanged; suites pass. Next: the user's playtest at their
location and a second level. Not committed. See
[note 107](107-d08o1-swim-fire.md).

## 2026-10-06 - Backlog: D08A4, D08S reopened, D08Q4, D08Z1, D11F; D08O1 started

User requests: D08A4 EDuke32-style portable steroids (stored item, R to use);
D08S reopened as the EDuke32 jetpack scheme (instant J on/off, midair, 61 s
of fuel); D08Q4 `dnkroz` gives unlimited jetpack fuel; D11F first person while
flying the jetpack; D08Z1 keep jump momentum when bumping a wall (Modernized
option for the future menu). All Todo. D08O1 (fire while swimming) selected
and In progress.

## 2026-10-06 - D23H accepted

The user played with several saves and accepted D23H: presents recover after
saves and hitches. Documentation, commit and push not yet authorized.

## 2026-10-06 - D23H: presents recover after saves and hitches (Needs playtest)

At 120 Hz and Match Display a D23E shed could never be undone, and any single
slow second (a savestate save is about 90 ms, an F7 menu visit more) shed. So
presents fell to 60 and then 40 for the rest of the session. Load shedding now
needs two behind seconds in a row and steps back up by itself in every
frame-rate mode. Offscreen: 120 presents/s through a 21-minute session with
20 saves, 8 loads and 13 simulated hitches. Awaiting the user's long play
session. See [note 106](106-d23h-present-rate-recovery.md).

## 2026-10-05 - D23F accepted: fast CPU timing is the Modernized default

The user accepted D23F; Modernized now uses fast CPU timing unless
`--cpu-timing accurate` is chosen. Next candidates: D23G (finish the fast
path) and D17S (auto frame-rate default).

## 2026-10-05 - D23F fast CPU timing: Needs playtest

`run.py --cpu-timing fast` (Modernized option, default accurate) runs the
recompiled game code on a lighter, calibrated timing model during gameplay.
At 150% and 120 Hz the western town (slots 9, 10) and slot 1 present every
refresh (120/s, was about 40) without overclock pauses; game speed and the
emulated CPU speed are unchanged. Not yet playtested. See
[note 105](105-d23f-fast-timing.md).

## 2026-10-05 - D23F added: faster timing model (big Todo)

The user prefers 100% CPU for now. At 150% and 120 Hz one emulation thread
cannot run the faster game and 120 redraws per second, so presents drop even
in light scenes (slot 1 75/s, slot 9 54/s); still images are identical. New
big Todo D23F: observers off in player sessions, then a design study of a
Disruptor-style fast timing model (cycle model is about 34% of the thread).
D23E stays Needs playtest at 100%. User authorized commit and push.

## 2026-10-05 - D23E western-town stutter: Needs playtest

Slots 9 and 10 are in the western town. At the player's `cpu_overclock 100`
the 16:9 view overloads the emulated CPU (game frames 4 fields with 5-6 steps;
Vanilla 4:3 slot 9 is a steady 3). At 120 Hz the default 150% could not hold:
redraws took 19% of the emulation thread, emulation fell behind and the
safety net paused the overclock 5 s at a time. Now the redraws shed load
first (presents every 2nd/3rd refresh) and the overclock stays. 150%/120 Hz:
slot 9 about 93% of frames at 2-3 fields (was about 30%), slot 10 98% at 3.
Candidate `18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`,
[note 104](104-d23e-busy-scene-stutter.md). Nothing committed. Next: the user
plays slots 9/10 at `--cpu-overclock 150` and judges turning and audio.

## 2026-10-05 - D11D and D12B accepted; closeout

User: "i can confirm that i accept both jobs as complete! document commit
push". First-person joints are found by part through the model's table, so
the cowboy levels (1, 2, 3, 27) get the right eye height, head hide and kick
leg. Regression baseline
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
No next job selected.

## 2026-10-05 - D11D and D12B: first-person joints found by part, Needs playtest

The cowboy costume (levels 1, 2, 3, 27) orders Duke's joints differently: the
neck is joint 15 there, not 9, and the right leg is joints 5-8, not 14-17. The
eye, head hide, weapon torso, arm hiding and kick leg now find joints through
the model's own part table (`desc+0x24`). The neck sits 691-705 above the toes
in all 21 levels, and the cowboy kick shows the jeans and brown boot.
Medieval, Roman and the first map are unchanged. Candidate
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
Nothing committed. Next: the user checks slot 8 (dancer height) and the kick
in a cowboy level. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 accepted; D08T2/D22C accepted; closeout

User: "accepted!! done, commit. great work". Accepted today: D22C (F10
recapture opts back in to automatic capture), D08T2 (object type range 1062:
Duke-symbol blocks push with RMB; hints on every contact) and D08U1 (slot-12
ladder: last-rung stop and let-go, mount blend finished after dropped frames).
Regression baseline
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
New Todo: D08T3 (free manual push/pull, suggested next), D08O1 (fire while
swimming); D17R gained the UI slot 2 medieval sky report. No next job
selected.

## 2026-10-05 - D08U1 cause found: mount blend cut short by dropped frames

The user's `[TTK ladder]` log showed the mount ending at y -9114 (offscreen
always -8901), and Duke frozen there while S looped the step poses. With
dropped frames the original 156 transfer ends before the host's 12-update
blend, leaving Duke above the climbing line. Reproduced with a longer
diagnostic blend (stuck at -9115 with the old code); the host now finishes
the blend while Duke rests on the ladder. Candidate
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 not reproduced; robustness fixes and ladder log line

User: slot-12 ladder still glitches on S. Their log shows Duke never reached
the bottom (only 186-189 near the top, three retries). Not reproduced with a
private copy of their profile (first person, 120 fps) for held/tapped S, E
held, E taps or mouse look. Fixed: E-held swing at the bottom, rest-pose
latch, E's Cross blocking the let-go. New `[TTK ladder]` session-log line per
ladder pose change. Candidate
`071487b1f35bcbfd9bf4734c69fd06926fb1b1c11283c2c29dce80156428784a`.
Next: the user retries and describes the glitch; read their newest
`recomp/build-local/logs/session-*.log`.

## 2026-10-05 - D08T2 accepted; D08U1 slot-12 ladder bottom fixed, Needs playtest

User accepted D08T2 (type range, hints) except the slot-12 ladder ("he just
glitches out"). Cause: the ladder's last rung is ~990 above the floor, so the
original swings off sideways (156 -> 211) into a bottom-rung hang (207). Now
the player update asks the original's own probe `0x8007ded0`; at an open
bottom the descent flag is cleared so the original stops on the last rung,
and S lets go with the original Square (clean fall and landing). Hold or tap
S both work in third and first person; W climbs back; the sewer ladder keeps
its 185 step-off. Candidate
`cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.
Nothing committed. Next suggested: D08T3 (free push/pull).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 retest fixes; D08T3 added

User retest: hints never reappeared (D08T1 capped them per session; now every
fresh touch/grab/ladder top, at most every ~5 s); slot-12 ladder S glitch
(the ladder ends above the floor and the original hangs from its bottom rung;
S now lets go with the original Square and Duke drops to the floor). New Todo
D08T3: free manual push/pull while holding Grab, suggested as the next job.
D08T2 and D08U1 Needs playtest. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 Needs playtest (also fixes D08U1)

The medieval Duke-symbol block (UI slot 1) is object type 924; the shared
object type lookup in `push.inc` refused types `>= 512`, but the original has
1062. Fixed from the original allocation (`0x7428 / 28`). RMB now grabs and
pushes it in third and first person; the D08U1 slot-12 ladder (type 639) now
mounts with E and descends; dumpster and Vanilla unchanged; native tests pass.
114 more climbable types now reach the D08X mantle too. Candidate
`06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
Nothing committed. [Note 103](103-d08t2-object-type-range.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22C accepted; D08T2 in progress; D08O1 added

User: "I accept this work." D22C is Accepted; candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b` is the new
regression baseline. D08T2 selected (UI slot 1, file 00, medieval
Duke-symbol block). New Todo D08O1: fire weapons while swimming (UI slot 2,
file 01), modelled on the D08Q3 jetpack work. D17R gained the UI slot 2
medieval sky report ("strange behaviour with the sky directly ahead").

## 2026-10-05 - D22C control mode persists through level select, Needs playtest

Reproduced with real keys: after an F10 release and F10 recapture, every
later `level N` (11 and 12 tested) arrived with the mouse free, which looks
like Legacy controls until F10. F10 that captures now opts back in to
automatic capture (`recomp/src/ttk/pc_input.cpp`); an F10 release still opts
out. Not specific to Level 11. Native input/controls/aim/near tests pass.
Candidate `f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22B accepted; game-wide playtest backlog

User: "Accepted." D22B is Accepted; executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the new
regression baseline (the candidate entry below is historical). The user's
`level N` playtest passed: first-person height and costume-aware kick in the
medieval and Roman/HOG HEAVEN eras, Level 9 armed rolls, general playability.
New Todo jobs, none started: D23C (medieval moat/Necro slowdown, profile
later), D23D (Level 9 strip-club-area slowdown, low priority), D08U1 (player
slot 12 ladder, E cannot descend; file 11 SHA-256 `1608ee9c...`), D08T2
(Duke-symbol pushable blocks, modern RMB grab game-wide), D22C (Level 11
started in Legacy controls), D26B (console leaves first person), D26C (console
history), D26D (authoritative level-select numbering; level numbers in these
reports are console indices until D26D). D17R now also reproduces in HOG
HEAVEN. Documentation only. No next job selected.

## 2026-10-04 - D22B every level candidate, Needs playtest

Modernized controls and the selected first- or third-person view now work in
all 21 levels `levels` lists. The lease authenticates each level by a body
derived from the owned disc (`recomp/tools/local/level_overlay_guards.py`,
`--check` keeps `control_guards.inc` honest). Dodge rolls and steep-slope
slides keep the mouse camera and view; the original's own back-steps and
strafes are taken over by the lease; unowned jumps keep the camera in third
person too; landing poses 94/95/106 keep it. Statistics screen, savestates,
pause and level select return Modernized control by themselves. Offscreen:
zero identity refusals in every level; LEVEL00 slots and Vanilla unchanged.
Not covered: first person while swimming (new D11E), real death/Continue and
natural level exits outside LEVEL00; occasional jump camera gaps seen in two
runs are unexplained. Candidate
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017`; codegen
hash unchanged, savestates load. Nothing committed. See
[note 102](102-d22b-every-level.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D26A accepted; next D22B

User: "excellent! mark as accepted." D26A is Accepted; executable
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851` is the new
regression baseline. The candidate entry below is historical. **Next: D22B,
Todo - next** (not started): Modernized controls and first person in every
level. Today only LEVEL00 and LEVEL01 authenticate; the other 19 selectable
levels fall back to original controls and third person. Use `levels` /
`level N` to reach them. The user authorized documentation, commit and push.

## 2026-10-04 - D26A debug level select candidate

Backtick console: `levels` lists the 21 levels the original title-screen
level-select cheat offers, with the game's own names (0 TIME TO KILL, 1-3, 5-12,
21-26 challenge stages, 27-29 bosses). `level N` ends the current level with the
pause menu's restart code and loads level N through the original mode 1 init.
The level index changes only inside that init; changing it earlier crashed Old
West pairs. Offscreen: all 21 levels load in one session with their own
overlay; savestates, Vanilla and the pause/title refusals work. Modernized
controls stay limited to LEVEL00/LEVEL01 (D22B). Candidate
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`; codegen hash
unchanged. **Needs playtest:** in a game, backtick, `levels`, `level 2`,
savestate there. Nothing committed. See
[note 101](101-d26a-level-select.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D22A accepted; new Level 2 jobs

User playtest: "I can confirm and accept D22A as working." The portal into
LEVEL01 (the user's Level 2) works and the level is completely playable with
Modernized controls. D22A is Accepted; executable
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0` is the new
regression baseline. The candidate entry below is historical. See
[note 100](100-d22a-portal-level-identity.md).

New Todo jobs (reproductions: player UI slots 8/9 = files 07/08, LEVEL01;
hashes in the board entries): D11D first-person eye height (slot 8), D12B
costume-aware first-person kick (slots 8/9), D17R sky black at the edges when
looking up (slot 9), D26A debug level-select panel, D22B Modernized controls
owning every level, state and transition end to end. The user's rule: fix the
shared system, not the level. The user plans to survey the rest of the game
with D26A. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D22A portal level identity candidate, Needs playtest

Cause: the Modernized lease authenticated only the LEVEL00 overlay body; the
first map's portal loads LEVEL01.OVR over the same base `0x800ca968`, so the
lease failed closed and original controls/third person returned. LEVEL01 is
now an authenticated level overlay (tag-keyed `level_overlays[]` in
`control_guards.inc`, owned-disc digests, same scratch-tail rule as LEVEL00);
unknown levels still fail closed and the apartment conveniences require
LEVEL00. Offscreen: movement, mouse look, jump, fire, pause/resume and first-
and third-person all work after the portal, repeated in one session, with zero
identity refusals; the 12 LEVEL00 slots and Vanilla are unchanged. Tests pass;
codegen hash unchanged. Candidate
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0`.
**Needs playtest:** UI slot 7 -> portal -> continue, then play the Old West
level with Modernized controls in first and third person. Nothing committed.
See [note 100](100-d22a-portal-level-identity.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q accepted (Done)

User, after extended play: "the whole job is 100% accepted", and
praised how cleanly the game now plays. D17Q (residual wall
flicker from per-triangle UV seams) is Done, user-accepted. Accepted
executable `a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960` is the new regression baseline. The candidate entry below is
historical. See [note 99](99-d17q-uv-seams.md) for cause, fix and limits. The
user authorized documentation, commit and push. No next job selected or
started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q UV seam candidate, Needs playtest

The diagonal line still visible after D17N (UI 12 panel at the crosshair on
load, flickering on W/S) is a one-texel UV seam, not geometry. The GL/VK
backends applied the 2D mirrored-sprite UV bump (`gpu_uv.h`) per triangle.
A world-wall triangle with an exactly vertical integer edge qualified while
its partner did not, so the two halves of one polygon sampled one texel
apart, and movement toggled which halves qualified. Traced positions, integer
SZ and UVs form one consistent perspective map (about 0.03 texel); an offline
exact render is seamless. Fix: perspective-corrected 3D triangles use full UV
limits and no sprite bump (`PSX_UV_3D_LEGACY=1` compares). Original textures
and Vanilla are pixel-identical; UI 12 and the UI 11 stage canopy lose their
seams. The 12-slot 120 Hz sweep and 60/180 Hz runs had zero replay misses.
Tests pass and the codegen hash is unchanged, so savestates load. Candidate
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with
texture precision `corrected`. Nothing committed. See
[note 99](99-d17q-uv-seams.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N accepted; D17Q added

User playtest: "The result is excellent." D17N (world subdivision zigzag; the
user's message called it D17P) is Accepted. Accepted executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Cause, solution and fallback behaviour: [note 98](98-d17n-world-subdivision.md).
The candidate entry below is historical. New Todo job D17Q: residual subtle
wall/surface flicker as independent Corrected-renderer stability polish (see
the job board; references D17N, D17P, D15 and R01). D18D (music after Continue)
remains Todo. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N world subdivision candidate, Needs playtest

The vibrating diagonal lines on walls (UI slot 12, and generally) come from the
original world renderer: every polygon within `0x2000` units goes through a
screen-space subdivision (`0x80012960` triangles, `0x8001205c` quads) whose
midpoints are integer screen averages with rounded UVs and no projection data.
D15's Corrected textures therefore skipped those pieces and they were drawn
affine; the bends follow piece diagonals and move with the eye. With Corrected
texture precision, world polygons whose corners all have exact projections and
fit the GPU primitive limit are now drawn whole with exact perspective (hooks
`0x800114EC`/`0x8001160C`; `DNTTK_WORLD_SUBDIVISION=1` compares the old path).
Original textures, Vanilla and software are unchanged.

Private offscreen evidence: straight panel borders across 120 Hz W/S sequences;
12-slot sweep, exact-projection share 0.29-0.43 to 0.73-1.00, lower packet use,
game rate unchanged, zero budget hits or replay misses; 60/180 Hz checked.
Tests pass; codegen hash unchanged, so savestates load. Candidate executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with texture
precision `corrected`; recheck accepted D15/D17 cases. Nothing committed.
New backlog job D18D (music silent after death and Continue) was added, not
started. See [note 98](98-d17n-world-subdivision.md).

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P accepted (Done)

User: "i fully accept this fix." D17P is Done, user-accepted: the Modernized
Draw distance option (`extended` default) removes the UI slot 8 subway bands.
Accepted executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
The user authorized documentation, commit and push. The candidate entry below
is historical. See [note 97](97-d17p-distant-bands.md) for cause, evidence and
limits. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P distant bands candidate, Needs playtest

The UI slot 8 subway bands are three original rendering limits, present in
Vanilla too: the far cut-off (rooms past the render limit are never drawn and
the fog fade before it spans a pixel or two), integer portal rectangles a pixel
or two short of distant openings (whole ceiling strips dropped), and integer
NCLIP on one-pixel distant faces. New Modernized **Draw distance** option,
`extended` by default (`original` restores the limits; Vanilla always original):
render-only limits doubled, portal rectangles 2 native pixels wider, exact PGXP
NCLIP signs when geometry or texture precision is Corrected. Profile schema 24,
`run.py --draw-distance original|extended`, settings menu **D**.

Private offscreen evidence: matched 4x views show a continuous ceiling and the
real corridor end; all 12 private slots keep their game rate with ring use at
most 50% and no budget hits or replay misses; 120 Hz cost unchanged; Vanilla
untouched; existing savestates still load (codegen hash unchanged). Tests pass.
Candidate executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
**Needs playtest:** walk forwards in UI slot 8 and look around; recheck the
accepted D15 cases. Nothing committed. See
[note 97](97-d17p-distant-bands.md).

Launch: `python3 recomp/tools/local/run.py` (the first launch migrates saved
settings to schema 24 with Draw distance `extended` and keeps a recovery copy).


## 2026-10-04 - D15 Accepted; new subway slot 8 is D17P

The user accepts the final playtest: slot 5 left-side black/missing areas no
longer reproduce, slot 6 closet and surrounding furniture are stable, and the
previous slot 8 door remains opaque. Geometry stability, floor seams and idle
polish were already accepted. D15 is **Accepted** at the primary 120 Hz target.
See [implementation, tests, user evidence and limits](96-d15-accepted-precision.md).

The user explicitly authorized documentation, implementation commit and push
on the current branch. Preserve the title changes and exported framework patch;
Vanilla remains available and precision is optional in Modernized/OpenGL.
Accepted executable: `35756df57b9a6ddd31ee0dabdffb51876faf7a3b0c05effb78be285dfd9dc9a7`.

**Immediate next job: D17P**, selected by the user. They replaced UI slot 8
with a subway corridor state: walk forwards and watch multiple horizontal
black bands in distant geometry/textures. Treat "possible culling" as a
hypothesis, compare rendering paths, and preserve the accepted D15 baseline.
This is separate from the accepted previous slot 8 opacity case. The new
state identity, private-copy intake and acceptance criteria are on the
[canonical board](../MODERNIZATION_JOBS.md#d17p---distant-horizontal-black-bands-in-the-new-subway-slot-8).
No technical cause is established by intake alone.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L accepted (Done)

User: "first off, i accept this as complete!" D17L is Done, user-accepted.
Preserve implementation `1e6fb0a` and the evidence in
[note 95](95-d17l-tabletop-depth.md). Earlier Needs playtest and unresolved
candidate entries below are historical. Busy-scene 180 FPS limitations remain;
this acceptance does not close other jobs or establish full campaign coverage.

The user clarified the workflow: implement and test, obtain acceptance, then
wait for an explicit instruction to document, commit and push. They authorized
this closeout in the next message. AGENTS.md records that approval requirement;
background development testing remains authorized. This closeout changes only
documentation and instructions, with no new build, launch or gameplay test.
No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L tabletop depth fix, Needs playtest

Runtime primitive/provenance capture identifies the remaining cup cut-off:
its depth-tested triangles were followed by a native tabletop quad that
painted over them without depth testing. The fix gives eligible opaque upper
faces of authenticated static eight-corner boxes depth while retaining native
integer positions, affine textures, quad topology, culling and OT order.
The broad upper-surface experiment was rejected on measured cost.

Approach/retreat captures at 60/120/180 retain the full cup. Accepted shelf,
closets, blood and subway routes remain intact; replay restore oracle passes
312 checks, Vanilla reports no host depth/replay workers, and Python runs 102
tests: 100 pass, two conditional skips. Primary 120 samples have zero output
underruns; 180 remains below target in these busy scenes with a small added
candidate deficit. User visual/audio acceptance is still required.

The user replaced the no-launch restriction: background/offscreen development
tests are authorized; foreground windows are reserved for necessary visual
verification. AGENTS.md records this. Player files are unchanged and test
games are closed. No Disruptor code was copied; no framework/generated code
changed. See [cause, fix and evidence](95-d17l-tabletop-depth.md).
Launch: `python3 recomp/tools/local/run.py`; F7 UI 11, F10 if capture is needed.


## 2026-10-04 - D17L selected; offline primitive diagnostics ready

D17L is In progress for the bounded cup diagnosis. Added opt-in title-owned
per-instance/per-face tracing and offline observational-equivalence checks.
No Disruptor code copied. The private UI 11 hash matches note 89. Native packet
and controls fixtures pass; the player build succeeds. No new game run or
cause/fix is claimed. Isolated diagnostic launch authorization was requested
under AGENTS.md and is pending. See [scope, evidence and continuation](94-d17l-primitive-diagnostics.md).
R01, D08Q3 and D17O remain Done; accepted rendering baselines are unchanged.


## 2026-10-04 - R01 DisruptorRecomp reference research complete

[Comparative review and pinned source index](93-disruptor-reference-research.md).
R01 is Done as a research-only job. Retain clean, ignored
`research/DisruptorRecomp` at `408f214d3109dbc6cbdded7edd128cbf8de6466a`,
framework `193a60b805e1eaa853129d6ccf63022440d4b143`; project patches were read,
not applied. The review records acquisition, architecture, all requested systems,
source/functions, applicability, licensing/provenance and ranked experiments.

Recommended next choices: a primitive/provenance census for D17L/M/N, D15's
optional Original/Corrected geometry with independent texture correction, and
measured CPU-accounting overhead. No experiment selected or started. Disruptor's
image crossfade does not replace D17; its gap rims do not establish fixes for
TTK culling/depth; TTK already has gameplay-scoped CPU overclock. Preserve
accepted D17/D17B/D17O and the primary 120 FPS baseline.

User confirms research-only reuse: learn techniques and write purpose-built TTK
code, with no 1:1 copying or mechanical translation.

The included license text differs from standard PolyForm Noncommercial despite
its label; clarify before future code adaptation. No source imported. Ignore
and clean-reference checks passed. Static research only: no build, launch,
gameplay/renderer edit or player-file write; no new gameplay evidence claimed.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O accepted (Done)

User: "amazing work!!! accepted. well done. document. commit. push".
D17O is Done, user-accepted. Preserve implementation `25734d1` and player binary
`8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
The preceding implementation's Needs playtest status below is historical.

[Architecture, cause, verification and remaining limits](92-d17o-sky-intake.md).
Original sky geometry/coarse animation and busy-club 180 FPS limitations remain;
this acceptance does not establish full campaign coverage or close other jobs.
Documentation-only acceptance closeout: no new gameplay test, build, launch or
player-file write. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O sky camera-relative replay repair (Needs playtest)

Verified original sky architecture and a high-refresh defect: the three
camera-relative sky matrices were recorded under the last world object, then
substituted with older world-space matrices. A narrow, resident-code-authenticated
exemption retains the original sky transforms, art and timer. In the sampled
turn sweep, camera-origin displacement fell from up to 1075.397 units to zero.
No framework, generated C, flight-control, clipping or media changes.

Private captures cover 60/120/180, ground first person, third-person flight,
multiple headings/pitches and a second street location. Near/input/aim/controls/
inventory suites and final player/shard checks pass. Modern/Classic pistol fire,
reticle eligibility, actual pack-off, landing/first-person return and reflight
pass. 1106 worker surface checks had zero mismatch. All 28 player file hashes
and mtimes match intake. Existing runtime patch stack verified already applied.

Primary 120 FPS routes were clean. Busy club 180 FPS still repeats frames and
can underrun audio, reproduced with the previous sky path too; do not claim
universal 180 FPS stability. Original coarse cloud motion and high-pitch geometry
limits remain. User sky appearance confirmation is required; no next job started.

[Full architecture, provenance, diagnosis, A/B evidence and limits](92-d17o-sky-intake.md).
Build SHA-256: `8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10 and look/turn/move;
F10 captures Modernized input if needed. The fix applies to high-refresh redraws.


## 2026-10-04 - D08Q3 accepted; next D17O sky appearance

User accepts D08Q3: "i accept this! great work." Done; preserve `f284dc1`.
The older Needs playtest entry below is historical. No new gameplay test was
performed for this acceptance closeout.

**Next: D17O, Todo - next.** Load UI slot 10 (file 09), look up at the sky and
investigate its strange/glitchy movement. This is not jetpack-specific. First
establish how the original game renders/animates sky, then diagnose and test a
bounded stabilization. The user explicitly requires substantial documentation
of architecture, cause, evidence, implementation and remaining limitations.
[Canonical job](../MODERNIZATION_JOBS.md#d17o---unstable-sky-appearance-when-looking-up-slot-10).

[Detailed intake/plan and exact save identity](92-d17o-sky-intake.md).
Immutable private snapshot: `recomp/analysis/d17o-sky-intake-20261004/cards-intake`;
28 files with adjacent hash/mtime manifest. Current slot 10 differs from the
older jetpack fixture. Use a separate writable copy and private preferences.
Do not assume a skybox, parallax defect or high-refresh cause from the report.
Preserve Vanilla, accepted visual work and D08Q2/Q3.

This checkpoint only records acceptance/backlog and copies intake data. Sky
investigation has not started; no code edit, build, game launch or player-file
write. Commit/push this handoff before the user clears context.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q3 implemented, Needs playtest

Flight now admits Modernized view aiming, weapon presentation, energy-beam
completion and the enabled crosshair through the existing guarded jetpack lease.
Ground movement eligibility, flight physics, Vanilla and D08Q2 are preserved.

Private Modern/Classic routes verify six weapon types, mouse yaw/pitch, firing
while moving/climbing/descending, I toggle, J-off, fuel exhaustion, landing and
first-person fallback/return. Native aiming/controls/input/inventory suites and
the player build pass. All 28 player file hashes/mtimes match intake.
[Cause, verification and coverage limits](91-d08q3-jetpack-aim.md).

User confirmation remains. Select weapons before takeoff; flight switching is
unchanged. Thrown/upgraded variants are not all separately live-validated.
No next job started. Build SHA-256:
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.

Launch: `python3 recomp/tools/local/run.py`. F10 captures Modernized input if
needed. Check the reported jetpack aiming/crosshair in both flight schemes.


## 2026-10-04 - D08Q2 accepted; next D08Q3 jetpack aiming/crosshair

User confirms D08Q2: "confirmed fixed! accepted." Marked Done; preserve the
accepted recovery from `2f02967`. Earlier Needs playtest and immediate-next
D08Q2 entries below are historical. No additional gameplay tests in this closeout;
the natural map-pickup replay remains outside the recorded automated evidence.

**Next: D08Q3, Todo - next.** User cannot aim to shoot while using the jetpack,
and the crosshair is missing. Cause, affected weapons/scheme and exact location
are unverified. Read the [canonical job](../MODERNIZATION_JOBS.md#d08q3---jetpack-weapon-aiming-and-missing-crosshair)
for scope/acceptance. Investigate view aiming and reticle eligibility separately;
preserve Modern/Classic flight, ground aiming, existing flight camera transitions,
Vanilla and the accepted D08Q2 fix. The old slot-10 private snapshot is a regression
fixture, not yet a confirmed reproduction of this new issue.

Documentation only this session: D08Q3 has not started; no code edits, build or
launch. Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q2 repaired, Needs playtest

The reported UI slot 10 is repaired automatically in captured Modernized ground
play: original jetpack pending `0x8001` becomes owned/off `0x1`, with full fuel
unchanged. Death during deployment can strand pending across Continue; the
original `dnstuff` grant preserves it. A private extended-deployment timing
fixture plus real RPG death/Continue reproduced the baseline failure and verifies
the repair. The user's exact original timing is not established.

Only the inactive closed/off endpoint is repaired. Picker/J, Modern and Classic
flight, fuel rejection/depletion and private save/load pass. Native controls,
input, aiming and inventory suites and the player build pass. All 28 player
card/state hashes/mtimes still match intake. Framework/renderer unchanged.
Natural map-pickup replay and user acceptance remain; no next job selected.
See [D08Q2 evidence](90-d08q2-jetpack-continue.md).

Build: `c8f979d50bd502ded05294fde2e55b59c90f132f386d21a89865538ca8836a41`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10, try brackets,
J and Space; F10 captures Modernized input if needed. No repeat cheat required.


## 2026-10-04 - Immediate next investigation: D08Q2 jetpack unavailable

The user will clear context after this checkpoint. **Start next with D08Q2**,
not another D17 job. Status: Todo - immediate next; investigation has not
started. User sequence: died -> Continue -> returned to gameplay -> entered
`dnstuff` -> jetpack cannot be used, and the inventory picker skips it.
Failing state is **UI save slot 10 (file 09)**.

Private snapshot: `recomp/analysis/d08q2-jetpack-20261004/cards-reported`;
manifest alongside it. Slot hash:
`161253aa41f4f2c407f57ef7cff1371ba7889c4bdeba793183f9045b6184e18e`.
Use private cards/settings. The saved result may not reproduce the preceding
transition by itself; compare working inventory and the full death/Continue/
cheat sequence. Trace ownership, fuel, picker eligibility and activation
alongside Continue/reset and cheat grants. Cause is unknown; picker exclusion
is evidence to inspect inventory state, not proof of a particular defect.
See the canonical [D08Q2 job](../MODERNIZATION_JOBS.md#d08q2---jetpack-unavailable-after-death-continue-and-dnstuff).

This checkpoint only records the report and preserves a copy. No gameplay
code, build, launch or player-file changes. D17L stays open; D17M/N remain
queued; D17C/D/E/F/K accepted behavior must be preserved.


## 2026-10-04 - D17E/K accepted; D17L open; new D17M/N backlog

The user accepts D17E subway peripheral visibility and D17K stable ground
blood. Preserve their current behavior from `72085d7`. D17C remains Done;
D17D/F remain Accepted. D17L returns to Todo: the isolated cup still flickers,
while nearby bar cups/bowl remain stable. It does not merely await acceptance.

Subsequent user retest on 2026-10-04 reports no visible change. D17L remains
open (Todo); no new fix is claimed. This checkpoint documents that retest
only. Further implementation is deferred.

Offline inspection of current UI 11 confirms polygonal table/cup meshes and
shared cup/bowl prototypes across table and bar placements. The residual
instance-level failure is not yet explained; the next L pass must trace the
specific cup and occluding surface across good/bad frames. No gameplay changes
or game launch occurred in this follow-up.

New separate Todo jobs: D17M, UI 9 shotgun ammo visible through its platform
while climbing the ladder; D17N, UI 12 jagged diagonal/vibrating walls during
forward/back movement. Causes remain unclassified. Slots 9/12 have changed;
preserve older private shelf/blood states for accepted regression checks.
[User results, current save hashes and mesh inspection](89-d17-playtest-followup.md).

Launch from this workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17C accepted; D17E/K/L visual candidate

D17C is Done on the user's acceptance of the current camera stability and look.
D17E/K/L were selected together and are now Needs playtest. D17D/F remain
accepted; no other job started.

- D17E: no missing peripheral subway wall reproduced on full-width wall/edge
  and stair routes at 60/120/180. Awaiting player closure, not a claimed new
  visibility fix.
- D17K: enhanced floor pieces now retain the original source polygon's
  farthest-corner ordering key, preserving native blood in front of the floor.
- D17L: compact static props (up to 256 local units per axis, authenticated
  caller) keep depth across the near radius. Opaque faces share an ordering
  key to avoid excessive draw-state changes. Actors and larger distant
  scenery keep the previous takeover boundary; Vanilla remains available.

[Causes, private save identities, implementation and verification](88-d17e-k-l-visuals.md).
Current UI 11 is the table report, UI 12 the blood report; older closet states
were checked separately. The accepted shelf/closets and camera are preserved.
Final 120-target samples have zero audio underruns, worker failures or new
mesh-budget fallbacks. At 180, final club samples are about 178-179 distinct
images/s, with a small table-route output underrun still measured; do not claim
zero-cost 180 FPS or actual-device audio acceptance. User should also listen
for crackle at their usual refresh setting.

New owned-EXE packet/guard tests and existing near-plane math pass. Python:
98 passed, two skipped. Original player files/settings were not written.
Framework patch and generated game code are unchanged.
Candidate SHA-256: `151ea6aab6957eeb2c3da5cf990700d566a227eb723eb829d13600146c098520`.

Historical candidate next action (superseded by the 2026-10-04 report above):
user playtest of UI slots 3, 11 and 12, starting at 120
and then their usual refresh setting. F10 captures Modernized input if needed.
Launch from the workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17D/F accepted; next-round visual and portal bugs

The user explicitly accepts the UI 9 prop, UI 11 closet and closet inside
the strip club as fixed. D17D and the related closet job D17F are Accepted.
Preserve the source-plane depth correction and bounded static-prop contact
tolerance from `b88d0e5`; do not reopen these jobs for the separate new reports.
Accepted binary: `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
[Cause, implementation, tests and user acceptance](87-d17d-contact-depth.md).

Next-round backlog (all Todo, none started):

- D17K: UI save slot 12, blood on the ground directly in front of Duke.
  User describes coplanar blood; exact renderer/placement cause unverified.
- D17L: UI save slot 11, props sitting on the table directly ahead. Walk
  forwards/backwards: props get cut off and reappear as Duke moves closer.
  This is distinct from the accepted closet in that save.
- D22A: UI save slot 7 -> walk into the portal -> start the next level.
  Duke loses all Modernized controls and first-person view. Cause unverified;
  inspect transition/overlay coverage and control/view state without bypassing
  identity guards. Preserve the selected mode and camera when gameplay resumes.

Read the new board entries for scope and acceptance. Verify current save hashes
and make dated private copies before testing; slot numbers alone do not prove
that an older `cards-user8` snapshot is still the current reproduction. Use
background/offscreen tests and preserve the accepted UI 9/11/club fixes.
The user will clear context and choose the next job; no implementation starts
from this handoff alone. This closeout changed documentation only, with no
build, game launch or player-data changes. Launch: `python3 recomp/tools/local/run.py`.


**D17A ACCEPTED. D17B ACCEPTED (2026-10-03).** The user completed natural
first-level play at 120 and 180 FPS and explicitly closed the parent jobs.
120 FPS is the primary quality/regression baseline; 180+ excellent high-refresh
support, 240+ robustness/compatibility, Unlimited stress/debug. No technical
ceiling or gameplay-speed change. Residuals are D17D-J and D18C, not reasons
to keep the accepted parent jobs open. No follow-up implementation started.

Accepted binary: `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
Preserve mouse response, opening/club playability, apartment/culling fixes,
FMVs/audio, FPS reporting, idle stability and intentional PS1 motion.
[Acceptance, source baseline and reproduction ledger](86-d17-acceptance-and-regression-baseline.md).

The repository now tracks the actual `recomp/` implementation and complete
framework patch with pinned dependencies. The prior documentation-only exclusion
was explicitly revoked by the user. ROMs/generated game code/builds/saves/research
remain ignored. Original disc dump moved intact under `game/`; prepared runtime
copy retained. Older statuses and results below are historical and superseded.


**Latest: accepted playability baseline preserved; bounded polish investigation.**
The user reports the opening, strip club and apartment are now smooth, enjoyable
and substantially more stable. This acceptance applies to build `28554051...`.
The follow-up disables unnecessary per-batch GPU timing queries in ordinary
launches and adds opt-in pose-readiness tracing. No interpolation, culling,
physics, audio-buffer or mouse-sampling change is shipped.

Slot 12 Shift+W/S still exhausts usable interpolation history while presentation
intervals remain mostly regular; complete geometry arrives too late for some
planned movement frames. Larger buffering and stronger phase correction were
rejected. Slot 10 can starve audio production under the scripted stress route;
slot 1's crackle did not reproduce as an underrun. These remain open polish
items. Opening/dancer mouse regressions retain about 179-180 images/s, zero
underruns and no camera turn over one degree. D17D now holds isolated visual
artifacts (slot 9 also reproduces at 60 Hz). D17A/B remain Needs playtest.
See [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md).
Current executable SHA-256:
`1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.


**Latest: save-driven camera stability candidate, D17A/B Needs playtest.**
This supersedes note 83's performance conclusions. Five-field dancer updates
were incorrectly resetting camera yaw; expired replay jobs also hid their
cost from scheduling. Those causes are fixed, compatible opaque/translucent
batches reduce the slow states' main-thread submission cost, and X11 mouse
timestamps are repaired. Known scratch allocations now precede worker startup.
Final 180 Hz saved-state sweeps produce about 179-180 distinct images/s with
zero audio underruns and no adjacent camera turn over one degree. The fresh
opening has presentation p99 6.07 ms, max 7.83 ms from its first camera redraw.

Unlimited still exposes stale-camera episodes. Subway UI slot 3 did not
reproduce on the bounded 60/180 Hz capture route and remains open, alongside
earlier minor geometry pops. Player saves/cards/settings remain untouched.
Build SHA-256 `28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
See [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md).
The next step is a player playtest of this bounded candidate, especially UI 5.


**Latest: opening/club responsiveness candidate, D17B Needs playtest.** This
supersedes the older performance results below. Final fresh boot at CPU 100%,
first person, 4x, 180 Hz: camera presentation p99 6.0 ms, maximum 8.3 ms from
the first valid redraw, zero audio underruns or failed/killed workers. Final
Match Display dancers turn: 179.5 distinct images/s, p99 6.18 ms, realtime
emulation and zero audio underruns; apartment comparison 179.8 images/s,
p99 5.98 ms. Mouse injection to traced camera presentation: median 17.85 ms
club / 12.48 ms apartment. These are bounded software measurements.

Systemic fixes cover timestamped scheduling, restored/worker code-state
coherence, an exactly guarded initialized math routine, GL batching, unnecessary
worker replacement for invalidated movie code, and a worker timeout race.
Vanilla and the accepted idle-eye behavior remain available. Minor closet and
furniture pops remain D17A playtest work. Existing player settings/saves/cards
are untouched. Build SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
See [83-opening-responsiveness.md](83-opening-responsiveness.md) for evidence,
limits and reproduction. The next step is the user's fresh-game full playtest.

**2026-10-03 renderer quality pass:** D17A/B have a new stabilization
candidate, with replay color/depth isolation fixes, coherent eye setup for
room walks, transform lookup and vertex-provenance safeguards, and tail-aware
redraw scheduling. Final 2x isolation: 459 checks, zero mismatches. Normal
visual route: 24 sequences, no detected back-and-forth jumps. Performance is
still below the target in the club: the final 180-target run produced about
85 distinct images/s while turning; Unlimited about 100. Do not interpret the
older response-time/smoothness claims below as acceptance of this workload.
See [82-renderer-quality-pass.md](82-renderer-quality-pass.md) for profiles,
remaining endpoint uncertainty, regression evidence and build identity.

**2026-10-03 (later):** loss of control fixed (host Circle requests are taps;
held Circle entered the game's inventory hold, `+0x224 0x200`); camera
re-seeded after savestate loads; late camera rebuilt (one redraw per present,
fixed-lead mouse sampling, adaptive lead, pacing hysteresis).

**2026-10-03:** see-through outdoors and in the apartment fixed (host depth
cleared per frame); dancer jerk addressed (in-betweens planned per paced
present, overrun redraws, lead 16 ms); idle stability user-accepted.

**D17A / D17B / D17C - Needs playtest.** Late camera on with adaptive pacing:
every present uses the newest mouse look (step response about 17-19 ms, was
66-130 ms), a new image every 1st, 2nd or 3rd refresh as the PC keeps up. Near
geometry: host precise vertices (exact positions, depth, texture coordinates)
plus a depth buffer among them; correct perspective beside the eye, closet and
collar artifacts gone. First person: stable idle eye (no breathing world), PS1
wobble kept while moving, deliberate view bob option `view_bob`
off/subtle/on/strong. See documentation/81-d17a-instability.md.

**D17 high refresh rate - Done (user-accepted 2026-10-02).** Modernized
`--frame-rate display|30|60|120|144|165|180|240|unlimited` (default 60;
Vanilla always 60): the runtime presents at the display's rate and TTK's own
renderer draws interpolated in-between images in worker processes; game logic,
physics, animation timing and audio unchanged. The user saw about 180 FPS at
Match Display with very good stability, correct FMVs and an accurate fps
readout. Follow-ups: D17A (texture/geometry instability, black areas), D17B
(mouse responsiveness and input latency). Details:
documentation/80-d17-high-refresh-audit.md.

**D08J1 overhead ladder grab - Done (user-accepted).** New job from the user's
slot-6 report. That ladder's bottom rung is about 1040 above the floor, so
the original ground mount never sees it and walking into it stops at the
wall; jumps from the wrong distance bounce (107). Modernized now jumps for
Duke once per E hold when he heads at such a ladder inside the catching
window (run 750, walk 800, standing 450 from the panel), keeps the reach held
through the leap (an E tap at the wall works) and eases him along the panel
in the air; the original catch and climb do the rest. Private slot-6 copy:
all E approaches caught (first/third person, manual/assisted jump, armed or
holstered), none without E; D08U/D08X/D08Y/D08W regressions unchanged.
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Details: documentation/79-d08j1-ladder-leap.md.

**D08Z manual jump style - Done (user-accepted).** Optional Modernized
`--jump manual` (assisted stays the default): jump on the press near gaps,
~0.15 s grace after running off an edge, WASD air steering up to the takeoff
speed (also during the E reach), standing/walking takeoff ~3x quicker. Slot-5
gap clearable by timing alone (last ~200 units or just after the edge; earlier
presses scramble up the low lip; E mantles from 1000 back). Assisted unchanged.
Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`. Details: documentation/78-d08z-manual-jump.md.

**D08Y gap jump - Done (user-accepted).** Jump, freezes and audio confirmed fixed. The overclock now applies only in
gameplay and backs off if emulation falls behind (fixed boot/loading audio
slowdowns; binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`). Round 4: Random 20 fps freezes were
the emulated CPU overrunning in views the Modernized camera/widescreen reveal;
Modernized now emulates the CPU at 150% (`--cpu-overclock`, Vanilla stock): 0
slow frames on the player's GPU (was 144/59). Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`. Earlier: Reach freeze frames
and the jump-mantle pop fixed (binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`). The gap was not a level-design
slip: the original launches a run jump pressed near a gap from the lip, and
Modernized had switched that off. Restored for real gaps (furniture keeps the
instant jump); a tapped Space works like the original's held button. Slot 5:
without E 8/10 clean landings (was 0/10), with E 14/14. The hitch after E jumps
(up to 27 ms per update) is down to ~6.5 ms. Mid-air mantles remain the safety
net. Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. Details: documentation/77-d08y-gap-jump-research.md.

**D14 widescreen - Done (user-accepted).** Modernized opens in native 16:9 (also
16:10, 21:9, auto; `--widescreen off` for 4:3): wider view with the original
projection, no stretching, no holes at the widened edges (TTK portal root
rectangle widened; near clip in third person), HUD in the corners, movies and
2D menus 4:3. Vanilla unchanged. Binary
`3f726b9173ef236ca7c5f38b5c12b6713a877f85b6bbd5c0b2a8aca5e392af2c`. Unlike
DuckStation's widescreen hack, text, HUD and movies keep their original
proportions. Details: documentation/76-d14-widescreen-first-pass.md.

**D08U, D08W, D08A3, D08X - Done (user-accepted).** Ladder-top mount and full
descent; shallow-water jumps in the held direction; original TTK HUD icons in
the switcher; hold-E jump grabs and mantles (ledges and climbable objects at an
angle, extra reach, flush hang, stacked-crate release, identity-guard fix).
D14 widescreen: first pass only (inert 16:9 plugin; culling gaps open). Binary
`60161f81ab42a013ebfa326d6b80b97772b29b9d8fb051b598ddba45cbde369c`.

**Autonomous chain (user away): D08U, D08X, D08W, D08A3 Needs playtest; D14
In progress.** E at a ladder top climbs down onto it and S descends and steps
off (D08U); E held through a jump catches close ledges (D08X); shallow-water
jumps follow the held direction (D08W); the switcher uses TTK's own HUD item
icons rebuilt from /DATA/FONTS.RAW (D08A3); widescreen first pass found the
activation path and TTK's culling blocker, no player option yet (D14). Binary
`db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`. Native
and Python suites pass. Details: documentation/72 to 76 and 09-handoff.md.

**D04A Escape frees the mouse - Done (user-accepted).** Escape now pulses Start so a
quick tap reaches the original pause poll, and automatic recapture waits until
gameplay has actually stopped (pause open) instead of 12 frames. Native input
test PASS; resume Escape no longer blocks recapture; binary
`198673f5f5643aed5f25c543b79ec69ed0216d69e2f211a075394f3b86ded68e`. User: "perfect, accept".

**D08V sewer mantle lease - Done (user-accepted).** Mantles, hangs and pull-ups,
the frames right after them and falls no jump owns now keep the mouse camera
and modern buttons (no `ORIGINAL MOVEMENT` flicker); the original traversal
routines are unchanged. Private slot-12 sweep: tank fallbacks 12 -> 0, mantles
identical. Current binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`
(includes D07D and D08T1). Details: documentation/71-d08v-sewer-mantle-lease.md.

**D08T1 hold to grab, E always mantles - Done (user-accepted).** In Modernized, E
climbs a pushable object (the alley dumpster) exactly like any climbable
object; holding right mouse (or Alt) grabs it, W/S push/pull, releasing lets
go. With `original` weapon aiming right mouse stays precision aim and Alt
grabs. Profile schema 17. Live private-state checks in third and first person
and with legacy aiming pass; Vanilla push/pull unchanged. Binary
`66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`
(includes D07D). Details: documentation/70-d08t1-grab-manipulate.md.

**D07D red dot off by default in Modernized - Done (user-accepted).** Profile schema
16 turns the original red autoaim dot off in Modernized (older profiles are
migrated off once, with a backup; later choices are kept; `--red-dot on`
restores it). The hide hook now proves ownership at the marker's own enqueue,
so it no longer depends on the first-map camera/identity lease. Isolated
OpenGL route: hidden markers in third person (51), held aim (59) and first
person (65); none with the dot on. Vanilla unchanged. Binary
`db5bf9640893ae582acefa27fca8f98f27017bd6f1aa4c956d203cf31e6ec67b`.

**D13 higher internal resolution and display scaling - Done (user: "correct correct correct, D13 is good. I'd say let's approve it.").** Each
profile now saves an internal 3D resolution (1x original to 4x with OpenGL),
display mode (windowed, borderless or exclusive fullscreen), window width and
output filter (linear or nearest): `run.py --settings` choice R, or
`--internal-scale`, `--display`, `--window-width`, `--output-filter`.
Vanilla stays 1x; Modernized defaults to 4x (512x240 gameplay drawn at
2048x960; user: "holding 60 FPS the whole time it just looks amazing"). The software renderer always runs at 1x (29 fps at 2x). In an
isolated CPU-OpenGL instance 1x-3x held ~60 fps and 4x 50 fps; FMV framing,
HUD and pause menu are unchanged; settings survive a relaunch. Playtest
follow-up: **F11** is now the fullscreen key (replacing Alt+Enter/Ctrl+F,
which never worked because of a runtime modifier-matching bug, now fixed), and exclusive is now a real exclusive mode under SDL3.
The game now opens windowed by default and remembers
how it was left (windowed or fullscreen, exclusive or borderless, window
width). All accepted by the user. Separate backlog from the playtest:
D08U (top-of-ladder mount), D08V (sewer mantle flicker), D08W (subway
shallow-water sideways jumps), D10B (close-camera transparency, doors) and
D11C (the slot-12 headless Duke: a first-person head-hide flag saved inside
the savestate). Details: documentation/67-d13-resolution-display.md. Binary
`79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

**D11C slot-12 headless Duke - Done (user: "duke's head is back").** A first-person head-hide
bit captured in a savestate is now reclaimed at Duke's next draw (Modernized,
either view). The original never sets that bit (static check of the executable
and all overlays). Headless check on a private slot-12 copy: head shown in
third person, still hidden in first person. Binary
`d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

**D08L inertial edge run-off - Done (user: "im happy with that!").** Running off a large ledge in
Modernized no longer brakes: on the fire-escape platform outside the apartment
window the original fall cut Duke from ~47 to 16.5 units per frame (velocity
3058) and froze the mouse camera; he now keeps his running speed (9172) and
the camera through the fall and lands into his run. Walking still stops at
large edges (original), the running jump is unchanged and the late-jump grace
still applies to small drops only. Every edge departure is capped below the
running jump (a run-start stride spike could launch the bed run-off at 14000).
Vanilla route exit 0; native suites and Python PASS. Limit: only this
platform and the bed were measured. Details:
documentation/66-d08l-edge-run-off.md. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

**D12A first-person quick kick - Done (user: "its done! accepted. ... this has been yet another amazing feat of engineering.").** In the Modernized eye
view, **Q** is now a short Duke 3D style kick: Duke's own right leg (original
meshes, posed from the original straight front kick) snaps out in front of the
eye with any weapon still drawn, standing, moving or in a jump, and the view
never leaves first person. The hit uses the original kick's damage sphere,
radius and damage where the crosshair ray meets something within reach
(about 4 contacts, 1500 to a pig cop; three kicks kill; low props like the
alley garbage bags are hit without crouching). With the Boot selected a held
left-click kicks while moving; the thigh is now drawn. With the Boot selected, the original spinning kicks (Q, E,
held attack) become the quick kick in first person. Third person and Vanilla
keep the original kick. Checked in an isolated instance against pig cops on
the first street; 59.9 fps; Vanilla route exit 0; native suites PASS. Limit:
no knockback and lower damage per kick (both accepted by the user); thigh
13% lower on screen (user's choice). Only Q or the attack kicks (never E);
Q chains while running with any weapon. Details:
documentation/65-d12a-first-person-kick.md. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

**D08T pushable objects - Done (user: "it works so much better than the original now. this is it rock solid. confidence level is very high.").** User report: the alley dumpster
could not be grabbed in Modernized; E climbed it or did nothing. In
Modernized, **E** now grabs any original pushable object (even with W held
into it), **W/S** push/pull relative to the camera, **E** lets go, and
**Space** climbs a climbable one. Mouse look stays live while grabbing, and
first person blends to the orbit during the grab. The original grab, push/pull
and climb animations run unchanged; Vanilla keeps the original Action rules.
Checked with real keys from a private copy of the user's slot-5 state (third
and first person); Vanilla route exit 0; native suites PASS. Only the dumpster
was checked. Details: documentation/64-d08t-pushable-objects.md. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

**D12 first-person weapons - Done (user: "finally, we can mark this as accepted!!").** Next: D12A (kick stays in first person). In the Modernized eye view
Duke's own right hand and the original weapon mesh are drawn in front of the
eye (no new assets), held in each weapon's firing pose, always on top of
walls, with the original muzzle flash and a short kick. All 13 weapons on the
first map checked; pitch-stable; 59.95 fps; third person and Vanilla
unchanged. The twin cannons (key 5) are framed like Duke 3D's Devastator;
the HUD stays on top. Limits: instant holster/draw, no left hand, no reload
motion. Details: documentation/63-d12-first-person-weapons.md. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798` (after framing pass 6).

**D11B first-person near-wall clipping - Done (user: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available")).** In the Modernized
eye view, walls, floors and props beside the eye are clipped and drawn by the
host instead of tearing or turning black; everything else stays on the
original renderers (`0x80011020` world, `0x80010000` object). Club side wall
and entry corridor sweeps render continuous walls where the unclipped build
shows black gaps; first person 59.96 fps, third person and Vanilla unchanged.
Third pass: after the user's 1080p report of black floor lines, the default
is a conservative mode (clip only what the original gets wrong, no
subdivision). Compare with `DNTTK_NEAR_CLIP=0`; the subdividing mode is
`DNTTK_NEAR_MODE=full` (research only). User: conservative "looks the very
best". Props no longer fade see-through in the eye view (original occluder
fade; `DNTTK_FP_OCCLUDER_FADE=1` restores). Both are future D19 menu toggles.
Details: documentation/62-d11b-near-clip.md. Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

**D11 first-person prototype - Done (user: "this is phenomenal, and it's like a dream come true").** Modernized (independent
camera): **P** toggles an eye-level view with a wider field of view (about 64
degrees); saved in profile schema 13 (`--view first|third`, default third).
The eye follows Duke's neck through the original camera solve; his head is
hidden only during his own draw; mouse look, camera-relative WASD, view aiming,
crouch and jumps all work in it. Swimming and jetpack flight return to third
person. An isolated first-level route (street, club door fight, doorway,
entry hall, main room) passed all 14 checks at 59.9 fps with 0 underruns;
Vanilla route exit 0. Known limit: steep-angle wall contact can drop wall
polygons (new job D11B). Hands and weapon: D12. Details:
documentation/61-d11-first-person.md. Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`.

**D10 Done (user: "it's done, fully accepted"); D08S cancelled (may revisit).** Modernized
camera: **V** recenters (smooth swing to the original follow angle; the mouse
cancels it), **H** cycles centered / right / left shoulder (off by default),
and Alt+wheel distance plus shoulder side are now saved in the profile (schema
12; `--camera-distance`, `--shoulder`). Shots still meet the crosshair and walls
still constrain the camera. On foot Duke already faces the view, so V mostly
levels pitch. Details: documentation/60-d10-camera-polish.md. Binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`.

**D08R Done (user-accepted: "it's absolutely rock solid").** **D08Q1 Done
(user: "verified working!!!"):** Modern jetpack Ctrl descent 2800 -> 5500, steady 29.5
units/frame to match the underwater Ctrl dive; soft landings, no damage.
Binary `9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

**D08R selectable jetpack scheme (Done).** `run.py --jetpack
classic|modern` (persisted, Modernized, default modern). Classic (revision 2,
after user feedback) keeps all modern controls - mouse look, camera-relative
WASD, J - with the original burst physics: Space boost, gravity, no host
hover or Ctrl descent. Details: documentation/57-jetpack-controls.md (D08R
section). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`.

**D23B Done - intro FMV stutter was a stranded native movie shard.** The
shard's cache folder is keyed by the `game.local.toml` hook list; the 28 Sep
hook additions moved it, so the intro decoder ran interpreted (~52 fps,
continuous underruns). Rebuilt: 59.92 fps, 0 underruns; the user confirmed
smooth playback. The build preset and `run.py` now re-check the shard on every
build/launch (~0.2 s), and the session log says `ttk-fmv: native movie
decoder active` or warns. Details: documentation/59-fmv-shard-namespace.md.
Binary `3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`.

**D23A Done (user-accepted) - the Modernized stutter/slowness had a measured
cause and is fixed in the current build.** Isolated probe: Modernized ran
at 47.5–49.3 fps with continuous audio underruns (output fill 17–34 ms
against a 180 ms target) while Vanilla ran 60 fps clean. `ttk::identity()`
re-read all 106 guards (81 KB) word-by-word through the guest bus on every
call — 46 calls per frame, 108 µs each, 5 ms of every 16.7 ms frame (24 %
of wall time). It now memcmps the runtime RAM image once per host frame
(re-verifying immediately on any RAM-code generation change) and costs
28 µs per frame. Result: 59.94 fps, 0 underruns, fill 267 ms; jetpack flight
60.1 fps. Details: documentation/58-modernized-frame-budget.md.

**D08Q Done (user-accepted) - modern jetpack flight.** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." User: in the jetpack only
Space worked, WASD were "blocked", mouse dead. Original mode 10 had no lease;
WASD reached it only through the tank fallback relative to an unturnable body,
and its idle gravity landed Duke within seconds. New host layer
(`recomp/src/ttk/jetpack.inc`) over the unchanged original handler: mouse
orbit with the body facing the view (so the original body-relative thrust is
camera-relative), W/S/A/D → original Up/Down + L2/R2, Space climb (original),
release everything → hover (original hover lock at the current height), Ctrl
→ steady descent to a soft landing, J in the air → original cut-out fall with
the camera still live. Probes: headings within 4° of the camera, level flight
±25 units, 25–29 units/frame forward/back, Ctrl 489 units/30 frames then
landing, ground controls resume. Known: stop-on-release is immediate; after
Ctrl release Duke settles ~130–300 units more near a floor; fuel drains while
hovering (original rule). Details: documentation/57-jetpack-controls.md.

Binary `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

**D08P Done (user-accepted) — area-wide control loss was overlay identity;
mid-depth wade now runs at land pace.** The user's session log showed `ident=0` from the
flooded-corridor ledge onward. The LEVEL00 zone script writes a hit position
into the overlay's trailing 16-byte scratch vector (`0x800ccf1c`), which the
9668-byte overlay guard covered, so identity failed permanently the first time
that script fired. The guard is now the 9652-byte code/table body. Separately,
that water is depth 512: the original swaps every gait for wade clips 80/81
(Vanilla 8–14 units/frame, forward only). The wade handler `0x800539f8` is
now hooked (new generated entry) and its root retargeted to camera-relative
WASD at the land run band: 45–58 units/frame, mouse steering, strafe, Space
jump; 0 identity refusals across and past the ledge. Wading straight into a
wall idles as in the original; turn the view to resume. Details:
documentation/56-scripted-camera-controls.md (iteration 6).

Pending playtest jobs D07A, D07B, D08A, D08H and D10 distance were **Done** on
user acceptance of ongoing play. D08N scuba remains cancelled. D10 broader work
stays In progress. D08B is Done on 2026-09-29 after the user playtested all
of its recorded limits (see the job board work log).

Previous binary `79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`
(D08P acceptance).

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
