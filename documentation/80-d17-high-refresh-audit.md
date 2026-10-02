# D17: high refresh rate without faster simulation

Status: **Needs playtest** (2026-10-02). Sections 1-8 are the audit and plan
written before any timing change; section 9 holds the measurements that changed
the plan; sections 10-12 describe what shipped, the evidence and the limits.

Job: [D17](../MODERNIZATION_JOBS.md). Brief: [68](68-d17-high-refresh-brief.md).
This is the required first step: the audit and plan come before any change
to timing. Static reading of the owned SLUS-00583 executable plus the pinned
runtime source. Started 2026-10-01.

## 1. What sets simulation frequency

Two clocks, nested:

- **Guest VBlank clock (runtime).** `psx_vblank_clock` raises VBlank every
  564,480 emulated cycles (59.94 Hz NTSC). Timers, CD, SPU, DMA and the BIOS
  `VSync()` count are all driven by emulated cycles, not by host frames.
- **TTK's own frame (guest code).** The per-frame driver is `0x8002666c`:
  game update `0x80025ac8`, then the view composition `0x80026164` once per
  player view (camera render `0x8002e48c`, status bar `0x8008ba30`,
  overlays), then the flip `0x8001fcbc`. The flip waits on `DrawSync`, reads
  `VSync(-1)`, and waits until at least **2 fields** have passed since the
  previous flip (`s2 = 2`; 6 in one special mode). It then stores
  `[0x800d21fc] = 5 * fields` (fields clamped to 8) and swaps buffers
  (`0x8001fa78`).

So **TTK is a variable-timestep game**. The update delta `[0x800d21fc]` is 10 at
the normal 30 fps (2 fields) and 15 at 20 fps (3 fields). Everything that moves
scales by that delta: animation ticks, root motion, velocities and timers.
D08Y measured this (and the CPU overclock that keeps busy views at 2 fields).
D12A found a counter-example: the kick tests for hits once per update, so the
number of updates matters as well as the elapsed time.

Consequences:

- The current "60 FPS" is **30 unique game images per second, each shown
  twice**, and 20 per second in views that overrun. The 60 Hz figure is the
  VBlank and present rate, not the image rate.
- The simulation is already independent of the host present rate: the guest
  is paced by emulated cycles against the wall clock. Presenting more often does
  not run game logic more often unless the guest clock itself is changed.

**Rejected: running TTK's logic at 60 Hz** (patching the 2-field minimum to 1).
The engine supports it, and the D08Y 200% overclock did it by accident. But
it changes gameplay. Per-update effects (kick contacts, hit tests, AI decisions,
animation events) would happen twice as often, and integration with delta 5
instead of 10 changes arcs and collisions. That breaks the core requirement.
It also caps out at 60.

## 2. What sets rendering frequency

- Guest: one image per TTK frame (30 or 20 Hz), rasterised by the runtime
  GPU (`gpu.c`, OpenGL backend `gpu_gl_renderer.c`) into the high-resolution
  VRAM render target as the game's display list is submitted.
- Host: the frontend presents at every guest VBlank (59.94 Hz). The cadence is
  owned by the wall-clock pacer (`frame_pacing.c`, deadline-based) or by driver
  vsync when the panel matches 59.94 within 2% (`present_vsync_owns_cadence`).
  On the user's 180 Hz panel the pacer owns cadence, vsync is off (swap
  interval 0), and each 30 Hz image is swapped twice, unsynchronised with the
  panel's 5.56 ms refresh. That can tear and gives uneven 3/3 or 2/4
  refresh-per-image beats.
- Existing runtime option, not used by TTK: `psx_mod_set_frame_interpolation`
  (GL "temporal blending"). It crossfades the last two completed **images**
  at the host refresh rate. It has no motion vectors, so moving objects
  double-image (ghosting). It also works at the 59.94 Hz VBlank rate, so
  most blends are between two copies of the same image. **Not a solution**,
  though its presentation scheduler (`frame_interpolation.c`, deadlines
  anchored to the guest interval) is reusable.

## 3. Systems that stay frame-dependent

Because the simulation clock is never touched in this plan, the brief's list
(movement, jumping, jetpack, AI, weapons, projectiles, doors, lifts, timers,
scripts, audio, menus, transitions) **keeps its exact original timing at
every render rate by construction**. What depends on the render rate is only
what the player sees and when:

| Item | Today | With D17 |
| --- | --- | --- |
| Camera position and orientation | Steps at 30/20 Hz | Interpolated, plus live mouse look (section 5) |
| Duke, actors, projectiles, pickups | Step at 30/20 Hz | Interpolated where their state can be matched frame to frame |
| Skeletal animation pose | Steps per update | Interpolated animation time where safe, otherwise steps |
| Doors, lifts, moving sectors | Step | Interpolated transforms where present in data, otherwise step |
| Particles, sprites, flashes, HUD | Step | Step (left at the latest state; never extrapolated) |
| Host overlays (crosshair, Duke font, inventory) | Drawn every present | Unchanged |
| FMV, menus, loading, 2D screens | Step | Step (interpolation off) |

Rule: anything not explicitly interpolated is drawn exactly as the game
produced it in the latest frame. That is never wrong, only not smoother.

