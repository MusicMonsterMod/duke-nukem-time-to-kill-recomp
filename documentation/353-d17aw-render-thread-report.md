# D17AW - Polymorph: present redraws from a host render thread (stage 1 report)

2026-11-01. Job board: D17AW (report first). Goal: the emulation thread only
emulates; a host render thread draws and presents the worker redraws, so the
slot 10 vista and the slot 2 moat hold 180 presents/s with the game at 60
fields/s, and loads have no slow second. Polymorph only; Classic frozen;
Vanilla unchanged. Status: Done (user-accepted 2026-11-01: "it plays
beautifully ... fully accept"). The stage 1 report is below; R2 was chosen
and is implemented (see Implementation at the end).

Evidence is local under `recomp/analysis/d17aw/`: `budget.py` (per interval:
game fields/s, presents/s, main-thread CPU share from `/proc`, share of wall
time inside redraw sessions, GP0 words per redraw; a post-load timeline in
250 ms steps), `measure.py` (D17AR5 probe copy), perf data, JSON results and
logs. All runs used private copies of the D17AQ fixture card and savestates,
offscreen video, dummy audio, internal scale 4, shedding and late pacing off
(fixed 180), and hash-checked the player's saves and profile before and after.
Build: the current player build (D17AX, 2026-11-01).

## Measured: where the emulation thread goes (slot 10 vista, 180 Hz)

| Case | Game fields/s | Presents/s | Main thread CPU | In redraw sessions | Per redraw | GP0 words per redraw |
| --- | --- | --- | --- | --- | --- | --- |
| Still (3 runs) | 41.4-43.4 | 155-159 | 99.6-100% | 50-51% | 5.18-5.20 ms | 79,400-79,600 |
| Turning | 55.9-57.1 | 173-175 | 87% | 39-40% | 2.44-2.49 ms | 29,900-32,100 |
| D17AR5 (note 342, pre-D17AX) | 59.1 | 178-180 | - | - | 3.07 ms | about 46,000 |

- The thread is saturated, and half of it is the redraw feed. In the vista the
  game gets the other half and makes about 42 fields/s, so the governor steps
  to every second refresh (90).
- **Relation to D17AX:** the vista's redraw grew from about 46,000 to about
  79,500 GP0 words (+73%) and from 3.07 to 5.2 ms. That's the distant geometry
  that used to overflow the 350 KB region and go to the original renderer.
  It now fits in the 1.35 MB region and is drawn in every redraw. D17AR5's
  1-3 frames/s margin over the governor's 57 line is gone. The flicker fix is
  correct; the cost moved into the feed.
- Main-thread perf (3 s, LBR): 78% our executable, 13% the NVIDIA driver, 5%
  libc. The feed's own time is mostly the GPU emulator's command path
  (`gp0_execute_command`, `gpu_textured_triangle`, `prepare_precise_triangle`,
  batch flushes, `gpu_host_vertex_store`), not the driver.
- GPU: 24-49% SM utilisation during the run (`nvidia-smi dmon`); GL timestamp
  span per redraw p50 4.1 ms, p95 14 ms (queue latency, not busy time). Swap
  CPU time 0.02-0.03 ms. The GPU is not the limit.
- After a load (3 loads, 4 s each in 250 ms steps): the game runs 26-57
  fields/s with the thread at 90-106% and the feed at 30-74%. The first
  quarter second is not the worst (42-53 fields/s); the slow time is the
  whole stretch under the feed until the governor steps down. The 3-5 frames
  drawn on the emulation thread before the first worker redraw (D17AQ log)
  are a smaller, separate part (about 0.2 s).

### Headroom

- Without the feed, the game's own thread costs about 12 ms per field in the
  still vista ((100% - 50%) / 41.6 fields) and 8.4 ms turning, so 60 fields/s
  needs about 72% of a thread still and 50% turning. Moving the feed off
  leaves about 28% margin in the worst view (D17AR5 had about 5%).
- The render thread inherits the feed: 5.2 ms x 180 = 936 ms/s, about 94% of
  one thread in the still vista. One render thread would sit at the edge
  there. Two would each run at about 47%. Turning (2.5 ms) is 45% on one.

## How it works today (what a thread must take over)

- **One GL context**, created and made current once on the emulation thread
  (`gpu_gl_renderer.c` `gl_renderer_init_context`). Every GL call runs on
  that thread: live frames, redraw sessions, presents, OSD, captures, debug
  dumps.
- **Presents happen on the emulation thread.** `gl_renderer_rp_tick` runs
  at guest device-service edges and in the frame pacer's wait. At each
  display deadline it asks the title's provider (`frame_replay.cpp`) for an
  image, swaps it, and between deadlines lets the provider prepare
  (feed) the next one.
- **A redraw borrows the live GPU emulator and the live VRAM.**
  `render_replay_begin` saves the GPU emulator snapshot, cycle counters and
  renderer state; `gl_replay_begin` copy-on-writes the VRAM framebuffer; the
  feed pushes the worker's GP0 stream through the same `gpu.c` and
  `gpu_gl_renderer.c` code as live play (switched by
  `g_render_replay_active`); `gl_replay_end` copies the image into a cache
  texture and restores everything. That's why it can only run between
  guest instructions on the same thread.
- **The feed reads guest RAM.** The sprite-tag lookup, the native-4:3
  classifier and the linked-list walk read live RAM and scratchpad. D17AR5's
  memos rely on "no guest code runs during a feed". On another thread the
  live guest keeps running, so those reads would see later frames.
- **State volume:** `gpu.c` (6,300 lines, about 180 file-scope variables) and
  `gpu_gl_renderer.c` (5,900 lines, about 230), all single-instance globals.
- **Workers** are `fork()`ed from the emulation process and refork after
  overlay or mod-memory changes (once per load). They never touch GL.
- **The title side** (`frame_replay.cpp`, 1,900 lines) mixes emulation-thread
  work (capture at the composition hook, publish, submit) with presenter work
  (pick a finished redraw, render it into a cache entry, late-camera pacing)
  over one shared timeline.

## Options

### R1 - The render thread owns all of GL (live frames queued to it)

The emulation thread would send every live GP0 command and VRAM transfer to a
GPU thread, as some emulators do. Not recommended: the live command path reads
guest RAM at command time (sprite tags, classifier, list walk) and feeds host
vertex tables, so the command parse has to stay on the emulation thread and
only the GL backend could move. That means splitting `gpu.c` from its GL calls
at hundreds of sites, re-validating Classic (frozen) and every live frame,
and adding readback synchronisation. It's the largest change and isn't needed
to get the feed off the emulation thread.

### R2 - Redraw renderer instances on their own threads (recommended)

The emulation thread keeps the live GPU, its GL context and presentation
outside Polymorph redraws. Each render thread owns:

- **A shared GL context** (textures, buffers and programs shared; FBOs and
  VAOs per context) and **its own renderer instance**. The file-scope state
  of `gpu.c`, `gpu_gl_renderer.c` and `render_replay.c` becomes per-thread
  (`_Thread_local` in the `.c` files: mechanical, no hashed header touched,
  local-exec TLS is a segment-relative load). This also makes a second
  instance free, if measurement asks for it.
- **A per-frame snapshot instead of borrowed live state:** at publish (30 Hz)
  the emulation thread blits the VRAM framebuffer into a snapshot texture
  (GPU copy, fenced) and records the GPU emulator snapshot. The CPU VRAM
  array is copied only when written (as D17AR5 does), and RAM and scratchpad
  reads during a feed are redirected to the worker slot image that already
  holds that frame's memory in shared memory. A feed then never sees the
  running guest, so D17AR5's memos stay valid per instance.
- **Output:** cache images as shared textures with fences. The thread
  presents them (stage 2b), so presents follow the display on their own clock
  and no longer wait for guest device-service edges. Real images (loading,
  menus outside redraws, FMV) arrive as shared textures from the emulation
  context. When the provider is off (Vanilla, Classic, 60 Hz, netplay),
  presentation goes back to the emulation thread through a synchronous
  handoff.
- **Title split:** capture, publish and submit stay on the emulation thread.
  Pick, render and pacing move to the render thread over a locked timeline.
- **Fallback:** a developer switch (`DNTTK_RENDER_THREAD=0`) keeps today's
  single-thread path. On any render-thread failure (context, snapshot,
  fence timeout) it falls back automatically and logs why.

### R3 - Workers produce ready vertex batches (later, only if needed)

Workers would run the triangle preparation and send vertex batches instead of
GP0, which would shrink the render thread's 5.2 ms. It touches the
render-state, dirty-tracking and ordering logic in `gpu.c`. Keep it in reserve
in case R2 with two instances still misses 180.

## Risks to manage (R2)

- `fork()` with render threads alive: the child gets only the forking
  thread and never runs GL. glibc protects malloc across fork. Our queue
  locks must not be needed by the child (workers use the shared map's
  atomics only).
- GPU memory per instance at scale 4 (VRAM framebuffer, high-resolution and
  depth/stencil surfaces, widescreen surfaces): roughly 150-250 MB each, to
  be measured.
- Savestate loads: an epoch number on snapshots and jobs. A load drops
  in-flight work and the cached images, then resumes on the first new
  snapshot.
- Debug server commands, screenshots and present dumps that read GL state
  must run on the thread that owns the image.
- The OSD and in-game menu overlay are drawn at swap time (`gl_swap_with_osd`)
  and move with presentation.
- HD texture pack uploads and widescreen surfaces: shared textures are fine;
  per-context FBOs are rebuilt per instance.
- NVIDIA submission from two or three contexts: time-sliced by the driver,
  to be measured.

## Plan if chosen (each stage accept-or-stop)

1. **2a - Render thread produces redraws, emulation thread still swaps.**
   Per-thread renderer instance, shared context, per-frame snapshot, feed
   on the render thread, cache images handed back with fences. The emulation
   thread's present becomes a texture blit. Checks: the same pictures (the
   existing `PSX_REPLAY_VALIDATE_BATCH`-style compare against the
   single-thread path), game 60 fields/s in the vista at 180, per-thread
   budgets.
