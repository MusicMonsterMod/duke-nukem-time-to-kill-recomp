# D26A - Debug level select

**Accepted 2026-10-04.** User: "excellent! mark as accepted." Follow-up: D22B
(Modernized controls and first person in every level). Backtick console commands `levels` and
`level N` let the user travel to any level the original game's own level-select
cheat offers, then play and use savestates normally. Accepted executable
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`.

## Use

In a one-player game, press backtick and type:

- `levels` - lists the selectable levels with the game's own names. `*` marks
  the current level.
- `level N` - closes the console, ends the current level as the pause menu's
  restart does and loads level N from its normal start. No statistics screen.

Refused (with a reason in the console) before a game starts, in the pause menu,
while a level is already ending and in two-player games. Unknown numbers are
rejected. Works in Vanilla and Modernized.

## The game's own level data (owned SLUS-00583)

- Level index: `0x800be570` (word). File id `0x1ad + index` is the level
  overlay (`80025018`), loaded at `0x800ca968`.
- Names: `0x800c3d2c` holds one string id per level (halfwords), resolved
  through the string pointer table `0x800c60f4`.
- Selectable set: the title screen's level-select cheat (flag `0x800bdea6`,
  string "LEVEL SELECT ENABLED") cycles `0x800bdeaa` at `80022d48`. It covers
  0-31 and skips 4 and 13 (`XXX`), 14 (animation test), 15-20 (two-player
  arenas) and 30-31 (empty, sound test). The console lists the same 21 levels:

| Index | Name | Index | Name |
| --- | --- | --- | --- |
| 0 | TIME TO KILL | 9 | PIG FACTORY |
| 1 | DUKE HILL | 10 | HOG HEAVEN |
| 2 | MINER 69ER | 11 | LET THE GAMES BEGIN |
| 3 | GOLD AND GUNS | 12 | BLOOD BATHS |
| 5 | OBEY OR DIE | 21-26 | CHALLENGE STAGE 1-6 |
| 6 | FAMILY JEWELS | 27 | THE REAPER |
| 7 | RESISTANCE IS FEUDAL | 28 | WING'ED DEATH |
| 8 | HOLY TERROR | 29 | MOLOCH THE GATEKEEPER |

Only the 21 numbers are hard-coded (the cheat's skip rule); names come from
guest RAM at run time.

## Level flow

- Mode machine `8001bb84`: mode `0x800bcbb0`, phase `0x800ce1b0`, handler
  table `0x800bcbb4` (init, enter, run, exit per mode); `set_mode` is
  `8001bb70`. Mode 0 is the title, 1 gameplay, 2 the statistics screen.
- Mode 1 run handler `8002666c` returns (ends the level) as soon as end code
  `0x800be55e` is not 1. The pause menu's restart writes 0xfd there.
- Mode 1 exit `800270e8`: 0xfd goes straight back to mode 1; 4 (level complete,
  `80094a38`) goes to the statistics screen, whose exit `80028124` sets
  `index = next(index)` (`80027fc0`) and returns to mode 1.
- Mode 1 init `80025438`: a pending saved-game load (`0x800be560`) replaces the
  index from the save block; then `8002b9f4(1)` tears the old scene down
  (call `8002546c`, return `80025474`) and `80024fdc(index, -1, -1)` loads the
  level.

## Implementation

`recomp/src/ttk/level_select.cpp`:

1. `level N` checks the state (gameplay mode and phase, end code 1, no
   saved-game load, not dying, pause menu `0x800be56a` = 0, one player),
   stores N as pending and writes only end code 0xfd.
2. A function-entry plugin on the already-hooked `8002b9f4` waits for the
   init's own call (return `80025474`, a0 = 1). The ending level's earlier
   teardown call (return `800270d0`) is ignored. At the init's call it writes
   the index, provided the end code is still 0xfd, no saved-game load is
   pending and no savestate was loaded since the command. Otherwise the request
   is dropped and logged.
3. All of this requires a SHA-256 code identity over the mode machine, mode
   table, mode 1 init/run check/exit, the loader entry, the title cheat's cycle
   and the level name table.

Framework (`time-to-kill-accepted-source.patch`): the console passes unknown
lines to `ttk::level_console_command` and lists both commands in `help`. The
debug port gained `{"cmd":"console","line":"..."}`, which runs a console line
and returns its replies, for offscreen tests.

No hook list, generated code or hashed header changed. Existing savestates
still load.

### First attempt (rejected)

The first build wrote the index together with 0xfd. Travel between two Old
West levels (1 to 2, 1 to 3, 2 to 1) exited with a null PC. The crash trace and
RAM dump showed LEVEL01.OVR still resident and unchanged while the main loop
(`80025cec`) called the per-level player routine `0x800c5580[index]` for the
new index, a LEVEL02 address inside LEVEL01's code. The old level runs one more
frame after the request, so the index must not change until the init. Other
pairs only survived because the new address happened to be harmless. The
deferred write fixed every pair.

## Evidence (offscreen, private card copy, port 9246)

- Survey, one session from private UI slot 6 (LEVEL00), Modernized: `levels`,
  three rejected numbers (4, 99, `x`), then `level` 1, 2, 3, 5-12, 21-29 and
  0. Every arrival reached mode 1, phase 2, end code 1 with the requested
  index in 11-13 s. The resident overlay was the level's own file
  (`LEVELxx.OVR`, byte-equal to the disc except the 16-byte scratch tail). Its
  tag matched (3-0x1b) and health was 100. Screenshots show each level with its
  era's Duke costume.
- Old West round trip 1, 2, 3, 1, 2, 1, 3 (the crashing pairs): all arrive.
- Modernized after `level 1` (DUKE HILL): lease ready, first person active,
  W moved Duke (58 moves).
- Savestate: save in DUKE HILL, travel to 5, load: back in DUKE HILL with
  LEVEL01 resident and controls active.
- Pause menu open (`0x800be56a` = 12): "Cannot travel now: Close the pause
  menu first". After resume, `level 2` works.
- Vanilla: `level 2` then `level 0` both arrive. No Modernized activity.
- Before a game starts: "Cannot travel now: Start or load a game first".
- ttk-controls-test (32 groups), ttk-input-test, ttk-aim-test, ttk-near-test
  pass. The framework patch applies to the pinned clean dependency and
  reproduces the working tree.

## Limits

- Travel uses the restart path, so Duke starts the new level with what the
  game's restart gives him (health 100 in every survey arrival). The weapons
  and inventory carried over were not audited. Use `dnstuff` if a level needs
  more.
- Arrivals were verified up to normal play for a few seconds. Completing
  levels, the statistics screen after a selected level, and challenge/boss
  rules were not exercised.
- Modernized controls still authenticate only LEVEL00 and LEVEL01. Every other
  level runs the original controls (lease refusal `[TTK identity]`) until D22B.
- The title screen is not supported: start or load a game first.
