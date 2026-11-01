# D23G - Dispatch fast path and per-call observers (part 2)

Status: **Accepted** (part 2, user 2026-11-01: "the stability now is literally
next level. lock it in, close us out!"; closed out; D23G remains open).
Regression baseline executable
`f53b391249e45ce816b56ef1cf0cb76d7e93cccfcad2001b43faa61da53afd6e`. Steps 1 and 2 of D23G. Framework
changes exported to `recomp/patches/time-to-kill-accepted-source.patch`.
Steps 3 (overlay and interpreter timing) and 5 (Vanilla flag cost) remain.

## What a game call cost

Every call or return between compiled game functions goes through the
dispatcher that the recompiler emits into the BIOS image (`psx_dispatch_impl`).
For an ordinary, unmodified game function it did all of this:

1. per-call observers (`fntrace_record`: dispatch tail ring, mod and
   text-translation hooks);
2. the BIOS HLE hook and the vector-stub check;
3. a binary search of the 3989-entry BIOS table, which can never match game
   text (the table's highest main-RAM key is 0x4810; the shell window is
   handled separately);
4. `dirty_ram_dispatch`, which recorded a boundary probe (`xprobe_event`), then
   asked "is this function's code still the original?" (`psx_game_text_native_ok`,
   a walk over every page of every code range the function was compiled from),
   then called `psx_dispatch_game_compiled`, which asked the same question
   again before running the function.

Hooked functions (92 entries with game-mod hooks) also scanned the whole hook
list on every call.

Measured on the current player build (D17AW), slot 10 vista at 180 Hz,
600,000 dispatches per second: about 190 ns of host time per dispatch, 12-13%
of the emulation thread.

## Changes

1. **Code-validity cache** (`memory.c`). A "still the original" answer that
   came from clean pages only is remembered per function range list and
   resume address. It is forgotten when anything it depends on changes: a page
   turning dirty or a RAM replacement (`g_dirty_ram_code_gen`, already bumped on
   those), a guard-modified page, the reference image registered or blessed, a
   restore rebuild, or the overlay floor. Answers that needed a byte compare are
   never cached. `PSX_TEXT_OK_CACHE=0` turns it off; `PSX_TEXT_OK_CACHE=verify`
   re-runs the full check on every hit and counts disagreements
   (`g_text_ok_cache_disagree`, `g_text_ok_cache_stale`).
2. **One check per call** (`dirty_ram_interp.c`). `psx_dispatch_game_compiled`
   already refuses a function that is not native-safe, so it is called directly;
   the separate pre-check stays only while the per-function load/store window
   (`ls_func_set_window`) is armed, since that window brackets only bodies that run.
3. **Boundary probe only when wanted** (`dirty_ram_interp.c`, `debug_server.c`).
   The per-call `xprobe_event` record is skipped in player sessions
   (`PSX_FORENSICS=0`) unless a trip (`PSX_XPROBE_FRAME_TRIP`, `PSX_XPROBE_STK_KB`,
   `xprobe_arm`) or a watch is armed. Interpreter call sites always record.
4. **Mod hook index** (`mod_runtime.cpp`). Function-entry hooks are matched
   against a compact address list kept in registration order.
5. **No BIOS-table search for game text** (`full_function_emitter.cpp`, generated
   BIOS dispatcher). Game text above every main-RAM key of the static table and
   outside the ROM-keyed shell window goes straight to `dirty_ram_dispatch` and,
   if that refuses, to `psx_unknown_dispatch` - the same calls in the same order
   as after the old search's guaranteed miss. The bound is computed from the
   table when the BIOS is generated.

## Savestates

Change 5 is emitter output, so the codegen hash moved from `0x8bab543c` to
`0xebce51ea`. The user agreed to lose savestates if needed (2026-11-01), but
they are kept: the regenerated game code (all 70 generated files) and every BIOS
function body are byte-identical to the previous build; only
`psx_dispatch_impl` in `OpenBIOS_dispatch.c` differs. `boot_state.c` therefore
accepts `0x8bab543c` (and the two hashes it already accepted) under
`0xebce51ea`, the same reviewed exception as the earlier dispatch-lookup change.
All six D17AQ fixture saves (made under the old hash) load, and a save written
by the new build reloads. Any later emitter change needs the same review.

`build.py` now passes `--force-bios` to `generate`, so an existing checkout
regenerates the BIOS C (about 6 s) instead of linking the old dispatcher. The
movie overlay is rebuilt for the new hash by the normal build.

## Evidence

Harness `recomp/analysis/d23g2/` (local): `percall.sh` (dispatch-family perf
samples divided by dispatches counted in the same 6 s window, slot 10 vista at
180 Hz), `prof.py`, `sweep.py` (21 levels by `level n`), `shots.py`,
`saveload.py`, `vanilla.py`. Private copies of the D17AQ fixture card and
savestates, offscreen real GPU, dummy audio, player files hash-checked before and
after every run.

| Build | ns of dispatch path per dispatch | Share of thread |
| --- | --- | --- |
| Before (3 runs) | 188.5-195.8 | 11.6-13.0% |
| Runtime changes 1-4 | 171.2-171.7 | 12.2-13.7% |
| All changes (2 runs) | 146.5-148.1 | 9.9-10.8% |

About 23% off the dispatch path, 2.5-3 points of the emulation thread.
`xprobe_event` 47-62 samples -> 1-2, the duplicate check 39-56 -> 5-12,
`psx_dispatch_impl` itself 250 -> 81-116. The vista's own frame cost varies by
14-18 ms per game frame between runs, more than this saving, so game frames per
second are not a usable comparison there.

- Verify mode over all 21 levels (load, walk and turn at each): 80.4 million
  cache hits re-checked, 0 disagreements, 0 stale.
- Final build, same 21 levels without verify: every level arrives and plays at
  59.9-60.1 fields/s; no unknown-dispatch or fatal lines in the log.
- Images 30 fields after loading fixture slots 0, 1, 2, 7, 8 and 9 at 60 Hz are
  identical to the previous build (and between runs).
- Vanilla (`--vanilla`, slot 1, 10 s, two runs each): 59.89-59.99 fields/s on
  both builds, 0 audio underruns, thread CPU 71-73% (was 72-77%).
- Native suites: ttk-input, near, sky, gas, loading, aim, controls pass.
- A pristine pinned framework checkout plus the exported patch reproduces all
  changed framework files.

## Limits

- Offscreen measurements on one machine; the user's playtest is the acceptance.
- Still per call: `fntrace_record` and its hooks, the BIOS HLE hook,
  `fntrace_is_game_started`, two hashed lookups (`psx_game_find_entry` and the
  validity cache) and interrupt polling. Removing more means changing generated
  game code (`psx_dispatch_game_compiled`, the emitted stale-code guards).
- Steps 3 and 5 of D23G are not done.
