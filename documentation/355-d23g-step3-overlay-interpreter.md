# D23G step 3 - Overlay and interpreter timing: measured, no change

Status: **closed, no code change** (2026-11-02; the user accepted the
recommendation to close step 3 as measured). Executable
`f53b391249e45ce816b56ef1cf0cb76d7e93cccfcad2001b43faa61da53afd6e` (D23G part 2
baseline) measured as is.

## Question

D23G step 3: give level overlay code (compiled overlay libraries) and, if
measurable, the dirty-RAM interpreter the D23F fast timing model. D23F's
limits ([note 105](105-d23f-fast-timing.md)) estimated overlay libraries at
about 2% and the interpreter at about 5% of the emulation thread (slot 9,
150%, 60 Hz, before D23G parts 1-2).

## Harness

`recomp/analysis/d23g3/` (local only): `probe.py` (fixture slot, moving,
`dirty_ram_stats` / `overlay_loader_status` rates over 6 s), `lsweep.py`
(every selectable level by `level N`, 3 s of walking and turning, rates plus
`phase_profile`), and part 2's `prof.py` with perf (slot 9, 180 Hz, moving).
D17AQ fixture copies, private port and profile, offscreen real GPU, idle host
(load about 1, no backups running). Player saves and cards untouched (hashes
checked by the harness).

## Findings

1. **Level overlays are not compiled libraries.** Every `LEVELxx.OVR` loads
   at `0x800ca968`; the overlay cache (`build-local/cache`) holds only two
   libraries for the current codegen hash: MOVIE.OVR (`000CA968_223F1B49`)
   and the initialized geometry math routine (`000B3D9C_5B2AAC32`). In play
   `overlay_loader_status` shows the level's CRC (level 6: `0x07406F51`)
   with no matching library, so level overlay code runs in the dirty-RAM
   interpreter. The "compiled overlay library" in gameplay is the math
   routine.
2. **The interpreter runs little code.** Per level, moving (instructions per
   second interpreted, including the BIOS interrupt handler in low RAM, which
   is most of it in slot 9):

   | Levels | Interpreted insns/s | Phase interp share | Math library calls/s | Phase native share |
   | --- | --- | --- | --- | --- |
   | 0-12 (11 levels) | 37,500-94,100 | 1.1-3.1% | 6,600-23,600 | 0.6-3.7% |
   | 21-29 (9 levels) | 31,500-139,900 | 0.9-3.1% | 6,100-25,300 | 0.6-2.5% |

   Highest: level 23 (140k) and level 25 (122k). The guest executes roughly
   50 million instructions per second at 150%, so interpreted code is under
   0.3% of guest instructions everywhere. Slot 9 breakdown: BIOS handler
   `0x2818..0x369c` about 65% of interpreted instructions, level overlay
   (`0xcbbbc`, `0xcc188`, ...) about 10%, patched main-executable pages
   `0xb0xxx`/`0xb4xxx` the rest.
3. **Host cost (slot 9, 180 Hz, moving; thread 59%, 60 fields/s, 180
   presents/s).** Math library DSO 0.56% of thread samples, of which its
   accurate timing helpers (`psx_icache_fetch`, `psx_slice_block`,
   `psx_advance_cycles`) about 0.11%. `dirty_ram_dispatch` 1.38% (interpreter
   plus overlay entry overhead, spread over the function with no hot spot).
   `dirty_ram_text_native_ok_ranges_from` 1.67%, but perf annotate puts 46%
   of it on the cache-entry load of part 2's cache hits (about 600,000 per
   second); the failing check made by each math call is not visible.
4. **Guest timing effect.** Fast timing would change the guest cycles charged
   for the math routine (about 0.5M instructions/s) by under 1% of guest CPU
   and would shift D23F's calibration, which was matched with this routine
   on the accurate model.

## Conclusion

Step 3 has no measurable emulation-thread saving to win: the timing model
inside the math library is about 0.1 of a point and interpreted code is a few
tenths of a percent of guest work. Compiling level overlays (21 libraries,
new build step, savestate and guard review) would remove at most the
level-overlay share of `dirty_ram_dispatch`, well under one point. Not
implemented. Recommended: close step 3 as measured; the remaining D23G work
is step 5 (Vanilla flag cost).

Other measured items seen on the way (not step 3, not changed): `getenv`
0.63% of thread samples on a hot path (caller not attributed),
`psx_rt_poll` about 6%, `gl_renderer_rp_tick` 5.6%, `psx_write_word_raw`
3.4%, the pacer's idle clock reads about 14% (idle, not work).