## 4. Display refresh detection today

`main.cpp` reads `SDL_GetCurrentDisplayMode` once, at window creation, as an
integer Hz. It is only used to decide whether vsync may own the 59.94 Hz
cadence. Window moves between monitors and mode changes are not tracked.
There is no frame-rate option. Swap interval is 1 (vsync) only on a panel within
2% of 59.94, otherwise 0.

## 5. Interpolation: where and how

A PlayStation game projects its own vertices on the CPU/GTE, so the GPU only
receives finished 2D screen primitives. Primitives have no identity between
frames, so there is nothing safe to interpolate at the GPU level.
Image crossfading ghosts (section 2). The workable approach is the one PC
ports and the N64 recomps use: **draw extra frames from interpolated game
state**.

**Render replay.** At each extra present between game frame N-1 and N:

1. Snapshot the guest machine: main RAM (2 MB), scratchpad, CPU registers,
   GTE registers, the GPU's drawing state, and the cycle/timer/IRQ
   bookkeeping.
2. Write interpolated values into the snapshot's live state: camera and
   viewer transforms, Duke and actor positions and rotations, animation
   time, mover transforms. These are blended between the values recorded
   at the end of frame N-1 and frame N, with alpha taken from the present
   deadline.
3. Call TTK's own view composition (`0x80026164` path: camera render, status
   bar, overlays) and submit its display list to a **host-only render
   target**. Guest VRAM and the real displayed image are not touched, and
   no device (CD, SPU, timers, IRQ, DMA completion) advances.
4. Restore the snapshot byte for byte, then present the host target.

Steps 1 and 4 make the replay gameplay-neutral by construction. Guest state
afterwards is identical whether 0 or 7 replays ran. That is directly testable
(section 7).

**Latency.** Blending N-1 to N shows frame N up to one game frame (33 ms)
later than now. To keep high refresh rates from feeling worse, camera
orientation is not interpolated from the old frames. It comes from the latest
mouse input at present time, using the same yaw/pitch mapping the D06
independent camera uses. Mouse look therefore updates at the display rate with
less delay than today. Positions use interpolation, not extrapolation:
extrapolation is wrong on every stop, collision and teleport.

**Discontinuities.** Interpolation is skipped (alpha forced to 1) when the
camera room or mode changes, a camera cut or script camera starts, a
teleport is detected (position delta above a threshold), an actor slot is
reused, a level loads, a savestate is loaded, a menu, pause or FMV is
active, or the game drops to a different field count. Each rule is logged
in a diagnostics counter.

## 6. Frame-rate option and pacing

Profile setting `controls.frame_rate` (Modernized; Vanilla always `60`, the
current behaviour), launcher `run.py --frame-rate VALUE`, env
`DNTTK_FRAME_RATE`:

- `display` (Match Display, default for Modernized once accepted): the
  refresh rate of the display that currently holds the window, re-read on
  SDL display-change and window-move events, using SDL3's float refresh rate.
- `30`: present only unique game images (lowest cost, the compatibility
  and reference mode).
- `60`: today's behaviour, unchanged code path (the baseline).
- `120`, `144`, `165`, `180`, `240`: fixed targets.
- `unlimited`: no cap. Present as fast as replay plus GPU allow.

Pacing: presents are scheduled on wall-clock deadlines inside the guest
interval (the existing `frame_interpolation_schedule`), with sleep-then-spin
waits. When the target equals the panel rate, vsync (swap interval 1) owns
the present cadence for tear-free, even delivery. Otherwise the pacer owns it
with vsync off. The guest VBlank clock keeps its own pacer exactly as now.
The game never waits for presents, and presents never advance the guest. If
a replay overruns its slot, that present is skipped (coalesced), not delayed.

## 7. Verification plan

- **Gameplay equivalence (numeric, strongest):** deterministic input replay
  from the same savestate at 30/60/120/180/240/unlimited. Compare RAM digests
  of the guest every game frame. They must be byte-identical. Identical guest
  state means identical movement, jumps, jetpack, firing, projectiles,
  enemies, doors, timers and scripts. Also record Duke position per frame
  for the known-distance route as a readable cross-check.
- **Cadence:** per-present host timestamps (new debug ring): mean, p50,
  p99 and max interval against the target (5.56 ms at 180 and so on). Count
  unique game images, replayed images and skipped presents separately,
  and report the unique-image rate honestly.
- **Visual:** captures at alpha 0, 0.5 and 1 for camera turn, walk,
  enemies and doors. Replay at alpha 1 must match the real frame pixel for
  pixel. Check that discontinuities do not smear.
- **60 baseline:** with `60` selected, the replay path is not entered.
  Vanilla route, Python and native suites as before.

## 8. Staged path

1. **Replay feasibility (highest risk first).** Snapshot, call the view
   composition, redirect the GPU, restore. Prove that the guest digest is
   unchanged and the replayed image is identical to the real frame at
   alpha 1. Measure host cost per replay at 4x.
2. **Present scheduling and the option.** Frame-rate setting, Match Display
   detection and tracking, deadline scheduling of replays, vsync policy,
   cadence ring and diagnostics. With alpha fixed at 1 the output is
   visually today's game at a higher present rate. That alone fixes the
   tearing and uneven beats on a 180 Hz panel.