2. **2b - Presentation moves to the render thread**, with a second instance
   if one thread cannot hold the still vista at 180.
3. **3 - Loads, menus, FMV, mode switches, failure fallback, debug and
   capture paths.** The first post-load frames: hold the loading image until
   the first redraw, if the 0.2 s still shows.
4. **4 - 21-level sweep, native tests (controls, near, input, auto rate),
   audio check at 180 and 120, user playtest.**

Acceptance stays as on the board: slot 10 and slot 2 hold 180 at 60 fields/s,
loads without a slow second, no audio dropouts, sweep and tests pass, user
playtest.

## Implementation (stages 2a and 2b, accepted)

User, 2026-11-01: "im so ready, lets go" (R2). Built as reported, with two
changes found while measuring: one render thread by default (a second one
gives no gain on this 8-core machine), and the title's governors adapted to
an asynchronous feed.

### What was built

- **A second renderer instance without touching the live one.** The six
  renderer sources (`gpu.c`, `gpu_gl_renderer.c`, `gpu_sw_renderer.c`,
  `gpu_render.c`, `gpu_vram_dirty.c`, `render_replay.c`) are compiled again
  through wrappers in `runtime/src/render_copy/` with every global they define
  renamed (`rc_names.h`, generated by `render_copy/gen_names.py`; a second set
  `rc2_*` for a developer second thread). File-scope statics are separate per
  translation unit. A global missing from the rename list fails the link as a
  duplicate, so state can never be shared by accident. `rc_overrides.h` points
  the copy's PGXP mesh lookup, text-translation upload hook and guest RAM
  write at no-op replacements (a feed ran with PGXP suppressed and must not
  touch the guest).
