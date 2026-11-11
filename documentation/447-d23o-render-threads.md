# D23O part 1 - Two render threads where the CPUs allow

Status: part 1 accepted 2026-11-11; part 2 below. Job board: D23O (part 1 of the leaner
render thread).

## Report

Playtest 2026-11-11, session log 19:00 (real display, 180 Hz, after D50A):
UI slot 9 (the level 3 corridor) "still unplayable", UI slot 1 slow in the
same level, `light bake on|off` no change. The log: redraw-only switched off
for 25-50 presents again and again ("workers behind", "a redraw failed"),
and almost every miss was "render thread drawing": the worker had finished
the redraw in time (job age at its present mostly 0 ms) but the render
thread had not drawn it yet.

## Root cause

One render thread draws every redraw (`render_thread.c`, `s_threads` 1; the
second instance existed, off by default). In the corridor at 180 Hz it ran
at about 90% of a core; its CPU profile (`PSX_PROF_RT=1`) there: libc 19%,
the NVIDIA driver 12%, per-triangle light lists (`light_mask_tri`, D41G)
7.6% and shadow classes (`sh_classify_tri`, `cl_classify`, `cl_mix`, D41F2)
10.3%, the rest stream playback and triangle setup. Finished redraws queued
behind it and missed their presents.

Corridor walk, 5 s, offscreen on the idle desktop (`PSX_RENDER_THREADS`
forcing the count), redraw-only switch-offs / render-thread misses a
launch:

| Configuration | Switch-offs | Misses |
| --- | --- | --- |
| RELIT, one thread | 9, 9 | 824, 709 |
| RELIT baked lamps, one thread | 1, 6 | 115, 582 |
| ORIGINAL, one thread | 4, 2 | 454, 125 |
| RELIT, two threads | 0, 0 | 0, 0 |

So the lighting adds to the render thread's load, but even ORIGINAL misses
redraws there: one thread is not enough for this scene at 180 Hz.

## Change (framework, `render_thread.c`)

- Both render-thread instances are started when the process may run on 8
  or more CPUs (`sched_getaffinity` on Linux, else SDL's count); one below.
  `PSX_RENDER_THREADS=1|2` still overrides. The choice is logged
  (`psxrecomp: render threads: N (C CPUs ...)`).
- The second instance takes a redraw only while the first is drawing (a
  backlog), so it stays idle in light views and loads nothing there.

## Evidence (offscreen, idle desktop, RELIT + SMOOTH, private save copies)

| View | Rate | One thread: GPU / CPU / switch-offs | Now: GPU / CPU / switch-offs |
| --- | --- | --- | --- |
| UI slot 9 corridor, walking | 120 | 33-34% / 3.5 cores / 0 | 37% / 3.6 / 0 |
| UI slot 9 corridor, walking | 180 | 40% / 4.7 / 9-10 | 53% / 5.1 / 0 |
| UI slot 9 corridor, walking | 240 | 42% / 4.8 / 9-10 | 55% / 5.4 / 0 |
| UI slot 2, turning | 180 | 28% / 2.2 / 0 | 33% / 2.6 / 0 |
| UI slot 8, turning | 180 | 28% / 2.4 / 0 | 31% / 2.5 / 0 |

The higher GPU use in the corridor is the redraws now drawn instead of
dropped. With 4 CPUs (`taskset -c 0-3`) the game keeps one render thread.
Vanilla loads as before.

## Limits and next

- Needs the user's playtest in UI slots 9 and 1 on the real display.
- The render thread is still heavy: part 2 of D23O is the per-triangle
  light work (18% of it here), the driver share and libc copies, then the
  GPU share of the second instance in light views (2-5 points more).

# D23O part 2 - A lighter render thread

Status: Needs playtest (2026-11-11). Framework only; Vanilla, ORIGINAL and
Classic draw no lit triangles and take none of the light paths below.

## Profile first

Render thread 0, UI slot 9 corridor walk at 180 Hz, offscreen, both render
threads, the developer sampling profiler with call stacks
(`PSX_PROF=FILE PSX_PROF_RT=1 PSX_PROF_STACKS=FILE`, new `PSX_PROF_LEAF=1`
so the stacks keep library leaves; thread names `psx-render0` and
`psx-render1` now show in `/proc` and profiles):

| Share of render thread 0 | Where |
| --- | --- |
| 17% | per-triangle light work: light lists 8%, shadow classes 9% (classifying 4.6%) |
| 11% | NVIDIA driver (`libnvidia-eglcore`) |
| 11% | priority-inheritance mutex contention inside the driver (`__futex_lock_pi64` and unlock) |
| 9% | copies (`memmove`): the snapshot's state copied twice a redraw, the stream copied for the reuse check, the whole 1 MB CPU VRAM saved and restored when a redraw uploads, the vertex upload |
| the rest | triangle setup (host precise vertices, widescreen per primitive), stream playback |

The contention: with two render threads and the emulation thread in one GL
share group, every `glBufferData` of a batch's vertices (0.8 MB a redraw in
the corridor) was copied by the driver under a lock the other threads then
waited for. With one render thread it was half as large.

The shadow classes (D41F2) had a root-cause defect: a drawn triangle's world
corners are rebuilt from eye-relative positions with an error of 0.1-1 unit
that changes with the camera (measured: R - nearest integer, 2 million
corners), so the cache key quantized at 0.25 unit changed at almost every
redraw. Each of the three renderer instances also kept its own table and
classified within a 400 us budget per image, so most pairs waited
(1.3 million "deferred" lookups per 5 s per instance) and every instance
classified the same pairs.

