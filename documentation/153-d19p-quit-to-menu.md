# D19P - Death screen: QUIT TO MENU and QUIT GAME

Job: [D19P](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "lock it in ... it's great").
Builds on [D19H](147-d19h-pause-menu.md) and [D19M](149-d19m-quit-game.md).

## What the player gets (Modernized)

- Death list: CONTINUE, LOAD GAME, RESTART LEVEL, **QUIT TO MENU**, QUIT GAME.
- QUIT TO MENU asks **QUIT TO MENU?** (NO highlighted). YES gives up as the
  original's QUIT did: the game over screen, then the title (user choice
  2026-10-19: keep the game over screen).
- Unchanged (user choice): the pause list's MAIN MENU (asks REALLY QUIT?),
  the challenge stages' GIVE UP (asks REALLY QUIT?), QUIT GAME. Vanilla keeps
  the original death screen.

`recomp/src/ttk/game_menu.cpp`: the label in `build_list`; `ask_heading`
returns -2 for the non-challenge `Act::Quit`, drawn as "QUIT TO MENU?" with
the game's text call (every glyph is in the menu font).

## Identity guard fix found while testing

A level 1 savestate showed the original death screen in Modernized
("executable identity mismatch: original OPTIONS kept"). The D19 guard
"font glyph tables" (`0x800c3f38`, 0x3f8 bytes) covered `0x800c3f94`, the
selected inventory item (D08A6, written by `[` / `]` and pickups). The
identity is checked once per session, so if the first menu use came with an
item selected (for example a savestate loaded before any menu), every
Modernized menu stayed original until restart. At a normal launch the title
menu checks first with the item still 0, which hid it. The guard is now two
ranges around that word (`0x800c3f38` 0x5c, `0x800c3f98` 0x398); digests
taken from slot 5, whose full range matches the old digest.

## Evidence

Private profile, cards and savestates (`recomp/analysis/d19p-quit`, local;
`q.py` offscreen, `x.py` on Xvfb with capture). Executable
`959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63`.

| Check | Result |
| --- | --- |
| Level 0 death (slot 8 and a fresh death from slot 5) | list shows QUIT TO MENU; Enter asks QUIT TO MENU? with NO; Escape back; YES: game over (mode 3), Enter: title (mode 0) |
| Level 1 death (D22D savestate, god mode cleared), before the guard fix | original death screen; identity mismatch at `0x800c3f94` only |
| Level 1 death, after the fix | our list with QUIT TO MENU (screenshot) |

Not verified: QUIT TO MENU YES in level 1 and in first person (on Xvfb the
harness's key presses did not reach the menu; the action is the unchanged
D19H QUIT path); the real display; the user's playtest.
