# D24B - Savestate menu (F7) in TTK fonts and disc art

Status: **Accepted** (2026-10-06, "im happy with all progress tonight"). Built to the mockup the user approved
in the D24A font picker ("build it out!").

## Design

640x480 panel, as before, scaled to the window by the framework:

- Backdrop: vertical steel gradient (`#14161c` to `#0a0b0f`), with the disc's
  radiation emblem (sprite record 0x800c4454) at 4x and about 10% opacity.
- Header: "SAVE STATES" in TTK Big Italic gold (CLUT 225) with the navy shadow,
  the F7 key label and the slot range in System 8x8 `#848484`.
- Three slot cards (the selected one has a blue border and edge,
  `#5fa8ff`):
  - 104x78 thumbnail;
  - "SLOT NN - <level name>" in TTK Medium Italic gold (blue CLUT 226 when
    selected);
  - save time in System 8x8 `#dedede`;
  - "LEVEL N <era>" in `#848484`.
- Prompt bar: the disc's Up/Down, Cross, Square and Circle sprites with
  Medium Italic gold labels, then the keyboard line.

## Implementation

| Piece | Where |
| --- | --- |
| Panel renderer | `recomp/src/ttk/savestate_panel.cpp` (Modernized only; returns 0 to keep the framework panel) |
| Glyph sets 3-7 (panel title, slot, selected slot, text, dim text) | `tools/local/build_ttk_fonts.py` -> `ttk-fonts.pack`; `ttk_font_draw()` / `ttk_font_line_height()` in `ttk_font.cpp` |
| Button sprites and emblem | `ttk-ui.pack` (TTKUI1) from the same builder, from the player's disc |
| Level per slot | `<slot>.pst.ttk` sidecar, `level N`, written by `ttk_savestate_saved()` after a successful save while guest RAM holds the saved state (gameplay only) |
| Level names | `ttk::level_title()` (the game's own strings, as the level select uses) |
| Framework seam | `psx_savestate_menu_set_panel_renderer()` in `psx_savestate_menu.c`; a call in `psx_frontend_on_savestate_notify()` (`main.cpp`); exported to the accepted patch, no header change |

Slot files are compressed (`BXSP`), so the level is recorded at save time
rather than read back from the state.

## Evidence

Private Xvfb runs on a private card copy (`recomp/analysis/d26b-spawn/`,
`d24b_test.py`, `d24b_load.py`):

- Saving slot 2 in level 6 wrote `level 6`. The panel showed
  "SLOT 02 - FAMILY JEWELS", "LEVEL 6 MEDIEVAL" and the fresh thumbnail. Older
  slots showed number and time only.
- Loading slot 2 through the panel from level 1 reached level 6, in Modernized
  and in Vanilla. Vanilla showed the framework panel.
- Tests: 115 Python tests (new `test_ui_sprite_pack`; 8-set font pack) and
  `ttk-font-test`, `ttk-inventory-test`, `ttk-controls-test`; the repo check
  passes.

## Limits

- Slots saved before this build have no sidecar: no level name until saved again.
- A savestate saved outside gameplay (title, menus) records no level.
- The panel is only drawn in Modernized; Vanilla keeps the plain framework menu.
