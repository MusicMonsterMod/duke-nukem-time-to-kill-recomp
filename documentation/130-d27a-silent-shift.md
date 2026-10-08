# D27A - Modern Shift/run is silent

Status: **Done** (user-accepted 2026-10-08: "i fully accept this. great work. i have wanted this one for a long time so this minor change has a huge impact on me as a player."). Modernized only; Vanilla unchanged.

User request: in Modern controls, pressing and releasing Shift to run played a
click. Shift should be silent (hold -> run, release -> walk), while the D08A10
steroids heartbeat, which plays the same sound on purpose, keeps working.

## The original toggle

Sound `0x0001` (bank 0, SPU sample `0x012F0`, pitch `0x228`-`0x22F`; see
[note 129](129-d08a10-steroids-heartbeat.md)) comes from the player update
`0x800412A4`, in its ground branch (`player+0x22c` == 0, `+0x224` bit `0x40`
clear), after the analog-stick path (`0x800417f0`, which never clicks):

- `0x8004181c`: the run button event `e` is 2 unless `0x800bda74` == 0 and
  `0x800bcbb0` != 0, then `e = word(0x800bda8c + 4*player[0x233] + 0x90)`.
  1 = held, 2 = not held; 0 is the original's toggle-mode press.
- `mode = word(word(0x800d1cf0 + 4*player[0x233]))` (controller config).
- `e == 1`: run when `mode & 1`, else walk. `e == 2`: the reverse.
  `e == 0` with `mode & 3 == 1`: flip.
- The previous state is `s2 = player+0x27b` (read only here, at `0x80041850`).
  When the new state differs and `+0x224 & 0x241` is clear, the code calls
  `0x8006bbd8(1)` (jal at `0x800418cc`, `0x80041934`, `0x80041994`,
  `0x800419c8`), then sets or clears `+0x224` bit 1 and stores the bit back at
  `+0x27b` (`0x800419ec`).

Modern Shift holds the original run button (`pads[walk]`, L1), so each press
and release changed the state and clicked. Vanilla's own L1 toggle (Q on the
keyboard) clicks the same way.

## Fix

`recomp/src/ttk/run_click.inc`. A new entry hook on `0x800412A4` (one
regenerated line in `game.local.toml`; codegen hash `0x8bab543c` unchanged,
savestates load) runs for Duke only (`a0` == player). In Modernized, with player
identity ready, it computes `e`, `mode` and the state the toggle is about to
store, exactly as above, and writes it to `+0x27b` first. The original then sees
no change: no click, and it still sets `+0x224` bit 1 itself, so running is the
original's own. Toggle-mode presses (`e == 0`), the analog path and every other
state are left alone. The heartbeat's own `0x8006bbd8(1)` call
(`steroids_beat.inc`) does not go through the toggle and is unaffected.

- Guard: `control_guards.inc` hashes the whole toggle region
  `0x8004178c`..`0x800419f0` (612 bytes), so identity fails closed if the code
  differs.
- Vanilla: `player_identity_ready()` requires Modernized; the hook does nothing.
- `DNTTK_RUN_CLICK=original` keeps the click (diagnostics).
- Debug: `ttk_input` -> `controls.run_click` (`silent`, `syncs`, `events`,
  `last_event`).

## Evidence (executable `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`)

Private Xvfb runs, private profile, a copy of the test cards, 60 fps
(`recomp/analysis/d27a-run-click/`, local). Each leg from a fresh savestate
load. Counts are SPU KEYONs of sample `0x012F0`.

| Leg | `DNTTK_RUN_CLICK=original` | D27A |
| --- | --- | --- |
| Idle, 2 s | 0 | 0 |
| 5 Shift taps, standing | 10 | 0 |
| Shift held 3 s, then released | 2 | 0 |
| Walking, Shift taps (3-4) | 8 | 0 |
| Walk 1.2 s (W) / run 1.2 s (Shift+W), slot 3 | - | 667 / 4093 units, 0 clicks |
| `dnhyper`, no Shift, slot 4 | 19 beats, 15/15/18 fields | 19 beats, 15/15/18 fields |
| `dnhyper`, 5 Shift taps, slot 4 | 29 (beats plus 10 clicks) | 19 beats, 15/15/18 fields, no skips |

- Heartbeat pitches stay `0x228`-`0x22E` (same sample and spread as D08A10).
- Vanilla (Q = L1, Up = forward): 5 Q taps 10 clicks, Q held 2, run leg 2,
  walk with Q taps 6; `run_click` events and syncs 0. Unchanged by design.
- Suites: `ttk-controls-test` (PS-X EXE, LEVEL00 fixture, LEVEL01, all
  levels), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK (2 skipped),
  `level_overlay_guards.py --check`, `check_repo.py`.

## Limits

- Caps Lock autorun drives the same button and should be silent too; not
  measured separately.
- The gait plant footsteps `0x2000` / `0x2001` (D27's old "clunk") are a
  different sound and stay as they are.
- The original's toggle-mode press (`e == 0`, config `& 3 == 1`) still clicks;
  Modern Shift is a hold and did not reach it (no Shift press clicked in any
  measured leg).
