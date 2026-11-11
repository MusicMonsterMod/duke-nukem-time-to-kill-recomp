# D23M - Redraws restored from their snapshot (first step of direct redraws)

Status: Needs playtest (2026-11-11). Job board: D23M. Framework only; the
emulation thread's own redraws (validation, single-thread path), Vanilla,
ORIGINAL and Classic are unchanged.

## What a redraw paid for drawing into emulated VRAM

`PSX_GPU_PHASES=2`, turning at 180 Hz, offscreen on the idle desktop, after
D23O part 2, per redraw (GPU timeline; "other" in these tables is the time
between marks, mostly the GPU waiting for the render thread, not work):

| Kind | UI slot 2 | UI slot 8 |
| --- | --- | --- |
| textured draws | 0.38-0.42 ms | 0.66-0.94 ms |
| copy-on-write save (9 copies, incl. the full raw mirror) | 0.065-0.094 ms | 0.066-0.068 ms |
| copy-on-write restore | 0.076-0.080 ms | 0.075-0.077 ms |
| uploads (11) | 0.046-0.11 ms | 0.044-0.05 ms |
| depth clear, band, mirror, fills, wide clears | 0.09-0.13 ms | 0.08 ms |
| end of redraw (output pass) | 0.11 ms | 0.11 ms |

A render thread loads a snapshot's surfaces (the emulation thread's hr
colour, depth-stencil, raw mirror and native-wide surfaces) into its own,
draws a redraw into them, and for the next redraw of the same snapshot has
to undo what it drew: it saved each region before its first write (into a
backup surface) and copied it back at the end, and saved and restored the
whole raw mirror.

## Change (framework, `gpu_gl_renderer.c`, `render_thread.c`)

- **Restore from the snapshot.** The snapshot a render thread loaded stays
  unchanged while the thread references it, so what a session overwrites
  can be copied back from the snapshot's own textures: nothing is saved
  first. The render thread now holds the snapshot reference until the
  session ends (it dropped it after the load). `gl_rt_load` records the
  snapshot's texture names per surface; a region with no snapshot texture
  (a surface created during the session) keeps the old save-and-restore.
- **Deferred.** When everything a session overwrote can come back from the
  snapshot, the restore waits for the next load on that thread: the same
  snapshot again (copy the regions back then), or a new one (the full load
  overwrites them: nothing to copy). About a third of the sessions are
  followed by a new snapshot on the same thread.
- **CPU VRAM**: the 1 MB array is copied from the snapshot only when the
  surfaces are loaded; every session puts back each row it writes (D23O
  part 2), so later sessions of the same snapshot find it as loaded.
- Developer: `PSX_REPLAY_SNAP_RESTORE=0` saves and restores as before;
  `PSX_REPLAY_COW_CHECK=1` restores at the end of each session (so its
  check stays exact); `render_thread` status shows `publish_busy`, publishes
  skipped because every snapshot was held (0 in the corridor walk).

## Evidence (offscreen, the player's profile copy, private card copies)

Per redraw after the change (same views): copy-on-write save 0, restore in
the session 0.001 ms (the deferred restores run in the next load), uploads
flushed in 4 groups instead of 11, 0.032-0.035 ms (inferred: each save
copy flushed the pending uploads first and split them). Summed GPU kinds per redraw without the output pass and the
gaps, UI slot 2: about 0.74 -> 0.58 ms (with the deferred restores).

Redraw phases on the GPU timeline (`PSX_GPU_PHASES=2`, mean of the steady
samples, turning at 180 Hz), before / after:

| View | setup | scene | end of redraw | sum |
| --- | --- | --- | --- | --- |
| UI slot 2 | 0.013 / 0.003 ms | 0.878 / 0.786 ms | 0.117 / 0.038 ms | 1.008 / 0.827 ms |
| UI slot 8 | 0.013 / 0.003 ms | 0.865 / 0.765 ms | 0.112 / 0.038 ms | 0.990 / 0.806 ms |

About 18-19% less a redraw (plus the deferred restores, about 0.025 ms on
average, now in the next load). The "end of redraw" that D23K took for the
output pass (0.11 ms) was mostly the copy-on-write restore: it is now
0.04 ms, so D23L's "output pass halved" is met here.

Whole GPU, clock-weighted, turning, 12 s, before (D23O part 2 + D41M
build) / after; every run held its rate:

| View | 120 Hz | 180 Hz | 240 Hz |
| --- | --- | --- | --- |
| UI slot 2 | 21.8 / 20.2% | 29.2 / 27.9% | 34.4 / 33.2% |
| UI slot 8 | 16.8 / 16.9% | 22.1 / 22.6% | 30.1 / 28.8% |

(Single runs; the GPU clock moved between 1258 and 1987 MHz in some, so
differences under a point are noise. The per-redraw timers above are the
cleaner measure.)

Pictures: `PSX_RT_VALIDATE` exact against the single-thread path, UI slot 8
turning 326 of 326, corridor walk 131 of 131 (classes and reuse off);
`PSX_REPLAY_COW_CHECK` (surfaces and CPU array) clean.

## Limits and next

- The acceptance's "a third less GPU per redraw" is reached only in part:
  the save copies are gone, the restore is mostly free, but the output pass
  (0.11 ms, D23L) and the uploads remain. Still open from the research:
  drawing a redraw into a target of its own (no snapshot surfaces at all),
  which needs to know which regions a redraw reads back (texture pages over
  drawn pixels, VRAM copies and fills); and skipping uploads whose bytes the
  snapshot already holds (measured 4 a redraw now).
- Needs the player's playtest at 180 Hz (UI slots 9 and 1: no new flicker,
  stale patches or missing redraws).
