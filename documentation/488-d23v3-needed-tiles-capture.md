# 488 - D23V3: the snapshot capture copies only the tiles redraws read (field laptop)

Autonomous overnight session of 2026-11-16 (laptop lane: code in a desktop
worktree, built and measured on the field laptop over SSH). User: "D23V3:
list every reader of the snapshot surface (render threads' lazy loads, the
validator, rp_band_direct, the band copy), log the tiles each reads in UI 2,
8, 9 at 120 and 180, then capture only tiles a redraw can read before
writing them. Exact: readback check, PSX_RT_VALIDATE, matched shots,
Vanilla unchanged; measure with d23v2.sh/v2sum.py (target <= 0.53 ms a
frame)."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Every reader of the snapshot (CODE)

`gl_rt_capture` (emulation thread) fills one of three snapshot slots
(`GlRtSnap`): the canonical colour and depth-stencil (`hr_tex`, `hr_rb`),
the raw mirror, the wide surfaces, the CPU VRAM array. Since D23V2 the
canonical surface is copied by 32 x 32 tiles stamped since the slot was
last filled. Its readers:

| Reader | What it reads |
| --- | --- |
| a render thread's lazy load (`lz_copy`, from `lz_ensure`) | canonical and wide tiles on a session's first read or partial write of each; a tile a write covers whole is made valid without a read |
| a whole load (`gl_rt_load` when lazy loading is off, the copy check on, or the wide surface over 2048 texels) | the whole canonical surface and the wide surfaces |
| the session's restores (`rt_pend_restore`, `rr_cow_restore`) | rectangles the session wrote: only when not lazy, and only into the thread's own surfaces |
| `rp_band_direct`, the band copy, `wide_blit_center` | the thread's own surfaces, after `lz_ensure` (so through the lazy load) |
| texture pages and CLUTs | the raw mirror (copied whole every capture), never the canonical tiles |
| `PSX_RT_VALIDATE` | none: it redraws on the emulation thread from the live surfaces |
| `PSX_REPLAY_CAPTURE_CHECK` | the slot's canonical surface against a fresh whole copy |

So every redraw read of a canonical tile is a lazy load or a whole load.

## 2. What redraws read (MEASURED)

Lazy loads in UI 2, 8 and 9 at 120 and 180 (`[lazy load]`, `[lazy rows]`;
note 486 for UI 2 at 120): 38.0 tiles a session, 16 canonical and 22 wide,
all in tile row 7 (VRAM rows 224-255, below the frame's 0-239 clearing
fill); 266 more are made whole by a write. The learned set below is the
same 16 canonical tiles in all three views.

## 3. The change (CODE; developer `PSX_REPLAY_CAPTURE_NEED=1`, off by default)

- **What redraws loaded.** A render thread ORs each canonical tile it
  lazily loads into its slot's `rt_read` (a word per tile row, atomic); the
  emulation thread folds it into a sticky set of needed tiles when it next
  captures that slot (no render thread holds a slot it captures into).
- **The capture.** For its first `PSX_REPLAY_CAPTURE_LEARN` captures
  (default 120) the capture is whole-incremental as before, while it
  learns. After that it copies a stamped tile only if it is needed; the
  others are marked stale in that slot (`cap_stale`) and copied once they
  are needed. A whole copy leaves nothing stale.