- **Render thread** (`render_thread.c`): a hidden 16x16 window and a GL
  context shared with the emulation thread's, its own renderer instance
  initialised at the player's internal scale and filter, a request queue and
  fence-ordered handoff of shared textures.
- **Per-frame snapshot** at each worker publish (`gl_rt_capture`, ring of 3):
  GPU copies of the high-resolution VRAM surface (colour, depth, stencil), the
  raw VRAM mirror and every widescreen surface; the CPU VRAM array; the GPU
  emulator state (`gpu_snapshot_write`); the renderer state; and the
  widescreen configuration and live classifier inputs of `gpu.c`
  (`gpu_rt_ws_export`: sprite tags, GTE and overhang stamps, backdrop ring,
  cull sites). The render thread loads a snapshot into its own surfaces only
  when it changes (`gl_rt_load`) and runs `render_replay_rt_session`: the
  GPU and renderer parts of a session, none of the guest's cycle counters,
  interrupts, PGXP or debug state.
- **Asynchronous redraws.** `psx_mod_replay_render_job` queues the redraw;
  the cache entry becomes valid when the emulation thread adopts the finished
  image (`gl_rt_adopt`, fence-waited). Job release is deferred until the
  thread has read the job's stream. Presenting a cache image leaves a fence
  the thread waits on before redrawing that entry. A savestate load drops
  queued work (`psx_rt_drop`). `exit()` and the GL shutdown stop the thread
  first (`atexit`), which fixed a crash on the debug quit.
