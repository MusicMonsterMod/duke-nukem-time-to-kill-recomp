# D36J1 - Quick-kick realism and refinement investigation

## Acceptance and authorized closeout - 2026-10-28

D36J1 is Done for the bounded no-body prototype and its evidence. User playtested,
reported that it fixes most problems and looks great, and then explicitly asked
to register the three follow-ups and commit/push work so far. No-body is the
accepted direction; enabling it by default remains D36J2, not implemented here.

The original broad research scope is split: D36J2 integrates the default and
checks action/restoration coverage; D36H5 investigates arms/weapons with exposed
or disembodied shoulders as the user's principal concern; D36J3 reviews kick
motion/framing/weapon overlap. All are Todo. Unrun pose experiments and remaining
coverage are transferred, not silently marked complete. No shoulder solution is
assumed, and no arm redesign starts during closeout.

Prior build/native/runtime results stand with their recorded limits below.
Authored implementation, tests, manual, backlog and artifact are authorized for
commit/push as MusicMonsterMod. No new build, game launch or player-file write
is needed. Existing framework changes are preserved. Historical candidate
wording below describes the review state before acceptance.

## No-body prototype - Needs playtest

2026-10-28. User selected a no-body prototype and suggested the apparent defect
may largely be visibility exposed by wider FOV. This selection supersedes the
initial research-only scope below. It does not accept a permanent body policy.

The opt-in `DNTTK_FP_BODY=hidden` suppresses ordinary primary first-person body
parts 0-9/20/21 at the existing guarded draw boundary. The camera-relative Q and
door-kick owners run first, so their geometry remains. Arms and equipment retain
their accepted owners; the D14E1 unsafe-eye arm guard remains. Head hiding,
reflections, third person and Vanilla retain their existing paths. No pose,
mesh, camera, FOV, timing, damage, sound, gameplay matrix or save-format change.
No added allocation or replay latch. Omitted/other values retain the accepted
body baseline, enabling an A/B comparison without changing saved preferences.

### Observations and recommendation

L: `body` and `nobody` each capture 60/80/90 at level/down/up with idle and Q,
using private current slot 4 (debug index 3), Xvfb/OpenGL, 16:9 and a 60 setting.
Kick display is frozen at phase 13 for controlled pose comparison. Downward
samples have matching recorded pitch 1.047198, yaw 2.513462 and camera position
across both runs. These are sequential measurements, not atomic draw snapshots.
`comparison.png` shows little visible torso at 60 and substantially more vest/
belt beneath the kick at 80/90. No-body removes that conflicting attachment cue
without moving the leg. This supports the visibility hypothesis for this route;
it does not prove the cause of every reported airborne frame.

L: `motion120` uses unfrozen Q at FOV 90, 16:9 and a 120 setting. Captured native
poses include 63/72/76/78/178, jump preparation 96, airborne 97 with changing
vertical position, and landing 105. Q is active through sampled jump poses and
inactive on recovery. `jump-sheet.png` retains samples 07/08/10/12; the full
private recording is `motion120.mp4`. Reviewed stills cover sampled motion, not
every video frame. Physical P toggles effective first -> third -> first; private
save/load and Escape pause/resume retain safe recovery. Copied slots 2/6/7/9/12
provide additional scene samples, not full interaction/costume certification.

L: `aspect43fixed` (launcher widescreen `off`) and `aspect219` sample downward
idle/Q at 60/90, at 60 and 120 settings respectively. `equipment` confirms
pistol (4), weapon record 8, dynamite (12), and empty hands (equipment 0 after
Boot selection), including movement, Q and recovery. The first two attempted Z
holsters remained armed and are excluded from holster coverage. C in the first
motion route was crouch, not holster. Actual state wins over intended labels.
The empty-handed movement image retains the native arm chain and ground shadow.

Recommendation: playtest this visibility-only candidate before selecting a new
kick angle, narrower kick projection, dynamic hip solver or arm redesign. The
kick still follows world FOV and can show substantial thigh during chambering;
this prototype intentionally exposes whether that remains objectionable without
a visible torso. Existing black ground-edge triangles appear in the matched
body baseline too and are not claimed fixed here.

### Verification and limits

S: player build and controls/input/aim/near suites pass. Extended owned-data
controls tests cover hidden/visible/invalid overrides across two costume orders,
FOV inputs and armed/unarmed body guards; original native matrices remain equal.
Active Q output matrix is byte-identical with the prototype enabled, while the
ordinary opposite leg is suppressed. Door-kick output is unchanged. Existing
hit-count/arguments, timing, ownership and restoration checks pass.

L: `motion120` reports 1202 replay sessions and 1388 completed worker jobs with
zero failed sessions/jobs/plugin failures. Packet peak is 139452/139744 bytes,
zero skips/copy overflows; this is limited headroom, not a performance claim.
`aspect219` reports 447 sessions and 538 completed jobs, zero failures. Requested
120 is not sustained measured 120 FPS; adaptive pacing remains active. Dummy
audio does not establish audible sound quality. All completed harnesses preserve
SHA-256 and mtime for 41 player save/card/settings files; writes use private
copies. Diagnostic processes are stopped after the runs.

