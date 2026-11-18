# 510 - D23AK: what a redraw job shares with the other redraws of its game frame

Job: [D23AK](../jobs/campaign-performance.md#d23ak---a-lighter-redraw-worker).
Builds on [note 509](509-d23ag-overnight-chain.md) section 6 (the worker is
saturated on the field laptop at BALANCED 60) and [note 493](493-d23n5-shared-frame-work-desktop.md)
(D23N5: the same question at the AUTO grade in UI 2, 5, 9 and the FAMILY
JEWELS start). Labels: MEASURED (a run on the named machine), CODE (read
from the source), INFERRED.

User, 2026-11-18: "Profile the worker per redraw, find what depends only on
the game frame and not the in-between camera, and compute it once per game
frame for all of that frame's redraws. It must stay exact (PSX_RT_VALIDATE,
VRAM dumps) and start behind a developer switch. Measure on the laptop's
real screen with the d23ag census. Goal: BALANCED at 60 new pictures a
second in those levels."

## 1. Summary

- The world walk is 30-34% of a redraw job on the laptop (THE REAPER, 29),
  17% on level 22, where the sleeping enemies (D17BF) take another 18%.
  Note 509's "about 20%" came from 149 eu-stack samples; these are
  2,273-3,082 job samples a level.
- Almost all of it follows the in-between camera (CODE, section 3):
  projection of every corner, culls, clipping, subdivision, depth slots,
  packet addresses, host vertices and lit world positions. The part that
  depends only on the game frame (record and texture reads, the baked lamp
  surface lookup, the list decode) is about 2% of a job, as D23N5 found at
  the AUTO grade (2-3%).
- So computing the walk once per game frame cannot be exact for more than
  about 2% of a job; the goal needs about 30% (24 ms a new picture on THE
  REAPER, 16.7 ms for 60). Nothing was built.
- D23N5's exact same-image restore (`PSX_RW_RESTORE=1`, off by default)
  measured at BALANCED: within the runs' spread (section 4).

## 2. Profiles (MEASURED)

Census route (d23ag `census.py`), BALANCED rows, FRAME RATE 60, third
person, one level a launch, one redraw worker, `PSX_PROF_WORKERS` with
`PSX_PROF_STACKS` (1 ms of worker CPU a sample); summarized by
`recomp/analysis/d23ak/stk.py` and `sub.py` (local).

Field laptop, real 60 Hz screen (host gate OK at each level):

| level | new pictures median / lowest tenth | worker % | job samples |
| --- | --- | --- | --- |
| 27 THE REAPER | 39.5 / 28.9 | 95.6 | 2,273 |
| 29 | 38.4 / 32.5 | 95.1 | 3,052 |
| 22 | 43.8 / 32.9 | 91.4 | 3,082 |

Inclusive share of a job (each function counted once a sample):

| part | 27 | 29 | 22 |
| --- | --- | --- | --- |
| world walk (`draw_distance_apply` > `host_vis_walk` > `near_clip_host_world_piece` > `world`) | 30.0 | 33.6 | 17.0 |
| of it `draw` / `emit` | 83.6 / 61.2 of the walk | - | - |
| object path (`object`) | 14.1 | 12.9 | 18.6 |
| sleeping enemies (`sleeping_draw`, D17BF) | - | - | 17.6 |
| generated game code (self, of all worker samples) | 14.8 | 14.9 | - |

Outside the job, in the worker loop: the image load 4.7% and the text
guard's resync 1.2% of the worker's samples (THE REAPER). The worker's self
time on THE REAPER is spread thin: our `draw` 8.2%, `emit` 5.7%, host
vertex recording 3.8%, guest reads and writes through the bus about 11%,
dispatch lookups about 8%, the debug, trace and cycle helpers about 4%; no
single block near the 30% the goal needs.

