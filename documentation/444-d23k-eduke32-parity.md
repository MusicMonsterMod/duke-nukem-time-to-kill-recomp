# D23K - Performance parity with EDuke32: research and job plan

Status: report for review (2026-11-11). Job board: D23K; the plan's jobs are
D23L-D23R with D41G and D41F2. User goal: "performance comparable or better
than eduke32".

## 1. Method

Same machine and display for both games: Intel i7-5960X (8 cores, 16
threads, 2014), NVIDIA GTX 1080 Ti (driver 580), one 1920x1080 monitor at
180 Hz, Linux Mint with Cinnamon (Muffin compositor, X11).

- **GPU**: clock-weighted utilization of the whole GPU (`nvidia-smi`,
  utilization x graphics clock / 1999 MHz, every 200 ms over 15 s), plus
  `nvidia-smi pmon` per program (game, `cinnamon`, `Xorg`). pmon's per-program
  split is coarse; the whole-GPU figure is the one to compare.
- **CPU**: per thread from `/proc` over the same 15 s, summed over all the
  game's processes (the redraw workers are separate processes).
- **Our game**: lab `recomp/analysis/d23k/lab.py` (local), the player's own
  saved profile copied read-only (Relit, SHADOW EDGES SMOOTH, internal scale
  3, 16:9, AUTO = 180 Hz, window 1346 wide), private card copies, turning all
  the time (`SPIN=dx=3`). Views: UI slot 2 (level 6, the most drawn in
  natural play) and UI slot 8 (level 6). No test light.
- **EDuke32**: the player's own build (r10669) copied to a scratch folder
  with a copy of their settings, Duke 3D attract demos 1 and 2 (real levels,
  moving camera), `r_maxfps 180`, `r_vsync 0`, frames counted with a
  `SDL_GL_SwapWindow` shim.
- Developer `PSX_GPU_PHASES=2` (D23J) for our GPU time per kind of work in
  each redraw; headless runs (SDL `offscreen`, EGL on the same GPU) for CPU
  and low-end tests, so the display does not mix in.

## 2. The yardstick on the real display

Whole GPU, clock-weighted, 15 s. Desktop idle: 0.5%.

