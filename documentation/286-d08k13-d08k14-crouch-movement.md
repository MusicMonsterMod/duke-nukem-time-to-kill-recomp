# D08K13/D08K14 - Crouch movement ownership and run animation

2026-10-27. D08K13 and D08K14 Done, accepted and closed by the user.
Canonical scope: [job board](../MODERNIZATION_JOBS.md). The user explicitly
requested lock-in, closeout, an updated HTML artifact and extensive documentation.
Revision 2 is the accepted baseline. Preserve its travel speeds and gait poses;
do not replace them with the rejected first run candidate. Commit/push of the
implementation, tests and documentation is authorized as MusicMonsterMod.

This record retains the chronological investigation, rejected candidate and
revision decisions. Earlier pending statements are historical. Final automated,
live and user-acceptance evidence appears below; natural vent traversal remains
unverified and acceptance does not turn that limit into a passing test.

## Baseline and evidence

- User report: backwards crouch running can enter a retail roll after 3-5
  seconds in both views. Not yet reproduced in this investigation.
- Source: D08K12 changes pace through `modern_pace_step(input_running(false))`;
  walk poses advance with accepted travel. There is no separate run gait.
- Source: `pc_input.cpp` suppresses directional D-pad bits while
  `crouch_walk_input_ready()` reports ownership. `crouch_walk.inc` requires
  crouch animation 176/178 and supported ground; trace the loss of ownership
  and native input before proposing the fix. This is a lead, not a proven cause.
- Prior D08K12 measurements used 0.4 seconds of explicit sleeps plus tool overhead per trial;
  they do not establish the newly reported sustained-input behavior.
- Root implementation is clean at intake. Framework has pre-existing local
  modifications at pinned revision `08ec704a974b1f3a16335b4afeb340b9eff19926`;
  preserve them. UI pin `be8ac1d03ee19d55394b5a5f2d9d1506edd56659`.

## Reference study - preliminary

Both supplied MGS videos are present under `research/screencaps/`. Full-duration
sparse contact sheets were extracted and inspected under local
`recomp/analysis/d08k13/`. These establish the broad silhouette only, not foot
contact timing. Detailed temporal study remains required before authoring.
The run shows a longer rear-leg extension and stronger alternating arm action;
the walk keeps a deeper compact stance with small steps. Both incline the torso
and keep the head directed ahead. Duke's accepted walk remains the baseline.

## Verification ledger

Pending: live baseline reproduction, ownership trace, fix regression, both views,
both crouch controls, sustained backwards travel, repeated direction/run changes,
clearance/collision/ledge regressions. D08K14 additionally requires the HTML
preview, in-game animation review at real speed, vent traversal and limitations.
No new build or gameplay test has run yet. Player settings/cards are untouched.


## D08K13 investigation and candidate

Private real-SDL baseline (60 presentation, third person, copied slot 5) catches
178 -> 179 -> 181 with C-toggle and 178 -> 181 with Ctrl held. Held S/Shift
remains active. The route reaches a wall before the transition; it establishes
sustained-input failure, not uninterrupted travel for the entire sample.
Evidence: `recomp/analysis/d08k13/before-third.json` and `baseline.py`.

Native resident routine 0x80053038 checks gamepad activity and refreshes the
player+0x240 timer to 3000. Neutral input lets it count down; expiry with crouch
178 selects low fidget 179 at 0x8005323c. Modern crouch suppresses guest
movement input, so its activity is invisible to that routine. Fidget 179 fails
our 176/178 eligibility, drops the direction suppression, and held movement
then reaches the legacy forward-roll selector. Backwards host movement can
therefore produce roll 181, since the fallback injects guest Forward.

Candidate refreshes the native timeout only inside the eligible Modern crouch
178 owner, including blocked movement and blocked standing. Existing collision,
clearance, native fall handoff and pose stepping remain intact. The resident
routine is added to the authenticated code guards. No generated C or framework
edits. Player build and native controls/input suites pass. A sustained fixture
covers direction and pace changes and blocked movement; actual retail timer
behavior is being checked in live tests.

A first-person long trial stayed at 178 until enemy damage/death animation 218
near its end. This is not counted as a full passing trial. Shorter repeated
trials reload the private scene before that unrelated interruption.


## D08K14 reference study and implementation

Inspected both complete-video contact sheets and dense consecutive frame
sequences: run at 2s and 8s, walk at 2s and 18s. The run has a longer trailing
leg, quicker foot recovery and stronger alternating arms; the walk stays more
compact with a deeper bend. Dense sequences clarify foot recovery better than
sparse stills, but are not motion capture or a measured retargeting of MGS.
The original MGS assets are neither imported nor redistributed.