Inside the world walk (laptop, share of the walk's samples, 27 / 29 / 22):
`world`'s own loops 7.8 / 8.2 / 8.8, its guest reads 3.5 / 3.7 / 2.5,
`near_project` 2.5 / 2.6 / 1.3, `lamp_bake_surface` 1.9 / 1.4 / 0.6; the
rest is `draw` and below.

Desktop, offscreen, THE REAPER, the same settings (1,311 job samples,
worker 62.8% at 59.9 new pictures): world walk 29.6%, object path 20.4%,
`draw`/`emit` 35.9 / 24.8% inclusive, `psx_write_word_raw` 12.7%; image
load 5.1% outside the job.

## 3. What depends on the in-between camera (CODE)

`world()` (near_clip.cpp) for each visible room piece: every vertex record
is projected through the GTE matrix the job's camera set (`near_project`),
its fog colour taken from that depth (`near_dpcs`); faces are taken or
left by those positions (unsafe, oversize, near), back-face culled with the
view-space determinant (`front_facing`), rectangle culled on screen,
given their ordering slot from view depth, and ranked. `draw()` clips each
face against the near and four guard planes in view space, splits by
screen size (`unified_oversize`, `subdivide`), and `emit()` allocates its
packet from the frame's arena (an address that depends on everything
emitted before it), writes colour and screen positions into guest memory,
links it into the ordering table and records the host vertices (exact
sub-pixel screen position, view depth, the depth-plane fix at the raster
position) and, with LIGHTING ENHANCED, each corner's world position
relative to the eye. All of it follows the camera, which differs in every
job: the camera is interpolated and the late camera (D17B) gives each job
the mouse look a fixed lead before its own present.

Camera-free in the walk: the vertex and face records, texture entries and
animation frames read from guest memory, `lamp_bake_surface` (D41M), the
list's coplanar signature (already cached per mesh by D17AR3). About 2% of
a job; note 493 section 3 lists the hidden state that also stands against
reusing them (the colour-index nibble clear, the packet allocator's order).

The sleeping enemies (level 22): each stand-in is drawn by the game's own
actor renderer 0x80032e78 through the GTE, joints composed with the camera;
per job too.

The one whole-image share: the game frame's own image, which the emulation
thread draws anyway. D17B replaced it on purpose with an alpha-0 redraw:
shown, it swings the view back to the older game camera at every game
frame. Reusing it as one of a frame's two pictures at 60 would halve the
worker's jobs but is not exact and brings that judder back.

## 4. D23N5's restore at BALANCED (MEASURED)

Field laptop, real screen, BALANCED 60, census route, off and
`PSX_RW_RESTORE=1` interleaved, two runs each (new pictures median / lowest
tenth, worker %):

| level | off | restore |
| --- | --- | --- |
| 27 | 52.9 / 36.7, 92.8; 45.4 / 31.7, 95.2 | 43.8 / 30.4, 92.7; 51.4 / 36.9, 92.0 |
| 29 | 39.8 / 30.9, 94.8; 41.7 / 32.3, 92.9 | 39.6 / 33.3, 95.6; 44.3 / 30.1, 95.3 |

Within the spread (THE REAPER alone moves 45-53 between identical runs).

## 5. Directions (PROPOSED, not built)

- Exact per-job cuts in the worker: a RAM fast path for the title's own
  guest reads and writes, the worker's diagnostics (function trace, debug
  observer) off in workers, cheaper dispatch lookups. Each block is 1-11%
  of the worker; together perhaps 10-15% (INFERRED from the profile), short
  of the 30%.
- Picture trade-offs (D23AI, note 509 section 3): RESOLUTION 1x (worker
  -30%: the CPU reaches full turbo), draw distance ORIGINAL (worker 92 ->
  69% on level 6; it also removes the sleeping enemies), LIGHTING ORIGINAL.
- The game frame's own image as one of each frame's two pictures at 60:
  half the jobs, not exact, the D17B judder.

## 6. Option 1: exact per-job cuts (`PSX_RW_LEAN=1`)

User, 2026-11-18: "option 1 it is" (resolution 1x ruled out: "we just cant
play the game like that").

### What changed (CODE, developer switch, off by default)

Framework (`memory.c`, `render_replay.c`, `render_worker.c`, `mod_plugins.h`;
exported into the tracked patch, a clean pinned checkout plus the patch
reproduces all four files):

- `psx_mod_write_words` / `psx_mod_read_words`: n consecutive words in one
  call where the word-by-word path has no effect beyond the bytes (no
  lockstep or data-shard recorder, no overlay cycle flush, no RAM read
  watch, cache not isolated, the range inside one RAM or GPU DMA memory
  span); otherwise word by word. The write does exactly what the word path
  does there: PGXP store invalidation per word, the guest store counter and
  last-store record, the extra region's high-water mark.
- The replay's DMA walk reads each list node in one batch and appends its
  words to the stream as one run (`rec_words`, the same items as word by
  word).
- The record of which vertex words a host draw described (`rec_hv_mark` /
  `rec_hv_marked`, a 32,768-slot hash probed 16 deep): within a job no slot
  is freed, so a placed address is always met before any free slot, and
  whether a mark is kept depends only on which slots are taken. A 4 KB
  bitmap of taken slots plus a table indexed by address (packets are written
  in address order) give the same keeps, drops and answers without the
  hash's cache misses (78% of the recorder's own time was one miss).
