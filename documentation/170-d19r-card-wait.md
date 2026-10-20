# D19R - Startup card wait at host speed

Job: [D19R](../MODERNIZATION_JOBS.md). Status: Done (2026-10-20; the user delegated
testing and closeout: "you can easily test that yourself and close it out").
Follows [D19K](169-d19k-load-game.md), which hid CHECKING MEMORY CARD / CHECKING
SAVE GAME FILE and skipped the original's waits but kept the card work.

## The problem

After the intro movies (or Escape), Modernized showed about 1.5 s of black
before the title. Frontend state 1 probes the card (`0x80082e6c(0,1)`) and state
2 reads the save file (`0x80083490`); both block the game's main loop inside
the BIOS card driver, which advances once per VBlank. Measured: state 1 = 90
fields, state 2 = 25 fields, paced at 59.94 Hz, nothing new presented. The
guest PCs were the kernel loop at `0x3530` / `0x354c` polling the SIO
registers.

## The fix

Run those VBlanks without wall-clock pacing, and let the framework's idle skip
fast-forward the poll loop while they run. The guest sees exactly the same
fields, interrupts and card transfers; only host waiting is removed.

- Framework (`mod_plugins.h`, exported patch): `psx_mod_request_unpaced(max_vblanks)`
  opens a bounded window (at most `PSX_MOD_UNPACED_MAX` = 600 VBlanks; a new
  request replaces the budget, 0 ends it). Each VBlank in the window takes the
  turbo-through-loads path (no pacing, 1 present in 30) at host speed whatever
  the load multiplier, and `g_idle_skip_window` turns the existing idle skip on
  (`psx_cycles.c`, `interrupts.c`) for that window only. Netplay and the resim
  self-check never enter it. `psx_mod_unpaced_vblanks()` counts the VBlanks run
  so. No hashed header changed: codegen hash stays `0x8bab543c`, savestates
  load.
- TTK (`game_menu.cpp`, `title_entry`): while the frontend is in state 1 or 2,
  each tick renews a 300-VBlank window (well above the probe's 90; it also
  bounds a wait that never returns); any later state ends it. Modernized only;
  Vanilla never requests it. `render_replay.game_menu.card` is now
  `[skips, hidden texts, unpaced VBlanks]`.

Rejected options (research in the job): keeping the title image up during the
wait (the screen is already black in state 0, right after the movies, so the
gap reads as one transition once it is short); moving the probe after PRESS
START (the original orders it before the settings are needed); reading the
settings from the card file on the host (an HLE of the game's check).

## Evidence (offscreen, private profile and cards, `recomp/analysis/d19k-load/light.py`)

| Run | State 1 | State 2 | Notes |
| --- | --- | --- | --- |
| Before (D19K, research) | 1.50 s (90 fields) | 25 fields paced (about 0.4 s) | paced |
| Unpaced only | 0.40 s (86 fields) | 0.09 s | about 220 fields/s |
| Unpaced + idle skip, Modernized x3 | 0.16-0.17 s (75 fields) | 0.08 s (25 fields) | 121 unpaced VBlanks; 5144 idle skips, all inside the window (count unchanged afterwards); saved settings loaded (`card_loads 1`) |
| Blank card | - | - | state 1 to 3 to PRESS START at once (13 unpaced VBlanks) |
| Fresh (new) card | 0.17 s | 0.05 s | no save file, title reached |
| Vanilla | 1.70 s (102 fields) | 0.74 s (44 fields) | paced, texts shown, 0 idle skips: unchanged |

Black from the movie's end to the title is now about 0.35 s (state 0 about
0.12 s, states 1-2 about 0.24 s), down from about 2 s. The title appears
normally afterwards (screenshot checked).

Tests: Python 160 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`; a
pristine pinned framework plus the exported patch reproduces all 47 modified
framework files. Executable
`2fb1bd3cc0e8ee78fd94c41da6c66c39a5fe7d5ff4eddd813e321bc2f2833408`.

## Limits

- A short black stretch remains (the guest still runs 100 card fields; host
  speed in the window is about 450-600 fields/s).
- Not judged by the user on their own machine yet; their real card takes the
  same path (the probe reads the same directory).
