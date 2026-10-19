# D19I - In-game menu text at full brightness

Job: [D19I](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "it's working as intended").
Follows [D19H](147-d19h-pause-menu.md) (pause and death lists) and
[D19](145-d19-game-menu.md) (OPTIONS drawn by the game's own calls).

## Problem

User at D19H acceptance: the in-game menu text "is dimmed and needs a touch
of work to make it look right". Our pause list, death list, briefing, YES /
NO prompts and the pause OPTIONS screens were drawn at about half the
original pause text's brightness, while the same calls on the title were
full brightness. Duke was also drawn in front of the list.

## Cause

Gameplay frames are drawn in two chains:

1. The view composition `0x80026164` (world, HUD), closed by `0x8001fba0`
   right after it (return `0x80026904`).
2. While paused (`0x800be568`), the main loop calls `0x80025f28`, which
   puts a full-screen darkening quad (`0x800be4f8`) into a new chain at depth
   1 (`0x8002bc18`). It then calls the pause dispatch `0x80025fb0`, where the
   original pause and death handlers draw their text (depth 0). The flip
   `0x8001fcbc` closes this chain after the world's.

Our pages were drawn at the composition entry (`game_menu_frame`, from D19),
so their glyphs went into the world's chain: under the darkening quad and
behind nearer world polygons. Glyph prims are opaque textured quads with
colour 0x80 (`0x8008b210`), so the dimming came only from the quad.

## Change

`recomp/src/ttk/game_menu.cpp`:

- In gameplay (mode 1) the page is drawn in the pause-dispatch tick, after
  the tick's actions, so it lands in the second chain after the darkening
  quad, as the original's text does. The title (mode 0) still draws at the
  composition, unchanged.
- A final choice (resume, continue, restart, quit) draws the page before
  acting, so the image of that tick still shows the list, as the original
  handler draws its screen in the tick that acts on the press. Without this,
  one image of the darkened world without the list or HUD showed after
  RESUME (two presents at 120).
- Pages are drawn in gameplay only while the game is paused. The tick hook
  `0x800220f4` also runs during level loads in mode 1 (level start
  `0x800257c8`).
- A savestate-reopen of the list is skipped right after our own resume: the
  level start that follows the death screen's RESTART LEVEL runs the tick
  with the old screen value 9, unpaused, before the composition clears it.

No hooks, guards or framework files changed.

## Evidence

Offscreen on the real GPU, 16:9, 4x, private profile, cards and savestates
(`recomp/analysis/d19i-bright`, copied from D19H; `b.py`, `m.py`, `p.py`,
local). Executable
`f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c`.

Brightest tenth of the orange text pixels (mean RGB):

| Screen | Before (D19H) | After | Original |
| --- | --- | --- | --- |
| Pause list | 135, 101, 42 | 255, 187, 68 | 255, 176, 34 (pause briefing) |
| Mission briefing | 135, 95, 25 | 255, 176, 34 | 255, 176, 34 |
| YES / NO prompt | 135, 97, 29 | 255, 179, 44 | - |
| Pause OPTIONS | 135, 101, 42 | 255, 187, 68 | - |
| PC CONTROLS (settings) | 134, 98, 26 | 255, 177, 37 | - |
| Death list | - | 255, 187, 68 | 255, 180, 46 |
| CONFIRM CONTINUE | - | 255, 187, 67 | 255, 187, 68 |

- The paused world is darkened as before: mean colour of two text-free
  regions identical in the original, D19H and D19I images.
- 120 Hz, 24 presents each: opening the pause list, moving, the briefing,
  OPTIONS and the death list show no image without the text. RESUME shows
  the list, then gameplay (no empty image). The death screen and the pause
  menu present repeated real images (no redraws), so text drawn after the
  D17 capture is never lost.
- D19H checks rerun on this executable, same results as D19H: list walk
  (briefing, OPTIONS, RESTART / MAIN MENU prompts and Escape back), RESUME
  (Escape and Enter), RESTART LEVEL, MAIN MENU (mode 3), LOAD and back, death
  CONTINUE, LOAD and back, RESTART LEVEL (level restarts, Duke at the start
  with 100 health), QUIT (game over, then title), challenge stage 1 (REPLAY
  3 -> 2 continues), title OPTIONS (MODERNIZED, SOUND, back to the main menu;
  text unchanged at 255 red), Vanilla (original pause screen 12, Cross
  resumes).
- Python 146 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`.

Test note: the D19H death RESTART check read the state about 25 s after
YES. The level does restart (loading spinner, Duke at the start with 100
health), but the idle test Duke is killed again by nearby enemies within about
20 s, so that late check depends on timing. `p.py deathrtrace` checks the
first seconds instead.

## Limits

- Prompts still show PlayStation buttons (D19F).
