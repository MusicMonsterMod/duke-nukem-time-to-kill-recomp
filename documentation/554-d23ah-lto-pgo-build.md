# D23AH - link-time and profile-guided optimized builds

[D23AH](../jobs/campaign-performance.md#d23ah---profile-guided-and-link-time-optimized-build),
2026-11-23/24. Done, user-accepted 2026-11-24 ("the performance increase on d23ah is significant, lock it in and close it out"). Follows the overnight measurements in
[note 509](509-d23ag-overnight-chain.md) sections 4 and 6, which found LTO
5-16% and PGO 12-25% lighter on the emulation thread but did not check that the
optimized executables play the same game. This note: the opt-in build tools
(section 1), a real difference an LTO build made and its cause, fixed for every
build (section 2), the exactness evidence (section 3), and the field laptop's
numbers (sections 4-5).

## 1. Tools

- `recomp/tools/local/optimized_build.py` builds in its own trees, so the
  player build (`build-local`) and its objects are never touched:
  `lto` (build-lto: the player build's preset and flags, plus
  `-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`), `instrument` (build-pgo: LTO plus
  `-fprofile-generate -fprofile-update=atomic`), `train`, `use` (the same tree
  rebuilt with `-fprofile-use -fprofile-partial-training`, so functions changed
  since the training keep the normal optimization), `pgo` (all three) and
  `install` (`--from lto|pgo` copies that executable into build-local, the
  normal one kept as `.normal`; `--restore` puts it back). Run `build.py` first.
  No `-march`, no fast-math: the same instructions and floating point rules as
  the normal build. It reuses build-local's fetched SDL3 and libchdr sources
  and its Python interpreter (the asset steps need numpy).
- `recomp/tools/local/pgo_train.py`: the training route. Offscreen through
  `run.py`, private settings (the player's, read only) and a temporary card
  copy; from a savestate it enters every level with the console's `level N`
  and plays the census route (stand, turn, walk with turns, look, fire) in
  first and third person, then a short Vanilla pass; the redraw governor's
  fallback off (`DNTTK_REPLAY_FLOOR=0`) so the workers run and are trained;
  every launch ends with the debug `quit` so the game and its retiring workers
  write their counts.
- `recomp/tools/local/build_compare.py`: are two executables the same game?
  Each loads the same savestates offscreen, no input, and the runtime's exactness
  hook (`PSX_VRAM_DUMP_DIR`, now also writing the guest's main RAM) dumps RAM,
  VRAM and the presented picture at +60, +200 and +400 vblanks; every executable
  runs twice, interleaved, so a scene that does not repeat within one build is
  reported as such rather than as a difference. `--ctrl` overrides Modernized
  controls; `--allow-running` beside another lab game.
- `run.py`: developer `DNTTK_EXE` runs another executable with the same disc,
  config and settings. Both lab tools copy a foreign executable into
  build-local under its own name first (`...Recompiled.build-lto`): the runtime
  takes its overlay shard cache from the executable's directory
  (`main.cpp`, `exe_dir / "cache"`), and an executable elsewhere would run the
  game's overlays without their native shards (different code paths and cycle
  timing, so a different game; the first training run was discarded for that).
- `recomp/CMakeLists.txt`: developer `PSX_NO_LTO_GLOB` (a list of source
  patterns built with `-fno-lto` in an LTO build), used to find the cause below.

## 2. The difference an LTO build made, and the fix

MEASURED (desktop, offscreen, Vanilla, the private card copy's slot 3, no input):
the LTO executable's guest RAM differed from the normal build's from the first
frame after the savestate load (54 bytes at +1, 272 at +60), while each
executable repeated itself exactly. Ruled out one by one: the executable's name
and path (the normal binary under another name: identical), the boot frame of
the load (identical), strict aliasing (`-fno-strict-aliasing`: same
difference), generated build inputs and cache entries (identical), the shard
cache (both from build-local: same difference), relinking (a second, fresh
non-LTO tree: identical). The write trace (`wtrace_add`, `PSX_FORENSICS=1`)
put the first different store in the game's pad and memory-card driver; a
bisection with `PSX_NO_LTO_GLOB` put the cause in the title sources, then in
`gas_widescreen.cpp` and `loading_widescreen.cpp`.

CODE: each title source registers its hooks from a constructor, and two things
depend on the order those constructors run in:

- the GPU DMA arenas allocated there: `near_clip.cpp` (host and mask arenas)
  and `loading_widescreen.cpp` (its backdrop regions). Savestates carry that
  aperture with the saving process's allocation records, and a load restores it
  only up to the first record that differs (`memory.c`,
  `psx_mod_memory_snapshot_read`: the rest is filled with end-of-list headers).
- function-entry hooks registered at the same guest address run in
  registration order (`mod_runtime.cpp`, `psx_mod_function_entry`): 0x8002bc18
  (near_clip, sky_widescreen, gas_widescreen, loading_widescreen), 0x8001fba0,
  0x80026164, 0x8002b9f4, 0x8001ca4c and 0x80010000.

A normal build runs those constructors in link order (one `.init_array` entry
each, the CMakeLists source order). GCC's LTO merges them into one function
(`.init_array` 74 entries -> 19) whose order is not the link order, so the
arenas came out in another order and every savestate's aperture came back only
in part.

Fix (`recomp/src/ttk/ttk_register.h`): the 17 registering sources use
`TTK_REGISTER(name, TTK_RANK_...)`, a constructor with an explicit priority
(GCC and Clang; other compilers keep link order as before). The ranks are the
normal build's link order, so every build, compiler and optimization level
registers exactly as the normal build always has. Prioritized constructors run
before the others; registration only touches function-local registries, plain C
statics and constant-initialized pointers (checked for every registering
source, including the handlers game_menu sets in pc_input and the pointers
modern_controls sets for prop_lights), so it depends on no other file's dynamic
initialization. The framework's own constructors register activation plugins
only, kept in a map by id, so their order never mattered.

MEASURED: the normal build with the fix against the normal build before it,
Vanilla slot 3: guest RAM identical at +1, +2, +60 and +400 (the change is a
no-op for the normal build); `.init_array` shows the 17 registrations in the
old order. The fixed LTO build against the normal build: identical at +1, +2,
+60 and +400, both loads `modmem=restored`.

## 3. Exactness

MEASURED, `build_compare.py`, desktop offscreen, two interleaved runs each:

| executable | Vanilla slots 0, 3, 4 (RAM, VRAM, pictures, 3 frames) | Modernized slots 3, 4, 9, 10, 11 (VRAM) |
| --- | --- | --- |
| LTO (fixed) | identical, 9 of 9 dumps each | identical |
| LTO + PGO | identical on a fresh load of each (slot 4: see below) | identical |

LTO + PGO, slot 4 loaded after slot 3 in the same launch (the tool's order
3, 4, 0): different from the normal build on a quiet desktop, repeatably,
while slot 4 loaded first in a launch was identical (RAM at +1 to +400 and
pictures). MEASURED, the same PGO executable, slot 3 then slot 4: on a quiet
machine `a8fab06d` (RAM at +60), with twelve busy loops beside it
`f53a9347`, the normal build's value; with the forensic rings on
(`PSX_FORENSICS=1`, slower) identical to the normal build as well; the
normal build gave `f53a9347` after 430, 500 and 700 frames of slot 3 and
fresh. Both builds report `modmem=partial` for that second load (slot 3's
run had allocated past slot 4's saved layout). So the outcome there depends
on host timing, not on the executable's code: the fastest build lands on the
other side of a timing-sensitive point that the normal build also reaches
under other timing. FINDING (not changed, no job filed: the user asked for
no new jobs on 2026-11-23): Vanilla's continuation after a second savestate
load whose mod memory is only partly restored is host-timing dependent. A
first load in a launch is exact. `build_compare.py` now gives every launch a
fresh card copy as well.

Modernized RAM and pictures do not repeat within one build (the same normal
executable from the same savestate, no input, differs run to run, also with
FRAME RATE ORIGINAL and CPU TIMING ACCURATE: the host-timed systems), so they
cannot show a difference between builds. For Modernized the evidence is
structural: the only order-dependent registration is pinned (section 2), every
Modernized load reported `modmem=restored`, and the build changes no code,
instruction set or floating point rule; the look is for the player's eyes.

## 4. Field laptop: normal, LTO and LTO + PGO

MEASURED, same setup as the first round below, three interleaved rounds of
the three executables (the training of section 5; the levels by level select,
the lava pit replays in a second batch). Emulation thread % (mean, then each
round), redraw worker %, new pictures a second (median, lowest tenth):

| scene | normal | LTO | LTO + PGO |
| --- | --- | --- | --- |
| level 0 (TIME TO KILL) | 52.8 (52/54/53) | 53.1 (50/56/54) | 44.5 (44/44/46), -16% |
| level 3 (GOLD AND GUNS) | 48.6 (49/48/49) | 46.0 (44/46/47) | 42.2 (43/42/42), -13% |
| level 22 | 81.3 (85/79/80) | 72.5 (77/70/71) | 55.5 (55/56/56), -32% |
| lava pit UI 10 | 54.1 (53/56/53) | 52.4 (53/52/52) | 45.3 (45/45/46), -16% |
| lava pit UI 12 | 62.0 (65/63/58) | 53.8 (53/56/53) | 46.7 (46/47/47), -25% |

| scene | worker normal / LTO / PGO | pictures median | lowest tenth |
| --- | --- | --- | --- |
| level 22 | 92.5 / 87.5 / 75.7 | 42.1 / 45.5 / 58.5 | 23.5 / 22.5 / 27.1 |
| lava pit UI 12 | 53.2 / 44.3 / 43.3 | 59.9 / 60.0 / 59.8 | 31.0 / 39.0 / 55.6 |

Game speed 60 in every run; levels 0 and 3 and lava pit UI 10 hold 60 new
pictures in every build (lowest tenth 58-60). LTO + PGO is the clear step:
13-32% off the emulation thread, and at the two struggle points the pictures
come back (level 22's median 42 -> 58.5; the lava pit's lowest tenth 31 ->
56). Level 22's lowest tenth stays in the twenties: its redraw worker is
still 76% of a core.

First round (normal against LTO only, two rounds each, a separate batch):

MEASURED, field laptop (i7-5600U, 15 W, HD 5500), real 60 Hz screen, the
laptop player's own settings (CUSTOM, FRAME RATE 60, first person, 2x,
Polymorph, draw distance extended), executables built on the desktop (GCC 13)
and copied, the laptop's own restored afterwards (checked byte for byte). The
census route on levels 0 (TIME TO KILL), 3 (GOLD AND GUNS) and 22 by level
select, then the GOLD AND GUNS lava pit (the desktop's UI 10 and 12 states)
held turning for 25 s. Two interleaved rounds; emulation thread % (median of
the seconds):

| scene | normal | LTO | change |
| --- | --- | --- | --- |
| level 0 | 50.2, 55.0 | 56.3, 52.6 | within the spread |
| level 3 | 47.4, 47.5 | 45.7, 46.4 | -3% |
| level 22 | 80.4, 78.6 | 75.5, 74.1 | -6% |
| lava pit, UI 10 | 55.3, 55.5 | 51.5, 51.4 | -7% |
| lava pit, UI 12 | 69.6, 61.9 | 52.7, 55.4 | -17% |

Game speed 60 in every run. New pictures: level 22 median 31, 50 (normal)
against 46, 47 (LTO), lowest tenth 20-26 in both: the redraw worker (85-93%)
limits level 22, which neither build changes much. The lava pit UI 12's lowest
tenth: 29, 34 (normal) against 35, 30 (LTO).

## 5. PGO build

MEASURED, desktop (i7-5960X, GCC 13.3): `instrument` 9.6 min (3.6 GB peak),
`train` 58 min (first and third person, the 21 census levels each, 26 min a
pass, every level arrived; Vanilla on levels 0, 3, 22; 487 count files; the
player's files hashed unchanged), `use` 7.2 min (1.8 GB). LTO alone 6.5-9 min,
1.8 GB. The training run must use the shard cache (run from build-local,
section 1): the first attempt ran without it and was discarded. A profile
stays usable across small source changes (`-fprofile-partial-training`); after
large ones, `optimized_build.py pgo` rebuilds and retrains (about 75 min).

## 6. State and reproduce

- Both the desktop and the field laptop run the LTO + PGO executable (the
  user's playtest on the laptop: 2 slow seconds in 5.5 minutes of GOLD AND
  GUNS and no audio underrun, against 33 slow seconds and 44 underruns in 3
  minutes the evening before). The laptop keeps the normal one as
  `.pre-d23ah`. Install: `python3 recomp/tools/local/optimized_build.py
  install --from pgo`; since D62 `install --restore` relinks the normal
  executable instead of copying a saved `.normal`. A later normal rebuild of
  build-local puts the normal executable back. D62 (2026-11-26): dev sessions
  on the desktop play the normal build; the optimized build is made when the
  user asks (end of day, laptop and citadel copies, releases, measurements),
  see [04-build-and-run](04-build-and-run.md#dev-builds-and-optimized-builds-d62).
- Not done: citadel (the build is copied, not built, there as on the laptop;
  untested this round); Modernized RAM and pictures cannot be compared across
  builds (section 3).
- Lab (local): `recomp/analysis/d23ah/` (`ramdiff.py`, `seqdiff.py`,
  `leak.py`, `wt.py` write traces, `slots.py`, `mk22.py` the lab card's level 22
  state); on the laptop `recomp/analysis/d23ah/` (`lap-ab.sh`, `lap-lava.sh`,
  `sum.py`, the census copy, the lab card with the desktop's lava pit states).