Separate extra clips 21-24 carry 32 run poses plus sentinel, with matching upper
clips 31-34. Travel is 1000 units per forward/backward cycle (walk 640), and 520
per strafe cycle (walk 380). Stance occupies 54 percent of each foot's cycle;
stance travel matches body travel, recovery lifts 65 units, the boots stay flat
while planted and pitch modestly during recovery. Chest lean is 66 degrees,
arm counter-swing 30 units, while the accepted hip height remains unchanged.
Duke's skeleton, the existing solver, pose packing, upper-channel separation
and weapon ownership remain authoritative. No mesh/material revision.

Runtime selects run clips using the configured speed policy and falls back to
accepted walk clips with old packs. Phase continues through pace changes; the
existing six-update blend starts from the actually displayed pose and height,
including interrupted transitions. Movement still advances phase by accepted
travel, so blocked movement stops the gait. No movement-speed change.

`preview_crouch_run.py` regenerates the local textured HTML artifact with every
direction, side/rear views, pause, scrubbing, slow playback and matched-cycle
comparison. It reads owned media, and its retail-derived images remain ignored.
The artifact is a 16-pose flipbook, not an in-game recording or a preview of
runtime transition blending. Its 1x cadence assumes 30 player updates/second;
actual simulation timing can vary. JavaScript syntax and all asset paths and
control handlers pass a Node harness. Browser UI verification was unavailable
(no browser connected); rendered pose sheets were inspected directly.

## Completed verification and remaining limits

- Player builds pass; native controls and input suites pass. Controls retain
  low-ceiling collision, blocked standing, walk/run ledge handoff and Original/
  Vanilla exclusions. Added sustained countdown regression and run/walk frame
  selection assertions exercise the actual Modern owner with fixture geometry.
- All 16 animation tests pass, including all-direction planted-ankle world travel
  under 3 units per adjacent stance sample, packing and headroom. Highest sampled
  run vertex is about 569 units, below the 704-unit crouch hull. Tests do not
  establish every interpolated mesh pose, obstacle or weapon combination.
- Every accepted walk and idle host clip, both upper/lower, compares equal to
  HEAD's generator. The normal-speed walk asset is preserved exactly.
- Private real-SDL/Xvfb/OpenGL runs at 60 third-person/default and 120 first-person/
  rebound controls pass both momentary and toggle crouch. Each sustained backwards
  trial includes 180 samples with at least nine seconds of explicit wait, plus
  tool overhead; all sampled animation IDs remain 178. Routes reach a wall, so
  late samples test held input against collision, not continuous displacement.
- Each view/control also repeats twelve forward/backward changes while run is
  pressed/released; all samples remain 178. The combined candidate records run
  frames 5497-5528 and switches into accepted walk ranges on release. Final
  capture scripts, JSON and MP4 files are under `recomp/analysis/d08k14/`;
  isolated bug-fix runs are under `recomp/analysis/d08k13/`.
- Inspected third-person overview and consecutive motion frames, plus first-person
  captures. No roll or standing pop is seen in those samples. Foot travel tests
  support flat-ground contact, but sampled images are not proof of perceptually
  perfect motion. Human review at the actual pace remains required.

Natural vent traversal is NOT verified. The earlier hull study (note 185) found
no crouch-only passage in its level scans; existing air vents were reached by
jump/grab. A specific route/save location has been requested. This does not
replace the requested vent playtest. Live crouched ledge departure is also not
newly established; native collision-contract regressions pass.

Design review: preserving the accepted running speed still gives a brisk gait,
especially sideways. The longer stride reduces cadence relative to accelerating
the walk, but Duke's short, rigid torso and boots constrain a literal MGS match.
Review lateral foot crossing, rapid reversals and armed firing while running;
broad terrain/weapon/clipping coverage is not established by this street route.
No change to speed or the accepted walk is proposed without user review.

No closeout, commit or push has occurred. Player settings, saves and cards are
untouched; tests use copied lab data. Framework local changes predate this work
and were not edited. Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Use Modernized and CROUCH: MODERN; F10 captures gameplay input if needed.

## Revision 2 - reference timing and quality rejection

2026-10-27. User rejects revision 1: run still looks sped up, both gaits have
unnatural arms and the hands read as "sneaky". D08K14 is In progress again;
user explicitly opens the walk gait and hand posture for improvement.
D08K13's idle-timeout fix stays separate and intact.

