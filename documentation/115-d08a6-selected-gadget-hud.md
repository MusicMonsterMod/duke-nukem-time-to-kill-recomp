# D08A6 - Selected gadget shown on the HUD

Status: **Accepted** (2026-10-07, "i fully accept!"). Modernized only; Vanilla is
unchanged. Built to the approved design A (board entry D08A6; mockups in section
9 of the local D24A page).

## Player use

The gadget chosen with `[` / `]` (the one Enter / U uses) always has a box at
the original item slot, over the ammo box in the bottom right corner:

- the HUD's own 46x16 box, the gadget's HUD icon (health cross for the medkit)
  and the red HUD digits showing its charge (charge x 100 / capacity, the
  number the original box shows);
- lit digits while the gadget is on, dim digits (38%) while it is selected but
  off; the medkit is always lit;
- the box follows the selection at once (`[` / `]`, M / J / B / N, depletion,
  the original Select menu, savestate load, level change);
- no box when Duke owns no gadget.

If another gadget is on, its original box stays visible:

- a jetpack that is on but not selected moves up one box row (20 rows at 1x,
  the original's own step: box height 16 + 4);
- Bio Mask / goggles that are on but not selected keep their original place
  left of the ammo box.

## How the original status bar draws gadget boxes

Status bar `0x8008ba30` (called from `0x80026588`, player in a0) draws each
element from the layout `0x800dd778` (8 bytes per element, one (x,y) pair per
player slot `player+0x233`, relative to the screen centre) and a per-element
state at `0x800dd7b8` (bit 0 visible, then a slide-in offset). Live values in
level 1 (single player, before the D14 shift):

| Element | Offset | (x,y) | Drawn while | Icon record |
| --- | --- | --- | --- | --- |
| health | 0 | (-246,89) | always | `0x800c44e4` |
| ammo | 8 | (169,89) | always | per weapon |
| armour / steroids | 16 | (-165,89) | `+0x364` bit 1 or armour `+0x234` > 0 | `0x800c44f4` |
| air | 24 | (-246,69) | `+0x22c` == 5 or `+0x236` < 12000 | `0x800c44d4` |
| jetpack | 32 | (169,69) | `+0x358` bit 1 | `0x800c44b4` |
| Bio Mask / goggles | 40 | (82,89) | `+0x35c` or `+0x360` bit 1 | `0x800c44c4` / `0x800c4504` |
| counter | 48 | (169,-105) | levels 21-26 or multiplayer | `0x800c4554` |

Each gadget box is three calls:

1. `0x8008b98c(buffer, charge * 100 / capacity)` (integer to text; capacity is
   `0x800c2712 + 4 * item`);
2. `0x8008b454(buffer, x + [0x800d22a4] / 16, y + 2, 0x310)`: text in HUD font
   3 (the red digits); each glyph is one 0x28-byte POLY_FT4 packet (code 0x2c,
   colour 0x80 0x80 0x80) written at the packet cursor `0x800d67a8` by
   `0x8008b210`, wrapping to the buffer start (`+4`) before its end (`+8`);
3. `0x8008b678(icon, x + 1, y, 0, 100)` then `0x8008b678(0x800c44a4, x, y, 0,
   100)`: the icon and the 46x16 box (fifth argument on the stack: scale %).

When the jetpack appears, its slide offset starts at `ammo_y - jet_y` and
moves to 0 (it rises out of the ammo box). In single player Bio Mask / goggles
slide horizontally from the ammo box (`ammo_x - x`). The Bio Mask and goggles
are never on together: switching one on switches the other off (checked
live), so the original shows at most two gadget boxes. The medkit has no HUD
element.

## Implementation

`recomp/src/ttk/gadget_hud.inc`, called from the status bar entry hook after
the D14 widescreen shift (`modern_controls.cpp`, `0x8008ba30`, Modernized,
single player, `ra == 0x80026590`).

- The selected gadget is read from guest memory, the original menu's
  remembered ID `0x800c3f94`, which `shortcuts.inc` keeps equal to the host
  selection (`selected_item`). Host state must not be used here: above 60 fps
  the D17 workers replay the status bar on a copy of guest memory, where host
  variables are stale. It is shown while owned with charge or on.
- `[` / `]` move the selection in every live gameplay state (flight, swimming,
  firing, weapon animations); Enter / U keep their use restrictions.
- When the selected jetpack switches on, its slide out of the ammo box is
  skipped (state `0x800dd7b8 + 34` set to 0), so the lit box replaces the dim
  one in place.
- **Selected and on:** the original draws it. The jetpack already sits in the
  slot; for the Bio Mask / goggles, element 5's layout is moved to the slot
  for this draw.
- **Selected and off, or the medkit:** the box is drawn with the original's
  own three calls (on a saved stack and scratchpad, like `original_call`), so
  it uses the game's sprites, CLUTs, OT and widescreen position. For dim
  digits, the glyph packets the text call wrote get colour 0x31 instead of
  0x80: PSX texture modulation at 38%, matching the mockup.
- **Another gadget on:** element 4 (jetpack) moves up by the box record height
  + 4 when it is not the selection.
- Layout changes use the widescreen save/restore window. `widescreen.inc`
  now saves the whole layout (x and y) whenever the single-player status bar
  draws, not only when widescreen is live. `0x8001fc44` and `0x800b4d9c`
  restore it, so saves, menus and the original never see the moved values.
- No framework, codegen or profile change; savestates are unaffected.

## Evidence

Private Xvfb Modernized runs on a copy of the test cards
(`recomp/analysis/d08a6-hud/`, local: `t1.py` to `t4.py`, screenshots):

- Level 1 after `dninventory`: jetpack, Bio Mask and goggles selected show
  their icon with dim "100"; medkit shows the health cross with lit "100";
  `]` four times wraps; `[` goes back.
- N: goggles on and selected, lit in the slot (the original box, moved). J:
  jetpack on and selected in the slot, goggles back at their own place (82,89)
  with their own slide. Enter switches the jetpack off: dim box.
- Jetpack on, `]` to medkit: jetpack box one row up, medkit lit in the slot.
  Bio Mask selected and switched on with Enter: lit in the slot, jetpack above.
- Savestate saved (Bio Mask selected), selection changed to medkit, savestate
  loaded: the box returns to the Bio Mask (the original menu ID is restored
  with the state and the host selection follows it). `level 2`: only the
  jetpack is owned; dim jetpack box.
- GL 4:3 (`widescreen off`) and 16:9: boxes in the right corner with the ammo
  box. Software 4:3: correct.
- Vanilla, jetpack and medkit given by RAM write: no extra box.
- `ttk-inventory-test`, `ttk-input-test`, Python 120 OK (2 skipped).

## Limits

- Not run here: death / Continue, depletion of a gadget to zero in play,
  jetpack transitions in flight, window resize, level-ending scripted HUD
  states. The selection itself already handled these (D08A1); the box only
  follows it.
- Software renderer at 16:9 cuts off the right edge of the whole HUD (the
  original ammo box too). The same build without D08A6 does the same, so it
  is an existing D14 / Software issue, not this job.
- The medkit box reads "100" with a health cross, like the health box on the
  left; the spec left the medkit number open.
- No flash when the selection changes or Enter uses it (left open in the spec).
- Bio Mask / goggles that are on but not selected stay at their original
  place left of ammo instead of stacking one row higher (the spec's mockup
  assumed every gadget box stacks; the original does not).

## Playtest fix (2026-10-07): box followed a stale selection at 120 fps

User: "medkit was selected in the new box, but i selected jetpack from the
inventory picker, and now both medkit and jetpack appear in the bottom right";
"just by hovering over the item, you should be making it hot. enter is what
actually uses it."

- **Cause (reproduced):** with the user's profile (first person, 120 fps) the
  in-between images are redrawn by D17 worker processes that replay the frame
  on a copy of guest memory, status bar included. The box read the host
  selection, which is stale in a worker (the medkit), so replayed images
  showed the old selection while live images showed the new one. 60 fps has no
  replays, which is why the first tests passed. Intermittent: two of four runs.
- **Fix:** the box reads the selection from guest memory, the original menu's
  remembered ID `0x800c3f94` (already written by `[` / `]`); `select_weapon`
  now also puts that ID back on the selection when the original menu left it
  on a key or an unowned item (unless that item is mid-activation).
- **Hover everywhere:** `[` / `]` now move the selection in every live
  gameplay state (jetpack flight, swimming, firing, weapon animations);
  before, they were ignored there. Only Enter / U keep the use restrictions.
- **In place:** when the selected jetpack switches on, its original slide up
  out of the ammo box is skipped (slide offset set to its resting 0), so the
  lit box replaces the dim one.
- **Evidence:** private Xvfb runs from a copy of the user's savestate slot 3
  with the user's settings (first person, 120 fps, 150% CPU, 16:9): `dnstuff`,
  M, `]`, Enter, three runs, four captures per step: the box is the medkit,
  then the dim jetpack, then the lit jetpack in every capture; in flight `[`
  moves to the medkit (jetpack box one row up, fuel counting down). 60 fps
  third person the same. Native tests and Python 120 OK.
