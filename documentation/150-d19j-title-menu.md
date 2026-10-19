# D19J - Title main menu re-implemented

Job: [D19J](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "fully accept").
Builds on [D19](145-d19-game-menu.md) (our screens drawn by the game's own
calls), [D19H](147-d19h-pause-menu.md) (each choice is the original's own
result), [D19M](149-d19m-quit-game.md) (QUIT GAME) and
[D19E](143-d19e-menu-closeout.md) (taps over the loading spinner).

## What the player gets (Modernized)

After PRESS START, over the attract demo, in the original's look:

    DUKE NUKEM / TIME TO KILL
    ONE PLAYER / TWO PLAYER / LOAD GAME / OPTIONS / QUIT GAME
    SELECT  CONFIRM

- **ONE PLAYER** opens CHOOSE DIFFICULTY LEVEL (WUSSY, GET SOME, LET'S GO,
  DEATH WISH; SELECT / CONFIRM / RETURN). Enter starts the game at the
  highlighted difficulty. Escape returns to the main menu and puts back the
  difficulty the page opened with, as the original does.
- **TWO PLAYER** opens TWO PLAYER OPTIONS (LEVEL, TIME, KILLS, SPLIT; Left /
  Right change), then CHOOSE CHARACTER. The original reads each player's own
  pad there, and the PC build has no second pad, so the original could never
  get past this screen. One keyboard now chooses for both: player 1 first
  (Left / Right, Enter), then player 2, then Enter on START. Escape takes
  back the last confirmed choice, then goes back to TWO PLAYER OPTIONS.
- **LOAD GAME** is the original memory card screen (D19K will replace it).
  CANCEL comes back to the main menu on LOAD GAME.
- **OPTIONS** is D19's OPTIONS. Back returns to the main menu on OPTIONS.
- **QUIT GAME** asks QUIT GAME? with NO highlighted. YES closes the program
  (D19M's window close event), NO or Escape returns to the item.
- Keys: arrows or the wheel (held: repeat after 350 ms, every 80 ms), Enter
  confirms, Escape or Backspace go back. Escape on the main menu does
  nothing, as the original.
- Left alone on the main menu for the original's idle time (`0x4650` fields,
  about five minutes), the game goes back to PRESS START, as the original
  does.
- If the original's debug level choice is on (`0x800d2528` = 5), the main
  menu shows its fifth item, the level name, with the original's Left / Right
  steps (0-3, 5-12, 21-29).
- Vanilla keeps the original menus.

PRESS START stays the original screen. It has no cursor and already works
with Enter (D19D).

## How it works

`recomp/src/ttk/game_menu.cpp`. The frontend tick `0x80022324` dispatches the
state `0x800d24e4` (table `0x800131b0`). The original pages:

| State | Page | Handler | Variables |
| --- | --- | --- | --- |
| 6, 7 | main menu | `0x80022b44` / `0x80022b64` (inline) | cursor `0x800d24e6`, items `0x800d2528`, idle `0x800d24d0` |
| 8, players `0x800bdea4` = 1 | CHOOSE DIFFICULTY LEVEL | `0x80084534` | cursor `0x800c3d7c`, saved `0x800c3df6`, fresh `0x800c3df8` |
| 8, players = 2 | TWO PLAYER OPTIONS | `0x8008484c` | row `0x800dd1b0`, level / time / kills `0x800c3dfc` / `fe` / `0x800c3e00`, split `0x800dd1b2`, fresh `0x800c3e04` |
| 9 | CHOOSE CHARACTER | `0x80085060` | choices `0x800dd1b8` / `bc`, locks `0x800dd1c0`, saved `0x800c3e08`, fresh `0x800c3e0c` |
| 10 | LOAD GAME | `0x80086e8c` | (original) |
| 11, 12 | OPTIONS | `0x800841c4` | sub-screen `0x800d24d2` (D19) |

The main menu's Cross runs table `0x800131e8`: ONE PLAYER and the level item
set players 1 and state 8; TWO PLAYER players 2 and state 8; LOAD GAME
players 1 and state 10; OPTIONS state 11.

