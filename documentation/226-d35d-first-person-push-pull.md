# D35D - First-person push, pull and grab

2026-10-24. Done, user-accepted. User: "lock it in, and close it out!!".
Explicit implementation, documentation, commit and push closeout authorized.

## Behavior and implementation

PUSH/PULL VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers First Person
(default) / Third Person. CONFIRM applies and saves it. Global Third Person
wins. Releasing the object follows the current global view. Modern controls
and Independent camera remain prerequisites. Vanilla retains its original play.
Profile schema 43 migrates existing preferences and adds `push_view`;
`run.py --push-view first|legacy-third` saves it independently of movement.

The shared perspective resolver now admits the authenticated push owner:
119/120/121, mode/history 0/0, holstered, with a valid pushable object. Existing
camera ownership, code identity, action and movement gates remain. No movement
handler, lane policy, collision, support, speed, sound or animation is changed.

A push eye follows the body at the measured standing offset of 294 units above
its root, instead of following the grab animation's forward neck lean. Existing
root-anchor rebasing eases entry/exit; walking bob/glide is suppressed while
holding the object. The normal camera's obstruction solve remains active.

The primary Duke draw admits only authenticated arm/hand parts 0x0a-0x0f to
ordinary near clipping. Native matrices, poses and world depth remain intact;
these are not weapon-overlay hands pasted on top of scenery. Looking down at
the contact point shows the hands. Torso/head, other actors, reflections and
third-person draws retain their earlier policies. The resolved action is already
part of the render-replay snapshot. The per-joint draw token is recomputed from
that policy and bounded by the primary actor draw.

The first probe only removed the perspective exclusion. It retained the
animated neck lean and the old body near-clipping exclusion; close-up captures
made the need for a stable eye and arm clipping explicit. The candidate adds
both. No generated C or framework source was edited; pinned dependency changes
already present at intake remain untouched.

## Verification

Accepted executable SHA-256:
`09c5d8237e1f6da7da0c2651cf794ec27e1db12c466f88cebe7f9d558c0bca45`.

- Player build passes. Existing pc_input.cpp snprintf warnings remain.
- Native controls, input, aim and near-clipping suites pass. Controls include
  owned EXE, LEVEL00 fixture, LEVEL01 and all selectable level overlays.
- Added controls checks cover all three push poses, both choices, global view,
  paused input, load, replay, native arm matrices and exclusion of other joints.
  Movement remains unavailable to the camera-only push lease. Existing movement
  tests cover support, wall/ladder refusal, stop/reverse, release and Vanilla.
- Added near-clipping check admits close interaction arms while preserving the
  ordinary first-person body exclusion. Arms retain normal world-depth packets.
- All 93 profile tests pass, including schema-42 migration, retained global view
  and flight preference, menu-sidecar persistence, validation, CLI and Vanilla.
- Real keyboard/mouse, private Xvfb/OpenGL, 120 FPS setting: prior private UI
  slots 9-12 cover three LEVEL06 symbol blocks and the LEVEL00 alley dumpster.
  Held-W acquisition, neutral hold, reverse, sustained push to an obstruction,
  reverse from that obstruction, mouse pitch/yaw, blocked fire and release with
  weapon redraw pass. All 1,072 sampled states retained full first-person blend;
  991 resolved to push. This is sampled evidence, not a frame-perfect proof.
- The dumpster stopped at X 7292 and slot 9's symbol block at X 8700. Another
  block stopped against a nearby actor; dynamic obstacles can change a stop's
  coordinate. The unsupported-edge and ladder limits are unchanged code.
- 60 FPS setting: global P toggles while holding, pause/resume, private save
  while grabbing and load without Grab pass. Loading ends the grip safely.
  Real menu cancel writes nothing; CONFIRM switches the subsequent grip to
  Third Person; releasing returns to First Person; selecting First Person again
  restores the eye. Profile persistence is covered by the profile suite.
- Opposite-end dumpster regrab and push back from the ladder limit pass on
  the earlier private slot 7. The fixed-by-design LEVEL05 dumpster remains
  ungrabbable and unmoved under held Grab+W.
- Held W+E from the grip reaches native mantle mode 8, uses the existing
  traversal perspective fallback and returns to First Person after climbing.
  An earlier short E tap only released the grip and did not establish mantle
  coverage; the subsequent held-input route does.
- Captures of both object types show native hands when looking down during
  hold/push/pull. No new hand animation or artificial camera-facing pose.

Private diagnostics, profiles, captures and copied test saves live under
`recomp/analysis/d35d/`. Source fixtures came from earlier private D08T7 copies;
no player's saves, cards or settings were opened for writing. All diagnostic sessions are stopped. Original media
remains read-only. Dummy audio does not verify audible sound quality, and rate
settings do not establish display cadence or performance budgets.

## Acceptance and coverage limits

The user accepted the candidate and authorized closeout. Their acceptance does
not enumerate individual playtest routes. Hands remain
at their native contact point; looking away does not make them follow the view.
Full campaign, every costume, unusual geometry and damage interruptions are
not certified. Mantles retain their existing third-person policy under D35G.
This candidate does not finish any other perspective job.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Use P for global First Person if needed. PUSH/PULL VIEW defaults to First Person.
Hold RMB/Alt at the object, W/S to push/pull, release to let go. Try both menu
choices and CONFIRM. Gameplay captures automatically; no F10 step is required.


## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export was
unchanged; application to the clean pinned framework index reproduced all 51
modified/new files. No pins changed. No post-acceptance gameplay edits, builds
or launches; no player data touched. No next job started.
