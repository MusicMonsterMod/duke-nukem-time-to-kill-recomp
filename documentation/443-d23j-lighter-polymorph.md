# D23J - Lighter Polymorph: base cost of the 180 Hz redraws

Status: Done (user-accepted 2026-11-11: "the playtest felt absolutely perfect"; at most about 50% GPU over the castle, about 40% in UI slot 2, whole GPU, windowed). Job board: D23J; follow-up D23K (parity with EDuke32). Framework only
(`recomp/psxrecomp`, exported in `recomp/patches/time-to-kill-accepted-source.patch`).
No player-facing option changed; Vanilla draws the same pixels.

## 1. How it was measured

The player's monitor was switched off during the work (xrandr: every output
disconnected; a windowed game then stalls before its first frame), so the
lab ran the game headless on the player's GPU: SDL's `offscreen` video driver
(EGL on the NVIDIA GTX 1080 Ti), frame rate 180, the player's presentation
(OpenGL, scale 3, linear output), UI slot 8 (level 6) from a private copy of
the cards, lab `recomp/analysis/d23j/lab.py` (local; `OFFSCREEN=1`, case
`perf2`, `SPIN=dx=3` turns all the time). Figures are clock-weighted GPU
utilization (`nvidia-smi`, utilization x graphics clock / 1999 MHz, sampled
every 200 ms over 12-15 s), per program with `nvidia-smi pmon`.

Developer `PSX_GPU_PHASES=2` (new) puts a GPU timestamp at every change of
kind inside each redraw and logs the mean per kind every 5 s (`[gpu kinds]`).
The kinds add up to the redraw; the intervals include gaps where the GPU waits
for the render thread, so they rank the work but overstate busy time.

## 2. Findings

- Batching is already good: about 5 textured draws and 2 flat draws a
  redraw (`PSX_GL_FLUSH_DIAG=1`). Merging batches (plan step 2) has nothing to
  gain.
- Utilization rises only with the redraw rate (about 0.8 ms of busy GPU a
  redraw at full clock); at 30 or 60 Hz the GPU idles at low clock.
- One redraw, lighting ORIGINAL, still view, before: textured draws 0.30-0.41
  ms, copy-on-write save 0.13 + restore 0.14, depth clear 0.13-0.15, band
  capture 0.07, uploads 0.05, flat 0.03-0.05, the rest about 0.08.
- The depth clear was slow only because the copy-on-write used framebuffer
  blits of depth-stencil rectangles: after `glCopyImageSubData` copies the same
  clear takes 0.014 ms.
- Compiling the lighting layer out of the textured shader cut lighting
  ORIGINAL by about a fifth while turning (16.1% -> 12.8%).
- The render thread draws each redraw from a fixed snapshot of its game frame,
  so the same snapshot and the same command stream give the same image.

## 3. Changes (`runtime/src/gpu_gl_renderer.c`, `render_thread.c`, `render_worker.c`)

1. **Raw copies.** Copy-on-write saves and restores, the band capture and the
   wide centre copy use `glCopyImageSubData` (GL_ARB_copy_image) instead of
   framebuffer blits. The first copy is checked; a GL error keeps the blits
   for the rest of the run. Developer `PSX_REPLAY_COPY_IMAGE=0` keeps them.
2. **Colour-only copy-on-write on the render thread.** The render thread's
   surfaces only ever serve redraws of a loaded snapshot. Depth is never
   observable there (every session and drawing area clears its depth before
   the first host triangle), nor is stencil while it is stale
   (`s_stencil_valid` 0, as opaque batches leave it; it is rebuilt from colour
   alpha before any mask test). Such a session saves and restores colour only.
   The emulation thread's own sessions keep everything. Developer
   `PSX_REPLAY_DS_SKIP=0`.
3. **Identical redraws reused.** The render thread keeps a copy of the last
   stream it drew; a request with the same snapshot (and generation), the same
   stream and the same SHADOW EDGES setting copies that image into its cache
   entry (`gl_rt_copy_cache`) instead of drawing it. Counted as `reused` in the
   debug port's `render_thread`. Developer `PSX_REDRAW_REUSE=0`.
