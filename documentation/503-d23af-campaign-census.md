# D23AF - Campaign-wide performance census and hot-spot surgery

**Status: Done (2026-11-17, user: "lock in this work and close us out"); fixes built off by default except D23AF7.** Job:
[D23AF](../jobs/campaign-performance.md#d23af---campaign-wide-performance-census-and-hot-spot-surgery).

The D23 programme so far measured a handful of fixed scenes (UI slots 2, 8,
9, DUKE HILL's town, the FAMILY JEWELS start). This note measures every
level the debug level select offers, on all three test machines, with one
scripted route, and ranks where the game is actually heavy.

## 1. Method

### 1.1 Machines

| | desktop (ripspace) | field laptop | citadel (TV PC) |
| --- | --- | --- | --- |
| CPU | i7-5960X, 8 cores / 16 threads | i7-5600U, 2 / 4, 15 W package | Ryzen 3 PRO 3200GE, 4 / 4 |
| GPU | GTX 1080 Ti | Intel HD 5500 (shares the package) | Radeon Vega 8 (integrated) |
| Display used | SDL offscreen on the real GPU | real display, 1080p 60 Hz | real display (TV), 1080p 60 Hz, KDE Wayland |
| Settings | GRAPHICS CUSTOM with the ULTRA rows, FRAME RATE 180 and 120 | pure AUTO (GRAPHICS AUTO, FRAME RATE AUTO); SIMPLE 120; ULTRA 120 | pure AUTO |

The desktop runs offscreen because the ULTRA grade sets FRAME RATE AUTO,
which follows the display: offscreen it sees no refresh rate, so the rate
is fixed with the ULTRA rows kept (GRAPHICS CUSTOM). Offscreen numbers
leave out the window present and the desktop compositor (notes 475
and 476); they are lower bounds for the player's
whole-GPU figures.

All three run the same build source: the checkout's HEAD (`fe18de2`) plus
the tracked framework patch. They also use the same base profile (the
desktop player's controls and presentation, copied privately) and the same
private card copy (start savestate: private slot 7). Before every launch
and every level, the host is checked and contention waited out: CPU busy
outside the game at most 12%, I/O pressure at most 5%, and no package
manager, backup, snapshot, indexer or compiler running. Each check is
logged with load, memory, swap and temperatures. The player's saves, cards
and profile are hashed before and after every launch.

### 1.2 Route

For each of the 21 selectable levels (0-3, 5-12, 21-29; note 101), the
backtick console's `level N` travels to the level's normal start; the
route begins once the game is back in gameplay on that level:

1. stand still 5 s (level start: bake, first redraws);
2. a slow 360-degree turn, closed loop on the camera yaw (about 45 degrees
   a second whatever the frame rate);
3. walk forward 3 s, forward turning right 3 s, forward turning left 3 s,
   run forward 3 s;
4. look up 1.5 s, down 3 s, up 1.5 s;
5. fire (left mouse) 3 s, then 1 s still.

The inputs come from `DNTTK_TEST_INPUT` and are the same on every machine.
Each machine runs the route once in first person and once in third person
(one launch each).

### 1.3 Per-second record

Every second of the route records:

- game fields a second (the emulated 60) and game ticks;
- presents and fresh pictures a second; whether the redraws own the cadence
  (otherwise the original presentation) and the governor's fallback events;
- present intervals p50, p95 and max: microsecond swap cadence while the
  redraws present, the present ring's millisecond stamps otherwise;
- spikes: present intervals over two target intervals;
- CPU, as % of one core: the emulation thread (the busiest thread of the
  main process), the render threads, the redraw workers (forked processes),
  the rest of the main process, and everything else on the host;
- CPU clocks: the mean over all cores and the emulation thread's core;
- GPU: busy % and clock, weighted as busy x clock / maximum clock
  (nvidia-smi; amdgpu `gpu_busy_percent` and `pp_dpm_sclk`; i915 RC6
  residency and the actual frequency);
- package power where RAPL is readable;
- the governor's log lines in that second.

A second's score is the pictures missing against the target (% of the
target) + 3 x game fields lost below 59.5 + 4 x spikes + 2 x ms of p95 over
the target interval. The worst second of each level gets a savestate (in
the private card copy, copied out per level) and a present shot.

Tools (local, `recomp/analysis/d23af/`): `census.py` (one launch: the
host gate, the launch, the 21-level route, the JSON), `queue.sh` (configs
x views), and the summary and ranking scripts named in section 3.

## 2. Census

Evidence labels: MEASURED (this census or its repros), CODE (read in the
source), INFERRED.

### 2.1 Runs

Every second of every level, all runs (MEASURED; after-savestate seconds
left out). Targets: the desktop's set rate; the field laptop's and
citadel's 60 Hz screens. The second view of a pass reused the bakes of
the first.

| Machine | Settings | View | Seconds | Redraws own the cadence | Game frames/s median (lowest tenth) | Seconds under 57 | Fresh pictures/s lowest tenth (target) | Emulation thread % median | GPU % clock-weighted median | Package W median |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Desktop | ULTRA rows, 180 | first | 732 | 100% | 59.9 (59.3) | 0 | 179 (180) | 45 | 16 | 58.6 |
| Desktop | ULTRA rows, 180 | third | 740 | 100% | 59.9 (59.3) | 1 | 179 (180) | 47 | 16 | 57.3 |
| Desktop | ULTRA rows, 120 | first | 741 | 100% | 59.9 (59.3) | 0 | 119 (120) | 42 | 11 | 52.0 |
| Desktop | ULTRA rows, 120 | third | 740 | 100% | 59.9 (59.3) | 0 | 119 (120) | 45 | 11 | 52.5 |
| Field laptop | pure AUTO (cold bakes) | first | 657 | 11% | 59.8 (43.6) | 134 | 42 (60) | 63 | 21 | 12.7 |
| Field laptop | pure AUTO (bakes from the first view) | third | 667 | 89% | 60.0 (59.2) | 0 | 40 (60) | 59 | 44 | 14.9 |
| Field laptop | pure AUTO (warm) | first | 676 | 100% | 60.0 (59.2) | 4 | 31 (60) | 58 | 42 | 14.9 |
| Field laptop | SIMPLE 120 | first | 688 | 100% | 60.0 (59.3) | 3 | 48 (60) | 58 | 32 | 14.9 |
| Field laptop | SIMPLE 120 | third | 689 | 100% | 60.0 (59.3) | 2 | 56 (60) | 62 | 34 | 14.9 |
| Field laptop | ULTRA 120 | first | 659 | 1% | 57.9 (32.0) | 307 | 32 (60) | 63 | 46 | 14.9 |
| Field laptop | ULTRA 120 | third | 643 | 1% | 59.4 (52.4) | 154 | 53 (60) | 58 | 56 | 14.9 |
| Citadel | pure AUTO (cold bakes) | first | 718 | 100% | 60.0 (59.3) | 0 | 59 (60) | 58 | 12 | 10.1 |
| Citadel | pure AUTO (bakes from the first view) | third | 728 | 100% | 60.0 (59.3) | 0 | 59 (60) | 58 | 12 | 9.8 |
| Citadel | pure AUTO (warm) | first | 725 | 100% | 60.0 (59.3) | 0 | 59 (60) | 56 | 12 | 9.7 |

Per level and run: `recomp/analysis/d23af/census-summary.json` and the
report page (local). What bounds each machine:

- Desktop: nothing at 120; at 180 only FAMILY JEWELS in first person (three
  workers saturated; D23AF5). Emulation thread 42-47%, GPU 11-16%.
- Citadel: nothing; 60 game frames and 59-60 fresh pictures a second in the
  lowest tenth on every level, the GPU about 12%, the package about 10 W.
- Field laptop: the 15 W package. At SIMPLE 120 the redraws hold every
  second (fresh pictures lowest tenth 48-56); at pure AUTO (BALANCED 2x)
  they hold when warm but fresh pictures drop to 31 in the lowest tenth
  (late redraws repeat); at ULTRA 120 the redraws run 1% of the time and
  the game falls under 57 in 154-307 seconds (D23AF4). Cold bakes cost
  134 seconds under 57 in one pass (D23AF1).

### 2.2 Instrument caveats

- Present intervals come from two instruments. While the redraws own the
  cadence: the swap ring's microsecond stamps (`render_replay` cadence).
  On the original presentation: the present ring's millisecond stamps,
  taken on the emulation thread before the swap. At the same 60 pictures a
  second the second instrument reads p95 20-32 ms where the first reads
  17-18 ms. p95 and spikes are compared only within one presentation mode;
  the ranking uses game speed lost and fresh pictures missing, which both
  modes measure the same way.
- The first census pass on the field laptop and citadel paid every level's
  first lamp bake (the bake's key changed with D55A); it is kept as the
  first-visit measurement (D23AF1) and repeated warm (`auto2`).
- The desktop's per-second "emulation" figure is the busiest thread of the
  main process; during a lamp bake that can be a bake thread (same thread
  name). From 09:00 the census also records `emu_main` (the main thread,
  which is the emulation thread: checked on citadel and the field laptop).
- The desktop is offscreen (no window present, no compositor): a lower
  bound for the player's whole-GPU figures.

## 3. Hot spots

Ranked by the cost a player sees (game speed lost, fresh pictures missing,
one-off hitches), across machines. Each is a child job of D23AF.

### 3.1 D23AF1: a level's first lamp bake (field laptop, citadel)

MEASURED (cold pass): bakes of 15-27 s on the field laptop (TIME TO KILL
27.2 s, LET THE GAMES BEGIN 22.8 s, MINER 69ER 17.9 s), 5-8 s on citadel,
under 4 s on the desktop. On the laptop the seconds with the game under 57
frames scale with the bake (TIME TO KILL 26 of 36, LET THE GAMES BEGIN 18,
levels without a bake 0-4), and the governor's fallback (D23Y1) taken
during the first bake kept the redraws off for every later level of the run
(backoff 2, 4, 8 minutes): 11% of the cold run's seconds had the redraws,
against 88.6% warm. Warm, the level starts are clean on all machines (fresh
pictures in the first 2 s 99.8-100% of the target).

CODE: `lamp_bake.cpp` runs half the logical CPUs (2 on the laptop) at nice
19. Niceness does not stop a bake thread sharing the emulation thread's
physical core (SMT sibling) or the 15 W package. Owner: the bake pool and
the per-second judge (`modern_controls.cpp`, `frame_replay.cpp`).
Class: real code cost on weak machines, made worse by a policy (backoff).

### 3.2 D23AF2: redraw worker restarts (every machine)

MEASURED: the session's first shot stalls one present by 50-61 ms on the
desktop (6 of 6 runs), 44-47 ms on citadel and 30.5 ms on the field laptop;
THE REAPER 62-72 ms once. `DNTTK_SPIKE_TRACE=20` placed it in fields
2771-2773 (44.5 ms of emulation-thread CPU, hooks under 3 ms of it); the
field-tagged wall-clock profile (`PSX_PROF_FRAMES=1`) put the extra in
`submit_job` > `maintain` > `stop_workers` (10 ms a field) and
`start_workers`. `PSX_REPLAY_PROFILE=1`: mod memory grows by 1,300 bytes at
the shot (a plugin's lazy allocation) and `maintain()` re-forks every
worker; the session's first level start grows it about 30 times in under a
second (+2,192 bytes each), each a full restart.

CODE: `render_worker.c` re-forks on any mod-memory growth because a plugin
keeps the guest address it was given in host state; a worker forked before
would hold a stale copy and allocate again. The stop sends SIGKILL and then
waits in `waitpid` for each worker's full teardown; the fork copies the page
tables of the process's private memory (383 MB anonymous, 243 MB private
dirty during TIME TO KILL at ULTRA 180). Class: real code cost.

### 3.3 D23AF4: the field laptop's package at AUTO

MEASURED (warm, third person, AUTO = BALANCED 2x): with the redraws on the
GPU runs 75-80% busy at 500-900 MHz, the package sits at 15 W and the
emulation thread's core drops to 2.1-2.5 GHz; the game slips to 56-57 and
the governor takes the original presentation (CPU back at 3.1 GHz, GPU
300-600 MHz). SIMPLE 120 kept the redraws in 100% of its seconds (3 seconds
of 688 under 57 frames); BALANCED 88.6%. On the desktop the redraw's GPU
time is 80-87% the textured scene draw (`PSX_GPU_PHASES=2`): shading, no
waste found. Class: package limit; the lever is AUTO's grade (policy).

### 3.4 D23AF5: FAMILY JEWELS start in first person at 180 (desktop)

MEASURED: the only desktop cell under target (fresh pictures p10 140 at
180, three workers at 300%). From the worst-second savestate with one
worker: 82 fresh pictures a second in first person, 138 in third; a job
costs 11.9 ms against 7.3 ms (`render_replay` workers, 3,621 jobs each),
with the same hooks in the same shares (world pass 27-30%, object path
15-23%, game code 23-26%): the eye-height view sees more of the castle and
moat. D23AA's geometry finding; real code cost, no new exact cut.

### 3.5 D23AF6: the render-thread validator at ULTRA with three workers

MEASURED: `PSX_RT_VALIDATE=1` reported differing pixels in every one of
158 and 166 comparisons with the new switches off (desktop, ULTRA 180,
third person, levels 0, 27, 1): 80,000-280,000 pixels, 1-174 levels, in
boxes inside the centre of the picture. D23Y10 measured 908 of 908 exact
with one worker in UI slots 2, 8, 9. Unexplained; left as an investigation.
It also means this validator could not serve as D23AF2's exactness proof
here (section 4).

### 3.6 Not hot

- Citadel at pure AUTO: 60 game frames and 60 fresh pictures a second in
  every second of all three runs (the redraws 100% of the time), GPU about
  12% clock-weighted. Its first-visit bakes (5-8 s) are D23AF1's.
- Level starts with warm caches, on every machine (section 3.1).
- The desktop at ULTRA 120 and 180 everywhere but D23AF5.

## 4. Fixes

All built into the player build, off by default except D23AF7's crash
fix. Framework edits are in the tracked patch; the pinned framework
(`08ec704`) plus the patch reproduces all 92 patched files.

### 4.1 D23AF1: the bake yields (`DNTTK_BAKE_YIELD=1`)

Change (game code, `lamp_bake.cpp/.h/.inc`, `modern_controls.cpp`): while
the last judged second ran under 58.5 game frames (sticky 3 s) each bake
thread sleeps three times its last chart's work after it (a quarter duty);
one bake thread on CPUs with 2 physical cores; seconds while a bake runs
are not judged by the redraw governor, so a bake no longer starts the
fallback and its backoff.

Exactness (MEASURED): a lamp bake's charts are independent and the switch
only changes when work runs. The .bake files written with the switch on and
off are byte-identical: desktop 5 levels, citadel 4 levels x 2 repeats
(one yield-on repeat ended early in the D23AF7 abort; every bake it wrote
matched).

A/B, cold caches (fresh cache folder a run; levels 0, 2, 3, 5; pure AUTO;
alternating):

| Machine | Off | On |
| --- | --- | --- |
| Desktop (ULTRA 180) | no effect: 16 CPUs, game 60, 180 pictures | the same |
| Citadel (real TV) | bakes 6.0-9.7 s, game 59.5-60.2 during them, redraws on throughout, 0 fallbacks | bakes 6.1-10.0 s, game 60.0-60.1, the same |
| Field laptop, route only (35 s a level, 2 repeats) | seconds under 57 frames: 8-23 a level; redraws after the bake started: none (one fallback a run, held); fresh pictures median 45.9-59.4 | 0-7 a level; redraws on throughout; 59.0-60.0; 0 fallbacks; but most bakes not finished within the 35 s |
| Field laptop, staying 100 s (levels 0 and 3, 2 repeats) | TIME TO KILL: bake 28.1-28.2 s, 24 of 127 s under 57, redraws 2%; GOLD AND GUNS: bake 16.7-16.8 s, 7-8 s under 57, redraws 72-80% | TIME TO KILL: bake 110 s / over 120 s, 10 and 29 of 119 s under 57, redraws 100%; GOLD AND GUNS: bake 28.6-31.9 s, 0 s under 57, redraws 100% |

Bakes: every bake written on the field laptop (8 runs, 4 levels) is
byte-identical with the switch on and off, and the same levels' bakes match
citadel's byte for byte.

Verdict (MEASURED): a clean win for a medium bake (GOLD AND GUNS: no slow
second, the redraws kept, the bake 1.8x longer). For the heaviest
(TIME TO KILL) the quarter duty stretches the bake about 4x (110 s or more,
the level lit by the live lamps meanwhile), and because bake seconds are
not judged the redraws keep running while the game is behind: one repeat
had 29 slow seconds against 24 with the switch off. It stays off. Next
step (D23AF1): keep judging the seconds during a bake but let a fallback
taken during it retry as soon as the bake ends without doubling the
backoff, and use one bake thread at full duty on 2-core CPUs (the SMT
sibling) instead of a quarter duty; re-measure the same two levels.

### 4.2 D23AF2: restarts without the wait (`PSX_RW_ASYNC_REAP=1`, `PSX_RW_COALESCE_MS=N`)

Change (framework, `render_worker.c`): see section 3.2 for the cause.

- Asynchronous reap: the generation (`epoch`) is bumped before the kill;
  a worker takes it at fork, re-checks it after every claim (the claim's
  acquire makes the job's generation visible) and hands a job of another
  generation back and exits; the job slots a killed worker held stay out
  of use (quarantine) until `maintain()` reaps it with `WNOHANG`; shutdown
  still waits for all.
- Coalescing: with N > 0 a growth retires the workers once and the new ones
  are forked N ms after the last growth; meanwhile redraw-only mode
  disengages ("workers inactive") and the emulation thread draws the live
  frames itself, so the pictures stay exact and only in-between redraws
  pause.
- Detector (always compiled, inert unless something is wrong): every
  finished job is stamped with its worker's generation; a finished job of
  another generation is dropped and logged (`[replay epoch]`), and the
  hand-backs are counted in the worker statistics.
- `runtime/tests/test_render_worker_epoch.py` (registered, and in CI) pins
  the ordering: generation set before the QUEUED release, bumped before any
  kill, re-checked after the claim, stamped before a finished job is
  published, quarantine kept until the last killed worker is reaped.

Desktop A/B (ULTRA 180, third person, levels 0, 27, 1 from a fresh launch,
rotated, two repeats each):

| | Off | Async reap | Async reap + coalesce 120 ms |
| --- | --- | --- | --- |
| Restarts | 34, 37 | 21, 25 | 4, 4 (plus 8 retire-only) |
| Emulation thread held, whole run | 745, 768 ms | 347, 370 ms | 76, 81 ms |
| A restart, median / max | 20.5-21.7 / 39-41 ms | 15.0-17.1 / 22-26 ms | 19.8-20.0 / 23-24 ms |
| First shot, longest present interval | 32.4, 35.0 ms | 29.7, 30.1 ms | 29.4, 31.9 ms |
| Hand-backs / foreign-generation jobs | - | 0 / 0 | 0 / 0 |

The session's restart stall falls about tenfold; the first shot's single
restart remains (its fork, about 20 ms): D23AF3. Citadel and the field
laptop: section 4.4.

### 4.3 D23AF7: the watchdog never wraps (always on)

`starvation_ring.c`: the heartbeat is read and written atomically
(`__atomic` relaxed) and the check computes `now > beat ? now - beat : 0`.
`runtime/tests/test_starvation_watchdog_wrap.py` (registered, and in CI)
compiles the expression and checks a heartbeat 149 us after the sample.

### 4.4 Remote A/B

D23AF2 on the real displays (pure AUTO, third person, levels 0, 27, 1 from
a fresh launch, rotated; citadel three repeats, the third on the final
build; the field laptop two, on the final build):

| | Off | Async reap | Async reap + coalesce 120 ms |
| --- | --- | --- | --- |
| Citadel: restarts | 6-12 | 6-7 | 4 |
| Citadel: emulation thread held, whole run | 101-214 ms | 81-89 ms | 60-67 ms |
| Citadel: a restart, max | 27.7-32.9 ms | 18.4-22.1 ms | 18.7-20.9 ms |
| Citadel: first shot, longest present (its second, the next) | 28.9-32.7, 17.3-17.5 ms | 19.2-29.8, 17.1-19.8 ms | 26.1-30.2, **32.8-38.6 ms** |
| Field laptop: a restart, median / max | 8.6-11.1 / 14.1-25.1 ms | 4.8-5.2 / 7.2-7.4 ms | 4.4-4.7 / 4.8-5.8 ms |
| Field laptop: emulation thread held, whole run | 46-89 ms | 30 ms | 12-19 ms |
| Field laptop: first shot (its second, the next) | 25.7-32.0, 19.7-21.5 ms | 19.3-21.4, 20.3-20.7 ms | 21.0-21.8, **58.7-64.3 ms** |
| Hand-backs / foreign-generation jobs | - | 0 / 0 | 0 / 0 |

Verdict (MEASURED on all three machines):

- Asynchronous reap helps everywhere and regresses nothing: a restart's
  worst case falls from 14-41 ms to 7-26 ms (desktop 39-41 to 22-26,
  citadel 28-33 to 18-22, field laptop 14-25 to 7), the first shot's
  present gap by about 3-12 ms; exact by construction, detector 0 in every run. **Proposed
  default on.**
- Coalescing folds a restart storm into one restart, but in the second
  after a coalesced restart every machine shows a new longest gap (field
  laptop 59-64 ms, citadel 33-39 ms, desktop 16-21 ms against 7-13): the
  hand-over from the live frames back to the redraws (redraw-only mode
  re-engages after 8 good frames). It stays off; it needs a smoother
  hand-over (keep redraw-only engaged across a planned restart) before it
  can pay.

The citadel A/B's second coalesce repeat and a cold-bake repeat ended in
D23AF7's watchdog abort (the builds before the fix); none of the three
runs on the final build did.

### 4.5 Exactness and regression checks

- Vanilla (`--vanilla`, UI slots 1 and 7, shots at +60, +200, +400 frames,
  offscreen): HEAD's build against this build, byte-identical 6 of 6.
- Modernized at FRAME RATE 60 (original presentation, same shots): not
  usable as a proof; HEAD against itself already differs run to run in UI
  slot 1 (1 of 3 frames), and this build differs in 1 of 6, the same class.
- The render-thread validator cannot judge RELIT pictures (section 3.5).
- `python3 -m unittest discover -s recomp/tests/local`: 260 tests, OK (2
  skipped); the five runtime tests CI runs (three before, two new) pass;
  `check_jobs`, `check_repo` pass.

## 5. State, tests and next

State at the end of the session (2026-11-17, about 12:10): the player
build on all three machines is the same source (HEAD `fe18de2` plus this
work, uncommitted), with D23AF1 and D23AF2 off by default and D23AF7 on.

Tests for the user (each needs only a normal session; the variables apply
to one launch):

- Desktop: play a few minutes from a fresh launch into a level with
  sleeping enemies and fire the first shot, with the asynchronous reap on:
  `env PSX_RW_ASYNC_REAP=1 python3 recomp/tools/local/run.py`. Expected: no
  change in look; the first shot's hitch shorter. Then confirm D23AF7: no
  "without heartbeat" abort in any session.
- Field laptop: the same command at pure AUTO; then, for D23AF4, one
  session at SIMPLE against one at AUTO (`--graphics simple` and
  `--graphics auto`; these save the setting): which looks acceptable and
  which feels smoother. The census says SIMPLE keeps the redraws every
  second (fresh pictures lowest tenth 48-56) where BALANCED drops to 31.
- Citadel: the same `PSX_RW_ASYNC_REAP=1` session on the TV.

Not pursued, with reasons:

- D23AF2's coalescing stays off: its hand-over back to the redraws costs
  more than the restarts it folds (section 4.4).
- Eager plugin allocations (D23AF3's second route) change the mod-memory
  layout savestates record; not changed unattended.
- D23AF5 (FAMILY JEWELS at 180, first person) has no new exact cut; it is
  D23AA's geometry, for D23N-class work.
- D23AF6: the validator needs a frozen light state before it can judge
  RELIT pictures.

Next: D23AF1's refinement (section 4.1), D23AF2's asynchronous reap on by
default once accepted, D23AF8 (the user's decision after this census:
GRAPHICS grades leave FRAME RATE alone, so SIMPLE keeps the redraws), D23AF3 (cheaper forks: the observability rings out
of the fork, each proven untouched by workers), D23AF4 (the user's choice of
AUTO's grade on 15 W laptops).
