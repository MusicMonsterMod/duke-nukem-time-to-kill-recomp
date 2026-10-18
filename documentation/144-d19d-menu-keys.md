# D19D - PC menu keys: Escape backs out, Enter confirms

Job: [D19D](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-18: "amazing work").
Builds on [D19E](143-d19e-menu-closeout.md) (menu map, tap hold) and D04A
(Escape pauses and frees the mouse).

## Before

Modernized fixed keys while the mouse is free: arrows = D-pad, X = Cross,
C = Circle, Z = Triangle, Right Shift = Select, **Enter and Escape = Start**.
No key reached Square. The menus show PlayStation buttons, so a player had to
know that Z is the back button. Escape (Start) did nothing useful in most
sub-menus, and on two screens it was dangerous:

- the death screen's **REALLY QUIT?** prompt accepts Start as well as Cross,
  so Escape there quit to the game over screen;
- Start on PRESS START opens the main menu, so Escape went forward, not back.

## What each screen uses (measured, private savestates)

| Screen | Where | Cross | Triangle | Start | Square |
| --- | --- | --- | --- | --- | --- |
| PRESS START | mode `0x800bcbb0` 0, state `0x800d24e4` 5 | nothing | nothing | opens the menu | - |
| Main menu | state 7 | select | nothing | select | - |
| Difficulty, two player, CHOOSE CHARACTER, LOAD GAME | states 8, 9, 10 | confirm | cancel / back | confirm | - |
| OPTIONS list, SOUND, GAME SETTINGS | state 12, screen `0x800d24d2` 9 / 7 / 11 | confirm | back | confirm | - |
| CONFIGURE CONTROLLER (browse) | screen 4, `0x800dd276` = 0 | **assigns Cross** | **assigns Triangle** | opens the prompt | assigns |
| its CONFIRM / CANCEL / RESET / DEFAULT prompt | `0x800dd276` = 1 | confirm | cancel | - | reset |
| Pause menu | mode 1, `0x800be56a` 12, `0x800dd1c4` 0 | RESUME | **QUIT prompt** | resume | OPTIONS |
| Pause QUIT prompt (RESTART / MAIN MENU / CANCEL) | 12, `0x800dd1c4` 1 | - | cancel | - | restart |
| Pause OPTIONS / SOUND / CONTROLLER / GAME SETTINGS / LOAD | `0x800be56a` 2 / 7 / 4 / 11 / 5 | confirm | back | - | - |
| Death screen (CONTINUE / LOAD / RESTART / QUIT) | `0x800be56a` 15, `0x800dd1c6` 0 | CONTINUE prompt | **QUIT prompt** | nothing | RESTART prompt |
| Death prompts | `0x800dd1c6` 1 CONTINUE, 2 QUIT, 3 LOAD, 4 RESTART | confirm | cancel | **confirm** | - |
| Statistics | mode 2, `0x800d2672` 0 | next level | nothing | next level | SAVE GAME |
| SAVE GAME | `0x800d2672` 1 | OK | cancel | OK | - |
| Game over | mode 3 | CONTINUE | nothing | CONTINUE | - |
| Intro movies | mode `0xffff` | - | - | skip | - |

`0x800dd1c6` is the death screen's own prompt selector. The screen resets it
to 0 when it opens. The pause menu leaves a stale 4 in it, so it is read only
when `0x800be56a` is 15 (and `0x800dd1c4` only when it is 12).

## What changed

`recomp/src/ttk/pc_input.cpp` (Modernized only, `input_pad()`):

- `menu_screen()` classifies the screen from the addresses above: PRESS
  START, a plain menu, the pause/death top level, the controller assignment
  view, or none (gameplay, movies, anything not listed).
- **Escape** = Triangle on a plain menu; Start on the pause menu (resume)
  and in the controller assignment view; nothing on PRESS START and the death
  screen (there Start is CONTINUE); Start elsewhere as before (pause, skip
  movies). The D04A Escape pulse (a quick tap reaches the pad poll) carries
  the same button.
- **Backspace** = Escape on menus, nothing in gameplay or movies (it never
  pauses). It is not a bindable key.
- **Enter** = Cross on menus, the pause menu (RESUME) and the death screen
  (CONTINUE, then its confirmation); Start on PRESS
  START, in the controller assignment view and elsewhere (skip movies). Enter
  while captured is still the selected gadget (D08A).