- **Title (`frame_replay.cpp`)**: cache victims skip entries being drawn;
  the late-camera leads include the render thread's 90th-percentile latency;
  a due present whose image is already being drawn waits for it (at most 4 ms,
  and at most 30 ms of waiting per second, `DNTTK_RT_DUE_WAIT_MS`); the slack
  and budget governors no longer hold back queueing (they kept main-thread
  feeds out of the game's time; queueing costs the game almost nothing, and
  holding finished jobs back fed a loop where the game drew live frames
  itself). Polymorph turns the render thread on; Classic and
  `DNTTK_RENDER_THREAD=0` keep the single-thread path. Vanilla never draws
  redraws.
- **Developer**: `PSX_RENDER_THREAD=0|1`, `PSX_RENDER_THREADS=2`,
  `PSX_RT_VALIDATE=1` (every 8th render-thread image is compared with the
  same job drawn the single-thread way, exact RGBA), debug command
  `render_thread`.

### Evidence (offscreen, player settings: scale 4, 16:9, CPU 150%; private fixture copies)

- **Same pictures**: with validation, 1646 of 1646 sampled images exact
  across all 10 fixture slots (still and turning), 0 different, 0 skipped,
  0 failed redraws; 597 of 597 with two render threads.
- **Slot 10 vista, fixed 180 (shedding off)**: game 41-43 to 59.9 fields/s,
  presents 155-159 to 180, emulation thread 100% to 67-74%, its feed share 50%
  to 0. Distinct images about 110-115/s (was 89): the workers now limit it
  (below).
- **All fixture slots, fixed 180**: 60 fields/s and 180 presents in every
  slot, emulation thread 53-74%. The other nine slots show 178-180 distinct
  images/s, as the single-thread path did.
- **Route sweep at 180 with the governor (D17AR5 `routes.py`)**: 162 of 180
  route seconds at full rate (D17AX build the same evening: 130). Every slot
  except the vista holds 180 on every route; slot 2 (fixture 1) goes from 99
  to 180 presents/s, fixture 2 from 107 to 180. The vista: 92 (was 80).
  Game 58-60 fields/s on every route.
- **Loads (D17AQ `crackle.py`, governor on)**: slot 2: 0 audio underruns and
  180 presents (D17AX build: 504 and 90). Slot 10: 583-4365 underruns in
  three loads against 796-6177 for the D17AX build; both still run about 45
  fields/s in the first second after the load. Not improved there.
- **Quit**: 4 of 4 clean exits (`atexit`), Vanilla 60 fields/s with no
  redraws and no render thread, Classic single-thread path.
- Tests: `ttk-controls-test` 119 PASS (full level set), `ttk-near-test`,
  `ttk-input-test`, `test_auto_frame_rate.py`. The patch is exported and
  applies to the pinned commit; the codegen hash is unchanged (0x8bab543c).

### Limits found

- **The slot 10 vista standing still cannot hold 180 fresh images.** Three
  workers take 21-23 ms per vista job (about 133 jobs/s), so their p99
  readiness reaches 53-58 ms at 180 and many images miss their present. More
  workers make it worse on this 8-core CPU (each job, the render thread and
  the game all slow down). The render thread itself is not the limit there
  (two threads were each about 40% busy). The fix is cheaper worker jobs, a
  follow-up job.
- The slow first second after loading the vista remains (both builds about
  45 fields/s; stage 3).
- Not measured: GPU memory of the second instance, 120 Hz, the player's
  display, the 21-level sweep with this build (the route sweep covers the 10
  fixture slots).

### Acceptance

User, 2026-11-01: "perfect. it plays beautifully. you can check my logs to
make sure its all in order. fully accept. lock it in, close us out". Their
session (slot 2, about 7 minutes): one step down to every 2nd refresh right
after the load (audio ring 28 of 180 ms), recovered, then 180.0 presents/s in
every full minute; window-close exit without a crash; RSS 408 to 472 MB,
levelling off.
