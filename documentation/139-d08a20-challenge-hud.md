# D08A20 - Challenge item on the HUD (option A)

Accepted 2026-10-09. User: "fully accepted. ... this is it, excellent!!!"

## Behaviour

- Object type 633 sets `player+0x85e` and queues message 204 ("SURPRISE").
- Modernized only: while that flag is set, the top-right HUD shows the challenge
  icon in the native gadget box's **left cell only** (width 19; the divider is
  the right edge). No digits, no tick, no switcher entry.
- Right edge matches the ammo box; the ammo box's bottom inset is mirrored at
  the top (including widescreen). Vanilla and split-screen leave this path.
- Save/load and level reset follow the original flag. Level travel clears it.

## Art and layout choice

- Active art: `recomp/assets/ui/items/hud-challenge.png` (14x14 in a 16x16
  canvas) and `hud-challenge-15col.png`.
- Full 16x16 comparison art: `recomp/assets/ui/items/16px/`.
- Playtest first tried option D (borderless 16x16). Accepted is **option A**
  from the font picker mockups (section 15).

## Implementation

- `recomp/src/ttk/gadget_hud.inc`: `challenge_hud`, `k_challenge_*` from the
  14x14 15-colour PNG, `hud_box_draw(..., cell_only=true)`.
- Tests: `recomp/tests/local/test_ui_art.py` (cell equals PNG).
- Local font picker (`recomp/analysis/d24a-fonts/`, not tracked) marks A accepted.

## Evidence

- Executable `a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95`.
- Earlier option-D offscreen runs (levels 0 and 5, savestate round trips) plus
  user playtest of option A. Natural campaign routes for every hidden challenge
  item were not exhaustively walked.
