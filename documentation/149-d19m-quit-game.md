# D19M - QUIT GAME closes the program

Job: [D19M](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "perfect!! thats working"); title main menu item in D19J (accepted 2026-10-19).
Builds on [D19H](147-d19h-pause-menu.md) (the pause and death lists) and
[D19I](148-d19i-menu-brightness.md).

## What the player gets (Modernized)

- Pause list: RESUME, MISSION BRIEFING, OPTIONS, LOAD GAME, RESTART LEVEL,
  MAIN MENU, **QUIT GAME**.
- Death list: CONTINUE, LOAD GAME, RESTART LEVEL, QUIT, **QUIT GAME**
  (challenge stages: REPLAY LEVEL, GIVE UP, QUIT GAME). The death list's QUIT
  keeps its original meaning (give up to the game over screen); user choice
  2026-10-19.
- QUIT GAME asks **QUIT GAME?** with YES / NO, NO highlighted, like RESTART
  LEVEL and MAIN MENU. NO or Escape returns to the list on QUIT GAME. YES
  closes the program.
- From RESUME (pause) or CONTINUE (death), Up reaches QUIT GAME in one press
  (the lists wrap).
- The title main menu has no QUIT GAME yet: it is still the original's code
  and gets the item when D19J re-implements it (user choice 2026-10-19).
- Vanilla keeps the original screens.

## How it works

`recomp/src/ttk/game_menu.cpp`: a new list action `Act::QuitGame`, appended to
both lists by `build_list`. The prompt reuses the D19H YES / NO page; its
heading "QUIT GAME?" is drawn with the game's text call (the game's text
table has no such string; every glyph is in the menu font).

YES pushes `SDL_QUIT`, the event the window's close button sends and the
console's `quit` command already pushes. The main loop's handler runs the
usual shutdown (`shutdown_runtime()`, exit code 0, exit origin
`sdl_window_close`), and run.py folds the confirmed menu settings
(`DNTTK_MENU_SETTINGS_FILE`, written on each CONFIRM) into the profile as on
any close. Menu keys are ignored once the event is sent. No guest state is
written, so saves and memory cards are never touched.

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19m-quit`, copied from D19I; script `q.py`, local; slot 5
gameplay level 0, slot 8 a private death screen). Executable
`de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43`.

| Check | Result |
| --- | --- |
| Pause list | seven items, QUIT GAME last, fits under the level name; Up from RESUME highlights it |
| QUIT GAME, Enter | QUIT GAME? with NO highlighted |
| Escape / NO | back to the list on QUIT GAME |
| YES (pause) | process exits with code 0 after 0.23 s |
| Death list | CONTINUE / LOAD GAME / RESTART LEVEL / QUIT / QUIT GAME under "3 CONTINUES REMAINING" |
| YES (death) | exit code 0 after 0.18 s |
| PC CONTROLS INVERT MOUSE ON, CONFIRM, then QUIT GAME YES | exit code 0; profile `invert_y` false -> true |
| MAIN MENU YES (regression) | mode 3, as before |
| Challenge stage 1, GIVE UP YES (regression) | statistics (mode 2), then the game goes on |
| Vanilla Start | original pause screen (12), Cross resumes |

Not verified: the real display and window (on a desktop session the same
event is sent as by the window's close button), Windows.
