# D17T - Smooth title menu and attract demo

Job: [D17T](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-18). Follow-ups: D17U (loading screens), D19C (menu responsiveness), D19D (Esc and menu navigation).
Related: [D17 audit](80-d17-high-refresh-audit.md), D19B (responsive modern
menus; this is an interim improvement before our own menu system).

## Why the frontend looked slow

The frontend is game mode 0 (`0x800bcbb0`; table `0x800bcbb4`, 16 bytes per
mode: mode 0 tick `0x80022324`, mode 1 tick `0x8002666c`). It covers the
title, the attract demo behind PRESS START, the main menu, options and load
game. The intro FMV runs in mode `0xffff` and is not affected.

- The flip `0x8001fcbc` measures fields since the last flip (`VSync(-1)` minus
  `0x800bdbd8`) and waits until a minimum: **6 fields in mode 0** (or when
  `0x800bda74` is set), 2 in gameplay. The frontend therefore produces 10
  images a second by design (measured 9.6-10.0 by polling the display buffer
  index `0x800bdbd0`), not because the CPU is short.
- D17 render replay only ran in gameplay (`gameplay_context()`, mode 1, not
  paused), so each frontend image was shown about 6 times at 60 Hz.
- The frontend tick calls the same view composition `0x80026164` (from
  `0x800234cc`, return `0x800234d4`), its tail always reaches the flip
  (`0x8002352c`), and the flip closes the drawing chain itself (`0x8001fba0`
  with return `0x8001fce0`).
- The pad is read once per tick, so presses shorter than a tick are often lost
  (title menu, debug route: 3-field Down taps moved the cursor 1 time in 8,
  6-field presses 8 in 8, one step each; 12-field holds sometimes two steps,
  the original repeat). This is much of the menu "clunk".

## Rejected: run the frontend at 30 Hz

Developer experiment (removed): present mode 1 to the flip's minimum-field
check in the frontend (hooks on `0x8001fcbc`/`0x8001fa78`). The frontend ran
at 29.6 images a second, but the attract demo desynced at once: Duke's path
differed from the first sample, he stuck on a wall and the demo cut to its
next scene early. The demo is recorded per-frame input, so the guest's 10 Hz
pace must stay. The hooks were removed and the code regenerated (codegen hash
`0x8bab543c` throughout).

## What changed

**Interpolated frontend images** (`recomp/src/ttk/frame_replay.cpp`). A
composition with return `0x800234d4` in mode 0 is captured like a gameplay
frame (`Frame::frontend`):

- Workers start, and the present timeline runs, while a frontend image was
  composed within the last 15 fields (`frontend_live()`), as well as in
  gameplay.
- The worker keeps the published return address, so it runs the frontend
  tick's own drawing tail up to the flip (menu text, logo, PRESS START).
- No late camera: the game owns this camera, so redraws never take the mouse
  direction (this also guards against a stale gameplay orbit after quitting
  to the title).
- World delay: one measured frontend interval plus the late lead plus 8 ms
  (about 122-128 ms; gameplay keeps 90). The shown content is then
  consistently about 125 ms old, where the original showed each image 100 to
  200 ms after it was composed, so the menu does not respond later on average.
- The timeline resets when images switch between frontend and gameplay, or
  after a frontend gap over 15 fields (a demo scene loading).
- `DNTTK_FRONTEND_INTERP=off` (developer) keeps the original 10 Hz images;
  `DNTTK_FRONTEND_DELAY_MS` (developer, 60-250) fixes the delay.
- Debug: `render_replay` -> `plugin.frontend` (`live`, `captures`, `resets`,
  `interval_ms`, `delay_ms`, `input.held_taps`).

**Frontend tap hold** (`recomp/src/ttk/frontend_input.cpp`, called from the
framework's `capture_pad_slot` for player 1 after `input_pad_context`).
Modernized, mode 0 only: each new press is held for at least 6 fields from
its start, so the tick always reads it once. Longer holds, gameplay, Vanilla
and netplay are untouched. Covers keyboard and controllers.
`DNTTK_FRONTEND_HOLD=off` (developer) disables it.

The diagnostic test-input file (`DNTTK_TEST_INPUT`) also accepts `up`, `down`
and `enter`.

## Evidence

Executable `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`;
offscreen on the real GPU, private profile and blank private cards, a private
savestate of the attract demo (`recomp/analysis/d17t-frontend/`, local:
`fe.py`, `trace.py`, `seq.py`, `long.py`, `menu.py`, `taps2.py`).

| Case | Guest images/s | Distinct presents/s |
| --- | --- | --- |
| Demo, before (120) | 9.9 | 9.9 |
| Demo, after, 120 | 9.6-10.0 | 118-119 |
| Demo, after, 180 | 10.0 | 179 |
| Demo, after, 240 | 9.9 | 239 |
| Title menu, after, 120 | 9.4-9.9 | 118-119 |
| Rate 60 / Vanilla | 10.0 | replay off (original) |
| Rate 30 | 10.0 | 10 (unique images) |

- 90 s of demo at 120: about 120 distinct presents a second, 0 failures, 0
  repeats, 0 camera cuts; the scene change reset the timeline once, and the 2D
  screens between scenes stay at their native rate. The present trace steps
  the blend evenly (12 presents per image, 0.08 to 1.00).
- Demo logic is untouched by construction (the guest never sees a replay).
  Duke's position, sampled once a second for 40 s from the demo savestate,
  with interpolation at 120 against the build before D17T: 32 of 40 samples
  identical; the other 8 were read 1-5 fields apart while Duke moved and lie
  on the same path. Two runs of the earlier build were identical sample for
  sample, so the demo replays deterministically from the savestate.
- Starting ONE PLAYER from the menu reaches gameplay (mode 1) with the normal
  gameplay replay; frontend capture stops.
- Keyboard taps (test input) on the title menu, Down:

| Tap | Hold off (original) | Hold on |
| --- | --- | --- |
| 45 ms | 7 of 12 | 12 of 12 |
| 80 ms | 11 of 12 | 12 of 12 |
| 200 ms | - | 6 of 6 (one step each) |

- Framework patch exported; a clean pinned checkout plus the patch matches all
  47 framework files. `ttk-input-test` passes. Codegen hash unchanged.

## Limits

- Menu text and the highlighted entry still change 10 times a second (they
  are part of the game's image); only motion between images is smoothed.
- Interpolation adds a fixed display delay of about 125 ms in the frontend
  (see above for why the average age does not grow).
- Not judged on the user's display yet; Match Display was not measured
  offscreen (it uses the same path as the fixed rates).
- Options, load game and other frontend screens share the path but were not
  individually inspected.