- `PSX_RW_LEAN_VERIFY=N` (developer): every Nth job is run again from a whole
  image load on the first run's draw clock with the cuts suspended
  (`g_psx_rw_lean_suspend`), the two streams compared word for word
  (`[rw lean]` lines).

Title (`near_clip.cpp`, `live_env.cpp`, `frame_replay.cpp`,
`lamp_bake_map.cpp/.h`):

- `emit` writes a packet's corner words in one batch and, when that took the
  batched path, uses the screen words it wrote instead of reading them back.
- `world` reads a mesh's vertex records in one batch.
- `draw` runs on a shape type that neither initialises nor copies unused
  corners (`ShapeLean`; a `Vertex` is 112 bytes, a shape holds 12); `draw`
  is a template over the two shape types.
- `subdivide` (unified geometry) emits a piece that is not split before
  computing the edge lengths it reads only for a split.
- `lround_exact`: for finite |x| < 2^52, rounding half away from zero from
  the exact fraction x - trunc(x), lround's result without the library
  call; colour, texture and depth words.
- `ttk_live_env` caches what a name resolves to per thread by the name's
  address and the slot list's generation (every caller passes a literal;
  nothing sets a `DNTTK_` variable after start); a slot's value is still
  read at each call. It had read the environment with `getenv` on hot draw
  paths.