| Game | Mode | Picture | GPU |
| --- | --- | --- | --- |
| EDuke32 Polymost | windowed 1280x720 | demos 1, 2 (167-174 fps) | 7.2%, 7.5% |
| EDuke32 Polymer | windowed 1280x720 | demos 1, 2 (174 fps) | 8.2%, 8.2% |
| EDuke32 Polymost | fullscreen 1920x1080 | demo 1 | 3.6% |
| EDuke32 Polymer | fullscreen 1920x1080 | demo 1 | 3.9% |
| Ours, UI slot 2 | windowed (player's) | ORIGINAL / RELIT / RELIT + SMOOTH | 26.6% / 28.9% / 28.8% |
| Ours, UI slot 8 | windowed (player's) | same | 22.4% / 26.2% / 25.3% |
| Ours, UI slot 2 | borderless 1920x1080 | same | 20.7% / 23.3% / 25.7% |
| Ours, UI slot 8 | borderless 1920x1080 | same | 17.2% / 19.3% / 19.0% |

Ours held 180 presents a second in every run. Per presented frame at full
clock, EDuke32 uses about 0.2 ms of GPU (3.6-3.9% of a 5.6 ms frame); we
use about 1.0-1.4 ms in fullscreen: **five to seven times as much**.

The compositor: a windowed game at 180 Hz is composited by Cinnamon every
frame (pmon: `cinnamon` 8-14 and `Xorg` 4-7 with our window; at most 2 in
borderless, where SDL's bypass request is honoured). Windowed costs us about
5-6 points more than borderless; EDuke32 about 3.6 points. The player's
40-50% whole-GPU reading (D23J playtest) is consistent with these windowed
figures in the heaviest views plus the desktop and other programs.

## 3. Where our cost goes

### GPU per redraw

`PSX_GPU_PHASES=2`, UI slot 2, turning, headless, 180 Hz, steady samples:

| Work | ORIGINAL | RELIT + SMOOTH |
| --- | --- | --- |
| textured draws (6 batches) | 0.13-0.28 ms | 0.28-0.47 ms |
| copy-on-write save (9) + restore | 0.067 + 0.075 ms | 0.067 + 0.076 ms |
| uploads (11) | 0.055 ms | 0.062 ms |
| depth clear | 0.013 ms | 0.04-0.055 ms |
| flat draws | 0.04-0.05 ms | 0.03-0.06 ms |
| band capture, mirror, fills, wide clear, other | 0.08 ms | 0.08 ms |
| end of redraw (output pass) | 0.11 ms | 0.11 ms |

So a redraw is about 0.55-0.65 ms ORIGINAL and 0.75-0.95 ms RELIT + SMOOTH.
About 0.35-0.4 ms of it is the cost of redrawing into emulated PS1 VRAM
(saving and restoring what the redraw overwrites, re-uploading, clears, the
band and the output pass) rather than drawing the picture itself.

Lights and shadows in natural play: RELIT adds 2-3 points over ORIGINAL in
these views; SMOOTH adds 0-2.5 points over shadows off (no test light). The
D41F1 figure of about 10 points for SMOOTH was measured with a test light
next to walls; natural shadowed lights (rounds in flight, blasts, placed
lamps) need their own measurement (D41F2).

### CPU

Headless, UI slot 2, RELIT + SMOOTH, turning:

| Frame rate | Presents/s | GPU | CPU (cores) | of it workers |
| --- | --- | --- | --- | --- |
| 180 | 180 | 24.2% | 2.60 | 1.50 (3 x 0.50) |
| 120 | 120 | 17.2% | 1.96 | 1.06 |
| 60 | 60 (no redraws) | 5.5% | 0.57 | 0 |

The emulation thread uses 0.6 of a core and the render thread about 0.5 at
180 Hz. The three redraw workers re-run the game's own drawing code for each
redraw (render_worker.c): about 10 ms of CPU for each of the 150 redraws a
second. EDuke32 renders on one thread (it shows 100% of one core, but most of
that is its frame limiter waiting; its real work is less).

### Fewer cores and less GPU

| Run (headless, 180 Hz asked) | Result |
| --- | --- |
| 4 logical CPUs (`taskset -c 0-3`) | 177 presents/s, "audio running low", the D23H governor steps down |
| 2 CPUs | 88 presents/s (governor at every 2nd refresh) |
| 1 core, 2 threads | 58 presents/s |
| internal scale 1 | GPU 16.3% (24.2% at scale 3) |

The governor sheds cleanly, but 180 Hz needs more than four CPUs today.
Also seen: a fixed 90 Hz setting presented 60 a second in one headless run
(to check in D23P), and 4 of 7 launches on the real display tripped the 4 s
starvation watchdog in the renderer's restart right after a savestate load
at boot (lab only; the runs above used a 60 s watchdog; D23Q).

### Weak hardware (estimate)

From public figures (sources in section 4), a Steam Deck GPU has about 1/7
of a 1080 Ti's arithmetic (1.6 against 11.3 TFLOPS) and 1/5.5 of its
bandwidth; an Intel Iris Xe laptop GPU is similar. Our 1.0-1.4 ms a redraw
would be about 7-10 ms there: under 120 presents a second, and close to 60
with lights. EDuke32's 0.2 ms would be about 1.4 ms. The Deck's CPU (4 cores,
8 threads, lower clocks) would also hold us below 180 (table above).

## 4. How others do it

Sources gathered for this report (a research pass, 2026-11-11):

- **EDuke32 Polymer** draws lights in passes, at most `r_pr_maxlightpasses`
  (default 5) per surface, shadows only for spotlights (SE50), no shadow
  caching; its author called the walk of the map for each shadow light its
  main CPU weak point. Polymost has no lights. A cheap yardstick, not because
  its lighting is clever but because it does little of it.
  [Polymer wiki](https://wiki.eduke32.com/wiki/Polymer),
  [console commands](https://wiki.eduke32.com/wiki/Console_commands),
  [SE50](https://wiki.eduke32.com/wiki/SE50).
- **RT64 / Zelda64Recomp**: draw calls are recorded once and replayed by a
  second renderer at another rate; matched transforms are interpolated and
  vertices re-processed on the GPU, without running game code again.
  [RT64](https://github.com/rt64/rt64). This is the model for removing our
  workers (D23N).
- **Doom ports** interpolate view and objects between 35 Hz tics and redraw
  in full each frame, as we do. **Ironwail** keeps a full redraw cheap with
  persistent mapped buffers and indirect multi-draw.
  [Ironwail](https://github.com/andrei-drexler/ironwail).
- **Shadow caching**: Doom 2016 keeps shadow maps in an atlas and redraws only
  those that changed; Unreal caches static geometry's shadow depths for
  movable lights (14 ms -> 1 ms in Epic's example). Our maps are already
  redrawn only when a light or the shadow mesh changes; the remaining cost is
  the lookup per pixel (D41F2).
  [Courreges, Doom 2016](https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/),
  [Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/movable-light-mobility-in-unreal-engine).
- **Clustered shading** (Olsson, Billeter, Assarsson 2012) pays off with many
  overlapping lights; with at most 32 lights a list per triangle culled on
  the CPU is as good and cheaper to build (D41G).
  [Chalmers](https://research.chalmers.se/en/publication/161725).
- **Reprojection and frame generation**: VR timewarp and ASW halve the
  rendering cost but leave holes and halos; FSR 3 frame generation needs
  about 60 real frames a second. Only worth it as a weak-hardware option on
  top of real redraws, never from the 30 Hz frames.
  [Meta ASW](https://developers.meta.com/horizon/blog/asynchronous-spacewarp/),
  [AMD](https://gpuopen.com/amd-fsr-framegeneration/).
- **Compositor**: SDL sets `_NET_WM_BYPASS_COMPOSITOR`; Muffin and Mutter
  unredirect only a fullscreen window, so a windowed game is composited every
  frame. [SDL](https://wiki.libsdl.org/SDL2/SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR),
  [muffin #847](https://github.com/linuxmint/muffin/issues/847).
- **Weak GPUs**: [Steam Deck](https://en.wikipedia.org/wiki/Steam_Deck),
  [GTX 1080 Ti](https://www.vortez.net/articles_pages/nvidia_gtx_1080_ti_review,2.html).

## 5. The parity bar

Measured on the player's machine with the lab above, against EDuke32 on the
same display:

| Metric | EDuke32 | Ours now | Bar |
| --- | --- | --- | --- |
| GPU per presented frame, 180 Hz, full clock (fullscreen) | about 0.2 ms | 1.0-1.4 ms | 0.4 ms or less |
| Whole GPU, 180 Hz, windowed | 7-8% | 22-29% | 12% or less |
| CPU at 180 Hz | under 1 core | 2.6 cores | 1.5 cores or less |
| 180 Hz with 4 logical CPUs | yes | no (governor sheds) | yes |
| Weak GPU estimate (1/7 of ours) | about 1.4 ms | 7-10 ms | under 4 ms (holds 120+) |

Pictures identical (image comparison as in D23J), feel unchanged or better,
Vanilla unchanged, 180 presents a second held in a 60 s walk.

## 6. The plan

Ranked by expected gain against risk. Gains are estimates from section 3.

| Order | Job | What | Expected gain |
| --- | --- | --- | --- |
| 1 | D41G | Light lists per triangle and the lit path | RELIT's 2-3 points, more with many lights |
| 2 | D41F2 | Shadow lookups: skip them where a surface is wholly lit or shadowed | SMOOTH's cost with shadowed lights |
| 3 | D23M | Direct redraws: draw each redraw into its own target, not emulated VRAM | 0.3-0.4 ms of 0.6-0.95 ms a redraw (about 35-45%) |
| 4 | D23N | Captured redraws: replay a recorded world-space frame instead of re-running game code in workers | about 1.5 cores |
| 5 | D23O | Leaner render thread submission | part of 0.5 core |
| 6 | D23L | Display: compositor and the output pass | 5-6 points windowed, 0.11 ms a redraw |
| 7 | D23P | Low-end scaling: presets, AUTO from measured headroom | weak hardware holds 120/90 |
| - | D23Q | Startup stall after an early savestate load (lab) | reliability |
| - | D23R | Parity benchmark: the repeatable comparison and final acceptance | the evidence |

D41G and D41F2 come first because they were selected for this session and
are bounded shader work; D23M and D23N are the large structural wins and
need their own research steps before any change.
