# D43 - Level title card

2026-11-03. Status: Done (user-accepted 2026-11-03: "It's superb"; "I really like the red text"; locked in after the font tweak below).

## Request

User (backlog intake 2026-11-03): when a level begins, show its name
prominently near the centre, as Duke 3D shows "HOLLYWOOD HOLOCAUST", briefly,
fading by itself, never interrupting play, in first and third person.
Reference: `research/screencaps/Video_2026-11-03_20-09-20.mp4` (and the D42
reference, where EDuke32 shows it again after a load).

Decisions (user, 2026-11-03): show it on a new level, RESTART LEVEL and a
savestate load, not after CONTINUE; red, Duke 3D style.

## Where things are

- Level names: the game's own string ids per level at `0x800c3d2c`
  (guarded), strings through the text table `0x800c60f4` (the same names the
  pause menu, LOAD GAME and the statistics screen show).
- Level entry: gameplay is `0x800bcbb0` = 1 with phase `0x800ce1b0` = 2.
  A level load (seen: console `level N`, RESTART LEVEL; expected the same:
  the next level after the statistics screen, the challenge stages' REPLAY)
  leaves and re-enters that state; CONTINUE respawns in place and does not. A savestate
  load is counted by `psx_mod_savestate_loads()`.

## Change (Modernized only)

- `recomp/src/ttk/game_menu.cpp` `level_title_tick` (once per gameplay
  composition, from `game_menu_frame`): on entering play, or after a
  savestate load into play, the level's name (upper case) starts a card that
  lasts as EDuke32's map name does: 85 tics at 30 Hz (2.83 s), the last 32
  (1.07 s) fading. Its clock runs only while play runs; pause, menus and
  other screens hold it and hide it.
- `ttk_level_title_image` renders the name with a new TTK font set 13
  `level_title` (user, 2026-11-03): TTK Big Italic from the disc, 1x,
  spacing -1, with a shadow, in a red ramp (`tools/local/build_ttk_fonts.py`
  `RED`, a project palette: the Console steel steps in red, not the disc's
  CLUT 227) over a near-black shadow (`#140000`), alpha scaled for the fade.
  The palette is in the D24A font picker as "Title red (project)"
  (`recomp/analysis/d24a-fonts/make_picker.py`, local).
- Framework (`host_osd.c` `host_osd_title_image`, `gpu_gl_renderer.c`, the
  SDL presenter path): drawn in the game's screen pixels, as the pause menu's
  text is (the game's screen width, `0x800d2210` +4, across the 4:3 picture,
  240 lines down), so it is the menu heading's size: centred horizontally,
  its top just under the crosshair's lower tip (`host_osd_title_top`), as in
  EDuke32 (user, 2026-11-03: "a touch up" from the first placement; measured
  on the D42 reference frame, 1280x720: crosshair rows 353-368, title top
  370). Without a crosshair it uses EDuke32's crosshair size (15 of 720
  lines). The gap is 2 lines on 720, scaled with the picture. 4:3 and widescreen follow the presented picture.
  It sits under the D39 present tint, like the HUD.

First version (2x in 480-line units, spacing 1) read about 1.8 times wider
than the pause menu's level name; user: "the same size and kerning as the
menu font". Measured on the same 1024x576 frame after the change: title 326
px wide (shadow included), 40 px tall; the pause heading 308 px wide, the same
height. After the placement tweak, on 1024x576: crosshair rows 279-296,
title rows 298-338.

Nothing else on screen is there at that moment in normal play (the top
notices and mission card are at the top, the HUD at the edges); a third-person
Duke stands behind it briefly.

## Evidence

Executable `65ec2d1d1e7e103943d302d630e4751e07ed33d8a794638769ed0baa1e887126`.
`recomp/analysis/d43/probe.py`, private fixture copies, offscreen; player
files unchanged.

- Savestate load (slot 10, first person, 16:9): "FAMILY JEWELS" at 0.3 s,
  fading at 2.5 s, gone at 3.2 s.
- Console `level 8` (third person, 4:3): "HOLY TERROR" 0.6 s after play
  began, gone 2.6 s later. RESTART LEVEL from the pause list: shown again.
- Death, CONTINUE: not shown.
- Pause list opened while it showed: hidden under the menu.
- `test_ttk_fonts.py` (14 sets, set 13 red 2x with its shadow) and
  `ttk-font-test` pass.

## Limits

- 180 Hz not run by the agent (the clock is in host milliseconds,
  independent of the frame rate).
- The next level after the statistics screen and the two-player arenas were
  not run; they use the same entry rule.
- No option to turn it off (none was asked for).
