# D20B - Statistics screen rebuilt; SAVE opens SAVE GAME

Job: [D20B](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21). Follow-ups: D20F (selectable
CONTINUE / SAVE GAME), D20D (richer lines), D20E (research).
Option B of the statistics screen review (user, 2026-10-21): part 1 rebuilds
the screen as ours with SAVE going to our savestates; part 2 is D20D (richer
lines), with counters still to be found in D20E. Modernized only, one-player
games; Vanilla and two-player keep the original.

## The original (game mode 2)

Mode handler table `0x800bcbb4` (init, enter, run, exit per mode): mode 2 is
`0x800271d0`, `0x8002720c`, run `0x80027df8`, exit `0x80028124`. Mode 1's exit
`0x800270e8` goes there for end code `0x800be55e` 4 (level complete), and for
0xfe in a challenge stage.

Run handler, each tick:

- Fresh entry (`0x800beb1a`): timer `0x800d2670` = 300, page `0x800d2672` = 0.
- `0x8001fe60(2)`, then by page: 0 or 2 the statistics, 1 the memory card's
  SAVE GAME (`0x8008747c(-1, 0x800d1440)`; -1 cancel to page 0, 0 saved to
  page 2). Result `0x800d2694`; 1 from the statistics opens page 1.
- The screens: challenge stages 21-26 `0x80027214`, two players `0x80027474`,
  one player `0x800277dc`. Arguments: prompts (timer < 1) and SAVE allowed
  (page 0, and level < 29 for one player).
- Flip `0x8001fcbc(1)` (return `0x80027f4c`), timer -= fields `0x800d21fc`
  (floor 0); a negative result leaves the mode. Exit: two players back to the
  title (or a replay on -2), level 29 the ending, else the next level
  (`0x80027fc0`; a found challenge item leads to its stage).

One-player screen (`0x800277dc`), with geometry `0x800d2210` {x0, y0, w, h}
and row = y0 + (h - 60) / 2:

| What | Text / value | Where |
| --- | --- | --- |
| Level name | `level_names[level]`, flags 5 (small, centred, blue) | 0, row - 30 |
| ENEMIES KILLED: | id 0x86; player +0x3b6 (half) | label -(w/4) - 20 left, value w/4 + 20 right (flags 8) |
| TIME TAKEN: | id 0x87; player +0x398 ms, `%02d:%02d` | row + 15 |
| CONTINUES REMAINING: | id 0xf9; player +0x279 (byte) | row + 30 |
| SECRETS FOUND: | id 0x88; `%d OF %d`, player +0x3b0, `0x800beb18` | row + 45, not levels 27-30 |
| CHALLENGE FOUND: | id 0xfa; YES / NO from `0x800d79f6` | next row, levels 0, 2, 5, 7, 9, 11 |
| Prompts | CONFIRM (0x12) at x0 + w/3, SAVE (0x17) at -x0 - w/3, or CONTINUE (0x19) at 0 | prompt row |

Challenge stage screen (`0x80027214`): the name, then COMPLETED (0xd5) or
FAILED (0xd6) from bit (level - 21) of `0x800d79f7`, centred on the row.
Input (prompts only): Square (`0x800d1440` +0x40) returns 1, Cross (+0x3c) or
Start (+0x10) -1, each with sound 1. Player = `0x800d7198`.

## What Modernized does

- The original screen still runs. At each flip from the run handler (new
  entry hook `0x8001fcbc`, acting only when its return is `0x80027f4c`) the
  timer is set back to 300, so the original never draws its prompts or reads
  the pad; its text call (`0x8008b454` from either screen) is pointed at the
  string's end (the D19K technique).
- At that flip our page draws with the original's text ids, flags and
  arithmetic (`draw_stats`), so it matches glyph for glyph and sits in the
  same drawing chain. Our own countdown keeps the original's 300 fields
  before the prompts; after a savestate load they show at once.
