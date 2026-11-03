# D42 - Death scene: stay on the scene, Open opens the menu, unlimited continues

2026-11-03. Status: Done (user-accepted 2026-11-03: "it's working! it's really working"; D08G9 revive in note 375).

## Request

User (backlog intake 2026-11-03, then "lets get started on the new ones we
just logged"): a death should not open the continues menu at once. Duke stays
on a visible death scene, as in Duke 3D / EDuke32, with a line at the top
naming the Open (USE) key. Open opens the existing death menu in the game's
own style; Escape on that menu returns to the death scene with Duke still
dead. Continues are unlimited and the "N CONTINUES REMAINING" text goes.
The console stays usable while dead. Reference: EDuke32 `deathtest`
recording `research/screencaps/Video_2026-11-03_20-08-25.mp4`.

Decisions (user, 2026-11-03, when selected):

- First-person death: floor-level first person (reverses D35L's accepted
  third-person death presentation, for first person only).
- Escape on the death scene opens the real pause (Escape) menu; Open opens
  the death menu; Escape on the death menu returns to the death scene.

## How the original reaches the death screen

Duke's death updates `0x80042be8` (ordinary deaths, player state `+0x15` = 3)
and `0x80042d04` (blasts and gibs, state 4) count the player's `+0x3a` down by
the tick's fields (`0x800d21fc`). When it runs out in a one-player game:

- continues left (`+0x279`, byte): waiting state `+0x15` = `0x19`, continue
  request `+0x35` = 1, `paused` `0x800be568` = 1, screen `0x800be56a` = 15
  (the death screen, which D19H replaces with our list);
- none left: event `0xff` (game over).

The waiting state's handler `0x80042de8` runs CONTINUE when `+0x35` = 2: it
takes one continue and respawns Duke at the checkpoint. The mode-1 pause entry
`0x800269a0` pauses on Start in any state except the waiting state `0x19`, so
the original already allows Escape during the dying states.

## Change (Modernized only)

- **Death scene** (`recomp/src/ttk/game_menu.cpp`, `death_update_entry`):
  new entry hooks on `0x80042be8` and `0x80042d04` (`game.local.toml`
  `mod_function_entry_funcs`, regenerated: two generated lines, codegen hash
  unchanged, savestates load). While Duke is dying in a one-player game the
  count is held just above the tick's fields, so the scene stays live
  (enemies, fire, sound). The continues byte is kept at one or more, so the
  game over for running out never happens.
- **Open**: `input_take_open_presses()` (`pc_input.cpp`) counts presses of
  the USE binding (keyboard, mouse or pad), not while the console holds input.
  Presses made before the death, or in the pause menu, are dropped. Open lets
  the count run out in that update; the original then opens the death screen
  and D19H shows our list.
- **Escape on the death list** (`Final::DeathScene`): the dying state the
  waiting state replaced goes back (`+0x15`), the continue request is cleared,
  the screen value is reset and play resumes through the main loop's own
  resume, as for RESUME. Unknown after a load made on the list (an old save):
  Escape then does nothing there, as before.
- **Escape on the death scene** is the original Start: the normal pause list.
  RESUME returns to the scene. SAVE GAME is left off the list while Duke is
  dead (quick save already says CAN'T SAVE WHILE DEAD).
- **Prompt**: "PRESS E TO OPEN MENU" (the bound key's name, upper case) in
  the centred top notice. It never replaces another notice (a cheat message
  shows first), and goes when a menu opens or Duke is up again. Framework:
  `host_osd_text_is()` (`host_osd.c`) tells whether a given text is the
  visible notice.
- **Unlimited continues**: the death list has no "N CONTINUES REMAINING";
  REPLAY LEVEL in the challenge stages no longer spends a continue; the
  statistics screen's CONTINUES line is gone (the count no longer means
  anything).
- **First-person death eye** (`modern_controls.cpp`, `first_person.inc`,
  `action_perspective.h`): with first person selected and Duke dead with no
  health in a one-player game, the normal camera keeps a presentation-only
  lease (`death_eye_state`, never movement, aim or fire). Perspective action
  `death` keeps the eye: it rides Duke's neck joint (32 above it) as the body
  falls, so the view ends at floor level; mouse and key look are frozen; the
  head is hidden as usual and the weapon is not drawn. Third person keeps the
  original death camera and animation.

Vanilla does not run any of this (the hooks return at once outside
Modernized).

## Evidence

Executable `65ec2d1d1e7e103943d302d630e4751e07ed33d8a794638769ed0baa1e887126`.
Private copies of the player's cards and savestates
(`recomp/analysis/d42/fixture`), offscreen, `DNTTK_TEST_EDGES`; player files
hash-checked unchanged after every run. `recomp/analysis/d42/probe.py`
(slot 10 save, `hurt=type,30000`).

- Baseline (before): death in first person showed the third-person body, then
  after about 2.2 s the dimmed list under "3 CONTINUES REMAINING".
- Ordinary death (type 1, state 3) and blast death (type 4, state 4, gibs):
  the count held for 8 s and more (no list), "PRESS E TO OPEN MENU" at the
  top. E: the list (no continues line). Escape: back to the scene, state 3 /
  4 again, unpaused, count held. Escape on the scene: the pause list; E there
  ignored; Escape (RESUME): back to the scene, still held.
- First person: the eye falls to the floor and stays there, no body or
  weapon in view; the list and the pause list show over the floor view. Third
  person: the original death animation and camera, gibs for a blast.
- Console open/close (grave) on the scene: no state change. `dnkroz` while
  dead: no effect (D08G9).
- Continues byte set to 0 before the death: held at 1, the list opened,
  CONTINUE respawned Duke (no game over).
- CONTINUE (YES): Duke back at the checkpoint, 100 health. RESTART LEVEL
  (YES): level restarts, Duke alive. QUIT TO MENU (YES): game over screen
  (mode 3), Enter, title (mode 0). LOAD GAME: slot 01 loaded, play resumed.
- 120 Hz: same hold, eye and round trip.
- `ttk-controls-test` (142 groups; the D35L-era checks now set health for the
  dead-flag case, plus a new D42 case: death eye only dead with no health,
  one player, first person), `ttk-input-test`, `ttk-flash-test`,
  `ttk-inventory-test` pass. Runtime patch exported; it applies to a clean
  pinned checkout and reproduces the working tree.

## Limits

- Not yet the user's playtest, real display or real controller. Vanilla was
  launched but could not be killed through the test input (Modernized only);
  Vanilla rests on the code gates.
- Typed `dnkroz` while dead is accepted as input but does nothing: the revive
  is D08G9, now unblocked.
- A savestate taken on the old death list loads to the list; Escape does
  nothing there until a new death (the dying state is not in the save).
- The view does not roll as Duke 3D's does; the first-person eye sits where
  the neck lies and keeps the heading at the moment of death.

## Round 2: EDuke32's first-person death view (clean room)

User (playtest round 1): the mouse still moved the dead view, with "a
vibrating look", and the view was not "at the angle right on the floor like
with eduke"; asked for a clean-room re-implementation of EDuke32's death
view, "as close as 1:1" as possible, using the `deathtest` save.

### What EDuke32 does (its behaviour, from its source and the recording)

From `research/eduke32` (`P_FragPlayer`, `P_Dead`, `G_MovePlayers`, the
camera setup in `game.cpp`) and the user's recording
`Video_2026-11-03_20-08-25.mp4`:

- Look input stops while dead; the view is level (horizon 100, no look
  offset); the angle does not follow the mouse. It eases toward the actor
  that killed the player (half the difference per tic), if any.
- The eye follows the body: x/y the body's, height 20 units above it (of 38
  standing). At the moment of death the body is lifted 16 units, so the eye
  starts about 36 above the floor and falls with sprite gravity (176 per tic
  squared, 30 tics a second) to 20: about 0.23 s.
- The screen rolls (Polymost's true 3D roll): `dead_flag` is random (0..255
  or 1024..1279 of 2048) and the per-tic roll adds `(floor z + eye z) >> 7`,
  a map-height term, so the final angle is effectively any angle; it turns
  32 / 2048 of a turn more while the body falls, then stays. The recording's
  death settled at about 45-50 degrees within the first frames.
- Nothing of the player's body is drawn; the weapon is gone.

The user's EDuke32 was run privately (Xvfb, copies of the config and
`deathtest` in a job folder, `-usecwd`): the save loaded, but in that
headless setup the game stopped responding after loads, so the live runs gave
no death captures; the source and the recording are the evidence. The user's
EDuke32 saves and config are unchanged (one `-help` probe rewrote the log
file `~/.config/eduke32/eduke32.log`).

