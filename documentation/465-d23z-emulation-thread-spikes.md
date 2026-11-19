# 465 - D23Z: one-off CPU spikes on the emulation thread

User, 2026-11-13 (autonomous session on the field laptop): get as much
performance as possible out of the 2-core laptop at the user's settings
without changing pictures or behaviour; profile each one-off event, move the
work off the emulation thread or spread it, decide with measurements whether
the overclock pause should wait for a second slow second, and reach no
"behind" second in 30 s walks of UI slots 2, 8 and 9.

Evidence labels: MEASURED (the field laptop, this session), CODE, INFERRED.

## 1. Setup

Field laptop (note 461): i7-5600U, 2 cores and 4 threads, HD Graphics 5500,
mains, performance profile, RAPL PL1 15 W for CPU and GPU together. The
user's settings: 3x internal scale, LIGHTING RELIT, the PRETTY grade's live
shadows (`shadow_edges` smooth), DRAW DISTANCE EXTENDED, FRAME RATE AUTO (the original 60 on 2
cores), first person; the real display. Private settings and card copies
only. Lab `recomp/analysis/d23z/` (local): the D23W lab with per-second game
logic frames, `runs.sh`, `ab.sh` and `abn.sh` (interleaved A/B with the order
rotated per run), `comb.py` (medians, lowest tenth, slow seconds counted from
the game's own `[TTK hitch]` seconds over the whole log; the load's own second
is never a hitch line). The CPU clock stayed at 3.09-3.11 GHz in all these
runs (median of the per-second average): the original 60 presentation at
60-70% of a core stays under the power limit.

New developer tools:

- `DNTTK_SPIKE_TRACE=MS` (`modern_controls.cpp`): every update of Duke whose
  emulation-thread CPU time or wall time since the previous one exceeds MS,
  with the field counter and the plugin hooks that took most of it (through
  the existing hook observer).
- `PSX_PROF_FRAMES=1` (`host_profiler.c`): each `PSX_PROF_STACKS` sample is
  tagged with the field counter (`#stack@FIELD`), so a time window can be
  profiled; `stackdiff.py` in the lab compares two windows.

## 2. What the events cost (MEASURED)

Baseline, 3 runs per slot. UI slots 2 and 9 had one slow second (under 57
game frames) in every run, UI slot 8 in two of three.