- Each of these keys keeps the button it was pressed as until it is
  released. Otherwise a press that takes Triangle back from OPTIONS to the
  pause menu would become Start there and resume.
- **V** = Square while the mouse is free (V stays camera recenter while
  captured).
- **Mouse wheel** while the mouse is free: one Up/Down tap per notch (4
  fields pressed, 4 released; at most 6 waiting) on plain menu screens only.
  Clicks are not mapped: the click that focuses the window would confirm.
- Test-only: `DNTTK_TEST_INPUT` also takes `v`, `backspace`, `wheelup`,
  `wheeldown`.

The guest, the D19E tap hold, Vanilla and the F7/console overlays (the pad is
cut while they have focus) are untouched. No framework files changed, so the
codegen hash and savestates are unaffected.

## Evidence

Final executable `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe`. Offscreen on the real GPU, private profile, private
cards and savestates (`recomp/analysis/d19d-menus/cards`, copied from D19E;
slot 1 attract demo, 5 gameplay, 6 statistics, 7 title with a test save).
Scripts (local): `seq.py` (keys or pad presses, prints the menu state after
each), `psub.py`, `csub.py`, `dsub.py` (RAM diffs that found the prompt
flags), `sweep.py` (D19E per-screen tap counts).

Every row is a key press through the test input (uncaptured, as a player
in the menus) and the menu state read back after it:

| Screen | Escape / Backspace | Enter | Other |
| --- | --- | --- | --- |
| PRESS START | nothing / nothing | main menu | - |
| Main menu | nothing | ONE PLAYER -> difficulty, OPTIONS, two player | wheel down/down/up: 3 -> 0 -> 1 -> 0 |
| OPTIONS -> SOUND | SOUND -> OPTIONS -> main menu | OPTIONS, SOUND | - |
| Difficulty, two player, CHOOSE CHARACTER | back one level each (Backspace too) | confirm | - |
| LOAD GAME with the test save | back to the main menu (both keys) | - | - |
| CONFIGURE CONTROLLER (frontend and pause) | prompt, then cancel to OPTIONS | prompt, then confirm to OPTIONS | bindings unchanged (screenshots) |
| Pause menu | resume (45 ms taps too) | RESUME | V OPTIONS, C LOAD, Z QUIT prompt |
| Pause OPTIONS / LOAD / QUIT prompt | back to the pause menu | - | - |
| Death screen | nothing / nothing | CONFIRM CONTINUE, then gameplay | Z QUIT prompt, V RESTART prompt |
| Death QUIT / CONTINUE / RESTART prompts | cancel (before: Escape on REALLY QUIT? quit to game over) | confirm | - |
| Game over | nothing | title | - |
| Statistics / SAVE GAME | nothing / cancel (both keys) | next level | V SAVE GAME |
| Captured gameplay | Escape pauses and resumes; Backspace nothing | gadget, no pause | - |
| Intro movies | Escape skips | - | - |

- D19E sweep at 120 (45 ms taps, random phase): main menu 12/12, pause menu
  (Escape on/off) 12/12, main <-> OPTIONS (X, Z) 12/12, main <-> LOAD GAME
  with a save 12/12; latencies as in D19E (main 76 / 105 ms guest / screen,
  pause 59-60 ms).
- A first build that read the death prompt selector in the pause menu made
  Escape open the QUIT prompt there (1 of 12 in the sweep); the selector is
  now read on the death screen only, and the unit test covers the stale 4.
- Vanilla (`van.py`): menu reached with Start, cursor moves, 3-field taps
  still partly missed (2 of 5) as in D19E; replay off. Vanilla does not use
  this path (`pad_from_keyboard` calls `input_pad()` only in Modernized).
- `ttk-input-test` passes with new D19D cases (each screen class, the stale
  selector, the latch across a screen change, movies, the wheel taps and
  the wheel ignored in gameplay); local unit tests 144 run, OK (2 skipped);
  `check_repo.py` OK.

## Limits

- Not exercised: the two-player game itself, memory card error prompts, the
  pause LOAD with a saved game on the card (its screen is the same
  `0x800be56a` 5), RESTART on the death screen past its prompt.
- Screens the classification does not know keep Escape/Enter = Start, as
  before. The prompts are still drawn by the original with PlayStation
  button icons; the manual lists the keys.
- Accepted by the user.
