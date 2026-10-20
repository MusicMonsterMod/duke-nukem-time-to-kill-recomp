# D19K - LOAD GAME from our savestates

Job: [D19K](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "confirmed now working as intended. lock it in"). Follow-ups D19Q, D19R.
Builds on [D19J](150-d19j-title-menu.md) (title pages), [D19H](147-d19h-pause-menu.md)
(pause and death lists) and [D24B](113-d24b-savestate-menu.md) (F7 slots, level
sidecar, eras). Design: layout A of the private D19K mockup canvas (user
2026-10-20: "it's got to be A!").

## What the player gets (Modernized)

LOAD GAME on the title main menu, the pause list and the death list opens:

    LOAD GAME
    01 EMPTY                  [thumbnail]
    02 SAVED GAME
    03 TIME TO KILL            PRESENT DAY
    ...                        2026-10-20 22:41
    SELECT  CONFIRM  RETURN

- The 12 savestate slots (the F7 slots) in the main menu's font: number and the
  game's own level name. EMPTY slots and saves from another build are greyed.
  A slot saved before D24B's level record (or outside gameplay) reads SAVED
  GAME. Nine rows show at a time; the list scrolls, with the SELECT sprite
  beside it while rows are hidden.
- The list opens on slot 01 (user, 2026-10-20; it first opened on the newest save).
- Beside it: the chosen slot's thumbnail in a gold frame, its era (menu
  font) and save time (Console steel, revision 1). An empty slot shows EMPTY there; a save from another build shows its
  thumbnail darkened, CANNOT LOAD and FROM ANOTHER BUILD.
- Enter on a loadable slot asks LOAD THIS GAME? (larger thumbnail, SLOT NN -
  LEVEL; in game also UNSAVED PROGRESS WILL BE LOST in Console steel), CONFIRM /
  CANCEL as the original LOAD screen. Enter loads; Escape returns to the list.
  Enter on an empty or incompatible slot does nothing.
- Escape on the list returns to the list it came from with LOAD GAME
  highlighted (the title as the original LOAD's CANCEL does: state 6's idle
  time and level choice reset).
- A load that fails (the file is damaged) keeps the game as it was and shows
  THIS SAVE COULD NOT BE LOADED on the list.
- Vanilla keeps the original memory card LOAD GAME everywhere.

Defaults taken where the user did not choose (2026-10-20): the details are
level, era and save time only (no difficulty, health or armor, which the
sidecar does not record); memory card saves stay with Vanilla (no import);
the statistics screen's SAVE GAME still writes the memory card (D20).

## How it works

`recomp/src/ttk/game_menu.cpp`: two new pages, Load and LoadAsk, on the
screens the lists already own (title state 12 / sub-screen 12, pause screen
8, death screen 9), so nothing of the original LOAD (`0x80086e8c`: frontend
state 10, pause screen 5, death prompt 3) runs in Modernized. The D19H
`pending_load` / `death_load` hand-off to it is gone.

- **Slots** are read when the page opens: `savestate_slot_exists`,
  `savestate_slot_compatible`, `savestate_slot_mtime` and the D24B sidecar
  (`ttk::savestate_slot_level`, moved with `ttk::level_era` into
  `savestate_slots.h`). Level names come from the game's text table through the
  guarded level name ids (`0x800c3d2c`), as the pause list's title does.
- **Text** is drawn with the game's text, sprite and prompt calls like every
  other page (font 0 rows 14 apart from y -66; heading id `0x31`; prompts with
  ids SELECT / CONFIRM / RETURN / CANCEL). Dimmed text is D19N's modulation.
- **Thumbnail.** The menu font cannot draw a picture, so the thumbnail is a host
  image. The page passes the slot's 128x96 thumbnail with a 2-pixel frame and
  its rectangle as fractions of the 4:3 menu area (the 512x240 geometry at
  `0x800d2210`; a 4:3 image is `h * w / h_geo` units wide).
  `ttk_menu_thumb_image()` hands it to the renderer: `gl_swap_with_osd`
  (OpenGL) and `host_osd_draw_sdl` (software) place that rectangle in the
  4:3 area centred in the fitted picture, so it stays put in 4:3, 16:9 and the
  pillarboxed title. It is shown while the page draws it and goes 250 ms after
  the last draw, or at once when the page changes, the context ends or a
  savestate loads.
- **Loading** is F7's: `savestate_request_load(slot)`. The framework applies
  it at its next safe point (never inside one of our guest calls, D19J) and
  `savestate_check` drops the menu state when `psx_mod_savestate_loads()`
  changes. Keys wait meanwhile. If the request is consumed and no load was
  applied, the page shows the failure.
- **Taps.** A key queued in the same frontend tick behind the Enter that opened
  LOAD GAME (or QUIT GAME?) now acts on that page; before, the queue was
  cleared once the page left the title pages.

### Framework

Exported to `recomp/patches/time-to-kill-accepted-source.patch` (no hashed
header changed; old savestates load):

- `gpu_gl_renderer.c`, `host_osd.c`: the thumbnail draw (`ttk_menu_thumb_image`,
  declared beside `ttk_aim_crosshair_image`; its own SDL texture).
- `boot_state.c`: `boot_state_check_buffer` decided by "a reason was
  collected", so a caller passing no reason buffer
  (`savestate_slot_compatible(slot, NULL, 0)`) got 1 for every slot, an edited
  codegen hash included. It now collects into a local buffer then. Every
  existing caller passes a buffer and is unchanged.

## Evidence

Offscreen on the real GPU, 16:9, 4x, 120 Hz, private profile, cards and
savestates (`recomp/analysis/d19k-load`, copied from D19N; slot 08 a death
screen from D19H; slot 10 a copy of 07 with its codegen hash edited; script
`k.py`, local). Executable
`2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`.

