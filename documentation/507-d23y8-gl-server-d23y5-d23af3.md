# 507 - D23Y8 GL server, D23Y5 cuts, D23AF3 cheaper forks (field laptop, citadel)

Autonomous session of 2026-11-17 (evening and night), steps 1-3 of the
performance chain after the baseline in [note 506](506-d23y-chain-baseline.md).
User: "Step 1 - D23Y8: move the emulated GPU's command processing and
presentation off the emulation thread ... Exact: identical VRAM and pictures
(readback compare); Vanilla unchanged. Behind a developer switch first, then
measure on vs off. Step 2 - D23Y5: cut the emulation thread's biggest
remaining host-side costs ... until it's under 50% of a core at 3.1 GHz at the
original 60 on the laptop, or show with numbers why it can't. Step 3 -
D23AF3: cheaper worker forks ... Measure fork time and the hitch it causes."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.
Machines, lab and method as in note 506 sections 1-2 (real display unless
stated; two or more interleaved runs a cell; the host gate before every run).
Everything new is behind developer switches, off by default.

## 1. Summary (MEASURED)

| Job | What was built (developer switch, off) | Exact | Field laptop | Citadel | Recommendation |
| --- | --- | --- | --- | --- | --- |
| D23Y8 | `PSX_GL_SERVER=1`: the GL renderer and presentation on their own thread | VRAM and frame-exact pictures identical on the desktop (Vanilla, Modernized) and in Vanilla on both real screens; Modernized VRAM identical on the field laptop; **not exact on citadel with LIGHTING on** (section 2.4) | the original 60: emulation thread +3 to +6 points unpinned, +3 to +4 with the GL thread pinned to the other core; pure AUTO -4 to +0; ULTRA 120 unchanged (GPU-bound) | the original 60: -1 to +1; pure AUTO -3; ULTRA 120 -4 | **retired** (removed, 2026-11-18) |
| D23Y8 (driver) | `PSX_GL_THREAD=1`: Mesa `glthread` / NVIDIA threaded optimizations | identical everywhere tested | no change | no change | **retired** (removed) |
| D23Y5 | `PSX_STARVATION_COARSE=1`: diagnostic stamps from the tick clock | exact by construction (diagnostics only) | no change (TSC) | -1 to -4 points (HPET) | on for HPET hosts; better: fix citadel's clocksource |
| D23Y5 | target under 50% at 3.1 GHz at the original 60 | | 53-61% | 46-60% | not reachable with exact cuts (section 3.3) |
| D23AF3 | `PSX_FORK_WIPE=1`: 29 observability rings wipe-on-fork | proven untouched by workers (0 accesses, every scene, 3 machines) | copy-on-write faults 120-190 -> 38-78 a second; fork 3.1-3.4 ms either way | faults 136-222 -> 76-106; fork unchanged | on (small, exact) |
| D23AF3 | `PSX_FORK_DEV=1`: the GL driver's device maps kept out of the fork | 0 worker deaths in every run | no consistent change | fork -5 to -10% with the wipe | desktop only (-35-40% on NVIDIA); off elsewhere |

Frame rates (game and pictures, median and lowest tenth) did not change at
any setting on either machine beyond the run-to-run spread: pure AUTO
holds 60 / 60 everywhere already, and the field laptop's ULTRA 120 is
GPU- and package-bound (note 506 section 3.3) whatever runs on the CPU.

## 2. D23Y8: the GL server

### 2.1 What could move (from note 506)

At the original 60 the GL backend's batching, the Mesa driver on the
emulation thread and the frame's flush in the present are about 10-12
points of the field laptop's 54-62%; GP0 decode (about 10 points) has to
stay (guest-visible GPU state, the widescreen classifier reads guest RAM
there). At pure AUTO and ULTRA 120 with redraws on, the live GP0 path is
mostly skipped and the movable share is the presents and the redraw tick.

### 2.2 Design (CODE)

`PSX_GL_SERVER=1` (developer, off by default; Linux) creates the
renderer's GL context on a thread of its own, `psx-glserver`, when the
renderer starts. Every function another module can call into the renderer
gets a wrapper, generated from the built objects by
`runtime/src/gl_server_gen.py` (147 renderer entry points, 39 of them the
backend's `gr_*` vtable, and 10 render-thread entry points that use the
main context):

