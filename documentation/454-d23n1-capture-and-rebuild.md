# D23N1 - Capturing a redraw job and rebuilding it without the game's code

Status: built and verified (2026-11-12), developer check only, no
player-facing change. Job board: D23N1 (first step of D23N, captured
redraws; plan in [note 449](449-d23n-captured-redraws-research.md) sections
6-7). Evidence labels: CONFIRMED (our measurement or code), INFERRED.

## 1. Why this step comes first

At 180 Hz every presented image is a redraw job: a forked worker process
loads the game frame's memory image, patches in the job's camera and
blended poses, and runs the game's view composition and drawing tail, while
our host renderer (`near_clip.cpp` and its companions) draws every world,
prop and actor polygon from hooks inside that code. The workers use about
1.5 cores. D23N's goal is for the later jobs of a frame to skip the game's
code and be rebuilt from what the first job recorded.

That only works if a record can hold everything our host code needs. Our
code and the game's are tightly interleaved (note 449 section 6, and the
entry-point inventory in this job's work log): the game builds GTE matrices
that our hooks read, our hooks rewrite mesh pointers and matrices that the
game's code then uses, and our packets share the game's ordering table. So
before any skipping, D23N1 proves the record is complete on the one case
where the answer is known: a job rebuilt from its own record must produce
its own stream, word for word.

## 2. The check

`recomp/src/ttk/redraw_check.inc`, included by `frame_replay.cpp`
(`worker_draw`), developer switch `DNTTK_N1_CHECK=N` (every Nth gameplay
job; off by default, nothing of it runs).

Recording, in the job (CONFIRMED in code):

- Before the composition runs, the worker forks a copy of itself. The copy
  holds the job's exact starting state, host and guest, and waits.
- The job's CPU write pointers are wrapped: every store the game's own code
  makes at the top level is logged in order (address, value, size). Stores
  made inside a plugin callback, or by guest code a callback dispatches, are
  not logged; the rebuild runs those callbacks for real.
- A runtime observer of top-level hook entries (`psx_mod_function_entry`,
  all 97 hooked sites) records each entry's address, the CPU state before its
  callbacks, and the stream position before and after them.
- At the flip: the CPU state and the replay's GPU DMA channel registers.

Rebuilding, in the copy, without running the game's top-level code:

- Apply the logged stores in order, except stores to I/O registers
  (`0x1F801000`-`0x1F802FFF`): the job's own DMA kicks already reached its
  stream and are spliced in below, so running them again would repeat them.
- At each recorded hook, call it with its recorded CPU state; our host code
  and any guest code it dispatches run for real and emit our packets and
  stream items.
- Between hooks, splice in the stream items the game emitted itself (its
  small lists and uploads) from the job's stream.
- Restore the channel registers, then run the chain close (`0x8001fba0`) and
  the submit (`0x8001fa78`) for real, so every packet, ours and the game's,
  reaches the stream from the rebuilt memory through the real DMA walk.

Compared: the two streams item by item, except the PGXP precision records
(kind 4) of the game's own packets: they come from GTE work of game code
the rebuild does not run. When such a record is a stream's first vertex
record, it also switches on source annotations (kind 5) for the rest of
that stream; for that case only, the check also compares without
annotations and counts the job separately. On any difference the check
reports the first differing item, compares main RAM, the scratchpad and all
mod memory (both GPU DMA apertures and the enhancement memory) at the flip,
and finds the first differing store after the flip.

## 3. Results (CONFIRMED)

Offscreen, 180 Hz, the player's profile, private card copies; every third
job checked unless stated.

| View | Play | Checked jobs | Exact | Composition ms | Rebuild ms |
| --- | --- | --- | --- | --- | --- |
| UI slot 2 | walk, turn, fire | about 1,630 | all | 6.1-6.4 | 3.5-3.6 |
| UI slot 5 | walk, turn, fire | about 960 | all | 8.5-8.7 | 5.3-5.5 |
| UI slot 8 | walk, turn, fire | about 1,540 | all | 8.5-8.7 | 4.9 |
| UI slot 9 (level 3 corridor) | walk, turn, fire | about 1,620 | all | 7.3-7.6 | 3.9 |
| UI slot 2, third person | walk, turn, fire | about 1,340 | all | 6.7-7.2 | 4.2-4.6 |
| UI slot 2 | turning | about 1,520 | all | 7.8-8.0 | 4.8-5.0 |
| UI slot 8 | turning | about 1,700 | all | 7.9-8.0 | 4.2-4.3 |
| UI slot 9 | turning | about 1,730 | all | 7.0 | 3.9 |
| UI slot 2, third person (every 2nd job) | walk | about 390 | all but 1 a worker | - | - |

