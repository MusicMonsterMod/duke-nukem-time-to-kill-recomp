# D36K1 - Unarmed jetpack arm continuity

2026-10-26. Done, user accepted: "lock it in, close it out". Implementation,
tests, manual and documentation commit/push authorized. Earlier candidate and
pending-review wording below is historical. Evidence labels and verification
limits follow the accepted D36 audit.

## Investigation

**U:** flying backwards exposes disembodied hands without arms.
**S:** the D36H4 Body owner admits unarmed parts 10-15 only for Ground,
Committed and Fall. Jetpack has a separate resolved action and misses that
admission. The native joint loop rejects origins closer than GTE H before
polygon clipping. Armed display arms already have their own admission policy.

**L:** private copied slot 8 (LEVEL05), Boot selected through normal input,
jetpack enabled through normal input. Equipment 0, backward pose 166, forward
165, strafe 169/170 and hover 164 verified from RAM. At downward backward pitch,
the sampled left/right hand depths are 299/307, forearms 179/184 and upper arms
74, against H=256. This confirms mismatched whole-joint admission. These reads
are sequential, not atomic snapshots. Baseline captures do not clearly expose
the user's floating-hand silhouette at every sampled pitch; do not treat the
reported appearance as comprehensively reproduced from these sparse stills.
An initial slot-12 route remained ladder/ground and is excluded as flight evidence.

## Implementation

Extend the existing Body owner to unarmed semantic arm parts during Jetpack.
Retain native transforms, world clipping/occlusion, proximity-fade correction
and per-joint cleanup. Flight legs remain on their previous path. Ground-only
120-unit arm lowering remains ground-only; flight retains its native poses.
Armed/item display ownership is unchanged. No new allocation, host latch,
framework modification, generated C edit, asset change or flight physics change.
Primary actor, camera, matrix and effective-view guards remain intact.

## Verification

Player build and owned-data controls, aim and near-clipping suites pass.
Controls cover all eight flight animations, all three schemes and normal/cowboy
semantic layouts. New assertions check equipment 0/1/2, arm-only Body ownership,
unchanged native matrix pointers and bytes, world depth, joint-depth admission
and cleanup. Existing flight/view/Vanilla/load/replay and ground-arm tests pass.

Local scripts, copied cards/profiles, logs, traces and screenshots are under
`recomp/analysis/d36k1/`. Runs use offscreen OpenGL and dummy audio. Player
cards/saves are never opened for writes. Native tests cover more schemes and
costume layouts than the live route; this is not full campaign certification.

Private `candidate60` and `candidate120` routes verify hover, backward,
forward and strafe flight, Boot/pistol switching, actual global first/third/first
restoration and private save/load in flight. The immediate post-load sample is
hover 164, so it establishes flight restoration rather than backward pose 166.
The 120 run has three workers, zero worker failures and zero plugin failures;
this is bounded replay evidence, not a performance benchmark. `pitch120/pitch-7`
shows both hands continuing into forearms at steep downward pitch during
backward flight. Shallower samples keep the arms below the viewport. Human
motion/appearance review remains; all schemes/costumes, software, Vanilla and
lighting variants were not freshly swept in live play. Diagnostics are stopped.

Candidate SHA-256:
`1af5089a7d377ae6480dadfd496a97724797f743e538259ca29b3cd18c430427`.

## User playtest

Use first person with Boot/empty hands and jetpack on. Fly backwards, then
forward/strafe/hover and sweep pitch. Check wrist continuity and switch to a gun
and back. No preference change is required. Human motion/appearance acceptance
remains pending. No acceptance closeout, commit or push authorized.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```


## Acceptance and closeout - 2026-10-26

User accepted the candidate and explicitly requested closeout. D36K1 is Done.
Accepted executable:
`1af5089a7d377ae6480dadfd496a97724797f743e538259ca29b3cd18c430427`.
No implementation adjustment, rebuild or gameplay retest was needed for
closeout. Existing framework modifications predate this job and remain untouched.
Authored source, regression tests, manual and documentation are included in the
authorized MusicMonsterMod commit/push. No next job started.