- The bake table: a developer re-run on the first run's clock also reads the
  bake half the first run pinned (`lamp_bake_pinned`, `lamp_bake_pin_half`),
  so a bake published between the two runs no longer makes them differ
  (D23N5's level-load "baseline" differences had the same cause).
- `DNTTK_LEAN_BISECT=mask` (developer) turns single title cuts back off.

### Exactness (MEASURED)

- `PSX_RW_LEAN_VERIFY=2`, desktop offscreen, BALANCED 60, census route on
  27, 22 and 29, one worker, with D23N5's restore on: 3,776 and 3,840
  checked jobs (final build), 0 streams differing. Before the bake-half fix,
  2 of 3,776 differed, both on the job beside a level's bake publish.
- `PSX_RT_VALIDATE=1`, the same route with both switches: 952 exact, 0 bad,
  1 skipped (a store change during level 29's travel).
- VRAM dumps (d23y8 `vx.sh`, savestate, no input, +60/+200/+400, FRAME RATE
  `original`, so the emulation thread's own frames with the title cuts):
  Modernized UI 2 and Vanilla UI 9 byte-identical off against on.
  Modernized UI 9's +400 differs (level timer one field apart, a speck on
  the weapon), but every subset of the cuts matched off, and the off build
  under CPU load gave a third +400 result: that frame follows host timing,
  as note 508 found for the FAMILY JEWELS start.
- Local CI: unit tests OK, the five framework test scripts pass.

### Cost (MEASURED)

Desktop, offscreen, census route, one worker, two interleaved runs each
(worker ms a finished job from the worker's own counters; pictures 60):

| level | off | `PSX_RW_LEAN=1` + `PSX_RW_RESTORE=1` |
| --- | --- | --- |
| 27 | 10.7, 9.9 | 8.5, 8.7 |
| 29 | 10.6, 10.7 | 8.7, 8.9 |
| 22 | 9.7, 9.8 | 7.3, 8.1 |

About 17-20% off a job; the emulation thread 2-4 points lower too (it runs
the same `emit`, lookups and rounding).

Field laptop, real 60 Hz screen, BALANCED 60, two interleaved runs each
(new pictures / jobs finished a second / worker ms a job):

| level | off | lean + restore |
| --- | --- | --- |
| 27 | 38.6 / 51.7 / 18.4; 36.1 / 45.9 / 20.8 | 40.4 / 52.4 / 18.3; 42.3 / 49.8 / 19.0 |
| 29 | 38.3 / 42.6 / 22.3; 38.0 / 44.5 / 21.3 | 37.7 / 52.2 / 18.2; 43.2 / 48.8 / 19.4 |
| 22 | 41.7 / 47.7 / 19.2; 43.6 / 51.2 / 18.6 | 44.6 / 53.2 / 17.6; 45.7 / 52.6 / 15.4 |

About 10% off a job on the laptop; new pictures a few a second higher,
within the runs' spread. **The goal is not met.** Two findings:

- About a fifth of the jobs the worker finishes are never shown (46-53
  finished, 36-46 new pictures): a job that misses its present is wasted.
  At 15-19 ms a job against a 16.7 ms cadence they are late all the time,
  so 60 needs a job of about 15 ms or less there, about 20% more.
- What is left on the laptop (lean profile, THE REAPER): the game's own
  code 17.6% self; the dispatcher's lookups (overlay dispatch, entry
  lookup, native text ranges) about 11% inclusive; the image load and
  resync about 6%; the title's world walk about 25% inclusive, now mostly
  arithmetic. The game code's per-block calls (`psx_icache_fetch`,
  `debug_server_cyc_observe`, about 3%) need a code generator change.

## 6b. Further: fast CPU timing in the workers (MEASURED)

User, 2026-11-18: "keep pushing forward with it as far as you can ... I think
the dispatcher yes".

- The dispatcher (CODE): every call runs a generated chain (function trace,
  BIOS HLE hook, call stubs, text checks, `dirty_ram_dispatch`, the overlay
  loader, the static table, the per-function text validity check). Every
  image load also cools all its caches (`image_load_tail`). Upper bound,
  measured by skipping that invalidation (not exact, measurement only, then
  removed): 0.2-0.6 ms a job on the desktop. Not pursued; the chain itself
  is generated code (an emitter change and a savestate allowlist).
- The per-block timing hooks: D23F's fast model (`fast_timing.h`, already
  force-included in the game shards) skips the icache, pipeline and
  per-load hooks when `ttk_ft_on` is set, but its lease comes from the player
  update, which a worker never runs, so every worker job ran the accurate
  model's hooks. Now (`PSX_RW_LEAN=1`) the outermost worker job takes the fast
  model (`ttk_fast_timing_job`, fast_timing.c). A frozen job's guest sees no
  clock (the replay's device policy reads the root counters as 0 and holds
  the interrupt state; `psx_check_interrupts_at` returns at once in a
  replay), so its stream is unchanged; the lean check's reference run keeps
  the accurate model. A worker never services devices, so the job also lifts
  the device deadline (it only decided how often cycles were published; left
  in the past it sent every branch edge down the slow path: 2.6% -> 0.07%)
  and puts it back after the job.
- Smooth model normals: each face's length once, not again for every corner
  that touches it (the same expression; the build has no FMA contraction).
- Tried and dropped: claiming jobs oldest first (field laptop: 18-25 new
  pictures, worse: the oldest job is the one already late), latest first
  and earliest feasible present first (no better than the default order).

Exactness (final build): `PSX_RW_LEAN_VERIFY` desktop 3,776 and field laptop
real screen 1,536 checked jobs, 0 differing; `PSX_RT_VALIDATE` 964 exact, 0
bad; VRAM dumps and pictures identical off against on (Modernized UI 2,
Vanilla UI 9); the exported patch reproduces every touched framework file
from a clean pinned checkout; local CI passes.

Cost, worker ms a finished job, census route, BALANCED 60:

| level | desktop off | desktop lean | laptop off | laptop lean |
| --- | --- | --- | --- | --- |
| 27 | 10.3 | 7.5 | 18.6, 17.4 | 14.3, 12.9 |
| 29 | 11.0 | 7.9 | 20.7, 22.6 | 14.3, 15.7 |
| 22 | 10.8 | 7.7 | 18.9, 20.1 | 13.0, 13.5 |

Field laptop, real 60 Hz screen, new pictures a second, median / lowest
tenth, two interleaved runs each:

| level | off | lean + restore |
| --- | --- | --- |
| 27 | 35.0 / 27.8; 53.2 / 35.4 | 57.8 / 38.7; 59.5 / 38.1 |
| 29 | 38.1 / 27.0; 38.2 / 28.4 | 52.5 / 32.0; 42.2 / 29.0 |
| 22 | 46.2 / 34.8; 41.9 / 33.6 | 51.4 / 33.5; 54.0 / 31.8 |

