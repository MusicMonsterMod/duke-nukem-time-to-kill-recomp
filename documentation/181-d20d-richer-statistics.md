# D20D - Richer statistics screen

Job: [D20D](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "accepted! ... fully accepted"). Builds on
[note 179](179-d20b-statistics-screen.md) (D20B) and
[note 180](180-d20f-statistics-list.md) (D20F).

## Decisions (user, 2026-10-21)

- Layout approved from the in-game mock-up (below).
- Cheats are not mentioned on the screen and do not affect best times.
- The best time is the game's own level timer; a savestate load rewinds it,
  which is accepted.
- `%` and `!` are wanted in the game's own font, as the Modernized text pack
  already has them (D24C, D24E).

## Layout

`recomp/src/ttk/game_menu.cpp`, `draw_stats`, Modernized one-player only:

| Line | Source |
| --- | --- |
| Level name (small, blue), row - 44 | `level_names[level]` |
| DIFFICULTY: name (small, centred), row - 30 | `0x800c3d7c` (the value damage scaling reads), names from the difficulty menu's ids `0x800c3de4`: WUSSY, GET SOME, LET'S GO, DEATH WISH |
| ENEMIES KILLED: | player `+0x3b6` |
| TIME TAKEN: | player `+0x398` ms |
| BEST TIME: | our file; NEW BEST! (blue) when this finish set it, `--:--` with no record |
| CONTINUES REMAINING: | player `+0x279` |
| SECRETS FOUND: | `+0x3b0` of `0x800beb18`, not levels 27-30 |
| MISSION ITEMS: | D08A5's table, collected = held (bit 0) or used (D08A19 `0x4000`) of `player+0x354+4*i`; only levels that have mission items |
| CHALLENGE FOUND: | `0x800d79f6`, levels 0, 2, 5, 7, 9, 11 |
| HEALTH / ARMOR: | `+0x32 / 100`, `+0x234 / 100`, as `62% / 0%` |

Rows are 13 apart (the original's 15 did not fit), from row - 12, in the
original's columns; the D20F list and prompts keep their place. On level 0,
the busiest, the last row ends above CONTINUE. Separate HEALTH and ARMOR
rows were tried and ran into CONTINUE. Challenge stages: name, difficulty,
COMPLETED / FAILED.

## Best times

`ttk-best-times.txt` in the savestate directory (`savestate_dir()`), one
line `level difficulty milliseconds` per record, written through a temporary
file and rename. When the statistics screen opens after a finished level
(not after a savestate load), TIME TAKEN is recorded if there is no record or
it is shorter; the screen then reads NEW BEST!. After a load the stored best
is shown, never NEW BEST!. Challenge stages record nothing. Vanilla and
two-player keep the original screen and never touch the file.

## '!' and '%' in the game's small font

The game's text call draws from its own VRAM font, which has neither glyph.
They are the Modernized pack's glyphs (`build_ttk_fonts.py`: D24C '!',
D24E '%'), rebuilt in the game's drawing chain: `analysis/d20d-stats/runs.py`
splits each shipped glyph into 1-row runs found in the font's own I, '.' and
'/' texels (13 runs for '!', 17 for '%'), and `Guest::say_runs` draws each as
a glyph quad (an 'I' call, its UVs and corners rewritten; the game's quads are
u..u+w, v..v+11 at x..x+w, y..y+11, exclusive). Advances: '!' as I, '%' as
'/' + 7. `say` and `width` treat them like D19T's quotes, so centring and
right alignment hold. Small font only (font 0); the menu font is unchanged.
The tables are coordinates in the font sheet, no pixels.

## Font picker

The local font picker (`recomp/analysis/d24a-fonts/`) now builds '(' ')'
(D24D), '%' (D24E) and the double quotes (D19T) with the shipped functions,
alongside D24C's '!', so the glyph sheets show what ships. New section 16
shows them in messages, the console and at 6x.

## Evidence

Offscreen, real GPU, private cards and savestates
(`recomp/analysis/d20d-stats`, copied from D20B; slot 5 is level 0), `t1.py`:

| Check | Result |
| --- | --- |
| First finish (timer set to 2:05) | NEW BEST!; file `0 1 125033` |
| Slower (3:20) | BEST TIME 02:05; file unchanged |
| Faster (1:35) | NEW BEST!; file `0 1 95533` |
| Difficulty LET'S GO, 5:00 | DIFFICULTY: LET'S GO, NEW BEST!; own line `0 2 300033` |
| One item held, one used; health 137, armor 50 | MISSION ITEMS 2 OF 5; `137% / 50%` |
| Level index rewritten (drawing only) | boss 27: no secrets, mission or challenge rows; stage 21: FAILED; 29: CONTINUE only; level 1: 1 OF 4, no challenge row |
| New launch, 3:20 | BEST TIME 01:35 from the file |
| 1:30, saved to slot 9, slot 9 loaded | NEW BEST!; after the load BEST TIME 01:30, file unchanged; CONTINUE -> level 1 |
| Vanilla | original screen; file unchanged |
| Tests | Python 164 OK; `ttk-input-test`, `ttk-near-test`; `check_repo.py` |

No hooks, guards or framework files changed; codegen hash unchanged.
Executable `076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.

## Limits

Offscreen only, levels ended by writing the end code. Mission and health
values were set by writing memory, not collected in play; whether the
original clears mission flags before this screen on a real exit is not yet
seen (they were intact at the written end code). The 1 s wait is unchanged.
