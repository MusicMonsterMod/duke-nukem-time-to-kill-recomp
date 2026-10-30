# D17AR3 part 1 - Host visibility at 120 Hz: where the time goes

2026-10-30. First D17AR3 item, chosen by the user after the D17AR2 playtest:
"make host visibility silky smooth at 120 Hz (slots 10 and 2 drop to every
second refresh)". Host visibility is still behind `DNTTK_HOST_VISIBILITY=1` /
console `hostvis on`; the default switch, level 28 and the full regression
sweep remain for later D17AR3 parts. Status: part 1 accepted after the user's
playtest ("it's feeling really good ... especially on my absurd state save on
slot 2. that's actually really smooth now"); redraw-only live frames (end of
this note) approved as part 2.

Evidence is local under `recomp/analysis/d17ar3/` (`probe.py`, `timeline.py`,
`headroom.py`, `bench.py`, `check.py`, `cats.py`, `incl.py`; JSON and logs).
Every run used private copies of the D17AQ fixture card and savestates,
offscreen video, dummy audio and private debug ports, at the player's internal
scale 4 for 120 Hz runs, and hash-checked the player's saves and profile
before and after. Fixture slot 10 is the jetpack view over the castle, slot 2
the moat. Host: i7-5960X (8 cores, 3 GHz), GTX 1080 Ti; no other load.

## How 120 Hz works here (summary)

At `auto` (120 Hz) every presented image is a redraw: three worker processes
re-run the game's view composition for each present with the newest mouse
look (D17B late camera) and record its GP0 stream; the emulation thread then
feeds that stream through the GPU frontend into the OpenGL renderer. The live
game frame is still emulated and drawn, but only about 1% of presents show it
(when no redraw is ready). The emulation thread therefore carries: the game
itself (about 30 frames/s), the live frame's host hooks and GPU drawing, and
one redraw feed per present (120 a second).

## Findings

1. **Most redraws at slot 10 were thrown away.** A worker records each redraw
   into a 192K-word buffer. Host precise vertices take ten words each, so
   host-visibility frames (up to about 220K words at the castle) overflowed and
   the redraw failed: about 8% of redraws while turning, and in the loaded
   slot-10 view all of them (3919 of 3923 in 30 s). The presenter then showed
   the live 30 fps image again at every refresh, which counted as 120 presents
   but was 30 distinct images.
2. **Once redraws succeed, the emulation thread is the limit.** It falls
   behind real time and the D23E/D23H governor drops presents to every second
   refresh. In the heaviest view (slot 10, loaded view, forced full rate) host
   visibility needs about 128% of the thread. Wall-clock attribution there:
   redraw feeds 35-39%, CPU timing machinery 14-16%, TTK mesh hooks 8-11%,
   dispatch 9-11%, live GPU drawing 9%, guest code 8%, host world pass 3-7%,
   live presentation 3-5%.
3. **The precise-vertex table was the thread's largest single cost.** Each
   host vertex was stored in, and looked up from, an 8-way probed hash spread
   over three arrays (about three cache misses per vertex), in every feed and
   in the live frame.
4. **The host world pass repeated view-independent work.** Per piece (48 us at
   slot 10): coplanar-face search 13 us, an unused mesh copy 2.5 us, drawing
   23 us, the rest projection and setup.
5. `water_live()` read the environment on every water query (about 12 ms a
   second over the moat).
6. The redraw feed is CPU-bound in the GPU frontend (about 0.85 us per
   primitive, spread over primitive setup, widescreen classification, VRAM
   copy-on-write bookkeeping and the driver); GL batches are few (about five
   per redraw). NVIDIA's threaded driver mode (`__GL_THREADED_OPTIMIZATIONS=1`)
   lowered feed time in light views but not in the heaviest one, so it was not
   adopted.

## Changes

- **Redraw buffer** (`render_worker.c`): 1M words per job (pages are touched
  only as far as a stream reaches); worker stats report `overflows`.
- **Direct precise-vertex table** (`gpu.c`): packets in the GPU DMA aperture,
  where title plugins write theirs, are indexed by word offset (no probing, no
  eviction). Other addresses keep the probed table. `PSX_HOST_PV_DIRECT=0`
  (developer) restores the old table; `PSX_HOST_PV_CHECK=1` stores in both and
  compares every lookup.
- **Coplanar pairs cached per mesh** (`near_clip.cpp`): world pieces do not
  move (D17AR2 proof 2), so which faces of a mesh are coplanar and overlap is
  found once in the mesh's exact integer coordinates and kept, checked against
  a hash of the vertex positions and polygon list that `world()` folds in as it
  reads them. Only the view-dependent ranking runs per frame. Also used with
  host visibility off. `DNTTK_COPLANAR_CACHE=0` (developer) repeats the search
  every frame; `DNTTK_COPLANAR_CHECK=1` runs both and counts differences.
