# D44B - Earthquakes from scripted level events

2026-11-05. Status: Done (user-accepted 2026-11-05: "lock it in and close it
out"). Source added: the TIME TO KILL subway train crash. Builds on [note 397](397-d44a-earthquakes.md) (D44A, the
earthquake itself) and [note 396](396-d44-earthquake-research.md) (EDuke32
research).

Evidence labels: **D** static disassembly of SLUS_005.83 and the disc's level
overlays, **M** measured in a private run.

## Request

Job: survey the level overlays' event code for collapses, crashes and large
set pieces (Duke 3D's SE 2 analogue) and add them to the D44A timer. User,
2026-11-05: "the train on time to kill would be SO good for this", with a
save state at the ticket booth (UI slot 10, runtime 9: "all you need to do is
press E and it will crash the train"), then "and the gas canisters near that
spot also".

## Survey of the level overlays (D)

Every `LEVELxx.OVR` loads at `0x800ca968` and runs in the dirty-RAM
interpreter ([note 355](355-d23g-step3-overlay-interpreter.md)). A scan of
every overlay for calls to the native blast shake `0x80028a2c`, the pad
latch `0x8001e97c`, the TNT, vest and weapon blast routines, and for direct
stores to the camera's shake fields `+0x48` / `+0x4a`
(`recomp/analysis/d44b/ovr_calls.py`, `shake_writers.py`, local) finds:

| Level | Code | What it does | Earthquake now |
| --- | --- | --- | --- |
| 0 TIME TO KILL | train handler, jump table `0x800caa2c` | the subway train (below); no shake of its own | **yes, 48 tics at the crash** |
| 2 MINER 69ER | `0x800cc954` (Duke's per-frame room hook) | while Duke is in room 28 or 35 the camera's jitter is held at strength 2 (pad 25), room 36 at 4 (pad 65), room 38 at 6 (pad 100), time 300; a constant tremor, not an event | no (native) |
| 5 OBEY OR DIE, 9 PIG FACTORY | `0x800cbaa0` (LEVEL05), the same code at `0x800cb4a4` (LEVEL09) | a level copy of the blown-apart death `0x80063f0c` for types 621-623 (blast radius 2000, 5000 or by size), sound 0x11, native shake 400 | no (native) |
| 28 WING'ED DEATH | `0x800cb6fc` | a boss attack: native shake 450 at Duke's camera | no (native) |
| 29 MOLOCH THE GATEKEEPER | `0x800cb7f4`, `0x800cb95c` | boss attacks (state flags `+0xb8` 0x100 and 2): native shake 450 | no (native) |

No other overlay or main-executable code writes the shake fields directly.
The D44A hook acts only on its two main-executable return addresses, so every
overlay call above still gets TTK's native jitter unchanged. Set pieces that
shake nothing natively, like the train, cannot be found by this scan; each
needs its own trace.

## The train (D, M)

Object `0x801d5d64` in the save, type 77, kind 1 (the static-object
renderer), radius 9573 (the D17AZ object, [note 365](365-d17az-object-reach.md)).
The LEVEL00 handler switches on its `+0x35` through the table at
`0x800caa2c`; the level state is `0x800dd878` (`+0x38` the sound, `+0x40` the
speed):

| State | Code | Behaviour |
| --- | --- | --- |
| 0 | `0x800cc8b8` | E at the booth: three map changes, speed 1, train sound `0xa002`, state 1 |
| 1 | `0x800cc920` | X -= speed, speed doubles to 256, state 2 |
| 2 | `0x800cc95c` | X -= 256 a game frame, relinks rooms; at X < 8001 state 3 (`0x800cca2c`) |
| 3 | `0x800cca30` | the crash: X -= v, Z -= 4v, v halves each frame, debris from `0x8006d6ac(6, ...)`; at v = 0 state 4 |
| 4 | | stopped at (7399, -5784, 39932) |

Measured (private copy of the user's state, 60 Hz game frames): E, state 1
at frame 206, state 2 at 223, state 3 at 413-415 (X 7910), state 4 at 435-439
(X 7399), so the train slides to a stop in about 0.4 s. The native camera
shake stays 0 throughout: the original crash has no shake. The write trace
sees no stores because the overlay is interpreted.

### Change

`recomp/src/ttk/earthquake.inc`, `earthquake_scripted()`, called at the start
of `earthquake_compose()` (once per live game frame, not while paused):

- LEVEL00 only, through `first_map()` (the D22 identity check authenticates
  the whole LEVEL00 overlay, its handler and jump table included).
- The train is found by walking every room's object list (table
  `0x800da454`, 88 bytes a room, list `+0x48`) for type 77, kind 1. The
  address is cached and checked each frame; a search runs at most once a
  second. It is reset by a savestate load.
- When `+0x35` steps from 2 to 3 the earthquake starts: 48 tics (1.6 s),
  Duke 3D's SE 2 length (D44A uses 16 for one detonation). It sets, never
  adds, like every source. A load that lands mid-crash does not start one.
- Modernized with EARTHQUAKES on only; Vanilla and the option off unchanged.
  `earthquake_json` counts `trains`.

### Evidence (M)

Private copy of the user's UI slot 10, offscreen, debug port,
`DNTTK_QUAKE_TRACE=1` (`recomp/analysis/d44b/probe.py`, case `crash`).
Player saves and profile were hash-checked unchanged after every run.

| Case | Result |
| --- | --- |
| Third person, 180 Hz | `start 48.00 tics (train)` at field 413, the frame state 3 appears; 45 shaken images, t 48 to 1 |
| First person, 180 Hz | the same start; 41 images (tics skipped where the game ran at 20 fps, the D44A clock) |
| EARTHQUAKES off | crash as before, no earthquake |
| Vanilla | no earthquake (the probe's E did not start the train under Vanilla's controls, so this run shows only that nothing started) |

Native `ttk-controls-test` 153 PASS, `ttk-aim-test` passes,
`level_overlay_guards.py --check` matches 21 levels. Executable
`d24f6a94247e680abfd30b3deeb87695f61ad7fd8f695311b4c3623b61398abe`. No
framework edit.

## Open

- Accepted at 48 tics, crash only (no earthquake while the train runs).
- The gas canisters near the booth (user: "the gas canisters actually do
  rumble now! they must have been caught by the barrel logic"): they already
  quake through D44A. The user's live game (this executable) counted 2
  trains and 6 barrel blasts (`0x80097090`, return `0x800973e0`) after two
  crashes and the canisters, 0 vests; so the canisters are that game routine
  and need no new source. (The player build has no quake trace, so the
  counter does not name each object.)
- Later in the same session (user: "its earthquaking the turret too ... the
  flaming barrels in time to kill"): the counter reached 9 barrel blasts, 2
  trains, 0 vests, so the turret and the flaming barrels go through the same
  blast routine `0x80097090`. D44A's barrel source covers every object that
  uses it, not only the TNT barrels.
- Candidates for the user to choose: the MINER 69ER tremor rooms (an
  earthquake while inside, or the native jitter kept), the LEVEL05/09 blown
  apart types 621-623 (16 tics, like the vest, once the objects are
  identified in play), the WING'ED DEATH and Moloch boss attacks (Duke 3D's
  bosses do not quake). Other set pieces need a play survey.
