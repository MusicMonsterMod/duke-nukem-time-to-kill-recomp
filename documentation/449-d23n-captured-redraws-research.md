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
