# D42 - Death scene: stay on the scene, Open opens the menu, unlimited continues

2026-11-03. Status: In progress (playtest round 1: the mouse still moves the view, with a jitter, and the floor view is not EDuke32's angle; an EDuke32 clean-room study of the death view follows). Not yet committed.

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
