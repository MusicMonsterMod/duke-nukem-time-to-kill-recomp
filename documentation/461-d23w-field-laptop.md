# 461 - D23W: the field laptop, measured

The first measurements on real weak hardware, replacing the Deck-class
estimates of notes 444 and 459 for this class of machine. The user considers
this laptop "basically the lower limit".

## Hardware

- CPU: Intel Core i7-5600U, 2 physical cores, 4 logical CPUs (SMT), 2.6 GHz
  base, up to about 3.1 GHz seen; one 15 W package shared with the GPU.
- GPU: Intel HD Graphics 5500 (Broadwell GT2), Mesa 26.2.6, OpenGL 4.6 core;
  300-950 MHz.
- RAM: 8 GB. Display: 1920x1080 at 60 Hz, X11, XFCE with its compositor.
- Power profile "performance", governor schedutil, on mains.

How the user plays other games here, with a good experience: UZDoom at native
1080p with hardware GL, dynamic lights, SSAO, shadow maps and MSAA off, a
60 fps cap; EDuke32 Polymost at 1600x900; Banjo Recompiled at automatic
resolution with MSAA 2x (the user reports it does not run well here). The bar
is therefore a steady 60 at full or near-full resolution.

## Measurement

Private lab `recomp/analysis/d23w/` (local only: `lab.py` from the D23K lab,
`gpu.py`): private copies of the player's cards and savestates, a fresh
default profile (`PLAYER=0`), the `rates` case walking, turning and firing for
25-40 s with one line a second: game frames (`frame`), pictures shown
(`present_ring` total plus replay presents), the busiest thread's CPU share
and the average CPU clock; GPU busy over the run. GPU busy is vendor-neutral
(`gpu.py`): `nvidia-smi`, `gpu_busy_percent` on AMD, and on Intel 1 minus the
RC6 (idle) residency over wall time from
`/sys/class/drm/card*/power/rc6_residency_ms`, readable without root
(`intel_gpu_top` needs CAP_PERFMON). Busy percentages are at the clock the GPU
chose, so a low clock reads as busier than the same work at full clock.

UI slot N is savestate file slot N-1 (`SLOT=1`, `7`, `8` for UI slots 2, 8, 9).

## 1. What a fresh install did: slow motion

Fresh Modernized defaults before this job: FRAME RATE AUTO, internal scale 4x,
lighting Enhanced with sharp live shadows, extended draw distance. The user's
first play: "it runs like ass". Measured, offscreen, UI slot 2:

| Setting | Game frames/s | Pictures/s | Busiest thread | GPU busy | Underruns (25 s) |
| --- | --- | --- | --- | --- | --- |
| AUTO (replay, 3 workers) | 33 | 53 | 95% | 95% | 14 |
| fixed 60 (original presentation) | 60 | 60 | 63% | 60% | 0 |

AUTO counted 4 logical CPUs, so D23P's cap allowed up to 120, and AUTO at any
cap keeps the replay path (redraw workers and per-frame capture) on: the
emulation thread ran out of time and the game itself slowed to about half
speed, with the overclock safety net paused every few seconds ("emulation
behind real time"). One or two workers instead of three changed nothing (the
emulation thread stayed at about 95%): the cost is the replay path's own share
of the emulation thread, not the worker processes. D23P's comment promised
"60 (no redraws)" below 4 CPUs, but `apply_rate` turned redraws on for every
AUTO.

Fix (framework `render_thread.c`, `replay_rate.h`; game
`frame_replay.cpp`): `psx_host_physical_cores()` counts the distinct
(package, core) pairs of the CPUs the process may run on (Linux sysfs; the
logical count elsewhere), the cap is `replay_auto_cap_for(cpus, cores)` (2 or
fewer cores: 60, else as D23P), and AUTO capped at 60 launches, and switches
in OPTIONS, exactly as a fixed 60: the original presentation, no replay, no
workers. The log line is now `AUTO frame rate: at most 60 images a second (4
CPUs, 2 cores; ...)`. After it, AUTO in UI slot 2: 60 game frames, 60
pictures, emulation thread 62%, no underrun.

## 2. The CPU