- **synchronous** (the default): the caller's arguments stay on its stack,
  the GL thread runs the body after everything queued before it, and the
  caller waits. Nothing of the emulation thread runs while the body runs,
  so these are exact by construction (savestates, readbacks, presents, the
  redraw tick, diagnostics, renderer start and stop, render threads);
- **asynchronous** (primitives, draw state, wide-surface targets, live-list
  notes, interpolation suspend): by-value arguments copied into a queue of
  variable-size records, and the emulation thread goes on with the guest;
- **direct**: two flags and the present ring, which change only inside
  synchronous calls and so can be read without a hop.

What an asynchronous body may read was audited from the object (functions
reachable from the asynchronous entry points through the file's own
statics, and every external symbol they reference): renderer state (only
changed by queued calls, in order), the arguments, settings fixed at
start, diagnostic counters, the shadow classifier (built for off-thread
use, D23O) and two inputs that are evaluated when the call is queued
instead: the widescreen backdrop gate (`psx_ws_prim_in_backdrop`, which
reads `gpu.c`'s current GP0 command; its extra evaluations only widen a
diagnostic bound) and the CPU VRAM array the backend shares with `gpu.c`
(`gpu.c` waits for an idle GL thread before its two direct VRAM writes,
`psx_glsrv_quiesce`; the GL thread writes it only in synchronous calls).
With the dual CPU raster (netplay) or the debug backdrop heuristic every
call is synchronous. Other threads (render threads, forked workers) call
the bodies directly, as before; the render threads' renderer copies
(`render_copy/`) are compiled without any of it.

Three refinements came from measuring it:

- `gpu.c` now sets a host triangle's state in one call
  (`gl_set_host_triangle`: the same eight setters, same order), which
  halves the queue traffic (1.49 M to 0.73 M records a second on the
  desktop at the original 60); exact (the off build's VRAM and pictures are
  byte-identical to before);
- the redraw tick, called 54,000-64,000 times a second by the pacer and the
  device-service edges, answers from the caller while its last deadline is
  in the future, no other synchronous call happened and no prefetch is due
  (0.5 ms): about 1,000 hops a second remain;
- on 2-core SMT CPUs the server applies D23Y4's placement (the GL thread on
  the other physical core, the emulation thread on its own), which
  otherwise only happens once redraw workers start.

Presentation runs on the GL thread; the emulation thread waits for it at
the vblank exactly where it used to block in the swap, so pacing is the
original one. An asynchronous present (a frame in flight) is a further
step this note did not take.

### 2.3 Two defects found and fixed while measuring (MEASURED)

- Citadel at first ran at 7-20 fps with the server on: the GL thread read
  the clock twice per queued call for its own statistics, and on citadel's
  HPET a clock read is a system call (note 506 section 1): the GL thread
  spent most of its time reading the clock and the emulation thread queued
  behind it (`full` 3,300 times a second, 62% waiting). The statistics now
  read the clock only when the GL thread goes idle and wakes. After the
  fix: 60.0 / 59.1.
- The lab's per-second present-ring query called `gl_renderer_pres_get`
  once per ring entry: up to 1,500 synchronous round trips a second, each a
  drain, inflating the server's cost in the first laptop runs. Now direct.

### 2.4 Exactness (MEASURED)

Developer `PSX_VRAM_DUMP_DIR` writes all of VRAM (synced from the GL
backend) and the next presented picture at exact vblanks after a savestate
load (+60, +200, +400), so two runs compare byte for byte. Lab slot 4
(DUKE HILL town) at the original 60, no input:

| Machine | Vanilla (VRAM, pictures) | Modernized VRAM | Modernized pictures |
| --- | --- | --- | --- |
| desktop, offscreen | identical (off, off, server) | identical | identical (frame-exact shots) |
| field laptop | identical, real screen | identical (off, off, server, all switches) | differ between two off runs already (asynchronous host lighting) |
| citadel | identical, real screen | **+200 and +400 differ** with the server (off, off, two-CPU off, synchronous-neon off all identical) | as VRAM |

