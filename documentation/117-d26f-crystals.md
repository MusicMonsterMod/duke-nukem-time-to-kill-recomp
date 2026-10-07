# D26F - Spawning in-game items, continued: level crystals

Status: **Done (2026-10-07, user: "this is awesome. it works! the crystals
really do work.").** Research, then the approved design
(user: "that's how it should work, i love it!") is built. Modernized console
only; Vanilla unchanged.

This note is also the project's first **custom pickup** contract: how to make
an object the game never meant to be picked up behave like a real pickup,
using only the game's own routines. See "Custom pickups" below.

## Question

In level 0 (UI slot 4), `spawn subway security key` appears in the inventory but
`spawn green energy crystal` does not. The user guessed that the crystals are
palette swaps of one green crystal and that the in-level ones have special
properties. Both guesses are right.

## Two different things called "green energy crystal"

| | Type 761 (what `spawn green energy crystal` made before D26F) | Level crystals |
| --- | --- | --- |
| Types | 761 | 176 red, 177 blue, 178 green |
| Type flags (`*0x800d2660 + 28*type`) | `0x2010901a`: walk-over pickup bit `0x20000000` | `0x0010901a`: no pickup bit |
| How it is collected | Touch: the pickup loop `0x8007fd04` calls the dispatcher `0x80081a48` | Action button at a holder: `0x80091cec` |
| Inventory item set | 14 (`player+0x38c`), message 115 | 11 / 12 / 13 (`+0x380/384/388`) |
| Model id | `0x371` | `0x1d1` / `0x26d` / `0x371` |

The Select inventory (`0x800884bc`) and the D08A5 mission inventory list only
the items the level names. Level 0 names 6, 7 and 11-13, so item 14 never
shows. In level 1, item 14 is the Scrap of Paper. In level 6 it is the first
Family Jewel. A 761 pickup there would mark that item as found (not tested;
spawn needs the model loaded).

The subway key (153) works because its dispatcher case sets item 6, which
level 0 names.

## The original crystal take

`0x80091cec(player)` runs from the player state code (`0x80047030`,
`0x8004937c`) during animations 0x83 and 0x10c:

1. Frame flag `player+0xb8` bit `0x20`: the holder at `player+0x290` has a state
   in `+0x35` (1, 2 or 3). `0x80092d78(level, state-1)` gives a slot in the
   level object table `0x800d2550`:
   - level 0: slots 6, 10, 19;
   - level 5: slots 2, 3, 4;
   - level 9: slots 19, 20, 21.
2. The crystal object is unlinked from its cell. Duke holds it:
   `+0x3b8 = 1` and `+0x3b9 = 11/12/13`, chosen by type 176/177/178.
3. Frame flag bit `0x10`: item `+0x3b9` gets flag bit 0 and the held state is
   cleared. The item is now in the inventory.

The receptacle code counts a crystal as found when its flag is set or when it is
held (`+0x3b8 == 0x0b01/0x0c01/0x0d01`). This code is the main exe at
`0x80091ea0` and `0x80092f10`, plus LEVEL00/05/09.OVR. LEVEL09.OVR also sets
crystal flags itself after `0x80092bdc`.

Level 0 objects, from UI slot 3 (the user's later level-0 save):

| Slot | Type | Position | Holder |
| --- | --- | --- | --- |
| 6 | 176 red | (75264, -9958, 53751) | slot 14, type 180, state 1 |
| 10 | 178 green | (18436, -6886, 127618) | slot 37, type 180, state 2 |
| 19 | 177 blue | (-8641, -4838, 55299) | slot 18, type 181, state 3 |

## Palette swap

All three crystal models are loaded in level 0. Their headers, bounds, vertex
blocks and face lists are identical, using texture page `0x3804`. The only
difference is the face CLUT halfword: red `0x0bb9`, green `0x1538`. So they
are one mesh and one texture, with a different palette each. Type 761 uses the
green model.

This corrects note 111's limit that "red and blue crystals are not loaded".
Their models are loaded. They are just not walk-over pickup types, which is
what `items` lists.

## Evidence

Private Xvfb Modernized runs on a copy of the user's cards
(`recomp/analysis/d26f-crystals/`, local):
- UI slot 4, level 0: `spawn green energy crystal`, then walk over it.
  Items 1-16 before: all 0. After: item 14 = 1, nothing else.
- UI slot 3, level 0: item 7 found, no crystals. The table above comes from
  the live object table.
- Static reading of the prepared executable and the LEVEL overlays.

Not done: a real take from a holder in play. Levels 5 and 9 were checked only by
code reading.

## What was built

`spawn 1761` / `2761` / `3761` (names `red / blue / green energy crystal`) in
levels 0, 5 and 9:

| Console number | Object type | Model | Item | Message |
| --- | --- | --- | --- | --- |
| 1761 | 176 red | `0x1d1` | 11 (`+0x380`) | 113 RED ENERGY CRYSTAL |
| 2761 | 177 blue | `0x26d` | 12 (`+0x384`) | 114 BLUE ENERGY CRYSTAL |
| 3761 | 178 green | `0x371` | 13 (`+0x388`) | 115 GREEN ENERGY CRYSTAL |

The numbers are console-only: the type table has 1062 entries (`0x7428 / 28`),
so 1761-3761 never collide with a real type. They are the generic crystal's
number 761 with the colour's position in the TTK font picker order (red, blue,
green) in front.

Behaviour:
- The crystal drops a few steps ahead along the camera view, falls, bounces and
  rests like any dropped item.
- Walking over it collects it: original message and sound, then it leaves the
  world. Mid-bounce collection works too.
- Collection sets only the inventory flag. The Select inventory, the D08A5
  mission inventory and the receptacles read that flag.
- Already found: the spawn still works; the notice says "(already found)".
- Other levels: "Spawn: no energy crystals in this level (levels 0, 5 and 9)".
- `spawn 761` keeps the original generic crystal (renamed "generic crystal" in
  the list). In levels 0/5/9 the console adds: "Note: 761 is the generic
  crystal (item 14); this level's crystals are 1761-3761", and the notice says
  "SPAWNED GENERIC CRYSTAL (ITEM 14)".
- `items` adds "Crystals: red energy crystal (1761), blue (2761), green (3761)"
  in those levels.

Code: `recomp/src/ttk/spawn.inc` (`spawn_crystals`, `spawn_crystal_update`,
`spawn_crystal_collect`, `spawn_crystal_touched`), called from the existing
`spawn_update` on the authenticated player update (`modern_controls.cpp`).

## Custom pickups

The general recipe, worked out on the crystals. Every step is an original
routine or an original field write, in the order the game itself uses them.

### Why an object is or is not a pickup

- **Walk-over pickups** are chosen by type flags, not by the object:
  `*0x800d2660 + 28*type`, word 0, bit `0x20000000`. The player's pickup loop
  `0x8007fd04` queries nearby objects (`0x80077b5c`, box 0x200 x 0x258),
  keeps those with that bit, tests `|dx|` and `|dz|` against `max(0x100,
  object+0x5a)`, then calls the dispatcher `0x80081a48(player, object)`.
- **The dispatcher** switches on `object+0x2c` and does the item's effect
  (flags, ammo, health), then a common tail. A type with no case does nothing.
- **Per-type behaviour** (fall, spin) is the pickup update `0x800814d4(object)`,
  which only pickup types run. Static types (class 0, like the crystals) never
  run it.
- **Mission items** in the inventory are flag halfwords
  `player+0x354+4*item`, bit 0. The Select inventory (`0x800884bc`) and the
  D08A5 mission inventory list the items the level names
  (`0x80087d4c(item)` for the level at `0x800be570`).

### The recipe (as built for the crystals)

1. **Create:** `0x80095a74 CreateObject(type, cell hint, position*)` (D26E).
   The type sets the model and look. Any loaded type works, pickup or not.
2. **Fall:** write `object+0x35 = 1` (the drop marker `0x80096b98` uses), then
   call `0x800814d4(object)` once. With `+0x35 == 1` and `+0x3a == 0` it
   registers the object with the bounce engine `0x8008f150` (handle in
   `+0x10`; the engine moves `+4..+12` and writes `+0x35`). If registration
   fails, it rests the object at floor `0x8005d358` - 50. Its spin code only
   touches types 0x99-0x9d and 0x415-0x417.
3. **Land:** the engine ends with `+0x35 = 0xff`. A pickup's own update turns
   that into 0 (`0x800816bc`). A non-pickup type has no such update, so the
   host writes 0 itself. A static object is not drawn while `+0x35` is set.
4. **Touch:** each player update, the same test as the pickup loop:
   `|dx|, |dz| < max(0x100, object+0x5a)`, plus `|dy| < 0x258` (the query
   box height).
5. **Collect,** in the dispatcher tail's order (`0x80082d04..0x80082e4c`):
   - unlink from the object's cell: `0x8001c9ac(cells + 88*cell + 0x48,
     object)`, with `cells = *0x800da454` and `cell = (int8)object+0x2e`.
     It returns the object, or 0 when the object is not in the list. Stop on 0,
     so nothing else changes;
   - the effect: here the flag `player+0x354+4*item |= 1`, as the original
     take does (`0x80091fc4`);
   - the message: halfword id at `0x800dcd90 + 2*player+0x233`, display time
     word `0x4b0 / players` at `0x800dcd98 + 4*player+0x233`;
   - the sound: `0x8006bbd8(0x7005)` (761's; most items use `0x7004`);
   - still bouncing (`+0x35 == 2`): `0x8008f104(object+0x10 word)` releases the
     bounce handle;
   - unlink from the active list `0x8001c9ac(0x800c5694, object)` (safe when
     absent);
   - `+0x30 == -1` (a spawned object): `0x80095c7c(object)` returns it to the
     free list. Otherwise `+0x35 = 0x63` (the multiplayer respawn wait).
6. **Forget safely:** the host's list of its own objects is cleared on any
   savestate load (`psx_mod_savestate_loads()`) or level change, and an
   object is dropped when its type changes or when it appears in the level
   object table `0x800d2550` (the real crystals live there). A recycled address
   is never collected by mistake.

Calls run through `cheat_call` (saved stack and scratchpad, identity, one
player, safe stack), only between the original player updates.

### Reusing it

A new custom pickup needs a row: console number, object type (the look), the
effect (flag, or another field), message id and sound. The effect is the only
part that is item-specific. Candidates: mission items with no walk-over type in
their level, or a portable item (D08A4 steroids). Check each new type's
`+0x35` / drawing and its update before relying on steps 2-3.

## Evidence (build)

Private Xvfb Modernized runs on a copy of the user's cards
(`recomp/analysis/d26f-crystals/`, local), from UI slot 4 (level 0):
- Level 0, god mode: 1761, 2761, 3761 each spawned, fell and rested at
  y -9276 (the steroids / 761 rest point, floor - 53). Each was visible (red,
  blue, green on the street) and collected by walking over it. Flags changed
  only 11, 12, 13 (0 -> 1). Message ids 113, 114, 115 showed on screen. The
  object went back to the free list (the next spawn reused its address). 3761
  was collected mid-bounce, 4 frames after spawning.
- The `.` mission inventory showed 3/5 with the three crystals in colour. A
  second `spawn 1761` showed "SPAWNED RED ENERGY CRYSTAL (ALREADY FOUND)".
  `spawn 761` showed the note.
- Level 5 and level 9 (via `level N`): all three spawned and collected. On the
  first level 9 run the scripted walk did not reach the green crystal (it was
  not collected and stayed in the world); a second run collected it.
- Level 1: both `spawn 2761` and `spawn red energy crystal` refused clearly.
- Before the fixes: a spawned 176 with `+0x35 = 1` never fell and was not
  drawn; writing the pickup class byte `+0x15 = 1` did not start the fall; with
  `+0x35 = 0` at floor height it was drawn. After calling `0x800814d4` it fell,
  and ended at `+0x35 = 0xff` until the host cleared it.
- `ttk-controls-test` (D07 LEVEL00 fixture), `ttk-input-test`, Python 121 OK
  (2 skipped), `level_overlay_guards.py --check` match.

Not done:
- Receptacle acceptance in play: code reading says the flag is enough (the
  `dnitems` cheat relies on the same flag).
- Savestate load or level change with a spawned crystal lying in the world.
  The code forgets it; not run.
- Two-player mode: spawn requires one player.
- Vanilla: the existing D26E refusal covers it; not rerun.