### Our pages live on state 12, sub-screen 12

State 12 calls the frontend background `0x800220f4` (D19's tick hook), then
dispatches its sub-screen. Sub-screen 12 has no handler, so the guest draws no
menu there and reads no menu input, while the tick's tail goes on running the
attract demo and its scene loads. Nothing outside the tick reads the state
(every load and store of `0x800d24e4` is in `0x80022324`-`0x80023350`).

- **Entry hook `0x80022324`** (new in `game.local.toml`): before the pad read
  and the dispatch. States 6 and 7 (the original main menu, after PRESS START,
  LOAD's CANCEL, or a savestate) and 8 / 9 (a savestate on those pages)
  become state 12 / sub-screen 12 with our page, before the original handler
  can draw anything. State 6's part (idle time, level choice 0) is done by our
  main menu.
- **Keys** come from `input_pad()` as in D19 / D19H. On a title page they are
  queued, and at state 12's call to `0x800220f4` (return address
  `0x80023248`) each one runs the original handler's response on the
  original's variables, with its sound: the same wraps, steps, fresh-flag
  entries, saved difficulty, split mode (`0x800be564`), level choice
  (`0x800bdeaa` = level + 15) and so on. Savestates therefore hold the pages'
  real state.
- **Leaving** for LOAD GAME (10) or PRESS START (4, idle) is set at the next
  tick's entry, so the image of the choosing tick still shows our page, as the
  original handler draws its screen in the tick that chooses.
- **Starting a game.** The original difficulty (8) and character (9)
  handlers exit the frontend themselves (the tick returns 1 from
  `0x80023548`). Our CONFIRM on those pages sets that state at the next
  entry, with the page's variables, and at that state's call to `0x800220f4`
  (return address `0x80023130` / `0x800231b4`) clears the new-press bits of
  the pad history (`0x800d1aa0`) and sets Cross (`+0x3c`). The handler then
  draws its page, which is ours call for call (same text ids, positions,
  flags and order), sounds and starts the game exactly as on the original's
  Cross. For two player the locks are 3 (both chosen), so it draws START /
  CANCEL as we did. No key reaches the pad until the frontend has exited.
- **OPTIONS** sets sub-screen 9, which D19 turns into ours in the same call.
  Back from OPTIONS writes sub-screen 12 with OPTIONS highlighted (before,
  sub-screen 10 gave one tick with no menu, then state 6).
- **Drawing** is D19's: from the composition and loading-logo hooks, once per
  image, with the game's text (`0x8008b454`) and the prompt helper
  `0x8008411c` itself, called with the original's text ids. QUIT GAME and
  QUIT GAME? are drawn from our strings in the same fonts.

Code identity adds the frontend tick and states (`0x80022324`, `0xfd8`), the
state and choice tables, the three handlers, the difficulty ids
(`0x800c3de4`) and the two player / character tables and formats
(`0x80016e8c`). A mismatch keeps the original menus. `DNTTK_GAME_MENU=off`
(developer) does the same.

### Savestates during our guest calls (framework fix)

The first savestate test on a title page crashed the run
(`FAIL-FAST unknown dispatch addr=0x800C0000`). The save was admitted at an
interrupt return inside one of our own text calls (pc `0x8008B4DC`, return
address our sentinel `0x800000FC`). That admission longjmps
(`savestate_poll_irq_return`, `interrupts.c`), so it abandoned our host frames:
the stack and scratchpad were never restored and the guest went on inside the
call. Every plugin that calls guest code on its own stack had this exposure
(the menus, the weapon aim query); the title pages make many calls per image,
so it showed.

Framework change (exported to the tracked patch, codegen hash unchanged):
`psx_mod_host_call_begin()` / `psx_mod_host_call_end()` in `mod_plugins.h`.
While a plugin call is open, `savestate_poll` and `savestate_poll_irq_return`
wait (save and load). The menu's `Guest::call` and the weapon aim query use
it. The frame replay's calls run inside the frozen replay state and are left
as they were.

The menu also drops its host state when `psx_mod_savestate_loads()` changes:
a loaded state reopens our screens from the guest's own values (the title
pages at the main menu, D19 at OPTIONS, D19H at the lists). Before, a load
taken on one page could keep a different page's host state.