Citadel's difference is small colour changes in the displayed area. It is
not timing: off runs restricted to two CPUs and off runs with the neon
pass's VRAM copy made synchronous (`DNTTK_NEON_CHECK=1`) match the normal
off runs. It goes away with LIGHTING off and stays with live shadows or
model shadows off, so an input of the dynamic-light path is read at
execution time on citadel's GL thread (candidates: the bake store taken in
`gl_set_lights`, the time-budgeted lightmap uploads). Not found this
session; it blocks any default for the server.

### 2.5 Cost (MEASURED)

Emulation thread, % of a core (median of the per-second values, two runs
a cell, real screens):

| Setting, scene | laptop off | laptop server | citadel off | citadel server |
| --- | --- | --- | --- | --- |
| original 60, DUKE HILL town | 61 | 64 | 60 | 58 |
| original 60, FAMILY JEWELS start | 56 | 59 | 51 | 52 |
| original 60, UI 2 | 53 | 59 | 49 | 48 |
| original 60, UI 9 | 57 | 60 | 51 | 50 |
| pure AUTO, DUKE HILL town | 68 | 67 | 63 | 60 |
| pure AUTO, FAMILY JEWELS start | 59 | 55 | 54 | 51 |
| ULTRA 120, DUKE HILL town | 59 | 58 | 69 | 65 |
| ULTRA 120, FAMILY JEWELS start | 64 | 57 | 55 | 57 |

With D23Y4's placement applied by the server (the GL thread pinned to the
other physical core, field laptop, the original 60, one or two runs a
cell): DUKE HILL town 64 against 62, FAMILY JEWELS start 59 against 56,
UI 2 58 against 53, UI 9 61 against 56: no better. The
server's own counters there: 420,000 queued calls a second, 555
synchronous calls (300 of them texture uploads), 90 drains, 5,900
wake-ups.

At the original 60 the GL thread takes 10-15% of a core; on the field
laptop without placement it often shares the emulation thread's physical
core (both slow down). What the queue costs the emulation thread
(desktop profile): about 4.4% in the wrappers and record writes before the
triangle merge, the futex wake-ups (1,300-6,000 a second when the GL
thread idles between bursts), a drain before each direct VRAM write burst
(24-90 a second) and the synchronous calls that remain (texture
uploads, lights, shadows, lamp levels and the present: 300-600 a second,
each waiting for the GL thread to catch up). Radeonsi already runs most of
its own work on a driver thread (`gdrv0`), so citadel had little to move.

### 2.6 The driver's own thread (`PSX_GL_THREAD=1`)

Mesa's `glthread` (laptop iris, citadel radeonsi) and NVIDIA's threaded
optimizations: VRAM and pictures identical wherever tested; the
emulation thread within the run-to-run spread at every setting on both
machines (laptop pure AUTO 63/70/76/59 against 66/69/75/60; citadel
54/61/56/54 against 53/62/55/54). Not worth keeping.

### 2.7 Retired (2026-11-18)

User: "i continue with your recommendation to retire it". The GL server
and `PSX_GL_THREAD` are removed from the code: the generated wrappers and
`gl_server_gen.py`, `gpu.c`'s VRAM-write waits and the merged host-triangle
call (`gl_set_host_triangle`) are gone, the renderer's definitions are as
before. Verified: the desktop's frame-exact VRAM and pictures (Vanilla and
Modernized, lab slot 4, +60/+200/+400) are byte-identical to the
references taken before any of this work, with D23Y5's and D23AF3's
switches off and on; the framework patch shrank from about 4,300 added
lines to about 480. Kept: the developer dump `PSX_VRAM_DUMP_DIR`
(`main.cpp`), useful for any exactness check.

## 3. D23Y5

### 3.1 The diagnostic clock (`PSX_STARVATION_COARSE=1`)

The starvation ring stamps every SIO event, and its watchdog heartbeat, with
`clock_gettime`: about 2.4% of citadel's emulation thread (HPET), under
0.5% on TSC hosts. With the switch they use `CLOCK_MONOTONIC_COARSE`
(the kernel tick, read in the vDSO without hardware access); the stamps
are diagnostics and the watchdog works in hundreds of milliseconds, so
nothing else changes. Citadel, the original 60: 47/59/46/49 against
51/60/49/51 (FAMILY JEWELS, town, UI 2, UI 9); pure AUTO 52/60/54/52
against 53/62/55/54. Field laptop: 57/61/54/57 against 56/61/53/57
(no change, TSC).