3. **Camera.** Interpolated camera position plus live mouse orientation
   (Modernized independent camera, first and third person).
4. **Duke and actors.** Positions, yaw and animation time. Then
   projectiles and pickups.
5. **Movers and polish.** Doors, lifts and sectors where the data has a
   transform. Cut detection coverage.
6. **Equivalence and cadence evidence** at all rates, then the playtest.

Stages 1 and 2 decide whether the approach works. If stage 1 fails (the
render path has device side effects that cannot be isolated), fall back to
stage 2 with alpha 1. That gives even pacing at the panel rate but no new
images, and is reported as such.

## 9. Measured findings (2026-10-01, stages 1 and 2)

Harness: `recomp/analysis/d17-high-refresh/` (`d17.py`). It runs on the
user's GPU, offscreen (SDL offscreen plus NVIDIA EGL), with private profile,
cards and savestate copies on port 9317. State is slot 4 (boxes room),
Modernized 16:9 at 4x with 150% CPU, measured on the i7-5960X.

**Replay works.** The runtime part is `render_replay.c` with the GL save,
restore and capture code; the plugin part is `src/ttk/frame_replay.cpp`.
It snapshots at the composition entry `0x80026164`, called from
`0x800268ec` with return address `0x800268f4`. A replay runs the composition and the
frame tail up to the flip, then `0x8001fba0` and `0x8001fa78`; GPU DMA runs
synchronously. It produces the frame: about 8.3K GP0 words, 8 DMA lists
and the same draw area as the real frame. Guest timing is unaffected
(60 fields/s, no audio underruns). With `DNTTK_REPLAY_TEST=prev`,
replaying the frame on screen gives an image identical to the real one
except on Duke's animated body (one-frame pairing still to be settled).
`replay_dump` writes both images.

**Replay is too expensive for the emulation thread on this CPU.** One
replay is about 670K emulated cycles (the composition is about half of a
game frame's work). It takes 11.5 ms of host time: about 9 ms of guest
code, 2.5 ms of GP0 processing (0.5 ms `gpu.c` decode and 1.9 ms GL
backend), and 1-2 ms of VRAM save and restore. The emulator runs TTK at
about 2x PlayStation speed: real game work uses about 25 ms of every
33 ms game frame (`PSX_RUNTIME_PERF_DIAG`: 760-880 ms/s of guest work).
Internal scale makes no difference (1x 869, 4x 899), so this is CPU-bound
emulation. Calling replays from the mid-frame tick starved the guest
(livelock, `rmid` run); replays need a budget governor.

**Presentation had to leave the VBlank.** Presents were issued only at guest
VBlanks, on the emulation thread. With the guest busy about 85% of each
field, display-rate presents bunched into the idle tail (about 45/s at a
180 target). Presents now run on their own deadlines
(`gl_renderer_rp_tick`). They are served from the guest's device-service
edges (`psx_set_service_tick`, at least every 16K emulated cycles) and from
the frame pacer's wait (`frame_pacer_set_tick`). The VBlank only captures
the current game image. Measured cadence at 180: mean 5.555 ms, p50 5.58,
p95 6.35, p99 7.09, max under 10 ms; a sixth of presents carry a new game
image (30 unique per second). At 30: 33.4 ms; every new image is shown
once.

**TTK's VSync wait is now idle-skippable.** Runtime idle-skip was off for
TTK, and its detector could not see TTK's loop. PsyQ `VSync` waits in
`0x800aefe4`, which spans two blocks and decrements a timeout word on the
stack. The detector now anchors on one PC across up to 8 blocks and accepts
one word store that drops by exactly 1 per iteration. A skip applies the
k decrements to that word and stops before the timeout. Result: skips at
`0x800AF00C`, about 12M cycles/s elided, and about 40 ms/s of host time
freed. With it on (launcher: any Modernized frame rate other than 60),
presenting at 180 costs about the same host time as today's 60 (856 vs
880 ms/s).

**Consequence for interpolation.** Drawing extra images by replaying the
guest renderer needs about 10-12 ms per image, and the emulation thread
has about 5-8 ms spare per game frame in heavy scenes. Real interpolation
on this machine therefore needs the guest part of each replay on other
cores: a forked worker process or a worker thread with thread-local
runtime state. The main thread would still spend about 3 ms per extra image
(GL backend plus save and restore), which caps heavy scenes at about 1-2
extra images per game frame (60-90 unique fps). Two other paths are
cheaper but approximate. The main one is image-space reprojection of
camera rotation from live mouse input at display rate: exact in first
person, approximate in third person (orbit parallax), and positions still
step at 30. The direction was a decision for the user, who chose **parallel
redraw** (2026-10-01).

## 10. What shipped

### Player view

- New Modernized option **frame rate** (profile schema 21, `run.py --frame-rate
  display|30|60|120|144|165|180|240|unlimited`, settings menu choice F,
  `DNTTK_FRAME_RATE`). Default `60` = the original presentation, unchanged code
  path. Vanilla always runs 60.
- `display` follows the refresh rate of the monitor that holds the window and
  re-reads it when the window moves to another display or the mode changes.