About 12,000 jobs checked, every one word for word identical. The single
exception is in third person: the first job after the save loads (63,466
stores and 3,632 hooks against about 13,000 and 500 normally). There a game
packet's PGXP record is the first vertex record of the whole stream, the
documented exclusion above, and the job is identical once annotations are
set aside. A job records 12,000-17,000 top-level stores and 490-750 hook
entries.

With the check off: `PSX_RT_VALIDATE` exact on 693 of 693 redraws in two
runs (a third run, also with the check off, had one redraw with 1,073 pixels
one level apart in the sky band: the render-thread comparison's own
rounding, unrelated to the recorded stream); 180 presents a second held; Vanilla boots and draws (shots
taken). Worker CPU in the turning view read 161-162% against 148-151% in
earlier runs today; run-to-run spread today has been wider than that (total
CPU 252-286% in identical runs) and the always-on additions are a few
instructions a hook, so this is treated as noise; D23N4 measures on matched
builds.

### What the timing says (INFERRED)

The rebuild runs our hook work alone (with the guest code our hooks
dispatch themselves, such as the per-object draw calls of the host
visibility pass), so it is an upper bound on a rebuilt later job before
D23N2 replaces per-job work with re-projection: 55-62% of the composition.
Skipping the game's top-level code therefore saves about 40% of the
composition by itself; the rest of the saving has to come from D23N2
(the hooks' own guest calls and our per-camera work).

## 4. Defects found and fixed on the way (CONFIRMED)

The check is strict enough to expose real nondeterminism and bookkeeping
errors; each was fixed in the system, not worked around:

1. **Hook depth in workers.** Workers can be forked from inside a plugin
   callback, so each worker started with the new depth counter at 1 and
   every store looked nested. `render_worker.c` now starts each worker at
   the top level.
2. **Wall clock in drawing.** Prop glows (sway and flicker) and the
   security monitor read SDL's clock while drawing, so two redraws of the
   same frame differed even with the same camera, and a glow could move
   while one image was drawn. A draw clock (`ttk_draw_clock_ms`,
   `live_env.h`) is held at each redraw job's start; live it is SDL's clock.
3. **The lamp bake appearing mid-job.** A level's first bake is published
   while the game runs; a job read the active table per face, so one image
   could be part unbaked and part baked. The table is now held for the
   whole job (`lamp_bake_pin`, `lamp_bake_map.h`).
4. **Splicing and source annotations.** A spliced host vertex record now
   counts as recorded, so later list nodes carry their source address as
   they would after recording it (`render_replay_record_splice`).
5. **I/O stores in the rebuild.** Replaying the game's DMA kicks repeated
   its small lists (the draw environment of the wrong display buffer
   appeared next); the rebuild now leaves I/O registers alone and restores
   the channel registers at the flip.

## 5. Framework changes (tracked in the accepted patch)

- `mod_runtime.cpp`, `mod_plugins.h`: `g_psx_mod_hook_depth` (plugin
  callbacks in progress) and `g_psx_mod_hook_observer` (developer observer
  of top-level entries, null by default).
- `render_replay.c`, `render_replay.h`: `render_replay_record_words`,
  `render_replay_record_splice`, `render_replay_channel_get/_set`; the
  render-thread copies' rename headers regenerated (`gen_names.py`).
- `render_worker.c`: a worker's jobs start at hook depth 0.

`python3 recomp/tools/local/export_runtime_patch.py` exported them; the patch
applies cleanly to the pinned upstream commit `08ec704a`.

## 6. Using it

```
DNTTK_N1_CHECK=3 python3 recomp/tools/local/run.py
```

The lab passes it with `LAB_DNTTK_N1_CHECK=3` (`recomp/analysis/d23k/lab.py`,
local). `DNTTK_N1_DUMP=path` also writes the first differing jobs' streams
and hook table. A checked job costs a fork, a 2 MB RAM copy and a 1.2 MB
mod-memory snapshot, so checked jobs can miss their present; use it offscreen.

## 7. What it means for D23N2-D23N4

- The record is complete: our host code is a pure function of the hook
  entries' CPU state and the game's top-level stores, plus the draw clock
  and the lamp table, both now held per job. A rebuilt job can be trusted
  to be what the job would have drawn, given the same inputs.
- D23N2 changes the inputs: a later job of the same frame has another camera
  and other blended poses. The logged stores and hook CPU states that depend
  on them must be recomputed instead of replayed. Diffing the records of two
  jobs of one frame now lists exactly which ones (the tool is in place).
- The I/O and PGXP findings carry over: the game's own lists are spliced
  from the first job, and its packets' PGXP records will have to come from
  that job too (D23N3).
