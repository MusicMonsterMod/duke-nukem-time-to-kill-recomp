# D20I - LEVEL STATS and the FPS row

2026-11-03. Status: Done (user-accepted 2026-11-03: "lock it in and close it out, this is awesome!", after the steady-text fix of playtest round 1). Job: [D20I](../MODERNIZATION_JOBS.md).
Builds on [note 179](179-d20b-statistics-screen.md),
[note 181](181-d20d-richer-statistics.md),
[note 183](183-d20e-statistics-research.md) and the D24A fonts.

## Request

User (2026-11-02): EDuke32's level stats in Modernized play, positioned the
same way (bottom left, above the HUD), called LEVEL STATS, with an ON / OFF row
in OPTIONS > DISPLAY, and an FPS row there that stays in step with the
console's `fps` command. The kill total is dynamic, as in EDuke32. User
(2026-11-03): "fire up eduke32 and check it out. we wanna replicate it. we
wanna go cleanroom. check the ttk-fonts doc too".

## EDuke32, observed (clean room)

A private copy of the user's EDuke32 build (Linux, the user's `DUKE3D.GRP`,
`-usecwd`, Xvfb at 1280x720, classic renderer, `hud_stats 1`) in a job folder;
the user's EDuke32 config and saves were not touched. Observation only: test
CON scripts read and set player fields (`actors_killed`, `max_actors_killed`,
`secret_rooms`, `max_secret_rooms`, `player_par`) and screenshots were measured.
No EDuke32 source was read for this job. Under Xvfb the game stalls after 4
tics unless `r_vsync 0` (the stall seen in D39E and D42).

| Behaviour | Observed |
| --- | --- |
| Lines | `T: 0:19.18`, `K: 0/35`, `S: 0/8`, letters and values in separate colours, value column aligned |
| Time | minutes:seconds.hundredths; minutes uncapped (113000 tics read `62:47.26`), no hours, minutes unpadded |
| Kill total | rises at once when an enemy is spawned mid-level (33 -> 34 -> 35 with two spawned) |
| Complete count | the value turns red when X = Y: `33/33`, `8/8`, and `0/0` on a map with no secrets |
| Colours (720p) | letters #8C200C..#D83414 (dark outline #180400 / #280400); values #986444..#ECD8B0 (outline browns) |
| Size and place (720p) | glyphs 9 px tall, 12.5 px line pitch; x = 8; over the mini HUD, 17 px gap; HUD hidden, 10 px over the bottom |
| Other | shown while dead; dimmed under the menu; the FPS counter is top right, so they never meet |

## TTK: where the numbers come from

- Time: player `+0x398` (ms), the statistics screen's TIME TAKEN. It stops
  while Duke is dead and a savestate load rewinds it (D20D).
- Kills: player `+0x3b6` (D20E).
- Secrets: player `+0x3b0` of `0x800beb18`. Boss levels 27-30 and the
  challenge stages read 0 of 0 (levels 21 and 27 seen), shown `0/0` in red as
  EDuke32 shows a map without secrets.
- Kill total: below.

### Dynamic kill total (static survey, then confirmed in play)

The level load `0x800242b0` copies the level's enemy rows to the spawner table
`*0x800de720` (48-byte rows: +0 type, +4 live actor, +0x14 flags, bit 0
spawned), stores their number in the half `0x800d253e`, then adds 6 spare rows
(flag 0x10) and sets the spawner count `0x800c56a0` = placed + 6. The table
count `0x800de724` goes on with object rows (D20E's enemy_total walked all of
them; spare rows read type 0 until first used).

Only one routine makes new enemies after the load: the reinforcement call
`0x80091920(position, n)`. It arms free spare rows (type 57, or 95 on levels 7,
9 and 11) at the nearest free spawn points (table `*0x800d2548`); the spawner
tick creates them when in range and under the live cap. Callers: the damage
class 4 handler `0x800a47bc` (`jal` at `0x800a4868`, n = 4) when Duke kills an
innocent (types 122, 123, 167, 274, 319, 343, 344, 538, 543, 893, 895-900,
1013, 1022, 1023: dancers and other civilians), and level scripts through
`0x80091c34` (levels 1, 7, 8, 9) and directly (level 2). A spare's enemy that
dies or despawns frees the row for the next reinforcement, so a spare row is a
new enemy each time it is instantiated; the level's own rows come back as the
same enemy. `CreateObject 0x80095A74` cannot make an enemy (it skips actor
setup for types with flag bit 0), and the scripted wake-ups `0x80091828`
(levels 0-2) re-arm placed rows that are already counted. D20E's "runtime
increment" in challenge stage 25 is a placed type-0x7f object at load (flag bit
0 clear, not counted). Unverified: only absolute-address stores were scanned.

Rule (`game_menu.cpp`): total = the D20E rule over rows `[0, 0x800d253e)` plus
N, the spare enemies that have appeared this level. `enemy_extras_tick` looks
at the spare rows once per gameplay composition; a row whose enemy is live
(+4 set, flag bit 0) and was not at the last look adds 1. Entering play (a new
level, RESTART LEVEL) clears N; a savestate load brings its own. N is kept in
mod memory, the word after the D20C mirror (D08G7's retired word, now tagged
`0xd201xxxx`; older tags read 0), so savestates carry it. The statistics
screen's ENEMIES KILLED X OF Y uses the same total, so the two never disagree.

