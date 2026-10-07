# D08A7 - Custom medkit gadget icon

Status: **Done** (user-accepted 2026-10-07: "amaazing work. i accept"). Modernized only; Vanilla unchanged.

The medkit's inventory picture is the user's own 16x16 sprite instead of the
HUD health cross, in the `[` / `]` switcher strip and in the D08A6 selected
gadget box. The health box keeps the cross.

## Art

| File | Use |
| --- | --- |
| `recomp/assets/ui/items/gadget-medkit.png` | Full colour (176 colours), switcher strip |
| `recomp/assets/ui/items/gadget-medkit-15col.png` | 15 colours + transparent (k-means in PSX 15-bit colour, no dither), HUD box |

Both are listed in `items.json` (entry `MEDKIT`, `"gadget": 5`); the mission
item builder skips gadget entries.

## Switcher strip

`tools/local/build_ttk_inv_icons.py` takes item 5 from `--medkit` (default
`gadget-medkit.png`), trimmed to its opaque pixels, instead of the health cross
cell. The cross is still extracted for review. The provenance JSON records the
art file and hash. CMake rebuilds the pack when the PNG changes.

## HUD box

The medkit box and the health box both used record `0x800c44e4` (the cross),
so the medkit needs its own cell, palette and record.

- **Record.** `0x8008b678` reads a 16-byte record: CLUT id (u16), width (u8),
  height (u8), VRAM x (u32), VRAM y (u32), flags (u32, semi-transparency). It
  draws a POLY_FT4 with `GetTPage(0, 0, x, y)` (4bpp), u = (x mod 64) * 4,
  v = y mod 256. The medkit record is the cross record with the new cell and
  CLUT, written on the stack area `gadget_box_draw` already saves and restores.
- **Where in VRAM.** The disc's HUD sheet (`/DATA/FONTS.RAW`, 64 x 256 at VRAM
  960,0) has empty rows 205-222. A live survey compared VRAM x 960-1023,
  y 0-255 against the disc file: from row 223 down, x 960-991 is filled at run
  time with other data (610 texels, the same in every state checked), so the
  first plan (rows 208-223) would have overlapped by one row. Rows 205-222
  stayed empty during the intro movie, title, savestate load, pause, the Select
  screen and levels 1, 2, 3, 5, 7, 9. The cell is at (960, 205), 4 VRAM words
  x 16 rows; the palette row at (1008, 206).
- **Upload.** GPU linked-list DMA is asynchronous in the runtime, so writing GP0
  directly from the status bar hook could split a packet in flight. Instead,
  every time the medkit box is drawn, a 47-word packet goes into the HUD
  ordering-table slot 0 through the original add routine `0x8002bc18`:
  `A0` cell (3 + 32 words), `A0` palette (3 + 8), `01` texture-cache clear.
  It comes from the same packet ring as the HUD sprites (cursor `0x800d67a8`,
  start, end; wrap when the packet would pass the end). The add routine puts a
  packet at the head of its slot, so the load, added after the icon and box,
  is sent before them. The widescreen UI prepass already stops at `A0` nodes.
- **Why every frame.** Nothing has to survive: level loads, savestates made
  before this build or after, and D17 replay workers (which redraw on a copy of
  guest memory) all get the cell from the frame that draws the box. It is 47
  words, only while the medkit is the selected gadget in Modernized.
- **Tables.** `k_medkit_cell` / `k_medkit_clut` in `src/ttk/gadget_hud.inc`:
  palette index 0 transparent, then colours in raster order.
  `tests/local/test_ui_art.py` re-derives both from the PNG.

## Evidence (private Xvfb runs, `recomp/analysis/d08a7-medkit/`, local)

- VRAM survey (`m1`, `m3`): band clean in all states above; the cell and
  palette match the tables as soon as the medkit box is drawn, and stay after
  `[` / `]` moves away (nothing else writes there).
- Captures: GL 16:9 and 4:3 (`widescreen off`), Software 4:3: box shows the
  medkit at the slot, scale and grid of the other icons; health box keeps the
  cross; strip shows the full-colour medkit (`m3-medkit`, `m5gl43`, `m5sw43`).
- 120 fps, first person, 16:9, 150% CPU (`m4`): four consecutive captures with
  the medkit; savestate saved with the medkit selected, level change, reload:
  medkit right away; M use: still the medkit, charge 92.
- Vanilla with all inventory given: no box, band stays empty (no upload).
- `ttk-inventory-test`, Python 121 OK (2 skipped), `check_repo.py` OK.

## Limits

- Not run: death / Continue, an in-play FMV after level start, resize, a
  statistics-screen level transition (debug travel was used), the bonus,
  challenge and boss levels. Software at 16:9 cuts the right HUD with or
  without this change (existing D14 issue).
- The user confirmed the look in play (accepted).