A job about 30% lighter on both machines; the worker is no longer saturated
on the laptop (75-86% busy against 93-96%), the medians come near 60 on THE
REAPER and rise on 22 and 29. The lowest tenth hardly moves: those seconds
are no longer bound by the worker (in the earlier per-second rows they come
with the CPU at 2.1-2.5 GHz under the 15 W package limit and the emulation
thread at 70-80%). Goal not yet met in every second.

## 6c. The remaining dips: placement and the fallback wait (MEASURED)

User, 2026-11-18: "keep pushing, tackle the remaining dips too".

- What the dip seconds are (field laptop, per-second census rows, lean): the
  heavy views (walking, firing) with the worker saturated again (93-98%),
  the emulation thread at 80-90%, the render thread at 40-55% (10-15% in
  good seconds; 62% of its samples waiting, the rest drawing the redraw
  streams: GP0 execution and the GL batcher, `sudo eu-stack`), and the CPU at
  2.0-2.4 GHz under the 15 W package limit (3.1 GHz in good seconds).
- D23Y4's placement puts the emulation thread on one core and every redraw
  thread (worker and render thread) on the other core's two CPUs, so in those
  seconds the worker and the render thread shared one core. Developer
  `PSX_PLACEMENT_RENDER=side|emu|all` (render_thread.c; default `side`, as
  before) places the render threads alone. Field laptop, real screen,
  BALANCED 60, lean + restore + tracked publish, census lowest tenth of new
  pictures (22 / 27 / 29): `side` 30-38 / 44-53 / 31-33; `emu` 56-60 /
  56-59 / 57-58 but the governor fell back for 18-19 s in two of six runs
  (the emulation thread slows with the render thread on its sibling, as
  D23Y4 found for workers); `all` 46-59 / 55-59 / 37-51, one 4 s and one 12 s
  fallback. First person with `all`: 22 58-60 / 35-57, 27 57-59 / 40,
  29 47-51 / 32-35 (with `side`: 53-59 / 30-33, 43-47 / 23-26, 36-42 /
  25-27).
- The fallback (D23Y1): three slow seconds at the last pace step drop the
  redraws for `floor_wait` seconds of gameplay, 120 at first. One heavy patch
  cost two minutes of the original presentation although the emulation was
  back in seconds. A retry that falls behind in its first 20 s returns at
  once, so a short first wait costs little; the developer
  `DNTTK_REPLAY_FLOOR_WAIT=15` (existing) was used below.
- D23Y10's tracked publish (`PSX_PUBLISH_TRACK=1`) with lean: the emulation
  thread 1-3 points lower, within the spread; kept in the bundle.
- Not changed: the clear lens (D10F1, up to about ten of the game's own
  collision queries a camera update, 9.7% of the emulation thread on level
  29: gameplay camera logic); the object path on hidden redraw-only live
  frames (D17AR3 skips only the world; the object hook feeds first-person and
  light state, so a skip needs an audit first); the render thread's drawing
  (real per-triangle work).

Campaign census, field laptop, real 60 Hz screen, BALANCED 60, third
person, all 21 levels, with `PSX_RW_LEAN=1 PSX_RW_RESTORE=1
PSX_PUBLISH_TRACK=1 PSX_PLACEMENT_RENDER=all DNTTK_REPLAY_FLOOR_WAIT=15`:
every level's median 59.5-60.0 new pictures, lowest tenth 52.8-59.8 except
THE REAPER 41.6 (presents counted in a fallback, as in note 509 section 3);
fallbacks of 2-14 s on six levels, 70 seconds of 749 without redraws in all
(note 509: 249 of 744 and level medians down to 30). All seconds, the
fallback ones counted at the game's own new images: median 59.9, lowest
tenth 30.7 (note 509: 47.5 / 29.5); the lowest tenth is the fallback seconds.

## 6d. THE REAPER and first person on level 29 (MEASURED)

User, 2026-11-18: "keep pushing, fix the reaper dips and first person 29".

- What limits them (field laptop, per-second rows, the bundle of 6c): THE
  REAPER's slow seconds come with the game itself at about 19 game frames a
  second and the workers finishing 44-58 jobs of which 25-47 are shown;
  first person on 29 has the worker (95%), the emulation thread (85-90%)
  and the render thread (55-63%) all busy at 2.0-2.4 GHz.