### 3.2 Other candidates looked at

- The always-on function trace (`fntrace_record`, about 1%): kept; the
  framework's rule is that observability rings stay on.
- Dispatch (`psx_game_find_entry`, the code-validity check): already cached
  (D23G); the lookup itself is emitted code, and changing the emitter would
  reject existing savestates.
- Coplanar ranking of objects (1-2%): world meshes already cache their
  pairs (D17AR3); objects would need a per-model cache keyed by vertex
  hash. Title work, a candidate for later.

### 3.3 Why the target is not reachable (INFERRED from note 506 section 4)

At the original 60 and 3.1 GHz the field laptop's emulation thread is
53-61% (target under 50%). Of it: the title's host world about 25 points
(object pass, visibility walk, near clip, emit: per-frame work the
Polymorph renderer needs), device service about 31 (of which GP0 decode
about 10 must stay, SPU, timers and DMA the rest), recompiled game code,
its memory accessors and dispatch about 30, presentation about 6. The only
block of the needed size (4-11 points) is what D23Y8 moves, and a GL
thread measured here does not shed its queue's cost on these CPUs. The
target needs title-side work (fewer host-world passes per frame, which
changes what is drawn and so is not exact by construction) or a coarser
GPU thread (whole GP0 packets instead of backend calls).

## 4. D23AF3: cheaper worker forks

### 4.1 Where a fork's time goes (MEASURED)

The game process's mappings during a run (desktop, ULTRA 120): 237 MB of
private anonymous memory (the executable's BSS 1.35 GB virtual with about
100 MB resident, the heap 54 MB, several 70-100 MB arenas), 97 MB of the
NVIDIA driver's PFN maps, 466 mappings; the 252 MB shared image region and
the file mappings cost a fork nothing. The resident BSS objects are mostly
observability rings (`resident.py`, pagemap): `s_events` 32 MB (SPU event
ring), the CD, DMA, SIO and debug traces.

### 4.2 Wipe-on-fork for the observability rings (`PSX_FORK_WIPE=1`)

Each ring registers its address from a constructor (`psx_obs.h`,
`PSX_OBS_REGISTER`); before the first fork the whole pages inside each are
marked `MADV_WIPEONFORK`: a worker gets zero pages there, so the kernel
copies no page tables for them and the parent takes no copy-on-write faults
writing them. Developer `PSX_FORK_WIPE_CHECK=1` maps them `PROT_NONE` in
every worker and logs any access: workers touched exactly two of the 31
first candidates (`g_dirty_ram_insn_log`, `g_xdet`: workers run the
dirty-RAM interpreter), which were removed; the remaining 29 (749 MB
virtual) were never touched in any scene on any of the three machines
(three worker generations a run). Exact by construction for the rest.

### 4.3 The driver's device maps (`PSX_FORK_DEV=1`)

Before every fork the driver's `/dev/dri` and `/dev/nvidia*` mappings are
marked `MADV_DONTFORK` (the map is read again each time; workers never
touch the GPU). 0 worker deaths in every run.

### 4.4 Results (MEASURED)

| | desktop (indicative) | field laptop | citadel |
| --- | --- | --- | --- |
| fork, off | 3.5-3.8 ms | 3.1-3.4 ms | 3.0-3.5 ms |
| wipe | 3.2-3.3 ms | 2.9-3.3 ms | 3.0-3.2 ms |
| device maps | 2.8 ms | 2.7-4.2 ms | 2.8-3.3 ms |
| both | 2.3 ms (-35-40%) | 3.1-3.8 ms | 2.7 ms (-5-10%) |
| emulation thread's copy-on-write faults, steady (a second) | | 120-190 -> 38-78 (wipe) | 136-222 -> 64-106 (wipe) |
| restart of the workers, worst | 12-13 -> 9.8 ms | unchanged (5-9 ms) | unchanged (13-16 ms) |
| longest present interval | | within the spread | within the spread |

