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
  over it, it did not appear in the inventory. Explained by D26F
  ([note 117](117-d26f-crystals.md)): 761 sets item 14, while the level's
  crystals are items 11-13 and come from separate objects (176-178).
- (Corrected by D26F.) The red and blue crystal models are loaded in level 0.
  The level crystals are not walk-over pickup types, so `items` does not list
  them.
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


## D26H: out of Duke's pickup reach (2026-11-02)

Status: **Accepted** (2026-11-02, "fully accepted!!").

User, testing D08A33: "make sure the weapons spawn a little in front of duke
because when i spawn a flamethrower for example, duke automatically picks it
up."

Cause (original code). The pickup loop `0x8007fd04` touches an item when
`|dx|` and `|dz|` are **each** below `max(0x100, +0x5a)`: a square, not a
circle. `+0x5a` is the object's bounding radius, computed from its model box
at creation (`0x800961c0..0x80096244`). Measured in level 0: flamethrower 579,
RPG 559, buffalo rifle 485, freezer 442, Gatling gun 417, energy weapon 380,
shotgun 373, crossbow 348, axe 301, pipe bomb 253, knife 240, steroids 235,
shotgun ammo 221, medkit 205, Desert Eagle 183, flamethrower ammo 117. Spawned
600 units along the view, an item is only 424 units out on each axis when the
camera looks diagonally, so anything with a radius above that (the flamethrower
first) was taken at once. Straight down an axis, 600 already cleared every type.

Fix (`spawn_clear_of_reach` in `recomp/src/ttk/spawn.inc`). After
`CreateObject`, the radius is read from the new object. If its larger axis
offset would not clear `max(0x100, radius) + 128`, the object is moved out:
- along the view first, then 10, 20, 30 and 40 degrees to either side (nearest
  first; a direction nearer a map axis needs less distance);
- a spot is used only when the original segment query (`0x8006d980`, through
  `view_segment_query`) from Duke's position to the spot + 128 hits no world;
  a straight-down query from the spot finds a floor within 256 below Duke's
  feet (`+0x1c8`), so no ledge; and the way, widened by 128, misses every solid
  placed object in the room lists (`0x800da454`, the same objects and boxes
  `push_step_clear` stops at: a dumpster is not world to the segment query);
- the object is relinked to its new room with the original `0x80095cb0`
  (guarded since D08T3), then dropped as before (`+0x35 = 1`);
- with no usable spot, it stays at the old 600 position and the notice adds
  "(close: way ahead blocked)".
Items that already clear the reach (most items straight ahead) are unchanged.
The log line now reports the object's final position and room.

Evidence (executable
`85c3eee19a0a343e54b978e2772bd8f75df2ec3fcd0422171e517de249d45d50`; private
card copies of `saves/local-play` UI slot 12, level 0 alley, Xvfb, debug port,
`recomp/analysis/d26h-spawn-distance/probe.py`; player saves, cards and
profiles unchanged after every run):

| Check | Before | After |
| --- | --- | --- |
| Flamethrower / RPG, camera at 135 and 225 degrees | picked up on the first frame (owned 9 / 8) | not picked up; 707 / 687 out on the larger axis |
| Flamethrower, RPG, buffalo rifle at 45, 135, 180, 225, 270, 315 degrees | - | 18 of 18 not picked up; walking along the view took 17 (52-74 frames); the RPG at 45 degrees lay 30 degrees off the view, so a straight walk passed it |
| Same three at 90 degrees, a dumpster ahead | - | every spot blocked by the dumpster; kept at 600, not picked up (larger axis 591-599 for radii 485-579), picked up by walking |
| Duke against the dumpster, flamethrower | spot behind the dumpster (before the object check) | kept at 600 with the blocked notice |
| Every weapon and ammo level 0 loads, steroids, medkit, straight ahead | none picked up at 600 | small items unchanged at 600; buffalo rifle 613, RPG 687, flamethrower 707 |
| `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels), `ttk-input-test`, `ttk-aim-test`, `level_overlay_guards.py --check` | | pass (122 groups) |

Limits: a ledge was not found in the test spot, so the drop refusal is
checked only as code (every accepted spot found a floor). Holy hand grenade and
dynamite are not loaded in level 0 and were not measured. Spawning in a tight
space can still drop a large item within reach (the old placement, now with the
notice).
