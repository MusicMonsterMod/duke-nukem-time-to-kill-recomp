# D08A17 - Power-up countdowns

Status: **Done** (user-accepted 2026-10-08: "confirmed it's all working as intended"). User request: give invincibility,
invisibility and Double Duke a visible countdown like the steroids box
(D08A8), Quake-style. Modernized only; Vanilla unchanged.

User decisions when selected (2026-10-08): the boxes show **percent**, like
steroids; the 5 s invincibility the original gives after Continue **shows** a
box; **no** expiry warning (no blink, no sound).

## The original

| Piece | Address | Meaning |
| --- | --- | --- |
| Pickup, type 1047 invincibility | dispatcher `0x80081a48` case `0x80082830` | `+0x8a0` = 6000 (`0x80082848`), message `0x11d` |
| Pickup, type 1045 invisibility | case `0x80082864` | `+0x89c` = 6000 (`0x8008287c`), message `0x11e` |
| Pickup, type 1046 Double Duke | case `0x80082898` | `+0x89e` = 6000 (`0x800828b8`), message `0x11f` |
| Shared tail | `0x800828d0` | pickup sound `0x100f` at Duke (as steroids) |
| Drain | Duke's update `0x800412a4`, `0x80042208..0x80042294` | each timer -= frame step `0x800d21fc` while > 0; zeroed when it reaches 0 or below |
| Reset | `0x8003f948`, `0x8003fd00..0x8003fd08` | zeroes all three (death, level start) |
| Continue protection | `0x80042de8` (`0x80042f58`) | `+0x8a0` = 1500 (5 s) after the reset |
| Effects | `0x80034e48..` (Duke's draw: translucent while invisible, tints), `0x800a4130` (Double Duke doubles Duke's damage), `0x800a4510` (invincibility in Duke's damage handler), `0x800a5cbc` (invisibility for enemy AI) | unchanged |

6000 units at the drain's 300 a second is 20 s. A second coin of the same kind
writes 6000 again (no stacking). The original has no countdown on screen; the
only signs are Duke's look and the pickup message. `dnkroz` is the separate
global `0x800c3cc6` and does not touch `+0x8a0`.

Survey (`recomp/analysis/d08a17-powerups/t0.py`, local; UI savestate slot 3,
level 0, spawned coins through the original pickup code):

| Check | Result |
| --- | --- |
| Pickup | the timer reads 5610-5625 once the walk ends (6000 at pickup) and falls about 5 a frame at 60 Hz |
| `dnkroz` on / off | `+0x8a0` keeps draining, no change |
| Death with invisibility and Double Duke running | all three 0 at death |
| Continue | invincibility 1200 at the first read, the others 0 |
| Savestate save and reload | timers restored with the state |
| `level 1` travel | all three 0 |

## Implementation

`recomp/src/ttk/gadget_hud.inc` (status bar entry `0x8008ba30`, Modernized single
player, the existing hook):

- `hud_box_draw()` is the D08A6/D08A8 box draw with the icon (disc record or the
  project's art) and the number passed in; `gadget_box_draw()` keeps the item
  form for gadgets.
- `powerups[]`: each timer offset with its art. Every running timer (> 0) draws
  a lit box with `timer * 100 / 6000`, the steroids box's rule, each a box row
  above the previous rows: the selected gadget in the slot, an unselected
  jetpack that is on, running steroids, then invincibility, invisibility and
  Double Duke. The full value 6000 is the pickup cases' constant (there is no
  capacity table entry for coins).
- Art: the user's D08A16 icons as 4bpp cells, uploaded with each box like the
  medkit and steroids (note 116): cells at VRAM (968,205), (972,205), (976,205)
  and palettes at (1008,208), (1008,209), (1008,210), in the band note 116
  surveyed as empty (rows 205-222).
- `items.json` lists the three icons as HUD-only art (`"hud": true`, coin type);
  the mission-item pack skips them.
- Debug: `ttk_input` `controls.powerups` (`draws`, and the three timers).
- No new hook, guard, generated code or game-state write; the timers are only
  read.

## Evidence (executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`)

Private Xvfb runs on a copy of the D08A14 cards, port 9317,
`recomp/analysis/d08a17-powerups/` (local): `t1.py` (sequence), `t2.py` (window
capture), `t3.py` (Vanilla).

| Check | Result |
| --- | --- |
| One coin | a lit invincibility box in the slot over ammo (nothing selected), counting down (93, then 83 two seconds later) |
| Three coins | three boxes stacked: invincibility, invisibility, Double Duke |
| Full stack (Bio Mask selected, jetpack on, steroids running, three coins) | seven boxes in the right column, top one at about y 95 of 240, nothing overlaps |
| Near the end | `+0x8a0` written to 200: box at 2-3, then it goes when the timer hits 0; the others move down a row |
| Savestate save and reload | boxes follow the reloaded timers |
| Death | all timers 0, no boxes |
| Continue | invincibility box at 19 (1170 at the first read after the menu), gone after about 5 s |
| 60 and 120 fps | same timers and boxes (the drain follows game time) |
| GL 4:3 and 16:9, Software 4:3 | same layout |
| Software 16:9 | right column cut off at the window edge, ammo box too: existing, recorded with D08A6 |
| Vanilla, timers written to 6000 | timers drain, `draws` 0, no boxes |
| Suites | `ttk-controls-test` (`d08-camera-final` fixture, LEVEL01, all levels), `ttk-input-test`, `ttk-inventory-test`, Python 132 OK (2 skipped; `test_ui_art.py` checks the three cell tables and that the 15-colour reductions are reproducible), `level_overlay_guards.py --check`, `check_repo.py` |

## Limits

- Natural (non-spawned) coins were not picked up; spawned coins use the same
  type and pickup code.
- Two-player games and boss/challenge levels were not exercised (player one
  only).
- No profile option: the boxes are part of Modernized.
- The number floors like the steroids box, so a box shows 0 for its last
  frames (under 60 units, about 0.2 s).
