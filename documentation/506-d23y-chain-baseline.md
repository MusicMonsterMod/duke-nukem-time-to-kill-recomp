# 506 - Performance chain baseline: where the emulation thread's time goes (field laptop, citadel)

Autonomous session of 2026-11-17 (evening), step 0 of the chain D23Y8,
D23Y5, D23AF3 (acceptance scenes D23Y7, D23AA, D23AF5). User: "Profile
today's HEAD where the time goes on the emulation thread and the redraw
workers ... at pure AUTO and at ULTRA 120: DUKE HILL town (D23Y7), FAMILY
JEWELS start (D23AA/D23AF5), UI slots 2 and 9. Record CPU clock, package
power, GPU busy, game fps / pictures per second and the lowest tenth. Check
host state first. Write the ranked cost table into a new note before
changing anything."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Machines and host state (MEASURED)

| | field laptop | citadel |
| --- | --- | --- |
| CPU | i7-5600U, 2 cores / 4 threads, 15 W package, max 3.2 GHz | Ryzen 3 PRO 3200GE, 4 / 4, max 3.8 GHz |
| GPU | Intel HD 5500 (Mesa iris, shares the package) | Radeon Vega 8 (Mesa radeonsi, shares the 5.7 GB) |
| Screen | 1080p 60 Hz panel, X11 | TV 1080p 60 Hz, KDE Wayland |
| Kernel, governor | CachyOS 7.3.3, schedutil, power profile performance | CachyOS 7.3.4, schedutil, power profile balanced |
| Clocksource | tsc | **hpet** (see below) |
| Memory | 7.6 GB, 5.4 GB available, 0.6 GB swap in zram | 5.7 GB, 4.1 GB available, 0.4 GB swap in zram |
| Backups, updates | none scheduled (no Timeshift/snapper timers); last package change 2026-11-12 | none scheduled; last package change 2026-11-16 |

Before every run the lab's gate waited for an idle host (CPU busy outside
the game at most 12% over 3 s, I/O pressure at most 5%, no package
manager, backup, indexer or compiler process) and logged the host line;
every run passed (two short waits on the laptop: this session's own rsync
dry run and one I/O blip). An idle `dirmngr` of pacman's keyring (1 s of
CPU in 10 hours) is excluded from the gate. Kernel logs after the
real-display batches: no GPU hang, ring timeout or reset on either machine.