On the Mesa machines the device maps are small and a fork's time is the
private memory that workers do need (the heap, the title's host state) and
the mapping count: the remaining route is no-restart reuse (D23AF3's
second route, eager plugin allocations), which changes the mod-memory
layout savestates record.

## 5. Defaults waiting on the user (MEASURED)

Pure AUTO, real screens, two runs a cell, interleaved; emulation thread %
and game lowest tenth:

| | laptop off | laptop D23N5 | laptop D23Y10 | citadel off | citadel D23N5 | citadel D23Y10 |
| --- | --- | --- | --- | --- | --- | --- |
| DUKE HILL town | 71, 56.7 | 70, 58.1 | 68, 58.7 | 61, 59.6 | 62, 59.5 | 60, 59.6 |
| FAMILY JEWELS start | 63, 58.7 | 62, 58.7 | 60, 55.7 | 54, 59.3 | 53, 59.5 | 52, 59.3 |
| UI 2 | 64, 58.9 | 63, 58.8 | 64, 59.0 | 55, 59.5 | 52, 59.4 | 50, 59.4 |
| UI 9 | 75, 58.8 | 69, 58.9 | 69, 58.7 | 55, 59.2 | 55, 59.5 | 54, 59.4 |

D23N5 (`PSX_RW_RESTORE=1`): the emulation thread 0-6 points lower on the
laptop, 0-3 on citadel, no frame-rate change; nothing against it.
D23Y10 (`PSX_PUBLISH_TRACK=1`): 0-6 lower on the laptop, 1-5 on citadel,
but its write tracking costs 1,800-2,250 page faults a second on the
emulation thread (against 90-190), and the laptop's FAMILY JEWELS start had
its lowest tenth at 55.7 (one run fell back to the original presentation:
workers 38% against 58%).

D23AF4 (GRAPHICS SIMPLE against BALANCED, FRAME RATE AUTO):

| | laptop BALANCED | laptop SIMPLE | citadel BALANCED | citadel SIMPLE |
| --- | --- | --- | --- | --- |
| game lowest tenth (town, FJ, UI 2, UI 9) | 57.9, 58.5, 59.0, 59.1 | 58.8, 59.1, 59.0, 58.9 | 59.5, 59.4, 59.5, 59.4 | 59.5, 59.6, 59.6, 59.5 |
| emulation thread % | 70, 60, 61, 73 | 71, 57, 55, 67 | 62, 53, 54, 55 | 61, 54, 54, 57 |
| GPU busy % | 72-80 | 76-78 | 59-63 | 45-53 |
| package W (median) | 14.1-15.1 | 14.9-15.0 | 8.6-10.1 | 8.2-9.4 |
| worker restart, worst | 4.9-6.4 ms | 2.9-4.3 ms | 12.6-16.1 ms | 8.8-10.6 ms |

On the field laptop SIMPLE keeps a slightly better lowest tenth in the
town and at FAMILY JEWELS and a lighter emulation thread in UI 2 and 9;
both sit at the 15 W package. On citadel both hold 60; SIMPLE saves 0.4-0.7
W and 10 points of GPU. The look is the user's call.

## 6. State

The remaining switches (`PSX_STARVATION_COARSE`, `PSX_FORK_WIPE`,
`PSX_FORK_DEV`) are developer switches, off by default and never set by
the launcher; Vanilla is unchanged. D23Y8's code is removed (section 2.7). The framework changes are exported to the
tracked patch. Both timing machines run the same final source.

## 7. Reproduce

Lab `recomp/analysis/d23y8/` (local): `vx.sh` (exactness: `PSX_VRAM_DUMP_DIR`
dumps, case `vwait`), `ab1.sh`, `ab60.sh`, `fin.sh` (the batches of this
note), `y8sum.py`, `prof.py`, `smaps2.py`, `resident.py`. Switches still in
the code: `PSX_STARVATION_COARSE=1`, `PSX_FORK_WIPE=1` (+ `PSX_FORK_WIPE_CHECK=1`),
`PSX_FORK_DEV=1`, `PSX_VRAM_DUMP_DIR=dir` (+ `PSX_VRAM_DUMP_FRAMES`).
The GL server (`PSX_GL_SERVER`) and `PSX_GL_THREAD` were removed when D23Y8
was retired (section 2.7).
