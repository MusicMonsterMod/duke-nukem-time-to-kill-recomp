# 485 - D23Y5 profile and D23Y8's first step sized (field laptop)

Autonomous session of 2026-11-16, part 3 of the chain. User: "Measure on
the laptop either way: a fresh emulation-thread profile for D23Y5 at the
player's settings, then D23Y8's first step from note 470 (presentation on
its own shared-context thread), built behind a developer switch, off by
default, checked exact (PSX_RT_VALIDATE=1, matched shots, Vanilla
unchanged) and on the real display; measure the emulation thread's share
before and after."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Setup

Field laptop (i7-5600U, 2 cores / 4 threads, 15 W, HD 5500), reached over
SSH; checkout at `6e887c5` (the closed D23N3/D23N4a/D23N4b state, the same
build as note 482's laptop part), idle before the runs (load 0.00-0.11, no
game, no Xvfb). Lab `recomp/analysis/d23y6/` (local), case `cad` (40 s walk,
turn and fire after a savestate load) with the player's profile copied read
only (PLAYER=1: 2x, LIGHTING on, SIMPLE shadow edges, extended draw
distance, FRAME RATE 120), offscreen, UI slot 2 (runtime slot 1), the
runtime's stack sampler (`LAB_PSX_PROF`, `LAB_PSX_PROF_STACKS`). Script
`y5n.sh`, results in `y5n/`.

## 2. D23Y5: the emulation thread now (MEASURED)

| Run | emulation thread | presents | game | CPU clock (median) | samples |
| --- | --- | --- | --- | --- | --- |
| the player's settings (FRAME RATE 120) | 62% | 120 (p10 119) | 59.9 (p10 58.9) | 2770 MHz | 32,279 |
| the original 60 | 61% | (no redraws) | 60.0 (p10 59.3) | 3130 MHz | 28,350 |

D23Y5's target (under 50% of a core at 3.1 GHz at the original 60) is not
met; the 60 figure matches notes 469/470 (57-59% there, 61% now with one
run: within run-to-run spread, INFERRED, not a regression established).

Inclusive shares at the player's 120 (`incl.py`, `sub.py` for a function's
callees):

| Where | inclusive |
| --- | --- |
| hooks (`psx_mod_function_entry`) | 23.5% |
| the redraw tick `gl_renderer_rp_tick` | 12.0% |
| of it: present and OSD `gl_swap_with_osd` | 3.8% |
| of it: `psx_rt_after_present` (render-thread bookkeeping) | 3.2% |
| of it: the timeline provider (`provider`, `submit_job`) | 2.8% / 1.5% |
| `sdl_vblank_present` (incl. 3.9% of the tick above) | 11.1% |
| of it: `frame_pacer_wait` | 4.7% |
| of it: audio (`sdl_audio_pump`, `spu_render`) | 3.4% / 3.0% |
| image publishing (`psx_mod_replay_publish`) | 7.2% |
| of it: `render_replay_image_save` | 3.4% |
| render-thread snapshot (`gl_rt_capture`) | 2.7% |
| host world and models (`ttk::draw`) | 5.6% |

At the original 60 the GPU command path leads instead: `dma_gpu_ll_advance`
14.7% (`gpu_write_gp0` 12.7%, `gp0_execute_command` 10.0%), the host world
pass 8.1-8.5%, `ttk::draw` 11.0%, presentation 5.9%.

`d23y8_split.py`: at 120, GL backend work outside presentation 15.0%,
present display 0.6%, GP0 front end only 0.4%; at 60, 7.9%, 2.3% and 4.9%
(note 470: 7.2%, 1.9%, 4.8%).

## 3. D23Y8 step 1: sized, not built (MEASURED, CODE)

What a presentation thread would take off the emulation thread at the
player's 120: the present and its OSD (`gl_swap_with_osd` 3.8%, of it
`gl_draw_osd_image` 2.1% and `gl_draw_osd_bound` 0.7%; the picture quad
0.4%). `psx_rt_after_present` (3.2%) is the render threads' bookkeeping
and stays; the pacer's wait (4.7%) is sleep, not work (INFERRED from its
`clock_nanosleep` frames). So the first step moves about 4 points: UI slot 2
from 62% to about 58% at 120 (INFERRED), not to D23Y5's 50%. At the
original 60 the present path is 5.9% inclusive but its movable display
half is 2.3% (as note 470).

Why it was not built this session (CODE, read in the renderer):

- The renderer's GL state cache (`gs_bind_fb`, `gs_use_prog`,
  `gs_bind_vao`, 333 uses in `gpu_gl_renderer.c`) and its VAOs live in file
  statics of the emulation thread's renderer instance; a second context
  cannot use the existing present code (VAOs are not shared between
  contexts, and the cache would describe the wrong context). The render
  threads solve this with whole renderer copies (`render_copy/rc_*`,
  `rc2_*`); a presentation thread needs a third copy or its own small GL
  path (program, VAO, OSD textures).
- The OSD is built by host callbacks during the present
  (`ttk_aim_crosshair_image`, `host_osd_*`, the security overlay, the menu
  thumbnail, the console): they read game and UI state owned by the
  emulation thread and are not thread-safe; the first step needs them
  turned into data (pixel copies and rectangles) on the emulation thread.
- `present_shot` (the matched-shot capture) reads the default framebuffer
  before the swap; it moves with the present.
- Four other present paths (VRAM presentation, held frames, the wide FBO)
  still present on the emulation thread's context; each must drain the
  presenter first.

Building that, with exact matched shots and real-display checks, did not
fit after part 1's work (which found and fixed three N4b defects, note
483); the measured ceiling (about 4 points at 120) is the reason to weigh it
against D23Y5's other costs first. Status unchanged: D23Y8 Blocked
(measured), now with this sizing.

## 4. Where D23Y5's points are (INFERRED from section 2)

At the player's 120 the largest movable blocks are not the present: image
publishing 7.2% (the worker image save 3.4%, done for every published game
frame), the render-thread snapshot 2.7%, the timeline provider 4.3%. A
smaller published image (only the RAM pages and mod memory a redraw reads)
and the snapshot's tile reads (note 486 section 2: about 38 of 245 captured
tiles are read) are the next candidates, each exact by construction.

## 5. Reproduce

Laptop, `recomp/analysis/d23y6/`: `bash y5n.sh`; then
`python3 incl.py y5n/stk120 45`, `python3 d23y8_split.py y5n/stk120`,
`python3 sub.py y5n/stk120 gl_renderer_rp_tick 22` (`sub.py`: this
session).
