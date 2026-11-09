# D23I - Memory growth in long sessions

Status: Done (accepted 2026-11-09: "i'm playing the game now, all is fine.
lock this in and close it out"). User chose the history-ring fix plus
diagnostics: "we absoltely dont need to be recording audio. yes lets go with
your recommendation".

## Question

The user's 2026-11-09 session logs (`[TTK pace] minute` lines, main process
RSS only) showed 401 to 511 MB in 8 minutes and one session at 402 to 893 MB
in 21 minutes. The 893 MB log has since been rotated out: `run.py` kept five
session logs. The minute line is written every minute in Modernized.

## Method

Offscreen Modernized runs at 180 Hz, first person, Polymorph, from a copy of
the player profile (SDL offscreen on the real GPU, dummy audio, private empty
cards, new game, invulnerable). Scripted input: strafe and turn sweeps, or a
mix of walking, firing and turning. Sampled every 5 s: `smaps_rollup` of the
game and its three redraw workers. Snapshots: full `smaps`, and `pagemap`
present pages of writable private mappings, attributed to `.bss` symbols with
`nm`. A scratch `LD_PRELOAD` heap tracker (a header on every allocation, live
bytes per 7-frame call stack, main process only) measured live malloc memory.
Scratch tools only; nothing is tracked.

## Results

| Run | Scenario | Main RSS |
|---|---|---|
| r1 | 4 min strafing in place, level 0 | 397 -> 493 at 100 s, then 0.5 MB/min |
| r2 | 6 min, savestate load every 30 s | 404 -> 498 at 90 s, then 0.5 MB/min |
| r3 | 8 min, a new level every 40 s | +10-25 MB per first visit, revisits +1-15 |
| r5 | 12 min mixed play, heap tracker | live malloc +0.47 MiB over 9 min while RSS +41 MB |
| r6 / r8 | 7 min mixed, duke3d / ttk monitors | 540 / 547 MB at 410 s |
| r7 | 20 min mixed, 10 levels | 419 -> 589 MB |

- **Not a heap leak.** Live malloc bytes were flat (r5). The growth is in the
  executable's 1.3 GB `.bss` (static arrays), pages being touched for the
  first time.
- **D53 ruled out.** Monitors on `ttk` grew as on `duke3d` (r6, r8).
- **Host-vertex aperture tables** (`gpu.c` `s_hpv_ap_live` /
  `s_hpv_ap_replay`): one 72-byte `HostPV` per word of the 3 MiB GPU DMA
  window, 54 MiB per table. The main process uses two (the emulation thread's
  live table and the render-thread copy's replay table, `render_copy/`), so
  their ceiling is 108 MiB. Near-clip / Polymorph packets go into regions of
  that window from its start each frame, so the resident part follows the
  largest frame so far (a high-water mark): 12 MiB each at 2.5 min, 23 MiB at
  10 min, 23.6 MiB at 20 min (r7).
- **D23's history rings** (note 252): `sio_irq_buf` (40 MiB), `sio_trace_buf`
  (28 MiB) and the audio `s_events` (16 MiB) fill at about 2.2 MiB/min
  together and were 22.6, 19.7 and 7.7 MiB at 20 min. After 10 min they were
  almost all of the remaining growth. Two more audio histories were already
  full in the first minutes: the SPU event ring (`spu.c` `s_events`, 32 MiB)
  and the PCM taps (`audio_trace.c` `s_taps`, 4 x 16 MiB, about 95 s each).
- Level loads add 10-25 MB per first visit (heap and driver memory),
  much less on revisits.

Projected main-process ceiling for the r7 pattern: about 590 MB + 34 MiB
(rings full) + up to 61 MiB (aperture tables full) = **about 690 MB**, reached
after roughly 40 minutes. Short sessions see the steep first minutes (warm-up,
about 100 MB in 2 min) and level loads, which is the "14-23 MB a minute" read
from 6-8 minute logs.

## Not explained

The 402 -> 893 MB session is about 200 MB above that ceiling. Not reproduced:
real display window (GLX instead of offscreen EGL), real audio output, mouse
play and real combat, or a scene that was not scripted. The log is gone.

## Fix (chosen by the user: options 1 and 2)

- **History rings opt-in.** `PSX_TRACE_HISTORY=1` (developer; `run.py
  --diagnostics` and `fmv_reliability.py` set it) records the SIO byte and IRQ
  rings, SPU and audio event rings and PCM taps. Without it their pages are
  never touched: SIO entries go to one scratch entry, PCM and event writes are
  skipped, `spu_event_reset` no longer zeroes 32 MiB. Counters, statistics,
  `audio_stats`, sequence totals and audio output are unchanged; the debug
  history commands report empty. Framework: `audio_trace.h` (inline switch,
  not a codegen-hashed header), `audio_trace.c`, `spu.c`, `sio.c`; exported
  to the runtime patch and verified against a clean pinned checkout.
- **Memory line.** Each `[TTK pace] minute` line is followed by
  `[TTK memory] minute N: rss ... = static, heap (malloc allocated), shared,
  anon, files` (read from `/proc/self/smaps` on a short-lived thread, Linux
  only). `run.py` keeps 20 session logs instead of 5.

## Verification

Same 20-minute scripted route as r7 (r9, new build):

| Time | Main RSS before / after | Combined PSS before / after |
|---|---|---|
| 10 s | 419 / 400 | 462 / 451 |
| 5 min | 532 / 456 | 757 / 617 |
| 10 min | 562 / 474 | 748 / 595 |
| 15 min | 575 / 480 | 795 / 631 |
| 20 min | 589 / 482 | 814 / 656 |

Minutes 10 to 20 grew 8 MB (was 27). The memory line at minute 20: rss 481
MB = static 144, heap 70, shared 84, anon 37, files 145; static stops at 144
from minute 15 (the host-vertex tables' high-water mark). 0 audio underruns
in all 29 hitch lines. With `PSX_TRACE_HISTORY=1` the audio, SPU and SIO
history commands return entries; without it they are empty with live
totals for audio and SPU. Vanilla smoke (performance harness): 59.9 fields/s,
0 underruns. `ttk-input-test` passes. No codegen-hashed header changed, so
existing savestates load (the harness loads its own private states).

## Remaining

- The 402 -> 893 MB session is still not reproduced; the next long session's
  `[TTK memory]` lines will name its owner, and 20 logs are now kept.
- The host-vertex tables (up to 108 MiB in the main process) remain; they
  are a bounded high-water mark of the busiest frame.
- The user's real session after the fix was fine (accepted); later long
  sessions can be read from the `[TTK memory]` lines.

## Options considered (2026-11-09)

1. Bounded fix: player builds stop recording SIO/audio forensic history (or
   keep a small ring), saving up to 84 MiB; the aperture tables stay.
2. Diagnostics first: the minute line adds a breakdown (aperture tables,
   rings, heap in use, other) and is written every minute; keep more session
   logs, so the next large session names its owner.
3. Accept a reasoned ceiling (about 700 MB main RSS in offscreen play) and
   close once a long real session agrees.
