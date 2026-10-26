# D36H4 - Unarmed locomotion arm continuity

2026-10-26. Done, user accepted: "lock it in and close it out!".
The lowered revision is accepted; implementation, tests, manual and documentation
commit/push authorized. Earlier candidate and pending-acceptance wording below
is historical. Evidence labels and boundaries follow the accepted D36 audit
(note 257); coverage limits remain in force.

## Revision 2 - substantially lower unarmed arms

User requested a significant lowering after reviewing the first candidate.
Both complete arm chains now receive a 120-unit camera-down translation in
reserved draw scratch. Rotations, relative bone positions and native gameplay
matrices remain unchanged. World depth/clipping, original swing and empty-hand
locomotion eligibility are retained. Weapon/item and interaction poses retain
their existing framing. The existing scratch allocation and replay snapshots
are reused; there is no new allocation or serialized latch.

Player build and full owned-data controls suite pass. Tests now verify copied
rotations, equal translation for all six parts across both semantic layouts,
unchanged native matrices and held-equipment exclusion. Earlier assertions that
holstered hands retained their original matrix pointer were updated for the
intentional draw-only translation; unrelated-actor and outside-draw guards remain.

Private `lowered60` and `lowered120` captures show the swing substantially lower,
mostly near/below the bottom edge, with hands still visible during the higher
part of the swing. Crouch, Q active/return, pistol/Boot switching and private
save/load were sampled again. `lowered-view120` verifies actual first/third/first
restoration and pitch extremes. As before, C-key samples from the lifecycle
script are excluded as view-toggle evidence; the separate Z-bound route asserts
the effective view. Both 120 runs have zero worker/plugin failures. The earlier
coverage limitations remain; human approval of the new height is pending.

Revision 2 executable:
`e975163dad5028dc629fae8bf43031c0901086c7d64143f623a8345261ff60e3`.
Needs playtest. No acceptance closeout, commit or push. Earlier candidate details
below describe revision 1; the draw translation above supersedes its unshifted
presentation, while native gameplay transforms are still untouched.

## Research

**U:** slot 1, empty hands/Boot, running with slight downward pitch shows
floating hands. **S:** `weapon_held` excludes equipment 0, so connected display
arms do not own this state. The Ground/Committed/Fall native-body clip policy
admits parts 1-8/21 only. Arm parts 10-15 therefore retain whole-joint depth
rejection before polygon clipping, unlike accepted interaction arms. The native
world matrices already form the arm chains; moving hands to a gun grip would
replace the unarmed animation unnecessarily.

**L:** private copied slot 8 LEVEL05 run selects Boot through normal input:
equipment is 0, with stale record byte 4 (not an equipped pistol). Native run
poses 76/78 and walk 72/74 are observed. Forward movement eventually reaches
wall-bump 95; exclude those samples from unobstructed run evidence. Initial
small-pitch samples scarcely expose hands, so a larger pitch route is retained
separately. Source and capture evidence is local under `recomp/analysis/d36h4`.

Implementation direction: extend the existing world-depth Body owner to native
arm parts only when equipment is empty and the accepted locomotion policy is
active. Preserve primary actor/camera/matrix authentication, per-joint cleanup,
native matrices/events, quick kick, interaction and armed display ownership.
No new mesh, allocation, replay latch, animation timing or camera adjustment.

## Acceptance and regression plan

Connected visible wrists/forearms during idle, walk, run, strafe and start/stop;
pitch sweeps, crouch, Q, weapon switching and first/third restoration. Native
semantic-costume and draw-guard tests, build and controls/aim/near suites;
private 60/120 captures and save/load. Human motion acceptance remains required.

## Implementation and verification

The Ground/Committed/Fall Body owner now includes semantic parts 10-15 when
`player+0x3b8 == 0`. Record byte +0x3b9 is deliberately not used as proof of
empty hands; it retains the last weapon. Original matrices and animation remain
untouched. Armed/item paths keep their accepted owners, and interaction paths
retain their native lighting policy. Body proximity-fade correction and safe
world clipping are reused. Existing primary-draw, camera, actor, matrix and
perspective gates remain. No framework or generated-code change.

**L:** `before-down60/shift w-1.png` visibly contains a floating right glove.
Candidate downward run captures show connected forearms continuing to the edge
of the viewport. These are sampled routes, not phase-aligned before/after frames.
`candidate60` and `lifecycle120` capture native run 76/78, walking 72/74 and stop,
strafe, crouch 178, Q active frame 4 and inactive return, pistol equipment 2/4,
Boot equipment 0 and a private save/load. The 24 ordered sequence captures were
reviewed for arm continuity; offscreen continuation remains intentional. All
runs use copied cards/saves, private profiles, offscreen OpenGL and dummy audio.
No player card/save is opened for writes. Diagnostics have stopped.

The first `lifecycle120` C-key view attempt remained first person because the
private binding conflicted; it is excluded as perspective coverage. `view120`
uses the binding helper to assign Z and asserts effective first/third/first
states; captures confirm third-person Duke and first-person return, plus upward
and downward look. Both 120 runs report three workers, zero worker failures
and zero plugin failures. This is bounded replay evidence, not a performance
benchmark. RUN MODE, all costumes in play, full campaign, swim/flight, mirrors,
power-up lighting and software/Vanilla live runs were not freshly swept.

**S:** player build and controls/aim/near suites pass with the owned executable;
controls uses LEVEL00, LEVEL01 and the complete local selectable-overlay fixtures.
New tests exercise every arm segment across normal/cowboy semantic layouts,
equipment 0/1/2, native-matrix preservation, world depth and draw cleanup.
The existing transient-action test initially expected no arm admission during
empty-handed ground/idle states. Its expectation now distinguishes Body from
Interaction ownership; original interaction/Vanilla/third-person refusal checks
remain. Full controls suite passes after that expectation update. Existing
regressions cover view/load/identity guards, weapons, Q and native callbacks.

Candidate SHA-256:
`f400701023c3173a1581c213566406ff629b43f800d5ce381b036ed2a96ca937`.

## User playtest

Select first person and Boot/empty hands on key 1. Run while looking slightly
down; sweep pitch, strafe, stop, Q and switch to a gun and back. Judge native
arm swing and wrist continuity. No new setting is required. Needs playtest;
no acceptance closeout, commit or push.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```


## Acceptance and closeout - 2026-10-26

User accepted revision 2 and explicitly authorized closeout. D36H4 is Done.
Accepted executable:
`e975163dad5028dc629fae8bf43031c0901086c7d64143f623a8345261ff60e3`.
No source adjustment, rebuild or gameplay retest was needed during closeout.
Existing framework modifications predate this job and remain untouched.
No next job started.
