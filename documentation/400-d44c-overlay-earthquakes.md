# D44C - Earthquakes for the level overlays' own shakes

2026-11-06. Status: Done (user-accepted 2026-11-06: "i accept it, i played
this irl, and they are all working!"). Builds on [note 397](397-d44a-earthquakes.md)
(D44A, the earthquake) and [note 399](399-d44b-scripted-earthquakes.md) (D44B,
the overlay survey and the train).

Evidence labels: **D** static disassembly of SLUS_005.83, the disc's level
overlays and the runtime's overlay interpreter, **M** measured in a private
run.

## Request

User, 2026-11-06, answering D44B's open questions: the train's 1.6 s
earthquake stays as it is (crash only); the MINER 69ER tremor rooms, the
OBEY OR DIE / PIG FACTORY blown-apart types and the WING'ED DEATH and Moloch
boss attacks all become earthquakes.

## What the sources are (D)

| Level (tag) | Code | What it is | Earthquake |
| --- | --- | --- | --- |
| 2 MINER 69ER (`0x5`) | `0x800cc954`, Duke's per-frame room hook | while Duke's room `+0x2e` is 28 or 35 the camera jitter is held at strength 2, room 36 at 4, room 38 at 6 (time 300 each); the pad is latched at 25, 65 or 100 | **held while Duke is inside** |
| 5 OBEY OR DIE (`0x8`) | `0x800cbaa0`, called at `0x800cc888` | the robot turrets that drive around the level (user: "a robotic turret that drives around", two at the start), types 621-623, blown up by a level copy of the vest's blast routine; its one direct caller is a vest-like proximity fuse on `+0x1f0` (Duke's distance, stored at `0x800cc480`; beeps below 0x2401, blows at 0x800 or less) (blast 2000, 5000 or by size), sound 0x11, native shake from the return `0x800cbcf0` | 16 tics |
| 9 PIG FACTORY (`0xc`) | `0x800cb4a4`, the same code | the same, return `0x800cb4ac` | 16 tics |
| 28 WING'ED DEATH (`0x1a`) | `0x800cb6fc` | the boss lands (animation 0x143, velocities cleared, landing sound 0x1016 or 0x2007), shake at Duke's camera, return `0x800cb704` | 16 tics |
| 29 MOLOCH (`0x1b`) | `0x800cb7f4`, `0x800cb95c` | the boss's landing (state flag `+0xb8` 0x100) and stomp (`+0xb8` 2), returns `0x800cb7fc`, `0x800cb964` | 16 tics |

The D44B survey called the LEVEL05/09 code "the blown-apart death". It is a
level copy of the vest's blast (the overlay's function table at `0x800ce080`
holds it beside the main executable's `0x80063f0c`), with a vest-like fuse
as its one direct caller, so this note first took the types for vest
bombers. The user identified them in play as the robot turrets (2026-11-06,
UI slot 6 below): the blast is how a turret is destroyed. They get the
vest's 16 tics.

### The overlay calls reach the hook (D)

The overlays run in the dirty-RAM interpreter. Its `jal` (`dirty_ram_interp.c`,
case `0x03`) sets `ra` to `pc + 8`, runs the delay slot and enters the compiled
`0x80028a2c` through `interp_enter_compiled`, so D44A's entry hook sees the
overlay's return address and the camera in `a0`, exactly as for a main
executable call. The bosses pass `*0x800d796c`, which is Duke's `+0x7d4`, the
camera `0x800d6eb0`.

## Change

`recomp/src/ttk/earthquake.inc` (no framework edit, no new hook, no codegen
change):

