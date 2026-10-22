# D08K10 - First-person crouched weapon alignment

Status: Done (user-accepted 2026-10-22). User: "perfect!! lock it in, close it
out. great work". Revision 2 accepted; documentation, commit and push authorized.
The diagnostic stages below preserve the first candidate's failure and its fix.

## Cause and change

The knife (1), axe (2), crossbow (3), pipe bomb (12), Holy Hand Grenade (13)
and dynamite (14) use D12's live hand-relative-to-chest display pose. Guns
4-11 instead use their existing fixed firing seeds. D08K6 compensates the
weapon-owned shoulders for the crouched pelvis and chest so that third-person
weapons keep pointing correctly. Reading that compensated hand relative to
the chest into the eye-view copy also imports the crouch lean into the
viewmodel. This is especially visible during crouch walking. The crossbow
also changes in the stationary kneel.

`first_person.inc` now rebuilds the animated viewmodel's hand from channel 1's
own interpolated shoulder, forearm and hand rotations and the current Duke
model's bone offsets. It uses the existing animation decoder and verified
part-to-joint mapping, composing the three joints relative to the chest.
Draw, charge, throw and reload frames remain animated; there are no new
weapon-specific offsets or pose seeds. Held items (the pipe-bomb detonator)
use the same path.

This runs only in Modernized first person with CROUCH MODERN during owned
crouch entry, kneel, walk, or the upper body's exit ramp. Standing keeps its
existing D12 path, framing and offsets. Slots 4-11 keep their existing seeds.
CROUCH ORIGINAL, Vanilla, third-person records, weapon simulation, aiming,
eye height, view bob and the kick are not changed. Missing channel joints or
an unexpected skeleton falls back to the existing display path. No framework
source, generated C, hook list, animation pack or save format changed.

## Initial candidate evidence (superseded for high-refresh walking)

Candidate executable: `fd7ed0ef639a96d79cfdb27b4cf18e2c72e3128a856575e125198f4418abced0`.

Private lab: `recomp/analysis/d08k10-lab/`, port 9410, offscreen OpenGL,
dummy audio, private profiles and a copy of the D08K9 test cards. The player's
saves and settings were not written.

- Baseline: the six weapons use the same arm-channel rest frames in standing,
  kneel and crouch walk, but the displayed live pose changes. For example,
  the crossbow's hand relative to the chest changes from roughly
  (79, -99, 95) standing to (84, -211, 105) crouch walking. Reading the raw
  arm chain gives roughly (77, -97, 93) in every stance. Remaining small
  numerical differences are the original fixed-point matrix rounding.
- At 60 Hz, captures of all six affected weapons plus pistol and shotgun
  show the standing framing carried into kneel and crouch walk. Numerical
  view rotations differ from the standing samples by at most 0.0042 per
  matrix component (the original fixed-point rounding); both seeded reference
  guns have identical rotations. Moving position samples are asynchronous
  RAM reads and must not be used as exact screen-space error measurements.
- At 120 Hz with view bob on, `validation.py` captures standing, entry,
  kneel, forward/side/back crouch walk, stop, exit, attacks and release for
  all six weapons plus pistol and shotgun. Throws/shots consume ammo and
  return to the carry animation; crossbow and shotgun reload. Eight crouched
  Q quick kicks start and recover. A grenade attempt overlapped the original
  blast hit-reaction (animation 159); a fresh stationary-kneel retry consumes
  one grenade normally. This was an interrupted input, not a viewmodel fault.
- `transition.py`: standing and moving jumps with Ctrl held reach the kneel
  or crouch walk in first person; release returns to standing. Captured
  landing sequences retain the crossbow framing through the transition.
- The crossbow produces no muzzle-flash hook in the tested firing/reload
  sequence, so the hypothesized first-shot pose capture did not occur.
- Player build and native input, aiming and controls suites pass. The new
  D08K10 controls group verifies all six slots and held items: changing the
  world hand matrix cannot change the crouched viewmodel; changing the arm
  channel does animate it; Original crouch and standing retain the live
  path; absent hand data falls back. The seeded slots retain their matrices.

The local scripts and captures are diagnostic evidence, not a full campaign
or all-costume certification. D08K5 remains the game-wide crouch coverage
job; D08K11 shallow-water crouching and D11E/D11F swimming/jetpack eye views
remain separate. This job does not enable those states.

## Player review

With CROUCH MODERN and first person (P), compare standing, held Ctrl,
Ctrl+W/A/S/D, and release for knife, axe, crossbow, pipe bomb, Holy Hand
Grenade and dynamite. Check throws/shots, switching and Q quick kick, and
hold Ctrl through a jump to land directly crouched. Judge the framing and
transition smoothness against standing. F10 captures the mouse if needed.

## Revision 2 - Publish crouch eligibility to replay workers

User: crouch idle is correct for every affected weapon, but crouch walking
still shows the undesirable poses. Reproduced in actual presented-window
captures at 120 Hz: `r2-before-walk-1.png` shows the crossbow rising across
the view even though the live viewmodel matrix is corrected.

Cause: the display helper read `crouch_owned`, `crouch_upper_weight` and
`crouch_fire_tilting`. Those simulation statics were not included in
`RenderState`; a high-refresh worker had their values from when it forked.
The live game could therefore draw the corrected pose while replay workers
continued drawing the old one. The first candidate's native test only
exercised the live path; its RAM measurements did not prove the worker's
presented pose. The earlier claim that 120 Hz walking was verified was too
broad and is superseded by this reproduction.

`crouch_weapon_active()` now resolves eligibility on the live frame and
`render_state_save()` publishes it. `render_state_load()` gives each replay
job that captured value, including false when standing or using Original
crouch. Workers still read the arm channels and model from that frame's RAM.
They do not depend on their stale simulation ownership or mutate it. This
changes title code only, not framework source or the interpolation schedule.

Candidate: `72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`.
Player build and native controls suite pass. The regression test forks an
isolated child, clears crouch ownership as if it predated the frame, then
loads active/inactive/active render snapshots. It checks the actual hand
draw matrix each time. Existing standing, Original, seeded-gun, animation
and missing-data checks also pass.

The identical 120 Hz crossbow route now keeps the low standing framing in
forward/back crouch walking and returns correctly through idle and exit
(`r2-after-*.png`). Three replay workers completed the route with zero
worker failures; the final cadence sample was entirely replay presentation.
The user's visual acceptance is still required.

`r2-all.py` also captures each of the six affected weapons at 120 Hz while
standing, kneeling, moving forward/sideways/backward, firing/releasing and
exiting crouch. The presented-image contact sheet (`shots/r2-contact.png`)
confirms the same framing in standing and both forward/back movement for
all six. This verifies the replay output itself, not only live RAM.

## Acceptance

The user accepted revision 2 and authorized closeout. Accepted regression
baseline: `72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f`.
Earlier pending-playtest statements describe the development stages and are
superseded by this acceptance. D08K5 and D08K11 remain separate work.