| Check | Result |
| --- | --- |
| Pause > LOAD GAME (level 0) | our page, newest slot selected, thumbnail beside the list, date; Down moves; Enter: LOAD THIS GAME? with the warning; Escape twice: list on LOAD GAME |
| Pause, slot 03 | loads level 0, gameplay, no thumbnail left behind |
| Title > LOAD GAME (4:3 title) | page and thumbnail inside the pillarboxed picture; slot 03 loads: mode 1, level 0 |
| Title, Escape | main menu, LOAD GAME highlighted |
| Death list > LOAD GAME | page; Escape: death list on LOAD GAME; slot 03 loads: gameplay |
| Edited codegen hash (slot 11) | greyed, darkened thumbnail, CANNOT LOAD / FROM ANOTHER BUILD; Enter does nothing |
| Damaged body, valid header | load staged, framework "LOAD FAILED", game unchanged, page back on the list with the message |
| Save to slot 12, then LOAD GAME | slot 12 selected, TIME TO KILL, its fresh thumbnail (the death list) and time |
| Enter, then Down, Down in one tick (title) | cursor moved two slots (dropped before the fix) |
| Vanilla, title LOAD GAME | original memory card LOAD (state 10, "NO SAVED GAME ON THE MEMORY CARD") |
| Tests | Python 160 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`, `check_repo.py`; a pristine pinned framework plus the exported patch reproduces all 47 modified framework files |

## Revision 1 (2026-10-20): steel text and the startup card check

User review: the date / time and the in-game warning in Console steel (TTK
Medium Italic, the console's set), and the PlayStation memory card check gone
from Modernized startup without losing real initialisation.

### Console steel text

The host image is now composed per page in the 4:3 menu area at 960x720 and
cropped to what is drawn: the thumbnail (bilinear from the 128x96 capture,
dark outer line and gold frame) and text lines in glyph set 2 (TTK Medium
Italic, Console steel, navy drop shadow), one pixel of extra letter spacing,
drawn at 1x and scaled 2x (a third larger than the console's own size in the
480-line space, so it reads beside the menu font without competing with it).
It is recomposed only when the slot, rectangle or text changes. The GL path
blends it (`host_osd_text_has_alpha` knows `ttk_menu_thumb_pixels`); the SDL
path always blends. The warning reads UNSAVED PROGRESS WILL BE LOST: the
longer line was wider than the menu area at this size. Without the font pack
both lines stay guest text.

### The startup memory card check

The original's frontend, after the intro movies:

| State | Shows | Does |
| --- | --- | --- |
| 1 | DUKE NUKEM / TIME TO KILL, CHECKING MEMORY CARD (text `0x39`) | counts status `0x800bdeb0` up from -3 to -1, then probes the card `0x80082e6c(0,1)`; 0 / 1 (a card) to state 2, otherwise state 3 |
| 2 | the title, CHECKING SAVE GAME FILE (`0xf2`) | counts `0x800d24d0` up from -3, reads the save file `0x80083490(0)`, then if it read, unpacks the saved settings `0x80083070(0x800dd038)`; state 4 |
| 3 | THERE IS NO MEMORY CARD / IS NOT FORMATTED / DOES NOT HAVE 1 BLOCK AVAILABLE, YOU MUST HAVE ..., CONTINUE | re-probes every 15 ticks; Cross (or Start) gives state 4 |
| 4, 5 | PRESS START | |

The settings unpack is real initialisation: GORE, STATUS BARS, GAME SPEED and
the pad layouts are kept on the card (D19N), and D19N's hook at the end of the
unpack records them. So Modernized keeps every card call and removes only the
presentation and the waits:

- An entry hook on the text call `0x8008b454` (added to `game.local.toml`;
  regenerated with `psxrecomp_cli.py generate`; codegen hash unchanged
  `0x8bab543c`): from states 1 and 2 (return address in `0x80022440` -
  `0x80022664`), the CHECKING MEMORY CARD and CHECKING SAVE GAME FILE strings
  are drawn as their own empty end. The title heading stays, as on PRESS
  START next.
- At the frontend tick's entry (`title_entry`): state 1 with the status below
  -1 starts at -1 (probe now), state 2 with its wait below 0 starts at 0 (read
  now), state 3 goes to state 4 as its CONTINUE does.

Evidence (cold boot offscreen, Escape through the movies, `st.py`):

| Run | Result |
| --- | --- |
| Modernized, private card with a save | no CHECKING text (2 strings hidden, 2 waits skipped); settings unpacked (`card_loads` 1); state 1 to PRESS START 2.4 s, of which state 1 (the card probe, black screen) about 1.8 s as before |
| Modernized, blank (unformatted) card | no message; PRESS START 0.6 s after state 1 |
| Vanilla, blank card | the original NOT FORMATTED message with CONTINUE |

## Revision 2 (2026-10-20)

User: everything works as intended; slot 01 should be selected by default.
The list now always opens on slot 01. Executable
`37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9`.

## Limits

- Not verified on the user's display or with a real mouse: whether the mouse
  is captured after a load from the title (the test input forces the capture
  state). F7 loads already recapture automatically.
- Two-player: the pause list offers LOAD GAME in one-player games only, as
  the original.
- The software renderer path is built but was not run.
- Prompts still show PlayStation buttons (D19F).
- The startup card probe itself still blocks for about 1.5 s (black screen)
  in the BIOS card routines; only its presentation and the original's tick
  waits were removed. D19R has the research and options.
- SAVE GAME to match this page is D19Q.
