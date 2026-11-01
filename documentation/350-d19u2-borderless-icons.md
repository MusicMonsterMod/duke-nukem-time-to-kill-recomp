# D19U2 - Borderless HUD icons without black squares

Status: Done, user-accepted 2026-11-01 ("lock it in close it out!"). Dependencies: D19U1 (note 314).

## Cause

Every native HUD icon cell in the disc's HUD sheet (`/DATA/FONTS.RAW` at VRAM
960,0) fills its background with an opaque black palette entry, `0x8000`
(black with the semi-transparency bit, which the GPU draws). In Full the box
frame (`0x800c44a4`) surrounds it, so it reads as the box interior; D19U1
removes only the frame, leaving the square. The square is part of the icon
cell, not a separate fill.

Inventory of every Borderless element (records are 16 bytes: CLUT id, width,
height, VRAM x, VRAM y):

| Element | Record | Cell | Background | D19U2 |
| --- | --- | --- | --- | --- |
| Health cross | `0x800c44e4` | (1020,189) 16x16 | index 15 = `0x8000` | keyed |
| Armour | `0x800c44f4` | (1016,173) 16x16 | index 15 = `0x8000`, 1 enclosed black texel | keyed, enclosed kept |
| Jetpack | `0x800c44b4` | (1008,137) 16x16 | index 15 = `0x8000` | keyed |
| Bio Mask | `0x800c44c4` | (1020,173) 16x16 | index 15 = `0x8000`, 1 enclosed | keyed, enclosed kept |
| Night-vision goggles | `0x800c4504` | (1008,121) 16x16 | indices 14/15 = `0x8000` | keyed |
| Counter (levels 21-26, multiplayer) | `0x800c4554` | (1008,153) 16x13 | index 15 = `0x8000`, 3 enclosed | keyed, enclosed kept (not seen in play) |
| Ammo strip | `0x800c4514`.. | (1008,169..) 16x13 | index 15 = `0x0000` | already transparent |
| Air | `0x800c44d4` | (1016,189) 16x16 | none: a full water tile | unchanged (the tile is the icon) |
| Medkit, steroids, power-ups, challenge | project art (D08A7/A4/A17/A20) | 960-980,205 | index 0 = `0x0000` | already transparent |
| Digits | HUD font 3 | - | dim segments are font art | unchanged |

The selected-gadget boxes drawn by `gadget_hud.inc` use the same disc records
(`gadget_icon`), so they follow automatically.

## Implementation

`recomp/src/ttk/gadget_hud.inc` (`keyed_icons_begin`/`keyed_icons_restore`),
called from the status bar entry `0x8008ba30` (after the D14 shift, before
`gadget_hud`) and from `widescreen_hud_restore` on every exit.

- Modernized, single-player HUD scope, Borderless only, authenticated code, and
  a guest stack inside the expected range; otherwise nothing changes.
- For each record still holding its exact original words, the cell and palette
  are read from live VRAM (`gpu_get_vram`, the CPU-authoritative copy). Black
  palette entries (`0x0000` or `0x8000`) connected to the cell edge are flood
  filled to the palette's own `0x0000` entry: the D08A3 switcher rule
  (`build_ttk_inv_icons.py`). Black texels enclosed by the icon stay. A palette
  without a `0x0000` entry is left alone.
- The keyed cells (same palette, same size) are written by one GP0 A0 packet
  in the HUD packet ring and linked into ordering-table slot 1 with the
  original add `0x8002bc18`. Armour, jetpack, goggles and counter go to the
  empty band right of D08A7's cells (x 988-1007, row 205). The cross and Bio
  Mask keep their column x 1020 (see below), in the sheet's blank stretches
  rows 54-69 and 100-115 (empty on the disc and in play in slots 1, 7, 10). The GPU walks slot 1 just before the status bar's
  slot 0, so the load always precedes the icons that sample it.
- Only after the load is queued do the records' VRAM x/y point at the copies;
  the restore puts back exactly those still pointing at a copy. Nothing
  retail-derived is stored in the source; nothing survives in saves.
- Every frame reloads the cells, so savestates, level loads and high-refresh
  replay workers (which replay the captured packets) need no host state.
- `ttk_input` reports `powerups.keyed_icon_loads`.

No framework edit, new hook, generated-C edit or asset change. Full, Hidden,
split screen and Vanilla take no new path.

## Icon size (user report, same day)

User: the cross seemed about a pixel smaller in Borderless. Measured: 28 vs
30 screen pixels wide at scale 2 (height equal). The sprite routine
`0x8008b678` sets the right edge u = (x % 64) * 4 + width and subtracts one
when it reaches 256. The cross and Bio Mask cells sit at x 1020 (u 240), so the
original draws 15 texel columns stretched over the 16-wide quad. The first
copies at x 984/996 drew all 16 at true size. Copies at x 1020 reproduce the
original: the cross's red pixels now match Full exactly and the Bio Mask's
size matches. The pause menu (native fonts from the same sheet) is identical
after Borderless has run (`pfull/`, `pborderless/`).

## Verification

Evidence: `recomp/analysis/d19u2/` (private fixture copies, offscreen, 1280
16:9, player files hash-checked by the harness).

- Before: `base-s10.png` (current build before the change, Borderless, slot 10):
  cross, armour and jetpack on black squares.
- After, 60 Hz: `key-s10.png`, `key/sheet.png`: cross, armour, jetpack, Bio
  Mask (on), goggles (on, lit and unselected-left-of-ammo) all without squares;
  records read back original between frames.
- After, 120 Hz (replay workers live): `r120/`, slots 10 and 7, selected-off
  gadget boxes. `b120`: 12 consecutive presents while turning, 0% near-black
  around the cross (Full: 48%, its box). Replay worker failures were 1 in both
  Full and Borderless on the same load route: route-related, not this change.
- Full and Hidden (`full/`, `hidden/`): original boxes / nothing, 0 keyed loads.
- Vanilla with the Borderless variable forced (`van/`): original 4:3 boxes,
  0 keyed loads.
- `ttk-controls-test` (LEVEL00, LEVEL01, all levels): exit 0, including the new
  D19U2 group (edge black keyed, enclosed black kept, slot-1 packet layout,
  restore on both exits, wrong stack, Full, Vanilla, changed record, palette
  without a transparent entry). The target needed two missing D17AT host-vertex
  stubs to link. Python `test_ui_art`, `test_ttk_inv_icons`,
  `test_ttk_hook_contract`, `test_player_profiles`, `test_guard_writer_audit` OK.

## Limits

Not seen in play: the counter icon (levels 21-26), the air icon under water,
21:9 and the player's 180 Hz display. The digits keep their dim LCD segments
(font art, not a square). User visual review at the player's window size is
the remaining acceptance.
