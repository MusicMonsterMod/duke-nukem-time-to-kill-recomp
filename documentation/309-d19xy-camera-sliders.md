# D19X / D19Y - Gameplay camera sliders

2026-10-28. Both jobs selected together. Candidate ready for user playtest;
no acceptance, commit or push is implied.

## Delivered behavior

- First-person FOV sits directly after View Bob. It displays requested horizontal
  degrees at 4:3, ranges from 60 to 90 in steps of one, and defaults to 64.
  The existing D14A setter owns projection and replay behavior. The menu retains
  the requested number instead of reconstructing it from quantized projection.
- Camera Distance uses a slider with Original plus 768-6144 game units, adjusted
  by 128 units. Held keyboard/gamepad adjustment accelerates through the range.
  Original remains serialized as zero and means the native follow boom, not a
  camera at zero distance. Existing custom values, including values off the
  step grid, survive opening and confirming the screen.
- Up/Down selects rows. Left/Right or controller D-pad changes values. Wheel up
  increases the selected camera slider; wheel down decreases it. Other menu
  rows retain wheel navigation. Pointer targeting, clicking and dragging remain
  D19Z work; this does not claim full mouse navigation.
- Confirm applies changes for subsequent gameplay and writes the existing menu
  sidecar. Cancel discards the draft. Defaults is also a draft until confirmed;
  it restores FOV 64 and Original distance along with the other Gameplay rows.
- Labels and numeric values sit above native slider bars, using three compact
  row positions to prevent clipping/overlap. The existing list scrolling keeps
  the remaining Gameplay rows accessible.
- Alt-wheel distance changes use the same existing camera owner. Reopening the
  menu reads its current preference, independent of obstruction compression.
- Profile schema 58 adds controls.first_person_fov. Missing, malformed,
  nonfinite or out-of-range FOV values recover to 64 without resetting unrelated
  valid preferences. Valid numeric values are rounded to whole slider degrees.
  The launcher supplies the preference before gameplay. Explicit DNTTK_FP_FOV
  remains a diagnostic launch override; Vanilla receives the original default.

Implementation: recomp/src/ttk/game_menu.cpp, recomp/tools/local/player_profiles.py
and recomp/tools/local/run.py. No framework edits, new projection hooks,
collision changes, animation changes or retail assets.

## Verification

- Player build passes. Native controls and input suites pass, including existing
  camera-distance/collision independence, scoped FOV, replay and Vanilla gates.
- The 107-test profile suite passes. An additional launcher regression passes
  for saved FOV, Vanilla exclusion, explicit developer override and preservation
  of the saved preference. Profile tests cover old-schema missing values,
  invalid FOV recovery, unrelated/custom-distance retention and sidecar/restart
  round trips at both ranges' endpoints and defaults.
- Private Xvfb runs at 60 and 120 presentation settings use copied cards and
  separate profiles. Real keyboard events, SDL diagnostic virtual gamepad
  events and mouse-wheel events adjust the new rows. Keyboard 64 -> 65,
  gamepad 65 -> 66 -> 65 and wheel 65 -> 68 pass. Gamepad distance adjustment
  preserves an initial 2345 and changes it to 2473.
- Confirmed FOV 68 produces H=237. Cancelled FOV edits and cancelled Defaults
  retain 68/2473. Third-person switching settles at H=386 while retaining the
  first-person target; returning to first person restores H=237.
- Alt-wheel increases distance to 2665 without changing the FOV target. A full
  process restart retains 68/2665. Confirmed Defaults restores H=256 and the
  native follow-distance preference; zero does not become a zero-radius boom.
- At the 120 setting, repeated wheel input clamps FOV at 60/90 and distance at
  Original/6144. Confirmed sidecars contain those exact values. Menu captures
  were inspected, including full labels, degree display and Original.
- All 41 player save/card/profile files retain their original hashes and mtimes.
  Test processes exited. Local evidence is in recomp/analysis/d19xy/;
  pass60/ retains the completed first pass.

## Limits and excluded attempts

An initial synthetic-menu route entered Display rather than Gameplay; those
attempts do not count as slider evidence. The natural keyboard route replaced
it. A first 120-setting comparison sampled the third-person transition before
its blend finished (H=313); the final test waits for blend completion. This is
not evidence of a projection regression. The final 120 run uses internal scale
1; the 60 run uses scale 4. Neither run certifies sustained frame rate or audio.

Live coverage is a bounded saved scene, menu routes and view/restoration checks,
not a new full campaign, scripted-camera or obstruction sweep. Native regressions
cover the unchanged owners' guards. Physical-controller feel and user visual
acceptance remain pending. The existing one-degree/integer-projection rounding
and brief replay-history delay remain as documented by D14A.

## Menu order follow-up

The user requested both sliders after View Bob. The Gameplay order is now View,
Shoulder, Crosshair, Gore, Status Bars, View Bob, First-person FOV, Camera
Distance. This changes row order only; slider behavior is retained. The player
build passes; private in-game captures verify the order and scrolling to both
sliders. All 41 player files remain unchanged.

## Player check

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Open Options > Gameplay. Adjust First-person FOV and Camera Distance, Confirm,
and resume. Check both views, Alt-wheel synchronization, restart persistence,
Cancel and Defaults. Both jobs remain Needs playtest until user review.


## 2026-10-28 - User acceptance

User confirms D19X and D19Y are complete and accepted. Both are Done.
This instruction marks acceptance only; documentation/commit/push closeout
was not requested. Earlier pending-review wording records the candidate stage.


## 2026-10-28 - Authorized closeout

Following acceptance of D19X/D19Y, the user accepted the D19U/D19U1 HUD work
and instructed "lock it in, close it out!" The shared implementation, tests,
manual and both engineering notes are included in this MusicMonsterMod
commit/push closeout. No additional gameplay changes or next job.
