# D19E - Menu responsiveness close-out (loading spinner, full sweep)

Job: [D19E](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-18: "excellent work").
Builds on [D19C](142-d19c-menu-response.md) (newer-image frontend redraws),
[D17U](141-d17u-loading-screens.md) (loading images) and
[D17T](140-d17t-frontend-smoothness.md) (tap hold).

## What happens while the spinner plays

When a demo scene ends behind an open menu, the frontend tick
(`0x80022324`) loads the next scene through its sub-state `0x800d24e2`:

| Sub-state | What the player sees | What the guest does |
| --- | --- | --- |
| 0 (about 1.8 s) | spinning logo only; **the menu disappears** | the tick's load setup (`0x8002334c`) blocks in the CD load for about 110 fields; the blocking load loop draws the logo; no tick, no pad read |
| 1 (about 6 s) | the menu over the spinning logo | normal ticks (state handler, then the load branch `0x8002341c` draws the logo and flips), but some ticks are 10-45 fields apart |
| 2 | the menu over the demo | normal ticks |

The menu handler still runs in sub-state 1 and the menu is drawn, but:

- **Taps were lost.** The D17T hold kept a tap for 6 fields. A tick that
  came later than that never saw it: 3 of 9 taps over the spinner, every
  tap during sub-state 0, and 6 of 30 in a mixed run. The tick decodes the
  pad (`0x8001d5ac`) into one history word per button at
  `0x800d1aa4 + 4 * bit`, shifted each tick, bit 0 = pressed at that tick.
- **Screen latency over the spinner was the D17T value.** D17U loading
  images were redrawn from the older image of each pair, so the menu reached
  the screen one image late: 191-220 ms mean press to screen (D19C fixed this
  only for demo-backed images).
- The same hold also lost every tap on **LOAD GAME with a saved game**:
  11 of 11 Cancel taps did nothing. That screen keeps polling the memory card
  and the pad shares the serial port, so pad reads are intermittent. Taps
  there needed about 17 fields to register. The original needs long presses
  there too.

## What changed

`recomp/src/ttk/frontend_input.cpp` (Modernized, game mode 0 only):

- A new frontend press is held until the pad history shows the tick read it
  (the word changed, bit 0 set). The minimum stays 6 fields and the maximum
  is 3 seconds. In ordinary ticks this is the same 6 fields as before. In a
  stall, the press lands at the next tick.
- Taps made while one is still waiting are queued, up to 6. Once the tick has
  read a press, the button is held released until a tick reads that, then the
  next tap is pressed (2 ticks per queued tap). A tap made while the tick last
  read the button as pressed (it never saw the previous release, after a
  stall) waits for a read release first. Otherwise the guest sees no new edge.
- A re-press after a release shorter than 2 fields (contact bounce, a
  flickering input) continues the tap being delivered instead of adding one.
- `DNTTK_FRONTEND_SEEN=off` (developer) restores the D17T 6-field hold.
  `DNTTK_FRONTEND_TRACE` prints each press, queue, read and release.
  `render_replay` -> `plugin.frontend.input` gains `waited_taps`,
  `waited_fields`, `queued_taps` and `dropped_taps`.

`recomp/src/ttk/frame_replay.cpp`:

- Loading images captured in game mode 0 (the menu over the spinning logo)
  are redrawn from the **newer** image of each pair. The logo angle is the
  same interpolation as D17U. Level loads in gameplay keep D17U's path.
  `DNTTK_LOADING_NEWEST=off` (developer) restores D17U's base.
- Diagnostics: the menu latency log keeps 32 entries and tags loading images.
  Its menu signature includes the options sub-screen `0x800d24d2` and an
  optional halfword named by `DNTTK_MENU_WATCH` (developer, hex) for
  sub-menus that have their own cursor.

`recomp/src/ttk/pc_input.cpp` (diagnostics only, never set by `run.py`):
`DNTTK_TEST_INPUT` also takes `left right x z c esc`, and
`DNTTK_TEST_UNCAPTURED` runs the test path uncaptured, so X/Z/C are pad
buttons as they are for a player in the menus.

The guest, its pace, the demo, loading and Vanilla are untouched. No hooks or
framework files changed, so the codegen hash is unchanged.

## Menu map (Modernized keyboard, uncaptured)

