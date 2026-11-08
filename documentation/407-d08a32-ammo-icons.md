# D08A32 - A distinct ammo icon for every weapon

Status: **Done** (user-accepted 2026-11-07, revision 2: "that is fixed"). Modernized only; Vanilla unchanged.
Executable `2a6f59a4426c42f5436b73333332967783cacfab2611372d2bdfd69ac6526322`.

The bottom-right ammo box shows each weapon's own icon: the user-approved
round 3 set (original 14x14 pieces in Time to Kill's style inside a 16x16
canvas). The Gatling gun keeps the game's stepping cartridge belt (user
decision). Four icons animate as designed on the review page.

## The original ammo box

The status bar `0x8008ba30` draws element 1 (layout `0x800dd778 + 8`):

- weapon slot: player `+0x3ba`; ammo `+0x2c6 + 4 * slot`, or `+0x336`,
  `+0x33a`, `+0x332` for an upgraded Gatling, RPG, flamethrower (`+0x2c4 +
  4 * slot` bit 3) (`0x8008ba50..0x8008baf0`);
- digits through `0x8008b454` (`0x8008bb54`);
- icon `0x8008b678` with record `0x800c4514 + 16 * (3 - ammo % 4)` (four
  16x13 belt records, CLUT `0x3b7f`, VRAM (1008, 169..172)) at (x + 2, y + 2),
  return `0x8008bb98`;
- the 46x16 box `0x800c44a4` at (x, y).

The sprite routine `0x8008b678` has no entry hook in the generated code;
adding one would change generated code (savestate hashes), so the icon is
not swapped at that call.

## Implementation

`recomp/src/ttk/ammo_hud.inc`, called from the status bar hook after
`gadget_hud` inside the D14 / D08A6 layout window:

- the chosen 16x16 cell is drawn with `hud_box_draw` (bare), the gadget
  icons' own path: the original sprite call at (x + 1, y) with an own-art
  record, its cell and palette loaded in front of it through OT slot 0;
- the four belt records get width 0 for the status bar's own call (a
  degenerate quad, as Borderless does to the box record) and are restored by
  `widescreen_hud_restore` (only records still as left);
- one cell (984, 205) and one palette row (1008, 212) in the HUD sheet's empty
  band (D08A7), uploaded every drawn frame, so level loads, savestates and
  redraw workers need nothing kept;
- live frames choose the cell; D17 redraws (workers with stale host state)
  take it from `RenderState.ammo_view`.

Art: `recomp/assets/ui/ammo/` (full colour and `-15col` per frame,
`ammo.json` manifest, README). `recomp/tools/local/build_ammo_icons.py`
writes `recomp/src/ttk/ammo_icons.h` (55 unique cells for 59 frames);
`tests/local/test_ui_art.py` checks the header against the PNGs (`--check`)
and that every weapon 1-14 except the Gatling has frames.

## Animation (game clock `0x800d21fc`, 300 a second)

Shots are taken from the count the box shows (Desert Eagle: the clip).

| Weapon | Behaviour |
| --- | --- |
| Crossbow | Rest by bolts (3+ frame 0, two 4, one 10, none 13). A shot plays 1-2-3 (from 4+), 1-2-6 (from 3), 7-8-9 (from 2), 11-12 (from 1) at 60 ms steps, rest at 200 ms. |
| Desert Eagle | Clip = player `+0x231`, the high byte of `+0x230` (5632 = 22 rounds; live probe: it reaches 0 at the clip's last shot and is refilled about 1.06 s later, at the end of the game's reload). Rest 0-4 by rounds; a shot from a clip of n plays 5 + 3 * (min(n, 5) - 1) + 0..2 over 160 ms; when the game reloads (upper track `+0x74` = 17, also for a part clip) the magazine drops once the last shot's frames are done (20 for 150 ms, then 21), and at 0 with a reserve before the track starts, from 200 ms, until the refill raises a loaded one (22, 23 for 150 ms each when four or more go in). A shot wins over the reload frames. |
| Energy weapon | A shot plays 1-7 at 60 ms then 0; shots during the 480 ms cycle extend it to the end of the current cycle. |
| Freezer | A shot: 1 (peak), 2, 3, rest at 230 ms; shots within that keep pulsing 1-2-3-2 at 70 ms, then 3 and rest. |

State resets on a weapon change and a savestate load.

## Evidence (private offscreen runs, UI slot 12 copy, `recomp/analysis/d08a32/integ/`, local)

- `dnstuff`, every slot by its key or the wheel, 16:9 framed and borderless
  and 4:3, 60 Hz: weapons 1-6 and 8-14 draw their icon in the box's left cell
  at the gadget icons' position; 0 (no weapon) and 7 (Gatling) keep the belt.
- Frame logs while firing (60 Hz): crossbow from 5 bolts to empty, Desert
  Eagle through a full clip, the reload and the next clip, energy weapon and
  freezer held and released: every sequence as in the table above.
- 180 Hz replay presentation (2472 of 2475 presents redraws; the redraws'
  uploads at (984,205) and (1008,212)): 30 captures of shotgun, freezer and
  Desert Eagle while firing all show the icon and its animation, never the
  belt.
- Vanilla: no draws (`ammo_icons.draws` 0), original belt.
- `test_ui_art.py` 12 OK; Python local suites run. `ttk-controls-test` with
  the LEVEL00 fixture stops at the D14 layout check
  (`modern_controls_native.cpp:1751`, `hud_intact`) with or without this
  change: the D20J air-box move writes layout element 6 inside the status bar
  window, which that check does not expect (pre-existing, not changed here).

## Revision 2

User (2026-11-07): "the desert eagle always ends before reload with 1 bullet
in the mag" (holding fire). Two causes found and fixed: the clip
is the high byte `+0x231` (the low byte of `+0x230` is written separately, and
rounding the halfword up showed a leftover round), and the game also reloads a
part clip (seen offscreen: a press with 2 left reloaded), which the icon only
followed at an empty clip. The magazine now drops when the reload starts (upper
track `+0x74` = 17) and stays gone until the refill. Offscreen at 180 Hz, held
fire through a full clip and the early reload both show the drop. Needs the
user's retest.

## Limits

- Not seen by the user in play yet. Not run: thrown-weapon throws, the
  upgraded energy weapon, swimming / jetpack weapon states, multiplayer (one
  player only, as the D14 window), a level change while animating.
- The energy weapon's continuous beam drains ammo every few updates, so held
  fire cycles the atom continuously, as designed.

## D08A38 clean-edge energy atom

User (2026-11-08): the same atom and rules, "but none of the fuzzy edges",
made by editing the frames. The eight shipped energy frames were edited pixel
by pixel (local `atom5_clean.py` beside the atom design):

- the halo pass's pixels outside the drawn rings, nucleus and electrons are
  cleared (23 to 40 per frame);
- grey flecks on the outline take the most common neighbouring green (2 to 6
  per frame); flecks inside the atom stay;
- lone pixels and darkest-green tips are cleared (none remained once the
  halo was gone). Thin rest-pose spokes are kept (8-connected, one pass).

Every other pixel equals the shipped frame; at most 15 colours. The cells are
`energy-clean-0..7-15col.png`; `ammo.json` lists them for slot 10 and the
earlier `energy-N` frames are kept. Animation rules unchanged (a shot plays
1-7 at 60 ms then 0). Offscreen private run (fixture copy, UI slot 12,
Modernized 16:9 framed, 60 Hz): held fire drew cells 40-47 and settled on the
rest pose. User-accepted in play: "its perfect".

