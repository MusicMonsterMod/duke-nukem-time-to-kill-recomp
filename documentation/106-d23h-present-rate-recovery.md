# D23H - present rate recovery after hitches

2026-10-06. Modernized, fast CPU timing, 150%, 120 Hz (the D23F default).

## Report

After about 10 minutes of play the picture rate fell from 120 to what looked
like 60 and did not come back. The session logs tied every slowdown line to a
savestate save or load from the F7 menu.

## Cause (measured)

Two faults combined:

1. **A shed was permanent at fixed and display frame rates.** D23E's
   `replay_shed_load()` raised the present divisor (every 2nd, then 3rd
   refresh) and relied on `late_pace()` to step back up. The default present
   timeline (`timeline_provider`) calls `late_pace()` only at Unlimited
   (`target_hz < 0`), so at 120 Hz or Match Display nothing ever lowered the
   divisor again. The new per-minute log showed `late_pace` never ran at 120 Hz.
   The `pace_backoff` growth suspected when the job was opened played no part.
2. **One-off hitches counted as load.** `overclock_lease()` shed on any single
   second under 57 host frames. A savestate save stalls the emulation thread
   80-100 ms (measured with frame-counter gaps: saves 83-98 ms, loads 36-57
   ms), which alone brings a second to about 54 frames. A second containing a
   short F7 menu visit is much lower (the player's logs: 12-26 frames/s).
   Two such seconds per session gave 40 presents/s until restart; once at
   every 3rd refresh, further hitches paused the overclock 5 s at a time.

No slow time-based decline was found. The 21-minute run below held 120 presents/s
with flat memory (see Results).

## Fix (game code only)

`replay_load_window()` (`src/ttk/frame_replay.cpp`) now owns load shedding in
both directions, judged once per gameplay second by `overclock_lease()`
(`src/ttk/modern_controls.cpp`), the same in every frame-rate mode:

- Seconds are kept up, behind (under 57 frames), or not judged (3 s or more
  between player updates: menus, loads; and seconds while the overclock is
  paused).
- Shed only after **two behind seconds in a row**. A save, load, menu visit
  or worker start is one bad second and sheds nothing.
- While shed, after 5 s x backoff clean seconds, try the next faster rate. If
  it falls behind within 10 s the probe failed: shed again, backoff doubles
  (at most 16, one probe every 80 s in a scene that cannot hold the rate).
- Backoff halves after each clean minute at the full rate.
- The overclock pause (audio safety net) still follows when nothing is left
  to shed, on the same two-second rule.

Unlimited keeps `late_pace()` for repeats; the shed controller tracks only the
divisor steps it added.

Logging (session log, cheap): `[TTK pace]` on every present-rate change with
its reason, and one summary per minute (presents/s, divisor, backoffs, mixed
windows, changes, RSS). `render_replay` reports `shed_steps`, `shed_backoff`,
`shed_probes`, `shed_failed_probes`, `pace_backoff`, `pace_clean`.

Not changed: savestate saves still take 80-100 ms on the emulation thread
(compression and file write in the framework). That is a single short hitch at
an explicit save and no longer affects the present rate. Moving it off the thread
would be a framework change; not needed for this job.

## Results

Harness: `recomp/analysis/d23h-20261006/` (`h.py` from D23F with `NOTRACE`,
`long.py`, `stall.py`, `hitch.py`, `recover.py`, `recover2.py`). Offscreen real
GPU, private copy of the player's profile (Modernized, first person, fast,
150%, 120 Hz), private savestate copies (slots 1, 9, 10), private port 9273.
One-off hitches are made by stopping the game process (SIGSTOP) for 0.8 s;
sustained load by stopping it 60 ms of every 100 ms.

| Run | Build | Presents/s by 10 s |
| --- | --- | --- |
| 150 s, 0.8 s stop every 20 s, save every 30 s (slot 1) | before | 120, 114, 60, 55, 40, 38, 40 ... 40 (every 3rd from 40 s; 5 overclock pauses) |
| same | after | 120, 114, 119, 111, 120 ... 120 (no rate change) |
| 130 s, save every 30 s (slot 1) | after | 120 throughout |
| 15 s sustained load, 40 s free, twice (slot 1) | after | load: 40 (every 3rd); free: 40 -> 60 -> 120 about 15 s after the load ends (5 s overclock pause included), both cycles |

| 21 min (slot 1): save every 60 s (20), load every 150 s (8), 0.8 s stop every 90 s (13) | after | 120 in 124 of 125 ten-second blocks (lowest per-second 24 during a stop); one shed at 721 s, where a stop and a save fell in the same second, back to 120 after 5 s |
| 90 s, 0.8 s stop every 30 s, save every 20 s (UI slots 9 and 10, western town) | after | 119-120 with no rate change in both |
| 90 s Unlimited, same hitches (slot 1) | after | only the unchanged repeat-based `late_pace` acted (14 changes, all repeats); no emulation shed |

Save and load stalls (`stall.py`, slot 1): saves 83-98 ms, loads 36-57 ms of
no game fields.

Memory over the 21-minute run: 400 MB after load, 466 MB at the end (about
3 MB a minute, steady). The present rate was unaffected. Not investigated
further; worth a look in a longer session (D23).

Native tests: `ttk-input-test`, `ttk-aim-test`, `ttk-near-test`,
`ttk-controls-test` (with the D23E fixtures) PASS; the controls test stub is now
`replay_load_window`. Profile and hook-contract unit tests (60) PASS.
Executable `c6c221d16534b2dceedb631aa263939b545f1697ceb7c6a38d3db365398681e6`.
No framework file or hashed header changed: savestates load as before.
Vanilla does not reach this code (the lease and the provider run only in
Modernized).

## Limits

- Offscreen measurements with simulated hitches (process stops), not the F7
  menu itself. The user's long play session is the acceptance test.
- Unlimited's repeat pacing (`late_pace`, `pace_backoff`) still never decays.
  At several hundred presents per second its windows are short, so recovery
  takes seconds rather than minutes. Unchanged here.
- A genuinely busy scene that cannot hold the faster rate now tries it every
  5 s at first, then 10, 20, 40, 80 s; each failed try costs about 2 s at the
  faster rate before it sheds again.
