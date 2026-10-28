# D36J4 - Door-kick fixed display FOV

2026-10-28. Done (user-accepted; closeout authorized). User selected the current slot-10 door kick and requested
this job and a full record. S source/tests, L private runtime, U user report.

## Intake

U: the door-opening leg has the long wide-FOV appearance previously corrected
for weapons and Q. Scope is this leg, not a wholesale body/contact-arm change.
S: `LimbPresentation::fixed_projection()` explicitly admits only Kick parts 1-4.
Door parts 5-8 and thigh attachment 21 already use camera-relative pose and
viewmodel depth, but retain world projection. D36J3 deliberately excluded them.
Baseline includes pending D19X/D19Y camera-slider work; preserve those changes.
Existing framework modifications are not part of this job. No commit/push.

## Work log

- Registered D36J4 In progress. Read accepted D36 audit and contact-arm records,
  current board/status/handoff, build workflow and D36J3 evidence.
- Identified the projection-policy omission; completed the bounded repair and
  verification below. User subsequently accepted the result; see closeout below.

## Implementation

S: `LimbPresentation::fixed_projection()` now selects every authenticated Kick
owner, alongside DisplayHand/DisplayArm. Both Q and the native-timed door leg
therefore use H=256 in the existing near-clip emitter. Door semantic parts 5-8
and thigh attachment 21 share the correction. No per-part positional workaround,
pose change, new allocation, replay field or setting is introduced.

The renderer changes the mesh-local projection only. Native GTE registers,
world/contact-arm projection, UV/depth, gameplay joints, door callbacks and
animation timing stay authoritative. Existing actor/perspective/Vanilla guards
still decide ownership. No framework or generated code edit is required.

## Reproduction log

L: `recomp/analysis/d36j4/` contains copied current player cards/settings,
reproduction scripts, state samples, continuous Xvfb recordings and PNGs.
User slot 10 is debug slot 9. The current save is facing a hinged door,
not the historical slot-10 vending-machine fixture in the D36 audit.
The first `baseline` attempt pressed E without approaching and never reached
116; excluded. The corrected `approach` run walks forward for two seconds,
then holds E. It observes native lower/upper 116, six channel keys 3574-3579,
first-person ownership, door opening and return to idle 63. It produces 134
116 samples across FOV 60/default/80/90, with zero host Q attacks/hits.
The wide-FOV strike exposes the long thigh/attachment appearance.

The initial baseline still filenames reused each channel key across FOVs;
only their last captures survive. The continuous recording and labelled state
trace retain the earlier runs. Candidate filenames include FOV to avoid that
loss. No claim of exact phase matching is made for asynchronous screenshots.

S: the expanded emitted-packet regression fails on the original header at its
screen-coordinate equality assertion when the door part sees another world H.
The first no-argument near-test invocation skipped owned-data packet tests;
only the corrected invocation with the owned executable is red-test evidence.

## Verification and limits

S: player build and native controls/input/aim/near suites pass. Controls use the
owned executable, LEVEL00 guard fixture, LEVEL01 and all level overlays. The
door actual-hook regression now checks fixed projection for each native key,
including thigh attachment 21, while retaining immutable native matrices,
no extra attacks and existing perspective/Vanilla guards. The packet test
checks parts 1-8/21 at H=160/177/191/209/256/277 and three aspect margins:
identical emitted screen positions/words with unchanged UV/depth and shared
GTE registers. World/contact geometry still responds to world projection.

L: `fixed60` (16:9) and `fixed120` (21:9) each load private current slot 10
and open the door at FOV 60/default/80/90. They retain 133 and 143 pose-116
samples respectively, first person throughout the action, zero host Q
attacks/hits and final idle 63. Inspected key sequences and strike/recovery
stills show the corrected framing, an opened door and restored weapon with
no lingering leg. Captures are sequential and animation phases differ between
runs; exact projection invariance is established by packet tests, not by
claiming pixel-identical runtime screenshots. Continuous recordings are retained
for review; this is not frame-by-frame certification of every animation instant.

At requested 120, replay completes 2067 sessions and 2193 worker jobs across
three workers, with zero session/job/plugin failures or worker deaths. This is
replay correctness evidence, not measured sustained 120 FPS. All 41 player
save/card/profile files preserve hashes and modification times. Diagnostic
processes exited. Repository privacy/content and diff-whitespace checks pass.

The initial no-approach run, no-argument near test and baseline overwritten
per-key stills are excluded/qualified above. Dummy audio supplies no listening
review. Broader costume/campaign doors, look extremes, interruptions, mid-action
save/load, software rendering and rates above 120 are not newly runtime-tested.
Existing native guard/gameplay tests pass; no broad body or contact-arm projection
change is claimed or implemented. D19X/D19Y changes remain pending independently.

Candidate executable SHA-256:
`58dabb763f7cad9e150b4012c544c2fdd8c008a033c77315070836e0687403b9`.
No framework source edits. User subsequently accepted the result; see closeout below.

## Playtest

Launch, load slot 10, walk up to the door and press E. Check the opening leg
with First-person FOV at 60 and 90: its size/framing should stay stable like Q
and the weapon. The native door animation and opening event remain unchanged.

```sh
python3 recomp/tools/local/run.py
```

## Acceptance and closeout - 2026-10-28

User: "lock it in, close it out". D36J4 is Done for the delivered door-kick
fixed projection, including the thigh attachment. Implementation, regressions,
manual and this evidence record are included in the authorized MusicMonsterMod
commit/push. Existing verification and coverage limits stand; acceptance does
not certify unrun combinations. No post-acceptance gameplay edit, rebuild or
launch. Pending D19X/D19Y camera-slider work is preserved outside this commit.
The existing framework delta is exported and checked against the pinned clean
source; repository privacy/content and diff-whitespace checks pass.
No next job selected.
