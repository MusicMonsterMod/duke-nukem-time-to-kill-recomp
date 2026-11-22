# D53G - First-person idle gestures

Status: Done (user-accepted 2026-11-22: "perfect!! excellent work. this is better than i could have ever imagined it would be. lock it in and close it out"). Board: `MODERNIZATION_JOBS.md` D53G.
Dependencies: D53B (Done), D36 (Done). References: the [D36 audit](257-d36-first-person-presentation-audit.md),
the [D36H5 arm rig](305-d36h5-contact-arm-rig.md), [note 531](531-d53b-no-idle-lines-at-camera.md).

## Request

User, 2026-11-21 (D53B): "you cant actually see him crack his knuckles or
whatever in first person, so just randomly lowering the weapon just feels off.
... if we could provide first person specific poses for the idle animations".
D53B made IDLE ANIMATIONS ON play as VOICE ONLY in first person. Acceptance: a
visible, interruptible first-person idle that snaps out at once like the D53B
fidgets; third person and Vanilla unchanged. User at selection (2026-11-22):
"will likely need some reposing".

## Ownership (D36 audit)

In first person a drawn weapon is a camera-relative display pose: the right
hand joint draws at `weapon_matrix` (`prepare_display_matrix`,
`first_person.inc`), and the right arm and, for two-handed weapons, the support
arm are rebuilt from that matrix (`display_arm_matrix`). A native fidget's
skeleton is not what the eye sees, so the gesture belongs to the display
pose, not to the original animation. Layering it there keeps every arm
connected (the arms follow the matrix) and leaves the game's hand, aim,
events and timers untouched. It joins the existing layers: firing pose, recoil
kick, per-weapon tilt, the D08J7D high ready.

## Design (Modernized only)

- `console_idle.inc` (the D53B idle owner): IDLE ANIMATIONS ON with the global
  VIEW first person is a new mode, `IdleMode::FirstPerson`, instead of VOICE
  ONLY. It keeps VOICE ONLY's cadence: the native countdown stays full (no
  holster, no native fidget), the owner's beat runs every 3000 units standing
  idle (about 10 s), and the fidget-66 beat says one of its four lines (not
  with DUKE TALK OFF; ON with DUKE TALK OFF gestures without words).
- On every beat with a weapon at its ready pose (equipment 2, a weapon other
  than the boot, upper 5/20/29/35/39, Duke standing 63 on the ground, no
  weapon request flags `0x2cd`), one of three gestures starts, never the same
  twice in a row:
  - **inspect** (3.4 s): turned to show its side and brought in, turned to the
    other side, back;
  - **heft** (2.0 s): dipped and bounced in the hand;
  - **rest** (3.0 s): muzzle tipped up and pulled back, held, back to ready.
  Each is a few keys of (pitch muzzle up, yaw muzzle right, roll top left,
  x right, y down, z forward), Catmull-Rom between keys and flat at both ends,
  timed by the game's frame step (300 units a second), so pause holds it.
- Energy weapon (display slot 10), after the user's review (2026-11-22: "this
  is absolutely insanely good work ... the only one i want tweaking is 7,
  energy weapon. can see exposed shoulder joint looking disembodied"): held
  toward the middle with the support hand on its front, any lift of the muzzle
  or the gun raised the support upper arm until its cut shoulder end showed
  bottom left. Scaling down was not enough (still visible at 0.35), so it has
  its own three gestures of the same lengths that never rise above ready:
  inspect turns the gun while dropping it a little (y +20), heft bounces only
  below ready, rest lowers the gun (muzzle 20 down, y +30) instead of tipping
  it up. Stills at 13 times through the three: the arm runs off the bottom of
  the view throughout.
- Per-weapon amplitude: the RPG (display slot 8, also the Incendiary RPG)
  0.35, since it already fills the lower view at ready; rest on two-handed
  weapons (support slots 3, 5-11, not the energy weapon) 0.6, since it lifted the bare support forearm
  into the eye.
- Snap-out: any player activity (D53B's activity serial: keys, movement,
  mouse or stick look of 3+ counts) eases the gesture out over 0.1 s; fire or
  a weapon key cuts it in the same update, and so does any upper action (a
  shot, a reload), another weapon or holster; leaving first person, a camera
  view or the console also end it. The game itself never waits on it.
- Draw: `prepare_display_matrix` applies the pose about the grip in the level
  frame after the high ready and adds its offset to the view position.
  `RenderState.fp_idle` carries the live pose to redraw workers and is blended
  between ticks (pair and newer blends) when the weapon slot is unchanged, as
  the high ready.
