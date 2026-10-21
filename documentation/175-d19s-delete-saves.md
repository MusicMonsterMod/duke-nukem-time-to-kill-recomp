# D19S - Delete saves from LOAD GAME and SAVE GAME

Job: [D19S](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "perfect. lock it in").
Pairs with [D19K](169-d19k-load-game.md) (LOAD GAME) and
[D19Q](171-d19q-save-game.md) (SAVE GAME).

## What the player gets (Modernized)

- On LOAD GAME (title, pause and death lists) and SAVE GAME, Delete on a used
  slot opens DELETE SAVED GAME: with the slot's name below it in quotes, in
  Console steel lettering (as the save time on the list; user request), e.g.
  `"SLOT 02 - TIME TO KILL"`, under the slot's picture, then YES / NO
  with NO highlighted. The heading stays the page's (LOAD GAME or SAVE GAME).
- Enter on YES deletes; Enter on NO or Escape returns to the list on the
  same slot, nothing changed.
- After a delete the list shows the slot EMPTY at once (LOAD GAME: EMPTY
  beside the list; SAVE GAME: NEW SAVE) and says DELETED SLOT NN. If the
  slot file cannot be removed the slot is unchanged and the page says SLOT NN
  COULD NOT BE DELETED.
- Delete on an empty slot does nothing. A save from another build (greyed,
  not loadable) can be deleted.
- Vanilla is unchanged (no LOAD / SAVE GAME pages; the menu keys are
  Modernized only).

## Decisions taken (the job's open questions; the user reviews)

| Question | Decision |
| --- | --- |
| SAVE GAME too | Yes: same key and prompt, since both pages are the same slot list. |
| Rebindable | No. Delete is a fixed menu key like Enter, Escape and V (KEY BINDINGS rows are gameplay actions). It is not bindable in play either, so there is no conflict. |
| Shown as a prompt | Not yet: the bottom bar keeps SELECT / CONFIRM / RETURN. Showing PC keys is D19F's job; the manual documents Delete. |
| Name line | Console steel lettering (user request after the first build: clearer to read). Steel has real double quotes; without the font pack it falls back to the menu font, which draws them as single quotes. |
| What is removed | The slot's `.pst`, any leftover `.pst.tmp`, its `.thumb` (framework `savestate_delete_slot`), then the `.ttk` sidecar (game side), only after the slot file is gone. Memory cards, other slots and rewind are not touched. |

## How it works

- `recomp/src/ttk/pc_input.cpp`: Delete is menu key-down 7 (`delete_taps`);
  the diagnostic steer file accepts `delete`.
- `recomp/src/ttk/game_menu.cpp`: `Page::DeleteAsk`, `delete_ask` (from Load
  or Save, used slots only), `delete_now`, `draw_delete_ask`. The title tick
  treats DeleteAsk as one of its LOAD pages.
- `savestate_panel.cpp`: `ttk::savestate_slot_delete` (framework delete, then
  sidecar).
- Framework (exported patch, verified against the pinned dependency: 48
  files reproduced; `savestate.h` is not a hashed header, savestates stay
  valid): `savestate_delete_slot(slot)`, refused while a save or load is
  staged; returns 1 only when the slot file is gone.

## Evidence

Offscreen, OpenGL, 4x, 16:9, 120 Hz, private profile and a private copy of
the D19Q test savestates (`recomp/analysis/d19k-load/cards-s`, `k.py`;
local). Executable `64e051548f31702e0509e9d9cb0131ef749e438c345ab2ca50ea4ee51a4d5974`.

| Check | Result |
| --- | --- |
| Pause > LOAD GAME, slot 02, Delete | DELETE SAVED GAME: / 'SLOT 02 - TIME TO KILL', NO highlighted (first build, menu font); steel build: "SLOT 01 - TIME TO KILL" in steel, rest unchanged |
| Enter on NO; Delete then Escape | back on the list, slot 02, files unchanged |
| Up, Enter (YES) | slot 02 EMPTY, DELETED SLOT 02; exactly its `.pst`, `.pst.ttk`, `.thumb` gone; all other files (cards included) identical by md5 |
| Delete on the now-empty slot | nothing happens |
| SAVE GAME, slot 10 holding a file from "another build" (zeros) | prompt 'SLOT 10'; YES deletes, NEW SAVE; Enter then saves there again (SAVED TO SLOT 10, picture, time) |
| Title > LOAD GAME after a restart | slot 02 still EMPTY |
| Title, save folder read-only, delete slot 01 | SLOT 01 COULD NOT BE DELETED; slot 01 files identical |
| Death list > LOAD GAME, delete slot 03 | deleted; Escape returns to the death list; LOAD GAME slot 01 then loads and resumes play |
| Vanilla boot | runs; savestates untouched |
| Tests | Python 162 OK (2 skipped), `ttk-input-test`, `ttk-live-env-test` |

## Limits

- Real double quotes in the menu font itself: follow-up D19T.

- No on-screen hint that Delete exists (D19F).
- The F7 panel has no delete of its own (unchanged). Its view of a slot
  deleted on LOAD GAME was not tested separately (it reads the same files).
- Not run on the software renderer (the menu code path is shared).
