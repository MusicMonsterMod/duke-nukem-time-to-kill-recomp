# D20 - EDuke32-style quick save and quick load

Job: [D20](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "its all working!").
Builds on the savestate slots of [D19K](169-d19k-load-game.md) (LOAD GAME),
[D19Q](171-d19q-save-game.md) (SAVE GAME) and [D24B](113-d24b-savestate-menu.md) (F7).
The original memory card saves are split off to D20B; a framework savestate
bug found while testing is D20A (below).

## Reference: what EDuke32 does

Read from EDuke32's source (NBlood mirror, `source/duke3d/src/game.cpp`,
`menus.cpp`, `osdcmds.cpp`), not from memory:

- `gamefunc_Quick_Save` (default F6), only in `MODE_GAME`: with no
  `g_lastusersave` it jumps to `FAKE_F2` (the save menu). Dead: quote
  `QUOTE_SAVE_DEAD` and nothing else. Otherwise it captures a new screenshot,
  saves over `g_lastusersave` and points `g_quickload` at it; "Game Saved".
- `gamefunc_Quick_Load` (default F9), only in `MODE_GAME`: with no valid
  `g_quickload` it jumps to `FAKE_F3` (the load menu); otherwise it loads
  `*g_quickload` at once, with no confirmation.
- `g_quickload` is the last save made (menu or F6), the last game loaded from
  the menu, or the level-start autosave. Loading another slot from the menu
  resets `g_lastusersave`, so F6 then opens the save menu again. Nothing is
  kept across launches.
- Console: `quicksave` / `quickload` set `g_doQuickSave` 1 / 2, the same code
  as the keys; outside a game "Can't quicksave while not in a game."
- F2 / F3 open the save / load menus; a save from the menu closes it.

## Decisions (user, 2026-10-21: "go with your recommendations")