### TTK implementation (our own code)

- `modern_controls.cpp` `DeathView`: on the first camera update of a death
  in first person: a random roll (uniform over the turn) and the death time.
  The eye height above Duke's root is 294 (the upright eye used for swim and
  climb views) times (36 - 16 * fall) / 38, fall from the same gravity curve;
  the roll grows by 32 / 2048 of a turn over the fall. The pitch levels out
  (quartered per tick). `view_matrix` turns its right and down rows about the
  forward row by the roll (forward unchanged, so aim/seeding reads are not
  affected). The eye sits over the root, not the neck.
- Mouse: the late (high refresh) redraw look (`late_view`) now refuses the
  death eye too. The vibration was those redraws applying live mouse motion
  between game frames that kept the frozen view.
- Body: a dying Duke is drawn from the game's second object loop (call
  `0x8003771c`, list step `0x80037724`), which the eye-view draw hooks did
  not recognise; while the death eye runs that draw is Duke's, and every
  joint takes the camera matrix (the native depth test rejects it), so
  nothing of Duke shows.
- Not reproduced: the turn toward the killer (TTK's attacker is not known
  here yet; the reference death is self-inflicted, with no turn).

### Evidence (round 2)

Private fixture, offscreen, slot 10 save, first person:
- fall frames at 0.05-2 s: rolled level view, no body or weapon; the roll
  differs per death (seen about 150, 70 degrees);