- CONFIRM (Enter / X) writes -1 to the result at the next flip and plays sound
  1: the original's Cross. SAVE (V) opens SAVE GAME (D19Q's page); Escape
  returns, a save returns with GAME SAVED. Escape on the statistics page does
  nothing, as on the original.
- The picture: gameplay's exit (new entry hook `0x800270e8`) grabs the image
  on screen, the level's last. By mode 2's first flip the screen is already
  black (first try: a black thumbnail).
- Sidecar: `level N` (the level just completed) and `complete 1`, no
  `resume`. LOAD GAME shows LEVEL COMPLETE instead of the era; loading opens
  the statistics screen, then CONTINUE goes to the next level.
- A state saved by Vanilla on the card SAVE GAME page (page 1) is left to the
  original.
- Guards: the mode 2 code `0x800271d0`-`0x800281e8` and the mode 1 exit
  `0x800270e8`-`0x800271d0` join the menu's identity set.
- Hooks `0x8001FCBC` and `0x800270E8` added to `game.local.toml` and
  regenerated with `psxrecomp_cli.py generate`; codegen hash `0x8bab543c`
  unchanged, existing savestates load.

## Evidence

Offscreen, real GPU, private profile, cards and savestates
(`recomp/analysis/d20b-stats`, copied from D19N; slot 5 is level 0, ended by
writing end code 4). Scripts: `ex1.py` (the original), `t1.py`-`t4.py`,
`v1.py` (Vanilla); disassembly `m2.txt`.

| Check | Result |
| --- | --- |
| Statistics screen, first 5 s | the five lines, no prompts; X ignored |
| After the wait | CONFIRM and SAVE, matching the original's capture |
| V | SAVE GAME, slot list and pictures |
| Save to an empty slot / over a used one | back to the statistics screen, GAME SAVED; sidecar `level 0`, `complete 1` |
| CONFIRM | DUKE HILL (level 1) starts |
| F3 LOAD GAME in play | TIME TO KILL, LEVEL COMPLETE, the level's last picture |
| Load it | the statistics screen with its prompts; CONFIRM -> level 1 |
| Level 27 / 21 / 29 / 1 (index rewritten on screen) | no secrets row / FAILED, COMPLETED / CONTINUE only / no challenge row |
| Vanilla | original screen; Square opens the memory card SAVE GAME, Triangle cancels |
| Tests | Python 164 OK; `ttk-input-test`, `ttk-near-test`, `ttk-sky-test`, `ttk-aim-test` |

Executable `b217e8898cbfa52a2baabe0efa01d13a3cbea83d9b016950fff636a6757bbbf4`.

## Testing aid

Console `endlevel` (one-player play, refused where `level N` is) writes end
code 4, as reaching the exit does. Listed in `help` (framework `main.cpp`;
patch exported and verified). Executable
`c4bc40729ab7a1efaf7ac90b3ec8492bbdc7bd2a7ad2c9cf123c574a182bc82a`.

## First playtest: date over the line under the picture

First playtest (2026-10-21, user): works; on SAVE GAME from the statistics
screen the date overlapped the line under the picture in a 1444x1020 window
(`research/screencaps/overlap.png`). Cause: the host image (picture, steel
date) was placed in the display-aspect (16:9) fit, but menu frames outside
gameplay are presented fitted at 4:3; in a window taller than 16:9 the two
differ (in a 16:9 window they match, hence not seen offscreen). Fix
(framework `gpu_gl_renderer.c`): the image follows the rect of the last
present that showed a picture. Checked in a real 1444x1020 window: aligned on
the statistics SAVE GAME (4:3) and the pause SAVE GAME (16:9). The title LOAD
GAME in tall windows had the same cause. Patch exported, verified (50 files),
codegen hash unchanged. Executable
`f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.

## Limits

Not yet played through a real level exit in a window; the variants were
checked for drawing only. The original's 5-second wait is kept (part 1 is
faithful). Two-player statistics are the original's.
