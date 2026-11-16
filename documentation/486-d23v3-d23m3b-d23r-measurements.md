# 486 - D23V3 and D23M3B measured, what D23R still needs (field laptop)

Autonomous session of 2026-11-16, part 4 of the chain. User: "If time
remains: D23V3 and D23M3B measurements; note what D23R (the EDuke32 parity
benchmark) still needs." Measurements only; nothing built.

Evidence labels: MEASURED, CODE, INFERRED.

## 1. Setup

Field laptop as note 485 (checkout `6e887c5`), lab `recomp/analysis/d23y6/`,
`d23v2.sh` with `REPS=1 RATES=120 PRE=v3base` (case `perf2`, offscreen,
turning, `PSX_GPU_PHASES=2`, the incremental capture against the whole copy,
UI slots 2, 8 and 9), summarized by `v2sum.py v3base 120 "1 7 8"`. One run a
configuration (the spread of note 479 applies).

## 2. D23V3: the capture now, and what the redraws read (MEASURED)

| View | whole copy | incremental (today's default) |
| --- | --- | --- |
| UI 2 | 1.59 ms | 1.04 ms (-34%) |
| UI 8 | 1.63 ms | 1.03 ms (-37%) |
| UI 9 | 1.59 ms | 1.07 ms (-32%) |

GPU time of the capture a game frame at 120 (`[gpu copies]`, timeline);
note 479's 1.04-1.08 ms reproduces. Every capture copies 245 of the 512
canonical tiles (150 of them incremental).

New: the render threads' lazy loads (`[lazy load]`, `[lazy rows]`, UI 2,
576 sessions) copy only **38.0 tiles a session** out of the snapshot (16
high-resolution and 22 wide), **all in tile row 7** (VRAM rows 224-255);
266 more tiles a session are made whole by the redraw's own writes before
any read. So about 38 of the 245 captured tiles are ever read by a redraw
(INFERRED for UI 8 and 9 from the same capture count; their lazy rows were
not extracted). D23V3's target (at most about 0.53 ms) is then reachable by
capturing only the tiles a redraw can read before it writes them, rather
than by the band rules note 479 proposed, provided the validator's and the
band copy's readers (`rp_band_direct`) are covered: the next step is to
list every reader of the snapshot surface (CODE) and log their tiles in the
validator's runs too.

## 3. D23M3B: what the output-sized target would remove (MEASURED)

GPU a redraw on the laptop, UI 2 at 120 (`[gpu kinds]`, 576 redraws, mean):
scene 4.70 ms (textured 3.67, flat 0.29), end 0.35 ms; and the copies the
output-sized target of note 460 removes: **band copy 0.345 ms, margin
mirror 0.092 ms, VRAM-wide depth clear 0.066 ms** (0.50 ms together, about
10% of the redraw), beside the fill copy 0.197 ms and wide clears 0.274 ms
it keeps. Note 460 estimated the saving at about 0.05 ms on the desktop
(1080 Ti); on this GPU the same copies cost ten times that, as note 460
expected for a weak GPU. D23M3B therefore matters on the field laptop; it
stays Todo with this measurement (its dependency D23V is done).

## 4. What D23R still needs

D23R is the programme's acceptance (note 444 section 5): the same views on
the same display as EDuke32 Polymost and Polymer, windowed and fullscreen,
0.4 ms GPU or less a presented frame at 180 Hz, 12% or less windowed, 1.5
cores or less, 180 Hz on 4 logical CPUs, under 4 ms a redraw on the weak GPU
estimate; pictures identical, feel unchanged, Vanilla unchanged. Its
dependencies and what is open:

- D23N4 (CPU: 1.4-1.5 of 2.4-2.6 cores are the redraw workers): N4b is
  measured cheaper and nearly exact this session (note 483) but has an open
  residue and stays off; the bar of 1.5 cores needs N4b on by default (the
  user's decision) and the multi-worker case (N4b needs one worker today).
- GPU per redraw (0.81-0.83 ms on the desktop against 0.4 ms): D23M2 and
  D23M3 are done; D23M3B (built from D23M3's design) is measured here at
  about 0.5 ms a redraw on the laptop and is still Todo.
- D23P2, D23U and D23V are done; only D23N4 of D23R's listed dependencies
  is open.
- The EDuke32 side and windowed figures need the real display with the user
  present (note 453); the 4-CPU test runs headless.
- A repeatable script: the interim readings of 2026-11-11 and 2026-11-12
  were taken by hand; D23R needs one lab case that runs our views and the
  EDuke32 maps with the same pacing, GPU sampler and CPU accounting.

## 5. Reproduce

Laptop: `REPS=1 RATES=120 PRE=v3base bash d23v2.sh`, then
`python3 v2sum.py v3base 120 "1 7 8"`; `grep "lazy rows\|gpu kinds"
v3base-inc-120-s1-r1.log`.
