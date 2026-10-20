# D19Q - SAVE GAME to match LOAD GAME

Job: [D19Q](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "im happy with this";
implemented autonomously at the user's request).
Pairs with [D19K](169-d19k-load-game.md) (LOAD GAME); slots are F7's
([D24B](113-d24b-savestate-menu.md)).

## What the player gets (Modernized)

- The pause list reads RESUME, (MISSION BRIEFING), OPTIONS, **SAVE GAME**,
  LOAD GAME, RESTART LEVEL, MAIN MENU, QUIT GAME. SAVE GAME is offered where
  LOAD GAME is (one-player games, challenge stages included).
- SAVE GAME is LOAD GAME's page (layout A): same rows, font, highlight,
  thumbnail, era and Console steel save time, heading SAVE GAME. Every slot can
  be chosen; an empty one shows NEW SAVE beside the list, one from another
  build SAVE OVER / FROM ANOTHER BUILD.
- Enter on an empty slot saves at once. On a used slot: OVERWRITE THIS SAVE?
  with the slot's picture, SLOT NN - LEVEL, YES / NO with NO highlighted
  (as the other prompts that lose something); Escape returns to the list.
- After the save the page stays, on that slot, with its new picture and time
  and SAVED TO SLOT NN; a refused or failed save says THE GAME COULD NOT BE
  SAVED and the slot keeps what it had. Escape returns to the pause list on
  SAVE GAME. (The framework's corner toast "Saved slot N" shows too, as for
  F7 and LOAD GAME.)
- Vanilla is unchanged (no SAVE GAME; the original memory card saves).

## Decisions taken (the job's open questions; the user reviews)

| Question | Decision |
| --- | --- |
| Where | Pause list beside LOAD GAME. The statistics screen's memory card SAVE GAME stays (D20); the death list has none (a save there is a death). |
| What state is saved | The paused game (it is the only state there is at that point). The slot's `.ttk` sidecar records `resume 1`; LOAD GAME on such a slot (title, pause or death) resumes play at once instead of showing the pause list again (no frame of the list). An F7 load of it opens the pause list, as any save taken while paused. |
| Thumbnail | When the pause list opens, the displayed image is still the last image of play: it is kept (`savestate_grab_thumb`) and written as the slot's picture after the save (`savestate_write_thumb`) instead of the framework's capture of the menu. This applies to F7 saves taken while our pause menu is up too. After a LOAD GAME into a paused save that does not resume, the loaded slot's picture is used; after an F7 load into a pause menu there is no picture of play and the menu image is used. |
| Failed writes | Framework fix: `boot_state_save` writes `<slot>.pst.tmp` and renames it over the slot when complete. Before, the slot was opened for writing first, so a failed write destroyed the old save. |
| Another build's slot | Can be overwritten after the prompt (it cannot be loaded anyway). |

## How it works

- `recomp/src/ttk/game_menu.cpp`: pages `Save` / `SaveAsk`, `Act::Save`;
  `draw_load(g, geo, save)` draws both lists; `draw_save_ask`; `save_now`
  stages `savestate_request_save(slot)` and keys wait (as for loads) until the
  framework is no longer pending; `game_menu_saved(slot)` (from
  `ttk_savestate_saved`, called only for a successful write) marks it written
  and puts the pause picture in. The resume after a load runs the pause list's
  own RESUME final in that tick without drawing the list.
- `savestate_panel.cpp`: the sidecar gains `resume 1` for SAVE GAME's saves;
  `savestate_slot_resumes(slot)`.
- Framework (exported patch; no hashed header, codegen hash `0x8bab543c`):
  `savestate_grab_thumb` / `savestate_write_thumb` (split from
  `savestate_capture_thumb`), atomic `boot_state_save`.

### Identity guard fix found on the way

The menu's executable identity check covered `0x800c3f98`-`0x800c4330`, which
holds GAME SETTINGS' word `0x800c3f9c` (GORE, STATUS BARS) and GAME SPEED
`0x800c3fa0`: the player's settings, restored from the memory card. A check
first made after a card with other settings had loaded (a savestate loaded
right after launch, before the title ran) failed, and every modern menu fell
back to the originals for the session (seen here as an F7 load into a paused
save showing no menu). Those 8 bytes are no longer guarded (two ranges, digests
taken from the disc executable; the rest of the range matches the executable
in every test state).

## Evidence

Offscreen, real GPU, 16:9, 4x, 120 Hz, private profile, cards and savestates
(`recomp/analysis/d19k-load`, `k.py`, `guards.py`, `gdiff.py`; local).
Executable `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b`.

| Check | Result |
| --- | --- |
| Pause list | SAVE GAME between OPTIONS and LOAD GAME (8 items with the briefing) |
| Empty slot 01 | NEW SAVE; Enter: saved, SAVED TO SLOT 01, gameplay thumbnail (no menu), PRESENT DAY, time; sidecar `level 0` / `resume 1` |
| Used slot 02 | OVERWRITE THIS SAVE?, NO highlighted; Up, Enter: SAVED TO SLOT 02 |
| Escape | pause list on SAVE GAME |
| Pause > LOAD GAME > slot 01 | load applied, play resumed (paused 0, screen 0, HUD) |
| Title > LOAD GAME > slot 01 | gameplay, resumed |
| Slot folder read-only, overwrite 02 | THE GAME COULD NOT BE SAVED; slot file unchanged (md5), no temp file left |
| F7 load of slot 01 right after launch | pause list (before the guard fix: no menu at all, identity false) |
| Tests | Python 160 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`; a pristine pinned framework plus the exported patch reproduces all 48 modified framework files |

## Limits

- Prompts still show PlayStation buttons (D19F).
- The software renderer path and Vanilla F7 saves through the new atomic write
  were not run separately (the write path is the same function).
- A paused save loaded by F7 then saved again on SAVE GAME gets the menu image
  as its picture (no picture of play exists in that session).