## Evidence

Offscreen on the real GPU, 16:9, 4x, 120 Hz unless noted, private profile,
cards and savestates (`recomp/analysis/d19j-title`, copied from D19M; script
`t.py`, local; slot 1 attract demo / PRESS START, 3 main menu 2.5 s before a
demo scene load, 5 gameplay, 7 title with a test save on the card). Final
executable `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d`.

| Check | Result |
| --- | --- |
| Original screens (`DNTTK_GAME_MENU=off`) | main menu, difficulty, two player options, choose character captured for comparison |
| PRESS START, Enter | state 12 / 12, our main menu with QUIT GAME, same font, colours, title and prompts as the original |
| Up from ONE PLAYER | QUIT GAME; Enter: QUIT GAME? NO highlighted; Escape back; Escape on the menu: nothing |
| ONE PLAYER | CHOOSE DIFFICULTY LEVEL as the original; Down moves; Escape restores the difficulty and returns on ONE PLAYER |
| TWO PLAYER | TWO PLAYER OPTIONS; Right / Left change MESA -> CASTLE, 5 -> 10 MINUTES, KILLS 5 -> UNLIMITED, SPLIT VERTICAL -> HORIZONTAL |
| CHOOSE CHARACTER | player 1 then player 2 lock, both names gold, START / CANCEL; Escape unlocks, then back to options, then main menu on TWO PLAYER |
| LOAD GAME, Escape | original LOAD (state 10); back on LOAD GAME |
| LOAD GAME with a save (slot 7) | original LOAD THIS GAME?, Enter loads it: mode 1, level 1 |
| OPTIONS, SOUND, PC CONTROLS confirm, back | D19 OPTIONS, original SOUND and back, setting confirmed; back on our main menu on OPTIONS |
| Start one player at WUSSY | handoff traced (`title handoff 8`); mode 1, difficulty 0, level 0 |
| Presents around the start (72) | the difficulty page, heading pixels identical in every image, then black, then the loading screen; the original start shows the same black presents |
| Start two player | mode 1, level 15 (MESA), characters 2 / 1, split mode 2 (horizontal), as the original handler stores them |
| QUIT GAME YES | exit code 0 in 0.18-0.23 s |
| Idle time run out (timer set low) | PRESS START (state 5) |
| Taps through a demo scene load (slot 3) | 23 / 23 at 120 and 60, 31 / 31 at 180 (seed 7): cursor ends where the count says |
| PRESS START -> menu, 72 presents | no image of the original four-item menu |
| Cursor move, 72 presents | new highlight after 9 presents (original: 21); one present shows the old highlight (original: 13) |
| 12 saves and 4 loads on our pages | no crash; no save inside a menu call; menu works after |
| Pause list (slot 5) | list, OPTIONS, back, resume |
| Pause MAIN MENU YES | mode 3, Enter: PRESS START |
| Vanilla | original main menu (state 7) and TWO PLAYER OPTIONS (state 8) |
| Tests | Python 146 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`, `check_repo.py`; the patch applies to the pinned framework and reproduces the live files |

## Limits

- Not verified on the user's display; two player not played past the start
  (the second player has no input in the PC build, as before).
- PRESS START is the original screen.
- Menu changes flash back to the previous image after a change (the D19C
  frontend redraw shows the newer image, then the older real image). The
  original menus show the same: 13 presents of PRESS START after the menu
  first appears, in both builds. It is one present on cursor moves in ours.
  Follow-up D19O: these sequences were taken right after the dump's own
  stall; the flash follows a replay reset, not each change
  ([note 155](155-d19o-menu-flashback.md)).
- A savestate taken on a title page loads to the main menu.
- The original pad's other buttons on these pages (Square, Circle) did
  nothing there and do nothing here.
- Prompts show PlayStation buttons (D19F).
