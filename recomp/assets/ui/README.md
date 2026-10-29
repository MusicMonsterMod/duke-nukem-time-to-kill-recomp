# UI art

Interface art made for this project, plus openly licensed third-party art in
`fonts/` (each with its own licence and `SOURCE.md`).

| Third-party | Licence | Used for |
| --- | --- | --- |
| `fonts/microfont/` 3x5 Microfont by nimaid | CC0 1.0 | Inventory switcher digits and `%`, generated into `ttk-inv-digits.pack` by `tools/local/build_ttk_inv_digits.py` in every build (D24A; replaced the Duke Nukem 3D THREEBYFIVE digits); also the D08A5 mission row's "MISSION" and count, as font sets 9-11 of `ttk-fonts.pack` |

## Original art

Interface art by **MusicMonsterMod**, made for this project. None of it is taken or derived from Duke
Nukem 3D or from the Time to Kill disc, so it is tracked here and distributed
with the repository under its licence (see `LICENSE`).

| File | Size | Used for |
| --- | --- | --- |
| `item-frame.png` | 25x23 RGBA | Modernized inventory switcher selection frame (D24A). It replaces the Duke Nukem 3D tile 20 frame, drop-in, through `tools/local/build_ttk_inv_icons.py` (pack entry kind 1) |
| `items/` (21 PNGs + `items.json`) | 16x16 RGBA | Mission item, key, keycard and steroids icons (D24A), the TTK-style open art for the mission item inventory and the steroids HUD icon. The D08A5 mission row uses them through `ttk-mission-items.pack` (`tools/local/build_ttk_mission_items.py`, every build), with the frame below in grey, tile0020 orange and Console steel. `items.json` maps each of the 15 mission items, steroids and the medkit to its file, the game's object types and levels; the seven keycards share one drawing, and Torn Paper shares Scrap of Paper's. The medkit gadget icon (D08A7): `gadget-medkit.png` in the inventory switcher (through `tools/local/build_ttk_inv_icons.py`) and its 15-colour reduction `gadget-medkit-15col.png` in the HUD box, compiled into `src/ttk/gadget_hud.inc` (`tests/local/test_ui_art.py` keeps them equal). Portable steroids (D08A4): `hud-steroids.png` in the switcher and its 15-colour reduction `hud-steroids-15col.png` (`tools/local/reduce_icon_15col.py`, deterministic; the test re-derives it) in the HUD box |
| `crosshair.png` | 9x9, 2 colours (lime `#80ff00` and transparent) | Modernized view crosshair. Its pixels are compiled into `src/ttk/weapon_aim.cpp` (`k_crosshair`, ARGB); `tests/local/test_ui_art.py` keeps the two equal. It replaced the EDuke32 CROSSHAIR tile 2523 (yellow) |

### Complete original-design inventory

The local TTK Font Picker's original-art, mission-item, gadget, power-up,
used-tick and challenge-item sections document **17 distinct designs**:

- Inventory selection frame and crosshair (2).
- Mission items and keys (8): shared keycard, skeleton key, red/blue/green
  energy crystals, Scrap of Paper (also Torn Paper), Old Note and Family Jewel.
- Medkit and steroids (2).
- Invincibility, invisibility and Double Duke power-up icons (3).
- Challenge-item icon and used-item green tick (2).

The `items/` directory contains 15 of these designs as primary PNGs plus six
15-colour HUD reductions, for 21 PNGs. The frame and crosshair live one level
above it. Palette reductions and retained full-size variants in `items/16px/`
are variants, not additional designs. `items.json` maps the shared drawings to
21 named mission items, gadgets and power-ups; the used tick is a separate
status overlay. The older mission/gadget description above does not exhaust
the collection: the three power-up icons and challenge-item icon also have
15-colour HUD reductions.

Disc-derived art (fonts, HUD icons, buttons) is never stored here. The build
extracts it from the player's own disc.
