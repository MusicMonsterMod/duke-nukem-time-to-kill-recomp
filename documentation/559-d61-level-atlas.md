# D61 - Level atlas and goto: see every part of every level, and go there

Status: **Done 2026-11-25** (user: "lock it in and close it out"; built the same day, scope agreed with the user
2026-11-24; full buildout authorized: "you may proceed autonomously until all
components are done and this feature is working to a high standard").

Two parts, built to work together:

- **`goto`** (backtick console, Modernized): puts Duke anywhere in any level,
  standing on the floor, facing into the room, ready to play.
- **`atlas.py`** (`recomp/tools/local/`, offscreen, no human needed): reads
  every level's rooms, floors, portals, objects, enemy spawners and lamps from
  the game's own memory, draws a plan of each, tours every room with `goto` and
  photographs it, and flags leads for cut or unused content.

## goto

```
goto [LEVEL] room R [DEG] [quiet]           a standing point in room R
goto [LEVEL] object N [DIST] [DEG] [quiet]  beside level object N (table 800D2550), facing it
goto X Y Z [DEG] [quiet]                    the floor nearest that point
goto LEVEL at X Y Z [DEG] [quiet]           the same in another level
goto NAME [quiet]                           a landmark (recomp/assets/atlas/landmarks.txt)
goto LEVEL                                  the start of a level (hidden ones too)
goto here | goto landmarks | goto rooms | goto face [+|-]DEG | goto fix | goto help
(anywhere in the line: face DEG, quiet, safe)
```

Code: `recomp/src/ttk/goto.inc` (included by `modern_controls.cpp` after
`level_lights.inc`; parsed first in `spawn_console_command`, run by
`spawn_update` on the game thread, in the player update), `level_travel()` and
`level_current()` in `level_select.cpp`, `atlas_segment_query()` in
`weapon_aim.cpp`. Modernized only (Vanilla refuses, unchanged); no card or
save writes.

How Duke is placed:

1. **Another level** is entered first with level select's own travel (the
   restart end code `0xFD`, note 101), so the arrival is the game's normal
   level start. Duke is placed after the level has been in play for 60 frames.
   Within the current level Duke keeps everything he has.
2. **Floors from the game's own collision.** The target room becomes Duke's
   cell (`+0x2E`), which seeds the original segment query `0x8006D980`. Vertical
   rays then find every surface in the column from the room's top down. Duke
   stands on the lowest one with `0x500` clear above it, where play happens.
   A room box can reach above the real ceiling: a ray that starts inside solid
   is answered at its own start (`room -1`), so the column steps down 128 units
   until it is in open space. Height measured: Duke's position is 498 above the
   ground under him (`+0x1C8`), written as the floor.
3. **Solid props count as floor.** The segment query does not see placed
   objects, a platform block or a crate. A solid object (spawn's rule: type
   flags set, no flag 8), whose `push_bounds` box covers Duke's spot and
   overlaps the space he would stand in, lifts him onto its top (stacked ones
   too). If that leaves less than `0x500` below the ceiling, the column is
   refused.
4. **Where in the room.** A room target tries columns on the 1024 grid, the
   room's centre first. An object target tries the given side, then every 30
   degrees at the distance, half of it and double it, then the nearest
   standing point of the object's own room. A point target tries the point,
   then rings up to 1536 away.
5. **Facing.** The given compass angle (0 = +Z, 90 = +X). Otherwise an object
   target faces the object, a room target faces the longest clear view at eye
   height (16 rays), and a point target keeps Duke's facing. The view yaw
   follows.
6. **quiet** sets the game's own invisibility timer (`+0x89C`, enemy AI
   `0x800A5CBC`, note 132) to 2400 units: 8 s at the drain of 300 a second.
   Duke is drawn translucent and the gadget timer shows. It does not stop an
   enemy already at Duke (GOLD AND GUNS rooms 20-23 tour, below).