- **Never drawn from a stale tile.** A lazy load of a stale tile (or a
  whole load of a slot with any stale tile) records the tile as read, marks
  the slot missed and fails that session (`gl_rt_cache_image` returns 0, so
  its image is never adopted); `psx_rt_render` then refuses the slot's
  later redraws, which are drawn on the emulation thread from the live
  surfaces (the validator's own path), until the slot's next capture, which
  copies the tile. A redraw is therefore drawn either from tiles equal to
  the live surface at capture or on the emulation thread: the picture is
  the same by construction; a miss only moves work.
- **The checks.** `PSX_REPLAY_CAPTURE_CHECK` skips stale tiles (counted
  apart: never read from that slot). Counters: `[gpu copies] ... D23V3
  skipped, needed tiles`, `[lazy load] ... D23V3 stale tiles loaded`,
  `[rt missed]`.
- **The validator.** With misses possible, a validation could wait on a
  redraw that failed while its cache entry was redrawn for another job;
  it then compared two different jobs (found on the desktop: one whole
  frame "differing" at the first miss). A new redraw for the entry a
  validation waits on now drops that validation (`[rt validate] ...
  validations dropped`).

Files: `render_thread.h` (`GlRtSnap`: `cap_stale`, `rt_read`,
`rt_missed`), `render_thread.c` (the refusal), `gpu_gl_renderer.c` (the
capture, `lz_copy`, `gl_rt_load`, `gl_rt_cache_image`, the check),
`render_worker.c` (the validator). Exported into the tracked patch.

## 4. Exactness (MEASURED)

Field laptop, 120, UI 2, 8 and 9, 40 s each, with the readback check every
10th capture and the validator: 16 of 512 canonical tiles copied a
capture (229 skipped), readback check 0 differing tiles in 133-134
captures each, 0 stale loads, 0 misses.

Desktop (the lane's build, 180, case `cad`, 40 s each): validator, UI 2, 8
and 9 together, 898 of 899 images exact with the needed-tiles capture
against 907 of 908 without it; the one bad image is UI 9's level-load
frame, identical in both (896,250 pixels, after the bake loads, notes 481,
483 and 487). UI 2 had one miss (2 stale tiles, one slot, 3 redraws drawn
on the emulation thread) and no bad image.

Vanilla (`--vanilla`, case `vshots`, every switch off, UI 8 and UI 2,
+60/+200/+400 frames): the lane's build against the committed `5226b66`
build: byte-identical, 6 of 6.

Matched shots with the switch on (added after the commit): field laptop, UI
2, 8 and 9 at 120, case `vshot` (+60, +200, +400 frames after the load):
two runs of today's capture already differ from each other in all nine
shots (the presented redraw depends on the run's timing, also at the
original 60 on the desktop), so whole-frame shots cannot judge a redraw
change in the Modernized profile; the validator (each checked redraw
against the emulation thread's drawing of the same job) and the readback
check are the proof there, and Vanilla's shots (deterministic) the proof
with the switch off.

## 5. Cost (MEASURED)

Field laptop, offscreen, turning, `d23v3.sh` (case `perf2`,
`PSX_GPU_PHASES=2`), two runs a configuration, interleaved and rotated,
`v3sum.py v3ab`. Capture GPU time a game frame:

| Rate | View | today (incremental) | needed tiles | change |
| --- | --- | --- | --- | --- |
| 120 | UI 2 | 1.05 ms | 0.46 ms | -56% |
| 120 | UI 8 | 1.06 ms | 0.46 ms | -57% |
| 120 | UI 9 | 1.07 ms | 0.47 ms | -57% |
| 180 | UI 2 | 1.09 ms | 0.48 ms | -56% |
| 180 | UI 8 | 1.06 ms | 0.46 ms | -57% |
| 180 | UI 9 | 1.08 ms | 0.47 ms | -56% |

The target (at most 0.53 ms) is met in every view at both rates. 0 misses
and 0 stale loads in all 12 runs. Whole-GPU work (busy x clock) moved -9%
to +2% (within the runs' spread: UI 2 at 120 504/468 against 546/523), CPU
clock and presents unchanged; the lowest tenths of two runs (one each way)
stepped to 60 or 90 for a few seconds, the D23Y6 governor's spread. What
remains of the capture is the raw mirror, the wide margins and the CPU
array.

Host: idle before the runs (load under 1, no game, Xvfb, update or backup).

## 6. Status

Exact and on target, off by default (`PSX_REPLAY_CAPTURE_NEED=1`). Default
on is the user's decision after a look on the real display at 120 (queued:
nothing should change in the picture; a miss costs one game frame's
redraws on the emulation thread).

## 7. Reproduce

Laptop, `recomp/analysis/d23y6/` (local): `REPS=2 RATES="120 180" PRE=v3ab
bash d23v3.sh`, then `CFGS="inc need" python3 v3sum.py v3ab 120 "1 7 8"`
(and 180). Exactness: `EXTRA="LAB_PSX_REPLAY_CAPTURE_CHECK=10
LAB_PSX_RT_VALIDATE=1"` (`v3x.sh`).