Revisited full-resolution clips, consecutive sequences, front/side/rear views,
starts, stops and turns. Measured the steady run at 0-2s and the walk at 3-8s,
excluding later held poses and stops. A simple image-space colour-region track
of the near arm, detrended for camera drift, has repeating peaks at 43 frames
for run and 105 frames for walk (60 fps source): approximately 0.72s and 1.75s
per complete left/right cycle. Correlations are about 0.79 and 0.98. These are
screen-space timing estimates, not recovered bone transforms or an exact rig.
Local script/data and annotated frame sequences are under
`recomp/analysis/d08k14/revision2/`.

At the existing nominal 30 player updates/second, revision 1 instead gives a
0.34s run cycle (1000 / 2928.5 units/s), and a 1.03s walk cycle (640 / 624.5).
The run is about twice the reference cadence despite its longer stride.
Matching a 0.72s cycle at that speed would require roughly 2100 units per
cycle; the low stance cannot plant feet over that travel with Duke's leg lengths
without sliding, overextension or a much larger airborne phase. Speed and
cadence must be designed together. A user preference on crouch-run pace has
been requested; no new pace change is assumed from elapsed time.

Pose findings: MGS walk hands hang near the knees, elbows flex loosely and
wrists continue the forearm rather than staying locked horizontally. The run
bends the elbows more, alternates the forearms, lifts the rear heel and recovers
the foot under the pelvis. The head is comparatively quiet; the pelvis transfers
weight and compresses at contact. Revision 1 used a new leg path on top of
unrelated walk pelvis timing and retained world-pinned, high/inward fists. Its
constant flat stance foot also lacked the contact-to-push-off articulation.
These are the revision targets, beyond changing a playback multiplier.


### Revision 2 speed decision and implementation

User explicitly chooses to keep current travel speed and show the best achievable
animation, allowing a slightly cartoony result while aiming for realism. Both
walk and run travel remain unchanged. This supersedes the pending speed question
above. User has also authorized revising the walk and hand posture.

Rebuilt both moving gaits with 48 poses per cycle. Walk hands hang beside the
knees, run elbows bend more, and wrists follow the forearms instead of staying
pinned horizontally. Arm swing opposes the same-side leg. Pelvis compression,
shoulder counter-rotation and foot recovery share the contact phase. Run uses
1600 units/cycle forward/backward and 1350 laterally, 40 percent support per foot
and brief flight intervals; the previous candidate used 1000/520. Forward run
cycle is about 0.55s at nominal 30 player updates/s, still faster than the 0.72s
reference. Walk keeps its 640/380 cycle distances. The captured walk authoring
function remains available for comparison, but the new candidate revises its
moving poses as requested. Idle kneel and gameplay travel speeds are unchanged.

Boot clearance now constrains actual sole vertices, including the long toe tip,
rather than only the ankle or toe bone. During toe-off, the floor-contact corner
can change vertically; the regression checks horizontal toe sliding separately
from full-sole floor clearance. This caught an 18-unit tip penetration before
the correction. The retail hands are rigid closed meshes: wrist and arm posture
can improve, but opening individual fingers needs separate mesh/rig work.

### Revision 2 verification and review artifact

- Build passed. All 16 animation tests passed in 91.5s. Authored poses in all
  eight moving clips stay below the 704-unit crouch envelope; walk maximum is
  approximately 529 units and run approximately 585. Tests check full boot
  vertices against the floor and planted ankle/toe travel. These are authored
  pose checks, not a proof of every interpolated pose on uneven terrain.
- Native controls, input and aim suites passed, including crouch collision,
  low-clearance release, ledge ownership and the 1200-update timeout regression.
  Travel assertions still require 20.8154296875 walk and 97.6171875 run per
  accepted update. No player pace change was made.
- Private real SDL/Xvfb tests passed at 60 presentation setting in third person
  with Ctrl/C/Shift, and 120 setting in first person with rebound Z/G/T. Each
  view used both held and toggled crouch, 180 sustained backwards-run samples
  per control (at least nine wall-clock seconds), then 12 forward/backward and
  run-state switches per control. All 1200 combined samples reported 178.
  Test-route damage is disabled with god mode after each private baseline load;
  an earlier attempt was interrupted by enemy death, not a crouch state escape.
- Third-person free-look side inspection exercised forward/backward and lateral
  walk/run at actual game speed. Dense frame strips and captures show the low
  poses and transitions without a legacy roll or a standing-pose snap. Tree
  transparency and bench occlusion limit some foot observations. Longer run
  reaches are pronounced, particularly backwards; their visual acceptability
  remains for user review. Broad terrain, armed firing and mesh-intersection
  coverage are not established by this route.
- Natural vent traversal remains unverified. The existing low-clearance and
  ledge fixtures pass, and the authored silhouette remains inside the crouch
  envelope, but these do not substitute for a playable vent route.