- Debug: `ttk_input` -> `controls.idle` gains `fp_gesture`, `fp_t`,
  `fp_weight`, `fp_starts`, `fp_ends`, `fp_cuts`. Developer console (session
  only): `fpidle inspect|heft|rest` plays one now (first person, ready weapon),
  `fpidle <gesture> <seconds>` holds that pose, `fpidle pose p y r x y z` holds
  a raw pose, `fpidle off`. `DNTTK_FP_IDLE=0` turns the gestures off (A/B).
- Player text: the OPTIONS description reads "DUKE LEFT ALONE. FIRST PERSON:
  GUN GESTURES"; `run.py --idle-animations` help, the profile summary and
  `GAME_MANUAL.md` describe it. No schema change.
- Vanilla: every path needs `input_modernized()` (the owner and the gesture
  readiness); `run.py --vanilla` passes ON, which there is the original idle.
- No hook, codegen, framework, savestate or asset change.

## Evidence

Offscreen (SDL offscreen, Xvfb-free), private copies of the player's cards and
saves (`recomp/analysis/d53g/cards`, removed at closeout), capture on, debug port; LEVEL00 fresh
from `to_level` with the `dnstuff` cheat. Player save, card and profile hashes
unchanged after every run. Probes in `recomp/analysis/d53g/` (local):
`stills.py`, `weapons.py`, `axes.py`, `motion.py`, `fixture.py`.

- The UI slot 9 savestate (LEVEL00 security room) never drew Duke's weapon in
  this harness (`fp.weapon.hands` 0); a fresh LEVEL00 load draws it (305
  hands). Not investigated further: unrelated to the gesture.
- Axis sweep (`axes.py`): the first pitch sign tipped the muzzle down toward the
  eye, so the draw now uses +pitch as muzzle up; yaw +30 turns the muzzle right,
  roll +30 the top left, y -80 raises, x -80 moves left.
- Stills of every gesture's keys on the pistol, shotgun, Gatling, RPG, pipe
  bomb, energy weapon, flamethrower, Buffalo rifle and freezer; ready uppers
  observed: 5 (pistol), 20 (shotgun, energy, flamethrower, rifle), 29 (Gatling,
  freezer), 35 (RPG), 39 (pipe bomb). Key 1 gave no knife in this run, so
  knife, axe, crossbow, dynamite and the Holy Hand Grenade were not seen.
  Matched shots in `research/before-after/d53g/` (local): the "before" shot is
  the ready pose, which is what the previous build kept on screen (first
  person ON was VOICE ONLY); not captured from the previous build itself.
- Natural idle at 180 (`motion.py`, pistol, standing still): first gesture
  9.5 s after standing still, then a beat every 6.7-9.6 s of stillness; a full
  3 s rest sampled every 0.05 s (smooth in the stills); lines on the third
  beats (2 in about 45 s).
- Snap-out, final executable: W in a gesture, weight 0.67 at frame 4, gone by
  frame 7, walking (lower 72) at frame 4 as without a gesture; mouse 30 counts,
  gone by frame 8; fire, gone in the same update the shot starts (frame 3,
  upper 15; frame 8 before the fire cut).
- The yellow dots in some stills are LEVEL00 street fireflies, moving
  independently of the pose.
- Native: `ttk-controls-test` new D53G group (gesture on the beat with the
  weapon kept and the countdown full, activity eases out, a shot and fire cut
  at once, none in third person, Vanilla none); 159 groups pass, D53B
  included. The old LEVEL00 guard fixture no longer matched the test's overlay
  identity check; a private fixture was taken from a live LEVEL00 run with
  the two apartment-patch words restored to the original values the test
  itself names (`recomp/analysis/d53g/fixture.py` recreates it; the file was removed at closeout). Local unit
  tests 269 OK (2 skipped).
- Executable after the energy weapon change `0a231f94acf3d669dbeb003a29c2f23056ee5205247318292f901146d7bbfd0d` (controls test 159 groups and unit tests 269 OK again).

## Remaining

- User review 2026-11-22: every weapon's idles watched in first person ("they
  are incredible"); only the energy weapon needed the change above, which
  waits on the user's look.
- Not seen: thrown weapons and the crossbow (animated weapon channels), the
  upgraded weapons (they share their base framing), crouched idle (the beat
  needs lower 63), swimming and jetpack (no beat there).
- Unarmed first person keeps VOICE ONLY behaviour (lines, no visible gesture);
  the native fidget is not played there.
- The OPTIONS screen text change was not seen by the agent.
