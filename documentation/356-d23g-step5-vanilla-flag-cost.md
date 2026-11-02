# D23G step 5 - Vanilla cost of the fast-timing flag tests

Status: **Accepted** (2026-11-02; the user: "the game is feeling better than
ever ... The save states loaded with no issue in vanilla and modern play
too"). Executable
`20c7e77484093a6e7ddbdc8cdd268d50bcf34cb0b3aeefc49a335910a0c51f6e`. Generated
C, emitter and framework untouched; codegen hash unchanged (`0xebce51ea`),
existing savestates load.

## Question

D23F ([note 105](105-d23f-fast-timing.md)) force-includes
`recomp/src/ttk/fast_timing.h` in front of every generated game shard. Each
timing hook the emitter writes (per instruction, per load, per block, per
branch edge) tests the global flag that selects fast or accurate timing.
Vanilla always runs accurate, so the tests are pure overhead there. D23F
estimated about 3 points of the emulation thread (slot 9, 100%, 0.72 against
0.69 for a build without the include). Step 5: remove the cost if a clean
design allows.

## Measurement

Harness `recomp/analysis/d23g5/` (local only): `turbo.py` loads a D17AQ
fixture slot (private copies, private port and profile, offscreen real GPU,
dummy audio), turns the debug turbo on (no pacing, no presentation) and
samples game fields against the emulation thread's CPU time for 30 s, no
input. Fields per CPU-second is the thread's throughput, free of the pacer's
idle spin. `ab.sh` alternates executables; `TIMING` sets the private
profile's `cpu_timing`. Idle host (an `updatedb` run ended before the counted
runs).

Accurate model (what Vanilla runs), fields per CPU-second, mean of runs:

| Slot | Current (`f53b39...`) | No include (reference) | New |
| --- | --- | --- | --- |
| 9 | 40.94 | 43.64 | 43.71 |
| 1 | 51.72 | 55.63 | 54.25 |
| 10 | 99.16 | 108.06 | 103.09 |

The flag tests cost the accurate model 7-9%, more than D23F's estimate. The
new build recovers all of it in slot 9 and about two thirds in slots 1
and 10.

Fast model (Modernized gameplay):

| Slot | Current | New |
| --- | --- | --- |
| 9 | 64.63 | 65.79 |
| 1 | 84.27 | 87.62 |
| 10 | 217.58 | 226.93 |

## Cause

The emitter puts a timing hook before every instruction. `ttk_ft_step` was
`if (!g_ttk_ft_on) psx_cyc_step(...)`; the accurate step writes byte fields of
the CPU state, and byte stores may alias any object, so the compiler reloads
the flag from memory before every instruction, and again in every load hook.

## Change

`fast_timing.h` and `fast_timing.c` only:

- The flag is now `ttk_ft_on`. Every generated function opens with
  `psx_cyc_bb_defer_begin()`; the header turns that into the framework call
  plus a local `int ttk_ft_on = ttk_ft_now();`, which shadows the global in
  that function.
- The hooks take the flag as an argument (the local copy, or the global in
  the 101 alias bodies that have no such opening). The compiler keeps the
  copy in a register.
- The branch edge returns the flag for the next block: unchanged on the
  inline fast path, read again from the global after any host call (slow
  edge with lease lapse, accurate interrupt check).
- Calls leave a generated function (CPS) and re-enter it through the same
  opening, so the copy is read again after every call. The flag changes only
  in host code (player update lease, slow-edge lapse); a lease granted inside
  a block takes effect at the next edge instead of the next instruction.
  Vanilla never sets the flag, so its guest behavior cannot change.

## Checks

- Images after loading fixture slots 0, 1, 2, 7, 8 and 9 and 30 fields:
  identical hashes, current and new build.
- Vanilla (`--vanilla`, 60 Hz, slots 1 and 9, 10 s): 59.96-59.99 fields/s,
  0 audio underruns; emulation thread (includes the pacer's spin) 71.6 ->
  64.2% in slot 1 and 67.2 -> 63.4% in slot 9.
- Savestates: fixture saves load; a new save made on this build (private
  slot 11) reloads.
- Level sweep (`sweep.py`, fixture slot 7 then `level N` for levels 0-3,
  5-12 and 21-29, 180 Hz, walking and turning): every level arrived,
  59.8-60.0 fields/s, no errors.
- Native: `ttk-aim-test`, `ttk-sky-test`, `ttk-gas-test`, `ttk-loading-test`
  (with `disc/SLUS_005.83`), `ttk-input-test`, `ttk-near-test`,
  `ttk-live-env-test`, `ttk-audio-test`, `ttk-xa-test` pass. The other suites
  need fixtures and do not cover this code.
- `export_runtime_patch.py`: patch unchanged (no framework edit).

## Limits

- Slots 1 and 10 keep 2.5-4.5% of the accurate-model cost; the remaining
  tests are register tests and the block-leader / edge structure.
- Offscreen measurements on one machine, dummy audio; the user's playtest is
  the acceptance (Vanilla and Modernized, same feel).
