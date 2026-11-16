# 496 - D23AB: three machines at 60, 120 and 180 (citadel, field laptop, desktop)

Autonomous three-machine session of 2026-11-16. User: "citadel: clone the
repo, venv, copy my owned disc image ... build, and set up its lab like the
laptop's ... Record its AUTO choices (cores, GPU probe, GRAPHICS grade). C1.
Baseline on citadel: FRAME RATE 60/120/180 at AUTO and at ULTRA, UI 2, 8,
9, DUKE HILL town, FAMILY JEWELS start: game and present rates (median,
lowest tenth), emulation thread %, worker %, GPU busy x clock, CPU clock.
Same table on the laptop at the player's settings. Write the cross-machine
picture: what bounds each machine (CPU vs GPU) at 120."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. The machines and citadel's setup

| | desktop (ripspace) | field laptop | citadel (TV PC) |
| --- | --- | --- | --- |
| CPU | i7-5960X, 8 cores / 16 threads | i7-5600U, 2 / 4, 15 W | Ryzen 3 PRO 3200GE, 4 / 4, 35 W |
| GPU | GTX 1080 Ti | Intel HD 5500 (shares the package) | Radeon Vega 8 (integrated, shares 5.7 GB RAM) |
| OS | Ubuntu, kernel 6.8 | CachyOS, 7.3 | CachyOS, 7.3 |

Citadel (MEASURED): the repository cloned into `~/CODE` with its
submodules, a venv with the local requirements (and cmake from pip; no
system packages changed), the owned disc copied from the desktop's `game/`
(private, never in git; SHA-256 checked on arrival), the optional local
font pack copied, `build.py --jobs 4`: about 50 minutes, a 602 MB binary.
Lab `recomp/analysis/d23ab/` (local), the field laptop's lab with a
`GRADE` switch (a grade's rows applied as `run.py` applies them, then
FRAME RATE set) and AMD GPU clock sampling; the private card copy is the
field laptop lab's (UI slots 2, 8, 9, DUKE HILL town in private slot 4,
the FAMILY JEWELS start in slot 11).

Citadel's AUTO choices: 4 logical CPUs (no SMT), 3 redraw workers;
GRAPHICS AUTO resolved to BALANCED, but as the fallback: the GPU probe
fails offscreen (`gpu probe: failed (video init)`), so no probe was
cached; the real AUTO grade on its TV needs a launch on the display
(queued for the user). The field laptop's cached probe: 6.32 ms on the HD
5500, BALANCED. The desktop's: ULTRA.

Every run: offscreen (SDL's offscreen driver on the real GPU), case `cad`
(30 s walking, turning, firing), interleaved; the host checked before each
run (`xm.sh`: CPU idle over 3 s, I/O pressure, no package, backup, build or
indexer process, logged to `PRE-host.log`). All runs passed the check
except citadel's very first, which waited 20 minutes on an idle KDE search
runner that the check mistook for the indexer (fixed). Offscreen numbers
leave out the display's vsync, the window present and the compositor.

## 2. Citadel (MEASURED; two runs each)

Pictures are swaps a second from the present ring; "emulation" the
emulation thread, "workers" the redraw workers together, in % of a core;
GPU busy at the average shader clock.

| Grade, rate | Scene | game median / lowest tenth | pictures | emulation | workers | GPU busy @ MHz | CPU MHz |
| --- | --- | --- | --- | --- | --- | --- | --- |
| AUTO (2x), 60 | UI 2 / 8 / 9 / town / FJ | 59.9-60.0 / 59.2-59.6 | 45 | 49-61 | - | 25-31 @ 200 | 1988-2260 |
| AUTO, 120 | UI 2 | 59.9 / 58.8 | 120 | 62 | 98 | 73 @ 327 | 3255 |
| | town | 59.9 / 59.3 | 120 | 74 | 113 | 73 @ 432 | 3425 |
| | UI 8 | 59.9 / 59.4 | 120 | 77 | 125 | 71 @ 336 | 3432 |
| | UI 9 | 59.9 / 59.0 | 114 (109, 120) | 56 | 80 | 71 @ 264 | 3104 |
| | FJ start | 59.9 / 58.9 | 110 | 62 | 84 | 67 @ 333 | 3235 |
| AUTO, 180 | UI 2 / 8 / 9 / town / FJ | 59.6-59.9 / 54.2-58.6 | 166 / 147 / 165 / 143 / 123 | 65-83 | 104-163 | 68-72 @ 382-617 | 3256-3525 |
| ULTRA (4x), 60 | all five | 59.9-60.0 / 59.1-59.6 | 45 | 50-62 | - | 53-64 @ 223-266 | 2086-2249 |
| ULTRA, 120 | UI 2 | 59.6 / 55.0 | 91 | 77 | 110 | 87 @ 954 | 3257 |
| | town | 59.9 / 58.7 | 65 | 74 | 73 | 77 @ 1046 | 3112 |
| | UI 8 | 60.0 / 58.9 | 63 | 74 | 77 | 74 @ 843 | 3221 |
| | UI 9 | 60.0 / 57.2 | 104 | 73 | 111 | 92 @ 1053 | 3282 |
| | FJ start | 59.9 / 57.3 | 76 | 75 | 82 | 82 @ 959 | 3176 |
| ULTRA, 180 | all five | 59.7-60.0 / 53.2-59.0 | 69-84 | 62-82 | 74-102 | 75-84 @ 882-1058 | 3050-3252 |

At 60, citadel shows 45 pictures a second offscreen while the game runs
at 60 (a third of the intervals 29-40 ms): the field laptop shows 60 in
the same runs. Whether this is the offscreen driver on Mesa/AMD or real on
the TV needs the display (queued for the user).

## 3. Field laptop at the player's settings (MEASURED; two runs each)

The player's profile (4x, ULTRA's rows, kept as CUSTOM):

