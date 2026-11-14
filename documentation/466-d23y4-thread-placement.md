# 466 - D23Y4: thread placement on 2 cores

Part of the D23Y plan (note 463): with redraws on a 2-core, 4-thread CPU,
keep the emulation thread's physical core free of redraw workers, one worker
on the other core, A/B against the default over five runs each. Acceptance:
redraws at 60 with 60 game frames and the lowest tenth 57 or more, or a
measured reason why not. Autonomous session on the field laptop, 2026-11-13
(user: ship it as the default only if it wins clearly).

Evidence labels: MEASURED (the field laptop, this session), CODE.

## 1. Built (CODE)

`render_thread.c` (`psx_placement_active`, `psx_placement_pin_emulation`,
`psx_placement_pin_side`): on Linux, with exactly 2 physical cores and more
logical CPUs than cores, the logical CPUs of the first core in the process
affinity (sysfs `core_id`) go to the emulation thread, the rest to the
redraw side. When the workers start (`psx_mod_replay_workers_start`, on the
emulation thread) the emulation thread is pinned to its core; each worker
process (`render_worker.c`, after the fork) and each render thread pins
itself to the other core. Developer `PSX_PLACEMENT=1` turns it on (off by
default, section 3); the log says
`psxrecomp: placement: emulation on 2 CPUs of one core, redraws on 2 of the other`.
Checked on the laptop: the emulation thread on CPUs 0-1, the three workers
and the render thread on 2-3.

## 2. Measured (MEASURED)

Redraws at 60 as in note 463 (FRAME RATE DISPLAY on the 60 Hz panel; the
developer `PSX_AUTO_CPUS=8` lifts the 2-core cap that now gives DISPLAY the
original presentation, and `DNTTK_REPLAY_FLOOR=0` keeps the redraws on), the
user's 3x Relit settings, the real display, 30 s walks of UI slots 2, 8 and
9, five runs per configuration and slot, interleaved with the order rotated
each run (`recomp/analysis/d23z/abn.sh`, local).

| Configuration | Runs | Game frames/s median | Lowest tenth (pooled) | Seconds under 57 | Per-run lowest tenth, median (range) | All threads | CPU clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Default (3 workers, no affinity) | 15 | 59.9 | 55.2 | 12.4% | 56.1 (37.1-59.2) | 120% | 2.39 GHz |
| Placement, 3 workers | 15 | 59.9 | 55.3 | 13.6% | 54.4 (43.7-59.0) | 126% | 2.30 GHz |
| Placement, 1 worker | 15 | 59.9 | 56.2 | 11.7% | 54.0 (43.0-59.5) | 123% | 2.49 GHz |

Per slot, lowest tenth (default / placement / placement with 1 worker):
UI slot 2 56.9 / 52.9 / 54.7, UI slot 8 54.1 / 56.1 / 52.3, UI slot 9 54.8 /
58.2 / 59.0. The emulation thread's share was the same in all three (UI
slot 8: 79-80%); GPU busy rose slightly with placement (71-73% -> 73-83%).
The original 60 presentation on the same walks: lowest tenth 59.2-59.7, all
threads 61-68%, 3.1 GHz (note 465).

## 3. Result

No configuration wins clearly: the differences between them are smaller than
the run-to-run spread (per-run lowest tenths from 37 to 59 in each), and none
reaches the acceptance (lowest tenth 57). The measured reason: with redraws
the package runs at its 15 W limit (all threads 120-126% against 61-68% at
the original 60) and the clock falls from 3.1 GHz to 2.3-2.5 GHz, so the
emulation thread, about 80% of a core in UI slot 8 even at that clock, falls
behind whichever core the workers use. Keeping the core's sibling free does
not reduce the work or the power; less redraw work does (D23Y2's late warp,
D23N's captured redraws, D23Y5's lighter emulation thread). Placement stays a
developer option, off by default; the AUTO and DISPLAY cap of 2-core machines
to the original presentation (D23W) is unchanged.

## 4. Reproduce

```
CTRLX='{"graphics":"custom","dynamic_lights":"relit","shadow_edges":"smooth","draw_distance":"extended","frame_rate":"display","view":"first"}' \
  recomp/analysis/d23z/abn.sh 5 "1 7 8" \
  y4d "LAB_PSX_AUTO_CPUS=8 LAB_DNTTK_REPLAY_FLOOR=0 LAB_PSX_PLACEMENT=0" \
  y4p "LAB_PSX_AUTO_CPUS=8 LAB_DNTTK_REPLAY_FLOOR=0 LAB_PSX_PLACEMENT=1"
```