**Host finding, citadel: the kernel distrusts the TSC.** At boot
(2026-11-16) the clocksource watchdog marked the TSC unstable ("TSC found
unstable after boot, most likely due to broken BIOS") and the system runs
on the HPET. Every `clock_gettime` there is a system call plus an HPET
read (about 1-2 us) instead of a vDSO TSC read (about 25 ns): 6-8% of
citadel's emulation thread is clock reads (section 4). This is host
configuration, not a game regression; whether to boot citadel with
`tsc=reliable` (or to re-check after a cold boot) is the user's decision.

Also found: one profiled run on citadel crashed (2026-11-17 19:45, ULTRA
120, UI 2): the developer stack sampler (`PSX_PROF_STACKS`) called
`backtrace()` from its signal handler while the sample was inside the Mesa
driver, and libgcc's unwinder faulted. Fixed in the sampler
(`host_profiler.c`: the unwind of a sample outside the executable runs
under a fault guard that drops the sample); a measuring-tool defect only.

## 2. Method

- Build: HEAD `aed7f7d` plus the tracked framework patch, the same source
  on both machines (laptop built 19:23, citadel 19:21).
- Lab `recomp/analysis/d23y8/` (local, not tracked): `lab.py` (the D23AB
  lab, case `cad`: the savestate, then 30 s of walking, turning and firing),
  `y8.sh` (gated runner, real display, interleaved configurations),
  `y8sum.py` (summary), `prof.py` (profile tables), `gpu.py` (GPU busy and
  clock; package power from RAPL `energy_uj` through `sudo -n`).
- Scenes (the lab's private card copy, identical files on all three
  machines): DUKE HILL town (lab slot 4), FAMILY JEWELS start (slot 11),
  UI slot 2 (slot 1), UI slot 9 (slot 8).
- Settings: pure AUTO (AUTO grade rows as `run.py` resolves them, FRAME
  RATE AUTO; laptop and citadel resolve BALANCED, 2x); ULTRA at FRAME RATE
  120; and the original 60 (FRAME RATE 60, AUTO grade rows) for D23Y5's
  target. Each machine's own player controls and presentation copied read
  only. Real display.
- Two timing runs per cell, then one profiled run (the runtime's sampler:
  emulation thread every 1 ms of its CPU time with stacks and library
  leaves; workers flat). Profiles cost a few points, so timing comes only
  from the unprofiled runs.
- "Lowest tenth" is the 10th percentile over all seconds of both runs
  (first 2 s of each dropped). Pictures a second are counted from the
  present ring's swap times.

## 3. Baseline (MEASURED)

### 3.1 Field laptop

| Setting, scene | game med / low tenth | pictures med / low tenth | emu thread % | workers % | GPU busy @ MHz | package W med (p90) | CPU MHz |
| --- | --- | --- | --- | --- | --- | --- | --- |
| pure AUTO, DUKE HILL town | 59.7 / 58.3 | 60.0 / 59.9 | 68 (68, 69) | 45 | 74 @ 669 | 18.3 (22.5) | 2794 |
| pure AUTO, FAMILY JEWELS start | 60.0 / 59.1 | 60.0 / 59.9 | 59 (55, 64) | 45 | 75 @ 621 | 13.9 (22.4) | 2594 |
| pure AUTO, UI 2 | 59.9 / 58.7 | 60.0 / 59.8 | 65 (56, 73) | 59 | 78 @ 721 | 17.2 (22.5) | 2421 |
| pure AUTO, UI 9 | 60.0 / 59.1 | 60.0 / 59.9 | 75 (74, 76) | 72 | 78 @ 663 | 15.0 (22.2) | 2195 |
| ULTRA 120, DUKE HILL town | 41.8 / 22.7 | 49.4 / 36.0 | 65 (66, 64) | 15 | 85 @ 798 | 15.0 (23.6) | 1896 |
| ULTRA 120, FAMILY JEWELS start | 56.3 / 37.0 | 58.0 / 50.0 | 61 (59, 62) | 16 | 85 @ 833 | 19.4 (24.3) | 2899 |
| ULTRA 120, UI 2 | 58.6 / 34.9 | 59.9 / 56.4 | 54 (54, 54) | 14 | 86 @ 824 | 17.6 (23.9) | 3049 |
| ULTRA 120, UI 9 | 53.5 / 31.7 | 59.2 / 51.5 | 58 (57, 59) | 14 | 84 @ 796 | 18.0 (24.0) | 3055 |
| original 60, DUKE HILL town | 60.0 / 59.6 | 60.0 / 59.9 | 62 (61, 62) | 0 | 69 @ 460 | 14.2 (15.8) | 3126 |
| original 60, FAMILY JEWELS start | 60.0 / 59.2 | 60.0 / 59.3 | 56 (57, 56) | 0 | 52 @ 580 | 11.5 (16.3) | 3093 |
| original 60, UI 2 | 60.0 / 59.5 | 60.0 / 59.9 | 54 (54, 54) | 0 | 63 @ 465 | 10.1 (14.4) | 3125 |
| original 60, UI 9 | 59.9 / 59.6 | 60.0 / 59.9 | 57 (58, 56) | 0 | 58 @ 465 | 10.9 (15.3) | 3113 |

(RAPL's package figure includes the turbo window above 15 W, hence p90 of
22-24 W.)

### 3.2 Citadel

| Setting, scene | game med / low tenth | pictures med / low tenth | emu thread % | workers % | GPU busy @ MHz | package W med (p90) | CPU MHz |
| --- | --- | --- | --- | --- | --- | --- | --- |
| pure AUTO, DUKE HILL town | 60.0 / 59.5 | 60.0 / 60.0 | 62 (61, 62) | 28 | 59 @ 218 | 10.1 (11.1) | 3124 |
| pure AUTO, FAMILY JEWELS start | 60.0 / 59.2 | 60.0 / 60.0 | 54 (55, 53) | 54 | 65 @ 224 | 9.0 (12.4) | 2994 |
| pure AUTO, UI 2 | 60.0 / 59.6 | 60.0 / 60.0 | 53 (53, 53) | 50 | 59 @ 205 | 8.9 (11.3) | 2991 |
| pure AUTO, UI 9 | 60.0 / 59.5 | 60.0 / 60.0 | 55 (55, 56) | 56 | 57 @ 209 | 9.2 (11.4) | 3002 |
| ULTRA 120, DUKE HILL town | 60.0 / 59.3 | 60.0 / 58.1 | 70 (70, 71) | 29 | 73 @ 961 | 12.0 (13.0) | 3248 |
| ULTRA 120, FAMILY JEWELS start | 52.5 / 40.1 | 51.1 / 40.7 | 55 (59, 52) | 53 | 67 @ 565 | 7.8 (14.8) | 3006 |
| ULTRA 120, UI 2 | 60.0 / 58.9 | 60.0 / 57.4 | 62 (62, 63) | 56 | 66 @ 787 | 10.9 (12.4) | 3091 |
| ULTRA 120, UI 9 | 58.9 / 36.2 | 56.3 / 35.5 | 52 (42, 63) | 49 | 67 @ 595 | 8.5 (13.1) | 2970 |
| original 60, DUKE HILL town | 60.0 / 59.4 | 60.0 / 60.0 | 61 (60, 61) | 0 | 36 @ 200 | 5.2 (6.0) | 2718 |
| original 60, FAMILY JEWELS start | 60.0 / 59.2 | 60.0 / 60.0 | 51 (50, 52) | 0 | 27 @ 200 | 4.7 (6.5) | 2674 |
| original 60, UI 2 | 60.0 / 59.3 | 60.0 / 59.9 | 48 (49, 48) | 0 | 29 @ 200 | 4.3 (5.6) | 2566 |
| original 60, UI 9 | 60.0 / 59.3 | 60.0 / 59.9 | 52 (52, 51) | 0 | 31 @ 200 | 4.5 (6.4) | 2712 |

(amdgpu's `pp_dpm_sclk` reports the active level; at AUTO it sits at the
200-225 MHz level with the GPU 57-65% busy, so the busy figure there is
time at a low clock, not load.)

### 3.3 What the baseline says

- **Pure AUTO holds the game at 60 and 60 pictures on both machines in
  all four scenes** (lowest tenth 58.3-59.6). The acceptance scenes'
  stutter (D23Y7, D23AA) is an ULTRA 120 problem.
- **ULTRA 120 on the field laptop is GPU- and package-bound.** The 4x GPU
  is 84-86% busy at about 800 MHz; the redraws are mostly off ("workers
  behind", workers 14-16%); in the town the CPU drops to 1.9 GHz and the
  game itself falls to 41.8 (lowest tenth 22.7). Nothing on the emulation
  thread grows (65%); it is note 494's finding again.
- **Citadel at ULTRA 120 is unstable rather than slow:** DUKE HILL and
  UI 2 hold 60, FAMILY JEWELS start and UI 9 have one bad run each (a
  fallback to the original presentation; emulation thread 42% in that run
  against 63% in the good one). Comparisons there need more runs than two.
- **D23Y5's target is not met:** at the original 60 and 3.1 GHz the
  laptop's emulation thread is 54-62% of a core (target under 50%);
  citadel's 48-61% at about 2.7 GHz.

## 4. Where the emulation thread's time goes (MEASURED)

Inclusive share of the emulation thread's CPU time, mean over the four
scenes (one profiled run each; the laptop's ULTRA 120 profile of UI 2 is
included, citadel's crashed, section 1). A block can contain another
(indented). "*" = the function was outside the profile's 45 largest in
some scenes, so the mean is a lower bound; "<0.5" = never in them.

| Block | laptop AUTO | laptop ULTRA 120 | laptop 60 | citadel AUTO | citadel ULTRA 120 | citadel 60 |
|---|---|---|---|---|---|---|
| Hooks (title host code, all) | 28.7 | 26.1 | 25.6 | 27.8 | 28.2 | 23.4 |
| &nbsp;&nbsp;object pass | 10.0 | 9.2 | 9.4 | 8.5 | 8.4 | 7.7 |
| &nbsp;&nbsp;host world draw (`ttk::draw`) | 7.9 | 9.4 | 10.3 | 6.0 | 5.7 | 9.7 |
| &nbsp;&nbsp;visibility walk | 1.8* | 5.3 | 6.3 | <0.5 | <0.5 | 6.1 |
| &nbsp;&nbsp;near clip of world pieces | 1.7* | 5.0 | 6.0 | <0.5 | <0.5 | 5.8 |
| &nbsp;&nbsp;emit | 4.4 | 5.5 | 6.0 | 3.4 | 3.4 | 6.0 |
| &nbsp;&nbsp;coplanar ranking | 2.2* | 1.8* | 2.4* | 1.5* | 1.8* | 1.5* |
| &nbsp;&nbsp;redraw composition capture | 3.7* | <0.5 | <0.5 | 8.1 | 7.9 | <0.5 |
| Device service (all devices) | 21.7 | 28.8 | 30.9 | 21.3 | 20.7 | 33.0 |
| &nbsp;&nbsp;GPU DMA + GP0 (the emulated GPU) | 5.7* | 15.2 | 15.4 | 0.6* | 3.6 | 13.7 |
| &nbsp;&nbsp;&nbsp;&nbsp;GP0 command execute | 3.0* | 10.5 | 10.4 | <0.5 | 1.0* | 8.1 |
| &nbsp;&nbsp;&nbsp;&nbsp;GL backend triangle path | 0.7* | 4.5 | 4.6 | <0.5 | <0.5 | 1.5* |
| Presentation (vblank present) | 7.4 | 5.4 | 5.8 | 9.0 | 8.6 | 5.7 |
| &nbsp;&nbsp;wide picture present (flushes the frame's draws) | <0.5 | 1.5* | 4.0 | <0.5 | 1.3* | 3.6 |
| &nbsp;&nbsp;swap and OSD | 3.2 | 3.7 | 2.6* | 2.9 | 3.0 | 0.8* |
| Redraw tick (presents, provider) | 5.1 | 1.9* | <0.5 | 5.9 | 8.4 | <0.5 |
| Worker image publish | 3.6* | <0.5 | <0.5 | 7.9 | 7.6 | <0.5 |
| &nbsp;&nbsp;image save (a copy) | 0.8* | <0.5 | <0.5 | 3.8 | 3.8 | <0.5 |
| Render-thread capture | 1.6* | <0.5 | <0.5 | 2.9 | 2.1* | <0.5 |
| Audio pump | 2.7* | 1.6* | 2.4* | 0.6* | <0.5 | <0.5 |
| Clock reads (vDSO, a system call on HPET) | <0.5 | <0.5 | <0.5 | 7.9 | 6.2 | 6.3 |
| Mesa driver, on this thread | 3.9 | 6.3 | 5.4 | 4.0 | 4.4 | 3.9 |
| of the clock reads: starvation ring stamps | <0.5 | <0.5 | <0.5 | 2.0* | 0.7* | 0.7* |

Leaf view (exclusive, laptop at the original 60, mean of four scenes):
recompiled game code and its memory accessors about 30% together
(`psx_write_word_raw`, `psx_read_word`, `psx_check_interrupts`), dispatch
(`dirty_ram_dispatch`, `psx_game_find_entry`, the code-validity check
`dirty_ram_text_native_ok_ranges_from`, all already cached: D23G) about
10%, the title's host code about 11%, libc (copies) 6-15% depending on
the redraw path, the Mesa driver 4-6%.

## 5. Ranked costs and what each step can reach (INFERRED from section 4)

1. **The title's host code (hooks), 23-29%:** the Polymorph world (object
   pass, visibility walk, near clip, emit) and, when redraws run, the
   composition capture. Title work, not framework; exact cuts here are
   caches of view-independent results (D17AR3 already caches world-mesh
   coplanar pairs; objects still search every frame, about 1-2%).
2. **The emulated GPU, 14-15% at the original 60 and at the laptop's ULTRA
   120** (redraws off), about 1-6% when the redraws draw: GP0 decode (stays
   on this thread: guest-visible state and the widescreen classifier read
   guest RAM there), the GL backend's batching and the driver. Of it, the
   GL backend and the driver (about 10 points with the frame's flush in the
   present) are what **D23Y8** can move.
3. **The redraw machinery, 11-19% on citadel, about 10% on the laptop at
   AUTO:** tick, provider, image publish (D23Y10's tracked publish targets
   the copy), render-thread capture.
4. **Presentation, 5-9%.**
5. **Clock reads, 6-8% on citadel only** (HPET). On the laptop's TSC they
   are under 0.5%.
6. **The Mesa driver, 4-6% on this thread** (Mesa's own `glthread` would
   move it; measured in note 507).

D23Y5's gap at the original 60 on the laptop is 4-12 points (54-62% to
under 50%). Only D23Y8's full move (about 10 points: the GL backend's
batching, the driver and the frame's flush) is of that size; the other
candidates are 1-3 points each.

## 6. Reproduce

Lab `recomp/analysis/d23y8/` on each machine: `b0.sh` (pure AUTO and ULTRA
120, timing then profiles), `b60.sh` (the original 60);
`python3 y8sum.py b0 b0p` and `python3 prof.py <tag>`.
