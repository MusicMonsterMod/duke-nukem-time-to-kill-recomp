# Session wrap — inventory strip polish + EDuke crosshair

2026-09-28 (evening). Player accepted the locked frame and transparent EDuke
crosshair. Session complete; safe to clear.

## Build

SHA256 `f09b657e31ded01e987d4b39695d469c452bdda621f6f6aa68dd01ebf8edec97`.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

Assets beside the executable: `ttk-inv-icons.pack`, `ttk-inv-digits.pack`,
`ttk-fonts.pack`. No commits this session.

## What landed

### Inventory switcher (centered strip)

- **[ / ]** only opens the temporary strip. Direct keys **M / J / B / N** select
  and use **without** showing the switcher.
- **Enter** (and **U**) uses the currently selected gadget while captured —
  never opens pause. **Escape** alone pauses.
- **M** still quotes centered `MEDKIT N%` when owned; silent if missing.
- Scuba/air stays automatic in-world (no dedicated scuba key required).
- Icons from `research/inv` (medkit/jetpack/goggles/steroids tiles). Selection
  uses EDuke **ARROW** `tile0020.png` stretched over a fixed cell.
- Charge digits/% from **`research/inv/font/03. Green (Palette 22)`**
  (`TILES012_15..24` + `TILES012_4` for `%`) at 2× — not the older my-export set.
- Layout decision (from EDuke screenshot-1/2 in `research/inv/`): keep the
  full strip (all charges visible) rather than pure EDuke icon-only picker, but
  keep `%` on one baseline and draw it **after** the frame so the border never
  clips the numbers.

### Locked selector frame (player-tuned)

| Param | Value |
| --- | --- |
| Cell width | **50** |
| Cell height | **60** |
| Icons/% top | `cell_top = 4` (content stays put) |
| Frame-only nudge | `frame_dy = -6` |

Earlier nudges that moved the whole cell were wrong; only the frame moves via
`frame_dy`.

### Crosshair

- Procedural white cross replaced with exact EDuke **CROSSHAIR** tile
  `research/inv/tile2523.png` (9×9 yellow open center), drawn at 2× × UI scale.
- First GL present drew it without alpha blend → black square under the arms.
  Fixed by teaching `host_osd_text_has_alpha` to recognize crosshair pixels
  (`ttk_aim_crosshair_pixels`), same pattern as inventory strip alpha.

## Controls summary (Modernized, captured)

| Input | Behavior |
| --- | --- |
| Escape | Pause / release capture |
| Enter / U | Use selected inventory item |
| [ / ] | Cycle gadgets + show strip |
| M / J / B / N | Use that gadget, no strip |
| Scroll Lock | Holster (prior job) |
| Caps Lock | RUN MODE quotes (prior job) |
| Backtick | `fps` console (prior job) |

## Jobs touched

- **D08A1 / D08A2 (revised):** strip + icons + green % + Enter-as-use accepted
  for this session’s polish. Always-on bottom-left host gadget stays rejected.
- Prior same-day queue (D27/D28/D26) unchanged; Shift-run **clunk** silence still
  optional/open if it returns via SFX path.

## Preserve

Vanilla, aiming, furniture, ladders, silent cheats, assets/saves, unrelated dirty
work. Never hand-edit generated C.

## Next session (optional)

Nothing blocking from this wrap. Possible later: Shift clunk via SFX if it
returns; ON/OFF text beside toggles like EDuke status bar; commits only if asked.