With the original presentation the emulation thread runs at 55-70% in UI slots
2, 8 and 9. Its share rises when the CPU clock falls: in UI slot 8 the
busiest thread went from about 60% at 3.1 GHz to 91-97% at 1.6-2.1 GHz, with
the GPU busy. The CPU and GPU share one 15 W budget, so GPU work costs CPU
headroom too. Single setting changes in UI slot 9 (CPU overclock 100%,
lighting off, original draw distance, Vanilla) all ran the thread at about
60%; one early baseline at 98-99% for 15 s did not repeat in two longer runs
and is taken as a clock dip, not steady work.

## 3. The GPU and AUTO resolution

UI slot 8, original presentation, 60 Hz:

| Internal scale | Offscreen GPU busy | Real display |
| --- | --- | --- |
| 1x | 19% | - |
| 2x | 30% | 60 every second, GPU busy 71%, 0 underruns |
| 3x | 46% | 58 average, one second at 42, 2 underruns |
| 4x (old default) | 64% | 55 average, dips to 41-48, 3 underruns, GPU busy 85% at 852 MHz |

Showing the picture in a 1080p window through the compositor adds about 20
points over offscreen. Lighting off took UI slot 9 from 59% to 39% (Vanilla:
11%), so the lighting is most of the GPU work; at 2x it fits with lighting on,
and the user's other games here keep 60 at native resolution, so lighting
stays on and the resolution adapts.

The internal scale is fixed when the GL context is created, so AUTO picks it
before launch from a measurement:

- `--gpu-probe` (runtime `gpu_probe.c`, first argument) opens a hidden GL 3.3
  window, draws a synthetic 4x frame (2048x960, three layers, a shader shaped
  like the lit textured one: a nearest texture fetch with the texel-0 discard,
  a linear lightmap fetch, four light terms), scales it to 1920x1080, and
  times 24 frames with GL_TIME_ELAPSED after 30 warm-up frames: 6.33-6.94 ms
  here over 8 runs, about one second each.
- `gpu_auto.py` takes the largest scale whose estimate (probe x scale^2/16)
  fits 2.5 ms: 4x at 2.5 ms or less, 3x at 4.44 or less, 2x at 10 or less,
  else 1x. Calibrated here: 2x holds, 3x does not quite (estimate 3.6 ms), so
  the cut lies between them. A GPU about 2.6 times faster than the HD 5500
  gets 4x.
- The result is cached in `recomp/config/gpu-probe.json` (ignored) with a
  fingerprint of the DRM cards' PCI ids and the runtime binary, and measured
  again when either changes. A failed probe is not cached and gives 2x.
- `internal_scale` 0 is AUTO, the new Modernized default (`--internal-scale
  auto`, OPTIONS > DISPLAY > RESOLUTION > AUTO); saved fixed scales are kept.

## 4. Fresh install after the fix, real display

Fresh default profile, real 60 Hz display, windowed fitting the screen,
walking, turning and firing for 30 s:

| View | Game frames/s | Pictures/s (lowest tenth) | Busiest thread | GPU busy | Underruns |
| --- | --- | --- | --- | --- | --- |
| UI slot 2 | 60 | 60 (59) | 65% | 65% at 525 MHz | 0 |
| UI slot 8 | 60 | 60 (59) | 66% | 71% at 580 MHz | 0 |
| UI slot 9 | 60 | 60 (59) | 63% | 66% at 574 MHz | 0 |

The worst single seconds were 51 (UI slot 2) and 54 (UI slot 9), each with
one "emulation behind" warning; no audio underrun in any run.

## 5. The D23U depth prepass

`PSX_GL_ZPREPASS=1` on the same fresh-install runs: UI slot 8 GPU busy 70%
(71% without), UI slot 2 68% (65% without), 60 throughout; within the
run-to-run spread. The prepass statistics print only in the redraw phase
report, so whether the original present path engaged it was not established
from the logs. With no measured gain here, mixed results on the 1080 Ti and a
loss there with one render thread (which a 4-7 CPU machine gets), it stays off
by default.

## 6. Second pass, the user's own settings (2026-11-13)

