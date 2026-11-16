# 493 - D23N5: what a frame's redraw jobs share, and the same-image restore (desktop)

Autonomous three-machine session of 2026-11-16 (desktop lane). User: "D1.
Profile a redraw worker job by host hook (ttk::draw, the near clip, D36
joint callbacks, the world pass, model lighting) in UI 2, 5, 9 and FAMILY
JEWELS: ms per job, calls, what depends on the in-between camera/pose and
what is the same for every job of a frame. D2. Build the largest exact cut:
compute once per game frame what every in-between job of that frame shares
(results keyed by inputs, reused by the later jobs, verified word for word
with a developer check), behind a developer switch; prove with
verify-first-style stream comparison (0 differences), PSX_RT_VALIDATE,
Vanilla shots; measure worker CPU at 1 and 2 workers, then on the laptop
and citadel."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Setup

Desktop (i7-5960X, GTX 1080 Ti), the session worktree at `33e40e1`, lab
`recomp/analysis/d23n5/` (local; a copy of the d23n2b lab and its private
card copy), `Frame rate: 180`, offscreen, case `wcpu` (load, settle, 40 s of
walking, turning and firing). Views: UI 2, UI 5, UI 9 and the FAMILY JEWELS
start (runtime slots 1, 4, 8, 11). Host checked before every run (CPU idle
over 3 s, I/O pressure, no backup, package or build process; every run
passed; `packagekitd` was resident, idle). Worker profiles:
`PSX_PROF_WORKERS` with `PSX_PROF_STACKS` (1 ms of worker CPU a sample),
`d1prof.sh`, summarized by `cats.py`, `fincl.py`, `tree.py`, `split.py`.

A finding on the way (CODE): this desktop's framework working tree did not
hold last night's laptop-lane framework commits (D23V3's capture, the
D23Y5 OSD cache); they existed only in the tracked patch. The profiles and
A/B in sections 2-5 ran on that tree (neither change runs in a worker job
with its switches as set); the proofs in section 6 ran after merging this
work onto HEAD's framework (three-way merge, the exported patch verified
against a clean pinned checkout on citadel).

## 2. Where a worker job goes (MEASURED, D1)

Share of the worker's job time (`worker_draw`), one worker, first runs
(the second runs agree within 2 points of whole-game CPU):