| Rate | Scene | game median / lowest tenth | pictures | emulation | workers | GPU busy @ MHz | CPU MHz |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 60 | all five | 59.8-60.0 / 58.8-59.4 | 59.5-60 | 57-70 | - | 51-72 @ 596-741 | 2793-3072 |
| 120 | UI 2 | 59.7 / 36.5 | 52 | 61 | 24 | 76 @ 752 | 2809 |
| | town | 50.4 / 41.6 | 46 | 91 | 21 | 87 @ 814 | 1297 |
| | UI 8 | 59.5 / 48.7 | 50 | 85 | 27 | 75 @ 712 | 2295 |
| | UI 9 | 59.5 / 55.6 | 49 | 77 | 49 | 82 @ 785 | 1504 |
| | FJ start | 59.7 / 51.0 | 52 | 72 | 39 | 74 @ 707 | 2818 |
| 180 | all five | 49.2-59.9 / 37.7-54.0 | 52-63 | 61-93 | 21-38 | 74-90 @ 713-805 | 1297-2988 |

And at SIMPLE (2x, from C2 and C3 the same day): 120 and 180 pictures
held in all five scenes, the game at 59.8-60.0 / 58.6-59.3, the single
worker 70-85%, the GPU 75-81% busy at 430-630 MHz.

## 4. Desktop at 120 (MEASURED; one run each)

AUTO (ULTRA, 4x) and ULTRA: all five scenes 120 pictures, the game
59.9-60.0 / 59.2-59.8; the emulation thread 35-52%, the workers 53-87%,
the GPU 12-29% busy at 895-1974 MHz, the CPU 3,498 MHz.

## 5. What bounds each machine at 120

- Desktop: nothing at 120 (MEASURED headroom: emulation thread at most
  52%, GPU at most 29%).
- Citadel at AUTO's 2x: close to 120 with AUTO's three workers (UI 2, 8
  and the town hold it; UI 9 and the FAMILY JEWELS start 110-114), and 120
  in all five scenes with one worker (MEASURED later the same day, the
  D23N5 A/B's runs without the restore, note 493 section 6b: UI 9 and the
  FAMILY JEWELS start 120 every second, the one worker at 70-86% of a
  core). The GPU's shader clock is as low either way (264-432 MHz of
  1,200, 67-74% busy), so the shortfall is not the GPU: three workers
  share four cores with the emulation thread (56-77%) and the render
  thread (26-41%) (INFERRED: contention; late repeats 54-92 a run in UI 9
  and FJ with three workers, 11-37 with one). AUTO's worker count is too
  high for a 4-core CPU without SMT at 120. At 180 the CPU binds (the
  workers and the emulation thread fill the four cores). At ULTRA (4x) the
  GPU binds: 74-92% busy at about 1 GHz, 63-104 pictures.
- Field laptop at the player's 4x: the package binds. The GPU is 74-87%
  busy and takes the 15 W from the CPU, whose clock falls to 1.3-2.8 GHz;
  redraws mostly stay off and the town's game rate itself falls to 50
  (note 494). At SIMPLE (2x) it holds 120 and 180, with the single worker
  then the limit (85% of a core at 180).

So at 120 the field laptop is power-bound (GPU-led), citadel holds 120 at
2x with one worker but not with AUTO's three (CPU contention, INFERRED)
and is GPU-bound at 4x, and the desktop has headroom. CPU-side cuts (D23N5's restore, D23Y10's tracked publish)
help where a CPU limit shows: the laptop at 2x and citadel at 180.

## 6. Real-display checks queued for the user

- Citadel on its TV: launch once at GRAPHICS AUTO to take the GPU probe
  and record the grade it picks; then FRAME RATE 60 and 120 at that grade
  in UI 2 (the offscreen 45 pictures at 60).
- Field laptop: DUKE HILL's town at ULTRA and at PRETTY at 120 (note 494).

## 6b. Follow-up

AUTO's redraw worker count on a 4-core CPU without SMT: one worker held
120 where three did not (offscreen, two runs a scene). A job to choose the
count from the measured cores is proposed (not built).

## 7. Reproduce

`recomp/analysis/d23ab/` on each machine (local): `xm.sh` (env PRE,
RATES, GRADES, SLOTS, REPS, CFGS), `xmsum.py PRE`, `citadel_queue.sh`,
`laptop_queue.sh`, `citadel_probe.sh`.