- `30` shows each new game image once.
- Above 60, images are presented on their own evenly spaced deadlines, and
  **in-between images** are redrawn by TTK's own renderer with the camera,
  Duke and every object drawn through the object loop interpolated between two
  game frames. `DNTTK_FRAME_INTERP=off` (developer) keeps even pacing without
  redraws.
- Game logic, physics, AI, animation timing, audio and the guest VBlank clock
  are untouched at every setting.

### Presentation (runtime patch `time-to-kill-zzzzzzzzzz-render-replay.patch`)

- `gpu_gl_renderer.c`: replay presentation. Each guest VBlank only captures the
  displayed image; `gl_renderer_rp_tick()` presents when a display deadline is
  due. It is called from the guest's device-service edges
  (`psx_set_service_tick`, at least every 16K emulated cycles) and from the
  frame pacer's wait (`frame_pacer_set_tick`), so presents are evenly spaced
  whatever the emulation thread is doing. A present-time ring gives
  mean/p50/p95/p99/max intervals and the share of presents showing a new game
  image or a redraw (debug command `render_replay`). Swap interval 0 by default
  (`PSX_REPLAY_VSYNC=1` opts into vsync); vsync behaviour still needs the real
  monitor.
- `render_replay.c`: a frozen-machine session. MMIO policy (GP0 and GPU DMA run
  synchronously or are recorded; every other device access is a no-op), no
  interrupt, savestate or idle-skip edge, guest time and GPU registers restored.
  `gl_replay_begin/end` save and restore all VRAM surfaces (high-resolution
  colour and mask stencil, raw texture mirror, native-wide surfaces, CPU VRAM,
  coherence bookkeeping) and capture the redrawn frame into an image cache.
- `render_worker.c`: worker processes (Linux/POSIX). Forked copies of the game
  run redraws on other cores. The live process publishes guest memory (RAM,
  scratchpad, used mod memory) plus CPU state and the widescreen margin into
  shared memory. A worker loads it, runs the plugin's draw in record mode and
  returns the GP0 stream; the live process feeds that stream into the GPU inside
  a session. Workers re-fork on code-overlay loads and plugin mod-memory
  allocations, are killed and replaced if they die or overrun 250 ms, and die
  with the game (`PR_SET_PDEATHSIG`). Elsewhere the API reports unavailable and
  only pacing is used.
- `psx_cycles.c` idle skip (opt-in, not used by the launcher): multi-block loops
  anchor on one PC; `PSX_IDLE_MEM=1` adds the decrementing stack-timeout rule
  for PsyQ `VSync` (see section 11 for why it is opt-in).

### Interpolation (`recomp/src/ttk/frame_replay.cpp`)

- At the composition entry `0x80026164` (return `0x800268f4`) the frame is
  published. At composition end (`0x8001fba0`, return `0x80026904`) the previous
  frame is queued for redraw toward this one at the interpolation fractions
  (default one redraw per game frame, alpha 1/2).
- The image of a frame published at display-change count c is on screen from
  change c+2. If the previous flip (buffer index `0x800bdbd0`, `DISPENV`
  `0x800d1cfc + index*0x74`) has not yet reached the presenter, it is counted.
  Redraws therefore blend from the image on screen toward the next one: **no
  latency is added** compared with the original pipeline.
- Camera `0x800d6eb0`: rotation `+0` (slerp), view matrix `+0x20` and
  translation `+0x34`, eye `+0x14`, anchor `+0x64`, distance `+0x42`
  (lerp). Room `+0x90`: when the two frames' rooms differ, the worker runs the
  game's own portal walk for the in-between eye (section 13). A 2048-unit
  anchor jump keeps the base camera.
- Duke `0x800d7198`: world joint matrices at `+0x3c` (19 x PsyQ MATRIX) are
  blended in RAM (quaternion slerp of the rotation with column scale, lerp of
  translation; a 1536-unit jump or a turn over 120 degrees is a cut).
- Every other object drawn by the object loop (draw table `0x800c0bf8` by kind
  `+0x14`: `0x800632b0`, `0x80031d10`, `0x80032e78`, `0x800348d8`,
  `0x80031c14`) loads its transforms through `0x800292a0(camera, matrix)`. Live
  frames record (object, call index, matrix); the worker substitutes the
  interpolated matrix at the same call. This covers animated actors whose
  matrices are built at draw time (kind 2), props, doors and pickups alike.
- Draw-time plugin state the hooks read (first-person blend, orbit, eye
  offset, live and recorded weapon poses, recoil, mod-memory pointers, the
  camera lease as the live draw evaluated it) ships with each job
  (`render_state_save/load` in `modern_controls.cpp`).
- Main-thread governor: redraws are drawn into a six-entry image cache ahead
  of their present, from a token bucket per guest field and only with 0.5 ms of
  pacer slack, never starting one that would run past the next present. The
  number of in-betweens follows the rate (section 13; `DNTTK_INTERP_STEPS`,
  `DNTTK_REPLAY_BUDGET_MS`, `DNTTK_REPLAY_SLACK_MS` override).
- Hooks added (regenerated): `0x80026164`, `0x800632b0`, `0x80031d10`,
  `0x80032e78`, `0x80031c14`, `0x8001fba0`.

