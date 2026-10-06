# D08A3 - Original TTK inventory icons for the switcher

Status: **Done** (user-accepted 2026-10-01). Modernized switcher only; Vanilla and
the original inventory screen are unchanged.

## Where the game's own item art is

- The original inventory screen (Select) is a text list with no icons.
- The HUD shows an item icon in a metal frame at the bottom right while the
  jetpack, Bio Mask or goggles is on (checked in Vanilla on a private copy of
  UI slot 1 by switching each item on from the inventory screen).
- Those icons are 4bpp cells of a HUD sheet that lives in **`/DATA/FONTS.RAW`**
  (LBA 23943, 32768 bytes, sha256
  `54e16c1af6b63edcc5a7d92eefe45220b9921ebd0a41fa8047c92ebb5f18e1be`): a raw
  64 x 256 16-bit VRAM image uploaded to VRAM (960, 0), found by matching the
  in-game VRAM texels against the disc's user data. Its 16-colour palettes
  (CLUTs) are in the same file, column x = 1008.
- The executable's HUD sprite table has 16-byte records (CLUT id, width,
  height, VRAM x, VRAM y). The draw routine for each record shows what it
  stands for:

| Item | Record | Cell (VRAM) | CLUT (VRAM) | Drawn while |
| --- | --- | --- | --- | --- |
| Jetpack | `0x800c44b4` | (1008, 137) 16x16 | (1008, 229) | `player+0x358` jetpack |
| Bio Mask | `0x800c44c4` | (1020, 173) 16x16 | (1008, 235) | `+0x360` branch (confirmed on screen) |
| Goggles | `0x800c4504` | (1008, 121) 16x16 | (1008, 228) | `+0x360` branch (confirmed on screen) |
| Armor | `0x800c44f4` | (1016, 173) 16x16 | (1008, 236) | armor value `+0x234` (code `0x8008be00` draws `+0x234 / 100`) |
| Health cross | `0x800c44e4` | (1020, 189) 16x16 | (1008, 231) | health `+0x32` |

  The game has **no separate medkit icon**; the health cross is its own art for
  health and is used for the medkit. **Correction (D24A, 2026-10-06):** the icon
  first listed here as steroids is the armor icon (the user identified it, and its
  routine at `0x8008be00` draws the armor value `+0x234 / 100`). The steroid
  pickup (`0x800827e8`) sets `+0x364` value 2, and the HUD has no icon for it,
  which is why `dnhyper` showed none. The armor icon is extracted but not
  added to the switcher.

## Builder

`recomp/tools/local/build_ttk_inv_icons.py` reads `/DATA/FONTS.RAW` from the
prepared disc image (pinned hash) and the records from `SLUS_005.83`, renders
each cell with its CLUT (PSX colour 0 transparent), clears the cell's opaque
near-black background where it touches the cell border, trims, and writes
`recomp/assets/ttk-inv-icons.pack` (TTKICO2: items 1, 2, 3, 5 plus the
unchanged selection frame copied from the previous pack) and
`ttk-inv-icons.provenance.json`. Review PNGs (native cells and keyed icons) go to
the ignored `recomp/analysis/d08a3-ttk-icons/png`. Everything it writes is
retail-derived and local only. The previous pack is kept at
`recomp/analysis/d08a3-ttk-icons/ttk-inv-icons.before.pack`.

## Evidence

- Pack `477b45e9...6bdf`, binary `7ca63455...3328` (the pack is copied beside
  it by the build).
- Live Modernized capture on a private copy of UI slot 1 (`strip-win-0.png`):
  `]` shows the health cross, jetpack (selected, frame unchanged), Bio Mask and
  goggles with the green % unchanged.
- `ttk-inventory-test` passes with the new pack and with a missing pack (the
  strip falls back to the % without icons, as before).
- New `test_ttk_inv_icons.py`: the builder is byte-reproducible, writes the
  four switcher icons (8..16 px) and keeps the old frame; Python 88 OK.

## Limits

- Icons are 13..16 px originals drawn at 2x in the 36 px icon box, like the
  art they replace; the user confirms the look.
- Medkit uses the health cross (no original medkit icon exists).
- The selection frame was the Duke Nukem 3D tile 20 frame (`research/inv/tile0020.png`).
  **D24A (2026-10-06):** it is now the project's own `recomp/assets/ui/item-frame.png`
  (tracked, 25x23 RGBA, same size), read by the builder (`--frame`) instead of copying
  the old pack's frame.
