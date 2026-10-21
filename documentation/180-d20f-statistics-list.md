# D20F - Statistics screen choices as a selectable list

Job: [D20F](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21:
"thats so much better! lock it in, close it out, 10/10"). Builds on [note 179](179-d20b-statistics-screen.md).

## Why

The PS1 prompt pair (Cross CONFIRM / Square SAVE) meant SAVE was V on a
keyboard, which a keyboard and mouse player cannot discover (user,
2026-10-21).

## What changed

`recomp/src/ttk/game_menu.cpp`, Page::Stats, Modernized one-player only:

- `draw_stats`: the lines are drawn `stats_lift` (25) higher than the
  original's row; once the wait ends, CONTINUE and SAVE GAME are drawn at
  row + `stats_list_y` (95), 19 apart, centred in the menu font with the
  cursor in blue, and SELECT / CONFIRM at the prompt row (as the death list).
  The list's place does not depend on how many lines the level has.
- Where the original offers no SAVE (level 29, or page 2 after a Vanilla card
  save) only CONTINUE is drawn, highlighted, with CONFIRM.
- `stats_move`: Up / Down and the wheel toggle the cursor (sound 1); nothing
  during the wait or with one item. `stats_choose`: cursor 1 opens SAVE GAME,
  otherwise CONTINUE as D20B. V (`defaults`) still opens SAVE GAME.
- Leaving SAVE GAME (Escape, or a save with GAME SAVED) puts the cursor on
  SAVE GAME, as the pause list returns to the item it came from.

No hooks, guards or framework files changed; codegen hash unchanged.

## The wait

The original's timer starts at 300 and loses `0x800d21fc` per tick, which
reads 10 on this screen: about 1 s, measured in Vanilla (`f4.py`) and in
ours (`f3.py`). Note 179 called it 5 s; corrected there.

## Evidence

Offscreen, real GPU, private cards (`recomp/analysis/d20b-stats`):

| Check | Result |
| --- | --- |
| After the wait | five lines, CONTINUE (blue) / SAVE GAME, SELECT / CONFIRM |
| Down, wheel up, wheel down | cursor moves each time |
| Enter on SAVE GAME | SAVE GAME page |
| Escape there | statistics, SAVE GAME highlighted |
| Save to slot 9 | statistics, SAVE GAME highlighted, GAME SAVED; sidecar `level 0`, `complete 1` |
| Up, Enter on CONTINUE | DUKE HILL (level 1) |
| Level 29 (index rewritten) | CONTINUE only; Down does nothing |
| V | SAVE GAME |
| Down / Enter / X during the wait | ignored |
| Vanilla (`v1.py`) | original screen; Square card SAVE GAME; Triangle back |
| Tests | Python 164 OK; `ttk-input-test`, `ttk-near-test`; `check_repo.py` |

Executable `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496`.

## Alternative offered by the user

Enter for CONTINUE and F2 for SAVE GAME as two prompts. The list was already
working when this came in; it is kept unless the user prefers the prompts.

## Limits

Offscreen only; not yet judged in a window or after a real level exit.