## 11. Evidence (2026-10-02)

Harness `recomp/analysis/d17-high-refresh/` on the user's GPU (offscreen EGL),
private profile, cards and savestate copies: `d17.py` (launch, stats, dumps),
`trip.sh` (real image k, the redraw shown during k, real image k+1),
`equiv.py` (per-frame guest fingerprints), `cadence.sh`. Binary
`67127d8073983fc6c4a1cc9bee2d520d7aae40e0e3ebcd2e568d4742c098849e`.

**Redraw fidelity** (mean absolute pixel difference at 2728x960; real
consecutive frames differ by 50-120):

| Scene | Redraw at alpha 0 vs image k | alpha 1 vs image k+1 |
| --- | --- | --- |
| Street, third person, 16:9 | 0.00-0.03 | 0.01-0.05 |
| Street, first person | 0.02-0.29 | 0.00-0.14 |
| Slot 7 (animated kind-2 objects) | 0.01-0.03 | 0.01-0.98 (one triple 21: a cut) |
| Boxes room, underwater | 0.01-0.05 | 0.65-1.43 (bubbles) |

At alpha 1/2 the redraw lies between the two real images (street: distance to
each 42-60 against 55-68 between them); captures show the police car and Duke's
run pose in between (`shots/finthird1-stack.png`).

**Gameplay equivalence.** The runtime's per-frame fingerprint ring now also
hashes RAM and scratchpad contents (`PSX_FP_RAM_HASH=1`). Runs from the same
savestate are aligned by emulated cycle count and compared over 630 frames
(about 10 s). The emulator is deterministic at 100% CPU (60 against 60: all
630 identical); the 150% overclock lease is wall-clock based and is not.

| Comparison (100% CPU) | Identical frames |
| --- | --- |
| No input, slot 7: 180 with workers and interpolation | 630 / 630 |
| No input, street: unlimited | 630 / 630 |
| Driven run and turn, street: 30, 120, 144, 165, 240 | 630 / 630 each |
| Driven, street: 180 (three runs) | 630, 42, 42 |
| Driven, street: unlimited | 42 (from frame 43) |

Bisection found and fixed three real leaks on the way: a plugin allocation in
Expansion 1 mod memory (guest-visible: the game reads that region), a
`lease_ready()` call whose `state()` has side effects, and the stack-timeout
idle skip (interrupt landing at another instruction; now opt-in). The remaining
driven-input differences start at one pad sample (controller buffer
`0x800d1612`: `0xFF` against `0xEF`) and come and go between identical
settings. The synthetic driver (`DNTTK_TEST_DRIVE`) holds host-side capture and
camera state, which makes it a weak oracle here; this is recorded as open, not
explained.

**Cadence** (150% CPU, 4x, 16:9, street, running and turning; 8 s):

| Rate | Presents/s | p50 | p95 | p99 | max ms | New image | Redraw shown | Guest |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 30 | 30.1 | 33.34 | 34.90 | 36.46 | 40.1 | 100% | 0 | 59.93 Hz |
| 120 | 120.6 | 8.34 | 9.44 | 10.06 | 10.9 | 25% | 32% | 59.91 |
| 144 | 144.9 | 6.95 | 8.09 | 8.78 | 9.5 | 21% | 31% | 59.93 |
| 165 | 165.6 | 6.08 | 7.51 | 8.60 | 13.5 | 18% | 28% | 59.93 |
| 180 | 181.0 | 5.56 | 6.73 | 7.39 | 8.2 | 17% | 34% | 59.77 |
| 240 | 241.4 | 4.17 | 5.27 | 5.92 | 6.6 | 13% | 29% | 59.90 |
| unlimited | 919.5 | 1.00 | 2.67 | 4.43 | 11.7 | 4% | 16% | 59.78 |

No audio underruns at any rate. Game images stay 30 per second (the game's own
rate); 62-72% of game frames got an in-between redraw under the budget, so
about 50 distinct images per second reach the screen on this CPU. A redraw costs
about 10 ms on a worker core and 2.6-4 ms on the emulation thread (GP0 decode
0.5 ms, GL backend about 1.9 ms, VRAM save and restore about 0.4 ms).

**Regression.** Python suite 97 OK (2 skipped); `ttk-input-test`,
`ttk-controls-test`, `ttk-aim-test` pass; `ttk-near-test` fails to link on
`frame_trace_*` (pre-existing, unrelated). Vanilla route `d17-vanilla-1` exit 0,
captures normal. Patch-stack test OK.

## 12. Limits and next steps

- Not yet seen on the real 180 Hz monitor: perceived smoothness, tearing with
  swap interval 0 in exclusive fullscreen, and vsync (`PSX_REPLAY_VSYNC=1`).
  `display` could not be measured offscreen (no refresh rate there).
- Interpolation needs Linux (worker processes). Elsewhere, even pacing only.
- Particles, sprites, bubbles, HUD and anything not drawn through
  `0x800292a0` step at 30. Above about 120 Hz the heaviest scenes get fewer
  in-betweens than the rate asks for (section 13).