- Admission (tried, removed): not queuing a redraw that the workers'
  recent job time says will miss its present by more than one present. Within
  the spread on 27 and 29 in both views; fallbacks in both arms.
- Hidden objects (developer `DNTTK_REDRAW_ONLY_OBJECTS=1`, near_clip.cpp
  `object()`): on a redraw-only live frame (never shown; D17AR3 part 2), a
  world object keeps every decision the hook makes (the faces left to the
  original, the copy or null mesh it is given) and emits nothing: no
  packets, coplanar ranks, smooth normals, model shades or prop glows, as the
  world's pieces already do. What the skipped work writes is packets in the
  host arena and links in the game's own world ordering table and its slot
  bitmap (0x800d27a0 / 0x800d26a0), which a hidden frame's GPU lists drop.
  Duke's own layers (the weapon, the hand, the body) still draw: the
  first-person draw records the barrel tip shots launch from (weapon_aim.cpp
  `drawn_barrel`) and their native draws advance pose counters. Workers are
  untouched (`live_frame_redraw_only` is false in a replay). The emulation
  thread: desktop 48.1-48.9 -> 44.4-44.5% (levels 27, 29); field laptop 4-13
  points lower. THE REAPER third person, lowest tenth 55.0-57.7 -> 58.1-59.1;
  first person 27 about the same (42-47); first person 29 within the spread
  and without fallbacks in two runs (26.7-29.7 against 33.8 and an 18 s
  fallback).
- Clip planes: the two guard ratios once a polygon instead of at every call
  (`plane_g`, bit 128); exact, within the spread on the laptop.
- Exactness: the in-worker check in first person (27, 29), 2,432 jobs, 0
  differing; the exported patch reproduces every touched framework file.

First person on level 29 stays short (median 47-58, lowest tenth 26-35): in
those seconds the two cores are full at 2.3 GHz and what is left is real work
(the game, the world walk's clipping and packets, the render thread's
triangles). The smaller exact cuts that remain are below what these runs can
resolve (the same configuration moves 5-10 new pictures between runs with
the package clock).

## 7. Closeout: the bundle as the Modernized default

User playtest on the field laptop, 2026-11-18, with the bundle (a desktop
launcher with `PSX_RW_LEAN=1 PSX_RW_RESTORE=1 PSX_PUBLISH_TRACK=1
PSX_PLACEMENT_RENDER=all DNTTK_REDRAW_ONLY_OBJECTS=1
DNTTK_REPLAY_FLOOR_WAIT=15`): "the only slowdowns on the reaper is if he
hits you. i get like a massive framerate drop there". That session's log
(GOD MODE on, first person, weapon 7): two "workers behind" switch-offs with
the render thread still drawing 779 and 238 finished redraws at their
presents, and one 15 s fallback. Reproductions on the laptop's real screen
(the census route with 88-99 REAPER hits in first and third person,
`hurt=` test hits, then a live recorder while the user played): no such
drop; the user could not repeat it either ("i cant replicate the issue").
Recorded as a possible recurrence, not a confirmed cause.

User, 2026-11-18: "lock in the way the game plays right now, and then close
us out. im impressed with the performance, if it ever happens in the future
ill just let you know". `run.py` now sets the six switches as Modernized
defaults (`MODERNIZED_REDRAW_DEFAULTS`, with `setdefault`, so an explicit
value such as `PSX_RW_LEAN=0` still wins); Vanilla gets none of them
(checked in the launched process's environment). The developer switches,
checks and the bisect mask stay. GAME_MANUAL's fallback schedule now reads
15 s, then 30 s, one, two, four and at most eight minutes.

## 8. Reproduce

`recomp/analysis/d23ak/` (local): `lprof.sh` (laptop profiles, one level a
launch), `lab.sh` (laptop A/B; env RUNS "name:EXTRA_ENV"), `stk.py`
(job profile), `sub.py` (inside one function), `callers.py` (who calls a
function), `jobsum.py` (pictures, jobs a second and ms a job from a census
run; `census.py` now records the worker's job counters per second),
`dab.sh` (desktop A/B), `perf.sh` (desktop `perf` of the worker), `laptop/`
(the runs). Exactness: census with `NOGATE=1` and `EXTRA_ENV=...,PSX_RW_LEAN=1,PSX_RW_LEAN_VERIFY=2`; d23y8 `vx.sh` with `CFG="LAB_PSX_RW_LEAN=1"`.