## Changes (framework)

- **Light lists four at a time** (`gpu_gl_renderer.c`): the lights as
  structure-of-arrays with their squared reach, padded to four, SSE2 tests
  (scalar fallback). Same arithmetic, the same lists bit for bit.
- **Shadow classes shared and off-thread** (`shadow_mesh.c`): one lock-free
  table (2^18 entries, 64-bit words) for all three instances and one
  low-priority classifier thread (`psx-shclass`) running the D41F2 tests
  unchanged; a renderer only looks a pair up (an 8192-entry per-instance
  table first), a miss is queued once and drawn per pixel meanwhile, as a
  budget miss was. **Snapping**: each corner coordinate within a
  distance-scaled tolerance (0.5 + a/2365 units, a the largest axis distance
  from the eye, at most 6) of the 16-unit
  lattice of level geometry takes the lattice value, so the key and the
  triangle classified are exact; a triangle off the lattice (actors, props)
  is not classified. The corners' hash is made once per triangle.
- **Vertex ring** (`gpu_gl_renderer.c`): textured batches are written into a
  persistent mapped staging ring and copied by the GPU
  (`glCopyBufferSubData`) into the same place of a ring in video memory,
  which the draws read with a first-vertex offset; six fenced segments of
  32768 vertices. A first version drew straight from the mapped ring
  (system memory): game CPU fell but the GPU rose from 52% to 70% (every
  vertex fetched over the bus), hence the copy. Without ARB_buffer_storage,
  or developer `PSX_GL_VERTEX_RING=0`, `glBufferData` as before. Side
  effect: a native-wide mirror drained inside a batch flush no longer
  overwrites the batch's vertices before its own mirror draw.
- **Copies** (`render_thread.c`, `render_replay.c`, `gpu.c`): the
  snapshot's GPU and widescreen state are imported once, straight from the
  snapshot (held by its reference), outside the queue lock, and not at all
  for a reused redraw; the reuse check (D23J) compares a 128-bit hash of the
  stream instead of keeping and comparing a copy; the CPU VRAM array is
  saved and restored by row (only the rows a redraw writes); a GP0 upload
  gathers its rectangle a row at a time.
- Developer: `PSX_REPLAY_COW_CHECK=1` now also checks the CPU VRAM array;
  `PSX_RT_VALIDATE=1` prints how many pixels differ on a mismatch (and
  `PSX_RT_VALIDATE_DUMP=DIR` writes the pair); `render_thread` status shows
  textured batches per thread; `PSX_SHADOW_CLASSIFY_THREAD=0` disables the
  classifier (`PSX_SHADOW_CLASSIFY_US` is gone with the budget).

## Evidence (offscreen, idle desktop, RELIT + SMOOTH, the player's profile copy, private card copies)

Before = the same tree built with the framework of part 1 (exported patch).
Corridor walk (UI slot 9), 8 s, whole GPU clock-weighted, CPU of the game
process (emulation and render threads; the redraw workers are separate):

| Rate | GPU before / after | Game process CPU before / after | Render threads after | Switch-offs |
| --- | --- | --- | --- | --- |
| 120 | 37.0% / 35.2% | 162% / 144% | 63 + 11% | 0 / 0 |
| 180 | 52.9% / 51.0% | 223% / 194% | 69 + 50% | 2 / 0 |
| 240 | 52.8% / 54.3% | 236% / 215% | 77 + 57% | 4 / 0 |

Turning in place at 180 Hz (light views): UI slot 2 render threads 41 + 14%
-> 37 + 10%, GPU 28.4% -> 28.1%; UI slot 8 41 + 4% -> 32 + 3%, GPU 30.1% ->
28.9%. The light lists fell from 7-8% to 2-4% of the render thread, the
shadow classes from 9% (and 2 s of classifier CPU per 5 s) to 4-6% (16 ms
per 5 s): 96% of pairs are now known (was about a third), which also takes
shadow lookups off the GPU. The driver mutex fell from 11% to 6%.

Pictures: verify modes `PSX_SHADOW_CLASSIFY=2|3` (magenta where a class
disagrees with the per-pixel lookup), 3 frames each in UI slots 2, 8 and 9:
0 pixels (D41F2 measured 2 and 20 in UI slot 8 with the jittered corners).
`PSX_RT_VALIDATE` in the corridor walk: images differing between a render
thread and the emulation thread's own redraw fell from 28-42 of about 100
(per-instance classes) to 2-7; with classes off 0-2 of 190, and with
redraw reuse off as well 0 of 190; the unmodified build shows the same 4-5
from redraw reuse in a walk (D23J; pre-existing, not from this change).
`PSX_REPLAY_COW_CHECK` (surfaces and CPU array) clean.

## Limits

- The acceptance's "render thread halved" is not met: about 15-20% less in
  every view. What remains is triangle setup (host precise vertex lookups,
  widescreen per primitive, stream playback), which re-running the frame's
  stream on the render thread implies: D23N (captured redraws) and D23M
  (direct redraws) are the jobs that remove it.
- Needs the player's playtest at 180 Hz in UI slots 9 and 1 (feel, no new
  shimmer on shadow edges).