| Event | Measured on the emulation thread | Verdict |
| --- | --- | --- |
| Neon-sign snapshot (D50A), 45 ticks after a level start or savestate load | 13.3-15.2 ms in one tick: `gpu_vram_copy` waited for the GPU to finish the frame in flight (`ensure_cpu`, a synchronous `glReadPixels` at 3x) | Fixed: 3.7-5.1 ms |
| Frame pacer, every field | 7.7% of all emulation-thread samples were the clock read (vDSO) in `frame_pacer_wait`'s last-2 ms spin, calling the present tick in a loop | Fixed: sleeps to 0.3 ms before the deadline |
| Sleeping-enemy handovers | No traced update above 75% of its 2-field budget at any of the 4 handovers seen (UI slots 2 and 9) | Not a spike here |
| Relit room changes | None within a traced slow update | Not a spike here |
| Heavy views just after a load (the walk's turn in UI slots 2 and 9) | 14.5-16.6 ms of CPU a 16.7 ms field for about 1.5 s, against 9.5-10 ms later in the same walk | Not a one-off event: D23Y5 |

The heavy window, profiled by field (`PSX_PROF_FRAMES`, fields 300-390
against 600-1000): host world emission through host visibility
(`host_vis_walk` -> `near_clip_host_world_piece`) +1.7 ms a field, the
emulated GPU's command processing (`dma_gpu_ll_advance` -> `gpu_write_gp0` ->
the GL backend's triangle setup) +2.7 ms, in UI slot 9 the sleeping-enemy
copies (`sleeping_draw`, the game's own actor renderer) +1.4 ms. All grow
with the geometry in view; the same walk pattern 8 s later, facing about the
same way but nearer the walls, costs nothing extra. The overclock is already
paused there: the load's own slow second pauses it for 5 s.

## 3. Built (CODE)

- **Neon snapshot without the GPU wait.** `gpu_vram_copy_async` (framework
  `gpu.c`, `gpu_render.c`, the GL backend's `glb_vram_snapshot`): the same
  queued work as the synchronous readback (draws, CPU uploads, the pack into
  the raw mirror), then a read of the raw mirror into a persistent mapped
  pixel-pack buffer behind a fence, polled on later ticks. The content is VRAM
  as of the begin call; the CPU mirror and its dirty state are untouched, so
  the emulation sees no difference. `level_lights.inc` copies RAM at the same
  tick and starts the scan when the VRAM arrives (one poll later in every run
  measured). Developer `PSX_GL_ASYNC_SNAPSHOT=0` restores the synchronous
  path; `DNTTK_NEON_CHECK` keeps it beside the old pass.
- **Pacer without the spin (Linux).** `frame_pacing.c`: a wait under 2 ms
  sleeps (`nanosleep`) to 0.3 ms before the target and spins only the rest.
  `nanosleep` overshoots by 0.1 ms here (p99 0.17, max 0.2 ms), so deadlines
  are met as before. Windows keeps its waitable-timer path. Developer
  `PSX_PACER_SPIN=1` restores the spin.
- **The overclock pause stays at the first slow second.** Built as an option
  (`overclock_pause_after`, developer `DNTTK_OC_PAUSE_AFTER=N`), measured,
  default 1 (section 4).

## 4. Measured before and after

30 s walks (walk, strafe, turn and fire), the real display; medians and the
lowest tenth of the lab's per-second counts; slow seconds are the game's own.

| UI slot | Runs | Game frames/s median (lowest tenth) | Slow seconds per run | Worst second | Emulation thread | All threads | GPU busy |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2 before | 3 | 60.0 (59.7) | 1, 1, 1 | 55.7 | 60% | 65% | 57% |
| 2 after | 6 | 60.0 (59.5) | 2, 1, 1, 1, 0, 0 | 55.4 | 59% | 64% | 58% |
| 8 before | 3 | 60.0 (59.4) | 0, 1, 1 | 56.1 | 71% | 73% | 56% |
| 8 after | 6 | 60.0 (59.7) | 0, 1, 0, 0, 0, 0 | 55.7 | 66% | 68% | 58% |
| 9 before | 3 | 60.0 (59.3) | 1, 1, 1 | 52.8 | 57% | 62% | 51% |
| 9 after | 6 | 60.0 (59.2) | 1, 1, 1, 1, 1, 1 | 52.4 | 56% | 61% | 55% |

The pacer alone, interleaved against `PSX_PACER_SPIN=1` (3 runs each): the
emulation thread 71% -> 66% (UI slot 8), 58% -> 56% (9), 60% -> 59% (2); all
threads -1 to -5 points; UI slot 8's slow seconds 3 in 3 runs with the spin,
1 in 6 without.

Overclock pause, interleaved, 3 runs each: after 2 slow seconds in a row
instead of 1, UI slot 8 had a slow second in every run (worst 52.7), UI
slot 2 four in three runs (worst 54.0), UI slot 9's worst second fell to
39.0 (52.4-53.0 with the pause at the first). The pause relieves the
emulation thread in a heavy view; waiting a second lets the slow spell
deepen. The default stays at the first slow second on every machine.

Pictures (still view after 240 settle frames, UI slots 2 and 9, `present_shot`):
the old paths (`PSX_GL_ASYNC_SNAPSHOT=0 PSX_PACER_SPIN=1`) against the new
defaults: 0 differing pixels; the pacer alone: 66 pixels, all in the LEVEL
STATS clock (it ticks with wall time). The neon face sets are the same
(level 3: 4 faces, level 6: none). One early reference run differed
everywhere and so did a second run of the same old paths against it: a
different game moment, not a rendering change.

## 5. Result

- The one-off spikes D23W saw are gone or small: the neon snapshot from
  13-15 ms to 4-5 ms; handovers and Relit room changes were not spikes here.
- The target, no slow second in 30 s walks, is met in UI slot 8 in five of
  six runs and not in UI slots 2 and 9: their slow second is the heaviest
  view of the walk shortly after the load (host geometry and GPU command
  processing on the emulation thread), not an event. Written into D23Y5's
  work log with the profile.
- Vanilla unchanged: none of these paths change what it runs (the neon pass
  and the overclock are Modernized; the pacer only waits differently).

## 6. Reproduce

`recomp/analysis/d23z/runs.sh TAG REPS "" 1 7 8`, `ab.sh`, `comb.py`;
`DNTTK_SPIKE_TRACE=25` (two fields at 60 are about 33 ms), and
`PSX_PROF=... PSX_PROF_STACKS=... PSX_PROF_FRAMES=1` with
`stackdiff.py STACKS A0 A1 B0 B1`.

## 7. Re-measured on main after D23AO (2026-11-19)

The field laptop's checkout was brought to main (e95c5fa: the 18 November
build, D23AK, D23AO's fixed frame rate, D23AP, D11E1, D14G1) and rebuilt; the
framework patch, and so this job's code, is the same as on 13 November. The
player's settings have changed since then: BALANCED at 2x, FRAME RATE 60,
first person (the lab copies them; `graphics` kept at `balanced`). Same 30 s
walks on the real display, 3 runs a slot, host idle (load 0.00, mains,
schedutil). MEASURED:

| UI slot | Slow seconds per run (walk window) | Worst second | Busiest thread | All threads | Pictures a second |
| --- | --- | --- | --- | --- | --- |
| 2 | 0, 1, 0 | 53.3 | 53-59% | 110-125% | 60 (lowest tenth 59-60) |
| 8 | 2, 2, 1 | 45.3 | 75-79% | 177-185% | 60 (lowest tenth 59-60) |
| 9 | 0, 2, 0 | 40.8 | 53-59% | 120-134% | 60 (lowest tenth 59-60) |

No "emulation behind" line in any walk; the frame rate never changed. Per
thread (UI slot 8, sampled by name each second): the emulation thread is the
one that saturates, 87-90% for about 15 s from the walk's ninth second, Duke
near -282 5630 21198; the redraw worker (`psx-render0`) 25-50%.

`DNTTK_SPIKE_TRACE=25`, one run a slot (at these settings an update of two
fields has a 33 ms budget, so 25 ms traces ordinary updates; the counts below
are after the load, field 600 on):

- UI slot 2: 70 traced, largest 38.0 ms.
- UI slot 8: 631 traced (most updates), median 30.8 ms, 4 over 45 ms
  (46-56 ms); the largest hook in each is 1.8-6.4 ms. The cost is the whole
  update, outside any one hook.
- UI slot 9: 81 traced, 1 over 45 ms (47.5 ms); its slow seconds are the
  first three of the walk, the heavy view just after the load (section 2).

Verdict: no one-off event spike is left on the emulation thread. The slow
seconds that remain are sustained heavy views (UI slot 8's stretch near
-282 5630 21198, UI slot 9's view after the load), the kind of struggle point
D23AQ owns; written into D23AQ's work log. The acceptance as written (no slow
second in these walks) is not met at the current settings, and is not a
one-off-event problem any more.