Keys: arrows move, **X** = Cross (confirm), **C** = Circle, **Z** = Triangle
(back/cancel/quit), **Enter**/**Esc** = Start. No fixed key reaches Square
while uncaptured. Square is SAVE on the statistics screen and OPTIONS in the
pause menu (D19D).

| Screen | Where | Cursor or marker |
| --- | --- | --- |
| PRESS START / main menu | mode 0, state `0x800d24e4` 5 / 7 | `0x800d24e6` (4 entries) |
| CHOOSE DIFFICULTY LEVEL | state 8 from ONE PLAYER | `0x800c3d7c` |
| TWO PLAYER OPTIONS, CHOOSE CHARACTER | state 8, then 9 | `0x800dd1b0` |
| LOAD GAME | state 10 | blank card: message; with a save: confirm/cancel |
| OPTIONS | state 12, sub-screen `0x800d24d2` 9 | `0x800c3de0` |
| SOUND / CONTROLLER / GAME SETTINGS | state 12, `0x800d24d2` 11... | `0x800c3fac` / `0x800dd274` / `0x800dd230` |
| Pause menu (button-driven) | mode 1, `0x800be56a` = 12 | none |
| Statistics / SAVE GAME | mode 2 | SAVE GAME: `0x800d2672` = 1 |

Options, difficulty and LOAD GAME also play over the attract demo.

## Evidence

Final executable `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf`. Offscreen on the real GPU, private
profile, private cards (`recomp/analysis/d19e-menus/cards`, holding a
private test save made through the level-complete statistics screen),
private savestates: 1 attract demo, 3 main menu 2.5 s before a demo scene
load, 5 gameplay (TIME TO KILL), 6 statistics screen, 7 title after a fresh
boot with the test save. Scripts (local): `latld2.py` (taps through a load,
a poller counts every cursor change), `sweep.py` (per screen), `m2.py`
(mode 2), `ps.py` (PRESS START, smoothness, load timing), `van.py`, `go.py`.
Taps are 45 ms keyboard taps at random phases.

**Over the loading spinner** (slot 3, 30 taps through demo, blocking load,
spinner and demo, at 120):

| Build | Taps landed | Press to screen over the spinner | Over the demo |
| --- | --- | --- | --- |
| D19C (`DNTTK_FRONTEND_SEEN=off DNTTK_LOADING_NEWEST=off`) | 24 of 30 | 191-220 ms mean (142-453) | 110 ms |
| D19E, seeds 1-4 (before the final build) | 120 of 120 | 99-136 ms mean | 105-140 ms |
| D19E final build, seeds 1-3 | 90 of 90 | 100-113 ms mean (42-354) | 110-145 ms |
| D19E at 180 / 60 | 30 of 30 / 30 of 30 | 111 ms / (replay off) | 111 ms / - |

- The guest part over the spinner is 76-98 ms mean. The few highs (220-350
  ms) are taps that met a 45-field tick gap; they now land late instead of
  being lost.
- Taps made while the menu is hidden (sub-state 0) are delivered when it
  returns, one per 2 ticks (up to 7 taps). Before, they were lost.
- Before D17U (`DNTTK_LOADING_INTERP=off`, `DNTTK_FRONTEND_NEWEST=off`),
  loading images were shown as the guest drew them. The same taps were lost.

**Per screen** (16 taps at 120, 12 at 180 and 60; steps exact):

| Screen | 120: landed, guest / screen mean (max) | 180 | 60 |
| --- | --- | --- | --- |
| Main menu | 16/16, 75 / 110 ms (147) | 12/12, 76 / 101 | 12/12, 75 / - |
| Difficulty | 16/16, 92 / 123 (160) | 12/12, 92 / 117 | 12/12, 92 / - |
| Two player options | 16/16, 83 / 122 (174) | - | - |
| Options | 16/16, 88 / 126 (176) | 12/12, 88 / 122 | 12/12, 95 / - |
| Sound | 16/16, 78 / 116 (157) | - | - |
| Controller | 16/16, 94 / 131 (173) | - | - |
| Game settings | 16/16, 78 / 117 (172) | 12/12, 85 / 118 | 12/12, 83 / - |
| Main <-> Options (X, Z) | 12/12 on the right screen, 138 / 123 | - | - |
| Main <-> LOAD GAME with a save (X, Z) | 12/12; D19C logic **1/12** | - | - |
| PRESS START -> menu | 6/6, menu opening on screen 78-151 ms, menu state 155-238 ms (same as D19C) | - | - |
| Pause menu (Esc on/off) | 16/16, 60 ms (72) | 12/12, 65 | 12/12, 62 |
| Statistics screen X | 6/6, 66-95 ms (same with D19C logic) | - | - |
| SAVE GAME cancel (Z) | 6/6, about 50 ms (same with D19C logic) | - | - |

- Over the demo, the menus match D19C (the D19C-logic sweep gives the same
  numbers within a few ms). The pause menu (mode 1) and statistics/SAVE
  (mode 2) tick fast enough that short taps already worked. The hold stays
  mode 0 only.
- No doubled steps anywhere; `dropped_taps` 0 in every run.
- Smoothness: 119.4 distinct presents a second over the demo, 119.2 over the
  spinner (guest 9.9-10.0 images). Demo scene load timing 3.89 / 5.92 s
  versus 3.87 / 5.94 s with D19C logic. ONE PLAYER -> difficulty ->
  gameplay: 29.9 guest images, 119.2 distinct presents a second.
- Vanilla: replay off, menu works with pad presses, 3-field taps still
  partly missed (2 of 5) as in the original.
- `ttk-input-test` passes; local unit tests: 144 run, OK (12 skipped);
  `check_repo.py` OK.

## Limits

- While a demo scene's load blocks (sub-state 0, about 1.8 s), the original
  removes the menu and no tick runs. Presses there land when the menu
  returns. This part cannot be faster without changing the guest's load.
- Menu text still changes 10 times a second, and the guest's own one-tick
  wait remains (D19C).
- LOAD GAME with a save registers presses only when the card polling lets a
  pad read through, about 280 ms on average. They are no longer lost.
- Two-player CHOOSE CHARACTER, the SAVE GAME write itself and the
  memory-card prompts during a real card error were not timed individually.
- Accepted by the user on their display.