- **Overlay sources**: a table of five (level tag, return address) pairs.
  The D44A entry hook of `0x80028a2c` accepts an overlay return address only
  while `identity()` holds and its tag is that level's (the whole overlay
  body matches the owned disc), and only if `ra - 8` is still
  `jal 0x80028a2c`. As for the barrels, the request is pointed at the scratch
  camera so the native jitter is replaced, and 16 tics start, but only when
  fewer than 16 are left: a state that asks twice cannot hold t still (the
  bob flips on t), and a longer earthquake (the console's) is not cut short.
- **MINER 69ER tremor**: each live game frame (`earthquake_scripted`), while
  LEVEL02 is authenticated and Duke's room is 28, 35, 36 or 38, the
  earthquake keeps running: a new one starts when none runs, and a running one
  under 16 tics gets 16 more. 16 is a multiple of 4, so the bob and the yaw
  twitch run on unbroken. Leaving the room lets the last tics play out
  (up to about half a second).
- **Tremor jitter**: at the camera update entry (`0x8003ade4`, Duke's camera,
  caller `0x80025ee8`), which runs `0x80028f2c` and so the shake player
  `0x80028bdc`, a native shake at or below the room's strength is cleared
  (`+0x48`, `+0x4a`). A stronger shake in the room (a rocket) plays as usual.
  The overlay's own pad latch (25 to 100) is left as it is, stronger than
  EDuke32's rumble.
- Modernized with EARTHQUAKES on only; the option off and Vanilla keep every
  native shake. `earthquake_json` adds `robots`, `bosses`, `tremors`
  (earthquakes started by a tremor room) and `tremor_masks`.

## Evidence (M)

Private runs: `recomp/analysis/d44c/probe.py` (local), a copy of the D44B
private cards, offscreen, debug port, `DNTTK_QUAKE_TRACE=1`, levels reached
with the debug `level N` command. Player saves and profile were hash-checked
unchanged after every run.

| Case | Result |
| --- | --- |
| `level 2/5/9/28/29` | overlay tags `0x5`, `0x8`, `0xc`, `0x1a`, `0x1b` as expected |
| MINER 69ER, Duke's room field held at 36 for 4 s | one tremor earthquake, kept running: tics 16..1 then topped up (27, 24, 21 ... at the slowed frame rate of the polling run), camera `+0x48` 0 in all 146 samples, 12 masks; after the room returns to 10 the last tics play out, then nothing |
| the same, room 38 | the same (11 masks, no native shake) |
| the same, EARTHQUAKES off | native tremor (strength 4, time 300 counting down), no earthquake |
| the same, Vanilla | native tremor, no earthquake |
| TIME TO KILL train (D44B regression) | `start 48.00 tics (train)` at field 413, state 3 at frame 415, 45 images, as in D44B |

The room test writes Duke's room field each poll (he stays at the level
start), so it exercises the overlay's real room check, the mask and the
earthquake, not the walk into the rooms.

Not exercised in these first runs: the turrets and the two bosses. In OBEY OR DIE a
spawned type 622 (a turret) drove away from Duke and stayed at 8000 health under fire; its
fuse never armed. In WING'ED DEATH and MOLOCH the bosses did not land or
stomp in 60 s of Duke idling at the start (MOLOCH's start barrels did quake,
7 barrel blasts through D44A). They need the user's save states or playtest.

Native `ttk-controls-test` 153 PASS, `ttk-aim-test` passes,
`level_overlay_guards.py --check` matches 21 levels. Executable
`10b1afe336b4dbc725ba499b8805bd1c51dfed61d91603388b6c29ef433bb6b4`.

### The user's save states (M)

User, 2026-11-06, UI slots (runtime one lower), private copies of the
player's cards:

| State | Action | Result |
| --- | --- | --- |
| UI 8 WING'ED DEATH | one axe | the boss lands: native shake from `0x800cb704` (strength 450), replaced by 16 tics (`boss`); the overlay path works in play |
| UI 9 MINER 69ER bomber | shots, then walk at him | an ordinary vest pig, type 1044 (D44A's vest source), 16 tics |
| UI 6 OBEY OR DIE, the two robot turrets at the start | shoot them | one turret destroyed: native shake from `0x800cbcf0` (strength 400), replaced by 16 tics (`robot turret`) |
| UI 7 MOLOCH | strafe left | the explosive items behind Duke go up through the barrel routine (D44A): 3 or 4 barrel earthquakes per run; his landing and stomp not reached |

The user reports an earthquake when Moloch dies, and the drill section
tremor: "I am absolutely blown away how effective the rumble is here".

## Open

- The tremor rooms now hold a full-strength Duke 3D earthquake for as long as
  Duke stays (the native tremor was a mild jitter of strength 2 to 6). Duke 3D
  has no weaker earthquake; whether that reads right in the mine is the
  user's call.
- Moloch's landing and stomp: verified statically only; WING'ED DEATH and the
  OBEY OR DIE turrets confirm the same overlay path in play. PIG FACTORY's
  turrets (the same code) not seen in play.
