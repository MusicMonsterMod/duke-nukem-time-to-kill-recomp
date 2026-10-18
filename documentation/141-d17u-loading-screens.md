# D17U - Smooth loading screens (spinning logo)

Job: [D17U](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-18: "i accept!! absolutely perfect.").
Builds on [D17T](140-d17t-frontend-smoothness.md) (frontend images) and the
D17 replay ([note 80](80-d17-high-refresh-audit.md)).

## How the original draws a loading screen

While the CD loads, three callers draw the same image and then flip
(`0x8001fcbc`):

| Caller | Return | When |
| --- | --- | --- |
| Blocking load loop `0x8001c440` (while `0x800ce1c4` > 0; draws only in modes 0 and 1) | `0x8001c49c` | level loads, demo scene loads |
| Frontend tick load branch (`0x8002341c`, pending loads) | `0x8002343c` | title/demo while loading |
| Level start `0x800257c8` | `0x80025818` | one image before a load |

- Backdrop `0x800220f4`: three static primitives at `0x800d24e8` (a
  full-screen textured quad sized by `0x800d2210/2212`), linked into the
  drawing chain at depth `0x7ff`.
- Logo `0x80025554`: a GTE model turned about Y. It adds the frame's field
  delta (`0x800d21fc`, 5 per field) to the 12-bit angle `0x800beaf8`, builds
  the rotation (`0x8002ac54`), translation (0, 0, `0xc00`), screen distance
  `0x182`, then draws the model `0x800bdeb4` through `0x80010000`. Its
  arguments are unused. The logo turns about 300/4096 of a turn a second.
- The flip waits 6 fields in mode 0 and 2 in gameplay, so loading screens
  run at **10 images a second between demo scenes** and **30 in level loads**
  (measured 9.8 and 26-27 by polling `0x800bdbd0`; the polling undercounts
  30). The guest sometimes stops drawing for over 15 fields during a load (a
  blocking read); the logo then freezes and jumps, in the original too.
- D17 replay ran only for compositions (`0x80026164`), so these images were
  presented as is.

## What changed

`recomp/src/ttk/frame_replay.cpp`; one entry hook, `0x80025554`, added to
`recomp/game.local.toml` and regenerated (`psxrecomp_cli.py generate`; one
generated line, codegen hash unchanged, existing savestates load).

- A live call of the logo draw from one of the three returns above is
  captured as a **loading image** (`Frame::loading`, also `frontend`: no
  mouse look, frontend world delay). The published state is the logo's
  entry, after the backdrop is in the chain. It records the angle the image
  shows (`angle + delta`).
- Workers run from the logo draw to the flip, then close and submit as
  before; the caller's tail (title text, PRESS START) stays crisp. The job
  carries only the interpolated angle (shortest way round, flag 4); the
  worker writes `angle - delta` so the draw shows exactly that angle.
  Camera, actors and transforms are not used.
- The timeline resets when images switch between gameplay, frontend and
  loading, or after a gap of over 15 fields. A load starts with the mode's
  interval (6 fields in mode 0, 2 in gameplay); the interval is then measured
  like D17T's. Workers start on the first loading image.
- A tick that already captured a composition never captures its logo too.
- `DNTTK_LOADING_INTERP=off` (developer) keeps the original images.
- Debug: `render_replay` -> `plugin.loading` (`live`, `captures`, `resets`,
  `interval_ms`).

The guest, its CD reads and its pace are untouched; Vanilla and rate 60 never
register a capture (replay off).

## Evidence

Executable `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc`;
offscreen on the real GPU, private profile, blank private cards and private
savestates (`recomp/analysis/d17u-loading/`, local: `fe.py`, `loadtest.py`,
`demoload.py`, `tracestat*.py`, `angle.py`). Level loads: savestate at the
start of TIME TO KILL, then the debug console `level 1` (DUKE HILL) or
`level 0`.

| Case | Logo on screen | Guest images/s | Distinct presents/s |
| --- | --- | --- | --- |
| Level 1 load, before (120) | 6.84 s | 27.0 | 5.1 |
| Level 1 load, after, 120 | 6.82 s | 27.1 | 119.4 |
| Level 1 load, after, 180 | 6.83 s | 27.1 | 179.8 |
| Level 1 load, after, 240 | 6.83 s | 26.9 | 239.1 |
| Level 0 load, before / after (120) | 5.09 / 5.11 s | 26.1 / 26.0 | 8.4 / 116.7 |
| Demo scene load, after, 120 | 8 s | 9.8 | 112.2 |
| Demo scene load, after, 180 / 240 | 8 s | 9.8 | 167.8 / 222.8 |
| Rate 60 / Vanilla / `DNTTK_LOADING_INTERP=off` | 6.84 / 6.83 / 6.82 s | 27 | replay off / off / 5.2 (original) |

- "Distinct presents" before is the runtime's own count of new images, which
  counts the guest's real images only partly; the guest image rate is the
  original's 30 (level) and 10 (demo).
- Present trace during a level load at 120: four presents per guest image at
  blends 0.17, 0.42, 0.67, 0.92, even. Over a whole load: 0 backward steps;
  repeats 43 of 905 presents (DUKE HILL) and 91 of 734 (TIME TO KILL), almost
  all 50 ms holds at the first image after a gap in the guest's drawing (the
  original freezes there too); a few single late redraws.
- Present screenshots: the logo is centred and turning; the demo-load phase
  with DUKE NUKEM TIME TO KILL / PRESS START keeps its text.
- Regression at 120: gameplay 118.9 distinct/s with 29.4 images/s; attract
  demo 119.1 distinct/s with 9.9 images/s (D17T unchanged).

## Limits

- After a gap in the guest's drawing (a blocking read) the logo holds for
  about 50 ms and then continues: the timeline restarts there rather than
  blending across a stop of unknown length.
- The first image of each load is shown as the guest drew it (no pair yet).
- Accepted by the user on their display; Match Display not measured offscreen.