The 922 dancer of level 5 is class 8, not an innocent: killing it credits a
kill and calls no reinforcements. A reinforcement that despawns unkilled stays
in the total (it existed), so X can then not reach Y.

## Change

- `recomp/src/ttk/game_menu.cpp`: `level_stats_tick` (each gameplay
  composition, after `enemy_extras_tick`) publishes the three lines while one
  player plays and the game runs (not paused, not other screens; lines not
  refreshed for 250 ms are hidden, which covers the statistics screen) with
  LEVEL STATS on. `ttk_level_stats_image` renders them (1x font pixels in
  720-line units) only when the text changes. DISPLAY gains LEVEL STATS
  (`level_stats`, `DNTTK_LEVEL_STATS`, live) and FPS (`fps`) after CAMERA
  DISTANCE. `fps_sync` follows the console command and the performance key and
  saves the row; the row sets the framework state.
- Fonts (`tools/local/build_ttk_fonts.py`, `ttk_font.h`): sets 14
  `level_stats_label` and 15 `level_stats_value`, TTK Medium Italic 1x with a
  shadow, in two project palettes measured from EDuke32 (`STATS_RED`,
  `STATS_BEIGE`; in the local D24A picker as "Stats red" and "Stats beige").
  A complete count is drawn in the letters' red.
- Framework (exported patch): `psx_mod_fps_display` /
  `psx_mod_set_fps_display` (`mod_plugins.h`, `main.cpp`, applied on the
  frontend frame); `host_osd_stats_image` / `host_osd_stats_rect`
  (`host_osd.c`), drawn by the GL and SDL presenters under the D39 tint like
  the HUD. Placement: 720-line units of the presented picture; left edge on
  the health box (10 of 512 screen units across the 4:3 picture from the
  picture's left edge, where D14 keeps it in widescreen); bottom 17 lines over
  the HUD's top (line 209 of 240), or 10 lines over the bottom with STATUS
  BARS HIDDEN.
- Profile schema 63: `level_stats` (true) and `fps` (false); `run.py
  --level-stats` / `--fps`; terminal V and Y; Vanilla gets
  `DNTTK_LEVEL_STATS=0` and no `PSX_FPS_TELEMETRY`.
- `near_clip.cpp`: `near_clip_enemy_extras_word`.

Swatches (local): `recomp/analysis/d20i/swatches.png` (A title red and disc
gold, B the EDuke32-matched pair (built), C title red and beige, D stats red and
disc gold), beside `eduke32-reference-crop.png`.

## Evidence

`recomp/analysis/d20i/probe.py` (private fixture copies, offscreen, player
files hash-checked unchanged after every run).

| Check | Result |
| --- | --- |
| Slot 10 load, 16:9 borderless | `T: 6:24.36 K: 13/44 S: 1/5` above the health icon; 13 + 31 left = 44 |
| 16:9 full status bars | above the health box |
| 4:3, STATUS BARS HIDDEN | at the bottom-left edge |
| Level 9, `spawn enemy 1013` (moves a placed row) and killed | `0/30` -> `1/34`: kills 1, four type-95 spare rows live |
| Save slot 7, `level 9` again, load slot 7 | `1/34` -> `0/30` -> `1/34` |
| `endlevel` | ENEMIES KILLED 1 OF 34; no LEVEL STATS over the statistics screen |
| Level 5, dancer 922 killed | kills 1, no reinforcements, total 59 |
| Levels 27, 21 | `K: 0/1 S: 0/0`, `K: 0/18 S: 0/0` (0/0 red) |
| Pause | hidden |
| DISPLAY rows | drawn; LEVEL STATS OFF + FPS ON confirmed: lines gone, FPS block shown; console `fps` turned it off and the row read OFF; private profile saved `level_stats: false, fps: false` |
| Console `fps` alone | saved `fps: true` |
| Vanilla | original HUD, no LEVEL STATS |
| Tests | Python 227: all pass except the known `test_primitive_trace`; `test_ttk_fonts` (16 sets), `test_player_profiles` (schema 63, new LevelStatsOptionTest) |
| Framework patch | exported; applied to a clean archive of pin `08ec704a`, 78 files byte-identical |

## Playtest round 1: steady text

User (2026-11-03): "incredibly legit, looking excellent, but ... has some kind
of small amount of motion to the text". Cause: the presenter samples the
overlay nearest at a fractional scale (720-line units on the window), and the
image's width followed the text. TTK's digits are proportional, so every
change of the time changed the width, the scale and which pixel columns were
doubled: every letter, `T:` `K:` `S:` included, redrew slightly differently
several times a second, and characters after a changing digit slid sideways.
Fix (`game_menu.cpp`): a fixed 160-pixel canvas (constant scale) and digits in
cells of the widest digit (`stats_draw_value`), as tabular figures. Seen
(`t15`, seven frames 0.3 s apart): the letters, the K and S lines and the
time's minutes and colon are pixel-identical; only the ticking digits change.
Executable `01970d99ddfebeba77d9f3209a5790d656c95ca823e4494cb8b5788da867b583`.

## Limits

- Not run by the agent at 120 or 180 Hz (the image is cached and redrawn only
  when the text changes, at most once per game tick).
- The scripted reinforcements of levels 1, 2, 7 and 8 were not triggered in
  play; they use the same spare rows.
- Two-player games show no LEVEL STATS.
- The D43 level title can still show faintly over the statistics screen if a
  level ends within its three seconds (seen with `endlevel` right after a
  load; D43's own publish, not changed here).