Review `recomp/analysis/d08k14/revision2/preview/index.html`. It contains all four
travel directions, three views, 48 poses, pause/scrub, slower playback, matched
cycle timing and both original local MGS videos. Side-view ground marks track
forward/backward travel; they are omitted for lateral travel viewed side-on.
The preview is a textured flipbook, not a collision or transition simulation.
A headless Chrome screenshot verifies its layout and rendered poses; the JS
control harness exercises all 12 direction/view combinations, playback rates,
matched timing and scrub/pause/resume. Local gameplay recordings, JSON samples,
reference sheets and logs are in the parent revision2 directory.

D08K13 and D08K14 remain Needs playtest. This candidate is available for review;
no acceptance, documentation closeout, commit or push is implied.


## Acceptance and maintenance contract - 2026-10-27

User: "lock it in and close it out"; "this work is perfect". The user accepts
revision 2 and explicitly requests the HTML artifact update and extensive
recording. Both jobs close as Done on that acceptance, with the evidence limits
above preserved. No further gait, hand, pace or collision adjustment belongs
to this closeout. A new revision requires a newly selected task.

### Accepted implementation map

| File | Responsibility |
| --- | --- |
| `recomp/src/ttk/crouch_walk.inc` | Refresh eligible Modern crouch idle timeout; select walk/run clips; accepted-distance phase; six-update displayed-pose blending; preserve collision owner and weapon upper-body ownership. |
| `recomp/src/ttk/control_guards.inc` | Authenticate the resident crouch idle routine used by the timeout fix. |
| `recomp/tools/local/ttk_crouch_walk.py` | Author 48-pose reference-informed gaits on the retail skeleton, solve limbs, constrain boot soles, pack separate lower/upper clips. Retain `captured_frames` for source-motion comparison. |
| `recomp/tools/local/preview_crouch_run.py` | Reproducible local textured HTML study; acceptance record, reference videos and engineering-note link. |
| `recomp/tests/local/modern_controls_native.cpp` | Separate clip selection, unchanged travel pace, sustained timeout ownership, collision and clearance fixtures. |
| `recomp/tests/local/test_ttk_anim.py` | Encoding, clip layout, original captured-source preservation, pose envelope, full-sole clearance, planted-foot travel and idle-kneel preservation. |

### Accepted gait parameters

| Property | Walk | Run |
| --- | --- | --- |
| Forward/backward cycle travel | 640 units | 1600 units |
| Lateral cycle travel | 380 units | 1350 units |
| Nominal pace at 30 player updates/s | 624.46 units/s | 2928.52 units/s |
| Forward/backward cycle duration | 1.025s | 0.546s |
| Lateral cycle duration | 0.609s | 0.461s |
| Planted fraction per foot | 0.66 | 0.40 |
| Recovery lift | 32 units | 100 units |
| Poses per cycle | 48 | 48 |
| Lower/upper pack clips | 1-4 / 11-14 | 21-24 / 31-34 |

Stationary clips 5/15 preserve the original kneel. The run modifier selects
separate clips while retaining phase; six-update blending starts from the last
displayed pose, including interrupted changes. Actual accepted displacement
advances phase, so a blocked path does not continue cycling feet. The timer
refresh is scoped to eligible Modern crouch, not a global disable of native idle
behavior. Original crouch and Vanilla retain their existing ownership paths.

### Reproduce and maintain the preview

From the workspace root, with the owned disc prepared and animation dependencies
installed:

```sh
python3 recomp/tools/local/preview_crouch_run.py recomp/analysis/d08k14/revision2/preview
```

For page-only acceptance/documentation updates, reusing the existing 24 sheets:

```sh
python3 recomp/tools/local/preview_crouch_run.py recomp/analysis/d08k14/revision2/preview --html-only
```

Open `recomp/analysis/d08k14/revision2/preview/index.html`. The accepted page has
24 sheets (four directions, two paces, three views), each containing 48 textured
poses, plus the two supplied MGS videos. Reference files remain local under
`research/screencaps/`. The page links back to this engineering record. Generated
HTML, textures, recordings, private cards and media remain ignored; the tracked
generator preserves reproducibility without publishing retail-derived assets.

Closeout checks refresh the HTML without changing any accepted pose sheet,
verify links and sheet dimensions, run the interaction harness and browser
render, check source diffs and run repository hygiene before committing.
The gameplay build and 16 animation/native test results above apply unchanged:
closeout changes presentation and documentation only. No new game launch or
player-setting/save mutation is needed. Framework modifications predate this
job; its pinned pointer and accepted patch are unchanged by this work.
