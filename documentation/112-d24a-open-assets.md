# D24A - Public clone gives the full experience (no Duke Nukem 3D art)

Status: **Needs playtest** (2026-10-06). The look needs the user's confirmation,
and the fresh-clone test must be re-run once the change is committed.

## Result

No Duke Nukem 3D art and no `research/` input remain in the build or the game:

| Use | Before | Now | Source and licence |
| --- | --- | --- | --- |
| Messages (styles 0/2) | Duke 3D message font | TTK Big Italic, Console steel, 1x, spacing 1, navy drop shadow | Player's disc (`/DATA/FONTS.RAW`), built at build time |
| Headings (style 1) | Duke 3D Atomic font | TTK Big Italic, gold CLUT 225, 2x | Player's disc |
| Console / compact (style 3) | Duke 3D message font 1x | TTK Medium Italic, Console steel, 1x, spacing 0, shadow | Player's disc |
| Missing glyphs (`! % ( ) & + "` ...) | - | TTK system 8x8 (PSY-Q FntPrint) in the set's palette | Player's disc (`SLUS_005.83` 0x800c7d70) |
| Switcher digits and % | Duke 3D THREEBYFIVE | 3x5 Microfont, `#989c58` | Tracked, CC0 1.0 (nimaid) |
| Switcher selection frame | Duke 3D tile 20 | `item-frame.png` | Tracked, original project art |
| Crosshair | EDuke32 tile 2523 | `crosshair.png`, lime `#80ff00` | Tracked, original project art |
| HUD item icons | (D08A3, local pack) | built from the disc in every build | Player's disc |

The choices are the user's, made in the D24A font picker (2026-10-06). Uses
that do not exist yet (menus, inventory screen, save slots, mission items)
keep their recorded choices for later jobs: menu titles and inventory headings
in Big Italic gold 2x; menu items, mission item and save slot names in Medium
Italic gold 2x (selected item in blue CLUT 226); lists and details in the
system 8x8 (`#dedede`, selected `#00bdef`, details `#848484`), all capitals.
"Console steel" is a project palette (not disc data), so it ships openly.

## Implementation

- `tools/local/build_ttk_fonts.py`: reads the prepared disc (FONTS.RAW hash
  pinned) and the executable. It segments the two italic fonts, renders three
  coloured glyph sets and writes `ttk-fonts.pack` (TTKFONT2: per-set line
  height, scale, shadow flag and colour, then 95 glyphs with ARGB pixels; FNV-1a
  checked) plus `ttk-fonts.json` provenance. Lower case maps to capitals.
- `src/ttk/ttk_font.cpp/.h`: the renderer for TTKFONT2, replacing
  `duke_font.cpp`. Same `ttk_font_rasterize` API and style numbers, so
  `host_osd.c` (framework) is unchanged.
- `CMakeLists.txt`: generates the font and icon packs from `recomp/disc/` when a
  prepared disc exists, and the digit pack from the tracked Microfont always.
  The research font ZIP/PK3 step and the local pack copies are gone.
- Removed: `build_duke_fonts.py`, `test_duke_fonts.py`, `duke_font_native.cpp`,
  `duke_font.cpp/.h`.

## Evidence

- `ttk-font-test`: all styles, widths and messages render. Messages and the
  console carry the navy shadow; lower case equals upper case; deterministic;
  Vanilla and a corrupt pack are refused.
- `test_ttk_fonts.py` (reproducible, three sets with the chosen line height,
  scale and shadow, all caps), `test_ui_art.py`, `test_ttk_inv_icons.py`; the
  whole Python suite: 114 tests OK (2 skipped). `ttk-inventory-test` and
  `ttk-controls-test` pass.
- The disc-built icon pack is byte-identical to the accepted local pack.
- A private Xvfb Modernized run in level 1 shows the console (`help`) in TTK
  Medium Italic steel with shadow, and a cheat message in TTK Big Italic steel.

## Remaining

- The user confirms the look in play.
- Fresh-clone test after commit: empty folder, README steps, owned disc. Check
  that fonts, icons, digits, frame and crosshair are present.
- Old local Duke 3D copies (`recomp/assets/fonts/Messages`,
  `recomp/assets/ttk-inv-icons.pack`) are no longer read. They are ignored and
  can be deleted locally.