The user then played with FRAME RATE DISPLAY ("to actually attempt playing at
60"), RESOLUTION 3x, LIGHTING RELIT and first person. The session log showed
DISPLAY had brought the replay path back: 17-56 game frames a second, 10-30
pictures, underruns most seconds. DISPLAY now takes the same CPU cap as AUTO
(`launch_rate`: 2 or fewer cores -> the original 60 presentation); a fixed
number is still never changed.

Reference, UZDoom 4.14.3 with the user's own settings, Ancient Aliens MAP01,
walking (xdotool), 60 cap: GPU busy 62% at 624 MHz, main thread 28% of a CPU,
CPU clock 2.4 GHz. (Its title loop and the WAD's demos were not usable:
"Cannot play non-UZDoom demos.")

Where the GPU went, measured per render pass with Mesa's `INTEL_MEASURE`
(120 frames, UI slot 8, real display): about 9.5-10 ms of GPU a frame, of it
5-6 ms the scene and about 3.6 ms around it: a 1920x1080 copy of the drawable
at every present for hold-last (1.0 ms), the widescreen centre copy over the
full VRAM height (1.1 ms), a full clear of the window to paint its pillarbox
bars (0.4 ms). The `frame_perf` present timer spans the swap and is not a
drawing cost. Changes (framework `gpu_gl_renderer.c`, picture unchanged):

- hold-last's drawable copy runs only while rewind is enabled or a netplay
  session is active, its only callers (`PSX_HOLD_CAPTURE=1` restores it);
- the widescreen centre copy honours the band each caller passes, as replays
  already did (`PSX_WIDE_CENTER_FULL=1` restores the full height); the
  opening's 30 screenshots match the full-height copy row for row;
- a letterboxed or pillarboxed present clears only the bars (screenshot bars
  read 0 on both sides);
- developer `PSX_GL_BLIT_STATS=1` prints each blit site's count and pixels.

Blits 2.21 -> 0.55 ms a frame, colour clears 0.84 -> 0.42 ms. The user's
settings on the real display, 30 s walks, before -> after the pass: lowest
tenth 56/55/55 -> 58/59/58 (UI slots 2/8/9), GPU busy 80/78/75% ->
72/73/68%, "behind" seconds 2/4/1 -> 2/1/1, averages 59-60.

The remaining short slowdowns are CPU, one-off events on the emulation thread
(the 24 ms lamp snapshot at a level start, sleeping-enemy handovers, relit room
changes), each followed by the 5 s overclock pause. Continuous firing showed
none; a walk with lights off showed none; shadow quality made no difference.
The emulation thread's profile (host profiler, 15,008 samples) is spread thin:
generated code 16%, memory access 13.5%, host geometry (`near_clip.cpp`) 9.5%,
cycle timing 8.2%, emulated GPU 6.1%, no single function above 3.4%.

Jumps (user: Duke's jump "seems to have taken on some of the sounds he made
when doing say, long jump"): no jump code changed this session or this month,
and the frame-rate mode does not change which jump runs. Found: walking at a
step, the step probe switches the gait to run (76) to climb it, and the D08Z6
quick takeoff accepted only the walk gait, so that jump was the original
running jump 104 (the long jump). It now takes a step-forced run gait as the
walk it is (`modern_controls.cpp`), checked in UI slot 2 (96 -> 98 instead of
104); the step's scramble that follows still plays 0x226A0/0x2F360, so whether
those are the sounds the user heard is for the user's ear.

## Limitations

- One laptop. The scale budget is calibrated on one GPU; a faster mid-range
  GPU at a high refresh rate can still be over budget (the probe budget is for
  60 pictures a second). D23P2 still owns refresh-aware presets and light and
  shadow quality.
- The probe is synthetic: it tracks fill and shading throughput, not the
  game's draw-call or CPU submission cost.
- Real-display figures are from the lab with an uncaptured mouse; the user's
  own play of the new defaults is the gameplay acceptance.
- The physical core count reads Linux sysfs (with the process affinity) or, on
  Windows, GetLogicalProcessorInformation (all cores, not the affinity); the
  Windows branch was compile-checked with MinGW (-Wall -Wextra -Werror), not run.
