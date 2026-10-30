# D23G - Renderer feed cost (part 1)

Status: **Accepted** (part 1, user 2026-10-30: "buttery smooth ... the best it's ever been by far"; closed out; D23G remains open). Framework changes only, exported to
`recomp/patches/time-to-kill-accepted-source.patch`; no generated code or
hashed header changed, so savestates still load.

## Why the renderer came first

The 2026-10-30 re-profile (D23G work log) found that at the player's 120 Hz
the GL feed, not the dispatch path, was the largest emulation-thread cost, and
that the D14F 90-degree default had tipped the busy scenes over budget (slot 9
110 presents/s, slot 10 about 107). The user then reported the UI slot 2 lake
(saved 2026-10-05) as the one place that still slows down, worst in the water.

## What slot 2 was doing

A per-quarter-second sampler (`hitch.py`, local) showed the emulation thread at
100% CPU while advancing only 1-9 fields per quarter second, with single game
frames of 40-53 fields (up to 0.9 s), whenever Duke entered the water (swim
animations 122/125/126). The GPU was never saturated (peak 80%). A profile
bounded to those frames put 46% of the thread inside the NVIDIA driver.

The `frame_perf` batch counters named the cause: almost every textured batch
break had reason 4, a change between depth-tested and painter-order
primitives. The lake interleaves depth-tested ground with translucent water
that keeps painter order, so a frame became over 1000 one- or two-triangle
batches (up to 23,000 in half a second, against about 180 in ordinary play).
Every batch paid three texture binds, about ten uniforms, a vertex upload and,
in 16:9, two framebuffer binds for the native-wide mirror pass.

## Changes

1. **Depth runs inside one batch** (`gpu_gl_renderer.c`). A textured batch is
   now a list of runs that differ only in depth testing. The batch is set up
   and uploaded once; each run draws its range with its own depth state, in
   submission order, and the wide mirror pass binds its target once and
   replays the runs in the same order. Runs that stay inside the 4:3 frame
   still skip the mirror, now per run. `PSX_GL_DEPTH_RUNS=0` restores one
   batch per run for comparison. Slot 2 water: about 10 batches per frame.
2. **Depth test in the GL state cache.** `GL_DEPTH_TEST` joins blend, scissor
   and stencil; every toggle in the file already went through
   `gs_enable`/`gs_disable`, and context creation forgets the cache.
3. **`d16_match` without stack probes.** The per-primitive check declared the
   128 KiB readback buffer of its rare validation path. GCC's default
   stack-clash protection probed all 32 pages of that frame on every call,
   before the early return (10.5% of redraw time). The validation is now an
   out-of-line function. An object scan found no other hot function with a
   probed frame except `process_cdda_stream` (about 0.4%, left alone).
4. **Display information memoised** (`gpu.c`). `gpu_get_display_info` is called
   several times per primitive by the widescreen paths; it now returns the
   last result while all twelve display registers it reads are unchanged.
5. **Compact host-vertex probes** (`gpu.c`). The 8-slot probe of the 5 MiB
   host-vertex tables reads separate key and age arrays instead of whole
   40-byte entries. Probe order, the oldest-entry choice and the starting
   state are unchanged.

## Evidence

Harness: `recomp/analysis/d23g-20261030/` (local): D23F `h.py`/`motion.py`
plus `hitch.py` and `bp.py`, offscreen real GPU, private copy of the current
player profile (FOV 90, 4x, 16:9, 150%, fast timing), dummy audio, idle host.
User slots run from private copies of UI slots 2, 8 and 10; the player's
files were only read. 150%, 120 Hz, 30 s moving:

| Scene | Before | After |
| --- | --- | --- |
| D23F slot 9 | 0.85 thread, 110.0 presents/s, 1 reduced | 0.81, 120.4, 0 |
| D23F slot 10 | 0.93, 106.7-107.1, 2 reduced | 0.92, 120.0-120.3, 0 |
| User slot 2 (lake) | 0.78, 72-82, frames up to 53 fields | 0.67, 110.6, longest 3-4 fields, 0 reduced |
| User slot 10 (jetpack) | 0.67, 120.3 | 0.66, 120.3 |

Slot 2 water route, 40 s, quarter-second windows below 12 fields: 17-19 per
run before, 0 in two runs after. Still open: the view right after loading
slot 2 needs 0.8-0.95 of the thread at 120 Hz (kernel time about 5%, so not
fork copy-on-write cost). It falls about 15% behind for two seconds and the
D23H pacer presents every second refresh for about 5 s before returning to
120. That happens on every measured run (before the motion window starts, so
the table's reduced count above excludes it).

Image check: one frame after loading private UI slots 2, 8 and 10 is
pixel-identical to the build before these changes (`PSX_GL_DEPTH_RUNS=0` on
the first build of the series). Slot 10 has two timing-dependent frames (an
in-game lighting flicker); the old path produces both (2 of 6 runs), and each
new-build image equals one of them exactly.

Native: `ttk-input-test`, `ttk-near-test` (also with `PSX_GL_DEPTH_RUNS=0`),
`ttk-sky-test`, `ttk-gas-test`, `ttk-loading-test`, `ttk-aim-test` pass.
Vanilla slot 2 runs at 59.93 fields/s with original 3-field game frames and no
underruns. A pristine pinned framework checkout plus the exported patch
reproduces all 55 modified framework files.

## Limits

- Offscreen measurements on one machine; the user's display and feel are the
  acceptance. Still images cover the live frame after a load, not a moving
  comparison of every redraw.
- D23G steps 1-3 and 5 (dispatch fast path, per-call observers, overlay and
  interpreter timing, Vanilla flag cost) are not done. Remaining redraw cost
  at slot 2 (redraw-only profile): NVIDIA driver about 24%, host-vertex stores
  11%, precise-triangle lookups 8%, textured triangle setup 6%. Feeding
  redraws off the emulation thread (step 4's larger option) is the main
  remaining lever for the post-load slot 2 view.
- Slot 10 pop-in at jetpack height is the game's room visibility, not a frame
  rate problem; it is tracked separately as D17AQ.
