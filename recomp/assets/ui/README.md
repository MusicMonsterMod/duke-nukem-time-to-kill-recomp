# UI art

Interface art made for this project, plus openly licensed third-party art in
`fonts/` (each with its own licence and `SOURCE.md`).

| Third-party | Licence | Used for |
| --- | --- | --- |
| `fonts/microfont/` 3x5 Microfont by nimaid | CC0 1.0 | Inventory switcher digits and `%`, generated into `ttk-inv-digits.pack` by `tools/local/build_ttk_inv_digits.py` in every build (D24A; replaced the Duke Nukem 3D THREEBYFIVE digits) |

## Original art

Interface art made for this project. None of it is taken or derived from Duke
Nukem 3D or from the Time to Kill disc, so it is tracked here and distributed
with the repository under its licence (see `LICENSE`).

| File | Size | Used for |
| --- | --- | --- |
| `item-frame.png` | 25x23 RGBA | Modernized inventory switcher selection frame (D24A). It replaces the Duke Nukem 3D tile 20 frame, drop-in, through `tools/local/build_ttk_inv_icons.py` (pack entry kind 1) |
| `crosshair.png` | 9x9, 2 colours (lime `#80ff00` and transparent) | Modernized view crosshair. Its pixels are compiled into `src/ttk/weapon_aim.cpp` (`k_crosshair`, ARGB); `tests/local/test_ui_art.py` keeps the two equal. It replaced the EDuke32 CROSSHAIR tile 2523 (yellow) |

Disc-derived art (fonts, HUD icons, buttons) is never stored here. The build
extracts it from the player's own disc.