- Room geometry that moves (lifts, doors drawn as room meshes) has not been
  checked; transforms outside the object loop are not interpolated.
- Driven-input equivalence at 180/unlimited: two runs at the same rate can
  differ from each other (the synthetic driver), and a rerun matches 60
  (section 13).
- Third-person camera lerp can still pass a wall corner between two game
  frames; only the room choice is corrected.
- Default stays `60` until the playtest; `display` is the intended default.

## 13. Playtest review fixes (2026-10-02, second pass)

The first playtest found heavy outdoor areas (the strip-club street) choppy with
occasional audio artifacts while the sewer was smooth, outdoor textures less
stable while moving, easier views through doors and geometry, FMVs flickering
and dim, and the first-person quick-kick leg left visible and flickering after
the kick. Profiling first: a CPU-time sampling profiler for the emulation
thread (`PSX_PROF=FILE`, `PSX_PROF_CALLERS=1` attributes GL-driver and libc
time to the calling runtime function, `PSX_PROF_REPLAY=1` samples only inside
redraw feeds; `host_profiler.c`, symbolized by `symprof.py`).

**What limited heavy scenes.** The emulation thread was saturated (guest work
about 900-970 ms of every second in the heavy slots, against 500-700 in the
sewer), so the governor refused most redraws (street at 180: 1 in 3 presents a
redraw; the ladder room 0-12%). The causes, largest first:

| Cost (street, 180 Hz) | Share of the thread | Fix |
| --- | --- | --- |
| Forensic display ring: full 1024x512 VRAM read back from the GPU every frame | 9.2% | Off in player sessions |
| Per-store and per-block forensic observers (write traces, recorder, fingerprints, block observer, call log) | about 8% | Off in player sessions |
| GL driver revalidation: every batch flush bound the hr FBO, program and VAO and unbound all three after | half of 1.6 ms per frame, and the same per redraw | GL binding cache |
| Host clock read at every device event (SPU sample events are 44.1 kHz) for the presentation tick | 2-5% | Tick every 4096 guest cycles (about 0.12 ms) |
| `spu_get_global_state` (full struct copy) per SPU sample deadline query | 3.7% | `spu_ctrl_reg()` reads SPUCNT only |

- `PSX_FORENSICS=0`: the launcher sets it for every player session (not with
  `--diagnostics`, and an explicit value wins). It keeps the runtime's
  always-on observers quiet for the whole run and turns the display ring off;
  the debug server, one-shot screenshots and explicitly armed traces still
  work. The D17 harness keeps forensics on for fingerprint runs.
- GL binding cache (`gpu_gl_renderer.c`): all framebuffer, program and VAO
  binds and the blend/scissor/stencil enables go through filters; deletes and
  context creation forget them. `hr_end` keeps the batch bindings; the five
  window-drawing paths bind framebuffer 0 themselves (an audit of every draw,
  clear, blit and read found no other implicit use). `PSX_GL_STATE_CACHE=0`
  passes everything through. CPU time in batch flushes halved (ladder room:
  1.65 to 0.83 ms per frame), redraw feed 2.8 to 2.0 ms; real and redrawn
  images and the presented window are pixel-identical with the cache on and off.

Guest work in the heavy slots fell to 530-680 ms/s at 150% CPU (street 895 to
663), leaving 300-450 ms/s of pacer slack.

**In-betweens follow the rate.** One redraw per game frame gave 60 distinct
images per second at every rate, which is what made the sewer look smooth. The
plugin now asks for one in-between per display refresh of a 30 fps game frame
(round(rate/30) - 1: 120 Hz 3, 144 and 165 Hz 4, 180 Hz 5, 240 Hz 7; Match
Display and Unlimited use the reported presentation rate, capped at 240), with a
main-thread budget of 1.2 + 0.6 x steps ms per field (at most 4). A preparation
is never started when the next present is closer than the recent feed time. All
ten savestate slots at the player's settings (120 Hz, 100% CPU, first person,
4x, 16:9):

| Slot | Redraw share | New image | p99 ms | Guest work ms/s | Pacer slack ms/s |
| --- | --- | --- | --- | --- | --- |
| 0 | 0.718 | 0.237 | 10.6 | 540 | 433 |
| 1 | 0.733 | 0.233 | 10.7 | 567 | 405 |
| 2 (ladder room) | 0.748 | 0.250 | 10.7 | 631 | 343 |
| 3 (street) | 0.742 | 0.248 | 10.9 | 643 | 310 |
| 4 | 0.750 | 0.250 | 9.7 | 637 | 314 |
| 5 | 0.742 | 0.250 | 9.2 | 506 | 459 |
| 6 | 0.723 | 0.227 | 9.6 | 511 | 449 |
| 7 | 0.808 | 0.162 | 10.1 | 546 | 413 |
| 8 | 0.707 | 0.250 | 10.3 | 582 | 370 |
| 9 | 0.690 | 0.167 | 9.7 | 476 | 475 |

At 30 game images per second the ceiling is 0.75 (0.83 when the game drops to
20), so nearly every present at 120 Hz is a distinct image in every slot,
including the street and the ladder room, with no audio underruns and the guest
at 59.9 Hz. At 180 Hz with five in-betweens the street reached 0.75 of 0.83 and
the heaviest slot 0.50. Drawing a missing in-between at its present (instead of
repeating the previous image) costs some regularity (p99 11.0 against 9.7 ms
without it) for 0.74 against 0.69 coverage; it is kept.