- **No mesh copy when every face is host-drawn** (the original then gets a null
  mesh and never reads it); clip shapes without heap allocation.
- Environment switches cached (`DNTTK_WATER_CLIP`, `DNTTK_SKY_PAINTER`,
  `DNTTK_SKY_SOURCE_ORDER`); widescreen classification reads guest RAM
  directly and display info in place (no measurable change; kept as harmless).
- Developer tools: `PSX_PROF_WALL=1` (wall-clock sampling, counts blocked
  time), `PSX_PROF_STACKS=FILE` (call stacks for inclusive attribution),
  `PSX_PROF_CALLERS_SKIP=N`, `DNTTK_REPLAY_SHED=0` (never shed presents, to
  measure a scene at full rate). Thread CPU-clock profiles undercount at short
  intervals (they fire at the kernel tick); compare their shares, not totals.

## Checks

- Coplanar cache: 67,735 meshes checked over all 10 fixture slots with turns,
  0 differences in rank, sort depth or order.
- Direct table: 46 million lookups at 60 and 120 Hz over all fixture slots,
  identical to the probed table (no hit lost, none gained).
- D17AR2 sweep (all 21 selectable levels, 84 matched views, host on/off): one
  view above noise, level 26 view 1, the far opening D17AR2 already filled; no
  pieces handed back, no refusals, no piece data changes.
- `ttk-near-test` passes. The runtime patch was re-exported and applies to the
  pinned framework, reproducing the working tree exactly.

## Results

Same conditions, accepted D17AR2 build ("before") against this build, normal
pacing as the player runs it, 120 Hz, internal scale 4, 30 s, seconds that
held full rate out of 24:

| Route | Host visibility | Before | After | Failed redraws before / after | Present p95 before / after |
| --- | --- | --- | --- | --- | --- |
| Slot 10, turning | on | 10 | 21 | 574 / 0 | 15.5 / 9.2 ms |
| Slot 10, turning | off | 20 | 24 | 0 / 0 | 9.0 / 9.1 ms |
| Slot 2, turning | on | 3 | 21 | 76 / 1 | 18.1 / 9.9 ms |
| Slot 2, turning | off | 12 | 20 | 0 / 0 | 9.2 / 8.9 ms |
| Slot 10, standing at the loaded view | on | 24 (30 distinct images/s, every redraw failed) | 1 (60 distinct images/s) | 3919 / 0 | |
| Slot 2, standing at the loaded view | on | 2 | 7 | 1 / 0 | |

The first 3-4 seconds after a load always run at every second refresh (load
and warm-up), so 21 of 24 means full rate from then on. Host world pass cost
per piece: 48 to 30 us (slot 10: 6.0 to 3.7 ms a game frame); the worker
redraws run the same code.

## What remains, and the option that closes it

Standing in the heaviest views (slot 10's loaded castle view, parts of slot 2)
the thread still needs 110-128% of a core with host visibility, so presents
fall to every second refresh there. The remaining costs are spread thinly; no
further single change measured more than a few percent, except one.

At 120 Hz every present is a redraw, so the live frame's drawing is almost
never shown, yet the thread still emits its host world and draws all of it.
A throwaway experiment (not in the build) that skipped both in live frames
brought the heaviest views to full rate at forced 120 Hz: slot 10 60.0 game
fields/s (from about 47), slot 2 60.7 with 1.5 ms to spare. Skipping only the
GPU drawing gave about half of that.

Making that real ("redraw-only live frames") is a design change, not a tweak:
it must keep a correct image whenever no redraw is ready (repeat the last
redraw instead of the live image), stay off at 60 Hz, in menus, loading, FMV,
split screen and Vanilla, survive worker restarts, and keep anything that
reads the live frame from VRAM (savestate thumbnails, screenshots, pause
backdrops, any in-game effect) correct. The user approved it as part 2.

## Other items checked

- The launch environment: `run.py` passes `DNTTK_HOST_VISIBILITY` through to
  the game unchanged; launched that way, all four processes log
  `[TTK hostvis] host world visibility on`. The user's earlier launch could not
  be reproduced from its session log (which does not record the environment);
  the default switch (later in D17AR3) makes it moot for players.
- Presents with no finished redraw ("no job") still occur a few times a second
  where the thread has under 0.5 ms of slack; they follow the same capacity
  limit.