- mouse `dx` / `dy` while dead: identical view; at 180 Hz, six frames 30 ms
  apart with alternating mouse motion: the same geometry (only water
  animates);
- E opens the list and Escape returns to the scene as before;
- `ttk-controls-test` (142 groups), `ttk-input-test`, `ttk-flash-test` pass.

## Round 3 (user, 2026-11-03: "the death scene feels incredible")

Requests: turn the view toward the killer; CONTINUE without "confirm
continue" (unlimited now); `dnkroz` on the death scene revives Duke (D08G9,
[note 375](375-d08g9-dnkroz-revive.md)); the music should keep playing.

- **Killer turn** (EDuke32's `wackedbyactor`: half the angle per tic toward
  the actor that killed the player): Duke's damage handler `0x800a40a8`
  receives the attacker or source object at stack +0x10; the D39E hook now
  records it for every hit (both renderers). At death, if the last hit was
  within 2 s and its source is not Duke, the death view turns toward that
  object's position (+4 / +12), half the remaining yaw per 1/30 s (time
  based, so the same at any refresh), for as long as the object keeps its
  type id (+0x2c). Self-inflicted and test hits do not turn. Evidence: slot 3,
  health 1, the pig cop (id 57) kills Duke; the trace named it as the killer
  and the rolled view kept the pig cop centred while it moved (yaw
  2.49 -> 2.11 over 1.75 s following it).
- **CONTINUE**: acts at once from the death list (REPLAY LEVEL too); the
  other prompts stay (NO first).
- **Music**: the killing hit pauses the CD music (`0x800a470c` ->
  `0x8003c478`, drive state `0x800c26a8` = 0). When the death scene starts,
  Modernized calls the game's own resume `0x8003c240` (as CONTINUE does;
  guarded). Evidence: drive state 3 (seek) at the death, 2 (playing) from
  then on; the music position kept advancing through the scene, the death
  list and after CONTINUE (`0x2410a` -> `0x24747`, never back).