**Texture instability and views through geometry.** Redraws are faithful (at
alpha 0 they equal the real image), and geometry and texture correction are off
in this build (`geom_correction`: no precise vertices, affine textures), so
real frames and redraws use the same pipeline. Two causes remained:
- The redraw share itself: in heavy scenes most presents repeated an image and
  then jumped, so outdoor surfaces stepped unevenly between smooth stretches.
  Fixed by the coverage above.
- The eye's room. Portal rendering starts from camera `+0x90`, which the
  in-betweens took from whichever frame was nearer. The camera update stores it
  at `0x8003b0f8` from `0x80039dd0(target position, eye, target room, previous
  room)` (target Duke `0x800d7198`: position `+4`, room byte `+0x2e`), a portal
  walk from Duke to the eye. When the two frames' rooms differ, the worker now
  runs that walk for the interpolated eye and uses its result, so an in-between
  at a doorway renders from the room the eye is really in. Checks: forcing the
  walk on every redraw leaves images pixel-identical (same room returned), and
  a probe that writes the result plus one blanks the view (the result drives
  rendering). `DNTTK_ROOM_WALK=always|off|probe` (developer).
- Not addressed: native PS1 vertex snapping (whole 320x240 pixels) and affine
  texture warp become more visible with small camera steps; geometry/texture
  correction (PGXP) is off for this game and would need the precise vertices
  recorded in the worker to apply to redraws. A third-person camera lerp can
  also pass a wall corner between two frames.

**FMV.** CPU-path presents (24-bit movies) and blank presents invalidate the
replay presenter, so it never shows a stale game image between movie frames.
Intro movie at 120 against 60 Hz: no redraw presented during it, mean luma 42.2
against 41.5 on the same frame.

**Quick kick.** The kick state (active, frame, scratch pointers) ships with
each job, and `kick_frame()` returns the live frame's value in workers. The kick
scratch (`kick_point`, `leg_matrices`) is allocated at the first player update
instead of at the first kick: those allocations reforked the workers and
dropped the in-betweens during the first kick. Sequence dumps at 120 Hz in first
person (`DNTTK_TEST_KICK=90` with the test drive): the leg rises consistently
across real and redrawn images, and none of the 24 presents after the kick ends
shows it; no reforks at kicks.

**Equivalence after the changes** (100% CPU, 630 frames): driven input slot 7
60 against 120 identical, against the earlier 60 baseline identical; 180
identical on a rerun (a single differing 180 run differed from another 180 run
too: the synthetic driver); no input at 180 identical. First person slot 3 at
144: RAM and scratchpad contents identical in all 630 frames, per-frame write
counts equal; only the cumulative MMIO hash differs, from a one-time burst
around the savestate load.

**Regression.** Python suite 97 OK (2 skipped); `ttk-input-test` passes; Vanilla
at 4:3 unchanged (no replay, guest 59.8 Hz, no underruns). Runtime patch
regenerated (17 files, reverse-check OK) by
`recomp/analysis/d17-high-refresh/genpatch.py`. Binary
`5ffcbd643ae8e37f924d794ac76ca68938c4cb64f39de997115197ae7f0dc1e3`.

## 14. Second playtest fixes (2026-10-02, third pass)

The second playtest (fresh game, Match Display = 180 Hz): FMVs fixed; slight
choppiness outside the strip club, in the apartment and in the subway control
room; textures and geometry popping in and out near the eye (sink, wardrobe
sides, light switch and the wall around it, subway walls past the EXIT sign,
the power button); the console `fps` stuck at 60. The session log showed
`emulation behind real time (55-56 frames/s)`.

New harness pieces made these reproducible offscreen on the real GPU:
`DNTTK_TEST_INPUT=FILE` (developer, never set by the launcher) steers the game
from a script (held keys and mouse counts, re-read every input frame);
`steer.py` turns to a heading and walks, `monitor.py` prints per-second guest
fields, game images, presents, distinct images, redraw sessions and GPU load,
`popsweep.py` walks into walls and strafe-turns along them while dumping
present sequences and finding one-present pops, `seqcheck.sh` checks redraws
against their real image at alpha 0, `shift2.py` measures the image shift
between presents. The apartment and subway states from the D11B near-clip work
were copied to private card folders (`CARDS=cards-apartment|cards-subway`).
A present trace (per present: time since the flip, wanted alpha, index shown,
redraws ready and cached, drawn at its present) is in the plugin status.

**Choppiness: images repeated, then the game slowed.**
- The redraw share counted a redraw shown twice as two redraws. Measuring the
  image shift between presents showed the truth: in the apartment at 180 Hz the
  picture moved on the first two or three presents of a game frame and then
  held for four. The image cache held six entries; preparing the next frame's
  five in-betweens evicted the current frame's unshown ones, which were then
  redrawn again at their present (up to 520 redraw sessions per second for 150
  needed). The cache now has 16 entries and evicts images of frames already
  off screen first: exactly 150 sessions per second at 180 Hz.