| Part | UI 2 | UI 5 | UI 9 | FJ start |
| --- | --- | --- | --- | --- |
| world pass (`draw_distance_apply` > `host_vis_walk` > `near_clip_host_world_piece` > `world`) | 27.0 | 13.6 | 11.7 | 23.6 |
| of it in our Polymorph `draw`/`emit` | 21.1 | 10.6 | 8.5 | 17.4 |
| object path (`object`, game `0x80010000`) | 12.0 | 20.2 | 3.3 | 14.9 |
| water (`water_clip`) | 4.8 | 0 | 15.4 | 5.0 |
| small models (`model_link`) | 1.3 | 3.3 | 0 | 0.2 |
| D36/aim hooks (`aim_hook_body`, `observe`) | 1.9 | 0.3 | 1.9 | 1.8 |
| other host hooks (HUD, gadget, keyed icons) | 2.5 | 3.2 | 5.9 | 2.2 |
| GPU port writes (the game's OT submit, `gp0`) | 5.4 | 4.3 | 3.8 | 5.3 |
| GTE | 2.3 | 3.5 | 3.1 | 3.1 |
| the game's own code | 29.5 | 36.0 | 36.5 | 29.4 |
| dirty-RAM dispatch checks | 4.1 | 4.4 | 5.2 | 4.4 |
| other (dispatch, overlay and translation bookkeeping) | 9.1 | 11.2 | 13.3 | 9.9 |

Our Polymorph `draw`/`emit` alone (inclusive, every caller): 33.0, 25.3,
23.6, 30.0% of a job. Outside the job, in the worker loop, the image load
before each job: 5.3, 5.1, 16.2, 5.4% of the worker (UI 9's jobs are
cheap, so its fixed load weighs more), plus the text-guard resync after it
(0.6-1.7%).

Calls: a job runs `world()` once per visible room piece and `object()`
once per drawn mesh (CODE); the profile does not count calls, and the
`[n4 cost]` counts of note 487 give the scale (575-1,140 top-level hooks a
job). The per-job ms follow from the shares: the mean job is 8-10 ms in UI
2 and the FAMILY JEWELS start, 6-8 ms in UI 5, 3-5 ms in UI 9 (note 487
section 3, `job_us_avg`).

## 3. What depends on the in-between camera or pose (CODE, D1)

Mapped from `worker_draw` (frame_replay.cpp) down:

- Per job: the camera (slerped/lerped in `build_params`, the late mouse
  view), the eye's room walk, Duke's joint matrices, every drawn object's
  interpolated transform (`transform_load` at `0x800292a0`), the blended
  RenderState (kick, door frame, projection blend and the rest) and the
  held draw clock. `world()` and `object()` read the GTE rotation and
  translation the camera sets, so every projection (`near_project`), fog
  colour (`near_dpcs` on depth), back-face and rectangle cull, depth slot,
  clip and `emit` is per job; so are the D36 callbacks (camera, joints,
  blended state) and the normals of `smooth_model_normals` (view space).
- The same for every job of the image: the guest image itself, the mesh,
  room, texture and animation tables, the vertex and face records, the
  lamp-bake surface of a face (`lamp_bake_surface`), the frame's lights
  (`lights_publish`, once per tick), the topology of normal smoothing and
  the room/portal decode.

Measured size of the shareable part: `world()` outside its draw core is
5.5-5.7% of a job and `object()` 4.4%, and those include per-job work
(`near_project`, `front_facing`, `near_dpcs`, the culls); the camera-free
part (record reads, texture entries, `lamp_bake_surface` 0.4-0.6%,
topology about 1%) is about 2-3% of a job, with hidden state against
reuse (`clear_color_index` rewrites vertex nibbles in guest RAM during a
job, `model_round`'s per-thread reach map, `image_serial`, the packet
allocator's order). The portal decode is under 0.5%. So the host hooks
note 487 found to be most of a job are almost entirely per job.

The largest shared cost is the image itself. Each job loads its slot image
whole (`render_replay_image_load`); a developer statistic
(`PSX_RW_IMAGE_STATS=1`, render_worker.c) measured what a job changes
(4 KB pages differing from the slot image after the job, 40 s each):

| View (1 worker) | a load copies | pages a job changes (RAM / DMA / mod) | next job loads the same image |
| --- | --- | --- | --- |
| UI 2 | 4,586 KB | 23.7 / 31.3 / 1.1 | 50.0% |
| UI 5 | 3,523 KB | 22.1 / 16.9 / 1.1 | 50.2% |
| UI 9 | 5,488 KB | 16.0 / 4.8 / 0.2 | 50.0% |
| FJ start | 4,141 KB | 22.4 / 20.5 / 1.7 | 51.4% |

With two workers the jobs of an image spread over both processes: 17-26%
of jobs load the image their worker already holds.

## 4. The cut: restore only the written pages (CODE, D2)

`PSX_RW_RESTORE=1` (developer, off by default; Linux 6.7+). The worker
tracks writes to its RAM, mod memory and DMA window with the kernel's
asynchronous userfaultfd write protection (`UFFD_FEATURE_WP_ASYNC`,
unprivileged through `UFFD_USER_MODE_ONLY`): a write to a protected page
is resolved in the kernel without a signal and leaves the page "written";
`PAGEMAP_SCAN` lists the written pages and protects them again. When a job
draws the same published image as the worker's previous job (same slot,
no publish in between: a new per-slot publish generation), only the
written pages are copied back, clipped to the extents a whole load
copies; the scratchpad is copied whole and the load's bookkeeping runs as
before. Exact by construction: an unwritten page still holds the image's
bytes, and bytes past an extent are left as a whole load leaves them. The
text guard's bitmap is kept from the whole load and only the written pages
are compared again (keyed by a new reference-image epoch, moved by
registration and by the bless of intentional patches). memory.c
(`render_replay_image_load_tracked`), render_worker.c; the three arrays
gained 4 KB alignment. If any kernel step fails the worker loads whole.

A first version tracked with `mprotect` and a SIGSEGV handler: exact, but
its 36-44 signal round trips a job cost 2.2% of the worker, as much as the
copy it saved; replaced.

Developer check `PSX_RW_RESTORE_VERIFY=N`: every Nth job that draws the
same image as the previous one is checked: the restored image byte for
byte against the slot image, the text bitmap against a whole recompute,
then the job is run again from a whole load, untracked, on the first run's
draw clock (frame_replay.cpp's clock hold takes a framework-owned repeat
value), and the two streams are compared word for word. With the restore
off it measures how repeatable a job is.

Per load (UI 2, one worker): a whole load 647 us before, 790-800 us now
(the protection around the copy), a restore 74 us (about 70 pages); 34-36
written pages a job.

## 5. Worker CPU (MEASURED)

Interleaved and rotated, two runs each, worker processes only (% of a
core), `ab.sh`:

| View | 1 worker off | 1 worker on | 2 workers off | 2 workers on |
| --- | --- | --- | --- | --- |
| UI 2 | 43.2, 42.4 | 42.5, 42.7 | 44.7, 45.5 | 45.4, 45.2 |
| UI 5 | 28.6, 27.8 | 27.6, 27.3 | 29.4, 29.7 | 31.5, 30.6 |
| UI 9 | 22.6, 24.1 | 23.1, 20.8 | 27.1, 25.6 | 26.7, 25.3 |
| FJ start | 31.0, 32.9 | 32.2, 33.7 | 37.3, 31.6 | 31.9, 32.9 |

Within the run-to-run spread at one worker, and at two (UI 5 perhaps 1-2
points dearer: fewer jobs reuse an image and every whole load pays the
protection). The arithmetic agrees: about 210 us saved a job at one
worker, 2% of a job, minus the faults and the dearer whole loads.

## 6. Proof (MEASURED; merged onto HEAD's framework)

`proof.sh`, every other same-image job checked (`PSX_RW_RESTORE_VERIFY=2`),
40 s a view:

| View | jobs checked, 1 worker / 2 workers (image and stream each) | images differing | text bitmaps checked, differing | streams differing |
| --- | --- | --- | --- | --- |
| UI 2 | 526 / 245 | 0 | 1,546, 0 | 1 / 1 |
| UI 5 | 551 / 237 | 0 | 1,580, 0 | 0 / 1 |
| UI 9 | 630 / 314 | 0 | 1,889, 0 | 0 / 0 |
| FJ start | 477 / 260 | 0 | 1,477, 0 | 0 / 0 |
| All | 3,240 | 0 | 6,492, 0 | 3 |

Every stream difference is the job on the line before the level's
lightmap upload (`[gl bake] ... uploaded once`): the bake lands between
the check's two runs. The same check with the restore off shows the same
difference at the same moment (UI 5, identical word counts and first
differing word), so it is the check's own level-load baseline, not the
restore; outside it, 0 differences in 3,240 compared streams (the
pre-merge proof the same way: one difference, UI 5's level load).

- Render-thread validator (`PSX_RT_VALIDATE=1`, restore on, one worker, UI
  2, 8, 9, 40 s each): 910 of 912 images exact; both bad images are in UI
  9's savestate load (one before, one after the bake upload). With the
  restore off, two runs: 910 of 910 and 914 of 916, the two bad in the same
  level loads (the same 896,250-pixel image after the bake), as notes 481
  and 483 also recorded one there. The pre-merge build with the restore
  on: 918 of 918.
- Vanilla (`--vanilla`, case `vshots`, UI 8 and UI 2, shots at +60, +200
  and +400 frames), `ab/now5` (the build before this work) against this
  build: byte-identical, 6 of 6 (before and after the merge).
- Framework patch: exported, and a clean pinned checkout
  (`08ec704`) plus the patch reproduces all five touched files (SHA-256 on
  citadel); it differs from HEAD's patch only in memory.c and
  render_worker.c.

## 7. Decision

D1: the host hooks are almost all per job; the part every job of a frame
shares is small: the decode (2-3% of a job, with state that blocks reuse)
and the image (5-16% of the worker). D2 built the exact cut on the image
and it does not pay on the desktop. It stays a developer switch, off by
default, for the field laptop, where a 4.5 MB copy costs more against its
memory bandwidth and one worker gives half the jobs a reusable image.
Larger worker savings need cheaper per-job host draws (`draw`/`emit` are
23-33% of a job), not sharing between jobs.

## 8. Reproduce

`recomp/analysis/d23n5/` (local): `d1prof.sh` (env PRE, SLOTS, REPS,
WORKERS, NOPROF, EXTRA), `cats.py`, `fincl.py`, `tree.py`, `split.py`
(profiles), `ab.sh` (A/B), `proof.sh` (Vanilla shots, validator, checks).