4. **Lean textured program.** A second build of the textured shader without
   the lighting layer (`SH_LIGHTS 0`, which also drops the shadow layers). A
   batch drawn while no light has any effect (no lights and ambient 1, the
   shader's own condition for its plain path) uses it. Both programs' uniform
   locations are kept in tables (`TEX_LOCS`); values set once are set in both,
   and the light and shadow uniforms are sent again when the full program
   returns. Developer `PSX_TEX_LEAN=0`.

The redraw stays a full redraw at every present; nothing is interpolated,
skipped or shown from an older frame except an image proven identical (3).

## 4. Correctness checks

| Check | Result |
| --- | --- |
| `PSX_RT_VALIDATE=1`: every 8th render-thread image against a full redraw on the emulation thread (old copy-on-write, no reuse), still and walking, ORIGINAL, ENHANCED + SMOOTH, RELIT + SMOOTH and SIMPLE | 3195 + 1821 exact, 0 bad |
| `PSX_REPLAY_VALIDATE_LEAN=1` (new, with `PSX_RENDER_THREAD=0`): each redraw drawn again with the full program only and compared | 2112 exact, 0 bad (ORIGINAL, ENHANCED, RELIT + SMOOTH; lean and full mixed) |
| `PSX_REPLAY_COW_CHECK=1` (surfaces read back before and after each session), 15 s walks, render thread on and off | 0 mismatches |
| 60 s walk at 180 Hz, ENHANCED + SMOOTH | 173-180 presents a second, pace minute 179.8/s (old path the same) |
| `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`, `ttk-sky-test` | PASS |
| Vanilla (`--vanilla`) | boots and draws; all its batches use the lean program |
| Patch on a clean pinned `08ec704a` | applies; 1644 files match |

The copy-on-write checker is slow enough to trip the starvation watchdog at
120 Hz with the old path too; the runs above used
`PSX_STARVATION_TIMEOUT_US=60000000`. Python suite: 248 tests, 2 failures
that predate D23J (`test_gl_host_flat_depth` extracts renderer code that has
lacked D50-era symbols since before this job; `test_primitive_trace` fails in
`ttk-near-test`, which no D23J file feeds). The flat-depth harness gained
stubs for the new timing hooks.

## 5. Results (headless, player's GPU, 180 Hz, scale 3, slot 8)

Clock-weighted utilization, mean of two alternating runs, old path = the four
developer switches off.

| View | Lighting | Before | After |
| --- | --- | --- | --- |
| level 6 view (still) | ORIGINAL | 16.0% | 10.0-13.3% |
| level 6 view (still) | ENHANCED + SMOOTH | 20.8% | 15.1-16.3% |
| level 6 view (still) | RELIT + SMOOTH | 22.1% | 15.3% |
| turning all the time | ORIGINAL | 19.0% | 13.8-14.1% |
| turning all the time | ENHANCED + SMOOTH | 25.0% | 21.1-21.4% |
| turning all the time | RELIT + SMOOTH | 26.7% | 21.9% |

Each change alone (same conditions): raw copies about 0.5-0.7 points;
colour-only copy-on-write cut copy-on-write from about 0.26 to 0.17 ms a
redraw; reuse about 20% of the still-view redraws (14.8% -> 12.1% ORIGINAL),
hardly any while turning; lean program while turning, ORIGINAL 17.6% ->
14.7%. When the GPU has less to do it also drops a clock tier.

## 6. Not measured yet, and what is left

- **The real display.** The player's 28% (lighting off, level 6 view) was the
  whole GPU on the desktop; headless the game alone measured about 16%. A
  windowed game at 180 Hz is also composited by Cinnamon 180 times a second
  (`org.cinnamon.muffin unredirect-fullscreen-windows` is false; SDL asks for
  a compositor bypass, which Muffin honours for fullscreen windows). To check
  with the monitor on: windowed against F11 borderless, per program
  (`nvidia-smi pmon`: the game, cinnamon, Xorg).
- Copy-on-write could skip the wide surface's centre (the band capture would
  take the centre from the canonical surface): about 0.05 ms a redraw.
- The repeated small uploads (about 0.05 ms a redraw) could be skipped when
  the bytes already match and the rect holds no GPU drawing.
- With lights (ENHANCED, RELIT) the textured draw is most of the remaining
  cost: the light and shadow work per pixel (D41G light lists, D41F2 shadow
  reuse).
