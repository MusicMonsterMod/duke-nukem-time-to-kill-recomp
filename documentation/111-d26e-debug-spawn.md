# D26E - Debug spawn console command

Status: **Accepted** (2026-10-06, "im happy with all progress tonight"). Modernized only; Vanilla unchanged.

User request: a debugging command that summons an item in front of Duke, like
EDuke32's [`spawn`](https://wiki.eduke32.com/wiki/Spawn), to test the open
icon designs (D24A) against the real pickups.

## Player use

Backtick console, captured Modernized gameplay:

- `items` - names the level can spawn, then every other pickup-class type with a
  loaded model, by number.
- `spawn <name>` or `spawn <type>` - the console closes and the item appears
  about 600 units in front of Duke. Walking over it picks it up through the
  original pickup code.

Names: `skeleton key`, `scrap of paper`, `old note`, `torn paper`,
`family jewel` (2, 3), `green energy crystal`, `steroids`, `armor`, `health`,
`atomic health`, `medkit`, `biomask`, `goggles`, `jetpack`, `invulnerability`,
`invisibility`, `double duke`, and the per-level key names `subway security key`
/ `transport room id` (level 0), `warehouse key` (5), `gantry key` / `valve key`
(7), `lab key` / `valve room key` (9), plus `key 1` / `key 2` anywhere.

## Original contracts

| Piece | Address | Meaning |
| --- | --- | --- |
| CreateObject(type, cell hint, position*) | `0x80095a74` | Takes a 96-byte object from the free list `0x800ddafc`, clears it, copies x/y/z/w, sets rotation by type, finds the map cell of the position (`0x8005d790`, the hint if that fails), links the object into the cell's list and calls `0x80095504`. Returns the object or 0 |
| SetObjectType(type, object) | `0x80095504` | Writes `+0x2c`; if the type's flags (`*0x800d2660 + 28*type`) have bit 2, attaches model `*0x800ddb18[type +0x10]` and logs an error when the level has not loaded it |
| Drop contents | `0x80096b98` | A destroyed container or enemy with a contents type (`+0x36`, from its placement record `+0x2c`) calls CreateObject at its own position and sets the new object's `+0x35` to 1. In level 0 the Subway Security Key (153) and Transport Room ID (154) are carried this way by actors 50 and 18 |
| Pickup dispatcher | `0x80081a48` | Switches on the touched object's `+0x2c`. Mission flags: 153 `+876`, 154 `+880`, 155-157 `+884/888/892`, papers 547-549 and jewels 854-856 `+908/912/916`, steroids 638 sets `+0x364` value 2 |
| Placement tables | `*0x800de720` (count `0x800c56a0`), and the object table | 48-byte records: type, live object, x, y, z ... contents type at `+0x2c` |

Object layout used: `+4/+8/+12` position, `+0x1c` yaw (0 = +Z, 1024 = +X),
`+0x2c` type, `+0x2e` cell, `+0x35` dropped flag, `+0x40` model.

## Implementation

- `recomp/src/ttk/spawn.inc` (in `modern_controls.cpp`): the item table, the
  model check (the original's own rule), the console parser (event-pump thread:
  parse, Modernized and gameplay checks, model check, queue) and `spawn_update`
  (emulation thread, after `cheats_update`: identity, player, one-player and
  stack checks, then the original call).
- `cheats.inc`: `cheat_call` gained optional a1, a buffer written into the
  saved stack window and passed as a2, and the v0 result. Calls without a
  buffer behave exactly as before.
- `control_guards.inc`: new guard for `0x80095a74` (520 bytes).
- `level_select.cpp/.h`: the console entry hands `items`/`spawn` lines over first.
- Framework `main.cpp`: two `help` lines (exported to the accepted patch).

## Evidence

Private Xvfb Modernized runs on a private card copy
(`recomp/analysis/d26b-spawn/`):

| Check | Result |
| --- | --- |
| Level 6 `spawn steroids` | type 638 created ahead; walking over it showed STEROIDS and set `+0x364` 0 -> 2 |
| Level 0 `spawn key 1`, `spawn key 2` | 153/154 created; pickup messages SUBWAY SECURITY KEY and TRANSPORT ROOM ID; both in the Select inventory |
| Level 1 `spawn scrap of paper`, `spawn torn paper` | both picked up and listed in the inventory |
| Level 0 `spawn 547`, `spawn 854` | refused: not loaded in this level |
| `spawn 99999`, `spawn nothing` | refused: unknown item |
| Vanilla `spawn steroids` | refused: Modernized profile only; nothing created |
| `ttk-controls-test`, `level_overlay_guards.py --check` | pass / match |

## Limits

- Level 0's green energy crystal (761) spawns and is visible, but after walking
  over it, it did not appear in the inventory. Its pickup is unconfirmed.
- Level 0's red and blue crystals are not loaded as pickup types at level start,
  so they cannot be spawned there yet and have no names.
- Types 155-157 share the key model and set flags (`+884/888/892`) that level
  0's inventory does not name. They are spawnable by number only.
- Any type with a loaded model can be spawned by number, including scenery that
  blocks Duke (for example 858). `items` lists only pickup-class types.
- The item is placed at Duke's height 600 units ahead, without a floor search.
  On stairs or ledges it can float or sink.

## Fix (2026-10-06): "Spawn unavailable here" from the real console

User report: every `spawn` refused with "Spawn unavailable here" (starting from
UI slot 4). Typing in the backtick console releases the mouse capture, and
`spawn_update` required captured gameplay input on the very next update. The
earlier tests sent the line through the debug port, which never opens the
console, so they did not catch it. Now:
- the request needs only a verified Modernized gameplay player
  (`player_identity_ready`, one player, safe stack), not captured input;
- it waits up to 300 host frames (about 5 s) for one before refusing.
Evidence: a private copy of the user's UI slot 4 (level 0), via the real console
(backtick, `items`, `spawn steroids`, Enter) spawned type 638 in front of Duke
with "SPAWNED STEROIDS".

## Landing height, direction and console recall (2026-10-06)

User: spawned items fall and "wedge half in the ground". Is that us or the game?
- **It is the game.** The original fall (started by `+0x35 = 1`, as
  `0x80096b98` does for a destroyed enemy or crate) overrides the start height
  on its first update, bounces, and rests at a level-defined height. On a
  private copy of the user's UI slot 4 (level 0 street), a spawned steroids
  settled at y -9276. A pig cop killed with god mode and aim assist dropped
  shotgun ammo (type 65) that settled at the same -9276. Raising the start
  height had no effect, so it was removed.
- Without `+0x35` the object stays at the given height but is not drawn, so
  the original drop path is kept.
- The item now spawns along the camera view (`view_yaw()`), not Duke's body
  facing, so it lands where the player looks with the independent camera.
  Checked: holding W walks Duke straight toward the spawned item.

The console (framework `main.cpp`, exported to the patch) now has typed-command
history: Up/Down recall (newest first, Down past the newest clears the line),
`history` lists up to 64 commands, consecutive duplicates are kept once, and
`help` lists `history`. Checked through the real console: Up recalled
`spawn steroids` and Enter spawned again; `history` listed "1 SPAWN STEROIDS,
2 ITEMS".