| Question | Decision |
| --- | --- |
| EDuke32's load-resets-last-save quirk | Not copied: the quick slot is the slot last saved to **or** loaded from, by any route. |
| Remember the slot across launches | No (as EDuke32). |
| Level-start autosave | Not in D20 (would need a reserved slot and an option; later job if wanted). |
| Vanilla | Unchanged: no quick keys, F6 stays the framework's port swap. |
| Original memory card saves (the D20 card's other half) | Split to D20B. |

## What the player gets (Modernized, one-player games)

- **F6** saves over the quick slot without stopping play; GAME SAVED (centered
  notice, Console steel). No slot yet: SAVE GAME opens. Dead (also on the death
  list): CAN'T SAVE WHILE DEAD.
- **F9** loads the quick slot at once. A SAVE GAME save (taken paused,
  `resume 1`) resumes play, as from LOAD GAME. No slot, or the slot is gone or
  from another build: LOAD GAME opens. On the death list F9 loads; with no slot
  it opens LOAD GAME there (Escape returns to the death list).
- **F2 / F3** open SAVE GAME / LOAD GAME straight from play. Escape on that
  page returns to play; a save there returns to play with GAME SAVED.
- Pause menu, title, two-player: the keys do nothing. Outside one-player
  gameplay the console commands say they are not in a game.
- KEY BINDINGS gains a SAVES section (QUICK SAVE F6, QUICK LOAD F9), bindable
  to F4, F5, F6, F9, F12 or the usual keys. F2 / F3 are fixed.
- Console `quicksave` / `quickload`, listed in `help`.
- Framework: in Modernized the F6 controller-port swap is off (still in the F1
  runtime menu); Vanilla unchanged.

## How it works

- `pc_input.cpp`: `quick_press` on a key or mouse press (no Ctrl / Alt / GUI,
  no repeats) maps the QUICK SAVE / QUICK LOAD bindings and F2 / F3 to
  `Quick`; `input_quick` asks the handler `game_menu.cpp` registers and applies
  its answer: `Done` (keep the mouse), `Load` (free it; recapture follows, as
  after F7), `Pause` (the Escape path: Start pulse, mouse freed). `Ignored`
  falls through to the usual F-key handling.
- `game_menu.cpp`: `game_menu_quick` (key or console thread, under the menu
  mutex) checks game mode 1, one player, identity, nothing pending, no menu but
  the death list, and Duke alive for saves (player word bit 1, health 0 as in
  `steroids.inc`). Saves and loads are the framework's `savestate_request_*`;
  `quick_poll` (from `game_menu_frame`) shows the notices once the framework is
  done. `quick_slot` is set in `game_menu_saved` (any successful save) and in
  `savestate_check` from `savestate_status_json` (any load). The pause opened
  for F2 / F3 goes straight to the page (`direct`, 1.5 s), and `direct_open`
  makes Escape and a finished save resume play.
- `level_select.cpp`: the console commands. `main.cpp` (framework patch): help
  lines and the Modernized port-swap gate.
- Launcher schema 38: `quick_save` / `quick_load` added (F6 / F9 when free);
  F4, F5, F6, F9, F12 are valid tokens; `describe` lists the keys.
- Diagnostics: steering word `quick=save|load|savemenu|loadmenu`
  (`DNTTK_TEST_INPUT`), and `game_menu.quick` = [slot, direct_open, direct] in
  `render_replay`.

## Evidence

Offscreen, real GPU, 120 Hz, private profile, cards and savestates
(`recomp/analysis/d20-quick`, `k.py`, `newgame.py`; local). Executable
`20f8edc1d1d6260e59747df7d31ae4a438f4081a8eb19ee402ea9924f1b3e930`.

| Check | Result |
| --- | --- |
| F6 after a load (slot 05) | slot 05 rewritten with a gameplay thumbnail and sidecar `level 0`; GAME SAVED; play not paused |
| Walk away, F9 | slot 05 back: start and loaded frames identical; play running |
| Console `quicksave` / `quickload` | same as the keys; `help` lists them |
| F2 from play | SAVE GAME (page 12, direct); Escape: play |
| F2, empty slot 03 | saved, GAME SAVED, play resumed; quick slot 3 (index 2), sidecar `resume 1` |
| F3 from play, Escape | LOAD GAME, then play (edge-only keys; with held steering keys the harness re-paused, an artifact) |
| F9 with the quick slot's file deleted | LOAD GAME opens; Escape: play |
| Death list: F3, Escape / F6 / F9 | LOAD GAME, death list with LOAD GAME highlighted / CAN'T SAVE WHILE DEAD / slot loaded |
| New game from the title (no load this session) | quick slot -1; F6 opens SAVE GAME |
| Title, pause menu | F6 / F9 do nothing |
| KEY BINDINGS | SAVES: QUICK SAVE F6, QUICK LOAD F9, hint SAVES OVER THE LAST SLOT USED |
| Tests | `ttk-input-test` (D20 key cases: F6 keeps capture, Ctrl+F6 and repeats ignored, F9 frees, F2 Start pulse, F3, console route, Ignored fall-through); Python 164 OK (12 skipped) incl. schema 38 migration and F-key tokens; pristine pinned framework + exported patch reproduces all 48 modified framework files |

## D20A: savestates taken shortly after a load (framework, pre-existing)

Found while testing; not caused by the quick keys. A save taken within about
0.3-1.5 s after a savestate load can produce a file that loads fine in the
same session but halts the emulator when loaded after a restart (or after
another load in a new session): `FATAL: GPU GP0 unknown command 0xFA (word
0xFA000045)`, DMA channel 2 walking a linked list at `0x80F40BD0`.

- Reproduced with the framework's own save (debug `savestate` op, the F7
  path), the console and F6, Modernized and Vanilla, 60 and 120 Hz. Saves
  2 s or more after a load, and saves without a recent load, were all good in
  these runs (about 20 trials). Intermittent at 0.7 s.
- The saved DMA state is idle and the CPU is in libetc `VSync` (0x800AF054),
  where most saves land, including good ones. So the bad list is built after
  the load from state the snapshot does not hold, or holds inconsistently. Not
  yet isolated (CD-ROM streaming state is a candidate: its section differs in
  a read-progress field).
- Impact: F6 right after F9 is a natural Duke habit, so quick saves make this
  more likely. The manual tells players to play a moment after a load before
  saving until D20A is fixed.

## D20C: colours after a load (opened, not investigated)

User report on acceptance: loading sometimes shows wrong colours,
intermittently, with their save in slot 1 ("try loading several times").
Backlog job D20C.
