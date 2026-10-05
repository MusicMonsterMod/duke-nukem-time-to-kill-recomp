# D23F - Fast CPU timing for the game code

Status: **Accepted** (2026-10-05; the user: "the behaviour is amazing ... the
responsiveness is literally like PC accurate now"). Modernized default from
profile schema 26. Executable
`e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966`.
Codegen hash unchanged (`0x8bab543c`): existing savestates still load.

## Problem

At 150% CPU and 120 Hz one emulation thread could not run the faster game and
120 redraws per second (D23E, [note 104](104-d23e-busy-scene-stutter.md)).
Profile of the emulation thread, slot 9, 150%, 60 Hz, moving: the
framework's per-instruction cycle model plus the per-branch interrupt check
took about 44% of the samples (`psx_cyc_base` 13%, `psx_cyc_lds` 7%,
`psx_cyc_charge` 4%, instruction cache 4%, `psx_check_interrupts*` 5%,
...). The recompiled game code itself was 4.5%.

## Design

A purpose-built timing model for the recompiled game code only, selected at
run time. Studied from the Disruptor reference ([note 93](93-disruptor-reference-research.md),
section 9) and implemented independently for TTK; no foreign source.

- `recomp/src/ttk/fast_timing.h` is force-included (CMake `-include`) in
  front of every `generated/SLUS_005.83_full_*.c`. The generated C, the
  emitter and the hashed framework headers are untouched. The header
  redirects the timing hooks the emitter writes (`psx_slice_block`,
  `psx_icache_fetch`, `psx_cyc_step`, `psx_cyc_load_word/half/byte`,
  `psx_cyc_bb_defer_flush`, `psx_check_interrupts_at` and the debug
  block/call observers) to small inline functions that test one flag,
  `g_ttk_ft_on`. With the flag clear every hook calls the accurate model
  exactly as before.
- With the flag set:
  - a block charges once at its leader from its static instruction count
    (1.5625 cycles per instruction, carried in 1/256 cycles);
  - main-RAM loads read RAM directly and charge a flat 6 extra cycles; other
    loads (scratchpad, MMIO, lockstep, data shards) keep the accurate path;
  - no instruction-cache simulation and no per-instruction pipeline state;
  - the branch edge only publishes the pending cycles and runs the full
    interrupt check when it can matter: the next device deadline is reached
    (`psx_cycle_count + batch >= psx_next_service_cycle`, so devices see every
    event at the same block boundary as before), an interrupt is pending
    (`i_stat & i_mask`), a COP0 software interrupt is raised, or every 64th
    edge for maintenance (staged savestates, debug requests, lease expiry);
  - mult/div and GTE stalls, LWC2, stores and MMIO are unchanged;
  - the debug-server block and call observers are skipped only when
    `PSX_FORENSICS=0` (player sessions), where the framework already makes
    them return at once.
- `recomp/src/ttk/fast_timing.c` holds the flag and the slow edge. Like the
  CPU overclock, the model is leased from the Modernized player update
  (`modern_controls.cpp`): active during gameplay, lapsing 12 video fields
  after the updates stop. Boot, menus, movies and loading always run on the
  accurate model. The lease is longer than the overclock's 3 fields because
  western-town frames take up to 6 fields at 100%; with 3 fields it lapsed
  once per game frame (measured: about 300 lapses in 25 s).
- Profile setting `cpu_timing` (schema 25): `fast` (Modernized default since
  schema 26; an older saved `accurate` switches once) or `accurate`.
  `run.py --cpu-timing` saves it; run.py passes `DNTTK_CPU_TIMING` for
  Modernized and always `accurate` for Vanilla. Debug JSON:
  `ttk_input.input.cpu_timing` (enabled, active, constants, slow edges,
  maintenance checks, leases, lapses).

The render workers fork with the flag as it is; a worker never services
devices or interrupts (frozen machine), so the fast block charge is inert
there.

## Calibration

Goal: the same overclock percentage means the same emulated speed in both
models. A developer build (`cmake -DTTK_FT_CALIBRATE=ON`,
`fast_timing_calibrate.h`) runs the accurate model and records, for every
branch interval of game code, the cycles it charged minus the charges the
fast model keeps (stalls, GTE, non-RAM loads), against instructions, main-RAM
loads and instruction-cache misses. Gameplay windows (25 s, UI slots 1, 5,
9, 10, 100%):

| Fit | Per instruction | Per RAM load | Per cache miss | r2 |
| --- | --- | --- | --- | --- |
| three terms | 0.92 | 5.7 | 6.9 | 0.996 |
| two terms | 1.22 | 7.0 | - | 0.90 |

The coefficients were the same in all four scenes. Averages: 0.15 RAM loads
and 0.079 cache misses per instruction, 2.35 cycles per instruction. These
totals include the game's vsync wait loop, which almost never misses the
cache, so the real per-frame work misses more often than average. The
two-term fit made the game too fast at "100%" (slot 9 23.1 game frames/s
against 19.2). The final constants were matched on game frame rate at 100%
and 60 Hz (`analysis/d23f-20261005/tune.py`, 25 s each, accurate in
brackets):

| Instr charge | Slot 1 | Slot 9 | Slot 10 |
| --- | --- | --- | --- |
| 1.50 | - | 21.1 | 15.5 |
| **1.5625** | **24.9** (25.0) | **20.2** (19.2) | **15.1** (15.2) |
| 1.625 | 23.7 | 19.1 | 14.7 |
| 1.75 | - | 18.5 | 14.2 |

Repeated accurate runs in slot 9 measured 18.6-19.4. The flat model cannot
follow every scene's cache behavior, so expect a few percent either way.

## Results

Harness: `recomp/analysis/d23f-20261005/` (`h.py`, `motion.py` from D23E plus
per-thread CPU and game frame counts), offscreen real GPU, profile copied from
the player's (Modernized, first person, 4x, 16:9), private savestate copies,
private port, silent audio device.

Emulation-thread CPU use (share of one core, includes the pacer's idle spin)
at 100%, 60 Hz, moving:

| Slot | Accurate | Fast |
| --- | --- | --- |
| 1 | 0.78 | 0.44 |
| 5 | 0.71 | 0.39 |
| 9 | 0.72-0.79 | 0.39-0.41 |
| 10 | 0.80 | 0.41-0.44 |

150%, 120 Hz, walking and turning (`motion.py`, 30 s):

| Slot | Model | Thread | Game frames/s | Presents/s | Pace | `[TTK cpu]` events |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | accurate | 0.92 | 28.1 | 42.7 | every 2nd-3rd | 3 |
| 1 | fast | 0.73 | 29.8 | 120.1 | every refresh | 0 |
| 9 | accurate | 0.91 | 20.6 | 40.1 | every 3rd | 5 |
| 9 | fast | 0.73 | 26.2 | 120.1 | every refresh | 0 |
| 10 | accurate | 0.93 | 17.5 | 40.0 | every 3rd | 4 |
| 10 | fast | 0.82 | 21.6 | 120.2 | every refresh | 0 |

60 s at 150%, 120 Hz, fast (`h.py`, D23E routes): slot 9 strafe: game frames
3 fields 1194, 2 fields 4, 4-5 fields 5; 120 presents/s; no underruns; no
overclock pauses or sheds. Slot 10 walk: 3 fields 1193, 2 fields 7, 4 fields
4; same. D23E's best on the slot 9 route was 3 (915), 2 (346), 4-6 (49), one
pause, every 3rd refresh.

Profile, fast, slot 9, 150%, 60 Hz: 41% fewer emulation-thread samples over
the same 20 s than the baseline. Remaining large items: the frame pacer's idle
spin (vDSO clock reads, 14%), the call/return dispatch path
(`psx_dispatch_impl`, `psx_game_find_entry`, `dirty_ram_*`, `fntrace_record`,
`xprobe_event`, about 13%), GL driver 5%, game code 6%.

Other checks:

- Savestate made while fast (private slot 7): saved in 90 ms, reloaded, game
  image correct, fast model active after the load. Player savestates load
  (codegen hash unchanged).
- Native: `ttk-input-test`, `ttk-aim-test`, `ttk-near-test`,
  `ttk-controls-test` (34 groups) PASS; profile/launcher unit tests (64) PASS
  including a new schema 25 migration, CLI and environment test.
- Vanilla: always `accurate`. Its guest behavior is unchanged: same game frame
  rate. Host cost of the flag tests in accurate mode: slot 9 at 100% 0.72
  against 0.69 for a build without the include (0.74 with branch hints toward
  fast; the hints were removed).
- Vanilla route (`vanilla_regression.py`): d23f-vanilla-1/2/3 completed with
  exit 0 but diverged from the long-standing captures (enemy at spawn, health
  92/85, the fire step not registering in 2 of 3). A build without the D23F
  include diverged the same way 3 of 3 (d23f-noft-1/2/3), so this is not
  caused by D23F. The route has been host-timing sensitive before (about a
  quarter of the 60 historical runs differ at the fire step); why it now
  diverges every time is not established. The last matching run was
  d11d-d12b-vanilla (13:04, before D23E).

## Limits

- Accepted on the user's playtest; audio was measured only on the silent
  dummy device here. Follow-up work is D23G.
- The fast model applies to the statically recompiled game code only. Level
  overlay code (compiled overlay libraries, about 2%), the interpreter and the
  BIOS keep the accurate model.
- The dispatch-path observers (`fntrace_record`, `xprobe_event`, ...) still
  run in player sessions (about 2%); the job's step 1 was covered only for the
  in-block observers.
- Step 3 (cheaper in-between pictures) was not needed for acceptance and was
  not done.
- Game frames at 150% are faster with fast timing than with accurate (fewer
  pauses and no shedding), so the feel will differ from the user's earlier
  150% test.
