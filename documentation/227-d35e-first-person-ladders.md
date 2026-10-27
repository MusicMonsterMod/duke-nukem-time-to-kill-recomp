# D35E - First-person ladder lifecycle

2026-10-24. Done, user-accepted. User: "lock it in, close it out!!!".
Explicit implementation, documentation, commit and push closeout authorized.

## Behavior

LADDER VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers FIRST PERSON
(default) / THIRD PERSON. CONFIRM applies and saves it. Global Third Person
wins; Modern controls and Independent camera remain prerequisites. Profile
schema 44 adds `ladder_view`; `run.py --ladder-view first|legacy-third` saves
that choice. Vanilla retains its original camera and play.

Plain ladders use a steady eye 294 units above the native body root, without
walking bob or animation-driven neck sway. The native attached root remains
200 units from the ladder panel. Mouse look changes the view, not the body,
collision or climbing direction. The existing normal-camera obstruction solve
remains active. Hands keep their native climbing poses and world-depth
occlusion; they do not turn into camera-facing weapon overlays.

## Implementation

- Shared presentation policy adds Ladder. Attached 147/148/156, 185-190,
  207/210/211 require the authenticated plain-ladder object family. Shared
  pole, chain, wall and ledge clips retain their earlier policy.
- Ladder-exclusive 185/190 exits in mode 8, previous 3/8, can already own
  their landing surface. Ground entry 185 at 8/0 requires a plain ladder.
- An observed drop is 108 at 9/3 with its attachment already cleared. The
  preceding Ladder presentation carries that handoff until the usual fall or
  landing policy takes over. It does not grant movement/aim/fire permissions.
- Only first-person ladder presentation adds the ground-entry/drop camera
  eligibility. Existing action leases, identity, owner, death and script
  restrictions remain. Third-person camera eligibility remains unchanged.
- Ladder arms join the existing interaction near-clipping path. The native
  actor loop otherwise rejects a joint nearer than GTE H before its polygons
  can reach the clipper. A guarded 0x800b4d2c hook, only from 0x80034c8c for
  Duke's ladder arms under the normal camera, raises the saved loop depth to
  that threshold. It does not alter GTE H or the native joint matrices. The
  loop's camera is saved at sp+0x98; s3 is its render context, not the camera.
- The getter's 12 bytes have an owned-EXE SHA-256 guard; the caller is inside
  the existing guarded 0x800348d8 actor loop. The standard generator added
  the new hook to one generated shard. Generated C was not hand-edited.
- Effective action is already in the replay snapshot; arm eligibility is
  recalculated per joint. No new gameplay latch or saved movement state.

No ladder movement, mounting, enemy-at-top handling, collision, animation,
speed, input binding or shoulder policy was edited. No framework source or
submodule pin changed; existing pinned-framework modifications were preserved.

## Verification

Accepted executable SHA-256:
`f1a319d4581e1713f7cf2a78c81480a0aa1674916690856c06c4ca5cf9b55ebc`.

- Player build passes, including regenerated hook and movie/math shard checks.
- Native controls suite passes against owned EXE, clean LEVEL00 fixture,
  LEVEL01 and all 21 selectable overlay fixtures. Added checks cover all
  ladder clips, shared-family exclusion, ground entry, detached exit/drop,
  both action choices, global view, pause, load, replay, native hand matrices
  and the actor-scoped near-cull bridge. Existing ladder movement and enemy
  exit checks remain in that suite.
- Native input, aim and near-clipping suites pass.
- All 94 profile tests pass, including schema-43 migration with backup,
  retained preferences, menu-sidecar merge, validation, CLI and Vanilla.
- Private Xvfb/OpenGL route at the 120 FPS setting: ground mount, climb,
  look, save/load on a rung, pause/resume, top exit, top re-entry, bottom
  step-off, airborne catch, high-bottom drop, Space drop, sewer top mount
  and descent. All 1,115 sampled states retained full first-person blend;
  683 resolved to Ladder. This route preceded only the final arm-cull
  camera-pointer correction, which does not affect camera or movement.
- That route exercised the alley enemy-at-top exit once. Source and native
  tests also retain the original obstruction and character-exit rules.
- Final-build 60 FPS setting: native hands visible while looking up/down
  on the rung; global P toggles; menu cancel writes nothing; CONFIRM switches
  to Third Person live and back to First Person. Menu layout inspected.
- Final-build Third Person action comparison at 60 FPS setting covers the
  same lifecycle: 1,288 samples, 675 Ladder samples, none with effective
  first person. Exits ease back to global First Person. Enemy-at-top exit
  handling was exercised once here too.
- Final-build extra run at 120 FPS setting covers a jump without Grab (no
  attachment), standing catch 147, bottom hang 207, climb transfer 210/156,
  rung rest and mouse look. All 457 samples retained full first person.
  The final Space tap on the ordinary rung did not drop in this probe;
  the earlier high-bottom/207 Space-drop route is the drop evidence.

Initial probes exposed and corrected ground mount 185 at 8/0 and drop 108 at
9/3 camera gaps. An early unprotected alley run died after its top exit;
subsequent lifecycle routes used the existing god cheat on private saves.
That death is not evidence of a ladder camera regression or death support.

Private scripts, copied saves, profiles, logs and captures are under
`recomp/analysis/d35e/`. Sources were historical private D08J3, D08J1 and D08U
copies, not the player's writable cards. The test save on rung uses private
slot 0. No player's settings, saves or cards were changed. Original media
was read-only. All diagnostic sessions are stopped. Rate settings are not display-cadence benchmarks; dummy audio
is not an audible-sound test.

## Acceptance and coverage limits

The user accepted the candidate and authorized closeout. Their acceptance
does not enumerate additional playtest routes. Native tests cover 147/148/156/
207/210/211; sampled live routes do not prove every intermediate frame was
observed. Every ladder, costume and campaign geometry is not certified. A save
loaded directly into the attachment-cleared 108/9/3 transient has no previous
host Ladder provenance; ordinary fall/landing reacquires the selected view.
Poles, chains, climbing walls and ledge mantles remain separate D35 jobs.

```sh
python3 recomp/tools/local/run.py
```

Use P for global First Person if needed. LADDER VIEW defaults to First Person.
E mounts, W/S climb and Space drops as before. Try both action menu choices
and CONFIRM. Gameplay captures automatically; F10 is not required.


## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export was
unchanged; application to the clean pinned framework index reproduced all 51
modified/new files. No pins changed. No post-acceptance gameplay edits, builds
or launches; no player data touched. No next job started.