The first 4:3 attempt supplied unsupported `--widescreen 4:3`; the launcher
rejected it before gameplay. The corrected `off` run supplies the evidence.
No framework edit, retail asset change, generated C edit, commit or push.

Not newly certified: all weapons/costumes, every interaction/swim/flight route,
live mirror/door action, software renderer, all high-refresh modes, full-motion
visual acceptance or sound. These remain explicit broader research/regression
items. No-body is not a new permanent default and D36J1 research is not closed.

Candidate SHA-256:
`b893287dbaddc1a7fb4442d37f2d0f233256fcbc4a9a8334135b9b7c0b54c1f4`.

Prototype, from workspace root (one launch; no saved body preference):

```sh
DNTTK_FP_BODY=hidden python3 recomp/tools/local/run.py
```

Use console `fov 60`, `fov 80`, `fov 90`; compare downward Q and jump + Q.
Normal accepted body baseline:

```sh
python3 recomp/tools/local/run.py
```

## Initial setup record

2026-10-28. In progress: job and evidence artifact stood up. This delivery is
research setup and initial source review, not a completed deep investigation or
playable fix. No build, game launch, gameplay edit, commit or push.

[Open the standalone investigation artifact](artifacts/quick-kick-realism.html).
It opens directly from disk without a server. Its historical screenshot uses a
relative local path and remains untracked; the text remains usable without it.
The canonical scope and acceptance criteria are D36J1 in the job board.

## Evidence and starting point

Preserve [D36's evidence labels and ownership contract](257-d36-first-person-presentation-audit.md):
S source inspection, L private runtime evidence, A prior accepted behavior,
U/H user report or hypothesis. No new L evidence exists for D36J1 yet.

U: FOV 80/90 downward Q looks shoulder-originated; jump + Q exposes an above-belt
origin. H: the camera-relative kick and world-body pelvis may separate under
pitch and airborne eye/body motion. This needs matched pose/matrix/capture proof.

S: `recomp/src/ttk/kick.inc` has default `kick_anchor` (-170,80,160), `kick_tilt`
(0,12,0), phase interpolation in `leg_pose`, and camera-derived output transforms
in `kick_leg_joint`. The overrides `DNTTK_FP_KICK` and `DNTTK_FP_KICK_TILT` are
cached on first use; controlled override trials need separate private processes.
A fixed offset can explore framing but does not establish anatomical attachment.

A: [D36J revision 2](261-d36j-body-and-kick.md) uses actual left parts 1-4 for Q;
right leg/accessory stay native. Door 116 owns its separate right-leg route.
[D14E](301-d14e-first-person-body.md) adds a world-depth pelvis/torso with a
clearance-based torso pose. [D14E1](302-d14e1-corner-body-guard.md) rejects unsafe
external body views, preserving kick ownership. Do not remove that guard to
make an attachment appear continuous. Note 257's original right-kick proposal is
superseded by accepted revision 2, not the current leg-choice specification.

## Research execution contract

The artifact defines E0-E5 and the source bibliography. Capture baseline first,
then change one factor at a time. Compare identical scene, costume, equipment,
pitch, aspect, FOV and kick phase; review continuous motion as well as stills.
Record actual native state, not only requested keys or screenshot filenames.
Any implementation recommendation remains provisional until tested in game.
Read note 265 before any new mesh/material revision. Keep Modernized default
and Vanilla unchanged; use private settings/cards/saves and never generated C.

Local evidence destination: `recomp/analysis/d36j1/`. Each retained case needs:
case ID, executable/source hashes and dirty diff, private slot hash and actual
level/costume, renderer, actual FOV/projection/aspect, presentation setting,
equipment/action IDs, pitch/yaw, camera/pelvis/hip transforms, kick phase,
input/override recipe, capture/trace paths, result and evidence label. Record
measurement synchronization limits, rejected trials and failed harness attempts.
Screenshot labels alone cannot prove airborne state or performance.

Research completion means a reasoned solution proposal with evidence and open
risks; visual implementation acceptance and authorized closeout are separate.
No production fix, hidden-limb policy, new FOV default/range, or camera restriction
is selected by standing up this artifact.

## Intake verification

Root worktree was clean apart from existing modified framework submodule.
Framework pin `08ec704a974b1f3a16335b4afeb340b9eff19926`; UI pin
`be8ac1d03ee19d55394b5a5f2d9d1506edd56659`. Existing framework changes preserved.
Historical screenshot visually inspected: `recomp/analysis/d14e/pelvisfixed-kick-80.png`.
It is contextual D14E evidence, not a newly reproduced jump or matched D36J1 baseline.
Initial public sources were checked on 2026-10-28; GDC session abstracts were
reviewed, not their complete talks. No reference game was run.

Launch unchanged player build from workspace root:

```sh
python3 recomp/tools/local/run.py
```