- Every redraw saved and restored the whole VRAM surfaces (4x colour and
  stencil plus every native-wide surface, about 300 MB per redraw). The GPU's
  memory bus saturated and the driver stalled the emulation thread: guest
  fields fell to 7-46 per second while turning in the apartment (the game
  itself slowed). Redraw sessions now save copy-on-write: a draw area is saved
  at its first primitive (primitives are clipped to it), the wide band it
  mirrors into likewise, and fill, copy and upload destinations before they
  are written; the saved rectangles are restored in reverse order at the end.
  Two rectangles per redraw (its back buffer, 512x240 native, and the wide
  band). `PSX_REPLAY_COW_CHECK=1` hashes every surface (colour and stencil)
  before and after each session: no mismatch in any checked session (apartment,
  street, ladder room). `PSX_REPLAY_COW=0` restores the whole-surface copies.
- In-betweens follow the measured game frame interval (a 20 fps stretch at
  180 Hz gets eight, not five), the budget with them; at most eight.

Offscreen the GPU stays in its P5 power state (746 MHz core, 810 MHz memory;
the driver reports "Idle" with no display attached), so these runs are a worst
case. At those clocks, 180 Hz, first person, 100% CPU:

| Scene | Guest fields/s | Distinct images/s (of about 183 presents) | GPU |
| --- | --- | --- | --- |
| Apartment, full turn | 60-62 every second | 180+ (every present advances; shift test) | 80-90% |
| Street, running and turning, first person | 61-62 | mean 180, min 162 | 70-77% |
| Street, third person | 61-62 | mean 174, min 145 | about 80% |
| Subway states 1-4 | 60-62 | 180-187 steady; 88-133 only in the first 1-3 s after a load (workers re-fork) | 30-60% |

Before: the apartment turn ran the guest at 7 fields per second with whole-
surface copies, 47 with copy-on-write but the cache thrash.

**Popping near the eye.** Several causes, each fixed and checked:
- Out-of-order and stale images (above): near objects move most between
  frames, so a present that went back and forth read as a switch or door frame
  popping. The shift test is now monotonic.
- Rotation blends were not exact at their ends: the quaternion round trip
  perturbs Q12 matrices by about one unit, so an in-between at alpha 0 was a
  sub-pixel shift of every texel edge against the real image, and every
  in-between carried that noise against both neighbours (texture shimmer).
  The endpoint residuals are now added back by weight: redraws at alpha 0 are
  pixel-exact (street first and third person, ladder room, apartment).
- The view matrix (camera `+0x20`, the rotation with row 0 scaled by 1.6) was
  lerped element by element, which skews and shrinks in-between views while
  the camera rotation `+0` was slerped; the first-person weapon (placed from
  `+0`) drifted against the scene. The view matrix is now slerped with row
  scales.
- Near-clip crack seams: two pieces sharing an edge cut it at a frustum plane
  in opposite directions, and the floored corner could land one pixel apart.
  Edge cuts are now computed from the lexicographically smaller end, so
  neighbours get bit-identical corners.
- Remaining, inherent to the original renderer: native-pixel vertex snapping
  (a camera-locked weapon or a wall edge can step one native pixel between
  in-betweens) and T-junction cracks in the original meshes (a crack seen in a
  real game image, not a redraw). Geometry and texture correction (PGXP-style)
  were tried: TTK builds its packets with CPU stores, so the address-keyed
  shadow resolves only 9% of vertices and perspective texturing never arms,
  even in CPU mode; it stays off.

**FPS readout.** With replay presentation the console `fps` overlay and window
title show presents per second, distinct images per second (in-betweens
included, repeats not) and game images per second, then the guest VBlank rate
and real-time speed (offscreen at 180 Hz: FPS 180, Unique 170 while turning,
Game 30, Guest 59.9/60, RT 100%). `render_replay` reports `distinct` and the
copy-on-write counters (`cow`).

**Checks.** Gameplay equivalence at 100% CPU: driven third person slot 7,
60 against 180, 629/629 frames identical; first person street at 180, RAM and
scratchpad identical in all 630 frames. Python suite 97 OK (2 skipped).
Vanilla unchanged (no replay, guest 60.3 Hz, no underruns). `ttk-input-test`
could not run here: it opens a real X window, which the desktop did not map
during this session, and the offscreen driver lacks relative mouse mode (the
`pc_input.cpp` changes are developer switches only). Runtime patch regenerated
(17 files, reverse-check OK). Binary
`71a1d0a20ab7d0a09d0dd03d7ce0f483f4d63f305866ab94544fda0fef6441fd`.

## 15. Acceptance and follow-ups (2026-10-02)

Accepted by the user after a third playthrough at Match Display (180 Hz):
about 180 FPS with very good stability, FMVs correct, fps readout correct.
Remaining issues moved to their own jobs: D17A (texture/geometry instability,
popping and black areas at specific places), D17B (mouse responsiveness and
input latency; the input-latency measurement in this job's acceptance moves
there), D17C (view bob). Binary at acceptance
`71a1d0a20ab7d0a09d0dd03d7ce0f483f4d63f305866ab94544fda0fef6441fd` plus the
D17C experiment (plugin only).
