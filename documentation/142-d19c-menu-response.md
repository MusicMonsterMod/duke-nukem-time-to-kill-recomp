# D19C - More responsive original menus

Job: [D19C](../MODERNIZATION_JOBS.md). Status: Accepted (2026-10-18) for demo-backed menus; the user found menus over the loading spinner less responsive, continued in D19E.
Related: [D17T](140-d17t-frontend-smoothness.md) (frontend in-betweens, tap
hold), D19D (Esc and menu navigation), D19/D19B (our own menus later).

## What limits the menu

- The frontend tick `0x80022324` dispatches on the state `0x800d24e4`
  (table `0x800131b0`, 13 states). Seen: 5 = PRESS START over the attract
  demo, 6 = opening the menu, 7 = main menu (cursor `0x800d24e6`, 0-3),
  8 = starting ONE PLAYER, 10 = LOAD GAME. The state handler reads the pad
  copy made at the start of the tick (`0x8001d5ac` copies the BIOS pad buffer
  `0x800d1610`), then the common tail updates the demo world (sub-state
  `0x800d24e2` = 2), composes the view (`0x80026164`) and flips.
- **The attract demo keeps playing behind every menu screen** (the main menu
  and LOAD GAME were checked). It is recorded per-frame input and desyncs when
  the frontend ticks faster (D17T), so the 6-field tick (10 a second) cannot
  change while a menu is open. The flip `0x8001fcbc` measures real fields and
  stores the delta (`0x800d21fc` = 5 per field, `0x800d2200` Q12 seconds), but
  per-tick counters (for example `0x800d24d0`) also exist.
- Measured on the main menu at 120 with the D17T build (45 ms keyboard Down
  taps at random phases, 12 taps): **220 ms from press to screen on average**
  (176-267). The guest reacts 1-7 fields after the press (one tick, about
  88 ms including the tick's own work before its composition); the D17T
  interpolation delay then added a constant 131 ms, because each in-between
  was drawn from the older image of its pair, so a new menu image appeared
  only when the timeline reached it.

## What changed

`recomp/src/ttk/frame_replay.cpp`: in the frontend, each in-between is
drawn from the **newer** image of its pair, with the camera, Duke and objects
blended back toward the older image's poses (`submit_job`: base and target
swapped, alpha reversed, the newer image's snapshot slot, transforms and
draw-time state). World motion is the same interpolation as before; the menu
text, highlight and screen changes come from the newest image, one image
interval (about 100 ms) sooner. The guest, its pace and the demo are
untouched (the guest never sees a redraw). Jobs stay in the older frame's
list and are released with it, before the newer frame's slot is reused.
Loading images (D17U) and gameplay are unchanged.

`DNTTK_FRONTEND_NEWEST=off` (developer) restores the D17T base.

Diagnostics: `render_replay` -> `plugin.frontend.newest` and `.menu`: per
menu change (state and cursor, `from`/`to`), the guest fields and
milliseconds from the press to the new image, and to the first present that
shows it (`screen_ms`; last 16). `frontend_input.cpp` records the host time
and field of each new frontend press for it.

## Evidence

Executable `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828`;
offscreen on the real GPU, private profile, blank private cards and the
private attract-demo savestate (`recomp/analysis/d19c-menus/`, local:
`lat.py`, `chk.py`, `go.py`, `van2.py`).

| Main menu at 120, Down taps | Press to screen (mean) | Range | Guest part |
| --- | --- | --- | --- |
| D17T (`DNTTK_FRONTEND_NEWEST=off`) | 220 ms | 176-267 | 88 ms |
| D19C | 124-126 ms | 71-171 | 88 ms |
| D19C, 24 taps (seed 7) | 119 ms (last 16) | 73-168 | 79 ms |

- 24 of 24 taps moved the cursor exactly one step (no missed or doubled
  inputs). Confirm on LOAD GAME: 95 ms to the new screen. PRESS START to the
  menu: 121 ms.
- Smoothness unchanged: demo 118.6 and menu 119.1 distinct presents a second
  at 120 with the guest at 9.9-10.0 images; LOAD GAME screen clean.
- ONE PLAYER from the menu reaches gameplay (29.4 guest images, 118 distinct
  presents a second).
- Rate 60 and Vanilla: no redraws (0 presents from the replay), guest 10
  images a second, the menu works (Vanilla driven through the pad route).
- `ttk-input-test` passes. No hooks or framework files changed; codegen hash
  unchanged.

## Limits

- The guest still reads the menu once per tick (10 a second), so up to one
  tick of waiting remains; it cannot shrink without changing the demo behind
  the menu. The remaining screen part (about 35 ms) is the redraw lead and
  margin.
- Menu text still changes 10 times a second.
- Options and the two-player screens were not individually inspected; they
  share the path.
- User playtest: more responsive while the demo plays; less responsive while
  the loading spinner plays (D19E).