7. **safe** (added 2026-11-25 at the user's request) sets the game's own
   invincibility timer (`+0x8A0`, Duke's damage handler `0x800A4510`, note
   132) to 6000 units, 20 s, as the invincibility pickup does. It can go with
   `quiet`, in either order. Measured in GOLD AND GUNS room 53 (lava, no god
   mode): without it, health went from 40 to 0 within a second. With `safe` it
   stayed at 100 for 20 s, then he died a second after the timer ran out. A
   first 8 s version also held for exactly 8 s; 20 s gives time to walk out.
8. **Hidden indices.** The level indices the level select skips are entered
   with the developer `level force N` (or `goto N room R`). Their overlays are
   not in the Modernized identity list, so `player_identity_ready()` is false
   there. `goto` then uses `atlas_segment_query()`, gated only by the
   executable code the query runs (`weapon_aim.cpp`'s identity), and a plain
   one-player gameplay check.

Results are logged as `[TTK goto] arrived level L room R at X Y Z facing D
floor F headroom H` or `[TTK goto] refused <why>`. A level entered for a goto
that arrives without pistol ammo (`+0x2D6`) logs a warning and shows "Goto:
arrived without the pistol".

### Pistol start (the D53D report)

The D53D lab arrivals (a private copy of a player save, `level N`, note 528)
came without weapons. That did not reproduce here, by any route:

| Route | Arrival |
| --- | --- |
| New game, `goto 3 room 2` | level 3, health 100, pistol ammo 200 |
| Savestate saved, loaded, `goto 9 room 1` | level 9, health 100, pistol ammo 200 |
| The same load, then `level 5` | level 5, health 100, pistol ammo 200 |
| `level N` into each of the 21 levels from a new game (atlas capture) | pistol drawn, 200, health 100 in every start screenshot |

The cause of the D53D case is not established. It probably comes from that
save's own level-start snapshot, which the restart path restores. The warning
above makes any recurrence visible.

## Fix: Duke's body and gun sound after a goto or warp (2026-11-25)

User report: after `goto room 17` Duke "materializes without a body". His
shots make "a quite little pip pap sound instead of the real blast". The same
happened in agent-made saves: UI slot 5 lost Duke's body after a 180-degree
turn, and the D53D security-monitor saves loaded without his body.

Cause, reproduced on a private copy of UI slot 5 (GOLD AND GUNS, made by an
earlier lab's `warp` at 45000 -500 -20600):

- Duke's cell (`+0x2E`) said room 50, but his node sat in room 48's actor list
  (`+0x4C`), and stayed there after walking.
- The game draws, and by all signs hears, Duke by the room whose list holds
  him. So turning away from room 48 hid his body and weapon.
- `goto` and `warp` wrote the position and the cell but never moved the node.
  A fresh `goto 9 room 25` left him listed in the level's start room 1.

Fix (`goto.inc` `goto_relink`, called by goto, warp and the new `goto fix`):

- The game's own actor relink `0x80095D9C(actor, position*)` (disassembled;
  the function starts at `0x80095D9C` with its stack frame) does the move. It
  finds the room at the position (`0x8005D790`, hint the cell), takes the
  actor out of its cell's `+0x4C` list (`0x8001C9AC`), puts it in the new
  room's (`0x8001C6DC`) and sets the cell.
- It moves an actor only when that room differs from its cell, so the cell is
  first set back to the room whose list holds Duke.
- `0x80095CB0`, the relink spawn uses, is the placed-object version (list
  `+0x48`). Tried first, it added Duke to an object list without leaving the
  actor list, and was dropped. No state was saved from those runs.
- `goto here` now says "drawn by another room: type goto fix" when the two
  disagree. `goto fix` relinks Duke into the room he stands in, which repairs
  an old save once it is loaded.

| Case | Before | After |
| --- | --- | --- |
| UI slot 5 copy, loaded | cell 50, listed in 48; no pistol after the turn | `goto fix`: listed in 50, pistol drawn after the turn |
| `goto room 17` (GOLD AND GUNS) | listed in the old room | listed in 17; pistol after a 180 turn |
| `goto 9 room 25` (travel) | listed in start room 1 | listed in 25 |
| `warp 52620 -9700 54160` | listed in the old room | listed in 25 |

The gun sound was not heard (offscreen runs have no audio). It is expected to
follow the same link; the user's ear decides.

## Fix: still swimming after a goto out of water (2026-11-25)

User report: in GOLD AND GUNS, `goto room 10` (underwater), then a few seconds
later `goto room 8` (land), and Duke "still thinks he's underwater, cant
control properly, and then suffocates".

- **Cause.** Dumps of Duke's struct on land (room 8), underwater (room 10) and
  back. Both mode bytes (`+0x22C`) stayed 5/5 (land 0/0), the animation stayed
  127 (underwater idle; land 63), and `+0x224` kept bit `0x100`. The game
  recomputes the surface height (`+0x834`) at the new place, but the cached
  water depth `+0x838` stayed 1407. `0x80054CC8` sets mode 5 again whenever
  that depth is `0x280` or more. The game only leaves swimming at the surface,
  which a teleport skips.
- **Fix** (`swim.inc` `swim_teleport_exit`, called by goto and warp after the
  placement, only when Duke was wading or swimming, mode 1, 4 or 5, or had a
  water mask):
  - the land values: mode bytes 0, `+0x224` without `0x100`, depth `+0x838` 0,
    surface `+0x834` `0x40000000`, water mask `+0x20C` 0;
  - then the wade jump's own exit, the original airborne start `0x8003E2D0`
    (animation 97), with no speed and the swim code's 22-update water holdoff;
  - air to 12000.
  Duke drops onto the floor he was placed on and lands. Where the destination
  is water, the game's water query fills the plane in again and he swims.
- **Verified** (the user's sequence, offscreen):

| Step | Mode | Animation | Air | Note |
| --- | --- | --- | --- | --- |
| `goto room 8` | 0 | 63 | 12000 | land |
| `goto room 10`, 4 s | 5 | 127 | 10630 | underwater |
| `goto room 8` (before the fix) | 5 | 127 | draining | could not move (0 units in 0.8 s) |
| `goto room 8` (fixed) | 0 | 63 | 12000 | walked 796 units in 0.8 s; still mode 0 and air 12000 after 10 s |
| `goto room 10` again | 5 | 127 | | swims as before |

  The health lost in room 8 in these runs was two enemies firing down the
  tunnel (no god mode), not drowning.

## Console help and forms (2026-11-25)

User: the help was "quite hard to follow", and they were trying to reach a
hidden level.

- `goto help` (and `goto` alone) is now a list of examples, one per line, each
  with what it does. The extras are listed after them, then the hidden
  levels.
- **`goto N`** goes to the start of level N, hidden indices included.
- **`face N`** is a word, and `face N`, `quiet` and `safe` may stand anywhere
  in the line. The old trailing bare number still works.
- **Errors** suggest a working example ("Try: goto room 12, or goto 3 room
  12").
- **`levels`** lists the hidden indices under "Hidden (developer: goto N, or
  level force N)". `level 13` answers "Level 13 is hidden: type goto 13 (or
  level force 13)". Replies name True Grit and Modern 4.
- **Tested offscreen:** the help, `levels`, the `level 13` hint, `goto 13`
  (arrived in level 13), several orders of `face`, `quiet` and `safe`, a
  landmark with `face`, and the error replies.
- **Not checked:** the help on screen. Offscreen runs cannot open the console
  (D26L). It uses only characters the earlier help used (no `|`).
- The console review is filed as D26L: a text caret, the missing glyphs such
  as the pipe, and every command made consistent.

## Fix: the club music went on after a goto (2026-11-25)

User report (TIME TO KILL): `goto room 12`, walk into the club and its music
plays; `goto room 17` and it goes on playing. The user asked whether this is
the drowning bug again ("the game sets a flag but teleporting somewhere else
doesnt unset it"), and whether "the bug could affect pretty much any state".
Yes: any state the game changes when Duke crosses a room boundary.

- **Mechanism** (disassembled):
  - Every frame, Duke's update (`0x800412A4`) relinks him (`0x80095D9C`).
  - When his room changes, it calls the room-transition handler
    `0x8006C138(old, new)`. That applies the new room's ambience: flags `0x20`
    and `0x10`, and its ambient zone `+0x34`, with two loop slots
    (`0x800C39D8`/`0x800DA964` and `0x800C39DC`/`0x800DA968`).
  - It then runs the level's own table of cues, keyed on the rooms left and
    entered.
  - The club rooms (12, 75) have zone `0x9000`, a looping track in the second
    slot.
- **The game's own teleport,** the respawn at a re-entry point
  (`0x80042DE8`), relinks Duke and then calls `0x8006C138(0, new room)` and
  the music reset `0x8003C240`. Besides those it resets the camera, gives
  invincibility and tops up pistol ammo.
- **Fix:** `goto_arrival_events`. After every goto and warp, the respawn's
  two calls: `0x8006C138(0, room)`, then `0x8003C240`. Not the invincibility
  or ammo. Developer A/B: `DNTTK_GOTO_EVENTS=0`.
- **Verified** (offscreen, the user's sequence):

| After `goto room 17` (walked into the club first) | events off | events on |
| --- | --- | --- |
| Club loop (zone `0x9000`, handle `0x800DA968`) | still playing | stopped |
| Room 17's own ambience (zone `0xA00C`) | not started | started (`0x800DA964`) |

  Regression with the events on: the water exit (land mode, walked 787 units,
  air full after 10 s, swimming again in room 10), the relink (UI slot 5 copy,
  goto, travel, warp), `safe`.
- **Not heard.** Offscreen runs have no audio. Each goto now also runs the
  music reset, as a respawn does, so the level music may restart at a goto.
- **Not covered.** Cues the game fires only for a particular room-to-room
  crossing (the level tables key some on both rooms) do not fire for a
  teleport, as they would not for a respawn. State tied to an attached
  traversal (a ladder, a pole, a moving platform) is not reset beyond what
  the room handler and the water exit do.

## Modernized controls in the hidden levels (2026-11-25)

User report: in True Grit "modern controls and first person doesnt work".

- **Cause.** Modernized controls authenticate the loaded level overlay
  (`control_guards.inc` `level_overlays`, made by
  `tools/local/level_overlay_guards.py` from the owned disc). Only the 21
  selectable levels were listed, so any other tag fails closed and the
  original controls run.
- **Fix.** The generator also lists the five hidden indices one player can
  enter. Each loads, byte for byte as the atlas RAM dumps show, the file
  named:

| Index | Overlay | Tag |
| --- | --- | --- |
| 4 True Grit | `LEVEL04.OVR` (1840 bytes) | `0x7` |
| 13 Modern 4 | `LEVEL13.OVR` (1960 bytes) | `0x10` |
| 14 Animation test | `LEVEL14.OVR` (604 bytes) | `0x11` |
| 30 Empty | `LEVEL30.OVR` (568 bytes) | `0x1C` |
| 31 Sound test | its `BASEOVR.OVR` (1420 bytes) | `0x12` |

  Each passes the generator's layout checks (no scratch tail).
  `level_overlay_guards.py --check` matches 26 levels, and `--arenas --check`
  still matches the six arenas, which stay out: merely authenticating an
  arena must never enable the single-player leases.
- **Verified.** In 4, 13, 14 and 31: first person with the weapon drawn,
  Modernized walking (769 to 799 units in 0.8 s; 291 in the club, against the
  bar), and no identity refusal in the log. Index 30 was not visited.
- **Not checked.** Features tuned per level (lights, level fixes) know
  nothing of these levels, and their own content is unfinished.

## Landmarks

`recomp/assets/atlas/landmarks.txt` (tracked: names and coordinates only, no
retail data): `name level x y z [degrees]` or `name level room R [degrees]`.
`goto here` prints a ready line. Found from the CMake source path
(`TTK_ATLAS_DIR`, as the lamp files), `DNTTK_ATLAS_DIR`, or `atlas/` beside the
executable. Seeded from spots the notes record with a level and coordinates,
each visited in game (2026-11-25):

| Landmark | Arrival |
| --- | --- |
| ttk-start | level 0 room 2, floor -9216 |
| ttk-power-switch (note 524) | level 0 room 57, floor -6144 |
| ttk-alley-ladder (notes 72, 210) | level 0 room 7, floor -9216 |
| gag-lava-south, gag-lava-north (note 549) | level 3 room 50, on the walkways |
| gag-lava-corner (note 556) | level 3 room 50, north walkway facing east |
| l05-monitors (note 528) | level 5 room 18, square in front of the three monitors |
| fj-red-jewel (note 516) | level 6 room 34 |
| l09-monitors (note 528) | level 9 room 25, on the monitor platform, floor -9600 |

After arriving at l09-monitors, l05-monitors and gag-lava-south, Duke walked
about 790 units backward in a second, so he was not stuck. The D53D warp had
left him stuck inside the LEVEL09 platform at y -9600. Two candidates were
dropped: the "lava channel" coordinate in note 549 is a lamp position, and room
49's lowest surface is the lava itself. Spots that exist only as player save
slots (the slot-12 payphone, the MINER 69ER drill) have no coordinates on
record. Add them with `goto here` when one is visited.

## atlas.py

```
python3 recomp/tools/local/atlas.py survey            # every selectable level
python3 recomp/tools/local/atlas.py survey --force 4 13 14 15-20 30 31
python3 recomp/tools/local/atlas.py tour 3            # every room of GOLD AND GUNS, four views each
python3 recomp/tools/local/atlas.py shot "3 room 41"  # one place, four views
python3 recomp/tools/local/atlas.py models 3          # photograph each loaded-but-unused model
```

- **No saves needed.** The tool boots the player build offscreen with a
  private copy of the player's settings and **empty** memory cards. Enter
  skips the intro and starts ONE PLAYER, which reaches play in about 27 s. In
  Vanilla, the test-input keys do not reach the menus, so it presses the pad's
  own Start and Cross through the debug port (active low), which takes about
  39 s. No
  savestate, card or save of the player's is used, and their files are hashed
  before and after every run (the run stops if they change).
- **capture** visits each level with `level N` (or `level force N`), waits
  120 frames, and saves the 2 MB RAM and a start screenshot to
  `research/atlas/raw/`. About 7 s a level.
- **build** reads the dumps with `level_atlas.py`. It writes
  `research/atlas/levelNN.json` and a plan `levelNN.svg` per level, plus
  `index.json` and `findings.md`. All of this is local, because it is derived
  from retail data.
  - Rooms: box, portals (corners are room-relative in memory), world faces.
  - Standing points: the same rule as goto, on the faces' vertex grid (1024
    across, 128 up). Polygon winding does not tell floor from ceiling here:
    in level 0 the floor under Duke faces one way, in level 3 the floor and
    the ceiling face the same way.
  - Objects from both room lists (`+0x48`, `+0x4C`), with the type flags,
    lock flag, model state and level-object-table index.
  - Enemy spawner records, lamps (`assets/lights`), level names (string table
    `0x800C60F4`) and portal reachability.
- **tour** runs `goto room R quiet` for every room, with god mode on (`dnkroz`
  through the test-input cheat word; `--mortal` leaves it off). It takes VIEWS
  shots turning around, then records health, height and room after 1.5 s. The
  contact sheet goes to `research/atlas/tour/levelNN/sheet.html`. If Duke dies
  (mortal), the tour stops: after a death the level did not come back in
  play within 40 s in tests. Use `--mortal` with care.
- **models** spawns each flagged type in front of Duke with the console's
  `spawn <type>` (the game's own creator) and photographs it.

### Measured

| Run | Result |
| --- | --- |
| Capture, 21 selectable levels | all loaded, 6-8 s each |
| Capture, 11 hidden indices (4, 13-20, 30, 31) | all loaded and reached play |
| Tour GOLD AND GUNS (god mode) | 69 of 72 rooms in 327 s; rooms 3, 19, 61 refused (no standing headroom) |
| Tour GOLD AND GUNS (first run, quiet only) | enemies wore Duke down 100 -> 4 over rooms 20-23 and he died: hence god mode |
| Tour index 13 / 4 / 14 / 31 | 45/48, 48/49, 5/5, 16/16 |
| Props | level 0 types 6 and 166: floor lifted 149 and 468 onto their tops; walking forward stepped down to the street |
| Installed LTO+PGO build | `goto gag-lava-south`, `goto 3 room 17`, `goto 13 room 12` (straight into a hidden index), `goto here`, `atlas.py shot l05-monitors` |
| Vanilla (`--vanilla`) | `goto room 3` refused: "Goto: Modernized profile only (Vanilla is unchanged)"; `levels` unchanged |

Arriving inside a secret area triggers the game's own "FOUND A SECRET AREA"
(GOLD AND GUNS rooms 13, 37, 47). A goto counts secrets as found.

Limits:

- Lava and other hazard floors are not told apart from floors. In the GOLD
  AND GUNS lava pit rooms (49-53) the lowest standing surface is the lava.
  Landmarks or a `--mortal` tour find them.
- A few small rooms (vents, alcoves) have no standing point.
- Tours use the player's view settings (third person in the captures).

## Every level (atlas index, 2026-11-25)

| Index | Name | Rooms | Objects | Spawners | Standing points | Islands | Unused models (none named in code) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | TIME TO KILL | 84 | 393 | 65 | 225 | 0 | 12 (9) |
| 1 | DUKE HILL | 78 | 445 | 64 | 187 | 3 | 18 (6) |
| 2 | MINER 69ER | 70 | 388 | 47 | 178 | 0 | 30 (16) |
| 3 | GOLD AND GUNS | 72 | 391 | 46 | 196 | 0 | 36 (20) |
| 4 | XXX (True Grit) | 49 | 118 | 1 | 132 | 0 | 19 (14) |
| 5 | OBEY OR DIE | 77 | 411 | 59 | 202 | 0 | 8 (4) |
| 6 | FAMILY JEWELS | 58 | 202 | 45 | 150 | 0 | 13 (11) |
| 7 | RESISTANCE IS FEUDAL | 40 | 318 | 31 | 114 | 0 | 47 (27) |
| 8 | HOLY TERROR | 64 | 379 | 38 | 174 | 0 | 25 (18) |
| 9 | PIG FACTORY | 76 | 337 | 33 | 184 | 0 | 34 (14) |
| 10 | HOG HEAVEN | 48 | 417 | 57 | 135 | 0 | 39 (31) |
| 11 | LET THE GAMES BEGIN | 58 | 383 | 25 | 158 | 0 | 55 (43) |
| 12 | BLOOD BATHS | 65 | 330 | 80 | 166 | 5 | 18 (12) |
| 13 | XXX (Modern 4) | 48 | 233 | 30 | 130 | 0 | 19 (7) |
| 14 | ANIMATION TEST LEVEL | 5 | 30 | 7 | 15 | 0 | 1 (0) |
| 15-20 | MESA, CASTLE, DUNGEON, COLISEUM, WAREHOUSE, FACTORY | 9-38 | 27-65 | 0 | 24-96 | 0 | 0-1 |
| 21-26 | CHALLENGE STAGE 1-6 | 9-38 | 12-54 | 0-24 | 24-96 | 0 | 0-4 |
| 27 | THE REAPER | 4 | 92 | 1 | 12 | 0 | 0 |
| 28 | WING'ED DEATH | 3 | 46 | 1 | 9 | 0 | 1 |
| 29 | MOLOCH THE GATEKEEPER | 7 | 143 | 1 | 21 | 0 | 1 (1) |
| 30 | (no name) | 1 | 0 | 0 | 3 | 1 | 0 |
| 31 | SOUND TEST | 16 | 8 | 7 | 48 | 0 | 0 |

## Cut content: what the atlas found

Evidence labels: **memory** (read from the game's RAM in a level), **seen**
(photographed in the recomp), **original** (checked on the original game).
Nothing here has been checked on the original game yet. The recomp runs the
original's code, but a find is only called retail content after that check.

**Known online already.** Four of these are described in a Duke4.net thread,
"Duke Nukem: Time To Kill Prototype Differences", reached through GameShark
codes: index 4 ("True Grit"), index 13 ("Modern 4"), the animation test level
and the sound test. Cheat lists name index 14 "Animation Test Level" and
indices 4 and 13 "XXX (incomplete level)". The Cutting Room Floor's page could
not be read by the agent (it serves agents an instruction page), so it was not
checked. What the atlas adds:

1. **Every hidden index loads and plays in the recomp** (memory, seen):
   `level force N` or `goto N room R`. The game's own name table gives 4 XXX,
   13 XXX, 14 ANIMATION TEST LEVEL, 15-20 MESA, CASTLE, DUNGEON, COLISEUM,
   WAREHOUSE, FACTORY (the two-player arenas), 30 no name (one empty room, a
   black void) and 31 SOUND TEST.
2. **Index 4 (True Grit), full room atlas** (memory, seen):
   - 49 rooms, 118 objects, one spawner (type 26: the "single lizard
     guardian" of the forum account).
   - Sandstone and Egyptian-style caverns, lava rooms, a Greek-key frieze (room 16),
     surfaces drawn with missing textures.
   - **18 of its 20 object types appear in no shipped level** (types
     702-726, 461, 81): a prop set used nowhere else.
   - It shares no room box with any shipped level.
3. **Index 13 (Modern 4), full room atlas** (memory, seen):
   - 48 rooms, 233 objects, 30 spawners.
   - A city street, the pink Bootylicious club (room 12), then a green alien
     facility and a chamber resembling Moloch's (room 47).
   - **Three spawner types that no shipped level places**: 538 and 543
     (non-hostile class 8, live in the club: the forum's "two female
     strippers") and 121 (class 12, room 47, not yet spawned when visited).
   - 11 object types in no shipped level.
   - A quarter of its object types are shared with PIG FACTORY.
4. **Index 14 (animation test)**: object types 173-191 in no shipped level.
   **Index 31 (sound test)**: type 621, a spawner type in no shipped level.
5. **Shipped levels load models they never use** (memory; GOLD AND GUNS
   seen):
   - Per level, the models the level loads that nothing it places, drops or
     spawns uses, and that at most four levels load (the generic pickups and
   Duke's weapons are excluded). Then whether any instruction in the
   executable or overlay loads the type number as a constant.
   - GOLD AND GUNS has 36 such models, 20 of them never named in code.
     Photographed with `atlas.py models 3`: a large mounted gun (types 540
     and 541, loaded in three levels and placed in none of them here), mine
     carts and rails (432, 433, 710), crates (606, 610), a bucket (104), a pot
     (254), shelves and a table with missing textures (558, 559), and plain
     shards (3, 64, 240: probably debris).
   - It also loads the Skeleton Key's model (type 542), the item note 517
     found named with no pickup and no lock: the level was built with it.
   - Totals over all levels are in the table above. Debris and effects that
     data tables create are expected among them; the photographs decide.
6. **Islands.**
   - DUKE HILL rooms 4, 9 and 10 are zero-width entries in the room table
     (min x = max x = 0), with no objects: placeholders.
   - BLOOD BATHS splits into five portal groups, each full of objects. Its
     sections are joined some other way (teleporters), so this is not cut
     space.
7. **One object outside every room** in TIME TO KILL (type 203 at
   6679 -7786 38308).

8. **Secret areas are marked in the room table** (memory, seen):
   - Room flags (`+0xC`) bit `0x4` marks a secret area. In GOLD AND GUNS the
     tour got "FOUND A SECRET AREA" in rooms 13, 37 and 47, the only rooms
     with that bit set.
   - Across the 12 campaign levels the bit marks 53 rooms. TIME TO KILL's 7
     match its HUD `S: 0/7`.
   - `atlas.py secrets 0-12` stood Duke in each, in one boot. 47 of 53 were
     photographed, and every photo shows the game's own "FOUND A SECRET AREA".
     The other 6 have no room to stand: level 0 rooms 10 and 80, level 2
     rooms 65 and 68, level 8 room 20, level 9 room 10.
   - A goto into a secret area counts it found and collects its pickup. The
     atlas JSON marks `secret` per room, `index.json` lists them, and the plans
     outline them yellow.

Next steps for the hunt (D61A):

- Tour the other shipped levels.
- Run `models` on each level.
- Spawn the hidden creatures (538, 543, 121, 621) where their levels load them.
- Check finds on the original game (DuckStation portable copy) and online.
- Write up whatever stands as new.

## Tests

- `recomp/tests/local/test_level_atlas.py` (CI-safe; a synthetic level, no
  retail data) covers the reader:
  - rooms, portals made absolute, reachability and islands;
  - the standing point and headroom on a floor quad;
  - objects, the lock flag, the level-object table index, outside-every-room;
  - spawners (empty records skipped), the unused-model flag with the
    code-reference scan, the plan and lamp parsing.
- In game: the runs above.
