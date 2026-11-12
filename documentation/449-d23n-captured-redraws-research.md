# D23N - Captured redraws: research and design

Status: research and design for review (2026-11-11); implementation not
started. Job board: D23N. Evidence labels: CONFIRMED (our measurement or
code), INFERRED, UNKNOWN.

## 1. What a redraw worker spends its time on

At 180 Hz three forked worker processes each re-run the game's frame tail
(the title plugin's draw callback) with the late camera, about 10 ms of CPU
a job in light views and 15-17 ms in the level 3 corridor; they ran at
94-100% of a core while walking the corridor (D41M, D50A). Profile of all
three workers, UI slot 9 corridor walk at 180 Hz, offscreen, after D23O part
2 and D23M (`PSX_PROF_WORKERS`, 1 ms CPU samples, about 8,200 each)
(CONFIRMED):

| Share | What |
| --- | --- |
| 40-41% | emulation around the game's code: guest memory reads and writes with their write tracking (`psx_write_word_raw` 4.9%, `psx_read_word_raw` 3.1%), the timing model, dispatch, GTE, recording the stream and host vertices (`rec_hv_mark` 2.6%) |
| 28-30% | our own title code: the unified near-clip renderer (`near_clip.cpp`: world, props and actors drawn host-side since D17AM3: `draw`, `emit`, depth planes, projection), model normal smoothing (D41H), light and vertex records |
| 10-11% | libc and libm (copies, `sqrt`, `floor`) |
| about 1% | game overlay code (`000B3D9C_*.so`) |
| the rest | small items (allocation, vector growth, hashing) |

So most of a job is not the game's own drawing code: it is the emulation
needed to run that code at all, and our host renderer, which already draws
every world, prop and actor polygon itself (CONFIRMED, `near_clip.cpp`
header). The game code proper decides which rooms and objects are drawn,
animates them, sorts them into its ordering table, and draws the sky, the
first-person layer, sprites, particles and the HUD.

## 2. What depends on the camera

- Per polygon (our host renderer): view transform, frustum and near-plane
  clipping, subdivision, projection, host depth, ordering-table slot.
  Everything else (world-space corners, colours, fog source, texture page
  and CLUT, UVs, command bits, light and surface records) is fixed for the
  game frame (CONFIRMED by the renderer's inputs: guest meshes plus the
  render context 0x800d67a8 plus the camera).
- Per object (game code): visibility culling against the view, level of
  detail, billboard sprites facing the camera, particle quads, the sky's
  scroll with yaw (INFERRED from the draw callback's inputs; to be listed
  per routine before building).
- Not at all: the HUD and screen overlays (CONFIRMED: the redraw stream's
  HUD prims are identical across a frame's redraws in still views, D23J's
  reuse).

## 3. How others do it

RT64 (Zelda64Recomp) records each game frame's draw calls once and replays
them at the display rate, interpolating matched transforms on the GPU; the
game's code never runs again for an interpolated frame
([RT64](https://github.com/rt64/rt64)). Doom ports re-render the whole
scene each display frame from interpolated object positions, which is what
our workers do with the game's own code.

## 4. Design

Three steps, each keeping the worker path as the fallback and checked image
for image against it (`PSX_RT_VALIDATE`-style exact comparison of the
stream and of the image):

1. **Scene capture in the frame's first job (N1).** The first worker job
   of a game frame records, besides its GP0 stream, every polygon the
   unified renderer emits in world space with its fixed inputs (a "scene
   list"), and the rest of the stream (sky, first-person layer, sprites,
   particles, HUD) as it is. Later jobs of the same frame run a new host
   path instead of the game's callback: re-project the scene list with the
   job's camera through the same clipping, subdivision and projection code
   (so the decisions are identical), then append the captured remainder.
   Expected: a later job falls from 10-17 ms to the unified renderer's own
   share (about a third of a job) plus no emulation; with two of three
   redraws a frame being later jobs, about 1.0 core saved at 180 Hz
   (INFERRED from the profile shares).
2. **Camera-dependent remainder (N2).** List what in the remainder moves
   with the camera (sky scroll, billboards, particles) and either recompute
   it host-side or keep the game callback for those routines only. Until
   then a frame whose remainder holds such prims keeps the worker path for
   every job (correct, slower).
3. **Re-projection on the render thread (N3).** Once the scene list is the
   input, the render thread can project it itself (CPU or a vertex shader),
   removing the workers from redraws entirely; the parity bar's 1.5 cores
   and the 4-CPU target need this step.

Risks: identical pictures depend on re-running exactly the same clipping
and subdivision decisions (they are ours, so this is controllable); the
game's ordering-table placement must follow the same rule per camera;
objects culled by the game at frame time but visible from the late camera
would be missing at the screen edges (the frame-time culling must use the
widest camera of the frame, or the edge objects keep the worker path).

## 5. Recommendation

Build N1 first, behind a developer switch, on level 3 and UI slots 2 and 8,
with the exact stream comparison against the worker path; measure worker
CPU at 120/180/240 Hz. N2 and N3 follow if N1 holds pictures identical.
This is a multi-session job; nothing of it is built yet.

## 6. Review before building (2026-11-12)

A second profile and a code read before starting N1 change the design.

Profile, all three workers, UI slot 2 turning at 180 Hz, offscreen, the
player's profile (`PSX_PROF_WORKERS` flat samples, about 40,000, plus call
stacks of one worker, about 13,500) (CONFIRMED; stack attribution is
approximate, backtraces from a signal handler):

| Inclusive share | What |
| --- | --- |
| about 25% | host room walk and pieces (`host_vis_walk` -> `near_clip_host_world_piece` -> `world`, `emit`, `subdivide`, `water_clip`): Polymorph walks the rooms itself, per camera |
| about 12% | host props and actors (`object`, `draw`, `smooth_model_normals`) |
| about 58% | the game's own code and its emulation, spread thinly: guest memory access 17% (`memory.c`), cycle and icache bookkeeping 6-7%, compiled dispatch 5% (entry lookup, code-integrity cache, fast-timing observe; each already cached), GTE 3%, PGXP 3%, recording 5%, libc/libm 12% |

No single emulation item is above 4%; the ones that look like overhead
(the code-integrity check, the entry lookup, the debug observer, the
function trace) are already cached or return at once, so trimming them
would save a few percent at most. The saving is in not running the game's
code, as section 4 says.

Corrections to section 2 (CONFIRMED in `frame_replay.cpp` `worker_draw` and
`build_params`):

- Each job blends actor joint matrices (`MatPatch`), object transforms
  (`Frame::Xf`) and object poses (`ObjectPose`) by its own fraction, so
  actor and moving-object corners are **not** fixed for the game frame; only
  static room geometry and fixed props are. A captured actor polygon must be
  stored in model space with the joint or transform it hangs from, and a
  later job must rebuild its matrix as (job camera) x (job's blended world
  matrix).
- Host pieces are not a separate stream: `world()` and `object()` write
  packets into the game's own packet arena and link them into its ordering
  table (0x800d27a0), interleaved with the game's sky, first-person layer,
  sprites, particles and HUD; the recorded GP0 stream is the game's DMA of
  that table. The "captured remainder" therefore has to be rebuilt per job
  in ordering-table order, and much of it is camera- or fraction-dependent
  (billboards, particles, the sky scroll, the weapon's animation).

Revised plan (multi-session; not started):

1. **N0 - remainder inventory.** List, per guest routine, every primitive a
   gameplay job emits outside `world()`/`object()` and what it depends on
   (camera, fraction, neither). Measured with a stream tag per routine, in
   UI slots 2 and 8 and the level 3 corridor. This decides whether N1 can
   skip the game's code at all for typical frames.
2. **N1 - host scene for later jobs.** First job of a slot records a scene
   list in shared memory: static room and prop polygons (world space), actor
   and moving-object polygons (model space plus joint/transform index), the
   render-context inputs, and the remainder routines' inputs. A later job of
   the same slot runs a host path: blended matrices from its params, the same
   `world()`/`object()` clipping and emission into a fresh ordering table,
   then the remainder routines it needs (host versions, or the game routine
   alone for those not yet ported). Exact stream comparison against the
   worker path (`PSX_RT_VALIDATE`-style) gates every step.
3. **N3** as before (render-thread projection).

Until N0 is done the expected saving of section 4 (about 1.0 core) is an
upper bound.

## 7. N0 - what the game draws itself (2026-11-12)

Measured with a temporary developer dump in the worker loop (removed after
the measurement; the framework patch is unchanged): for every fourth base
frame, each recorded job's GP0 packets with their source address (stream
kind 5) and a hash of their words. Sources in main RAM are the game's own
packets; sources in the GPU DMA aperture are our host pieces. Offscreen,
180 Hz, the player's profile, turning (CONFIRMED):

| View | Jobs (frames) | Host packets a job | Game packets a job | Game packets whose content changes between the jobs of a frame |
| --- | --- | --- | --- | --- |
| UI slot 2 | 844 (142) | 463-7,783 (mostly 5,600-6,000) | 26 | 11% |
| UI slot 8 | 717 (120) | 779-2,132 | 29 | 12% |
| UI slot 9, level 3 corridor | 705 (117) | 775-1,727 | 25 | 10% |
| UI slot 2 (repeat, with words) | 532 (88) | - | 26 | 10% |

By kind, over all runs: drawing-mode packets (`e1`, `e3`), flat quads
(`28`) and VRAM uploads (`a0`, 47 and 205 words) never change between the
jobs of a frame. What changes is one semi-transparent textured quad (`2e`)
in almost every job (in the samples it lies below the visible screen,
y 828-1023, and moves by a pixel or two with the camera) and about two of
the 18-20 textured quads (`2c`) a job. Comparing by source address alone
overstates the change: the same packet often sits at a different arena
address in another job.

So nearly all of the 58% of a job spent in the game's code goes into
deciding and positioning what our host renderer draws (room walk, actors,
animation, the calls into `world()`/`object()`); its own output is about 26
packets, nine in ten identical across the frame. N1 therefore needs host
equivalents for only a handful of camera-dependent packets (or the game
routine that emits them alone), on top of re-running `world()`/`object()`
with per-job matrices. Next: identify the routines behind the changing
`2c`/`2e` packets (their texture pages and positions point at sprites or
an effect quad), then build N1 as section 6 describes.
